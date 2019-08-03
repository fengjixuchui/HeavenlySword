#include "config.h"

#include "advancedcharactercontroller.h"
#include "hsRagdollInstance.h"

#include "game/entity.h"
#include "game/entity.inl"
#include "camera/camman.h"
#include "camera/camview.h"
#include "game/shellconfig.h"
#include "anim/hierarchy.h"
#include "input/inputhardware.h"
#include "game/randmanager.h"
#include "game/attacks.h"
#include "game/syncdcombat.h"
#include "game/strike.h"
#include "game/messagehandler.h"
#include "core/timer.h"
#include "gfx/renderable.h"
#include "game/renderablecomponent.h"

#include "hierarchy_tools.h"
#include "collisionbitfield.h"
#include "world.h"
#include "maths_tools.h"
#include "physicstools.h"
#include "system.h"
#include "collisionlistener.h"
#include "physics/havokthreadutils.h"
#include "gfx/display.h"

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
#include <hkbase/stream/hkIstream.h>
#include <hkutilities/serialize/hkPhysicsData.h>
#include <hkserialize/util/hkLoader.h>
#include <hkserialize/util/hkBuiltinTypeRegistry.h>
#include <hkserialize/packfile/hkPackfileReader.h>
#include <hkserialize/util/hkRootLevelContainer.h>
#include <hkbase/class/hkClass.h>
#include <hkRagdoll/utils/hkRagdollUtils.h>
#include <hkanimation/mapper/hkSkeletonMapperData.h>
#include <hkanimation/mapper/hkSkeletonMapperUtils.h>
#include <hkanimation/mapper/hkSkeletonMapper.h>
#include <hkdynamics/constraint/motor/hkConstraintMotor.h>
#include <hkdynamics/constraint/motor/position/hkPositionConstraintMotor.h>
#include <hkutilities/constraint/hkconstraintutils.h>
#include <hkragdoll/controller/poweredconstraint/hkRagdollPoweredConstraintController.h>
#include <hkutilities/keyframe/hkKeyFrameUtility.h>
#include <hkdynamics/constraint/hkConstraintData.h>
#include <hkinternal/dynamics/constraint/bilateral/ragdoll/hkRagdollConstraintData.h>
#include <hkutilities/inertia/hkInertiaTensorComputer.h>
#include <hkvisualize/type/hkColor.h>
#include <hkvisualize/hkDebugDisplay.h>
#include <hkragdoll/controller/rigidbody/hkRagdollRigidBodyController.h>
#include <hkragdoll/controller/rigidbody/hkKeyFrameHierarchyUtility.h>
#include <hkbase/memory/hkLocalBuffer.h>
#include <hkdynamics/collide/hkCollisionListener.h>
#include <hkdynamics/action/hkArrayAction.h>
#include <hkcollide/collector/pointcollector/hkSimpleClosestContactCollector.h>
#include <hkcollide/collector/pointcollector/hkAllCdPointCollector.h>
#include <hkcollide/collector/pointcollector/hkClosestCdPointCollector.h>
#include <hkcollide/agent/hkCdBodyPairCollector.h>
#include <hkcollide/collector/bodypaircollector/hkAllCdBodyPairCollector.h>
#include <hkdynamics\constraint\hkConstraintInstance.h>
#include <hkdynamics\constraint\bilateral\ballandsocket\hkBallAndSocketConstraintData.h>
#include <hkdynamics/constraint/motor/springdamper/hkSpringDamperConstraintMotor.h>
#include <hkdynamics/phantom/hkSimpleShapePhantom.h>
#include <hkdynamics/world/listener/hkIslandActivationListener.h>
#include <hkcollide/collector/raycollector/hkAllRayHitCollector.h>
#include <hkcollide/castutil/hkWorldRayCastInput.h>
#include <hkcollide/collector/raycollector/hkClosestRayHitCollector.h>
#endif


#include "objectdatabase/dataobject.h"
#include "objectdatabase/objectdatabase.h"

#include "effect/psystem_utils.h"
#include "effect/effect_manager.h"

#include "game/continuationtransition.h"
#include "game/movement.h"

#include "physics/physicsloader.h"

#include "core/gatso.h"
#include "core/osddisplay.h"

#include "shapephantom.h"

#include "camera/camutils.h"

#include "world.h"


namespace Physics {
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD

	bool IsStateAnimated(RAGDOLL_STATE state) 
	{
		return state != DEACTIVATED && state != DEAD;
	};

	class AscendingActiveTimeComparator
	{
	public:
		bool operator()( Physics::AdvancedRagdoll* pFirst, Physics::AdvancedRagdoll* pSecond ) const
		{
			return ( pFirst->GetActiveTime() < pSecond->GetActiveTime() );
		}
	};

    /** The ragdolls simulation is quite CPU intensive. We want to reduce number of ragdolls simulating at once. 
	  * That is a role of this Manager. 
	  * 
	  * Ragdolls can be simulated:
	  * - as a animated body -- in fact full simulation.
	  * - as a dynamic body -- full simulation.
	  * - as a phantom -- they are not simulated. They will be switched back to dynamic state, only if:
	  *                        -- phantom collides with hero
	  *						   -- phantom collides with enemy and there is some power left for simulation. 
	  *                            (less than MAX_DYNAMIC_RAGDOLLS are currently in scene).
	  *                        -- phantom collides with dynamic rigid body and there is some power left for simulation. 
	  *                            (less than MAX_DYNAMIC_RAGDOLLS are currently in scene).  
	  */
	
	RagdollPerformanceManager::RagdollPerformanceManager() 
		: Singleton<RagdollPerformanceManager>(), m_nAnimatedRagdolls(0) 
	{};

	void RagdollPerformanceManager::Clear()
	{
		m_dynamicRagdolls.clear();
		m_phantomRagdolls.clear();
	};

	void RagdollPerformanceManager::Update()
	{
		// Don't need to phantomise non-interactive ragdolls, as they will automatically do so when at rest.
		if ( g_ShellOptions->m_bInteractableRagdolls )
		{
			if ( (m_nAnimatedRagdolls + m_dynamicRagdolls.size()) > USUAL_DYNAMIC_RAGDOLLS)
			{
				int iRagdollsToPhantomise = m_dynamicRagdolls.size() - USUAL_DYNAMIC_RAGDOLLS;
				ntstd::Vector<AdvancedRagdoll *> dynRagdolls(m_dynamicRagdolls);
	
				for(unsigned int i = 0; i < dynRagdolls.size() && iRagdollsToPhantomise > 0; i++)
				{
					if (dynRagdolls[i]->CanBePhantom())
					{
						dynRagdolls[i]->DynamicToPhantom();
						if (dynRagdolls[i]->IsPhantom())
							iRagdollsToPhantomise--;
					}
				}
			}
		}

		if ((m_nAnimatedRagdolls + m_phantomRagdolls.size() + m_dynamicRagdolls.size()) > MAX_RAGDOLLS)
		{
			ntstd::sort(m_phantomRagdolls.begin(),m_phantomRagdolls.end(),AscendingActiveTimeComparator());
			int iRagdollsToRemove = m_phantomRagdolls.size() + m_dynamicRagdolls.size() - MAX_RAGDOLLS;
			ntstd::Vector<AdvancedRagdoll *> phantomRagdolls(m_phantomRagdolls);

			for(unsigned int i = 0; i < phantomRagdolls.size() && iRagdollsToRemove > 0; i++)
			{
				if (phantomRagdolls[i]->CanBeRemoved())
				{
#ifdef _DEBUG
					int nRagdolls = m_phantomRagdolls.size();
#endif
					// Deactivate physical representation permanently
					// JML Change for respawning need change back...
					phantomRagdolls[i]->Deactivate(false);

					// Stop rendering it
					phantomRagdolls[i]->GetEntity()->Hide();

					// Flag entity for removal
					CMessageSender::SendEmptyMessage("msg_removefromworld",phantomRagdolls[i]->GetEntity()->GetMessageHandler());
					iRagdollsToRemove--;
#ifdef _DEBUG
					// just be sure that only one is removed
					ntAssert( nRagdolls - m_phantomRagdolls.size() < 2);
#endif
				}
			}
		}
	};

	// do we have power to change phantom to dynamic in optional case
	bool RagdollPerformanceManager::CanPhantomToDynamic() const 
	{
		return (m_nAnimatedRagdolls + m_dynamicRagdolls.size()) < MAX_DYNAMIC_RAGDOLLS;
	};

	void RagdollPerformanceManager::AddDynamicRagdoll(AdvancedRagdoll * ragdoll)
	{
#ifdef _DEBUG
		// verify that such a ragdoll is not in list yet 
		for(unsigned int i = 0; i < m_dynamicRagdolls.size(); i++)
		{
			if (m_dynamicRagdolls[i] == ragdoll)
			{
				ntAssert(false);
				return;
			}
		}
#endif

		m_dynamicRagdolls.push_back(ragdoll);
	};

	void RagdollPerformanceManager::AddPhantomRagdoll(AdvancedRagdoll * ragdoll)
	{
#ifdef _DEBUG
		// verify that such a ragdoll is not in list yet 
		for(unsigned int i = 0; i < m_phantomRagdolls.size(); i++)
		{
			if (m_phantomRagdolls[i] == ragdoll)
			{
				ntAssert(false);
				return;
			}
		}
#endif

		m_phantomRagdolls.push_back(ragdoll);
	};

	void RagdollPerformanceManager::AddAnimatedRagdoll(AdvancedRagdoll * ragdoll)
	{
		UNUSED(ragdoll);
		m_nAnimatedRagdolls++;
	};

	void RagdollPerformanceManager::RemoveDynamicRagdoll(AdvancedRagdoll * ragdoll)
	{
		for(unsigned int i = 0; i < m_dynamicRagdolls.size(); i++)
		{
			if (m_dynamicRagdolls[i] == ragdoll)
			{
				m_dynamicRagdolls.erase(m_dynamicRagdolls.begin() + i);
				return;
			}
		}

		ntAssert(false);
	};

	void RagdollPerformanceManager::RemovePhantomRagdoll(AdvancedRagdoll * ragdoll)
	{
		for(unsigned int i = 0; i < m_phantomRagdolls.size(); i++)
		{
			if (m_phantomRagdolls[i] == ragdoll)
			{
				m_phantomRagdolls.erase(m_phantomRagdolls.begin() + i);
				return;
			}
		}

		ntAssert(false);
	};

	void RagdollPerformanceManager::RemoveAnimatedRagdoll(AdvancedRagdoll * ragdoll)
	{
		UNUSED(ragdoll);
		m_nAnimatedRagdolls--;
		ntAssert(m_nAnimatedRagdolls >= 0);
	};


	void RagdollPerformanceManager::PhantomToDynamicRagdoll(AdvancedRagdoll * ragdoll)
	{
		AddDynamicRagdoll(ragdoll);
		RemovePhantomRagdoll(ragdoll);
	};

	void RagdollPerformanceManager::DynamicToPhantomRagdoll(AdvancedRagdoll * ragdoll)
	{
		AddPhantomRagdoll(ragdoll);
		RemoveDynamicRagdoll(ragdoll);
	};

	void RagdollPerformanceManager::AnimatedToDynamicRagdoll(AdvancedRagdoll * ragdoll)
	{
		AddDynamicRagdoll(ragdoll);
		RemoveAnimatedRagdoll(ragdoll);
	};

	void RagdollPerformanceManager::PhantomToAnimatedRagdoll(AdvancedRagdoll * ragdoll)
	{
		AddAnimatedRagdoll(ragdoll);
		RemovePhantomRagdoll(ragdoll);
	};

	void RagdollPerformanceManager::DynamicToAnimatedRagdoll(AdvancedRagdoll * ragdoll)
	{
		AddAnimatedRagdoll(ragdoll);
		RemoveDynamicRagdoll(ragdoll);
	};


	bool RagdollPerformanceManager::CanAnimated()
	{
		return (m_nAnimatedRagdolls + m_dynamicRagdolls.size()) < MAX_RAGDOLLS;
	};	

	

	class RagdollAntiGravityAction : public hkArrayAction 
	{
		public:

			RagdollAntiGravityAction( const hkArray<hkEntity*>& entities ):
				hkArrayAction( entities )
			{
			}

		private:

			virtual void applyAction( const hkStepInfo& stepInfo )
			{
				hkArray< hkEntity * > entitiesOut;
				getEntities(entitiesOut);
				for( int i = 0; i < entitiesOut.getSize(); i++)
				{
					hkRigidBody* m_body = (hkRigidBody*)entitiesOut[i];
					float fForce=m_body->getMass() * (-fGRAVITY);
					m_body->applyForce(stepInfo.m_deltaTime, hkVector4(0.0f,fForce,0.0f));
				}
			}
			virtual hkAction *  clone (const hkArray< hkEntity * > &newEntities, const hkArray< hkPhantom * > &newPhantoms) const { return 0; };
	};

	class RagdollUberGravityAction : public hkArrayAction 
	{
		public:

			RagdollUberGravityAction( const hkArray<hkEntity*>& entities ):
				hkArrayAction( entities )
			{
			}

		private:

			virtual void applyAction( const hkStepInfo& stepInfo )
			{
				hkArray< hkEntity * > entitiesOut;
				getEntities(entitiesOut);
				for( int i = 0; i < entitiesOut.getSize(); i++)
				{
					hkRigidBody* m_body = (hkRigidBody*)entitiesOut[i];
					float fForce=m_body->getMass() * (fGRAVITY);
					m_body->applyForce(stepInfo.m_deltaTime, hkVector4(0.0f,fForce,0.0f));
				}
			}
			virtual hkAction *  clone (const hkArray< hkEntity * > &newEntities, const hkArray< hkPhantom * > &newPhantoms) const { return 0; };
	};

	class ActivatePhantom : public hkAabbPhantom
	{
		AdvancedRagdoll* m_ragdoll;

	public:
		ActivatePhantom ( const hkAabb& aabb, AdvancedRagdoll* ragdoll)
			:hkAabbPhantom(aabb), m_ragdoll(ragdoll)
		{
		}

		/* This is called when something in the Havok world overlaps with a fixed ragdoll */
		virtual void addOverlappingCollidable( hkCollidable* collidable )
		{
			EntityCollisionFlag infoOther;
			infoOther.base = collidable->getCollisionFilterInfo();
				
			if ( infoOther.flags.i_am & CHARACTER_CONTROLLER_PLAYER_BIT )
			{
				// Always allow the player to activate phantomised ragdolls.
				m_ragdoll->PhantomToDynamic();  
			}
			else if ( infoOther.flags.i_am & CHARACTER_CONTROLLER_ENEMY_BIT
					&& RagdollPerformanceManager::Get().CanPhantomToDynamic() 
					&& g_ShellOptions->m_bInteractableRagdolls ) 
			{
				// NPCs are less important, so only allow activation if there is room to do so
				// in the performance manager.
				m_ragdoll->PhantomToDynamic();	
			}
		}
		
		/* This is called when something in the Havok world stops overlapping with a fixed ragdoll */
		virtual void removeOverlappingCollidable( hkCollidable* collidable )
		{
			EntityCollisionFlag infoOther;
			infoOther.base = collidable->getCollisionFilterInfo();
			
			if ( infoOther.flags.i_am & RAGDOLL_BIT ) 
			{
				// A ragdoll has been removed, this ragdoll should be activate, but maybe it 
				// will be removed aswell. So wait for next update
				m_ragdoll->PhantomToDynamicNextUpdate();	
			}
			else if ( infoOther.flags.i_am & SMALL_INTERACTABLE_BIT )
			{
				hkEntity *pobEnt = static_cast<hkEntity *>( collidable->getOwner() );
				if ( pobEnt->m_motion.getType() == hkMotion::MOTION_KEYFRAMED )
				{
					// An object has been picked up, activate in case it disturbs us.
					m_ragdoll->PhantomToDynamic();	
				}
			}
		}
	};

	class RagdollCollisionListener : public hkCollisionListener
	{
	public:

		RagdollCollisionListener (AdvancedRagdoll* pobRag): m_Rag(pobRag)
		{
			Physics::WriteAccess mutex;
			for (int i=0; i < pobRag->m_RagdollInstance->getNumBones(); i++)
			{
				hkRigidBody* rb = pobRag->m_RagdollInstance->getRigidBodyOfBone(i);
				rb->addCollisionListener(this);
			}
		  };

		~RagdollCollisionListener () {}

		// Callbacks for the listeners (derived from hkCollisionListener)
		virtual void contactPointAddedCallback (hkContactPointAddedEvent& event)		{};
		virtual void contactPointRemovedCallback (hkContactPointRemovedEvent& event)	{};
		virtual void contactProcessCallback (hkContactProcessEvent& event)				{};
		virtual void contactPointConfirmedCallback( hkContactPointConfirmedEvent& event)
		{
			// Get the intersecting collidables from Havok.
			hkRigidBody* obRBA	= hkGetRigidBody(&event.m_collidableA);
			hkRigidBody* obRBB	= hkGetRigidBody(&event.m_collidableB);
			
			if( obRBA && obRBB )
			{
				// Initialise pointers to colliding entities.
				CEntity* pobEntityA = ( obRBA->hasProperty(PROPERTY_ENTITY_PTR) ) ? (CEntity*) obRBA->getProperty(PROPERTY_ENTITY_PTR).getPtr() : 0; 
				CEntity* pobEntityB = ( obRBB->hasProperty(PROPERTY_ENTITY_PTR) ) ? (CEntity*) obRBB->getProperty(PROPERTY_ENTITY_PTR).getPtr() : 0; 

				if( pobEntityA != pobEntityB )
				{
					// Get the entities' collision info.
					EntityCollisionFlag infoA;
					infoA.base = obRBA->getCollidable()->getCollisionFilterInfo();
					
					
					// Identify which entity is the ragdoll, and which is the 'other'.
					CEntity* pobRagdoll = pobEntityB;
					CEntity* pOther 	= pobEntityA;
					hkRigidBody* pobBodyPart = obRBB;
					hkRigidBody* pobOtherRB = obRBA;

					if( infoA.flags.i_am & RAGDOLL_BIT )
					{
						pobRagdoll 	= pobEntityA;
						pOther 		= pobEntityB;
						pobBodyPart = obRBA;
						pobOtherRB = obRBB;
					}


					// Are we colliding with static geometry (landscape)?
					if( pobOtherRB->isFixed() )
					{
						if ( (infoA.flags.i_am & Physics::CHARACTER_CONTROLLER_ENEMY_BIT) ||
							(infoA.flags.i_am & Physics::CHARACTER_CONTROLLER_PLAYER_BIT) )
						{
							// I've collided with the spherical fixed core of a phantom rigid body, I don't want to do anything in this case.
							return;
						}

						if ( m_Rag->GetRagdollThrown() )
						{
							// Flag tumbling ragdolls contact with the ground.
							m_Rag->m_bTumbleContact = true;
							pobBodyPart->getMaterial().setFriction( max( pobBodyPart->getMaterial().getFriction(), 0.1f ) );
							pobOtherRB->getMaterial().setFriction(0.9f);
						}
						else
						{
						    // Don't allow animated ragdolls to turn dynamic by collision of the feet.
							Physics::RagdollCollisionFlag iRagFlag; 
							iRagFlag.base = pobBodyPart->getCollidable()->getCollisionFilterInfo();
		
							if ( iRagFlag.flags.ragdoll_material == RAGDOLL_L_KNEE_MATERIAL || 
								 iRagFlag.flags.ragdoll_material == RAGDOLL_R_KNEE_MATERIAL ) 
							{
								return;
							}
						}
					}

					
					// Reactivate phantoms to be dynamic again. Interactions from the player/NPCs is
					// handled by the phantom, when overlaps are added.
					if ( m_Rag->IsPhantom() )
					{
						EntityCollisionFlag infoOther;
						infoOther.base = pobOtherRB->getCollidable()->getCollisionFilterInfo();
						
						unsigned int test_flags = ( SMALL_INTERACTABLE_BIT | LARGE_INTERACTABLE_BIT | RAGDOLL_BIT );
						if ( g_ShellOptions->m_bInteractableRagdolls )
						{
							test_flags |= RIGID_PROJECTILE_BIT;
						}
						
						if ( infoOther.flags.i_am & test_flags
							 && RagdollPerformanceManager::Get().CanPhantomToDynamic() )
						{
							m_Rag->PhantomToDynamic();	
						}
					}  				
					
				
					// Send notifications to the ragdoll update function.
					if ( ( ( m_Rag->GetState() != DEAD ) && m_Rag->m_bTurnDynamicOnContact )
						|| ( m_Rag->m_bRegisterImpact && m_Rag->m_bTurnDynamicOnContact ) )
					{
						// Should we turn over this ragdoll to full dynamic simulation?
						// (i.e. if not already dead.)
						if( m_Rag->GetState() != DEAD )
						{
							m_Rag->m_bSetDeadNextUpdate = true;
						}
						
						// Sound stuff.
						m_Rag->m_bDoSoundNextUpdate = true;
						m_Rag->m_fSoundProjectedVelocity = event.m_projectedVelocity;
						m_Rag->m_bRegisterImpact = false;
						
						// Does this impact warrant particle effects?
						if( m_Rag->GetState() == ANIMATED && pobBodyPart->getLinearVelocity().lengthSquared3() >= 6.0f ) 
						{
							if( m_Rag->GetLinearVelocity().LengthSquared() > 40.0f )
							{
								CMatrix mat = pobRagdoll->GetMatrix();
								mat.SetTranslation( Physics::MathsTools::hkVectorToCPoint(event.m_contactPoint->getPosition()) );
								
								m_Rag->m_bDoParticleEffectNextUpdate = true;
								m_Rag->m_obParticleEffectMatrix = mat;
							}
						}
					}					
				}
			} 
		}

	protected:
		AdvancedRagdoll* m_Rag;

	};
#endif

	
	const float fPHYSICS_WANTED_TIME_STEP = ( 0.0166667f * 2.f );
	const float fPHYSICS_WANTED_STEP_FRAMERATE	= 1.f / fPHYSICS_WANTED_TIME_STEP;
	
	float AdvancedRagdoll::m_fWaistPlaneAngle = 0.0f;
	float AdvancedRagdoll::m_fWaistTwistAngle = 0.0f;
/*#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	float AdvancedRagdoll::maxPosPowerHinge	= 0.0f;
	float AdvancedRagdoll::maxPosPowerTwist	= 0.0f;
	float AdvancedRagdoll::maxPosPowerCone	= 0.0f;
	float AdvancedRagdoll::maxPosPowerPlane	= 0.0f;
	float AdvancedRagdoll::maxNegPowerHinge	= 0.0f;
	float AdvancedRagdoll::maxNegPowerTwist	= 0.0f;
	float AdvancedRagdoll::maxNegPowerCone	= 0.0f;
	float AdvancedRagdoll::maxNegPowerPlane	= 0.0f;
#endif*/
	//float AdvancedRagdoll::m_maxSpringConstant	= 1000.0f;
	//float AdvancedRagdoll::m_maxSpringDamping	= 30.0f;
	

	
	// ------------------------------- HARDCODED SETTINGS -------------------------------------------------------
	static char *asRagdollBodyName[] = 
	{
		"_Invalid_Bone_",			//CHARACTER_BONE_ROOT = CHARACTER_BONE_HIGH_DETAIL_START,		//0	'root' 
		"Biped_Pelvis",				//CHARACTER_BONE_PELVIS,										//1	'pelvis'
		"Biped_Spine0",				//CHARACTER_BONE_SPINE_00,										//2	'spine_00'
		"Biped_Spine1",				//CHARACTER_BONE_SPINE_01,										//3	'spine_01'
		"Biped_Spine2",				//CHARACTER_BONE_SPINE_02,										//4	'spine_02'
		"_Invalid_Bone_",			//CHARACTER_BONE_NECK,											//5	'neck'
		"Biped_Head",				//CHARACTER_BONE_HEAD,											//6	'head'
		"_Invalid_Bone_",			//CHARACTER_BONE_HIPS,											//7	'hips'

		"_Invalid_Bone_",			//CHARACTER_BONE_L_SHOULDER,									//8	'l_shoulder'
		"Biped_L_UpperArm",			//CHARACTER_BONE_L_ARM,											//9	'l_arm'
		"Biped_L_ForeArm",			//CHARACTER_BONE_L_ELBOW,										//10	'l_elbow'
		"_Invalid_Bone_",			//CHARACTER_BONE_L_WRIST,										//11	'l_wrist'
		"_Invalid_Bone_",			//CHARACTER_BONE_L_WEAPON,										//12	'l_weapon'

		"Biped_L_Thigh",			//CHARACTER_BONE_L_LEG,											//13	'l_leg'
		"Biped_L_Calf",				//CHARACTER_BONE_L_KNEE,										//14	'l_knee'

		"_Invalid_Bone_",			//CHARACTER_BONE_R_SHOULDER,									//15	'r_shoulder'
		"Biped_R_UpperArm",			//CHARACTER_BONE_R_ARM,											//16	'r_arm'
		"Biped_R_ForeArm",			//CHARACTER_BONE_R_ELBOW,										//17	'r_elbow'
		"_Invalid_Bone_",			//CHARACTER_BONE_R_WRIST,										//18	'r_wrist'
		"_Invalid_Bone_",			//CHARACTER_BONE_R_WEAPON,										//19	'r_weapon'

		"Biped_R_Thigh",			//CHARACTER_BONE_R_LEG,											//20	'r_leg'
		"Biped_R_Calf",				//CHARACTER_BONE_R_KNEE,										//21	'r_knee'

	};
	static int asRagdollBodyNameSize = 22;

	void AdvancedRagdoll::DoParticleEffect()
	{
		if (!m_pSparksDef)
		{
			m_pSparksDef = ObjectDatabase::Get().GetPointerFromName<void*>("HitDust");
			ntError_p( m_pSparksDef, ("Missing 'Hit_Sparks_Definition' for Havok ParticleCollisionListener\n") );
		}
		
		PSystemUtils::ConstructParticleEffect( m_pSparksDef, m_obParticleEffectMatrix );
	}

	void AdvancedRagdoll::DoSound()
	{
		// TODO (chipb) replace
		// m_EnemyWallIImpactAliveDef->AddBounceEvent( m_fSoundProjectedVelocity, GetPosition());
	}

	/*void AdvancedRagdoll::AddConstraintOnLeftFoot()
	{
		// If you uncomment this code, make sure you convert the call to getTransform below to approxTransformAt - assuming we're still using asynchronous timing that is
		hkBallAndSocketConstraintData* data = HK_NEW hkBallAndSocketConstraintData;

		hkWorld* w = m_RagdollInstance->getWorld();
		hkRigidBody* bd1 = w->getFixedRigidBody();
		data->setInWorldSpace(		bd1->getTransform(),  
									m_RagdollInstance->getRigidBodyOfBone( L_KNEE_BONE )->getTransform(),
									MathsTools::CPointTohkVector( m_entity->GetHierarchy()->GetCharacterBoneTransform( CHARACTER_BONE_L_ANKLE )->GetWorldTranslation() ));
		
		hkConstraintInstance* c =  HK_NEW hkConstraintInstance( bd1,  m_RagdollInstance->getRigidBodyOfBone( L_KNEE_BONE ), data );

		CPhysicsWorld::Get().AddConstraint( c );
		c->getData()->removeReference();
		c->removeReference();
	}*/
	

	// ------------------------------- PUBLIC INTERFACE -------------------------------------------------------
	AdvancedRagdoll::AdvancedRagdoll( Character* p_entity, const char* p_filename )
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	{
		Physics::WriteAccess mutex;

		m_entity = p_entity;
		m_iAnimatedBoneFlags = 0;
		m_pcAtRestMsg = 0;

		m_bPermanentlyDeactivated 	= false;
		m_bExemptFromDisposal 		= false;
		m_bSendRagdollFloored 		= false;
		m_bIsActive 				= false;
		m_bFirstTime 				= true;
		m_bTurnDynamicOnContact 	= false;
		m_bRagdollIsThrown 			= false;
		m_bRegisterImpact 			= false;
		m_bTumbleContact			= false;
		m_bRagdollIsHeld			= false;
		m_bSetDeadNextUpdate 		= false;
		m_bDoParticleEffectNextUpdate = false;
		m_bDoSoundNextUpdate 		= false;
		m_bIsPhantom 				= false;
		m_bMoving					= true;
		m_bPhantomToDynamicNextUpdate = false;
		
		m_fCumulativeLinearVelocitySqr = -1.0f;
		m_fForceDistanceMultiplier 	= 1.0f;
		m_fAngleDistanceMultiplier 	= 1.0f;
		m_AnimatedSkeleton	= 0;		//< This skeleton represent the high def animated skeleton.
		m_RagdollSkeleton	= 0;
		m_RagdollInstance	= 0;
		m_AnimatedToRagdollMapper = 0;
		m_RagdollToAnimatedMapper = 0;
		m_pobAntiGravityAction = 0;
		m_pobUberGravityAction = 0;

		m_stateBeforePause = Undefined;

		// Build the ragdoll from ps.xml
		SetupRagdoll( p_filename );
		
		// Set all bones dynamic
		SetFullRagdoll();

		hkArray<hkEntity*> antiArray;
		antiArray.pushBack( m_RagdollInstance->getRigidBodyOfBone(HEAD_BONE ));
		antiArray.pushBack( m_RagdollInstance->getRigidBodyOfBone(R_FORARM_BONE));
		antiArray.pushBack( m_RagdollInstance->getRigidBodyOfBone(L_FORARM_BONE));
		antiArray.pushBack( m_RagdollInstance->getRigidBodyOfBone(R_ELBOW_BONE));
		antiArray.pushBack( m_RagdollInstance->getRigidBodyOfBone(L_ELBOW_BONE));
		antiArray.pushBack( m_RagdollInstance->getRigidBodyOfBone(SPINE_BONE));

		antiArray.pushBack( m_RagdollInstance->getRigidBodyOfBone(PELVIS_BONE));
		antiArray.pushBack( m_RagdollInstance->getRigidBodyOfBone(L_LEG_BONE));
		antiArray.pushBack( m_RagdollInstance->getRigidBodyOfBone(R_LEG_BONE));
		antiArray.pushBack( m_RagdollInstance->getRigidBodyOfBone(L_KNEE_BONE));
		antiArray.pushBack( m_RagdollInstance->getRigidBodyOfBone(R_KNEE_BONE));
		m_pobAntiGravityAction = HK_NEW RagdollAntiGravityAction( antiArray );

		hkArray<hkEntity*> uberArray;
		uberArray.pushBack( m_RagdollInstance->getRigidBodyOfBone(HEAD_BONE ));
		uberArray.pushBack( m_RagdollInstance->getRigidBodyOfBone(R_FORARM_BONE));
		uberArray.pushBack( m_RagdollInstance->getRigidBodyOfBone(L_FORARM_BONE));
		uberArray.pushBack( m_RagdollInstance->getRigidBodyOfBone(R_ELBOW_BONE));
		uberArray.pushBack( m_RagdollInstance->getRigidBodyOfBone(L_ELBOW_BONE));
		uberArray.pushBack( m_RagdollInstance->getRigidBodyOfBone(SPINE_BONE));

		uberArray.pushBack( m_RagdollInstance->getRigidBodyOfBone(PELVIS_BONE));
		uberArray.pushBack( m_RagdollInstance->getRigidBodyOfBone(L_LEG_BONE));
		uberArray.pushBack( m_RagdollInstance->getRigidBodyOfBone(R_LEG_BONE));
		uberArray.pushBack( m_RagdollInstance->getRigidBodyOfBone(L_KNEE_BONE));
		uberArray.pushBack( m_RagdollInstance->getRigidBodyOfBone(R_KNEE_BONE));
		m_pobUberGravityAction = HK_NEW RagdollUberGravityAction( uberArray );

		m_listener = NT_NEW RagdollCollisionListener( this );

		m_obWorldMatrixBeforeUpdate = m_entity->GetHierarchy()->GetRootTransform()->GetWorldMatrix();
		m_obPelvisPositionLastFrame = GetPosition();

		m_obParticleEffectMatrix = CMatrix( CONSTRUCT_IDENTITY );
		m_fSoundProjectedVelocity = 0.0f;
		m_pSparksDef = 0;
		m_phantom = 0;

		m_currentState = DEACTIVATED;
	}
#else
	{
		UNUSED( p_entity );
		UNUSED( p_filename );
	}
#endif

	AdvancedRagdoll::~AdvancedRagdoll()
	{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		Physics::WriteAccess mutex;

		Deactivate();

		if( m_listener )
		{
			NT_DELETE( m_listener );
		}

		if( m_pobAntiGravityAction )
		{
			HK_DELETE( m_pobAntiGravityAction );
		}

		if( m_pobUberGravityAction )
		{
			HK_DELETE( m_pobUberGravityAction );
		}


		NT_DELETE_ARRAY( m_AnimatedSkeleton->m_parentIndices );
		HK_DELETE_ARRAY( m_AnimatedSkeleton->m_referencePose );
		
		for(int i = 0; i < m_AnimatedSkeleton->m_numBones; i++ )
		{
			NT_DELETE_ARRAY( m_AnimatedSkeleton->m_bones[i]->m_name );
			HK_DELETE( m_AnimatedSkeleton->m_bones[i] ); 	
		};

		if( m_AnimatedSkeleton )
		{
			HK_DELETE( m_AnimatedSkeleton );
		}

		if(m_RagdollToAnimatedMapper)
			m_RagdollToAnimatedMapper->removeReference();

		if(m_AnimatedToRagdollMapper)
			m_AnimatedToRagdollMapper->removeReference();

		if(m_RagdollSkeleton)
			hkRagdollUtils::destroySkeleton(m_RagdollSkeleton);

		if(m_RagdollInstance) {
			if(m_RagdollInstance->getWorld())
				m_RagdollInstance->removeFromWorld();
			m_RagdollInstance->removeReference();
		}

		/*if (m_aobBoneConnectivityGraph)
		{
			NT_DELETE_ARRAY( m_aobBoneConnectivityGraph );
		}*/

		if (m_aobBoneTransformTracking)
		{
			NT_DELETE_ARRAY( m_aobBoneTransformTracking );
		}

		if (m_phantom)
		{
			HK_DELETE(m_phantom);
		}
#endif
	}


	//------------------------------------------------------------------------------------------
	//
	// Function: AdvancedRagdoll::CalculateMotionStatus
	// Author: SCEE_SWRIGHT (22/11/2006)
	//
	//------------------------------------------------------------------------------------------
	void AdvancedRagdoll::CalculateMotionStatus( void )
	{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		m_fCumulativeLinearVelocitySqr = 0.0f;
		m_bMoving = false;
		
		if ( m_RagdollInstance && !IsPhantom() )
		{
			Physics::ReadAccess mutex;
			for (int i=0; i < m_RagdollInstance->getNumBones(); i++)
			{
				// Total up the squared velocity of each rigid body.
				hkRigidBody *pobBody = m_RagdollInstance->getRigidBodyOfBone(i);
				hkVector4 obLinearVel = pobBody->getLinearVelocity();
				m_fCumulativeLinearVelocitySqr += obLinearVel.lengthSquared3();
				
				// Are any of the rigid bodies active?
				if ( !m_bMoving && pobBody->isActive() )
				{
					m_bMoving = true;
				}
			}
		}
#endif
	}
	
	
	void AdvancedRagdoll::Activate( RAGDOLL_STATE p_state )
	{	
		if (m_bPermanentlyDeactivated)
			return;

		if (p_state == DEAD)
		{
			// Need to set a bool and do this in the main update because this can be called from another thread thus causing synchronisation badness
			m_bSendRagdollFloored = true;
			m_fCumulativeLinearVelocitySqr = -1.0f;
			
			if (m_currentState != p_state)
			{
				// Add ragdoll to the ragdoll cleanup manager.
				ntAssert(!m_bIsPhantom);
				if (IsStateAnimated(m_currentState)) 
				{
					// was animated before
					RagdollPerformanceManager::Get().AnimatedToDynamicRagdoll(this);
				}
				else
				{
					RagdollPerformanceManager::Get().AddDynamicRagdoll(this);
				}
				
				// For thrown ragdolls, aim them at the player.
				if ( GetRagdollThrown() )
				{
					AimAtPlayer();
				}
			}
		}

		if (m_currentState == p_state) 
			return; // Already active in this state

		Physics::WriteAccess mutex;

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		if (p_state != DEACTIVATED)
		{
			// Reset the amount of time we've been active in this state for
			m_fActiveTime = 0.0f;
			m_fNonPhantomTime = 0.0f;

			// Reset everything to do with the ragdoll if previously it was doing nothing at all
			if( !m_bIsActive )
			{
				// If it's not already in there, place it in the world
				if( m_RagdollInstance->getWorld() == 0 )
				{
					m_RagdollInstance->addToWorld( true );	
				}

				ResetEntireRagdoll(true,true, true);				
				AddLinearVelocity(m_entity->GetCalcVelocity()); 
			}

			if (m_currentState == DEACTIVATED && IsStateAnimated(p_state))
			{
				// some kind of animated ragdoll
				RagdollPerformanceManager::Get().AddAnimatedRagdoll(this);
			} 
			else if (m_currentState == DEAD)
			{
				if (IsPhantom())
					RagdollPerformanceManager::Get().PhantomToAnimatedRagdoll(this);
				else
					RagdollPerformanceManager::Get().DynamicToAnimatedRagdoll(this);
			}

			// Set the state properties
			SetState( p_state );
			
			// If it's our first time in the world, register collision listeners
			if( m_bFirstTime )
			{
				for(int iRigid = 0; iRigid< m_RagdollInstance->getNumBones(); ++iRigid)
				{
					m_entity->GetPhysicsSystem()->GetCollisionListener()->RegisterRigidBody( m_RagdollInstance->getRigidBodyOfBone( iRigid ) );
				}
				m_bFirstTime = false;
			}

			// Set position.
			m_obPelvisPositionLastFrame = GetPosition();
			m_obIdealEntityRootMatrix = m_entity->GetHierarchy()->GetRootTransform()->GetWorldMatrix();

			// Ragdoll is now active.
			m_bIsActive = true;
			m_currentState = p_state;
		}
		else
		{
			CMatrix obNewRootMatrix( m_obWorldMatrixBeforeUpdate );
			obNewRootMatrix.SetTranslation( GetPosition() );
			m_entity->GetHierarchy()->GetRootTransform()->SetLocalMatrix( obNewRootMatrix );

			// I'm deactivated, so do nothing of any real consequence
			m_currentState = p_state;
			m_bIsActive = false;
		}
#endif
	}

	void AdvancedRagdoll::ForceFreeze()
	{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		Physics::WriteAccess mutex;

		for(int iRigid = 0; iRigid< m_RagdollInstance->getNumBones(); ++iRigid)
		{
			m_RagdollInstance->getRigidBodyOfBone(iRigid)->setLinearVelocity(hkVector4::getZero());
			m_RagdollInstance->getRigidBodyOfBone(iRigid)->setAngularVelocity(hkVector4::getZero());
			m_RagdollInstance->getRigidBodyOfBone(iRigid)->deactivate();
		}
#endif
	}

	void AdvancedRagdoll::Deactivate( bool bPermanently )
	{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD

		if( m_bIsActive )
		{
  			Physics::WriteAccess mutex;

			if (m_currentState == DEAD)
			{
				if (m_bIsPhantom) 
				{
					RagdollPerformanceManager::Get().RemovePhantomRagdoll(this);
					m_bIsPhantom = false;
					ntAssert(m_phantom && m_phantom->getWorld());
					m_phantom->getWorld()->removePhantom(m_phantom);
				}
				else
				{
					RagdollPerformanceManager::Get().RemoveDynamicRagdoll(this);
				}
			} else if (IsStateAnimated(m_currentState))
			{
				/// was in animated state 
				RagdollPerformanceManager::Get().RemoveAnimatedRagdoll(this);
			}

			// Update the ideal matrix, this is so if the char controller is next, it's got a sensible position to inherit
			m_obIdealEntityRootMatrix = m_obWorldMatrixBeforeUpdate;
			m_obIdealEntityRootMatrix.SetTranslation(GetPosition());

			if( m_RagdollInstance != NULL && m_RagdollInstance->getWorld() )
			{
				for(int iRigid = 0; iRigid< m_RagdollInstance->getNumBones(); ++iRigid)
				{
					m_RagdollInstance->getRigidBodyOfBone( iRigid )->removeCollisionListener( m_entity->GetPhysicsSystem()->GetCollisionListener() );
				}
				m_RagdollInstance->removeFromWorld();
			}

			m_bFirstTime = true;										
			m_bIsActive = false;			
			m_currentState = DEACTIVATED;
			SetCharacterDead(false);
		}

		m_bPermanentlyDeactivated = bPermanently;
#endif
	}

	bool AdvancedRagdoll::IsActive()
	{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		return m_bIsActive;
#else
		return false;
#endif
	}

	void AdvancedRagdoll::PausePresenceInHavokWorld(bool pause)
	{
		if (pause)
		{			
			if (!m_RagdollInstance->getWorld())
			{
				m_stateBeforePause = NotInWorld;
				return;
			}

			if (m_RagdollInstance->getRigidBodyArray()[0]->isActive())
				m_stateBeforePause = Activated;
			else
				m_stateBeforePause = Deactivated;

			m_RagdollInstance->removeFromWorld(); 
		}
		else
		{
			if (m_stateBeforePause == NotInWorld || m_stateBeforePause == Undefined)
				return; // nothing to do 

			m_RagdollInstance->addToWorld(true,
				(m_stateBeforePause == Activated) ? HK_ENTITY_ACTIVATION_DO_ACTIVATE : HK_ENTITY_ACTIVATION_DO_NOT_ACTIVATE);

			m_stateBeforePause = Undefined; 
		}
	}


	void AdvancedRagdoll::SetFullRagdoll()
	{
		Physics::WriteAccess mutex;
		for(int iRigid = 0; iRigid< m_RagdollInstance->getNumBones(); ++iRigid)
		{
			m_RagdollInstance->getRigidBodyOfBone( iRigid )->setMotionType( hkMotion::MOTION_DYNAMIC );
		}
	}

	void AdvancedRagdoll::SetState( RAGDOLL_STATE p_state )
	{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		Physics::WriteAccess mutex;

		switch( p_state )
		{		
		case ANIMATED:
			{								
				// we do not have last pose for inicialization
				m_lastPose.setSize(0);
			}
		case ANIMATED_ABSOLUTE:
			{		
				// Disable ragdoll-ragdoll collisions.		
				SetBoneBoneCollision( false );
				
				// Add an anti-gravity action so we can animate without getting pulled down
				SetAntiGravity(true);

				break;
			}
		case TRANSFORM_TRACKING:
			{
				// Reset mappings ready for a new set of mappings
				ResetBoneTransformTrackingArray();

				// Remove any anti-gravity action
				SetAntiGravity(false);

				// Enable ragdoll-ragdoll collisions. 
				SetBoneBoneCollision( true );
				
				// We do not have last pose for initialization
				m_lastPose.setSize(0);
				
				break;
			}
		case TRANSFORM_TRACKING_ANIMATED:
			{
				// Enable ragdoll-ragdoll collisions. 
				SetBoneBoneCollision( true );
				
				// Remove any anti-gravity action
				SetAntiGravity(false);
				
				break;
			}
		case DEACTIVATED:
			break;
		case DEAD:
			{
				
				// Enable ragdoll-ragdoll collisions. 
				SetBoneBoneCollision( true );
				
				// Remove any anti-gravity action
				SetAntiGravity(false);	
				
				// Alter ragdoll setup for full dynamics.
				if (m_currentState == ANIMATED && m_lastPose.getSize() > 0)
				{
					// apply velocities from animation
					for(int i = 0; i < m_aiBonesToAnimate.getSize(); i++)
					{
						int iBone = m_aiBonesToAnimate[i];
						hkRigidBody * body = m_RagdollInstance->getRigidBodyOfBone(iBone);
						body->setLinearVelocity(m_lastPose[iBone].m_forLastAnimLinearVelocity); 
						body->setAngularVelocity(m_lastPose[iBone].m_forLastAnimAngularVelocity); 	
					}
				}

				m_lastPose.setSize(0);
				
				// When we start being dead, push a new controller on which allows us to take care of ourselves when we're like this
				RagdollContinuationTransitionDef obRagDef;
				obRagDef.SetDebugNames("Dead Ragdoll Transition","RagdollContinuationTransitionDef");
				m_entity->GetMovement()->BringInNewController( obRagDef, CMovement::DMM_STANDARD, 0.0f ) ;

				// 1. be sure that ragdolls is activated
				// 2. reduce velocities. This is a bit tricky, we want to reduce velocities becuase some part
				// of our animation are to fast. Ragdoll are change to DEAD state if it colides during animated state, 
				// and it preserve the velocities from animation. That's not the best solution, so if there
				// will be any problems in transition from animated to dead state, be sure that it is not 
                // done by the following code.

				// This screws up ragdolls that are accelerating under physical forces. Like thrown ragdolls.
				if ( !GetRagdollThrown() )
				{
					const hkArray<hkRigidBody *>& bodies = m_RagdollInstance->getRigidBodyArray();
					float maxVel = 0; 
					for(int i = 0; i < bodies.getSize(); i++)
					{
						bodies[i]->activate(); 
						float vel = bodies[i]->getLinearVelocity().lengthSquared3();
						maxVel = max(maxVel, vel);
					}

					if (maxVel > MAX_VEL_FOR_DEAD_RAGDOLL * MAX_VEL_FOR_DEAD_RAGDOLL)
					{
						float coef = MAX_VEL_FOR_DEAD_RAGDOLL / sqrtf(maxVel);
						for(int i = 0; i < bodies.getSize(); i++)
						{
							hkRigidBody * body = bodies[i]; 
							hkVector4 temp;
	
							temp.setMul4(coef, body->getAngularVelocity());
							body->setAngularVelocity(temp);
	
							temp.setMul4(coef, body->getLinearVelocity());
							body->setLinearVelocity(temp);
						}
					}
				}

				break;
			}
		}
#endif
	}

	
	
	//------------------------------------------------------------------------------------------
	//
	// Function: AdvancedRagdoll::FlagAtRest
	// Author: SCEE_SWRIGHT (22/11/2006)
	//
	//------------------------------------------------------------------------------------------
	void AdvancedRagdoll::FlagAtRest( void )
	{
		if (m_pcAtRestMsg != 0)
		{
			CMessageSender::SendEmptyMessage(m_pcAtRestMsg, m_entity->GetMessageHandler());
			m_pcAtRestMsg = 0;
		}
	}
	
	
	
	//------------------------------------------------------------------------------------------
	//
	// Function: AdvancedRagdoll::SetRagdollTrajectory
	// Author: SCEE_SWRIGHT (21/8/2006)
	//
	//------------------------------------------------------------------------------------------
	void AdvancedRagdoll::SetRagdollTrajectory( float fDistMultiplier, float fAngleMultiplier )	
	{ 
		SetRagdollThrown( true ); 
		m_fForceDistanceMultiplier = fDistMultiplier; 
		m_fAngleDistanceMultiplier = fAngleMultiplier; 
	}

	
	
	//------------------------------------------------------------------------------------------
	//
	// Function: AdvancedRagdoll::SetRagdollThrown
	// Author: SCEE_SWRIGHT (15/8/2006)
	//
	//------------------------------------------------------------------------------------------
	void AdvancedRagdoll::SetRagdollThrown( bool bThrown )			
	{ 
		m_bRagdollIsThrown = bThrown; 
		
		if ( bThrown ) 
		{
			m_entity->GetPhysicsSystem()->GetCollisionStrikeHandler()->Enable(); 
		}
		else
		{
			m_entity->GetPhysicsSystem()->GetCollisionStrikeHandler()->Disable(); 
			m_entity->GetPhysicsSystem()->GetCollisionStrikeHandler()->SetStrikeFilter(Physics::CollisionStrikeHandler::ENEMY  | Physics::CollisionStrikeHandler::FRIENDLY); 
		}
	}
	
	
	
	//------------------------------------------------------------------------------------------
	//
	// Function: AdvancedRagdoll::AimAtPlayer
	// Author: SCEE_SWRIGHT (15/8/2006)
	//
	//------------------------------------------------------------------------------------------
	void AdvancedRagdoll::AimAtPlayer( void )
	{
		#define MAX_Y_TRAJECTORY		( 0.2f )  
		   
		// Cancel out the pelvis' velocity inherited from the animation.
		hkVector4 obZero( 0.0f, 0.0f, 0.0f );
		for (int i=0; i < m_RagdollInstance->getNumBones(); i++)
		{
			hkRigidBody *pobBody = m_RagdollInstance->getRigidBodyOfBone(i);
			pobBody->setLinearVelocityAsCriticalOperation(obZero); 
			pobBody->setAngularVelocityAsCriticalOperation(obZero);
		}
		
		// Cache the position at the start of the throw.
		//m_ptThrowOrigin = GetEntity()->GetPosition();
		
		// Get the player position.
		Player* pobPlayer = CEntityManager::Get().GetPlayer();
		if ( pobPlayer )
		{
			CPoint 	ptPlayerPos = pobPlayer->GetPosition();
			
			// Get the ragdoll's torso position.
			Physics::ReadAccess mutex;
			hkRigidBody *pobTorso 	= m_RagdollInstance->getRigidBodyOfBone(SPINE_BONE);
			hkVector4 	obTorsoPos  = pobTorso->getPosition();
			CPoint ptTorsoPos( obTorsoPos(0), obTorsoPos(1), obTorsoPos(2) );
	
			// Calculate impulse angle and magnitude based on distance?
			CDirection vImpulse = (CDirection)( ptPlayerPos - ptTorsoPos );
			float fDistance = vImpulse.Length();
			vImpulse.Normalise();
			vImpulse.Y() =  min( vImpulse.Y() + ( fDistance * m_fAngleDistanceMultiplier ), MAX_Y_TRAJECTORY ); 
			
			// Scale impulse magnitude with distance to player.
			vImpulse *= ( fDistance * m_fForceDistanceMultiplier );
			
			// Adjust impulse strenght for non-default masses.
			user_warn_p( pobTorso->getMass() > 2.0f, ( "This ragdoll is very light! Use the correct ragdoll setup please." ) );
			if ( pobTorso->getMass() > 2.0f )
			{
				vImpulse *= 6.0f; 
			}
			
			// Apply impulse to the ragdoll torso.
			hkVector4 obImpulse( vImpulse.X(), vImpulse.Y(), vImpulse.Z() );
			pobTorso->applyLinearImpulseAsCriticalOperation( obImpulse );
		}
	}
	
	
	
	//------------------------------------------------------------------------------------------
	//
	// Function: AdvancedRagdoll::SetRagdollThrowProperties
	// Author: SCEE_SWRIGHT (1/11/2006)
	//
	//------------------------------------------------------------------------------------------
	void AdvancedRagdoll::SetRagdollHeld( bool bHeld )
	{
		if ( m_bRagdollIsHeld != bHeld )
		{
			// Get the waist joint's (ragdoll) constraint.
			hkConstraintInstance* pobWaist = m_RagdollInstance->getConstraintArray()[WAIST_JOINT];
			ntAssert_p( pobWaist->getData()->getType() == hkConstraintData::CONSTRAINT_TYPE_RAGDOLL, ("Expecting waist joint to be a ragdoll constraint!") );
			
			// Get the constraint's limit data.
			hkRagdollConstraintData *pobWaistJointData = (hkRagdollConstraintData*)( pobWaist->getData() );
			float fWaistJointConeAngle = pobWaistJointData->getConeAngularLimit();
			user_warn_p( fabsf( pobWaistJointData->getTwistMinAngularLimit() ) == fabsf( pobWaistJointData->getTwistMaxAngularLimit() ), ("Entity %s's ragdoll has non-symmetrical twist limits on it's waist joint.\n", m_entity->GetName().c_str()) );
			user_warn_p( fabsf( pobWaistJointData->getPlaneMinAngularLimit() ) == fabsf( pobWaistJointData->getPlaneMaxAngularLimit() ), ("Entity %s's ragdoll has non-symmetrical plane angles on it's waist joint.\n", m_entity->GetName().c_str()) );
				
			if ( bHeld == true )
			{
				// Cache the current wait constraint limits.
				m_fWaistPlaneAngle = pobWaistJointData->getPlaneMaxAngularLimit();
				m_fWaistTwistAngle = pobWaistJointData->getTwistMaxAngularLimit();
				
				// Reduce waist freedom to prevent jitteriness while held.
				pobWaistJointData->setConeAngularLimit( fWaistJointConeAngle / 2.0f );
				pobWaistJointData->setPlaneMaxAngularLimit(-0.01f);
				pobWaistJointData->setPlaneMinAngularLimit(0.01f);
				pobWaistJointData->setTwistMinAngularLimit(-0.01f);
				pobWaistJointData->setTwistMaxAngularLimit(0.01f);
			}
			else
			{
				// Restore original waist configuration.
				pobWaistJointData->setConeAngularLimit( fWaistJointConeAngle * 2.0f );
				pobWaistJointData->setPlaneMaxAngularLimit(m_fWaistPlaneAngle);
				pobWaistJointData->setPlaneMinAngularLimit(-m_fWaistPlaneAngle);
				pobWaistJointData->setTwistMaxAngularLimit(m_fWaistTwistAngle);
				pobWaistJointData->setTwistMinAngularLimit(-m_fWaistTwistAngle);
			}
			
			m_bRagdollIsHeld = bHeld;
		}
	}
	
	
	
	
	void AdvancedRagdoll::SetCharacterDead(bool dead)
	{
		Physics::WriteAccess mutex;

		if (dead)
			m_entity->GetMovement()->ClearControllers();

		for(int iRigid = 0; iRigid< m_RagdollInstance->getNumBones(); ++iRigid)
		{
			hkRigidBody * body = m_RagdollInstance->getRigidBodyOfBone( iRigid );
			RagdollCollisionFlag obFlag;
			
			obFlag.base = body->getCollisionFilterInfo();
			obFlag.flags.character_dead = dead ? 1 : 0;
			body->getCollidableRw()->setCollisionFilterInfo(obFlag.base);

			if (body->getWorld())
				body->getWorld()->updateCollisionFilterOnEntity( body, HK_UPDATE_FILTER_ON_ENTITY_FULL_CHECK , HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS);   
		}
	}

	void AdvancedRagdoll::UpdateCollisionFilter()
	{
		for(int iRigid = 0; iRigid< m_RagdollInstance->getNumBones(); ++iRigid)
		{
			if (m_RagdollInstance->getRigidBodyOfBone(iRigid)->getWorld())
				m_RagdollInstance->getRigidBodyOfBone(iRigid)->getWorld()->updateCollisionFilterOnEntity( m_RagdollInstance->getRigidBodyOfBone(iRigid), HK_UPDATE_FILTER_ON_ENTITY_FULL_CHECK , HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS);   
		}
	}

	void AdvancedRagdoll::SetKOState(KO_States state)
	{
		Physics::WriteAccess mutex;

		for(int iRigid = 0; iRigid< m_RagdollInstance->getNumBones(); ++iRigid)
		{
			hkRigidBody * body = m_RagdollInstance->getRigidBodyOfBone( iRigid );
			RagdollCollisionFlag obFlag;
			obFlag.base = body->getCollidable( )->getCollisionFilterInfo();
			obFlag.flags.i_am_in_KO_state = state;
			body->getCollidableRw()->setCollisionFilterInfo(obFlag.base);
			if (body->getWorld())
				body->getWorld()->updateCollisionFilterOnEntity( body, HK_UPDATE_FILTER_ON_ENTITY_FULL_CHECK , HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS);   
		}
	}

	
	/*bool AdvancedRagdoll::CheckForStaticGeometry(hkAllRayHitCollector& obRayCollector, hkVector4& obRayContactNormal, float& fDistanceFrom)
	{
		Physics::WriteAccess mutex;

		// Go through all hits, and if any are fixed RBs then AHHH
		for ( int iHit = 0; iHit < obRayCollector.getHits().getSize(); iHit++ )
		{
			// Get the shape that collided
			const hkCollidable* pobCollided = obRayCollector.getHits()[iHit].m_rootCollidable;
			
			// If we have a rigid body...
			hkRigidBody* pobRB = hkGetRigidBody( pobCollided );

			if (
				(pobCollided->getShape()->getType() == HK_SHAPE_MOPP || 
				pobCollided->getShape()->getType() == HK_SHAPE_BV_TREE ||
				pobCollided->getShape()->getType() == HK_SHAPE_BV) // We have static level geom shapes
				||
				(pobRB && !(pobRB->getMass() > 0.0f)) // We have a fixed rigid body of some form
				)
			{
				// There's something fixed between my bones
				obRayContactNormal = obRayCollector.getHits()[iHit].m_normal;
				fDistanceFrom = obRayCollector.getHits()[iHit].m_hitFraction;
				return true;
			}
		}

		return false;
	}*/

	/*void AdvancedRagdoll::CheckBetweenBonesAndApplyCorrection(hkRigidBody* pobBone1, hkRigidBody* pobBone2, hkAabbPhantom* pobPhantom)
	{
		hkAllRayHitCollector obRayCollector;
		hkWorldRayCastInput obRayInput;

		obRayInput.m_from = pobBone1->getPosition();
		obRayInput.m_to = pobBone2->getPosition();
		pobPhantom->castRay(obRayInput,obRayCollector);

		bool bHasHit = false;
		for ( int iHit = 0; iHit < obRayCollector.getHits().getSize(); iHit++ )
		{
			hkRigidBody* pobBody = hkGetRigidBody(obRayCollector.getHits()[iHit].m_rootCollidable);
			if (pobBody && (pobBody == pobBone1 || pobBody == pobBone2))
			{
				bHasHit |= false; // This is ourselves
			}
			else
			{
				bHasHit |= true; // This is something we need to deal with
			}
		}

		// If there's a hit between bones
		if (bHasHit)
		{				
			// Move the ragdoll a little towards the player
			CEntity* pobPlayer = CEntityManager::Get().GetPlayer();
			CPoint obBone1Position = Physics::MathsTools::hkVectorToCPoint(pobBone1->getPosition());
			CPoint obBone2Position = Physics::MathsTools::hkVectorToCPoint(pobBone2->getPosition());
			CPoint obPlayerPosition = pobPlayer->GetPosition();
			CDirection obBoneToPlayer(obPlayerPosition - obBone1Position);
			
			// If we're below/quite close to the floor
			if (obBone1Position.Y() < obPlayerPosition.Y() + 0.25f)
			{
				// Get some kind of half vector by combining with an up vector
				obBoneToPlayer.Normalise();
				obBoneToPlayer.Y() += 1.0f;
				obBoneToPlayer *= 0.5f;
				obBoneToPlayer.Normalise();
			}
			obBoneToPlayer *= 0.2f; // Start with a relatively big movement

			// Move the bones this far then check again
			obBone1Position += obBoneToPlayer;
			obBone2Position += obBoneToPlayer;
			pobBone1->setPosition(Physics::MathsTools::CPointTohkVector(obBone1Position));
			pobBone2->setPosition(Physics::MathsTools::CPointTohkVector(obBone2Position));

			// While we're still penetrating something, move it a little bit more
			while (TestBonePenetration(pobBone1))
			{
				obBone1Position += obBoneToPlayer;
				pobBone1->setPosition(Physics::MathsTools::CPointTohkVector(obBone1Position));
			}
			while (TestBonePenetration(pobBone2))
			{
				obBone2Position += obBoneToPlayer;
				pobBone2->setPosition(Physics::MathsTools::CPointTohkVector(obBone2Position));
			}
		}
	}*/

	/*void AdvancedRagdoll::CheckBetweenEachBoneAndApplyCorrection()
	{
		Physics::WriteAccess mutex;

		// Quick and dirty havok check first
		for (int i = 1; i < m_RagdollInstance->getNumBones(); i++)
		{
			hkConstraintUtils::checkAndFixConstraint(m_RagdollInstance->getConstraintOfBone(i)); 
		}

		// Get an AABB that surrounds the ragdoll
		hkVector4 obAvg(0,0,0,0);
		for (int bone=0; bone<m_RagdollInstance->getNumBones(); bone++)
		{
			hkRigidBody* rb = m_RagdollInstance->getRigidBodyOfBone(bone);
			hkVector4 obPosition = rb->getPosition();
			obAvg.add4(obPosition);
		}
		obAvg.mul4(1.0f/m_RagdollInstance->getNumBones());

		hkVector4 min(-3.0,-3.0,-3.0);
		hkVector4 max(3.0,3.0,3.0);
		min.add4(obAvg);
		max.add4(obAvg);
		hkAabb obAabb(min,max);

		// Create an hkAabbPhantom with it and use it for bone to bone raycasting
		hkAabbPhantom obRagdollPhantom(obAabb);
		obRagdollPhantom.setAabb(obAabb);
		CPhysicsWorld::Get().GetHavokWorldP()->addPhantom(&obRagdollPhantom);
	
		CheckBetweenBonesAndApplyCorrection(m_RagdollInstance->getRigidBodyOfBone(SPINE_BONE),m_RagdollInstance->getRigidBodyOfBone(HEAD_BONE),&obRagdollPhantom);
		CheckBetweenBonesAndApplyCorrection(m_RagdollInstance->getRigidBodyOfBone(PELVIS_BONE),m_RagdollInstance->getRigidBodyOfBone(SPINE_BONE),&obRagdollPhantom);
		CheckBetweenBonesAndApplyCorrection(m_RagdollInstance->getRigidBodyOfBone(PELVIS_BONE),m_RagdollInstance->getRigidBodyOfBone(L_LEG_BONE),&obRagdollPhantom);
		CheckBetweenBonesAndApplyCorrection(m_RagdollInstance->getRigidBodyOfBone(PELVIS_BONE),m_RagdollInstance->getRigidBodyOfBone(R_LEG_BONE),&obRagdollPhantom);
		CheckBetweenBonesAndApplyCorrection(m_RagdollInstance->getRigidBodyOfBone(R_LEG_BONE),m_RagdollInstance->getRigidBodyOfBone(R_KNEE_BONE),&obRagdollPhantom);
		CheckBetweenBonesAndApplyCorrection(m_RagdollInstance->getRigidBodyOfBone(L_LEG_BONE),m_RagdollInstance->getRigidBodyOfBone(L_KNEE_BONE),&obRagdollPhantom);
		CheckBetweenBonesAndApplyCorrection(m_RagdollInstance->getRigidBodyOfBone(SPINE_BONE),m_RagdollInstance->getRigidBodyOfBone(L_FORARM_BONE),&obRagdollPhantom);
		CheckBetweenBonesAndApplyCorrection(m_RagdollInstance->getRigidBodyOfBone(SPINE_BONE),m_RagdollInstance->getRigidBodyOfBone(R_FORARM_BONE),&obRagdollPhantom);
		CheckBetweenBonesAndApplyCorrection(m_RagdollInstance->getRigidBodyOfBone(R_FORARM_BONE),m_RagdollInstance->getRigidBodyOfBone(R_ELBOW_BONE),&obRagdollPhantom);
		CheckBetweenBonesAndApplyCorrection(m_RagdollInstance->getRigidBodyOfBone(L_FORARM_BONE),m_RagdollInstance->getRigidBodyOfBone(L_ELBOW_BONE),&obRagdollPhantom);

		CPhysicsWorld::Get().GetHavokWorldP()->removePhantom(&obRagdollPhantom);
	}*/

	/*bool AdvancedRagdoll::TestBonePenetration(hkRigidBody* pobBone)
	{
		Physics::WriteAccess mutex;

		hkClosestCdPointCollector collec;
		CPhysicsWorld::Get().GetClosestPoints( pobBone->getCollidable(), (const hkCollisionInput &)*CPhysicsWorld::Get().GetCollisionInput(), collec );

		// If we got a contact...
		if ( collec.hasHit() )
		{
			// ...is it penetrating?
			if (collec.getHitContact().getDistance() < 0)
			{
				return true;
			}
		}				

		return false;
	}*/

	/*bool AdvancedRagdoll::TestPenetrations()
	{
		Physics::WriteAccess mutex;

		// Get our closest contact for each bone
		//hkSimpleClosestContactCollector collec;
		hkClosestCdPointCollector collec;
		for (int i=0; i < m_RagdollInstance->getNumBones(); i++)
		{
			hkRigidBody* rb = m_RagdollInstance->getRigidBodyOfBone(i);
			Physics::WriteAccess mutex;

			collec.reset();
			CPhysicsWorld::Get().GetClosestPoints( rb->getCollidable(), (const hkCollisionInput &)*CPhysicsWorld::Get().GetCollisionInput(), collec );

			// If we got a contact...
			if ( collec.hasHit() )
			{
				hkVector4 n = collec.getHitContact().getNormal();

				// ...is it penetrating more than we set havok to allow?
				if (collec.getHitContact().getDistance() < 0.01f)
				{
					// MOVE IT AWAY!
					//hkVector4 obRBVelocity = rb->getLinearVelocity();
					hkVector4 obAway(0,0,0,0);						
					obAway = collec.getHitContact().getNormal();
					float fMovementDistance = 0.3f;

					//if (i == L_KNEE_BONE || i == R_KNEE_BONE)
					//{
					//	obAway = hkVector4(0.0f,1.0f,0.0f,0.0f);
					//	fMovementDistance = 0.5f;
					//}

					// Never correct downwards
					if (obAway(1) < 0.0f)
					{
						//ntPrintf("Ragdoll tried to correct downwards! Flipped it upwards.\n");
						obAway(1) *= -1;
					}
					
					obAway.normalize3();
					obAway.mul4(fMovementDistance); 
					// Move every bone by this amount
					for (int j=0; j < m_RagdollInstance->getNumBones(); j++)
					{
						hkVector4 obNewPosition = m_RagdollInstance->getRigidBodyOfBone(j)->getPosition();
						obNewPosition.add4(obAway);
						m_RagdollInstance->getRigidBodyOfBone(j)->setPosition(obNewPosition); // Push us away at once, in one frame we should cover the distance we need to
						//m_RagdollInstance->getRigidBodyOfBone(j)->setLinearVelocity(obAway);
					}

					//for (int i = 1; i < m_RagdollInstance->getNumBones(); i++)
					//{
					//	hkConstraintUtils::checkAndFixConstraint(m_RagdollInstance->getConstraintOfBone(i)); 
					//}

					//ntPrintf("Ragdoll penetration corrected.\n");

					return true;
				}
			}
		}		

		return false;
	}*/
	
	/*bool AdvancedRagdoll::CheckAllBonePenetrationsAndApplyCorrection()
	{
		Physics::WriteAccess mutex;

		bool bRet = false;
		
		for(int iRigid = 0; iRigid< m_RagdollInstance->getNumBones(); ++iRigid)
		{
			bRet = bRet || CheckBonePenetrationsAndApplyCorrection(m_RagdollInstance->getRigidBodyOfBone( iRigid ));
		}

		return bRet;
	}*/
	
	/*bool AdvancedRagdoll::CheckBonePenetrationsAndApplyCorrection(hkRigidBody* rbody)
	{
		Physics::WriteAccess mutex;

		// Our return value
		bool bSkipThisBone = false;

		// Check for collisions per bone
		// If this bone is pentrating something then DON'T force it anywhere

		hkAllCdPointCollector obCollector;
		obCollector.reset();
		CPhysicsWorld::Get().GetClosestPoints( rbody->getCollidable(), (const hkCollisionInput &)*CPhysicsWorld::Get().GetCollisionInput(), obCollector );

		// If we got contacts...
		if ( obCollector.getHits().getSize() > 0 )
		{
			// Go through each contact...
			for (int c = 0; c < obCollector.getHits().getSize(); c++)
			{
				// If it's penetrating...
				if (obCollector.getHits()[c].m_contact.getDistance() < 0)
				{
					hkRigidBody* pobBody = hkGetRigidBody(obCollector.getHits()[c].m_rootCollidableB);
					if (pobBody)
					{
						EntityCollisionFlag infoB;
						infoB.base = pobBody->getCollidable()->getCollisionFilterInfo();

						// If it's a ragdoll, it's most likely to be us so ignore it, else...
						if (infoB.flags.i_am & RAGDOLL_BIT)
						{
							bSkipThisBone = false;
						}
						else if (pobBody->getMass() <= 0.0f)
						{
							// Skip it.
							bSkipThisBone = true;
							// Now we need to move it out of the world 
							hkVector4 obNewPosition, obDirectionToMove;
							obDirectionToMove = obCollector.getHits()[c].m_contact.getNormal();
							obDirectionToMove.mul4( fabsf(obCollector.getHits()[c].m_contact.getDistance()) );
							obNewPosition = rbody->getPosition();
							obNewPosition.add4(obDirectionToMove);
							rbody->setPosition(obNewPosition);
						}
					}
				}						
			}
		}

		return bSkipThisBone;
	}*/
	

	/*void AdvancedRagdoll::FeetCollide( bool mode )
	{
		Physics::WriteAccess mutex;

		Physics::RagdollCollisionFlag iRagFlagLeft; iRagFlagLeft.base = m_RagdollInstance->getRigidBodyOfBone(L_KNEE_BONE)->getCollidable()->getCollisionFilterInfo();
		Physics::RagdollCollisionFlag iRagFlagRight; iRagFlagRight.base = m_RagdollInstance->getRigidBodyOfBone(R_KNEE_BONE)->getCollidable()->getCollisionFilterInfo();

		if( mode )
		{			
			// I collide with
			iRagFlagLeft.flags.i_collide_with = (	Physics::CHARACTER_CONTROLLER_PLAYER_BIT	| 
													Physics::CHARACTER_CONTROLLER_ENEMY_BIT	|
													Physics::RAGDOLL_BIT						|
													Physics::SMALL_INTERACTABLE_BIT				|
													Physics::LARGE_INTERACTABLE_BIT				|
													Physics::TRIGGER_VOLUME_BIT					|
													Physics::AI_WALL_BIT);
			iRagFlagRight.flags.i_collide_with = iRagFlagLeft.flags.i_collide_with;

		} 
		else 
		{			
			// I collide with nobody			
			iRagFlagLeft.flags.i_collide_with = ( 0 );
			iRagFlagRight.flags.i_collide_with = ( 0 );

		}
		
		m_RagdollInstance->getRigidBodyOfBone(L_KNEE_BONE)->getCollidableRw()->setCollisionFilterInfo(iRagFlagLeft.base);
		m_RagdollInstance->getRigidBodyOfBone(R_KNEE_BONE)->getCollidableRw()->setCollisionFilterInfo(iRagFlagRight.base);

		m_RagdollInstance->getRigidBodyOfBone(L_KNEE_BONE)->getWorld()->updateCollisionFilterOnEntity(m_RagdollInstance->getRigidBodyOfBone(L_KNEE_BONE), HK_UPDATE_FILTER_ON_ENTITY_FULL_CHECK, HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS);		
		m_RagdollInstance->getRigidBodyOfBone(R_KNEE_BONE)->getWorld()->updateCollisionFilterOnEntity(m_RagdollInstance->getRigidBodyOfBone(R_KNEE_BONE), HK_UPDATE_FILTER_ON_ENTITY_FULL_CHECK, HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS);
	}*/

	/*void AdvancedRagdoll::CollideAtAll( bool bCollide )
	{
		Physics::WriteAccess mutex;

		for (int i = 0; i < m_RagdollInstance->getNumBones(); i++)
		{
			Physics::RagdollCollisionFlag iRagFlag; 
			iRagFlag.base = 0;
			if( bCollide )
			{
				// Everything
				iRagFlag.flags.i_am |= Physics::RAGDOLL_BIT;
				iRagFlag.flags.i_collide_with = (	Physics::CHARACTER_CONTROLLER_PLAYER_BIT	| 
													Physics::CHARACTER_CONTROLLER_ENEMY_BIT		|
													Physics::RAGDOLL_BIT						|
													Physics::SMALL_INTERACTABLE_BIT				|
													Physics::LARGE_INTERACTABLE_BIT				|
													Physics::TRIGGER_VOLUME_BIT					|
													Physics::AI_WALL_BIT);

			} 
			else 
			{
				// Nothing
				iRagFlag.flags.i_am |= Physics::RAGDOLL_BIT;
				iRagFlag.flags.i_collide_with = 0;
			}

			if (i == L_KNEE_BONE)
				iRagFlag.flags.ragdoll_material = Physics::RAGDOLL_L_KNEE_MATERIAL;
			else if (i == R_KNEE_BONE)
				iRagFlag.flags.ragdoll_material = Physics::RAGDOLL_R_KNEE_MATERIAL;
			else if (i == L_LEG_BONE)
				iRagFlag.flags.ragdoll_material = Physics::RAGDOLL_L_LEG_MATERIAL;
			else if (i == R_LEG_BONE)
				iRagFlag.flags.ragdoll_material = Physics::RAGDOLL_R_LEG_MATERIAL;
			else if (i == L_FORARM_BONE)
				iRagFlag.flags.ragdoll_material = Physics::RAGDOLL_L_ARM_MATERIAL;
			else if (i == R_FORARM_BONE)
				iRagFlag.flags.ragdoll_material = Physics::RAGDOLL_R_ARM_MATERIAL;
			else if (i == L_ELBOW_BONE)
				iRagFlag.flags.ragdoll_material = Physics::RAGDOLL_L_ELBOW_MATERIAL;
			else if (i == R_ELBOW_BONE)
				iRagFlag.flags.ragdoll_material = Physics::RAGDOLL_R_ELBOW_MATERIAL;
			else if (i == HEAD_BONE)
				iRagFlag.flags.ragdoll_material = Physics::RAGDOLL_HEAD_MATERIAL;
			else if (i == SPINE_BONE)
				iRagFlag.flags.ragdoll_material = Physics::RAGDOLL_SPINE_00_MATERIAL;
			else if (i == PELVIS_BONE)
				iRagFlag.flags.ragdoll_material = Physics::RAGDOLL_PELVIS_MATERIAL;

			m_RagdollInstance->getRigidBodyOfBone(i)->getCollidableRw()->setCollisionFilterInfo(iRagFlag.base);			
			m_RagdollInstance->getRigidBodyOfBone(i)->getWorld()->updateCollisionFilterOnEntity(m_RagdollInstance->getRigidBodyOfBone(L_KNEE_BONE), HK_UPDATE_FILTER_ON_ENTITY_FULL_CHECK, HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS);
		}
	}*/

	void AdvancedRagdoll::SetBoneBoneCollision( bool bCollide )
	{
		Physics::WriteAccess mutex;

		for (int i = 0; i < m_RagdollInstance->getNumBones(); i++)
		{		
			hkRigidBody * body = m_RagdollInstance->getRigidBodyOfBone(i);
			Physics::RagdollCollisionFlag iRagFlag; iRagFlag.base = body->getCollidable()->getCollisionFilterInfo();

			if (bCollide)
				iRagFlag.flags.i_collide_with |= Physics::RAGDOLL_BIT;
			else				
				iRagFlag.flags.i_collide_with &= ~Physics::RAGDOLL_BIT;

			body->getCollidableRw()->setCollisionFilterInfo(iRagFlag.base);
			body->getWorld()->updateCollisionFilterOnEntity(body, HK_UPDATE_FILTER_ON_ENTITY_FULL_CHECK, HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS);			
		}
	}

	RAGDOLL_STATE AdvancedRagdoll::GetState( )
	{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		return m_currentState;
#else
		return ALIVE;
#endif
	}

	
	void AdvancedRagdoll::Twitch()
	{
		Physics::WriteAccess mutex;

		m_RagdollInstance->getRigidBodyOfBone(HEAD_BONE)->setAngularVelocity(hkVector4(grandf(1.0f)-0.5f,grandf(1.0f)-0.5f,grandf(1.0f)-0.5f));
		m_RagdollInstance->getRigidBodyOfBone(L_ELBOW_BONE)->setAngularVelocity(hkVector4(grandf(1.0f)-0.5f,grandf(1.0f)-0.5f,grandf(1.0f)-0.5f));
		m_RagdollInstance->getRigidBodyOfBone(R_ELBOW_BONE)->setAngularVelocity(hkVector4(grandf(1.0f)-0.5f,grandf(1.0f)-0.5f,grandf(1.0f)-0.5f));
		m_RagdollInstance->getRigidBodyOfBone(L_KNEE_BONE)->setAngularVelocity(hkVector4(grandf(1.0f)-0.5f,grandf(1.0f)-0.5f,grandf(1.0f)-0.5f));
		m_RagdollInstance->getRigidBodyOfBone(R_KNEE_BONE)->setAngularVelocity(hkVector4(grandf(1.0f)-0.5f,grandf(1.0f)-0.5f,grandf(1.0f)-0.5f));
	}

	
	
	void AdvancedRagdoll::Update( float fTimeDelta, CPoint& obGoalTranslation, CQuat& obGoalOrientation, WS_SYNC_TYPE eSynchronised, bool bUpdateEntityTransform )
	{
		GATSO_PHYSICS_START("***RAGDOLL");
		
		// If we're permanently deactivated (from culling), return here, and kick up a fuss because we should never have got here.
		// Also, bosses should never KO as they die in ninja sequences. blame Duncan if it happens.
		if (m_bPermanentlyDeactivated || m_entity->IsBoss() )
		{
			ntAssert(0);
			return;
		}

		if (m_bIsPhantom && m_bPhantomToDynamicNextUpdate)		
			PhantomToDynamic();

		m_bPhantomToDynamicNextUpdate = false;
		
		// Take care of notifications from collision listener
		if (m_bSetDeadNextUpdate)
		{
			// Need to set a combat killed message to make sure proper state transitions occur.
			Activate(DEAD);	
			m_bSetDeadNextUpdate = false;
		}

		if (m_bDoParticleEffectNextUpdate)
		{
			DoParticleEffect();
			m_bDoParticleEffectNextUpdate = false;
		}

		if (m_bDoSoundNextUpdate)
		{
			DoSound();
			m_bDoSoundNextUpdate = false;
		}

		if( m_bSendRagdollFloored )
		{			
			if (m_entity->GetAttackComponent() && (m_entity->GetAttackComponent()->AI_Access_GetState() == CS_KO))
			{
				CMessageSender::SendEmptyMessage("msg_combat_ragdollfloored", m_entity->GetMessageHandler());
			}
			m_bSendRagdollFloored = false;
		}
		
		// Default motion status. (Cache the previous moving status, so that we can prolong certain updates
		// for a frame after the ragdoll is deactivated.)
		bool bMovingLastFrame = IsMoving();
		m_fCumulativeLinearVelocitySqr = -1.0f; 	// i.e. unset.
		m_bMoving = true; 							// for all ragdoll types other than DEAD.
				
		float fCurrentHavokWorldTime = m_RagdollInstance->getWorld()->getCurrentTime();
		float fTimeOfNextHavokPsi = m_RagdollInstance->getWorld()->getCurrentPsiTime();
		float fHavokWorldTimeAfterThisFrame = fCurrentHavokWorldTime + fTimeDelta;
		bool bHavokWillPSINextFrame = fHavokWorldTimeAfterThisFrame > fTimeOfNextHavokPsi;
		UNUSED(bHavokWillPSINextFrame);

		// Incase we want a sensical root matix at any point, make note of what it is now before we twist it and mess it up by mapping it to the pelvis bone
		m_obWorldMatrixBeforeUpdate = m_entity->GetHierarchy()->GetRootTransform()->GetWorldMatrix();
		
		// Lock the world, we'll be writing lots of new info
		Physics::WriteAccess mutex;


#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		// Update how long we've been active in the current state
		m_fActiveTime += fTimeDelta;

		// Prepare some arrays to hold our poses
		const int ragdollNumBones = m_RagdollInstance->getNumBones();
		hkArray<hkQsTransform> ragdollPose(ragdollNumBones);
		ragdollPose.setSizeUnchecked(ragdollNumBones);		
		const int numBones = m_AnimatedSkeleton->m_numBones;
		hkArray<hkQsTransform> animatedPose(numBones);
		animatedPose.setSizeUnchecked(numBones);

		switch( m_currentState )
		{
		case TRANSFORM_TRACKING: 
			{
				// This will push bones that have a transform associated with them towards that transform
				// Loop through each mapping and, if it's active, push the bone towards it
				for (int i = 0; i < m_RagdollInstance->getNumBones(); i++)
				{
					if (m_aobBoneTransformTracking[i].m_bIsActive)
					{
						// Get the bone and the transform it's supposed to track
						hkRigidBody* pobBone = m_RagdollInstance->getRigidBodyOfBone(i);
						const Transform* pobTransformToTrack = m_aobBoneTransformTracking[i].m_pobTransformToTrack;

						// Get where it is now
						hkTransform obBoneTransform;
						pobBone->approxTransformAt(Physics::CPhysicsWorld::Get().GetFrameTime(),obBoneTransform);
						hkQuaternion obPelvisRotation(obBoneTransform.getRotation());
						
						// Use the useful havok utility function to get how much we need to rotate
						hkVector4 obBoneRotationNeeded;
						obPelvisRotation.estimateAngleTo(Physics::MathsTools::CQuatTohkQuaternion(CQuat(pobTransformToTrack->GetWorldMatrix())), obBoneRotationNeeded);
						obBoneRotationNeeded.mul4(1/fTimeDelta);
						pobBone->setAngularVelocity(obBoneRotationNeeded);
						
						// Get a movement vector for translation
						hkVector4 obBoneTranslationNeeded = Physics::MathsTools::CPointTohkVector(pobTransformToTrack->GetWorldMatrix().GetTranslation());
						obBoneTranslationNeeded.sub4(obBoneTransform.getTranslation());
						obBoneTranslationNeeded.mul4(1/fTimeDelta);
						pobBone->setLinearVelocity(obBoneTranslationNeeded);
					}	
				}

				// Synchronise skeletons.
				m_RagdollInstance->getPoseModelSpace( ragdollPose.begin(), hkQsTransform::getIdentity() );					
				HierarchyTools::GetBindPoseRecursive( animatedPose,  m_entity->GetHierarchy(),  m_entity->GetHierarchy()->GetRootTransform()->GetFirstChild() );
				AnimationPelvisMappedAsRoot( ragdollPose, animatedPose );

				// Update ideal entity root matrix - this isn't ideal at all, it's just where we are
				m_obIdealEntityRootMatrix = m_entity->GetHierarchy()->GetRootTransform()->GetWorldMatrix();

				break;
			}
			
			
		case TRANSFORM_TRACKING_ANIMATED:
			{
				// This is very similar to normal animation though we use a different hierarchy overwriting method and we don't update the root explicitly
				// This works assuming that we have been reparented onto a transform on another entity which is itself animated
				// E.g. in ragdoll pickups and holds, the ragdoll is reparented to the r_weapon transform on the hero - this transform is then animated by 
				// the hero entity, and we follow it by setting this state
				HierarchyTools::GetWorldPoseRecursive(animatedPose, m_entity->GetHierarchy(), m_entity->GetHierarchy()->GetRootTransform()->GetFirstChild());

				// Map the pose of the current animation onto our low-res ragdoll skeleton
				m_AnimatedToRagdollMapper->mapPose(	animatedPose.begin(), 
													m_RagdollInstance->getSkeleton()->m_referencePose, 
													ragdollPose.begin(), hkSkeletonMapper::CURRENT_POSE );

				// This will take care of local space orientations etc, but overall translations we take care of below
				DriveRagdollBonesToAnimation( ragdollPose, m_aiBonesToAnimate, fTimeDelta );

				// Get how far the hierarchy pelvis is from the ragdoll pelvis, and push it there
				CMatrix obHierarchyPelvis = m_entity->GetHierarchy()->GetRootTransform()->GetFirstChild()->GetWorldMatrix();
				hkVector4 obPelvisTranslationNeeded(Physics::MathsTools::CPointTohkVector(obHierarchyPelvis.GetTranslation()));
				hkTransform obPelvisTransform;
				m_RagdollInstance->getRigidBodyOfBone(PELVIS)->approxTransformAt(Physics::CPhysicsWorld::Get().GetFrameTime(),obPelvisTransform);
				hkVector4 obPelvisTranslation(obPelvisTransform.getTranslation());
				obPelvisTranslationNeeded.sub4(obPelvisTranslation);
				obPelvisTranslationNeeded.mul4(1.0f/fTimeDelta);
				m_RagdollInstance->getRigidBodyOfBone(PELVIS)->setLinearVelocity(obPelvisTranslationNeeded);

				// Update the ideal matrix
				m_obIdealEntityRootMatrix = m_obWorldMatrixBeforeUpdate;
				m_obIdealEntityRootMatrix.SetTranslation( obGoalTranslation );

				m_RagdollInstance->getPoseModelSpace( ragdollPose.begin(), hkQsTransform::getIdentity() );
				HierarchyTools::GetLocalPoseRecursive(  animatedPose,  m_entity->GetHierarchy(),  m_entity->GetHierarchy()->GetRootTransform()->GetFirstChild() );
				TransformTrackingAnimation( ragdollPose, animatedPose );

				break;
			}
			
			
		case ANIMATED_ABSOLUTE:
			{
				// This will set the positions of the bones and root according to the current animated pose - no velocities, so it's bad for the solver
				// Don't use unless the physical animation mode doesn't look at all right, unless this is used in safe places, you will get bones in walls
				ResetRagdollPoseToMatchCurrentHierarchy();
				
				CMatrix obNewRootMatrix( m_obWorldMatrixBeforeUpdate );
				obNewRootMatrix.SetTranslation(obGoalTranslation);
				m_entity->GetHierarchy()->GetRootTransform()->SetLocalMatrix( obNewRootMatrix );
				m_obIdealEntityRootMatrix = obNewRootMatrix;

				break;
			}
			
			
		case ANIMATED:
			{	
				GATSO_PHYSICS_START("Ragdoll::UpdateAnimated");
				// This will use velocities to push bones and the whole skeleton according to the current hierarchy
				// Much safer and better than the absolute animation mode

				m_obIdealEntityRootMatrix = CMatrix( obGoalOrientation,obGoalTranslation );
				m_entity->GetHierarchy()->GetRootTransform()->SetLocalMatrixFromWorldMatrix( m_obIdealEntityRootMatrix);

				// Sample the active animations and get a pose
				HierarchyTools::GetWorldPoseRecursive(animatedPose, m_entity->GetHierarchy(), m_entity->GetHierarchy()->GetRootTransform()->GetFirstChild());

				// Map the pose of the current animation onto our low-res ragdoll skeleton
				m_AnimatedToRagdollMapper->mapPose(	animatedPose.begin(), 
													m_RagdollInstance->getSkeleton()->m_referencePose, 
													ragdollPose.begin(), hkSkeletonMapper::CURRENT_POSE ); 

				// Push us towards that local pose
				DriveRagdollBonesToAnimation( ragdollPose, m_aiBonesToAnimate, fTimeDelta );
				
				m_RagdollInstance->getPoseModelSpace( ragdollPose.begin(), hkQsTransform::getIdentity() );
				HierarchyTools::GetLocalPoseRecursive(  animatedPose,  m_entity->GetHierarchy(),  m_entity->GetHierarchy()->GetRootTransform()->GetFirstChild() );
				FullyAnimatedRagdollAnimation( ragdollPose, animatedPose );

				//ntAssert((m_entity->GetHierarchy()->GetRootTransform()->GetWorldTranslation() - obGoalTranslation).LengthSquared() < 0.000001f)				
				GATSO_PHYSICS_STOP("Ragdoll::UpdateAnimated");
				break;
			}			
			
			
		case DEAD:
			{
				GATSO_PHYSICS_START("Ragdoll::UpdateDead");
			    /*CInputKeyboard* pobKeyboard = CInputHardware::Get().GetKeyboardP();
				if ( pobKeyboard->IsKeyPressed( KEYC_L, KEYM_SHIFT ) )
				{
					const hkArray< hkRigidBody * >& bodies = m_RagdollInstance->getRigidBodyArray();

					for(int i = 0; i < bodies.getSize();i++)
					{
						bodies[i]->applyLinearImpulse(hkVector4(0,10,0));
					}
				}*/
				
				// Some initial corrective action.
				/*if (activeTime > 1.0f && !m_bDoneDeadConstraintCheck)
				{
					//CheckBetweenEachBoneAndApplyCorrection();
					//TestPenetrations();
					m_bDoneDeadConstraintCheck = true;
				}*/
				

				// Bones can be fixed by a listener to save CPU, so just break here
				// (i.e. the ragdoll has been phantomised.)
				if (m_RagdollInstance->getRigidBodyOfBone(0)->getMotionType() == hkMotion::MOTION_FIXED)
				{
					// Send at rest message if we haven't already
					FlagAtRest();
					
					// Update the motion status appropriately.
					m_fCumulativeLinearVelocitySqr = 0.0f;
					m_bMoving = false;
			
					GATSO_PHYSICS_STOP("Ragdoll::UpdateDead");
					break;
				}		

				
				// Update timer used to prevent oscillation between dynamic/phantom.
				m_fNonPhantomTime += fTimeDelta;
				
				
				// Update the moving status of the ragdoll, before we make any decisions about being at-rest, 
				// deactivation etc. 
				CalculateMotionStatus();
				
				
				// If character is dead and ragdoll does not know about it, let it know. (We set 'is dead' flags on the 
				// rigid bodies, so that Havok doesn't have to chase up pointers during collision detection.)
				if (m_entity->IsDead())
				{
					RagdollCollisionFlag collFlags; 
					collFlags.base = m_RagdollInstance->getRigidBodyOfBone(0)->getCollisionFilterInfo(); 
					if (!collFlags.flags.character_dead )
					{
						SetCharacterDead(true);
					}
				}

				
				if ( bMovingLastFrame )
				{
					// Only bother with the expensive animation update if we are still moving.
					// Allow an extra frame's update after stopping moving, to allow animation to catch up.
					m_RagdollInstance->getPoseModelSpace( ragdollPose.begin(), hkQsTransform::getIdentity() );					
					HierarchyTools::GetBindPoseRecursive( animatedPose,  m_entity->GetHierarchy(),  m_entity->GetHierarchy()->GetRootTransform()->GetFirstChild() );
					AnimationPelvisMappedAsRoot( ragdollPose, animatedPose );
					m_obIdealEntityRootMatrix = m_entity->GetHierarchy()->GetRootTransform()->GetWorldMatrix();
				}
				else if ( CanBePhantom() ) 
				{
					// If ragdolls are non-interactable, then immediately phantomise any that have come to rest.
					DynamicToPhantom();
				}
				
				
				// Below a certain cumulative-velocity-squared threshold, come to rest.
				float fStationaryThreshold = sqr(0.01f);
				float fAtRestThreshold = g_ShellOptions->CanPickupRagdolls() ? sqr(1.0f) : fStationaryThreshold;
				
				if ( GetMotionStatus() < fAtRestThreshold )
				{
					// Send at rest message if we haven't already
					FlagAtRest();
				}
				
				
				// Update for thrown ragdolls.
				if ( GetRagdollThrown() )
				{
					// Make sure thrown ragdolls are enabled to strike the player.
					m_entity->GetPhysicsSystem()->GetCollisionStrikeHandler()->Enable();
		
					// Reset thrown flag after coming to rest.
					if ( GetMotionStatus() < fAtRestThreshold )
					{
						SetRagdollThrown(false);  
					}
				}
				
				GATSO_PHYSICS_STOP("Ragdoll::UpdateDead");
				break;
			}
			
			
		case DEACTIVATED:		
			break;
		}

		m_obPelvisPositionLastFrame = GetPosition();
#endif

		
#if 0 //The following code will reset position to binding poses. It will places at [0,0,0] coordinates
		CMatrix mat = m_entity->GetHierarchy()->GetRootTransform()->GetWorldMatrix();
		m_entity->GetHierarchy()->ResetToBindPose();
		m_entity->GetHierarchy()->GetRootTransform()->SetLocalMatrixFromWorldMatrix(mat);


		hkPose ragdollPose2(m_RagdollSkeleton);
		ragdollPose2.setToReferencePose();
		m_RagdollInstance->setPoseModelSpace(ragdollPose2.getPoseModelSpace().begin(), MathsTools::CMatrixTohkQsTransform(mat));//hkQsTransform::getIdentity());

		for(int i = 0; i < m_RagdollInstance->getRigidBodyArray().getSize(); i++)
		{
			m_RagdollInstance->getRigidBodyArray()[i]->deactivate();
		}
#endif

		GATSO_PHYSICS_STOP("***RAGDOLL");
	}

	void AdvancedRagdoll::SetupRagdoll( const char* p_filename )
	{
		Physics::WriteAccess mutex;

		LoadRagdoll( p_filename );
		SetupAnimatedSkeleton();
		SetupRigidBodies();
		SetupMappers();
		SetupBoneTransformTrackingArray();
	}

	/*void AdvancedRagdoll::SetupBoneConnectivityGraph()
	{
		// This is done in a way that is seperate from the XML constraint set ups, it's done manually according to indices
		// got in the loading of the ragdoll because that's all we need. We rely on humanoid bipedal skeleton setup.
		// It's possible to have a completely flexible way of doing this whil loading, but for E3 it's really not needed.
		m_aobBoneConnectivityGraph = NT_NEW BoneConnectivity[m_RagdollInstance->getNumBones()];

		m_aobBoneConnectivityGraph[PELVIS_BONE].m_iThisBoneIndex = PELVIS_BONE;
		m_aobBoneConnectivityGraph[PELVIS_BONE].m_aiConnectedToBoneIndices[0] = SPINE_BONE;
		m_aobBoneConnectivityGraph[PELVIS_BONE].m_aiConnectedToBoneIndices[1] = L_LEG_BONE;
		m_aobBoneConnectivityGraph[PELVIS_BONE].m_aiConnectedToBoneIndices[2] = R_LEG_BONE;
		m_aobBoneConnectivityGraph[PELVIS_BONE].m_aiConnectedToBoneIndices[3] = -1;

		m_aobBoneConnectivityGraph[L_LEG_BONE].m_iThisBoneIndex = L_LEG_BONE;
		m_aobBoneConnectivityGraph[L_LEG_BONE].m_aiConnectedToBoneIndices[0] = PELVIS_BONE;
		m_aobBoneConnectivityGraph[L_LEG_BONE].m_aiConnectedToBoneIndices[1] = L_KNEE_BONE;
		m_aobBoneConnectivityGraph[L_LEG_BONE].m_aiConnectedToBoneIndices[2] = -1;
		m_aobBoneConnectivityGraph[L_LEG_BONE].m_aiConnectedToBoneIndices[3] = -1;

		m_aobBoneConnectivityGraph[R_LEG_BONE].m_iThisBoneIndex = R_LEG_BONE;
		m_aobBoneConnectivityGraph[R_LEG_BONE].m_aiConnectedToBoneIndices[0] = PELVIS_BONE;
		m_aobBoneConnectivityGraph[R_LEG_BONE].m_aiConnectedToBoneIndices[1] = R_KNEE_BONE;
		m_aobBoneConnectivityGraph[R_LEG_BONE].m_aiConnectedToBoneIndices[2] = -1;
		m_aobBoneConnectivityGraph[R_LEG_BONE].m_aiConnectedToBoneIndices[3] = -1;

		m_aobBoneConnectivityGraph[SPINE_BONE].m_iThisBoneIndex = SPINE_BONE;
		m_aobBoneConnectivityGraph[SPINE_BONE].m_aiConnectedToBoneIndices[0] = HEAD_BONE;
		m_aobBoneConnectivityGraph[SPINE_BONE].m_aiConnectedToBoneIndices[1] = L_FORARM_BONE;
		m_aobBoneConnectivityGraph[SPINE_BONE].m_aiConnectedToBoneIndices[2] = R_FORARM_BONE;
		m_aobBoneConnectivityGraph[SPINE_BONE].m_aiConnectedToBoneIndices[3] = PELVIS_BONE;

		m_aobBoneConnectivityGraph[L_KNEE_BONE].m_iThisBoneIndex = L_KNEE_BONE;
		m_aobBoneConnectivityGraph[L_KNEE_BONE].m_aiConnectedToBoneIndices[0] = L_LEG_BONE;
		m_aobBoneConnectivityGraph[L_KNEE_BONE].m_aiConnectedToBoneIndices[1] = -1;
		m_aobBoneConnectivityGraph[L_KNEE_BONE].m_aiConnectedToBoneIndices[2] = -1;
		m_aobBoneConnectivityGraph[L_KNEE_BONE].m_aiConnectedToBoneIndices[3] = -1;

		m_aobBoneConnectivityGraph[R_KNEE_BONE].m_iThisBoneIndex = R_KNEE_BONE;
		m_aobBoneConnectivityGraph[R_KNEE_BONE].m_aiConnectedToBoneIndices[0] = R_LEG_BONE;
		m_aobBoneConnectivityGraph[R_KNEE_BONE].m_aiConnectedToBoneIndices[1] = -1;
		m_aobBoneConnectivityGraph[R_KNEE_BONE].m_aiConnectedToBoneIndices[2] = -1;
		m_aobBoneConnectivityGraph[R_KNEE_BONE].m_aiConnectedToBoneIndices[3] = -1;

		m_aobBoneConnectivityGraph[HEAD_BONE].m_iThisBoneIndex = HEAD_BONE;
		m_aobBoneConnectivityGraph[HEAD_BONE].m_aiConnectedToBoneIndices[0] = SPINE_BONE;
		m_aobBoneConnectivityGraph[HEAD_BONE].m_aiConnectedToBoneIndices[1] = -1;
		m_aobBoneConnectivityGraph[HEAD_BONE].m_aiConnectedToBoneIndices[2] = -1;
		m_aobBoneConnectivityGraph[HEAD_BONE].m_aiConnectedToBoneIndices[3] = -1;

		m_aobBoneConnectivityGraph[R_FORARM_BONE].m_iThisBoneIndex = R_FORARM_BONE;
		m_aobBoneConnectivityGraph[R_FORARM_BONE].m_aiConnectedToBoneIndices[0] = SPINE_BONE;
		m_aobBoneConnectivityGraph[R_FORARM_BONE].m_aiConnectedToBoneIndices[1] = R_ELBOW_BONE;
		m_aobBoneConnectivityGraph[R_FORARM_BONE].m_aiConnectedToBoneIndices[2] = -1;
		m_aobBoneConnectivityGraph[R_FORARM_BONE].m_aiConnectedToBoneIndices[3] = -1;

		m_aobBoneConnectivityGraph[L_FORARM_BONE].m_iThisBoneIndex = L_FORARM_BONE;
		m_aobBoneConnectivityGraph[L_FORARM_BONE].m_aiConnectedToBoneIndices[0] = SPINE_BONE;
		m_aobBoneConnectivityGraph[L_FORARM_BONE].m_aiConnectedToBoneIndices[1] = L_ELBOW_BONE;
		m_aobBoneConnectivityGraph[L_FORARM_BONE].m_aiConnectedToBoneIndices[2] = -1;
		m_aobBoneConnectivityGraph[L_FORARM_BONE].m_aiConnectedToBoneIndices[3] = -1;

		m_aobBoneConnectivityGraph[R_ELBOW_BONE].m_iThisBoneIndex = R_ELBOW_BONE;
		m_aobBoneConnectivityGraph[R_ELBOW_BONE].m_aiConnectedToBoneIndices[0] = R_FORARM_BONE;
		m_aobBoneConnectivityGraph[R_ELBOW_BONE].m_aiConnectedToBoneIndices[1] = -1;
		m_aobBoneConnectivityGraph[R_ELBOW_BONE].m_aiConnectedToBoneIndices[2] = -1;
		m_aobBoneConnectivityGraph[R_ELBOW_BONE].m_aiConnectedToBoneIndices[3] = -1;

		m_aobBoneConnectivityGraph[L_ELBOW_BONE].m_iThisBoneIndex = L_ELBOW_BONE;
		m_aobBoneConnectivityGraph[L_ELBOW_BONE].m_aiConnectedToBoneIndices[0] = L_FORARM_BONE;
		m_aobBoneConnectivityGraph[L_ELBOW_BONE].m_aiConnectedToBoneIndices[1] = -1;
		m_aobBoneConnectivityGraph[L_ELBOW_BONE].m_aiConnectedToBoneIndices[2] = -1;
		m_aobBoneConnectivityGraph[L_ELBOW_BONE].m_aiConnectedToBoneIndices[3] = -1;
	}*/

	void AdvancedRagdoll::CalculateRagdollFromHierarchy(hkPhysicsSystem* pobRagdollSystem)
	{
		Physics::WriteAccess mutex;

		// Get all the transformations we need
		
		CMatrix obRootT = m_entity->GetMatrix();
		CMatrix obPelvisT = m_entity->GetHierarchy()->GetTransform(m_entity->GetHierarchy()->GetTransformIndex("pelvis"))->GetWorldMatrix();
		obPelvisT.SetTranslation( obPelvisT.GetTranslation() - m_entity->GetPosition() );		
		CMatrix obSpine00T = m_entity->GetHierarchy()->GetTransform(m_entity->GetHierarchy()->GetTransformIndex("spine_00"))->GetWorldMatrix();
		obSpine00T.SetTranslation( obSpine00T.GetTranslation() - m_entity->GetPosition() );
		CMatrix obSpine01T = m_entity->GetHierarchy()->GetTransform(m_entity->GetHierarchy()->GetTransformIndex("spine_01"))->GetWorldMatrix();
		obSpine01T.SetTranslation( obSpine01T.GetTranslation() - m_entity->GetPosition() );
		CMatrix obSpine02T = m_entity->GetHierarchy()->GetTransform(m_entity->GetHierarchy()->GetTransformIndex("spine_02"))->GetWorldMatrix();
		obSpine02T.SetTranslation( obSpine02T.GetTranslation() - m_entity->GetPosition() );
		CMatrix obNeckT = m_entity->GetHierarchy()->GetTransform(m_entity->GetHierarchy()->GetTransformIndex("neck"))->GetWorldMatrix();
		obNeckT.SetTranslation( obNeckT.GetTranslation() - m_entity->GetPosition() );
		CMatrix obLLegTopT = m_entity->GetHierarchy()->GetTransform(m_entity->GetHierarchy()->GetTransformIndex("l_leg"))->GetWorldMatrix();
		obLLegTopT.SetTranslation( obLLegTopT.GetTranslation() - m_entity->GetPosition() );
		CMatrix obRLegTopT = m_entity->GetHierarchy()->GetTransform(m_entity->GetHierarchy()->GetTransformIndex("r_leg"))->GetWorldMatrix();
		obRLegTopT.SetTranslation( obRLegTopT.GetTranslation() - m_entity->GetPosition() );
		CMatrix obLLegMiddleT = m_entity->GetHierarchy()->GetTransform(m_entity->GetHierarchy()->GetTransformIndex("l_knee"))->GetWorldMatrix();
		obLLegMiddleT.SetTranslation( obLLegMiddleT.GetTranslation() - m_entity->GetPosition() );
		CMatrix obRLegMiddleT = m_entity->GetHierarchy()->GetTransform(m_entity->GetHierarchy()->GetTransformIndex("r_knee"))->GetWorldMatrix();
		obRLegMiddleT.SetTranslation( obRLegMiddleT.GetTranslation() - m_entity->GetPosition() );
		CMatrix obLLegEndT = m_entity->GetHierarchy()->GetTransform(m_entity->GetHierarchy()->GetTransformIndex("l_ankle"))->GetWorldMatrix();
		obLLegEndT.SetTranslation( obLLegEndT.GetTranslation() - m_entity->GetPosition() );
		CMatrix obRLegEndT = m_entity->GetHierarchy()->GetTransform(m_entity->GetHierarchy()->GetTransformIndex("r_ankle"))->GetWorldMatrix();
		obRLegEndT.SetTranslation( obRLegEndT.GetTranslation() - m_entity->GetPosition() );
		CMatrix obLArmTopT = m_entity->GetHierarchy()->GetTransform(m_entity->GetHierarchy()->GetTransformIndex("l_arm"))->GetWorldMatrix();
		obLArmTopT.SetTranslation( obLArmTopT.GetTranslation() - m_entity->GetPosition() );
		CMatrix obRArmTopT = m_entity->GetHierarchy()->GetTransform(m_entity->GetHierarchy()->GetTransformIndex("r_arm"))->GetWorldMatrix();
		obRArmTopT.SetTranslation( obRArmTopT.GetTranslation() - m_entity->GetPosition() );
		CMatrix obLArmMiddleT = m_entity->GetHierarchy()->GetTransform(m_entity->GetHierarchy()->GetTransformIndex("l_elbow"))->GetWorldMatrix();
		obLArmMiddleT.SetTranslation( obLArmMiddleT.GetTranslation() - m_entity->GetPosition() );
		CMatrix obRArmMiddleT = m_entity->GetHierarchy()->GetTransform(m_entity->GetHierarchy()->GetTransformIndex("r_elbow"))->GetWorldMatrix();
		obRArmMiddleT.SetTranslation( obRArmMiddleT.GetTranslation() - m_entity->GetPosition() );
		CMatrix obLArmEndT = m_entity->GetHierarchy()->GetTransform(m_entity->GetHierarchy()->GetTransformIndex("l_middle_02"))->GetWorldMatrix();
		obLArmEndT.SetTranslation( obLArmEndT.GetTranslation() - m_entity->GetPosition() );
		CMatrix obRArmEndT = m_entity->GetHierarchy()->GetTransform(m_entity->GetHierarchy()->GetTransformIndex("r_middle_02"))->GetWorldMatrix();
		obRArmEndT.SetTranslation( obRArmEndT.GetTranslation() - m_entity->GetPosition() );

		// Prepare radii
		float fHeadRadius = 0.11f;
		float fSpineWidth = (CDirection(obLArmTopT.GetTranslation() - obRArmTopT.GetTranslation())).Length() * 0.5f;
		//float fSpineLength = (CDirection(obNeckT.GetTranslation() - obSpine01T.GetTranslation())).Length() * 0.5f;
		//float fSpineDepth = 0.075f;
		float fPelvisWidth = fSpineWidth;
		//float fPelvisLength = ( CDirection( obSpine01T.GetTranslation() - ( ( CPoint( obLLegTopT.GetTranslation() + obRLegTopT.GetTranslation() ) ) * 0.5f ) ) ).Length() * 0.5f;
		//float fPelvisDepth = fSpineDepth;
		float fUpperLegRadius = 0.11f;
		float fLowerLegRadius = 0.09f;
		float fUpperArmRadius = 0.085f;
		float fLowerArmRadius = 0.085f;

		// Get all the positions for RBs we need
		CPoint obHeadPosition = obNeckT.GetTranslation();
		obHeadPosition.Y() += fHeadRadius;
		CPoint obSpinePosition = obSpine01T.GetTranslation() + obNeckT.GetTranslation();
		obSpinePosition *= 0.5f;
		CPoint obPelvisPosition = obSpine01T.GetTranslation();
		obPelvisPosition.Y() -= fabsf(obSpine01T.GetTranslation().Y() - obRLegTopT.GetTranslation().Y()) * 0.5f;
		CPoint obLeftUpperLegPosition = obLLegTopT.GetTranslation() + obLLegMiddleT.GetTranslation();
		obLeftUpperLegPosition *= 0.5f;
		CPoint obRightUpperLegPosition = obRLegTopT.GetTranslation() + obRLegMiddleT.GetTranslation();
		obRightUpperLegPosition *= 0.5f;
		CPoint obLeftLowerLegPosition = obLLegMiddleT.GetTranslation() + obLLegEndT.GetTranslation();
		obLeftLowerLegPosition *= 0.5f;
		CPoint obRightLowerLegPosition = obRLegMiddleT.GetTranslation() + obRLegEndT.GetTranslation();
		obRightLowerLegPosition *= 0.5f;
		CPoint obLeftUpperArmPosition = obLArmTopT.GetTranslation() + obLArmMiddleT.GetTranslation();
		obLeftUpperArmPosition *= 0.5f;
		CPoint obRightUpperArmPosition = obRArmTopT.GetTranslation() + obRArmMiddleT.GetTranslation();
		obRightUpperArmPosition *= 0.5f;
		CPoint obLeftLowerArmPosition = obLArmMiddleT.GetTranslation() + obLArmEndT.GetTranslation();
		obLeftLowerArmPosition *= 0.5f;
		CPoint obRightLowerArmPosition = obRArmMiddleT.GetTranslation() + obRArmEndT.GetTranslation();
		obRightLowerArmPosition *= 0.5f;

		// Shift the positions up by the radius of the capsules because it looks like they need it
		obHeadPosition -= obRootT.GetZAxis() * fHeadRadius * 0.5f;
		obSpinePosition -= obRootT.GetZAxis() * fSpineWidth * 0.5f;
		obPelvisPosition -= obRootT.GetZAxis() * fPelvisWidth * 0.5f;
		obLeftUpperLegPosition -= obRootT.GetZAxis() * fUpperLegRadius * 0.5f;
		obRightUpperLegPosition -= obRootT.GetZAxis() * fUpperLegRadius * 0.5f;
		obLeftLowerLegPosition -= obRootT.GetZAxis() * fLowerLegRadius * 0.5f;
		obRightLowerLegPosition -= obRootT.GetZAxis() * fLowerLegRadius * 0.5f;
		obLeftUpperArmPosition -= obRootT.GetZAxis() * fUpperArmRadius * 0.5f;
		obRightUpperArmPosition -= obRootT.GetZAxis() * fUpperArmRadius * 0.5f;
		obLeftLowerArmPosition -= obRootT.GetZAxis() * fLowerArmRadius * 0.5f;
		obRightLowerArmPosition -= obRootT.GetZAxis() * fLowerArmRadius * 0.5f;

		CPoint obPelvisCapsuleV1 = obSpine01T.GetTranslation();
		CPoint obPelvisCapsuleV2 = (obLLegTopT.GetTranslation() + obRLegTopT.GetTranslation()) * 0.5f;
		CDirection obPelvisCapsuleVector( obPelvisCapsuleV2 - obPelvisCapsuleV1 );
		obPelvisCapsuleVector.Normalise();
		obPelvisCapsuleVector *= fPelvisWidth;
		obPelvisCapsuleV1 += obPelvisCapsuleVector;
		obPelvisCapsuleVector *= -1;
		obPelvisCapsuleV2 += obPelvisCapsuleVector;
		obPelvisCapsuleV1 -= obPelvisPosition;
		obPelvisCapsuleV2 -= obPelvisPosition;

		CPoint obSpineCapsuleV1 = obNeckT.GetTranslation();
		CPoint obSpineCapsuleV2 = obSpine01T.GetTranslation();
		CDirection obSpineCapsuleVector( obSpineCapsuleV2 - obSpineCapsuleV1);
		obSpineCapsuleVector.Normalise();
		obSpineCapsuleVector *= fSpineWidth;
		obSpineCapsuleV1 += obSpineCapsuleVector;
		obSpineCapsuleVector *= -1;
		obSpineCapsuleV2 += obSpineCapsuleVector;
		obSpineCapsuleV1 -= obSpinePosition;
		obSpineCapsuleV2 -= obSpinePosition;
		
		/*CPoint obHeadCapsuleV1 = obNeckT.GetTranslation();
		obHeadCapsuleV1.Y() += fHeadHeight;
		CPoint obHeadCapsuleV2 = obNeckT.GetTranslation();
		obCapsuleVector = CDirection( obHeadCapsuleV2 - obHeadCapsuleV1);
		obCapsuleVector.Normalise();
		obCapsuleVector *= fHeadRadius;
		obHeadCapsuleV1 += obCapsuleVector;
		obCapsuleVector *= -1;
		obHeadCapsuleV2 += obCapsuleVector;
		obHeadCapsuleV1 -= obHeadPosition;
		obHeadCapsuleV2 -= obHeadPosition;*/

		CPoint obLeftUpperArmCapsuleV1 = obLArmTopT.GetTranslation();
		CPoint obLeftUpperArmCapsuleV2 = obLArmMiddleT.GetTranslation();
		CDirection obLeftUpperArmCapsuleVector( obLeftUpperArmCapsuleV2 - obLeftUpperArmCapsuleV1);
		obLeftUpperArmCapsuleVector.Normalise();
		obLeftUpperArmCapsuleVector *= fUpperArmRadius;
		obLeftUpperArmCapsuleV1 += obLeftUpperArmCapsuleVector;
		obLeftUpperArmCapsuleVector *= -1;
		obLeftUpperArmCapsuleV2 += obLeftUpperArmCapsuleVector;
		obLeftUpperArmCapsuleV1 -= obLeftUpperArmPosition;
		obLeftUpperArmCapsuleV2 -= obLeftUpperArmPosition;

		CPoint obRightUpperArmCapsuleV1 = obRArmTopT.GetTranslation();
		CPoint obRightUpperArmCapsuleV2 = obRArmMiddleT.GetTranslation();
		CDirection obRightUpperArmCapsuleVector( obRightUpperArmCapsuleV2 - obRightUpperArmCapsuleV1);
		obRightUpperArmCapsuleVector.Normalise();
		obRightUpperArmCapsuleVector *= fUpperArmRadius;
		obRightUpperArmCapsuleV1 += obRightUpperArmCapsuleVector;
		obRightUpperArmCapsuleVector *= -1;
		obRightUpperArmCapsuleV2 += obRightUpperArmCapsuleVector;
		obRightUpperArmCapsuleV1 -= obRightUpperArmPosition;
		obRightUpperArmCapsuleV2 -= obRightUpperArmPosition;

		CPoint obLeftLowerArmCapsuleV1 = obLArmMiddleT.GetTranslation();
		CPoint obLeftLowerArmCapsuleV2 = obLArmEndT.GetTranslation();
		CDirection obLeftLowerArmCapsuleVector( obLeftLowerArmCapsuleV2 - obLeftLowerArmCapsuleV1);
		obLeftLowerArmCapsuleVector.Normalise();
		obLeftLowerArmCapsuleVector *= fLowerArmRadius;
		obLeftLowerArmCapsuleV1 += obLeftLowerArmCapsuleVector;
		obLeftLowerArmCapsuleVector *= -1;
		obLeftLowerArmCapsuleV2 += obLeftLowerArmCapsuleVector;
		obLeftLowerArmCapsuleV1 -= obLeftLowerArmPosition;
		obLeftLowerArmCapsuleV2 -= obLeftLowerArmPosition;
		
		CPoint obRightLowerArmCapsuleV1 = obRArmMiddleT.GetTranslation();
		CPoint obRightLowerArmCapsuleV2 = obRArmEndT.GetTranslation();
		CDirection obRightLowerArmCapsuleVector( obRightLowerArmCapsuleV2 - obRightLowerArmCapsuleV1);
		obRightLowerArmCapsuleVector.Normalise();
		obRightLowerArmCapsuleVector *= fLowerArmRadius;
		obRightLowerArmCapsuleV1 += obRightLowerArmCapsuleVector;
		obRightLowerArmCapsuleVector *= -1;
		obRightLowerArmCapsuleV2 += obRightLowerArmCapsuleVector;
		obRightLowerArmCapsuleV1 -= obRightLowerArmPosition;
		obRightLowerArmCapsuleV2 -= obRightLowerArmPosition;

		CPoint obLeftUpperLegCapsuleV1 = obLLegTopT.GetTranslation();
		CPoint obLeftUpperLegCapsuleV2 = obLLegMiddleT.GetTranslation();
		CDirection obLeftUpperLegCapsuleVector( obLeftUpperLegCapsuleV2 - obLeftUpperLegCapsuleV1);
		obLeftUpperLegCapsuleVector.Normalise();
		obLeftUpperLegCapsuleVector *= fUpperLegRadius;
		obLeftUpperLegCapsuleV1 += obLeftUpperLegCapsuleVector;
		obLeftUpperLegCapsuleVector *= -1;
		obLeftUpperLegCapsuleV2 += obLeftUpperLegCapsuleVector;
		obLeftUpperLegCapsuleV1 -= obLeftUpperLegPosition;
		obLeftUpperLegCapsuleV2 -= obLeftUpperLegPosition;

		CPoint obRightUpperLegCapsuleV1 = obRLegTopT.GetTranslation();
		CPoint obRightUpperLegCapsuleV2 = obRLegMiddleT.GetTranslation();
		CDirection obRightUpperLegCapsuleVector(obRightUpperLegCapsuleV2 - obRightUpperLegCapsuleV1);
		obRightUpperLegCapsuleVector.Normalise();
		obRightUpperLegCapsuleVector *= fUpperLegRadius;
		obRightUpperLegCapsuleV1 += obRightUpperLegCapsuleVector;
		obRightUpperLegCapsuleVector *= -1;
		obRightUpperLegCapsuleV2 += obRightUpperLegCapsuleVector;
		obRightUpperLegCapsuleV1 -= obRightUpperLegPosition;
		obRightUpperLegCapsuleV2 -= obRightUpperLegPosition;

		CPoint obLeftLowerLegCapsuleV1 = obLLegMiddleT.GetTranslation();
		CPoint obLeftLowerLegCapsuleV2 = obLLegEndT.GetTranslation();
		CDirection obLeftLowerLegCapsuleVector( obLeftLowerLegCapsuleV2 - obLeftLowerLegCapsuleV1);
		obLeftLowerLegCapsuleVector.Normalise();
		obLeftLowerLegCapsuleVector *= fUpperLegRadius;
		obLeftLowerLegCapsuleV1 += obLeftLowerLegCapsuleVector;
		obLeftLowerLegCapsuleVector *= -1;
		obLeftLowerLegCapsuleV2 += obLeftLowerLegCapsuleVector;
		obLeftLowerLegCapsuleV1 -= obLeftLowerLegPosition;
		obLeftLowerLegCapsuleV2 -= obLeftLowerLegPosition;

		CPoint obRightLowerLegCapsuleV1 = obRLegMiddleT.GetTranslation();
		CPoint obRightLowerLegCapsuleV2 = obRLegEndT.GetTranslation();
		CDirection obRightLowerLegCapsuleVector( obRightLowerLegCapsuleV2 - obRightLowerLegCapsuleV1);
		obRightLowerLegCapsuleVector.Normalise();
		obRightLowerLegCapsuleVector *= fUpperLegRadius;
		obRightLowerLegCapsuleV1 += obRightLowerLegCapsuleVector;
		obRightLowerLegCapsuleVector *= -1;
		obRightLowerLegCapsuleV2 += obRightLowerLegCapsuleVector;
		obRightLowerLegCapsuleV1 -= obRightLowerLegPosition;
		obRightLowerLegCapsuleV2 -= obRightLowerLegPosition;

		// Capsule shapes
		hkCapsuleShape* pobPelvisCapsule = HK_NEW hkCapsuleShape(Physics::MathsTools::CPointTohkVector(obPelvisCapsuleV1),Physics::MathsTools::CPointTohkVector(obPelvisCapsuleV2),fPelvisWidth);
		//hkSphereShape* pobPelvisSphere = HK_NEW hkSphereShape(fPelvisRadius);
		//hkCapsuleShape* pobHeadCapsule = HK_NEW hkCapsuleShape(Physics::MathsTools::CPointTohkVector(obHeadCapsuleV1),Physics::MathsTools::CPointTohkVector(obHeadCapsuleV2),fHeadRadius);
		hkSphereShape* pobHeadSphere = HK_NEW hkSphereShape(fHeadRadius);
		hkCapsuleShape* pobSpineCapsule = HK_NEW hkCapsuleShape(Physics::MathsTools::CPointTohkVector(obSpineCapsuleV1),Physics::MathsTools::CPointTohkVector(obSpineCapsuleV2),fSpineWidth);
		//hkSphereShape* pobSpineSphere = HK_NEW hkSphereShape(fSpineRadius);
		//hkBoxShape* pobPelvisBox = HK_NEW hkBoxShape( hkVector4( fPelvisWidth, fPelvisLength, fPelvisDepth ) );
		//hkBoxShape* pobSpineBox = HK_NEW hkBoxShape( hkVector4( fSpineWidth, fSpineLength, fSpineDepth ) );
		hkCapsuleShape* pobLeftUpperArmCapsule = HK_NEW hkCapsuleShape(Physics::MathsTools::CPointTohkVector(obLeftUpperArmCapsuleV1),Physics::MathsTools::CPointTohkVector(obLeftUpperArmCapsuleV2),fUpperArmRadius);
		hkCapsuleShape* pobRightUpperArmCapsule = HK_NEW hkCapsuleShape(Physics::MathsTools::CPointTohkVector(obRightUpperArmCapsuleV1),Physics::MathsTools::CPointTohkVector(obRightUpperArmCapsuleV2),fUpperArmRadius);
		hkCapsuleShape* pobLeftLowerArmCapsule = HK_NEW hkCapsuleShape(Physics::MathsTools::CPointTohkVector(obLeftLowerArmCapsuleV1),Physics::MathsTools::CPointTohkVector(obLeftLowerArmCapsuleV2),fLowerArmRadius);
		hkCapsuleShape* pobRightLowerArmCapsule = HK_NEW hkCapsuleShape(Physics::MathsTools::CPointTohkVector(obRightLowerArmCapsuleV1),Physics::MathsTools::CPointTohkVector(obRightLowerArmCapsuleV2),fLowerArmRadius);
		hkCapsuleShape* pobLeftUpperLegCapsule = HK_NEW hkCapsuleShape(Physics::MathsTools::CPointTohkVector(obLeftUpperLegCapsuleV1),Physics::MathsTools::CPointTohkVector(obLeftUpperLegCapsuleV2),fUpperLegRadius);
		hkCapsuleShape* pobRightUpperLegCapsule = HK_NEW hkCapsuleShape(Physics::MathsTools::CPointTohkVector(obRightUpperLegCapsuleV1),Physics::MathsTools::CPointTohkVector(obRightUpperLegCapsuleV2),fUpperLegRadius);
		hkCapsuleShape* pobLeftLowerLegCapsule = HK_NEW hkCapsuleShape(Physics::MathsTools::CPointTohkVector(obLeftLowerLegCapsuleV1),Physics::MathsTools::CPointTohkVector(obLeftLowerLegCapsuleV2),fLowerLegRadius);
		hkCapsuleShape* pobRightLowerLegCapsule = HK_NEW hkCapsuleShape(Physics::MathsTools::CPointTohkVector(obRightLowerLegCapsuleV1),Physics::MathsTools::CPointTohkVector(obRightLowerLegCapsuleV2),fLowerLegRadius);

		// Infos
		hkRigidBodyCinfo obPelvisInfo;
		hkRigidBodyCinfo obHeadInfo;
		hkRigidBodyCinfo obSpineInfo;
		hkRigidBodyCinfo obLeftUpperArmInfo;
		hkRigidBodyCinfo obLeftLowerArmInfo;
		hkRigidBodyCinfo obRightUpperArmInfo;
		hkRigidBodyCinfo obRightLowerArmInfo;
		hkRigidBodyCinfo obLeftUpperLegInfo;
		hkRigidBodyCinfo obRightUpperLegInfo;
		hkRigidBodyCinfo obLeftLowerLegInfo;
		hkRigidBodyCinfo obRightLowerLegInfo;

		obPelvisInfo.m_shape = pobPelvisCapsule;
		obHeadInfo.m_shape = pobHeadSphere;
		obSpineInfo.m_shape = pobSpineCapsule;
		obLeftUpperArmInfo.m_shape = pobLeftUpperArmCapsule;
		obLeftLowerArmInfo.m_shape = pobLeftLowerArmCapsule;
		obRightUpperArmInfo.m_shape = pobRightUpperArmCapsule;
		obRightLowerArmInfo.m_shape = pobRightLowerArmCapsule;
		obLeftUpperLegInfo.m_shape = pobLeftUpperLegCapsule;
		obRightUpperLegInfo.m_shape = pobRightUpperLegCapsule;
		obLeftLowerLegInfo.m_shape = pobLeftLowerLegCapsule;
		obRightLowerLegInfo.m_shape = pobRightLowerLegCapsule;

		obPelvisInfo.m_position = Physics::MathsTools::CPointTohkVector(obPelvisPosition);
		obHeadInfo.m_position =  Physics::MathsTools::CPointTohkVector(obHeadPosition);
		obSpineInfo.m_position =  Physics::MathsTools::CPointTohkVector(obSpinePosition);
		obLeftUpperArmInfo.m_position = Physics::MathsTools::CPointTohkVector(obLeftUpperArmPosition);
		obLeftLowerArmInfo.m_position =  Physics::MathsTools::CPointTohkVector(obLeftLowerArmPosition);
		obRightUpperArmInfo.m_position =  Physics::MathsTools::CPointTohkVector(obRightUpperArmPosition);
		obRightLowerArmInfo.m_position =  Physics::MathsTools::CPointTohkVector(obRightLowerArmPosition);
		obLeftUpperLegInfo.m_position =  Physics::MathsTools::CPointTohkVector(obLeftUpperLegPosition);
		obRightUpperLegInfo.m_position =  Physics::MathsTools::CPointTohkVector(obRightUpperLegPosition);
		obLeftLowerLegInfo.m_position =  Physics::MathsTools::CPointTohkVector(obLeftLowerLegPosition);
		obRightLowerLegInfo.m_position =  Physics::MathsTools::CPointTohkVector(obRightLowerLegPosition);

		obPelvisInfo.m_mass = 10;
		obHeadInfo.m_mass = 10;
		obSpineInfo.m_mass = 10;
		obLeftUpperArmInfo.m_mass = 10;
		obLeftLowerArmInfo.m_mass = 10;
		obRightUpperArmInfo.m_mass = 10;
		obRightLowerArmInfo.m_mass = 10;
		obLeftUpperLegInfo.m_mass = 10;
		obRightUpperLegInfo.m_mass = 10;
		obLeftLowerLegInfo.m_mass = 10;
		obRightLowerLegInfo.m_mass = 10;

		obPelvisInfo.m_motionType = hkMotion::MOTION_FIXED;
		obHeadInfo.m_motionType = hkMotion::MOTION_FIXED;
		obSpineInfo.m_motionType = hkMotion::MOTION_FIXED;
		obLeftUpperArmInfo.m_motionType = hkMotion::MOTION_FIXED;
		obLeftLowerArmInfo.m_motionType = hkMotion::MOTION_FIXED;
		obRightUpperArmInfo.m_motionType = hkMotion::MOTION_FIXED;
		obRightLowerArmInfo.m_motionType = hkMotion::MOTION_FIXED;
		obLeftUpperLegInfo.m_motionType = hkMotion::MOTION_FIXED;
		obRightUpperLegInfo.m_motionType = hkMotion::MOTION_FIXED;
		obLeftLowerLegInfo.m_motionType = hkMotion::MOTION_FIXED;
		obRightLowerLegInfo.m_motionType = hkMotion::MOTION_FIXED;

		/*obPelvisInfo.m_qualityType = HK_COLLIDABLE_QUALITY_CRITICAL;
		obHeadInfo.m_qualityType = HK_COLLIDABLE_QUALITY_CRITICAL;
		obSpineInfo.m_qualityType = HK_COLLIDABLE_QUALITY_CRITICAL;
		obLeftUpperArmInfo.m_qualityType = HK_COLLIDABLE_QUALITY_CRITICAL;
		obLeftLowerArmInfo.m_qualityType = HK_COLLIDABLE_QUALITY_CRITICAL;
		obRightUpperArmInfo.m_qualityType = HK_COLLIDABLE_QUALITY_CRITICAL;
		obRightLowerArmInfo.m_qualityType = HK_COLLIDABLE_QUALITY_CRITICAL;
		obLeftUpperLegInfo.m_qualityType = HK_COLLIDABLE_QUALITY_CRITICAL;
		obRightUpperLegInfo.m_qualityType = HK_COLLIDABLE_QUALITY_CRITICAL;
		obLeftLowerLegInfo.m_qualityType = HK_COLLIDABLE_QUALITY_CRITICAL;
		obRightLowerLegInfo.m_qualityType = HK_COLLIDABLE_QUALITY_CRITICAL;*/

		hkMassProperties obPelvisMassProperties;
		obPelvisMassProperties.m_centerOfMass = obPelvisInfo.m_centerOfMass;
		obPelvisMassProperties.m_mass = obPelvisInfo.m_mass;
		hkInertiaTensorComputer::computeCapsuleVolumeMassProperties(pobPelvisCapsule->getVertex(0), pobPelvisCapsule->getVertex(1), pobPelvisCapsule->getRadius(), obPelvisMassProperties.m_mass, obPelvisMassProperties);
		//hkInertiaTensorComputer::computeSphereVolumeMassProperties( pobHeadSphere->getRadius(), obPelvisMassProperties.m_mass, obPelvisMassProperties);
		//hkInertiaTensorComputer::computeBoxVolumeMassProperties(hkVector4(fPelvisWidth,fPelvisLength,fPelvisDepth), obPelvisMassProperties.m_mass, obPelvisMassProperties);
		obPelvisInfo.m_inertiaTensor = obPelvisMassProperties.m_inertiaTensor;
		hkMassProperties obHeadMassProperties;
		obHeadMassProperties.m_centerOfMass = obHeadInfo.m_centerOfMass;
		obHeadMassProperties.m_mass = obHeadInfo.m_mass;
		//hkInertiaTensorComputer::computeCapsuleVolumeMassProperties(pobHeadCapsule->getVertex(0), pobHeadCapsule->getVertex(1), pobHeadCapsule->getRadius(), obHeadMassProperties.m_mass, obHeadMassProperties);
		hkInertiaTensorComputer::computeSphereVolumeMassProperties( pobHeadSphere->getRadius(), obHeadMassProperties.m_mass, obHeadMassProperties);
		obHeadInfo.m_inertiaTensor = obHeadMassProperties.m_inertiaTensor;		
		hkMassProperties obSpineMassProperties;
		obSpineMassProperties.m_centerOfMass = obSpineInfo.m_centerOfMass;
		obSpineMassProperties.m_mass = obSpineInfo.m_mass;
		//hkInertiaTensorComputer::computeBoxVolumeMassProperties(hkVector4(fSpineWidth,fSpineLength,fSpineDepth), obSpineMassProperties.m_mass, obSpineMassProperties);
		hkInertiaTensorComputer::computeCapsuleVolumeMassProperties(pobSpineCapsule->getVertex(0), pobSpineCapsule->getVertex(1), pobSpineCapsule->getRadius(), obSpineMassProperties.m_mass, obSpineMassProperties);
		//hkInertiaTensorComputer::computeSphereVolumeMassProperties( pobSpineSphere->getRadius(), obSpineMassProperties.m_mass, obSpineMassProperties);
		obSpineInfo.m_inertiaTensor = obSpineMassProperties.m_inertiaTensor;		
		hkMassProperties obLeftUpperArmMassProperties;
		obLeftUpperArmMassProperties.m_centerOfMass = obLeftUpperArmInfo.m_centerOfMass;
		obLeftUpperArmMassProperties.m_mass = obLeftUpperArmInfo.m_mass;
		hkInertiaTensorComputer::computeCapsuleVolumeMassProperties(pobLeftUpperArmCapsule->getVertex(0), pobLeftUpperArmCapsule->getVertex(1), pobLeftUpperArmCapsule->getRadius(), obLeftUpperArmMassProperties.m_mass, obLeftUpperArmMassProperties);
		obLeftUpperArmInfo.m_inertiaTensor = obLeftUpperArmMassProperties.m_inertiaTensor;		
		hkMassProperties obRightUpperArmMassProperties;
		obRightUpperArmMassProperties.m_centerOfMass = obRightUpperArmInfo.m_centerOfMass;
		obRightUpperArmMassProperties.m_mass = obRightUpperArmInfo.m_mass;
		hkInertiaTensorComputer::computeCapsuleVolumeMassProperties(pobRightUpperArmCapsule->getVertex(0), pobRightUpperArmCapsule->getVertex(1), pobRightUpperArmCapsule->getRadius(), obRightUpperArmMassProperties.m_mass, obRightUpperArmMassProperties);
		obRightUpperArmInfo.m_inertiaTensor = obRightUpperArmMassProperties.m_inertiaTensor;		
		hkMassProperties obLeftLowerArmMassProperties;
		obLeftLowerArmMassProperties.m_centerOfMass = obLeftLowerArmInfo.m_centerOfMass;
		obLeftLowerArmMassProperties.m_mass = obLeftLowerArmInfo.m_mass;
		hkInertiaTensorComputer::computeCapsuleVolumeMassProperties(pobLeftLowerArmCapsule->getVertex(0), pobLeftLowerArmCapsule->getVertex(1), pobLeftLowerArmCapsule->getRadius(), obLeftLowerArmMassProperties.m_mass, obLeftLowerArmMassProperties);
		obLeftLowerArmInfo.m_inertiaTensor = obLeftLowerArmMassProperties.m_inertiaTensor;		
		hkMassProperties obRightLowerArmMassProperties;
		obRightLowerArmMassProperties.m_centerOfMass = obRightLowerArmInfo.m_centerOfMass;
		obRightLowerArmMassProperties.m_mass = obRightLowerArmInfo.m_mass;
		hkInertiaTensorComputer::computeCapsuleVolumeMassProperties(pobRightLowerArmCapsule->getVertex(0), pobRightLowerArmCapsule->getVertex(1), pobRightLowerArmCapsule->getRadius(), obRightLowerArmMassProperties.m_mass, obRightLowerArmMassProperties);
		obRightLowerArmInfo.m_inertiaTensor = obRightLowerArmMassProperties.m_inertiaTensor;		
		hkMassProperties obLeftUpperLegMassProperties;
		obLeftUpperLegMassProperties.m_centerOfMass = obLeftUpperLegInfo.m_centerOfMass;
		obLeftUpperLegMassProperties.m_mass = obLeftUpperLegInfo.m_mass;
		hkInertiaTensorComputer::computeCapsuleVolumeMassProperties(pobLeftUpperLegCapsule->getVertex(0), pobLeftUpperLegCapsule->getVertex(1), pobLeftUpperLegCapsule->getRadius(), obLeftUpperLegMassProperties.m_mass, obLeftUpperLegMassProperties);
		obLeftUpperLegInfo.m_inertiaTensor = obLeftUpperLegMassProperties.m_inertiaTensor;		
		hkMassProperties obRightUpperLegMassProperties;
		obRightUpperLegMassProperties.m_centerOfMass = obRightUpperLegInfo.m_centerOfMass;
		obRightUpperLegMassProperties.m_mass = obRightUpperLegInfo.m_mass;
		hkInertiaTensorComputer::computeCapsuleVolumeMassProperties(pobRightUpperLegCapsule->getVertex(0), pobRightUpperLegCapsule->getVertex(1), pobRightUpperLegCapsule->getRadius(), obRightUpperLegMassProperties.m_mass, obRightUpperLegMassProperties);
		obRightUpperLegInfo.m_inertiaTensor = obRightUpperLegMassProperties.m_inertiaTensor;		
		hkMassProperties obLeftLowerLegMassProperties;
		obLeftLowerLegMassProperties.m_centerOfMass = obLeftLowerLegInfo.m_centerOfMass;
		obLeftLowerLegMassProperties.m_mass = obLeftLowerLegInfo.m_mass;
		hkInertiaTensorComputer::computeCapsuleVolumeMassProperties(pobLeftLowerLegCapsule->getVertex(0), pobLeftLowerLegCapsule->getVertex(1), pobLeftLowerLegCapsule->getRadius(), obLeftLowerLegMassProperties.m_mass, obLeftLowerLegMassProperties);
		obLeftLowerLegInfo.m_inertiaTensor = obLeftLowerLegMassProperties.m_inertiaTensor;		
		hkMassProperties obRightLowerLegMassProperties;
		obRightLowerLegMassProperties.m_centerOfMass = obRightLowerLegInfo.m_centerOfMass;
		obRightLowerLegMassProperties.m_mass = obRightLowerLegInfo.m_mass;
		hkInertiaTensorComputer::computeCapsuleVolumeMassProperties(pobRightLowerLegCapsule->getVertex(0), pobRightLowerLegCapsule->getVertex(1), pobRightLowerLegCapsule->getRadius(), obRightLowerLegMassProperties.m_mass, obRightLowerLegMassProperties);
		obRightLowerLegInfo.m_inertiaTensor = obRightLowerLegMassProperties.m_inertiaTensor;

		// RB Pointers
		hkRigidBody* pobPelvisRB = HK_NEW hkRigidBody(obPelvisInfo);
		hkRigidBody* pobSpineRB = HK_NEW hkRigidBody(obSpineInfo);
		hkRigidBody* pobHeadRB = HK_NEW hkRigidBody(obHeadInfo);
		hkRigidBody* pobLeftUpperArmRB = HK_NEW hkRigidBody(obLeftUpperArmInfo);
		hkRigidBody* pobLeftLowerArmRB = HK_NEW hkRigidBody(obLeftLowerArmInfo);
		hkRigidBody* pobRightUpperArmRB = HK_NEW hkRigidBody(obRightUpperArmInfo);
		hkRigidBody* pobRightLowerArmRB = HK_NEW hkRigidBody(obRightLowerArmInfo);
		hkRigidBody* pobLeftUpperLegRB = HK_NEW hkRigidBody(obLeftUpperLegInfo);
		hkRigidBody* pobLeftLowerLegRB = HK_NEW hkRigidBody(obLeftLowerLegInfo);
		hkRigidBody* pobRightUpperLegRB = HK_NEW hkRigidBody(obRightUpperLegInfo);
		hkRigidBody* pobRightLowerLegRB = HK_NEW hkRigidBody(obRightLowerLegInfo);

		pobPelvisRB->setName("Biped_Pelvis");
		pobSpineRB->setName("Biped_Spine0");
		pobHeadRB->setName("Biped_Head");
		// For some reason, the arms and legs have to be swapped L and R
		pobLeftUpperArmRB->setName("Biped_R_UpperArm");
		pobLeftLowerArmRB->setName("Biped_R_ForeArm");
		pobRightUpperArmRB->setName("Biped_L_UpperArm");
		pobRightLowerArmRB->setName("Biped_L_ForeArm");
		pobLeftUpperLegRB->setName("Biped_R_Thigh");
		pobLeftLowerLegRB->setName("Biped_R_Calf");
		pobRightUpperLegRB->setName("Biped_L_Thigh");
		pobRightLowerLegRB->setName("Biped_L_Calf");	
	
		// Constraints - all ragdoll atm for simplicity
		// Child is A, parent is B
		hkRagdollConstraintData* pobPelvisToSpineJoint = HK_NEW hkRagdollConstraintData();
		hkVector4 obPelvisToSpinePivot, obPelvisToSpineTwistAxis, obPelvisToSpinePlaneAxis;
		hkRagdollConstraintData* pobPelvisToLUpperLegJoint = HK_NEW hkRagdollConstraintData();
		hkVector4 obPelvisToLUpperLegPivot, obPelvisToLUpperLegTwistAxis, obPelvisToLUpperLegPlaneAxis;
		hkRagdollConstraintData* pobPelvisToRUpperLegJoint = HK_NEW hkRagdollConstraintData();
		hkVector4 obPelvisToRUpperLegPivot, obPelvisToRUpperLegTwistAxis, obPelvisToRUpperLegPlaneAxis;
		hkRagdollConstraintData* pobLUpperLegToLLowerLegJoint = HK_NEW hkRagdollConstraintData();
		hkVector4 obLUpperLegToLLowerLegPivot, obLUpperLegToLLowerLegTwistAxis, obLUpperLegToLLowerLegPlaneAxis;
		hkRagdollConstraintData* pobRUpperLegToRLowerLegJoint = HK_NEW hkRagdollConstraintData();
		hkVector4 obRUpperLegToRLowerLegPivot, obRUpperLegToRLowerLegTwistAxis, obRUpperLegToRLowerLegPlaneAxis;
		hkRagdollConstraintData* pobSpineToHeadJoint = HK_NEW hkRagdollConstraintData();
		hkVector4 obSpineToHeadPivot, obSpineToHeadTwistAxis, obSpineToHeadPlaneAxis;
		hkRagdollConstraintData* pobSpineToLUpperArmJoint = HK_NEW hkRagdollConstraintData();
		hkVector4 obSpineToLUpperArmPivot, obSpineToLUpperArmTwistAxis, obSpineToLUpperArmPlaneAxis;
		hkRagdollConstraintData* pobSpineToRUpperArmJoint = HK_NEW hkRagdollConstraintData();
		hkVector4 obSpineToRUpperArmPivot, obSpineToRUpperArmTwistAxis, obSpineToRUpperArmPlaneAxis;
		hkRagdollConstraintData* pobLUpperArmToLLowerArmJoint = HK_NEW hkRagdollConstraintData();
		hkVector4 obLUpperArmToLLowerArmPivot, obLUpperArmToLLowerArmTwistAxis, obLUpperArmToLLowerArmPlaneAxis;
		hkRagdollConstraintData* pobRUpperArmToRLowerArmJoint = HK_NEW hkRagdollConstraintData();
		hkVector4 obRUpperArmToRLowerArmPivot, obRUpperArmToRLowerArmTwistAxis, obRUpperArmToRLowerArmPlaneAxis;

		obPelvisToSpinePivot = Physics::MathsTools::CPointTohkVector( (obPelvisCapsuleV1 + obPelvisPosition) + (obPelvisCapsuleVector * fPelvisWidth) );
		obPelvisToSpineTwistAxis.setSub4(pobSpineRB->getPosition(),obPelvisToSpinePivot);
		obPelvisToSpineTwistAxis.normalize3();
		obPelvisToSpinePlaneAxis = Physics::MathsTools::CDirectionTohkVector(m_entity->GetMatrix().GetZAxis());
		obPelvisToSpinePlaneAxis.normalize3();
		pobPelvisToSpineJoint->setInWorldSpace(pobSpineRB->getTransform(), pobPelvisRB->getTransform(), obPelvisToSpinePivot, obPelvisToSpineTwistAxis, obPelvisToSpinePlaneAxis);
		pobPelvisToSpineJoint->setTwistMinAngularLimit(-0.025f);
		pobPelvisToSpineJoint->setTwistMaxAngularLimit(0.025f);
		pobPelvisToSpineJoint->setAsymmetricConeAngle( -0.1f, 0.1f);
		pobPelvisToSpineJoint->setPlaneMinAngularLimit(-0.1f);
		pobPelvisToSpineJoint->setPlaneMaxAngularLimit(0.1f);

		obPelvisToLUpperLegPivot = Physics::MathsTools::CPointTohkVector( (obLeftUpperLegCapsuleV1 + obLeftUpperLegPosition) + (obLeftUpperLegCapsuleVector * fUpperLegRadius) );
		obPelvisToLUpperLegTwistAxis.setSub4(pobLeftUpperLegRB->getPosition(),obPelvisToLUpperLegPivot);
		obPelvisToLUpperLegTwistAxis.normalize3();
		obPelvisToLUpperLegPlaneAxis = Physics::MathsTools::CDirectionTohkVector(m_entity->GetMatrix().GetZAxis());
		obPelvisToLUpperLegPlaneAxis.normalize3();
		pobPelvisToLUpperLegJoint->setInWorldSpace(pobLeftUpperLegRB->getTransform(), pobPelvisRB->getTransform(), obPelvisToLUpperLegPivot, obPelvisToLUpperLegTwistAxis, obPelvisToLUpperLegPlaneAxis);
		pobPelvisToLUpperLegJoint->setTwistMinAngularLimit(-0.25f);
		pobPelvisToLUpperLegJoint->setTwistMaxAngularLimit(0.25f);
		pobPelvisToLUpperLegJoint->setAsymmetricConeAngle( -0.5f, 0.5f);
		pobPelvisToLUpperLegJoint->setPlaneMinAngularLimit(-0.5f);
		pobPelvisToLUpperLegJoint->setPlaneMaxAngularLimit(0.5f);
		
		obPelvisToRUpperLegPivot = Physics::MathsTools::CPointTohkVector( (obRightUpperLegCapsuleV1 + obRightUpperLegPosition) + (obRightUpperLegCapsuleVector * fUpperLegRadius) ); 
		obPelvisToRUpperLegTwistAxis.setSub4(pobRightUpperLegRB->getPosition(), obPelvisToRUpperLegPivot);
		obPelvisToRUpperLegTwistAxis.normalize3();
		obPelvisToRUpperLegPlaneAxis = Physics::MathsTools::CDirectionTohkVector(m_entity->GetMatrix().GetZAxis());
		obPelvisToRUpperLegPlaneAxis.normalize3();
		pobPelvisToRUpperLegJoint->setInWorldSpace(pobRightUpperLegRB->getTransform(), pobPelvisRB->getTransform(), obPelvisToRUpperLegPivot, obPelvisToRUpperLegTwistAxis, obPelvisToRUpperLegPlaneAxis);
		pobPelvisToRUpperLegJoint->setTwistMinAngularLimit(-0.25f);
		pobPelvisToRUpperLegJoint->setTwistMaxAngularLimit(0.25f);
		pobPelvisToRUpperLegJoint->setAsymmetricConeAngle( -0.5f, 0.5f);
		pobPelvisToRUpperLegJoint->setPlaneMinAngularLimit(-0.5f);
		pobPelvisToRUpperLegJoint->setPlaneMaxAngularLimit(0.5f);

		obLUpperLegToLLowerLegPivot = Physics::MathsTools::CPointTohkVector( ((obLeftLowerLegCapsuleV1 + obLeftLowerLegPosition) + (obLeftUpperLegCapsuleV2 + obLeftUpperLegPosition)) * 0.5f );
		obLUpperLegToLLowerLegTwistAxis.setSub4(pobLeftLowerLegRB->getPosition(),obLUpperLegToLLowerLegPivot);
		obLUpperLegToLLowerLegTwistAxis.normalize3();
		obLUpperLegToLLowerLegPlaneAxis = Physics::MathsTools::CDirectionTohkVector(m_entity->GetMatrix().GetZAxis());
		obLUpperLegToLLowerLegPlaneAxis.normalize3();
		pobLUpperLegToLLowerLegJoint->setInWorldSpace(pobLeftLowerLegRB->getTransform(), pobLeftUpperLegRB->getTransform(), obLUpperLegToLLowerLegPivot, obLUpperLegToLLowerLegTwistAxis, obLUpperLegToLLowerLegPlaneAxis);
		pobLUpperLegToLLowerLegJoint->setTwistMinAngularLimit(-0.2617990f);
		pobLUpperLegToLLowerLegJoint->setTwistMaxAngularLimit(0.2617990f);
		pobLUpperLegToLLowerLegJoint->setAsymmetricConeAngle( -1.0472f, 1.0472f);
		pobLUpperLegToLLowerLegJoint->setPlaneMinAngularLimit(-0.523599f);
		pobLUpperLegToLLowerLegJoint->setPlaneMaxAngularLimit(0.523599f);

		obRUpperLegToRLowerLegPivot = Physics::MathsTools::CPointTohkVector( ((obRightLowerLegCapsuleV1 + obRightLowerLegPosition) + (obRightUpperLegCapsuleV2 + obRightUpperLegPosition)) * 0.5f );
		obLUpperLegToLLowerLegTwistAxis.setSub4(pobLeftLowerLegRB->getPosition(),obLUpperLegToLLowerLegPivot);
		obRUpperLegToRLowerLegTwistAxis.setSub4(pobRightLowerLegRB->getPosition(), Physics::MathsTools::CPointTohkVector( obRightLowerLegCapsuleV1 + obRightLowerLegPosition ));
		obRUpperLegToRLowerLegTwistAxis.normalize3();
		obRUpperLegToRLowerLegPlaneAxis = Physics::MathsTools::CDirectionTohkVector(m_entity->GetMatrix().GetZAxis());
		obRUpperLegToRLowerLegPlaneAxis.normalize3();
		pobRUpperLegToRLowerLegJoint->setInWorldSpace(pobRightLowerLegRB->getTransform(), pobRightUpperLegRB->getTransform(), obRUpperLegToRLowerLegPivot, obRUpperLegToRLowerLegTwistAxis, obRUpperLegToRLowerLegPlaneAxis);
		pobRUpperLegToRLowerLegJoint->setTwistMinAngularLimit(-0.2617990f);
		pobRUpperLegToRLowerLegJoint->setTwistMaxAngularLimit(0.2617990f);
		pobRUpperLegToRLowerLegJoint->setAsymmetricConeAngle( -1.0472f, 1.0472f);
		pobRUpperLegToRLowerLegJoint->setPlaneMinAngularLimit(-0.523599f);
		pobRUpperLegToRLowerLegJoint->setPlaneMaxAngularLimit(0.523599f);

		obSpineToHeadPivot = pobHeadRB->getPosition();
		obSpineToHeadPivot(1) -= fHeadRadius;
		obSpineToHeadTwistAxis.setSub4(pobHeadRB->getPosition(),pobSpineRB->getPosition());
		obSpineToHeadTwistAxis.normalize3();
		obSpineToHeadPlaneAxis = Physics::MathsTools::CDirectionTohkVector(m_entity->GetMatrix().GetZAxis());
		obSpineToHeadPlaneAxis.normalize3();
		pobSpineToHeadJoint->setInWorldSpace(pobHeadRB->getTransform(), pobSpineRB->getTransform(), obSpineToHeadPivot, obSpineToHeadTwistAxis, obSpineToHeadPlaneAxis);
		pobSpineToHeadJoint->setTwistMinAngularLimit(-0.2617990f);
		pobSpineToHeadJoint->setTwistMaxAngularLimit(0.2617990f);
		pobSpineToHeadJoint->setAsymmetricConeAngle( -1.0472f, 1.0472f);
		pobSpineToHeadJoint->setPlaneMinAngularLimit(-0.523599f);
		pobSpineToHeadJoint->setPlaneMaxAngularLimit(0.523599f);

		obSpineToLUpperArmPivot = Physics::MathsTools::CPointTohkVector( obLeftUpperArmCapsuleV1 + obLeftUpperArmPosition );
		obSpineToLUpperArmTwistAxis.setSub4(pobLeftUpperArmRB->getPosition(), Physics::MathsTools::CPointTohkVector( obLeftUpperArmCapsuleV1 + obLeftUpperArmPosition ));
		obSpineToLUpperArmTwistAxis.normalize3();
		obSpineToLUpperArmPlaneAxis = Physics::MathsTools::CDirectionTohkVector(m_entity->GetMatrix().GetZAxis());
		obSpineToLUpperArmPlaneAxis.normalize3();
		pobSpineToLUpperArmJoint->setInWorldSpace(pobLeftUpperArmRB->getTransform(), pobSpineRB->getTransform(), obSpineToLUpperArmPivot, obSpineToLUpperArmTwistAxis, obSpineToLUpperArmPlaneAxis);
		pobSpineToLUpperArmJoint->setTwistMinAngularLimit(-0.2617990f);
		pobSpineToLUpperArmJoint->setTwistMaxAngularLimit(0.2617990f);
		pobSpineToLUpperArmJoint->setAsymmetricConeAngle( -2.0f, 2.0f);
		pobSpineToLUpperArmJoint->setPlaneMinAngularLimit(-0.523599f);
		pobSpineToLUpperArmJoint->setPlaneMaxAngularLimit(0.523599f);

		obSpineToRUpperArmPivot = Physics::MathsTools::CPointTohkVector( obRightUpperArmCapsuleV1 + obRightUpperArmPosition);
		obSpineToRUpperArmTwistAxis.setSub4(pobRightUpperArmRB->getPosition(), Physics::MathsTools::CPointTohkVector( obRightUpperArmCapsuleV1 + obRightUpperArmPosition));
		obSpineToRUpperArmTwistAxis.normalize3();
		obSpineToRUpperArmPlaneAxis = Physics::MathsTools::CDirectionTohkVector(m_entity->GetMatrix().GetZAxis());
		obSpineToRUpperArmPlaneAxis.normalize3();
		pobSpineToRUpperArmJoint->setInWorldSpace(pobRightUpperArmRB->getTransform(), pobSpineRB->getTransform(), obSpineToRUpperArmPivot, obSpineToRUpperArmTwistAxis, obSpineToRUpperArmPlaneAxis);
		pobSpineToRUpperArmJoint->setTwistMinAngularLimit(-0.2617990f);
		pobSpineToRUpperArmJoint->setTwistMaxAngularLimit(0.2617990f);
		pobSpineToRUpperArmJoint->setAsymmetricConeAngle( -2.0f, 2.0f);
		pobSpineToRUpperArmJoint->setPlaneMinAngularLimit(-0.523599f);
		pobSpineToRUpperArmJoint->setPlaneMaxAngularLimit(0.523599f);

		obLUpperArmToLLowerArmPivot = Physics::MathsTools::CPointTohkVector( obLeftLowerArmCapsuleV1 + obLeftLowerArmPosition);
		obLUpperArmToLLowerArmTwistAxis.setSub4(pobLeftLowerArmRB->getPosition(),Physics::MathsTools::CPointTohkVector( obLeftLowerArmCapsuleV1 + obLeftLowerArmPosition));
		obLUpperArmToLLowerArmTwistAxis.normalize3();
		obLUpperArmToLLowerArmPlaneAxis = Physics::MathsTools::CDirectionTohkVector(m_entity->GetMatrix().GetZAxis());
		obLUpperArmToLLowerArmPlaneAxis.normalize3();
		pobLUpperArmToLLowerArmJoint->setInWorldSpace(pobLeftLowerArmRB->getTransform(), pobLeftUpperArmRB->getTransform(), obLUpperArmToLLowerArmPivot, obLUpperArmToLLowerArmTwistAxis, obLUpperArmToLLowerArmPlaneAxis);
		pobLUpperArmToLLowerArmJoint->setTwistMinAngularLimit(-0.2617990f);
		pobLUpperArmToLLowerArmJoint->setTwistMaxAngularLimit(0.2617990f);
		pobLUpperArmToLLowerArmJoint->setAsymmetricConeAngle( -1.0472f, 1.0472f);
		pobLUpperArmToLLowerArmJoint->setPlaneMinAngularLimit(-0.523599f);
		pobLUpperArmToLLowerArmJoint->setPlaneMaxAngularLimit(0.523599f);

		obRUpperArmToRLowerArmPivot = Physics::MathsTools::CPointTohkVector( obRightLowerArmCapsuleV1 + obRightLowerArmPosition );
		obRUpperArmToRLowerArmTwistAxis.setSub4(pobRightLowerArmRB->getPosition(),Physics::MathsTools::CPointTohkVector( obRightLowerArmCapsuleV1 + obRightLowerArmPosition ));
		obRUpperArmToRLowerArmTwistAxis.normalize3();
		obRUpperArmToRLowerArmPlaneAxis = Physics::MathsTools::CDirectionTohkVector(m_entity->GetMatrix().GetZAxis());
		obRUpperArmToRLowerArmPlaneAxis.normalize3();
		pobRUpperArmToRLowerArmJoint->setInWorldSpace(pobRightLowerArmRB->getTransform(), pobRightUpperArmRB->getTransform(), obRUpperArmToRLowerArmPivot, obRUpperArmToRLowerArmTwistAxis, obRUpperArmToRLowerArmPlaneAxis);
		pobRUpperArmToRLowerArmJoint->setTwistMinAngularLimit(-0.2617990f);
		pobRUpperArmToRLowerArmJoint->setTwistMaxAngularLimit(0.2617990f);
		pobRUpperArmToRLowerArmJoint->setAsymmetricConeAngle( -1.0472f, 1.0472f);
		pobRUpperArmToRLowerArmJoint->setPlaneMinAngularLimit(-0.523599f);
		pobRUpperArmToRLowerArmJoint->setPlaneMaxAngularLimit(0.523599f);
	
		//hkConstraintInstance* pobPelvisToSpineConstraintInstance = HK_NEW hkConstraintInstance( pobSpineRB, pobPelvisRB, pobPelvisToSpineJoint );
		//hkConstraintInstance* pobPelvisToLUpperLegConstraintInstance = HK_NEW hkConstraintInstance( pobLeftUpperLegRB, pobPelvisRB, pobPelvisToLUpperLegJoint );
		//hkConstraintInstance* pobPelvisToRUpperLegConstraintInstance = HK_NEW hkConstraintInstance( pobRightUpperLegRB, pobPelvisRB, pobPelvisToRUpperLegJoint );
		//hkConstraintInstance* pobLUpperLegToLLowerLegConstraintInstance = HK_NEW hkConstraintInstance( pobLeftLowerLegRB, pobLeftUpperLegRB, pobLUpperLegToLLowerLegJoint );
		//hkConstraintInstance* pobRUpperLegToRLowerLegConstraintInstance = HK_NEW hkConstraintInstance( pobRightLowerLegRB, pobRightUpperLegRB, pobRUpperLegToRLowerLegJoint );
		//hkConstraintInstance* pobSpineToHeadConstraintInstance = HK_NEW hkConstraintInstance( pobHeadRB, pobSpineRB, pobSpineToHeadJoint );
		//hkConstraintInstance* pobSpineToLUpperArmConstraintInstance = HK_NEW hkConstraintInstance( pobLeftUpperArmRB, pobSpineRB, pobSpineToLUpperArmJoint );
		//hkConstraintInstance* pobSpineToRUpperArmConstraintInstance = HK_NEW hkConstraintInstance( pobRightUpperArmRB, pobSpineRB, pobSpineToRUpperArmJoint );
		//hkConstraintInstance* pobLUpperArmToLLowerArmConstraintInstance = HK_NEW hkConstraintInstance( pobLeftLowerArmRB, pobLeftUpperArmRB, pobLUpperArmToLLowerArmJoint );
		//hkConstraintInstance* pobRUpperArmToRLowerArmConstraintInstance = HK_NEW hkConstraintInstance( pobRightLowerArmRB, pobRightUpperArmRB, pobRUpperArmToRLowerArmJoint );

		Physics::RagdollCollisionFlag iRagFlag; iRagFlag.base = 0;

		// I am a ragdoll body
		iRagFlag.flags.i_am |= Physics::RAGDOLL_BIT;
		
		// I collide with
		iRagFlag.flags.i_collide_with = (	Physics::RAGDOLL_BIT						|
											Physics::SMALL_INTERACTABLE_BIT				|
											Physics::LARGE_INTERACTABLE_BIT				|
											Physics::TRIGGER_VOLUME_BIT					);
		if ( g_ShellOptions->m_bInteractableRagdolls )
		{
			iRagFlag.flags.i_collide_with |= ( Physics::CHARACTER_CONTROLLER_PLAYER_BIT	|
											   Physics::CHARACTER_CONTROLLER_ENEMY_BIT );
		}
		
		
		hkPropertyValue val((void*)m_entity);

		pobPelvisRB->getCollidableRw()->setCollisionFilterInfo(iRagFlag.base);
		pobPelvisRB->addProperty(Physics::PROPERTY_ENTITY_PTR, val);
		pobSpineRB->getCollidableRw()->setCollisionFilterInfo(iRagFlag.base);
		pobSpineRB->addProperty(Physics::PROPERTY_ENTITY_PTR, val);
		pobLeftUpperLegRB->getCollidableRw()->setCollisionFilterInfo(iRagFlag.base);
		pobLeftUpperLegRB->addProperty(Physics::PROPERTY_ENTITY_PTR, val);
		pobRightUpperLegRB->getCollidableRw()->setCollisionFilterInfo(iRagFlag.base);
		pobRightUpperLegRB->addProperty(Physics::PROPERTY_ENTITY_PTR, val);
		pobLeftLowerLegRB->getCollidableRw()->setCollisionFilterInfo(iRagFlag.base);
		pobLeftLowerLegRB->addProperty(Physics::PROPERTY_ENTITY_PTR, val);
		pobRightLowerLegRB->getCollidableRw()->setCollisionFilterInfo(iRagFlag.base);
		pobRightLowerLegRB->addProperty(Physics::PROPERTY_ENTITY_PTR, val);
		pobHeadRB->getCollidableRw()->setCollisionFilterInfo(iRagFlag.base);
		pobHeadRB->addProperty(Physics::PROPERTY_ENTITY_PTR, val);
		pobLeftUpperArmRB->getCollidableRw()->setCollisionFilterInfo(iRagFlag.base);
		pobLeftUpperArmRB->addProperty(Physics::PROPERTY_ENTITY_PTR, val);
		pobRightUpperArmRB->getCollidableRw()->setCollisionFilterInfo(iRagFlag.base);
		pobRightUpperArmRB->addProperty(Physics::PROPERTY_ENTITY_PTR, val);
		pobLeftLowerArmRB->getCollidableRw()->setCollisionFilterInfo(iRagFlag.base);
		pobLeftLowerArmRB->addProperty(Physics::PROPERTY_ENTITY_PTR, val);		
		pobRightLowerArmRB->getCollidableRw()->setCollisionFilterInfo(iRagFlag.base);
		pobRightLowerArmRB->addProperty(Physics::PROPERTY_ENTITY_PTR, val);

		CPhysicsWorld::Get().GetHavokWorldP()->addEntity(pobPelvisRB);
		//CPhysicsWorld::Get().GetHavokWorldP()->addEntity(pobSpineRB);
		CPhysicsWorld::Get().GetHavokWorldP()->addEntity(pobLeftUpperLegRB);
		CPhysicsWorld::Get().GetHavokWorldP()->addEntity(pobRightUpperLegRB);
		CPhysicsWorld::Get().GetHavokWorldP()->addEntity(pobLeftLowerLegRB);
		CPhysicsWorld::Get().GetHavokWorldP()->addEntity(pobRightLowerLegRB);
		//CPhysicsWorld::Get().GetHavokWorldP()->addEntity(pobHeadRB);
		//CPhysicsWorld::Get().GetHavokWorldP()->addEntity(pobLeftUpperArmRB);
		//CPhysicsWorld::Get().GetHavokWorldP()->addEntity(pobRightUpperArmRB);
		//CPhysicsWorld::Get().GetHavokWorldP()->addEntity(pobLeftLowerArmRB);
		//CPhysicsWorld::Get().GetHavokWorldP()->addEntity(pobRightLowerArmRB);

		//CPhysicsWorld::Get().GetHavokWorldP()->addConstraint(pobPelvisToSpineConstraintInstance);
		//CPhysicsWorld::Get().GetHavokWorldP()->addConstraint(pobPelvisToLUpperLegConstraintInstance);
		//CPhysicsWorld::Get().GetHavokWorldP()->addConstraint(pobPelvisToRUpperLegConstraintInstance);
		//CPhysicsWorld::Get().GetHavokWorldP()->addConstraint(pobLUpperLegToLLowerLegConstraintInstance);
		//CPhysicsWorld::Get().GetHavokWorldP()->addConstraint(pobRUpperLegToRLowerLegConstraintInstance);
		//CPhysicsWorld::Get().GetHavokWorldP()->addConstraint(pobSpineToHeadConstraintInstance);
		//CPhysicsWorld::Get().GetHavokWorldP()->addConstraint(pobSpineToLUpperArmConstraintInstance);
		//CPhysicsWorld::Get().GetHavokWorldP()->addConstraint(pobSpineToRUpperArmConstraintInstance);
		//CPhysicsWorld::Get().GetHavokWorldP()->addConstraint(pobLUpperArmToLLowerArmConstraintInstance);
		//CPhysicsWorld::Get().GetHavokWorldP()->addConstraint(pobRUpperArmToRLowerArmConstraintInstance);

		/*pobRagdollSystem->addRigidBody(pobPelvisRB);
		pobRagdollSystem->addRigidBody(pobSpineRB);
		pobRagdollSystem->addRigidBody(pobLeftUpperLegRB);
		pobRagdollSystem->addRigidBody(pobRightUpperLegRB);
		pobRagdollSystem->addRigidBody(pobLeftLowerLegRB);
		pobRagdollSystem->addRigidBody(pobRightLowerLegRB);
		pobRagdollSystem->addRigidBody(pobHeadRB);
		pobRagdollSystem->addRigidBody(pobLeftUpperArmRB);
		pobRagdollSystem->addRigidBody(pobRightUpperArmRB);
		pobRagdollSystem->addRigidBody(pobLeftLowerArmRB);
		pobRagdollSystem->addRigidBody(pobRightLowerArmRB);

		pobRagdollSystem->addConstraint(pobPelvisToSpineConstraintInstance);
		pobRagdollSystem->addConstraint(pobPelvisToLUpperLegConstraintInstance);
		pobRagdollSystem->addConstraint(pobPelvisToRUpperLegConstraintInstance);
		pobRagdollSystem->addConstraint(pobLUpperLegToLLowerLegConstraintInstance);
		pobRagdollSystem->addConstraint(pobRUpperLegToRLowerLegConstraintInstance);
		pobRagdollSystem->addConstraint(pobSpineToHeadConstraintInstance);
		pobRagdollSystem->addConstraint(pobSpineToLUpperArmConstraintInstance);
		pobRagdollSystem->addConstraint(pobSpineToRUpperArmConstraintInstance);
		pobRagdollSystem->addConstraint(pobLUpperArmToLLowerArmConstraintInstance);
		pobRagdollSystem->addConstraint(pobRUpperArmToRLowerArmConstraintInstance);*/
	}

	void AdvancedRagdoll::LoadRagdoll(  const char*  filename )
	{
		Physics::PhysicsData * data = CPhysicsLoader::Get().LoadPhysics_Neutral(filename);
		if (!data)
				{
			ntError_p(false, ("%s couldn't load in any ragdoll description.\n", ntStr::GetString(ObjectDatabase::Get().GetNameFromPointer(m_entity))));
		}

		m_loaderDataRef = data;

		hkPhysicsSystem* ragdollSystem = data->GetPhysicsSystem().clone();
        if (ragdollSystem)
		{
			hkArray<hkRigidBody *> arrayRB;
			arrayRB.reserveExactly( ragdollSystem->getRigidBodies().getSize() );

			CHierarchy * ragdollHierarchy = m_entity->GetRagdollHierarchy();

			for( int32_t i = 0; i < ragdollSystem->getRigidBodies().getSize(); i++ )
			{
				// change the name of rigid body, remove rigid_ prefix
				hkRigidBody * body = ragdollSystem->getRigidBodies()[i];
#ifndef BINARY_PHYSICS_LOADER
				body->setName((body->getName() + 6));
#endif
				arrayRB.pushBackUnchecked(body);
				
				// 26.10.2006 at the moment ragdolls are wrongly set up. That why just for next
				// few days we are doing this bloody hack. If you read this after 3.11.2006 something
				// is very wrong. 
				//body->getMaterial().setFriction(0.3f);
				//body->getMaterial().setRestitution(0); // bodies are not bouncy at all... 
				//body->setLinearDamping(0.1f);
				//body->setAngularDamping(0.3f);

				// find transformation and set correct position for bodies
				Transform * trans = (int) data->GetTransformHashes().size() > i ? ragdollHierarchy->GetTransformFromHash(data->GetTransformHashes()[i]) : NULL;
				if (!trans)
					trans = ragdollHierarchy->GetRootTransform();

				CMatrix mat = trans->GetWorldMatrix(); 
				body->setTransform(Physics::MathsTools::CMatrixTohkTransform(mat));														
			}

			hkArray<hkConstraintInstance *> arrayConstraint;
			arrayConstraint.reserveExactly( ragdollSystem->getConstraints().getSize() );

			for( int32_t i = 0; i < ragdollSystem->getConstraints().getSize(); i++ )
			{
				hkConstraintInstance* myConstraint = ragdollSystem->getConstraints()[i];
				arrayConstraint.pushBackUnchecked( myConstraint );
#ifndef _RELEASE
				myConstraint->setWantRuntime(true);				
#endif
			}

			// Check we have the right number of rigid bodies and constraints - should always have n-1 constraints
			ntError_p(ragdollSystem->getConstraints().getSize()+1 == ragdollSystem->getRigidBodies().getSize(),("%s has the wrong number of constraints/rigid bodies for a ragdoll.", ntStr::GetString(ObjectDatabase::Get().GetNameFromPointer(m_entity))));
			
			//hkRagdollUtils::reorderAndAlignForRagdoll (arrayRB, arrayConstraint, true);
			hkRagdollUtils::reorderForRagdoll (arrayRB, arrayConstraint);

			m_RagdollSkeleton = hkRagdollUtils::constructSkeletonForRagdoll(arrayRB, arrayConstraint);
			m_RagdollInstance = HK_NEW hsRagdollInstance(arrayRB, arrayConstraint, m_RagdollSkeleton);
		}
		else
		{
			ntError_p(false, ("%s couldn't load in any ragdoll description.\n", ntStr::GetString(ObjectDatabase::Get().GetNameFromPointer(m_entity))));
		}

		// Remove references if we have them
		if ( ragdollSystem ) 
		{
			ragdollSystem->removeReference();
		}


		/*for (int i = 0; i < m_RagdollInstance->getNumBones(); i++)
		{
			hkMatrix3 obInertia;
			m_RagdollInstance->getRigidBodyOfBone(i)->getInertiaLocal(obInertia);
			obInertia.getColumn(0).mul4(0.05f);
			obInertia.getColumn(1).mul4(0.05f);
			obInertia.getColumn(2).mul4(0.05f);
			m_RagdollInstance->getRigidBodyOfBone(i)->setInertiaLocal(obInertia);
		}*/
	}	

	void AdvancedRagdoll::SetLinearVelocity( const CDirection& p_linearVelocity )
	{
		Physics::WriteAccess mutex;

		hkVector4 obVelocity(p_linearVelocity.X(),p_linearVelocity.Y(),p_linearVelocity.Z());
		ntAssert(obVelocity.isOk3());

		for(int iRigid = 0;
			iRigid < m_RagdollInstance->getNumBones();
			++iRigid)
		{
			hkRigidBody * body = m_RagdollInstance->getRigidBodyOfBone(iRigid);
			if (body->isActive())
				body->setLinearVelocity( obVelocity );
		}
	}

	void AdvancedRagdoll::SetPelvisLinearVelocity( const CDirection& p_linearVelocity )
	{
		Physics::WriteAccess mutex;

		hkVector4 obVelocity(p_linearVelocity.X(),p_linearVelocity.Y(),p_linearVelocity.Z());
		ntAssert(obVelocity.isOk3());

		m_RagdollInstance->getRigidBodyOfBone(PELVIS)->setLinearVelocity( obVelocity );
	}

	void AdvancedRagdoll::AddLinearVelocity( const CDirection& p_linearVelocity )
	{
		Physics::WriteAccess mutex;
	
		hkVector4 obVelocity(p_linearVelocity.X(),p_linearVelocity.Y(),p_linearVelocity.Z());
		ntAssert(obVelocity.isOk3());
		
		for(int iRigid = 0;
			iRigid < m_RagdollInstance->getNumBones();
			++iRigid)
		{
			hkVector4 obVelocityThisBone = m_RagdollInstance->getRigidBodyOfBone(iRigid)->getLinearVelocity();
			obVelocityThisBone.add4( obVelocity );
			m_RagdollInstance->getRigidBodyOfBone(iRigid)->setLinearVelocity( obVelocityThisBone );
		}
	}

	void AdvancedRagdoll::SetAngularVelocity( const CDirection& obAngularVelocity )
	{
		Physics::WriteAccess mutex;

		hkVector4 obVelocity(obAngularVelocity.X(),obAngularVelocity.Y(),obAngularVelocity.Z());
		ntAssert(obVelocity.isOk3());

		for(int iRigid = 0;
			iRigid < m_RagdollInstance->getNumBones();
			++iRigid)
		{
			m_RagdollInstance->getRigidBodyOfBone(iRigid)->setAngularVelocity( obVelocity );
		}
	}

	void AdvancedRagdoll::ApplyLinearImpulse (const CDirection& obImpulse)
	{
		Physics::WriteAccess mutex;

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD

		if (IsActive())
		{
			for(int iRigid = 0; iRigid< m_RagdollInstance->getNumBones(); ++iRigid)
			{
				hkVector4 obImp(obImpulse.X(),obImpulse.Y(),obImpulse.Z());
				ntAssert(obImp.isOk3());
				m_RagdollInstance->getRigidBodyOfBone(iRigid)->applyLinearImpulse(obImp);
			}
		}
#else
		UNUSED( obImpulse );
#endif
	}

	void AdvancedRagdoll::ApplyAngularImpulse (const CDirection& obImpulse)
	{
		Physics::WriteAccess mutex;

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD

		if (IsActive())
		{
			for(int iRigid = 0; iRigid< m_RagdollInstance->getNumBones(); ++iRigid)
			{
				hkVector4 obImp(obImpulse.X(),obImpulse.Y(),obImpulse.Z());
				ntAssert(obImp.isOk3());
				m_RagdollInstance->getRigidBodyOfBone(iRigid)->applyAngularImpulse(obImp);
			}
		}
#else
		UNUSED( obImpulse );
#endif
	}

	void AdvancedRagdoll::ApplyLocalisedLinearImpulse (const CDirection& obImpulse,const CVector& obPoint)
	{
		Physics::WriteAccess mutex;

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD

		if (IsActive())
		{
			for(int iRigid = 0; iRigid< m_RagdollInstance->getNumBones(); ++iRigid)
			{
				hkVector4 obImp(obImpulse.X(),obImpulse.Y(),obImpulse.Z());
				ntAssert(obImp.isOk3());
				m_RagdollInstance->getRigidBodyOfBone(iRigid)->applyPointImpulse(obImp,hkVector4(obPoint.X(),obPoint.Y(),obPoint.Z()));
			}
		}
#else
		UNUSED( obImpulse );
		UNUSED( obPoint );
#endif
	}

	void AdvancedRagdoll::ApplyImpulseToBody(int iHitArea, const CPoint& obPosition, const CDirection& obImpulse)
	{
		if (IsActive())
		{
			int bone; 
			switch(iHitArea)
			{
			case Physics::RAGDOLL_PELVIS_MATERIAL:			
				bone = PELVIS_BONE;
				break;
			case Physics::RAGDOLL_SPINE_00_MATERIAL:
				bone = SPINE_BONE;
				break;
			case Physics::RAGDOLL_HEAD_MATERIAL:
				bone = HEAD_BONE;
				break;
			case Physics::RAGDOLL_L_ARM_MATERIAL:
				bone = L_FORARM_BONE;
				break;
			case Physics::RAGDOLL_L_ELBOW_MATERIAL:
				bone = L_ELBOW_BONE;
				break;
			case Physics::RAGDOLL_L_LEG_MATERIAL:
				bone = L_LEG_BONE;
				break;
			case Physics::RAGDOLL_L_KNEE_MATERIAL:
				bone = L_KNEE_BONE;
				break;
			case Physics::RAGDOLL_R_ARM_MATERIAL:
				bone = R_FORARM_BONE;
				break;
			case Physics::RAGDOLL_R_ELBOW_MATERIAL:
				bone = R_ELBOW_BONE;
				break;
			case Physics::RAGDOLL_R_LEG_MATERIAL:
				bone = R_LEG_BONE;
				break;
			case Physics::RAGDOLL_R_KNEE_MATERIAL:
				bone = R_KNEE_BONE;
				break;
			default:
				bone = -1;
				break;
			};

			if (bone >= 0)
			{
				m_RagdollInstance->getRigidBodyOfBone(bone)->applyPointImpulse(MathsTools::CDirectionTohkVector(obImpulse), MathsTools::CPointTohkVector(obPosition));
			}
		}
	}


	void AdvancedRagdoll::ApplyForce(const CDirection& obForce)
	{
		Physics::WriteAccess mutex;

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD

		for(int iRigid = 0;
			iRigid<m_RagdollInstance->getNumBones();
			++iRigid)
		{
			hkVector4 obFor(obForce.X() * 2.f , obForce.Y() * 2.f , obForce.Z() * 2.f );
			ntAssert(obFor.isOk3());
			m_RagdollInstance->getRigidBodyOfBone(iRigid)->applyForce(CPhysicsWorld::Get().GetStepInfo().m_deltaTime, obFor);	
		}
#else
		UNUSED( obForce );
#endif
	}	

	CDirection AdvancedRagdoll::GetAngularVelocity()
	{
		Physics::ReadAccess mutex;

		CDirection obAvgVelocity;
		obAvgVelocity.Clear();
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD

		for(int iRigid = 0;
			iRigid < m_RagdollInstance->getNumBones();
			++iRigid)
		{
			hkVector4 obThisHKVelocity = m_RagdollInstance->getRigidBodyOfBone(iRigid)->getAngularVelocity();
			
			obAvgVelocity.X() += obThisHKVelocity(0);
			obAvgVelocity.Y() += obThisHKVelocity(1);
			obAvgVelocity.Z() += obThisHKVelocity(2);
		}

		obAvgVelocity /= (float)m_RagdollInstance->getNumBones();
#endif
		return obAvgVelocity;
	}

	CDirection AdvancedRagdoll::GetLinearVelocity()
	{
		//Physics::ReadAccess mutex;

		CDirection obAvgVelocity;
		obAvgVelocity.Clear();
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD

		for(int iRigid = 0;
			iRigid < m_RagdollInstance->getNumBones();
			++iRigid)
		{
			hkVector4 obThisHKVelocity = m_RagdollInstance->getRigidBodyOfBone(iRigid)->getLinearVelocity();
			
			obAvgVelocity.X() += obThisHKVelocity(0);
			obAvgVelocity.Y() += obThisHKVelocity(1);
			obAvgVelocity.Z() += obThisHKVelocity(2);
		}

		obAvgVelocity /= (float)m_RagdollInstance->getNumBones();
#endif
		return obAvgVelocity;
	}

	void AdvancedRagdoll::SetupAnimatedSkeleton()
	{
		Physics::WriteAccess mutex;

		// Delete previous instance...
		if( m_AnimatedSkeleton )
		{
			ntError_p( false, ("We aren't deleting all the other memory held by the m_AnimatedSkeleton structure.") );
			HK_DELETE( m_AnimatedSkeleton );
		}

		// Create a new skeleton...
		m_AnimatedSkeleton		= HK_NEW hkSkeleton();
		CHierarchy* hierarchy	= m_entity->GetHierarchy();

		m_AnimatedSkeleton->m_numParentIndices	= HierarchyTools::NumberOfTransform(	hierarchy, 
																						hierarchy->GetRootTransform()->GetFirstChild() );
		m_AnimatedSkeleton->m_numBones			= m_AnimatedSkeleton->m_numParentIndices;

		// This array contains the parent information. For each bone i, the bone parent index is m_AnimatedSkeleton.m_hierarchy[ i ] 
		m_AnimatedSkeleton->m_parentIndices		= NT_NEW hkInt16[ m_AnimatedSkeleton->m_numParentIndices ];
		HierarchyTools::SetSkeletonRecursive(	m_AnimatedSkeleton->m_parentIndices, 
												hierarchy, 
												hierarchy->GetRootTransform()->GetFirstChild());

		// This array contains the actual skeleton bone.
		m_AnimatedSkeleton->m_bones				= hkAllocate<hkBone*> ( m_AnimatedSkeleton->m_numParentIndices, HK_MEMORY_CLASS_UTILITIES );

		m_AnimatedSkeleton->m_referencePose		= HK_NEW hkQsTransform[ m_AnimatedSkeleton->m_numParentIndices ];
		m_AnimatedSkeleton->m_numReferencePose	= m_AnimatedSkeleton->m_numParentIndices;


		HierarchyTools::SetSkeletonBonesRecursive(	m_AnimatedSkeleton->m_bones, 
													hierarchy, 
													m_AnimatedSkeleton->m_referencePose, 
													hierarchy->GetRootTransform()->GetFirstChild());
	}

	#define fRagdollInertiaFactor	15.f
	void AdvancedRagdoll::SetupRigidBodies()
	{
		Physics::WriteAccess mutex;

		// Set some filters !
		// we got a ragdoll body, so we create a ragdoll collision info
		Physics::RagdollCollisionFlag iRagFlag; iRagFlag.base = 0;

		// I am a ragdoll body
		iRagFlag.flags.i_am |= Physics::RAGDOLL_BIT;
		
		// I am important
		if (m_entity->IsImportant())
			iRagFlag.flags.i_am_important = 1; 

		// I collide with
		iRagFlag.flags.i_collide_with = (	Physics::RAGDOLL_BIT					|
											Physics::SMALL_INTERACTABLE_BIT			|
											Physics::LARGE_INTERACTABLE_BIT			|   
											Physics::TRIGGER_VOLUME_BIT				|
											Physics::AI_WALL_BIT );
		if ( g_ShellOptions->m_bInteractableRagdolls )
		{
			iRagFlag.flags.i_collide_with |= ( Physics::CHARACTER_CONTROLLER_PLAYER_BIT |
											   Physics::CHARACTER_CONTROLLER_ENEMY_BIT ) ;
		}
		
		hkRigidBody *pobSpine = NULL;
		hkRigidBody *pobPelvis = NULL;
		
		for(hkInt16 i = 0; i < m_RagdollInstance->getNumBones(); i++)
		{
			hkRigidBody *pobThis = m_RagdollInstance->getRigidBodyOfBone(i);
			hkMatrix3 inertia;
			pobThis->getInertiaLocal(inertia);
			inertia.mul(fRagdollInertiaFactor);
			pobThis->setInertiaLocal(inertia);

			int iIndex = 1;
			int iBone = -1;
			const char* currentBodyName = pobThis->getName();
			
			while(-1 == iBone) {
				if( 0 == strcmp(currentBodyName, asRagdollBodyName[iIndex]) )
					iBone = iIndex;
				iIndex++;

				if((iIndex >= asRagdollBodyNameSize) && (-1 == iBone))
					iBone = -2;
			};

			// We need to inform rigid bodies, which part of ragdoll they represents... 
			// Further we need to add ragdoll physics materials to bodies shapes. 
			// The following code is quite inflexible, but perfectly sufficient for now. 
			// Later all of this configuration should be move to data and config files... 
			switch(iBone)
			{
			case 1:
				{
					iRagFlag.flags.ragdoll_material = Physics::RAGDOLL_PELVIS_MATERIAL;
					PELVIS_BONE = i;
					pobPelvis = pobThis;
					const hkShape * shape = pobThis->getCollidable()->getShape();
					Tools::SetShapeUserData(const_cast<hkShape *>(shape), (hkUlong) &obPelvisPhysicsMaterial);
				}
				break;
			case 2:
				{
					iRagFlag.flags.ragdoll_material = Physics::RAGDOLL_SPINE_00_MATERIAL;
					SPINE_BONE = i;
					pobSpine = pobThis;
					const hkShape * shape = pobThis->getCollidable()->getShape();
					Tools::SetShapeUserData(const_cast<hkShape *>(shape), (hkUlong) &obSpinePhysicsMaterial);
				}
				break;
			case 6:
				{
					iRagFlag.flags.ragdoll_material = Physics::RAGDOLL_HEAD_MATERIAL;
					HEAD_BONE = i;
					const hkShape * shape = pobThis->getCollidable()->getShape();
					Tools::SetShapeUserData(const_cast<hkShape *>(shape), (hkUlong) &obHeadPhysicsMaterial);
				}
				break;
			case 9:
				{
					iRagFlag.flags.ragdoll_material = Physics::RAGDOLL_L_ARM_MATERIAL;
					L_FORARM_BONE = i;
					const hkShape * shape = pobThis->getCollidable()->getShape();
					Tools::SetShapeUserData(const_cast<hkShape *>(shape), (hkUlong) &obLArmPhysicsMaterial);
				}
				break;
			case 10:
				{
					iRagFlag.flags.ragdoll_material = Physics::RAGDOLL_L_ELBOW_MATERIAL;
					L_ELBOW_BONE = i;
					const hkShape * shape = pobThis->getCollidable()->getShape();
					Tools::SetShapeUserData(const_cast<hkShape *>(shape), (hkUlong) &obLArmPhysicsMaterial);
				}
				break;
			case 13:
				{
					iRagFlag.flags.ragdoll_material = Physics::RAGDOLL_L_LEG_MATERIAL;
					L_LEG_BONE = i;
					const hkShape * shape = pobThis->getCollidable()->getShape();
					Tools::SetShapeUserData(const_cast<hkShape *>(shape), (hkUlong) &obLLegPhysicsMaterial);
				}
				break;
			case 14:
				{
					iRagFlag.flags.ragdoll_material = Physics::RAGDOLL_L_KNEE_MATERIAL;
					L_KNEE_BONE = i;
					const hkShape * shape = pobThis->getCollidable()->getShape();
					Tools::SetShapeUserData(const_cast<hkShape *>(shape), (hkUlong) &obLLegPhysicsMaterial);
				}
				break;
			case 16:
				{
					iRagFlag.flags.ragdoll_material = Physics::RAGDOLL_R_ARM_MATERIAL;
					R_FORARM_BONE = i;
					const hkShape * shape = pobThis->getCollidable()->getShape();
					Tools::SetShapeUserData(const_cast<hkShape *>(shape), (hkUlong) &obRArmPhysicsMaterial);
				}
				break;
			case 17:
				{
					iRagFlag.flags.ragdoll_material = Physics::RAGDOLL_R_ELBOW_MATERIAL;
					R_ELBOW_BONE = i;
					const hkShape * shape = pobThis->getCollidable()->getShape();
					Tools::SetShapeUserData(const_cast<hkShape *>(shape), (hkUlong) &obRArmPhysicsMaterial);
				}
				break;
			case 20:
				{
					iRagFlag.flags.ragdoll_material = Physics::RAGDOLL_R_LEG_MATERIAL;
					R_LEG_BONE = i;
					const hkShape * shape = pobThis->getCollidable()->getShape();
					Tools::SetShapeUserData(const_cast<hkShape *>(shape), (hkUlong) &obRLegPhysicsMaterial);
				}
				break;
			case 21:
				{
					iRagFlag.flags.ragdoll_material = Physics::RAGDOLL_R_KNEE_MATERIAL;
					R_KNEE_BONE = i;
					const hkShape * shape = pobThis->getCollidable()->getShape();
					Tools::SetShapeUserData(const_cast<hkShape *>(shape), (hkUlong) &obRLegPhysicsMaterial);
				}
				break;
			default:
				iRagFlag.flags.ragdoll_material = Physics::RAGDOLL_NO_COLLIDE;
				break;
			};
		
			pobThis->getCollidableRw()->setCollisionFilterInfo(iRagFlag.base);

			// Conflict between old and new ragdoll system - need to check if there's already a prop there before we try to set it
			if ( m_entity && !m_RagdollInstance->getRigidBodyOfBone(i)->hasProperty(Physics::PROPERTY_ENTITY_PTR))
			{
				hkPropertyValue val((void*)m_entity);
				pobThis->addProperty(Physics::PROPERTY_ENTITY_PTR, val);
			}
		}
		
		const hkArray<hkConstraintInstance *>& constraints = m_RagdollInstance->getConstraintArray();
		for ( hkInt16 i = 0; i < constraints.getSize(); i++)
		{
			hkConstraintInstance* pobConstraint = constraints[i];
			if ( pobConstraint->getRigidBodyA() == pobSpine && pobConstraint->getRigidBodyB() == pobPelvis )
			{
				ntAssert_p( pobConstraint->getData()->getType() == hkConstraintData::CONSTRAINT_TYPE_RAGDOLL,( "Expecting waist joint to be a ragdoll constraint!" ));
				WAIST_JOINT = i;
			}
		}
	}

	int RagdollStrCmp(const char* pcOne, const char* pcTwo)
	{
		return strcmp(pcOne,pcTwo);
	}

	void AdvancedRagdoll::SetupMappers()
	{
		Physics::WriteAccess mutex;

		hkSkeletonMapperData animated_ragdoll_data;
		hkSkeletonMapperData ragdoll_animated_data;
			
		hkSkeletonMapperUtils::Params params;
		
		params.m_skeletonA = m_AnimatedSkeleton;
		params.m_skeletonB = m_RagdollSkeleton;

		params.m_compareNames = &RagdollStrCmp;

		// Explicit mappings
		/*{
			params.m_userMappingsAtoB.setSize(11);
			params.m_userMappingsAtoB[0].m_boneIn = "Biped_Pelvis";
			params.m_userMappingsAtoB[0].m_boneOut = "Biped_Pelvis";			
			params.m_userMappingsAtoB[1].m_boneIn = "Biped_L_Thigh";
			params.m_userMappingsAtoB[1].m_boneOut = "Biped_L_Thigh";
			params.m_userMappingsAtoB[2].m_boneIn = "Biped_R_Thigh";
			params.m_userMappingsAtoB[2].m_boneOut = "Biped_R_Thigh";
			params.m_userMappingsAtoB[3].m_boneIn = "Biped_L_Calf";
			params.m_userMappingsAtoB[3].m_boneOut = "Biped_L_Calf";
			params.m_userMappingsAtoB[4].m_boneIn = "Biped_R_Calf";
			params.m_userMappingsAtoB[4].m_boneOut = "Biped_R_Calf";
			params.m_userMappingsAtoB[5].m_boneIn = "Biped_Spine0";
			params.m_userMappingsAtoB[5].m_boneOut = "Biped_Spine0";
			params.m_userMappingsAtoB[6].m_boneIn = "Biped_L_UpperArm";
			params.m_userMappingsAtoB[6].m_boneOut = "Biped_L_UpperArm";
			params.m_userMappingsAtoB[7].m_boneIn = "Biped_R_UpperArm";
			params.m_userMappingsAtoB[7].m_boneOut = "Biped_R_UpperArm";
			params.m_userMappingsAtoB[8].m_boneIn = "Biped_L_ForeArm";
			params.m_userMappingsAtoB[8].m_boneOut = "Biped_L_ForeArm";
			params.m_userMappingsAtoB[9].m_boneIn = "Biped_R_ForeArm";
			params.m_userMappingsAtoB[9].m_boneOut = "Biped_R_ForeArm";
			params.m_userMappingsAtoB[10].m_boneIn = "Biped_Head";
			params.m_userMappingsAtoB[10].m_boneOut = "Biped_Head";

			params.m_userMappingsBtoA.setSize(11);
			params.m_userMappingsBtoA[0].m_boneIn = "Biped_Pelvis";
			params.m_userMappingsBtoA[0].m_boneOut = "Biped_Pelvis";			
			params.m_userMappingsBtoA[1].m_boneIn = "Biped_L_Thigh";
			params.m_userMappingsBtoA[1].m_boneOut = "Biped_L_Thigh";
			params.m_userMappingsBtoA[2].m_boneIn = "Biped_R_Thigh";
			params.m_userMappingsBtoA[2].m_boneOut = "Biped_R_Thigh";
			params.m_userMappingsBtoA[3].m_boneIn = "Biped_L_Calf";
			params.m_userMappingsBtoA[3].m_boneOut = "Biped_L_Calf";
			params.m_userMappingsBtoA[4].m_boneIn = "Biped_R_Calf";
			params.m_userMappingsBtoA[4].m_boneOut = "Biped_R_Calf";
			params.m_userMappingsBtoA[5].m_boneIn = "Biped_Spine0";
			params.m_userMappingsBtoA[5].m_boneOut = "Biped_Spine0";
			params.m_userMappingsBtoA[6].m_boneIn = "Biped_L_UpperArm";
			params.m_userMappingsBtoA[6].m_boneOut = "Biped_L_UpperArm";
			params.m_userMappingsBtoA[7].m_boneIn = "Biped_R_UpperArm";
			params.m_userMappingsBtoA[7].m_boneOut = "Biped_R_UpperArm";
			params.m_userMappingsBtoA[8].m_boneIn = "Biped_L_ForeArm";
			params.m_userMappingsBtoA[8].m_boneOut = "Biped_L_ForeArm";
			params.m_userMappingsBtoA[9].m_boneIn = "Biped_R_ForeArm";
			params.m_userMappingsBtoA[9].m_boneOut = "Biped_R_ForeArm";
			params.m_userMappingsBtoA[10].m_boneIn = "Biped_Head";
			params.m_userMappingsBtoA[10].m_boneOut = "Biped_Head";
		};*/

		params.m_autodetectChains = false;
		params.m_autodetectSimple = true;

		hkSkeletonMapperUtils::createMapping(params, animated_ragdoll_data, ragdoll_animated_data);

		m_AnimatedToRagdollMapper = HK_NEW hkSkeletonMapper(animated_ragdoll_data);
		m_RagdollToAnimatedMapper = HK_NEW hkSkeletonMapper(ragdoll_animated_data);
	}

	// This resets the ragdoll position in the world - it's only ever called when we've just placed the ragdoll
	// in the world and it's been previously completely inactive (i.e. when we switch from character controller to ragdoll)
	// Could use it on every state change but it looks funny, so I only use it when we're placing a ragdoll in the world from scratch.
	void AdvancedRagdoll::ResetEntireRagdoll(bool bPositions, bool bVelocities, bool bSquish)
	{
		Physics::WriteAccess mutex;

		// Reset bone positions?
		if (bPositions)
		{
			CHierarchy* hierarchy	= m_entity->GetHierarchy();

			// Prepare a couple of arrays to fill with bone transforms
			const int ragdollNumBones = m_RagdollInstance->getNumBones();
			hkArray<hkQsTransform> ragdollPose( ragdollNumBones );
			ragdollPose.setSizeUnchecked( ragdollNumBones );			
			const int numBones = m_AnimatedSkeleton->m_numBones;
			hkArray<hkQsTransform> animatedPose( numBones );
			animatedPose.setSizeUnchecked( numBones );
			
			// Sample the hierarchy and combine into a single animated pose
			HierarchyTools::GetWorldPoseRecursive( animatedPose, hierarchy, hierarchy->GetRootTransform()->GetFirstChild() );
	
			// Use our havok mapper to put it onto the low res ragdoll
			m_AnimatedToRagdollMapper->mapPose( animatedPose.begin(), 
												m_RagdollInstance->getSkeleton()->m_referencePose, 
												ragdollPose.begin(), hkSkeletonMapper::CURRENT_POSE ); 

			
			// For each bone in the ragoll, reset the position and the velocity if wanted
			for (int b=0; b<ragdollNumBones; b++)
			{			
				hkRigidBody* rbody = m_RagdollInstance->getRigidBodyOfBone(b);

				hkQsTransform rbTransform; 
				rbTransform.setMul(hkQsTransform::getIdentity(), ragdollPose[b]);
				rbTransform.m_rotation.normalize();	

				rbody->setPositionAndRotation(rbTransform.getTranslation(), rbTransform.getRotation());
				
				if (bVelocities)
				{
					rbody->setLinearVelocity(hkVector4::getZero());
					rbody->setAngularVelocity(hkVector4::getZero());
				}
			}

			if (bSquish && !g_ShellOptions->m_bDoOneSideCollision )
			{
				// Squish the whole skeleton towards something so that we have a chance of our bones being inside the world
				// This stops most on-activation wall penetrations
				Physics::AdvancedCharacterController* pobAdvCC = (Physics::AdvancedCharacterController*) m_entity->GetPhysicsSystem()->GetFirstGroupByType( Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER );
				ntError(pobAdvCC);
				for (int b=0; b<ragdollNumBones; b++)
				{
					// If we can, use the last active position of the character controller capsule phantom to squish towards
					if (pobAdvCC->GetCharacterControllerWasActiveLastFrame())
					{
						// Get a here to phantom vector
						hkVector4 obBonePosition(m_RagdollInstance->getRigidBodyOfBone(b)->getPosition());
						hkVector4 obMovement(Physics::MathsTools::CPointTohkVector( pobAdvCC->GetCharacterControllerPhantomPositionLastFrame() ));
						obMovement.sub4(obBonePosition);

						// Scale it by some amount so we don't end up with all bones completely on top of each other
						// Bone-bone penetration is fine as long as they stay in relatively the same positions in their constraints
						// The animation will take over next frame and push them to where they need to go
						obMovement(0) *= 0.75f;
						obMovement(1) *= 0.4f; // Scale the Y component less because we get ragdolls jumping up from the floor if they're being ragdolled on a move which end with them on their backs
						obMovement(2) *= 0.75f;

						// Move and set
						obBonePosition.add4(obMovement);
						m_RagdollInstance->getRigidBodyOfBone(b)->setPosition(obBonePosition);			
					}
					// Otherwise just use the head - I haven't seen this used in the E3 level, so don't worry about it
					else
					{
						if (b != HEAD_BONE)
						{
							// Get a here to head vector
							hkVector4 obBonePosition(m_RagdollInstance->getRigidBodyOfBone(b)->getPosition());
							hkVector4 obMovement(m_RagdollInstance->getRigidBodyOfBone(HEAD_BONE)->getPosition());
							obMovement.sub4(obBonePosition);
							obMovement.mul4(0.75f);
							// Move us towards it
							obBonePosition.add4(obMovement);
							m_RagdollInstance->getRigidBodyOfBone(b)->setPosition(obBonePosition);
						}
					}	

					// Use this for testing to see where the bones are immediately after activation in the havok debugger
					//m_RagdollInstance->getRigidBodyOfBone(b)->setMotionType(hkMotion::MOTION_FIXED);
				}
			}
		}
		// Just reset bone velocities?
		else if (bVelocities)
		{
			for(int iRigid = 0; iRigid< m_RagdollInstance->getNumBones(); ++iRigid)
			{
				m_RagdollInstance->getRigidBodyOfBone( iRigid )->setLinearVelocity( hkVector4::getZero() );
				m_RagdollInstance->getRigidBodyOfBone( iRigid )->setAngularVelocity( hkVector4::getZero() );
			}
		}		
	}

	void AdvancedRagdoll::SetAnimatedBones(int iBoneFlags)
	{
		m_iAnimatedBoneFlags = iBoneFlags;
		m_aiBonesToAnimate.clear();

		if (m_iAnimatedBoneFlags & ALL_BONES)
		{
			m_aiBonesToAnimate.pushBack(PELVIS_BONE);
			m_aiBonesToAnimate.pushBack(SPINE_BONE);
			m_aiBonesToAnimate.pushBack(L_FORARM_BONE);
			m_aiBonesToAnimate.pushBack(R_FORARM_BONE);
			m_aiBonesToAnimate.pushBack(L_ELBOW_BONE);
			m_aiBonesToAnimate.pushBack(R_ELBOW_BONE);
			m_aiBonesToAnimate.pushBack(HEAD_BONE);
			m_aiBonesToAnimate.pushBack(L_LEG_BONE);
			m_aiBonesToAnimate.pushBack(R_LEG_BONE);
			m_aiBonesToAnimate.pushBack(L_KNEE_BONE);
			m_aiBonesToAnimate.pushBack(R_KNEE_BONE);
			return;
		}

		if (m_iAnimatedBoneFlags & PELVIS)
			m_aiBonesToAnimate.pushBack(PELVIS_BONE);
		if (m_iAnimatedBoneFlags & SPINE)
			m_aiBonesToAnimate.pushBack(SPINE_BONE);
		if (m_iAnimatedBoneFlags & UPPER_ARM_LEFT)
			m_aiBonesToAnimate.pushBack(L_FORARM_BONE);
		if (m_iAnimatedBoneFlags & UPPER_ARM_RIGHT)
			m_aiBonesToAnimate.pushBack(R_FORARM_BONE);
		if (m_iAnimatedBoneFlags & LOWER_ARM_LEFT)
			m_aiBonesToAnimate.pushBack(L_ELBOW_BONE);
		if (m_iAnimatedBoneFlags & LOWER_ARM_RIGHT)
			m_aiBonesToAnimate.pushBack(R_ELBOW_BONE);
		if (m_iAnimatedBoneFlags & HEAD)
			m_aiBonesToAnimate.pushBack(HEAD_BONE);
		if (m_iAnimatedBoneFlags & UPPER_LEG_LEFT)
			m_aiBonesToAnimate.pushBack(L_LEG_BONE);
		if (m_iAnimatedBoneFlags & UPPER_LEG_RIGHT)
			m_aiBonesToAnimate.pushBack(R_LEG_BONE);
		if (m_iAnimatedBoneFlags & LOWER_LEG_LEFT)
			m_aiBonesToAnimate.pushBack(L_KNEE_BONE);
		if (m_iAnimatedBoneFlags & LOWER_LEG_RIGHT)
			m_aiBonesToAnimate.pushBack(R_KNEE_BONE);
	}

	void AdvancedRagdoll::AddAnimatedBone(int iBoneFlag)
	{
		m_iAnimatedBoneFlags |= iBoneFlag;

		if (iBoneFlag & PELVIS)
			m_aiBonesToAnimate.pushBack(PELVIS_BONE);
		else if (iBoneFlag & SPINE)
			m_aiBonesToAnimate.pushBack(SPINE_BONE);
		else if (iBoneFlag & UPPER_ARM_LEFT)
			m_aiBonesToAnimate.pushBack(L_FORARM_BONE);
		else if (iBoneFlag & UPPER_ARM_RIGHT)
			m_aiBonesToAnimate.pushBack(R_FORARM_BONE);
		else if (iBoneFlag & LOWER_ARM_LEFT)
			m_aiBonesToAnimate.pushBack(L_ELBOW_BONE);
		else if (iBoneFlag & LOWER_ARM_RIGHT)
			m_aiBonesToAnimate.pushBack(R_ELBOW_BONE);
		else if (iBoneFlag & HEAD)
			m_aiBonesToAnimate.pushBack(HEAD_BONE);
		else if (iBoneFlag & UPPER_LEG_LEFT)
			m_aiBonesToAnimate.pushBack(L_LEG_BONE);
		else if (iBoneFlag & UPPER_LEG_RIGHT)
			m_aiBonesToAnimate.pushBack(R_LEG_BONE);
		else if (iBoneFlag & LOWER_LEG_LEFT)
			m_aiBonesToAnimate.pushBack(L_KNEE_BONE);
		else if (iBoneFlag & LOWER_LEG_RIGHT)
			m_aiBonesToAnimate.pushBack(R_KNEE_BONE);
	}

	bool AdvancedRagdoll::CheckBoneInContactWithWorld(hkRigidBody* rbody)
	{
		Physics::WriteAccess mutex;

		hkClosestCdPointCollector obCollector;
		obCollector.reset();
		CPhysicsWorld::Get().GetClosestPoints( rbody->getCollidable(), (const hkCollisionInput &)*CPhysicsWorld::Get().GetCollisionInput(), obCollector );

		// If we got contacts...
		if ( obCollector.hasHit() )
		{			
			hkRigidBody* pobBody = hkGetRigidBody(obCollector.getHit().m_rootCollidableB);
			if (
				(obCollector.getHit().m_rootCollidableB->getShape()->getType() == HK_SHAPE_MOPP || 
				obCollector.getHit().m_rootCollidableB->getShape()->getType() == HK_SHAPE_BV_TREE ||
				obCollector.getHit().m_rootCollidableB->getShape()->getType() == HK_SHAPE_BV) // We have static level geom shapes
				||
				(pobBody && !(pobBody->getMass() > 0.0f)) // We have a fixed rigid body of some form
				)
			{
				return true;
			}
		}

		return false;
	}
	
	
	void AdvancedRagdoll::ResetRagdollPoseToMatchCurrentHierarchy()
	{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD

		Physics::WriteAccess mutex;

		CHierarchy* hierarchy	= m_entity->GetHierarchy();

		// Get the pose that we'll use to drive the ragdoll
		const int ragdollNumBones = m_RagdollInstance->getNumBones();
		hkArray<hkQsTransform> ragdollPose( ragdollNumBones );
		ragdollPose.setSizeUnchecked( ragdollNumBones );
		
		const int numBones = m_AnimatedSkeleton->m_numBones;
		hkArray<hkQsTransform> animatedPose( numBones );
		animatedPose.setSizeUnchecked( numBones );
		
		// Sample the active animations and combine into a single pose
		HierarchyTools::GetWorldPoseRecursive( animatedPose, hierarchy, hierarchy->GetRootTransform()->GetFirstChild() );
			
		m_AnimatedToRagdollMapper->mapPose( animatedPose.begin(), 
			m_RagdollInstance->getSkeleton()->m_referencePose, 
			ragdollPose.begin(), hkSkeletonMapper::CURRENT_POSE );

		for (int b=0; b<ragdollNumBones; b++)
		{			
			hkRigidBody* rbody = m_RagdollInstance->getRigidBodyOfBone(b);

			hkQsTransform rbTransform; 
			rbTransform.setMul(hkQsTransform::getIdentity(), ragdollPose[b]);
			rbTransform.m_rotation.normalize();

			// Try to pass on some velocity so if we get deactivated we don't stop dead
			hkTransform obCurrent;
			rbody->approxTransformAt(Physics::CPhysicsWorld::Get().GetFrameTime(),obCurrent);
			hkVector4 obImpulse(rbTransform.getTranslation());
			obImpulse.sub4(obCurrent.getTranslation());
			rbody->applyLinearImpulse(obImpulse);

			rbody->setPositionAndRotation(rbTransform.getTranslation(), rbTransform.getRotation());
		}
#endif
	}	
	
	void AdvancedRagdoll::DriveRagdollBonesToAnimation( const hkArray<hkQsTransform>& aobPoseToDriveTo, hkArray<int>& aiBonesToAnimate, float fFrameDelta )
	{
		if (fFrameDelta == 0)
			return; // nothing to do

		Physics::WriteAccess mutex;		

#ifdef USE_ASYCHRONOUS_SIMULATION
		hkWorld * world = Physics::CPhysicsWorld::Get().GetHavokWorldP();

		// All synchronization is done on Havok time steps. 

		// Why not on frame times? It leads to horrible instabilities if havok time steps and frame steps are different
		// If we ask hardkeyframe to reach position between the havok time steps, we have no control over position 
		// on time steps and it can diverge... 

		float predictTime = world->getCurrentPsiTime() - world->getCurrentTime();
		float physTime = 0;
		float worldStep = Physics::CPhysicsWorld::Get().GetLastStep();
			
		if (predictTime < fFrameDelta)			
		{
			// search for havok step close to next frame.
			float space  = fFrameDelta - predictTime;
			int n = (int) (space / worldStep) + 1;
			predictTime += n * worldStep;
			physTime += n * worldStep;
		}
		else
		{
			predictTime += worldStep;
			physTime += worldStep;
		}

		float predictCoef = 1 + predictTime / fFrameDelta;
				
#else
		float physTime = fFrameDelta; 
		float predictCoef = 2.0f;
#endif
		
		if (m_lastPose.getSize() == 0) 
		{
			// not intialized yet
			m_lastPose.setSize(aobPoseToDriveTo.getSize());

			// the first frame will be simply without prediction
			for(int i = 0; i < aiBonesToAnimate.getSize(); i++)
		{
				m_lastPose[i].m_lastTranslation = aobPoseToDriveTo[i].getTranslation();
				m_lastPose[i].m_lastRotation = aobPoseToDriveTo[i].getRotation();
				m_lastPose[i].m_lastAnimLinearVelocity.setZero4();
				m_lastPose[i].m_lastAnimAngularVelocity.setZero4();
				m_lastPose[i].m_forLastAnimLinearVelocity.setZero4();
				m_lastPose[i].m_forLastAnimAngularVelocity.setZero4();
			}
		}

		for(int i = 0; i < aiBonesToAnimate.getSize(); i++)
		{
			int iBone = aiBonesToAnimate[i];

			KeyframedMotionParams& keyframedParam = m_lastPose[iBone];
			CMatrix lastTransform = keyframedParam.GetLastTransform();
			CMatrix currentTransform = MathsTools::hkQsTransformToCMatrix(aobPoseToDriveTo[iBone]);

			// calculate current velocities in animation
			float invDeltaTime = 1.0f / fFrameDelta;
			keyframedParam.m_forLastAnimLinearVelocity = keyframedParam.m_lastAnimLinearVelocity;
			keyframedParam.m_forLastAnimAngularVelocity = keyframedParam.m_lastAnimAngularVelocity;

			keyframedParam.m_lastAnimLinearVelocity.setSub4( MathsTools::CPointTohkVector(currentTransform.GetTranslation()), keyframedParam.m_lastTranslation);
			keyframedParam.m_lastAnimLinearVelocity.mul4(invDeltaTime);

			hkQuaternion quatDif;
			hkQuaternion currentRot = MathsTools::CMatrixTohkQuaternion(currentTransform);

			quatDif.setMulInverse( currentRot, keyframedParam.m_lastRotation);
			quatDif.normalize();
			hkReal angle = quatDif.getAngle();
			if(angle < 1e-3f)
			{
				keyframedParam.m_lastAnimAngularVelocity.setZero4();
			}
			else
			{
				quatDif.getAxis(keyframedParam.m_lastAnimAngularVelocity);
				keyframedParam.m_lastAnimAngularVelocity.setMul4(angle * invDeltaTime, keyframedParam.m_lastAnimAngularVelocity);		
			}	
			
		    keyframedParam.m_lastRotation = currentRot;
			keyframedParam.m_lastTranslation = MathsTools::CPointTohkVector(currentTransform.GetTranslation());

			if (physTime <= 0)
			{
				// no havoc step in this frame
				continue;
			}

			CMatrix predictedTransform(CMatrix::Lerp(lastTransform, currentTransform, predictCoef));

			hkVector4		newRigidPosition( MathsTools::CPointTohkVector(predictedTransform.GetTranslation()));
			hkQuaternion	newRigidRotation = MathsTools::CMatrixTohkQuaternion( predictedTransform );				

			// hit it in next 
			hkKeyFrameUtility::applyHardKeyFrame( newRigidPosition, newRigidRotation, 1.0f/physTime, m_RagdollInstance->getRigidBodyOfBone(iBone)  );		
		}
	}	

	
	void AdvancedRagdoll::ApplyDriveToPoseAsynchronously( const hkVector4& nextPosition, const hkQuaternion& nextOrientation, hkReal invFrameTime, hkReal invPhysicsTime, hkRigidBody* body)
	{	
		// Get the number of times the world will step on the next frame
		hkInt32 numFrames = CPhysicsWorld::GetNumberOfStepsWorldWillTakeNextFrame( body->getWorld(), 1.0f / invFrameTime, 1.0f / invPhysicsTime );

		// Get lin vel required
		{
			hkVector4 linearVelocity(0,0,0,0);

			hkVector4 newCenterOfMassPosition(0,0,0,0);
			hkVector4 localCentreOfMass = body->getCenterOfMassLocal();
			newCenterOfMassPosition.setRotatedDir( nextOrientation, localCentreOfMass );
			newCenterOfMassPosition.add4( nextPosition );
			hkVector4 worldCentreOfMass = body->getCenterOfMassInWorld();
			linearVelocity.setSub4( newCenterOfMassPosition, worldCentreOfMass );

			linearVelocity.setMul4(invPhysicsTime, linearVelocity);

			if( numFrames != 0 )
			{
				linearVelocity.mul4( 1.0f / hkReal( numFrames) );
			}
			else
			{
				linearVelocity.setZero4();
			}

			body->setLinearVelocity(linearVelocity);
			//body->applyLinearImpulse(linearVelocity);
		}

		// Get ang vel required
		{
			hkVector4 angularVelocity(0,0,0,0);
			hkQuaternion quatDif(0,0,0,1);
			hkQuaternion bodyRotation = body->getRotation();
			quatDif.setMulInverse(nextOrientation, bodyRotation);
			quatDif.normalize();

			hkReal angle = quatDif.getAngle();
			if(angle < 1e-3f)
			{
				angularVelocity.setZero4();
			}
			else
			{
				quatDif.getAxis(angularVelocity);
				angularVelocity.setMul4(angle * invPhysicsTime, angularVelocity);
			}

			if( numFrames != 0 )
			{
				angularVelocity.mul4( 1.0f / hkReal(numFrames) );
			}
			else
			{
				angularVelocity.setZero4();
			}

			body->setAngularVelocity(angularVelocity);
			//body->applyAngularImpulse(angularVelocity);
		}
	}
	
	
	void AdvancedRagdoll::FullyAnimatedRagdollAnimation( hkArray<hkQsTransform>& ragdollWorldPoseIn, hkArray<hkQsTransform>& animatedWorldPoseOut )
	{
		Physics::ReadAccess mutex;

		//CMatrix obRoot		= m_entity->GetHierarchy()->GetRootTransform()->GetLocalMatrix();
		CMatrix obPelvis	= m_entity->GetHierarchy()->GetRootTransform()->GetFirstChild()->GetLocalMatrix();

		// Map the pose
		m_RagdollToAnimatedMapper->mapPose(ragdollWorldPoseIn.begin(), animatedWorldPoseOut.begin(), animatedWorldPoseOut.begin(), hkSkeletonMapper::NO_CONSTRAINTS );
		hkPose animPose(m_AnimatedSkeleton);
		animPose.setPoseModelSpace(animatedWorldPoseOut); 

		// Update the hierarchy
		HierarchyTools::SetLocalPoseRecursive(animPose.getPoseLocalSpace(), m_entity->GetHierarchy(), m_entity->GetHierarchy()->GetRootTransform()->GetFirstChild(), 0);

		CMatrix obPelvisNew	= m_entity->GetHierarchy()->GetRootTransform()->GetFirstChild()->GetLocalMatrix();
		m_entity->GetHierarchy()->GetRootTransform()->GetFirstChild()->SetLocalMatrixFromWorldMatrix( obPelvisNew );
	}

	void AdvancedRagdoll::AnimationPelvisMappedAsRoot( hkArray<hkQsTransform>& ragdollWorldPoseIn, hkArray<hkQsTransform>& animatedWorldPoseOut )
	{
		GATSO_PHYSICS_START("Ragdoll::AnimationPelvisMappedAsRoot");
		GATSO_PHYSICS_START("Ragdoll::DeadMapPose");
		Physics::ReadAccess mutex;

		CMatrix obRoot		= m_entity->GetHierarchy()->GetRootTransform()->GetLocalMatrix();
		CMatrix obPelvis	= m_entity->GetHierarchy()->GetRootTransform()->GetFirstChild()->GetLocalMatrix();

		// Map the pose
		m_RagdollToAnimatedMapper->mapPose(ragdollWorldPoseIn.begin(), animatedWorldPoseOut.begin(), animatedWorldPoseOut.begin(), hkSkeletonMapper::NO_CONSTRAINTS );
		
		GATSO_PHYSICS_STOP("Ragdoll::DeadMapPose");
		GATSO_PHYSICS_START("Ragdoll::DeadSetLocalPose");		
		hkPose animPose(m_AnimatedSkeleton);
		animPose.setPoseModelSpace(animatedWorldPoseOut); 

		// Update the hierarchy
		HierarchyTools::SetLocalPoseRecursive(animPose.getPoseLocalSpace(), m_entity->GetHierarchy(), m_entity->GetHierarchy()->GetRootTransform()->GetFirstChild(), 0);

		m_entity->GetHierarchy()->GetRootTransform()->SetLocalMatrix( m_entity->GetHierarchy()->GetRootTransform()->GetFirstChild()->GetLocalMatrix() );

		CMatrix obPelvisNew = m_entity->GetHierarchy()->GetRootTransform()->GetFirstChild()->GetLocalMatrix();
		m_entity->GetHierarchy()->GetRootTransform()->GetFirstChild()->SetLocalMatrixFromWorldMatrix( obPelvisNew );
		GATSO_PHYSICS_STOP("Ragdoll::DeadSetLocalPose");
		GATSO_PHYSICS_STOP("Ragdoll::AnimationPelvisMappedAsRoot");
	}

	void AdvancedRagdoll::TransformTrackingAnimation( hkArray<hkQsTransform>& ragdollWorldPoseIn, hkArray<hkQsTransform>& animatedWorldPoseOut )
	{
		Physics::ReadAccess mutex;

		CMatrix obRoot		= m_entity->GetHierarchy()->GetRootTransform()->GetLocalMatrix();
		CMatrix obPelvis	= m_entity->GetHierarchy()->GetRootTransform()->GetFirstChild()->GetLocalMatrix();

		// Map the pose
		m_RagdollToAnimatedMapper->mapPose(ragdollWorldPoseIn.begin(), animatedWorldPoseOut.begin(), animatedWorldPoseOut.begin(), hkSkeletonMapper::NO_CONSTRAINTS );
		hkPose animPose(m_AnimatedSkeleton);
		animPose.setPoseModelSpace(animatedWorldPoseOut); 

		// Update the hierarchy
		HierarchyTools::SetLocalPoseRecursive(animPose.getPoseLocalSpace(), m_entity->GetHierarchy(), m_entity->GetHierarchy()->GetRootTransform()->GetFirstChild(), 0);
		m_entity->GetHierarchy()->GetRootTransform()->SetLocalMatrix( obRoot );
		m_entity->GetHierarchy()->GetRootTransform()->GetFirstChild()->SetLocalMatrix( obPelvis );
	}

	const float increaseFactor = 10.4f;
	const float invIncreaseFactor = 1.0f / increaseFactor;

	hkRigidBody* AdvancedRagdoll::GetRagdollBone( int index )
	{
		Physics::WriteAccess mutex;

		if( index < 0 )
			return 0;

		if( index < m_RagdollInstance->getNumBones() )
		{
			return m_RagdollInstance->getRigidBodyOfBone( index );
		}

		return 0;
	}
	
	void AdvancedRagdoll::Debug_RenderCollisionInfo ()
	{
#ifndef _RELEASE
		Physics::ReadAccess mutex;

		// show rigid bodies
		for ( int i =0; i < m_RagdollInstance->getNumBones(); i++)
		{
			hkRigidBody* rb = m_RagdollInstance->getRigidBodyOfBone(i);
			//DebugCollisionTools::RenderCollisionFlags(rb);

			//g_VisualDebug->Printf3D(MathsTools::hkVectorToCPoint(rb->getPosition()),0.0f,-20.0f,0xff00ffff,DTF_ALIGN_HCENTRE,
			//	"%lf, %lf, %lf, %lf",(float) rb->getAngularDamping(),(float) rb->getLinearDamping(), (float) rb->getAngularVelocity().length3(), (float) rb->getLinearVelocity().length3());

			hkTransform trans;
			rb->approxTransformAt(CPhysicsWorld::Get().GetHavokWorldP()->getCurrentTime(),trans);
		
			CMatrix obWorldMatrix( MathsTools::hkTransformToCMatrix(trans));				
		
			DebugCollisionTools::RenderShape(obWorldMatrix,rb->getCollidable()->getShape());
		}

		// show constraints
		const hkArray<hkConstraintInstance *>& constraints = m_RagdollInstance->getConstraintArray();
		for ( int i =0; i < constraints.getSize(); i++)
		{
			DebugCollisionTools::RenderConstraint(*(constraints[i]));
		}



		/*float fY=-150.0f;
		char acMotionType [32];

		DebugCollisionTools::GetMotionType(m_RagdollInstance->getRigidBodyOfBone(HEAD_BONE),acMotionType); g_VisualDebug->Printf3D(m_entity->GetLocation(),0.0f,fY,0xff00ffff,DTF_ALIGN_HCENTRE,"Head:%s",acMotionType); fY+=10.0f;
		DebugCollisionTools::GetMotionType(m_RagdollInstance->getRigidBodyOfBone(SPINE_BONE),acMotionType); g_VisualDebug->Printf3D(m_entity->GetLocation(),0.0f,fY,0xff00ffff,DTF_ALIGN_HCENTRE,"Spine:%s",acMotionType); fY+=10.0f;
		DebugCollisionTools::GetMotionType(m_RagdollInstance->getRigidBodyOfBone(PELVIS_BONE),acMotionType); g_VisualDebug->Printf3D(m_entity->GetLocation(),0.0f,fY,0xff00ffff,DTF_ALIGN_HCENTRE,"Pelvis:%s",acMotionType); fY+=10.0f;
		DebugCollisionTools::GetMotionType(m_RagdollInstance->getRigidBodyOfBone(L_FORARM_BONE),acMotionType); g_VisualDebug->Printf3D(m_entity->GetLocation(),0.0f,fY,0xff00ffff,DTF_ALIGN_HCENTRE,"L Forearm:%s",acMotionType); fY+=10.0f;
		DebugCollisionTools::GetMotionType(m_RagdollInstance->getRigidBodyOfBone(R_FORARM_BONE),acMotionType); g_VisualDebug->Printf3D(m_entity->GetLocation(),0.0f,fY,0xff00ffff,DTF_ALIGN_HCENTRE,"R Forearm:%s",acMotionType); fY+=10.0f;
		DebugCollisionTools::GetMotionType(m_RagdollInstance->getRigidBodyOfBone(L_ELBOW_BONE),acMotionType); g_VisualDebug->Printf3D(m_entity->GetLocation(),0.0f,fY,0xff00ffff,DTF_ALIGN_HCENTRE,"L Elbow:%s",acMotionType); fY+=10.0f;
		DebugCollisionTools::GetMotionType(m_RagdollInstance->getRigidBodyOfBone(R_ELBOW_BONE),acMotionType); g_VisualDebug->Printf3D(m_entity->GetLocation(),0.0f,fY,0xff00ffff,DTF_ALIGN_HCENTRE,"R Elbow:%s",acMotionType); fY+=10.0f;
		DebugCollisionTools::GetMotionType(m_RagdollInstance->getRigidBodyOfBone(L_LEG_BONE),acMotionType); g_VisualDebug->Printf3D(m_entity->GetLocation(),0.0f,fY,0xff00ffff,DTF_ALIGN_HCENTRE,"L Leg:%s",acMotionType); fY+=10.0f;
		DebugCollisionTools::GetMotionType(m_RagdollInstance->getRigidBodyOfBone(R_LEG_BONE),acMotionType); g_VisualDebug->Printf3D(m_entity->GetLocation(),0.0f,fY,0xff00ffff,DTF_ALIGN_HCENTRE,"R Leg:%s",acMotionType); fY+=10.0f;
		DebugCollisionTools::GetMotionType(m_RagdollInstance->getRigidBodyOfBone(L_KNEE_BONE),acMotionType); g_VisualDebug->Printf3D(m_entity->GetLocation(),0.0f,fY,0xff00ffff,DTF_ALIGN_HCENTRE,"L Knee:%s",acMotionType); fY+=10.0f;
		DebugCollisionTools::GetMotionType(m_RagdollInstance->getRigidBodyOfBone(R_KNEE_BONE),acMotionType); g_VisualDebug->Printf3D(m_entity->GetLocation(),0.0f,fY,0xff00ffff,DTF_ALIGN_HCENTRE,"R Knee:%s",acMotionType); fY+=10.0f;*/

#endif // _RELEASE
	}

	void AdvancedRagdoll::Debug_RenderCollisionInfo_Animated ()
	{
#ifndef _RELEASE
		if (m_currentState != DEACTIVATED || !m_RagdollInstance || m_RagdollInstance->getWorld()) 
		{
			// ragdoll is allready in world or does not have an instance
			//ntAssert_p(false, ("CastRayOnAnimated cannot be performed on ragdoll that's are already in world"));
			return;
		}

		// 1.) animate ragdoll according to current position 				
		CHierarchy* hierarchy	= m_entity->GetHierarchy();

		// Get the pose that we'll use to drive the ragdoll
		const int ragdollNumBones = m_RagdollInstance->getNumBones();
		hkArray<hkQsTransform> ragdollPose( ragdollNumBones );
		ragdollPose.setSizeUnchecked( ragdollNumBones );
		
		const int numBones = m_AnimatedSkeleton->m_numBones;
		hkArray<hkQsTransform> animatedPose( numBones );
		animatedPose.setSizeUnchecked( numBones );
		
		// Sample the active animations and combine into a single pose
		HierarchyTools::GetWorldPoseRecursive( animatedPose, hierarchy, hierarchy->GetRootTransform()->GetFirstChild() );
			
		m_AnimatedToRagdollMapper->mapPose( animatedPose.begin(), 
			m_RagdollInstance->getSkeleton()->m_referencePose, 
			ragdollPose.begin() , hkSkeletonMapper::NO_CONSTRAINTS );

		m_RagdollInstance->setPoseWorldSpace(ragdollPose.begin()); //, hkQsTransform(hkQsTransform::IDENTITY)); 				

		// 2.) render it 
		for ( int i =0; i < m_RagdollInstance->getNumBones(); i++)
		{
			hkRigidBody* rb = m_RagdollInstance->getRigidBodyOfBone(i);			
			hkTransform trans;
			rb->approxTransformAt(CPhysicsWorld::Get().GetHavokWorldP()->getCurrentTime(),trans);
		
			CMatrix obWorldMatrix( MathsTools::hkTransformToCMatrix(trans));				
			DebugCollisionTools::RenderShape(obWorldMatrix,rb->getCollidable()->getShape());
		}

		return; 
#endif // _RELEASE
	}

	CPoint AdvancedRagdoll::GetPosition()
	{
		Physics::ReadAccess mutex;

		hkTransform obPelvisTransform = m_RagdollInstance->getRigidBodyOfBone(PELVIS_BONE)->getTransform();
		m_RagdollInstance->getRigidBodyOfBone(PELVIS_BONE)->approxTransformAt(Physics::CPhysicsWorld::Get().GetFrameTime(),obPelvisTransform);
		return Physics::MathsTools::hkVectorToCPoint(obPelvisTransform.getTranslation());
	}

	bool AdvancedRagdoll::IsInContactWithGround()
	{
		Physics::WriteAccess mutex;

		// If we're gonna get ragdolled when we hit something then return false because we don't want to pre-empt that
		if (m_bTurnDynamicOnContact)
		{
			return false;
		}
		else // Loop through each bone checking if it's in contact with some geometry
		{
			bool bRet = false;
			for (int i = 0; i < m_RagdollInstance->getNumBones(); i++)
			{
				bRet |= CheckBoneInContactWithWorld(m_RagdollInstance->getRigidBodyOfBone(i));
				if (bRet)
					break;
			}
			return bRet;
		}
	}

	void AdvancedRagdoll::SetupBoneTransformTrackingArray()
	{
		ntError(m_RagdollInstance);

		m_aobBoneTransformTracking = NT_NEW BoneTransformTrackingMapping[m_RagdollInstance->getNumBones()];
		for (int i = 0; i < m_RagdollInstance->getNumBones(); i++)
		{
			m_aobBoneTransformTracking[i].m_bIsActive = false;
		}
	}

	void AdvancedRagdoll::ResetBoneTransformTrackingArray()
	{
		ntError(m_RagdollInstance);

		for (int i = 0; i < m_RagdollInstance->getNumBones(); i++)
		{
			m_aobBoneTransformTracking[i].m_bIsActive = false;
		}
	}

	void AdvancedRagdoll::SetBoneTransformTrackingMapping(int iBoneFlag, const Transform* pobTransform)
	{
		int iIdx = GetBoneIndexFromFlag(iBoneFlag);

		// Mapping is now in effect
		m_aobBoneTransformTracking[iIdx].m_bIsActive = true;
		m_aobBoneTransformTracking[iIdx].m_pobTransformToTrack = pobTransform;

		// Loop through each mapping and, if it's active, set an initial position
		/*for (int i = 0; i < m_RagdollInstance->getNumBones(); i++)
		{
			if (m_aobBoneTransformTracking[i].m_bIsActive)
			{
				// Get the bone and the transform it's supposed to track
				hkRigidBody* pobBone = m_RagdollInstance->getRigidBodyOfBone(i);
				const Transform* pobTransformToTrack = m_aobBoneTransformTracking[i].m_pobTransformToTrack;

				// Get where it is now
				hkTransform obBoneTransform;
				pobBone->approxTransformAt(Physics::CPhysicsWorld::Get().GetFrameTime(),obBoneTransform);
				hkQuaternion obPelvisRotation(obBoneTransform.getRotation());

				// Get a movement vector for translation
				hkVector4 obBoneTranslationNeeded = Physics::MathsTools::CPointTohkVector(pobTransformToTrack->GetWorldMatrix().GetTranslation());
				hkVector4 obMovement = obBoneTranslationNeeded;
				obMovement.sub4(obBoneTransform.getTranslation());
				pobBone->setPosition(obBoneTranslationNeeded);

				// Loop through all the bones and apply the movement vector to each
				for (int j = 0; j < m_RagdollInstance->getNumBones(); j++)
				{
					hkRigidBody* pobSubBone = m_RagdollInstance->getRigidBodyOfBone(j);
					hkTransform obSubBoneTransform;
					pobSubBone->approxTransformAt(Physics::CPhysicsWorld::Get().GetFrameTime(),obSubBoneTransform);
					hkVector4 obSubBoneTranslation = obSubBoneTransform.getTranslation();
					obSubBoneTranslation.add4(obMovement);
					pobSubBone->setPosition(obSubBoneTranslation);
				}	
			}	
		}*/
	}

	int AdvancedRagdoll::GetBoneIndexFromFlag(int iFlag)
	{
		if (iFlag & PELVIS)
			return PELVIS_BONE;
		if (iFlag & SPINE)
			return SPINE_BONE;
		if (iFlag & UPPER_ARM_LEFT)
			return L_FORARM_BONE;
		if (iFlag & UPPER_ARM_RIGHT)
			return R_FORARM_BONE;
		if (iFlag & LOWER_ARM_LEFT)
			return L_ELBOW_BONE;
		if (iFlag & LOWER_ARM_RIGHT)
			return R_ELBOW_BONE;
		if (iFlag & HEAD)
			return HEAD_BONE;
		if (iFlag & UPPER_LEG_LEFT)
			return L_LEG_BONE;
		if (iFlag & UPPER_LEG_RIGHT)
			return R_LEG_BONE;
		if (iFlag & LOWER_LEG_LEFT)
			return L_KNEE_BONE;
		if (iFlag & LOWER_LEG_RIGHT)
			return R_KNEE_BONE;

		return -1;
	}

	void AdvancedRagdoll::SetExemptFromCleanup(bool bExemption)
	{
		if ( g_ShellOptions->m_bInteractableRagdolls )
		{
			m_bExemptFromDisposal = bExemption;
		}
	}

	void AdvancedRagdoll::SetAntiGravity(bool bAntiGrav)
	{
		if( bAntiGrav )
		{
			if (!m_pobAntiGravityAction->getWorld() )
			{
				CPhysicsWorld::Get().AddAction( m_pobAntiGravityAction );
			}
			if ( m_pobUberGravityAction->getWorld() )
			{
				//CPhysicsWorld::Get().RemoveAction( m_pobUberGravityAction );
			}
		}
		else
		{
			if ( m_pobAntiGravityAction->getWorld() )
			{
				CPhysicsWorld::Get().RemoveAction( m_pobAntiGravityAction );
			}
			if ( !m_pobUberGravityAction->getWorld() )
			{
				//CPhysicsWorld::Get().AddAction( m_pobUberGravityAction );
			}
		}
	}

	void AdvancedRagdoll::SetUberGravity(bool bUberGrav)
	{
		if( bUberGrav )
		{	
			if ( !m_pobUberGravityAction->getWorld() )
			{
				CPhysicsWorld::Get().AddAction( m_pobUberGravityAction );
			}			
		}
		else
		{
			if ( m_pobUberGravityAction->getWorld() )
			{
				CPhysicsWorld::Get().RemoveAction( m_pobUberGravityAction );
			}
		}
	}

	bool AdvancedRagdoll::GetUberGravity()
	{
		return m_pobUberGravityAction->getWorld();
	}

	bool AdvancedRagdoll::GetAntiGravity()
	{
		return m_pobAntiGravityAction->getWorld();
	}

	// Can be ragdoll simulated as phantom... simplified simulation... 
	bool AdvancedRagdoll::CanBePhantom() const
	{
		ntAssert(m_RagdollInstance->getNumBones() > 0);

		if ( !IsPhantom() 					// Not already phantom
			 && !IsMoving()					// Not active.
			 && !GetExemptFromCleanup() 	// Not exempt from phantomisation (e.g. when being held).
			 && m_entity->IsDead()  		// Entity must be dead and not about to recover!
			 && m_fActiveTime >= 3.0f		// Active long enough to be worthwhile.
			 && m_fNonPhantomTime > 1.0f )	// Not oscillating between phantom and dynamic.
		{
			return true;
		}
		
		return false;
	}

	// can be ragdoll removed from scene. Probably because we are lack of reasources
	bool AdvancedRagdoll::CanBeRemoved() 
	{
		ntError_p( IsPhantom(), ("Error! Should only be considering phantomised ragdolls for removal.") );
		ntError_p( !GetExemptFromCleanup(), ("Error! Should only be considering non-exempt ragdolls for removal.") );
		
		if ( IsPhantom() && !GetExemptFromCleanup() )
		{
			// Get where the ragdoll is on screen, and use this to decide if we're gonna cull it from the world or not
			CPoint obRagdollPosition = GetPosition();
			
			// Use a bounding box and test each of its verts
			CPoint obCullBoxHalfExtents(1.0f,1.0f,1.0f);
			CPoint obScreenPoints[8];
			obScreenPoints[0] = obRagdollPosition;
			obScreenPoints[0].Y() -= obCullBoxHalfExtents.Y();
			obScreenPoints[0].X() -= obCullBoxHalfExtents.X();
			obScreenPoints[1] = obRagdollPosition;
			obScreenPoints[1].Y() -= obCullBoxHalfExtents.Y();
			obScreenPoints[1].X() += obCullBoxHalfExtents.X();
			obScreenPoints[2] = obRagdollPosition;
			obScreenPoints[2].Y() -= obCullBoxHalfExtents.Y();
			obScreenPoints[2].Z() -= obCullBoxHalfExtents.Z();
			obScreenPoints[3] = obRagdollPosition;
			obScreenPoints[3].Y() -= obCullBoxHalfExtents.Y();
			obScreenPoints[3].Z() += obCullBoxHalfExtents.Z();
			obScreenPoints[4] = obRagdollPosition;
			obScreenPoints[4].Y() += obCullBoxHalfExtents.Y();
			obScreenPoints[4].X() -= obCullBoxHalfExtents.X();
			obScreenPoints[5] = obRagdollPosition;
			obScreenPoints[5].Y() += obCullBoxHalfExtents.Y();
			obScreenPoints[5].X() += obCullBoxHalfExtents.X();
			obScreenPoints[6] = obRagdollPosition;
			obScreenPoints[6].Y() += obCullBoxHalfExtents.Y();
			obScreenPoints[6].Z() -= obCullBoxHalfExtents.Z();
			obScreenPoints[7] = obRagdollPosition;
			obScreenPoints[7].Y() += obCullBoxHalfExtents.Y();
			obScreenPoints[7].Z() += obCullBoxHalfExtents.Z();
			
			// Assume we can cull...			
			const CCamera* pCurrCamera = CamMan::GetPrimaryView();
			for (int j = 0; j < 8; j++)
			{
				if ( pCurrCamera->CanSeePoint(obScreenPoints[j]) )
				   return false;
			}
			
			return true;
		}
		else
		{
		   return false;
		}
	}

	
	// JML - Added for respawning, please tidy as necessary
	void AdvancedRagdoll::UnfixBones()
	{
		for	(int bone=0; bone<m_RagdollInstance->getNumBones();	bone++)
		{
			hkRigidBody* rb	= m_RagdollInstance->getRigidBodyOfBone(bone);
			rb->setMotionType(hkMotion::MOTION_DYNAMIC, HK_ENTITY_ACTIVATION_DO_NOT_ACTIVATE);
		}
	}

	// ragdoll is simulated as phantom. Remove phantom and start standard dynamic ragdoll
	void AdvancedRagdoll::PhantomToDynamic()
	{
		if ( !m_bIsPhantom ) return;
		ntAssert(m_phantom && m_phantom->getWorld());
		
		Physics::WriteAccess mutex;
		RagdollPerformanceManager::Get().PhantomToDynamicRagdoll(this);
		
		m_bIsPhantom 	  = false;
		m_fNonPhantomTime = 0.0f; 

		for	(int bone=0; bone<m_RagdollInstance->getNumBones();	bone++)
		{
			hkRigidBody* rb	= m_RagdollInstance->getRigidBodyOfBone(bone);
			rb->setMotionType(hkMotion::MOTION_DYNAMIC, HK_ENTITY_ACTIVATION_DO_NOT_ACTIVATE);
		}

		m_phantom->getWorld()->removePhantom(m_phantom);
	}

	// ragdoll is simulated as dynamic. Fix the bodies and add phantom into world.
	void AdvancedRagdoll::DynamicToPhantom()
	{
		Physics::WriteAccess mutex;

		ntAssert(!m_bIsPhantom);
		RagdollPerformanceManager::Get().DynamicToPhantomRagdoll(this);
		

		hkVector4 min(0,0,0,0);
	    hkVector4 max(0,0,0,0);

		for	(int bone=0; bone<m_RagdollInstance->getNumBones();	bone++)
		{
			hkRigidBody* rb	= m_RagdollInstance->getRigidBodyOfBone(bone);
			rb->setMotionType(hkMotion::MOTION_FIXED);

			hkAabb world_aabb;
			hkTransform obTransform;
			rb->approxTransformAt(Physics::CPhysicsWorld::Get().GetFrameTime(),obTransform);
			rb->getCollidable()->getShape()->getAabb(obTransform, 0.0f, world_aabb);

			if (bone==0)
			{
				min = world_aabb.m_min;
				max = world_aabb.m_max;
			}
			else
			{
				min.setMin4(min, world_aabb.m_min);
				max.setMax4(max, world_aabb.m_max);
			}
		}

		hkAabb aabb(min,max);		
				
		if (!m_phantom)
		{
			m_phantom = HK_NEW ActivatePhantom(aabb, this);
			Physics::FilterExceptionFlag exceptionFlag; exceptionFlag.base = 0;
			exceptionFlag.flags.exception_set |= Physics::ALWAYS_RETURN_TRUE_BIT;
			hkPropertyValue val2((int)exceptionFlag.base);
			m_phantom->addProperty(Physics::PROPERTY_FILTER_EXCEPTION_INT, val2);
		}
		else
		{
			m_phantom->setAabb(aabb);
		}

		m_bIsPhantom = true; 	
		m_fNonPhantomTime = 0.0f;	
		CPhysicsWorld::Get().GetHavokWorldP()->addPhantom(m_phantom);  
	}

	bool AdvancedRagdoll::CastRayOnAnimated(const CPoint &obSrc, const CPoint &obTarg, Physics::RaycastCollisionFlag obFlag, TRACE_LINE_QUERY& stQuery)
	{
		if (m_currentState != DEACTIVATED || !m_RagdollInstance || m_RagdollInstance->getWorld()) 
		{
			// ragdoll is allready in world or does not have an instance
			ntAssert_p(false, ("CastRayOnAnimated cannot be performed on ragdoll that's are already in world"));
			return false;
		}

		// 1.) animate ragdoll according to current position 				
		CHierarchy* hierarchy	= m_entity->GetHierarchy();

		// Get the pose that we'll use to drive the ragdoll
		const int ragdollNumBones = m_RagdollInstance->getNumBones();
		hkArray<hkQsTransform> ragdollPose( ragdollNumBones );
		ragdollPose.setSizeUnchecked( ragdollNumBones );
		
		const int numBones = m_AnimatedSkeleton->m_numBones;
		hkArray<hkQsTransform> animatedPose( numBones );
		animatedPose.setSizeUnchecked( numBones );
		
		// Sample the active animations and combine into a single pose
		HierarchyTools::GetWorldPoseRecursive( animatedPose, hierarchy, hierarchy->GetRootTransform()->GetFirstChild() );
			
		m_AnimatedToRagdollMapper->mapPose( animatedPose.begin(), 
			m_RagdollInstance->getSkeleton()->m_referencePose, 
			ragdollPose.begin() , hkSkeletonMapper::NO_CONSTRAINTS );

		m_RagdollInstance->setPoseWorldSpace(ragdollPose.begin());   	

		// make it static... this will reduce cost of adding to world
		for (int b=0; b<ragdollNumBones; b++)
		{			
			hkRigidBody* rbody = m_RagdollInstance->getRigidBodyOfBone(b);
			rbody->setMotionType(hkMotion::MOTION_FIXED); 		
		}

		// 2.) add it to world
		hkWorld * workWorld = CPhysicsWorld::Get().GetAuxiliaryHavokWorldP();
		workWorld->markForWrite();
		workWorld->addEntityBatch( reinterpret_cast<hkEntity*const*> ( m_RagdollInstance->m_rigidBodies.begin() ), m_RagdollInstance->m_rigidBodies.getSize() ); 		

		// 3.) do detection collision		
		hkWorldRayCastInput obInput;
		obInput.m_from.set(obSrc.X(), obSrc.Y(), obSrc.Z());
		obInput.m_to.set(obTarg.X(), obTarg.Y(), obTarg.Z());
		obInput.m_filterInfo = obFlag.base;

		hkClosestRayHitCollector obOutput;
		workWorld->castRay(obInput, obOutput);

		bool ret = false; 
		if(obOutput.hasHit())
		{
			// Get the useful data from the results - the surface normal
			const hkVector4& obSurfaceNormal = obOutput.getHit().m_normal;
			float fHitFraction = obOutput.getHit().m_hitFraction;

			// Fill in information for the query
			stQuery.fFraction=fHitFraction;
			stQuery.obIntersect=CPoint::Lerp(obSrc,obTarg,fHitFraction);
			stQuery.obNormal=CDirection(obSurfaceNormal.getSimdAt( 0 ), obSurfaceNormal.getSimdAt( 1 ), obSurfaceNormal.getSimdAt( 2 ) );

			stQuery.obCollidedFlag.base = obOutput.getHit().m_rootCollidable->getCollisionFilterInfo();
			stQuery.pobEntity = Tools::GetEntity(*obOutput.getHit().m_rootCollidable);
			
			ret = true; 
		}

		//4.) clean up
		workWorld->removeEntityBatch( reinterpret_cast<hkEntity*const*> ( m_RagdollInstance->m_rigidBodies.begin() ), m_RagdollInstance->m_rigidBodies.getSize() ); 		

		// make bodies dynamic again
		for (int b=0; b<ragdollNumBones; b++)
		{			
			hkRigidBody* rbody = m_RagdollInstance->getRigidBodyOfBone(b);
			rbody->setMotionType(hkMotion::MOTION_DYNAMIC);
		}		
		workWorld->unmarkForWrite();

		return ret; 
	}

} // namespace Physics 

/*
m_RagdollInstance->getRigidBodyOfBone(PELVIS_BONE)->setPosition( Physics::MathsTools::CPointTohkVector( Physics::MathsTools::hkVectorToCPoint(m_RagdollInstance->getRigidBodyOfBone(PELVIS_BONE)->getPosition()) + obMovement) );
m_RagdollInstance->getRigidBodyOfBone(L_LEG_BONE)->setPosition( Physics::MathsTools::CPointTohkVector( Physics::MathsTools::hkVectorToCPoint(m_RagdollInstance->getRigidBodyOfBone(L_LEG_BONE)->getPosition()) + obMovement) );
m_RagdollInstance->getRigidBodyOfBone(R_LEG_BONE)->setPosition( Physics::MathsTools::CPointTohkVector( Physics::MathsTools::hkVectorToCPoint(m_RagdollInstance->getRigidBodyOfBone(R_LEG_BONE)->getPosition()) + obMovement) );
m_RagdollInstance->getRigidBodyOfBone(SPINE_BONE)->setPosition( Physics::MathsTools::CPointTohkVector( Physics::MathsTools::hkVectorToCPoint(m_RagdollInstance->getRigidBodyOfBone(SPINE_BONE)->getPosition()) + obMovement) );
m_RagdollInstance->getRigidBodyOfBone(L_KNEE_BONE)->setPosition( Physics::MathsTools::CPointTohkVector( Physics::MathsTools::hkVectorToCPoint(m_RagdollInstance->getRigidBodyOfBone(L_KNEE_BONE)->getPosition()) + obMovement) );
m_RagdollInstance->getRigidBodyOfBone(R_KNEE_BONE)->setPosition( Physics::MathsTools::CPointTohkVector( Physics::MathsTools::hkVectorToCPoint(m_RagdollInstance->getRigidBodyOfBone(R_KNEE_BONE)->getPosition()) + obMovement) );
m_RagdollInstance->getRigidBodyOfBone(HEAD_BONE)->setPosition( Physics::MathsTools::CPointTohkVector( Physics::MathsTools::hkVectorToCPoint(m_RagdollInstance->getRigidBodyOfBone(HEAD_BONE)->getPosition()) + obMovement) );
m_RagdollInstance->getRigidBodyOfBone(R_FORARM_BONE)->setPosition( Physics::MathsTools::CPointTohkVector( Physics::MathsTools::hkVectorToCPoint(m_RagdollInstance->getRigidBodyOfBone(R_FORARM_BONE)->getPosition()) + obMovement) );
m_RagdollInstance->getRigidBodyOfBone(L_FORARM_BONE)->setPosition( Physics::MathsTools::CPointTohkVector( Physics::MathsTools::hkVectorToCPoint(m_RagdollInstance->getRigidBodyOfBone(L_FORARM_BONE)->getPosition()) + obMovement) );
m_RagdollInstance->getRigidBodyOfBone(R_ELBOW_BONE)->setPosition( Physics::MathsTools::CPointTohkVector( Physics::MathsTools::hkVectorToCPoint(m_RagdollInstance->getRigidBodyOfBone(R_ELBOW_BONE)->getPosition()) + obMovement) );
m_RagdollInstance->getRigidBodyOfBone(L_ELBOW_BONE)->setPosition( Physics::MathsTools::CPointTohkVector( Physics::MathsTools::hkVectorToCPoint(m_RagdollInstance->getRigidBodyOfBone(L_ELBOW_BONE)->getPosition()) + obMovement) );
*/
