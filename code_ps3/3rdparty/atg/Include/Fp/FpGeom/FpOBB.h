//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.
**/
//--------------------------------------------------------------------------------------------------

#ifndef FP_OBB_H
#define FP_OBB_H

//--------------------------------------------------------------------------------------------------
//  INCLUDES
//--------------------------------------------------------------------------------------------------

#include	<Fw/FwMaths/FwTransform.h>
#include	<Fw/FwMaths/FwVector.h>

class FpAABB;
class FpPlane;

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
	@class			FpOBB

	@brief			oriented bounding box

	@note			Internally this class currently stores the world-to-local matrix that defines
					the box's frame, which makes certain intersection tests more efficient.
					This is the inverse of many OBB classes you see.
					Ultimately we should examime the usage patterns and if necessary adjust the
					storage accordingly.
**/
//--------------------------------------------------------------------------------------------------

class FpOBB
{
public:

	// constructors
	FpOBB() {}
	FpOBB( const FpOBB& rhs );
	FpOBB( const FwTransform& wtl, FwVector_arg v );
	FpOBB( const FwTransform& wtl, float h );
	explicit FpOBB( const FpAABB& b );

	// setting methods
	FpOBB&	operator = ( const FpOBB& rhs );
	void	Set( const FwTransform& wtl, FwVector_arg v );
	void	Set( const FwTransform& wtl, float h );
	void	Set( const FpAABB& b );
	void	SetFromAABBWithTransform( const FpAABB& b, const FwTransform& ltw );
	void	SetFromAABBWithInverseTransform( const FpAABB& b, const FwTransform& wtl );
	bool	Set( FwPoint_arg p0, FwPoint_arg px, FwPoint_arg py );

	// access
	const FwTransform&	GetWorldToLocal() const		{ return m_worldToLocal; }
	FwVector			GetExtents() const			{ return m_extents; }
	FwPoint				GetCentre() const			{ return -m_worldToLocal.GetTranslation(); }	// In world space
	
	// operations
	bool		ContainsPoint( FwPoint_arg p ) const;
	FwScalar	DistanceSquaredToPoint( FwPoint_arg p ) const;
	void		ApplyTransform( const FwTransform& xform );
	void		ConstructPlanes( FpPlane* pPlanes ) const;

protected:

	FwTransform		m_worldToLocal;					//!< world-to-local matrix describing box's frame
	FwVector		m_extents;						//!< extents in the three axis directions

};

// Non-member functions for FpOBB
bool		OBBOverlapsOBB( const FpOBB& b0, const FpOBB& b1 );

//--------------------------------------------------------------------------------------------------
//  INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

#include	<Fp/FpGeom/FpAABB.h>

//--------------------------------------------------------------------------------------------------
/**
	@brief			FpOBB copy constructor
**/
//--------------------------------------------------------------------------------------------------

inline FpOBB::FpOBB( const FpOBB& rhs ) : m_worldToLocal( rhs.m_worldToLocal ), m_extents( rhs.m_extents )
{
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			FpOBB constructor from world-to-local matrix and extents
**/
//--------------------------------------------------------------------------------------------------

inline FpOBB::FpOBB( const FwTransform& wtl, FwVector_arg v ) : m_worldToLocal( wtl ), m_extents( v )
{
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			FpOBB constructor from world-to-local matrix and symmetric extents
**/
//--------------------------------------------------------------------------------------------------

inline FpOBB::FpOBB( const FwTransform& wtl, float h ) : m_worldToLocal( wtl ), m_extents( h, h, h )
{
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			FpOBB constructor from FpAABB
**/
//--------------------------------------------------------------------------------------------------

inline FpOBB::FpOBB( const FpAABB& b )
: m_worldToLocal( -b.GetCentre() )
, m_extents( b.GetExtents() )
{
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			copy operator
**/
//--------------------------------------------------------------------------------------------------

inline FpOBB&	FpOBB::operator = ( const FpOBB& rhs )
{
	m_worldToLocal = rhs.m_worldToLocal;
	m_extents = rhs.m_extents;
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			set this box to new frame and extents

	@param			wtl				-	world-to-local matrix
	@param			v				-	extents
**/
//--------------------------------------------------------------------------------------------------

inline void	FpOBB::Set( const FwTransform& wtl, FwVector_arg v )
{
	m_worldToLocal = wtl;
	m_extents = v;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			set this box to new frame and extents

	@param			wtl				-	world-to-local matrix
	@param			h				-	symmetric extents
**/
//--------------------------------------------------------------------------------------------------

inline void	FpOBB::Set( const FwTransform& wtl, float h )
{
	m_worldToLocal = wtl;
	m_extents = FwVector( h, h, h );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			set this box from an AABB
**/
//--------------------------------------------------------------------------------------------------

inline void	FpOBB::Set( const FpAABB& b )
{
	m_worldToLocal = FwTransform( -b.GetCentre() );
	m_extents = b.GetExtents();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			set this box from a transformed AABB

	@param			t		- A local-to-world transform i.e. the transform that takes the AABB
								and transforms it into the desired position of the OBB.

	@note			The method uses OrthoInverse() on t. If you already have the inverse, or indeed
					if it's easier to construct the inverse in your context,
					use SetFromAABBWithInverseTransform().
**/
//--------------------------------------------------------------------------------------------------

inline void	FpOBB::SetFromAABBWithTransform( const FpAABB& b, const FwTransform& ltw )
{
	m_worldToLocal = OrthogonalInverse( ltw ) * FwTransform( -b.GetCentre() );
	m_extents = b.GetExtents();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			set this box from a transformed AABB

	@param			t		- The inverse of the transform that takes the AABB
								and transforms it into the desired position of the OBB.
**/
//--------------------------------------------------------------------------------------------------

inline void	FpOBB::SetFromAABBWithInverseTransform( const FpAABB& b, const FwTransform& wtl )
{
	m_worldToLocal = wtl * FwTransform( -b.GetCentre() );
	m_extents = b.GetExtents();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			test if point is contained by box

	@note			a point that is actually coincident with a face of the box is considered 
					to be inside the box
**/
//--------------------------------------------------------------------------------------------------

inline bool	FpOBB::ContainsPoint( FwPoint_arg p ) const
{
	// transform point to local space
	FwPoint local = p * m_worldToLocal;

	local = Abs( local );
	return AllComponentsLessThanOrEqual( FwVector( local - FwMaths::kOrigin ), m_extents );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Transform the OBB by the given matrix.
**/
//--------------------------------------------------------------------------------------------------

inline void	FpOBB::ApplyTransform( const FwTransform& xform )
{
	m_worldToLocal = OrthogonalInverse( xform ) * m_worldToLocal;
}

#endif // FP_OBB_H
