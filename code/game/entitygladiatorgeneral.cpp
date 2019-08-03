#include "entitygladiatorgeneral.h"
#include "fsm.h"

#include "game/entitymanager.h"
#include "objectdatabase/dataobject.h"
#include "core/timer.h"
#include "game/attacks.h"
#include "game/awareness.h"	
#include "Physics/system.h"
#include "Physics/compoundlg.h"
#include "Physics/animatedlg.h"
#include "Physics/rigidbody.h"
#include "Physics/world.h"
#include "movement.h"
#include "camera/camman.h"
#include "camera/camview.h"
#include "anim/animator.h"
#include "game/strike.h"
#include "game/randmanager.h"
#include "messagehandler.h"
#include "game/interactioncomponent.h"
#include "core/visualdebugger.h"
#include "hud/hudmanager.h"
#include "hud/buttonhinter.h"
#include "camera/camutils.h"
#include "game/combathelper.h"
#include "game/entityparticleemitter.h"

/*
NOTE: The shell breaks SHOULD have always gone in the order ShellBreak_1 -> ShellBreak_3. The actual attacks which trigger the messages
which trigger the breaks are in-order in the file. Due to the way that the objectdatabase works, these SHOULD then be loaded in order,
added to the attack component in-order, and subsequently checked and selected in order.
However... due to some strange bug (which I'm currently tracking)... the order of the shellbreaks is not always predictable, so sometimes
the third shell-break will trigger first etc.

As such, as a temporary fix, I've made the shell-break order unimportant... as long as all three are broken, the fight will continue as
expected. However, this SHOULD really be a temporary thing. To revert back to the old order-based system, uncomment the SHELLBREAK_FORCE_ORDER
define below.

NOTE2: With the new non-order-based system, we assume that the selected specific vulnerability zone is the one at m_iSmashAttackPhaseNumber
when disabling the zones after a successful shellbreak. If it's a simple case that the zones are being added to the attack component in the
wrong order, then this will be fine. If something more obfusticated/buggy is going wrong, then this is not guaranteed to be true at all (they
may be in the correct order but being selected in the wrong order, at which point we'll now be disabling the wrong zone etc).
... just a heads up.
*/
//#define SHELLBREAK_FORCE_ORDER	//Define this to break/crash/error-report if the shell-breaks aren't done in order.

START_STD_INTERFACE(GladiatorGeneral)
	COPY_INTERFACE_FROM(Boss)
	DEFINE_INTERFACE_INHERITANCE(Boss)

	OVERRIDE_DEFAULT(Description, "boss,gladiatorgeneral")
	OVERRIDE_DEFAULT(Clump, "Characters\\gladiatorgeneral\\gladiatorgeneral.clump")
	OVERRIDE_DEFAULT(CombatDefinition, "GladiatorGeneral_AttackDefinition")
	OVERRIDE_DEFAULT(AwarenessDefinition, "GladiatorGeneral_AttackTargetingData")
	OVERRIDE_DEFAULT(AnimationContainer, "GladiatorGeneralAnimContainer")
	OVERRIDE_DEFAULT(CollisionHeight, "1.2")
	OVERRIDE_DEFAULT(CollisionRadius, "0.8")
	OVERRIDE_DEFAULT(Health, "500")
	OVERRIDE_DEFAULT(InitialAttackPhase, "GladiatorGeneral_Phase1_Attack")

	PUBLISH_VAR_WITH_DEFAULT_AS(m_iMinHealthTillShellBreak1, 200, MinHealthTillShellBreak1)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_iMinHealthTillShellBreak2, 150, MinHealthTillShellBreak2)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_iMinHealthTillShellBreak3, 100, MinHealthTillShellBreak3)

	//Defensive-spin target points.
	//NOTE: These defaults were made for the positions in the gladiatorgeneral test level (combat/gladiatorgeneral) and may not match temple.
	PUBLISH_VAR_WITH_DEFAULT_AS(m_aobDefensiveSpinTargetPoints[0], CPoint(5.0f, -18.3f, 5.0f), DefensiveSpinTarget[0])
	PUBLISH_VAR_WITH_DEFAULT_AS(m_aobDefensiveSpinTargetPoints[1], CPoint(-5.0f, -18.3f, -6.0f), DefensiveSpinTarget[1])
	PUBLISH_VAR_WITH_DEFAULT_AS(m_aobDefensiveSpinTargetPoints[2], CPoint(-6.0f, -18.3f, 5.0f), DefensiveSpinTarget[2])
	PUBLISH_VAR_WITH_DEFAULT_AS(m_aobDefensiveSpinTargetPoints[3], CPoint(-6.0f, -18.3f, -6.0f), DefensiveSpinTarget[3])

	PUBLISH_VAR_WITH_DEFAULT_AS(m_aobPillarNames[0], CHashedString("NULL"), PillarName[0]);
	PUBLISH_VAR_WITH_DEFAULT_AS(m_aobPillarNames[1], CHashedString("NULL"), PillarName[1]);
	PUBLISH_VAR_WITH_DEFAULT_AS(m_aobPillarNames[2], CHashedString("NULL"), PillarName[2]);
	PUBLISH_VAR_WITH_DEFAULT_AS(m_aobPillarNames[3], CHashedString("NULL"), PillarName[3]);
	PUBLISH_VAR_WITH_DEFAULT_AS(m_aobPillarNames[4], CHashedString("NULL"), PillarName[4]);

	DECLARE_POSTPOSTCONSTRUCT_CALLBACK(OnPostPostConstruct)
END_STD_INTERFACE

START_STD_INTERFACE(GladiatorGeneralStompSpecial)
	COPY_INTERFACE_FROM(BossSpecialAttack)
	DEFINE_INTERFACE_INHERITANCE(BossSpecialAttack)

	PUBLISH_VAR_AS(m_iMaxNumberOfStomps, MaxNumberOfStomps)
	PUBLISH_VAR_AS(m_iMaxNumberOfStompsAdjust, MaxNumberOfStompsAdjust)
	PUBLISH_PTR_AS(m_pobStomp[0], Stomp[0])
	PUBLISH_PTR_AS(m_pobStomp[1], Stomp[1])
	PUBLISH_PTR_AS(m_pobStomp[2], Stomp[2])
	PUBLISH_PTR_AS(m_pobStomp[3], Stomp[3])
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fMinStompedNearDistance, 1.0f, MinStompedNearDistance)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fMaxStompedNearDistance, 3.0f, MaxStompedNearDistance)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obStompedNearPlayerAttackData, "NULL", StompedNearPlayerAttackData)
END_STD_INTERFACE

START_STD_INTERFACE(GladiatorGeneralRollSpecial)
	COPY_INTERFACE_FROM(BossSpecialAttack)
	DEFINE_INTERFACE_INHERITANCE(BossSpecialAttack)

	PUBLISH_VAR_WITH_DEFAULT_AS(m_fPillarGlanceAngle, 40.0f, PillarGlanceAngle)
	PUBLISH_VAR_AS(m_iMaxNumberOfTrackingRolls, MaxNumberOfTrackingRolls)
	PUBLISH_VAR_AS(m_iMaxNumberOfTrackingRollsAdjust, MaxNumberOfTrackingRollsAdjust)
	PUBLISH_PTR_AS(m_pobRollStart, RollStart)
	PUBLISH_PTR_AS(m_pobRollTrack, RollTrack)
	PUBLISH_PTR_AS(m_pobRollAttack, RollAttack)
	PUBLISH_PTR_AS(m_pobRollHitPillar, RollHitPillar)
	PUBLISH_PTR_AS(m_pobRollGlancePillarLeft, RollGlanceOffPillarLeft)
	PUBLISH_PTR_AS(m_pobRollGlancePillarRight, RollGlanceOffPillarRight)
	PUBLISH_PTR_AS(m_pobFromStunnedRecoil, RecoilFromStunned)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fDamageOnHitPillar, 25.0f, DamageOnHitPillar)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fDamageOnGlanceOffPillar, 15.0f, DamageOnGlanceOffPillar)
END_STD_INTERFACE

START_STD_INTERFACE(GladiatorGeneralDefensiveSpinSpecial)
	COPY_INTERFACE_FROM(BossSpecialAttack)
	DEFINE_INTERFACE_INHERITANCE(BossSpecialAttack)

	PUBLISH_VAR_AS(m_iMaxNumberOfRolls, MaxNumberOfRolls)
	PUBLISH_VAR_AS(m_iMaxNumberOfRollsAdjust, MaxNumberOfRollsAdjust)
	PUBLISH_PTR_AS(m_pobRollStart, RollStart)
	PUBLISH_PTR_AS(m_pobRoll, Roll)
	PUBLISH_PTR_AS(m_pobRollLeap, RollLeap)
END_STD_INTERFACE

START_STD_INTERFACE(GladiatorGeneralSmashAttackSpecial)
	COPY_INTERFACE_FROM(BossSpecialAttack)
	DEFINE_INTERFACE_INHERITANCE(BossSpecialAttack)

	PUBLISH_PTR_AS(m_pobSmashAttack1, SmashAttack1)
	PUBLISH_PTR_AS(m_pobSmashAttack2, SmashAttack2)
	PUBLISH_PTR_AS(m_pobSmashAttack3, SmashAttack3)
END_STD_INTERFACE

START_STD_INTERFACE(GGenSmashAttackSelector)
	COPY_INTERFACE_FROM(BossAttackSelector)
	DEFINE_INTERFACE_INHERITANCE(BossAttackSelector)

	PUBLISH_VAR_AS(m_fMinDistance, MinDistance)
	PUBLISH_VAR_AS(m_fMaxDistance, MaxDistance)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fMinSafeDistance, 4.5f, MinSafeDistance)
END_STD_INTERFACE

//--------------------------------------------------
//!
//! GladiatorGeneral 
//! Any specific construction or updating for gladiator general
//!
//--------------------------------------------------
GladiatorGeneral::GladiatorGeneral()
: Boss()
{
	m_eBossType = BT_GLADIATOR_GENERAL;
	m_bVulnerableToShellBreakSpecials = false;
	//Some reasonable values for the min-health levels per shell-break and the damage-buffer for this (overridden by any provided by welder or xml).
	m_iMinHealthTillShellBreak1 = 200;
	m_iMinHealthTillShellBreak2 = 150;
	m_iMinHealthTillShellBreak3 = 100;
	m_bJustFinishedStomp = false;
	m_bInSpecialAttack = false;
	m_bHitWhileStunned = false;
	m_bNotifyIfHitStunned = false;
}

GladiatorGeneral::~GladiatorGeneral()
{
}


//--------------------------------------------------
//!
//! CreateShellPiece
//! Creates an object as part of the gladiator-general's shell and attaches it to the appropriate transform.
//! Any bits of shell that are going to come off during the fight are created as Interactable_Thrown objects
//! so that we can pick them up during the fight and use them as weapons. All other parts are just created
//! as CEntity objects.
//!
//--------------------------------------------------
void GladiatorGeneral::CreateShellPiece(const char* pcName, const char* pcClump, const char* pcTransformName, bool bThrowable, bool bVisible,
										const CPoint &obOffset)
{
	char acFullPath[128] = {0};
	sprintf(acFullPath, "entities/characters/gladiatorgeneral/accessories/%s", pcClump);

	LuaAttributeTable* pobTable = LuaAttributeTable::Create();
	DataObject* pDO;
	if(bThrowable)
	{
		pDO = ObjectDatabase::Get().ConstructObject( "Interactable_Thrown", pcName, GameGUID(), 0, true, false );
	}
	else
	{
		pDO = ObjectDatabase::Get().ConstructObject( "CEntity", pcName, GameGUID(), 0, true, false );
	}
	CEntity* pobNewEntity  = (CEntity*) pDO->GetBasePtr();
	pobNewEntity->SetAttributeTable( pobTable );
	pobNewEntity->GetAttributeTable()->SetDataObject( pDO );
	pobTable->SetAttribute("Name", pcName );
	pobTable->SetAttribute("Clump", acFullPath );

	if(bThrowable)
	{
		pobTable->SetString("SharedAttributes", "Att_Thrown_Shield");	//TODO: Need our own one, Att_Thrown_GGShellPiece perhaps?
		pobTable->SetString("InitialState", "DefaultState");
		pobTable->SetString("ParentTransform", "ROOT");
		pobNewEntity->SetRotation(CQuat(0.0f, 0.0f, 0.0f, 1.0f));	//MUST be done or you'll get a very strange set of axis on the object + warp.
	}

	ObjectDatabase::Get().DoPostLoadDefaults( pDO );

	// Parent the shell piece
	Transform* pobParentTransform = this->GetHierarchy()->GetTransform( pcTransformName );
	Transform* pobTargTransform = pobNewEntity->GetHierarchy()->GetRootTransform();
	pobNewEntity->SetParentEntity( this );
	pobTargTransform->RemoveFromParent();
	//Change the local offset on this transform.
	pobTargTransform->SetLocalTranslation(obOffset);
	pobParentTransform->AddChild( pobTargTransform );

	//For throwable pieces, set them to their attached state (so they can't be interacted with or picked up whilst still attached and will be keyframed).
	if(bThrowable)
	{
		if(pobNewEntity->GetMessageHandler())
		{
			Message gotoattached(msg_goto_attachedstate);
			pobNewEntity->GetMessageHandler()->QueueMessage(gotoattached);
		}
	}

	if(pobNewEntity->GetPhysicsSystem())
	{
		pobNewEntity->GetPhysicsSystem()->Lua_DeactivateState("Rigid");
	}

	//If this piece is supposed to be invisible for now, then hide it.
	if(bVisible == false)
	{
		pobNewEntity->Hide();
	}

	//Push the shell onto our list (the list is used to check whether we still have it attached or not).
	m_obShellPieces.push_back(pobNewEntity);
}


//--------------------------------------------------
//!
//! DetachShellPiece
//! Detaches a shell piece and sends it's new velocity with the message.
//!
//--------------------------------------------------
void GladiatorGeneral::DetachShellPiece(const char* pcName, float fXDir, float fYDir, float fZDir, float fSpeed,
										const char* pcShow, const char* pcDetach)
{
	//If pcShow or pcDetach are NULL, then we probably just want to detach the pcName clump as a throwable object.
	if(!pcShow || !pcDetach)
	{
		//Check that it's in our list.
		for (ntstd::List<CEntity*>::iterator obIt = m_obShellPieces.begin(); obIt != m_obShellPieces.end(); obIt++)
		{
			if ((*obIt)->GetName() == pcName)
			{
				//Send the piece a detach message to turn it back into a throwable object.
				Message detachmessage(msg_detach_velocity);
				//Build our new velocity vector from the components passed (being sure to orient to character matrix).
				CDirection obDir(fXDir, fYDir, fZDir);
				obDir.Normalise();
				CDirection obDirSet = obDir * GetMatrix();
				obDirSet.Normalise(); obDirSet *= fSpeed;

				//Seeing as we can't attach a vector, just set-float the three components.
				detachmessage.SetFloat("XVel", obDirSet.X());
				detachmessage.SetFloat("YVel", obDirSet.Y());
				detachmessage.SetFloat("ZVel", obDirSet.Z());
				if((*obIt)->GetMessageHandler())
				{
					(*obIt)->GetMessageHandler()->QueueMessage(detachmessage);
				}

				//Now erase it from our list of shell pieces.
				m_obShellPieces.erase(obIt);
				break;
			}
		}
	}
	else
	{
		//Otherwise, we want to hide pcName, show pcShow and pcDetach, and detach pcDetach as a throwable object.
		for(int i = 0 ; i < 3 ; i++)
		{
			const char* pcFindThis = NULL;
			switch(i)
			{
			case 0:
				pcFindThis = pcName;
				break;
			case 1:
				pcFindThis = pcShow;
				break;
			case 2:
				pcFindThis = pcDetach;
				break;
			default:
				ntAssert_p(false, ("DetachShellPiece: Shouldn't get here (1)"));
				break;
			}

			//Check that it's in our list.
			for (ntstd::List<CEntity*>::iterator obIt = m_obShellPieces.begin(); obIt != m_obShellPieces.end(); obIt++)
			{
				if ((*obIt)->GetName() == pcFindThis)
				{
					switch(i)
					{
					case 0:
						//If this is pcName, remove.
						//Note: Previously we just hid the object but after cutscenes when the GGen was told to re-show it showed all the
						//previously hidden pieces too. So now we also reparent to the world so that it's not a child of the GGen anymore.
						//(This will need to be taken into account if for any reason we want to put them back later (reloading etc?))
						(*obIt)->Lua_ReparentToWorld();
						(*obIt)->Hide();
						break;
					case 1:
						//If this is pcShow, show.
						(*obIt)->Show();
						break;
					case 2:
						//If this is pcDetach, show and detach!
						{
							(*obIt)->Show();
							//Send the piece a detach message to turn it back into a throwable object.
							Message detachmessage(msg_detach_velocity);
							//Build our new velocity vector from the components passed (being sure to orient to character matrix).
							CDirection obDir(fXDir, fYDir, fZDir);
							obDir.Normalise();
							CDirection obDirSet = obDir * GetMatrix();
							obDirSet.Normalise(); obDirSet *= fSpeed;

							//Seeing as we can't attach a vector, just set-float the three components.
							detachmessage.SetFloat("XVel", obDirSet.X());
							detachmessage.SetFloat("YVel", obDirSet.Y());
							detachmessage.SetFloat("ZVel", obDirSet.Z());
							if((*obIt)->GetMessageHandler())
							{
								(*obIt)->GetMessageHandler()->QueueMessage(detachmessage);
							}
						}
						break;
					default:
						ntAssert_p(false, ("DetachShellPiece: Shouldn't get here (2)"));
						break;
					}


					//Now erase it from our list of shell pieces.
					m_obShellPieces.erase(obIt);
					break;
				}
			}
		}
	}
}


//--------------------------------------------------
//!
//! CreateVulnerabilityParticleEmitter
//! This function creates a particle emitter owned by the GGen that can be placed in
//! the correct position and shown when the GGen is vulnerable to shell-grabs.
//!
//--------------------------------------------------
void GladiatorGeneral::CreateVulnerabilityParticleEmitter()
{
	LuaAttributeTable* pobTable = LuaAttributeTable::Create();
	DataObject* pDO;
	pDO = ObjectDatabase::Get().ConstructObject( "Object_ParticleEmitter", "GGenVulnerabilityZoneMarker", GameGUID(), 0, true, false );

	CEntity* pobNewEntity  = (CEntity*) pDO->GetBasePtr();
	if(pobNewEntity)
	{
		pobNewEntity->SetAttributeTable( pobTable );
		pobNewEntity->GetAttributeTable()->SetDataObject( pDO );
		pobTable->SetAttribute("Name", "GGenVulnerabilityZoneMarker" );
		pobTable->SetAttribute("Clump", "" );
		pobTable->SetAttribute("ParticleFX", "NS_Location_PSystemSimpleDef");
		pobTable->SetAttribute("Active", "0");

		ObjectDatabase::Get().DoPostLoadDefaults( pDO );

		if(pobNewEntity->GetPhysicsSystem())
		{
			pobNewEntity->GetPhysicsSystem()->Lua_DeactivateState("Rigid");
		}

		//Store the pointer internally so that we can toggle the emitter on and off whenever we want.
		m_pobParticleEmitterEntity = pobNewEntity;
	}

	//It starts deactivated.
	m_bEmitterIsActive = false;
}


//--------------------------------------------------
//!
//! ShowVulnerabilityZoneMarker
//! Toggles display the of the vulnerability zone marker on and off.
//! Every time the vulnerability zone marker is turned on, the position is updated.
//!
//--------------------------------------------------
void GladiatorGeneral::ShowVulnerabilityZoneMarker(bool bShow)
{
	if(bShow)
	{
		//Seeing as the GGen can't move while stuck in the ground we can safely set the position once here.
		if(!m_bEmitterIsActive)
		{
			if(m_pobParticleEmitterEntity)
			{
				CPoint obGGenPos = GetPosition();

				CPoint obNewPos = obGGenPos + CPoint(-GetMatrix().GetZAxis() * 1.5f);
				Object_ParticleEmitter* pEmitter = (Object_ParticleEmitter*)m_pobParticleEmitterEntity;
				pEmitter->m_PosMatrix.SetIdentity();
				pEmitter->m_PosMatrix.SetTranslation(obNewPos);

				//Send an activate mesage.
				if(m_pobParticleEmitterEntity->GetMessageHandler())
				{
					m_pobParticleEmitterEntity->GetMessageHandler()->ReceiveMsg<Activate>();
				}

				m_bEmitterIsActive = true;
			}
		}
	}
	else
	{
		if(m_bEmitterIsActive && m_pobParticleEmitterEntity)
		{
			if(m_pobParticleEmitterEntity->GetMessageHandler())
			{
				m_pobParticleEmitterEntity->GetMessageHandler()->ReceiveMsg<Deactivate>();
			}
			m_bEmitterIsActive = false;
		}
	}
}


//--------------------------------------------------
//!
//! BreakShell_1
//! Handles breaking off the top parts of the shell and making them dynamic throwable objects.
//!
//--------------------------------------------------
void GladiatorGeneral::BreakShell_1()
{
#ifdef SHELLBREAK_FORCE_ORDER
	ntError_p((m_iSmashAttackPhaseNumber == 0), ("This should ONLY ever be called on the first shell-break action-sequence!"));
#else
	if(m_iSmashAttackPhaseNumber != 0)
	{
		ntPrintf("***###***ERROR (non-fatal now): Shell-breaks done in the wrong order. This should've been break 1***###***\n");
	}
#endif

	//We really need a non-hardcoded solution. Parenting to chains for the first frame and using that velocity could be dangerously fast though.
	//This solution will serve for now at least, we can always replace it later, just read these values from xml or something!
	DetachShellPiece("GGen_shell_1", 0.02f, 0.4f, 0.3f, 6.3f);
	DetachShellPiece("GGen_shell_2", 0.2f, 0.2f, 0.3f, 4.5f, "GGen_shell_2_a", "GGen_shell_2_b");
	DetachShellPiece("GGen_shell_3", -0.2f, 0.2f, 0.3f, 5.2f, "GGen_shell_3_a", "GGen_shell_3_b");

	//Disable the first specific vulnerability zone
	//Note: This is extremely hacky, but provides a safe fall-back for fail-path or attack not linking mid-way through sequence.
	//Note2: It does, however, rely on the fact that the three specific vulnerability zones are in-order on the character...
#ifdef SHELLBREAK_FORCE_ORDER
	int iDesiredZone = 0;
#else
	int iDesiredZone = m_iSmashAttackPhaseNumber;
#endif
	int iCurrentZone = 0;
	CAttackDefinition* pobAttackDefinition = const_cast<CAttackDefinition*>(GetAttackComponent()->GetAttackDefinition());
	for(ntstd::List<SpecificAttackVulnerabilityZone*, Mem::MC_ENTITY>::const_iterator obIt = pobAttackDefinition->m_obSpecificVulnerabilities.begin();
		obIt != pobAttackDefinition->m_obSpecificVulnerabilities.end(); obIt++)
	{
		if(iCurrentZone == iDesiredZone)
		{
			//Setting the "remove if successful" flag here should be enough.
			(*obIt)->SetRemoveIfUsedSuccessfully(true);
			break;
		}

		iCurrentZone++;
	}

	//Move onto the next smash-attack (has a different struggle animation, less time to execute the next shell-break special).
#ifdef SHELLBREAK_FORCE_ORDER
	m_iSmashAttackPhaseNumber = 1;
#else
	m_iSmashAttackPhaseNumber++;

	if(m_iSmashAttackPhaseNumber == 3)
	{
		float fCurrHealth = ToCharacter()->GetCurrHealth() - 90;

		ChangeHealth(-fCurrHealth, "The third shell break signifies the end of the battle, reduce his health to 5");
	}
#endif
}


//--------------------------------------------------
//!
//! BreakShell_2
//! Handles breaking off the (WHICH?) parts of the shell and making them dynamic throwable objects.
//!
//--------------------------------------------------
void GladiatorGeneral::BreakShell_2()
{
#ifdef SHELLBREAK_FORCE_ORDER
	ntError_p((m_iSmashAttackPhaseNumber == 1), ("This should ONLY ever be called on the second shell-break action-sequence!"));
#else
	if(m_iSmashAttackPhaseNumber != 1)
	{
		ntPrintf("***###***ERROR (non-fatal now): Shell-breaks done in the wrong order. This should've been break 2***###***\n");
	}
#endif

	//TODO: Break some shell pieces here (when we know which pieces are required to break off for the corresponding anim).
	DetachShellPiece("GGen_shell_4", -0.7f, 0.4f, 0.1f, 3.2f, "GGen_shell_4_a", "GGen_shell_4_b");
	DetachShellPiece("GGen_shell_5", 0.7f, 0.4f, 0.1f, 3.3f, "GGen_shell_5_a", "GGen_shell_5_b");
	DetachShellPiece("GGen_shell_6", -0.7f, 0.2f, 0.1f, 2.0f, "GGen_shell_6_a", "GGen_shell_6_b");
	DetachShellPiece("GGen_shell_7", 0.7f, 0.2f, 0.1f, 2.0f, "GGen_shell_7_a", "GGen_shell_7_b");
	DetachShellPiece("GGen_shell_8", -0.7f, 0.05f, 0.1f, 3.0f, "GGen_shell_8_a", "GGen_shell_8_b");
	DetachShellPiece("GGen_shell_9", 0.7f, 0.05f, 0.1f, 3.0f, "GGen_shell_9_a", "GGen_shell_9_b");

	//Disable the second specific vulnerability zone
	//Note: This is extremely hacky, but provides a safe fall-back for fail-path or attack not linking mid-way through sequence.
	//Note2: It does, however, rely on the fact that the three specific vulnerability zones are in-order on the character...
#ifdef SHELLBREAK_FORCE_ORDER
	int iDesiredZone = 1;
#else
	int iDesiredZone = m_iSmashAttackPhaseNumber;
#endif
	int iCurrentZone = 0;
	CAttackDefinition* pobAttackDefinition = const_cast<CAttackDefinition*>(GetAttackComponent()->GetAttackDefinition());
	for(ntstd::List<SpecificAttackVulnerabilityZone*, Mem::MC_ENTITY>::const_iterator obIt = pobAttackDefinition->m_obSpecificVulnerabilities.begin();
		obIt != pobAttackDefinition->m_obSpecificVulnerabilities.end(); obIt++)
	{
		if(iCurrentZone == iDesiredZone)
		{
			//Setting the "remove if successful" flag here should be enough.
			(*obIt)->SetRemoveIfUsedSuccessfully(true);
			break;
		}

		iCurrentZone++;
	}

	//Move onto the next smash-attack (has a different struggle animation, less time to execute the next shell-break special).
#ifdef SHELLBREAK_FORCE_ORDER
	m_iSmashAttackPhaseNumber = 2;
#else
	m_iSmashAttackPhaseNumber++;

	if(m_iSmashAttackPhaseNumber == 3)
	{
		float fCurrHealth = ToCharacter()->GetCurrHealth() - 90;
		ChangeHealth(-fCurrHealth, "The third shell break signifies the end of the battle, reduce his health to 5");
	}
#endif
}


//--------------------------------------------------
//!
//! BreakShell_3
//! Handles breaking off the (WHICH?) parts of the shell and making them dynamic throwable objects.
//!
//--------------------------------------------------
void GladiatorGeneral::BreakShell_3()
{
#ifdef SHELLBREAK_FORCE_ORDER
	ntError_p((m_iSmashAttackPhaseNumber == 2), ("This should ONLY ever be called on the third shell-break action-sequence!"));
#else
	if(m_iSmashAttackPhaseNumber != 2)
	{
		ntPrintf("***###***ERROR (non-fatal now): Shell-breaks done in the wrong order. This should've been break 3***###***\n");
	}
#endif

	//TODO: Break some shell pieces here (when we know which pieces are required to break off for the corresponding anim).
	DetachShellPiece("GGen_shell_10", -0.5f, 0.2f, 0.1f, 3.2f);
	DetachShellPiece("GGen_shell_11", 0.4f, 0.2f, 0.1f, 3.3f);
	DetachShellPiece("GGen_shell_12", -0.4f, 0.1f, 0.1f, 2.0f, "GGen_shell_12_a", "GGen_shell_12_b");
	DetachShellPiece("GGen_shell_13", 0.5f, 0.1f, 0.1f, 2.0f, "GGen_shell_13_a", "GGen_shell_13_b");
	DetachShellPiece("GGen_shell_14", -0.3f, -0.05f, 0.1f, 3.0f, "GGen_shell_14_a", "GGen_shell_14_b");
	DetachShellPiece("GGen_shell_15", 0.6f, -0.05f, 0.1f, 3.0f, "GGen_shell_15_a", "GGen_shell_15_b");
	DetachShellPiece("GGen_shell_16", -0.02f, -0.4f, 0.1f, 3.0f);

	//Disable the third specific vulnerability zone
	//Note: This is extremely hacky, but provides a safe fall-back for fail-path or attack not linking mid-way through sequence.
	//Note2: It does, however, rely on the fact that the three specific vulnerability zones are in-order on the character...
#ifdef SHELLBREAK_FORCE_ORDER
	int iDesiredZone = 2;
#else
	int iDesiredZone = m_iSmashAttackPhaseNumber;
#endif
	int iCurrentZone = 0;
	CAttackDefinition* pobAttackDefinition = const_cast<CAttackDefinition*>(GetAttackComponent()->GetAttackDefinition());
	for(ntstd::List<SpecificAttackVulnerabilityZone*, Mem::MC_ENTITY>::const_iterator obIt = pobAttackDefinition->m_obSpecificVulnerabilities.begin();
		obIt != pobAttackDefinition->m_obSpecificVulnerabilities.end(); obIt++)
	{
		if(iCurrentZone == iDesiredZone)
		{
			//Setting the "remove if successful" flag here should be enough.
			(*obIt)->SetRemoveIfUsedSuccessfully(true);
			break;
		}

		iCurrentZone++;
	}

	//Move onto the next smash-attack phase. In this phase, no smash attacks will initialise successfully, so this is the
	//end for the smash attacks. (At this point we're supposed to go into the Ninja-Sequence to finish the battle anyway).
#ifdef SHELLBREAK_FORCE_ORDER
	m_iSmashAttackPhaseNumber = 3;
#else
	m_iSmashAttackPhaseNumber++;
#endif

	if(m_iSmashAttackPhaseNumber == 3)
	{
		float fCurrHealth = ToCharacter()->GetCurrHealth() - 90;
		ChangeHealth(-fCurrHealth, "The third shell break signifies the end of the battle, reduce his health to 5");
	}
}


void GladiatorGeneral::OnPostPostConstruct()
{
	// Get pillars
	for(int i = 0 ; i < GGEN_NUM_PILLARS ; i++)
	{
		if(!ntStr::IsNull(m_aobPillarNames[i]))
		{
			DataObject* pobInterface = ObjectDatabase::Get().GetDataObjectFromName(m_aobPillarNames[i]);
			if(pobInterface)
			{
				m_obPillars.push_back((CEntity*)pobInterface->GetBasePtr());
			}
		}
	}


	// Shell pieces
	// PROBABLY NEED THESE EVENTUALLY IN SEPARATE LISTS AS CHUNKS TO BE BROKEN OFF DURING SPECIFIC ATTACKS
	// Piece 1->23
	CreateShellPiece("GGen_shell_1", "shell_upper_lame.clump", "shell_upperJ04", true, true, CPoint(0.022f, 0.0f, 0.4f));
	CreateShellPiece("GGen_shell_2", "shell_upperj03_rlame.clump", "shell_upperJ03_Rrib", false, true, CPoint(0.0f, 0.0f, 0.0f));
	CreateShellPiece("GGen_shell_2_a", "shell_broken_upperj03_rlame_a.clump", "shell_upperJ03_Rrib", false, false, CPoint(0.0f, 0.0f, 0.0f));
	CreateShellPiece("GGen_shell_2_b", "shell_broken_upperj03_rlame_b.clump", "shell_upperJ03_Rrib", true, false, CPoint(0.139073817f, 0.2641194493f, -0.5069231949f));
	CreateShellPiece("GGen_shell_3", "shell_upperj03_llame.clump", "shell_upperJ03_Lrib", false, true, CPoint(0.0f, 0.0f, 0.0f));
	CreateShellPiece("GGen_shell_3_a", "shell_broken_upperj03_llame_a.clump", "shell_upperJ03_Lrib", false, false, CPoint(0.0f, 0.0f, 0.0f));
	CreateShellPiece("GGen_shell_3_b", "shell_broken_upperj03_llame_b.clump", "shell_upperJ03_Lrib", true, false, CPoint(0.143955739f, 0.2621517334f, 0.4643258209f));
	CreateShellPiece("GGen_shell_4", "shell_upperj02_rlame.clump", "shell_upperJ02_Rrib", false, true, CPoint(0.0f, 0.0f, 0.0f));
	CreateShellPiece("GGen_shell_4_a", "shell_broken_upperj02_rlame_a.clump", "shell_upperJ02_Rrib", false, false, CPoint(0.0f, 0.0f, 0.0f));
	CreateShellPiece("GGen_shell_4_b", "shell_broken_upperj02_rlame_b.clump", "shell_upperJ02_Rrib", true, false, CPoint(0.1542331314f, 0.1650485229f, -0.520135603f));
	CreateShellPiece("GGen_shell_5", "shell_upperj02_llame.clump", "shell_upperJ02_Lrib", false, true, CPoint(0.0f, 0.0f, 0.0f));
	CreateShellPiece("GGen_shell_5_a", "shell_broken_upperj02_llame_a.clump", "shell_upperJ02_Lrib", false, false, CPoint(0.0f, 0.0f, 0.0f));
	CreateShellPiece("GGen_shell_5_b", "shell_broken_upperj02_llame_b.clump", "shell_upperJ02_Lrib", true, false, CPoint(0.1634004974f, 0.1655285645f, 0.5250742149f));
	CreateShellPiece("GGen_shell_6", "shell_upperj01_rlame.clump", "shell_upperJ01_Rrib", false, true, CPoint(0.0f, 0.0f, 0.0f));
	CreateShellPiece("GGen_shell_6_a", "shell_broken_upperj01_rlame_a.clump", "shell_upperJ01_Rrib", false, false, CPoint(0.0f, 0.0f, 0.0f));
	CreateShellPiece("GGen_shell_6_b", "shell_broken_upperj01_rlame_b.clump", "shell_upperJ01_Rrib", true, false, CPoint(0.1394982874f, 0.1092948617f, -0.5242307732f));
	CreateShellPiece("GGen_shell_7", "shell_upperj01_llame.clump", "shell_upperJ01_Lrib", false, true, CPoint(0.0f, 0.0f, 0.0f));
	CreateShellPiece("GGen_shell_7_a", "shell_broken_upperj01_llame_a.clump", "shell_upperJ01_Lrib", false, false, CPoint(0.0f, 0.0f, 0.0f));
	CreateShellPiece("GGen_shell_7_b", "shell_broken_upperj01_llame_b.clump", "shell_upperJ01_Lrib", true, false, CPoint(0.1486502457f, 0.111096344f, 0.4982301331f));
	CreateShellPiece("GGen_shell_8", "shell_j00_rlame.clump", "shell_upperJ00_Rrib", false, true, CPoint(0.0f, 0.0f, 0.0f));
	CreateShellPiece("GGen_shell_8_a", "shell_broken_j00_rlame_a.clump", "shell_upperJ00_Rrib", false, false, CPoint(0.0f, 0.0f, 0.0f));
	CreateShellPiece("GGen_shell_8_b", "shell_broken_j00_rlame_b.clump", "shell_upperJ00_Rrib", true, false, CPoint(-0.02435512543f, 0.1056547546f, -0.5506331825f));
	CreateShellPiece("GGen_shell_9", "shell_j00_llame.clump", "shell_upperJ00_Lrib", false, true, CPoint(0.0f, 0.0f, 0.0f));
	CreateShellPiece("GGen_shell_9_a", "shell_broken_j00_llame_a.clump", "shell_upperJ00_Lrib", false, false, CPoint(0.0f, 0.0f, 0.0f));
	CreateShellPiece("GGen_shell_9_b", "shell_broken_j00_llame_b.clump", "shell_upperJ00_Lrib", true, false, CPoint(-0.0131379509f, 0.1051834488f, 0.5444100761f));
	CreateShellPiece("GGen_shell_10", "shell_lowerj03_rlame.clump", "shell_lowerJ03_Rrib", true, true, CPoint(0.0f, 0.0f, 0.0f));
	CreateShellPiece("GGen_shell_11", "shell_lowerj03_llame.clump", "shell_lowerJ03_Lrib", true, true, CPoint(0.0f, 0.0f, 0.0f));
	CreateShellPiece("GGen_shell_12", "shell_lowerj02_rlame.clump", "shell_lowerJ02_Rrib", false, true, CPoint(0.0f, 0.0f, 0.0f));
	CreateShellPiece("GGen_shell_12_a", "shell_broken_lowerj02_rlame_a.clump", "shell_lowerJ02_Rrib", false, false, CPoint(0.0f, 0.0f, 0.0f));
	CreateShellPiece("GGen_shell_12_b", "shell_broken_lowerj02_rlame_b.clump", "shell_lowerJ02_Rrib", true, false, CPoint(-0.1471289444f, 0.09266170502f, -0.3587460136f));
	CreateShellPiece("GGen_shell_13", "shell_lowerj02_llame.clump", "shell_lowerJ02_Lrib", false, true, CPoint(0.0f, 0.0f, 0.0f));
	CreateShellPiece("GGen_shell_13_a", "shell_broken_lowerj02_llame_a.clump", "shell_lowerJ02_Lrib", false, false, CPoint(0.0f, 0.0f, 0.0f));
	CreateShellPiece("GGen_shell_13_b", "shell_broken_lowerj02_llame_b.clump", "shell_lowerJ02_Lrib", true, false, CPoint(-0.1419048595f, 0.0921600914f, 0.3579624367f));
	CreateShellPiece("GGen_shell_14", "shell_lowerj01_rlame.clump", "shell_lowerJ01_Rrib", false, true, CPoint(0.0f, 0.0f, 0.0f));
	CreateShellPiece("GGen_shell_14_a", "shell_broken_lowerj01_rlame_a.clump", "shell_lowerJ01_Rrib", false, false, CPoint(0.0f, 0.0f, 0.0f));
	CreateShellPiece("GGen_shell_14_b", "shell_broken_lowerj01_rlame_b.clump", "shell_lowerJ01_Rrib", true, false, CPoint(-0.1584386477f, 0.1268288284f, -0.4745765594f));
	CreateShellPiece("GGen_shell_15", "shell_lowerj01_llame.clump", "shell_lowerJ01_Lrib", false, true, CPoint(0.0f, 0.0f, 0.0f));
	CreateShellPiece("GGen_shell_15_a", "shell_broken_lowerj01_llame_a.clump", "shell_lowerJ01_Lrib", false, false, CPoint(0.0f, 0.0f, 0.0f));
	CreateShellPiece("GGen_shell_15_b", "shell_broken_lowerj01_llame_b.clump", "shell_lowerJ01_Lrib", true, false, CPoint(-0.1526823616f, 0.1285562134f, 0.4747077751f));
	CreateShellPiece("GGen_shell_16", "shell_lower_lame.clump", "shell_lowerJ04", true, true, CPoint(0.035f, 0.0f, 0.217f));
	CreateShellPiece("GGen_shell_17", "shell_j00_spine.clump", "shell_J00", false, true, CPoint(0.0f, 0.0f, 0.0f));
	CreateShellPiece("GGen_shell_18", "shell_lowerj01_spine.clump", "shell_lowerJ01", false, true, CPoint(0.0f, 0.0f, 0.0f));
	CreateShellPiece("GGen_shell_19", "shell_lowerj02_spine.clump", "shell_lowerJ02", false, true, CPoint(0.0f, 0.0f, 0.0f));
	CreateShellPiece("GGen_shell_20", "shell_lowerj03_spine.clump", "shell_lowerJ03", false, true, CPoint(0.0f, 0.0f, 0.0f));
	CreateShellPiece("GGen_shell_21", "shell_upperj01_spine.clump", "shell_upperJ01", false, true, CPoint(0.0f, 0.0f, 0.0f));
	CreateShellPiece("GGen_shell_22", "shell_upperj02_spine.clump", "shell_upperJ02", false, true, CPoint(0.0f, 0.0f, 0.0f));
	CreateShellPiece("GGen_shell_23", "shell_upperj03_spine.clump", "shell_upperJ03", false, true, CPoint(0.0f, 0.0f, 0.0f));

	//Disable our vulnerability zones.
	CAttackDefinition* pobAttackDefinition = const_cast<CAttackDefinition*>(GetAttackComponent()->GetAttackDefinition());
	for(ntstd::List<SpecificAttackVulnerabilityZone*, Mem::MC_ENTITY>::const_iterator obIt = pobAttackDefinition->m_obSpecificVulnerabilities.begin();
		obIt != pobAttackDefinition->m_obSpecificVulnerabilities.end(); obIt++)
	{
		(*obIt)->SetDisabled(true);
		//Also make sure their RemoveIfUsedSuccessfully flag is set to false. We do NOT want this happening or it will be removed
		//as soon as you successfully press the attack button in the zone and start the attack... that's not to say that you'll always
		//successfully COMPLETE the attack/action-sequence. We MANUALLY disable these ones instead.
		//Last-time someone changed the data on this it took a while to track down!
		(*obIt)->SetRemoveIfUsedSuccessfully(false);
	}

	//Create our particle emitter entity and hide it.
	CreateVulnerabilityParticleEmitter();

	//Initialise our smash attack phase to phase 0.
	m_iSmashAttackPhaseNumber = 0;
}

bool GladiatorGeneral::CanStartAnAttack()
{
	return GetAttackComponent()->AI_Access_GetState() == CS_STANDARD;
}

bool GladiatorGeneral::CanStartALinkedAttack()
{
	return GetAttackComponent()->IsInNextMoveWindow() && !GetAttackComponent()->HasSelectedANextMove();
}


//--------------------------------------------------
//!
//! SetShellbreakVulnerability 
//! This function sets the vulnerability of the gladiator general to shell-break attacks.
//! This involves toggling his shell-break specific-attack-vulnerability-zones on and off.
//! It should be toggled on when the smash-attack fails, and off at all other times!
//!
//--------------------------------------------------
void GladiatorGeneral::SetShellbreakVulnerability(bool bVulnerableToShellBreak)
{
	if(m_bVulnerableToShellBreakSpecials != bVulnerableToShellBreak)
	{
		m_bVulnerableToShellBreakSpecials = bVulnerableToShellBreak;

		//Enable or disable our vulnerability zones if we're entering or leaving this state.
		CAttackDefinition* pobAttackDefinition = const_cast<CAttackDefinition*>(GetAttackComponent()->GetAttackDefinition());
		for(ntstd::List<SpecificAttackVulnerabilityZone*, Mem::MC_ENTITY>::const_iterator obIt = pobAttackDefinition->m_obSpecificVulnerabilities.begin();
			obIt != pobAttackDefinition->m_obSpecificVulnerabilities.end(); obIt++)
		{
			(*obIt)->SetDisabled(!m_bVulnerableToShellBreakSpecials);
		}
	}
}


//--------------------------------------------------
//!
//! StopNotificationWithPillars 
//! This function toggles off the notification of collision with the pillars.
//! This is so that at the end of the roll attack (but before the 2fs anim) an
//! anim-event message can be used to turn off notification. Otherwise, during the 2fs
//! he'll be able to smash up against the pillars.
//!
//--------------------------------------------------
void GladiatorGeneral::StopNotificationWithPillars()
{
	//Remove all pillars from notification list
	for (ntstd::List<CEntity*>::iterator obIt = m_obPillars.begin(); obIt != m_obPillars.end(); obIt++)
	{
		GetInteractionComponent()->StopNotifyOnInteractionWith(*obIt);
	}
}


void GladiatorGeneral::UpdateBossSpecifics(float fTimeDelta)
{
	UNUSED(fTimeDelta);

	//We need a pointer to the player to query if they're in our vulnerability zones or not.
	CEntity* pPlayer = CEntityManager::Get().GetPlayer();

	//Put a button hint on the screen if we are vulnerable to shell-break attacks and the player is within one of our
	//specific-attack-vulnerability-zones.
	if (m_bVulnerableToShellBreakSpecials)
	{
		SpecificAttackVulnerabilityZone* pobVulnerabilityZone = 0;
		CAttackLink* pobAttackLink = 0;

		CAttackDefinition* pobAttackDefinition = const_cast<CAttackDefinition*>(GetAttackComponent()->GetAttackDefinition());

		for(ntstd::List<SpecificAttackVulnerabilityZone*, Mem::MC_ENTITY>::const_iterator obIt = pobAttackDefinition->m_obSpecificVulnerabilities.begin();
			obIt != pobAttackDefinition->m_obSpecificVulnerabilities.end(); obIt++)
		{
			//Perform any necessary checks here, breaking early if we find one the player is in.
			if((*obIt)->IsDisabled())
			{
				continue;
			}

			pobAttackLink = (*obIt)->IsInZone(this, pPlayer);
			if(pobAttackLink)
			{
				pobVulnerabilityZone = *obIt;
				break;
			}
		}

		//If we found a zone that the player was in...
		if(pobVulnerabilityZone && pobAttackLink)
		{
			//Render a button hint (using the data from pobAttackLink to determine which button based on attack-type I guess).
			//For a test, just put the AB_GRAB button.
			//We put this on the player's attack component, as the hud queries this.
			pPlayer->GetAttackComponent()->SetNeedsHintForButton(AB_GRAB, true);
		}
		else
		{
			//No button press set for this button.
			pPlayer->GetAttackComponent()->SetNeedsHintForButton(AB_GRAB, false);
			CHud::Get().GetCombatHUDElements()->m_pobButtonHinter->SetHintForButton(AB_GRAB, false);
		}

		//Also show the vulnerable emitter if it's not already shown.
		ShowVulnerabilityZoneMarker(true);
	}
	else
	{
		//Make sure the emitter isn't showing!
		ShowVulnerabilityZoneMarker(false);
	}

	//We fix the gladiator general's health above certain thresholds depending on how many of the
	//shell-break attacks have been successfully completed.
	if(m_iSmashAttackPhaseNumber < 3)
	{
		float fCurrHealth = ToCharacter()->GetCurrHealth();
		switch(m_iSmashAttackPhaseNumber)
		{
		case 0:
			if(fCurrHealth < m_iMinHealthTillShellBreak1)
			{
				//Set current health to m_iMinHealthTillShellBreak1
				float fChange = (float)(m_iMinHealthTillShellBreak1 - fCurrHealth);
				ChangeHealth(fChange, "Keeping gladiator general above threshold till shell-break 1 done");
			}
			break;
		case 1:
			if(fCurrHealth < m_iMinHealthTillShellBreak2)
			{
				//Set current health to m_iMinHealthTillShellBreak2
				float fChange = (float)(m_iMinHealthTillShellBreak2 - fCurrHealth);
				ChangeHealth(fChange, "Keeping gladiator general above threshold till shell-break 2 done");
			}
			break;
		case 2:
			if(fCurrHealth < m_iMinHealthTillShellBreak3)
			{
				//Set current health to m_iMinHealthTillShellBreak3
				float fChange = (float)(m_iMinHealthTillShellBreak3 - fCurrHealth);
				ChangeHealth(fChange, "Keeping gladiator general above threshold till shell-break 3 done");
			}
			break;
		default:
			ntError_p(false, ("Invalid m_iSmashAttackPhaseNumber"));	//This shouldn't ever happen unless somehow the value was negative.
			break;
		}
	}
}


//--------------------------------------------------
//!
//! GladiatorGeneralRollSpecial 
//! Gladiator generals special where he rolls towards you, and hopefully a pillar too
//!
//--------------------------------------------------
GladiatorGeneralRollSpecial::GladiatorGeneralRollSpecial()
{
	m_iRollsRemaining = -1;

	m_iMaxNumberOfTrackingRolls = 3;
	m_iMaxNumberOfTrackingRollsAdjust = 1;

	m_pobRollStart = m_pobRollTrack = m_pobRollAttack = m_pobRollHitPillar = m_pobRollGlancePillarLeft = m_pobRollGlancePillarRight = 0;
	m_pobFromStunnedRecoil = 0;

	m_pobInteractionWith = 0;
	m_bHitPillar = m_bHeadingTowardsPillar = false;
	m_bAttackEndTriggered = false;
	m_fPillarGlanceAngle = 40.0f;
	m_fDamageOnHitPillar = 25.0f;
	m_fDamageOnGlanceOffPillar = 15.0f;
}

bool GladiatorGeneralRollSpecial::IsVulnerableTo(CStrike* pobStrike)
{
	return IsVulnerableTo(pobStrike->GetAttackDataP());
}

bool GladiatorGeneralRollSpecial::IsVulnerableTo(const CAttackData* pobAttackData)
{
	return (m_bHitPillar || (m_pobBoss->GetAttackComponent()->IsInInvulnerabilityWindow() <= 0));
//	return m_bHitPillar;
}

bool GladiatorGeneralRollSpecial::Initialise(Boss* pobBoss, CEntity* pobPlayer)
{
	// Make sure we've got all the attack links we need, if not, bail time
	ntError_p(pobBoss->GetBossType() == Boss::BT_GLADIATOR_GENERAL, ("GladiatorGeneralRollSpecial should only be on gladiator general!"));
	ntError(m_pobRollStart && m_pobRollTrack && m_pobRollAttack && m_pobRollHitPillar && m_pobRollGlancePillarLeft && m_pobRollGlancePillarRight && m_pobFromStunnedRecoil);
	GladiatorGeneral* pGGen = (GladiatorGeneral*)pobBoss;

	if(pGGen->IsInSpecialAttack())
	{
		ntPrintf("***###*** GladiatorGeneralRollSpecial: SHOULD NOT HAVE HAPPENED (attempting to start special attack while already in one). Check attack-priorities (data) ***###***\n");
		return false;
	}

	m_iRollsRemaining = m_iMaxNumberOfTrackingRolls;
	m_iRollsRemaining -= grand() % (m_iMaxNumberOfTrackingRollsAdjust + 1);
	if (m_iRollsRemaining <= 0)
		m_iRollsRemaining = 1;
	
	m_pobInteractionWith = 0;
	m_bHitPillar = m_bHeadingTowardsPillar = false;

	pobBoss->GetBossMovement()->m_obTargetPoint = pobPlayer->GetPosition();
	pobBoss->GetBossMovement()->m_bTargetPointSet = true;
	
	m_pobBoss = pobBoss;
	m_bAttackEndTriggered = false;

	//Set that the gladiator general isn't currently hit while stunned, or needs notification of it (only when he's hit a pillar).
	pGGen->m_bNotifyIfHitStunned = false;
	pGGen->SetHitWhileStunned(false);

	// Kick off roll start - will link into roll cycle
	if (pobBoss->GetAttackComponent()->Boss_Command_RequestDirectAttack(m_pobRollStart))
	{			
		pobBoss->GetMessageHandler()->ReceiveMsg<msg_buttonattack>();
		pGGen->SetInSpecialAttack(true);

#ifdef GGEN_ATTACK_TIMEOUT_SAFETY
		m_fAttackTimeoutTimer = 0.0f;
#endif
		// Activate reverse camera angle mode
		CamView* pobPrimaryCameraView = CamMan::Get().GetPrimaryView();
		pobPrimaryCameraView->ActivateBossCameraReverseAngle( "Main_CoolCam_GladiatorGeneralDef_1" );

		return true;
	}

	return false;
}

void GladiatorGeneralRollSpecial::NotifyAttackFinished()
{
	m_iRollsRemaining--;
#ifdef GGEN_ATTACK_TIMEOUT_SAFETY
	m_fAttackTimeoutTimer = 0.0f;
#endif
}

void GladiatorGeneralRollSpecial::NotifyInteractionWith(CEntity* pobEnt)
{
	// This'll be one of our pillars - if it's not then something heinous has happened
	m_pobInteractionWith = pobEnt;
}

void GladiatorGeneralRollSpecial::NotifyAttackAutoLinked()
{
	m_iRollsRemaining--;
#ifdef GGEN_ATTACK_TIMEOUT_SAFETY
	m_fAttackTimeoutTimer = 0.0f;
#endif
}


BossAttack* GladiatorGeneralRollSpecial::NotifyAttackInterrupted()
{
	GladiatorGeneral* pobGGen = (GladiatorGeneral*)m_pobBoss;

	//Remove all pillars from notification list
	for (ntstd::List<CEntity*>::iterator obIt = pobGGen->m_obPillars.begin();
		obIt != pobGGen->m_obPillars.end();
		obIt++)
	{
		pobGGen->GetInteractionComponent()->StopNotifyOnInteractionWith(*obIt);
	}

	//Set/Reset any flags to reflect this
	pobGGen->SetInSpecialAttack(false);
	pobGGen->m_bNotifyIfHitStunned = false;
	pobGGen->SetHitWhileStunned(false);
	// Turn off reverse camera angle mode
	CamView* pobPrimaryCameraView = CamMan::Get().GetPrimaryView();
	pobPrimaryCameraView->DeactivateBossCameraReverseAngle( "Main_CoolCam_GladiatorGeneralDef_1" );
	//Return 0 (so we can move on).
	return 0;
}

BossAttack* GladiatorGeneralRollSpecial::Update(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	GladiatorGeneral* pobGGen = (GladiatorGeneral*)pobBoss;

	//Update whether or not we need to be notified of "hit while stunned"...
	if(!pobGGen->m_bNotifyIfHitStunned)
	{
		//Make sure this hit-while-stunned flag stays false during this attack if the ggen isn't stunned from hitting the pillar.
		pobGGen->SetHitWhileStunned(false);
	}

	// Keep the boss facing the player during autolinked rolls
	if (m_iRollsRemaining >= 0 )
	{
		pobGGen->GetBossMovement()->m_obTargetPoint = pobPlayer->GetPosition();
		pobGGen->GetBossMovement()->m_bTargetPointSet = true;

		// Set reverse camera angle lead-in
		CamView* pobPrimaryCameraView = CamMan::Get().GetPrimaryView();
		pobPrimaryCameraView->ActivateReverseAngleLeadIn( "Main_CoolCam_GladiatorGeneralDef_1" );
	}
	// And attacks towards pillars
	else if (m_bHeadingTowardsPillar)
	{
		CDirection obFromGGenToPillar = CDirection( m_obHitPillarPosition - pobGGen->GetPosition() );
		obFromGGenToPillar.Normalise();
		pobBoss->GetBossMovement()->m_obFacingDirection = obFromGGenToPillar;
		pobBoss->GetBossMovement()->m_obMoveDirection = obFromGGenToPillar;
		pobGGen->GetBossMovement()->m_obTargetPoint = m_obHitPillarPosition;
		pobGGen->GetBossMovement()->m_bTargetPointSet = true;
		// deactive reverse angle lead in
		CamView* pobPrimaryCameraView = CamMan::Get().GetPrimaryView();
		pobPrimaryCameraView->DeactivateReverseAngleLeadIn( "Main_CoolCam_GladiatorGeneralDef_1" );
	}
	else if (m_bHitPillar)
	{
		CDirection obFacingDirection = pobGGen->GetMatrix().GetZAxis(); obFacingDirection.Normalise();
		pobBoss->GetBossMovement()->m_obFacingDirection = obFacingDirection;
		pobBoss->GetBossMovement()->m_bTargetPointSet = false;
		// deactive reverse angle lead in
		CamView* pobPrimaryCameraView = CamMan::Get().GetPrimaryView();
		pobPrimaryCameraView->DeactivateReverseAngleLeadIn( "Main_CoolCam_GladiatorGeneralDef_1" );
	}
	else // But not otherwise
	{	
        pobGGen->GetBossMovement()->m_bTargetPointSet = false;
		// deactive reverse angle lead in
		CamView* pobPrimaryCameraView = CamMan::Get().GetPrimaryView();
		pobPrimaryCameraView->DeactivateReverseAngleLeadIn( "Main_CoolCam_GladiatorGeneralDef_1" );
	}

	// Check for pillar interaction
	if (m_pobInteractionWith && !m_bHitPillar)
	{
		// Note the position
		m_obHitPillarPosition = m_pobInteractionWith->GetPosition();

		//Get the angle at which we're hitting the pillar relative to our current direction. We may just glance off.
		CDirection obFromGGenToPillar = CDirection( m_pobInteractionWith->GetPosition() - pobGGen->GetPosition() );
		obFromGGenToPillar.Normalise();
		CDirection obCurrentlyMoving = pobGGen->GetMatrix().GetZAxis();

		//Angle in degrees.
		float fAngle = (float)((CMaths::SafeAcosf(obCurrentlyMoving.Dot(obFromGGenToPillar))) * (180.0f / 3.14159265358979323846f));

		if(fAngle < m_fPillarGlanceAngle)	//We collide directly and break the pillar below this angle.
		{
			// Collapse the pillar. Now that they're Animated_Collapsable objects this just involves sending a msg_smash message to them.
			if(m_pobInteractionWith && m_pobInteractionWith->GetMessageHandler())
			{
				Message obSmashMessage(msg_smash);
				m_pobInteractionWith->GetMessageHandler()->Receive(obSmashMessage);
			}

			//Apply a velocity to every component (so that breaks better/tips over).
			if(m_pobInteractionWith && m_pobInteractionWith->GetPhysicsSystem())
			{
				Physics::AnimatedLG* pLG = (Physics::AnimatedLG*)m_pobInteractionWith->GetPhysicsSystem()->GetFirstGroupByType(Physics::LogicGroup::ANIMATED_LG);
				if(pLG)
				{
					//Get each of the elements in this animated LG and apply a collision-relative velocity to it.
					ntstd::List<Physics::Element*>* pElementList = &pLG->GetElementList();

					for(ntstd::List<Physics::Element*>::iterator obIt = pElementList->begin() ; obIt != pElementList->end() ; obIt++)
					{
						//FIXME: This is a horrible hacky data-specific implementation, where we assume that the first rigid body is
						//the base of the pillar which shouldn't move. Ideally, later when our physics format supports mixed static/dynamic
						//actors, it will just be flagged differently (which is what the HS_MOTION_FIXED check below was supposed to catch).
						//For the meantime this works, but any data changes could result in very very bad wierdness with the pillars.
						//(In particular, setting a velocity on the base makes it float off at that velocity like a balloon...)
						if(obIt == pElementList->begin())
						{
							continue;
						}

						//This applies a force to all of the pieces.
						//Note: The pillars are only supposed to explode out of the arena, which for the current media is along -x
						//for the orientation of the pillar (all of them). Because of this, and to make it look like it's still based
						//on the angle of impact, we lerp part way between the angle the GGen hits the pillar at, and -x on the pillar,
						//so the pieces will always fly out of the arena.
						Physics::RigidBody* pRigidBody = (Physics::RigidBody*)(*obIt);
						if(pRigidBody && (pRigidBody->GetMotionType() != Physics::HS_MOTION_FIXED))
						{
							CDirection obExplosionDirection = CDirection::Lerp(obCurrentlyMoving, -m_pobInteractionWith->GetMatrix().GetXAxis(), 0.4f);
							float fRandX = grandf(1.0f) - 0.5f;
							float fRandY = grandf(1.0f) + 1.0f;
							obExplosionDirection += CDirection(fRandX, fRandY, 0.0f) * pobGGen->GetMatrix();
							obExplosionDirection.Normalise();
							obExplosionDirection *= (float)(grandf(4.0f) + 4.0f);
							pRigidBody->SetLinearVelocity(CVector(obExplosionDirection));
						}
					}
				}
			}

			// Remove it from our notifications
			pobGGen->GetInteractionComponent()->StopNotifyOnInteractionWith(m_pobInteractionWith);

			// ... and our internal list
			for (ntstd::List<CEntity*>::iterator obIt = pobGGen->m_obPillars.begin();
				obIt != pobGGen->m_obPillars.end();
				obIt++)
			{
				if ((*obIt) == m_pobInteractionWith)
				{
					pobGGen->m_obPillars.erase(obIt);
					break;
				}
			}

			// Set the directions
			pobBoss->GetBossMovement()->m_obFacingDirection = obFromGGenToPillar;
			pobBoss->GetBossMovement()->m_obMoveDirection = obFromGGenToPillar;

			// Start our roll pillar hit thing
			if (pobGGen->GetAttackComponent()->Boss_Command_RequestDirectAttack(m_pobRollHitPillar,true))
			{			
				pobGGen->GetMessageHandler()->ReceiveMsg<msg_buttonattack>();
				m_bAttackEndTriggered = true;
			}

			//Take direct-hit damage.
			pobGGen->ChangeHealth(-m_fDamageOnHitPillar, "Dead-hit on pillar damage");
		}
		else
		{
			//We want to glance off instead, left, or right?
			CDirection obGGenLeft = pobGGen->GetMatrix().GetXAxis();
			CDirection obGGenRight = -pobGGen->GetMatrix().GetXAxis();
			bool bGlanceRight = (obGGenLeft.Dot(obFromGGenToPillar) > obGGenRight.Dot(obFromGGenToPillar)) ? true : false;

			if(bGlanceRight)
			{
				ntPrintf("***Glancing right on pillar collision***\n");
				// Start our right-glancing-blow hit thing
				if (pobGGen->GetAttackComponent()->Boss_Command_RequestDirectAttack(m_pobRollGlancePillarRight,true))
				{			
					pobGGen->GetMessageHandler()->ReceiveMsg<msg_buttonattack>();
					m_bAttackEndTriggered = true;
				}
			}
			else
			{
				ntPrintf("***Glancing left on pillar collision***\n");
				// Start our left-glancing-blow hit thing
				if (pobGGen->GetAttackComponent()->Boss_Command_RequestDirectAttack(m_pobRollGlancePillarLeft,true))
				{			
					pobGGen->GetMessageHandler()->ReceiveMsg<msg_buttonattack>();
					m_bAttackEndTriggered = true;
				}
			}

			//Take glance-off damage.
			pobGGen->ChangeHealth(-m_fDamageOnGlanceOffPillar, "Glancing off of pillar damage");
		}

		m_bHitPillar = true;
		pobGGen->m_bNotifyIfHitStunned = true;
	}

#ifdef GGEN_ATTACK_TIMEOUT_SAFETY
	m_fAttackTimeoutTimer += fTimeDelta;

	if(m_iRollsRemaining > 0 && !m_bAttackEndTriggered && (m_fAttackTimeoutTimer >= 6.0f))	//6-second timeout (a few rolls)
	{
		//We've timed-out on the auto-linking rolls... force the GGen into the actual roll-attack.
		ntPrintf("***###*** GladiatorGeneralRollSpecial: TIMEOUT - Saved by SUPERHAX! (forcing roll-attack) ***###***\n");
		m_fAttackTimeoutTimer = 0.0f;
		m_iRollsRemaining = -1;	//The attack should then just force itself into the roll special. Only possibly hangs during roll-links.
	}
#endif

	// If we've finished rolling, start attack
	if (m_iRollsRemaining <= 0 && !m_bAttackEndTriggered)
	{
		// See if we're heading towards a pillar, if we are, orient directly towards it
		// Save time and add pillars to notification list at the same time as we do this
		for (ntstd::List<CEntity*>::iterator obIt = pobGGen->m_obPillars.begin();
			obIt != pobGGen->m_obPillars.end();
			obIt++)
		{
			// Add to notification
			pobGGen->GetInteractionComponent()->SetupNotifyOnInteractionWith(*obIt);

			// Look ahead in a small cone 10m long
			if (pobBoss->GetAwarenessComponent()->IsEntityInZone(pobBoss,*obIt,0.0f,25.0f,0.0f,10.0f))
			{
				// Set up movement so we head towards this pillar exactly
				pobGGen->GetBossMovement()->m_obTargetPoint = (*obIt)->GetPosition();
				pobGGen->GetBossMovement()->m_bTargetPointSet = true;
				CDirection obFromGGenToPillar = CDirection( (*obIt)->GetPosition() - pobGGen->GetPosition() );
				obFromGGenToPillar.Normalise();
				pobBoss->GetBossMovement()->m_obFacingDirection = obFromGGenToPillar;
				pobBoss->GetBossMovement()->m_obMoveDirection = obFromGGenToPillar;
				m_bHeadingTowardsPillar = true;
				m_obHitPillarPosition = (*obIt)->GetPosition();
			}
		}

		// Queue roll attack as next one
		if (pobGGen->GetAttackComponent()->Boss_Command_RequestDirectAttack(m_pobRollAttack,true))
		{			
			pobGGen->GetMessageHandler()->ReceiveMsg<msg_buttonattack>();
			m_bAttackEndTriggered = true;
		}

		m_iRollsRemaining--;
	}

	if(m_iRollsRemaining < 0 && m_bAttackEndTriggered && pobGGen->IsHitWhileStunned())
	{
		//Queue stunned recoil as next attack.
		ntPrintf("***###*** GLADIATOR GENERAL HIT AFTER HITTING PILLAR***###***\n");
		if(pobGGen->GetAttackComponent()->Boss_Command_RequestDirectAttack(m_pobFromStunnedRecoil, true))
		{
			pobGGen->GetMessageHandler()->ReceiveMsg<msg_buttonattack>();
			pobGGen->SetHitWhileStunned(false);
			pobGGen->m_bNotifyIfHitStunned = false;
		}
	}

	// Have we finished everything?
	if (m_iRollsRemaining < 0 && pobBoss->CanStartAnAttack() && m_bAttackEndTriggered)
	{
		// Remove all pillars from notification list
		for (ntstd::List<CEntity*>::iterator obIt = pobGGen->m_obPillars.begin();
			obIt != pobGGen->m_obPillars.end();
			obIt++)
		{
			pobGGen->GetInteractionComponent()->StopNotifyOnInteractionWith(*obIt);
		}

		// We've finished
		pobGGen->SetInSpecialAttack(false);
		pobGGen->SetHitWhileStunned(false);
		pobGGen->m_bNotifyIfHitStunned = false;
		// Turn off reverse camera angle mode
		CamView* pobPrimaryCameraView = CamMan::Get().GetPrimaryView();
		pobPrimaryCameraView->DeactivateBossCameraReverseAngle( "Main_CoolCam_GladiatorGeneralDef_1" );
		return 0;
	}

	return this;
}

void GladiatorGeneralRollSpecial::DebugRender(CPoint& obScreenLocation, float fXOffset, float fYOffset)
{
#ifndef _GOLD_MASTER
	if (m_bHitPillar)
		g_VisualDebug->Printf3D(obScreenLocation,fXOffset,fYOffset,DC_CYAN,0,"Roll Special %s: hit pillar", ObjectDatabase::Get().GetNameFromPointer(this).GetString(), m_iRollsRemaining );
	else
		g_VisualDebug->Printf3D(obScreenLocation,fXOffset,fYOffset,DC_CYAN,0,"Roll Special %s: %i rolls remaining", ObjectDatabase::Get().GetNameFromPointer(this).GetString(), m_iRollsRemaining );
#endif
}

//--------------------------------------------------
//!
//! GladiatorGeneralDefensiveSpinSpecial 
//! Gladiator generals special where he spins defensively and jumps away after a while
//!
//--------------------------------------------------
GladiatorGeneralDefensiveSpinSpecial::GladiatorGeneralDefensiveSpinSpecial()
{
	m_iRollsRemaining = -1;
	m_iCurrentChosenPoint = 0;

	m_iMaxNumberOfRolls = 3;
	m_iMaxNumberOfRollsAdjust = 1;

	m_pobRollStart = m_pobRoll = m_pobRollLeap = 0;
	m_bAttackEndTriggered = false;
	m_pobBoss = 0;
}

bool GladiatorGeneralDefensiveSpinSpecial::Initialise(Boss* pobBoss, CEntity* pobPlayer)
{
	GladiatorGeneral* pobGGen = (GladiatorGeneral*)pobBoss;

	if(pobGGen->IsInSpecialAttack())
	{
		ntPrintf("***###*** GladiatorGeneralDefensiveSpinSpecial: SHOULD NOT HAVE HAPPENED (attempting to start special attack while already in one). Check attack-priorities (data)  ***###***\n");
		return false;
	}

	m_iCurrentChosenPoint = 0;

	//Choose our target point as the one furthest from where the player currently is.
	CPoint obPlayerPos = pobPlayer->GetPosition();
	float fBestDist = 0.0f;
	for(int i = 0 ; i < GGEN_NUM_DEFENSIVE_SPIN_TARGET_POINTS ; i++)
	{
		CDirection obBetween(pobGGen->m_aobDefensiveSpinTargetPoints[i] - obPlayerPos);
		float fDist = obBetween.LengthSquared();
		if(fDist > fBestDist)
		{
			fBestDist = fDist;
			m_iCurrentChosenPoint = i;
		}
	}

	ntError_p(((m_iCurrentChosenPoint >= 0) && (m_iCurrentChosenPoint < GGEN_NUM_DEFENSIVE_SPIN_TARGET_POINTS)), ("Invalid target point chosen"));

	//Even if we're going to jump-to-point, we keep facing the player-position during initialisation and the first few spins to avoid
	//a rotation pop.
	pobGGen->GetBossMovement()->m_obTargetPoint = pobPlayer->GetPosition();
	pobGGen->GetBossMovement()->m_bTargetPointSet = true;
	m_pobBoss = pobGGen;
	m_bLeaping = false;
	m_bAttackEndTriggered = false;

	m_iRollsRemaining = m_iMaxNumberOfRolls;
	m_iRollsRemaining -= grand() % (m_iMaxNumberOfRollsAdjust + 1);
	if (m_iRollsRemaining <= 0)
		m_iRollsRemaining = 1;
	
	// Kick off rolling
	if (pobGGen->GetAttackComponent()->Boss_Command_RequestDirectAttack(m_pobRollStart))
	{			
		pobGGen->GetMessageHandler()->ReceiveMsg<msg_buttonattack>();
		pobGGen->SetInSpecialAttack(true);

#ifdef GGEN_ATTACK_TIMEOUT_SAFETY
		m_fAttackTimeoutTimer = 0.0f;
#endif

		return true;
	}

	return false;
}

void GladiatorGeneralDefensiveSpinSpecial::NotifyAttackFinished()
{
	m_iRollsRemaining--;
#ifdef GGEN_ATTACK_TIMEOUT_SAFETY
	m_fAttackTimeoutTimer = 0.0f;
#endif
}

void GladiatorGeneralDefensiveSpinSpecial::NotifyAttackAutoLinked()
{
	m_iRollsRemaining--;
#ifdef GGEN_ATTACK_TIMEOUT_SAFETY
	m_fAttackTimeoutTimer = 0.0f;
#endif
}


BossAttack* GladiatorGeneralDefensiveSpinSpecial::NotifyAttackInterrupted()
{
	ntPrintf("*** GladiatorGeneralDefensiveSpinSpecial::NotifyAttackInterrupted() - Should this ever happen? Supposed to invulnerable throughout. Will hang? ***\n");
	GladiatorGeneral* pobGGen = (GladiatorGeneral*)m_pobBoss;

	//Maybe don't do this after all. Shouldn't need to if a successful dodge attack is interrupting this one anyway.
	if (!m_bAttackEndTriggered)
	{
		// Force a leap time
		if (m_pobBoss->GetAttackComponent()->Boss_Command_RequestDirectAttack(m_pobRollLeap,true))
		{			
			pobGGen->GetMessageHandler()->ReceiveMsg<msg_buttonattack>();

			m_bLeaping = true;	//So that we suddenly face the point we want to jump on instead of the player.
			m_iRollsRemaining = -1;
			m_bAttackEndTriggered = true;
		}
	}
	pobGGen->SetInSpecialAttack(false);
	return 0;	//this?
}

#define GGEN_DEFENSIVESPINSPECIAL_LANDONPLAYER	//Uncomment this to have the defensive spin land on the player instead of a point.

BossAttack* GladiatorGeneralDefensiveSpinSpecial::Update(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	GladiatorGeneral* pobGGen = (GladiatorGeneral*)pobBoss;
	if(m_bLeaping)
	{
#ifdef GGEN_DEFENSIVESPINSPECIAL_LANDONPLAYER
		pobBoss->GetBossMovement()->m_obTargetPoint = pobPlayer->GetPosition();
		pobBoss->GetBossMovement()->m_bTargetPointSet = true;
#else
		pobBoss->GetBossMovement()->m_obTargetPoint = pobGGen->m_aobDefensiveSpinTargetPoints[m_iCurrentChosenPoint];
		pobBoss->GetBossMovement()->m_bTargetPointSet = true;
#endif
	}
	else
	{
		pobBoss->GetBossMovement()->m_obTargetPoint = pobPlayer->GetPosition();
		pobBoss->GetBossMovement()->m_bTargetPointSet = true;
	}

#ifdef GGEN_ATTACK_TIMEOUT_SAFETY
	m_fAttackTimeoutTimer += fTimeDelta;

	if(m_iRollsRemaining > 0 && !m_bAttackEndTriggered && (m_fAttackTimeoutTimer >= 6.0f))	//6-second time-out (a few spins).
	{
		//We've timed-out on the auto-linking spins... force the GGen into the actual leap.
		ntPrintf("***###*** GladiatorGeneralDefensiveSpecial: TIMEOUT - Saved by SUPERHAX! (forcing leap) ***###***\n");
		m_fAttackTimeoutTimer = 0.0f;
		m_iRollsRemaining = -1;	//The attack should then just force itself into the leap. Only possibly hangs during spin-links.
	}
#endif

	if (m_iRollsRemaining <= 0 && !m_bAttackEndTriggered)
	{
		// Force a leap time
		if (pobBoss->GetAttackComponent()->Boss_Command_RequestDirectAttack(m_pobRollLeap,true))
		{			
			pobBoss->GetMessageHandler()->ReceiveMsg<msg_buttonattack>();

			m_bLeaping = true;	//So that we suddenly face the point we want to jump on instead of the player.
			m_iRollsRemaining--;
			m_bAttackEndTriggered = true;
		}
	}

	if (m_iRollsRemaining < 0 && pobBoss->CanStartAnAttack() && m_bAttackEndTriggered)
	{
		// We've finished
		pobGGen->SetInSpecialAttack(false);
		return 0;
	}

	return this;
}

void GladiatorGeneralDefensiveSpinSpecial::DebugRender(CPoint& obScreenLocation, float fXOffset, float fYOffset)
{
#ifndef _GOLD_MASTER
	g_VisualDebug->Printf3D(obScreenLocation,fXOffset,fYOffset,DC_CYAN,0,"Defensive Spin %s: %i spins remaining", ObjectDatabase::Get().GetNameFromPointer(this).GetString(), m_iRollsRemaining );
#endif
}

//--------------------------------------------------
//!
//! GladiatorGeneralStompSpecial 
//! Gladiator generals special where he stomps repeatedly at the hero
//!
//--------------------------------------------------
GladiatorGeneralStompSpecial::GladiatorGeneralStompSpecial()
{
	m_iStompsRemaining = -1;
	m_iLastUsedStompIndex = -1;

	m_iMaxNumberOfStomps = 3;
	m_iMaxNumberOfStompsAdjust = 1;

	m_pobStomp[0] = m_pobStomp[1] = m_pobStomp[2] = m_pobStomp[3] = 0;
	m_fMinStompedNearDistance = 1.0f;
	m_fMaxStompedNearDistance = 3.0f;
}

bool GladiatorGeneralStompSpecial::Initialise(Boss* pobBoss, CEntity* pobPlayer)
{
	ntError_p( m_pobStomp[0] && m_pobStomp[1] && m_pobStomp[2] && m_pobStomp[3], ("One or more stomp-attack links was missing") );
	ntError_p( !ntStr::IsNull(m_obStompedNearPlayerAttackData), ("The stomp-near-player attack data was not provided"));

	GladiatorGeneral* pobGGen = (GladiatorGeneral*)pobBoss;

	if(pobGGen->IsInSpecialAttack())
	{
		ntPrintf("***###*** GladiatorGeneralStompSpecial: SHOULD NOT HAVE HAPPENED (attempting to start special attack while already in one). Check attack-priorities (data) ***###***\n");
		return false;
	}

	m_iStompsRemaining = m_iMaxNumberOfStomps;
	m_iStompsRemaining -= grand() % (m_iMaxNumberOfStompsAdjust + 1);
	if (m_iStompsRemaining <= 0)
		m_iStompsRemaining = 1;
	
	//Make sure we're not flagged as having just finished a stomp (possibly from the last attack).
	pobGGen->m_bJustFinishedStomp = false;

	// Kick off stomping with a random one
	int iStompIndex = grand() % 4;
	m_iLastUsedStompIndex = iStompIndex;
	if (pobGGen->GetAttackComponent()->Boss_Command_RequestDirectAttack(m_pobStomp[iStompIndex]))
	{			
		pobGGen->GetMessageHandler()->ReceiveMsg<msg_buttonattack>();
		pobGGen->SetInSpecialAttack(true);

#ifdef GGEN_ATTACK_TIMEOUT_SAFETY
		m_fAttackTimeoutTimer = 0.0f;
#endif

		return true;
	}

	return false;
}

void GladiatorGeneralStompSpecial::NotifyAttackFinished()
{
	m_iStompsRemaining--;
#ifdef GGEN_ATTACK_TIMEOUT_SAFETY
	m_fAttackTimeoutTimer = 0.0f;
#endif
}

void GladiatorGeneralStompSpecial::NotifyAttackAutoLinked()
{
	m_iStompsRemaining--;
#ifdef GGEN_ATTACK_TIMEOUT_SAFETY
	m_fAttackTimeoutTimer = 0.0f;
#endif
}

BossAttack* GladiatorGeneralStompSpecial::Update(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	GladiatorGeneral* pobGGen = (GladiatorGeneral*)pobBoss;

	pobGGen->GetBossMovement()->m_obTargetPoint = pobPlayer->GetPosition();
	pobGGen->GetBossMovement()->m_bTargetPointSet = true;
	pobGGen->GetBossMovement()->m_fMoveSpeed = 0.0f;

#ifdef GGEN_ATTACK_TIMEOUT_SAFETY
	m_fAttackTimeoutTimer += fTimeDelta;

	if(m_fAttackTimeoutTimer >= 7.0f)	//7 seconds since the last stomp seems like plenty.
	{
		//We've timed-out... end the attack gracefully.
		ntPrintf("***###*** GladiatorGeneralStompSpecial: TIMEOUT - Saved by SUPERHAX! ***###***\n");
		pobGGen->SetInSpecialAttack(false);
		m_fAttackTimeoutTimer = 0.0f;
		return 0;
	}
#endif

	if (m_iStompsRemaining < 0 && pobGGen->CanStartAnAttack())
	{
		// We've finished
		pobGGen->SetInSpecialAttack(false);
		return 0;
	}
	else if (m_iStompsRemaining >= 0 && pobGGen->CanStartALinkedAttack())
	{
		int iStompIndex = grand() % 4;
		while (iStompIndex == m_iLastUsedStompIndex)
		{
			iStompIndex = grand() % 4;
		}

		if (pobBoss->GetAttackComponent()->Boss_Command_RequestDirectNextAttack(m_pobStomp[iStompIndex]))
		{
			pobGGen->GetMessageHandler()->ReceiveMsg<msg_buttonattack>();
			m_iStompsRemaining--;
#ifdef GGEN_ATTACK_TIMEOUT_SAFETY
			m_fAttackTimeoutTimer = 0.0f;
#endif
		}
	}

	if(pobGGen->m_bJustFinishedStomp)
	{
		//Generate a near-stomp strike on the player (to stagger them) if they're within a certain distance threshold.
		CPoint obGGenPos = pobBoss->GetPosition();
		CPoint obPlayerPos = pobPlayer->GetPosition();
		CDirection obBetween = CDirection(obGGenPos - obPlayerPos);
		float fProximity = obBetween.Length();
		if((fProximity >= m_fMinStompedNearDistance) && (fProximity <= m_fMaxStompedNearDistance))
		{
			//Generate the strike (should have data to always put the player into stagger (unless evading of course)).
			if(!ntStr::IsNull(m_obStompedNearPlayerAttackData))
			{
				CombatHelper::Combat_GenerateStrike(pobBoss, pobBoss, pobPlayer, m_obStompedNearPlayerAttackData);
			}
		}
		//Clear our just-stomped flag anyway.
		pobGGen->m_bJustFinishedStomp = false;
	}

	return this;
}

void GladiatorGeneralStompSpecial::DebugRender(CPoint& obScreenLocation, float fXOffset, float fYOffset)
{
#ifndef _GOLD_MASTER
	g_VisualDebug->Printf3D(obScreenLocation,fXOffset,fYOffset,DC_CYAN,0,"Stomp %s: %i stomps remaining", ObjectDatabase::Get().GetNameFromPointer(this).GetString(), m_iStompsRemaining );
#endif
}


//------------------------------------------------------------------------------------------
//!
//!	GGenSmashAttackSelector
//! Class for selecting the gladiator general's smash-attack.
//!
//------------------------------------------------------------------------------------------
GGenSmashAttackSelector::GGenSmashAttackSelector()
{
	//Clear the values if none have been provided by xml/welder.
	m_fMinDistance = 0.0f;
	m_fMaxDistance = 0.0f;
	m_fMinSafeDistance = 3.0f;
}


//--------------------------------------------------
//!
//! GetPriority 
//! Checks whether a smash-attack is a viable selection at this point.
//!
//--------------------------------------------------
float GGenSmashAttackSelector::GetPriority(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	//Error if the boss this is on is not a gladiator general...
	ntError_p( pobBoss->GetBossType() == Boss::BT_GLADIATOR_GENERAL, ("Who put a GGenSmashAttackSelector on something other than the GGen!?!?") );

	GladiatorGeneral* pGGen = static_cast<GladiatorGeneral*>(pobBoss);

	//If we're in the middle of a special attack already, then we don't want to select this.
	if(pGGen->IsInSpecialAttack())
	{
		return m_fMinPriority;
	}

	//First of all we query the general's current health against that of it's fixed-points to make sure it's at one of those.
	//We use the current smash attack phase number to specify a health (rather than just returning max-priority if at any of
	//the shell-break healths) so that after completing a shell-break attack (which may or may-not do damage later, currently doesn't),
	//max-priority will not be returned if still at the same shell-break health (as now m_iSmashAttackPhaseNumber will have moved on).
	float fCurrHealth = pGGen->ToCharacter()->GetCurrHealth();
	int fDesiredHealthLimit = 0;
	switch(pGGen->m_iSmashAttackPhaseNumber)
	{
	case 0:
		fDesiredHealthLimit = pGGen->m_iMinHealthTillShellBreak1;
		break;
	case 1:
		fDesiredHealthLimit = pGGen->m_iMinHealthTillShellBreak2;
		break;
	case 2:
		fDesiredHealthLimit = pGGen->m_iMinHealthTillShellBreak3;
		break;
	default:
		//We've finished all the shell-breaks, the fight should be over, so just make sure we don't select a smash-attack here!
		return m_fMinPriority;
		break;
	}

	if(fCurrHealth == fDesiredHealthLimit)
	{
		//If we're at one of those critical healths, then we want to smash-attack if the player is within a specific range
		//Get the distance from the player to the boss.
		CPoint obPlayerPosition = pobPlayer->GetPosition();
		CPoint obBossPosition = pobBoss->GetPosition();
		CDirection obBossToPlayer = CDirection( obPlayerPosition - obBossPosition );
		float fDistance = obBossToPlayer.Length();

		if((fDistance >= m_fMinDistance) && (fDistance <= m_fMaxDistance))
		{
			//First of all we check that the player is within a certain angle of the gladiator general's forward vector. If not, we no
			//longer want to perform this attack (as the snap-to-angle thing no longer happens courtesy of Duncan).
			CDirection obAngleCheck(obBossToPlayer);
			obAngleCheck.Normalise();
			float fAngle = (float)(CMaths::SafeAcosf(obAngleCheck.Dot(pobBoss->GetMatrix().GetZAxis())) * (180.0f / 3.14159265358979323846f));
			//If we're over 10 degrees to either side then it's a no-go.
			if(fAngle > 10.0f)
			{
				return m_fMinPriority;
			}

			//As a final check, we want to make sure that there's enough room in-front of the GGen (where he'll be leaping towards
			//the player) that he can jump, get stuck in the ground, the player run over his back and break-off shell-pieces, and all
			//without anyone hitting any static geometry (which could make it look really rubbish!)
			bool bSafe = false;

			CPoint obPositionToCheck(CONSTRUCT_CLEAR);
			CPoint obGGenPos = pGGen->GetPosition();
			CDirection obJumpDir = CDirection(pobPlayer->GetPosition() - pGGen->GetPosition());
			obJumpDir.Y() = 0.0f;	//Flat horizontal direction from the GGen's root toward the player on x/z.
			obJumpDir.Normalise();
			//Reposition our box to be half the safe distance along the GGen's forward-vector.
			obPositionToCheck = obGGenPos + CPoint(obJumpDir * ((m_fMinSafeDistance + 2) / 2.0f));
			//Lift the box off of the floor so that it doesn't collide with that (after box extents)!
			obPositionToCheck.Y() += 0.6f;

			//Get the y-rotation of the GGen (which will be the rotation for our box phantom).
			float fGGenXRot, fGGenYRot, fGGenZRot;
			CCamUtil::EulerFromMat_XYZ(pGGen->GetMatrix(), fGGenXRot, fGGenYRot, fGGenZRot);	//TODO: EulerFromMat_ZYX ?
			//Now we want to convert this to be directly toward the player. To do this we can just take the GGen's
			//forward vector and the vector to the player, get the angle between, and add/subtract it from the GGen's
			//rotation to get a world-space rotation for that direction (on y).
			CDirection obGGenForward = pGGen->GetMatrix().GetZAxis();
			//Get the angle between in radians.
			float fAngleBetween = CMaths::SafeAcosf(obGGenForward.Dot(obJumpDir));

			float fFinalAngle = 0.0f;

			//Is this angle to be added-on or subtracted? (Add to the left, subtract to the right)
			CDirection obGGenLeft = pGGen->GetMatrix().GetXAxis();
			CDirection obGGenRight = -obGGenLeft;
			bool bRight = (obGGenLeft.Dot(obJumpDir) > obGGenRight.Dot(obJumpDir)) ? false : true;
			if(bRight)
			{
				fFinalAngle = fGGenYRot - fAngleBetween;
			}
			else
			{
				fFinalAngle = fGGenYRot + fAngleBetween;
			}

			//Now calculate the half-extends of the box (being sure to keep it off of the ground by enough!)
			CPoint obBoxHalfExtents(0.8f, 0.3f, m_fMinSafeDistance);

		
			//Now check to see if our phantom is intersecting any static geometry. If it is, then it's not safe to perform the smash
			//attack (or at least it's not safe for the player to perform the shell-break on failed smash-attack!)
			if(!Physics::CPhysicsWorld::Get().IsBoxIntersectingStaticGeometry(obPositionToCheck, obBoxHalfExtents, fFinalAngle))
			{
				bSafe = true;
			}

			//Draw the box so we can see the check. Render in a different colour if safe than if not safe.
#ifndef _GOLD_MASTER
			CQuat obDebugRenderOBBOrientation(CDirection(0.0f, 1.0f, 0.0f), fFinalAngle);
			if(bSafe)
			{
				g_VisualDebug->RenderOBB(obDebugRenderOBBOrientation, obPositionToCheck, CDirection(obBoxHalfExtents), 0xffffffff, DPF_WIREFRAME);
			}
			else
			{
				g_VisualDebug->RenderOBB(obDebugRenderOBBOrientation, obPositionToCheck, CDirection(obBoxHalfExtents), 0xffff0000, DPF_WIREFRAME);
			}
#endif
			if(bSafe)
			{
				return m_fMaxPriority;
			}
		}
	}

	//Otherwise, the conditions weren't met, so return minimum priority.
	return m_fMinPriority;
}


BossAttack* GGenSmashAttackSelector::BeginAttack(Boss* pobBoss, CEntity* pobPlayer)
{
	if (m_obAttacks.size() > 0)
	{
		int iAttack = grand() % m_obAttacks.size();
		int i = 0;
		ntstd::List<BossAttack*>::iterator obIt = m_obAttacks.begin();
		while (obIt != m_obAttacks.end())
		{
			if ((i == iAttack) && (*obIt)->Initialise(pobBoss, pobPlayer))
			{
				return (*obIt);
			}		

			i++;
			obIt++;
		}
	}

	return 0;
}


//--------------------------------------------------
//!
//! GladiatorGeneralSmashAttackSpecial 
//! Gladiator generals special where he performs a smash-attack down upon you and
//! gets stuck in the ground if he misses you. At this point he becomes vulnerable
//! to the shell-break attacks.
//!
//--------------------------------------------------
GladiatorGeneralSmashAttackSpecial::GladiatorGeneralSmashAttackSpecial()
{
	m_pobSmashAttack1 = m_pobSmashAttack2 = m_pobSmashAttack3 = 0;
	m_pobBoss = 0;
}


//--------------------------------------------------
//!
//! GladiatorGeneralSmashAttackSpecial::Initialise()
//! Sets up the internal variables for the gladiator general and chooses a suitable
//! attack-tree to run depending on which attack phase the gladiator general is
//! currently in (this allows us to have different struggle times that gradually
//! decrease in length as you successfully break off more of his shell).
//!
//--------------------------------------------------
bool GladiatorGeneralSmashAttackSpecial::Initialise(Boss* pobBoss, CEntity* pobPlayer)
{
	// Make sure we've got all the attack links we may need, if not, bail.
	ntError(m_pobSmashAttack1 && m_pobSmashAttack2 && m_pobSmashAttack3);
	ntError(pobBoss && pobPlayer);
	//Store the boss-pointer so we can debug-print info from the boss.
	m_pobBoss = pobBoss;

	pobBoss->GetBossMovement()->m_obTargetPoint = pobPlayer->GetPosition();
	pobBoss->GetBossMovement()->m_bTargetPointSet = true;
	m_bFinished = false;
	m_bVulnerableTo = false;

	GladiatorGeneral* pobGGen = (GladiatorGeneral*)pobBoss;

	if(pobGGen->IsInSpecialAttack())
	{
		ntPrintf("***###*** GladiatorGeneralSmashAttackSpecial: SHOULD NOT HAVE HAPPENED (attempting to start special attack while already in one) ***###***\n");
		return false;
	}

	//Kick off the attack with the appropriate smash attack link. Each will link into a seperate struggle, with differing lengths,
	//which is the whole point of this being a special attack!
	switch(pobGGen->GetSmashAttackPhase())
	{
	case 0:
		if(pobBoss->ToCharacter()->GetCurrHealth() == pobGGen->m_iMinHealthTillShellBreak1)
		{
			if (pobBoss->GetAttackComponent()->Boss_Command_RequestDirectAttack(m_pobSmashAttack1))
			{
				pobGGen->GetMessageHandler()->ReceiveMsg<msg_buttonattack>();
				pobGGen->SetInSpecialAttack(true);

				return true;
			}
			ntAssert_p(false, ("Failed to request direct attack m_pobSmashAttack1"));
		}
		break;
	case 1:
		if(pobBoss->ToCharacter()->GetCurrHealth() == pobGGen->m_iMinHealthTillShellBreak2)
		{
			if (pobBoss->GetAttackComponent()->Boss_Command_RequestDirectAttack(m_pobSmashAttack2))
			{
				pobGGen->GetMessageHandler()->ReceiveMsg<msg_buttonattack>();
				pobGGen->SetInSpecialAttack(true);

				return true;
			}
			ntAssert_p(false, ("Failed to request direct attack m_pobSmashAttack2"));
		}
		break;
	case 2:
		if(pobBoss->ToCharacter()->GetCurrHealth() == pobGGen->m_iMinHealthTillShellBreak3)
		{
			if (pobBoss->GetAttackComponent()->Boss_Command_RequestDirectAttack(m_pobSmashAttack3))
			{
				pobGGen->GetMessageHandler()->ReceiveMsg<msg_buttonattack>();
				pobGGen->SetInSpecialAttack(true);

				return true;
			}
			ntAssert_p(false, ("Failed to request direct attack m_pobSmashAttack3"));
		}
		break;
	default:
		//Otherwise just break and return false.
//		ntAssert_p(false, ("This shouldn't happen, there are only 3."));
		break;
	}

	return false;
}

void GladiatorGeneralSmashAttackSpecial::NotifyAttackFinished()
{
	m_bFinished = true;
}


void GladiatorGeneralSmashAttackSpecial::NotifyAttackAutoLinked()
{
	m_bFinished = true;
}

bool GladiatorGeneralSmashAttackSpecial::IsVulnerableTo(CStrike* pobStrike)
{
	return IsVulnerableTo(pobStrike->GetAttackDataP());
}

bool GladiatorGeneralSmashAttackSpecial::IsVulnerableTo(const CAttackData* pobAttackData)
{
	return m_bVulnerableTo;
}

void GladiatorGeneralSmashAttackSpecial::NotifyIsInFailedStrikeRecovery(bool bFailedStrike)
{
	m_bVulnerableTo = bFailedStrike;

	GladiatorGeneral* pobGGen = (GladiatorGeneral*)m_pobBoss;
	pobGGen->SetShellbreakVulnerability(bFailedStrike);

	//Clear up the button-hint if it's currently around and no-longer needed.
	if(!bFailedStrike)
	{
		CEntity* pPlayer = CEntityManager::Get().GetPlayer();
		pPlayer->GetAttackComponent()->SetNeedsHintForButton(AB_GRAB, false);
		CHud::Get().GetCombatHUDElements()->m_pobButtonHinter->SetHintForButton(AB_GRAB, false);
	}
}


BossAttack* GladiatorGeneralSmashAttackSpecial::Update(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	GladiatorGeneral* pobGGen = (GladiatorGeneral*)pobBoss;

	//We only set the target point for initializing the attack (so they can run around him).
	pobGGen->GetBossMovement()->m_bTargetPointSet = false;

	if(m_bFinished)
	{
		//We're done
		pobGGen->SetInSpecialAttack(false);
		return 0;
	}

	return this;
}


BossAttack* GladiatorGeneralSmashAttackSpecial::NotifyAttackInterrupted()
{
	GladiatorGeneral* pobGGen = (GladiatorGeneral*)m_pobBoss;
	pobGGen->SetShellbreakVulnerability(false);

	//Clear up the button-hint if it's currently around.
	CEntity* pPlayer = CEntityManager::Get().GetPlayer();
	pPlayer->GetAttackComponent()->SetNeedsHintForButton(AB_GRAB, false);
	CHud::Get().GetCombatHUDElements()->m_pobButtonHinter->SetHintForButton(AB_GRAB, false);
	pobGGen->SetInSpecialAttack(false);

	return this;
}


void GladiatorGeneralSmashAttackSpecial::DebugRender(CPoint& obScreenLocation, float fXOffset, float fYOffset)
{
#ifndef _GOLD_MASTER
	//Output our current smash attack phase number. This should change after each successful shell-break attack.
	GladiatorGeneral* pobBoss = (GladiatorGeneral*)m_pobBoss;
	g_VisualDebug->Printf3D(obScreenLocation,fXOffset,fYOffset,DC_CYAN,0,"Smash attack phase %i", pobBoss->GetSmashAttackPhase());
#endif
}
