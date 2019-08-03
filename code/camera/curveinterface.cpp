/***************************************************************************************************
*
*	$Header:: /game/curveinterface.cpp 1     11/08/03 16:30 Wil                                    $
*
*
*
*	CHANGES
*
*	11/6/2003	Wil	Created
*
***************************************************************************************************/

#include "camera/camutils.h"
#include "curves.h"

// name classes used for template curve serialisation
const char* SBezierName::gpcName		= "CCubicBezier";
const char* SHermiteName::gpcName		= "CCubicHermite";
const char* SCatmullName::gpcName		= "CCubicCatmull";
const char* SUBSName::gpcName			= "CCubicUBS";

const char* STimeBezierName::gpcName	= "CCubicTimeBezier";
const char* STimeHermiteName::gpcName	= "CCubicTimeHermite";
const char* STimeCatmullName::gpcName	= "CCubicTimeCatmull";
const char* STimeUBSName::gpcName		= "CCubicTimeUBS";



/***************************************************************************************************
*
*	FUNCTION		CCurveTimeModule::CCurveTimeModule
*
*	DESCRIPTION		construct the time thingy
*
***************************************************************************************************/
CCurveTimeModule::CCurveTimeModule( u_int uiTimes, const float* pfTimes, bool bOwnsData )
{
	ntAssert( pfTimes );
	ntAssert( uiTimes > 1 );

	m_uiNumTimes = uiTimes;

	if(bOwnsData)
	{
		m_pfTimes = pfTimes;
	}
	else
	{
		float* pfNewTimes = NT_NEW_CHUNK( Mem::MC_CAMERA ) float [m_uiNumTimes];
		NT_MEMCPY( pfNewTimes, pfTimes, sizeof(float)*m_uiNumTimes );
		m_pfTimes = pfNewTimes;
	}

#ifdef _DEBUG
	for (u_int i = 0; i < GetNumTimes()-1; i++)
	{
		ntAssert_p( GetTime(i) <= GetTime(i+1), ("Times must increase in CCurveTimeModule") );
	}
#endif
}

/***************************************************************************************************
*
*	FUNCTION		CCurveTimeModule::GetSegmentAndTime
*
*	DESCRIPTION		Get the segment relative time and segment index given a normailise time
*
***************************************************************************************************/
inline u_int CCurveTimeModule::GetSegmentAndTime( float& fU ) const
{
	ntAssert( fU <= 1.0f );
	ntAssert( fU >= 0.0f );

	float fStartTime = GetTime(0);
	fU = (fU * GetRange()) + fStartTime;

	for (u_int i = 0; i < GetNumTimes() - 1; i++)
	{
		float fEndTime = GetTime(i+1);
		if	(
			(fStartTime <= fU) &&
			(fU <= fEndTime)
			)
		{
			fU -= GetTime(i);
			if (GetDuration(i)>EPSILON)
				fU /= GetDuration(i);
			return i;
		}
	}
	ntAssert(0);
	return 0;
}









/***************************************************************************************************
*
*	FUNCTION		CTimeCurveInterface::Evaluate
*
*	DESCRIPTION		evaluate the curve at this time
*
***************************************************************************************************/
CVector	CTimeCurveInterface::Evaluate( float fU ) const
{
	ntAssert( !m_bInvalid );
	int iSegment = m_pobTimeModule->GetSegmentAndTime( fU );

	CVector aobCoeffs[CUBIC_COEFFICIENTS];
	GetCoeffs( iSegment, aobCoeffs );

	CVector obResult = aobCoeffs[0];
	for (int i = 1; i < CUBIC_COEFFICIENTS; i++)
	{
		obResult *= fU;
		obResult += aobCoeffs[i];
	}
	
	return obResult;
}

/***************************************************************************************************
*
*	FUNCTION		CTimeCurveInterface::Derivative1
*
*	DESCRIPTION		get the 1st derviative of the curve at this time
*
***************************************************************************************************/
CVector	CTimeCurveInterface::Derivative1( float fU ) const
{
	ntAssert( !m_bInvalid );
	int iSegment = m_pobTimeModule->GetSegmentAndTime( fU );

	CVector aobCoeffs[CUBIC_COEFFICIENTS];
	GetCoeffs( iSegment, aobCoeffs );

	CVector obResult = aobCoeffs[0] * (3.0f * fU);
	obResult += aobCoeffs[1] * 2.0f;
	obResult *= fU;
	obResult += aobCoeffs[2];
	obResult *= m_pobTimeModule->GetDuration(iSegment);
	
	return obResult;
}

/***************************************************************************************************
*
*	FUNCTION		CTimeCurveInterface::Derivative2
*
*	DESCRIPTION		Get the second derivative of the curve at this point
*
***************************************************************************************************/
CVector	CTimeCurveInterface::Derivative2( float fU ) const
{
	ntAssert( !m_bInvalid );
	int iSegment = m_pobTimeModule->GetSegmentAndTime( fU );

	CVector aobCoeffs[CUBIC_COEFFICIENTS];
	GetCoeffs( iSegment, aobCoeffs );

	CVector obResult = aobCoeffs[0] * (6.0f * fU);
	obResult += aobCoeffs[1] * 2.0f;
	obResult *= m_pobTimeModule->GetDuration(iSegment) * m_pobTimeModule->GetDuration(iSegment);
	
	return obResult;
}






/***************************************************************************************************
*
*	DATA			CCurveInterface:::gafPoly2__SomeCurve__
*
*	DESCRIPTION		These are the inverse basis matrices which convert the polynomial coeffs 
*					at^3 + bt^2 + ct + d for a curve to the respective 4 4vectors that govern a
*					segement of a curve. Thus to convert any curve to a hermite:
*
*					[ p0, p1, t0, t1 ] =	[	0	0	0	1 ] * [ a ]
*											[	1	1	1	1 ]   [ b ]
*											[	0	0	1/3	0 ]   [ c ]
*											[	1	2/3	1/3	0 ]   [ d ]
*
*	NOTES			We canot convert from a Nurbs curve in this manner, as polynomal coeffs
*					cannot be calculated for any given segment.
*
***************************************************************************************************/
const float CCurveInterface::gafPoly2Bezier[4][4] = 
{
	{	0.0f,	0.0f,		0.0f,		1.0f },
	{	0.0f,	0.0f,		1.0f/3.0f,	1.0f },
	{	0.0f,	1.0f/3.0f,	2.0f/3.0f,	1.0f },
	{	1.0f,	1.0f,		1.0f,		1.0f }
};

const float CCurveInterface::gafPoly2Hermite[4][4] = 
{
	{	0.0f,	0.0f,		0.0f,		1.0f },
	{	1.0f,	1.0f,		1.0f,		1.0f },
	{	0.0f,	0.0f,		1.0f/3.0f,	0.0f },
	{	1.0f,	2.0f/3.0f,  1.0f/3.0f,	0.0f }
};

const float CCurveInterface::gafPoly2Catmull[4][4] = 
{
	{	1.0f,	1.0f,	-1.0f,	1.0f },
	{	0.0f,	0.0f,	0.0f,	1.0f },
	{	1.0f,	1.0f,	1.0f,	1.0f },
	{	6.0f,	4.0f,	2.0f,	1.0f }
};

const float CCurveInterface::gafPoly2CubicUBS[4][4] = 
{
	{	0.0f,	2.0f/3.0f,	-1.0f,	1.0f },
	{	0.0f,	-1.0f/3.0f,	0.0f,	1.0f },
	{	0.0f,	2.0f/3.0f,	1.0f,	1.0f },
	{	6.0f,	11.0f/3.0f,	2.0f,	1.0f }
};

/***************************************************************************************************
*
*	FUNCTION			CCurveInterface::GetSegmentAndTime
*
*	DESCRIPTION		Get the segment relative time and segment index given a normailise time
*
***************************************************************************************************/
inline u_int CCurveInterface::GetSegmentAndTime( float& fU ) const
{
	if (fU == 1.0f) fU -= EPSILON;
	fU *= (float)GetNumSpan();
	u_int uiSegment = (int)fU;
	fU -= (float)uiSegment;
	return uiSegment;
}



/***************************************************************************************************
*
*	FUNCTION		CCurveInterface::CalcLength	
*
*	DESCRIPTION		Generate the length of the spline (expensive! prefer to pre-calc if possible)
*
***************************************************************************************************/
void	CCurveInterface::CalcLength( void ) const
{
	m_fLength = 0.0f;
	CPoint obStart = GetPoint(0.0f);
	for (float fU = GetSampleInterval(); fU < 1.0f; fU += GetSampleInterval())
	{
		CPoint obEnd = GetPoint(fU);
		CDirection obVec = obEnd ^ obStart;
		m_fLength += obVec.Length();
		obStart = obEnd;
	}

	// last segment
	CPoint	obEnd = GetPoint(1.0f);
	CDirection obVec = obEnd ^ obStart;
	m_fLength += obVec.Length();
}

/***************************************************************************************************
*
*	FUNCTION		CCurveInterface::Evaluate
*
*	DESCRIPTION		evaluate curve at this time
*
***************************************************************************************************/
CVector	CCurveInterface::Evaluate( float fU ) const
{
	ntAssert( !m_bInvalid );
	ntAssert( fU <= 1.0f );
	ntAssert( fU >= 0.0f );

	int iSegment = GetSegmentAndTime( fU );

	CVector aobCoeffs[CUBIC_COEFFICIENTS];
	GetCoeffs( iSegment, aobCoeffs );

	CVector obResult = aobCoeffs[0];
	for (int i = 1; i < CUBIC_COEFFICIENTS; i++)
	{
		obResult *= fU;
		obResult += aobCoeffs[i];
	}
	
	return obResult;
}

/***************************************************************************************************
*
*	FUNCTION		CCurveInterface::Derivative1
*
*	DESCRIPTION		get the 1st derivative at this time
*
***************************************************************************************************/
CVector	CCurveInterface::Derivative1( float fU ) const
{
	ntAssert( !m_bInvalid );
	ntAssert( fU <= 1.0f );
	ntAssert( fU >= 0.0f );

	int iSegment = GetSegmentAndTime( fU );

	CVector aobCoeffs[CUBIC_COEFFICIENTS];
	GetCoeffs( iSegment, aobCoeffs );

	CVector obResult = aobCoeffs[0] * (3.0f * fU);
	obResult += aobCoeffs[1] * 2.0f;
	obResult *= fU;
	obResult += aobCoeffs[2];
	
	return obResult;
}

/***************************************************************************************************
*
*	FUNCTION		CCurveInterface::Derivative2
*
*	DESCRIPTION		get the second derivative at this time
*
***************************************************************************************************/
CVector	CCurveInterface::Derivative2( float fU ) const
{
	ntAssert( !m_bInvalid );
	ntAssert( fU <= 1.0f );
	ntAssert( fU >= 0.0f );

	int iSegment = GetSegmentAndTime( fU );

	CVector aobCoeffs[CUBIC_COEFFICIENTS];
	GetCoeffs( iSegment, aobCoeffs );

	CVector obResult = aobCoeffs[0] * (6.0f * fU);
	obResult += aobCoeffs[1] * 2.0f;
	
	return obResult;
}

/***************************************************************************************************
*
*	FUNCTION		CCurveInterface:::CreateBezier
*
*	DESCRIPTION		Create a bezier curve via the curve coefficients and a conversion matrix
*
*	NOTES			This is good for all curves, as beziers make no assumptions about 
*					gradient contiuity
*
***************************************************************************************************/
CCubicBezier*	CCurveInterface::CreateBezier( void ) const
{
	ntAssert(HasPolynomial());

	int iNumBezierCV = (GetNumSpan() * 3) + 1;

	CVector* pobCVs = NT_NEW_CHUNK( Mem::MC_CAMERA ) CVector[ iNumBezierCV ];
	ntAssert(pobCVs);

	for (u_int i = 0; i < GetNumSpan(); i++)
	{
		CVector	aobCoeffs[CUBIC_COEFFICIENTS];
		CVector	aobModCoeffs[CUBIC_COEFFICIENTS];

		GetCoeffs( i, aobCoeffs );

		for (int iIndex = 0; iIndex < CUBIC_COEFFICIENTS; iIndex++)
		{
			aobModCoeffs[iIndex].Clear();
			for (int j = 0; j < CUBIC_COEFFICIENTS; j++)
				aobModCoeffs[iIndex] += aobCoeffs[j] * gafPoly2Bezier[iIndex][j];
		}

		pobCVs[(i*3)] = aobModCoeffs[0];
		pobCVs[(i*3)+1] = aobModCoeffs[1];
		pobCVs[(i*3)+2] = aobModCoeffs[2];
		pobCVs[(i*3)+3] = aobModCoeffs[3];
	}

	CCubicBezier* pobNewCurve = NT_NEW_CHUNK( Mem::MC_CAMERA ) CCubicBezier( GetOpen(), iNumBezierCV, pobCVs, true,  GetInvalidLength() );
	ntAssert(pobNewCurve);

	return pobNewCurve;
}

/***************************************************************************************************
*
*	DATA			CCurveInterface:::CreateHermite
*
*	DESCRIPTION		Create a hermite curve using the above conversion matrix
*
*	NOTES			This is good for all curves, as hermites make no assumptions about 
*					gradient contiuity
*
***************************************************************************************************/
CCubicHermite*	CCurveInterface::CreateHermite( void ) const
{
	ntAssert(HasPolynomial());

	int iNumHermiteCV = GetNumSpan() + 1;
	int iNumHermiteTan = GetNumSpan();

	CVector* pobCVs = NT_NEW_CHUNK( Mem::MC_CAMERA ) CVector[ iNumHermiteCV + (iNumHermiteTan * 2)];
	ntAssert(pobCVs);

	CVector* pobStarts = pobCVs + iNumHermiteCV;
	CVector* pobEnds = pobCVs + (iNumHermiteCV + iNumHermiteTan);

	for (u_int i = 0; i < GetNumSpan(); i++)
	{
		CVector	aobCoeffs[CUBIC_COEFFICIENTS];
		CVector	aobModCoeffs[CUBIC_COEFFICIENTS];

		GetCoeffs( i, aobCoeffs );

		for (int iIndex = 0; iIndex < CUBIC_COEFFICIENTS; iIndex++)
		{
			aobModCoeffs[iIndex].Clear();
			for (int j = 0; j < CUBIC_COEFFICIENTS; j++)
				aobModCoeffs[iIndex] += aobCoeffs[j] * gafPoly2Hermite[iIndex][j];
		}

		pobCVs[i] = aobModCoeffs[0];
		pobCVs[i+1] = aobModCoeffs[1];
		pobStarts[i] = aobModCoeffs[2];
		pobEnds[i] = aobModCoeffs[3];
	}

	CCubicHermite* pobNewCurve = NT_NEW_CHUNK( Mem::MC_CAMERA ) CCubicHermite( GetOpen(), iNumHermiteCV, pobCVs, pobStarts, pobEnds, true, false, GetInvalidLength() );
	ntAssert(pobNewCurve);

	return pobNewCurve;
}

/***************************************************************************************************
*
*	DATA			CCurveInterface:::CreateCatmull
*
*	DESCRIPTION		Create a catmull curve using the above conversion matrix
*
*	NOTES			this is only good for converting individual segments, as the source curve may
*					not be C2 continous at the segement boundaries
*					We could have a catmull representaion with degenerates inbetween each segment
*					but this would kindof defeat the point, so best not to bother really.
*
***************************************************************************************************/
CCubicCatmull*	CCurveInterface::CreateCatmull( void ) const
{
	ntAssert(HasPolynomial());

	int iNumCatmullCV = GetNumSpan() + 3;

	CVector* pobCVs = NT_NEW_CHUNK( Mem::MC_CAMERA ) CVector[ iNumCatmullCV ];
	ntAssert(pobCVs);

	for (u_int i = 0; i < GetNumSpan(); i++)
	{
		CVector	aobCoeffs[CUBIC_COEFFICIENTS];
		CVector	aobModCoeffs[CUBIC_COEFFICIENTS];

		GetCoeffs( i, aobCoeffs );

		for (int iIndex = 0; iIndex < CUBIC_COEFFICIENTS; iIndex++)
		{
			aobModCoeffs[iIndex].Clear();
			for (int j = 0; j < CUBIC_COEFFICIENTS; j++)
				aobModCoeffs[iIndex] += aobCoeffs[j] * gafPoly2Catmull[iIndex][j];
		}

		pobCVs[i] = aobModCoeffs[0];
		pobCVs[i+1] = aobModCoeffs[1];
		pobCVs[i+2] = aobModCoeffs[2];
		pobCVs[i+3] = aobModCoeffs[3];
	}

	CCubicCatmull* pobNewCurve = NT_NEW_CHUNK( Mem::MC_CAMERA ) CCubicCatmull( GetOpen(), iNumCatmullCV, pobCVs, true, GetInvalidLength() );
	ntAssert(pobNewCurve);

	return pobNewCurve;
}

/***************************************************************************************************
*
*	DATA			CCurveInterface:::CreateUBS
*
*	DESCRIPTION		Create a uniform B spline using the above conversion matrix
*
*	NOTES			this is only good for converting individual segments, as the source curve may
*					not be C2 countinous at the segement boundaries
*
***************************************************************************************************/
CCubicUBS*	CCurveInterface::CreateUBS( void ) const
{
	ntAssert(HasPolynomial());

	int iNumUBSCV = GetNumSpan() + 3;

	CVector* pobCVs = NT_NEW_ARRAY_CHUNK( Mem::MC_CAMERA ) CVector[ iNumUBSCV ];
	ntAssert(pobCVs);

	for (u_int i = 0; i < GetNumSpan(); i++)
	{
		CVector	aobCoeffs[CUBIC_COEFFICIENTS];
		CVector	aobModCoeffs[CUBIC_COEFFICIENTS];

		GetCoeffs( i, aobCoeffs );

		for (int iIndex = 0; iIndex < CUBIC_COEFFICIENTS; iIndex++)
		{
			aobModCoeffs[iIndex].Clear();
			for (int j = 0; j < CUBIC_COEFFICIENTS; j++)
				aobModCoeffs[iIndex] += aobCoeffs[j] * gafPoly2CubicUBS[iIndex][j];
		}

		pobCVs[i] = aobModCoeffs[0];
		pobCVs[i+1] = aobModCoeffs[1];
		pobCVs[i+2] = aobModCoeffs[2];
		pobCVs[i+3] = aobModCoeffs[3];
	}

	CCubicUBS* pobNewCurve = NT_NEW_CHUNK( Mem::MC_CAMERA ) CCubicUBS( GetOpen(), iNumUBSCV, pobCVs, true, GetInvalidLength() );
	ntAssert(pobNewCurve);

	return pobNewCurve;
}

/***************************************************************************************************
*
*	FUNCTION		CCurveInterface::GetNearFar
*
*	DESCRIPTION		find nearest / furthest point on curve to a given point
*					returns normalised parameter and fills in squared distance between points
*
*	INPUTS			obPoint-			point to test
*					fDistanceSq-		distance square return val
*					bXZOnly-			test distance in XZ only
*					fDistSqTolerance-	distance we say is good enough
*					fTimeTolerance-		time increment we say is good enough
*					bNear-				are we doing closest or furthest point?
*
***************************************************************************************************/
float	CCurveInterface::GetNearFar(	const CPoint& obPoint,
										float& fDistanceSq,
										bool bXZOnly,
										bool bNear,
										float fDistSqTolerance,
										float fTimeTolerance ) const
{
	ntAssert( fDistSqTolerance >= 0.0f );
	ntAssert( fTimeTolerance >= 0.0f );

	float fReturnVal;
	float fDistMinMax = bNear ? 1e10f : 0.0f;

	u_int uiMinMaxInd = 0;
	u_int uiNumTest = 2 * GetNumSpan();

	if (fTimeTolerance <= EPSILON)
		fTimeTolerance = 0.001f;

	// coarse test to find the nearest / furthest point with 2 * span tests
	for (u_int i = 0; i <= uiNumTest; i++)
	{
		float fTestVal = i*(1.0f/uiNumTest);
		float fDist = bXZOnly ? GetDistSqXZ( fTestVal, obPoint ) : GetDistSq( fTestVal, obPoint );
		
		if ( ((bNear) && (fDist < fDistMinMax)) || ((!bNear) && (fDist > fDistMinMax)) )
		{
			fDistMinMax = fDist;
			uiMinMaxInd = i;
		}
	}

	// resolve what segment(s) to test in depth
	if	(
		( !GetOpen() ) &&
		((uiMinMaxInd < 1) || (uiMinMaxInd > uiNumTest-1))
		)
	{
		// we're periodic and on the boundary
		float fLowerSegTime, fLowerSegDistance;
		float fUpperSegTime, fUpperSegDistance;

		fLowerSegTime = GetNearFarHighRez( 0, 1, obPoint, fLowerSegDistance, bXZOnly, bNear, fDistSqTolerance, fTimeTolerance );
		fUpperSegTime = GetNearFarHighRez( uiNumTest-1, uiNumTest, obPoint, fUpperSegDistance, bXZOnly, bNear, fDistSqTolerance, fTimeTolerance );

		if (fLowerSegDistance < fUpperSegDistance)
		{
			fDistanceSq = bNear ? fLowerSegDistance : fUpperSegDistance;
			fReturnVal = bNear ? fLowerSegTime : fUpperSegTime;
		}
		else
		{
			fDistanceSq = bNear ? fUpperSegDistance : fLowerSegDistance;
			fReturnVal = bNear ? fUpperSegTime : fLowerSegTime;
		}
	}
	else
	{
		// normal
		if (uiMinMaxInd < 1)
			uiMinMaxInd = 1;
		else if (uiMinMaxInd > uiNumTest-1)
			uiMinMaxInd = uiNumTest-1;

		fReturnVal = GetNearFarHighRez( uiMinMaxInd-1, uiMinMaxInd+1, obPoint, fDistanceSq, bXZOnly, bNear, fDistSqTolerance, fTimeTolerance );
	}

	return fReturnVal;
}

/***************************************************************************************************
*
*	FUNCTION		CCurveInterface::GetNearFarHighRez
*
*	DESCRIPTION		find nearest point on curve to given point
*					returns (un-normalised) parameter and fills in squared distance between points
*
*	NOTES			takes min and max segments of the spline as arguments
*
*	INPUTS			obPoint-			point to test
*					fDistanceSq-		distance square return val
*					bXZOnly-			test distance in XZ only
*					fDistSqTolerance-	distance we say is good enough
*					fTimeTolerance-		time increment we say is good enough
*					bNear-				are we doing closest or furthest point?
*
***************************************************************************************************/
float	CCurveInterface::GetNearFarHighRez(	u_int uiMin,
											u_int uiMax,
											const CPoint& obPoint,
											float& fDistanceSq,
											bool bXZOnly,
											bool bNear,
											float fDistSqTolerance,
											float fTimeTolerance ) const
{
	ntAssert( uiMin < uiMax );
	ntAssert( uiMin <= 2 * GetNumSpan() );
	ntAssert( uiMax <= 2 * GetNumSpan() );

	// generate higher res time bounds
	float fMin = uiMin*(1.0f/(2.0f * GetNumSpan()));
	float fMax = uiMax*(1.0f/(2.0f * GetNumSpan()));

	// get distances at this point
	float fD0 = bXZOnly ? GetDistSqXZ( fMin, obPoint ) : GetDistSq( fMin, obPoint );
	float fD1 = bXZOnly ? GetDistSqXZ( fMax, obPoint ) : GetDistSq( fMax, obPoint );
	
	float fD2;
	float fMid;

	while (1)
	{
		// get mid point distance
		fMid = (fMin + fMax) * 0.5f;
		fD2 = bXZOnly ? GetDistSqXZ( fMid, obPoint ) : GetDistSq( fMid, obPoint );
		
		// good enough ?
		if	(
			(bNear && (fD2 < fDistSqTolerance)) ||
			(!bNear && (fD2 > fDistSqTolerance)) ||
			((fMax-fMin) < fTimeTolerance)
			)
			break;

		// zero in on better half of segment
		if	(
			(bNear && (fD0 < fD1)) ||
			(!bNear && (fD0 > fD1))
			)
		{
			fMax = fMid;
			fD1 = bXZOnly ? GetDistSqXZ( fMax, obPoint ) : GetDistSq( fMax, obPoint );
		}
		else
		{
			fMin = fMid;
			fD0 = bXZOnly ? GetDistSqXZ( fMin, obPoint ) : GetDistSq( fMin, obPoint );
		}
	}
	fDistanceSq = fD2;
	return fMid;
}

/***************************************************************************************************
*
*	FUNCTION		CCurveInterface::GetNearestLinear
*
*	DESCRIPTION		find nearest point on curve by testing each curve segement at N intervals,
*					find next closest point to this either side of the point at a distance of N.
*					assume this section contains the closest point.
*					subdivide this section M times, get the closest point on these line segments
*
*	INPUTS			obPoint- Point to test against
*					fDistanceSq- returned distance square
*					iCurveSubDivs- number of times to subdivide each curve segments by
*					iSectionSubDivs- number of lines to carve subsection into.
*					fDistSqTolerance- distance tolerance
*
***************************************************************************************************/
float	CCurveInterface::GetNearestLinear(	const CPoint& obPoint,
											float& fDistanceSq,
											u_int iCurveSubDivs,
											u_int iSectionSubDivs,
											float fDistSqTolerance ) const
{
	ntAssert( iCurveSubDivs );
	ntAssert( iSectionSubDivs );
	ntAssert( fDistSqTolerance >= 0.0f );

	float fDistMin = 1e10f;
	float fBestVal = 0.0f;

	u_int uiNumTest = iCurveSubDivs * GetNumSpan();

	float fTestInterval = (1.0f/uiNumTest);

	// coarse test to find the nearest / furthest point with (SubDivs * NumSpan) tests
	for (u_int i = 0; i <= uiNumTest; i++)
	{
		float fDist = GetDistSq( fTestInterval * i, obPoint );
		
		if (fDist < fDistMin)
		{
			fDistMin = fDist;
			fBestVal = fTestInterval * i;
		}
	}

	float fStepBack = fBestVal - (fTestInterval * 1.0f);
	float fStepForw = fBestVal + (fTestInterval * 1.0f);

	if (GetOpen())
	{
		fStepBack = ntstd::Clamp( fStepBack, 0.0f, 1.0f );
		fStepForw = ntstd::Clamp( fStepForw, 0.0f, 1.0f );
	}
	
	float fReturnVal;

	// resolve what segment(s) to test in depth
	if	(
		( !GetOpen() ) &&
		( (fStepBack < 0.0f) || (fStepForw > 1.0f) )
		)
	{
		float fLowerSegTime, fLowerSegDistance;
		float fUpperSegTime, fUpperSegDistance;

		if ( fStepBack < 0.0f )
		{
			fLowerSegTime = GetNearestLinHighRez( fStepBack + 1.0f, 1.0f, obPoint, fLowerSegDistance, iSectionSubDivs, fDistSqTolerance );
			fUpperSegTime = GetNearestLinHighRez( 0.0f, fStepForw, obPoint, fUpperSegDistance, iSectionSubDivs, fDistSqTolerance );
		}
		else
		{
			fLowerSegTime = GetNearestLinHighRez( fStepBack, 1.0f, obPoint, fLowerSegDistance, iSectionSubDivs, fDistSqTolerance );
			fUpperSegTime = GetNearestLinHighRez( 0.0f, fStepForw - 1.0f, obPoint, fUpperSegDistance, iSectionSubDivs, fDistSqTolerance );
		}

		if (fLowerSegDistance < fUpperSegDistance)
		{
			fDistanceSq =  fLowerSegDistance;
			fReturnVal = fLowerSegTime;
		}
		else
		{
			fDistanceSq =  fUpperSegDistance;
			fReturnVal = fUpperSegTime;
		}
	}
	else
	{
		fReturnVal = GetNearestLinHighRez( fStepBack, fStepForw, obPoint, fDistanceSq, iSectionSubDivs, fDistSqTolerance );
	}
	
	return fReturnVal;
}

/***************************************************************************************************
*
*	FUNCTION		CCurveInterface::GetNearestLinHighRez
*
*	DESCRIPTION		subdivide this section M times, get the closest point on these line segments
*
*	INPUTS			float fMin, float fMax - range of parameter values to test by
*					obPoint- Point to test against
*					fDistanceSq- returned distance square
*					iSectionSubDivs- number of lines to carve subsection into.
*					fDistSqTolerance- distance tolerance
*
***************************************************************************************************/
float	CCurveInterface::GetNearestLinHighRez(	float fMin,
												float fMax,
												const CPoint& obPoint,
												float& fDistanceSq,
												u_int iSectionSubDivs,
												float fDistSqTolerance ) const
{
	ntAssert( iSectionSubDivs );

	float fRange = fMax - fMin;
	float fInterval = fRange / iSectionSubDivs;
	
	float fDistMin = GetDistSq( fMin, obPoint );
	float fParamMin = fMin;

	CPoint obStart = GetPoint(fMin);
	for (u_int i = 0; i < iSectionSubDivs; i++)
	{
		float fCurrStep = fMin + (fInterval * i);

		CPoint obNext = GetPoint( fCurrStep + fInterval );

		CDirection obSegApprox = obNext ^ obStart;

		float fDistCurr = 0.0f;
		float fParamCurr = fCurrStep;
		float fSegLen = obSegApprox.Length();

		if (fSegLen > EPSILON)
		{
			obSegApprox.Normalise();

			CDirection obPointVec = obPoint ^ obStart;
			
			float fDirDist = ntstd::Clamp( obSegApprox.Dot( obPointVec ), 0.0f, fSegLen );
			obPointVec = (obStart + (obSegApprox * fDirDist)) ^ obPoint;

			fDistCurr = obPointVec.LengthSquared();
			fParamCurr = ((fDirDist * fInterval)/fSegLen) + fCurrStep;
		}
		else
		{
			fDistCurr = GetDistSq( fParamCurr, obPoint );
		}

		if ( fDistCurr < fDistMin )
		{
			fDistMin = fDistCurr;
			fParamMin = fParamCurr;
		}

		if ( fDistMin < fDistSqTolerance )
			break;

		obStart = obNext;
	}
	fDistanceSq = fDistMin;
	return fParamMin;
}

/***************************************************************************************************
*
*	FUNCTION		CCurveInterface::GetNearestAccurate
*
*	DESCRIPTION		find nearest point on curve by testing each curve segement at N intervals,
*					assume! that segment contains the closest point then call GetNearestNewton
*					to resolve to the actual closest point
*
*	INPUTS			obPoint- Point to test against
*					fDistanceSq- returned distance square
*					iSubDivs- number of times to subdivide each curve segment
*					fDistToleranc- distance tolerance
*					fCosTolerance- cosine tolerance
*
***************************************************************************************************/
float	CCurveInterface::GetNearestAccurate(	const CPoint& obPoint,
												float& fDistanceSq,
												u_int iSubDivs,
												float fDistTolerance,
												float fCosTolerance ) const
{
	ntAssert( iSubDivs );
	ntAssert( fDistTolerance >= 0.0f );
	ntAssert( fCosTolerance >= 0.0f );

	float fDistMin = 1e10f;
	float fBestVal = 0.0f;

	u_int uiNumTest = iSubDivs * GetNumSpan();

	// coarse test to find the nearest / furthest point with (SubDivs * NumSpan) tests
	for (u_int i = 0; i <= uiNumTest; i++)
	{
		float fTestVal = i*(1.0f/uiNumTest);
		float fDist = GetDistSq( fTestVal, obPoint );
		
		if (fDist < fDistMin)
		{
			fDistMin = fDist;
			fBestVal = fTestVal;
		}
	}

	// get the actual closest point
	return GetNearestNewton( fBestVal, obPoint, fDistanceSq, fDistTolerance, fCosTolerance );
}

/***************************************************************************************************
*
*	FUNCTION		CCurveInterface::GetNearestNewton
*
*	DESCRIPTION		find nearest point on curve to a starting guess by newton 
*					iteration. Returns paramater and fills in square dist between points
*
*	INPUTS			fStart- Starting guess for closest point
*					obPoint- Point to test against
*					fDistanceSq- returned distance square
*					fDistToleranc- distance tolerance
*					fCosTolerance- cosine tolerance
*
***************************************************************************************************/
float	CCurveInterface::GetNearestNewton(	float fStart,
											const CPoint& obPoint,
											float& fDistanceSq,
											float fDistTolerance,
											float fCosTolerance ) const
{
	ntAssert( fStart <= 1.0f );
	ntAssert( fStart >= 0.0f );
	ntAssert( fDistTolerance >= 0.0f );
	ntAssert( fCosTolerance >= 0.0f );

	float fCurrent = fStart;

	// cap our exit conditions
	if (fDistTolerance <= EPSILON)
		fDistTolerance = EPSILON;

	if (fCosTolerance <= EPSILON)
		fCosTolerance = 0.001f;

	int iMaxIterations = 10;
	while (iMaxIterations--)
	{
		CPoint		obPosition = GetPoint(fCurrent);
		CDirection	ob1stDer = Get1stDerivative(fCurrent);
		CDirection	ob2ndDer = Get2ndDerivative(fCurrent);

		CDirection obProjection = obPosition ^ obPoint;

		// abort if we're on the curve
		float fProjLen = obProjection.Length();
		fDistanceSq = fProjLen * fProjLen;
		
		if (fProjLen <= fDistTolerance)
			break;
		
		float fCosAngle = 0.0f;
		float f1stDerLen = ob1stDer.Length();
		float fDotProduct = ob1stDer.Dot( obProjection );		

		if ( (f1stDerLen * fProjLen) >= EPSILON )
			fCosAngle = fDotProduct / (f1stDerLen * fProjLen);

		// abort if we're tangential to the curve
		if (fabs(fCosAngle) <= fCosTolerance)
			break;

		// calculate how much we wish to move by
		float fDenom = ob2ndDer.Dot( obProjection ) + ob1stDer.Dot( ob1stDer );
		ntAssert( fabsf(fDenom) >= EPSILON );
	
		float fOffset = fDotProduct / fDenom;

		// calculate our new current value
		float fNewCurrent = fCurrent - fOffset;

		// clamp range or wrap for periodic
		if (GetOpen())
		{
			fNewCurrent = ntstd::Clamp( fNewCurrent, 0.0f, 1.0f );
		}
		else
		{
			ntAssert( fNewCurrent >= -1.0f);
			ntAssert( fNewCurrent <= 2.0f);

			if (fNewCurrent < 0.0f)
				fNewCurrent += 1.0f;
			if (fNewCurrent > 1.0f)
				fNewCurrent -= 1.0f;
		}

		// abort if we're off the edge of the curve
		ob1stDer *= fNewCurrent - fCurrent;

		if ( ob1stDer.Length() <= fDistTolerance )
			break;

		fCurrent = fNewCurrent;
	}

	// check we dont have any weird cases
	ntAssert(iMaxIterations > 0);

	return fCurrent;
}

/***************************************************************************************************
*
*	FUNCTION		CCurveInterface::ConvergeOnTargetByFixed
*
*	DESCRIPTION		Traverse the curve towards the target value by a fixed amount.
*
*	INPUTS			fCurrVal - current paramaterised position on the curve
*					fTargetVal - where we want to be on the curve
*					fIncrement - max distance to move in this update (meters)
*					bXZOnly - are we testing just in the XZ plane?
*					fTolerance - Tolerance on distance testing
*
***************************************************************************************************/
float	CCurveInterface::ConvergeOnTargetByFixed(	float fCurrVal,
													float fTargetVal,
													float fIncrement,
													bool bXZOnly,
													float fTolerance ) const
{
	ntAssert( fCurrVal >= 0.0f );
	ntAssert( fCurrVal <= 1.0f );
	ntAssert( fTargetVal >= 0.0f );
	ntAssert( fTargetVal <= 1.0f );

	if (fIncrement == 0.0f)
		return fCurrVal;

	// move along the curve until we're far enough away or we hit the target
	CPoint obCurrentPos = GetPoint( fCurrVal );

	float fMin, fMax, fMid;
	float fDMin, fDMax, fDMid;

	fMin = fCurrVal;
	fMax = fTargetVal;

	fDMin = bXZOnly ? GetDistSqXZ( fMin, obCurrentPos ) : GetDistSq( fMin, obCurrentPos );
	fDMax = bXZOnly ? GetDistSqXZ( fMax, obCurrentPos ) : GetDistSq( fMax, obCurrentPos );

	float fTargDist = fIncrement*fIncrement;

	// need to cope with paramater wrapping on closed curves
	bool bMoveOpposite = false;	
	if ((!GetOpen()) && (fabs(fMax - fMin) > 0.5f))
	{
		bMoveOpposite = true;

		if (fMin < fMax)
			fMin += 1.0f;
		else
			fMax += 1.0f;
	}
			
	while (1)
	{
		// get mid point distance
		float fMidTest = fMid = (fMin + fMax) * 0.5f;

		if (bMoveOpposite)
		{
			if (fMidTest > 1.0f)
				fMidTest -= 1.0f;
			if (fMidTest < 0.0f)
				fMidTest += 1.0f;
		}

		fDMid = bXZOnly ? GetDistSqXZ( fMidTest, obCurrentPos ) : GetDistSq( fMidTest, obCurrentPos );
		
		// quit if we're close enough to the target dist
		if (fabsf(fDMid - fTargDist) < fTolerance)
			break;

		// quit if we've converged to the same place
		if (fabs(fMax - fMin) < EPSILON)
			break;

		// choose next convergence boundary
		if ( fDMid > fTargDist )
			fMax = fMid;
		else
			fMin = fMid;
	}

	// make sure our value is ranged
	if (bMoveOpposite)
	{
		if (fMid > 1.0f)
			fMid -= 1.0f;
		if (fMid < 0.0f)
			fMid += 1.0f;
	}

	return fMid;
}

/***************************************************************************************************
*
*	FUNCTION		CCurveInterface::ConvergeOnTargetAvoiding
*
*	DESCRIPTION		Get Paramater on Curve that is fOffset away from the avoid pos, towards the 
*					current value.
*
*	INPUTS			fCurrVal - current paramaterised position on the curve
*					fTargetVal - where we want to be on the curve
*					fOffset - offset radius from the target position
*					obAvoid - point to center avoidance around
*					bXZOnly - are we testing just in the XZ plane?
*					fTolerance - Tolerance on distance testing
*
***************************************************************************************************/
float	CCurveInterface::ConvergeOnTargetAvoiding(	float fCurrVal,
													float fTargetVal,
													float fOffset,
													CPoint& obAvoid,
													bool bXZOnly,
													float fTolerance ) const
{
	ntAssert( fCurrVal >= 0.0f );
	ntAssert( fCurrVal <= 1.0f );
	ntAssert( fTargetVal >= 0.0f );
	ntAssert( fTargetVal <= 1.0f );
	ntAssert( fOffset >= 0.0f );
	
	float fMin, fMax, fMid;
	float fDMin, fDMax, fDMid;

	float fTargOffset = fOffset*fOffset;

	bool bHighMin = false;
	if (fCurrVal > fTargetVal)
		bHighMin = true;

	// crossing a boundary, reverse direction of test
	float fDiff = fabs(fTargetVal - fCurrVal);

	bool bOverBoundary = false;
	if	(
		(!GetOpen()) &&							// closed
			(
			(fDiff > 0.5f) ||					// target has crossed over
			((fCurrVal + fDiff) > 1.0f) ||		// current is about to 
			((fCurrVal - fDiff) < 0.0f)			// current is about to 
			)
		)
	{
		bOverBoundary = true;
		if (fDiff > 0.5f)
		{	
			bHighMin = !bHighMin;
			fDiff = 1.0f - fDiff;
		}
	}

	fMax = fTargetVal;
	fMin = bHighMin ? 1.0f : 0.0f;
	fDMin = bXZOnly ? GetDistSqXZ( fMin, obAvoid ) : GetDistSq( fMin, obAvoid );
	fDMax = bXZOnly ? GetDistSqXZ( fMax, obAvoid ) : GetDistSq( fMax, obAvoid );

	// need to cope with paramater wrapping on closed curves by extending the test area
	if (bOverBoundary)
		fMin = bHighMin ? (fMin + 0.5f) : (fMin - 0.5f);

	while (1)
	{
		// get mid point distance
		float fMidTest = fMid = (fMin + fMax) * 0.5f;

		if (bOverBoundary)
		{
			if (fMidTest > 1.0f)
				fMidTest -= 1.0f;
			if (fMidTest < 0.0f)
				fMidTest += 1.0f;
		}

		fDMid = bXZOnly ? GetDistSqXZ( fMidTest, obAvoid ) : GetDistSq( fMidTest, obAvoid );
		
		// quit if we're close enough to the target dist
		if (fabsf(fDMid - fTargOffset) < fTolerance)
			break;

		// quit if we've converged to the same place
		if (fabs(fMax - fMin) < EPSILON)
			break;

		// choose next convergence boundary
		if ( fDMid < fTargOffset )
			fMax = fMid;
		else
			fMin = fMid;
	}
	
	// make sure our value is ranged
	if (bOverBoundary)
	{
		if (fMid > 1.0f)
			fMid -= 1.0f;
		if (fMid < 0.0f)
			fMid += 1.0f;
	}

	return fMid;
}

/***************************************************************************************************
*
*	FUNCTION		CCurveInterface::RenderPath	
*
*	DESCRIPTION		Renders spline
*
***************************************************************************************************/
void	CCurveInterface::RenderPath( void ) const
{
	#ifndef _RELEASE

	CPoint obStart = GetPoint(0.0f);
	for (float fU = GetSampleInterval(); fU < 1.0f; fU += GetSampleInterval())
	{
		CPoint	obEnd = GetPoint(fU);

		CCamUtil::Render_Line( obStart, obEnd, fU, 1.0f, 1.0f, 1.0f );
		obStart = obEnd;
	}

	CPoint	obEnd = GetPoint(1.0f);
	CCamUtil::Render_Line( obStart, obEnd, 1.0f,1.0f,1.0f,1.0f );

	#endif
}

/***************************************************************************************************
*
*	FUNCTION		CCurveInterface::RenderTans	
*
*	DESCRIPTION		Renders spline orientations
*
***************************************************************************************************/
void	CCurveInterface::RenderTans( void ) const
{
	#ifndef _RELEASE

	for (float fU = 0.00f; fU <= 1.0f; fU += GetSampleInterval())
	{
		CPoint obStart = GetPoint(fU);
		CDirection obTangent = Get1stDerivative(fU);
		obTangent.Normalise();

		CDirection obNormal( CVecMath::GetYAxis().Cross( obTangent ) );
		obNormal.Normalise();

		CPoint obNormEnd = obStart;
		obNormEnd += obNormal;

		CCamUtil::Render_Line( obStart, obNormEnd, 1.0f, fU, 0.0f, 1.0f );
	}

	#endif
}

/***************************************************************************************************
*
*	FUNCTION		CCurveInterface::RenderCVs	
*
*	DESCRIPTION		render the control points
*
***************************************************************************************************/
void	CCurveInterface::RenderCVs( void ) const
{
	#ifndef _RELEASE

	for (u_int i = 0; i < GetNumCV()-1; i++)
	{
		CCamUtil::Render_Line( CPoint( GetCV(i) ), CPoint( GetCV(i+1) ), 1.0,0.0f,1.0f,1.0f);
	}

	#endif
}

/***************************************************************************************************
*
*	FUNCTION		CCurveInterface::Render	
*
*	DESCRIPTION		Renders spline, orientations and conytol points
*
***************************************************************************************************/
void	CCurveInterface::Render( void ) const
{
	#ifndef _RELEASE

	RenderPath();
//	RenderTans();
	RenderCVs();

	#endif
}
