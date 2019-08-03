//---------------------------------------------------------------------------------------------------------
//!
//!	\file physics/system.cpp
//!	
//!	DYNAMICS COMPONENT:	
//!
//!	Author: Mustapha Bismi (mustapha.bismi@ninjatheory.com)
//! Created: 2005.07.11
//!
//---------------------------------------------------------------------------------------------------------

#include "config.h"


#include "physics/havokincludes.h"

#include "system.h"
#include "world.h"
#include "element.h"
#include "behavior.h"
#include "rigidbody.h"
#include "maths_tools.h"


#include "objectdatabase/dataobject.h"
#include "objectdatabase/objectdatabase.h"

//#include "xmlinterfaces.h"

#include "game/entity.h"
#include "game/entity.inl"

// These are beeded for collision strike stuff
#include "game/attacks.h" 
#include "game/syncdcombat.h"
#include "game/strike.h"
#include "game/messagehandler.h"
#include "core/timer.h"

#include "physics/collisionlistener.h"

#include "animatedlg.h"
#include "staticlg.h"
#include "singlerigidlg.h"
#include "spearlg.h"
#include "compoundlg.h"
#include "softlg.h"

// Headers needed for visual debugging stuff
#include "core/visualdebugger.h"
#include "camera/camutils.h"
#include <hkdynamics/phantom/hkShapePhantom.h>
#include <hkdynamics/motion/hkMotion.h>
#include <hkcollide/shape/mopp/hkMoppBvTreeShape.h>
#include <hkdynamics/constraint/bilateral/limitedhinge/hkLimitedHingeConstraintData.h>
#include <hkdynamics/constraint/hkConstraintData.h>


#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
#include "advancedcharactercontroller.h"

#include <process.h>

// #define AUTOMATIC_NEW_SYSTEM_CONVERSION

#endif

#include "objectdatabase/dataobject.h"
#include "objectdatabase/objectdatabase.h"

#include "audio/collisioneffecthandler.h"


namespace Physics {

	CollisionStrikeHandler::CollisionStrikeHandler (CEntity* pobParentEntity) :
		m_iNumCollidee( 0 ),
		m_pobParentEntity(pobParentEntity),
		m_iMaxStrikes(0),
		m_fTimeOut(0.0f),
		m_iStrikeCount(0),
		m_iStrikeFilter(ENEMY),//m_iStrikeFilter(ENEMY | PLAYER),
		m_bEnabled(false),
		m_bStruckByKO( false )
	{
	}

	void CollisionStrikeHandler::Update ()
	{
		int iNumCollidees = m_iNumCollidee;
		for( int i=0;i < iNumCollidees;i++ )
		{
			CEntity* pobCollidee = m_pCollidees[i];
			float projVel = m_fProjVel[i];

			if (m_pobParentEntity->GetMessageHandler())
			{
				Message obMessage(msg_collision);
				obMessage.SetEnt( "Collidee", pobCollidee );
				obMessage.SetFloat( "ProjVel", projVel );
				m_pobParentEntity->GetMessageHandler()->QueueMessage(obMessage);
			}

			if (pobCollidee->GetMessageHandler())
			{
				Message obMessage(msg_collision);
				obMessage.SetEnt( "Collidee", m_pobParentEntity );
				obMessage.SetFloat( "ProjVel", projVel );
				m_pobParentEntity->GetMessageHandler()->QueueMessage(obMessage);
			}
		}
		AtomicSet( &m_iNumCollidee, 0 );


		if (m_bEnabled)
		{
			if (m_fTimeOut>0.0f) // Time out has been set
			{
				m_fTimeOut -= CTimer::Get().GetGameTimeChange();

				if (m_fTimeOut<=0.0f) // Time out has expired, strikes are now disabled
				{
					m_fTimeOut=0.0f;
					Disable();
				}
			}
		}
	}

	void CollisionStrikeHandler::Enable (int iMaxStrikes,float fTimeOut)
	{
		m_bEnabled=true;
		m_iStrikeCount=0;
		m_iMaxStrikes=iMaxStrikes;
		m_fTimeOut=fTimeOut;
		m_bStruckByKO = false;

		//ntPrintf("### %s collision strike enabled\n",m_pobParentEntity->GetName().c_str());
	}

	void CollisionStrikeHandler::Disable ()
	{
		//ntPrintf("### %s collision strike disabled\n",m_pobParentEntity->GetName().c_str());

		m_bEnabled=false;
	}

	bool CollisionStrikeHandler::GenerateStrike (CEntity* pobTarget,const CAttackData* pobAttackData)
	{
		if (!m_bEnabled) // Strike behavior not enabled
		{
			return false;
		}

		if ((pobTarget->IsPlayer() && !(m_iStrikeFilter & PLAYER)) || // Target is a player and player isn't a strike target
			(pobTarget->IsEnemy() && !(m_iStrikeFilter & ENEMY)) || // Target is an enemy and enemy isn't a valid strike target
			((m_iStrikeFilter & ENEMY) && pobTarget->IsEnemy() && pobTarget->GetAttackComponent()->AI_Access_GetState() != CS_STANDARD)) // Don't do strike if target has already been hit
		{
			//ntPrintf("### Rejecting collision strike\n");
			return false;
		}

		CPoint obMidPoint(m_pobParentEntity->GetLocation()+pobTarget->GetLocation());
		obMidPoint*=0.5f;

		CEntity* pobOriginator;

		if (m_pobParentEntity->GetAttackComponent())
			pobOriginator=m_pobParentEntity;
		else
			pobOriginator=0;

		CStrike* pobStrike = NT_NEW CStrike(pobOriginator,pobTarget,pobAttackData,1.0f, 1.0f,false,false,false,false,false,false,0,obMidPoint);

		//ntPrintf("### %s is performing combat strike against %s\n",m_pobParentEntity->GetName().c_str(),pobTarget->GetName().c_str());

		SyncdCombat::Get().PostStrike( pobStrike );

		if ( pobTarget->GetMessageHandler() ) // Send a combat struck message to the target entity (if they have a message handler - which they should have!)
		{
			pobTarget->GetMessageHandler()->ReceiveMsg<msg_combat_struck>();
		}
		
		++m_iStrikeCount; // Successful strike was made

		if (m_iStrikeCount>=m_iMaxStrikes) // We have reached our maximum number of strikes
		{
			Disable();
			m_iStrikeCount=0;
		}

		return true;
	}

	//----------------------------------------------------------

	CollisionCallbackHandler::CollisionCallbackHandler (CEntity* pobParentEntity) :
		m_iNumCollidee( 0 ),
		m_pobParentEntity(pobParentEntity),
		m_bGenerateCallback(false)
	{
	}

	void CollisionCallbackHandler::SetCollisionCallback (float fMinProjVel)
	{
		m_bGenerateCallback=true;
		m_fProjVelThreshold=fMinProjVel;
	}

	void CollisionCallbackHandler::Process (const hkContactPointAddedEvent& event,CEntity* pobCollidee)
	{
		if (m_bGenerateCallback && fabsf(event.m_projectedVelocity) > m_fProjVelThreshold)
		{
			int32_t iLastNum = AtomicIncrement( &m_iNumCollidee );
			if( iLastNum < MAX_COLLIDEES_PER_FRAME )
			{
				m_pCollidees[iLastNum] = pobCollidee;
				m_fProjVel[iLastNum] = event.m_projectedVelocity;
			} else
			{
				AtomicSet( &m_iNumCollidee, MAX_COLLIDEES_PER_FRAME-1 );
			}

			//ntPrintf("Triggering collision callback for %s\n",m_pobParentEntity->GetName().c_str());

			m_bGenerateCallback=false;
		}
	}

	void CollisionCallbackHandler::Update ()
	{
		int iNumCollidees = m_iNumCollidee;
		for( int i=0;i < iNumCollidees;i++ )
		{
			CEntity* pobCollidee = m_pCollidees[i];
			float projVel = m_fProjVel[i];

			if (m_pobParentEntity->GetMessageHandler())
			{
				Message obMessage(msg_collision);
				obMessage.SetEnt( "Collidee", pobCollidee );
				obMessage.SetFloat( "ProjVel", projVel );
				m_pobParentEntity->GetMessageHandler()->QueueMessage(obMessage);
			}

			if (pobCollidee->GetMessageHandler())
			{
				Message obMessage(msg_collision);
				obMessage.SetEnt( "Collidee", m_pobParentEntity );
				obMessage.SetFloat( "ProjVel", projVel );
				m_pobParentEntity->GetMessageHandler()->QueueMessage(obMessage);
			}
		}

		AtomicSet( &m_iNumCollidee, 0 );
	}

	//----------------------------------------------------------
	//!
	//!	Construct a system.
	//!		\param  CEntity* - Entity associated with this system.
	//!		\param  p_entity, const ntstd::String& - System name.
	//!
	//----------------------------------------------------------

	System::System( CEntity* p_entity, const ntstd::String& p_name  ):
		LogicGroup( p_name, p_entity ), m_pobParentEntity(p_entity)
	{
		ATTACH_LUA_INTERFACE(System);
		/*
		m_doHurt = false;
		m_immuneToRC = false;
		m_hurtTiming = 0.0f;
		*/
		m_bMsgOnCollision = false;

		m_pobCollisionStrikeHandler = NT_NEW_CHUNK ( MC_PHYSICS ) CollisionStrikeHandler( p_entity );
		m_pobCollisionCallbackHandler = NT_NEW_CHUNK ( MC_PHYSICS ) CollisionCallbackHandler( p_entity );
		m_pobCollisionEffectHandler = NT_NEW_CHUNK ( MC_PHYSICS ) CollisionEffectHandler();

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		m_collisionListener = NT_NEW_CHUNK ( MC_PHYSICS ) CCollisionListener( this, p_entity );
#endif
		m_bSendCollisionMessageNextUpdate = false;
	}

	void System::SetCharacterCollisionRepresentation( CHARACTER_MODE eChar, COLLISION_MODE eCol )
	{
		Physics::AdvancedCharacterController* pobCharacterState = (Physics::AdvancedCharacterController*) GetFirstGroupByType( Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER );

		if( pobCharacterState )
		{
			switch( eChar )
			{
			case CHARACTER_CONTROLLER:
				pobCharacterState->ActivateCharacterController();
				break;
			case RAGDOLL_DEAD:
				pobCharacterState->SetRagdollDead();
				break;
			case RAGDOLL_ANIMATED_PHYSICALLY:
				pobCharacterState->SetRagdollAnimated(Physics::AdvancedRagdoll::ALL_BONES);
				break;
			case RAGDOLL_ANIMATED_ABSOLUTELY:
				pobCharacterState->SetRagdollAnimatedAbsolute(Physics::AdvancedRagdoll::ALL_BONES);
				break;
			case RAGDOLL_TRANSFORM_TRACKING_PHYSICALLY:
				//pobCharacterState->SetRagdollTransformTracking();
				break;
			case RAGDOLL_TRANSFORM_TRACKING_ABSOLUTELY:
				//pobCharacterState->SetRagdollTransformTrackingAbsolute();
				break;
			}

			switch( eCol )
			{
			case CHARACTER_CONTROLLER_COLLIDE_WITH_EVERYTHING:
				pobCharacterState->SetCharacterControllerCollidable( true );
				pobCharacterState->SetCharacterControllerDynamicCollidable( true );
				break;
			case CHARACTER_CONTROLLER_COLLIDE_WITH_STATIC_ONLY:
				pobCharacterState->SetCharacterControllerCollidable( true );
				pobCharacterState->SetCharacterControllerDynamicCollidable( false );
				break;
			case CHARACTER_CONTROLLER_DO_NOT_COLLIDE:
				pobCharacterState->SetCharacterControllerCollidable( false );
				break;
			case RAGDOLL_DEAD_ON_CONTACT:
				pobCharacterState->SetRagdollTurnDynamicOnContact( true );
				break;
			case RAGDOLL_IGNORE_CONTACT:
				pobCharacterState->SetRagdollTurnDynamicOnContact( false );
				break;
			}
		}
	}

	// JML - Added for respawning, please tidy as necessary
	void System::UnfixRagdoll()
	{		
		Physics::AdvancedCharacterController* pobCharacterState = (Physics::AdvancedCharacterController*) GetFirstGroupByType( Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER );
		ntError_p(pobCharacterState->GetAdvancedRagdoll(), ("Character cann't use function for ragdolls. It doesn't have any ragdoll."));
		pobCharacterState->GetAdvancedRagdoll()->UnfixBones();
	}

	void System::SetKOState( KO_States state )
	{
		Physics::AdvancedCharacterController* pobCharacterState = (Physics::AdvancedCharacterController*) GetFirstGroupByType( Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER );
		if (pobCharacterState)
			pobCharacterState->SetKOState(state);
	}

	//----------------------------------------------------------
	//!
	//!	System destructor.
	//!
	//----------------------------------------------------------

	System::~System()
	{
		for (	ntstd::List<Element*>::iterator it = m_elementList.begin(); 
				it != m_elementList.end(); ++it )
		{
			Element* current = (*it);
			current->RemovedFromSystem( this );
		}

		for (	ntstd::List<LogicGroup*>::iterator it = m_groupList.begin(); 
				it != m_groupList.end(); ++it )
		{
			LogicGroup* current = (*it);
			current->RemovedFromSystem( this );
			NT_DELETE( current );
		}

		/*if( m_character )
			NT_DELETE( m_character );*/

		m_groupList.clear();
		m_entity = 0;

		NT_DELETE_CHUNK( MC_PHYSICS, m_pobCollisionStrikeHandler );
		m_pobCollisionStrikeHandler = NULL;

		NT_DELETE_CHUNK( MC_PHYSICS, m_pobCollisionCallbackHandler );
		m_pobCollisionCallbackHandler = NULL;

		NT_DELETE_CHUNK( MC_PHYSICS, m_pobCollisionEffectHandler );
		m_pobCollisionEffectHandler = NULL;

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		NT_DELETE_CHUNK( MC_PHYSICS, m_collisionListener );
		m_collisionListener = NULL;
#endif
	}

	//---------------------------------------------------------------
	//!
	//!	Return the system type.
	//!		\return LogicGroup::TYPE - Get the type of the Element.
	//!
	//---------------------------------------------------------------

	const LogicGroup::GROUP_TYPE System::GetType( ) const
	{
		return LogicGroup::SYSTEM;
	}

	//----------------------------------------------------------
	//!
	//!	Update the system.
	//!		\param  float - Timestep
	//!
	//----------------------------------------------------------

	void System::Update( float p_timeStep )
	{

		if (m_bSendCollisionMessageNextUpdate)
		{
			CMessageSender::SendEmptyMessage( "msg_obj_collision", m_entity->GetMessageHandler() );
			m_bSendCollisionMessageNextUpdate = false;
		}

		if (m_pobCollisionStrikeHandler)
            m_pobCollisionStrikeHandler->Update();

		if( m_pobCollisionCallbackHandler)
			m_pobCollisionCallbackHandler->Update();

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
		m_collisionListener->Update();
#endif
		for (	ntstd::List<LogicGroup*>::iterator it = m_groupList.begin(); 
				it != m_groupList.end(); ++it )
		{
			LogicGroup* current = (*it);
			if( current->IsActive() )
				current->Update( p_timeStep );
		}

		for (	ntstd::List<Element*>::iterator it = m_elementList.begin(); 
				it != m_elementList.end(); ++it )
		{
			Element* current = (*it);
			current->Update( p_timeStep );
		}

		ntstd::List<Behavior*>::iterator itb = m_behaviorList.begin();
		while (	itb != m_behaviorList.end() )
		{
			Behavior* event = (*itb);
			++itb;
			bool remove = event->Update( this );
			if( remove )
			{
				m_behaviorList.remove( event );
				NT_DELETE( event );
			}
		}

		/*if( m_character )
			m_character->Update( p_timeStep );*/

		if (m_pobCollisionEffectHandler)
			m_pobCollisionEffectHandler->Update(p_timeStep);

#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
// HARVEY!!! Have not idea if this is right!!! but this function doesn't exist anymore
//		m_collisionListener->ClearEvents();
#endif
	}

	void System::EntityRootTransformHasChanged()
	{
		for (	ntstd::List<LogicGroup*>::iterator it = m_groupList.begin(); 
				it != m_groupList.end(); ++it )
		{
			LogicGroup* current = (*it);
			current->EntityRootTransformHasChanged(  );
		}
	}

	//----------------------------------------------------------
	//!
	//!	Activate the whole system.
	//!
	//----------------------------------------------------------

	void System::Activate( bool activateInHavok)
	{
		for (	ntstd::List<Element*>::iterator it = m_elementList.begin(); 
				it != m_elementList.end(); ++it )
		{
			Element* current = (*it);
			current->Activate(activateInHavok);
		}

		for (	ntstd::List<LogicGroup*>::iterator it = m_groupList.begin(); 
				it != m_groupList.end(); ++it )
		{
			LogicGroup* current = (*it);
			current->Activate(activateInHavok);
		}

		LogicGroup::Activate(activateInHavok);
	}

	//----------------------------------------------------------
	//!
	//!	Deactivate the whole system.
	//!
	//----------------------------------------------------------

	void System::Deactivate()
	{
		for (	ntstd::List<Element*>::iterator it = m_elementList.begin(); 
				it != m_elementList.end(); ++it )
		{
			Element* current = (*it);
			current->Deactivate();
		}

		for (	ntstd::List<LogicGroup*>::iterator it = m_groupList.begin(); 
				it != m_groupList.end(); ++it )
		{
			LogicGroup* current = (*it);
			current->Deactivate( );
		}

		LogicGroup::Deactivate();
	}

	//----------------------------------------------------------
	//!
	//!	Activate the whole system. Corresponding entity is visible
	//!
	//----------------------------------------------------------

	void System::PausePresenceInHavokWorld( bool pause )
	{
		for (	ntstd::List<LogicGroup*>::iterator it = m_groupList.begin(); 
				it != m_groupList.end(); ++it )
		{
			LogicGroup* current = (*it);
			current->PausePresenceInHavokWorld(pause);
		}

		LogicGroup::PausePresenceInHavokWorld(pause);
	}		


	//----------------------------------------------------------
	//!
	//!	Return the collision listener.
	//!		\param  const char* - Sound bank name 
	//!
	//----------------------------------------------------------

	CCollisionListener* System::GetCollisionListener( )
	{
		return  m_collisionListener;
	}

	//----------------------------------------------------------
	//	RegisterCollisionEffectFilteDef
	//!	Registers a definition with the collision effect handler.
	//!	@param obName	Definition object name in object database.
	//!
	//----------------------------------------------------------
	void System::RegisterCollisionEffectFilterDef(CHashedString obName)
	{
		GetCollisionEffectHandler()->SetDefName(obName, true);

		// The collision listener contact monitor is disabled by default,
		// so enable it when a collision effect handler is registered.
		GetCollisionListener()->SetContactMonitor(true);
	}

	//----------------------------------------------------------
	//!
	//!	Search for a specific logic group.
	//!		\param  const ntstd::String& - Gruoup name 
	//!		\return  LogicGroup* - Ptr to the logic group, 0 if not found. 
	//!
	//----------------------------------------------------------

	LogicGroup*	System::GetGroupByName( const ntstd::String& p_name )
	{
		for (	ntstd::List<LogicGroup*>::iterator it = m_groupList.begin(); 
				it != m_groupList.end(); ++it )
		{
			LogicGroup* current = (*it);
			if( current->GetName() == p_name )
				return current;
		}

		return 0;
	}

	//----------------------------------------------------------
	//!
	//!	Return the first group of a specified type.
	//!		\param   const TYPE - Gruoup type 
	//!		\return  LogicGroup* - Ptr to the logic group, 0 if not found. 
	//!
	//----------------------------------------------------------

	LogicGroup*	 System::GetFirstGroupByType( const LogicGroup::GROUP_TYPE p_type )
	{
		for (	ntstd::List<LogicGroup*>::iterator it = m_groupList.begin(); 
				it != m_groupList.end(); ++it )
		{
			LogicGroup* current = (*it);
			if( current->GetType() == p_type )
				return current;
		}

		return 0;
	}

	const LogicGroup*	 System::GetFirstGroupByType( const LogicGroup::GROUP_TYPE p_type ) const 
	{
		for (	ntstd::List<LogicGroup*>::const_iterator it = m_groupList.begin(); 
				it != m_groupList.end(); ++it )
		{
			const LogicGroup* current = (*it);
			if( current->GetType() == p_type )
				return current;
		}

		return 0;
	}

	//----------------------------------------------------------
	//!
	//!	Return the group list. 
	//!		\return  ntstd::List<LogicGroup*>& - Group list. 
	//!
	//----------------------------------------------------------

	ntstd::List<LogicGroup*>&	 System::GetGroupList( )
	{
		return m_groupList;
	}

	//----------------------------------------------------------
	//!
	//!	Add a group to the system. 
	//!		\param  LogicGroup* - Group to add. 
	//!
	//----------------------------------------------------------

	void System::AddGroup( LogicGroup* p_groupe )
	{
		if( p_groupe )
		{
			m_groupList.push_back( p_groupe );
			p_groupe->AddedToSystem( this );
		}
	}

	//----------------------------------------------------------
	//!
	//!	Tries to construct a system from a clump. 
	//!		\param  CEntity* - Entity Ptr. 
	//!
	//----------------------------------------------------------

	System* System::ConstructSystemFromClump( CEntity* p_entity )
	{
#ifdef AUTOMATIC_NEW_SYSTEM_CONVERSION
		static char acFileName[ MAX_PATH * 2 ];
		static char psFileName[ MAX_PATH ];
#endif
		ntAssert( p_entity );
		ntAssert( p_entity->GetHierarchy() );

		System* constructedSystem = 0;

		//const char* pcDefaultDynamics = "";

		DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromPointer( p_entity );
		StdDataInterface* pInterface = ObjectDatabase::Get().GetInterface( pDO );
		CHashedString pcDefaultDynamics = pInterface->Get<CHashedString>( pDO, "DefaultDynamics" );

		//if( strcmp( pcDefaultDynamics, "Static" ) == 0 )
		if (CHashedString(HASH_STRING_STATIC) == pcDefaultDynamics)
		{
#ifdef AUTOMATIC_NEW_SYSTEM_CONVERSION
			CHashedString clumpString = p_entity->GetClumpString();
			strcpy( psFileName, *clumpString);
			int stringLength = strlen(psFileName);
			strcpy( psFileName + stringLength - 5, "ps.xml\0" );
			ntPrintf("Converting %s to %s...\n",*clumpString, psFileName);

			ntstd::String temp = *g_ShellOptions->m_obContentPlatform;
			temp += "\\";
			temp += *clumpString;

			ntstd::String temp2 = g_ShellOptions->m_contentNeutral;
			temp2 += "\\";
			temp2 += psFileName;

			temp += " ";
			temp += temp2;

			Util::GetFiosFilePath( temp.c_str(), acFileName );

			ntPrintf("Command line %s...\n", acFileName);
			int ret = spawnl( P_WAIT, "C:\\alienbrainWork\\HS\\content_pc\\ClumpToPSXML.exe", "C:\\alienbrainWork\\HS\\content_pc\\ClumpToPSXML.exe", acFileName );

			if( -1 == ret )
			{
				ntPrintf("Failed to call ClumpToPSXML.exe!...\n");
			}
#endif

			//execl("ClumpToPSXML.exe",*clumpString);
			StaticLG* lg = NT_NEW StaticLG( p_entity->GetName(), p_entity );
			lg->Load();		
			lg->Activate();
			
				constructedSystem = NT_NEW_CHUNK(Mem::MC_ENTITY) System( p_entity, p_entity->GetName() );
				constructedSystem->AddGroup( lg );
			
		} else if (CHashedString(HASH_STRING_STATICENTITYONLY) == pcDefaultDynamics)//if( strcmp( pcDefaultDynamics, "StaticEntityOnly" ) == 0 )
		{
			// not used in the moment will be probably removed 
#ifdef AUTOMATIC_NEW_SYSTEM_CONVERSION
			CHashedString clumpString = p_entity->GetClumpString();
			strcpy( psFileName, *clumpString);
			int stringLength = strlen(psFileName);
			strcpy( psFileName + stringLength - 5, "ps.xml\0" );
			ntPrintf("Converting %s to %s...\n",*clumpString, psFileName);

			ntstd::String temp = *g_ShellOptions->m_obContentPlatform;
			temp += "\\";
			temp += *clumpString;

			ntstd::String temp2 = g_ShellOptions->m_contentNeutral;
			temp2 += "\\";
			temp2 += psFileName;

			temp += " ";
			temp += temp2;

			Util::GetFiosFilePath( temp.c_str(), acFileName );

			ntPrintf("Command line %s...\n", acFileName);
			int ret = spawnl( P_WAIT, "C:\\alienbrainWork\\HS\\content_pc\\ClumpToPSXML.exe", "C:\\alienbrainWork\\HS\\content_pc\\ClumpToPSXML.exe", acFileName );

			if( -1 == ret )
			{
				ntPrintf("Failed to call ClumpToPSXML.exe!...\n");
			}
#endif
			
			StaticLG* lg = NT_NEW StaticLG( p_entity->GetName(), p_entity );
			lg->Load();			
			lg->Activate();
			
				EntityCollisionFlag obFlag;
				obFlag.base = lg->GetCollisionFilterInfo();
				obFlag.flags.i_collide_with = CHARACTER_CONTROLLER_PLAYER_BIT | CHARACTER_CONTROLLER_ENEMY_BIT;
				lg->SetCollisionFilterInfo(obFlag.base);

				constructedSystem = NT_NEW_CHUNK(Mem::MC_ENTITY) System( p_entity, p_entity->GetName() );
				constructedSystem->AddGroup( lg );
			

		} else if (CHashedString(HASH_STRING_SOFT) == pcDefaultDynamics)//if( strcmp( pcDefaultDynamics, "Soft" ) == 0 )
		{
			ntAssert_p(0, ("Not implemented"));
		} else if (CHashedString(HASH_STRING_ANIMATED) == pcDefaultDynamics)//if( strcmp( pcDefaultDynamics, "Animated" ) == 0 )
		{
#ifdef AUTOMATIC_NEW_SYSTEM_CONVERSION
			CHashedString clumpString = p_entity->GetClumpString();
			strcpy( psFileName, *clumpString);
			int stringLength = strlen(psFileName);
			strcpy( psFileName + stringLength - 5, "ps.xml\0" );
			ntPrintf("Converting %s to %s...\n",*clumpString, psFileName);

			ntstd::String temp = *g_ShellOptions->m_obContentPlatform;
			temp += "\\";
			temp += *clumpString;

			ntstd::String temp2 = g_ShellOptions->m_contentNeutral;
			temp2 += "\\";
			temp2 += psFileName;

			temp += " ";
			temp += temp2;

			Util::GetFiosFilePath( temp.c_str(), acFileName );

			ntPrintf("Command line %s...\n", acFileName);
			int ret = spawnl( P_WAIT, "C:\\alienbrainWork\\HS\\content_pc\\ClumpToPSXML.exe", "C:\\alienbrainWork\\HS\\content_pc\\ClumpToPSXML.exe", acFileName );

			if( -1 == ret )
			{
				ntPrintf("Failed to call ClumpToPSXML.exe!...\n");
			}
#endif
			LogicGroup* lg = NT_NEW AnimatedLG( p_entity->GetName(), p_entity );
			lg->Load(); 
			lg->Activate();
			if( lg )
			{
				constructedSystem = NT_NEW_CHUNK(Mem::MC_ENTITY) System( p_entity, p_entity->GetName() );
				constructedSystem->AddGroup( lg );
			}
		} else if (CHashedString(HASH_STRING_RIGID) == pcDefaultDynamics)//if(strcmp(pcDefaultDynamics,"Rigid")==0)
		{
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
#ifdef AUTOMATIC_NEW_SYSTEM_CONVERSION
			CHashedString clumpString = p_entity->GetClumpString();
			strcpy( psFileName, *clumpString);
			int stringLength = strlen(psFileName);
			strcpy( psFileName + stringLength - 5, "ps.xml\0" );
			ntPrintf("Converting %s to %s...\n",*clumpString, psFileName);

			ntstd::String temp = *g_ShellOptions->m_obContentPlatform;
			temp += "\\";
			temp += *clumpString;

			ntstd::String temp2 = g_ShellOptions->m_contentNeutral;
			temp2 += "\\";
			temp2 += psFileName;

			temp += " ";
			temp += temp2;

			Util::GetFiosFilePath( temp.c_str(), acFileName );

			ntPrintf("Command line %s...\n", acFileName);
			int ret = spawnl( P_WAIT, "C:\\alienbrainWork\\HS\\content_pc\\ClumpToPSXML.exe", "C:\\alienbrainWork\\HS\\content_pc\\ClumpToPSXML.exe", acFileName );

			if( -1 == ret )
			{
				ntPrintf("Failed to call ClumpToPSXML.exe!...\n");
			}
#endif
			SingleRigidLG * lg = NT_NEW SingleRigidLG( p_entity->GetName(), p_entity );
			lg->Load();
			lg->Activate();
			
				constructedSystem = NT_NEW_CHUNK(Mem::MC_ENTITY) System( p_entity, p_entity->GetName() );
				constructedSystem->AddGroup( lg );
#endif
		} else if (CHashedString(HASH_STRING_RAGDOLL) == pcDefaultDynamics)//if( strcmp( pcDefaultDynamics, "Ragdoll" ) == 0 )
		{
			ntAssert( 0 );
		} else  if (CHashedString(HASH_STRING_DEFBYSCRIPT) == pcDefaultDynamics)
		{
		} 
		else 
		{
			// if default dynamics is not defined do nothing... only create system... 
			constructedSystem = NT_NEW_CHUNK(Mem::MC_ENTITY) System( p_entity, p_entity->GetName() );			

			
			/*
			// Hmmm big change: Do not load it directly to physics system but to LG instead.
			//SingleRigidLG * lg = NT_NEW SingleRigidLG( p_entity->GetName(), p_entity );
			//lg->Load();
			//lg->Activate();
			
			//constructedSystem = NT_NEW System( p_entity, p_entity->GetName() );
			//constructedSystem->AddGroup( lg );		

			constructedSystem = NT_NEW System( p_entity, p_entity->GetName() );
			constructedSystem->Load();

			ntstd::String psXMLFile = ClumpTools::AlterFilename( ntStr::GetString(p_entity->GetClumpString()) );

			if( File::Exists( psXMLFile.c_str() ) )
			{
				constructedSystem = NT_NEW System( p_entity, p_entity->GetName() );	
				constructedSystem->ReadFromXML( psXMLFile );
			}*/
		}

		return constructedSystem;
	}

	//----------------------------------------------------------
	//!
	//!	Update the collision filters. 
	//!
	//----------------------------------------------------------

	void System::UpdateCollisionFilter ()
	{
		ntstd::List<Element*>::iterator itElem = m_elementList.begin();
		while( itElem != m_elementList.end() )
		{
			Element* current = (*itElem);
			if( current->GetType() == Physics::Element::RIGID_BODY )
			{
				RigidBody* body = (RigidBody*) current;
				body->UpdateCollisionFilter();
				++ itElem;
			}
		}

		ntstd::List<LogicGroup*>::iterator itGroup = m_groupList.begin();
		while( itGroup != m_groupList.end() )
		{
			LogicGroup* current = (*itGroup);
			current->UpdateCollisionFilter();
			++ itGroup;
		}
	}

	//----------------------------------------------------------
	//!
	//!	Change the collision filter info. It NOT updates collision filter.
	//!
	//----------------------------------------------------------
	void System::SetCollisionFilterInfo(uint32_t info)
	{
		ntstd::List<Element*>::iterator itElem = m_elementList.begin();
		while( itElem != m_elementList.end() )
		{
			Element* current = (*itElem);
			if( current->GetType() == Physics::Element::RIGID_BODY )
			{
				RigidBody* body = (RigidBody*) current;
				body->SetCollisionFilterInfo(info);
				++ itElem;
			}
		}

		ntstd::List<LogicGroup*>::iterator itGroup = m_groupList.begin();
		while( itGroup != m_groupList.end() )
		{
			LogicGroup* current = (*itGroup);
			current->SetCollisionFilterInfo(info);
			++ itGroup;
		}
	}

	//----------------------------------------------------------
	//!
	//!	Change the collision filter info. It NOT updates collision filter.
	//!
	//! Just for testing. Will be removed
	//!
	//----------------------------------------------------------	
	void System::SetCollisionFilterInfoEx(int field, uint32_t info)
	{
		switch (field)
		{
		case I_AM:
			{
				EntityCollisionFlag obFlag;
				obFlag.base = GetCollisionFilterInfo();
				obFlag.flags.i_am = info;
				SetCollisionFilterInfo(obFlag.base);
				return;
			}
		case I_COLLIDE_WITH:
			{
				EntityCollisionFlag obFlag;
				obFlag.base = GetCollisionFilterInfo();
				obFlag.flags.i_collide_with = info;
				SetCollisionFilterInfo(obFlag.base);
				return;
			}
		case RAGDOLL_MATERIAL:
			{
				RagdollCollisionFlag obFlag;
				obFlag.base = GetCollisionFilterInfo();
				obFlag.flags.ragdoll_material = info;
				SetCollisionFilterInfo(obFlag.base);
				return;
			}
		case KO_STATES:
			{
				AIWallCollisionFlag obFlag;
				obFlag.base = GetCollisionFilterInfo();
				obFlag.flags.not_collide_with_KO_states_important = info;
				SetCollisionFilterInfo(obFlag.base);
				return;
			}
		case RAYCAST_MATERIAL:
			{
				AIWallCollisionFlag obFlag;
				obFlag.base = GetCollisionFilterInfo();
				obFlag.flags.raycast_material = info;
				SetCollisionFilterInfo(obFlag.base);
				return;
			}
		}
	}

	
	//----------------------------------------------------------
	//!
	//!	Return the collisoin filter info of the first element.
	//!
	//----------------------------------------------------------
	uint32_t System::GetCollisionFilterInfo() const
	{
		ntstd::List<Element*>::const_iterator itElem = m_elementList.begin();
		while( itElem != m_elementList.end() )
		{
			Element* current = (*itElem);
			if( current->GetType() == Physics::Element::RIGID_BODY )
			{
				RigidBody* body = (RigidBody*) current;
				return body->GetCollisionFilterInfo();				
			}
			++itElem;
		}

		ntstd::List<LogicGroup*>::const_iterator itGroup = m_groupList.begin();
		while( itGroup != m_groupList.end() )
		{
			LogicGroup* current = (*itGroup);
			return current->GetCollisionFilterInfo();			
		}
		return 0;
	}


	CDirection System::GetLinearVelocity( )
	{
		ntstd::List<LogicGroup*>::iterator itGroup = m_groupList.begin();
		while( itGroup != m_groupList.end() )
		{
			LogicGroup* current = (*itGroup);
			
			if( current->GetType() == LogicGroup::ADVANCED_CHARACTER_CONTROLLER )
				return current->GetLinearVelocity();

			if( current->GetType() == LogicGroup::CHARACTER_CONTROLLER_LG )
				return current->GetLinearVelocity();

			if( current->GetType() == LogicGroup::COMPOUND_RIGID_LG )
				return current->GetLinearVelocity();

			if( current->GetType() == LogicGroup::RAGDOLL_LG )
				return current->GetLinearVelocity();

			if( current->GetType() == LogicGroup::SINGLE_RIGID_BODY_LG )
				return current->GetLinearVelocity();

			if( current->GetType() == LogicGroup::SPEAR_LG )
				return current->GetLinearVelocity();

			if( current->GetType() == LogicGroup::PROJECTILE_LG )
				return current->GetLinearVelocity();

			++ itGroup;
		}

		return LogicGroup::GetLinearVelocity();
	}

	CDirection System::GetAngularVelocity( )
	{
		ntstd::List<LogicGroup*>::iterator itGroup = m_groupList.begin();
		while( itGroup != m_groupList.end() )
		{
			LogicGroup* current = (*itGroup);
			
			if( current->GetType() == LogicGroup::COMPOUND_RIGID_LG )
				return current->GetAngularVelocity();

			if( current->GetType() == LogicGroup::SINGLE_RIGID_BODY_LG )
				return current->GetAngularVelocity();

			if( current->GetType() == LogicGroup::SPEAR_LG )
				return current->GetAngularVelocity();

			++ itGroup;
		}

		return LogicGroup::GetAngularVelocity();
	}

	void System::SetLinearVelocity( const CDirection& p_vel )
	{
		ntstd::List<LogicGroup*>::iterator itGroup = m_groupList.begin();
		while( itGroup != m_groupList.end() )
		{
			LogicGroup* current = (*itGroup);
			
			current->SetLinearVelocity( p_vel );

			++ itGroup;
		}

		LogicGroup::SetLinearVelocity( p_vel );
	}

	void System::SetAngularVelocity( const CDirection& p_vel )
	{
		ntstd::List<LogicGroup*>::iterator itGroup = m_groupList.begin();
		while( itGroup != m_groupList.end() )
		{
			LogicGroup* current = (*itGroup);
			
			current->SetAngularVelocity( p_vel );

			++ itGroup;
		}

		LogicGroup::SetAngularVelocity( p_vel );
	}


	void System::ApplyLinearImpulse( const CDirection& p_vel )
	{
		ntstd::List<LogicGroup*>::iterator itGroup = m_groupList.begin();
		while( itGroup != m_groupList.end() )
		{
			LogicGroup* current = (*itGroup);
			
			current->ApplyLinearImpulse( p_vel );

			++ itGroup;
		}

		LogicGroup::ApplyLinearImpulse( p_vel );
	}

	void System::ApplyLocalisedLinearImpulse( const CDirection& p_vel, const CVector& p_point )
	{
		ntstd::List<LogicGroup*>::iterator itGroup = m_groupList.begin();
		while( itGroup != m_groupList.end() )
		{
			LogicGroup* current = (*itGroup);
			
			current->ApplyLocalisedLinearImpulse( p_vel, p_point );

			++ itGroup;
		}

		LogicGroup::ApplyLocalisedLinearImpulse( p_vel, p_point );
	}


	void System::ApplyAngularImpulse( const CDirection& p_vel )
	{
		ntstd::List<LogicGroup*>::iterator itGroup = m_groupList.begin();
		while( itGroup != m_groupList.end() )
		{
			LogicGroup* current = (*itGroup);
			
			current->ApplyAngularImpulse( p_vel );

			++ itGroup;
		}

		LogicGroup::ApplyAngularImpulse( p_vel );
	}

	void System::Pause( bool bPause )
	{
		ntstd::List<LogicGroup*>::iterator itGroup = m_groupList.begin();
		while( itGroup != m_groupList.end() )
		{
			LogicGroup* current = (*itGroup);
			current->Pause( bPause );

			++ itGroup;
		}
	}

	void System::Debug_RenderAllCollisionInfo ()
	{
		ntstd::List<LogicGroup*>::iterator itGroup = m_groupList.begin();
		while( itGroup != m_groupList.end() )
		{
			LogicGroup* current = (*itGroup);
			if( current->IsActive() )
			{
				current->Debug_RenderCollisionInfo();
			}

			++ itGroup;
		}

		ntstd::List<Element*>::iterator itElems = m_elementList.begin();
		while( itElems != m_elementList.end() )
		{
			Element* current = (*itElems);
			current->Debug_RenderCollisionInfo();

			++itElems;
		}
	}

	

	//------------------------------------------------------------------------------------------------------------------------------------

	void DebugCollisionTools::RenderCapsuleShape (const CMatrix& obWorldMatrix,const hkCapsuleShape* pobShape)
	{
#ifndef _GOLD_MASTER
		// Our capsule render primitive doesn't seem to get on with the havok def of what a capsule is, so for now I've put in an ugly low-res line hacky one - DGF

		hkVector4 obVert1(pobShape->getVertex(0));
		hkVector4 obVert2(pobShape->getVertex(1));

		CPoint obFrom(obVert1(0),obVert1(1),obVert1(2));
		CPoint obTo(obVert2(0),obVert2(1),obVert2(2));
		obFrom = obFrom * obWorldMatrix;
		obTo = obTo * obWorldMatrix;
	
		float fRadius=pobShape->getRadius();

		CDirection obDirection(obTo - obFrom);
		CDirection obNormalisedDirection(obDirection);
		obNormalisedDirection.Normalise();

		// Havok capsule are the distance between the 2 vertices, plus the radi of the 2 spheres which are then centred on those vertices
		//float fLength = obDirection.Length() + fRadius + fRadius;

		CPoint obEndPoint1 = obFrom;
		obEndPoint1 -= obNormalisedDirection * fRadius;
		CPoint obEndPoint2 = obTo;
		obEndPoint2 += obNormalisedDirection * fRadius;
		g_VisualDebug->RenderLine(obEndPoint1,obEndPoint2,DC_CYAN);

		CPoint obWidthPoint1 = obWorldMatrix.GetTranslation();
		obWidthPoint1 -= obWorldMatrix.GetXAxis() * fRadius;
		CPoint obWidthPoint2 = obWorldMatrix.GetTranslation();
		obWidthPoint2 += obWorldMatrix.GetXAxis() * fRadius;
		CPoint obWidthPoint3 = obWorldMatrix.GetTranslation();
		obWidthPoint3 -= obWorldMatrix.GetZAxis() * fRadius;
		CPoint obWidthPoint4 = obWorldMatrix.GetTranslation();
		obWidthPoint4 += obWorldMatrix.GetZAxis() * fRadius;
		CPoint obWidthPoint5 = obFrom;
		obWidthPoint5 -= obWorldMatrix.GetXAxis() * fRadius;
		CPoint obWidthPoint6 = obFrom;
		obWidthPoint6 += obWorldMatrix.GetXAxis() * fRadius;
		CPoint obWidthPoint7 = obFrom;
		obWidthPoint7 -= obWorldMatrix.GetZAxis() * fRadius;
		CPoint obWidthPoint8 = obFrom;
		obWidthPoint8 += obWorldMatrix.GetZAxis() * fRadius;
		CPoint obWidthPoint9 = obTo;
		obWidthPoint9 -= obWorldMatrix.GetXAxis() * fRadius;
		CPoint obWidthPoint10 = obTo;
		obWidthPoint10 += obWorldMatrix.GetXAxis() * fRadius;
		CPoint obWidthPoint11 = obTo;
		obWidthPoint11 -= obWorldMatrix.GetZAxis() * fRadius;
		CPoint obWidthPoint12 = obTo;
		obWidthPoint12 += obWorldMatrix.GetZAxis() * fRadius;

		g_VisualDebug->RenderLine(obWidthPoint1,obWidthPoint2,DC_CYAN);
		g_VisualDebug->RenderLine(obWidthPoint3,obWidthPoint4,DC_CYAN);
		g_VisualDebug->RenderLine(obWidthPoint5,obWidthPoint6,DC_CYAN);
		g_VisualDebug->RenderLine(obWidthPoint7,obWidthPoint8,DC_CYAN);
		g_VisualDebug->RenderLine(obWidthPoint9,obWidthPoint10,DC_CYAN);
		g_VisualDebug->RenderLine(obWidthPoint11,obWidthPoint12,DC_CYAN);
		g_VisualDebug->RenderLine(obWidthPoint1,obWidthPoint3,DC_CYAN);
		g_VisualDebug->RenderLine(obWidthPoint2,obWidthPoint4,DC_CYAN);
		g_VisualDebug->RenderLine(obWidthPoint5,obWidthPoint7,DC_CYAN);
		g_VisualDebug->RenderLine(obWidthPoint8,obWidthPoint8,DC_CYAN);
		g_VisualDebug->RenderLine(obWidthPoint9,obWidthPoint11,DC_CYAN);
		g_VisualDebug->RenderLine(obWidthPoint10,obWidthPoint12,DC_CYAN);
		g_VisualDebug->RenderLine(obWidthPoint1,obWidthPoint4,DC_CYAN);
		g_VisualDebug->RenderLine(obWidthPoint2,obWidthPoint3,DC_CYAN);
		g_VisualDebug->RenderLine(obWidthPoint5,obWidthPoint8,DC_CYAN);
		g_VisualDebug->RenderLine(obWidthPoint6,obWidthPoint7,DC_CYAN);
		g_VisualDebug->RenderLine(obWidthPoint9,obWidthPoint12,DC_CYAN);
		g_VisualDebug->RenderLine(obWidthPoint10,obWidthPoint11,DC_CYAN);
		g_VisualDebug->RenderLine(obWidthPoint1,obWidthPoint5,DC_CYAN);
		g_VisualDebug->RenderLine(obWidthPoint2,obWidthPoint6,DC_CYAN);
		g_VisualDebug->RenderLine(obWidthPoint3,obWidthPoint7,DC_CYAN);
		g_VisualDebug->RenderLine(obWidthPoint4,obWidthPoint8,DC_CYAN);
		g_VisualDebug->RenderLine(obWidthPoint5,obWidthPoint9,DC_CYAN);
		g_VisualDebug->RenderLine(obWidthPoint6,obWidthPoint10,DC_CYAN);
		g_VisualDebug->RenderLine(obWidthPoint7,obWidthPoint11,DC_CYAN);
		g_VisualDebug->RenderLine(obWidthPoint8,obWidthPoint12,DC_CYAN);
		g_VisualDebug->RenderLine(obWidthPoint5,obEndPoint1,DC_CYAN);
		g_VisualDebug->RenderLine(obWidthPoint6,obEndPoint1,DC_CYAN);
		g_VisualDebug->RenderLine(obWidthPoint7,obEndPoint1,DC_CYAN);
		g_VisualDebug->RenderLine(obWidthPoint8,obEndPoint1,DC_CYAN);
		g_VisualDebug->RenderLine(obWidthPoint9,obEndPoint2,DC_CYAN);
		g_VisualDebug->RenderLine(obWidthPoint10,obEndPoint2,DC_CYAN);
		g_VisualDebug->RenderLine(obWidthPoint11,obEndPoint2,DC_CYAN);
		g_VisualDebug->RenderLine(obWidthPoint12,obEndPoint2,DC_CYAN);
		
		// This is broken, so I'm just using lines and points right now
		/*CMatrix obMatrix;
		CCamUtil::CreateFromPoints(obMatrix,obFrom,obTo);
		CQuat obOrientation(obMatrix);
		g_VisualDebug->RenderCapsule(obOrientation,obWorldMatrix.GetTranslation(),fRadius,fLength,0xff00ffff,0x00001000);*/
#endif
	}

	void DebugCollisionTools::RenderCylinderShape (const CMatrix& obWorldMatrix,const hkCylinderShape* pobShape)
	{
#ifndef _GOLD_MASTER
		// Our capsule render primitive doesn't seem to get on with the havok def of what a capsule is, so for now I've put in an ugly low-res line hacky one - DGF

		hkVector4 obVert1(pobShape->getVertex(0));
		hkVector4 obVert2(pobShape->getVertex(1));

		CPoint obFrom(obVert1(0),obVert1(1),obVert1(2));
		CPoint obTo(obVert2(0),obVert2(1),obVert2(2));
		obFrom = obFrom * obWorldMatrix;
		obTo = obTo * obWorldMatrix;
	
		float fRadius = pobShape->getCylinderRadius();

		CDirection obDirection(obTo - obFrom);
		CDirection obNormalisedDirection(obDirection);
		obNormalisedDirection.Normalise();

		// Havok capsule are the distance between the 2 vertices, plus the radi of the 2 spheres which are then centred on those vertices
		//float fLength = obDirection.Length() + fRadius + fRadius;

		CPoint obEndPoint1 = obFrom;
		CPoint obEndPoint2 = obTo;
		g_VisualDebug->RenderLine(obEndPoint1,obEndPoint2,DC_CYAN);

		CPoint obWidthPoint1 = obWorldMatrix.GetTranslation();
		obWidthPoint1 -= obWorldMatrix.GetXAxis() * fRadius;
		CPoint obWidthPoint2 = obWorldMatrix.GetTranslation();
		obWidthPoint2 += obWorldMatrix.GetXAxis() * fRadius;
		CPoint obWidthPoint3 = obWorldMatrix.GetTranslation();
		obWidthPoint3 -= obWorldMatrix.GetZAxis() * fRadius;
		CPoint obWidthPoint4 = obWorldMatrix.GetTranslation();
		obWidthPoint4 += obWorldMatrix.GetZAxis() * fRadius;
		CPoint obWidthPoint5 = obFrom;
		obWidthPoint5 -= obWorldMatrix.GetXAxis() * fRadius;
		CPoint obWidthPoint6 = obFrom;
		obWidthPoint6 += obWorldMatrix.GetXAxis() * fRadius;
		CPoint obWidthPoint7 = obFrom;
		obWidthPoint7 -= obWorldMatrix.GetZAxis() * fRadius;
		CPoint obWidthPoint8 = obFrom;
		obWidthPoint8 += obWorldMatrix.GetZAxis() * fRadius;
		CPoint obWidthPoint9 = obTo;
		obWidthPoint9 -= obWorldMatrix.GetXAxis() * fRadius;
		CPoint obWidthPoint10 = obTo;
		obWidthPoint10 += obWorldMatrix.GetXAxis() * fRadius;
		CPoint obWidthPoint11 = obTo;
		obWidthPoint11 -= obWorldMatrix.GetZAxis() * fRadius;
		CPoint obWidthPoint12 = obTo;
		obWidthPoint12 += obWorldMatrix.GetZAxis() * fRadius;

		g_VisualDebug->RenderLine(obWidthPoint1,obWidthPoint2,DC_CYAN);
		g_VisualDebug->RenderLine(obWidthPoint3,obWidthPoint4,DC_CYAN);
		g_VisualDebug->RenderLine(obWidthPoint5,obWidthPoint6,DC_CYAN);
		g_VisualDebug->RenderLine(obWidthPoint7,obWidthPoint8,DC_CYAN);
		g_VisualDebug->RenderLine(obWidthPoint9,obWidthPoint10,DC_CYAN);
		g_VisualDebug->RenderLine(obWidthPoint11,obWidthPoint12,DC_CYAN);
		g_VisualDebug->RenderLine(obWidthPoint1,obWidthPoint3,DC_CYAN);
		g_VisualDebug->RenderLine(obWidthPoint2,obWidthPoint4,DC_CYAN);
		g_VisualDebug->RenderLine(obWidthPoint5,obWidthPoint7,DC_CYAN);
		g_VisualDebug->RenderLine(obWidthPoint9,obWidthPoint11,DC_CYAN);
		g_VisualDebug->RenderLine(obWidthPoint10,obWidthPoint12,DC_CYAN);
		g_VisualDebug->RenderLine(obWidthPoint1,obWidthPoint4,DC_CYAN);
		g_VisualDebug->RenderLine(obWidthPoint2,obWidthPoint3,DC_CYAN);
		g_VisualDebug->RenderLine(obWidthPoint5,obWidthPoint8,DC_CYAN);
		g_VisualDebug->RenderLine(obWidthPoint6,obWidthPoint7,DC_CYAN);
		g_VisualDebug->RenderLine(obWidthPoint9,obWidthPoint12,DC_CYAN);
		g_VisualDebug->RenderLine(obWidthPoint10,obWidthPoint11,DC_CYAN);
		g_VisualDebug->RenderLine(obWidthPoint1,obWidthPoint5,DC_CYAN);
		g_VisualDebug->RenderLine(obWidthPoint2,obWidthPoint6,DC_CYAN);
		g_VisualDebug->RenderLine(obWidthPoint3,obWidthPoint7,DC_CYAN);
		g_VisualDebug->RenderLine(obWidthPoint4,obWidthPoint8,DC_CYAN);
		g_VisualDebug->RenderLine(obWidthPoint5,obWidthPoint9,DC_CYAN);
		g_VisualDebug->RenderLine(obWidthPoint6,obWidthPoint10,DC_CYAN);
		g_VisualDebug->RenderLine(obWidthPoint7,obWidthPoint11,DC_CYAN);
		g_VisualDebug->RenderLine(obWidthPoint8,obWidthPoint12,DC_CYAN);
		g_VisualDebug->RenderLine(obWidthPoint5,obEndPoint1,DC_CYAN);
		g_VisualDebug->RenderLine(obWidthPoint6,obEndPoint1,DC_CYAN);
		g_VisualDebug->RenderLine(obWidthPoint7,obEndPoint1,DC_CYAN);
		g_VisualDebug->RenderLine(obWidthPoint8,obEndPoint1,DC_CYAN);
		g_VisualDebug->RenderLine(obWidthPoint9,obEndPoint2,DC_CYAN);
		g_VisualDebug->RenderLine(obWidthPoint10,obEndPoint2,DC_CYAN);
		g_VisualDebug->RenderLine(obWidthPoint11,obEndPoint2,DC_CYAN);
		g_VisualDebug->RenderLine(obWidthPoint12,obEndPoint2,DC_CYAN);

		// This is broken, so I'm just using lines and points right now
		/*CMatrix obMatrix;
		CCamUtil::CreateFromPoints(obMatrix,obFrom,obTo);
		CQuat obOrientation(obMatrix);
		g_VisualDebug->RenderCapsule(obOrientation,obWorldMatrix.GetTranslation(),fRadius,fLength,0xff00ffff,0x00001000);*/
#endif
	}

	void DebugCollisionTools::RenderBoxShape (const CMatrix& obWorldMatrix,const hkBoxShape* pobShape)
	{
#ifndef _GOLD_MASTER
		CDirection obHalfExtents(pobShape->getHalfExtents()(0),pobShape->getHalfExtents()(1),pobShape->getHalfExtents()(2));

		g_VisualDebug->RenderOBB(obWorldMatrix,obHalfExtents,0xff00ffff,0x00001000);
#endif
	}

	void DebugCollisionTools::RenderSphereShape (const CMatrix& obWorldMatrix,const hkSphereShape* pobShape)
	{
#ifndef _GOLD_MASTER
		g_VisualDebug->RenderSphere(CQuat(obWorldMatrix),obWorldMatrix.GetTranslation(),pobShape->getRadius(),0xff00ffff,0x00001000);
#endif
	}

	void DebugCollisionTools::RenderMeshShape(const CMatrix& obWorldMatrix,const hsMeshShape * pobShape)
	{
		UNUSED( obWorldMatrix );
		UNUSED( pobShape );
#		ifndef _RELEASE
			const hkMeshShape::Subpart& part = pobShape->getSubPart(0);
			g_VisualDebug->RenderPrimitive( const_cast<hsMeshShape *>(pobShape)->GetDebugRenderTriangleList(), part.m_numTriangles * 3,obWorldMatrix,0x8f00ff8f,0x00000100);
#		endif
	}

	void DebugCollisionTools::RenderConvexVerticesShape (const CMatrix& obWorldMatrix,const hkConvexVerticesShape* pobShape)
	{
#ifndef _GOLD_MASTER
		hkArray<hkVector4> vertices;
		pobShape->getOriginalVertices(vertices);

		for(int i = 0; i < vertices.getSize(); i ++)
		{	
			CPoint pt0 = MathsTools::hkVectorToCPoint(vertices[i]) * obWorldMatrix;
			for(int j = i + 1; j < vertices.getSize(); j ++)
			{
				CPoint pt1 = MathsTools::hkVectorToCPoint(vertices[j]) * obWorldMatrix;

				g_VisualDebug->RenderLine(pt0, pt1, 0xff00ffff);
			}		
		}
#endif
	}

	void DebugCollisionTools::RenderTransformShape (const CMatrix& obWorldMatrix,const hkTransformShape* pobShape)
	{
		hkTransform obTransform(pobShape->getTransform());

		hkQuaternion obRotation(obTransform.getRotation());

		CMatrix obLocalMatrix(
			CQuat(obRotation(0),obRotation(1),obRotation(2),obRotation(3)),
			CPoint(obTransform.getTranslation()(0),obTransform.getTranslation()(1),obTransform.getTranslation()(2)));

		CMatrix obChildWorldMatrix = obLocalMatrix * obWorldMatrix;

		const hkShape* pobChildShape=pobShape->getChildShape();

		RenderShape(obChildWorldMatrix, pobChildShape);	
	}

	void DebugCollisionTools::RenderConvexTransformShape (const CMatrix& obWorldMatrix,const hkConvexTransformShape* pobShape)
	{
		hkTransform obTransform(pobShape->getTransform());

		hkQuaternion obRotation(obTransform.getRotation());

		CMatrix obLocalMatrix(
			CQuat(obRotation(0),obRotation(1),obRotation(2),obRotation(3)),
			CPoint(obTransform.getTranslation()(0),obTransform.getTranslation()(1),obTransform.getTranslation()(2)));

		CMatrix obChildWorldMatrix = obLocalMatrix * obWorldMatrix;

		const hkShape* pobChildShape=pobShape->getChildShape();

		RenderShape(obChildWorldMatrix, pobChildShape);
	}

	void DebugCollisionTools::RenderListShape (const CMatrix& obWorldMatrix,const hkListShape* pobShape)
	{
		hkShapeCollection::ShapeBuffer buffer;

		for (hkShapeKey key = pobShape->getFirstKey(); key != HK_INVALID_SHAPE_KEY; key = pobShape->getNextKey( key ) )
		{
			const hkShape* pobChildShape=pobShape->getChildShape(key,buffer);

			RenderShape(obWorldMatrix,pobChildShape);
		}
	}

	void DebugCollisionTools::RenderShape (const CMatrix& obWorldMatrix,const hkShape* pobShape)
	{
		switch(pobShape->getType())
		{
			case HK_SHAPE_CONVEX_LIST:
			case HK_SHAPE_LIST:
			{
				RenderListShape(obWorldMatrix,(hkListShape*)pobShape);
				break;
			}

			case HK_SHAPE_CAPSULE:
			{
				RenderCapsuleShape(obWorldMatrix,(hkCapsuleShape*)pobShape);
				break;
			}

			case HK_SHAPE_CYLINDER:
			{
				RenderCylinderShape(obWorldMatrix,(hkCylinderShape*)pobShape);
				break;
			}

			case HK_SHAPE_BOX:
			{
				RenderBoxShape(obWorldMatrix,(hkBoxShape*)pobShape);
				break;
			}

			case HK_SHAPE_SPHERE:
			{
				RenderSphereShape(obWorldMatrix,(hkSphereShape*)pobShape);
				break;
			}

			case HK_SHAPE_TRANSFORM:
			{
				RenderTransformShape(obWorldMatrix,(hkTransformShape*)pobShape);
				break;
			}

			case HK_SHAPE_CONVEX_TRANSFORM:
			{
				RenderConvexTransformShape(obWorldMatrix,(hkConvexTransformShape*)pobShape);
				break;
			}

			case HK_SHAPE_TRIANGLE_COLLECTION:
			{
				RenderMeshShape(obWorldMatrix,(hsMeshShape*)pobShape);
				break;
			}

	                case HK_SHAPE_CONVEX_VERTICES:
			{
				RenderConvexVerticesShape(obWorldMatrix,(hkConvexVerticesShape*)pobShape);
				break;
			}

			case HK_SHAPE_MOPP:
			{
				RenderShape(obWorldMatrix,((hkMoppBvTreeShape *)pobShape)->getShapeCollection());
				break;
			}


			default:
			{
				break;
			}
		}
	}

	void DebugCollisionTools::RenderConstraint(const hkConstraintInstance& constInst)
	{
#ifndef _GOLD_MASTER
		switch (constInst.getData()->getType())
		{
		case hkConstraintData::CONSTRAINT_TYPE_LIMITEDHINGE:
			{
				hkLimitedHingeConstraintData * constrData = static_cast<hkLimitedHingeConstraintData *>(constInst.getData());
				
				hkConstraintData::ConstraintInfo infoOut;
				constrData->getConstraintInfo( infoOut ); 

				hkSetLocalTransformsConstraintAtom * atom = static_cast<hkSetLocalTransformsConstraintAtom*>(infoOut.m_atoms);

				// show pivot points
				float time = CPhysicsWorld::Get().GetHavokWorldP()->getCurrentTime();
				hkTransform transA, transB;
				constInst.getRigidBodyA()->approxTransformAt(time,transA);
				constInst.getRigidBodyB()->approxTransformAt(time,transB);

				hkTransform pivotA, pivotB; 
				pivotA.setMul(transA,atom->m_transformA);
				pivotB.setMul(transB,atom->m_transformB);

				hkTransform pivotA_c, pivotB_c; 
				pivotA_c.getColumn(0) = pivotA.getColumn(1);
				pivotA_c.getColumn(1) = pivotA.getColumn(2);
				pivotA_c.getColumn(2) = pivotA.getColumn(0);
				pivotA_c.getColumn(3) = pivotA.getColumn(3);

				pivotB_c.getColumn(0) = pivotA.getColumn(1);
				pivotB_c.getColumn(1) = pivotA.getColumn(2);
				pivotB_c.getColumn(2) = pivotA.getColumn(0);
				pivotB_c.getColumn(3) = pivotA.getColumn(3);
								
				g_VisualDebug->RenderCapsule(MathsTools::hkQuaternionToCQuat(hkQuaternion(pivotA_c.getRotation())), MathsTools::hkVectorToCPoint(pivotA.getTranslation()), 0.02f, 24.0f, DC_RED,0); //DPF_WIREFRAME);
				g_VisualDebug->RenderCapsule(MathsTools::hkQuaternionToCQuat(hkQuaternion(pivotB_c.getRotation())), MathsTools::hkVectorToCPoint(pivotB.getTranslation()), 0.02f, 24.0f, DC_WHITE,0);//DPF_WIREFRAME);

				// get max, min angle and actual angle... 
				float minAngle = constrData->getMinAngularLimit();
				float maxAngle = constrData->getMaxAngularLimit();

				//float coef = 0.1f; 
				
				hkTransform transAngle;
				transAngle.getColumn(0) = pivotA.getColumn(2);
				transAngle.getColumn(1) = pivotA.getColumn(0);
				transAngle.getColumn(2) = pivotA.getColumn(1);
				transAngle.getColumn(3) = pivotA.getColumn(3);

				hkVector4 angleAxe = transAngle.getColumn(2); 
				hkVector4 center = transAngle.getTranslation();
				center.addMul4((hkSimdRealParameter) 0.2f, angleAxe);

				hkLimitedHingeConstraintData::Runtime * runtimeData = static_cast<hkLimitedHingeConstraintData::Runtime *>(constInst.getRuntime());
				float currentAngle = runtimeData->getCurrentPos();				
				g_VisualDebug->RenderCapsule(MathsTools::hkQuaternionToCQuat(hkQuaternion(transAngle.getRotation())), MathsTools::hkVectorToCPoint(center), 0.01f, (maxAngle - minAngle) * 10, DC_WHITE,0);//DPF_WIREFRAME);

				// find current pos for ball
				float angleToLenght = 0.02f  * 10.0f;
				float currAngleRel = (currentAngle - minAngle) / (maxAngle - minAngle) - 0.5f;
				center.addMul4(currAngleRel * angleToLenght, angleAxe);

				g_VisualDebug->Printf3D(  MathsTools::hkVectorToCPoint(center), DC_WHITE, 0, "%lf", currentAngle * 180/3.1415f );
				g_VisualDebug->RenderSphere(  MathsTools::hkQuaternionToCQuat(hkQuaternion(transAngle.getRotation())), MathsTools::hkVectorToCPoint(center), 0.02f, DC_GREEN, 0);

			}
		}
#endif
	}

	void DebugCollisionTools::GetMotionType (const hkRigidBody* pobRigidBody,char* pcOutput)
	{
		switch(pobRigidBody->getMotionType())
		{
			case hkMotion::MOTION_DYNAMIC:
				strcpy(pcOutput,"MOTION_DYNAMIC");
				break;
			case hkMotion::MOTION_SPHERE_INERTIA:
				strcpy(pcOutput,"MOTION_SPHERE_INERTIA");
				break;
			case hkMotion::MOTION_STABILIZED_SPHERE_INERTIA:
				strcpy(pcOutput,"MOTION_STABILIZED_SPHERE_INERTIA");
				break;
			case hkMotion::MOTION_BOX_INERTIA:
				strcpy(pcOutput,"MOTION_BOX_INERTIA");
				break;
			case hkMotion::MOTION_STABILIZED_BOX_INERTIA:
				strcpy(pcOutput,"MOTION_STABILIZED_BOX_INERTIA");
				break;
			case hkMotion::MOTION_KEYFRAMED:
				strcpy(pcOutput,"MOTION_KEYFRAMED");
				break;
			case hkMotion::MOTION_FIXED:
				strcpy(pcOutput,"MOTION_FIXED");
				break;
			case hkMotion::MOTION_THIN_BOX_INERTIA:
				strcpy(pcOutput,"MOTION_THIN_BOX_INERTIA");
				break;
			default:
				strcpy(pcOutput,"MOTION_INVALID");
				break;
		}
	}

	void DebugCollisionTools::RenderCollisionFlags (const CPoint& obPosition,const Physics::EntityCollisionFlag& obFlag,const Physics::FilterExceptionFlag& obException)
	{
#ifndef _GOLD_MASTER

		char acIAm [16];
		char acICollideWith [32]="";

		if (obFlag.flags.i_am & Physics::TRIGGER_VOLUME_BIT)				strcpy(acIAm, "TV ");
		if (obFlag.flags.i_am & Physics::CHARACTER_CONTROLLER_PLAYER_BIT)	strcpy(acIAm, "PCC");
		if (obFlag.flags.i_am & Physics::CHARACTER_CONTROLLER_ENEMY_BIT)	strcpy(acIAm, "ECC");
		if (obFlag.flags.i_am & Physics::SMALL_INTERACTABLE_BIT)			strcpy(acIAm, "SI ");
		if (obFlag.flags.i_am & Physics::LARGE_INTERACTABLE_BIT)			strcpy(acIAm, "LI ");		
		if (obFlag.flags.i_am & Physics::RAGDOLL_BIT)						strcpy(acIAm, "R  ");
		if (obFlag.flags.i_am & Physics::AI_WALL_BIT)						strcpy(acIAm, "AIW");
		if (obFlag.flags.i_am & Physics::RIGID_PROJECTILE_BIT)				strcpy(acIAm, "P  ");

		if (obFlag.flags.i_collide_with & Physics::TRIGGER_VOLUME_BIT)				strcat(acICollideWith, "TV  ");
		if (obFlag.flags.i_collide_with & Physics::CHARACTER_CONTROLLER_PLAYER_BIT)	strcat(acICollideWith, "PCC ");
		if (obFlag.flags.i_collide_with & Physics::CHARACTER_CONTROLLER_ENEMY_BIT)	strcat(acICollideWith, "ECC ");
		if (obFlag.flags.i_collide_with & Physics::SMALL_INTERACTABLE_BIT)			strcat(acICollideWith, "SI  ");
		if (obFlag.flags.i_collide_with & Physics::LARGE_INTERACTABLE_BIT)			strcat(acICollideWith, "LI  ");		
		if (obFlag.flags.i_collide_with & Physics::RAGDOLL_BIT)						strcat(acICollideWith, "R   ");
		if (obFlag.flags.i_collide_with & Physics::AI_WALL_BIT)						strcat(acICollideWith, "AIW ");
		if (obFlag.flags.i_collide_with & Physics::RIGID_PROJECTILE_BIT)			strcat(acICollideWith, "P   ");

#ifndef JUST_COLLISION_RENDER
		//g_VisualDebug->Printf2D(fX,fY, 0xff00ffff, 0, "%s -> %s",acIAm,acICollideWith);
		g_VisualDebug->Printf3D(obPosition,0.0f,-20.0f,0xff00ffff,DTF_ALIGN_HCENTRE,"%s -> %s",acIAm,acICollideWith);
#endif

		if (obException.flags.exception_set!=0)
		{
			char acExceptions [128]="";

			if (obException.flags.exception_set & Physics::ALWAYS_RETURN_TRUE_BIT)		strcat(acExceptions, "Always True,");
			if (obException.flags.exception_set & Physics::ALWAYS_RETURN_FALSE_BIT)		strcat(acExceptions, "Always False,");
			if (obException.flags.exception_set & Physics::IGNORE_ENTITY_PTR_BIT)		strcat(acExceptions, "Ignore Ent Ptr,");
			if (obException.flags.exception_set & Physics::IGNORE_ENTITY_INTERACT_BIT)	strcat(acExceptions, "Ignore Int,");
			if (obException.flags.exception_set & Physics::IGNORE_ENTITY_FIGHT_BIT)		strcat(acExceptions, "Ignore Fight,");
			if (obException.flags.exception_set & Physics::CC_AND_RAGDOLL_ALWAYS_BIT)	strcat(acExceptions, "CC & Ragdoll Always,");
			if (obException.flags.exception_set & Physics::CC_AND_RAGDOLL_NEVER_BIT)	strcat(acExceptions, "CC & Ragdoll Never,");
			if (obException.flags.exception_set & Physics::IGNORE_FIXED_GEOM)			strcat(acExceptions, "Ignore Fixed,");
			if (obException.flags.exception_set & Physics::ONLY_FIXED_GEOM)				strcat(acExceptions, "Only Fixed,");
			if (obException.flags.exception_set & Physics::IGNORE_CCs)					strcat(acExceptions, "Ignore CCs,");
			if (obException.flags.exception_set & Physics::COLLIDE_WITH_PLAYER_ONLY)	strcat(acExceptions, "Player Only,");
			if (obException.flags.exception_set & Physics::COLLIDE_WITH_CCs_ONLY)		strcat(acExceptions, "CCs Only,");
			if (obException.flags.exception_set & Physics::COLLIDE_WITH_NMEs_ONLY)		strcat(acExceptions, "NMEs Only");

#ifndef JUST_COLLISION_RENDER
			//g_VisualDebug->Printf2D(fX,fY, 0xff00ffff, 0, "  Exceptions=%s",acExceptions);
			//fY+=10.0f;
			g_VisualDebug->Printf3D(obPosition,0.0f,-10.0f,0xff00ffff,DTF_ALIGN_HCENTRE,"  Exceptions=%s",acExceptions);
#endif
		}
#endif
	}

	void DebugCollisionTools::RenderCollisionFlags (const hkRigidBody* pobRigidBody)
	{
#ifndef _GOLD_MASTER
		if (!pobRigidBody)
			return;

		Physics::EntityCollisionFlag obFlag;
		Physics::FilterExceptionFlag obException;

		obFlag.base = pobRigidBody->getCollidable()->getCollisionFilterInfo();

		if (pobRigidBody->hasProperty(Physics::PROPERTY_FILTER_EXCEPTION_INT))
		{
			obException.base = pobRigidBody->getProperty(Physics::PROPERTY_FILTER_EXCEPTION_INT).getInt();
		}
		else
		{	
			obException.base=0;
			obException.flags.exception_set=0;
		}

		CPoint obPosition(pobRigidBody->getPosition()(0),pobRigidBody->getPosition()(1),pobRigidBody->getPosition()(2));

		RenderCollisionFlags(obPosition,obFlag,obException);

#ifndef JUST_COLLISION_RENDER
		char acMass[32];
		sprintf(acMass,"Mass: %f",pobRigidBody->getMass());
		g_VisualDebug->Printf3D(obPosition,0.0f,-30.0f,0xff00ffff,DTF_ALIGN_HCENTRE,acMass);
#endif
#endif
	}

	void DebugCollisionTools::RenderCollisionFlags (const hkShapePhantom* pobShapePhantom)
	{
		Physics::EntityCollisionFlag obFlag;
		Physics::FilterExceptionFlag obException;

		obFlag.base = pobShapePhantom->getCollidable()->getCollisionFilterInfo();

		if (pobShapePhantom->hasProperty(Physics::PROPERTY_FILTER_EXCEPTION_INT))
		{
			obException.base = pobShapePhantom->getProperty(Physics::PROPERTY_FILTER_EXCEPTION_INT).getInt();
		}
		else
		{	
			obException.base=0;
			obException.flags.exception_set=0;
		}

		CPoint obPosition(
			pobShapePhantom->getTransform().getTranslation()(0),
			pobShapePhantom->getTransform().getTranslation()(1),
			pobShapePhantom->getTransform().getTranslation()(2));

		RenderCollisionFlags(obPosition,obFlag,obException);
	}

	/*AdvancedCharacterController* System::GetCharacter()
	{
		return m_character;
	}

	void System::SetCharacter( AdvancedCharacterController* character )
	{
		m_character = character;
	}*/
	

/*
// [Mus] - 2005.07.28
// This methods was only used by the character controller interaction listener
// The main reason for this is because the character controller is not updated within the same loop that the rest of the world
void CDynamics::AddCollisionEvent (CEntity* pobSender,const hkCharacterObjectInteractionEvent& input)
{
	if(m_pobCollisionListener)
		m_pobCollisionListener->AddCollisionEvent(pobSender,input);
}

// [Mus] - 2005.07.28
// The collision listener is enabled by default and this method never seems called.
// Maybe it's unneeded...
void CDynamics::EnableCollisionListener ()
{
	if(m_pobCollisionListener)
		m_pobCollisionListener->Enable();
}
void CDynamics::DisableCollisionListener ()
{
	if(m_pobCollisionListener)
		m_pobCollisionListener->Disable();
}
*/


} // Physics
