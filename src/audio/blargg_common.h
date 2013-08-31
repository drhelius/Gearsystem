
// Common headers used by Shay Green's libraries

#ifndef BLARGG_COMMON_H
#define BLARGG_COMMON_H

#define BLARGG_COMPILER_HAS_BOOL 1

// allow prefix configuration file
#ifdef HAVE_CONFIG_H
	#include "config.h"
#endif

// check for boost availability
#include "boost/config.hpp"
#ifndef BOOST
	#define BOOST boost
#endif

// BLARGG_BIG_ENDIAN and BLARGG_LITTLE_ENDIAN
#if !defined (BLARGG_BIG_ENDIAN) && !defined (BLARGG_LITTLE_ENDIAN)
	#if defined (__powerc) || defined (macintosh)
		#define BLARGG_BIG_ENDIAN 1
	
	#elif defined (_MSC_VER) && defined (_M_IX86)
		#define BLARGG_LITTLE_ENDIAN 1
	#endif
#endif

// BLARGG_CPU_*
#if !defined (BLARGG_CPU_POWERPC) && !defined (BLARGG_CPU_X86)
	#if defined (__powerc)
		#define BLARGG_CPU_POWERPC 1
	
	#elif defined (_MSC_VER) && defined (_M_IX86)
		#define BLARGG_CPU_X86 1
	#endif
#endif

// BLARGG_MOST_PORTABLE (use most portable version when there's a choice)
#ifndef BLARGG_MOST_PORTABLE
	#define BLARGG_MOST_PORTABLE 0
#endif

// BLARGG_DEBUG
#ifndef BLARGG_DEBUG
	#ifdef NDEBUG
		#define BLARGG_DEBUG 0
	#else
		#define BLARGG_DEBUG 1
	#endif
#endif

// BOOST_STATIC_ASSERT( expr )
#include "boost/static_assert.hpp"

// BOOST::uint8_t, BOOST::int16_t, etc.
#include "boost/cstdint.hpp"

// bool, true, false
#ifndef BLARGG_COMPILER_HAS_BOOL
	#if !BOOST_MINIMAL
		#define BLARGG_COMPILER_HAS_BOOL 1
	
	#elif defined (__MWERKS__)
		#if !__option(bool)
			#define BLARGG_COMPILER_HAS_BOOL 0
		#endif
	
	#elif defined (_MSC_VER)
		#if _MSC_VER < 1100
			#define BLARGG_COMPILER_HAS_BOOL 0
		#endif
	
	#elif __cplusplus < 199711
		#define BLARGG_COMPILER_HAS_BOOL 0
	
	#endif
#endif
#ifndef BLARGG_COMPILER_HAS_BOOL
	#define BLARGG_COMPILER_HAS_BOOL 1
#endif

#if !BLARGG_COMPILER_HAS_BOOL
	typedef int bool;
	const bool true  = 1;
	const bool false = 0;
#endif

#include <stddef.h>
#include <assert.h>

#endif

