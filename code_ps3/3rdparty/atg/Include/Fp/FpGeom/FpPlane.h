//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.
**/
//--------------------------------------------------------------------------------------------------

#ifndef FP_PLANE_H
#define FP_PLANE_H

//--------------------------------------------------------------------------------------------------
//  INCLUDES
//--------------------------------------------------------------------------------------------------

#include	<Fw/FwMaths/FwVector4.h>
#include	<Fw/FwMaths/FwVector.h>
#include	<Fw/FwMaths/FwPoint.h>

//--------------------------------------------------------------------------------------------------
//  DECLARATIONS
//--------------------------------------------------------------------------------------------------

class FpPlane;

#ifdef	ATG_PC_PLATFORM
typedef	const FpPlane&	FpPlane_arg;
#else
typedef const FpPlane	FpPlane_arg;
#endif	// ATG_PC_PLATFORM

//--------------------------------------------------------------------------------------------------
//  CLASS DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@class			FpPlane

	@brief			class representing a plane in 3d

	@note			defined by a normal n and a scalar d
					such that for any point p on the plane
					dot( n, p ) + d = 0
**/
//--------------------------------------------------------------------------------------------------

class  FpPlane
{
public:

	/// enum used by FindSide()
	enum Side
	{
		kSideOfNormal,
		kOnPlane,
		kSideOppositeNormal,
	};

	// constructors
	inline FpPlane() {}
	inline FpPlane( float x, float y, float z, float w );
	inline FpPlane( FwVector_arg normal, FwPoint_arg p );
	inline FpPlane( FwVector_arg normal, FwScalar_arg scalar );
	inline FpPlane( FwPoint_arg p0, FwPoint_arg p1, FwPoint_arg p2 );

	// additional constructors
	explicit inline FpPlane( FwVector4_arg rhs );
	explicit inline FpPlane( const float* pData );
	explicit inline FpPlane( v128 rhs );

	// Element access
	FwScalar		X( void ) const;
	FwScalar		Y( void ) const;
	FwScalar		Z( void ) const;
	FwScalar		W( void ) const;

	// conversion
	inline FwVector4	AsVector4() const;

	// assignment
	inline FpPlane&	operator = ( FpPlane_arg rhs );

	// access
	inline FwVector	GetNormal() const;
	inline FwScalar	GetScalar() const;

private:
	FwVector4	m_value;
};

// Non-member functions for FpPlane
inline float			SignedDistance( FpPlane_arg plane, FwPoint_arg point );
inline FpPlane::Side	FindSide( FpPlane_arg plane, FwPoint_arg point );

//--------------------------------------------------------------------------------------------------
//  INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@brief			FpPlane constructor from 4 floats

	@param			x				-	normal x component
	@param			y				-	normal y component
	@param			z				-	normal z component
	@param			w				-	scalar
**/
//--------------------------------------------------------------------------------------------------

FpPlane::FpPlane( float x, float y, float z, float w )
{
	m_value = FwVector4( x, y, z, w );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			FpPlane constructor from normal and point on plane

	@param			normal			-	normal vector, assumed normalised
	@param			p				-	point on plane
**/
//--------------------------------------------------------------------------------------------------

FpPlane::FpPlane( FwVector_arg normal, FwPoint_arg p )
{
	m_value = normal.GetVector4();
	m_value.SetW( -Dot3( m_value, p.GetVector4() ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			FpPlane constructor from normal and scalar

	@param			normal			-	normal vector, assumed normalised
	@param			scalar			-	scalar
**/
//--------------------------------------------------------------------------------------------------

FpPlane::FpPlane( FwVector_arg normal, FwScalar_arg scalar )
{
	m_value = normal.GetVector4();
	m_value.SetW( scalar );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			FpPlane constructor from 3 points on plane

	@param			p0				-	point 0
	@param			p1				-	point 1
	@param			p2				-	point 2
**/
//--------------------------------------------------------------------------------------------------

FpPlane::FpPlane( FwPoint_arg p0, FwPoint_arg p1, FwPoint_arg p2 )
{
	m_value = Normalize( Cross( p1 - p0, p2 - p1 ) ).GetVector4();
	m_value.SetW( -Dot3( m_value, p0.GetVector4() ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			FpPlane constructor from FwVector4

	@param			rhs				-	vector
**/
//--------------------------------------------------------------------------------------------------

FpPlane::FpPlane( FwVector4_arg rhs )
{
	m_value = rhs;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			FpPlane constructor from array of floats

	@param			pData			-	pointer to floats
**/
//--------------------------------------------------------------------------------------------------

FpPlane::FpPlane( const float* pData )
{
	FW_ASSERT( pData );
	m_value = FwVector4( pData );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			FpPlane constructor from quadword

	@param			rhs				-	quadword value
**/
//--------------------------------------------------------------------------------------------------

FpPlane::FpPlane( v128 rhs )
{
	m_value = FwVector4( rhs );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			X component of plane
**/
//--------------------------------------------------------------------------------------------------

inline FwScalar	FpPlane::X( void ) const
{
	return m_value.X();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Y component of plane
**/
//--------------------------------------------------------------------------------------------------

inline FwScalar	FpPlane::Y( void ) const
{
	return m_value.Y();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Z component of plane
**/
//--------------------------------------------------------------------------------------------------

inline FwScalar	FpPlane::Z( void ) const
{
	return m_value.Z();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			W component of plane
**/
//--------------------------------------------------------------------------------------------------

inline FwScalar	FpPlane::W( void ) const
{
	return m_value.W();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			returns plane data as FwVector4
**/
//--------------------------------------------------------------------------------------------------

FwVector4	FpPlane::AsVector4() const
{
	return m_value;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			'this = rhs'
**/
//--------------------------------------------------------------------------------------------------

FpPlane&	FpPlane::operator = ( FpPlane_arg rhs )
{
	m_value = rhs.m_value;
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			get the plane's normal vector

	@return			normal vector
**/
//--------------------------------------------------------------------------------------------------

FwVector	FpPlane::GetNormal() const
{
	return FwVector( m_value );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			get the plane's scalar

	@return			the scalar
**/
//--------------------------------------------------------------------------------------------------

FwScalar		FpPlane::GetScalar() const
{
	return m_value.W();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			find the distance from a plane to a point

	@param			plane			-	the plane
	@param			point			-	the point

	@return			signed distance (positive is same side as normal)
**/
//--------------------------------------------------------------------------------------------------

float			SignedDistance( FpPlane_arg plane, FwPoint_arg point )
{
	return Dot( plane.GetNormal(), point - FwMaths::kOrigin ) + plane.GetScalar();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			find which side of a plane a point is

	@param			plane			-	the plane
	@param			point			-	the point

	@return			enum value
**/
//--------------------------------------------------------------------------------------------------

FpPlane::Side	FindSide( FpPlane_arg plane, FwPoint_arg point )
{
	float d = SignedDistance( plane, point );
	return ( d > 0.0f ? FpPlane::kSideOfNormal : ( d < 0.0f ? FpPlane::kSideOppositeNormal : FpPlane::kOnPlane ) );
}

#endif // FW_PLANE_H
