/***************************************************************************************************
*
*	DESCRIPTION		Allows the serialisation of different curve types
*
*	NOTES
*
***************************************************************************************************/

// Includes
#include "camera/curveeditor.h"
#include "camera/curves.h"
#include "core/visualdebugger.h"
#include "objectdatabase/dataobject.h"

//!-------------------------------------------------------------------------------------
//! Curve editor interfaces
//!-------------------------------------------------------------------------------------
START_CHUNKED_INTERFACE	(CCatmullEditor, Mem::MC_CAMERA)
	IBOOL		(CCatmullEditor, Open)
	IBOOL		(CCatmullEditor, DistanceParameterise)
	PUBLISH_CONTAINER_AS(m_obControlVertList, ControlVertList)	
	DECLARE_POSTCONSTRUCT_CALLBACK(PostConstruct)
	DECLARE_EDITORCHANGEVALUE_CALLBACK( EditorChangeValue )
END_STD_INTERFACE

START_CHUNKED_INTERFACE	(CMrEdCurveEditor, Mem::MC_CAMERA)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bOpen, true, Open)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bDistanceParameterise, true, DistanceParameterise)
	PUBLISH_CONTAINER_AS(m_obControlVertList, ControlVertList)	
	DECLARE_POSTCONSTRUCT_CALLBACK(PostConstruct)
	DECLARE_EDITORCHANGEVALUE_CALLBACK( EditorChangeValue )
END_STD_INTERFACE


/***************************************************************************************************
*
*	FUNCTION		CCurveEditor::CCurveEditor
*
*	DESCRIPTION		Default constructor - zero our actual cureve pointer
*
***************************************************************************************************/

CCurveEditor::CCurveEditor(void)
:	CCurveInterface(false),
	m_pobCurve(0)
{
}


/***************************************************************************************************
*
*	FUNCTION		CCurveEditor::~CCurveEditor
*
*	DESCRIPTION		Destruction - our curve pointer will be zero if not valid so go ahead and free
*
***************************************************************************************************/

CCurveEditor::~CCurveEditor(void)
{
	NT_DELETE_CHUNK(Mem::MC_CAMERA, m_pobCurve );
}


/***************************************************************************************************
*
*	FUNCTION		CCurveEditor::Evaluate
*
*	DESCRIPTION		If we have a valid pointer to the curve we are editing then use it, otherwise
*					return something harmless - this needs to run whilst editing
*
***************************************************************************************************/

CVector CCurveEditor::Evaluate(float fU) const
{
	if (m_pobCurve)
		return m_pobCurve->Evaluate(fU);
	else
		return CVector(CONSTRUCT_CLEAR);
}


/***************************************************************************************************
*
*	FUNCTION		CCurveEditor::Derivative1
*
*	DESCRIPTION		If we have a valid pointer to the curve we are editing then use it, otherwise
*					return something harmless - this needs to run whilst editing
*
***************************************************************************************************/

CVector CCurveEditor::Derivative1(float fU) const
{
	if (m_pobCurve)
		return m_pobCurve->Derivative1(fU);
	else
		return CVector(CONSTRUCT_CLEAR);
}


/***************************************************************************************************
*
*	FUNCTION		CCurveEditor::Derivative2
*
*	DESCRIPTION		If we have a valid pointer to the curve we are editing then use it, otherwise
*					return something harmless - this needs to run whilst editing
*
***************************************************************************************************/

CVector CCurveEditor::Derivative2(float fU) const
{
	if (m_pobCurve)
		return m_pobCurve->Derivative2(fU);
	else
		return CVector(CONSTRUCT_CLEAR);
}


/***************************************************************************************************
*
*	FUNCTION		CCurveEditor::GetLength
*
*	DESCRIPTION		If we have a valid pointer to the curve we are editing then use it, otherwise
*					return something harmless - this needs to run whilst editing
*
***************************************************************************************************/

float CCurveEditor::GetLength(void) const
{
	if (m_pobCurve)
		return m_pobCurve->GetLength();
	else
		return 1.0f;
}


/***************************************************************************************************
*
*	FUNCTION		CCurveEditor::GetOpen
*
*	DESCRIPTION		If we have a valid pointer to the curve we are editing then use it, otherwise
*					return something harmless - this needs to run whilst editing
*
***************************************************************************************************/

bool CCurveEditor::GetOpen(void) const
{
	if (m_pobCurve)
		return m_pobCurve->GetOpen();
	else
		return false;
}


/***************************************************************************************************
*
*	FUNCTION		CCurveEditor::GetNumSpan
*
*	DESCRIPTION		If we have a valid pointer to the curve we are editing then use it, otherwise
*					return something harmless - this needs to run whilst editing
*
***************************************************************************************************/

u_int CCurveEditor::GetNumSpan(void) const
{
	if (m_pobCurve)
		return m_pobCurve->GetNumSpan();
	else
		return 1;
}


/***************************************************************************************************
*
*	FUNCTION		CCurveEditor::GetNumCV
*
*	DESCRIPTION		If we have a valid pointer to the curve we are editing then use it, otherwise
*					return something harmless - this needs to run whilst editing
*
***************************************************************************************************/

u_int CCurveEditor::GetNumCV(void) const
{
	if (m_pobCurve)
		return m_pobCurve->GetNumCV();
	else
		return 0;
}


/***************************************************************************************************
*
*	FUNCTION		CCurveEditor::GetCV
*
*	DESCRIPTION		If we have a valid pointer to the curve we are editing then use it, otherwise
*					return something harmless - this needs to run whilst editing
*
***************************************************************************************************/

const CVector CCurveEditor::GetCV(u_int uiIndex) const
{
	if (m_pobCurve)
		return m_pobCurve->GetCV(uiIndex);
	else
		return CVector(CONSTRUCT_CLEAR);
}


/***************************************************************************************************
*
*	FUNCTION		CCurveEditor::Render
*
*	DESCRIPTION		If we have a valid pointer to the curve we are editing then use it, otherwise
*					return something harmless - this needs to run whilst editing
*
***************************************************************************************************/

void CCurveEditor::Render(void) const
{
	if (m_pobCurve)
		return m_pobCurve->Render();
}


/***************************************************************************************************
*
*	FUNCTION		CCurveEditor::RenderPath
*
*	DESCRIPTION		If we have a valid pointer to the curve we are editing then use it, otherwise
*					return something harmless - this needs to run whilst editing
*
***************************************************************************************************/

void CCurveEditor::RenderPath(void) const
{
	if (m_pobCurve)
		return m_pobCurve->RenderPath();
}


/***************************************************************************************************
*
*	FUNCTION		CCurveEditor::RenderTans
*
*	DESCRIPTION		If we have a valid pointer to the curve we are editing then use it, otherwise
*					return something harmless - this needs to run whilst editing
*
***************************************************************************************************/

void CCurveEditor::RenderTans(void) const
{
	if (m_pobCurve)
		return m_pobCurve->RenderTans();
}


/***************************************************************************************************
*
*	FUNCTION		CCurveEditor::RenderCVs
*
*	DESCRIPTION		If we have a valid pointer to the curve we are editing then use it, otherwise
*					return something harmless - this needs to run whilst editing
*
***************************************************************************************************/

void CCurveEditor::RenderCVs(void) const
{
	if (m_pobCurve)
		return m_pobCurve->RenderCVs();
}


/***************************************************************************************************
*
*	FUNCTION		CCurveEditor::DebugRender
*
*	DESCRIPTION		This allows the user of the PC editor to see the curve they are editing whilst
*					they are editing it.  Pretty useful huh.  We render some basic debug text if the
*					curve has not been set up correctly.
*
***************************************************************************************************/

void CCurveEditor::DebugRender(void)
{
#ifndef _GOLD_MASTER
	if (m_pobCurve)
	{
		m_pobCurve->Render();
	}
	else
	{
		g_VisualDebug->Printf2D(50.0f, 100.0f, DC_GREY, 0, "The curve cannot be created from the current data.");
	}
#endif
}


/***************************************************************************************************
*
*	FUNCTION		CCurveEditor::EditorChangeValue
*
*	DESCRIPTION		Tries to construct a new curve based on the settings
*
***************************************************************************************************/

bool CCurveEditor::EditorChangeValue(CallBackParameter, CallBackParameter)
{
	// If the data has changed then we 
	// completely recreate our curve
	ClearCurve();

	// Recreate the curve with the new data
	CreateCurve();

	// Return true - this just confirms that this has tried something
	return true;
}


/***************************************************************************************************
*
*	FUNCTION		CCatmullEditor::CCatmullEditor
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/
CCatmullEditor::CCatmullEditor(void)
:	m_bOpen(true) , m_bDistanceParameterise(true)
{ 
}


/***************************************************************************************************
*
*	FUNCTION		CCatmullEditor::~CCatmullEditor
*
*	DESCRIPTION		Destruction
*
***************************************************************************************************/
CCatmullEditor::~CCatmullEditor(void)
{
}


/***************************************************************************************************
*
*	FUNCTION		CCatmullEditor::CreateCurve
*
*	DESCRIPTION		Creates the contained curve if the given data is correct
*
***************************************************************************************************/
void CCatmullEditor::CreateCurve(void)
{
	// The only required data for a cutmull is four control vertices
	if (m_obControlVertList.size() >= 4)
	{
		// Build an array of vertex data for the new curve
		// [scee_st] CScopedArray doesn't seem to support deleting from specified chunks (although
		// it still works because of the memory system -- suppressed this for safety in case it
		// changes in the future
		CScopedArray<CVector> aobControlVertices(NT_NEW_ARRAY CVector[ m_obControlVertList.size() ]);

		// Loop through our list and set the data to the format it likes
		int iControlVert = 0;
		for (ControlVertContainerType::iterator obIt = m_obControlVertList.begin(); obIt != m_obControlVertList.end(); ++obIt)
		{
			aobControlVertices[ iControlVert ].X() = obIt->X();
			aobControlVertices[ iControlVert ].Y() = obIt->Y();
			aobControlVertices[ iControlVert ].Z() = obIt->Z();
			aobControlVertices[ iControlVert ].W() = 1.0f;

			// Move to the next vert
			iControlVert++;
		}

		// Use the new data to create the curve - it will clear up the control points if we ask nicely
		if (m_bDistanceParameterise)
		{
			// right, we're doing a sneaky time based parameterisation based on distances between CV's
			
			// for catmulls only!!
			int iNumSegments =  m_obControlVertList.size() - 2;

			float* pfTimes = NT_NEW_CHUNK( Mem::MC_CAMERA ) float[ iNumSegments ];
			
			pfTimes[0] = 0.0f;
			for (int i = 1; i < iNumSegments; i++)
			{
				// for catmulls only!!
				pfTimes[i] = (aobControlVertices[i+1] - aobControlVertices[i]).Length() + pfTimes[i-1];
			}

			m_pobCurve = NT_NEW_CHUNK( Mem::MC_CAMERA ) CCubicTimeCatmull(pfTimes, true, m_bOpen, (u_int)m_obControlVertList.size(), aobControlVertices.Get());
		}
		else
		{
			m_pobCurve = NT_NEW_CHUNK( Mem::MC_CAMERA ) CCubicCatmull(m_bOpen, (u_int)m_obControlVertList.size(), aobControlVertices.Get());
		}
		
		ntAssert(m_pobCurve);
	}

	// This data is not ready yet (dutch)

}


/***************************************************************************************************
*
*	FUNCTION		CMrEdCurveEditor::CMrEdCurveEditor
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/
CMrEdCurveEditor::CMrEdCurveEditor(void)
:	m_bOpen(true), 
	m_bDistanceParameterise(true)
{ 
}


/***************************************************************************************************
*
*	FUNCTION		CMrEdCurveEditor::~CMrEdCurveEditor
*
*	DESCRIPTION		Destruction
*
***************************************************************************************************/
CMrEdCurveEditor::~CMrEdCurveEditor(void)
{
}


/***************************************************************************************************
*
*	FUNCTION		CMrEdCurveEditor::CreateCurve
*
*	DESCRIPTION		Creates the contained curve if the given data is correct.  The Mr Ed curve type
*					is basically a catmull with duplicated control points at either end.
*
***************************************************************************************************/
void CMrEdCurveEditor::CreateCurve(void)
{
	// The only required data for a cutmull is four control vertices
	if (m_obControlVertList.size() >= 2)
	{
		// Build an array of vertex data for the new curve
		// [scee_st] CScopedArray doesn't seem to support deleting from specified chunks (although
		// it still works because of the memory system -- suppressed this for safety in case it
		// changes in the future
		CScopedArray<CVector> aobControlVertices(NT_NEW_ARRAY CVector[ m_obControlVertList.size() + 2 ]);

		// Set up the first control point
		aobControlVertices[ 0 ].X() = m_obControlVertList.front().X();
		aobControlVertices[ 0 ].Y() = m_obControlVertList.front().Y();
		aobControlVertices[ 0 ].Z() = m_obControlVertList.front().Z();
		aobControlVertices[ 0 ].W() = 1.0f;

		// Loop through our list and set the data to the format it likes
		int iControlVert = 1;
		for (ntstd::Vector<CPoint, Mem::MC_CAMERA>::iterator obIt = m_obControlVertList.begin(); obIt != m_obControlVertList.end(); ++obIt)
		{
			aobControlVertices[ iControlVert ].X() = obIt->X();
			aobControlVertices[ iControlVert ].Y() = obIt->Y();
			aobControlVertices[ iControlVert ].Z() = obIt->Z();
			aobControlVertices[ iControlVert ].W() = 1.0f;

			// Move to the next vert
			iControlVert++;
		}

		// Set up the last control point
		aobControlVertices[ iControlVert ].X() = m_obControlVertList.back().X();
		aobControlVertices[ iControlVert ].Y() = m_obControlVertList.back().Y();
		aobControlVertices[ iControlVert ].Z() = m_obControlVertList.back().Z();
		aobControlVertices[ iControlVert ].W() = 1.0f;

		// Use the new data to create the curve - it will clear up the control points if we ask nicely
		if (m_bDistanceParameterise)
		{
			// right, we're doing a sneaky time based parameterisation based on distances between CV's
			
			// for catmulls only!!
			int iNumSegments =  m_obControlVertList.size();

			float* pfTimes = NT_NEW_CHUNK( Mem::MC_CAMERA ) float[ iNumSegments ];
			
			pfTimes[0] = 0.0f;
			for (int i = 1; i < iNumSegments; i++)
			{
				// for catmulls only!!
				pfTimes[i] = (aobControlVertices[i+1] - aobControlVertices[i]).Length() + pfTimes[i-1];
			}

			m_pobCurve = NT_NEW_CHUNK( Mem::MC_CAMERA ) CCubicTimeCatmull(pfTimes, true, m_bOpen, (u_int)m_obControlVertList.size() + 2, aobControlVertices.Get());
		}
		else
		{
			m_pobCurve = NT_NEW_CHUNK( Mem::MC_CAMERA ) CCubicCatmull(m_bOpen, (u_int)m_obControlVertList.size() + 2, aobControlVertices.Get());
		}
		
		ntAssert(m_pobCurve);
	}

	// This data is not ready yet

}

