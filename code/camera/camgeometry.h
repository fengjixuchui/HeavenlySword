//--------------------------------------------------
//!
//!	\file camgeometry.h
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#ifndef _CAM_GEOM_H
#define _CAM_GEOM_H

/***************************************************************************************************
*
*	CLASS			CSimpleLine
*
*	DESCRIPTION		Utility class for AI testing
*
***************************************************************************************************/
class CSimpleLine
{
public:
	CSimpleLine( float fX1, float fY1, float fX2, float fY2 ) :
		m_fX1(fX1), m_fY1(fY1),
		m_fX2(fX2), m_fY2(fY2),
		m_fDX(fX2-fX1), m_fDY(fY2-fY1),
		m_fRCPDX(0.0f), m_fRCPDY(0.0f)
	{
		if (fabsf(m_fDX) > EPSILON) m_fRCPDX = 1.0f / m_fDX;
		if (fabsf(m_fDY) > EPSILON) m_fRCPDY = 1.0f / m_fDY;
	}

	float GetTimeAtX( float fX ) const { return ((fX - m_fX1) * m_fRCPDX); }
	float GetTimeAtY( float fY ) const { return ((fY - m_fY1) * m_fRCPDY); }
	
	float GetXAt( float fY ) const { return (m_fX1 + (GetTimeAtY( fY ) * m_fDX)); }
	float GetYAt( float fX ) const { return (m_fY1 + (GetTimeAtX( fX ) * m_fDY)); }
	
	int  CrossesX( float fX ) const
	{
		if		((m_fX1 < fX) && (fX < m_fX2))		return -1;
		else if ((m_fX2 < fX) && (fX < m_fX1))		return 1;
		else										return 0;
	}

	int  CrossesY( float fY ) const
	{
		if		((m_fY1 < fY) && (fY < m_fY2))		return -1;
		else if ((m_fY2 < fY) && (fY < m_fY1))		return 1;
		else										return 0;
	}
	
private:
	float m_fX1, m_fY1;
	float m_fX2, m_fY2;
	float m_fDX, m_fDY;
	float m_fRCPDX, m_fRCPDY;
};

/***************************************************************************************************
*
*	CLASS			C2DLineSegment
*
*	DESCRIPTION		Line class that represnts a line in the form: [ax + by + c = 0]
*
***************************************************************************************************/
class C2DLineSegment
{
public:
	// must specify start and end points of line
	C2DLineSegment( float fX1, float fY1, float fX2, float fY2 ) :
		m_obStart( fX1, fY1, 0.0f ), m_obEnd( fX2, fY2, 0.0f ) { CalcCoeffs(); }

	// member accesors
	CPoint	Start( void )	const { return m_obStart; }
	CPoint	End( void )		const { return m_obEnd; }
	float	DX( void )		const { return m_fDX; }
	float	DY( void )		const { return m_fDY; }
	float	DRSq( void )	const { return m_fDRSq; }
	float	Det( void )		const { return m_fDeterminant; }

	void	SetStart( float fX, float fY )	{ m_obStart = CPoint(fX,fY,0.0f); CalcCoeffs(); }
	void	SetEnd( float fX, float fY )	{ m_obEnd = CPoint(fX,fY,0.0f); CalcCoeffs(); }

	// coeff accesors
	float	A( void )		const { return DY(); }
	float	B( void )		const { return -DX(); }
	float	C( void )		const { return -Det(); }

	// test for special cases
	bool	IsVertical( void )		const { return (fabs(DX()) < EPSILON); }
	bool	IsHorizontal( void )	const { return (fabs(DY()) < EPSILON); }
	bool	IsPoint( void )			const { return (m_fDRSq < EPSILON); }

	// Get time on line for this projected point
	float	ProjectPoint( float fX, float fY ) const
	{
		ntAssert(!IsPoint());
		
		fX = (Start().X() - fX) * DX();
		fY = (Start().Y() - fY) * DY();
		return -(fX + fY) / DRSq();
	}

	// see if point projection is within line segment time range
	bool	WithinBounds( float fX, float fY ) const
	{
		// extreme case
		if (IsPoint())
		{
			if((fabsf(fX-Start().X()) < EPSILON) && (fabsf(fY-Start().Y()) < EPSILON))
				return true;
			else
				return false;
		}

		float fProjection = ProjectPoint(fX,fY);
		if	(
			(fProjection >= 0.0f) &&
			(fProjection <= 1.0f)
			)
			return true;

		return false;
	}

	// get closest distance to line from point
	float	DistanceTo( float fX, float fY ) const
	{
		CDirection obToPoint( Start() ^ CPoint(fX,fY,0.0f) );

		// extreme case
		if (IsPoint())
		{
			return obToPoint.Length();
		}
		else
		{
			CDirection obLine( End() ^ Start() );
			CDirection obCross( obLine.Cross( obToPoint ) );
			return (obCross.Length() / obLine.Length());
		}
	}

	// get closest point on line from point
	CPoint	ClosestPoint( float fX, float fY ) const
	{
		// extreme case
		if (IsPoint())
		{
			return Start();
		}
		else
		{
			float fProjection = ProjectPoint(fX,fY);
			fProjection = ntstd::Clamp( fProjection, 0.0f, 1.0f );
			return CPoint( (fProjection * DX()) + Start().X(), (fProjection * DY()) + Start().Y(), 0.0f );
		}
	}

	// evalutate line equation at this point
	float	Evaluate( float fX, float fY ) const { return (A() * fX) + (B() * fY) + C(); }

	// get t parameter
	float	GetTimeAtX( float fX ) const { ntAssert(!IsVertical());	return ((fX - Start().X()) / DX()); }
	float	GetTimeAtY( float fY ) const { ntAssert(!IsHorizontal());	return ((fY - Start().Y()) / DY()); }

	// solve line equation for x or y
//	float	GetXAtY( float fY ) const { return ( ((DX()/DY())*fY) + (Det() / DY()) ); }
//	float	GetYAtX( float fX ) const { return ( ((DY()/DX())*fX) - (Det() / DX()) ); }

	// cheaper parametric evaluation
	float	GetXAtY( float fY ) const { ntAssert(!IsPoint());	return ( (GetTimeAtY(fY) * DX()) + Start().X() ); }
	float	GetYAtX( float fX ) const { ntAssert(!IsPoint());	return ( (GetTimeAtX(fX) * DY()) + Start().Y() ); }

	float	GetXAtYSafe( float fY ) const
	{
		if (IsHorizontal())
		{
			ntAssert_p( fabsf(fY - Start().Y()) < EPSILON, ("fY not on line") );
			return Start().X();
		}
		else
			return GetXAtY(fY);
	}

	float	GetYAtXSafe( float fX ) const
	{
		if (IsVertical())
		{
			ntAssert_p( fabsf(fX - Start().X()) < EPSILON, ("fX not on line") );
			return Start().Y();
		}
		else
			return GetYAtX(fX);
	}

	enum INTERSECT_TYPE
	{
		NO_INTERSECT,
		CO_LINEAR,
		INTERSECTS
	};

	// do these lines cross?
	INTERSECT_TYPE	Intersects( const C2DLineSegment& obLine ) const
	{
		// see if one line is wholely on one side of the other
		if (IntersectReject(obLine))
			return NO_INTERSECT;
		
		// if the denominator is zero, we've co-linear lines.
		if (fabsf((DX()*obLine.DY()) - (obLine.DX()*DY()))<EPSILON)
			return CO_LINEAR;

		return INTERSECTS;
	}

	// do these lines cross? if so, compute intersect point
	INTERSECT_TYPE	GetIntersection( const C2DLineSegment& obLine, CPoint& obResult ) const
	{
		// see if one line is wholely on one side of the other
		if (IntersectReject(obLine))
			return NO_INTERSECT;

		// if the denominator is zero, we've co-linear lines.
		float fDenominator = (DX()*obLine.DY()) - (obLine.DX()*DY());
		if (fabsf(fDenominator)<EPSILON)
			return CO_LINEAR;

		obResult.Clear();
		obResult.X() = ( (DX() * obLine.Det()) - (Det() * obLine.DX()) )/fDenominator;
		obResult.Y() = ( (DY() * obLine.Det()) - (Det() * obLine.DY()) )/fDenominator;
		
		return INTERSECTS;
	}

private:
	static bool SameSigns(float fX1, float fX2) { return (((fX1 < 0.0f) && (fX2 < 0.0f)) || ((fX1 > 0.0f) && (fX2 > 0.0f))); }

	// see if one line is wholely on one side of the other
	bool IntersectReject( const C2DLineSegment& obLine ) const
	{
		float fP1 = Evaluate( obLine.Start().X(), obLine.Start().Y() );
		float fP2 = Evaluate( obLine.End().X(), obLine.End().Y() );

		if ((fabsf(fP1)>EPSILON) && (fabsf(fP2)>EPSILON) && (SameSigns(fP1,fP2)))
			return true;

		float fP3 = obLine.Evaluate( Start().X(), Start().Y() );
		float fP4 = obLine.Evaluate( End().X(), End().Y() );

		if ((fabsf(fP3)>EPSILON) && (fabsf(fP4)>EPSILON) && (SameSigns(fP3,fP4)))
			return true;

		return false;
	}

	void CalcCoeffs( void )
	{
		m_fDX = m_obEnd.X() - m_obStart.X();
		m_fDY = m_obEnd.Y() - m_obStart.Y();
		m_fDRSq = (m_fDX * m_fDX) + (m_fDY * m_fDY);
		m_fDeterminant = (m_obStart.X() * m_obEnd.Y()) - (m_obEnd.X() * m_obStart.Y());
	}

	CPoint	m_obStart, m_obEnd;
	float	m_fDX, m_fDY, m_fDRSq, m_fDeterminant;
};

/***************************************************************************************************
*
*	CLASS			CCircle
*
*	DESCRIPTION		Simple Circle class
*
***************************************************************************************************/
class CCircle
{
public:
	CCircle( float fX, float fY, float fR ) :
		m_obOrigin( fX, fY, 0.0f ), m_fRadius(fR) {}

	enum INTERSECT_TYPE
	{
		NO_INTERSECT,
		TANGENT,
		INTERSECTS_1,
		INTERSECTS_2,
	};

	CPoint	GetOrigin( void ) const { return m_obOrigin; }
	float	GetRadius( void ) const { return m_fRadius; }

	// do these lines cross? if so, compute intersect point
	INTERSECT_TYPE	GetIntersection( const C2DLineSegment& obLine, CPoint& obResult1, CPoint& obResult2 ) const
	{
		C2DLineSegment obLocal( obLine.Start().X() - GetOrigin().X(),
								obLine.Start().Y() - GetOrigin().Y(),
								obLine.End().X() - GetOrigin().X(),
								obLine.End().Y() - GetOrigin().Y() );

		float fRadiusSq = GetRadius() * GetRadius(); 
		float fDiscriminant = (fRadiusSq * obLocal.DRSq()) - (obLocal.Det() * obLocal.Det());

		// no results
		//-----------
		if (fDiscriminant < 0.0f) 
			return NO_INTERSECT;

		// line is tangetial, may have a result
		//-------------------------------------
		if (fabsf(fDiscriminant) < EPSILON) 
		{
			if (obLocal.IsPoint())
			{
				if ((obLocal.Start().LengthSquared() - fRadiusSq) < EPSILON)
				{
					obResult1 = obLocal.Start() + GetOrigin();
					return TANGENT;
				}
				return NO_INTERSECT;
			}

			obResult1.Clear();
			obResult1.X() = (obLocal.Det() * obLocal.DY()) / obLocal.DRSq();
			obResult1.Y() = (-obLocal.Det() * obLocal.DX()) / obLocal.DRSq();
			
			if (obLocal.WithinBounds( obResult1.X(), obResult1.Y() ))
			{
				obResult1 += GetOrigin();
				return TANGENT;
			}
			return NO_INTERSECT;
		}

		// line crosses circle, may have 0, 1, or 2 results.
		//-------------------------------------------------
		// get the first intersect point
		obResult1.Clear();
		GetIntersectPoint( obLocal, fDiscriminant, obResult1, true );
		if (obLocal.WithinBounds( obResult1.X(), obResult1.Y() ))
		{
			obResult1 += GetOrigin();

			// get the second intersect point
			obResult2.Clear();
			GetIntersectPoint( obLocal, fDiscriminant, obResult2, false );
			if (obLocal.WithinBounds( obResult2.X(), obResult2.Y() ))
			{
				obResult2 += GetOrigin();
				return INTERSECTS_2;
			}
			return INTERSECTS_1;
		}

		// get the second intersect point
		GetIntersectPoint( obLocal, fDiscriminant, obResult1, false );
		if (obLocal.WithinBounds( obResult1.X(), obResult1.Y() ))
		{
			obResult1 += GetOrigin();
			return INTERSECTS_1;
		}
		
		return NO_INTERSECT;
	}

private:
	static float Sign(float f) { return	((f < 0.0f) ? -1.0f : 1.0f); }

	void	GetIntersectPoint( const C2DLineSegment& obLine, float fDiscriminant, CPoint& obResult, bool bPositive ) const
	{
		float fPartialX = Sign(obLine.DY()) * obLine.DX() * fsqrtf( fDiscriminant );
		float fPartialY = fabsf(obLine.DY()) * fsqrtf( fDiscriminant );

		obResult.X() = obLine.Det() * obLine.DY();
		obResult.X() += bPositive ? fPartialX : -fPartialX;

		obResult.Y() = -obLine.Det() * obLine.DX();
		obResult.Y() += bPositive ? fPartialY : -fPartialY;

		obResult /= obLine.DRSq();
	}

	CPoint	m_obOrigin;
	float	m_fRadius;
};

/***************************************************************************************************
*
*	CLASS			CSimpleSphere
*
*	DESCRIPTION		Simple sphere class
*
***************************************************************************************************/
class CSimpleSphere
{
public:
	CSimpleSphere( const CPoint& obOrigin, float fR ) :
		m_obOrigin( obOrigin ), m_fRadius(fR) {}

	enum INTERSECT_TYPE
	{
		NO_INTERSECT,
		TANGENT,
		INTERSECTS_1,
		INTERSECTS_2,
	};

	CPoint	GetOrigin( void ) const { return m_obOrigin; }
	float	GetRadius( void ) const { return m_fRadius; }

	// do these lines cross? if so, compute intersect point
	INTERSECT_TYPE	GetIntersection( const CPoint& obStart, const CPoint& obEnd, CPoint& obResult1, CPoint& obResult2 ) const
	{
		float fRadiusSq = m_fRadius * m_fRadius; 

		CDirection obLineSeg( obEnd ^ obStart );
		CDirection obToStart( obStart ^ m_obOrigin );

		float fA = obLineSeg.LengthSquared();
		float fB = 2.0f * obLineSeg.Dot(obToStart);
		float fC = m_obOrigin.LengthSquared() + obStart.LengthSquared() - (2.0f * m_obOrigin.Dot(obStart)) - fRadiusSq;

		float fDiscriminant = (fB * fB) - (4 * fA * fC);

		// no results
		//-----------
		if (fDiscriminant < 0.0f) 
			return NO_INTERSECT;

		// line is tangetial, may have a result
		//-------------------------------------
		if (fabsf(fDiscriminant) < EPSILON)
		{
			if (fabsf(fA) < EPSILON)
			{
				if ((obStart.LengthSquared() - fRadiusSq) < EPSILON)
				{
					obResult1 = obStart;
					return TANGENT;
				}
				return NO_INTERSECT;
			}

			float fTime = -fB / (2.0f * fA);
			if ((fTime >= 0.0f) && (fTime <= 1.0f))
			{
				obResult1 = obStart + (obLineSeg * fTime);
				return TANGENT;
			}
			return NO_INTERSECT;
		}

		// line crosses sphere, may have 0, 1, or 2 results.
		//-------------------------------------------------
		fDiscriminant = fsqrtf(fDiscriminant);

		float fTime = (-fB + fDiscriminant) / (2.0f * fA);
		if ((fTime >= 0.0f) && (fTime <= 1.0f))
		{
			obResult1 = obStart + (obLineSeg * fTime);

			fTime = (-fB - fDiscriminant) / (2.0f * fA);
			if ((fTime >= 0.0f) && (fTime <= 1.0f))
			{
				obResult2 = obStart + (obLineSeg * fTime);
				return INTERSECTS_2;
			}
			return INTERSECTS_1;
		}

		fTime = (-fB - fDiscriminant) / (2.0f * fA);
		if ((fTime >= 0.0f) && (fTime <= 1.0f))
		{
			obResult1 = obStart + (obLineSeg * fTime);
			return INTERSECTS_1;
		}

		return NO_INTERSECT;
	}

private:
	CPoint	m_obOrigin;
	float	m_fRadius;
};

#endif // _CAM_GEOM_H
