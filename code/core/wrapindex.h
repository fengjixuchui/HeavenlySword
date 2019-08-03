//--------------------------------------------------
//!
//!	\file wrapindex.h
//!	wrap around index
//!
//--------------------------------------------------

#ifndef _ROTATIONALINDEX_H_
#define _ROTATIONALINDEX_H_


template<int SIZE>
class WrapIndex
{
public:

	WrapIndex( int initTo = 0 ) : m_iCurrentIndex(initTo) 
	{ 
		ntAssert(initTo<SIZE); 
	}

	WrapIndex( const WrapIndex& other ) : m_iCurrentIndex( other.m_iCurrentIndex )
	{

	}

	// get value
	operator int() { return m_iCurrentIndex; }
	
	// pre-increment
	WrapIndex& operator++() 
	{ 
		InternalIncrement();
		return *this; 
	}

	// post-increment
	const WrapIndex operator++(int)
	{
		WrapIndex current( *this );
		InternalIncrement();
		return current;
	}

private:

	void InternalIncrement()
	{
		++m_iCurrentIndex %= SIZE;
	}
	
private:

	int		m_iCurrentIndex;
};



#endif // end of _ROTATIONALINDEX_H_
