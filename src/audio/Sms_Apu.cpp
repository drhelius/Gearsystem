
// Sms_Snd_Emu 0.1.1. http://www.slack.net/~ant/nes-emu/

#include "Sms_Apu.h"

/* Library Copyright (C) 2003-2004 Shay Green. Sms_Snd_Emu is free
software; you can redistribute it and/or modify it under the terms of the
GNU General Public License as published by the Free Software Foundation;
either version 2 of the License, or (at your option) any later version.
Sms_Snd_Emu is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
more details. You should have received a copy of the GNU General Public
License along with Sms_Snd_Emu; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA */

// Sms_Osc

Sms_Osc::Sms_Osc() {
	output = NULL;
	output_select = 3;
	outputs [0] = NULL; // always stays NULL
	outputs [1] = NULL;
	outputs [2] = NULL;
	outputs [3] = NULL;
}

void Sms_Osc::reset() {
	delay = 0;
	last_amp = 0;
	volume = 0;
	output_select = 3;
	output = outputs [3];
}

// Sms_Square

Sms_Square::Sms_Square() {
}

void Sms_Square::reset() {
	period = 0;
	phase = 0;
	Sms_Osc::reset();
}

void Sms_Square::run( sms_time_t time, sms_time_t end_time )
{
	if ( !volume || period < 100 ) { // ignore 16kHz and higher
		if ( last_amp ) {
			synth->offset( time, -last_amp, output );
			last_amp = 0;
		}
		time += delay;
		if ( !period ) {
			time = end_time;
		}
		else if ( time < end_time ) {
			// keep calculating phase
			int count = (end_time - time + period - 1) / period;
			phase = (phase + count) & 1;
			time += count * period;
		}
	}
	else
	{
		int amp = phase ? volume : -volume;
		if ( amp != last_amp ) {
			synth->offset( time, amp - last_amp, output );
			last_amp = amp;
		}
		
		time += delay;
		if ( time < end_time )
		{
			Blip_Buffer* const output = this->output;
			amp *= 2;
			do {
				amp = -amp; // amp always alternates
				synth->offset_inline( time, amp, output );
				time += period;
				phase ^= 1;
			}
			while ( time < end_time );
			this->last_amp = phase ? volume : -volume;
		}
	}
	delay = time - end_time;
}

// Sms_Noise

static const int noise_periods [3] = { 0x100, 0x200, 0x400 };

inline Sms_Noise::Sms_Noise() {
}

inline void Sms_Noise::reset() {
	period = &noise_periods [0];
	shifter = 0x8000;
	tap = 12;
	Sms_Osc::reset();
}

void Sms_Noise::run( sms_time_t time, sms_time_t end_time )
{
	int period = *this->period * 2;
	if ( !volume ) {
		if ( last_amp ) {
			synth.offset( time, -last_amp, output );
			last_amp = 0;
		}
		delay = 0;
	}
	else
	{
		int amp = (shifter & 1) ? -volume : volume;
		if ( !period )
			period = 16;
		if ( amp != last_amp ) {
			synth.offset( time, amp - last_amp, output );
			last_amp = amp;
		}
		
		time += delay;
		if ( time < end_time )
		{
			Blip_Buffer* const output = this->output;
			unsigned shifter = this->shifter;
			amp *= 2;
			
			do {
				int changed = 1 & (shifter ^ (shifter >> 1));
				shifter = (((shifter << 15) ^ (shifter << tap)) & 0x8000) | (shifter >> 1);
				if ( changed ) { // prev and next bits differ
					amp = -amp;
					synth.offset_inline( time, amp, output );
				}
				time += period;
			}
			while ( time < end_time );
			
			this->shifter = shifter;
			this->last_amp = amp >> 1;
		}
		delay = time - end_time;
	}
}

// Sms_Apu

Sms_Apu::Sms_Apu()
{
	for ( int i = 0; i < 3; i++ ) {
		squares [i].synth = &square_synth;
		oscs [i] = &squares [i];
	}
	oscs [3] = &noise;
	
	volume( 1.0 );
	reset();
}

Sms_Apu::~Sms_Apu() {
}

void Sms_Apu::treble_eq( const blip_eq_t& eq ) {
	square_synth.treble_eq( eq );
	noise.synth.treble_eq( eq );
}

void Sms_Apu::volume( double vol ) {
	vol *= 0.85 / osc_count;
	square_synth.volume( vol );
	noise.synth.volume( vol );
}

void Sms_Apu::output( Blip_Buffer* center, Blip_Buffer* left, Blip_Buffer* right ) {
	for ( int i = 0; i < osc_count; i++ )
		osc_output( i, center, left, right );
}

void Sms_Apu::osc_output( int index, Blip_Buffer* center, Blip_Buffer* left,
		Blip_Buffer* right )
{
	assert(( "Sms_Apu::osc_output(): Index out of range", 0 <= index && index < osc_count ));
	
	Sms_Osc& osc = *oscs [index];
	if ( center && !left && !right ) {
		// mono
		left = center;
		right = center;
	}
	else {
		// must be silenced or stereo
		assert( (!left && !right) || (left && right) );
	}
	osc.outputs [1] = right;
	osc.outputs [2] = left;
	osc.outputs [3] = center;
	osc.output = osc.outputs [osc.output_select];
}

void Sms_Apu::reset() {
	stereo_found = false;
	last_time = 0;
	latch = 0;
	
	squares [0].reset();
	squares [1].reset();
	squares [2].reset();
	noise.reset();
}

void Sms_Apu::run_until( sms_time_t end_time )
{
	if ( end_time == last_time )
		return;
	assert(( "Sms_Apu::run_until(): End time is before current time", last_time < end_time ));
	
	// run oscillators
	for ( int i = 0; i < osc_count; ++i ) {
		Sms_Osc& osc = *oscs [i];
		if ( osc.output ) {
			if ( osc.output != osc.outputs [3] )
				stereo_found = true; // playing on side output
			osc.run( last_time, end_time );
		}
	}
	
	last_time = end_time;
}

bool Sms_Apu::end_frame( sms_time_t end_time )
{
	run_until( end_time );
	last_time = 0;
	
	bool result = stereo_found;
	stereo_found = false;
	return result;
}

void Sms_Apu::write_ggstereo( sms_time_t time, int data )
{
	assert(( "Sms_Apu::write_ggstereo(): Data out of range",  0 <= data && data <= 0xFF ));
	
	run_until( time );
	
	// left/right assignments
	for ( int i = 0; i < osc_count; i++ ) {
		Sms_Osc& osc = *oscs [i];
		int flags = data >> i;
		Blip_Buffer* old_output = osc.output;
		osc.output_select = ((flags >> 3) & 2) | (flags & 1);
		osc.output = osc.outputs [osc.output_select];
		if ( osc.output != old_output && osc.last_amp ) {
			if ( old_output )
				square_synth.offset( time, -osc.last_amp, old_output );
			osc.last_amp = 0;
		}
	}
}

static const char volumes [16] = {
	// volumes [i] = 64 * pow( 1.26, 15 - i ) / pow( 1.26, 15 )
	64, 50, 39, 31, 24, 19, 15, 12, 9, 7, 5, 4, 3, 2, 1, 0
};

void Sms_Apu::write_data( sms_time_t time, int data )
{
	assert(( "Sms_Apu::write_data(): Data out of range",  0 <= data && data <= 0xFF ));
	
	run_until( time );
	
	if ( data & 0x80 )
		latch = data;
	
	int index = (latch >> 5) & 3;
	if ( latch & 0x10 ) {
		// volume
		oscs [index]->volume = volumes [data & 15];
	}
	else if ( index < 3 ) {
		// square period
		Sms_Square& sq = squares [index];
		if ( data & 0x80 )
			sq.period = (sq.period & ~0xff) | ((data << 4) & 0xff);
		else
			sq.period = (sq.period & 0xff) | ((data << 8) & 0x3f00);
	}
	else {
		// noise period/mode
		int select = data & 3;
		if ( select < 3 )
			noise.period = &noise_periods [select];
		else
			noise.period = &squares [2].period;
		
		noise.tap = (data & 0x04) ? 12 : 16; // 16 disables tap
		noise.shifter = 0x8000;
	}
}

