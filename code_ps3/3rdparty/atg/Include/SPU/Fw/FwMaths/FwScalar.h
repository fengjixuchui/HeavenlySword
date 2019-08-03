//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		SIMD Scalar

	@note		(c) Copyright Sony Computer Entertainment 2005. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef FW_SCALAR_H
#define FW_SCALAR_H

#include <Fw/FwMaths/FwVecInternal.h>

//--------------------------------------------------------------------------------------------------
// Forward references

class	FwScalar;
class	FwVector4;
class	FwMatrix44;

//--------------------------------------------------------------------------------------------------
// Argument typedefs

#ifdef	_MSC_VER
typedef	const FwScalar&			FwScalar_arg;
#else
typedef	const FwScalar			FwScalar_arg;
#endif	//_MSC_VER

//--------------------------------------------------------------------------------------------------
/**
	@class			FwScalar
	
	@brief			Represents a quadword with X, Y, Z, and W components containing the same value
**/
//--------------------------------------------------------------------------------------------------

class	FwScalar
{
	friend	class	FwVector4;
	friend	class	FwMatrix44;

public:
	// standard constructor
	FwScalar();

	// implicit constructor *on purpose*. Do not change.
	FwScalar( float x );

	// copy constructor
#ifdef	_MSC_VER
	FwScalar( const FwScalar& rhs );
#endif	//_MSC_VER

	// construction from quadword
	explicit	FwScalar( v128 );
	
	// construction to known values
	explicit	FwScalar( const FwMaths::ZeroTag& );

	// conversion to float
	operator	float() const;

	// access to quadword data
	v128		QuadwordValue( void ) const;

	// assignment
	FwScalar	operator =( const FwMaths::ZeroTag& );
	FwScalar	operator =( FwScalar_arg v );

	// binary operators
	FwScalar	operator +( FwScalar_arg r ) const;
	FwScalar	operator -( FwScalar_arg r ) const;
	FwScalar	operator *( FwScalar_arg r ) const;
	FwScalar	operator /( FwScalar_arg r ) const;

	FwScalar	operator +( float f ) const;
	FwScalar	operator -( float f ) const;
	FwScalar	operator *( float f ) const;
	FwScalar	operator /( float f ) const;

	friend	FwScalar	operator +( float f, FwScalar_arg v );
	friend	FwScalar	operator -( float f, FwScalar_arg v );
	friend	FwScalar	operator *( float f, FwScalar_arg v );
	friend	FwScalar	operator /( float f, FwScalar_arg v );

	// assignment operators
	FwScalar	operator +=( FwScalar_arg r );
	FwScalar	operator -=( FwScalar_arg r );
	FwScalar	operator *=( FwScalar_arg r );
	FwScalar	operator /=( FwScalar_arg r );

	FwScalar	operator +=( float f );
	FwScalar	operator -=( float f );
	FwScalar	operator *=( float f );
	FwScalar	operator /=( float f );

	// negate
	FwScalar	operator - () const;

	// non-member functions need to be friends so they can feast on the goo inside.
	friend	bool		IsPositive( FwScalar_arg vec );
	friend	bool		IsNegative( FwScalar_arg vec );
	friend	FwScalar	Abs( FwScalar_arg vec );
	friend	FwScalar	Sign( FwScalar_arg vec );
	friend	FwScalar	Signum( FwScalar_arg vec );
	friend	FwScalar	Recip( FwScalar_arg vec );
	friend	FwScalar	Sqrt( FwScalar_arg vec );
	friend	FwScalar	RecipSqrt( FwScalar_arg vec );
	friend	FwScalar	Max( FwScalar_arg lhs, FwScalar_arg rhs );
	friend	FwScalar	Min( FwScalar_arg lhs, FwScalar_arg rhs );
	friend	FwScalar	Clamp( FwScalar_arg vec, FwScalar_arg mn, FwScalar_arg mx );
	friend	FwScalar	Lerp( FwScalar_arg lhs, FwScalar_arg rhs, FwScalar_arg alpha );
	
	// comparisons
	friend	bool     operator < ( FwScalar_arg l, FwScalar_arg r );
	friend	bool     operator <=( FwScalar_arg l, FwScalar_arg r );
	friend	bool     operator > ( FwScalar_arg l, FwScalar_arg r );
	friend	bool     operator >=( FwScalar_arg l, FwScalar_arg r );
	friend	bool     operator ==( FwScalar_arg l, FwScalar_arg r );
	friend	bool     operator !=( FwScalar_arg l, FwScalar_arg r );

	friend	bool     operator < ( FwScalar_arg l, float r );
	friend	bool     operator <=( FwScalar_arg l, float r ); 
	friend	bool     operator > ( FwScalar_arg l, float r );
	friend	bool     operator >=( FwScalar_arg l, float r );
	friend	bool     operator ==( FwScalar_arg l, float r );
	friend	bool     operator !=( FwScalar_arg l, float r );
	
	friend	bool     operator < ( float r, FwScalar_arg l );
	friend	bool     operator <=( float r, FwScalar_arg l ); 
	friend	bool     operator > ( float r, FwScalar_arg l );
	friend	bool     operator >=( float r, FwScalar_arg l );
	friend	bool     operator ==( float r, FwScalar_arg l );
	friend	bool     operator !=( float r, FwScalar_arg l );

private:
	v128	m_value;		
};

// Include our inlines
#include <Fw/FwMaths/FwScalar.inl>

#endif	// FW_SCALAR_H
