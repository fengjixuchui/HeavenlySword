//--------------------------------------------------
//!
//!	\file functioncurve.h
//!	XML object that represents an editable 1Dimensional
//! n-segment curve, with debug display 
//!
//--------------------------------------------------

#ifndef _FUNCTION_CURVE_H
#define _FUNCTION_CURVE_H

#include "effect/graphediting.h"

class FunctionGraph;

//--------------------------------------------------
//!
//!	FCurveNode_Hermite
//!
//--------------------------------------------------
class FCurveNode_Hermite
{
public:
	FCurveNode_Hermite();
	virtual ~FCurveNode_Hermite(){};
	virtual void PostConstruct( void );

	float		m_fStart;
	float		m_fOut;
	float		m_fIn;
	float		m_fTime;
};

//--------------------------------------------------
//!
//!	FunctionCurve_User
//!
//--------------------------------------------------
class FunctionCurve_User
{
public:
	FunctionCurve_User();
	virtual ~FunctionCurve_User();
	
	virtual void PostConstruct( void );
	virtual bool EditorChangeValue( CallBackParameter, CallBackParameter );
	virtual void EditorChangeParent();
	virtual void DebugRender( void );
	
	// if we're being edited the buffer curve will work for us instead...
	const CTimeCurveInterface*	GetCurveInterface() const
	{
#ifdef _RELEASE
		return &m_pFittedCurve->GetCurve();
#else
		if (m_pFittedCurve->IsValid())
			return &m_pFittedCurve->GetCurve();
		else if (m_pBufferCurve)
			return m_pBufferCurve;

		ntAssert_p(0,("Curve invalid"));
		return NULL;
#endif
	}

	const FunctionCurve_Fitted* GetFittedCurve() const { return m_pFittedCurve; }

	float EvaluateScaledAndOffset( float fU ) const
	{
		return (GetCurveInterface()->Evaluate( fU ).X() * m_fScale) + m_fOffset;
	}

	float Get1stDerivativeScaled( float fU ) const
	{
		return (GetCurveInterface()->Get1stDerivative( fU ).X() * m_fScale);
	}

	int									m_iNumSegments;
	float								m_fTolerance;
	float								m_fScale;
	float								m_fOffset;
	ntstd::List<FCurveNode_Hermite*>	m_obCurveNodes;
	CHashedString							m_obFunctionScript;

	bool	DetectCurveChanged() const
	{
		// m_bCurveChanged may be changed as a consequence of curve editing
		ntAssert(m_pFittedCurve);
		if ( (m_pFittedCurve->IsValid()) && (m_pFittedCurve->DetectFinalise()) )
			m_bCurveChanged = true;

		bool bResult = m_bCurveChanged;
		m_bCurveChanged = false;

		return bResult;
	}

	void RebuildCurve();
	void ForceCurve( CCubicTimeHermite* pForcedCurve );
	void InitialiseToArrayValues( int iNumEntries, const float* pfValues, const float* pfTimes );

private:
	class CComparator_FCurveNode_LT
	{
	public:
		bool operator()( const FCurveNode_Hermite* pFirst, const FCurveNode_Hermite* pSecond ) const
		{
			return pFirst->m_fTime < pSecond->m_fTime;
		}
	};

	void DebugRenderCurve( FunctionGraph& graph, float fMin, float fRange );
	void DebugRenderGradients( FunctionGraph& graph, float fMin, float fRange );

	void FinaliseCurve();
	void CheckForScriptFunction();
	void ConvertMouseToInputSpace( float& fX, float& fY );

	void ResetXMLData();

	FunctionCurve_Fitted*	m_pFittedCurve;
	CCubicTimeHermite*		m_pBufferCurve;
	float					m_fLastTime;
	bool					m_bScriptChecked;
	bool					m_bFirstTime;
	mutable bool			m_bCurveChanged;
	time_t					m_scriptModDate;
};

#endif //_FUNCTION_CURVE_H
