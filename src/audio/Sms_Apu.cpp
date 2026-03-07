// Sms_Snd_Emu 0.1.4. http://www.slack.net/~ant/

#include "Sms_Apu.h"

/* Copyright (C) 2003-2006 Shay Green. This module is free software; you
can redistribute it and/or modify it under the terms of the GNU Lesser
General Public License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version. This
module is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
details. You should have received a copy of the GNU Lesser General Public
License along with this module; if not, write to the Free Software Foundation,
Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA */

#undef require
#define require( expr ) assert( expr )

// Sms_Osc

Sms_Osc::Sms_Osc()
{
	output = 0;
	outputs [0] = 0; // always stays NULL
	outputs [1] = 0;
	outputs [2] = 0;
	outputs [3] = 0;
	volume_reg = 0;
	mute = false;
	debug_buf = 0;
	debug_last_amp = 0;
}

void Sms_Osc::reset()
{
	delay = 0;
	last_amp = 0;
	volume = 0;
	volume_reg = 15;
	output_select = 3;
	output = outputs [3];
	debug_last_amp = 0;
}

// Sms_Square

inline void Sms_Square::reset(bool ti_chip)
{
	ti = ti_chip;
	period = 0;
	phase = 0;
	Sms_Osc::reset();
}

void Sms_Square::run( blip_time_t time, blip_time_t end_time )
{
    int effective_period = period ? period : (ti ? 0x400 : 0);
    int amp = volume;
    if ( effective_period > 128 )
        amp = amp << 1 & -phase;

    {
        int delta = amp - last_amp;
        if ( delta )
        {
            last_amp = amp;
            synth->offset( time, delta, output );
        }
        if ( debug_buf )
        {
            int dbg_delta = amp - debug_last_amp;
            if ( dbg_delta )
            {
                debug_last_amp = amp;
                synth->offset( time, dbg_delta, debug_buf );
            }
        }
    }

    time += delay;
    delay = 0;
    if ( effective_period )
    {
        if ( time < end_time )
        {
            if ( !volume || effective_period <= 128 ) // ignore 16kHz and higher
            {
                // keep calculating phase
                int count = (end_time - time + effective_period - 1) / effective_period;
                phase = (phase + count) & 1;
                time += count * effective_period;
            }
            else
            {
                Blip_Buffer* const output_ = this->output;
                Blip_Buffer* const dbg_buf = this->debug_buf;
                int delta = amp * 2 - volume * 2;
                do
                {
                    delta = -delta;
                    synth->offset_inline( time, delta, output_ );
                    if ( dbg_buf )
                        synth->offset_inline( time, delta, dbg_buf );
                    time += effective_period;
                }
                while ( time < end_time );

                last_amp = (delta >> 1) + volume;
                debug_last_amp = last_amp;
                phase = (delta >= 0);
            }
        }
        delay = time - end_time;
    }
}

// Sms_Noise

static int const noise_periods [3] = { 0x100, 0x200, 0x400 };

inline void Sms_Noise::reset(bool ti_chip)
{
	ti = ti_chip;
	period = &noise_periods [0];
	shifter = 0x8000;
	feedback = 0x9000;
	Sms_Osc::reset();
}

void Sms_Noise::run( blip_time_t time, blip_time_t end_time )
{
	int amp = (shifter & 1) ? 0 : volume;

	if (!ti)
	{
		amp *= 2;
	}

	{
		int delta = amp - last_amp;
		if ( delta )
		{
			last_amp = amp;
			synth.offset( time, delta, output );
		}
		if ( debug_buf )
		{
			int dbg_delta = amp - debug_last_amp;
			if ( dbg_delta )
			{
				debug_last_amp = amp;
				synth.offset( time, dbg_delta, debug_buf );
			}
		}
	}
	
	time += delay;
	if ( !volume )
		time = end_time;

	if ( time < end_time )
	{
		Blip_Buffer* const output_ = this->output;
		Blip_Buffer* const dbg_buf = this->debug_buf;
		unsigned shifter_ = this->shifter;
		int delta = (shifter_ & 1) ? (-volume * 2) : (volume * 2);
		int period_ = *this->period * 2;
		if ( !period_ )
			period_ = 16;

		do
		{
			int changed = shifter_ + 1;
			shifter_ = (feedback & (unsigned)(-(int)(shifter_ & 1))) ^ (shifter_ >> 1);
			if ( changed & 2 ) // true if bits 0 and 1 differ
			{
				amp = (shifter_ & 1) ? 0 : volume * 2;
				delta = -delta;
				synth.offset_inline( time, delta, output_ );
				if ( dbg_buf )
					synth.offset_inline( time, delta, dbg_buf );
				last_amp = amp;
				debug_last_amp = amp;
			}
			time += period_;
		}
		while ( time < end_time );
		
		this->shifter = shifter_;
		this->last_amp = (shifter_ & 1) ? 0 : volume * 2;
		this->debug_last_amp = this->last_amp;
	}
	delay = time - end_time;
}

// Sms_Apu

Sms_Apu::Sms_Apu()
{
	for ( int i = 0; i < 3; i++ )
	{
		squares [i].synth = &square_synth;
		oscs [i] = &squares [i];
	}
	oscs [3] = &noise;
	debug_enabled = false;
	ti_chip_mode = false;
	
	volume( 1.0 );
	reset(false);
}

Sms_Apu::~Sms_Apu()
{
}

void Sms_Apu::volume( double vol )
{
	vol *= 0.85 / (osc_count * 64 * 2);
	square_synth.volume( vol );
	noise.synth.volume( vol );
}

void Sms_Apu::treble_eq( const blip_eq_t& eq )
{
	square_synth.treble_eq( eq );
	noise.synth.treble_eq( eq );
}

void Sms_Apu::osc_output( int index, Blip_Buffer* center, Blip_Buffer* left, Blip_Buffer* right )
{
	require( (unsigned) index < osc_count );
	require( (center && left && right) || (!center && !left && !right) );
	Sms_Osc& osc = *oscs [index];
	osc.outputs [1] = right;
	osc.outputs [2] = left;
	osc.outputs [3] = center;
	osc.output = osc.outputs [osc.output_select];
}

void Sms_Apu::output( Blip_Buffer* center, Blip_Buffer* left, Blip_Buffer* right )
{
	for ( int i = 0; i < osc_count; i++ )
		osc_output( i, center, left, right );
}

void Sms_Apu::reset(bool ti_chip )
{
	last_time = 0;
	latch = 0;
	ggstereo_save = 0xFF;
	ti_chip_mode = ti_chip;
	
	unsigned feedback = ti_chip ? 0x0003 : 0x0009;
	int noise_width = ti_chip ? 15 : 16;

	// convert to "Galios configuration"
	looped_feedback = 1 << (noise_width - 1);
	noise_feedback  = 0;
	while ( noise_width-- )
	{
		noise_feedback = (noise_feedback << 1) | (feedback & 1);
		feedback >>= 1;
	}
	
	squares [0].reset(ti_chip);
	squares [1].reset(ti_chip);
	squares [2].reset(ti_chip);
	noise.reset(ti_chip);
}

void Sms_Apu::run_until( blip_time_t end_time )
{
	require( end_time >= last_time ); // end_time must not be before previous time
	
	if ( end_time > last_time )
	{
		// run oscillators
		for ( int i = 0; i < osc_count; ++i )
		{
			Sms_Osc& osc = *oscs [i];
			if ( osc.output && !osc.mute )
			{
				if ( i < 3 )
					squares [i].run( last_time, end_time );
				else
					noise.run( last_time, end_time );
			}
		}
		
		last_time = end_time;
	}
}

void Sms_Apu::end_frame( blip_time_t end_time )
{
	if ( end_time > last_time )
		run_until( end_time );
	
	assert( last_time >= end_time );
	last_time -= end_time;

	if ( debug_enabled )
	{
		for ( int i = 0; i < osc_count; i++ )
			debug_bufs [i].end_frame( end_time );
	}
}

void Sms_Apu::write_ggstereo( blip_time_t time, int data )
{
	require( (unsigned) data <= 0xFF );
	
	ggstereo_save = data;

	run_until( time );
	
	for ( int i = 0; i < osc_count; i++ )
	{
		Sms_Osc& osc = *oscs [i];
		int flags = data >> i;
		Blip_Buffer* old_output = osc.output;
		osc.output_select = (flags >> 3 & 2) | (flags & 1);
		osc.output = osc.outputs [osc.output_select];
		if ( osc.output != old_output && osc.last_amp )
		{
			if ( old_output )
			{
				square_synth.offset( time, -osc.last_amp, old_output );
			}
			osc.last_amp = 0;
		}
	}
}

// volumes [i] = 64 * pow( 1.26, 15 - i ) / pow( 1.26, 15 )
static unsigned char const volumes [16] = {
	64, 50, 39, 31, 24, 19, 15, 12, 9, 7, 5, 4, 3, 2, 1, 0
};

void Sms_Apu::write_data( blip_time_t time, int data )
{
	require( (unsigned) data <= 0xFF );
	
	run_until( time );
	
	if ( data & 0x80 )
		latch = data;
	
	int index = (latch >> 5) & 3;
	if ( latch & 0x10 )
	{
		oscs [index]->volume = volumes [data & 15];
		oscs [index]->volume_reg = data & 15;
	}
	else if ( index < 3 )
	{
		Sms_Square& sq = squares [index];
		if ( data & 0x80 )
			sq.period = (sq.period & 0xFF00) | (data << 4 & 0x00FF);
		else
			sq.period = (sq.period & 0x00FF) | (data << 8 & 0x3F00);
	}
	else
	{
		int select = data & 3;
		if ( select < 3 )
			noise.period = &noise_periods [select];
		else
			noise.period = &squares [2].period;
		
		noise.feedback = (data & 0x04) ? noise_feedback : looped_feedback;
		noise.shifter = 0x8000;
	}
}

Sms_Apu_State Sms_Apu::GetState()
{
	Sms_Apu_State state;

	for ( int i = 0; i < 3; i++ )
	{
		state.channels [i].volume = squares [i].volume;
		state.channels [i].volume_reg = squares [i].volume_reg;
		state.channels [i].period = squares [i].period;
		state.channels [i].phase = squares [i].phase;
		state.channels [i].output_select = squares [i].output_select;
		state.channels [i].mute = &squares [i].mute;
	}

	state.channels [3].volume = noise.volume;
	state.channels [3].volume_reg = noise.volume_reg;
	state.channels [3].period = noise.period ? *noise.period : 0;
	state.channels [3].phase = 0;
	state.channels [3].output_select = noise.output_select;
	state.channels [3].mute = &noise.mute;

	state.latch = latch;
	state.ggstereo = ggstereo_save;
	state.noise_shifter = noise.shifter;
	state.noise_feedback = noise.feedback;
	state.noise_white = (noise.feedback == noise_feedback);
	state.ti_chip = ti_chip_mode;

	if ( noise.period == &squares [2].period )
		state.noise_rate = 3;
	else if ( noise.period == &noise_periods [0] )
		state.noise_rate = 0;
	else if ( noise.period == &noise_periods [1] )
		state.noise_rate = 1;
	else
		state.noise_rate = 2;

	return state;
}

void Sms_Apu::init_debug_buffers( long sample_rate, long clock_rate )
{
	debug_enabled = true;
	double vol = 0.85 / (osc_count * 64 * 2);
	debug_synth.volume( vol );

	for ( int i = 0; i < osc_count; i++ )
	{
		debug_bufs [i].set_sample_rate( sample_rate );
		debug_bufs [i].clock_rate( clock_rate );
		debug_bufs [i].clear();
		oscs [i]->debug_buf = &debug_bufs [i];
	}
}

void Sms_Apu::disable_debug_buffers()
{
	debug_enabled = false;
	for ( int i = 0; i < osc_count; i++ )
		oscs [i]->debug_buf = 0;
}

bool Sms_Apu::is_debug_enabled() const
{
	return debug_enabled;
}

void Sms_Apu::read_debug_samples( blip_sample_t* out, int channel, long max_samples, long* count )
{
	if ( !debug_enabled || channel < 0 || channel >= osc_count )
	{
		*count = 0;
		return;
	}

	long avail = debug_bufs [channel].samples_avail();
	if ( avail > max_samples )
		avail = max_samples;

	if ( avail > 0 )
		debug_bufs [channel].read_samples( out, avail );

	*count = avail;
}
