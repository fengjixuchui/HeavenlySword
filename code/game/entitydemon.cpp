#include "entitydemon.h"
#include "fsm.h"

#include "game/entitymanager.h"
#include "objectdatabase/dataobject.h"
#include "core/timer.h"
#include "game/attacks.h"
#include "game/awareness.h"	
#include "Physics/system.h"
#include "Physics/compoundlg.h"
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
#include "targetedtransition.h"
#include "continuationtransition.h"
#include "interactiontransitions.h"
#include "effect/fxhelper.h"
#include "core/exportstruct_anim.h"
#include "game/simpletransition.h"
#include "entitykingbohan.h"
#include "camera/camutils.h"
#include "game/combathelper.h"

const static float fDemonHeightAboveKing = 8.0f;

START_STD_INTERFACE(Demon)
	COPY_INTERFACE_FROM(Boss)
	DEFINE_INTERFACE_INHERITANCE(Boss)

	DECLARE_POSTPOSTCONSTRUCT_CALLBACK(OnPostPostConstruct)
END_STD_INTERFACE

//--------------------------------------------------
//!
//! Demon 
//! Any specific construction or updating for the king
//!
//--------------------------------------------------
Demon::Demon()
: Boss()
{
	m_eBossType = BT_DEMON;

	//Initialise all of our node's effect-ids to -1.
	for(int i = 0 ; i < NUM_DEMON_NODES ; i++)
	{
		m_iNodeEffectID[i] = -1;
	}

	//The demon starts out attached to the king.
	m_bAttachedToKing = true;
	m_bJustReattachedToKing = false;
	m_pobKing = 0;
	m_fTimeSinceAttachStateChange = 0.0f;
	m_bInMoveTransition = false;
}

Demon::~Demon()
{
	//Clean up our particle effects.
	RemoveDemonEffects();
}


void Demon::OnPostPostConstruct()
{
	//The demon starts off with it's standard effect attached.
	SetDemonEffectStandard();

	//Create our tentacle entity (used for visualisation of the tentacle).
	m_pobTentacle = 0;	//By default.

	LuaAttributeTable* pobTable = LuaAttributeTable::Create();
	DataObject* pDO;
	pDO = ObjectDatabase::Get().ConstructObject( "DemonTentacle", "DemonTentacle", GameGUID(), 0, true, false ); //true, false errors.
	CEntity* pobDemonTentacle = (CEntity*)pDO->GetBasePtr();
	pobDemonTentacle->SetAttributeTable( pobTable );
	pobDemonTentacle->GetAttributeTable()->SetDataObject( pDO );
	pobTable->SetAttribute("Description", "DemonTentacle");
	pobTable->SetAttribute("Name", "DemonTentacle");
	pobTable->SetAttribute("Clump", "entities\\characters\\king\\cloudtentacle.clump");
	pobTable->SetAttribute("AnimationContainer", "KingAnimContainer");
	pobTable->SetAttribute("HairConstruction", "");

	ObjectDatabase::Get().DoPostLoadDefaults( pDO );

	//Store a pointer to our tentacle.
	m_pobTentacle = (DemonTentacle*)pobDemonTentacle;
	//Make sure we have no rotation on the tentacle (this should never change).
	m_pobTentacle->SetRotation(CQuat(0.0f, 0.0f, 0.0f, 1.0f));
}


void Demon::SetKingPointer(Boss* pobKing)
{
	ntError_p( pobKing && (pobKing->GetBossType() == BT_KING_BOHAN), ("MUST send a valid pointer to the king so that the demon can be synchronised with him") );
	m_pobKing = pobKing;
}

bool Demon::CanStartAnAttack()
{
	return GetAttackComponent()->AI_Access_GetState() == CS_STANDARD;
}

bool Demon::CanStartALinkedAttack()
{
	return GetAttackComponent()->IsInNextMoveWindow() && !GetAttackComponent()->HasSelectedANextMove();
}

void Demon::UpdateBossSpecifics(float fTimeDelta)
{
	m_fTimeSinceAttachStateChange += fTimeDelta;
}

void Demon::PlayAnimation(CHashedString obAnimName, bool bLooping)
{
	//Check that we've been given an animation to play.
	if(ntStr::IsNull(obAnimName))
	{
		ntAssert_p(false, ("Demon::PlayAnimation called with no animation name"));
		return;
	}

	//Define our movement.
	SimpleTransitionDef obDef;
	obDef.m_obAnimationName = obAnimName;
	obDef.SetDebugNames(ntStr::GetString(obDef.m_obAnimationName), "SimpleTransitionDef");
	obDef.m_bLooping = bLooping;

	obDef.m_fTimeOffsetPercentage = 0.0f;
/*
	//Set random offset (for looping).
	if((obDef.m_bLooping) && (bRandomOffset))
	{
		obDef.m_fTimeOffsetPercentage = erandf(1.0f);
	}
*/

	obDef.m_bApplyGravity = false;

	//Push the controller onto our movement component.
	GetMovement()->BringInNewController(obDef, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND );
}


/**
void Demon::DetachFromKingForNS()

When the king and demon are put into absolute-movement-controllers, the
demon needs to be parented to the world, as absolute-movement only sets
the local transform matrix on the demon. This would obviously look very
wierd if it were still parented to the king!
*/
void Demon::DetachFromKingForNS()
{
	CHashedString obDemonRootTransformName("clump_demon");
	Transform* pobDemonRootTransform = GetHierarchy()->GetTransform(obDemonRootTransformName);
	if(!pobDemonRootTransform)
	{
		ntError_p(false, ("Could not find clump_demon transform on demon to detach and parent to world for externally-controlled movement state"));
		return;
	}

	CMatrix matWorld = pobDemonRootTransform->GetWorldMatrix();
	pobDemonRootTransform->RemoveFromParent();
	pobDemonRootTransform->SetLocalMatrix(matWorld);
	CHierarchy::GetWorld()->GetRootTransform()->AddChild(pobDemonRootTransform);
	SetParentEntity(0);
}


/**
void Demon::AttachToKingAfterNS()

Because the demon has to be parented only to the world for ninja sequences
and cutscenes (any externally-controlled movement), the demon needs to be
re-attached afterwards.
*/
void Demon::AttachToKingAfterNS()
{
	//Deactivate the physics again.
	if(GetPhysicsSystem())
	{
		GetPhysicsSystem()->Deactivate();
	}

	CHashedString obKingJoinTransform("vertibrae_mid");
	Transform* pobKingAttachTransform = m_pobKing->GetHierarchy()->GetTransform(obKingJoinTransform);
	if(!pobKingAttachTransform)
	{
		ntError_p(false, ("Could not find vertibrae_mid transform on king to attach demon to"));
	}
	CHashedString obDemonJoinTransform("clump_demon");
	Transform* pobDemonAttachTransform = GetHierarchy()->GetTransform(obDemonJoinTransform);
	if(!pobDemonAttachTransform)
	{
		ntError_p(false, ("Could not find clump_demon transform on demon to attach to king transform"));
	}
	pobDemonAttachTransform->RemoveFromParent();
	//Blat over the transform data (as we did when we first attached it).
	pobDemonAttachTransform->SetLocalRotation(CQuat(0, 0, 0, 1));
	pobDemonAttachTransform->SetLocalTranslation(CPoint(0, 0, 0));
	//Now attach it to the king.
	pobKingAttachTransform->AddChild(pobDemonAttachTransform);
	SetParentEntity(m_pobKing);
}


//===================== DEMON TENTACLE ENTITY =====================

START_STD_INTERFACE(DemonTentacle)
	COPY_INTERFACE_FROM(CEntity)
	DEFINE_INTERFACE_INHERITANCE(CEntity)

	DECLARE_POSTCONSTRUCT_CALLBACK(OnPostConstruct)
END_STD_INTERFACE


//--------------------------------------------------
//!
//! DemonTentacle State Machine
//!
//--------------------------------------------------
STATEMACHINE(DEMON_TENTACLE_FSM, DemonTentacle)
	DEMON_TENTACLE_FSM()
	{
		SET_INITIAL_STATE(DEFAULT);
	}

	STATE(DEFAULT)
		BEGIN_EVENTS
			ON_ENTER
			{
				//The demon will just need to process certain messages, it can stay in default state throughout perhaps?
			}
			END_EVENT(true)

			EVENT(msg_movementdone)
			{
				if(ME->m_bHideOnAnimDone)
				{
					ME->m_bHideOnAnimDone = false;
					ME->ToggleTentacleEffect(false);	//Hide the effect (no visible mesh... at least for the moment)
				}
				if(ME->m_bIsStrikeAnim)
				{
					ME->m_bIsStrikeAnim = false;
					ME->m_bStrikeAnimComplete = true;
				}
			}
			END_EVENT(true)

		END_EVENTS
	END_STATE

END_STATEMACHINE //DEMON_TENTACLE_FSM


//--------------------------------------------------
//!
//!	DemonTentacle::DemonTentacle()
//!	Default constructor
//!
//--------------------------------------------------
DemonTentacle::DemonTentacle()
{
	m_eType = EntType_Static;
	m_bHideOnAnimDone = false;
	m_bIsEffectOn = false;

	//Initialise all of our node's effect-ids to -1.
	for(int i = 0 ; i < NUM_DEMON_NODES ; i++)
	{
		m_iNodeEffectID[i] = -1;
	}
	m_iTentacleStrikeWarningEffect = -1;
}

//--------------------------------------------------
//!
//!	DemonTentacle::OnPostConstruct()
//!	Post Construct
//!
//--------------------------------------------------
void DemonTentacle::OnPostConstruct()
{
	CEntity::OnPostConstruct();

	//Add Components.
	InstallMessageHandler();
	InstallAnimator("KingAnimContainer");

	//We can't get away with having just the animator otherwise sometimes SetPosition() fails (positioning of the tentacle before
	//making visible and animating the strike).

	//Install the dynamics component (returns NULL if there's no ps.xml file, but that's fine, for now we don't need one).
	InstallDynamics();

	//Set up the movement, must be done after the dynamics.
	CMovement* pobMovement = NT_NEW_CHUNK(Mem::MC_ENTITY) CMovement(this, GetAnimator(), GetPhysicsSystem());
	SetMovement(pobMovement);

	//If we DID manage to create a physics system (someone made a ps.xml file or something), then deactivate it here to avoid unexpected
	//wierdness.
	if(GetPhysicsSystem())
	{
		GetPhysicsSystem()->Deactivate();
	}

	// Create and attach the statemachine
	DEMON_TENTACLE_FSM* pFSM = NT_NEW_CHUNK(Mem::MC_ENTITY) DEMON_TENTACLE_FSM();
	ATTACH_FSM(pFSM);
}

//--------------------------------------------------
//!
//!	DemonTentacle::~DemonTentacle()
//!	Default destructor
//!
//--------------------------------------------------
DemonTentacle::~DemonTentacle()
{
#ifdef DO_DEMON_EFFECTS
	//Make sure we don't have any effects running.
	for(int i = 0 ; i < NUM_DEMON_NODES ; i++)
	{
		if(m_iNodeEffectID[i] >= 0)
		{
			FXHelper::Pfx_Destroy(m_iNodeEffectID[i], false);
			m_iNodeEffectID[i] = -1;
		}
	}

	if(m_iTentacleStrikeWarningEffect >= 0)
	{
		FXHelper::Pfx_Destroy(m_iTentacleStrikeWarningEffect, false);
		m_iTentacleStrikeWarningEffect = -1;
	}
#endif
}


/**The entity plays an animation and is notified by message when the animation is done.*/
void DemonTentacle::PlayTentacleAnim(CHashedString obAnimName, bool bHideWhenAnimDone, bool bIsStrikeAnim)
{
	ToggleTentacleEffect(true);	//Always visible for anim-playing.

	//Check that we've been given an animation to play.
	if(ntStr::IsNull(obAnimName))
	{
		ntAssert_p(false, ("Demon::PlayAnimation called with no animation name"));
		return;
	}

	if(GetMovement())
	{
		GetMovement()->ClearControllers();	//Clear out any existing controllers to avoid receiving old msg_movementdone messages.
		m_bHideOnAnimDone = bHideWhenAnimDone;
		m_bIsStrikeAnim = bIsStrikeAnim;
		if(m_bIsStrikeAnim)
		{
			m_bStrikeAnimComplete = false;
		}

		//Define our movement.
		SimpleTransitionDef obDef;
		obDef.m_obAnimationName = obAnimName;
		obDef.SetDebugNames(ntStr::GetString(obDef.m_obAnimationName), "SimpleTransitionDef");
		obDef.m_bLooping = false;
		obDef.m_fTimeOffsetPercentage = 0.0f;
		obDef.m_bApplyGravity = false;

		//Push the controller onto our movement component.
		GetMovement()->BringInNewController(obDef, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND );
		Message obMovementMessage(msg_movementdone);
		GetMovement()->SetCompletionMessage(obMovementMessage, this);
	}
	else
	{
		ntError_p(false, ("Tentacle entity does not have a movement component (should've been created on-construction)!!!"));
	}
}


void DemonTentacle::ToggleTentacleEffect(bool bEffectOn)
{
	//If we're trying to re-set it to it's existing state, then ignore.
	if(bEffectOn == m_bIsEffectOn)
	{
		return;
	}

	if(bEffectOn)
	{
		//The node naming goes 1->NUM_TENTACLE_NODES inclusive, no 0, so we +1 when creating the names.
		//We leave it 0->NUM_TENTACLE_NODES-1 for the loop so that it matches the array positions to store the effect-refs in.
#ifdef DO_DEMON_EFFECT
		ntPrintf("***###*** Spawning effect on the demon tentacle (to show it) ***###***\n");
		for(int i = 0 ; i < NUM_TENTACLE_NODES ; i++)
		{
			char nodeName[16] = {0};
			sprintf(nodeName, "tentacle1_J0%d", i+1);
//			ntPrintf("Spawning tentacle effect on node %s\n", nodeName);
			m_iNodeEffectID[i] = FXHelper::Pfx_CreateAttached(CHashedString("DemonTentacleEffect_Def"), CHashedString(GetName()), CHashedString(nodeName));
		}
#endif

	}
	else
	{
#ifdef DO_DEMON_EFFECT
		ntPrintf("***###*** Removing effect from the demon tentacle (to hide it) ***###***\n");
		for(int i = 0 ; i < NUM_DEMON_NODES ; i++)
		{
			if(m_iNodeEffectID[i] >= 0)
			{
				FXHelper::Pfx_Destroy(m_iNodeEffectID[i], false);
				m_iNodeEffectID[i] = -1;
			}
		}
#endif
	}

	m_bIsEffectOn = bEffectOn;
}


//===================== DETACH AND ASCEND MOVEMENT TRANSITION ====================
START_STD_INTERFACE(DemonDetachAndAscendMovement)
	COPY_INTERFACE_FROM(BossTransitioningMovement)
	DEFINE_INTERFACE_INHERITANCE(BossTransitioningMovement)

	PUBLISH_VAR_AS(m_obAscendStartAnim, AscendStartAnim)
	PUBLISH_VAR_AS(m_obAscendCycleAnim, AscendCycleAnim)
	PUBLISH_VAR_AS(m_obAscendStopAnim, AscendStopAnim)
END_STD_INTERFACE


DemonDetachAndAscendMovement::DemonDetachAndAscendMovement()
{
	m_bDone = false;
#ifdef DEMON_TRANSITION_BACKUP_HAX
	m_fTimeInMovement = 0.0f;
#endif
}


BossMovement* DemonDetachAndAscendMovement::Initialise(Boss* pobBoss, CEntity* pobPlayer)
{
	//Check that this is on the demon!
	ntError_p( pobBoss->GetBossType() == Boss::BT_DEMON, ("Should not attach a DemonDetachAndAscendMovement on anyone other than the demon!") );
	Demon* pobDemon = (Demon*)pobBoss;
	ntError_p( pobDemon->GetKing() && (pobDemon->GetKing()->GetBossType() == Boss::BT_KING_BOHAN), ("Invalid king pointer in demon!") );
	KingBohan* pobKing = (KingBohan*)pobDemon->GetKing();
	UNUSED(pobKing);

	if(pobDemon->m_bAttachedToKing == false)
	{
		return 0;
	}

#ifdef DEMON_CAN_DETACH
	//We only want to go through the reparent-to-world process if the demon is currently attached to the king.
	if(pobDemon->m_bAttachedToKing)
	{
		//Detach the demon and give it it's own offset (a bit of Y perhaps so that we can scale it to make it fly).
		CHashedString obDemonRootTransformName("clump_demon");
		Transform* pobDemonRootTransform = pobDemon->GetHierarchy()->GetTransform(obDemonRootTransformName);
		if(!pobDemonRootTransform)
		{
			ntError_p(false, ("Could not find clump_demon transform on demon to detach and parent to world."));
		}
		CMatrix matWorld = pobDemonRootTransform->GetWorldMatrix();
		pobDemonRootTransform->RemoveFromParent();
		pobDemonRootTransform->SetLocalMatrix(matWorld);
		CHierarchy::GetWorld()->GetRootTransform()->AddChild(pobDemonRootTransform);
		pobDemon->SetParentEntity(0);

		pobDemon->m_bAttachedToKing = false;			//So that anims aren't forced to synchronise anymore.
		pobDemon->m_fTimeSinceAttachStateChange = 0.0f;	//Reset this now that we've changed attached state.

		if(pobDemon->GetPhysicsSystem())
		{
			pobDemon->GetPhysicsSystem()->Activate();
		}
	}

	//TODO: What will be the criteria for deciding where the demon should move to? Just a position relative to the king that looks good?
	//Should it move directly above the player then spread? etc.
	//For now just have it move up above the king.
	CPoint obPointToAscendTo = pobKing->GetPosition() + CPoint(0.0f, fDemonHeightAboveKing, 0.0f);

	CDirection obAscendVector = CDirection( obPointToAscendTo - pobDemon->GetPosition() );

	//First we need to make sure we reset the position and rotation of the entity to update the physics-position/rotation now.
	CPoint obPosition(pobDemon->GetMatrix().GetTranslation());
	CQuat obRotation(pobDemon->GetMatrix());
	pobDemon->SetPosition(obPosition);
	pobDemon->SetRotation(obRotation);

	//When we start ascending, we play our 'into' animation.
	SimpleTransitionDef obAscendStartDef;
	obAscendStartDef.SetDebugNames("Demon Ascend Start", "SimpleTransitionDef");
	obAscendStartDef.m_fTimeOffsetPercentage = 0.0f;
	obAscendStartDef.m_bApplyGravity = false;
	obAscendStartDef.m_bLooping = false;
	obAscendStartDef.m_obAnimationName = m_obAscendStartAnim;

	//While we're ascending we continue till we get there.
	TargetedTransitionToPointManualMoveDef obAscendCycle;
	obAscendCycle.SetDebugNames("Demon Ascend Cycle", "TargetedTransitionToPointManualMoveDef");
	obAscendCycle.m_obAnimationName = m_obAscendCycleAnim;
	obAscendCycle.m_obPoint = obPointToAscendTo;
	obAscendCycle.m_obRotationToFix = CDirection(-40.0f * (PI / 180.0f), 0.0f, 0.0f); //-77.939 in maya

	//The speed of the movement is modified so that it always gets to it's destination within 3 seconds, but has a minimum
	//speed of 4.0f.
	float fDistance = obAscendVector.Length();
	float fSpeed = (float)(fDistance / 3.0f);
	obAscendCycle.m_fSpeed = (fSpeed >= 4.0f) ? fSpeed : 4.0f;

	//When we finish ascending, we play our '2fs' animation.
	SimpleTransitionDef obAscendEndDef;
	obAscendEndDef.SetDebugNames("Demon Ascend End", "SimpleTransitionDef");
	obAscendEndDef.m_fTimeOffsetPercentage = 0.0f;
	obAscendEndDef.m_bApplyGravity = false;
	obAscendEndDef.m_bLooping = false;
	obAscendEndDef.m_obAnimationName = m_obAscendStopAnim;

	pobDemon->GetMovement()->BringInNewController( obAscendStartDef, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND );
	pobDemon->GetMovement()->AddChainedController( obAscendCycle, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND );
	pobDemon->GetMovement()->AddChainedController( obAscendEndDef, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND );

	Message obMovementMessage(msg_movementdone);
	pobDemon->GetMovement()->SetCompletionMessage(obMovementMessage, pobBoss);

#ifdef DEMON_TRANSITION_BACKUP_HAX
	m_fTimeInMovement = 0.0f;	//Reset on initialising the movement.
#endif

	//Tell our demon to go to it's cloud-effect (might be too early, do it after transition?)
	pobDemon->SetDemonEffectCloud();

	m_bDone = false;
#else
	pobDemon->SetDemonEffectStunned();	//Gradually disappear instead.

	pobDemon->m_bAttachedToKing = false;			//So that anims aren't forced to synchronise anymore.
	pobDemon->m_fTimeSinceAttachStateChange = 0.0f;	//Reset this now that we've changed attached state.

	m_bDone = true;	//For now that's all we want to do.
#endif
	//Successfully initialised.
	return this;
}


BossMovement* DemonDetachAndAscendMovement::DoMovement(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
#ifdef DEMON_TRANSITION_BACKUP_HAX
	m_fTimeInMovement += fTimeDelta;
#endif

	if(!m_bDone)
	{

#ifdef DEMON_TRANSITION_BACKUP_HAX
		//Horrid hack (just to stop it from hanging during development, after a set amount of time, just notify it as done.)
		if(m_fTimeInMovement > 10.0f)
		{
			ntAssert_p(false, ("Error: The DetachAndAscend movement must've hung, has taken far too long. **SAVED BY SUPERHAX**"));
			NotifyMovementDone();
			m_fTimeInMovement = 0.0f;
		}
#endif

		return this;
	}

#ifdef DEMON_CAN_DETACH
	//When we're done with this movement, force the demon into it's attack phase.
	CHashedString obAttackPhase("Demon_AttackPhase_Attack");
	pobBoss->SetNamedAttackPhase(obAttackPhase);
#else
	pobBoss->SetNoAttackPhase();
#endif	//DEMON_CAN_DETACH

#ifdef DEMON_TRANSITION_BACKUP_HAX
	m_fTimeInMovement = 0.0f;
#endif
	return 0;
}


//===================== DESCEND AND ATTACH MOVEMENT TRANSITION ====================
START_STD_INTERFACE(DemonDescendAndAttachMovement)
	COPY_INTERFACE_FROM(BossTransitioningMovement)
	DEFINE_INTERFACE_INHERITANCE(BossTransitioningMovement)

	PUBLISH_VAR_AS(m_obDescendStartAnim, DescendStartAnim)
	PUBLISH_VAR_AS(m_obDescendCycleAnim, DescendCycleAnim)
	PUBLISH_VAR_AS(m_obDescendStopAnim, DescendStopAnim)
END_STD_INTERFACE


DemonDescendAndAttachMovement::DemonDescendAndAttachMovement()
{
	m_bDone = false;
#ifdef DEMON_TRANSITION_BACKUP_HAX
	m_fTimeInMovement = 0.0f;
#endif
	m_bStartingAttach = true;
}


BossMovement* DemonDescendAndAttachMovement::Initialise(Boss* pobBoss, CEntity* pobPlayer)
{
	//Check that this is on the demon!
	ntError_p( pobBoss->GetBossType() == Boss::BT_DEMON, ("Should not attach a DemonDescendAndAttachMovement on anyone other than the demon!") );
	m_pobBoss = pobBoss;
	Demon* pobDemon = (Demon*)m_pobBoss;

	ntError_p( pobDemon->GetKing() && (pobDemon->GetKing()->GetBossType() == Boss::BT_KING_BOHAN),
		("Cannot complete or set-up DemonDescendAndAttachMovemnet without pointer to the king") );

#ifdef DEMON_CAN_DETACH
	//We tell the demon to descend to exactly where the king is. At this point the king should not be moving!
	//We aim for the transform that we want to connect to. This doesn't need to be too exact as after we get to the position
	//we're going to snap to it, play the cycle anim, then 2fs, so it should appear seamless enough with particles.
	CHashedString obKingJoinTransform("vertibrae_mid");
	Transform* pobKingAttachTransform = pobDemon->GetKing()->GetHierarchy()->GetTransform(obKingJoinTransform);
	if(!pobKingAttachTransform)
	{
		ntError_p(false, ("Could not find vertibrae_mid transform on king to aim demon at"));
	}
	CPoint obPointToDescendTo = pobKingAttachTransform->GetWorldTranslation();

	CDirection obDescendVector = CDirection( obPointToDescendTo - pobDemon->GetPosition() );

	//When we start descending, we play our 'into' animation.
	SimpleTransitionDef obDescendStartDef;
	obDescendStartDef.SetDebugNames("Demon Descend Start", "SimpleTransitionDef");
	obDescendStartDef.m_fTimeOffsetPercentage = 0.0f;
	obDescendStartDef.m_bApplyGravity = false;
	obDescendStartDef.m_bLooping = false;
	obDescendStartDef.m_obAnimationName = m_obDescendStartAnim;

	//While we're descending we continue till we get there.
	TargetedTransitionToPointManualMoveDef obDescendCycle;
	obDescendCycle.SetDebugNames("Demon Descend Cycle", "TargetedTransitionToPointManualMoveDef");
	obDescendCycle.m_obAnimationName = m_obDescendCycleAnim;
	obDescendCycle.m_obPoint = obPointToDescendTo;
	obDescendCycle.m_obRotationToFix = CDirection(-40.0f * (PI / 180.0f), 0.0f, 0.0f); //-77.939 in maya

	//The speed of the movement is modified so that it always gets back to the king within 3 seconds, but has a minimum
	//speed of 4.0f.
	float fDistance = obDescendVector.Length();
	float fSpeed = (float)(fDistance / 3.0f);
	obDescendCycle.m_fSpeed = (fSpeed >= 4.0f) ? fSpeed : 4.0f;

	pobDemon->GetMovement()->BringInNewController( obDescendStartDef, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND );
	pobDemon->GetMovement()->AddChainedController( obDescendCycle, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND );

	Message obMovementMessage(msg_movementdone);
	pobDemon->GetMovement()->SetCompletionMessage(obMovementMessage, m_pobBoss);

#ifdef DEMON_TRANSITION_BACKUP_HAX
	m_fTimeInMovement = 0.0f;	//Reset on initialising the movement.
#endif

	m_bStartingAttach = true;
	m_bDone = false;

	//Just as a safety, hide the tentacle. This is just in-case the king tells the demon to descend and attach in the middle
	//of a cloud-form tentacle attack. The king has main control after all.
	pobDemon->GetTentacle()->ToggleTentacleEffect(false);
	//Also make sure that we remove the "warning" effect on the player (pre-strike warm-up) in-case we were in that stage.
	if(pobDemon->GetTentacle()->m_iTentacleStrikeWarningEffect >= 0)
	{
		FXHelper::Pfx_Destroy(pobDemon->GetTentacle()->m_iTentacleStrikeWarningEffect, false);
		pobDemon->GetTentacle()->m_iTentacleStrikeWarningEffect = -1;
	}
#else	//DEMON_CAN_DETACH
	pobDemon->SetDemonEffectStandard();
	m_bDone = true;	//That's all we want to do for now.
#endif

	//Successfully initialised.
	return this;
}


BossMovement* DemonDescendAndAttachMovement::DoMovement(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{

#ifdef DEMON_TRANSITION_BACKUP_HAX
	m_fTimeInMovement += fTimeDelta;
#endif

	if(!m_bDone)
	{

#ifdef DEMON_TRANSITION_BACKUP_HAX
		//Horrid hack (just to stop it from hanging during development, after a set amount of time, just notify it as done.)
		if(m_fTimeInMovement > 10.0f)
		{
			ntAssert_p(false, ("Error: The DescendAndAttach movement must've hung, has taken far too long. **SAVED BY SUPERHAX**"));
			NotifyMovementDone();
			m_fTimeInMovement = 0.0f;

			return 0;
		}
#endif

		return this;
	}

#ifdef DEMON_TRANSITION_BACKUP_HAX
	m_fTimeInMovement = 0.0f;
#endif

#ifndef DEMON_CAN_DETACH
	NotifyMovementDone();	//If the demon can't detach then we go straight to the end of the movement (already attached).
#endif

	return 0;
}


void DemonDescendAndAttachMovement::NotifyMovementDone()
{
	//Manually re-attach the demon to the king here, and remove any local offset etc that it has again
	//(i.e. snap onto transform, it should be like 99.9% there anyway, and being made of particles the snap shouldn't notice).
	Demon* pobDemon = (Demon*)m_pobBoss;

#ifdef DEMON_CAN_DETACH
	KingBohan* pobKing = (KingBohan*)pobDemon->GetKing();

	ntError_p( pobKing && pobKing->GetHierarchy(), ("Error: King or King's hierarchy does not exist, we have a valid king pointer?") );

	//If we're attaching now...
	if(m_bStartingAttach == true)
	{
		//Deactivate the physics again.
		if(pobDemon->GetPhysicsSystem())
		{
			pobDemon->GetPhysicsSystem()->Deactivate();
		}

		CHashedString obKingJoinTransform("vertibrae_mid");
		Transform* pobKingAttachTransform = pobKing->GetHierarchy()->GetTransform(obKingJoinTransform);
		if(!pobKingAttachTransform)
		{
			ntError_p(false, ("Could not find vertibrae_mid transform on king to attach demon to"));
		}
		CHashedString obDemonJoinTransform("clump_demon");
		Transform* pobDemonAttachTransform = pobDemon->GetHierarchy()->GetTransform(obDemonJoinTransform);
		if(!pobDemonAttachTransform)
		{
			ntError_p(false, ("Could not find clump_demon transform on demon to attach to king transform"));
		}
		pobDemon->SetParentEntity(pobDemon->GetKing());
		pobDemonAttachTransform->RemoveFromParent();
		//Blat over the transform data (as we did when we first attached it).
		pobDemonAttachTransform->SetLocalRotation(CQuat(0, 0, 0, 1));
		pobDemonAttachTransform->SetLocalTranslation(CPoint(0, 0, 0));
		//Now attach it to the king.
		pobKingAttachTransform->AddChild(pobDemonAttachTransform);

		//Tell our demon to go to it's standard-effect (might be too late, do it after first transition before movement?)
		pobDemon->SetDemonEffectStandard();

		//Now that we're back and attached (but still in cycle-anim), play the cycle anim once more to ensure a smooth anim transition from
		//cycle-anim to 2fs (if we don't do this then the points will be at the end of the cycle anim on a different local matrix, so the blend
		//of node positions wouldn't look as the transform-into-wings anim had been planned)... then push on the '2fs' anim.
		SimpleTransitionDef obDescendLastCycleDef;
		obDescendLastCycleDef.SetDebugNames("Demon Descend End", "SimpleTransitionDef");
		obDescendLastCycleDef.m_fTimeOffsetPercentage = 0.0f;
		obDescendLastCycleDef.m_bApplyGravity = false;
		obDescendLastCycleDef.m_bLooping = false;
		obDescendLastCycleDef.m_obAnimationName = m_obDescendCycleAnim;

		pobDemon->GetMovement()->BringInNewController( obDescendLastCycleDef, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND );

		Message obMovementMessage(msg_movementdone);
		pobDemon->GetMovement()->SetCompletionMessage(obMovementMessage, m_pobBoss);

		m_bStartingAttach = false;
#ifdef DEMON_TRANSITION_BACKUP_HAX
		m_fTimeInMovement = 0.0f;	//The one at the end of DoMovement doesn't seem to always trigger (oddly enough), so we need one here just in-case.
#endif
	}
	else if(pobDemon->m_bJustReattachedToKing == false)
	{
		SimpleTransitionDef obDescendEndDef;
		obDescendEndDef.SetDebugNames("Demon Descend End", "SimpleTransitionDef");
		obDescendEndDef.m_fTimeOffsetPercentage = 0.0f;
		obDescendEndDef.m_bApplyGravity = false;
		obDescendEndDef.m_bLooping = false;
		obDescendEndDef.m_obAnimationName = m_obDescendStopAnim;

		pobDemon->GetMovement()->BringInNewController( obDescendEndDef, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND );

		Message obMovementMessage(msg_movementdone);
		pobDemon->GetMovement()->SetCompletionMessage(obMovementMessage, m_pobBoss);

		//Notify the demon that it has just re-attached again (so the king can go into his 2fs while we queue-up ours).
		pobDemon->m_bJustReattachedToKing = true;

#ifdef DEMON_TRANSITION_BACKUP_HAX
		m_fTimeInMovement = 0.0f;	//The one at the end of DoMovement doesn't seem to always trigger (oddly enough), so we need one here just in-case.
#endif
	}
	else
	{
		//Now that we're re-attached, we want to set our phase to NULL again and just act as a synchronised animation.
		//We'll stay that way until the king notifies us that we can leave again by forcing our state.
		m_bStartingAttach = true;
		pobDemon->m_bJustReattachedToKing = false;
		pobDemon->m_bAttachedToKing = true;	//So we can synchronise through anim-events now that we're done with our 2fs.
		pobDemon->m_fTimeSinceAttachStateChange = 0.0f;	//Reset this now that we've completed changing attached state.
		pobDemon->SetNoAttackPhase();

#ifdef DEMON_TRANSITION_BACKUP_HAX
		m_fTimeInMovement = 0.0f;	//The one at the end of DoMovement doesn't seem to always trigger (oddly enough), so we need one here just in-case.
#endif

		//...and we're done
		m_bDone = true;
	}
#else	//DEMON_CAN_DETACH
	pobDemon->m_bJustReattachedToKing = true;
	pobDemon->m_bAttachedToKing = true;	//So we can synchronise through anim-events now that we're done with our 2fs.
	pobDemon->m_fTimeSinceAttachStateChange = 0.0f;	//Reset this now that we've completed changing attached state.
	pobDemon->SetNoAttackPhase();
	m_bDone = true;
#endif
}



//===================== ATTACK PHASE MOVEMENT TRANSITION ====================

START_STD_INTERFACE(DemonAttackPhaseMovement)
	COPY_INTERFACE_FROM(BossTransitioningMovement)
	DEFINE_INTERFACE_INHERITANCE(BossTransitioningMovement)

	PUBLISH_VAR_AS(m_obMoveStartAnim, MoveStartAnim)
	PUBLISH_VAR_AS(m_obMoveCycleAnim, MoveCycleAnim)
	PUBLISH_VAR_AS(m_obMoveStopAnim, MoveStopAnim)
END_STD_INTERFACE


DemonAttackPhaseMovement::DemonAttackPhaseMovement()
{
	m_bDone = false;
#ifdef DEMON_TRANSITION_BACKUP_HAX
	m_fTimeInMovement = 0.0f;
#endif
}


BossMovement* DemonAttackPhaseMovement::Initialise(Boss* pobBoss, CEntity* pobPlayer)
{
	//Check that this is on the demon!
	ntError_p( pobBoss->GetBossType() == Boss::BT_DEMON, ("Should not attach a DemonAttackPhaseMovement on anyone other than the demon!") );
	Demon* pobDemon = (Demon*)pobBoss;
	ntError_p( pobDemon->GetKing() && (pobDemon->GetKing()->GetBossType() == Boss::BT_KING_BOHAN), ("Invalid king pointer in demon!") );
	KingBohan* pobKing = (KingBohan*)pobDemon->GetKing();
	UNUSED(pobKing);

	if(pobDemon->m_bAttachedToKing == true)
	{
		//We should never get here unless someone put a selector in the wrong place!
		return 0;
	}

	//TODO: What will be the criteria for deciding where the demon should move to? Just a position relative to the king that looks good?
	//For now just have it move up above the king.
	CPoint obPointToMoveTo = pobKing->GetPosition() + CPoint(0.0f, fDemonHeightAboveKing, 0.0f);

	CDirection obMoveVector = CDirection( obPointToMoveTo - pobDemon->GetPosition() );

	//When we start moving, we play our 'into' animation.
	SimpleTransitionDef obMoveStartDef;
	obMoveStartDef.SetDebugNames("Demon Move Start", "SimpleTransitionDef");
	obMoveStartDef.m_fTimeOffsetPercentage = 0.0f;
	obMoveStartDef.m_bApplyGravity = false;
	obMoveStartDef.m_bLooping = false;
	obMoveStartDef.m_obAnimationName = m_obMoveStartAnim;

	//While we're ascending we continue till we get there.
	TargetedTransitionToPointManualMoveDef obMoveCycle;
	obMoveCycle.SetDebugNames("Demon Ascend Cycle", "TargetedTransitionToPointManualMoveDef");
	obMoveCycle.m_obAnimationName = m_obMoveCycleAnim;
	obMoveCycle.m_obPoint = obPointToMoveTo;
	obMoveCycle.m_obRotationToFix = CDirection(-40.0f * (PI / 180.0f), 0.0f, 0.0f); //-77.939 in maya

	//The speed of the movement is modified so that it always gets to it's destination within 3 seconds, but has a minimum
	//speed of 4.0f.
	float fDistance = obMoveVector.Length();
	float fSpeed = (float)(fDistance / 3.0f);
	obMoveCycle.m_fSpeed = (fSpeed >= 4.0f) ? fSpeed : 4.0f;

	//When we finish moving, we play our '2fs' animation.
	SimpleTransitionDef obMoveEndDef;
	obMoveEndDef.SetDebugNames("Demon Ascend End", "SimpleTransitionDef");
	obMoveEndDef.m_fTimeOffsetPercentage = 0.0f;
	obMoveEndDef.m_bApplyGravity = false;
	obMoveEndDef.m_bLooping = false;
	obMoveEndDef.m_obAnimationName = m_obMoveStopAnim;

	pobDemon->GetMovement()->BringInNewController( obMoveStartDef, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND );
	pobDemon->GetMovement()->AddChainedController( obMoveCycle, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND );
	pobDemon->GetMovement()->AddChainedController( obMoveEndDef, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND );

	Message obMovementMessage(msg_movementdone);
	pobDemon->GetMovement()->SetCompletionMessage(obMovementMessage, pobBoss);

#ifdef DEMON_TRANSITION_BACKUP_HAX
	m_fTimeInMovement = 0.0f;	//Reset on initialising the movement.
#endif

	//Flag it that we're moving (so we don't attempt to attack whilst moving).
	pobDemon->m_bInMoveTransition = true;

	m_bDone = false;

	//Successfully initialised.
	return this;
}


BossMovement* DemonAttackPhaseMovement::DoMovement(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	Demon* pobDemon = (Demon*)pobBoss;
#ifdef DEMON_TRANSITION_BACKUP_HAX
	m_fTimeInMovement += fTimeDelta;
#endif

	if(!m_bDone)
	{

#ifdef DEMON_TRANSITION_BACKUP_HAX
		//Horrid hack (just to stop it from hanging during development, after a set amount of time, just notify it as done.)
		if(m_fTimeInMovement > 10.0f)
		{
			ntAssert_p(false, ("Error: The attack phase movement must've hung, has taken far too long. **SAVED BY SUPERHAX**"));
			//Force us to where we wanted to be (above the king).
			ntError_p(pobDemon && (pobDemon->GetKing()->GetBossType() == Boss::BT_KING_BOHAN), ("Error: Invalid king pointer on demon"));
			KingBohan* pobKing = (KingBohan*)pobDemon->GetKing();
			CPoint obKingPos = pobKing->GetPosition();
			CPoint obDesiredPosition = CPoint(obKingPos + CPoint(0.0f, fDemonHeightAboveKing, 0.0f));
			pobDemon->SetPosition(obDesiredPosition);

			//Notify that we're done.
			NotifyMovementDone();
			m_fTimeInMovement = 0.0f;
		}
#endif

		return this;
	}

#ifdef DEMON_TRANSITION_BACKUP_HAX
	m_fTimeInMovement = 0.0f;
#endif
	//Flag it that we're no-longer moving (so we can attack again).
	pobDemon->m_bInMoveTransition = false;

	return 0;
}


void DemonAttackPhaseMovement::NotifyMovementDone()
{
	m_bDone = true;
}


//===================== TENTACLE ATTACK SELECTOR ====================

START_STD_INTERFACE(DemonTentacleAttackSelector)
	COPY_INTERFACE_FROM(BossAttackSelector)
	DEFINE_INTERFACE_INHERITANCE(BossAttackSelector)

	PUBLISH_VAR_AS(m_fMinDistance, MinDistance)
	PUBLISH_VAR_AS(m_fMaxDistance, MaxDistance)
END_STD_INTERFACE


//------------------------------------------------------------------------------------------
//!
//!	DemonTentacleAttackSelector
//! Class for selecting the demon's tentacle attack.
//!
//------------------------------------------------------------------------------------------
DemonTentacleAttackSelector::DemonTentacleAttackSelector()
{
	//Clear the values if none have been provided by xml/welder.
	m_fMinDistance = 0.0f;
	m_fMaxDistance = 0.0f;
}


//--------------------------------------------------
//!
//! GetPriority 
//! Checks whether a tentacle-attack is a viable selection at this point.
//!
//--------------------------------------------------
float DemonTentacleAttackSelector::GetPriority(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	ntError_p( pobBoss->GetBossType() == Boss::BT_DEMON, ("Who put a DemonTentacleAttackSelector on something other than the Demon!?!?") );

	Demon* pobDemon = (Demon*)pobBoss;

	//Don't select if we're in a move transition.
	if(pobDemon->IsInMoveTransition())
	{
		return 0.0f;
	}

	//We can select this attack if the player is within a certain horizontal radius of the centre of the demon-cloud.
	CPoint obDemonPos = pobDemon->GetPosition();
	CPoint obPlayerPos = pobPlayer->GetPosition();
	CDirection obBetween = CDirection(CPoint(obDemonPos.X(), 0.0f, obDemonPos.Z()) - CPoint(obPlayerPos.X(), 0.0f, obPlayerPos.Z()));

	//If they're close enough, then we return max-priority on this attack.
	if(obBetween.LengthSquared() < (m_fMaxDistance * m_fMaxDistance))
	{
		return m_fMaxPriority;
	}

	return 0.0f;
}


BossAttack* DemonTentacleAttackSelector::BeginAttack(Boss* pobBoss, CEntity* pobPlayer)
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


//===================== TENTACLE ATTACK ====================

START_STD_INTERFACE(DemonTentacleAttack)
	COPY_INTERFACE_FROM(BossSpecialAttack)
	DEFINE_INTERFACE_INHERITANCE(BossSpecialAttack)

	PUBLISH_VAR_WITH_DEFAULT_AS(m_obTentacleStrikeData, "NULL", TentacleStrikeAttackData)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obTentacleStrikeAnim, "NULL", TentacleStrikeAnim)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obTentacleStrike2fsAnim, "NULL", TentacleStrike2fsAnim)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obStrikeImpactEffect[0], "NULL", StrikeImpactEffect1)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obStrikeImpactEffect[1], "NULL", StrikeImpactEffect2)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obStrikeImpactEffect[2], "NULL", StrikeImpactEffect3)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fTimeDelayTillStrike, 1.0f, TimeDelayTillStrike)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fTimeDelayTillGenerateStrikeTimeout, 2.0f, TimeDelayTillStrikeHitTimeout)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fTimeDelayTillStrikeOver, 1.0f, TimeDelayTillStrikeOver)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fStrikeRadius, 1.0f, StrikeRadius);
END_STD_INTERFACE


DemonTentacleAttack::DemonTentacleAttack()
{
	m_fStrikeTimerCounter = 0.0f;
	m_fTimeDelayTillStrike = 1.0f;
	m_fStrikeNodeCalculateDelay = 0.1f;	//Calculate which node the player is closest to every m_fStrikeNodeCalculateDelay seconds.
	m_bStrikeStarted = false;
	m_bStrikeCompleted = false;
	m_fStrikeOverTimerCounter = 0.0f;
	m_fTimeDelayTillStrikeOver = 1.0f;
	m_fStrikeGenerateHitCounter = 0.0f;
	m_fTimeDelayTillGenerateStrikeTimeout = 0.6f;
	m_fStrikeRadius = 1.0f;

	for(int i = 0 ; i < 3 ; i++)
	{
		m_obStrikeImpactEffect[i] = NULL;
	}
}


bool DemonTentacleAttack::Initialise(Boss* pobBoss, CEntity* pobPlayer)
{
	ntError_p( pobBoss->GetBossType() == Boss::BT_DEMON, ("Should not attach a DemonTentacleAttack on anyone other than the demon!") );
	Demon* pobDemon = (Demon*)pobBoss;
	ntError_p( ntStr::IsNull(m_obTentacleStrikeData) == false, ("No tentacle strike attack data specified") );
	ntError_p( pobDemon->GetTentacle(), ("Missing tentacle entity... required!") );

	ntError_p( (ntStr::IsNull(m_obTentacleStrikeAnim) == false) && (ntStr::IsNull(m_obTentacleStrike2fsAnim) == false),
		("Missing tentacle strike or tentacle strike-2fs anim (on demon tentacle attack data)") );

	UNUSED( pobDemon );

	m_pobBoss = pobBoss;
	
	m_fStrikeTimerCounter = 0.0f;
	m_fStrikeOverTimerCounter = 0.0f;
	m_fStrikeNodeCalculateTimer = m_fStrikeNodeCalculateDelay;	//Calculate on the first frame.
	m_iStrikeNode = -1;
	m_bStrikeStarted = false;
	m_bStrikeCompleted = false;
	m_fStrikeGenerateHitCounter = 0.0f;
	m_fTimeDelayTillGenerateStrikeTimeout = 2.0f;

	//Play a sound here to warn the player?

	return true;
}


void DemonTentacleAttack::NotifyAttackFinished()
{

}


void DemonTentacleAttack::NotifyAttackAutoLinked()
{

}


BossAttack* DemonTentacleAttack::Update(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	Demon* pobDemon = (Demon*)pobBoss;
	DemonTentacle* pobTentacle = pobDemon->GetTentacle();

	if(!m_bStrikeStarted)
	{
		m_fStrikeTimerCounter += fTimeDelta;
		m_fStrikeNodeCalculateTimer += fTimeDelta;

		//Keep track of which node the player is closest to, and make sure that node is playing a different effect to show-up.
		if(m_fStrikeNodeCalculateTimer >= m_fStrikeNodeCalculateDelay)
		{
			//Recalculate the node they player is closest to.
			int iClosestNode = -1;
			float fClosestDistanceSquared = 100000.0f;
			for(int i = 0 ; i < NUM_DEMON_NODES ; i++)
			{
				char nodeName[16] = {0};
				sprintf(nodeName, "god_node_%d", i);
				Transform* pobNodeTransform = pobDemon->GetHierarchy()->GetTransform(nodeName);
				if(!pobNodeTransform)
				{
					ntError_p(false, ("Could not find one or more of the demon's nodes to calculate player-proximity and attack-node from"));
				}

				CPoint obPlayerPos = pobPlayer->GetPosition();
				CPoint obNodePos = pobNodeTransform->GetWorldTranslation();

				//We're only interested in the horizontal distance. The one closest to being above the player is the one that takes priority.
				CDirection obBetween = CDirection(CPoint(obPlayerPos.X(), 0.0f, obPlayerPos.Z()) - CPoint(obNodePos.X(), 0.0f, obNodePos.Z()));
				float fDistanceSquared = obBetween.LengthSquared();

				if(fDistanceSquared < fClosestDistanceSquared)
				{
					fClosestDistanceSquared = fDistanceSquared;
					iClosestNode = i;
				}
			}

			//Now that we have our new closest node (assuming they weren't ALL over 10,000 units away).
			if(iClosestNode != -1)
			{
				//If this isn't the current closest node, then switch the effects on both and store the new closest node ID for next-time.
				if(iClosestNode != m_iStrikeNode)
				{
					//Return our last strike-node to be a cloud-node again.
					pobDemon->SetDemonNodeEffectCloud(m_iStrikeNode);
					//Store the closest node.
					m_iStrikeNode = iClosestNode;
					//Set our new attack-ndoe to use the correct particle effect.
					pobDemon->SetDemonNodeEffectCloudAttack(m_iStrikeNode);
				}
			}

			//Resett our timer so that we don't recalculate for another 'm_fStrikeNodeCalculateDelay' seconds
			m_fStrikeNodeCalculateTimer = 0.0f;
		}

		//This is the counter that applies during the run-up to the attack. This is where we'd want to spawn the effect at
		//the player's position so that they know they're going to be struck within a certain amount of time (the timing is something
		//they'll get used to as they play the game, as it's a fixed time, so not really annoying).
		if(pobTentacle->m_iTentacleStrikeWarningEffect < 0)
		{
			//Spawn the effect on the player's root to let them know we're going to strike.
			pobTentacle->m_iTentacleStrikeWarningEffect = FXHelper::Pfx_CreateAttached(CHashedString("TentacleStrikeWarning_Def"), CHashedString(pobPlayer->GetName()), CHashedString("ROOT"));
		}
	}
	else
	{
		m_fStrikeGenerateHitCounter += fTimeDelta;
		if(m_bStrikeCompleted)
		{
			m_fStrikeOverTimerCounter += fTimeDelta;
		}

		//Destroy our warning effect if we have one (we should!)
		if(pobTentacle->m_iTentacleStrikeWarningEffect >= 0)
		{
			FXHelper::Pfx_Destroy(pobTentacle->m_iTentacleStrikeWarningEffect, false);
			pobTentacle->m_iTentacleStrikeWarningEffect = -1;
		}
	}

	if((m_fStrikeTimerCounter >= m_fTimeDelayTillStrike) && !m_bStrikeStarted)
	{
		ntPrintf("****SUPER BAD-ASS DEMON-FORM TENTACLE STRIKE MASSIVE ATTACK... or something****\n");
		m_fStrikeTimerCounter = 0.0f;
		m_bStrikeStarted = true;

		//Play the anim and what-not...
		if(pobDemon->GetTentacle())
		{
			//The start-point for the demon will be at the demon's height, but at the player's position. The player
			//can of course move out of it's way after the attack starts (depending on how quick the strike is!)
			CPoint obPlayerPosition = pobPlayer->GetPosition();
			CPoint obDemonPosition = pobDemon->GetPosition();
			CPoint obTentacleStartPos(obPlayerPosition.X(), obDemonPosition.Y(), obPlayerPosition.Z());
			//Update our position
			pobDemon->GetTentacle()->SetPosition(obTentacleStartPos);
			//Make sure our rotation is correct (this will make it reset on each-anim even if at some point it manages to go wrong)
			pobDemon->GetTentacle()->SetRotation(CQuat(0.0f, 0.0f, 0.0f, 1.0f));
			if(!ntStr::IsNull(m_obTentacleStrikeAnim))
			{
				pobDemon->GetTentacle()->PlayTentacleAnim(m_obTentacleStrikeAnim, false, true);
			}
			else
			{
				ntPrintf("*** Demon tentacle strike anim missing... tentacle may not appear/disappear as expected ***\n");
			}
		}
	}

	//We use a strike-timeout here just in-case the anim-done message isn't recieved for any reason. This should be set to slightly
	//higher than the anim-time for the strike anim itself, so we give the strike anim a chance to recieve msg_animdone, and if not
	//then the time-out kicks in and generates the strike instead. Safeguard to stop it hanging on lost-animevent.
	if(m_bStrikeStarted && !m_bStrikeCompleted &&
		(pobDemon->GetTentacle()->m_bStrikeAnimComplete || (m_fStrikeGenerateHitCounter >= m_fTimeDelayTillGenerateStrikeTimeout)))
	{
		//Generate the strike itself if the player is within a certain radius of the point where it is striking.
		if(ntStr::IsNull(m_obTentacleStrikeData) == false)
		{
			//The strike is generated from the end transform on the tentacle.
			char acStrikeNodeName[16] = {0};
			sprintf(acStrikeNodeName, "tentacle1_J0%d", NUM_TENTACLE_NODES);
			CPoint obStrikePos(CONSTRUCT_CLEAR);
			Transform* pobStrikeTransform = pobDemon->GetTentacle()->GetHierarchy()->GetTransform(CHashedString(acStrikeNodeName));
			if(!pobStrikeTransform)
			{
				ntError_p(false, ("Failed to get tentacle end-transform %s to generate strike from", acStrikeNodeName));
				//In this unlikely event, we make sure the strike hits anyway.
				obStrikePos = pobPlayer->GetPosition();	//Can't fail to hit, shouldn't ever happen anyway, just a fallback.
			}
			else
			{
				//Generate it from the end transform on the tentacle entity.
				obStrikePos = pobStrikeTransform->GetWorldMatrix().GetTranslation();
			}
			CPoint obPlayerPos = pobPlayer->GetPosition();
			CDirection obBetween = CDirection(obStrikePos - obPlayerPos);
			if(obBetween.LengthSquared() < (m_fStrikeRadius * m_fStrikeRadius))
			{
				//TODO: The first 'pobBoss' here that specifies the object could maybe be the entity playing the anim itself?
				CombatHelper::Combat_GenerateStrike(pobBoss, pobBoss, pobPlayer, m_obTentacleStrikeData);
			}
			else
			{
				ntPrintf("** Distance from end of tentacle to player was %f, too far to hit [within %f]\n", obBetween.Length(), m_fStrikeRadius);
			}

			//Play some kind of area effect here, an explosion or something that warrents the radius on the strike, we can even increase
			//it then. Up to three particle effects can be triggered that will originate from the transform the strike area-check comes from.
			for(int i = 0 ; i < 3 ; i++)
			{
				if(!ntStr::IsNull(m_obStrikeImpactEffect[i]))
				{
					FXHelper::Pfx_CreateStatic(m_obStrikeImpactEffect[i], pobDemon->GetTentacle(), CHashedString(acStrikeNodeName));
				}
			}
		}

		//Flag that we've completed the strike either way so that the attack can finish.
		m_bStrikeCompleted = true;
		//With the strike done, we should play our animation where the tentacle retracts.
		if(pobDemon->GetTentacle())
		{
			if(!ntStr::IsNull(m_obTentacleStrike2fsAnim))
			{
				pobDemon->GetTentacle()->PlayTentacleAnim(m_obTentacleStrike2fsAnim, true, false);
			}
			else
			{
				ntPrintf("*** Demon tentacle strike 2fs anim missing... tentacle may not appear/disappear as expected ***\n");
			}
		}
	}

	if(m_bStrikeCompleted && (m_fStrikeOverTimerCounter >= m_fTimeDelayTillStrikeOver))
	{
		//We're done here.
		m_fStrikeOverTimerCounter = 0.0f;
		//Reset the effect on our "attack" node.
		pobDemon->SetDemonNodeEffectCloud(m_iStrikeNode);

		//Just for good measure, hide the tentacle (it should've been hidden already, but wouldn't have been if the 2fs
		//anim was missing, so we force it to hidden here just for good measure to make sure it doesn't stick around).
		if(pobDemon->GetTentacle())
		{
			pobDemon->GetTentacle()->ToggleTentacleEffect(false);
		}

		return 0;
	}

	return this;
}

void DemonTentacleAttack::DebugRender(CPoint& obScreenLocation, float fXOffset, float fYOffset)
{
#ifndef _GOLD_MASTER
	g_VisualDebug->Printf3D(obScreenLocation,fXOffset,fYOffset,DC_CYAN,0,"DemonTentacleAttack");
#endif
}


//===================== WING EFFECTS ====================
void Demon::RemoveDemonEffects()
{
#ifdef DO_DEMON_EFFECT
	ntPrintf("** Removing All Effects from Demon **\n");
	for(int i = 0 ; i < NUM_DEMON_NODES ; i++)
	{
		if(m_iNodeEffectID[i] >= 0)
		{
			FXHelper::Pfx_Destroy(m_iNodeEffectID[i], false);
			m_iNodeEffectID[i] = -1;
		}
	}
#endif
}


void Demon::SetDemonEffectStandard()
{
#ifdef DO_DEMON_EFFECT
	//Remove any current effects.
	RemoveDemonEffects();

	ntPrintf("** Setting Standard Effect on Demon **\n");

	//Spawn attached particle effects on all of the nodes on the demon object.
	for(int i = 0 ; i < NUM_DEMON_NODES ; i++)
	{
		char nodeName[16] = {0};
		sprintf(nodeName, "god_node_%d", i);
//		ntPrintf("Spawning standard-effect particles on node %s\n", nodeName);
		m_iNodeEffectID[i] = FXHelper::Pfx_CreateAttached(CHashedString("DemonFormStandard_Def"), CHashedString(GetName()), CHashedString(nodeName));
	}
#endif
}


void Demon::SetDemonEffectWingAttack()
{
#ifdef DO_DEMON_EFFECT
	//Remove any current effects.
	RemoveDemonEffects();

	ntPrintf("** Setting Wing-Attack Effect on Demon **\n");

	//Spawn attached particle effects on all of the nodes on the demon object.
	for(int i = 0 ; i < NUM_DEMON_NODES ; i++)
	{
		char nodeName[16] = {0};
		sprintf(nodeName, "god_node_%d", i);
//		ntPrintf("Spawning wing-attack effect particles on node %s\n", nodeName);
		m_iNodeEffectID[i] = FXHelper::Pfx_CreateAttached(CHashedString("DemonFormWingAttack_Def"), CHashedString(GetName()), CHashedString(nodeName));
	}
#endif
}


void Demon::SetDemonEffectRetreat()
{
#ifdef DO_DEMON_EFFECT
	//Remove any current effects.
	RemoveDemonEffects();

	ntPrintf("** Setting Retreat Effect on Demon **\n");

	//Spawn attached particle effects on all of the nodes on the demon object.
	for(int i = 0 ; i < NUM_DEMON_NODES ; i++)
	{
		char nodeName[16] = {0};
		sprintf(nodeName, "god_node_%d", i);
//		ntPrintf("Spawning retreat-effect particles on node %s\n", nodeName);
		m_iNodeEffectID[i] = FXHelper::Pfx_CreateAttached(CHashedString("DemonFormRetreat_Def"), CHashedString(GetName()), CHashedString(nodeName));
	}
#endif
}


void Demon::SetDemonEffectStunned()
{
#ifdef DO_DEMON_EFFECT
	//Remove any current effects.
	RemoveDemonEffects();

	ntPrintf("** Setting Cloud Effect on Demon **\n");

	//Spawn attached particle effects on all of the nodes on the demon object.
	for(int i = 0 ; i < NUM_DEMON_NODES ; i++)
	{
		char nodeName[16] = {0};
		sprintf(nodeName, "god_node_%d", i);
//		ntPrintf("Spawning cloud-effect particles on node %s\n", nodeName);
		m_iNodeEffectID[i] = FXHelper::Pfx_CreateAttached(CHashedString("DemonFormStunned_Def"), CHashedString(GetName()), CHashedString(nodeName));
	}
#endif
}


void Demon::SetDemonEffectCloud()
{
#ifdef DO_DEMON_EFFECT
	//Remove any current effects.
	RemoveDemonEffects();

	ntPrintf("** Setting Cloud Effect on Demon **\n");

	//Spawn attached particle effects on all of the nodes on the demon object.
	for(int i = 0 ; i < NUM_DEMON_NODES ; i++)
	{
		char nodeName[16] = {0};
		sprintf(nodeName, "god_node_%d", i);
//		ntPrintf("Spawning cloud-effect particles on node %s\n", nodeName);
		m_iNodeEffectID[i] = FXHelper::Pfx_CreateAttached(CHashedString("DemonFormCloud_Def"), CHashedString(GetName()), CHashedString(nodeName));
	}
#endif
}


void Demon::SetDemonNodeEffectCloud(int iNode)
{
#ifdef DO_DEMON_EFFECT
	if((iNode >= 0) && (iNode < NUM_DEMON_NODES))
	{
		//Remove the effect on this node.
		if(m_iNodeEffectID[iNode] >= 0)
		{
			FXHelper::Pfx_Destroy(m_iNodeEffectID[iNode], false);
			m_iNodeEffectID[iNode] = -1;
		}

		//Add the new effect.
		char nodeName[16] = {0};
		sprintf(nodeName, "god_node_%d", iNode);
		ntPrintf("Spawning demon-effect-cloud (returning from 'attack-cloud-node') on node %s\n", nodeName);
		m_iNodeEffectID[iNode] = FXHelper::Pfx_CreateAttached(CHashedString("DemonFormCloud_Def"), CHashedString(GetName()), CHashedString(nodeName));
	}
#endif
}


void Demon::SetDemonNodeEffectCloudAttack(int iNode)
{
#ifdef DO_DEMON_EFFECT
	if((iNode >= 0) && (iNode < NUM_DEMON_NODES))
	{
		//Remove the effect on this node.
		if(m_iNodeEffectID[iNode] >= 0)
		{
			FXHelper::Pfx_Destroy(m_iNodeEffectID[iNode], false);
			m_iNodeEffectID[iNode] = -1;
		}

		//Add the new effect.
		char nodeName[16] = {0};
		sprintf(nodeName, "god_node_%d", iNode);
		ntPrintf("Spawning demon-effect-cloud-attack (notification of impending attacking for player) on node %s\n", nodeName);
		m_iNodeEffectID[iNode] = FXHelper::Pfx_CreateAttached(CHashedString("DemonFormCloudAttack_Def"), CHashedString(GetName()), CHashedString(nodeName));
	}
#endif
}
