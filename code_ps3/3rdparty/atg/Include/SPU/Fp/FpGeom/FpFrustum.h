//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		Frustums

	@note		(c) Copyright Sony Computer Entertainment 2005. All Rights Reserved.
**/
//--------------------------------------------------------------------------------------------------

#ifndef FP_FRUSTUM_H
#define FP_FRUSTUM_H

//--------------------------------------------------------------------------------------------------
//  INCLUDES
//--------------------------------------------------------------------------------------------------

#include	<Fp/FpGeom/FpCube.h>
#include	<Fp/FpGeom/FpPlane.h>
#include	<Fp/FpGeom/FpSphere.h>

//--------------------------------------------------------------------------------------------------
//  DECLARATIONS
//--------------------------------------------------------------------------------------------------

class FpFrustum;
class FpFrustumTransposed;
class FpAABB;
class FpOBB;

//--------------------------------------------------------------------------------------------------
//  CLASS DEFINITIONS
//--------------------------------------------------------------------------------------------------

namespace FpGeom
{
	/// cull test results
	enum TestResult
	{
		kInside,					//!< test object entirely within frustum
		kOutside,					//!< test object entirely outside of frustum
		kUnclear,					//!< test could not determine inside or outside
	};

	/// cull test modes
	enum CullMode
	{
		kUseAllPlanes,				//!< use all 6 planes for cull checks
		kIgnoreFarPlane,			//!< use all but far plane
	};
};


//--------------------------------------------------------------------------------------------------
/**
	@class			FpFrustum

	@brief			Represents a general frustum, designed for collision/culling queries.

	@note			Internally the frustum is represented by six bounding planes. The normals of the
					planes point into the volume defined by the frustum.

	@note			By convention, the planes are ordered as follows (if the specific usage allows 
					this to be meaningful):

									0	-	Far plane
									1	-	Near plane
									2	-	Left plane
									3	-	Right plane
									4	-	Bottom plane
									5	-	Top plane

	@see			FpFrustumTransposed
**/
//--------------------------------------------------------------------------------------------------

class FpFrustum
{
public:

	// constructors
	inline	FpFrustum() {}
	explicit inline	FpFrustum( const FpFrustum& rhs );
	explicit inline	FpFrustum( const FpFrustumTransposed& rhs );
	explicit inline	FpFrustum( const FpPlane* pPlanes );
	explicit inline	FpFrustum( const FwVector4* pTransposed );

	// assignment methods
	inline	FpFrustum&		operator = ( const FpFrustum& rhs );
	inline	FpFrustum&		operator = ( const FpFrustumTransposed& rhs );

	// setting methods
	inline	void			SetFromPlanes( const FpPlane* pPlanes );
	void					SetFromTransposedPlanes( const FwVector4* pTransposed );
	void					SetPerspective( float fieldOfViewY, float aspect, float nearClip, float farClip );
	void					SetFrustum( float left, float right, float bottom, float top, float nearClip, float farClip );
	void					SetFrustum( float width, float height, float nearClip, float farClip );
	void					SetOrtho( float left, float right, float bottom, float top, float nearClip, float farClip );

	// access
	inline	const FpPlane*	GetPlanes( void ) const;

	// operations
	void					Transform( const FwMatrix44& matrix );

	// queries
	FpGeom::TestResult		TestAABB( const FpAABB& box, FpGeom::CullMode mode=FpGeom::kUseAllPlanes ) const;
	FpGeom::TestResult		TestCube( FpCube_arg cube, FpGeom::CullMode mode=FpGeom::kUseAllPlanes ) const;
	FpGeom::TestResult		TestOBB( const FpOBB& box, FpGeom::CullMode mode=FpGeom::kUseAllPlanes ) const;
	FpGeom::TestResult		TestSphere( FpSphere_arg sphere, FpGeom::CullMode mode=FpGeom::kUseAllPlanes ) const;

protected:
	FpPlane		m_planes[6];

};


//--------------------------------------------------------------------------------------------------
/**
	@class			FpFrustumTransposed

	@brief			Represents a general frustum, designed for collision/culling queries.

	@note			Internally the frustum is represented by six bounding planes. The normals of the
					planes point into the volume defined by the frustum. In this implementation, the
					planes are stored swizzled for optimised tests. As such, the frustum is defined
					by 8 FwVector4 objects (albeit only the X, Y, and Z components).

	@note			By convention, the planes are ordered as follows (if the specific usage allows 
					this to be meaningful):

									0	-	Far plane
									1	-	Near plane
									2	-	Left plane
									3	-	Right plane
									4	-	Bottom plane
									5	-	Top plane

	@note			The swizzled/transposed layout of the 8 FwVector4 objects is two groups of 4,
					as shown below:

									( plane[0].x, plane[1].x, plane[2].x )
									( plane[0].y, plane[1].y, plane[2].y )
									( plane[0].z, plane[1].z, plane[2].z )
									( plane[0].w, plane[1].w, plane[2].w )
									( plane[3].x, plane[4].x, plane[5].x )
									( plane[3].y, plane[4].y, plane[5].y )
									( plane[3].z, plane[4].z, plane[5].z )
									( plane[3].w, plane[4].w, plane[5].w )


	@see			FpFrustum
**/
//--------------------------------------------------------------------------------------------------

class FpFrustumTransposed
{
public:

	// constructors
	inline			FpFrustumTransposed() {}
	explicit inline	FpFrustumTransposed( const FpFrustumTransposed& rhs );
	explicit inline	FpFrustumTransposed( const FpFrustum& rhs );
	explicit inline	FpFrustumTransposed( const FpPlane* pPlanes );
	explicit inline	FpFrustumTransposed( const FwVector4* pTransposed );
	
	// assignment methods
	inline	FpFrustumTransposed&	operator = ( const FpFrustumTransposed& rhs );
	inline	FpFrustumTransposed&	operator = ( const FpFrustum& rhs );

	// setting methods
	void					SetFromPlanes( const FpPlane* pPlanes );
	inline	void			SetFromTransposedPlanes( const FwVector4* pTransposed );

	// access
	inline const FwVector4*	GetTransposedPlanes( void ) const;

	// queries
	FpGeom::TestResult		TestSphere( FpSphere_arg sphere ) const;

protected:
	FwVector4	m_transposed[8];

};


//--------------------------------------------------------------------------------------------------
//  INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@brief			Constructs from an FpFrustum object
	@param			rhs				Source FpFrustum object
**/
//--------------------------------------------------------------------------------------------------

FpFrustum::FpFrustum( const FpFrustum& rhs )
{
	SetFromPlanes( rhs.GetPlanes() );
}


//--------------------------------------------------------------------------------------------------
/**
	@brief			Constructs from an FpFrustumTransposed object
	@param			rhs				Source FpFrustumTransposed object
**/
//--------------------------------------------------------------------------------------------------

FpFrustum::FpFrustum( const FpFrustumTransposed& rhs )
{
	SetFromTransposedPlanes( rhs.GetTransposedPlanes() );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Constructs from an array of 6 FpPlane objects
	@param			pPlanes			Pointer to an array 6 FpPlane objects
**/
//--------------------------------------------------------------------------------------------------

FpFrustum::FpFrustum( const FpPlane* pPlanes )
{
	FW_ASSERT( pPlanes );
	SetFromPlanes( pPlanes );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Constructs from an array of 8 FwVector4 objects representing transposed planes
	@param			pPlanes			Pointer to an array 8 FwVector4 objects
	@note			see FpFrustumTransposed for the storage layout associated with transposed planes
**/
//--------------------------------------------------------------------------------------------------

FpFrustum::FpFrustum( const FwVector4* pTransposedPlanes )
{
	FW_ASSERT( pTransposedPlanes );
	SetFromTransposedPlanes( pTransposedPlanes );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Assigns from an FpFrustum object
	@param			rhs				Source FpFrustum object
**/
//--------------------------------------------------------------------------------------------------

FpFrustum&	FpFrustum::operator = ( const FpFrustum& rhs )
{
	SetFromPlanes( rhs.GetPlanes() );
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Assigns from an FpFrustumTransposed object
	@param			rhs				Source FpFrustumTransposed object
**/
//--------------------------------------------------------------------------------------------------

FpFrustum&	FpFrustum::operator = ( const FpFrustumTransposed& rhs )
{
	SetFromTransposedPlanes( rhs.GetTransposedPlanes() );
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Sets frustum from an array of 6 FpPlane objects at the specified address
	@param			pPlanes			Pointer to an array 6 FpPlane objects
**/
//--------------------------------------------------------------------------------------------------

void	FpFrustum::SetFromPlanes( const FpPlane* pPlanes )
{
	FW_ASSERT( pPlanes );
	for (int i=0; i<6; i++)
		m_planes[i] = pPlanes[i];
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns a read-only pointer to an array of 6 planes that form the frustum
**/
//--------------------------------------------------------------------------------------------------

const FpPlane*	FpFrustum::GetPlanes( void ) const
{
	return m_planes;
}



//--------------------------------------------------------------------------------------------------
/**
	@brief			Constructs from an FpFrustumTransposed object
	@param			rhs				Source FpFrustumTransposed object
**/
//--------------------------------------------------------------------------------------------------

FpFrustumTransposed::FpFrustumTransposed( const FpFrustumTransposed& rhs )
{
	SetFromTransposedPlanes( rhs.GetTransposedPlanes() );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Constructs from an FpFrustum object
	@param			rhs				Source FpFrustum object
**/
//--------------------------------------------------------------------------------------------------

FpFrustumTransposed::FpFrustumTransposed( const FpFrustum& rhs )
{
	SetFromPlanes( rhs.GetPlanes() );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Constructs from an array of 6 FpPlane objects
	@param			pPlanes			Pointer to an array 6 FpPlane objects
**/
//--------------------------------------------------------------------------------------------------

FpFrustumTransposed::FpFrustumTransposed( const FpPlane* pPlanes )
{
	FW_ASSERT( pPlanes );
	SetFromPlanes( pPlanes );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Constructs from an array of 8 FwVector4 objects representing transposed planes
	@param			pPlanes			Pointer to an array 8 FwVector4 objects
	@note			see FpFrustumTransposed for the storage layout associated with transposed planes
**/
//--------------------------------------------------------------------------------------------------

FpFrustumTransposed::FpFrustumTransposed( const FwVector4* pTransposedPlanes )
{
	FW_ASSERT( pTransposedPlanes );
	SetFromTransposedPlanes( pTransposedPlanes );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Assigns from an FpFrustum object
	@param			rhs				Source FpFrustum object
**/
//--------------------------------------------------------------------------------------------------

FpFrustumTransposed&	FpFrustumTransposed::operator = ( const FpFrustum& rhs )
{
	SetFromPlanes( rhs.GetPlanes() );
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Assigns from an FpFrustumTransposed object
	@param			rhs				Source FpFrustumTransposed object
**/
//--------------------------------------------------------------------------------------------------

FpFrustumTransposed&	FpFrustumTransposed::operator = ( const FpFrustumTransposed& rhs )
{
	SetFromTransposedPlanes( rhs.GetTransposedPlanes() );
	return *this;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Sets transposed frustum from an array of 8 FwVector4 objects
	@param			pTransposedPlanes	Pointer to an array 8 FwVector4 objects
**/
//--------------------------------------------------------------------------------------------------

void	FpFrustumTransposed::SetFromTransposedPlanes( const FwVector4* pTransposedPlanes )
{
	FW_ASSERT( pTransposedPlanes );
	for (int i=0; i<8; i++)
		m_transposed[i] = pTransposedPlanes[i];
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns a read-only pointer to an array of 8 FwVector4 objects representing the
					transposed planes.
**/
//--------------------------------------------------------------------------------------------------

const FwVector4*	FpFrustumTransposed::GetTransposedPlanes( void ) const
{
	return m_transposed;
}

#endif // FP_FRUSTUM_H
