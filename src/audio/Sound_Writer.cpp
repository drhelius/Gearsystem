
#include "Sound_Writer.hpp"

#include <string.h>

struct aiff_header_t {
	char form [4];
	char form_size [4];
	char aiff [4];
	char comm [4];
	char comm_size [4];
	char chan_count [2];
	char frame_count [4];
	char bits [2];
	char rate_scale [2];
	char rate [2];
	char zero [6];
	char none [4];
	char ssnd [4];
	char ssnd_size [4];
	char zero2 [8];
};

static void set_value( char h [4], long n ) {
	h [0] = char (n >> 24);
	h [1] = char (n >> 16);
	h [2] = char (n >> 8);
	h [3] = char (n);
}

static aiff_header_t aiff_header = {
	{ 'F','O','R','M' }, { 0 },
	{ 'A','I','F','F' },
	{ 'C','O','M','M' }, { 0, 0, 0, 22 },
	{ 0, 1 },
	{ 0 },
	{ 0, 16 },
	{ 0x40, 0x0f }, { 0 },
	{ 0 },
	{ 'N','O','N','E' },
	{ 'S', 'S', 'N', 'D' }, { 0 },
	{ 0 }
};

static char aiff_trailer [] = { 0, 0, 0, 0 };

Sound_Writer::Sound_Writer( long rate, const char* filename_in ) :
	file( fopen( filename_in, "wb" ) ),
	sample_count( 0 ),
	chan_count_( 1 )
{
	strncpy( filename, filename_in, sizeof filename );
	setvbuf( file, NULL, _IOFBF, 32 * 1024L );
	aiff_header.rate [0] = char (rate >> 9);
	aiff_header.rate [1] = char (rate >> 1);
	
	fwrite( &aiff_header, sizeof aiff_header, 1, file );
}

Sound_Writer::~Sound_Writer()
{
	if ( sample_count )
	{
		aiff_header.chan_count [1] = char (chan_count_);
		set_value( aiff_header.form_size, ftell( file ) - 8 );
		set_value( aiff_header.frame_count, sample_count / chan_count_ );
		set_value( aiff_header.ssnd_size, ftell( file ) - sizeof aiff_header );
		
		fwrite( &aiff_trailer, sizeof aiff_trailer, 1, file );
		
		fseek( file, 0, SEEK_SET );
		fwrite( &aiff_header, sizeof aiff_header, 1, file );
	}
	
	fclose( file );
	
	if ( !sample_count )
		remove( filename );
}

void Sound_Writer::write( const sample_t* in, size_t remain )
{
	const size_t buf_size = 1024;
	char buf [buf_size * 2];
	
	sample_count += remain;
	while ( remain )
	{
		size_t count = buf_size;
		if ( count > remain )
			count = remain;
		remain -= count;
		
		// convert to MSB first format for AIFF
		for ( size_t i = 0; i < count; i++ ) {
			int s = *in++;
			buf [i * 2] = char (s >> 8);
			buf [i * 2 + 1] = char (s & 0xFF);
		}
		fwrite( &buf, count * 2, 1, file );
	}
}

