//--------------------------------------------------
//!
//!	\file curvefitter.h
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#ifndef _CURVE_FITTER_H
#define _CURVE_FITTER_H

//--------------------------------------------------
//!
//!	CubicCurve
//!	Represents a cubic curve. The coefficients are 
//! stored so that the curve can be evaluated as 
//! ((c3*t + c2)*t + c1)*t + c0.
//!
//--------------------------------------------------
class CubicCurve
{
public:
	CubicCurve() {}
	
	explicit CubicCurve(const CVector* pCoeffs);

	CubicCurve(const CubicCurve& curve);
	CubicCurve& operator=(const CubicCurve& curve);

	CVector Evaluate(float fTime) const;

	CVector const& GetStart() const { return m_aCoeffs[0]; }
	CVector GetEnd() const { return m_aCoeffs[0] + m_aCoeffs[1] + m_aCoeffs[2] + m_aCoeffs[3]; }

	//! returns true if this curve is a constant value
	bool IsConstant() const;

	//! Allows access to the curve coefficients. 
	/*! \param iIndex	The coefficient index from 0 to 3 inclusive.
	*/
	CVector const& operator[](int iIndex) const { return m_aCoeffs[iIndex]; }

private:
	CVector m_aCoeffs[4];
};

//--------------------------------------------------
//!
//!	CurveFitter
//! Dynamically builds a spline as samples are added. Subclasses must implement
//! ProcessCurve, which is called when the curve fitter decides to replace a subset
//! of the current samples with a cubic curve.
//!
//! A curve is output when either a maximal curve is found for the current 
//! tolerance, the buffer becomes full, or the buffer is flushed.
//!
//--------------------------------------------------
class CurveFitter
{
public:
	//! Creates a new curve fitter.
	/*! \param iBufferLength	The size of the internal buffer, which also defines
								the maximum number of samples that can be approximated in one curve section.
		\param fTolerance		The maximum allowable deviation of the curve from the sample points.
	*/
	CurveFitter(int iBufferLength, float fTolerance);

	//! Destroy the curve fitter.
	virtual ~CurveFitter();

	//! This clears the buffer of ALL samples, unprocessed samples are lost.
	void Clear();

	//! Samples must be added in time order.
	void AddSample(float fTime, const CVector& value);

	//! This forces all buffered samples to be processed.
	/*! Warning: this will NOT clear the buffer; the last sample will remain at the start. This is
		to allow the curve to be continued if necessary. To start a completely new curve after 
		flushing the old one call Clear after FlushBuffer.
	*/
	void FlushBuffer();

protected:
	//! Must be implemented by a subclass.
	virtual void ProcessCurve(const CubicCurve& curve, float fStartTime, float fEndTime) = 0;

private:
	//! Processes the current samples into a new curve.
	bool BuildCurve(int iNumSamples);

	//! Forces the current curve to get processed, and starts a new curve on the remaining samples.
	void PopCurve();

	//! A simple structure for storing time/sample pairs.
	struct Sample
	{
		float fTime;
		CVector value;
	};

	Sample* m_pBuffer;			//!< The sample buffer.
	int m_iBufferLength;		//!< The capacity of the sample buffer.
	int m_iNumSamples;			//!< The number of current samples.
	
	float m_fTolerance;			//!< The ntError tolerance.
	int m_iSampleLookahead;		//!< The number of samples to look ahead at before giving up.

	CubicCurve m_curveCache;	//!< The currently cached curve.
	int m_iNumCachedSamples;	//!< The number of samples this curve approximates.
};

#endif // _CURVE_FITTER_H
