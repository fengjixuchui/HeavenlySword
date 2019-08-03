//--------------------------------------------------
//!
//!	\file graphediting.cpp
//!	Marrys our curve fitting algorithm to our
//! CCubicTimeHermite class
//!
//--------------------------------------------------

#include "graphediting.h"
#include "functiongraph.h"

//--------------------------------------------------
//!
//!	ProcessCurve
//! called during curve fitting, handles a new segment
//! being spat out by converting it to a hermite
//! segment and rebuilding the hermite curve.
//!
//--------------------------------------------------
void FunctionCurveFitter::ProcessCurve( const CubicCurve& curve, float fStartTime, float fEndTime)
{
	CCubicTimeHermite*	pHermite = *m_ppHermite;

	int iNumSegments = (pHermite) ? pHermite->GetNumSpan() + 1 : 1;

	float*		pTimes = NT_NEW float[ iNumSegments + 1 ];
	CVector*	pVerts = NT_NEW CVector[ iNumSegments + 1 ];

	CVector*	pStarts = NT_NEW CVector[ iNumSegments ];
	CVector*	pEnds = NT_NEW CVector[ iNumSegments ];

	if (pHermite)
	{
		for (int i = 0; i < iNumSegments; i++)
		{
			pTimes[i] = pHermite->GetTimeModule().GetTime(i);
			pVerts[i] = pHermite->GetCV(i);

			if (i<(iNumSegments-1))
			{
				pStarts[i] = pHermite->GetStartTan(i);
				pEnds[i] = pHermite->GetEndTan(i);
			}
		}

		NT_DELETE_CHUNK( Mem::MC_EFFECTS, pHermite );
	}

	static const float s_afPoly2Hermite[4][4] = 
	{
		{	0.0f,	0.0f,		0.0f,		1.0f },
		{	1.0f,	1.0f,		1.0f,		1.0f },
		{	0.0f,	0.0f,		1.0f/3.0f,	0.0f },
		{	1.0f,	2.0f/3.0f,  1.0f/3.0f,	0.0f }
	};

	CVector	aCoeffs[4];
	CVector	aModCoeffs[4];

	aCoeffs[0] = curve[3];
	aCoeffs[1] = curve[2];
	aCoeffs[2] = curve[1];
	aCoeffs[3] = curve[0];

	for (int iIndex = 0; iIndex < 4; iIndex++)
	{
		aModCoeffs[iIndex].Clear();

		for (int j = 0; j < 4; j++)
			aModCoeffs[iIndex] += aCoeffs[j] * s_afPoly2Hermite[iIndex][j];
	}

	if (iNumSegments==1)
	{
		pVerts[ iNumSegments - 1 ] = aModCoeffs[0];
		pTimes[ iNumSegments - 1 ] = fStartTime;
	}

	pVerts[ iNumSegments ] = aModCoeffs[1];
	pTimes[ iNumSegments ] = fEndTime;	

	pStarts[ iNumSegments -1 ] = aModCoeffs[2];
	pEnds[ iNumSegments -1 ] = aModCoeffs[3];

	*m_ppHermite = NT_NEW_CHUNK ( Mem::MC_EFFECTS ) CCubicTimeHermite( pTimes, true, true,  iNumSegments + 1, pVerts, pStarts, pEnds, true, true );
}





//--------------------------------------------------
//!
//!	Reset
//! Dump existing curve and set to an invalid state
//!
//--------------------------------------------------
CCubicTimeHermite* FunctionCurve_Fitted::Reset( bool bReturnNotDelete )
{
	CCubicTimeHermite* pReturn = 0;

	if ( bReturnNotDelete )
	{
		ntError_p( m_bIsValid, ("Curve must be valid for this to work") );
		ntError_p( m_pHermite, ("Curve must be valid for this to work") );

		pReturn = m_pHermite;
		m_pHermite = 0;
	}
	else
	{
		if ( m_pHermite )
		{
			NT_DELETE_CHUNK( Mem::MC_EFFECTS, m_pHermite );
			m_pHermite = 0;
		}
	}

	if ( m_pFitter )
	{
		NT_DELETE_CHUNK( Mem::MC_EFFECTS, m_pFitter );
		m_pFitter = 0;
	}

	while ( !m_samples.empty() )
	{
		NT_DELETE_CHUNK( Mem::MC_EFFECTS, m_samples.back() );
		m_samples.pop_back();
	}

	m_bIsValid = false;
	m_bFinaliseDetected = false;
	m_bRangesCalculated = false;
	m_bIntegralCalculated = false;
	m_fLastTime = -1.0f;

	return pReturn;
}

//--------------------------------------------------
//!
//!	Spit out another value in the function	
//!
//--------------------------------------------------
void FunctionCurve_Fitted::AddSampleToFit( float fVal, float fTime )
{
	ntError_p( !m_bIsValid, ("Editing a finalised curve. Call Reset() first.") );
	ntError_p( fTime > m_fLastTime, ("Samples must increase in time") );
	m_fLastTime = fTime;

	if (!m_pFitter)
		m_pFitter = NT_NEW_CHUNK ( Mem::MC_EFFECTS ) FunctionCurveFitter( &m_pHermite, m_fTolerance );

	m_pFitter->AddSample( fTime, CVector( fVal, 0.0f, 0.0f, 0.0f ) );
	m_samples.push_back( NT_NEW_CHUNK ( Mem::MC_EFFECTS ) CVector( fTime, fVal, 0.0f, 0.0f ) );
}

//-------------------------------------------------------
//!
//!	Finish the curve, get some bounds for normalised eval
//! \return true if we're now valid, false if not
//!
//-------------------------------------------------------
bool FunctionCurve_Fitted::Finalise( CCubicTimeHermite* pCurve )
{
	ntError_p( !m_bIsValid, ("Finalising a finalised curve.") );

	if (pCurve)
	{
		m_pHermite = pCurve;
	}
	else
	{
		if (!m_pFitter)
		{
			ntPrintf( "ATTN!!! Curve creation failed. Did you add more than 1 sample?\n");
			return false;
		}

		m_pFitter->FlushBuffer();

		if (!m_pHermite)
		{
			ntPrintf( "ATTN!!! Curve creation failed. Did you add more than 1 sample?\n");
			return false;
		}
	}

	m_bIsValid = true;
	m_bFinaliseDetected = true;

	return true;
}

//-------------------------------------------------------
//!
//!	Debug display of the curve.
//!
//-------------------------------------------------------
void FunctionCurve_Fitted::DebugRenderSamples( float fminX, float fmaxX, float fminY, float fmaxY )
{
#ifndef _GOLD_MASTER
	ntError( !m_bIsValid );

	FunctionGraph graph(fminX, fmaxX, fminY, fmaxY);
	graph.DrawBounds(0x800000ff);

	if (!m_samples.empty())
	{
		ntstd::List<CVector*, Mem::MC_EFFECTS>::iterator curr = m_samples.begin();
		ntstd::List<CVector*, Mem::MC_EFFECTS>::iterator next = curr;
		++next;

		for ( ; curr != m_samples.end() && next != m_samples.end(); ++curr, ++next )
		{
			graph.DrawLine( CPoint(**curr), CPoint(**next), 0xffff0000 );
		}
	}
#endif
}





//--------------------------------------------------
//!
//!	Reset
//! Dump existing curve and set to an invalid state
//!
//--------------------------------------------------
CCubicTimeHermite* TimeCurve_Fitted::Reset( bool bReturnNotDelete )
{
	CCubicTimeHermite* pReturn = 0;

	if ( bReturnNotDelete )
	{
		ntError_p( m_bIsValid, ("Curve must be valid for this to work") );
		ntError_p( m_pHermite, ("Curve must be valid for this to work") );

		pReturn = m_pHermite;
		m_pHermite = 0;
	}
	else
	{
		if ( m_pHermite )
		{
			NT_DELETE_CHUNK( Mem::MC_EFFECTS, m_pHermite );
			m_pHermite = 0;
		}
	}

	if ( m_pFitter )
	{
		NT_DELETE_CHUNK( Mem::MC_EFFECTS, m_pFitter );
		m_pFitter = 0;
	}

	m_bIsValid = false;
	m_bFinaliseDetected = false;
	m_fLastTime = -1.0f;

	return pReturn;
}

//--------------------------------------------------
//!
//!	Spit out another value in the function	
//!
//--------------------------------------------------
void TimeCurve_Fitted::AddSampleToFit( const CVector& sample, float fTime )
{
	ntError_p( !m_bIsValid, ("Editing a finalised curve. Call Reset() first.") );
	ntError_p( fTime > m_fLastTime, ("Samples must increase in time") );
	m_fLastTime = fTime;

	if (!m_pFitter)
		m_pFitter = NT_NEW_CHUNK ( Mem::MC_EFFECTS ) FunctionCurveFitter( &m_pHermite, m_fTolerance );

	m_pFitter->AddSample( fTime, sample );
}

//-------------------------------------------------------
//!
//!	Finish the curve, get some bounds for normalised eval
//! \return true if we're now valid, false if not
//!
//-------------------------------------------------------
bool TimeCurve_Fitted::Finalise( CCubicTimeHermite* pCurve )
{
	ntError_p( !m_bIsValid, ("Finalising a finalised curve.") );

	if (pCurve)
	{
		m_pHermite = pCurve;
	}
	else
	{
		if (!m_pFitter)
		{
			ntPrintf( "ATTN!!! Curve creation failed. Did you add more than 1 sample?\n");
			return false;
		}

		m_pFitter->FlushBuffer();

		if (!m_pHermite)
		{
			ntPrintf( "ATTN!!! Curve creation failed. Did you add more than 1 sample?\n");
			return false;
		}
	}

	m_bIsValid = true;
	m_bFinaliseDetected = true;

	return true;
}

