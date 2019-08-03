//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.
**/
//--------------------------------------------------------------------------------------------------

#ifndef FP_AABB_H
#define FP_AABB_H

//--------------------------------------------------------------------------------------------------
//  INCLUDES
//--------------------------------------------------------------------------------------------------

#include	<Fp/FpGeom/FpCube.h>

class FpBoundingBox;

//--------------------------------------------------------------------------------------------------
//  SWITCHES
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
//  MACRO DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
//  DECLARATIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
//  CLASS DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@class			FpAABB

	@brief			axis aligned bounding box, storage optimised for collision checks

	@see			FpBoundingBox
**/
//--------------------------------------------------------------------------------------------------

class FpAABB
{
public:

	// constructors
	FpAABB() {}
	FpAABB( const FpAABB& rhs );
	FpAABB( FwPoint_arg p, FwVector_arg v );
	FpAABB( FwPoint_arg p, float h );
	explicit FpAABB( FpCube_arg cube );
	explicit FpAABB( const FpBoundingBox& b );

	// setting methods
	FpAABB&	operator = ( const FpAABB& rhs );
	void	Set( FwPoint_arg p, FwVector_arg v );
	void	Set( FwPoint_arg p, float h );
	void	Set( FwPoint_arg minp, FwPoint_arg maxp );
	void	Set( FpCube_arg cube );
	void	Set( const FpBoundingBox& b );
	void	SetToIntersection( const FpAABB& b0, const FpAABB& b1 );

	// access
	inline	FwPoint		GetCentre() const			{ return m_position; }
	inline	FwVector	GetExtents() const			{ return m_extents; }
	inline	FwPoint		GetMin() const				{ return m_position-m_extents; }
	inline	FwPoint		GetMax() const				{ return m_position+m_extents; }

	// operations
	bool	ContainsPoint( FwPoint_arg p ) const;

protected:

	FwPoint			m_position;						//!< centre of box
	FwVector		m_extents;						//!< extents in the three axis directions

};

// Non-member functions for FpAABB
bool		AABBOverlapsAABB( const FpAABB& b0, const FpAABB& b1 );
bool		AABBTouchesAABB( const FpAABB& b0, const FpAABB& b1, float epsilon=1e-6f );
bool		AABBContainsAABB( const FpAABB& b0, const FpAABB& b1 );

//--------------------------------------------------------------------------------------------------
//  INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

#include	<Fp/FpGeom/FpBoundingBox.h>

//--------------------------------------------------------------------------------------------------
/**
	@brief			FpAABB copy constructor
**/
//--------------------------------------------------------------------------------------------------

inline FpAABB::FpAABB( const FpAABB& rhs ) : m_position( rhs.m_position ), m_extents( rhs.m_extents )
{
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			FpAABB constructor from centre and extents
**/
//--------------------------------------------------------------------------------------------------

inline FpAABB::FpAABB( FwPoint_arg p, FwVector_arg v ) : m_position( p ), m_extents( v )
{
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			FpAABB constructor from centre and symmetric extents
**/
//--------------------------------------------------------------------------------------------------

inline FpAABB::FpAABB( FwPoint_arg p, float h ) : m_position( p ), m_extents( h, h, h )
{
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			FpAABB constructor from cube
**/
//--------------------------------------------------------------------------------------------------

inline FpAABB::FpAABB( FpCube_arg cube )
{
	m_position = cube.GetPosition();
	float h = cube.GetHalfSize();
	m_extents = FwVector( h, h, h );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			FpAABB constructor from FpBoundingBox
**/
//--------------------------------------------------------------------------------------------------

inline FpAABB::FpAABB( const FpBoundingBox& b )
: m_position( b.GetCentre() )
, m_extents( b.GetExtents() )
{
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			copy operator
**/
//--------------------------------------------------------------------------------------------------

inline FpAABB&	FpAABB::operator = ( const FpAABB& rhs )
{
	m_position = rhs.m_position;
	m_extents = rhs.m_extents;
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			set this box to new centre and extents
**/
//--------------------------------------------------------------------------------------------------

inline void	FpAABB::Set( FwPoint_arg p, FwVector_arg v )
{
	m_position = p;
	m_extents = v;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			set this box to new centre and symmetric extents
**/
//--------------------------------------------------------------------------------------------------

inline void	FpAABB::Set( FwPoint_arg p, float h )
{
	m_position = p;
	m_extents = FwVector( h, h, h );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			set this box with min and max points
**/
//--------------------------------------------------------------------------------------------------

inline void	FpAABB::Set( FwPoint_arg minp, FwPoint_arg maxp )
{
	m_position = FwMaths::kOrigin + ( ( FwVector(minp - FwMaths::kOrigin) + FwVector(maxp - FwMaths::kOrigin) ) * 0.5f );
	m_extents = ( maxp - minp ) * 0.5f;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			set this box to a cube
**/
//--------------------------------------------------------------------------------------------------

inline void	FpAABB::Set( FpCube_arg cube )
{
	m_position = cube.GetPosition();
	float h = cube.GetHalfSize();
	m_extents = FwVector( h, h, h );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			set this box to represent the same volume as the passed bounding box
**/
//--------------------------------------------------------------------------------------------------

inline void	FpAABB::Set( const FpBoundingBox& b )
{
	m_position = b.GetCentre();
	m_extents = b.GetExtents();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			set this box to the intersection of two other boxes

	@note			result is undetermined if the two AABB's do not actually overlap
**/
//--------------------------------------------------------------------------------------------------

inline void	FpAABB::SetToIntersection( const FpAABB& b0, const FpAABB& b1 )
{
	FwPoint min0 = b0.m_position - b0.m_extents;
	FwPoint min1 = b1.m_position - b1.m_extents;
	FwPoint max0 = b0.m_position + b0.m_extents;
	FwPoint max1 = b1.m_position + b1.m_extents;

	FwPoint minp = Max( min0, min1 );
	FwPoint maxp = Min( max0, max1 );

	Set( minp, maxp );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			test if point is contained by box

	@note			a point that is actually coincident with a face of the box is considered 
					to be inside the box
**/
//--------------------------------------------------------------------------------------------------

inline bool	FpAABB::ContainsPoint( FwPoint_arg p ) const
{
	FwVector v = Abs( p - GetCentre() );
	return AllComponentsLessThanOrEqual( v, GetExtents() );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			test if two boxes overlap
**/
//--------------------------------------------------------------------------------------------------

inline bool		AABBOverlapsAABB( const FpAABB& b0, const FpAABB& b1 )
{
	FwVector d = Abs( b1.GetCentre() - b0.GetCentre() );
	FwVector bound = b0.GetExtents() + b1.GetExtents();
	return AllComponentsLessThanOrEqual( d, bound );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			test if the first box contains the second
**/
//--------------------------------------------------------------------------------------------------

inline bool		AABBContainsAABB( const FpAABB& b0, const FpAABB& b1 )
{
	// get centre of b1 in b0's frame
	FwVector c = b1.GetCentre() - b0.GetCentre();
	// get max extent point
	FwVector p = Abs( c ) + b1.GetExtents();
	// test against b0's extents
	return AllComponentsLessThanOrEqual( p, b0.GetExtents() );
}

#endif // FP_AABB_H
