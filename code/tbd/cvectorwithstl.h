#ifndef _CVECTORWITHSTL_H_
#define _CVECTORWITHSTL_H_

#include "core/explicittemplate.h"

//--------------------------------------------------
//!
//!	CVector -> Vec4 -> CVector.
//!	This class is only use to store a CVector in a ntstd container, which is not possible
//! wihtout this trick because stl doesn't like aligned stuff
//!
//--------------------------------------------------

class CVectorContainer: public Vec4
{
public:
	// null ctor
	CVectorContainer()
	{
		assign(0.0f);
	}
	
	///////////////////////////////////////////////////
	// conversion from CVector and other to CVectorContainer
	explicit CVectorContainer(const CVector& v)
	{
		(*this)[0] = v.X();
		(*this)[1] = v.Y();
		(*this)[2] = v.Z();
		(*this)[3] = v.W();
	}
	explicit CVectorContainer(const CQuat& v)
	{
		(*this)[0] = v.X();
		(*this)[1] = v.Y();
		(*this)[2] = v.Z();
		(*this)[3] = v.W();
	}
	explicit CVectorContainer(const CPoint& v)
	{
		(*this)[0] = v.X();
		(*this)[1] = v.Y();
		(*this)[2] = v.Z();
		(*this)[3] = 1.0f;
	}
	explicit CVectorContainer(const CDirection& v)
	{
		(*this)[0] = v.X();
		(*this)[1] = v.Y();
		(*this)[2] = v.Z();
		(*this)[3] = 0.0f;
	}
	// End conversion
	///////////////////////////////////////////////////
	
	
	
	///////////////////////////////////////////////////
	// conversion from class CVectorContainer to CVector and other
	CVector ToVector() const
	{
		return CVector((*this)[0],(*this)[1],(*this)[2],(*this)[3]);
	}
	CQuat ToQuat() const
	{
		return CQuat((*this)[0],(*this)[1],(*this)[2],(*this)[3]);
	}
	CDirection ToDirection() const
	{
		return CDirection((*this)[0],(*this)[1],(*this)[2]);
	}
	CPoint ToPoint() const
	{
		return CPoint((*this)[0],(*this)[1],(*this)[2]);
	}
	// End conversion
	///////////////////////////////////////////////////
};



#endif // end of _CVECTORWITHSTL_H_
