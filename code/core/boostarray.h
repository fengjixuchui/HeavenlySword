//--------------------------------------------------
//!
//!	\file cloud_array.h
//!	Fixed-size Array
//!
//--------------------------------------------------

#ifndef _CLOUDARRAY_H_
#define _CLOUDARRAY_H_


template<class T, int N>
class Array
{
private:
	T elems[N];    // fixed-size ArrayBase of elements of type T
public:
	// type definitions
	typedef T              value_type;
	typedef T*             iterator;
	typedef const T*       const_iterator;
	typedef T&             reference;
	typedef const T&       const_reference;
	typedef int            size_type;

	// fake resize, for compatibility with objectDatabase
	void resize(size_type i)
	{
		ntError_p(i==N, ("Try to resize static array. This error probably comes from bad boost::array size in an XML file"));
		UNUSED(i);
	}
	// fake resize, for compatibility with objectDatabase
	void erase(iterator pos)
	{
		ntError_p(false, ("Try to erase an elem from a static array. This error probably comes from bad boost::array size in an XML file"));
		UNUSED(pos);
	}
	// fake resize, for compatibility with objectDatabase
	void insert(iterator pos, const T& x)
	{
		ntError_p(false, ("Try to insert an elem into a static array. This error probably comes from bad boost::array size in an XML file"));
		UNUSED(pos); UNUSED(x);
	}
	
	/// default constructor, does nothing
	explicit Array(){}

	/// set all the values uniformly to the given value
	explicit Array( const T& t)
	{
		assign(t);
	}

	/// copy the values from the C array
	explicit Array( const T t[N])
	{
		for( int i=0; i<N; ++i ) 
			(*this)[i]=t[i];
	}

	/// Constructor for a 2d vector
	explicit Array( const T& x, const T& y )
	{
		ntAssert( N==2 );
		(*this)[0] = x;
		(*this)[1] = y;
	} 

	/// Constructor for a 3d vector
	explicit Array( const T& x, const T& y, const T& z )
	{
		ntAssert( N==3 );
		(*this)[0] = x;
		(*this)[1] = y;
		(*this)[2] = z;
	} 

	/// Constructor for a 4d vector
	explicit Array( const T& x, const T& y, const T& z, const T& t )
	{
		ntAssert( N==4 );
		(*this)[0] = x;
		(*this)[1] = y;
		(*this)[2] = z;
		(*this)[3] = t;
	}
	
	// iterator support
	iterator begin() { return elems; }
	const_iterator begin() const { return elems; }
	iterator end() { return elems+N; }
	const_iterator end() const { return elems+N; }

	// operator[]
	reference operator[](size_type i) { return elems[i]; }
	const_reference operator[](size_type i) const { return elems[i]; }

	// at() with range check
	reference at(size_type i) { ntAssert(isInRange(i)); return elems[i]; }
	const_reference at(size_type i) const { ntAssert(isInRange(i)); return elems[i]; }

	// front() and back()
	reference front() { return elems[0]; }
	const_reference front() const { return elems[0]; }
	reference back() { return elems[N-1]; }
	const_reference back() const { return elems[N-1]; }

	// size is constant
	static size_type size() { return N; }
	static bool empty() { return false; }
	static size_type max_size() { return N; }
	enum { static_size = N };

	// assign one value to all elements
	void assign (const T& value)
	{
		for(int i = 0 ; i < N ; i++ )
		{
			elems[i]=value;
		}
	}



	// copy from N1 to N2 with N2<N1
	template<int Naux>
	static void SafeCopy(const Array<T,Naux> src, Array<T,N>& dest)
	{
		ntAssert(N<=Naux);
		for(int iIndex = 0 ; iIndex < N ; iIndex++ )
		{
			dest[iIndex] = src[iIndex];
		}
	}
	
	template<int Naux>
	static Array<T,N> SafeCopy(const Array<T,Naux> src)
	{
		Array<T,N> dest;
		SafeCopy(src,dest);
		return dest;
	}

private:
	// check range (may be private because it is static)
	static bool isInRange (size_type i) { return ((i>=0) && (i < size())); }
};




//#include "core/boostarray.inl"



#endif // end of _CLOUDARRAY_H_
