//------------------------------------------------------------------------------------------
//!
//!	\file TimeScalarCurve.h
//!
//------------------------------------------------------------------------------------------

#ifndef _TIMESCALARCURVE_INC
#define _TIMESCALARCURVE_INC

//------------------------------------------------------------------------------------------
// Includes for Inheritance
//------------------------------------------------------------------------------------------

// TSC Curve editing.  TODO: Change to the new mouse driven curve editor
#ifndef _RELEASE
#define _CAM_CURVE_DEBUG
#endif

#include "gfx/graphing.h"
#include "editable/enumlist.h"

class TimeScalarCurve;

//------------------------------------------------------------------------------------------
//!
//!	TimeScalarCurvePoint
//!	Describes a point on the curve. Added through Welder. Because I can't be bothered to 
//! sort the list every frame, they should be added in order.
//!
//------------------------------------------------------------------------------------------
class TimeScalarCurvePoint
{
	HAS_INTERFACE(TimeScalarCurvePoint);
public:
	TimeScalarCurvePoint() :
		m_fTime( 1.0f ),
		m_fScalar( 1.0f ),
		m_eInterpolationMethod( TSCIM_STEP ),
		m_pobMyTimeScalarCurve( 0 )
		{};
	~TimeScalarCurvePoint() {};

	float m_fTime;
	float m_fScalar;
	TIME_SCALAR_CURVE_INTERPOLATION_METHOD m_eInterpolationMethod;

	TimeScalarCurve* m_pobMyTimeScalarCurve; // Pointer to the TSC I'm in

	bool EditorChangeValue(CallBackParameter, CallBackParameter) { return true; }
	#ifdef _CAM_CURVE_DEBUG
	void EditorSelect( bool bSelected );
	bool IsSelected() const;
	#endif // _CAM_CURVE_DEBUG
};

//------------------------------------------------------------------------------------------
//!
//!	TimeScalarCurve
//!	Describes time dilation for cool cameras.
//! This is the new version...
//!
//------------------------------------------------------------------------------------------
class TimeScalarCurve
{
public:
	TimeScalarCurve();
	~TimeScalarCurve()
	{
		#ifdef _CAM_CURVE_DEBUG
		if (m_pobGraph)
		{
			NT_DELETE_CHUNK(Mem::MC_CAMERA, m_pobGraph );
		}
		#endif
	}

	float GetScalar(float ft);

// Data Interface Callbacks
protected:
	bool EditorChangeValue(CallBackParameter, CallBackParameter) { return true; }

private:
	int m_iGraphSamples;
	TIME_SCALAR_CURVE_INTERPOLATION_METHOD m_eFinalPointInterpolationMethod;
	float m_fLastX, m_fLastY;
	bool m_bIsRenderingCurve;

// Data Interface Exposed Attributes
private:
	typedef ntstd::List<TimeScalarCurvePoint*,Mem::MC_CAMERA> PointList;
	PointList m_obCurvePoints;
	TimeScalarCurvePoint m_obStartReferencePoint, m_obEndReferencePoint;
	float m_fStartCurveRenderTime, m_fEndCurveRenderTime;

	bool  m_bAllowEarlyExit;
	friend class TimeScalarCurveInterface;

// Statics - Maybe Exposed Later
private:
	static const float m_fMinScalar;
	static const float m_fMaxScalar;

// TSC Debug / Visualisation
//--------------------------------------
#ifdef _CAM_CURVE_DEBUG
public:
	void RenderTimeCurve();
	
	void EditorSelect(bool bSelected);
	bool         IsSelected() const {return m_bSelected;}

private:
	mutable class CGraphSampleSet	*m_pobSamples;  // Visualising
	CGraph					*m_pobGraph;
	bool                             m_bSelected;
#endif
};

#endif //_TIMESCALARCURVE_INC
