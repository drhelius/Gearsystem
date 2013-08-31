
// Simple AIFF sound file writer for recording sound output

#ifndef SOUND_WRITER_HPP
#define SOUND_WRITER_HPP

#include <stddef.h>
#include <stdio.h>

typedef short sample_t;

sample_t const full_sample = 0x7FFF;

class Sound_Writer {
	FILE*   file;
	long    sample_count;
	int     chan_count_;
	char    filename [256];
public:
	// Create sound file of given filename and sampling rate (in Hz)
	Sound_Writer( long rate, char const* filename = "out.aif" );
	~Sound_Writer();
	
	void chan_count( int n )        { chan_count_ = n; }
	void stereo( int s )            { chan_count_ = s ? 2 : 1; }
	
	// Append samples to file
	void write( sample_t const*, size_t sample_count );
};

#endif

