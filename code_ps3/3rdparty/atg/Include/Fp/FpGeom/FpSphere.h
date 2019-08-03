//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.
**/
//--------------------------------------------------------------------------------------------------

#ifndef FP_SPHERE_H
#define FP_SPHERE_H

//--------------------------------------------------------------------------------------------------
//  INCLUDES
//--------------------------------------------------------------------------------------------------

#include	<Fw/FwMaths/FwVector.h>
#include	<Fw/FwMaths/FwPoint.h>

//--------------------------------------------------------------------------------------------------
//  DECLARATIONS
//--------------------------------------------------------------------------------------------------

class FpSphere;

#ifdef	ATG_PC_PLATFORM
typedef	const FpSphere&	FpSphere_arg;
#else
typedef	const FpSphere	FpSphere_arg;
#endif	// ATG_PC_PLATFORM

//--------------------------------------------------------------------------------------------------
//  CLASS DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@class			FpSphere

	@brief			class representing a sphere
**/
//--------------------------------------------------------------------------------------------------

class  FpSphere
{
public:

	// constructors
	inline FpSphere() {}
	inline FpSphere( float x, float y, float z, float r );
	inline FpSphere( FwPoint_arg p, FwScalar_arg r );

	// additional constructors
	explicit inline FpSphere( FwVector4_arg rhs );
	explicit inline FpSphere( const float* pData );
	explicit inline FpSphere( v128 rhs );

	// conversion
	inline FwVector4	AsVector4() const;
	
	// assignment
	inline FpSphere&	operator = ( FpSphere_arg rhs );

	// access
	inline FwPoint		GetPosition() const;
	inline FwScalar		GetRadius() const;

	// operations
	inline	bool		ContainsPoint( FwPoint_arg p ) const;

private:
	FwVector4	m_value;
};

//--------------------------------------------------------------------------------------------------
//  INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@brief			FpSphere constructor from 4 floats

	@param			x				-	position x coordinate
	@param			y				-	position y coordinate
	@param			z				-	position z coordinate
	@param			r				-	radius
**/
//--------------------------------------------------------------------------------------------------

FpSphere::FpSphere( float x, float y, float z, float r )
{
	m_value = FwVector4( x, y, z, r );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			FpSphere constructor from position and radius

	@param			p				-	position
	@param			r				-	radius
**/
//--------------------------------------------------------------------------------------------------

FpSphere::FpSphere( FwPoint_arg p, FwScalar_arg r )
{
	m_value = p.GetVector4();
	m_value.SetW( r );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			FpSphere constructor from a FwVector4

	@param			rhs				-	the vector
**/
//--------------------------------------------------------------------------------------------------

FpSphere::FpSphere( FwVector4_arg rhs )
{
	m_value = rhs;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			FpSphere constructor from an array of floats

	@param			pData			-	pointer to floats
**/
//--------------------------------------------------------------------------------------------------

FpSphere::FpSphere( const float* pData )
{
	FW_ASSERT( pData );
	m_value = FwVector4( pData );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			FpSphere constructor from quadword

	@param			rhs				-	the quadword
**/
//--------------------------------------------------------------------------------------------------

FpSphere::FpSphere( v128 rhs )
{
	m_value = FwVector4( rhs );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			returns sphere data as FwVector4
**/
//--------------------------------------------------------------------------------------------------

FwVector4	FpSphere::AsVector4() const
{
	return m_value;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			'this = rhs'
**/
//--------------------------------------------------------------------------------------------------

FpSphere&	FpSphere::operator = ( FpSphere_arg rhs )
{
	m_value = rhs.m_value;
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			get sphere's position

	@return			the position
**/
//--------------------------------------------------------------------------------------------------

FwPoint		FpSphere::GetPosition() const
{
	return FwPoint( m_value );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			get sphere's radius

	@return			the radius
**/
//--------------------------------------------------------------------------------------------------

FwScalar	FpSphere::GetRadius() const
{
	return m_value.W();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			tests for containment of a point by the sphere

	@return			true if the point is within the sphere
**/
//--------------------------------------------------------------------------------------------------

bool		FpSphere::ContainsPoint( FwPoint_arg p ) const
{
	return LengthSqr( GetPosition() - p ) <= sqr(GetRadius());
}


#endif // FP_SPHERE_H
