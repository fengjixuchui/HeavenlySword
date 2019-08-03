//------------------------------------------------------------------------------------------
//!
//!	\file TimeScalarCurve.cpp
//!
//------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------------------
#include "camera/timescalarcurve.h"
#include "camera/camman.h"
#include "camera/camtools.h"

#include "objectdatabase/dataobject.h"
#include "gfx/graphing.h"
#include "core/visualdebugger.h"

//------------------------------------------------------------------------------------------
// Statics
//------------------------------------------------------------------------------------------
const float TimeScalarCurve::m_fMinScalar = 0.01f;
const float TimeScalarCurve::m_fMaxScalar = 3.00f;


//------------------------------------------------------------------------------------------
// Interface
//------------------------------------------------------------------------------------------
START_CHUNKED_INTERFACE( TimeScalarCurve, Mem::MC_CAMERA)
	PUBLISH_PTR_CONTAINER_AS(m_obCurvePoints, CurvePoints)
	PUBLISH_VAR_AS(m_bAllowEarlyExit, AllowEarlyExit)
	IENUM	(TimeScalarCurve, FinalPointInterpolationMethod, TIME_SCALAR_CURVE_INTERPOLATION_METHOD)
	PUBLISH_VAR_AS(m_fStartCurveRenderTime, StartCurveRenderTime)
	PUBLISH_VAR_AS(m_fEndCurveRenderTime, EndCurveRenderTime)

	DECLARE_EDITORCHANGEVALUE_CALLBACK(EditorChangeValue)
#ifdef _CAM_CURVE_DEBUG
	DECLARE_EDITORSELECT_CALLBACK(EditorSelect)
#endif
END_STD_INTERFACE

START_CHUNKED_INTERFACE( TimeScalarCurvePoint, Mem::MC_CAMERA)
	PUBLISH_VAR_AS(m_fTime, Time)
	PUBLISH_VAR_AS(m_fScalar, Scalar)
	IENUM	(TimeScalarCurvePoint, InterpolationMethod, TIME_SCALAR_CURVE_INTERPOLATION_METHOD)

	DECLARE_EDITORCHANGEVALUE_CALLBACK(EditorChangeValue)
#ifdef _CAM_CURVE_DEBUG
	DECLARE_EDITORSELECT_CALLBACK(EditorSelect)
#endif
END_STD_INTERFACE


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	TimeScalarCurve::TimeScalarCurve
//!	Construction
//!                                                                                         
//------------------------------------------------------------------------------------------
TimeScalarCurve::TimeScalarCurve()
{
	m_obCurvePoints.clear();
	m_obEndReferencePoint.m_fScalar = 1.0f;
	m_obEndReferencePoint.m_fTime = 1.0f;
	m_obStartReferencePoint.m_fScalar = 1.0f;
	m_obStartReferencePoint.m_fTime = 0.0f;
	m_iGraphSamples = 200;
	m_eFinalPointInterpolationMethod = TSCIM_STEP;
	m_fLastX = m_fLastY = 0.0f;
	m_fStartCurveRenderTime = 0.0f;
	m_fEndCurveRenderTime = 1.0f;
	m_bIsRenderingCurve = false;

#ifdef _CAM_CURVE_DEBUG
	m_bSelected=false;

	m_pobGraph = 0;
	m_pobSamples = 0;
#endif
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	TimeScalarCurve::GetScalar
//!	Get the time scalar for this fraction through the curve.
//!                                                                                         
//------------------------------------------------------------------------------------------
float TimeScalarCurve::GetScalar(float f)
{
	// sometimes f is slightly over 1. Make sute it is within the range
	f = clamp(f, 0.0f, 1.0f);

	// If we're empty, return a harmless scalar
	if (m_obCurvePoints.size() == 0)
		return 1.0f;

	// Get a point in the curve points before this time and after this time and interpolate
	// For splines need a bit more information, 4 points, with our before and after in the middle of them
	PointList::iterator obBegin = m_obCurvePoints.begin();
	PointList::iterator obEnd = m_obCurvePoints.end();
	const TimeScalarCurvePoint *pobPointZero, *pobPointOne, *pobPointTwo, *pobPointThree, *pobPointFour;
	pobPointZero = pobPointOne = pobPointTwo = pobPointThree = pobPointFour = 0;
	
	while (obBegin != obEnd)
	{
		(*obBegin)->m_pobMyTimeScalarCurve = this; // Inform the point of which curve it's in (just for rendering convenience)

		if ((*obBegin)->m_fTime > f)
		{
			pobPointTwo = (*obBegin);

			// Look ahead to see if we've got more incase we're doing a spline
			obBegin++;
			if (obBegin != obEnd)
			{
				pobPointThree = (*obBegin);
				obBegin++;
				if (obBegin != obEnd)
				{
					pobPointFour = (*obBegin);
				}
			}
			
			break;
		}

		// Keep track of our last visited ones so when we find the first point > f, we can just break and have both points ready
		pobPointZero = pobPointOne;
		pobPointOne = (*obBegin);

		obBegin++;
	}

	// Set the interpolation for the final point of the curve
	m_obEndReferencePoint.m_eInterpolationMethod = this->m_eFinalPointInterpolationMethod;
	// If we need a beginning of the curve
	if (!pobPointZero)
		pobPointZero = &m_obStartReferencePoint;
	if (!pobPointOne)
		pobPointOne = &m_obStartReferencePoint;
	// If we've hit the end of the curve
	if (!pobPointTwo)
		pobPointTwo = &m_obEndReferencePoint;
	if (!pobPointThree)
		pobPointThree = &m_obEndReferencePoint;
	if (!pobPointFour)
		pobPointFour = &m_obEndReferencePoint;

	// Something heinous if this doesn't hold
	ntError(pobPointZero && pobPointOne && pobPointTwo && pobPointThree && pobPointFour);

	// Prepare some values for use    
	float fScalar = 1.0f; // Just initing it to something sensible
	float fSectionLength = clamp(pobPointTwo->m_fTime,0.0f,1.0f) - clamp(pobPointOne->m_fTime,0.0f,1.0f); // Length between these 2 points on the curve

	float fFractionIntoSection = 0;
	if (fSectionLength > 0)
		fFractionIntoSection = (f - pobPointOne->m_fTime) / fSectionLength; // How far we are between those 2 points

	// Step straight up to the next scalar
	if (pobPointTwo->m_eInterpolationMethod == TSCIM_STEP)
	{
		fScalar = pobPointTwo->m_fScalar;
	}
	// Cheap as, draw a line
	else if (pobPointTwo->m_eInterpolationMethod == TSCIM_LINEAR)
	{
		float fFractionLeftOfSection = 1-fFractionIntoSection;
	
		fScalar = (pobPointOne->m_fScalar*fFractionLeftOfSection) + (pobPointTwo->m_fScalar*fFractionIntoSection);
	}
	// Slightly less cheap, smooth out the line
	else if (pobPointTwo->m_eInterpolationMethod == TSCIM_SMOOTHSTEP)
	{		
		fFractionIntoSection = CMaths::SmoothStep(fFractionIntoSection);
		float fFractionLeftOfSection = 1-fFractionIntoSection;
		
		fScalar = (pobPointOne->m_fScalar*fFractionLeftOfSection) + (pobPointTwo->m_fScalar*fFractionIntoSection);
	}
	// Bit more involved, a spline (ported from the previous implementation)
	else if (pobPointTwo->m_eInterpolationMethod == TSCIM_SPLINE)
	{
		fScalar = 0.5f *( (2*pobPointOne->m_fScalar)																		             + 
		                ((-pobPointZero->m_fScalar+pobPointTwo->m_fScalar)														* fFractionIntoSection)     + 
				        ((2*pobPointZero->m_fScalar-5*pobPointOne->m_fScalar+4*pobPointTwo->m_fScalar-pobPointThree->m_fScalar) * fFractionIntoSection * fFractionIntoSection)   + 
				        ((-pobPointZero->m_fScalar+3*pobPointOne->m_fScalar-3*pobPointTwo->m_fScalar+pobPointThree->m_fScalar)	* fFractionIntoSection * fFractionIntoSection * fFractionIntoSection) );
	}

	if (!m_bIsRenderingCurve)
	{
		m_fLastX = f;
		m_fLastY = fScalar;
	}

	// Clamp for safety
	return clamp(fScalar, m_fMinScalar, m_fMaxScalar);
}


#ifdef _CAM_CURVE_DEBUG
//------------------------------------------------------------------------------------------
//!                                                                                         
//!	TimeScalarCurve::EditorSelect
//!	The curve has been selected/deselected in welder.
//!                                                                                         
//------------------------------------------------------------------------------------------
void TimeScalarCurve::EditorSelect(bool bSelect)
{
	if(bSelect)
	{
		CamMan::Get().SetSelectedCurve(this);
		m_bSelected = true;

		// Create a Graph to view the curve
		if(!m_pobGraph)
		{
			m_pobGraph = NT_NEW_CHUNK( Mem::MC_CAMERA ) CGraph(GRAPH_TYPE_STANDARD);
		}

		m_pobGraph->SetXAxis(0.0f, 1.0f, 1.0f/(float)m_iGraphSamples);
		m_pobGraph->SetYAxis(0.0f, m_fMaxScalar, 1.0f/(float)m_iGraphSamples);
		m_pobSamples = m_pobGraph->AddSampleSet("CoolCamCurve", m_iGraphSamples, NTCOLOUR_ARGB(0xff, 0xff, 0xff, 0x10));
		m_pobSamples->SetUsedSamples(m_iGraphSamples);
	}
	else
	{
		CamMan::Get().SetSelectedCurve(0);
		m_bSelected = false; 
	}
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	TimeScalarCurve::RenderTimeCurve
//!	Render the time curve for the CoolCamera
//!                                                                                         
//------------------------------------------------------------------------------------------
void TimeScalarCurve::RenderTimeCurve()
{
	// Clean up our old graph as it might have changed
	m_pobGraph->DeleteSampleSet("CoolCamCurve");
	// Create a new graph
	m_pobSamples = m_pobGraph->AddSampleSet("CoolCamCurve", m_iGraphSamples, NTCOLOUR_ARGB(0xff, 0xff, 0xff, 0x10));
	m_pobSamples->SetUsedSamples(m_iGraphSamples);

	if(m_pobGraph && m_pobSamples && IsSelected())
	{
		m_bIsRenderingCurve = true;
		float fInterval = fabs(m_fEndCurveRenderTime - m_fStartCurveRenderTime);
		for(int i = 0; i < m_iGraphSamples; i++)
		{
			float fSample = (float)i/(float)m_iGraphSamples; 
			fSample = m_fStartCurveRenderTime+(fSample*fInterval);

			m_pobSamples->GetSampleData()[i].X() = fSample;
			m_pobSamples->GetSampleData()[i].Y() = GetScalar(fSample);
		}

		// Fit all the data on graph
		m_pobGraph->RecomputeAxes();
		m_pobGraph->SetXAxis(m_fStartCurveRenderTime, m_fEndCurveRenderTime, 1.0f/(float)m_iGraphSamples);

		// Draw the graph
		CPoint obTL(g_VisualDebug->GetDebugDisplayWidth()-225.0f, 25.0f, 0.0f);
		CPoint obBR(g_VisualDebug->GetDebugDisplayWidth()-25.0f, 225.0f, 0.0f);
		g_VisualDebug->Draw2D();
		g_VisualDebug->RenderGraph(m_pobGraph, obTL, obBR);
		CPoint obProgress((obTL.X()-4)-(m_fLastX*((obTL.X()-obBR.X())*1.0f)), (obBR.Y())+(m_fLastY*(obTL.Y()-obBR.Y())*0.5f), 0.0f);
		
		// Render current progress in TSCurve
		g_VisualDebug->Printf2D(obProgress.X(),obProgress.Y(),DC_RED,0,"+");
		m_bIsRenderingCurve = false;

		// Output the name of this TSCurve
		CPoint obTextPos(g_VisualDebug->GetDebugDisplayWidth()-225.0f, 225.0f, 0.0f);
		obTextPos.Y() += 20.0f;
		g_VisualDebug->Printf2D(obTextPos.X(), obTextPos.Y(), DC_WHITE, 0, ntStr::GetString(ObjectDatabase::Get().GetNameFromPointer(this)));
	}
}

void TimeScalarCurvePoint::EditorSelect( bool bSelected ) 
{ 
	if (m_pobMyTimeScalarCurve) 
		m_pobMyTimeScalarCurve->EditorSelect(bSelected); 
};

bool TimeScalarCurvePoint::IsSelected() const 
{ 
	if (m_pobMyTimeScalarCurve) 
		return m_pobMyTimeScalarCurve->IsSelected();
	else
		return false;
}

#endif  //_CAM_CURVE_DEBUG


