//--------------------------------------------------
//!
//!	\file spatialinterpolator.h
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#ifndef _SPATIAL_INTERP_H
#define _SPATIAL_INTERP_H

//--------------------------------------------------
//!
//!	PointIDandFraction
//!	intermediate class used as a result type
//!
//--------------------------------------------------
struct PointIDandFraction
{
	u_int	ID;			// id of point in list
	float	fraction;	// 0.0f - > 1.0f
};

typedef ntstd::List< PointIDandFraction*, Mem::MC_GFX >			SIresultList;
typedef ntstd::Vector<const PointIDandFraction*, Mem::MC_GFX >	SIsortVector;

//--------------------------------------------------
//!
//!	SpatialInterpolator
//!	Associate some function(s) with an arbitary list of points.
//! Evaluate that function using spatial interpolation.
//! Usage: you supply a point list which corresponds to
//! 'known' values of your function. Supply a point
//! at eval time, and get back a list of point indicies,
//! with their normalised contribution values.
//!
//--------------------------------------------------
class SpatialInterpolator
{
public:
	SpatialInterpolator( const CPoint* pInputPoints, u_int iNumPoints );
	
	virtual ~SpatialInterpolator();
	virtual void CalcNewResult( const CPoint& pos ) = 0;

	const SIresultList& GetResultList() const { return m_results; }
	const CPoint*		GetPointArray() const { return m_pPointArray; }
	u_int				GetNumPoints() const { return m_iNumPoints; }

protected:
	SIresultList		m_results;
	PointIDandFraction*	m_pResultArray;
	CPoint*				m_pPointArray;
	u_int				m_iNumPoints;
};

//--------------------------------------------------
//!
//!	SI_DistanceWeight
//!	Brute force spatial interpolator that categorises
//! all points based on distance. bad because:
//! 1. You have to evaluate all points always
//! 2. 'Far' contributions are never fully discounted
//! 3. Inverse square falloff my not be completly appropriate
//! 4. Sampling outside the bounds of the original point set
//!  effectivly averages all points.
//!
//--------------------------------------------------
class SI_DistanceWeight : public SpatialInterpolator
{
public:
	SI_DistanceWeight( const CPoint* pInputPoints, u_int iNumPoints );
	
	virtual ~SI_DistanceWeight();
	virtual void CalcNewResult( const CPoint& pos );

private:
	float*	m_pDistSqArray;
};

//--------------------------------------------------
//!
//!	SI_DelaunayTriangulation
//!	Much better than above because:
//! 1. Only ever need to evaluate a max of 4 points
//! 2. far points contribute nothing to local eval
//! 3. point lookup can be very quick with a tetraherdral partition tree
//! 4. is a 'better' spatial function approximation
//! 5. sampling outside of bounds provides 'correct' function
//!  boundaries
//!
//--------------------------------------------------
class SI_DelaunayTriangulation : public SpatialInterpolator
{
public:
	SI_DelaunayTriangulation( const CPoint* pInputPoints, u_int iNumPoints );
	
	virtual ~SI_DelaunayTriangulation();
	virtual void CalcNewResult( const CPoint& pos );
};

#endif // end _SPATIAL_INTERP_H
