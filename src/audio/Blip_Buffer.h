
// Buffer of sound samples into which band-limited waveforms can be synthesized
// using Blip_Wave or Blip_Synth.

// Blip_Buffer 0.3.0. Copyright (C) 2003-2004 Shay Green. GNU GPL license.

#ifndef BLIP_BUFFER_H
#define BLIP_BUFFER_H

#include "blargg_common.h"

class Blip_Reader;

// Source time unit.
typedef long blip_time_t;

// Type of sample produced. Signed 16-bit format.
typedef BOOST::int16_t blip_sample_t;

class Blip_Buffer {
public:
	// Construct an empty buffer.
	Blip_Buffer();
	~Blip_Buffer();
	
	// Set output sample rate and buffer length in milliseconds (1/1000 sec),
	// then clear buffer. If length not specified, make as large as possible.
	// If there is insufficient memory for the buffer, set the buffer length
	// to 0 and return false (or propagate exception if compiler supports it).
	bool sample_rate( long samples_per_sec );
	bool sample_rate( long samples_per_sec, int msec_length );
	
	// Current output sample rate
	long sample_rate() const;
	
	// Number of source time units per second.
	void clock_rate( long );
	long clock_rate() const;
	
	// Set frequency at which high-pass filter attenuation passes -3dB.
	void bass_freq( int frequency );
	
	// Remove any available samples and clear buffer to silence.
	void clear();
	
	
	// Number of raw samples that can be mixed within frame of specified duration
	size_t count_samples( blip_time_t duration ) const;
	
	// Mix 'count' samples from 'buf' into buffer.
	void mix_samples( const blip_sample_t* buf, size_t count );
	
	
	// End current time frame of specified duration and make its samples available
	// (along with any still-unread samples) for reading with read_samples(). Begin
	// a new time frame at the end of the current frame. All transitions must have
	// been added before the end of the current frame.
	void end_frame( blip_time_t );
	
	// Number of samples available for reading with read_samples().
	size_t samples_avail() const;
	
	// Read at most 'max_samples' out of buffer into 'dest', removing them from from
	// the buffer. Return number of samples actually read and removed. If stereo is
	// true, increment 'dest' one extra time after writing each sample, to allow
	// easy interleving of two channels into a stereo output buffer.
	size_t read_samples( blip_sample_t* dest, size_t max_samples, bool stereo = false );
	
	// Remove 'count' samples from those waiting to be read.
	void remove_samples( size_t count );
	
	// Number of samples delay from synthesis to samples read out.
	int output_latency() const;
	
	// not documented yet
	
	void remove_silence( size_t count );
	
	typedef unsigned long resampled_time_t;
	
	resampled_time_t resampled_time( blip_time_t t ) const {
		return t * resampled_time_t (factor_) + offset_;
	}
	
	resampled_time_t resampled_duration( int t ) const {
		return t * resampled_time_t (factor_);
	}
	
private:
	// noncopyable
	Blip_Buffer( const Blip_Buffer& );
	Blip_Buffer& operator = ( const Blip_Buffer& );

	// Don't use the following members. They are public only for technical reasons.
	public:
		enum { widest_impulse_ = 24 };
		typedef BOOST::uint16_t buf_t_;
		
		unsigned long factor_;
		resampled_time_t offset_;
		buf_t_* buffer_;
		unsigned buffer_size_;
	private:
		long reader_accum;
		int bass_shift;
		long samples_per_sec;
		long clocks_per_sec;
		int bass_freq_;
		
		enum { accum_fract = 15 }; // less than 16 to give extra sample range
		enum { sample_offset = 0x7F7F }; // repeated byte allows memset to clear buffer
		
		friend class Blip_Reader;
};

// Low-pass equalization parameters (see notes.txt)
class blip_eq_t {
public:
	blip_eq_t( double treble = 0 );
	blip_eq_t( double treble, long cutoff, long sample_rate );
private:
	double treble;
	long cutoff;
	long sample_rate;
	friend class Blip_Impulse_;
};

// not documented yet (see Stereo_Buffer.cpp for an example of its use)
class Blip_Reader {
	const Blip_Buffer::buf_t_* buf;
	long accum;
	#ifdef __MWERKS__
	void operator = ( struct foobar ); // helps optimizer
	#endif
public:
	// avoid anything which might cause optimizer to put object in memory
	
	int begin( Blip_Buffer& blip_buf ) {
		buf = blip_buf.buffer_;
		accum = blip_buf.reader_accum;
		return blip_buf.bass_shift;
	}
	
	void end( Blip_Buffer& blip_buf ) {
		blip_buf.reader_accum = accum;
	}
	
	int read() const {
		return accum >> Blip_Buffer::accum_fract;
	}
	
	void next( int bass_shift = 9 ) {
		accum -= accum >> bass_shift;
		accum += ((long) *buf++ - Blip_Buffer::sample_offset) << Blip_Buffer::accum_fract;
	}
};



// End of public interface
	
	#define BLIP_BUFFER_ACCURACY 16
	
	const int blip_res_bits_ = 5;
	
	typedef BOOST::uint32_t blip_pair_t_;

	class Blip_Impulse_ {
		typedef BOOST::uint16_t imp_t;
		
		blip_eq_t eq;
		double  volume_unit_;
		imp_t*  impulses;
		imp_t*  impulse;
		int     width;
		int     fine_bits;
		int     res;
		bool    generate;
		
		void fine_volume_unit();
		void scale_impulse( int unit, imp_t* ) const;
	public:
		Blip_Buffer*    buf;
		BOOST::uint32_t offset;
		
		void init( blip_pair_t_* impulses, int width, int res, int fine_bits = 0 );
		void volume_unit( double );
		void treble_eq( const blip_eq_t& );
	};
	
	inline blip_eq_t::blip_eq_t( double t ) :
			treble( t ), cutoff( 0 ), sample_rate( 44100 ) {
	}
	
	inline blip_eq_t::blip_eq_t( double t, long c, long sr ) :
			treble( t ), cutoff( c ), sample_rate( sr ) {
	}
	
	inline size_t Blip_Buffer::samples_avail() const {
		return offset_ >> BLIP_BUFFER_ACCURACY;
	}

	inline long Blip_Buffer::sample_rate() const {
		return samples_per_sec;
	}
	
	inline void Blip_Buffer::end_frame( blip_time_t t ) {
		offset_ += t * factor_;
		assert(( "Blip_Buffer::end_frame(): Frame went past end of buffer",
				samples_avail() <= buffer_size_ ));
	}

	inline void Blip_Buffer::remove_silence( size_t count ) {
		assert(( "Blip_Buffer::remove_silence(): Not that many samples available to remove",
				count <= samples_avail() ));
		offset_ -= resampled_time_t (count) << BLIP_BUFFER_ACCURACY;
	}

	inline bool Blip_Buffer::sample_rate( long samples_per_sec ) {
		return sample_rate( samples_per_sec, 0 );
	}
	
	inline int Blip_Buffer::output_latency() const {
		return widest_impulse_ / 2;
	}

	inline long Blip_Buffer::clock_rate() const {
		return clocks_per_sec;
	}
	
	#include "Blip_Synth.h"

#endif

