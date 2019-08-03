/***************************************************************************************************
*
*	$Header:: /game/curverail.cpp 1     11/08/03 16:30 Wil                                         $
*
*
*
*	CHANGES
*
*	11/8/2003	Wil	Created
*
***************************************************************************************************/

#include "camera/curverail.h"
#include "camera/camutils.h"
#include "camera/curveeditor.h"

#include "objectdatabase/dataobject.h"

START_CHUNKED_INTERFACE(CurveRailDef, Mem::MC_CAMERA)
	PUBLISH_PTR_AS(m_pobCurve, Curve)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fSpeed, 0.0f, Speed)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fSpring, 0.0f, Spring)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fDamp, 0.0f, Damp)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bXZOnly, true, XZOnly)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bNearest, true, Nearest)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fAvoidance, 0.0f, Avoidance)
END_STD_INTERFACE

/***************************************************************************************************
*
*	FUNCTION		CurveRailDef::constructor
*
*	DESCRIPTION		Set from file (with optional curve overide)
*
***************************************************************************************************/
CurveRailDef::CurveRailDef() :
	m_pobCurve(0),
	m_fSpeed(CR_SPEED),
	m_fSpring(CR_SPRING),
	m_fDamp(CR_DAMP),
	m_bXZOnly(false),
	m_bNearest(true),
	m_fAvoidance(0.0f)
{
}

/***************************************************************************************************
*
*	FUNCTION		Constructor and destructor
*
*	DESCRIPTION		constructor
*
***************************************************************************************************/
CCurveRail::CCurveRail(const CurveRailDef& obDef) :
	m_obDef(obDef),
	m_pobCurve(obDef.GetCurve())
{
	ConstructCore();
}

/***************************************************************************************************
*
*	FUNCTION		CCurveRail::ConstructCore
*
*	DESCRIPTION		initialise
*
***************************************************************************************************/
void CCurveRail::ConstructCore()
{
	ntAssert(m_pobCurve);
	m_bInitialised = false;
	m_obFocusPos.Clear();
	m_obTargetPos.Clear();
	m_fTargetVal = 0.0f;
	m_fTargetDistSq = 0.0f;
	m_obConvergePos.Clear();
	m_fConvergeVal = 0.0f;
	m_fConvergeDistSq = 0.0f;
	m_obDampedPos.Clear();
	m_fDampedVal = 0.0f;
	m_fDampedValVel = 0.0f;
	m_fDampedDistSq = 0.0f;
}

/***************************************************************************************************
*
*	FUNCTION		destructor
*
*	DESCRIPTION		cleanup
*
***************************************************************************************************/
CCurveRail::~CCurveRail()
{
	//CCurveInterface::Destroy(const_cast< CCurveInterface* >(m_pobCurve));
}

/***************************************************************************************************
*
*	FUNCTION		Initialise(CPoint&	obTarget)
*
*	DESCRIPTION		Initilialise the state of the rail to be closest to the target
*
***************************************************************************************************/
void CCurveRail::Initialise(const CPoint& obTarget)
{
	m_obFocusPos = obTarget;
	m_fDampedVal = m_fConvergeVal = m_fTargetVal = m_pobCurve->GetNearFar(m_obFocusPos, m_fTargetDistSq, m_obDef.GetXZOnly(), m_obDef.GetNearest());

	Initialise(m_fTargetVal, m_obFocusPos);
}

/***************************************************************************************************
*
*	FUNCTION		Initialise(float fTargetVal )
*
*	DESCRIPTION		Initilialise the state of the rail to be closest to the target
*
***************************************************************************************************/
void CCurveRail::Initialise(float fTargetVal, const CPoint& obAvoidPos)
{
	m_obFocusPos = obAvoidPos;
	m_fDampedVal = m_fConvergeVal = m_fTargetVal = fTargetVal;
	
	if (m_obDef.GetNearest())	// push the target backwards by the avoidance radius
	{
		m_fTargetVal = m_pobCurve->ConvergeOnTargetAvoiding(m_fConvergeVal, m_fTargetVal, m_obDef.GetAvoidance(), m_obFocusPos, m_obDef.GetXZOnly());
		m_fDampedVal = m_fConvergeVal = m_fTargetVal;
	}
	
	m_obDampedPos = m_obConvergePos = m_obTargetPos = m_pobCurve->GetPoint(m_fTargetVal);

	m_fDampedDistSq = m_fConvergeDistSq = m_fTargetDistSq;
	m_fDampedValVel = 0.0f;

	m_bInitialised = true;
}

/***************************************************************************************************
*
*	FUNCTION		TrackPoint(CPoint&	obTarget)
*
*	DESCRIPTION		Attempt to move to the closest point to the target 
*
***************************************************************************************************/
void CCurveRail::TrackPoint(const CPoint& obTarget, float fTimeChange)
{
	if (m_bInitialised)
		m_obFocusPos = obTarget;
	else
		Initialise(obTarget);	

	// get the closest / furthest point knot time on the curve
	m_fTargetVal = m_pobCurve->GetNearFar(m_obFocusPos, m_fTargetDistSq, m_obDef.GetXZOnly(), m_obDef.GetNearest());

	TrackTarget(m_fTargetVal, m_obFocusPos, fTimeChange);
}

/***************************************************************************************************
*
*	FUNCTION		TrackTarget
*
*	DESCRIPTION		Attempt to move to the target value
*
***************************************************************************************************/
void CCurveRail::TrackTarget(float fTargetVal, const CPoint& obAvoidPos, float fTimeChange)
{
	if (m_bInitialised)
	{
		m_fTargetVal = fTargetVal;
		m_obFocusPos = obAvoidPos;
	}
	else
		Initialise(fTargetVal, obAvoidPos);	

	// this moves the position by a fixed distance, regardless of curve paramaterisation.
	//-----------------------------------------------------------------------------
	// 1. Move the ideal position so that its not intersecting the avoid boundary
	// 2. calculate a new curve pos that is X far away from us in the right direction
	// 3. dampen this convergence value via a spring to get the actual pos

	// 1. push the target backwards by the avoidance radius
	if	(
		(m_obDef.GetNearest()) &&
		(m_obDef.GetAvoidance() > EPSILON)
		)
		m_fTargetVal = m_pobCurve->ConvergeOnTargetAvoiding(m_fConvergeVal, m_fTargetVal, m_obDef.GetAvoidance(), m_obFocusPos, m_obDef.GetXZOnly());

	// get our actual target position
	m_obTargetPos = m_pobCurve->GetPoint(m_fTargetVal);
	m_fTargetDistSq = GetDistSqToFocusPoint(m_obTargetPos);

	float fFrameSpeed = m_obDef.GetSpeed() * fTimeChange; 

	// 2. move along the curve until we're far enough away or we hit the target
	m_fConvergeVal = m_pobCurve->ConvergeOnTargetByFixed(m_fConvergeVal, m_fTargetVal, fFrameSpeed, m_obDef.GetXZOnly());
	m_obConvergePos = m_pobCurve->GetPoint(m_fConvergeVal);
	m_fConvergeDistSq = GetDistSqToFocusPoint(m_obConvergePos);

	// 3. get a dampened version of this
	m_fDampedVal = CCamUtil::ConvergeBySpringDamp(!m_pobCurve->GetOpen(), m_fDampedVal, m_fConvergeVal, m_obDef.GetSpring(),
													m_obDef.GetDamp(), m_fDampedValVel, 0.0f, 1.0f, fTimeChange);
	m_fDampedVal = ntstd::Clamp(m_fDampedVal, 0.0f, 1.0f);

	m_obDampedPos = m_pobCurve->GetPoint(m_fDampedVal);
	m_fDampedDistSq = GetDistSqToFocusPoint(m_obDampedPos);

	//ntPrintf("%.2f, %.2f, %.2f\n", m_fTargetVal, m_fConvergeVal, m_fDampedVal);
}

/***************************************************************************************************
*
*	FUNCTION		Render()
*
*	DESCRIPTION		Initilialise the state of the rail to be closest to the target
*
***************************************************************************************************/
void CCurveRail::Render()
{
	ntAssert(m_bInitialised);

	// render our spline
	m_pobCurve->Render();

	// render where we are
	CCamUtil::Render_Sphere(m_obConvergePos, 0.2f, 0.0f, 1.0f, 1.0f, 1.0f);

	// render where we're trying to be
	CCamUtil::Render_Sphere(m_obTargetPos, 0.1f, 1.0f, 0.0f, 1.0f, 1.0f);

	float fActualTarget = m_pobCurve->GetNearFar(m_obFocusPos, m_fTargetDistSq, m_obDef.GetXZOnly(), m_obDef.GetNearest());
	CPoint obTargetPos = m_pobCurve->GetPoint(fActualTarget);

	CCamUtil::Render_Sphere(obTargetPos, 0.1f, 0.0f, 1.0f, 0.0f, 1.0f);

	// render our real world target pos
	if (m_obDef.GetNearest())
		CCamUtil::Render_Sphere(m_obFocusPos, m_obDef.GetAvoidance(), 1.0f, 0.0f, 0.0f, 1.0f);

	// render our damped position
	CCamUtil::Render_Sphere(m_obDampedPos, 0.1f, 1.0f, 1.0f, 1.0f, 1.0f);
}

