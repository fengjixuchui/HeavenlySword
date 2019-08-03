//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.
**/
//--------------------------------------------------------------------------------------------------

#ifndef FP_CUBE_H
#define FP_CUBE_H

//--------------------------------------------------------------------------------------------------
//  INCLUDES
//--------------------------------------------------------------------------------------------------

#include	<Fw/FwMaths/FwVector.h>
#include	<Fw/FwMaths/FwPoint.h>

//--------------------------------------------------------------------------------------------------
//  DECLARATIONS
//--------------------------------------------------------------------------------------------------

class FpCube;

#ifdef	ATG_PC_PLATFORM
typedef	const FpCube&	FpCube_arg;
#else
typedef	const FpCube	FpCube_arg;
#endif	// ATG_PC_PLATFORM

//--------------------------------------------------------------------------------------------------
//  CLASS DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@class			FpCube

	@brief			class representing a cube
**/
//--------------------------------------------------------------------------------------------------

class  FpCube
{
public:

	// constructors
	inline FpCube() {}
	inline FpCube( float x, float y, float z, float h );
	inline FpCube( FwPoint_arg p, float h );

	// additional constructors
	explicit inline FpCube( FwVector4_arg rhs );
	explicit inline FpCube( const float* pData );
	explicit inline FpCube( v128 rhs );

	// assignment
	inline FpCube&	operator = ( FpCube_arg rhs );

	// access
	inline FwPoint	GetPosition() const;
	inline float	GetHalfSize() const;

private:
	FwVector4	m_value;
};

//--------------------------------------------------------------------------------------------------
//  INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@brief			FpCube constructor from 4 floats

	@param			x				-	position x coordinate
	@param			y				-	position y coordinate
	@param			z				-	position z coordinate
	@param			h				-	half size
**/
//--------------------------------------------------------------------------------------------------

FpCube::FpCube( float x, float y, float z, float h )
{
	m_value = FwVector4( x, y, z, h );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			FpCube constructor from position and half size

	@param			p				-	position
	@param			h				-	half size
**/
//--------------------------------------------------------------------------------------------------

FpCube::FpCube( FwPoint_arg p, float h )
{
	m_value = p.GetVector4();
	m_value.SetW( h );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			FpCube constructor from a FwVector4

	@param			rhs				-	the vector
**/
//--------------------------------------------------------------------------------------------------

FpCube::FpCube( FwVector4_arg rhs )
{
	m_value = rhs;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			FpCube constructor from an array of floats

	@param			pData			-	pointer to floats
**/
//--------------------------------------------------------------------------------------------------

FpCube::FpCube( const float* pData )
{
	FW_ASSERT( pData );
	m_value = FwVector4( pData );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			FpCube constructor from quadword

	@param			rhs				-	the quadword
**/
//--------------------------------------------------------------------------------------------------

FpCube::FpCube( v128 rhs )
{
	m_value = FwVector4( rhs );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			'this = rhs'
**/
//--------------------------------------------------------------------------------------------------

FpCube&	FpCube::operator = ( FpCube_arg rhs )
{
	m_value = rhs.m_value;
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			get cube's position

	@return			the position
**/
//--------------------------------------------------------------------------------------------------

FwPoint		FpCube::GetPosition() const
{
	return FwPoint( m_value );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			get cube's half size
**/
//--------------------------------------------------------------------------------------------------

float		FpCube::GetHalfSize() const
{
	return m_value.W();
}

#endif // FP_CUBE_H
