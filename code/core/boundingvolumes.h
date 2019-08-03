/***************************************************************************************************
*
*	DESCRIPTION
*
*	NOTES
*
***************************************************************************************************/

#ifndef CORE_BOUNDINGVOLUMES_H
#define CORE_BOUNDINGVOLUMES_H

/***************************************************************************************************
*
*	CLASS			CAABB
*
*	DESCRIPTION		Simple axis aligned bounding box class.
*
***************************************************************************************************/

class CAABB
{
public:
	//! default ctor, makes an empty AABB
	CAABB( CLEAR_CONSTRUCT_MODE dum = CONSTRUCT_CLEAR) : m_obMin(CONSTRUCT_CLEAR), m_obMax(CONSTRUCT_CLEAR) { UNUSED(dum); }

	explicit CAABB( DONT_CLEAR_CONSTRUCT_MODE ){}

	//! create an infinite size volume (negative or positive)
	explicit CAABB( INFINITE_CONSTRUCT_MODE mode ) { InfiniteConstruct( mode ); }

	//! create the AABB from a bunch of points
	explicit CAABB( CPoint* points, unsigned int numPoints ) 
	{ 
		InfiniteConstruct( CONSTRUCT_INFINITE_NEGATIVE ); 
		for(unsigned int i=0;i < numPoints;i++)
		{
			Union( points[i] );
		}
	}

	//! create this AABB from a bunch of AABB
	explicit CAABB( CAABB* points, unsigned int numBoxes ) 
	{ 
		InfiniteConstruct( CONSTRUCT_INFINITE_NEGATIVE ); 
		for(unsigned int i=0;i < numBoxes;i++)
		{
			Union( points[i] );
		}
	}

	explicit CAABB	( const CPoint &min_point, const CPoint &max_point )
	:	m_obMin		( min_point )
	,	m_obMax		( max_point )
	{
		ntAssert( min_point.X() <= max_point.X() );
		ntAssert( min_point.Y() <= max_point.Y() );
		ntAssert( min_point.Z() <= max_point.Z() );
	}

	CPoint const& Min() const { return m_obMin; }
	CPoint const& Max() const { return m_obMax; }

	CPoint& Min() { return m_obMin; }
	CPoint& Max() { return m_obMax; }

	CPoint GetCentre() const			{ return 0.5f*( m_obMax + m_obMin ); }
	CDirection GetHalfLengths() const	{ return CDirection( 0.5f*( m_obMax - m_obMin ) ); }
	
	void DebugRender(CMatrix const& obTransform, uint32_t dwColour, int iFlags = 0) const;

	bool IntersectRayInverted(CPoint const& obStart, CPoint const& obEnd, CPoint& obResult);
	bool IntersectRay(CPoint const& obStart, CPoint const& obEnd, CPoint& obResult); 

	// slightly different version for the shadows
	bool IntersectRay(const CPoint& origPt, const CDirection& dir, float& outT, CPoint& outPoint ) const;


	//! i is 3 bits to describe which of the eight points making up the AABB you would like (i = ZYX)
	CPoint Point( int i ) const
	{
		return CPoint(	(i&0x1)? m_obMin.X() : m_obMax.X(), 
						(i&0x2)? m_obMin.Y() : m_obMax.Y(),
						(i&0x4)? m_obMin.Z() : m_obMax.Z() );
	}

	//! union, union this point to this AABB (expands it if nessecary)
	void Union( const CPoint& pnt )
	{
		m_obMin = m_obMin.Min( pnt );
		m_obMax = m_obMax.Max( pnt );
	}
	//! union, union this volume to this AABB (expands it if nessecary)
	void Union( const CAABB& box )
	{
		m_obMin = m_obMin.Min( box.m_obMin );
		m_obMax = m_obMax.Max( box.m_obMax );
	}
	//! intersection of 2 AABB's (this box becomes the part contained by both boxes)
	//! can return a negative volume box if the don't intersect
	void Intersection( const CAABB& box )
	{
		m_obMin = m_obMin.Max( box.m_obMin );
		m_obMax = m_obMax.Min( box.m_obMax );
	}

	//! transforms this AABB by the transform matrix (obviously AABB dimension may change)
	void Transform( const CMatrix& transform );


	//! Has positive volume
	bool HasPositiveVolume() const
	{
		return( (m_obMin.X() < m_obMax.X()) &&
				(m_obMin.Y() < m_obMax.Y()) &&
				(m_obMin.Z() < m_obMax.Z()) );
	}

	//! currenty validaty is just whether is has positive volume
	bool IsValid() const
	{
		return HasPositiveVolume();
	}

	//! Test if a point is inside the aabb.
	bool IsPointInside( const CPoint &p ) const
	{
		return	( p.X() >= m_obMin.X() && p.X() <= m_obMax.X() ) &&
				( p.Y() >= m_obMin.Y() && p.Y() <= m_obMax.Y() ) &&
				( p.Z() >= m_obMin.Z() && p.Z() <= m_obMax.Z() );
	}

	void InfiniteConstruct(INFINITE_CONSTRUCT_MODE mode)
	{
        // A hack around FLT_MAX problem
        const float max = 5000000; const float min = -5000000;
		if( mode == CONSTRUCT_INFINITE_NEGATIVE )
		{
			m_obMin = CPoint(max, max, max);
			m_obMax = CPoint(min, min, min);
		} else
		{
			m_obMin = CPoint(min, min, min);
			m_obMax = CPoint(max, max, max);
		}
	}

private:

	CPoint m_obMin, m_obMax;
};

/***************************************************************************************************
*
*	CLASS			CSphereBound
*
*	DESCRIPTION		Simple spherical bounding volume class.
*
***************************************************************************************************/

class CSphereBound
{
public:
	CSphereBound() : m_obPosition(CONSTRUCT_CLEAR), m_fRadius(0.0f) {}

	CSphereBound(CPoint const& obPosition, float fRadius) : m_obPosition(obPosition), m_fRadius(fRadius) {}

	explicit CSphereBound(CPoint* points, unsigned int numPoints);
	
	explicit CSphereBound(const CAABB& box);

	CPoint const& GetPosition() const { return m_obPosition; }
	float GetRadius() const { return m_fRadius; }

	void SetPosition(CPoint const& obPosition) { m_obPosition = obPosition; }
	void SetRadius(float fRadius) { m_fRadius = fRadius; }

	void DebugRender( CMatrix const& obTransform, uint32_t dwColour, int iFlags = 0 ) const;

	// Does the bounding sphere contain the point?
	bool Contains( const CPoint& rPoint ) const { return (rPoint - m_obPosition).LengthSquared() <= (m_fRadius*m_fRadius); }

	// Does the bounding sphere intersect another?
	bool Intersects( const CSphereBound& rOther ) const { return (rOther.m_obPosition - m_obPosition).LengthSquared() <= ((m_fRadius*m_fRadius)+(rOther.m_fRadius+rOther.m_fRadius)); }

private:
	CPoint m_obPosition;
	float m_fRadius;		//!< Might evilly move this to pos.W() at some point.
};

//-----------------------------------------------------------------------
//!
//!	COBB.
//!	Quick obb implementation (Quite poor implementation I admit!)
//! Author: Mustapha Bismi (mustapha.bismi@ninjatheory.com)
//!
//-----------------------------------------------------------------------
class COBB
{
public:
	COBB() : m_obTransform(CONSTRUCT_CLEAR) {}

	COBB(const CPoint& obMin, const CPoint& obMax, const CMatrix& obTransform) : m_obTransform(obTransform) { m_obAABB.Min() = obMin; m_obAABB.Max() = obMax;}

	CAABB&		GetAABB() { return m_obAABB; }
	CMatrix&	GetTransform() { return m_obTransform; }

	CAABB const& GetAABB() const { return m_obAABB; }
	CMatrix const& GetTransform() const { return m_obTransform; }
    
private:
	CAABB	m_obAABB;
	CMatrix m_obTransform;
};


#if 0 // Not currently used and has issues with alignement
//-----------------------------------------------------------------------
//!
//!	CBoundingCone.
//! used by the shadow stuff, represents a view cone bounding some
//! AABB. Very specialised originally from NVIDIA's PSM demo
//!
//-----------------------------------------------------------------------
class CBoundingCone
{
public:
	CBoundingCone();
	CBoundingCone( const ntstd::List<CAABB>& boxList, const CMatrix& projection, const CPoint& apex );
	CBoundingCone( const ntstd::List<CAABB>& boxList, const CMatrix& projection, const CPoint& apex, const CDirection& direction );

	// leave these public simple because the entire class is about calculating these having a bunch of accessors is just a 
	// waste of time in the case IMO

	CDirection	m_Direction;
	CPoint		m_Apex;
	float		m_fFovY;
	float		m_fFovX;
	float		m_fNear;
	float		m_fFar;
	CMatrix		m_LookAt;
};

#endif // end CBoundingCone removal

#endif // ndef CORE_BOUNDINGVOLUMES_H
