//--------------------------------------------------
//!
//!	\file curvefitter.cpp
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#include "curvefitter.h"

//--------------------------------------------------
//!
//!	shifted_legendre
//! Havent got a clue. try google if your interested
//!
//--------------------------------------------------
float shifted_legendre(int n, float x)
{
	switch(n)
	{
	case 0: return 1.0f;
	case 1: return 2.0f*x - 1.0f;
	case 2: return 6.0f*x*x - 6.0f*x + 1.0f;
	case 3: return 20.0f*x*x*x - 30.0f*x*x + 12.0f*x - 1.0f;
	default: ntAssert(false); return 0.0f;
	}
}

//--------------------------------------------------
//!
//!	shifted_legendre_partial_integral
//! see above
//!
//--------------------------------------------------
template<typename T>
T shifted_legendre_partial_integral(int n, T const& A, T const& B, float x)
{
	switch(n)
	{
	case 0: return 0.5f*x*x*A + x*B;
	case 1: return (2.0f/3.0f)*x*x*x*A + 0.5f*x*x*(2.0f*B - A) - x*B;
	case 2: return (3.0f/2.0f)*x*x*x*x*A + x*x*x*(2.0f*B - 2.0f*A) + 0.5f*x*x*(A - 6.0f*B) + x*B;
	case 3: return 4.0f*x*x*x*x*x*A + 0.25f*x*x*x*x*(20.0f*B - 30.0f*A) + x*x*x*(4.0f*A - 10.0f*B) + 0.5f*x*x*(12.0f*B - A) - x*B;
	default: ntAssert(false); return T();
	}
}

//--------------------------------------------------
//!
//!	CubicCurve::ctor
//!
//--------------------------------------------------
CubicCurve::CubicCurve(const CVector* pCoeffs)
{
	NT_MEMCPY(&m_aCoeffs[0], pCoeffs, 4*sizeof(CVector)); 
}

//--------------------------------------------------
//!
//!	CubicCurve::ctor
//!
//--------------------------------------------------
CubicCurve::CubicCurve(const CubicCurve& curve)
{ 
	NT_MEMCPY(&m_aCoeffs[0], &curve.m_aCoeffs[0], 4*sizeof(CVector)); 
}

//--------------------------------------------------
//!
//!	CubicCurve::assign
//!
//--------------------------------------------------
CubicCurve& CubicCurve::operator=(const CubicCurve& curve)
{
	NT_MEMCPY(&m_aCoeffs[0], &curve.m_aCoeffs[0], 4*sizeof(CVector)); 
	return *this;
}

//--------------------------------------------------
//!
//!	CubicCurve::Evaluate
//! Evaluate using Horners rule
//!
//--------------------------------------------------
CVector CubicCurve::Evaluate(float fTime) const
{
	return m_aCoeffs[0] + fTime*(m_aCoeffs[1] + fTime*(m_aCoeffs[2] + fTime*m_aCoeffs[3]));
}

//--------------------------------------------------
//!
//!	CubicCurve::IsConstant
//!
//--------------------------------------------------
bool CubicCurve::IsConstant() const
{
	return(
		m_aCoeffs[1].X()==0.0f &&
		m_aCoeffs[1].Y()==0.0f &&
		m_aCoeffs[1].Z()==0.0f &&
		m_aCoeffs[1].W()==0.0f &&
		m_aCoeffs[2].X()==0.0f &&
		m_aCoeffs[2].Y()==0.0f &&
		m_aCoeffs[2].Z()==0.0f &&
		m_aCoeffs[2].W()==0.0f &&
		m_aCoeffs[3].X()==0.0f &&
		m_aCoeffs[3].Y()==0.0f &&
		m_aCoeffs[3].Z()==0.0f &&
		m_aCoeffs[3].W()==0.0f );
}




//--------------------------------------------------
//!
//!	CurveFitter::ctor
//!
//--------------------------------------------------
CurveFitter::CurveFitter(int iBufferLength, float fTolerance) 
  : m_iBufferLength(iBufferLength), 
	m_iNumSamples(0), 
	m_fTolerance(fTolerance), 
	m_iNumCachedSamples(0)
{
	m_pBuffer = NT_NEW_CHUNK ( Mem::MC_EFFECTS ) Sample[m_iBufferLength];
}

//--------------------------------------------------
//!
//!	CurveFitter::dtor
//!
//--------------------------------------------------
CurveFitter::~CurveFitter()
{
	NT_DELETE_CHUNK( Mem::MC_EFFECTS, m_pBuffer );
}

//--------------------------------------------------
//!
//!	CurveFitter::AddSample
//! Add a new sample to fit
//!
//--------------------------------------------------
void CurveFitter::AddSample(float fTime, const CVector& value)
{
	// add the new sample
	ntAssert(m_iNumSamples < m_iBufferLength);
	ntAssert(m_iNumSamples == 0 || m_pBuffer[m_iNumSamples - 1].fTime < fTime);
	m_pBuffer[m_iNumSamples].fTime = fTime;
	m_pBuffer[m_iNumSamples].value = value;
	++m_iNumSamples;

	// attempt to build a curve
	if(m_iNumSamples > 1)
	{
		if((!BuildCurve(m_iNumSamples) && (m_iNumSamples - m_iNumCachedSamples) > m_iSampleLookahead) 
			|| m_iNumSamples == m_iBufferLength
		)
			PopCurve();
	}
}

//--------------------------------------------------
//!
//!	CurveFitter::Clear
//!
//--------------------------------------------------
void CurveFitter::Clear()
{
	m_iNumSamples = 0;
	m_iNumCachedSamples = 0;
}

//--------------------------------------------------
//!
//!	CurveFitter::FlushBuffer
//!
//--------------------------------------------------
void CurveFitter::FlushBuffer()
{
	while(m_iNumSamples > 1)
		PopCurve();
}

//--------------------------------------------------
//!
//!	CurveFitter::BuildCurve
//!
//--------------------------------------------------
bool CurveFitter::BuildCurve(int iNumSamples)
{
	// get the start and end time
	ntAssert(iNumSamples > 1);
	const float fStartTime = m_pBuffer[0].fTime;
	const float fTimeDiff = (m_pBuffer[iNumSamples - 1].fTime - fStartTime);

	ntAssert_p( fTimeDiff > 0.0f, ("Curve must increase with time"));

	// exactly integrate this piecewise linear sample set with the legendre polynomials
	CVector aCoeffs[4];
	for(int iCoeff = 0; iCoeff < 4; ++iCoeff)
	{
		aCoeffs[iCoeff].Clear();
		for(int iSample = 1; iSample < iNumSamples; ++iSample)
		{
			// normalise the times			
			float fPrevTime = (m_pBuffer[iSample - 1].fTime - fStartTime)/fTimeDiff;
			float fTime = (m_pBuffer[iSample].fTime - fStartTime)/fTimeDiff;

            // compute the line equation for this pair
			float fPrevTimeDiff = fPrevTime - fTime;
			CVector lineGradient;

			if (fabsf(fPrevTimeDiff)>EPSILON)
				lineGradient = (m_pBuffer[iSample - 1].value - m_pBuffer[iSample].value)/fPrevTimeDiff;
			else
				lineGradient.Clear();

            CVector obLineStart = m_pBuffer[iSample - 1].value - fPrevTime*lineGradient;

			// exactly integrate this line against the polynomial for this interval
			aCoeffs[iCoeff] += (shifted_legendre_partial_integral(iCoeff, lineGradient, obLineStart, fTime)
				- shifted_legendre_partial_integral(iCoeff, lineGradient, obLineStart, fPrevTime));
		}
		aCoeffs[iCoeff] /= 1.0f/(2.0f*static_cast<float>(iCoeff) + 1.0f);
	}

	// ensure the end-points are exact
	const CVector startError = m_pBuffer[0].value - (aCoeffs[0] - aCoeffs[1] + aCoeffs[2] - aCoeffs[3]);
	const CVector endError = m_pBuffer[iNumSamples - 1].value - (aCoeffs[0] + aCoeffs[1] + aCoeffs[2] + aCoeffs[3]);

	// out of all the welding methods I think linear is probably the best
	// since it introduces the least amount of discontinuity in the curve velocity
	// (although it spreads the ntError over more samples)
//#define CUBIC_WELDING
//#define QUADRATIC_WELDING
#define LINEAR_WELDING

#ifdef CUBIC_WELDING
	aCoeffs[0] += (3.0f/4.0f)*startError;
	aCoeffs[1] -= (9.0f/20.0f)*startError;
	aCoeffs[2] -= (1.0f/4.0f)*startError;
	aCoeffs[3] -= (1.0f/20.0f)*startError;

	aCoeffs[0] += (1.0f/4.0f)*endError;
	aCoeffs[1] += (9.0f/20.0f)*endError;
	aCoeffs[2] += (1.0f/4.0f)*endError;
	aCoeffs[3] += (1.0f/20.0f)*endError;
#endif
#ifdef QUADRATIC_WELDING
	aCoeffs[0] += (2.0f/3.0f)*startError;
	aCoeffs[1] -= (1.0f/2.0f)*startError;
	aCoeffs[2] -= (1.0f/6.0f)*startError;

	aCoeffs[0] += (1.0f/3.0f)*endError;
	aCoeffs[1] += (1.0f/2.0f)*endError;
	aCoeffs[2] += (1.0f/6.0f)*endError;
#endif
#ifdef LINEAR_WELDING
	aCoeffs[0] += (1.0f/2.0f)*startError;
	aCoeffs[1] -= (1.0f/2.0f)*startError;

	aCoeffs[0] += (1.0f/2.0f)*endError;
	aCoeffs[1] += (1.0f/2.0f)*endError;
#endif

	// convert from a legendre sum to a cubic curve
	CVector aCubicCoeffs[4];
    aCubicCoeffs[0] = aCoeffs[0] - aCoeffs[1] + aCoeffs[2] - aCoeffs[3];
    aCubicCoeffs[1] = 2.0f*aCoeffs[1] - 6.0f*aCoeffs[2] + 12.0f*aCoeffs[3];
    aCubicCoeffs[2] = 6.0f*aCoeffs[2] - 30.0f*aCoeffs[3];
    aCubicCoeffs[3] = 20.0f*aCoeffs[3];
	
	// compute the ntError and cache the curve if it passes
	CubicCurve curve(&aCubicCoeffs[0]);
	float fMaxDeviation = 0.0f;
	for(int iSample = 0; iSample < iNumSamples; ++iSample)
	{
		float fTime = (m_pBuffer[iSample].fTime - fStartTime)/fTimeDiff;
		CVector deviation = curve.Evaluate(fTime) - m_pBuffer[iSample].value;
		float fDeviation = deviation.LengthSquared();
		if(fDeviation > fMaxDeviation)
			fMaxDeviation = fDeviation;
	}
	bool bPassed = (sqrtf(fMaxDeviation) < m_fTolerance);
	if(bPassed)
	{
		m_curveCache = curve;
		m_iNumCachedSamples = iNumSamples;
	}

	// check to see if a constant curve also passes
	if (memcmp( &m_pBuffer[iNumSamples - 1].value, &m_pBuffer[0].value, sizeof(float)*4 ) == 0)
	{
		fMaxDeviation = m_fTolerance*m_fTolerance;
		for(int iSample = 0; ; ++iSample)
		{
			if(iSample == iNumSamples)
			{
				// constant curve passed so override our current approximation
				CVector constantCoeffs[4];
				constantCoeffs[0] = m_pBuffer[0].value;
				constantCoeffs[1].Clear();
				constantCoeffs[2].Clear();
				constantCoeffs[3].Clear();

				m_curveCache = CubicCurve(&constantCoeffs[0]);
				m_iNumCachedSamples = iNumSamples;
				return true;
			}

			// check this sample doesn't deviate from constant too much
			CVector deviation = m_pBuffer[0].value - m_pBuffer[iSample].value;
			if(deviation.LengthSquared() > fMaxDeviation)
				break;
		}
	}
	
	// return success
	return bPassed;
}

//--------------------------------------------------
//!
//!	CurveFitter::BuildCurve
//!
//--------------------------------------------------
void CurveFitter::PopCurve()
{
	// process the curve
	ntAssert(m_iNumCachedSamples > 1);
	ProcessCurve(m_curveCache, m_pBuffer[0].fTime, m_pBuffer[m_iNumCachedSamples - 1].fTime);

	// remove the processed samples
	int iNewSampleStart = m_iNumCachedSamples - 1;
	m_iNumSamples -= iNewSampleStart;
	ntAssert(m_iNumSamples > 0);
	memmove(&m_pBuffer[0], &m_pBuffer[iNewSampleStart], m_iNumSamples*sizeof(Sample));

	// build a curve from the remaining samples
	if(m_iNumSamples > 1)
	{
		for(int iBestSampleCount = m_iNumSamples;; --iBestSampleCount)
		{
			ntAssert(iBestSampleCount > 1);
			if(BuildCurve(iBestSampleCount))
				break;
		}
	}
	else
		m_iNumCachedSamples = 1;
}
