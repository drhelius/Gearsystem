
// Boost substitute. For full boost library see http://boost.org

#ifndef BOOST_SCOPED_PTR_HPP
#define BOOST_SCOPED_PTR_HPP

template<class T>
class scoped_ptr {
	T* ptr;
	
	// noncopyable
	scoped_ptr( const scoped_ptr& );
	scoped_ptr& operator = ( const scoped_ptr& );
public:
	scoped_ptr( T* p = 0 ) : ptr( p ) {
	}
	
	~scoped_ptr() {
		delete (ptr + 0); // ensure T is defined
	}
	
	void reset( T* p = 0 ) {
		assert( !p || p != ptr );
		T* old = ptr;
		ptr = p;
		delete (old + 0); // ensure T is defined
	}
	
	T& operator * () const {
		assert( ptr );
		return *ptr;
	}
	
	T* operator -> () const {
		assert( ptr );
		return ptr;
	}
	
	T* get () const {
		return ptr;
	}
	
	T* release() {
		T* old = ptr;
		ptr = NULL;
		return old;
	}
	
	typedef struct undefined_type* bool_type;
	
	operator bool_type () const {
		return (bool_type) ptr;
	}
	
	bool operator ! () const {
		return !ptr;
	}
	
	void swap( scoped_ptr& other ) {
		T* p = ptr;
		ptr = other.ptr;
		other.ptr = p;
	}
};

#endif

