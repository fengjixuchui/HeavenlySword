//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		SIMD Vector

	@note		(c) Copyright Sony Computer Entertainment 2005. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef FW_VECTOR_H
#define FW_VECTOR_H

#include <Fw/FwMaths/FwScalar.h>
#include <Fw/FwMaths/FwVector4.h>

//--------------------------------------------------------------------------------------------------
// Forward references

class	FwVector;

//--------------------------------------------------------------------------------------------------
// Argument typedefs

#ifdef	_MSC_VER
typedef	const FwVector&		FwVector_arg;
#else
typedef	const FwVector		FwVector_arg;
#endif	//_MSC_VER

//--------------------------------------------------------------------------------------------------
/**
	@class			FwVector
	
	@brief			Holds a 3-element vector in X, Y, Z. Contents of W are undefined.
**/
//--------------------------------------------------------------------------------------------------

class	FwVector
{
public:
	// standard constructors
	FwVector( );
	FwVector( float x, float y, float z );

	// copy constructor
#ifdef	_MSC_VER
	FwVector( const FwVector& rhs );
#endif	//_MSC_VER

	// additional constructors
	explicit	FwVector( const float* pData );
	explicit	FwVector( v128 rhs );
	explicit	FwVector( FwScalar_arg rhs );
	explicit	FwVector( FwVector4_arg rhs );

	// construction to known values
	FwVector( const FwMaths::ZeroTag& );
	FwVector( const FwMaths::UnitXAxisTag& );
	FwVector( const FwMaths::UnitYAxisTag& );
	FwVector( const FwMaths::UnitZAxisTag& );

	// assign common fields of rhs
	FwVector	operator = ( const FwMaths::ZeroTag& );
	FwVector	operator = ( const FwMaths::UnitXAxisTag& );
	FwVector	operator = ( const FwMaths::UnitYAxisTag& );
	FwVector	operator = ( const FwMaths::UnitZAxisTag& );
	FwVector	operator = ( FwVector_arg rhs );

	// operations with floats are broadcast operations
	FwVector	operator + ( FwVector_arg rhs ) const;
	FwVector	operator + ( FwScalar_arg rhs ) const;
	FwVector	operator - ( FwVector_arg rhs ) const;
	FwVector	operator - ( FwScalar_arg rhs ) const;
	FwVector	operator * ( FwVector_arg rhs ) const;
	FwVector	operator * ( FwScalar_arg rhs ) const;
	FwVector	operator / ( FwVector_arg rhs ) const;
	FwVector	operator / ( FwScalar_arg rhs ) const;

	FwVector	operator += ( FwVector_arg rhs );
	FwVector	operator += ( FwScalar_arg rhs );
	FwVector	operator -= ( FwVector_arg rhs );
	FwVector	operator -= ( FwScalar_arg rhs );
	FwVector	operator *= ( FwVector_arg rhs );
	FwVector	operator *= ( FwScalar_arg rhs );
	FwVector	operator /= ( FwVector_arg rhs );
	FwVector	operator /= ( FwScalar_arg rhs );

	// negate
	FwVector	operator - () const;

	// general member & array accessors
	FwScalar		X( void ) const;
	FwScalar		Y( void ) const;
	FwScalar		Z( void ) const;

	void			SetX( FwScalar_arg x );
	void			SetY( FwScalar_arg y );
	void			SetZ( FwScalar_arg z );

	FwScalar		Get( int index ) const;			
	void			Set( int index, FwScalar_arg val );
	const FwScalar	operator []( int index ) const;

	// read-only access to quadword value
	v128			QuadwordValue( void ) const;

	// return an equivalent FwVector4
	FwVector4		GetVector4( void ) const;

	// non-member functions, which need to be friends
	friend	FwScalar	MaxComp( FwVector_arg vec );
	friend	FwScalar	MinComp( FwVector_arg vec );
	friend	FwScalar	Sum( FwVector_arg vec );
	friend	FwScalar	LengthSqr( FwVector_arg vec );
	friend	FwScalar	LengthRcp( FwVector_arg vec );
	friend	FwScalar	Length( FwVector_arg vec );
	friend	FwVector	Abs( FwVector_arg vec );
	friend	FwVector	Sign( FwVector_arg vec );
	friend	FwVector	Signum( FwVector_arg vec );
	friend	FwVector	Recip( FwVector_arg vec );
	friend	FwVector	Sqrt( FwVector_arg vec );
	friend	FwVector	RecipSqrt( FwVector_arg vec );
	friend	FwVector	Max( FwVector_arg lhs, FwVector_arg rhs );
	friend	FwVector	Min( FwVector_arg lhs, FwVector_arg rhs );
	friend	FwVector	Clamp( FwVector_arg vec, FwVector_arg mn, FwVector_arg mx );
	friend	FwScalar	Dot( FwVector_arg lhs, FwVector_arg rhs );
	friend	FwVector	Cross( FwVector_arg lhs, FwVector_arg rhs );
	friend	FwVector	Normalize( FwVector_arg vec );
	friend	FwVector	SafeNormalize( FwVector_arg vec, FwVector_arg fallback );
	friend	FwVector	Lerp( FwVector_arg lhs, FwVector_arg rhs, FwScalar_arg alpha );
	friend	bool		AllComponentsLessThan( FwVector_arg lhs, FwVector_arg rhs );
	friend	bool		AllComponentsLessThanOrEqual( FwVector_arg lhs, FwVector_arg rhs );
	friend	bool		AllComponentsGreaterThan( FwVector_arg lhs, FwVector_arg rhs );
	friend	bool		AllComponentsGreaterThanOrEqual( FwVector_arg lhs, FwVector_arg rhs );
	friend	bool		AllComponentsEqual( FwVector_arg lhs, FwVector_arg rhs );
	friend	FwVector	operator * ( FwScalar_arg scalar, FwVector_arg vec );
	friend	FwVector	operator + ( FwScalar_arg scalar, FwVector_arg vec );

private:
	v128	m_value;
};

// Include our inlines
#include <Fw/FwMaths/FwVector.inl>

#endif	// FW_VECTOR_H
