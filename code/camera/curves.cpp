/***************************************************************************************************
*
*	$Header:: /game/curves.cpp 1     11/08/03 16:30 Wil                                            $
*
*
*
*	CHANGES
*
*	11/8/2003	Wil	Created
*
***************************************************************************************************/

#include "camera/camutils.h"
#include "camera/curves.h"


/***************************************************************************************************
*
*	FUNCTION		CRotCurveInterface::GetRotation
*
*	DESCRIPTION		Get rotation at this point in the curve
*
***************************************************************************************************/
CMatrix		CPosRotCurve::GetRotation( float fU ) const
{
	CPoint obRotations = m_pobRotCurve->GetPoint( fU );
	
	CMatrix obResult;

	switch(m_eROT)
	{
	case ROT_ORDER_XYZ: CCamUtil::MatrixFromEuler_XYZ( obResult, obRotations.X(), obRotations.Y(), obRotations.Z() ); break;
	case ROT_ORDER_ZXY: CCamUtil::MatrixFromEuler_ZXY( obResult, obRotations.X(), obRotations.Y(), obRotations.Z() ); break;
	default: ntAssert(0); obResult.SetIdentity(); break;
	}
	
	return obResult;
}

/***************************************************************************************************
*
*	FUNCTION		CRotCurveInterface::Render	
*
*	DESCRIPTION		Renders spline and orientations
*
***************************************************************************************************/
void	CPosRotCurve::Render( void ) const
{
	#ifndef _RELEASE

	RenderPath();
	RenderCVs();
	
	for (float fU = 0.0f; fU <= 1.0f; fU += GetSampleInterval())
	{
		CPoint obStart = GetPoint(fU);
		CMatrix obRotation = GetRotation(fU);

		obRotation.SetTranslation( obStart );
		CCamUtil::Render_Matrix( obRotation );
	}

	#endif
}







/***************************************************************************************************
*
*	FUNCTION		CNurbsBase
*
*	DESCRIPTION		construct normally
*
***************************************************************************************************/
void	CNurbsBase::ConstructCore( u_int uiDegree, u_int uiNumCV, const CVector* pobVerts, const float* pfKnots, bool bOwnsVerts )
{
	ntAssert( m_bInvalid );
	ntAssert( pobVerts );
	ntAssert( pfKnots );

	m_bInvalid = false;
	m_uiDegree = uiDegree;
	m_uiNumCV = uiNumCV;
	m_uiNumSpan = uiNumCV - uiDegree;
	m_uiNumKnot = GetNumControlV() + GetOrder();

	if(bOwnsVerts)
	{
		m_pobData = pobVerts;
		m_pfKnots = pfKnots;
	}
	else
	{
		CVector* pobData = NT_NEW_ARRAY_CHUNK( Mem::MC_CAMERA ) CVector[GetNumControlV()];
		ntAssert( pobData );
		NT_MEMCPY( pobData, pobVerts, sizeof(CVector)*GetNumControlV() );
		m_pobData = pobData;

		float* pfData = NT_NEW_ARRAY_CHUNK( Mem::MC_CAMERA ) float[GetNumKnot()];
		ntAssert( pfData );
		NT_MEMCPY( pfData, pfKnots, sizeof(float)*GetNumKnot() );
		m_pfKnots = pfData;
	}
}

/***************************************************************************************************
*
*	FUNCTION		CNurbsBase::FindSpan	
*
*	DESCRIPTION		Binary search for index of span bounding paramater value fU
*
***************************************************************************************************/
u_int	CNurbsBase::FindSpan( float fU ) const
{
	ntAssert( !m_bInvalid );

	if (fU <= GetMinKnot())
		return 0;
	if (fU >= GetMaxKnot())
		return GetNumSpans() - 1;

	u_int uiMin = GetDegree();
	u_int uiMax = GetNumControlV();
	u_int uiMid = (uiMin + uiMax) >> 1;

	while ( (fU < GetKnot(uiMid)) || (fU >= GetKnot(uiMid+1)) )
	{
		if (fU < GetKnot(uiMid))
			uiMax = uiMid;
		else
			uiMin = uiMid;

		uiMid = (uiMin + uiMax) >> 1;
	}

	return uiMid - GetDegree();
}

/***************************************************************************************************
*
*	FUNCTION		CNurbsBase::CoxDeBoor	
*
*	DESCRIPTION		Recursive calculation of basis functions at parameter fU
*
*	Variable definitions
*	m = maximum index of control points defining curve (P0, P1...Pm)
*	k = order of curve (degree + 1)
*	i = index of curve segment
*	t = time to evaluate curve at
*	Bi,k(t) = Basis function for a given time
*
*	Definition of a B-spline:	
*	Q(t) = SUM(i=0 to i=m) { P(i).Bi,k(t) } for ( t[k-1] <= t < t[m+1] )
*	
*	Normal Cox-de Boor algorithm for calculating basis functions:
*				
*	Bi,1(t) = {	1.0f,		if ( t[i] <= t < t[i+1] )
*				0.0f,		otherwise.
*	
*	Bi,k(t) =	( CoeffA(t) * Bi,k-1(t) ) + ( CoeffB(t) * Bi+1,k-1(t) )
*	
*	CoeffA =	  (t - t[i])      CoeffB =    (t[i+k] - t)
*				--------------		        ----------------
*				(t[i+k-1] - t[i])          (t[i+k] - t[i+1])
*
****************************************************************************************************/
float	CNurbsBase::CoxDeBoor( float fU, int iKnotIndex, int iOrder ) const
{
	if (iOrder == 1) 
		return ((GetKnot(iKnotIndex) <= fU) && (fU < GetKnot(iKnotIndex+1)) ? 1.0f : 0.0f);

	return	(
			CDBLeftTerm( fU, iKnotIndex, iOrder ) * CoxDeBoor(fU, iKnotIndex, iOrder-1) +
			CDBRightTerm( fU, iKnotIndex, iOrder ) * CoxDeBoor(fU, iKnotIndex+1, iOrder-1)
			);
};

/***************************************************************************************************
*
*	FUNCTION		CNurbsBase::CoxDeBoor1stDer	
*
*	DESCRIPTION		Recursive calculation of derivative basis functions at parameter fU
*
*	Variable definitions
*	m = maximum index of control points defining curve (P0, P1...Pm)
*	k = order of curve (degree + 1)
*	i = index of curve segment
*	t = time to evaluate curve at
*	Bi,k(t) = Basis function for a given time
*	B'i,k(t) = Derivative Basis function for a given time
*	
*	Definition of a B-spline:	
*	Q(t) = SUM(i=0 to i=m) { P(i).Bi,k(t) } for ( t[k-1] <= t < t[m+1] )
*	
*	Normal Cox-de Boor algorithm for calculating basis functions:
*				
*	B'i,1(t) = 0.0f
*	
*	B'i,k(t) =	(CoeffA(t) * B'i,k-1(t)) +
*				(CoeffA'(t) * Bi,k-1(t)) +
*				(CoeffB(t) * B'i+1,k-1(t)) +
*				(CoeffB'(t) * Bi+1,k-1(t));	
*	
*	CoeffA =	  (t - t[i])      CoeffB =    (t[i+k] - t)
*				--------------		        ----------------
*				(t[i+k-1] - t[i])          (t[i+k] - t[i+1])
*	
*	CoeffA' =	       1		  CoeffB' =       -1
*				--------------		        ----------------
*				(t[i+k-1] - t[i])          (t[i+k] - t[i+1])
*
****************************************************************************************************/
float	CNurbsBase::CoxDeBoor1stDer( float fU, int iKnotIndex, int iOrder ) const
{
	if (iOrder == 1) // terminate recursion
		return 0.0f;

	float fBasis = 0.0f;
	float fTemp = 0.0f;
	fBasis += CDBLeftTermDer( fTemp, fU, iKnotIndex, iOrder ) * CoxDeBoor1stDer(fU, iKnotIndex, iOrder-1);
	fBasis += fTemp * CoxDeBoor(fU, iKnotIndex, iOrder-1);
	fBasis += CDBRightTermDer( fTemp, fU, iKnotIndex, iOrder ) * CoxDeBoor1stDer(fU, iKnotIndex+1, iOrder-1);
	fBasis += fTemp * CoxDeBoor(fU, iKnotIndex+1, iOrder-1);
	return fBasis;
}

/***************************************************************************************************
*
*	FUNCTION		CNurbsBase::CoxDeBoor2ndDer	
*
*	DESCRIPTION		Recursive calculation of derivative basis functions at parameter fU
*
*	Variable definitions
*	m = maximum index of control points defining curve (P0, P1...Pm)
*	k = order of curve (degree + 1)
*	i = index of curve segment
*	t = time to evaluate curve at
*	Bi,k(t) = Basis function for a given time
*	B'i,k(t) = Derivative Basis function for a given time
*	
*	Definition of a B-spline:	
*	Q(t) = SUM(i=0 to i=m) { P(i).Bi,k(t) } for ( t[k-1] <= t < t[m+1] )
*	
*	Normal Cox-de Boor algorithm for calculating basis functions:
*				
*	B''i,1(t) = 0.0f
*	
*	B''i,k(t) =	(CoeffA(t) * B''i,k-1(t)) +
*				2*(CoeffA'(t) * B'i,k-1(t)) +
*				(CoeffB(t) * B''i+1,k-1(t)) +
*				2*(CoeffB'(t) * B'i+1,k-1(t));	
*	
*	CoeffA =	  (t - t[i])      CoeffB =    (t[i+k] - t)
*				--------------		        ----------------
*				(t[i+k-1] - t[i])          (t[i+k] - t[i+1])
*	
*	CoeffA' =	       1		  CoeffB' =       -1
*				--------------		        ----------------
*				(t[i+k-1] - t[i])          (t[i+k] - t[i+1])
*
****************************************************************************************************/
float	CNurbsBase::CoxDeBoor2ndDer( float fU, int iKnotIndex, int iOrder ) const
{
	if (iOrder == 1) // terminate recursion
		return 0.0f;

	float fBasis = 0.0f;
	float fTemp = 0.0f;
	fBasis += CDBLeftTermDer( fTemp, fU, iKnotIndex, iOrder ) * CoxDeBoor2ndDer(fU, iKnotIndex, iOrder-1);
	fBasis += 2.0f * fTemp * CoxDeBoor1stDer(fU, iKnotIndex, iOrder-1);
	fBasis += CDBRightTermDer( fTemp, fU, iKnotIndex, iOrder ) * CoxDeBoor2ndDer(fU, iKnotIndex+1, iOrder-1);
	fBasis += 2.0f * fTemp * CoxDeBoor1stDer(fU, iKnotIndex+1, iOrder-1);
	return fBasis;
}

/***************************************************************************************************
*
*	FUNCTION		CNurbsBase::Eval
*
*	DESCRIPTION		evaluate curve at this time
*
***************************************************************************************************/
CVector	CNurbsBase::Eval( float fU ) const
{
	ntAssert( !m_bInvalid );
	ntAssert(fU <= 1.0f);
	ntAssert(fU >= 0.0f);

	if (fU == 1.0f)
		fU -= EPSILON;

	fU = GetMinKnot() + ( fU * GetRange() );
	u_int uiSpan = FindSpan( fU );

	// we have a faster calc of the basis funcs for cubics
	CVector obResult( CONSTRUCT_CLEAR );

	if (GetDegree() == 3)
	{
		float afLeft[4], afRight[4], fBasisFuncs[4];

		fBasisFuncs[0] = 1.0f;
		for (int j=1; j<=3; j++)
		{
			afLeft[j] = fU - GetKnot( uiSpan + 4 - j );
			afRight[j] = GetKnot( uiSpan + 3 + j ) - fU;
			float fSaved = 0.0f;
			for (int r=0; r<j; r++)
			{
				float fTemp = fBasisFuncs[r]/(afRight[r+1] + afLeft[j-r]);
				fBasisFuncs[r] = fSaved + afRight[r+1]*fTemp;
				fSaved = afLeft[j-r]*fTemp;
			}
			fBasisFuncs[j] = fSaved;
		}

		for (u_int uiVert = 0; uiVert < 4; uiVert++)
			obResult += GetControlV(uiSpan+uiVert) * fBasisFuncs[uiVert];

		return obResult;
	}
	else
	{
		for (u_int uiVert = 0; uiVert < GetOrder(); uiVert++)
		{
			float fBasis = CoxDeBoor( fU, uiSpan + uiVert, GetOrder() );
			obResult += (GetControlV(uiSpan+uiVert) * fBasis);
		}

		return obResult;
	}
}

/***************************************************************************************************
*
*	FUNCTION		CNurbsBase::Eval1stDerivative
*
*	DESCRIPTION		get 1st derivative of curve at this point
*
***************************************************************************************************/
CVector	CNurbsBase::Eval1stDerivative( float fU ) const
{
	ntAssert( !m_bInvalid );
	ntAssert(fU <= 1.0f);
	ntAssert(fU >= 0.0f);

	if (fU == 1.0f)
		fU -= EPSILON;

	fU = GetMinKnot() + ( fU * GetRange() );
	u_int uiSpan = FindSpan( fU );

	CVector obResult( CONSTRUCT_CLEAR );
	for (u_int uiVert = 0; uiVert < GetOrder(); uiVert++)
	{
		float fBasis = CoxDeBoor1stDer( fU, uiSpan + uiVert, GetOrder() );
		obResult += GetControlV(uiSpan+uiVert) * fBasis;
	}

	return obResult;
}

/***************************************************************************************************
*
*	FUNCTION		CNurbsBase::Eval2ndDerivative
*
*	DESCRIPTION		get 2nd derviative of curve at this point
*
***************************************************************************************************/
CVector	CNurbsBase::Eval2ndDerivative( float fU ) const
{
	ntAssert( !m_bInvalid );
	ntAssert(fU <= 1.0f);
	ntAssert(fU >= 0.0f);

	if (fU == 1.0f)
		fU -= EPSILON;

	fU = GetMinKnot() + ( fU * GetRange() );
	u_int uiSpan = FindSpan( fU );

	CVector obResult( CONSTRUCT_CLEAR );
	for (u_int uiVert = 0; uiVert < GetOrder(); uiVert++)
	{
		float fBasis = CoxDeBoor2ndDer( fU, uiSpan + uiVert, GetOrder() );
		obResult += GetControlV(uiSpan+uiVert) * fBasis;
	}

	return obResult;
}


































/***************************************************************************************************
*
*	FUNCTION		CNurbs::CreateBezier
*
*	DESCRIPTION		Create a bezier curve from this one via the bohm algorithm
*
***************************************************************************************************/
//CCubicBezier*	CNurbs::CreateBezier( void )
//{
//	return CreateFromClamped();	
	/*
	{
		// make sure we're a uniform curve, evaluate clamping.
		// also count the number of bezier spans within this curve
		int iNumBezierSpans = 0;

		int iStartKnot = 3;
		int iEndKnot = m_uiNumKnot-3;
		
		float fKnotStep = 0.0f;
		float fFirst = GetKnot(iStartKnot);
		for (int i = (iStartKnot+1); i < iEndKnot; i++)
		{
			float fSecond = GetKnot(i);
			float fDiff = fSecond - fFirst;

			if (fDiff > EPSILON)		// is this a unique knot
			{
				if (!iNumBezierSpans)	// first unique knot? establish the knot step
					fKnotStep = fDiff;
				else
					ntAssert( fabsf(fKnotStep - fDiff) < EPSILON );
				iNumBezierSpans++;
			}
			fFirst = fSecond;
		}

		// to reduce a Nurbs to a bezier, each knot must have multiplicity 4.

		// to insert a knot at time t, defined by knots P0 ...Pm
		// where tj < t <= tj+1
		// get a new set of control verts Q0 to Qm+1
		// where	Q0 = P0
		//			Qi = (1-ai)*Pi-1 + aiPi		where 1<=i<=m
		//			Qm+1 = Pm
		//
		// and	ai = 1							if (1 <= i <= j-3 )
		//		ai = (t - ti) / (ti+3 - ti)		if (j-2 <= i <= j)
		//		ai = 0,							if (j+1 <= i <= m)
		//
		// this algorithm replaces two control points with three new ones

		// (1.0,2.0,3.0,4.0,5.0)
		// (0 0 0 0 1 2 2 2 2)
		// 1. find the first knot with multiplicity less than 4. (t = 1) (j = 3) (m = 4)
		// 2.	Q0 = P0 = 1.0f,
		//		Q1: a1 = (1-0) / (1-0) = P1 = 2.0f
		//		Q2: a2 = (1-0) / (2-0) = 0.5 P1 + 0.5 P2 = 2.5f
		//		Q3: a3 = (1-0) / (2-0) = 0.5 P2 + 0.5 P3 = 3.5f
		//		Q4: a4 = (1-1) / (2-1) = 1 P3 + 0 P4 = 4.0f
		//		Q5: = P4 = 5.0f
		// (1.0,2.0,2.5,3.5,4.0,5.0)
		// (0 0 0 0 1 1 2 2 2 2)

		// get our control points and knots in list format
		COldList<float>	m_obKnotList;
		COldList<CVector>*	pobCVList = NT_NEW COldList<CVector>;

		for (int i = 0; i < (int)m_uiNumKnot; i++)
		{
			float* pfKnot = NT_NEW float;
			*pfKnot = GetKnot(i);
			m_obKnotList.AddTail( *pfKnot );
		}

		for (u_int i = 0; i < (m_uiNumKnot-4); i++)
		{
			CVector* pobVector = NT_NEW CVector;
			*pobVector = GetCV(i);
			pobCVList->AddTail( *pobVector );
		}

	while (1)
	{
		COldListIterator<float>	obKnots(m_obKnotList);

		// Advance to the first knot that doesnt have multiplicity 4
		int		iKnotIndex = 0;
		++obKnots;

		// move past the first three unclamped knots
		while (iKnotIndex < iStartKnot)
		{
			++obKnots;
			iKnotIndex++;
		}

		// update our end knot
		iEndKnot = m_obKnotList.GetSize()-3;

		int		iUniqueKnotIndex = iKnotIndex;
		int		iMultiplicy = 1;
		float	fCurrent = *obKnots;

		while (++obKnots)
		{
			iKnotIndex++;
			
			float fDiff = *obKnots - fCurrent;
			if (fDiff > EPSILON)		// is this a unique knot
			{
				if (iMultiplicy == 4)	// we have four of these, move to next 
				{
					fCurrent = *obKnots;
					iUniqueKnotIndex = iKnotIndex;
					iMultiplicy = 1;
				}
				else
					break;				// this is the place to insert one.
			}
			else
			{
				iMultiplicy++;			// nope, move to next
			}
		}

		// check to see if we've done them all
//		if (obKnots.IsSentinel())
		if (iKnotIndex == (iEndKnot+1))
		{
			break;
		}

		int j = iUniqueKnotIndex-1; // this wont work for unclamped splines!!!!!!!
		int m = pobCVList->GetSize() - 1;

		// calculate new control vertices
		COldListIterator<CVector>	obCVs(*pobCVList);
		COldListIterator<float>	obKnots2(m_obKnotList);

		COldList<CVector>*			pobNewCVList = NT_NEW COldList<CVector>;

		for ( int i = 0; i <= (int)pobCVList->GetSize(); i++ )
		{
			// get the first valid CV and the first valid knot
			++obCVs;
			++obKnots2;

			CVector* pobVector = NT_NEW CVector;
			ntAssert(pobVector);

			if (i == 0)
			{
				*pobVector = *obCVs;
			}
			else if (i == (int)pobCVList->GetSize())
			{
				--obCVs;
				*pobVector = *obCVs;
			}
			else
			{
				float a;

				if ((1 <= i) && ( i <= (j-3)))
					a = 1.0f;
				else if (((j-2) <= i) && (i <= j))
				{
					a = (fCurrent - *obKnots2);
					
					float fTemp = m_obKnotList.GetAt(i+3) - *obKnots2;

					if (fTemp)
						a /= fTemp;
					else
						a = 0.0f;
				}
				else if (((j+1) <= i) && (i <= m))
				{
					a = 0.0f;
				}
				else
				{
					ntAssert(0);
				}

				--obCVs;
				*pobVector = *obCVs;
				*pobVector *= 1.0f - a;

				++obCVs;
				CVector obTemp = *obCVs;
				obTemp *= a;

				*pobVector += obTemp;
			}

			pobNewCVList->AddTail( *pobVector );
		}

		// swap to our new list
		pobCVList->DeleteAll( true );
		NT_DELETE( pobCVList );
		pobCVList = pobNewCVList;

		// insert the new knot
		float* pfKnot = NT_NEW float;
		*pfKnot = fCurrent;
		--obKnots;
		obKnots.Insert( *pfKnot );
	}

	int iNumControlPoints = (iNumBezierSpans * 3) + 1;

//	ntAssert( iNumControlPoints == ((int)pobCVList->GetSize() - (iNumBezierSpans - 1)) );

	CVector* pobBezierData = NT_NEW CVector[iNumControlPoints];
	ntAssert(pobBezierData)
	
	COldListIterator<CVector>	obCVs(*pobCVList);

	// skip the unwanted control points
	++obCVs;
	for (int i = 0; i < iStartKnot; i++)
	{
		++obCVs;
	}

	for (int i = 0; i < iNumControlPoints; i++)
	{
//		++obCVs;
		// skip every fourth index
		if ((i>=3) && (!((i-1) % 3)))
			++obCVs;

		pobBezierData[i] = *obCVs;
	}

	CCubicBezier* pobNewBezier = NT_NEW CCubicBezier( iNumControlPoints, pobBezierData, true );
	ntAssert(pobNewBezier);

	// clean our temp list
	pobCVList->DeleteAll( true );
	NT_DELETE( pobCVList );

	return (pobNewBezier);

	}
	*/
//}

/***************************************************************************************************
*
*	FUNCTION		CNurbs::CreateFromClamped
*
*	DESCRIPTION		Create a bezier curve from this one via the bohm algorithm
*
***************************************************************************************************/
/*
CCubicBezier*	CNurbs::CreateFromClamped( void )
{
	// make sure we're a uniform curve, evaluate clamping.
	// also count the number of bezier spans within this curve
	int iNumBezierSpans = 0;

	int iStartKnot = 0;
//	int iStartKnot = 3;

	int iEndKnot = m_uiNumKnot;
//	int iEndKnot = m_uiNumKnot-3;
	
	float fKnotStep = 0.0f;
	float fFirst = GetKnot(iStartKnot);
	for (int i = (iStartKnot+1); i < iEndKnot; i++)
	{
		float fSecond = GetKnot(i);
		float fDiff = fSecond - fFirst;

		if (fDiff > EPSILON)		// is this a unique knot
		{
			if (!iNumBezierSpans)	// first unique knot? establish the knot step
				fKnotStep = fDiff;
			else
				ntAssert( fabsf(fKnotStep - fDiff) < EPSILON );
			iNumBezierSpans++;
		}
		fFirst = fSecond;
	}

	// to reduce a Nurbs to a bezier, each knot must have multiplicity 4.

	// to insert a knot at time t, defined by knots P0 ...Pm
	// where tj < t <= tj+1
	// get a new set of control verts Q0 to Qm+1
	// where	Q0 = P0
	//			Qi = (1-ai)*Pi-1 + aiPi		where 1<=i<=m
	//			Qm+1 = Pm
	//
	// and	ai = 1							if (1 <= i <= j-3 )
	//		ai = (t - ti) / (ti+3 - ti)		if (j-2 <= i <= j)
	//		ai = 0,							if (j+1 <= i <= m)
	//
	// this algorithm replaces two control points with three new ones

	// (1.0,2.0,3.0,4.0,5.0)
	// (0 0 0 0 1 2 2 2 2)
	// 1. find the first knot with multiplicity less than 4. (t = 1) (j = 3) (m = 4)
	// 2.	Q0 = P0 = 1.0f,
	//		Q1: a1 = (1-0) / (1-0) = P1 = 2.0f
	//		Q2: a2 = (1-0) / (2-0) = 0.5 P1 + 0.5 P2 = 2.5f
	//		Q3: a3 = (1-0) / (2-0) = 0.5 P2 + 0.5 P3 = 3.5f
	//		Q4: a4 = (1-1) / (2-1) = 1 P3 + 0 P4 = 4.0f
	//		Q5: = P4 = 5.0f
	// (1.0,2.0,2.5,3.5,4.0,5.0)
	// (0 0 0 0 1 1 2 2 2 2)

	// get our control points and knots in list format
	COldList<float>	m_obKnotList;
	COldList<CVector>*	pobCVList = NT_NEW COldList<CVector>;

	for (int i = iStartKnot; i < iEndKnot; i++)
	{
		float* pfKnot = NT_NEW float;
		*pfKnot = GetKnot(i);
		m_obKnotList.AddTail( *pfKnot );
	}

	for (u_int i = 0; i < (m_uiNumKnot-4); i++)
	{
		CVector* pobVector = NT_NEW CVector;
		*pobVector = GetCV(i);
		pobCVList->AddTail( *pobVector );
	}

	while (1)
	{
		COldListIterator<float>	obKnots(m_obKnotList);

		// Advance to the first knot that doesnt have multiplicity 4
		int		iUniqueKnotIndex = 0;
		int		iKnotIndex = 0;
		++obKnots;

		int		iMultiplicy = 1;
		float	fCurrent = *obKnots;

		while (++obKnots)
		{
			iKnotIndex++;
			
			float fDiff = *obKnots - fCurrent;
			if (fDiff > EPSILON)		// is this a unique knot
			{
				if (iMultiplicy == 4)	// we have four of these, move to next 
				{
					fCurrent = *obKnots;
					iUniqueKnotIndex = iKnotIndex;
					iMultiplicy = 1;
				}
				else
					break;				// this is the place to insert one.
			}
			else
			{
				iMultiplicy++;			// nope, move to next
			}
		}

		// check to see if we've done them all
		if (obKnots.IsSentinel())
		{
			if (iMultiplicy == 4)
				break;
		}

		int j = iUniqueKnotIndex-1; // this wont work for unclamped splines!!!!!!!
		int m = pobCVList->GetSize() - 1;

		// calculate new control vertices
		COldListIterator<CVector>	obCVs(*pobCVList);
		COldListIterator<float>	obKnots2(m_obKnotList);

		COldList<CVector>*			pobNewCVList = NT_NEW COldList<CVector>;

		for ( int i = 0; i <= (int)pobCVList->GetSize(); i++ )
		{
			// get the first valid CV and the first valid knot
			++obCVs;
			++obKnots2;

			CVector* pobVector = NT_NEW CVector;
			ntAssert(pobVector);

			if (i == 0)
			{
				*pobVector = *obCVs;
			}
			else if (i == (int)pobCVList->GetSize())
			{
				--obCVs;
				*pobVector = *obCVs;
			}
			else
			{
				float a;

				if ((1 <= i) && ( i <= (j-3)))
					a = 1.0f;
				else if (((j-2) <= i) && (i <= j))
				{
					a = (fCurrent - *obKnots2);
					
					float fTemp = m_obKnotList.GetAt(i+3) - *obKnots2;

					if (fTemp)
						a /= fTemp;
					else
						a = 0.0f;
				}
				else if (((j+1) <= i) && (i <= m))
				{
					a = 0.0f;
				}
				else
				{
					ntAssert(0);
					a = 0.0f;
				}

				--obCVs;
				*pobVector = *obCVs;
				*pobVector *= 1.0f - a;

				++obCVs;
				CVector obTemp = *obCVs;
				obTemp *= a;

				*pobVector += obTemp;
			}

			pobNewCVList->AddTail( *pobVector );
		}

		// swap to our new list
		pobCVList->DeleteAll( true );
		NT_DELETE( pobCVList );
		pobCVList = pobNewCVList;

		// insert the new knot
		float* pfKnot = NT_NEW float;
		*pfKnot = fCurrent;
		--obKnots;
		obKnots.Insert( *pfKnot );

	}

	int iNumControlPoints = (iNumBezierSpans * 3) + 1;

	ntAssert( iNumControlPoints == ((int)pobCVList->GetSize() - (iNumBezierSpans - 1)) );

	CVector* pobBezierData = NT_NEW CVector[iNumControlPoints];
	ntAssert(pobBezierData);
	
	COldListIterator<CVector>	obCVs(*pobCVList);

	for (int i = 0; i < iNumControlPoints; i++)
	{
		++obCVs;

		// skip every fourth index
		if ((i>=3) && (!((i-1) % 3)))
			++obCVs;

		pobBezierData[i] = *obCVs;
	}

	CCubicBezier* pobNewBezier = NT_NEW CCubicBezier( iNumControlPoints, pobBezierData, true );
	ntAssert(pobNewBezier);

	// clean our temp list
	pobCVList->DeleteAll( true );
	NT_DELETE( pobCVList );

	return (pobNewBezier);
}

*/
