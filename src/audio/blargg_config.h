// $package user configuration file. Don't replace when updating library.

#ifndef BLARGG_CONFIG_H
#define BLARGG_CONFIG_H

// Uncomment to enable platform-specific (and possibly non-portable) optimizations.
//#define BLARGG_NONPORTABLE 1

// Uncomment if automatic byte-order determination doesn't work
//#define BLARGG_BIG_ENDIAN 1

// Uncomment to use zlib for transparent decompression of gzipped files
//#define HAVE_ZLIB_H

// Uncomment if you get errors in the bool section of blargg_common.h
//#define BLARGG_COMPILER_HAS_BOOL 1

// Uncomment to disable out-of-memory exceptions
//#include <memory>
//#define BLARGG_NEW new (std::nothrow)

// Use standard config.h if present
#ifdef HAVE_CONFIG_H
	#include "config.h"
#endif

#endif
