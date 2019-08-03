/***************************************************************************************************
*
*	$Header:: /game/converger.cpp 1     11/08/03 16:30 Wil                                         $
*
*
*
*	CHANGES
*
*	11/8/2003	Wil	Created
*
***************************************************************************************************/

#include "camera/converger.h"
#include "objectdatabase/dataobject.h"

START_CHUNKED_INTERFACE(ConvergerDef, Mem::MC_CAMERA)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bUseDamp, true, UseDamp)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fSpeedUp, 1.0f, SpeedUp)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fSpeedDown, 1.0f, SpeedDown)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fSpring, 1.0f, Spring)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fDamp, 0.5f, Damp)
	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
END_STD_INTERFACE


ConvergerDef::ConvergerDef()
{
	// Welder Defaults
	m_bUseDamp   = true;
	m_fSpeedUp   = 1.0f;
	m_fSpeedDown = 1.0f;
	m_fSpring	 = 1.0f;
	m_fDamp		 = 0.5f;
}

void ConvergerDef::PostConstruct()
{
	if(m_fSpeedDown < 0.0f)
		m_fSpeedDown = m_fSpeedUp;
}


/***************************************************************************************************
*
*	FUNCTION		CPointConverger::Update
*
*	DESCRIPTION		Dampen the movement of a position to a target value via convergence and a simple spring
*
***************************************************************************************************/
CPoint	CPointConverger::Update( const CPoint& obTarget, float fTimeChange )
{
	if (m_bInitialised)
	{
		m_obLastTarget = obTarget;

		// converge by fixed speed
		CDirection obMoveDir = m_obLastTarget ^ m_obConvergeVal;

		float fDisplace = obMoveDir.Length();
		float fSpeed = m_obDef.GetSpeed() * fTimeChange;

		if (fDisplace < fSpeed)
			fSpeed = fDisplace;

		obMoveDir.Normalise();
		m_obConvergeVal += obMoveDir * fSpeed;

		// dampen via simple spring
		if (m_obDef.UseDamp())
		{
			if(fTimeChange > 0.0f)
			{
				// WD removed this in time change re-write (14.02.04)
//				CDirection obDisplace = (m_obDampedVal ^ m_obConvergeVal) * (m_obDef.GetSpring() / CTimer::Get().GetGameTimeScalar());
				CDirection obDisplace = (m_obDampedVal ^ m_obConvergeVal) * m_obDef.GetSpring();
				m_obSpringVel = (m_obSpringVel * m_obDef.GetDamp()) - obDisplace;
				m_obDampedVal += m_obSpringVel * fTimeChange;
			}
		}
		else
			m_obDampedVal = m_obConvergeVal;

		return m_obDampedVal;
	}
	else
	{
		return Initialise( obTarget );
	}
}

