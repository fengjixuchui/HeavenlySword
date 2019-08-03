//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		SIMD Point

	@note		(c) Copyright Sony Computer Entertainment 2005. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef FW_POINT_H
#define FW_POINT_H

#include <Fw/FwMaths/FwScalar.h>
#include <Fw/FwMaths/FwVector4.h>
#include <Fw/FwMaths/FwVector.h>

//--------------------------------------------------------------------------------------------------
// Forward references

class	FwPoint;

//--------------------------------------------------------------------------------------------------
// Argument typedefs

#ifdef	_MSC_VER
typedef	const FwPoint&		FwPoint_arg;
#else
typedef	const FwPoint		FwPoint_arg;
#endif	//_MSC_VER

//--------------------------------------------------------------------------------------------------
/**
	@class			FwPoint
	
	@brief			Holds a 3-element point in X, Y, Z. Contents of W are undefined.
**/
//--------------------------------------------------------------------------------------------------

class	FwPoint
{
public:
	// standard constructors
	FwPoint( );
	FwPoint( float x, float y, float z );

	// copy constructor
#ifdef	_MSC_VER
	FwPoint( const FwPoint& rhs );
#endif	//_MSC_VER

	// additional constructors
	explicit	FwPoint( const float* pData );
	explicit	FwPoint( v128 rhs );
	explicit	FwPoint( FwScalar_arg rhs );
	explicit	FwPoint( FwVector4_arg rhs );

	// construction to known values
	FwPoint( const FwMaths::ZeroTag& );
	FwPoint( const FwMaths::UnitWAxisTag& );

	// assign common fields of rhs
	FwPoint		operator = ( const FwMaths::ZeroTag& );
	FwPoint		operator = ( const FwMaths::UnitWAxisTag& );
	FwPoint		operator = ( FwPoint_arg rhs );

	// operations with floats are broadcast operations
	FwPoint		operator + ( FwVector_arg rhs ) const;
	FwVector	operator - ( FwPoint_arg rhs ) const;
	FwPoint		operator - ( FwVector_arg rhs ) const;
	FwPoint		operator += ( FwVector_arg rhs );
	FwPoint		operator -= ( FwVector_arg rhs );

	// negate
	FwPoint		operator - () const;

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
	friend	FwScalar	MaxComp( FwPoint_arg vec );
	friend	FwScalar	MinComp( FwPoint_arg vec );
	friend	FwScalar	Sum( FwPoint_arg vec );
	friend	FwPoint		Abs( FwPoint_arg vec );
	friend	FwPoint		Sign( FwPoint_arg vec );
	friend	FwPoint		Signum( FwPoint_arg vec );
	friend	FwPoint		Recip( FwPoint_arg vec );
	friend	FwPoint		Sqrt( FwPoint_arg vec );
	friend	FwPoint		RecipSqrt( FwPoint_arg vec );
	friend	FwPoint		Max( FwPoint_arg lhs, FwPoint_arg rhs );
	friend	FwPoint		Min( FwPoint_arg lhs, FwPoint_arg rhs );
	friend	FwPoint		Clamp( FwPoint_arg vec, FwPoint_arg mn, FwPoint_arg mx );
	friend	FwScalar	Dot( FwPoint_arg lhs, FwPoint_arg rhs );
	friend	FwScalar	DistSqr( FwPoint_arg lhs, FwPoint_arg rhs );
	friend	FwScalar	DistRcp( FwPoint_arg lhs, FwPoint_arg rhs );
	friend	FwScalar	Dist( FwPoint_arg lhs, FwPoint_arg rhs );
	friend	FwPoint		Lerp( FwPoint_arg lhs, FwPoint_arg rhs, FwScalar_arg alpha );
	friend	bool		AllComponentsLessThan( FwPoint_arg lhs, FwPoint_arg rhs );
	friend	bool		AllComponentsLessThanOrEqual( FwPoint_arg lhs, FwPoint_arg rhs );
	friend	bool		AllComponentsGreaterThan( FwPoint_arg lhs, FwPoint_arg rhs );
	friend	bool		AllComponentsGreaterThanOrEqual( FwPoint_arg lhs, FwPoint_arg rhs );
	friend	bool		AllComponentsEqual( FwPoint_arg lhs, FwPoint_arg rhs );
	friend	FwPoint		operator + ( FwVector_arg lhs, FwPoint_arg rhs );
	friend	FwVector	operator - ( FwPoint_arg lhs, const FwMaths::OriginTag& );
	friend	FwPoint		operator + ( const FwMaths::OriginTag&, FwVector_arg lhs );

private:
	v128	m_value;
};

// Include our inlines
#include <Fw/FwMaths/FwPoint.inl>

#endif	// FW_POINT_H
