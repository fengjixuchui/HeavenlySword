//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		SIMD Vector

	@note		(c) Copyright Sony Computer Entertainment 2005. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef FW_VECTOR4_H
#define FW_VECTOR4_H

#include <Fw/FwMaths/FwScalar.h>

//--------------------------------------------------------------------------------------------------
// Forward references

class	FwVector4;

//--------------------------------------------------------------------------------------------------
// Argument typedefs

#ifdef	_MSC_VER
typedef	const FwVector4&		FwVector4_arg;
#else
typedef	const FwVector4			FwVector4_arg;
#endif	//_MSC_VER

//--------------------------------------------------------------------------------------------------
/**
	@class			FwVector4
	
	@brief			Holds a 4-element vector in X, Y, Z, and W		
**/
//--------------------------------------------------------------------------------------------------

class	FwVector4
{
public:
	// standard constructors
	FwVector4( );
	FwVector4( float x, float y, float z, float w );

	// copy constructor
#ifdef	_MSC_VER
	FwVector4( const FwVector4& rhs );
#endif	//_MSC_VER

	// additional constructors
	explicit	FwVector4( const float* pData );
	explicit	FwVector4( v128 rhs );
	explicit	FwVector4( FwScalar_arg rhs );

	// construction to known values
	FwVector4( const FwMaths::ZeroTag& );
	FwVector4( const FwMaths::UnitXAxisTag& );
	FwVector4( const FwMaths::UnitYAxisTag& );
	FwVector4( const FwMaths::UnitZAxisTag& );
	FwVector4( const FwMaths::UnitWAxisTag& );

	// assign common fields of rhs
	FwVector4	operator = ( const FwMaths::ZeroTag& );
	FwVector4	operator = ( const FwMaths::UnitXAxisTag& );
	FwVector4	operator = ( const FwMaths::UnitYAxisTag& );
	FwVector4	operator = ( const FwMaths::UnitZAxisTag& );
	FwVector4	operator = ( const FwMaths::UnitWAxisTag& );
	FwVector4	operator = ( FwVector4_arg rhs );

	// operations with floats are broadcast operations
	FwVector4	operator + ( FwVector4_arg rhs ) const;
	FwVector4	operator + ( FwScalar_arg rhs ) const;
	FwVector4	operator - ( FwVector4_arg rhs ) const;
	FwVector4	operator - ( FwScalar_arg rhs ) const;
	FwVector4	operator * ( FwVector4_arg rhs ) const;
	FwVector4	operator * ( FwScalar_arg rhs ) const;
	FwVector4	operator / ( FwVector4_arg rhs ) const;
	FwVector4	operator / ( FwScalar_arg rhs ) const;

	FwVector4	operator += ( FwVector4_arg rhs );
	FwVector4	operator += ( FwScalar_arg rhs );
	FwVector4	operator -= ( FwVector4_arg rhs );
	FwVector4	operator -= ( FwScalar_arg rhs );
	FwVector4	operator *= ( FwVector4_arg rhs );
	FwVector4	operator *= ( FwScalar_arg rhs );
	FwVector4	operator /= ( FwVector4_arg rhs );
	FwVector4	operator /= ( FwScalar_arg rhs );

	// negate
	FwVector4	operator - () const;

	// general member & array accessors
	FwScalar		X( void ) const;
	FwScalar		Y( void ) const;
	FwScalar		Z( void ) const;
	FwScalar		W( void ) const;	

	void			SetX( FwScalar_arg x );
	void			SetY( FwScalar_arg y );
	void			SetZ( FwScalar_arg z );
	void			SetW( FwScalar_arg w );

	FwScalar		Get( int index ) const;			
	void			Set( int index, FwScalar_arg val );
	const FwScalar	operator []( int index ) const;

	// read-only access to quadword value
	v128			QuadwordValue( void ) const;
	v128			QuadwordValueXYZ( void ) const;

	// non-member functions, which need to be friends
	friend	FwScalar	MaxComp3( FwVector4_arg vec );
	friend	FwScalar	MaxComp4( FwVector4_arg vec );
	friend	FwScalar	MinComp3( FwVector4_arg vec );
	friend	FwScalar	MinComp4( FwVector4_arg vec );
	friend	FwScalar	Sum3( FwVector4_arg vec );
	friend	FwScalar	Sum4( FwVector4_arg vec );
	friend	FwScalar	Length3Sqr( FwVector4_arg vec );
	friend	FwScalar	Length3Rcp( FwVector4_arg vec );
	friend	FwScalar	Length3( FwVector4_arg vec );
	friend	FwScalar	Length4Sqr( FwVector4_arg vec );
	friend	FwScalar	Length4Rcp( FwVector4_arg vec );
	friend	FwScalar	Length4( FwVector4_arg vec );
	friend	FwVector4	Abs( FwVector4_arg vec );
	friend	FwVector4	Sign( FwVector4_arg vec );
	friend	FwVector4	Signum( FwVector4_arg vec );
	friend	FwVector4	Recip( FwVector4_arg vec );
	friend	FwVector4	Sqrt( FwVector4_arg vec );
	friend	FwVector4	RecipSqrt( FwVector4_arg vec );
	friend	FwVector4	Max( FwVector4_arg lhs, FwVector4_arg rhs );
	friend	FwVector4	Min( FwVector4_arg lhs, FwVector4_arg rhs );
	friend	FwVector4	Clamp( FwVector4_arg vec, FwVector4_arg mn, FwVector4_arg mx );
	friend	FwScalar	Dot3( FwVector4_arg lhs, FwVector4_arg rhs );
	friend	FwScalar	Dot4( FwVector4_arg lhs, FwVector4_arg rhs );
	friend	FwVector4	Cross( FwVector4_arg lhs, FwVector4_arg rhs );
	friend	FwVector4	Normalize3( FwVector4_arg vec );
	friend	FwVector4	SafeNormalize3( FwVector4_arg vec, FwVector4_arg fallback );
	friend	FwVector4	Normalize4( FwVector4_arg vec );
	friend	FwVector4	SafeNormalize4( FwVector4_arg vec, FwVector4_arg fallback );
	friend	FwVector4	Lerp( FwVector4_arg lhs, FwVector4_arg rhs, FwScalar_arg alpha );
	friend	bool		AllComponentsLessThan( FwVector4_arg lhs, FwVector4_arg rhs );
	friend	bool		AllComponentsLessThanOrEqual( FwVector4_arg lhs, FwVector4_arg rhs );
	friend	bool		AllComponentsGreaterThan( FwVector4_arg lhs, FwVector4_arg rhs );
	friend	bool		AllComponentsGreaterThanOrEqual( FwVector4_arg lhs, FwVector4_arg rhs );
	friend	bool		AllComponentsEqual( FwVector4_arg lhs, FwVector4_arg rhs );
	friend	FwVector4	operator * ( FwScalar_arg scalar, FwVector4_arg vec );
	friend	FwVector4	operator + ( FwScalar_arg scalar, FwVector4_arg vec );

private:
	v128		m_value;
};

// Include our inlines
#include <Fw/FwMaths/FwVector4.inl>

#endif	// FW_VECTOR4_H
