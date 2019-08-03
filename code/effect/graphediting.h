//--------------------------------------------------
//!
//!	\file graphediting.h
//!	Marrys our curve fitting algorithm to our
//! CCubicTimeHermite class
//!
//--------------------------------------------------

#ifndef _GRAPH_EDITING_H
#define _GRAPH_EDITING_H

#include "effect/curvefitter.h"
#include "camera/curves.h"

//--------------------------------------------------
//!
//!	FunctionCurveFitter
//! Helper class to fit curves to mouse data
//!
//--------------------------------------------------
class FunctionCurveFitter : public CurveFitter
{
public:
	FunctionCurveFitter( CCubicTimeHermite** ppHermite,
						 float fTolerance) :
		CurveFitter( 60*10, fTolerance ),
		m_ppHermite(ppHermite)
	{}

protected:
	virtual void ProcessCurve( const CubicCurve& curve, float fStartTime, float fEndTime);
	CCubicTimeHermite**	m_ppHermite; 			//!< allocated in Mem::MC_EFFECTS
};

//--------------------------------------------------
//!
//!	FunctionCurve_Fitted
//!
//! Object used to generate a fitted curve from a
//! collection of float nodes at specified times
//!
//! Object also represents that 1 dimesional curve
//! once it has been finalised.
//!
//--------------------------------------------------
class FunctionCurve_Fitted
{
public:
	FunctionCurve_Fitted( float fTolerance = 0.01f ) :
		m_pHermite(0),
		m_pFitter(0),
		m_bIsValid(false),
		m_bFinaliseDetected(false),
		m_bRangesCalculated(false),
		m_bIntegralCalculated(false),
		m_fLastTime(-1.0f),
		m_fTolerance(fTolerance)
	{}

	~FunctionCurve_Fitted()
	{
		Reset();
	}

	CCubicTimeHermite*	Reset( bool bReturnNotDelete = false );
	void	AddSampleToFit( float fVal, float fTime );
	bool	Finalise( CCubicTimeHermite* pCurve = 0 );
	
	float	Evaluate( float fU ) const
	{
		ntAssert( m_bIsValid );
		return m_pHermite->GetPoint( fU ).X();
	}

	float	Get1stDerivative( float fU ) const
	{
		ntAssert( m_bIsValid );
		return m_pHermite->Get1stDerivative( fU ).X();
	}
	
	void	DebugRenderSamples( float fminX, float fmaxX,
								float fminY, float fmaxY );

	const CCubicTimeHermite& GetCurve() const
	{
		ntAssert( m_bIsValid );
		return *m_pHermite;
	}

	bool	IsValid() const { return m_bIsValid; }
	bool	DetectFinalise() const { ntAssert(IsValid()); bool bRes = m_bFinaliseDetected; m_bFinaliseDetected=false; return bRes; }

	// must have called CalcFunctionIntegral() before calling this
	float	GetFunctionIntegral() const { ntAssert(IsValid()); ntAssert(m_bIntegralCalculated); return m_fIntegral; }
	inline	void CalcFunctionIntegral();

	// these are for debug rendering only, so theyre not particluarly fast the first time theyre called
	float	GetFunctionMin() const { ntAssert(IsValid()); CheckRangesValid(); return m_fMinVal; }
	float	GetFunctionMax() const { ntAssert(IsValid()); CheckRangesValid(); return m_fMaxVal; }
	
private:

	inline void CheckRangesValid() const;

	CCubicTimeHermite*		m_pHermite;					//!< allocated in MC_EFFECTS
	FunctionCurveFitter*	m_pFitter; 					//!< allocated in MC_EFFECTS
	ntstd::List<CVector*, Mem::MC_EFFECTS>	m_samples;		//!< the vectors are allocated in MC_EFFECTS too
	bool					m_bIsValid;

	mutable bool			m_bFinaliseDetected;
	mutable bool			m_bRangesCalculated;
	bool					m_bIntegralCalculated;

	float					m_fLastTime;
	float					m_fTolerance;
	
	mutable float			m_fMinVal, m_fMaxVal;
	float					m_fIntegral;
};

//-------------------------------------------------------
//!
//!	Calc value range
//!
//-------------------------------------------------------
inline void FunctionCurve_Fitted::CheckRangesValid() const
{
	if (!m_bRangesCalculated)
	{
		m_fMinVal = MAX_POS_FLOAT;
		m_fMaxVal = -MAX_POS_FLOAT;

		int iNumSamples = m_pHermite->GetNumSpan() * 100;
		float fStep = 1.0f / iNumSamples;

		for ( int i = 0; i <= iNumSamples; i++ )
		{
			float fU = i * fStep;
			float fCurr = m_pHermite->GetPoint(fU).X();

			if (fCurr < m_fMinVal)
				m_fMinVal = fCurr;

			if (fCurr > m_fMaxVal)
				m_fMaxVal = fCurr;
		}

		m_bRangesCalculated = true;
	}
}

//-------------------------------------------------------
//!
//!	Calc area under the function curve
//!
//-------------------------------------------------------
inline void FunctionCurve_Fitted::CalcFunctionIntegral()
{
	ntAssert( IsValid() );
	ntAssert( !m_bIntegralCalculated );

	m_fIntegral = 0.0f;

	for (u_int i = 0 ; i < m_pHermite->GetNumSpan(); i++)
	{
		CVector	coeffs[4];
		m_pHermite->GetCoeffs( i, coeffs );

		float fSegement =	(coeffs[0].X() * 0.25f) +
							(coeffs[1].X() * (1.0f/3.0f)) + 
							(coeffs[2].X() * 0.5f) + 
							coeffs[3].X();

		float fFraction =	(m_pHermite->GetTimeModule().GetTime(i+1) -
							m_pHermite->GetTimeModule().GetTime(i)) /
							m_pHermite->GetTimeModule().GetRange();

		m_fIntegral +=	fSegement * fFraction;
	}

	m_bIntegralCalculated = true;
}

//--------------------------------------------------
//!
//!	TimeCurve_Fitted
//! Object used to generate a fitted curve from a
//! collection of CVector nodes at specified times
//! NB, implements less functionality than above class
//!
//--------------------------------------------------
class TimeCurve_Fitted
{
public:
	TimeCurve_Fitted( float fTolerance = 0.01f ) :
		m_pHermite(0),
		m_pFitter(0),
		m_bIsValid(false),
		m_bFinaliseDetected(false),
		m_fLastTime(-1.0f),
		m_fTolerance(fTolerance)
	{}

	~TimeCurve_Fitted()
	{
		Reset();
	}

	CCubicTimeHermite*	Reset( bool bReturnNotDelete = false );
	void	AddSampleToFit( const CVector& sample, float fTime );
	bool	Finalise( CCubicTimeHermite* pCurve = 0 );

	const CCubicTimeHermite& GetCurve() const { ntAssert( m_bIsValid ); return *m_pHermite; }

	bool	IsValid() const { return m_bIsValid; }
	bool	DetectFinalise() const { ntAssert(IsValid()); bool bRes = m_bFinaliseDetected; m_bFinaliseDetected=false; return bRes; }
	
private:

	CCubicTimeHermite*		m_pHermite;
	FunctionCurveFitter*	m_pFitter;
	bool					m_bIsValid;

	mutable bool			m_bFinaliseDetected;

	float					m_fLastTime;
	float					m_fTolerance;
};

#endif // _GRAPH_EDITING_H

