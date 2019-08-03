#include "config.h"
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD

#include "antigravityaction.h"
#include "maths_tools.h"
#include "core/timer.h"

#include <hkmath/hkmath.h>

#include "physics/world.h"

namespace Physics
{

	CAntiGravityAction::CAntiGravityAction( hkRigidBody* body, float fMinimumVelocity, float fDuration ) :
		hkUnaryAction( body ),
		m_fNoGravityDuration( fDuration ),
		m_fRestoreDuration( 2.0f ),
		m_fMinimumVelocitySquared( fMinimumVelocity * fMinimumVelocity ),
		m_body( body )
	{
	}

	void CAntiGravityAction::applyAction( const hkStepInfo& stepInfo )
	{
		if (m_fNoGravityDuration>0.0f)
		{
			//m_fNoGravityDuration-=stepInfo.m_deltaTime;
			m_fNoGravityDuration-=CTimer::Get().GetGameTimeChange();

			// If the velocity drops below the threshold, drop out of nogravity (so object doesn't look like its floating in air)
			
			if (m_body->getLinearVelocity().lengthSquared3() < m_fMinimumVelocitySquared )
			{
				m_fNoGravityDuration=0.0f;
			}

			float fForce=m_body->getMass() * -fGRAVITY;
			m_body->applyForce(stepInfo.m_deltaTime, hkVector4(0.0f,fForce,0.0f));
			
		}
		else if (m_fRestoreDuration>0.0f)
		{
			//m_fRestoreDuration-=stepInfo.m_deltaTime; // Restore gravity gradually
			m_fRestoreDuration-=CTimer::Get().GetGameTimeChange();

			if (m_fRestoreDuration < 0.0f)
			{
				m_fRestoreDuration=0.0f;
			}
			
			float fForce=m_body->getMass() * -fGRAVITY * (m_fRestoreDuration/2.0f);
			m_body->applyForce(stepInfo.m_deltaTime, hkVector4(0.0f,fForce,0.0f));
			
		}
	}
}

#endif

