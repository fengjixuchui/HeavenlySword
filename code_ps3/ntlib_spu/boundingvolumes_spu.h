//--------------------------------------------------
//!
//!	\file boundingvolumes_spu.h
//!	SPU realization of bounding volumes
//!
//--------------------------------------------------

#ifndef BOUNDINGVOLUMES_SPU_H
#define BOUNDINGVOLUMES_SPU_H

//#define FLT_MAX	((float)3.4e38)
#define FLT_MAX 3000000

//--------------------------------------------------
//!
//! SPU axes-aligned bounding box
//! Made to be binary compatible with PPU-side CAABB
//!
//--------------------------------------------------
class CAABB
{
public:
    CAABB() {}

	CPoint const&	Max() const
	{
		return m_obMax;
	}

	CPoint const& Min() const
	{
		return m_obMin;
	}

	CPoint GetCentre() const			{ return 0.5f*( m_obMax + m_obMin ); }

	//! union, union this volume to this AABB (expands it if nessecary)
	void Union( const CAABB& box )
	{
		m_obMin = m_obMin.Min( box.m_obMin );
		m_obMax = m_obMax.Max( box.m_obMax );
	}

	void Union( const CPoint& pnt )
	{
        m_obMin = m_obMin.Min( pnt );
		m_obMax = m_obMax.Max( pnt );
	}

	void Intersection( const CAABB& box )
	{
		m_obMin = m_obMin.Max( box.m_obMin );
		m_obMax = m_obMax.Min( box.m_obMax );
	}

    void Transform( const CMatrix& transform );

	bool HasPositiveVolume() const
	{
		//return( (m_obMin.X() < m_obMax.X()) &&
		//		(m_obMin.Y() < m_obMax.Y()) &&
		//		(m_obMin.Z() < m_obMax.Z()) );
        vector unsigned int cmpVec = spu_cmpgt( m_obMax.QuadwordValue(), m_obMin.QuadwordValue());
        vector unsigned int resultVec = spu_gather( cmpVec );

        return spu_extract(resultVec, 0);
	}

	//! currenty validaty is just whether is has positive volume
	bool IsValid() const
	{
		return HasPositiveVolume();
	}

	//! create the AABB from a bunch of points
	CAABB( CPoint* points, unsigned int numPoints );

     

	CPoint m_obMin, m_obMax;

};

class CPlane
{
public:
	//! Creates a ground plane.
	CPlane() : m_obNormal(0.0f, 1.0f, 0.0f), m_fDistance(0.0f) {}

	//! Creates a plane from 4 float poked directly into the .
	CPlane(float a, float b, float c, float d) : m_obNormal(a, b, c), m_fDistance(d)
	{
	}

	CPlane(CVector const& in) : m_obNormal(in[0], in[1], in[2]), m_fDistance(in[3])
	{
	}

	CDirection const&	GetNormal() const
	{
		return m_obNormal;
	}

	float GetDistance() const
	{
		return m_fDistance;
	}

//private:
public:
	CDirection m_obNormal;	//!< The normal to the plane (always kept normalised).
	float m_fDistance;		//!< The distance along the normal to the plane's surface.
};

// Not binary compatible with the PPU CullingFrustum!
class CullingFrustum
{
public:
	CullingFrustum(){}

	//! takes a matrix to build a frustum around (usually a view projection matrix)
	CullingFrustum( const CMatrix& matrix );

    void Init( const CMatrix& matrix );

	enum TEST_STATUS
	{
		COMPLETELY_OUTSIDE = 0,		//!< wholly outside the frustum
		PARTIALLY_INSIDE = 1,		//!< partially in and out (also used when the function is unable to tell the inside status)
		COMPLETELY_INSIDE = 2,		//!< wholly inside the frustum
	};

	CPlane* GetPlane( unsigned short i )
	{
		return m_CamPlanes+i;
	}

	CPlane const* GetPlane( unsigned short i ) const
	{
		return m_CamPlanes+i;
	}


//private:

	void RecursiveIntersect( const CAABB& testBox, const float minSplitDistance, CAABB& resultBox, int recursionLevel ) const;
    CAABB* CullingFrustum::Intersect( const CAABB& inBox, CAABB* resultAABB) const;

	CPlane				m_CamPlanes[6];		//!< actual planes

    CPoint			    m_pntList[8];		//!< points making up the extremes

    CAABB               m_frustumBox;

	vector unsigned int	m_nVertexLUT[6];

   
	//unsigned int    m_nVertexLUT[6];	//!< a fast AABB lookup table

	CMatrix			m_frustumMatrix;	//!< the frustum matrix (whats passed in) takes points to frustum space from world
};

class CSphereBound
{
public:
	CSphereBound() : m_obPosition(CONSTRUCT_CLEAR), m_fRadius(0.0f) {}

	CSphereBound(CPoint const& obPosition, float fRadius) : m_obPosition(obPosition), m_fRadius(fRadius) {}

	explicit CSphereBound(const CAABB& box);

	CPoint const& GetPosition() const { return m_obPosition; }
	float GetRadius() const { return m_fRadius; }

	void SetPosition(CPoint const& obPosition) { m_obPosition = obPosition; }
	void SetRadius(float fRadius) { m_fRadius = fRadius; }

private:
	CPoint m_obPosition;
	float m_fRadius;		//!< Might evilly move this to pos.W() at some point.
};


inline void AABB_ConstructInfiniteNegative(CAABB* aabb)
{
    const float max = 5000000; const float min = -5000000;
	aabb -> m_obMin.Quadword() = (v128){max, max, max, max};
	aabb -> m_obMax.Quadword() = (v128){min, min, min, min};
}


#endif
