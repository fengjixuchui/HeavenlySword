//---------------------------------------------------------------------------------------------------------
//!
//!	\file physics/antigravityaction.h
//!	
//!	DYNAMICS COMPONENT:
//!
//!	Author: Mustapha Bismi (mustapha.bismi@ninjatheory.com)
//! Created: 2005.08.10
//!
//---------------------------------------------------------------------------------------------------------

#ifndef _DYNAMICS_ANTI_GRAVITY_INC
#define _DYNAMICS_ANTI_GRAVITY_INC

#include "config.h"
#include "physics/havokincludes.h"

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
#include <hkdynamics/action/hkUnaryAction.h>
#include <hkdynamics/entity/hkRigidBody.h>

namespace Physics
{

	/***************************************************************************************************
	*	
	*	CLASS			CAntiGravityAction
	*
	*	DESCRIPTION		This action enables havok to effectively disable gravity on a rigid body.
	* Deano moved here to save compile times
	*
	***************************************************************************************************/
	class CAntiGravityAction : public hkUnaryAction
	{
		public:

			CAntiGravityAction( hkRigidBody* body, float fMinimumVelocity, float fDuration );

			float m_fNoGravityDuration;

			float m_fRestoreDuration;

		private:

			virtual void applyAction( const hkStepInfo& stepInfo );

			// do not clone !
			virtual hkAction *clone (const hkArray< hkEntity * > &newEntities, const hkArray< hkPhantom * > &newPhantoms) const { return 0; };

			float m_fMinimumVelocitySquared;


			hkRigidBody* m_body;
	};
}
#endif
#endif //_DYNAMICS_ANTI_GRAVITY_INC
