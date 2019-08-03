//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.
**/
//--------------------------------------------------------------------------------------------------

#ifndef FP_BOUNDING_BOX_H
#define FP_BOUNDING_BOX_H

//--------------------------------------------------------------------------------------------------
//  INCLUDES
//--------------------------------------------------------------------------------------------------

#include <Fw/FwMaths/FwMathsConstants.h>
#include <Fw/FwMaths/FwVector.h>

class FpAABB;

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
	@class			FpBoundingBox

	@brief			axis aligned bounding box, storage optimised for computing bounds

	@see			FpAABB

	@note			this class has a concept of being empty, which is different from having
					zero volume.
					for example a bounding box containing a single point is not empty, but does
					have zero volume.
**/
//--------------------------------------------------------------------------------------------------

class FpBoundingBox
{
public:

	// constructors
	FpBoundingBox() {}
	FpBoundingBox( const FpBoundingBox& rhs );
	FpBoundingBox( FwPoint_arg minp, FwPoint_arg maxp );
	explicit FpBoundingBox( const FpAABB& b );

	// setting methods
	FpBoundingBox&	operator = ( const FpBoundingBox& rhs );
	void			Set( FwPoint_arg minp, FwPoint_arg maxp );
	void			Set( const FpAABB& b );
	void			SetToEmpty();

	// operations
	void			ExtendBy( FwPoint_arg p );
	void			AddBorder( const float borderWidth );
	bool			ContainsPoint( FwPoint_arg p ) const;

	// access
	bool			IsEmpty() const;
	bool			HasZeroVolume() const;
	FwPoint			GetCentre() const;
	FwVector		GetExtents() const;
	FwPoint			GetMin() const;
	FwPoint			GetMax() const;

protected:

	FwPoint			m_min;						//!< minimum (most negative) point
	FwPoint			m_max;						//!< maximum (most positive) point

};

//--------------------------------------------------------------------------------------------------
//  INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

#include	<Fp/FpGeom/FpAABB.h>

//--------------------------------------------------------------------------------------------------
/**
	@brief			FpBoundingBox copy constructor
**/
//--------------------------------------------------------------------------------------------------

inline FpBoundingBox::FpBoundingBox( const FpBoundingBox& rhs ) : m_min( rhs.m_min ), m_max( rhs.m_max )
{
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			FpBoundingBox constructor from min and max points
**/
//--------------------------------------------------------------------------------------------------

inline FpBoundingBox::FpBoundingBox( FwPoint_arg minp, FwPoint_arg maxp ) : m_min( minp ), m_max( maxp )
{
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			FpBoundingBox constructor from FpAABB
**/
//--------------------------------------------------------------------------------------------------

inline FpBoundingBox::FpBoundingBox( const FpAABB& b )
: m_min( b.GetMin() )
, m_max( b.GetMax() )
{
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			copy operator
**/
//--------------------------------------------------------------------------------------------------

inline FpBoundingBox&	FpBoundingBox::operator = ( const FpBoundingBox& rhs )
{
	m_min = rhs.m_min;
	m_max = rhs.m_max;
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			set to new min and max
**/
//--------------------------------------------------------------------------------------------------

inline void			FpBoundingBox::Set( FwPoint_arg minp, FwPoint_arg maxp )
{
	m_min = minp;
	m_max = maxp;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			set to same box as passed FpAABB
**/
//--------------------------------------------------------------------------------------------------

inline void			FpBoundingBox::Set( const FpAABB& b )
{
	m_min = b.GetMin();
	m_max = b.GetMax();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			sets the bounding box to be empty (see note in class description)
**/
//--------------------------------------------------------------------------------------------------

inline void			FpBoundingBox::SetToEmpty()
{
	m_min = FwPoint( FwMaths::kMaxPosFloat, FwMaths::kMaxPosFloat, FwMaths::kMaxPosFloat );
	m_max = FwPoint( FwMaths::kMinNegFloat, FwMaths::kMinNegFloat, FwMaths::kMinNegFloat );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			extend the bounding box to contain the given point
**/
//--------------------------------------------------------------------------------------------------

inline void			FpBoundingBox::ExtendBy( FwPoint_arg p )
{
	m_min = Min( m_min, p );
	m_max = Max( m_max, p );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Extend the bounding box by a fixed amount in both directions. Use e.g. when you
					want to generate a bounding box for a load of spheres, generate the bounding
					box for the centres and then extend by the largest radius.
**/
//--------------------------------------------------------------------------------------------------

inline void			FpBoundingBox::AddBorder( const float borderWidth )
{
	// Don't want to extend by a negative value as that could invalidate the the bounding box.
	FW_ASSERT( borderWidth > 0.0f );
	FwVector	sizeStep( borderWidth, borderWidth, borderWidth );
	m_min -= sizeStep;
	m_max += sizeStep;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			test if point is contained by box

	@note			a point that is actually coincident with a face of the box is considered 
					to be inside the box
**/
//--------------------------------------------------------------------------------------------------

inline bool			FpBoundingBox::ContainsPoint( FwPoint_arg p ) const
{
	FwVector v = p - GetCentre();
	return AllComponentsLessThanOrEqual( v, GetExtents() );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			find out if the box is empty
**/
//--------------------------------------------------------------------------------------------------

inline bool			FpBoundingBox::IsEmpty() const
{
	return !AllComponentsGreaterThanOrEqual( m_max, m_min );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			find out if the box has zero volume
**/
//--------------------------------------------------------------------------------------------------

inline bool			FpBoundingBox::HasZeroVolume() const
{
	FwVector size = m_max - m_min;
	return ( size.X() * size.Y() * size.Z() ) > 0.0f;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			return the centre of the box
**/
//--------------------------------------------------------------------------------------------------

inline FwPoint			FpBoundingBox::GetCentre() const
{
	return FwPoint( ( ( FwVector(m_min.GetVector4()) + FwVector(m_max.GetVector4()) ) * 0.5f ).GetVector4() );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			get the extents (half-widths) of the box
**/
//--------------------------------------------------------------------------------------------------

inline FwVector		FpBoundingBox::GetExtents() const
{
	return ( m_max - m_min ) * 0.5f;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			
**/
//--------------------------------------------------------------------------------------------------

inline FwPoint			FpBoundingBox::GetMin() const
{
	return m_min;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			
**/
//--------------------------------------------------------------------------------------------------

inline FwPoint			FpBoundingBox::GetMax() const				
{
	return m_max; 
}

#endif // FP_BOUNDING_BOX_H
