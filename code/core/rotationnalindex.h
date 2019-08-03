#ifndef _ROTATIONNALINDEX_H_
#define _ROTATIONNALINDEX_H_

//--------------------------------------------------
//!
//!	\file rotationnalindex.h
//!	addressing array in a "ring way"
//!
//--------------------------------------------------



#include "boostarray.h"


//--------------------------------------------------
//!
//!	Just a way to access an array throught modulo
//!
//--------------------------------------------------
template<typename INT_T, INT_T SIZE>
class RotationnalIndex 
{
protected:
	// current index in the array
	INT_T m_iCurrent;

public:
	// ctor
	RotationnalIndex()
		:m_iCurrent(0)
	{
		// nothing
	}
	
	// incrementing and wrapping if necessary
	// warping is not absolutely necessary, but it's good to bound m_iCurrent
	void operator++(int)	
	{
		m_iCurrent++;
		m_iCurrent%=SIZE;
	}
	
	// get ringed index
	INT_T operator [] (INT_T iIndex) const
	{
		return (m_iCurrent+iIndex)%SIZE;
	}

	// convert to index 0
	operator INT_T()
	{
		return m_iCurrent;
	}
};

#endif // end of _ROTATIONNALINDEX_H_
