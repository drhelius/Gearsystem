
// Boost substitute. For full boost library see http://boost.org

#ifndef BOOST_ARRAY_HPP
#define BOOST_ARRAY_HPP

#include <stddef.h>

template<class T,size_t N>
struct array
{
	T elems [N];
	
	size_t size() const     { return N; }
	
	T      * begin()        { return elems; }
	const T* begin() const  { return elems; }
	
	T      * end()          { return elems + N; }
	const T* end() const    { return elems + N; }
	
	T& operator [] ( size_t i ) {
		assert( i < N );
		return elems [i];
	}
	
	const T& operator [] ( size_t i ) const {
		assert( i < N );
		return elems [i];
	}
	
	void assign( const T& value ) {
		for ( size_t i = 0; i < N; i++ )
			elems [i] = value;
	}
};

#endif

