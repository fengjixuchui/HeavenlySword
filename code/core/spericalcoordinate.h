#ifndef _SPERICALCOORDINATE_H_
#define _SPERICALCOORDINATE_H_

//--------------------------------------------------
//!
//!	\file spericalcoordinate.h
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#include <math.h>

template<typename Vec2>
struct SphericalCoordinate
{
	static inline CVector GetBaseDir(Vec2 angle)
	{
		float fSin = sin(angle[0]);
		return CVector(sin(angle[1])*fSin, cos(angle[0]),cos(angle[1])*fSin,0.0f);
	}
	
	static inline CVector GetBaseTeta(Vec2 angle)
	{
		float fCos = cos(angle[0]);
		return CVector(sin(angle[1])*fCos,-sin(angle[0]),cos(angle[1])*fCos,0.0f);
	}
	
	static inline CVector GetBasePhi(Vec2 angle)
	{
		return CVector(cos(angle[1]),0.0f,-sin(angle[1]),0.0f);
	}
	
	static inline void SetMatrix(CMatrix& m, Vec2 angle)
	{
		Vec2 sincos0(sin(angle[0]),cos(angle[0]));
		Vec2 sincos1(sin(angle[1]),cos(angle[1]));
		
		m.SetXAxis(CDirection(sincos1[0]*sincos0[0], sincos0[1],sincos1[1]*sincos0[0]));
		m.SetYAxis(CDirection(sincos1[0]*sincos0[1],-sincos0[0],sincos1[1]*sincos0[1]));
		m.SetZAxis(CDirection(sincos1[1],0.0f,-sincos1[0]));
	}
	
	// (x,y,z) -> (r,teta,phi)
	static inline void CartesianToSpherical(const CDirection& cartesian, Vec2& angle, float& fRadius)
	{
		fRadius = cartesian.Length();
		angle[0] = acos(cartesian.Y()/fRadius);
		angle[1] = atan2f( cartesian.X(), cartesian.Z() );
	}
	
	// (x,y,z) -> (r,teta,phi)
	static inline void CartesianToSpherical(const CDirection& cartesian, Vec2& angle)
	{
		float fLength = cartesian.Length();
		angle[0] = acos(cartesian.Y()/fLength);
		angle[1] = atan2f( cartesian.X(), cartesian.Z() );
	}
	// (r,teta,phi) -> (x,y,z)
	static inline CPoint SphericalToCartesian(Vec2& angle, float& fRadius)
	{
		float fSin = sin(angle[0]);
		return CPoint(sin(angle[1])*fSin,cos(angle[0]),cos(angle[1])*fSin)*fRadius;
	}

	static inline bool IsSphericalAngleBounded(Vec2 angle)
	{
		//static const float PI = 3.14159265f;
		static const float PI_PLUS_EPSILON = 3.15f;
		return (fabs(angle[0])<=PI_PLUS_EPSILON) && (fabs(angle[1])<=PI_PLUS_EPSILON);
	}
}; // end of namespace SphericalCoordinate





#endif // end of _SPERICALCOORDINATE_H_
