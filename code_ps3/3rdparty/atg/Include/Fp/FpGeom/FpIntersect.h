//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		low-level collision/culling stuff

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.
**/
//--------------------------------------------------------------------------------------------------

#ifndef FP_INTERSECT_H
#define FP_INTERSECT_H

//--------------------------------------------------------------------------------------------------
//  INCLUDES
//--------------------------------------------------------------------------------------------------

#include	<Fp/FpGeom/FpAABB.h>
#include	<Fp/FpGeom/FpCube.h>
#include	<Fp/FpGeom/FpOBB.h>
#include	<Fp/FpGeom/FpPlane.h>
#include	<Fp/FpGeom/FpSphere.h>
#include	<Fp/FpGeom/FpRay.h>
#include	<Fp/FpGeom/FpLine.h>
#include	<Fp/FpGeom/FpLineSegment.h>

//--------------------------------------------------------------------------------------------------
//  SWITCHES
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
//  MACRO DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
//  DECLARATIONS
//--------------------------------------------------------------------------------------------------

class FpAABB;

//--------------------------------------------------------------------------------------------------
//  CLASS DEFINITIONS
//--------------------------------------------------------------------------------------------------

/// @todo os - should these be in a namespace? 
/// cf. SignedDistance( FpPlane_arg , FwPoint_arg ) which isn't

		float			SignedDistance( FpPlane_arg plane, const FpAABB& box );
		FpPlane::Side	FindSide( FpPlane_arg plane, const FpAABB& box );
inline	float			SignedDistance( FpPlane_arg plane, FpCube_arg cube );
inline	FpPlane::Side	FindSide( FpPlane_arg plane, FpCube_arg cube );
		float			SignedDistance( FpPlane_arg plane, const FpOBB& box );
		FpPlane::Side	FindSide( FpPlane_arg plane, const FpOBB& box );
inline	float			SignedDistance( FpPlane_arg plane, FpSphere_arg sphere );
inline	FpPlane::Side	FindSide( FpPlane_arg plane, FpSphere_arg sphere );

inline	bool			SphereOverlapsSphere( FpSphere_arg s0, FpSphere_arg s1 );
inline	bool			SphereContainsSphere( FpSphere_arg s0, FpSphere_arg s1 );
		bool			SphereIntersectsRay( FpSphere_arg sphere, FwPoint_arg p1, FwPoint_arg p2 );

		bool			PlaneIntersectLine( FpPlane_arg plane, const FpLine& line, FwPoint& intersection );
		bool			PlaneIntersectLine( FpPlane_arg plane, FwPoint_arg p, FwVector_arg v, FwPoint& intersection );
		bool			PlaneIntersectLineSegment( FpPlane_arg plane, const FpLineSegment& line, FwPoint& intersection, float* pT=NULL );
		bool			PlaneIntersectLineSegment( FpPlane_arg plane, FwPoint_arg p0, FwPoint_arg p1, FwPoint& intersection, float* pT=NULL );
		bool			PlaneIntersectRay( FpPlane_arg plane, const FpRay& ray, FwPoint& intersection, float* pT=NULL );
		bool			PlaneIntersectRay( FpPlane_arg plane, FwPoint_arg p, FwVector_arg v, FwPoint& intersection, float* pT=NULL );
		bool			IntersectionOfThreePlanes( FpPlane_arg plane0, FpPlane_arg plane1, FpPlane_arg plane2, FwPoint& intersection );

		bool			IntersectionOfTwoMovingSpheres( FpSphere_arg s0, FpSphere_arg s1, FwPoint_arg p0, FwPoint_arg p1, float& t0, float& t1 );

inline	bool			OBBOverlapsSphere( const FpOBB& b, FpSphere_arg s );
		bool			OBBIntersectLine( const FpOBB& b, const FpLine& line );
		bool			OBBIntersectLine( const FpOBB& b, FwPoint_arg p, FwVector_arg v );
		bool			OBBIntersectLineSegment( const FpOBB& b, const FpLineSegment& line );
		bool			OBBIntersectLineSegment( const FpOBB& b, FwPoint_arg p0, FwPoint_arg p1 );
		bool			OBBIntersectRay( const FpOBB& b, const FpRay& ray );
		bool			OBBIntersectRay( const FpOBB& b, FwPoint_arg p, FwVector_arg v );
		bool			OBBIntersectRay( const FpOBB& b, FwPoint_arg p, FwVector_arg v, FwPoint& intersection, float* pT=NULL );
		bool			OBBIntersectRay( const FpOBB& b, const FpRay& ray, FwPoint& intersection, float* pT=NULL );

inline	bool			AABBOverlapsSphere( const FpAABB& b, FpSphere_arg s );
		bool			AABBOverlapsTriangle( const FpAABB& b, FwPoint_arg p0, FwPoint_arg p1, FwPoint_arg p2 );
		bool			AABBIntersectLine( const FpAABB& b, const FpLine& line );
		bool			AABBIntersectLine( const FpAABB& b, FwPoint_arg p, FwVector_arg v );
		bool			AABBIntersectLineSegment( const FpAABB& b, const FpLineSegment& line );
		bool			AABBIntersectLineSegment( const FpAABB& b, FwPoint_arg p0, FwPoint_arg p1 );
		bool			AABBIntersectRay( const FpAABB& b, const FpRay& ray );
		bool			AABBIntersectRay( const FpAABB& b, FwPoint_arg p, FwVector_arg v );
		bool			AABBIntersectRay( const FpAABB& b, FwPoint_arg p, FwVector_arg v, FwPoint& intersection, float* pT=NULL );
		bool			AABBIntersectRay( const FpAABB& b, const FpRay& ray, FwPoint& intersection, float* pT=NULL );

		FwPoint			ClosestPointOnLineToPoint( FwPoint_arg point, const FpLine& line );
		FwPoint			ClosestPointOnLineToPoint( FwPoint_arg point, FwPoint_arg p, FwVector_arg v );

//--------------------------------------------------------------------------------------------------
//  INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@brief			find signed distance between plane and (surface of) cube

	@param			plane			-	the plane
	@param			cube			-	the cube

	@return			distance, positive on side of normal, zero if cube intersects plane
**/
//--------------------------------------------------------------------------------------------------

float			SignedDistance( FpPlane_arg plane, FpCube_arg cube )
{
	FpAABB aabb;
	aabb.Set( cube );
	return SignedDistance( plane, aabb );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			find which side of a plane a cube is

	@param			plane			-	the plane
	@param			cube			-	the cube

	@return			enum value
**/
//--------------------------------------------------------------------------------------------------

FpPlane::Side	FindSide( FpPlane_arg plane, FpCube_arg cube )
{
	FpAABB aabb;
	aabb.Set( cube );
	return FindSide( plane, aabb );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			find signed distance between plane and (surface of) sphere

	@param			plane			-	the plane
	@param			sphere			-	the sphere

	@return			distance, positive on side of normal, zero if sphere intersects plane
**/
//--------------------------------------------------------------------------------------------------

float			SignedDistance( FpPlane_arg plane, FpSphere_arg sphere )
{
    float d = SignedDistance( plane, sphere.GetPosition() );
	float r = sphere.GetRadius();
	if ( d >= 0.0f )
		return max( d - r, 0.0f );
	return min( d + r, 0.0f );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			find which side of a plane a sphere is

	@param			plane			-	the plane
	@param			sphere			-	the sphere

	@return			enum value
**/
//--------------------------------------------------------------------------------------------------

FpPlane::Side	FindSide( FpPlane_arg plane, FpSphere_arg sphere )
{
    float d = SignedDistance( plane, sphere.GetPosition() );
	float r = sphere.GetRadius();
	return ( d > r ? FpPlane::kSideOfNormal : ( d < -r ? FpPlane::kSideOppositeNormal : FpPlane::kOnPlane ) );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			intersection test for two spheres

	@param			s0				-	one sphere
	@param			s1				-	the other

	@return			true if they overlap
**/
//--------------------------------------------------------------------------------------------------

bool			SphereOverlapsSphere( FpSphere_arg s0, FpSphere_arg s1 )
{
	return LengthSqr( s0.GetPosition() - s1.GetPosition() ) <= sqr( s0.GetRadius() + s1.GetRadius() );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			see if first sphere contains the second

	@param			s0				-	one sphere
	@param			s1				-	the other

	@return			true if first sphere contains the second
**/
//--------------------------------------------------------------------------------------------------

bool			SphereContainsSphere( FpSphere_arg s0, FpSphere_arg s1 )
{
	return ( Length( s0.GetPosition() - s1.GetPosition() ) + s1.GetRadius() ) <= s0.GetRadius();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			overlap test for OBB and sphere

	@param			b				-	the OBB
	@param			s				-	the sphere

	@return			true if overlap

	@note			This function is based on Arvo's standard algorithm (See Graphics Gems).
**/
//--------------------------------------------------------------------------------------------------

bool			OBBOverlapsSphere( const FpOBB& b, FpSphere_arg s )
{
	// work with sphere in box's local frame
	FwPoint p = s.GetPosition() * b.GetWorldToLocal();

	// now use AABB check
	FwVector v = b.GetExtents();
	float r = s.GetRadius();
	FwPoint minSeparation = Min( p + v, FwMaths::kZero );
	FwPoint maxSeparation = Max( p - v, FwMaths::kZero );
	return ( LengthSqr( minSeparation - FwMaths::kOrigin ) + LengthSqr( maxSeparation - FwMaths::kOrigin ) ) <= sqr( r );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			overlap test for AABB and sphere

	@param			b				-	the AABB
	@param			s				-	the sphere

	@return			true if overlap

	@note			This function is based on Arvo's standard algorithm (See Graphics Gems).
**/
//--------------------------------------------------------------------------------------------------

inline	bool			AABBOverlapsSphere( const FpAABB& b, FpSphere_arg s )
{
	// work with sphere in box's local frame
	FwVector p = s.GetPosition() - b.GetCentre();

	FwVector v = b.GetExtents();
	float r = s.GetRadius();
	FwVector minSeparation = Min( p + v, FwMaths::kZero );
	FwVector maxSeparation = Max( p - v, FwMaths::kZero );
	return ( LengthSqr( minSeparation ) + LengthSqr( maxSeparation ) ) <= sqr( r );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			find the intersection of a plane and an (infinite) line
**/
//--------------------------------------------------------------------------------------------------

inline bool			PlaneIntersectLine( FpPlane_arg plane, const FpLine& line, FwPoint& intersection )
{
	return PlaneIntersectLine( plane, line.GetPoint(), line.GetDirection(), intersection );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			find the intersection of a plane and a (finite) line segment
**/
//--------------------------------------------------------------------------------------------------

inline bool			PlaneIntersectLineSegment( FpPlane_arg plane, const FpLineSegment& line, FwPoint& intersection, float* pT )
{
	return PlaneIntersectLineSegment( plane, line.GetPoint0(), line.GetPoint1(), intersection, pT );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			find the intersection of a plane and a (half-infinite) ray
**/
//--------------------------------------------------------------------------------------------------

inline bool			PlaneIntersectRay( FpPlane_arg plane, const FpRay& ray, FwPoint& intersection, float* pT )
{
	return PlaneIntersectRay( plane, ray.GetStart(), ray.GetDirection(), intersection, pT );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Return the closest point on the given line to the given point

	@param			point			-	the point
	@param			line			-	the line
**/
//--------------------------------------------------------------------------------------------------

inline FwPoint		ClosestPointOnLineToPoint( FwPoint_arg point, const FpLine& line )
{
	return ClosestPointOnLineToPoint( point, line.GetPoint(), line.GetDirection() );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			
**/
//--------------------------------------------------------------------------------------------------

inline bool			OBBIntersectLine( const FpOBB& b, const FpLine& line )
{
	return OBBIntersectLine( b, line.GetPoint(), line.GetDirection() );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			
**/
//--------------------------------------------------------------------------------------------------

inline bool			OBBIntersectLineSegment( const FpOBB& b, const FpLineSegment& line )
{
	return OBBIntersectLineSegment( b, line.GetPoint0(), line.GetPoint1() );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			
**/
//--------------------------------------------------------------------------------------------------

inline bool			OBBIntersectRay( const FpOBB& b, const FpRay& ray )
{
	return OBBIntersectRay( b, ray.GetStart(), ray.GetDirection() );
}

//--------------------------------------------------------------------------------------------------
/**
@brief			
**/
//--------------------------------------------------------------------------------------------------

inline bool			OBBIntersectRay( const FpOBB& b, const FpRay& ray, FwPoint& intersection, float* pT )
{
	return OBBIntersectRay( b, ray.GetStart(), ray.GetDirection(), intersection, pT );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			
**/
//--------------------------------------------------------------------------------------------------

inline bool			AABBIntersectLine( const FpAABB& b, const FpLine& line )
{
	return AABBIntersectLine( b, line.GetPoint(), line.GetDirection() );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			
**/
//--------------------------------------------------------------------------------------------------

inline bool			AABBIntersectLineSegment( const FpAABB& b, const FpLineSegment& line )
{
	return AABBIntersectLineSegment( b, line.GetPoint0(), line.GetPoint1() );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			
**/
//--------------------------------------------------------------------------------------------------

inline bool			AABBIntersectRay( const FpAABB& b, const FpRay& ray )
{
	return AABBIntersectRay( b, ray.GetStart(), ray.GetDirection() );
}

//--------------------------------------------------------------------------------------------------
/**
@brief			
**/
//--------------------------------------------------------------------------------------------------

inline bool			AABBIntersectRay( const FpAABB& b, const FpRay& ray, FwPoint& intersection, float* pT )
{
	return AABBIntersectRay( b, ray.GetStart(), ray.GetDirection(), intersection, pT );
}

#endif // FP_INTERSECT_H
