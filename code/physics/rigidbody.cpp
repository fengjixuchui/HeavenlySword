//---------------------------------------------------------------------------------------------------------
//!
//!	\file physics/rigidbody.cpp
//!	
//!	DYNAMICS COMPONENT:	
//!
//!	Author: Mustapha Bismi (mustapha.bismi@ninjatheory.com)
//! Created: 2005.07.11
//!
//---------------------------------------------------------------------------------------------------------

#include "config.h"
#include <hkbase\config\hkConfigVersion.h>

#include "physics/collisionlistener.h"
#include "physics/havokincludes.h"
#include "rigidbody.h"
#include "anim/transform.h"
#include "physics/world.h"
#include "physics/havokthreadutils.h"
#include "physics/physicsTools.h"


#include "behavior.h"
#include "game/entity.h"
#include "game/entity.inl"
#include "system.h"
#include "maths_tools.h"

#include "core/gatso.h"

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
#	include <hkdynamics/entity/hkRigidBody.h>
#	include <hkutilities\keyframe\hkKeyFrameUtility.h>
#	include <hkvisualize/hkDebugDisplay.h>
#	include <hkdynamics/entity/hkSpatialRigidBodyDeactivator.h>
#	include <hkdynamics/constraint/constraintkit/hkGenericConstraintData.h>
#	include <hkdynamics/constraint/constraintkit/hkConstraintConstructionKit.h>
#	include <hkcollide/collector/bodypaircollector/hkFlagCdBodyPairCollector.h>
#	include <hkdynamics\constraint\breakable\hkBreakableConstraintData.h>
#	include <hkinternal/dynamics/constraint/bilateral/ragdoll/hkRagdollConstraintData.h>
#	include <hkcollide/agent/hkCdPointCollector.h>
#	include <hkcollide/agent/hkCdPoint.h>
#	include <hkcollide/castutil/hkLinearCastInput.h>
#endif



namespace Physics {

	//---------------------------------------------------------------
	//!
	//!	BodyCInfo Constructor.
	//!
	//---------------------------------------------------------------

	BodyCInfo::BodyCInfo( ):
		m_transform( 0 )
	{
		m_exceptionFlag.base = 0;
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		m_rigidBodyCinfo.m_allowedPenetrationDepth = 0.01f; 
#endif
		m_allowedPenetrationDepth = 0.01f;
	}

	//---------------------------------------------------------------
	//!
	//!	BodyCInfo Destructor.
	//!
	//---------------------------------------------------------------

	BodyCInfo::~BodyCInfo( )
	{
		m_transform = 0;
	}

	static const float fANGULAR_VELOCITY_THRESHOLD = 50.0f;

	//---------------------------------------------------------------
	//!
	//!	SetLastTransform.
	//!
	//---------------------------------------------------------------

	void KeyframedMotionParams::SetLastTransform(const CMatrix& trans)
	{
		m_lastTranslation = MathsTools::CPointTohkVector(trans.GetTranslation());
		m_lastRotation = MathsTools::CMatrixTohkQuaternion(trans);
	}

	CMatrix KeyframedMotionParams::GetLastTransform() const
	{
		return CMatrix(MathsTools::hkQuaternionToCQuat(m_lastRotation), MathsTools::hkVectorToCPoint(m_lastTranslation));
	}

	//---------------------------------------------------------------
	//!
	//!	Construct a body with the BodyCinfo.
	//!		\param  const BodyCInfo* - Ptr to the BdyInfo structure.
	//!
	//---------------------------------------------------------------

	void RigidBody::Construct( const BodyCInfo* p_info )
	{		
		m_transform	= p_info->m_transform;

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		Physics::WriteAccess wmutex;

		hkRigidBodyCinfo obNewInfo = p_info->m_rigidBodyCinfo;
		// According to havok warning 0xf03243df. The item with thin box like innertia must have limited
		// angulat motion to get stable simulation so do it.
		if (obNewInfo.m_maxAngularVelocity > 9.9f && p_info->m_rigidBodyCinfo.m_motionType == hkMotion::MOTION_DYNAMIC)
		{
			hkReal a = p_info->m_rigidBodyCinfo.m_inertiaTensor(0,0);
			hkReal b = p_info->m_rigidBodyCinfo.m_inertiaTensor(1,1);
			hkReal c = p_info->m_rigidBodyCinfo.m_inertiaTensor(2,2);

			hkReal ma = max( a, max( b, c ));
			hkReal mi = min( a, min( b, c ));
			if ( mi <= ma * 0.1f)
			{				
				// hmmm p_info is const. For this small change it is better to override const than to change all calls to nonconst values or shift this code elsewhere.
				obNewInfo.m_maxAngularVelocity = 9.9f; // in havok is test if ( maxAngularVelocity > 10.0f )
			}			
		}

		// switch deactivator
		//obNewInfo.m_rigidBodyDeactivatorType =  hkRigidBodyDeactivator::DEACTIVATOR_NEVER;

		m_rigidBody	= HK_NEW hkRigidBody( obNewInfo );
		//obNewInfo.m_shape->removeReference();

		if (p_info->m_rigidBodyCinfo.m_motionType != hkMotion::MOTION_FIXED)
		{
			hkSpatialRigidBodyDeactivator* deactivator = static_cast<hkSpatialRigidBodyDeactivator*>(m_rigidBody->getDeactivator());
			if (deactivator)
			{
				deactivator->m_minLowFrequencyTranslation  *= 0.5f;
				deactivator->m_minLowFrequencyRotation     *= 0.5f;
				//deactivator->m_minHighFrequencyTranslation *= 1.5f;
				//deactivator->m_minHighFrequencyRotation    *= 1.5f;
			}
		}
		
		m_rigidBody->setName( p_info->m_name );
		m_rigidBody->getCollidableRw()->setAllowedPenetrationDepth( p_info->m_allowedPenetrationDepth );
	
		hkPropertyValue val( (int)p_info->m_exceptionFlag.base );
		m_rigidBody->addProperty( Physics::PROPERTY_FILTER_EXCEPTION_INT, val );

		SetMotionTypeAfterConstruct(this);
#endif
	}

	//---------------------------------------------------------------
	//!
	//!	Synchronise the rigid body on the Transform position.
	//!		\param  float		- Current timestep.
	//!
	//---------------------------------------------------------------
	
	void RigidBody::SyncBodyOnTransform( const float p_timeStep )
	{
		//GATSO_PHYSICS_START("RigidBody::SyncBodyOnTransform");
		if (p_timeStep == 0)
			return; // nothing to do

		ntAssert(m_pobKeyframed);
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD							
		if( ( m_rigidBody ) && ( m_transform ) )
		{
			Physics::WriteAccess world_mutex;
			Physics::WriteAccess rb_mutex( m_rigidBody );
			
			CMatrix lastTransform = m_pobKeyframed->GetLastTransform();
			const CMatrix& currentTransform  = m_transform->GetWorldMatrix();

#ifdef USE_ASYCHRONOUS_SIMULATION
			hkWorld * world = Physics::CPhysicsWorld::Get().GetHavokWorldP();

			// All synchronization is done on Havok time steps. 

			// Why not on frame times? It leads to horrible instabilities if havok time steps and frame steps are different
			// If we ask hardkeyframe to reach position between the havok time steps, we have no control over position 
			// on time steps and it can diverge... 
			float predictTime = world->getCurrentPsiTime() - world->getCurrentTime();						
			float physTime = 0;
			float worldStep = Physics::CPhysicsWorld::Get().GetLastStep();
			
			if (predictTime < p_timeStep)			
			{
				// search for havok step close to next frame.
				float space  = p_timeStep - predictTime;
				int n = (int) (space / worldStep) + 1;
				predictTime += n * worldStep;
				physTime += n * worldStep;
			}
			else
			{
				predictTime += worldStep;
				physTime += worldStep;
			}

			ntAssert(physTime > 0);
			ntAssert(predictTime > 0);
            
            // BE WARE !!! Change of concept. 
			// Till now we have always predicted of the trasformation in next frame. 
			// Then we synchronized body position to predicted value. It works fine for slow and smooth 
			// movements. Unfortunally movement for throwing swords etc is very fast. Predicted position
			// suffers from great errors (breakable movement breaks to early etc.). That's why, 
			// we are NOT using predicted position during the breakable motion anymore. We use current position instead. 
			// Because of that rigid body is "one frame behind" animation, but it follows animation more exactly. 
			float interpolationCoef = predictTime / p_timeStep;
			if (m_eMotionType != HS_MOTION_BREAKABLE_KEYFRAMED)
				interpolationCoef += 1;							

			CMatrix predictedTrans(CMatrix::Lerp(lastTransform, currentTransform, interpolationCoef));		
#else
			const CMatrix& currentTransform  = m_transform->GetWorldMatrix();	
			CMatrix predictedTrans(CMatrix::Lerp(lastTransform, currentTransform, 2));
			float physTime = p_timeStep; 
#endif

			//const hkVector4 bodyPos = actBody->getPosition();
			//if (m_pobBreakableKeyframed)
			/*ntPrintf("old (%lf , %lf , %lf), current (%lf , %lf , %lf), bodyPos (%lf , %lf , %lf), predicted (%lf , %lf , %lf)\n",
				m_lastTransform.GetTranslation()[0],  m_lastTransform.GetTranslation()[1], m_lastTransform.GetTranslation()[2],
				currentTransform.GetTranslation()[0],  currentTransform.GetTranslation()[1], currentTransform.GetTranslation()[2],
				bodyPos(0), bodyPos(1), bodyPos(2),
				predictedTrans.GetTranslation()[0],  predictedTrans.GetTranslation()[1], predictedTrans.GetTranslation()[2]);*/

			// safe velocities from transform...
			float invDeltaTime = 1.0f / p_timeStep;		
			m_pobKeyframed->m_lastAnimLinearVelocity.setSub4( MathsTools::CPointTohkVector(currentTransform.GetTranslation()), m_pobKeyframed->m_lastTranslation);
			m_pobKeyframed->m_lastAnimLinearVelocity.mul4(invDeltaTime);
						
			hkQuaternion quatDif;
			hkQuaternion currentRot = MathsTools::CMatrixTohkQuaternion(currentTransform);

			quatDif.setMulInverse( currentRot, m_pobKeyframed->m_lastRotation);
			quatDif.normalize();
			hkReal angle = quatDif.getAngle();
			if(angle < 1e-6f || m_pobKeyframed->m_lastAnimAngularVelocity.length3() <= HK_REAL_EPSILON)
			{
				m_pobKeyframed->m_lastAnimAngularVelocity.setZero4();
			}
			else
			{
				quatDif.getAxis(m_pobKeyframed->m_lastAnimAngularVelocity);
				m_pobKeyframed->m_lastAnimAngularVelocity.setMul4(angle * invDeltaTime, m_pobKeyframed->m_lastAnimAngularVelocity);		
			}	
			
		    m_pobKeyframed->m_lastRotation = currentRot;
			m_pobKeyframed->m_lastTranslation = MathsTools::CPointTohkVector(currentTransform.GetTranslation());			

			const CPoint&	transformPosition = predictedTrans.GetTranslation();
			const CQuat		transformRotation = CQuat( predictedTrans );

			hkVector4		newRigidPosition( transformPosition.X(), transformPosition.Y(), transformPosition.Z());
			hkQuaternion	newRigidRotation = Physics::MathsTools::CQuatTohkQuaternion( transformRotation );	
			
			if (m_bHardKeyframing)
			{
				//move body to position where it have to be
				m_rigidBody->setTransform(MathsTools::CMatrixTohkTransform(currentTransform));
			}

			// hit it in next 
			/*ntPrintf("%s curr %lf %lf %lf pred %lf %lf %lf \n", m_entity->GetName().c_str() ,
				currentTransform.GetTranslation()[0], 
				currentTransform.GetTranslation()[1], 
				currentTransform.GetTranslation()[2], 
				newRigidPosition(0), 
				newRigidPosition(1), 
				newRigidPosition(2));*/			

			hkKeyFrameUtility::applyHardKeyFrame( newRigidPosition, newRigidRotation, 1.0f/physTime, m_rigidBody );
		
			if (!m_pobBreakableKeyframed) // for breakableKeyframed motion must be smooth.
				LimitVelocities( newRigidPosition, newRigidRotation );			
		}
#else
		UNUSED( p_timeStep );
#endif
		//GATSO_PHYSICS_STOP("RigidBody::SyncBodyOnTransform");
	}

	//---------------------------------------------------------------
	//!
	//!	Synchronise the Transform based on the rigid body position.
	//!
	//---------------------------------------------------------------

	void RigidBody::SyncTransformOnBody( )
	{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		if( ( m_rigidBody ) && ( m_transform ) )
		{
			Physics::WriteAccess mutex( m_rigidBody );

			hkTransform transformOut;
			m_rigidBody->approxTransformAt( CPhysicsWorld::Get().GetFrameTime(), transformOut );

			CPoint	transformPosition = Physics::MathsTools::hkVectorToCPoint( transformOut.getTranslation() );
			CQuat	transformRotation = Physics::MathsTools::hkQuaternionToCQuat( hkQuaternion( transformOut.getRotation() ) );

			CMatrix obNewRoot( transformRotation, transformPosition );

			if (m_pobKeyframed)
			{				
				m_pobKeyframed->SetLastTransform(obNewRoot);
			}			

			m_transform->SetLocalMatrixFromWorldMatrix( obNewRoot );
		}
#endif
	}

	//---------------------------------------------------------------
	//!
	//!	Velocity limits. We do impose velocity limits to rigid bodies
	//! when their are animated, because in slow motion, it can be
	//! set to really high velocities...
	//!		\param  hkVector4& 		- Desired position.
	//!		\param  hkQuaternion&	- Desired rotation.
	//!
	//---------------------------------------------------------------
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	void RigidBody::LimitVelocities( hkVector4& newRigidPosition, hkQuaternion& newRigidRotation )
	{
		hkVector4 currentLinVel( m_rigidBody->getLinearVelocity() );
		hkVector4 currentAngVel( m_rigidBody->getAngularVelocity() );

		if( currentAngVel.lengthSquared3() > fANGULAR_VELOCITY_THRESHOLD*fANGULAR_VELOCITY_THRESHOLD )
		{
			Physics::WriteAccess mutex;

			m_rigidBody->setPositionAndRotation( newRigidPosition, newRigidRotation );
			// body is on the result position so it do not have to move anymore. 
			m_rigidBody->setAngularVelocity(hkVector4(0,0,0,0) );
		}

		if ( currentLinVel.lengthSquared3() > fANGULAR_VELOCITY_THRESHOLD*fANGULAR_VELOCITY_THRESHOLD )
		{
			Physics::WriteAccess mutex;			

			m_rigidBody->setPositionAndRotation( newRigidPosition, newRigidRotation );
			// body is on the result position so it do not have to move anymore. 
			m_rigidBody->setLinearVelocity(hkVector4(0,0,0,0) );
		}
	}
#endif
	//---------------------------------------------------------------
	//!
	//!	Force the rigid body to the Transform position.
	//!
	//---------------------------------------------------------------

	void RigidBody::ForceBodyOnTransform( )
	{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		hkRigidBody * actBody = m_rigidBody;		

		if( ( actBody ) && ( m_transform ) )
		{
			Physics::WriteAccess mutex;

			if (m_pobKeyframed)
			{				
				m_pobKeyframed->SetLastTransform(m_transform->GetWorldMatrix());
			}

			const CPoint&	transformPosition = m_transform->GetWorldMatrix().GetTranslation();
			const CQuat		transformRotation = CQuat( m_transform->GetWorldMatrix() );

			hkVector4		newRigidPosition( transformPosition.X(), transformPosition.Y(), transformPosition.Z() );
			hkQuaternion	newRigidRotation = Physics::MathsTools::CQuatTohkQuaternion( transformRotation );

			actBody->setPositionAndRotation( newRigidPosition, newRigidRotation );
			actBody->setLinearVelocity( hkVector4::getZero() );
			actBody->setAngularVelocity( hkVector4::getZero() );
		}
#endif
	}

	//---------------------------------------------------------------
	//!
	//!	Add the rigid body to the physics world.
	//!
	//---------------------------------------------------------------

	void RigidBody::AddToWorld( )
	{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		ntAssert( m_rigidBody );

		if( !m_rigidBody->isAddedToWorld() )
		{
			if( m_firstActivation )
			{
				CPhysicsWorld::Get().AddEntity( m_rigidBody, HK_ENTITY_ACTIVATION_DO_NOT_ACTIVATE );
				m_firstActivation = false;
			}
			else
			{
				CPhysicsWorld::Get().AddEntity( m_rigidBody, HK_ENTITY_ACTIVATION_DO_ACTIVATE);
			}
		}

		ForceBodyOnTransform();

#endif
	}

	void RigidBody::AddToWorldForced( )
	{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		ntAssert( m_rigidBody );

		Physics::ReadAccess mutex( m_rigidBody );
		if( !m_rigidBody->isAddedToWorld() )
		{
			m_firstActivation=false;
			CPhysicsWorld::Get().AddEntity( m_rigidBody, HK_ENTITY_ACTIVATION_DO_ACTIVATE);		
		}
		ForceBodyOnTransform();
#endif
	}

	//---------------------------------------------------------------
	//!
	//!	Remove the rigid body from the physics world.
	//!
	//---------------------------------------------------------------

	void RigidBody::RemoveFromWorld( )
	{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		if (m_pobBreakableKeyframed)
			SetBreakableKeyframedMotion(false);

		if( m_rigidBody )
		{
			if( m_rigidBody->getWorld() )
			{
				Physics::WriteAccess world_mutex;
				Physics::WriteAccess rb_mutex( m_rigidBody );

				Physics::FilterExceptionFlag exceptionFlagBackup; exceptionFlagBackup.base = 0;
				if( m_rigidBody->hasProperty(Physics::PROPERTY_FILTER_EXCEPTION_INT) )
				{
					exceptionFlagBackup.base = m_rigidBody->getProperty( Physics::PROPERTY_FILTER_EXCEPTION_INT ).getInt() ;
					m_rigidBody->removeProperty( Physics::PROPERTY_FILTER_EXCEPTION_INT );
				}
				
				Physics::FilterExceptionFlag exceptionFlag; exceptionFlag.base = 0;
				exceptionFlag.flags.exception_set |= Physics::ALWAYS_RETURN_FALSE_BIT;
				hkPropertyValue val2((int)exceptionFlag.base);
				m_rigidBody->addProperty(Physics::PROPERTY_FILTER_EXCEPTION_INT, val2);
				m_rigidBody->getWorld()->updateCollisionFilterOnEntity( m_rigidBody, HK_UPDATE_FILTER_ON_ENTITY_FULL_CHECK, HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS );
				
				//if( m_rigidBody->getWorld() == 0 )
				//	ntPrintf( "What ?\n" );

				m_rigidBody->getWorld()->removeEntity( m_rigidBody );

				if( m_rigidBody->hasProperty(Physics::PROPERTY_FILTER_EXCEPTION_INT) )
					m_rigidBody->removeProperty( Physics::PROPERTY_FILTER_EXCEPTION_INT );

				if( exceptionFlagBackup.base != 0 )
				{
					hkPropertyValue val2((int)exceptionFlagBackup.base);
					m_rigidBody->addProperty(Physics::PROPERTY_FILTER_EXCEPTION_INT, val2);
				}
			}
		}
#endif
	}


	//---------------------------------------------------------------
	//!
	//!	Attach the body to an entity
	//!		\param  Entity*		- Entity pointer.
	//!
	//---------------------------------------------------------------

	void RigidBody::AttachToEntity( const CEntity* p_entity )
	{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		ntAssert( m_rigidBody );

		Physics::WriteAccess mutex( m_rigidBody );

		hkPropertyValue val( (void*)p_entity );
		m_rigidBody->addProperty( Physics::PROPERTY_ENTITY_PTR, val );
#else
		UNUSED( p_entity );
#endif
	}

	//---------------------------------------------------------------
	//!
	//!	Enables or disables BreakableKeyframed motion type. A body in BreakableKeyframed motion should not penetrate into other shapes. 
	//! Body is simulated as dynamic. Its position is synchronized with animation using hardkeyframe. If it collides it is "released", breakable keyframed
	//! state is changed to dynamic. 
	//! 
	//!		\param  on  - if true enable BreakableKeyframed motion, if false disable. 
	//!
	//---------------------------------------------------------------

	void RigidBody::SetBreakableKeyframedMotion(bool on)
	{
	    Physics::WriteAccess world_mutex;

		if (on)
		{
			if (m_eMotionType == HS_MOTION_BREAKABLE_KEYFRAMED)
				return;

			m_eMotionType = HS_MOTION_BREAKABLE_KEYFRAMED;
			ntAssert(!m_pobBreakableKeyframed);
			m_pobBreakableKeyframed = NT_NEW BreakableKeyframedMotionParams;
			
			m_rigidBody->setMotionType(hkMotion::MOTION_DYNAMIC);						 
			// during the keyframed state rigid body can have arbitrary velocities => override any limitations
            m_pobBreakableKeyframed->m_maxAngularVelocityBackUp = m_rigidBody->getMaxAngularVelocity();
			m_pobBreakableKeyframed->m_maxLinearVelocityBackUp = m_rigidBody->getMaxLinearVelocity();
			m_rigidBody->setMaxAngularVelocity(200);
			m_rigidBody->setMaxLinearVelocity(200);
					
			m_rigidBody->addCollisionListener(&m_pobBreakableKeyframed->m_isInContact);
		}
		else
		{
			if (m_eMotionType != HS_MOTION_BREAKABLE_KEYFRAMED)
				return;		

			m_eMotionType = HS_MOTION_DYNAMIC;
			ntAssert(m_pobBreakableKeyframed);
									
			// remove collision listener
			m_rigidBody->removeCollisionListener(&m_pobBreakableKeyframed->m_isInContact);

			// set back max velocities
			m_rigidBody->setMaxAngularVelocity(m_pobBreakableKeyframed->m_maxAngularVelocityBackUp);
			m_rigidBody->setMaxLinearVelocity(m_pobBreakableKeyframed->m_maxLinearVelocityBackUp);

			// delete m_pobBreakableKeyframed
			NT_DELETE(m_pobBreakableKeyframed);
			m_pobBreakableKeyframed = 0; 
		}
	}	

	//---------------------------------------------------------------
	//!
	//!	Factory: Construct a rigid body.
	//!		\return RigidBody*			- The newly constructed body.
	//!		\param  Entity*				- Pointer to the owner entity.
	//!		\param  const BodyCInfo*	- Construction informations.
	//!
	//---------------------------------------------------------------

	RigidBody* RigidBody::ConstructRigidBody(		CEntity*			p_entity, 
													const BodyCInfo*	p_info )
	{
		RigidBody* temp = NT_NEW RigidBody();

		temp->m_entity		= p_entity;
		temp->Construct( p_info );
		temp->AttachToEntity( p_entity );
		
		return temp;
	}

	//---------------------------------------------------------------
	//!
	//!	Factory: Construct a rigid body.
	//!		\return RigidBody*			- The newly constructed body.
	//!		\param  Entity*				- Pointer to the owner entity.
	//!		\param  const BodyCInfo*	- Construction informations.
	//!
	//---------------------------------------------------------------

	RigidBody* RigidBody::ConstructRigidBody(		hkRigidBody * body,
													CEntity*			entity, 
													Transform *         transform)													
	{
		RigidBody* temp = NT_NEW RigidBody();

		temp->m_entity		= entity;
		temp->m_transform   = transform;
		temp->m_rigidBody	= body; 
		temp->AttachToEntity( entity );
		SetMotionTypeAfterConstruct(temp);
		
		return temp;
	}

	//---------------------------------------------------------------
	//!
	//!	Factory: Construct a rigid body.
	//!		\return RigidBody*			- The newly constructed body.
	//!		\param  Entity*				- Pointer to the owner entity.
	//!		\param  const BodyCInfo*	- Construction informations.
	//!
	//---------------------------------------------------------------

	void RigidBody::SetMotionTypeAfterConstruct(RigidBody * body)
	{
		switch(body->GetHkRigidBody()->getMotionType())
		{
		case hkMotion::MOTION_DYNAMIC:
		case hkMotion::MOTION_SPHERE_INERTIA:
		case hkMotion::MOTION_STABILIZED_SPHERE_INERTIA:
		case hkMotion::MOTION_BOX_INERTIA:
		case hkMotion::MOTION_STABILIZED_BOX_INERTIA:
		case hkMotion::MOTION_THIN_BOX_INERTIA:
			body->m_eMotionType = HS_MOTION_DYNAMIC;
			break;
		case hkMotion::MOTION_KEYFRAMED:
			body->m_eMotionType = HS_MOTION_KEYFRAMED;
			body->m_pobKeyframed = NT_NEW_CHUNK ( MC_PHYSICS ) KeyframedMotionParams();
			break;
		case hkMotion::MOTION_FIXED:
			body->m_eMotionType = HS_MOTION_FIXED;
			break;
		default:
			body->m_eMotionType = HS_MOTION_DYNAMIC;
		}
	};

	//---------------------------------------------------------------
	//!
	//!	Called when the rigid body is added to the system
	//!		\param  System*				- Pointer to the system.
	//!		\param  const BodyCInfo*	- Construction informations.
	//!
	//---------------------------------------------------------------

	void RigidBody::AddedToSystem( System* p_system )
	{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		Physics::WriteAccess mutex;

		p_system->GetCollisionListener()->RegisterRigidBody( m_rigidBody );
#else
		UNUSED ( p_system );
#endif
	}

	void RigidBody::RemovedFromSystem( System* p_system )
	{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		Physics::WriteAccess mutex( m_rigidBody );

		m_rigidBody->removeCollisionListener( p_system->GetCollisionListener() );
#else
		UNUSED ( p_system );
#endif
	}

	//---------------------------------------------------------------
	//!
	//!	Return the element type.
	//!		\return Element::TYPE - Get the type of the Element.
	//!
	//---------------------------------------------------------------

	const Element::ELEMENT_TYPE RigidBody::GetType( ) const
	{
		return Element::RIGID_BODY;
	}

	//---------------------------------------------------------------
	//!
	//!	Set the motion type of the rigid body.
	//!		\param hkMotion::MotionType - Motion type.
	//!
	//---------------------------------------------------------------	
	void RigidBody::SetMotionType( EMotionType eMotionType)
	{
		Physics::WriteAccess mutex;

		switch (eMotionType)
		{
		case HS_MOTION_DYNAMIC:
			SetBreakableKeyframedMotion(false);
			m_rigidBody->setMotionType( hkMotion::MOTION_DYNAMIC);				
			break;
		case HS_MOTION_KEYFRAMED_STOPPED:								
			SetBreakableKeyframedMotion(false);

			if (!m_pobKeyframed)
			{
				m_pobKeyframed = NT_NEW_CHUNK ( MC_PHYSICS ) KeyframedMotionParams();
				m_pobKeyframed->SetLastTransform(m_transform->GetWorldMatrix());
			}

			m_pobKeyframed->m_lastAnimLinearVelocity.setZero4();
			m_pobKeyframed->m_lastAnimAngularVelocity.setZero4();
			m_rigidBody->setMotionType( hkMotion::MOTION_KEYFRAMED);
			break;

		case HS_MOTION_KEYFRAMED:						

			SetBreakableKeyframedMotion(false);
			if (!m_pobKeyframed)
			{
				m_pobKeyframed = NT_NEW_CHUNK ( MC_PHYSICS ) KeyframedMotionParams();
				m_pobKeyframed->m_lastAnimLinearVelocity = m_rigidBody->getLinearVelocity();
				m_pobKeyframed->m_lastAnimAngularVelocity = m_rigidBody->getAngularVelocity();	
				m_pobKeyframed->SetLastTransform( m_transform->GetWorldMatrix());
			}

			m_rigidBody->setMotionType( hkMotion::MOTION_KEYFRAMED);
			break;			 
		case HS_MOTION_FIXED:
			SetBreakableKeyframedMotion(false);
			m_rigidBody->setMotionType( hkMotion::MOTION_FIXED);
			if (m_pobKeyframed)
			{
				NT_DELETE_CHUNK ( MC_PHYSICS, m_pobKeyframed);
				m_pobKeyframed = 0; 
			}
			break;
		case HS_MOTION_BREAKABLE_KEYFRAMED:
			if (!m_pobKeyframed)
			{
				m_pobKeyframed = NT_NEW_CHUNK ( MC_PHYSICS ) KeyframedMotionParams();
				m_pobKeyframed->m_lastAnimLinearVelocity = m_rigidBody->getLinearVelocity();
				m_pobKeyframed->m_lastAnimAngularVelocity = m_rigidBody->getAngularVelocity();
				m_pobKeyframed->SetLastTransform( m_transform->GetWorldMatrix());
			}

			SetBreakableKeyframedMotion(true);
			break;
		};
		m_eMotionType = eMotionType; 
	}

	//---------------------------------------------------------------
	//!
	//!	Return the linear velocity for this body.
	//!		\return CVector - Get the linear velocity.
	//!
	//---------------------------------------------------------------

	CVector RigidBody::GetLinearVelocity( ) const
	{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		Physics::ReadAccess mutex( m_rigidBody );
		return Physics::MathsTools::hkVectorToCVector( m_rigidBody->getLinearVelocity() );
#else
		return CVector();
#endif
	}

	//---------------------------------------------------------------
	//!
	//!	Set the linear velocity for this body.
	//!		\param CVector - The linear velocity.
	//!
	//---------------------------------------------------------------

	void RigidBody::SetLinearVelocity( const CVector& p_newVel ) 
	{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		ntAssert(!m_pobBreakableKeyframed);
		Physics::WriteAccess mutex( m_rigidBody );
		m_rigidBody->setLinearVelocity( Physics::MathsTools::CVectorTohkVector( p_newVel ) );
#else
		UNUSED( p_newVel );
#endif
	}

	//---------------------------------------------------------------
	//!
	//!	Return the angular velocity for this body.
	//!		\return CVector - Get the angular velocity.
	//!
	//---------------------------------------------------------------

	CVector RigidBody::GetAngularVelocity( ) const
	{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		Physics::ReadAccess mutex( m_rigidBody );
		return Physics::MathsTools::hkVectorToCVector( m_rigidBody->getAngularVelocity() );
#else
		return CVector();
#endif
	}

	//---------------------------------------------------------------
	//!
	//!	Set the angular velocity for this body.
	//!		\param CVector - The angular velocity.
	//!
	//---------------------------------------------------------------

	void RigidBody::SetAngularVelocity( const CVector& p_newVel ) 
	{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		Physics::WriteAccess mutex( m_rigidBody );
		ntAssert(!m_pobBreakableKeyframed);
		m_rigidBody->setAngularVelocity( Physics::MathsTools::CVectorTohkVector( p_newVel ) );
#else
		UNUSED( p_newVel );
#endif
	}

	//---------------------------------------------------------------
	//!
	//!	Return the element name.
	//!		\return ntstd::String - Get the name of the Element.
	//!
	//---------------------------------------------------------------

	const ntstd::String RigidBody::GetName( ) const
	{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		ntAssert( m_rigidBody ); 

		Physics::ReadAccess mutex( m_rigidBody );
		return ntstd::String( m_rigidBody->getName() );
#else
		return ntstd::String("blah!");
#endif
	}

	//---------------------------------------------------------------
	//!
	//!	Main Update.
	//!		\param float - Timestep.
	//!
	//---------------------------------------------------------------

	void RigidBody::Update( float p_timeStep )
	{ 
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		ntAssert( m_rigidBody ); 
		if( m_rigidBody->isAddedToWorld() )
		{
			switch( m_eMotionType )
			{
				case HS_MOTION_BREAKABLE_KEYFRAMED :
				{
					ntAssert(m_pobBreakableKeyframed);											
					if ( m_pobBreakableKeyframed->m_isInContact.IsColliding()) 
					{
						// release from keyframed motion												
						SetBreakableKeyframedMotion(false);
						SyncTransformOnBody();
					}
					else
					{												
						m_pobBreakableKeyframed->m_isInContact.Update();
						SyncBodyOnTransform( p_timeStep );
					}								
					break;
				}
				case HS_MOTION_KEYFRAMED :
				{				
					SyncBodyOnTransform( p_timeStep );
					break;
				}
				case HS_MOTION_FIXED :
				case HS_MOTION_KEYFRAMED_STOPPED :
				{
					break;
				}

				default:
				{
					//ntAssert(!m_rigidBody->isFixedOrKeyframed());
					SyncTransformOnBody();
					break;
				}
			}


			ntstd::List<Behavior*>::iterator it = m_behaviorList.begin();
			while (	it != m_behaviorList.end() )
			{
				Behavior* event = (*it);
				++it;
				bool remove = event->Update( (Element*) this );
				if( remove )
				{
					m_behaviorList.remove( event );
					NT_DELETE( event );
				}
			}
		}
#else	
		UNUSED( p_timeStep );
#endif
	}

	//---------------------------------------------------------------
	//!
	//!	Activate the element.
	//!
	//---------------------------------------------------------------

	void RigidBody::Activate( bool activateInHavok )
	{ 
		AddToWorld();
		//DGF HACK
		if (activateInHavok)
			m_rigidBody->activate();		
	}

	void RigidBody::ActivateForced( )
	{ 
		AddToWorldForced();
	}

	//---------------------------------------------------------------
	//!
	//!	Deactivate the element.
	//!
	//---------------------------------------------------------------

	void RigidBody::Deactivate( )
	{
		for (	ntstd::List<Behavior*>::iterator it = m_behaviorList.begin(); 
				it != m_behaviorList.end(); ++it )
		{
			Behavior* current = (*it);
			NT_DELETE( current );
		}

		m_behaviorList.clear();

		RemoveFromWorld();

	}

	//---------------------------------------------------------------
	//!
	//!	Return the havok rigid body.
	//!		\return hkRigidBody* - Havok rigid body.
	//!
	//---------------------------------------------------------------
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
	hkRigidBody* RigidBody::GetHkRigidBody( ) const
	{
		return m_rigidBody;
	}
#endif
	//---------------------------------------------------------------
	//!
	//!	Return the associated transform.
	//!		\return Transform* - Current transform.
	//!
	//---------------------------------------------------------------

	Transform* RigidBody::GetTransform( ) const
	{
		return m_transform;
	}

	//---------------------------------------------------------------
	//!
	//!	Return the owner entity.
	//!		\return CEntity* - Owner Entity.
	//!
	//---------------------------------------------------------------

	const CEntity* RigidBody::GetEntity( ) const
	{
		return m_entity;
	}

	//---------------------------------------------------------------
	//!
	//! Flag the rigid body.
	//!		\param PhysicsProperty - Property to set.
	//!		\param int - Value.
	//!
	//---------------------------------------------------------------

	void RigidBody::SetHavokFlag( PhysicsProperty p_prop, int p_value )
	{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		Physics::WriteAccess mutex( m_rigidBody );

		if( m_rigidBody->hasProperty( p_prop ) )
		{
			m_rigidBody->removeProperty( p_prop );
		}

		hkPropertyValue val( p_value );
		m_rigidBody->addProperty( p_prop, val);
#else
		UNUSED( p_prop); UNUSED( p_value );
#endif
	}

	//---------------------------------------------------------------
	//!
	//! Rigid Body destructor.
	//!
	//---------------------------------------------------------------

	RigidBody::~RigidBody( )
	{
		m_transform = 0;
		Deactivate();
		m_entity	= 0;
		RemoveFromWorld();
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		if (m_bDestroyHavokObject)
		{
			m_rigidBody->removeReference();
			m_rigidBody = 0;
		}
		if (m_pobBreakableKeyframed)
		{
			SetBreakableKeyframedMotion(false);
		}	
		if (m_pobKeyframed)
		{
			NT_DELETE_CHUNK( MC_PHYSICS, m_pobKeyframed );
		}	

#endif
	}

	//---------------------------------------------------------------
	//!
	//! Rigid Body constructor.
	//!
	//---------------------------------------------------------------

	RigidBody::RigidBody( ):

		m_bDestroyHavokObject(true),
		m_pobKeyframed( 0 ),
	    m_firstActivation( true ),
		m_pobBreakableKeyframed(0),
		m_transform( 0 ),
		m_entity( 0 ),
		m_bHardKeyframing(false),
		m_stateBeforePause(Undefined)
	{ 
		m_eMotionType = HS_MOTION_DYNAMIC;
	}

	//----------------------------------------------------------
	//!
	//!	Update the collision filters. 
	//!
	//----------------------------------------------------------

	void RigidBody::UpdateCollisionFilter ()
	{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		ntAssert( m_rigidBody );
		Physics::ReadAccess mutex( m_rigidBody );

		if(m_rigidBody->isAddedToWorld() )
		{
			CPhysicsWorld::Get().UpdateCollisionFilterOnEntity(	m_rigidBody, 
																HK_UPDATE_FILTER_ON_ENTITY_FULL_CHECK, 
																HK_UPDATE_COLLECTION_FILTER_PROCESS_SHAPE_COLLECTIONS );
		}
#endif
	}

	void RigidBody::SetCollisionFilterInfo(uint32_t info)
	{
		ntAssert( m_rigidBody );
		Physics::WriteAccess mutex;
		m_rigidBody->getCollidableRw()->setCollisionFilterInfo(info);
	}

	uint32_t RigidBody::GetCollisionFilterInfo() const
	{
		ntAssert( m_rigidBody );
		Physics::ReadAccess mutex( m_rigidBody );

		return m_rigidBody->getCollidable()->getCollisionFilterInfo();
	}

	void RigidBody::ApplyLinearImpulse( const CDirection& obForce )
	{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		ntAssert(m_rigidBody );
		ntAssert(!m_pobBreakableKeyframed);
		Physics::ReadAccess rmutex( m_rigidBody );
		if( m_rigidBody->isAddedToWorld() )
		{
			Physics::WriteAccess wmutex;
			m_rigidBody->applyLinearImpulse( Physics::MathsTools::CDirectionTohkVector( obForce ) );
		}
#else
		UNUSED( obForce );
#endif
	}

	void RigidBody::ApplyLocalisedLinearImpulse( const CDirection& p_vel, const CVector& p_point )
	{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		ntAssert( m_rigidBody );
		ntAssert(!m_pobBreakableKeyframed);
		Physics::ReadAccess rmutex( m_rigidBody );
		if( m_rigidBody->isAddedToWorld() )
		{
			Physics::WriteAccess wmutex;
		    m_rigidBody->applyPointImpulse( Physics::MathsTools::CDirectionTohkVector( p_vel ), Physics::MathsTools::CVectorTohkVector( p_point )  );
		}
#else
		UNUSED( p_vel );
		UNUSED( p_point );
#endif
	}

	void RigidBody::ApplyAngularImpulse( const CDirection& obForce )	
	{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		ntAssert( m_rigidBody );
		ntAssert(!m_pobBreakableKeyframed);
		Physics::ReadAccess rmutex( m_rigidBody );
		if( m_rigidBody->isAddedToWorld() )
		{
			Physics::WriteAccess wmutex;
			m_rigidBody->applyAngularImpulse( Physics::MathsTools::CDirectionTohkVector( obForce ) );
		}
#else
		UNUSED( obForce );
#endif
	}

	void RigidBody::Debug_RenderCollisionInfo ()
	{
		DebugCollisionTools::RenderCollisionFlags(m_rigidBody);

		CMatrix obWorldMatrix(
			CQuat(m_rigidBody->getRotation()(0),m_rigidBody->getRotation()(1),m_rigidBody->getRotation()(2),m_rigidBody->getRotation()(3)),
			CPoint(m_rigidBody->getPosition()(0),m_rigidBody->getPosition()(1),m_rigidBody->getPosition()(2)));
	
		DebugCollisionTools::RenderShape(obWorldMatrix,m_rigidBody->getCollidable()->getShape());
	}

	/*bool RigidBody::IsInPenetration() const
	{		
		if (m_rigidBody && m_rigidBody->getWorld())
		{
			// Ensure the rigid body is synchronised with it's transform 
			// On other side keep position of the body after test. Its position should be changed during Update.

			Physics::ReadAccess rmutex;

			CMatrix obWorldMatrix(GetTransform()->GetWorldMatrix());
			CQuat obRotation(obWorldMatrix);

			hkTransform backUpTransform = m_rigidBody->getTransform(); 

			m_rigidBody->setPosition(
				hkVector4(obWorldMatrix.GetTranslation().X(),obWorldMatrix.GetTranslation().Y(),obWorldMatrix.GetTranslation().Z()));
			m_rigidBody->setRotation(
				hkQuaternion(obRotation.X(),obRotation.Y(),obRotation.Z(),obRotation.W()));

			hkFlagCdBodyPairCollector isPenetration;
			hkCollisionInput * collInput = (hkCollisionInput *)m_rigidBody->getWorld()->getCollisionInput();
			m_rigidBody->getWorld()->getPenetrations(m_rigidBody->getCollidable(), *collInput,isPenetration);

			m_rigidBody->setTransform(backUpTransform);

			return isPenetration.hasHit();
		}

		return false;
	}*/

	//---------------------------------------------------------------
	//!
	//! RigidBody::MoveToSafe 
	//! Move rigid body to new position, but only if it do not collide
	//! on path from current to new position
	//!
	//---------------------------------------------------------------

	class FlagCdPointCollector : public hkCdPointCollector
	{
	public:
		FlagCdPointCollector() : m_bFound(false) {};

		void addCdPoint (const hkCdPoint &point) {m_bFound = true;};		
		bool Found() const {return m_bFound;};

	protected:
		bool m_bFound;	
	};


	bool RigidBody::MoveToSafe(const CPoint& endPos)
	{
		ntAssert(m_rigidBody);

		if (m_rigidBody->getWorld())
		{						 
			Physics::ReadAccess rmutex;
			hkLinearCastInput input; 
			input.m_to = MathsTools::CPointTohkVector(endPos);

			FlagCdPointCollector coll;

			m_rigidBody->getWorld()->linearCast(m_rigidBody->getCollidable(), input, coll, &coll);
			if (!coll.Found())
			{
				// move rigid body to new position
				Physics::WriteAccess rmutex;
				m_rigidBody->setPosition(input.m_to); 
				m_entity->SetPosition(endPos);	
				return true;
			}
			return false;
		}
		else
		{
			m_rigidBody->setPosition(MathsTools::CPointTohkVector(endPos));
			m_entity->SetPosition(endPos);
			return true;
		}
	}

	//---------------------------------------------------------------
	//!
	//! RigidBody::SetRotationSafe 
	//! Set rotation, but only if body is not colliding in new rotation 	
	//!
	//---------------------------------------------------------------
	bool RigidBody::SetRotationSafe(const CQuat& orient)
	{
		ntAssert(m_rigidBody);

		if (m_rigidBody->getWorld())
		{			
			Physics::ReadAccess rmutex;
			hkQuaternion backUpRotation = m_rigidBody->getRotation(); 
			m_rigidBody->setRotation(MathsTools::CQuatTohkQuaternion(orient));				

			hkFlagCdBodyPairCollector isPenetration;
			hkCollisionInput * collInput = (hkCollisionInput *)m_rigidBody->getWorld()->getCollisionInput();
			m_rigidBody->getWorld()->getPenetrations(m_rigidBody->getCollidable(), *collInput,isPenetration);		

			if(isPenetration.hasHit())
			{
				// uppsss hit return old transform
				m_rigidBody->setRotation(backUpRotation);
				return false;			
			}
			else
			{
				// tell also entity
				m_entity->SetRotation(orient);
				return true;
			}
		}
		else
		{
			m_rigidBody->setRotation(MathsTools::CQuatTohkQuaternion(orient));	
			m_entity->SetRotation(orient);
			return true;
		}
	}

	//! Added collision listener to bodies in group
	void RigidBody::AddCollisionListener(hkCollisionListener * cl)
	{
		if (m_rigidBody)
			m_rigidBody->addCollisionListener(cl);
	}

	//! Remove collision listener from bodies in group
	void RigidBody::RemoveCollisionListener(hkCollisionListener * cl)
	{
		if (m_rigidBody)
			m_rigidBody->removeCollisionListener(cl);
	}

	void RigidBody::PausePresenceInHavokWorld(bool pause)
	{
		if (pause)
		{
			// remove it from havok world
			if (m_rigidBody->isAddedToWorld())
			{
				if (m_rigidBody->isActive())
					m_stateBeforePause = Activated;					
				else
					m_stateBeforePause = Deactivated;								
			}
			else
			{
				m_stateBeforePause = NotInWorld;
				return;
			}

			Physics::WriteAccess world_mutex;										
			m_rigidBody->getWorld()->removeEntity( m_rigidBody );
		}
		else
		{
			// add it from havok world
			if (m_stateBeforePause == NotInWorld || m_stateBeforePause == Undefined)
				return; // nothing to do

			Physics::WriteAccess world_mutex;
			hkWorld * world = CPhysicsWorld::Get().GetHavokWorldP();			
			world->addEntity(m_rigidBody,
					(m_stateBeforePause == Activated) ? HK_ENTITY_ACTIVATION_DO_ACTIVATE: HK_ENTITY_ACTIVATION_DO_NOT_ACTIVATE);

			m_stateBeforePause = Undefined;
		}
	}

	void RigidBody::SetLastAnimVelocities()
	{
		if (m_rigidBody && m_pobKeyframed) 
		{
		/*	char text[1024];
			sprintf(text, "------------------------------\n lin vel %lf %lf %lf\n ang vel %lf %lf %lf\n",
				m_pobKeyframed->m_forLastAnimLinearVelocity(0), m_pobKeyframed->m_forLastAnimLinearVelocity(1), m_pobKeyframed->m_forLastAnimLinearVelocity(2),
				m_pobKeyframed->m_forLastAnimAngularVelocity(0), m_pobKeyframed->m_forLastAnimAngularVelocity(1), m_pobKeyframed->m_forLastAnimAngularVelocity(2));
			Debug::AlwaysOutputString(text);*/
			// If animation ends in between frames last velocity is wrong, that's why we use velocities from one frame before
			m_rigidBody->setLinearVelocity(m_pobKeyframed->m_lastAnimLinearVelocity);
			m_rigidBody->setAngularVelocity(m_pobKeyframed->m_lastAnimAngularVelocity);
		}
	}

} // Physics

