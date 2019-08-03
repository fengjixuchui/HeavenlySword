//--------------------------------------------------
//!
//!	\file spatialinterpolator.cpp
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#include "gfx/spatialinterpolator.h"

//--------------------------------------------------
//!
//!	SpatialInterpolator::SpatialInterpolator
//!
//--------------------------------------------------
SpatialInterpolator::SpatialInterpolator( const CPoint* pInputPoints, u_int iNumPoints )
{
	m_iNumPoints = iNumPoints;
	m_pPointArray = NT_NEW_ARRAY_CHUNK( Mem::MC_GFX ) CPoint [iNumPoints];
	m_pResultArray = NT_NEW_ARRAY_CHUNK( Mem::MC_GFX ) PointIDandFraction [iNumPoints];
	NT_MEMCPY( m_pPointArray, pInputPoints, sizeof(CPoint) * iNumPoints );
}

SpatialInterpolator::~SpatialInterpolator()
{
	NT_DELETE_ARRAY_CHUNK( Mem::MC_GFX, m_pPointArray );
	NT_DELETE_ARRAY_CHUNK( Mem::MC_GFX, m_pResultArray );
}





//--------------------------------------------------
//!
//!	SI_DistanceWeight::SI_DistanceWeight
//!
//--------------------------------------------------
SI_DistanceWeight::SI_DistanceWeight( const CPoint* pInputPoints, u_int iNumPoints ) :
	SpatialInterpolator( pInputPoints, iNumPoints )
{
	m_pDistSqArray = NT_NEW_ARRAY_CHUNK( Mem::MC_GFX ) float [iNumPoints];

	// simple one where all points always contribute, so we can have a static
	// results list

	for ( u_int i = 0; i < iNumPoints; i++ )
	{
		m_pResultArray[i].ID = i;
		m_results.push_back( &m_pResultArray[i] );
	}
}

SI_DistanceWeight::~SI_DistanceWeight()
{
	NT_DELETE_ARRAY_CHUNK( Mem::MC_GFX, m_pDistSqArray );
}

//--------------------------------------------------
//!
//!	SI_DistanceWeight::CalcNewResult
//!
//--------------------------------------------------
void SI_DistanceWeight::CalcNewResult( const CPoint& pos )
{
	// calc curr sq distances, and total
	float m_fAccumulator = 0.0f;
	for ( u_int i = 0; i < m_iNumPoints; i++ )
	{
		m_pDistSqArray[i] = 1.0f / (pos - m_pPointArray[i]).LengthSquared();
		m_fAccumulator += m_pDistSqArray[i];
	}

	// calc our fractions
	m_fAccumulator = 1.0f / m_fAccumulator;
	for ( u_int i = 0; i < m_iNumPoints; i++ )
	{
		m_pResultArray[i].ID = i;
		m_pResultArray[i].fraction = m_pDistSqArray[i] * m_fAccumulator;
	}	
}




//--------------------------------------------------
//!
//!	SI_DelaunayTriangulation::SI_DelaunayTriangulation
//!
//--------------------------------------------------
SI_DelaunayTriangulation::SI_DelaunayTriangulation( const CPoint* pInputPoints, u_int iNumPoints ) :
	SpatialInterpolator( pInputPoints, iNumPoints )
{
	// construct a delaunay triangulation, using watson's algorithm
	// extended to 3D space, as outlined in:
	// "Implementing watson's algorithm in three dimensions" by David A. Field.

	// the result of this will be:
	// 1. a list of triangulation points (our input points plus 4 additional triangulation points)
	// 2. a list of integer 4 tuples, corresponding to the tetrahedra forming our triangulation
	// 3. An additional list storing the circumspheres for each tetrahedra is also maintained.
	// 4. (optional) an optimised lookup structure for quickly identifying the tetrahedra
	//  containing a given point.

	// stage 1. the initial selection of the 'super-simplex'
	// a. construct an AABB containing all the sample points.
	// b. construct a sphere that contains the AABB
	// c. construct a tetrahedra that contains the sphere
	// this tetrahedra is the 'super-simplex' used to start the triangulation
	// This is inserted into our list of tetrahedra.
	// (I'd suggest it would be better to base the AABB on the bounds of our world / level
	// as we always want to sample within this super-simplex)

	// stage 2. Insert input points one at a time, for each point:
	// a. determine which circumspheres contain the point (either check linearly
	//  ALL tetrahedras's circumspheres, OR find the tetrahedra containing the point,
	//  and perform a depth first search to find other tetrahedra with circumspheres
	//  that also contain us.
	// b. the union of all the tetrahedra selected in step A form a polyhedron called the 
	//  insertion polyhedron. All the boundary faces of that polyhedron must be identified.
	// c. new tetra hedra are created by connecting the input point to the boundary
	//  faces from step B. these tetrahedra (and their circumspheres) are added to the list
	//  and the old ones that formed the insertion polyhedron are killed.

	// stage 4. Finally, tetrahedra that share a vertex with the super-simplex are killed.

	// caveats.
	// When inserting a point in the mesh, it is possible that a point may be considerd 'on' 
	// the surface of a given hypersphere. This is known as an insertion degeneracy.
	// It is also possible that a change in the insertion polyhedrons volume may occour over the
	// course of insertion. This is know as a volume degeneracy.
	// Finally, newly created tetrahedra in the insertion polyhedron may have circumspheres
	// that have more than 4 triangulation points 'on' or 'inside'. This is known as a secondary degeneracy.
	
	// Proper operation of watsons algorithm can be ensured if insertion degeneracies are avoied.
	// Field suggests using two strategies to cope with this:
	// 1. whenever an insertion degeneracy is detected, delay the insertion of the point
	//  by puting it into an additional list. Reinsertion is then attemped after the original
	//  set of points is processed. Reinsertion is attempted recursively untill the only points
	//  remaining cause one of the three degeneracies listed above.
	// 2. Any remaining points in the list should then be treated as 'inside' a circumsphere
	//  giving rise to the insertion degeneracy, and normal insertion resumed.

	// Though insertion degeneracies seem unlikely, input points aranged as planar arrays
	// of grids give rise to them frequently, as do points following a smooth surface of some kind.
}

SI_DelaunayTriangulation::~SI_DelaunayTriangulation()
{
}

//--------------------------------------------------
//!
//!	SI_DelaunayTriangulation::CalcNewResult
//!
//--------------------------------------------------
void SI_DelaunayTriangulation::CalcNewResult( const CPoint& pos )
{
	UNUSED(pos);

	// 1. find the tetrahedra containing our point.
	//  this will be easier if we've build a KD tree of K=4
	//  (a tetra-tree if you will), based on our final delauny triangulation

	// 2. remove any vertices that are part of the super-simplex (i.e
	//  we've indexed outside of our sample set)

	// 3. use the barycentic coordinates of our input point relative
	//  to our tetrahedra as the lerp values for points in our results list.
	//  obviously adjusting the order of barycentric evaluation based on 2.
}


