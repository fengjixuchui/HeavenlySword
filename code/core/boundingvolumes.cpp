/***************************************************************************************************
*
*	DESCRIPTION	Its fine
*
*	NOTES
*
***************************************************************************************************/

#include "core/boundingvolumes.h"
#include "core/visualdebugger.h"

void CAABB::DebugRender(CMatrix const& obTransform, uint32_t dwColour, int iFlags) const
{
#ifndef _GOLD_MASTER
	CMatrix obLocalTransform = obTransform;
	obLocalTransform.SetTranslation( GetCentre()*obTransform );

	g_VisualDebug->RenderOBB( obLocalTransform, GetHalfLengths(), dwColour, iFlags );
#endif
}

bool CAABB::IntersectRayInverted(CPoint const& obStart, CPoint const& obEnd, CPoint& obResult)
{
	// Let's the ray be defined by PStart and PEnd --> PRay(t) = PStart + tPEnd
	// Find the 3 planes potentially intersecting the ray on the right side

	float fT = 0.f;

	CDirection lineVector(obEnd - obStart);
	/*bool isInside = (	( obStart.X() >= m_obMin.X() ) && 
						( obStart.X() <= m_obMax.X() ) &&
						( obStart.Y() >= m_obMin.Y() ) && 
						( obStart.Y() <= m_obMax.Y() ) &&
						( obStart.Z() >= m_obMin.Z() ) && 
						( obStart.Z() <= m_obMax.Z() ) );*/
	
	//if(isInside) 
		fT = 1.f;

	// For each axis X Y and Z

	// - X - 

	float sortedStart = obStart.X();
	float sortedEnd = obEnd.X();
	bool inverted = false;
	if(sortedEnd < sortedStart)
	{
		sortedEnd = sortedStart;
		sortedStart = obEnd.X();
		inverted = true;
	};
	
	// Get PBoxMinX and PBoxMaxX
	float fMin = m_obMin.X(); float fMax = m_obMax.X();

	// IfPBoxMinX > PEnd.X --> EARLY EXIT
	if(( fMin > sortedEnd ) && ( fMin > sortedStart ))
		return false;

	// If PBoxMaxX < PStart.X --> EARLY EXIT
	if(( fMax < sortedEnd ) && ( fMax < sortedStart ))
		return false;

	/*float planeAxis;
	
	if(( fMax >= sortedStart ) && ( fMax <= sortedEnd ))
	{
		planeAxis = fMax;
		if(( fMin >= sortedStart ) && ( fMin <= sortedEnd ) && (inverted))
		{
			planeAxis = fMin;
		};
	} else 
	{
		planeAxis = fMin;
	}*/

	float planeAxis;
	inverted ? planeAxis = fMin : planeAxis = fMax;

				
	// compute the t value for the interesection with the ray
	// A point T if in P if T.x = plane.x
	// T.x = PStart.x + t lineVector.x
	// t = (plane.x - PStart.x) / lineVector.x
	float fCurrentT = planeAxis - obStart.X();

	// If t < 0.f or t > 1.f --> EARLY EXIT
	fCurrentT = fCurrentT / lineVector.X();

	// Keep the biggest t value for the tree plane
	if((fCurrentT > 0.f)&&(fCurrentT < 1.f))
	{
		//if(isInside)
		//{
			if( fCurrentT < fT) fT = fCurrentT;
		//} else {
		//	if( fCurrentT > fT) fT = fCurrentT;
		//};
	};

	// - Y - 

	sortedStart = obStart.Y();
	sortedEnd = obEnd.Y();
	inverted = false;
	
	if(sortedEnd < sortedStart)
	{
		sortedEnd = sortedStart;
		sortedStart = obEnd.Y();
		inverted = true;
	};
	
	fMin = m_obMin.Y(); fMax = m_obMax.Y();

	if(( fMin > sortedEnd ) && ( fMin > sortedStart ))
		return false;

	if(( fMax < sortedEnd ) && ( fMax < sortedStart ))
		return false;
	
	/*if(( fMax >= sortedStart ) && ( fMax <= sortedEnd ))
	{
		planeAxis = fMax;
		if(( fMin >= sortedStart ) && ( fMin <= sortedEnd ) && (inverted))
		{
			planeAxis = fMin;
		};
	} else 
	{
		planeAxis = fMin;
	}*/
	inverted ? planeAxis = fMin : planeAxis = fMax;
				
	fCurrentT = planeAxis - obStart.Y();
	fCurrentT = fCurrentT / lineVector.Y();

	if((fCurrentT > 0.f)&&(fCurrentT < 1.f))
	{
		//if(isInside)
		//{
			if( fCurrentT < fT) fT = fCurrentT;
		//} else {
		//	if( fCurrentT > fT) fT = fCurrentT;
		//};
	};

	// - Z - 

	sortedStart = obStart.Z();
	sortedEnd = obEnd.Z();
	inverted = false;
	
	if(sortedEnd < sortedStart)
	{
		sortedEnd = sortedStart;
		sortedStart = obEnd.Z();
		inverted = true;
	};
	
	fMin = m_obMin.Z(); fMax = m_obMax.Z();

	if(( fMin > sortedEnd ) && ( fMin > sortedStart ))
		return false;

	if(( fMax < sortedEnd ) && ( fMax < sortedStart ))
		return false;

	
	/*if(( fMax >= sortedStart ) && ( fMax <= sortedEnd ))
	{
		planeAxis = fMax;
		if(( fMin >= sortedStart ) && ( fMin <= sortedEnd ) && (inverted))
		{
			planeAxis = fMin;
		};
	} else 
	{
		planeAxis = fMin;
	}*/
	inverted ? planeAxis = fMin : planeAxis = fMax;

	fCurrentT = planeAxis - obStart.Z();
	fCurrentT = fCurrentT / lineVector.Z();

	if((fCurrentT > 0.f)&&(fCurrentT < 1.f))
	{
		//if(isInside)
		//{
			if( fCurrentT < fT) fT = fCurrentT;
		//} else {
		//	if( fCurrentT > fT) fT = fCurrentT;
		//};
	};
	
	// Compute the result

	if((fT < 0.f) || (fT > 1.f))
		return false;

	CPoint obTempResult = obStart + fT * lineVector;

	bool doIntersect = 		( obTempResult.X() >= (m_obMin.X() - 0.005f ) ) && 
							( obTempResult.X() <= (m_obMax.X() + 0.005f ) ) &&
							( obTempResult.Y() >= (m_obMin.Y() - 0.005f ) ) && 
							( obTempResult.Y() <= (m_obMax.Y() + 0.005f ) ) &&
							( obTempResult.Z() >= (m_obMin.Z() - 0.005f ) ) && 
							( obTempResult.Z() <= (m_obMax.Z() + 0.005f ) ) ;
	
	if(doIntersect)
	{
		obResult = obTempResult;
	};

	return doIntersect;
};

bool CAABB::IntersectRay(CPoint const& obStart, CPoint const& obEnd, CPoint& obResult)
{
	// Let's the ray be defined by PStart and PEnd --> PRay(t) = PStart + tPEnd
	// Find the 3 planes potentially intersecting the ray on the right side

	float fT = 0.f;

	CDirection lineVector(obEnd - obStart);
	bool isInside = (	( obStart.X() >= m_obMin.X() ) && 
						( obStart.X() <= m_obMax.X() ) &&
						( obStart.Y() >= m_obMin.Y() ) && 
						( obStart.Y() <= m_obMax.Y() ) &&
						( obStart.Z() >= m_obMin.Z() ) && 
						( obStart.Z() <= m_obMax.Z() ) );
	
	if(isInside) fT = 1.f;

	// For each axis X Y and Z

	// - X - 

	float sortedStart = obStart.X();
	float sortedEnd = obEnd.X();
	bool inverted = false;
	if(sortedEnd < sortedStart)
	{
		sortedEnd = sortedStart;
		sortedStart = obEnd.X();
		inverted = true;
	};
	
	// Get PBoxMinX and PBoxMaxX
	float fMin = m_obMin.X(); float fMax = m_obMax.X();

	// IfPBoxMinX > PEnd.X --> EARLY EXIT
	if(( fMin > sortedEnd ) && ( fMin > sortedStart ))
		return false;

	// If PBoxMaxX < PStart.X --> EARLY EXIT
	if(( fMax < sortedEnd ) && ( fMax < sortedStart ))
		return false;

	float planeAxis;
	
	if(( fMin >= sortedStart ) && ( fMin <= sortedEnd ))
	{
		planeAxis = fMin;
		if(( fMax >= sortedStart ) && ( fMax <= sortedEnd ) && (inverted))
		{
			planeAxis = fMax;
		};
	} else 
	{
		planeAxis = fMax;
	}
				
	// compute the t value for the interesection with the ray
	// A point T if in P if T.x = plane.x
	// T.x = PStart.x + t lineVector.x
	// t = (plane.x - PStart.x) / lineVector.x
	float fCurrentT = planeAxis - obStart.X();

	// If t < 0.f or t > 1.f --> EARLY EXIT
	fCurrentT = fCurrentT / lineVector.X();

	// Keep the biggest t value for the tree plane
	if((fCurrentT > 0.f)&&(fCurrentT < 1.f))
	{
		if(isInside)
		{
			if( fCurrentT < fT) fT = fCurrentT;
		} else {
			if( fCurrentT > fT) fT = fCurrentT;
		};
	};

	// - Y - 

	sortedStart = obStart.Y();
	sortedEnd = obEnd.Y();
	inverted = false;
	
	if(sortedEnd < sortedStart)
	{
		sortedEnd = sortedStart;
		sortedStart = obEnd.Y();
		inverted = true;
	};
	
	fMin = m_obMin.Y(); fMax = m_obMax.Y();

	if(( fMin > sortedEnd ) && ( fMin > sortedStart ))
		return false;

	if(( fMax < sortedEnd ) && ( fMax < sortedStart ))
		return false;
	
	if(( fMin >= sortedStart ) && ( fMin <= sortedEnd ))
	{
		planeAxis = fMin;
		if(( fMax >= sortedStart ) && ( fMax <= sortedEnd ) && (inverted))
		{
			planeAxis = fMax;
		};
	} else 
	{
		planeAxis = fMax;
	}
				
	fCurrentT = planeAxis - obStart.Y();
	fCurrentT = fCurrentT / lineVector.Y();

	if((fCurrentT > 0.f)&&(fCurrentT < 1.f))
	{
		if(isInside)
		{
			if( fCurrentT < fT) fT = fCurrentT;
		} else {
			if( fCurrentT > fT) fT = fCurrentT;
		};
	};

	// - Z - 

	sortedStart = obStart.Z();
	sortedEnd = obEnd.Z();
	inverted = false;
	
	if(sortedEnd < sortedStart)
	{
		sortedEnd = sortedStart;
		sortedStart = obEnd.Z();
		inverted = true;
	};
	
	fMin = m_obMin.Z(); fMax = m_obMax.Z();

	if(( fMin > sortedEnd ) && ( fMin > sortedStart ))
		return false;

	if(( fMax < sortedEnd ) && ( fMax < sortedStart ))
		return false;

	
	if(( fMin >= sortedStart ) && ( fMin <= sortedEnd ))
	{
		planeAxis = fMin;
		if(( fMax >= sortedStart ) && ( fMax <= sortedEnd ) && (inverted))
		{
			planeAxis = fMax;
		};
	} else 
	{
		planeAxis = fMax;
	}
				
	fCurrentT = planeAxis - obStart.Z();
	fCurrentT = fCurrentT / lineVector.Z();

	if((fCurrentT > 0.f)&&(fCurrentT < 1.f))
	{
		if(isInside)
		{
			if( fCurrentT < fT) fT = fCurrentT;
		} else {
			if( fCurrentT > fT) fT = fCurrentT;
		};
	};
	
	// Compute the result

	if((fT < 0.f) || (fT > 1.f))
		return false;

	obResult = obStart + fT * lineVector;

	return(	( obResult.X() >= m_obMin.X() ) && 
			( obResult.X() <= m_obMax.X() ) &&
			( obResult.Y() >= m_obMin.Y() ) && 
			( obResult.Y() <= m_obMax.Y() ) &&
			( obResult.Z() >= m_obMin.Z() ) && 
			( obResult.Z() <= m_obMax.Z() ) );
}

bool CAABB::IntersectRay(const CPoint& origPt, const CDirection& dir, float& outT, CPoint& outPoint ) const
{
	CDirection normals[ 6 ] =
	{
		CDirection( 1.0f, 0.0f, 0.0f ),
		CDirection( 0.0f, 1.0f, 0.0f ),
		CDirection( 0.0f, 0.0f, 1.0f ),
		CDirection( -1.0f, 0.0f, 0.0f ),
		CDirection( 0.0f, -1.0f, 0.0f ),
		CDirection( 0.0f, 0.0f, -1.0f )
	};

	float push_out_direction = 1.0f;
	if ( IsPointInside( origPt ) )
	{
		for ( int i=0;i<6;i++ )
		{
			normals[ i ] = -normals[ i ];
		}

		push_out_direction = -1.0f;
	}

	float distance_to_origin[ 6 ] =
	{
		Max().Dot( normals[ 0 ] ),
		Max().Dot( normals[ 1 ] ),
		Max().Dot( normals[ 2 ] ),
		Min().Dot( normals[ 3 ] ),
		Min().Dot( normals[ 4 ] ),
		Min().Dot( normals[ 5 ] )
	};

	float NdotD[ 6 ];

	for ( int i=0;i<6;i++ )
	{
		NdotD[ i ] = normals[ i ].Dot( dir );
	}

	for ( int i=0;i<6;i++ )
	{
		if ( NdotD[ i ] > 0.0f )
		{
			// Plane is backfaced to us.
			continue;
		}

		// Is the ray parallel to this plane?
		if ( fabsf( NdotD[ i ] ) < EPSILON )
		{
			// Yes, ignore it.
			continue;
		}

		// Find the ray parameter for the intersection.
		float t = distance_to_origin[ i ] - origPt.Dot( normals[ i ] );
		t /= NdotD[ i ];

		CPoint I = origPt + t * dir;

		// This is only a candidate intersection if when I push it slightly inside
		// the aabb, it registers as actually being inside the aabb.
		if ( IsPointInside( I - normals[ i ] * 0.001f * push_out_direction ) )
		{
			outT = t;
			outPoint = I;

			return true;
		}
	}

	return false;
}


void CAABB::Transform( const CMatrix& transform )
{
#if 1 // Optimised version

	// compute AABB extents
	CDirection obDelta = m_obMax^m_obMin;
	obDelta *= 0.5f;
	
	// compute AABB old centre
	CPoint obCentre = m_obMin + obDelta;
	
	// compute AABB new centre
	CPoint obNewCentre = obCentre * transform;

	// get new AABB extents in the new transform space
	CDirection obr0 = transform.GetXAxis() * obDelta.X();
	CDirection obr1 = transform.GetYAxis() * obDelta.Y();
	CDirection obr2 = transform.GetZAxis() * obDelta.Z();

	// move the new AABB extents back to world space
	// we don't simply rotate since we still need an AABB! so we compute a worst case (non linear) transform
	CDirection obNewDelta;
static CDirection obMax(1.0f, 1.0f, 1.0f);

	// worst case..
	obr0 = obr0.Abs();
	obr1 = obr1.Abs();
	obr2 = obr2.Abs();

	// transpose (we have to go back to world space) and apply
	obNewDelta.X() = obr0.X() + obr1.X() + obr2.X();
	obNewDelta.Y() = obr0.Y() + obr1.Y() + obr2.Y();
	obNewDelta.Z() = obr0.Z() + obr1.Z() + obr2.Z();

	// compute new AABB
	m_obMax = obNewCentre+ obNewDelta;
	m_obMin = obNewCentre- obNewDelta;
	// -------------------
#else 
	CPoint pnts[8];
	pnts[0] = CPoint( Min().X(), Min().Y(), Min().Z() );
	pnts[1] = CPoint( Max().X(), Min().Y(), Min().Z() );
	pnts[2] = CPoint( Min().X(), Max().Y(), Min().Z() );
	pnts[3] = CPoint( Max().X(), Max().Y(), Min().Z() );
	pnts[4] = CPoint( Min().X(), Min().Y(), Max().Z() );
	pnts[5] = CPoint( Max().X(), Min().Y(), Max().Z() );
	pnts[6] = CPoint( Min().X(), Max().Y(), Max().Z() );
	pnts[7] = CPoint( Max().X(), Max().Y(), Max().Z() );

	// construct a AABB from our transformed points 
	m_obMin = CPoint(FLT_MAX, FLT_MAX, FLT_MAX);
	m_obMax = CPoint(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	Union( pnts[0] * transform );
	Union( pnts[1] * transform );
	Union( pnts[2] * transform );
	Union( pnts[3] * transform );
	Union( pnts[4] * transform );
	Union( pnts[5] * transform );
	Union( pnts[6] * transform );
	Union( pnts[7] * transform );
#endif
#if 0
	if ((!obMax.Compare(m_obMax, 0.0001f)) || (!obMin.Compare(m_obMin, 0.0001f)))
	{
		printf("1 %f %f %f - %f %f %f\n", obMax.X(), obMax.Y(), obMax.Z(), obMin.X(), obMin.Y(), obMin.Z());
		printf("2 %f %f %f - %f %f %f\n", m_obMax.X(), m_obMax.Y(), m_obMax.Z(), m_obMin.X(), m_obMin.Y(), m_obMin.Z());
	}
#endif
}

void CSphereBound::DebugRender( CMatrix const& obTransform, uint32_t dwColour, int iFlags ) const
{
#ifndef _GOLD_MASTER
	g_VisualDebug->RenderSphere( CQuat( obTransform ), m_obPosition*obTransform, m_fRadius, dwColour, iFlags );
#endif
}

CSphereBound::CSphereBound(CPoint* points, unsigned int numPoints) :
	m_fRadius(0.f)
{
	if( numPoints == 0 )
	{
		m_obPosition = CPoint(CONSTRUCT_CLEAR);
	} else
	{
		m_obPosition = points[0];
		for(unsigned int i=1;i < numPoints;i++)
		{
			const CPoint& pnt = points[i];
			CDirection dir = CDirection(pnt - m_obPosition);
			float d = dir.Dot( dir );
			if( d > (m_fRadius * m_fRadius) )
			{
				d = sqrtf(d);
				float r = 0.5f * (d+m_fRadius);
				float scale = (r - m_fRadius) / d;
				m_obPosition = m_obPosition + scale * dir;
				m_fRadius = r;
			}
		}
	}

}

CSphereBound::CSphereBound( const CAABB& box )
{
    m_obPosition = box.GetCentre();
    CDirection radiusVec = CDirection(box.Max() - m_obPosition);
    m_fRadius = radiusVec.Length();;
}


#if 0 // Not currently used and has issues with alignement
CBoundingCone::CBoundingCone() : 
	m_Direction(0, 0, -1 ),
	m_Apex( CONSTRUCT_CLEAR ),
	m_fFovY( 0.f ), 
	m_fFovX( 0.f ), 
	m_fNear( 0.f ),
	m_fFar( 0.f ),
	m_LookAt( CONSTRUCT_IDENTITY )
{
}
static void	MatrixLookAt( CMatrix& obMat, const CPoint& obEye, const CPoint& obAt, const CDirection& obUp )
{	
	CDirection	obZAxis( obAt ^ obEye );
	obZAxis.Normalise();
	CDirection	obXAxis = obUp.Cross( obZAxis );
	obXAxis.Normalise();
	CDirection	obYAxis = obZAxis.Cross( obXAxis );

	CPoint obTrans(	-obXAxis.Dot( CDirection(obEye) ),
					-obYAxis.Dot( CDirection(obEye) ),
					-obZAxis.Dot( CDirection(obEye) ) );

	obMat.Row(0) = CVector( obXAxis.X(), obYAxis.X(), obZAxis.X(), 0.f );
	obMat.Row(1) = CVector( obXAxis.Y(), obYAxis.Y(), obZAxis.Y(), 0.f );
	obMat.Row(2) = CVector( obXAxis.Z(), obYAxis.Z(), obZAxis.Z(), 0.f );
	obMat.Row(3) = CVector( obTrans.X(), obTrans.Y(), obTrans.Z(), 1.f );
}

CBoundingCone::CBoundingCone( const ntstd::List<CAABB>& boxList, const CMatrix& projection, const CPoint& apex, const CDirection& direction )	:	
	m_Direction( direction ),
	m_Apex( apex )
{
	const CDirection yAxis( 0, 1, 0 );
	const CDirection zAxis( 0, 0, 1 );
	m_Direction.Normalise();

	// find a reasonable up vector
	CDirection axis = yAxis;
	if( fabsf( yAxis.Dot( m_Direction ) > 0.99f) )
	{
		axis = zAxis;
	}

	MatrixLookAt( m_LookAt, m_Apex, (m_Apex+m_Direction), axis );
//	m_LookAt = m_LookAt.GetAffineInverse();

	float maxx = 0.f, maxy = 0.f;
	m_fNear = FLT_MAX;
	m_fFar = 0.f;

	CMatrix viewProjMat = m_LookAt * projection;

	ntstd::List<CAABB>::const_iterator boxIt = boxList.begin();
	while( boxIt != boxList.end() )
	{
		const CAABB& box = *boxIt;
		for( int j=0;j < 8;j++)
		{
			CPoint pnt = box.Point(j);
			CVector pntv(pnt);
			pntv.W() = 1.f;
			pntv = pntv * viewProjMat;
			pntv *= 1.f / pntv.W();
			pnt = CPoint(pntv);
			maxx = max( maxx, fabsf( pnt.X() / pnt.Z() ) );
			maxy = max( maxy, fabsf( pnt.Y() / pnt.Z() ) );
			m_fNear = min( m_fNear, pnt.Z() );
			m_fFar = max( m_fFar, pnt.Z() );
		}
		++boxIt;
	}

	m_fFovX = atanf( maxx );
	m_fFovY = atanf( maxy );
}

CBoundingCone::CBoundingCone( const ntstd::List<CAABB>& boxList, const CMatrix& projection, const CPoint& apex )	:	
	m_Apex( apex )
{

	unsigned int numBoxes = boxList.size();
	const CDirection yAxis( 0, 1, 0 );
	const CDirection zAxis( 0, 0, 1 );
	const CDirection negZAxis( 0, 0, -1 );
	if( numBoxes == 0 )
	{
		m_Direction = negZAxis;
		m_fFovX = 0.f;
		m_fFovY = 0.f;
		m_LookAt = CMatrix( CONSTRUCT_IDENTITY );
	} else
	{
		// get all the box's points in post perspective space
		CPoint* ppPnts = NT_NEW CPoint[ numBoxes * 8 ];
		ntstd::List<CAABB>::const_iterator boxIt = boxList.begin();
		int count = 0;
		while( boxIt != boxList.end() )
		{
			const CAABB& box = *boxIt;
			for( int j=0;j < 8;j++)
			{
				CVector tmp( box.Point(j) );
				tmp.W() = 1.0f;
				tmp = tmp * projection;
				tmp *= 1.f / tmp.W();
				ppPnts[(count*8)+j] = CPoint(tmp);
			}

			++boxIt;
			++count;
		}

		// make the direction, the vector to the center of the bounding sphere of the post perspective box
		CSphereBound sphere( ppPnts, numBoxes*8 );
		m_Direction = CDirection(sphere.GetPosition() - m_Apex);
		m_Direction.Normalise();

		// find a reasonable up vector
		CDirection axis = yAxis;
		if( fabsf( yAxis.Dot( m_Direction ) > 0.99f) )
		{
			axis = zAxis;
		}

		MatrixLookAt( m_LookAt, m_Apex, (m_Apex+m_Direction), axis );
//		m_LookAt = m_LookAt.GetAffineInverse();

		float maxx = 0.f, maxy = 0.f;
		m_fNear = FLT_MAX;
		m_fFar = 0.f;

		for( unsigned int i=0; i < numBoxes*8;i++)
		{
			CVector pntv = CVector(ppPnts[i]);
			pntv.W() = 1.f;
			pntv = pntv * m_LookAt;
			CPoint pnt( pntv.X() / pntv.W(), pntv.Y() / pntv.W(), pntv.Z() / pntv.W() );
			maxx = max( maxx, fabsf( pnt.X() / pnt.Z() ) );
			maxy = max( maxy, fabsf( pnt.Y() / pnt.Z() ) );
			m_fNear = min( m_fNear, pnt.Z() );
			m_fFar = max( m_fFar, pnt.Z() );
		}

		m_fFovX = atanf( maxx );
		m_fFovY = atanf( maxy );
		NT_DELETE_ARRAY( ppPnts );
	}
}

#endif // end CBoundingCone removal
