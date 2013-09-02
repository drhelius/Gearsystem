
// Boost substitute. For full boost library see http://boost.org

#ifndef BOOST_CSTDINT_HPP
#define BOOST_CSTDINT_HPP

#include <limits.h>

#if UCHAR_MAX != 0xFF
#   error "No suitable 8-bit type available"
#endif

typedef unsigned char   uint8_t;
typedef signed char     int8_t;

#if USHRT_MAX != 0xFFFF
#   error "No suitable 16-bit type available"
#endif

typedef short           int16_t;
typedef unsigned short  uint16_t;
typedef int             int32_t;
typedef unsigned int    uint32_t;

#endif

