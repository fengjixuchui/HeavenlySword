/***************************************************************************************************
*
*	$Header:: /game/frustum.h 3     24/07/03 11:53 Simonb                                          $
*
*
*
*	CHANGES
*
*	16/6/2003	SimonB	Created
*
***************************************************************************************************/

#ifndef CORE_FRUSTUM_H
#define CORE_FRUSTUM_H

#include "core/boundingvolumes.h"

// forward declarations
class CCamera;

template<class ElementType, class SpaceHierarchyPolicy>
class CPortal;

/***************************************************************************************************
*
*	CLASS			CPlane
*
*	DESCRIPTION		A plane bounding volume class.
*
***************************************************************************************************/

//! A plane class.
/*! The "interior" of the plane is defined to be the half-space { v | v.n < d }
*/
class CPlane
{
public:
	//! Creates a ground plane.
	CPlane() : m_obNormal(0.0f, 1.0f, 0.0f), m_fDistance(0.0f) {}

	//! Creates a plane from a normal and distance.
	CPlane(CDirection const& obNormal, float fDistance) : m_obNormal(obNormal), m_fDistance(fDistance) {}

	//! Creates a plane from a triangle.
	CPlane(CPoint const& obA, CPoint const& obB, CPoint const& obC);

	//! Creates a plane from 4 float poked directly into the .
	CPlane(float a, float b, float c, float d);


	//! Gets the normal.
	CDirection const& GetNormal() const { return m_obNormal; }
	float const& GetDistance() const { return m_fDistance; }


	//! Returns true if the plane contains a point.
	bool ContainsPoint(CPoint const& obPoint) const;
	
	//! Returns true if the plane contains any part of an oriented bounding box.
	bool IntersectsOBB(CMatrix const& obTransform, CDirection const& obHalfLengths) const;

	//! Returns the signed height of a point to the plane.
	float GetHeight(CPoint const& obPoint) const;

	//! Returns true if the line segment intersects the plane
	bool IntersectsLineSeg(const CPoint& obStart, const CPoint& obEnd, CPoint& obResult ) const;

	//! return true if the swept sphere intersects the plane also return the two t values if true
	bool IntersectsSweptSphere( const CSphereBound& bound, const CDirection& sweptDir, float& fOutT0, float& fOutT1 ) const;

private:
	CDirection m_obNormal;	//!< The normal to the plane (always kept normalised).
	float m_fDistance;		//!< The distance along the normal to the plane's surface.
};

/***************************************************************************************************
*
*	CLASS			CFrustum
*
*	DESCRIPTION		A capped square-based pyramid bounding volume class.
*					Added functionality from NVIDIA's PSM demo to cull shadow volumes
*
***************************************************************************************************/

//! A bounding volume enclosed by a camera.
class CFrustum
{
public:
	//! Creates an empty frustum.
	CFrustum();

	//! Creates a camera-space frustum from the given camera.
	CFrustum(const CCamera* pobCamera, float fAspectRatio);

	//! Gets the camera for this frustum.
	const CCamera* GetCamera() const { return m_pobCamera; }

	//! Returns true if this frustum has non-zero volume.
	bool IsValid() const { return m_bValid; }


	//! Returns true if the frustum intersects an oriented bounding box.
	/*! The bounding box must be in defined in view space.
	*/
	bool IntersectsOBB(CMatrix const& obTransform, CDirection const& obHalfLengths) const;

	//! Renders the frustum in wireframe.
	void DebugRender(uint32_t dwColour) const;

	//! Gets the 8 extreme points of the frustum.
	void GetExtremePoints(CPoint* pobPositions) const;
	//! Gets the 6 clipping planes of the frustum.
	void GetClipPlanes(CPlane* pobPlanes) const;

	//! Gets the 8 extreme points of the frustum in the space provided.
	void GetExtremePoints(CPoint* pobPositions, const CMatrix& matrix ) const;
	//! Gets the 6 clipping planes of the frustum in the space provided.
	void GetClipPlanes(CPlane* pobPlanes, const CMatrix& matrix ) const;


private:

	//! Creatss the clipping planes from the frustum bounds.
	void CreateClipPlanes();

	//! creates the clipping panes from the frustum in the space provided.
	void CreateClipPlanes( const CMatrix& matrix );

	const CCamera* m_pobCamera;			//!< The camera used to generate this frustum.
	float m_fAspectRatio;				//!< The aspect ratio used to generate this frustum.
	CDirection m_obBoundsMin;			//!< The post-projective bounds minimum of the frustum.
	CDirection m_obBoundsMax;			//!< The post-projective bounds maximum of the frustum.
	bool m_bValid;						//!< True if this frustum has non-zero volume.

	CPlane m_aobClipPlanes[6];			//!< The bounding clip planes, in view space.

};

//***************************************************************************************************
//!
//! Simon's magic magic frustum beats me, this is a new hopefully simplier frustum thats probaby slower
//! but should work as I expect... Deano
//! Based on NVIDIA PSM frustum
//!
//! Yes I'm naughty and dropped back to D3DX stuff but I've had a bad week and I'm sick of frustums!!!
//!
//***************************************************************************************************
class CullingFrustum
{
public:
	CullingFrustum(){}

	//! takes a matrix to build a frustum around (usually a view projection matrix)
	CullingFrustum( const CMatrix& matrix );

	enum TEST_STATUS
	{
		COMPLETELY_OUTSIDE = 0,		//!< wholly outside the frustum
		PARTIALLY_INSIDE = 1,		//!< partially in and out (also used when the function is unable to tell the inside status)
		COMPLETELY_INSIDE = 2,		//!< wholly inside the frustum
	};

	//! take a sphere and either returns COMPLETE_OUTSIDE or PARTIALLY_INSIDE classification
    TEST_STATUS TestSphere     ( const CSphereBound* sphere ) const;
	//! take an world space AABB and return exact status where it is
    TEST_STATUS  TestBox ( const CAABB* box ) const;
	//! take an world space AABB and either returns COMPLETE_OUTSIDE or PARTIALLY_INSIDE classification
    TEST_STATUS FastTestBox ( const CAABB* box ) const;
	//! sweep a world sphere through world direction, and either returns COMPLETE_OUTSIDE or PARTIALLY_INSIDE classification
    TEST_STATUS TestSweptSphere( const CSphereBound* sphere, const CDirection* sweepDir ) const;

	//! returns a world space AABB containing the frustum
	CAABB GetFrustumBox();

	//! returns the points intersecting the frustum
	void GetExtremePoints( CPoint outPoints[8] );

	//! return the minimal AABB of the box and the frustum (current implemention is recursive split not ananylatical, the min split distance
	//! is a parameter to control speed versus accuracy
	CAABB Intersect( const CAABB& box, const float minSplitDistance ) const;

	//! outVertex should be at least vertexCount*2*6 elements
	void ClipPolyAgainstPlane( const CPlane& plane, int vertexCount, CPoint* inVertex, int& outVertexCount, CPoint* outVertex ) const;
	void ClipPolyAgainstPlane( const CVector& plane, int vertexCount, CPoint* inVertex, int& outVertexCount, CPoint* outVertex ) const
	{
		CPlane xplane( plane.X(), plane.Y(), plane.Z(), plane.W() );
		ClipPolyAgainstPlane( xplane, vertexCount, inVertex, outVertexCount, outVertex );
	}

	//! inVertex and outVertex should be at least vertexCount*2*6 elements, both will be trashed and the result array is returned
	CPoint* ClipPoly( const int vertexCount, CPoint* inVertex, int& outVertexCount, CPoint* tmpBuffer ) const;

	CPlane* GetPlane( unsigned int i )
	{
		return m_CamPlanes+i;
	}
private:

	void RecursiveIntersect( const CAABB& testBox, const float minSplitDistance, CAABB& resultBox, int recursionLevel ) const;

	CPlane		m_CamPlanes[6];		//!< actual planes
    
	unsigned int	m_nVertexLUT[6];	//!< a fast AABB lookup table
    CPoint			m_pntList[8];		//!< points making up the extremes
	CAABB			m_frustumBox;		//!< AABB containing the frustum (for numerical reasons is actually slightly smaller);

	CMatrix				m_frustumMatrix;	//!< the frustum matrix (whats passed in) takes points to frustum space from world
	mutable CMatrix		m_invFrustumMatrix;	//!< the inverse frustum matrix takes points from frustum space to world
	
	mutable bool		m_bInverseOk;		//!< flag to see if the we need to compute the inverse
};


#endif // ndef _FRUSTUM_H
