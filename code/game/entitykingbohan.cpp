#include "entitykingbohan.h"
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
//#include "simpletransition.h"
#include "continuationtransition.h"
//#include "inputcomponent.h"
#include "entitydemon.h"
#include "effect/fxhelper.h"


START_STD_INTERFACE(KingBohan)
	COPY_INTERFACE_FROM(Boss)
	DEFINE_INTERFACE_INHERITANCE(Boss)

	OVERRIDE_DEFAULT(Description, "boss,king")
	OVERRIDE_DEFAULT(Clump, "entities\\characters\\king\\king.clump")
	OVERRIDE_DEFAULT(CombatDefinition, "King_AttackDefinition")
	OVERRIDE_DEFAULT(AwarenessDefinition, "King_AttackTargetingData")
	OVERRIDE_DEFAULT(AnimationContainer, "KingAnimContainer")
	OVERRIDE_DEFAULT(CollisionHeight, "0.61")
	OVERRIDE_DEFAULT(CollisionRadius, "0.3")
	OVERRIDE_DEFAULT(Health, "500")
	OVERRIDE_DEFAULT(DefaultWalkingController, "KingWalkRun")
	OVERRIDE_DEFAULT(InitialAttackPhase, "King_Phase1_Attack")

	PUBLISH_VAR_WITH_DEFAULT_AS(m_aobRetreatPoints[0], CPoint(0.0f, 0.0f, 0.0f), RetreatPoint[0])
	PUBLISH_VAR_WITH_DEFAULT_AS(m_aobRetreatHealths[0], 400, RetreatHealth[0])
	PUBLISH_VAR_WITH_DEFAULT_AS(m_aobRetreatPointDistances[0], 20.0f, RetreatPointDistance[0])
	PUBLISH_VAR_WITH_DEFAULT_AS(m_aobRetreatPoints[1], CPoint(0.0f, 0.0f, 0.0f), RetreatPoint[1])
	PUBLISH_VAR_WITH_DEFAULT_AS(m_aobRetreatHealths[1], 300, RetreatHealth[1])
	PUBLISH_VAR_WITH_DEFAULT_AS(m_aobRetreatPointDistances[1], 20.0f, RetreatPointDistance[1])
	PUBLISH_VAR_WITH_DEFAULT_AS(m_aobRetreatPoints[2], CPoint(0.0f, 0.0f, 0.0f), RetreatPoint[2])
	PUBLISH_VAR_WITH_DEFAULT_AS(m_aobRetreatHealths[2], 200, RetreatHealth[2])
	PUBLISH_VAR_WITH_DEFAULT_AS(m_aobRetreatPointDistances[2], 20.0f, RetreatPointDistance[2])
	PUBLISH_VAR_WITH_DEFAULT_AS(m_aobRetreatPoints[3], CPoint(0.0f, 0.0f, 0.0f), RetreatPoint[3])
	PUBLISH_VAR_WITH_DEFAULT_AS(m_aobRetreatHealths[3], 100, RetreatHealth[3])
	PUBLISH_VAR_WITH_DEFAULT_AS(m_aobRetreatPointDistances[3], 20.0f, RetreatPointDistance[3])
	PUBLISH_VAR_WITH_DEFAULT_AS(m_aobRetreatPoints[4], CPoint(0.0f, 0.0f, 0.0f), RetreatPoint[4])
	PUBLISH_VAR_WITH_DEFAULT_AS(m_aobRetreatHealths[4], 0, RetreatHealth[4])
	PUBLISH_VAR_WITH_DEFAULT_AS(m_aobRetreatPointDistances[4], 20.0f, RetreatPointDistance[4])

	DECLARE_POSTPOSTCONSTRUCT_CALLBACK(OnPostPostConstruct)
END_STD_INTERFACE


//--------------------------------------------------
//!
//! KingBohan 
//! Any specific construction or updating for the king
//!
//--------------------------------------------------
KingBohan::KingBohan()
: Boss()
{
	m_eBossType = BT_KING_BOHAN;
	m_iCurrentRetreatPoint = 0;
	m_bTooFarFromCurrentRetreatPoint = false;
	m_bRetreatingOnHealthBasis = false;
	m_pobDemon = 0;
	m_bInSpecialAttack = false;
	m_bPlayerInRetreatPointRadiusRange = false;
	m_bHitWhileStunned = false;
	m_bNotifyIfHitStunned = false;
}

KingBohan::~KingBohan()
{
}


void KingBohan::OnPostPostConstruct()
{
	//Construct the demon (we do it here so that we can pass a pointer to ourself to it, and also store it's pointer here,
	//we'll need to do lots of communicating during the battle to keep both working together, the king acts as the controller).
	LuaAttributeTable* pobTable = LuaAttributeTable::Create();
	DataObject* pDO;
	pDO = ObjectDatabase::Get().ConstructObject( "Demon", "Demon", GameGUID(), 0, true, false );
	CEntity* pobDemon = (CEntity*) pDO->GetBasePtr();
	pobDemon->SetAttributeTable( pobTable );
	pobDemon->GetAttributeTable()->SetDataObject( pDO );
	pobTable->SetAttribute("Description", "Demon");
	pobTable->SetAttribute("Name", "Demon");
	pobTable->SetAttribute("Clump", "entities\\characters\\king\\demon.clump");
	pobTable->SetAttribute("InitialAttackPhase", "");
	pobTable->SetAttribute("CombatDefinition", "Demon_AttackDefinition");
	pobTable->SetAttribute("AwarenessDefinition", "Demon_AttackTargetingData");
	pobTable->SetAttribute("HairConstruction", "");
	pobTable->SetAttribute("AnimationContainer", "KingAnimContainer");
	pobTable->SetAttribute("RagdollClump", "NULL");

	ObjectDatabase::Get().DoPostLoadDefaults( pDO );

	//We start the demon off parented to the king (where it will remain for the entire first phase anyway).
	CHashedString obKingJoinTransform("vertibrae_mid");
	Transform* pobKingAttachTransform = this->GetHierarchy()->GetTransform(obKingJoinTransform);
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
	pobDemon->SetParentEntity(this);
	pobDemonAttachTransform->RemoveFromParent();
	//Blat over the transform from maya so that we move this transform to (0, 0, 0) with no rotation.
	//This way all child nodes of this first joint essentially inherit directly from the king's vertibrae_mid node,
	//which in-maya had the same rotation as the clump_demon node, so no rotational data is lost, but we can set
	//a zero-translation to parent them directly (snapped to/constrained).
	pobDemonAttachTransform->SetLocalRotation(CQuat(0, 0, 0, 1));
	pobDemonAttachTransform->SetLocalTranslation(CPoint(0, 0, 0));
	//Now attach it to the king.
	pobKingAttachTransform->AddChild(pobDemonAttachTransform);

	//Disable the physics on the demon, we'll be manually controlling it up until it detaches, and then again once it's re-attached.
	if(pobDemon->GetPhysicsSystem())
	{
		pobDemon->GetPhysicsSystem()->Deactivate();
	}

	//Allow the demon and king to overlap.
	if(pobDemon->GetInteractionComponent())
	{
		pobDemon->GetInteractionComponent()->ExcludeCollisionWith(this);
	}

	//Store a pointer to the demon here.
	m_pobDemon = (Boss*)pobDemon;

	ntError_p(m_pobDemon->GetBossType() == BT_DEMON, ("We should've just created a demon!") );

	//Send the demon a pointer to ourself.
	((Demon*)m_pobDemon)->SetKingPointer(this);
}

bool KingBohan::CanStartAnAttack()
{
	return GetAttackComponent()->AI_Access_GetState() == CS_STANDARD;
}

bool KingBohan::CanStartALinkedAttack()
{
	return GetAttackComponent()->IsInNextMoveWindow() && !GetAttackComponent()->HasSelectedANextMove();
}


void KingBohan::SetInSpecialAttack(bool bInSpecialAttack, const char* pcAttackName)
{
	m_bInSpecialAttack = bInSpecialAttack;
#ifdef KING_STORE_CURRENT_SPECIAL_NAME
	if(!m_bInSpecialAttack)
	{
		m_obInSpecialName.clear();
	}
	else
	{
		m_obInSpecialName = pcAttackName;
	}
#else
	UNUSED(pcAttackName);
#endif
}


bool KingBohan::HasAttachedDemon()
{
	ntError_p(m_pobDemon && (m_pobDemon->GetBossType() == Boss::BT_DEMON), ("Missing demon for king"));
	
	return ((Demon*)m_pobDemon)->IsAttachedToKing();
}


void KingBohan::GoToPhase(unsigned int iPhaseNumber)
{
	if((iPhaseNumber < 1) || (iPhaseNumber > 4))
	{
		return;
	}

	//Phase-4 just means drop all the way to 5 health so that the ninja-sequence can be activated.
	if(iPhaseNumber == 4)
	{
		ntPrintf("TAKE THE KING TO 5 HEALTH (ready for superstyle/ninja-sequence)!\n");
		float fCurrHealth = ToCharacter()->GetCurrHealth() - 5;
		ToCharacter()->ChangeHealth(-fCurrHealth, "Forcing King to end of fight");

		//To stop his health from jumping straight back up we need to set any other internal-milestone variables for the king.
		m_iCurrentRetreatPoint = KING_NUM_RETREAT_POINTS - 1;	//Last retreat-point.
	}
}

void KingBohan::UpdateBossSpecifics(float fTimeDelta)
{
	UNUSED(fTimeDelta);

	//Limit the health of the king at specific levels depending on which retreat-point he's at.
	//As soon as he selects his next movement he should choose to retreat anyway and then this limit is moved to the next
	//one!
	if(m_iCurrentRetreatPoint < KING_NUM_RETREAT_POINTS)
	{
		if(m_iCurrentRetreatPoint < KING_NUM_RETREAT_POINTS - 1)
		{
			float fCurrHealth = ToCharacter()->GetCurrHealth();
			int fCurrHealthLimit = m_aobRetreatHealths[m_iCurrentRetreatPoint];
			if(fCurrHealth < fCurrHealthLimit)
			{
				float fChange = (float)(fCurrHealthLimit - fCurrHealth);
				ToCharacter()->ChangeHealth(fChange, "Keeping king above threshold till next retreat done");
				//We want to be retreating for one reason or the other, never both (two different points).
				//It's okay to flag this here, as at the next movement selection we'll retreat due to health value.
				//That should take priority over distance-based retreat.
				m_bRetreatingOnHealthBasis = true;
				m_bTooFarFromCurrentRetreatPoint = false;
			}
			else
			{
				//Account for whether it's exactly on the health limit too (to set the retreating flags).
				if((fCurrHealth == fCurrHealthLimit) && (!m_bRetreatingOnHealthBasis))
				{
					m_bRetreatingOnHealthBasis = true;
					m_bTooFarFromCurrentRetreatPoint = false;
				}
			}
		}

		//Only check the too-far stuff if we're not already retreating to a new point at a specific health level.
		//Also only check this if he's not currently off in a big flying-about attack (we can tell this because he'll be flagged
		//as m_bInSpecialAttack). This stops him from being flagged as "too far from retreat point" just because he has
		//flown off in a big attack, after which the attack returns him to his normal position but then he retreats anyway. This
		//can cause some silly-looking very-short retreats, so we avoid it here.
		if(!m_bInSpecialAttack)	//Read: "If not off doing a big attack which might take us too far from point temporarily".
		{
			if((!m_bTooFarFromCurrentRetreatPoint) && (!m_bRetreatingOnHealthBasis))
			{
				//Check if we're too far from our current point.
				CPoint obPosition = GetPosition();
				CPoint obRetreatPoint = m_aobRetreatPoints[m_iCurrentRetreatPoint];

				CDirection obDirToPoint = CDirection(obRetreatPoint - obPosition);
				float fDistanceFromPoint = obDirToPoint.Length();
				float fDistanceAllowedOnPoint = m_aobRetreatPointDistances[m_iCurrentRetreatPoint];

				if(fDistanceFromPoint > fDistanceAllowedOnPoint)
				{
					m_bTooFarFromCurrentRetreatPoint = true;
					ntPrintf("TOO FAR - Allowed[%4.4f] - Current[%4.4f]!\n", fDistanceAllowedOnPoint, fDistanceFromPoint);
				}
			}
		}
	}

	//Calculate whether or not the player is currently in the current-retreat-point/radius range.
	//We do this just once here, then store the result for use in the selectors.
	CPoint obCurrRetreatPointPos = m_aobRetreatPoints[m_iCurrentRetreatPoint];
	float fRetreatPointRange = m_aobRetreatPointDistances[m_iCurrentRetreatPoint];
	CEntity* pobPlayer = CEntityManager::Get().GetPlayer();
	if(pobPlayer)	//Should always be true.
	{
		//Check if the player is within range of the king's retreat point and range.
		CPoint obPlayerPos = pobPlayer->GetPosition();
		CDirection obBetween = CDirection(obPlayerPos - obCurrRetreatPointPos);
		float fDistSquared = obBetween.LengthSquared();
		//If the player is too far away, return 0.0f on this.
		if(fDistSquared >= (fRetreatPointRange * fRetreatPointRange))
		{
			//Out of range
			m_bPlayerInRetreatPointRadiusRange = false;
		}
		else
		{
			//In range.
			m_bPlayerInRetreatPointRadiusRange = true;
		}
	}
}


void KingBohan::DebugRenderBossSpecifics( CPoint& obScreenLocation, float fXOffset, float fYOffset )
{
#ifndef _GOLD_MASTER

	//For development purposes we want to be able to see whether or not the player is currently in-range of the king's retreat or not
	//as this will affect which attack selectors he uses.
	if(!m_bPlayerInRetreatPointRadiusRange)
	{
		g_VisualDebug->Printf3D(obScreenLocation, fXOffset, fYOffset, DC_CYAN, 0,
			"Player is out of retreat-point/radius range - KingRetreatPoint[%d of %d]", m_iCurrentRetreatPoint + 1, KING_NUM_RETREAT_POINTS);
	}
	else
	{
		g_VisualDebug->Printf3D(obScreenLocation, fXOffset, fYOffset, DC_CYAN, 0,
			"Player is in retreat-point/radius range - KingRetreatPoint[%d of %d]", m_iCurrentRetreatPoint + 1, KING_NUM_RETREAT_POINTS);
	}

#ifndef _RELEASE
	//Render each of the retreat points and radii.
	for(int i = 0 ; i < KING_NUM_RETREAT_POINTS ; i++)
	{
		CPoint obRetreatPoint = m_aobRetreatPoints[i];
		obRetreatPoint.Y() += 1.0f;	//Just so that it doesn't clip with the geometry/terrain.
		//Build a matrix for the point (for rendering purposes).
		CMatrix obPointMatrix;
		obPointMatrix.SetIdentity();
		obPointMatrix.SetTranslation(obRetreatPoint);
		if(m_iCurrentRetreatPoint == i)
		{
			g_VisualDebug->RenderPoint(obRetreatPoint, 10.0f, DC_RED);
			g_VisualDebug->RenderArc(obPointMatrix, m_aobRetreatPointDistances[i], TWO_PI, DC_RED);
		}
		else
		{
			g_VisualDebug->RenderPoint(obRetreatPoint, 10.0f, DC_BLUE);
			g_VisualDebug->RenderArc(obPointMatrix, m_aobRetreatPointDistances[i], TWO_PI, DC_BLUE);
		}
	}
#endif
#endif
}



//===================== RETREAT MOVEMENT TRANSITION ====================
START_STD_INTERFACE(KingRetreatToPointMovement)
	COPY_INTERFACE_FROM(BossTransitioningMovement)
	DEFINE_INTERFACE_INHERITANCE(BossTransitioningMovement)

	PUBLISH_VAR_AS(m_obRetreatStartAnim, RetreatStartAnim)
	PUBLISH_VAR_AS(m_obRetreatCycleAnim, RetreatCycleAnim)
	PUBLISH_VAR_AS(m_obRetreatStopAnim, RetreatStopAnim)
END_STD_INTERFACE


KingRetreatToPointMovement::KingRetreatToPointMovement()
{
	m_bDone = false;
	m_bReachedTarget = false;
}


BossMovement* KingRetreatToPointMovement::Initialise(Boss* pobBoss, CEntity* pobPlayer)
{
	//Check that this is on a king!
	ntError_p( pobBoss->GetBossType() == Boss::BT_KING_BOHAN, ("Should not attach a KingRetreatToPointMovement on anyone other than the king!") );
	KingBohan* pobKing = (KingBohan*)pobBoss;

	//Exit-early if we're already at the last point and not just returning to our current point!
	if((pobKing->m_iCurrentRetreatPoint >= KING_NUM_RETREAT_POINTS - 1) && (!pobKing->m_bTooFarFromCurrentRetreatPoint))
	{
		//Failed to initialise, any selector should just move-on now and select some other movement.
		return 0;
	}

	//If we're already in the middle of an attack, then don't attempt to retreat... ever.
	if(pobKing->IsInSpecialAttack())
	{
		ntPrintf("***###*** KingRetreatToPointMovement: SHOULD NOT HAVE HAPPENED (attempting to start retreat-to-point movement during special attack [%s]) ***###***\n",
			pobKing->GetCurrentSpecialName());
		return 0;
	}


	int iCurrentRetreatPoint = pobKing->m_iCurrentRetreatPoint;
	CPoint obPointToRetreatTo = CPoint(CONSTRUCT_CLEAR);

	//Moving on to the next point takes priority.
	if(!pobKing->m_bTooFarFromCurrentRetreatPoint)
	{
		obPointToRetreatTo = pobKing->m_aobRetreatPoints[iCurrentRetreatPoint + 1];
	}
	else
	{
		obPointToRetreatTo = pobKing->m_aobRetreatPoints[iCurrentRetreatPoint];
	}

	CDirection obRetreatVector = CDirection( obPointToRetreatTo - pobKing->GetPosition() );

	//When we start retreating, we turn towards the point.
	ZAxisAlignTargetedTransitionDef obRetreatStartAlignDef;
	obRetreatStartAlignDef.SetDebugNames("King Retreat Start", "ZAxisAlignTargetedTransitionDef");
	obRetreatStartAlignDef.m_bApplyGravity = false;
	obRetreatStartAlignDef.m_obAlignZTo = -obRetreatVector;
	obRetreatStartAlignDef.m_obAlignZTo.Normalise();
	obRetreatStartAlignDef.m_obAnimationName = m_obRetreatStartAnim;

	TargetedTransitionToPointDef obRetreatCycle;
	obRetreatCycle.SetDebugNames("King Retreat Cycle", "TargetedTransitionToPointDef");
	obRetreatCycle.m_bApplyGravity = false;
	obRetreatCycle.m_fExtraSpeed = m_fSpeed;
	obRetreatCycle.m_obAnimationName = m_obRetreatCycleAnim;
	obRetreatCycle.m_obPoint = obPointToRetreatTo;
	obRetreatCycle.m_bTravellingBackwards = true;
	obRetreatCycle.m_fRadius = 3.0f;	//A fair sized radius around our target point so that we don't ever go past it.

	pobKing->GetMovement()->BringInNewController( obRetreatStartAlignDef, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND );
	pobKing->GetMovement()->AddChainedController( obRetreatCycle, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND );

	Message obMovementMessage(msg_movementdone);
	pobKing->GetMovement()->SetCompletionMessage(obMovementMessage, pobBoss);

	m_bDone = false;
	m_bReachedTarget = false;

	//Store the boss pointer.
	m_pobBoss = pobBoss;

	//Tell our demon to change it's effect to the retreat effect.
	Demon* pobDemon = (Demon*)pobKing->GetDemon();
	ntError_p(pobDemon && (pobDemon->GetBossType() == Boss::BT_DEMON), ("KingRetreatToPointMovement::Initialise(), Invalid demon pointer from king"));
	pobDemon->SetDemonEffectRetreat();

	//Successfully initialised.
	return this;
}


BossMovement* KingRetreatToPointMovement::DoMovement(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	if(!m_bDone)
	{
		return this;
	}

	//Don't update the point until we actually reach our target, and then only if we weren't just returning to
	//our current retreat-point (having strayed too far from it during combat!).
	KingBohan* pobKing = (KingBohan*)pobBoss;
	if(!pobKing->m_bTooFarFromCurrentRetreatPoint)
	{
		pobKing->m_iCurrentRetreatPoint++;
	}
	
	//Clear our retreating flags.
	pobKing->m_bTooFarFromCurrentRetreatPoint = false;
	pobKing->m_bRetreatingOnHealthBasis = false;

	//Tell our demon to return to it's normal effect.
	Demon* pobDemon = (Demon*)pobKing->GetDemon();
	ntError_p(pobDemon && (pobDemon->GetBossType() == Boss::BT_DEMON), ("KingRetreatToPointMovement::DoMovement(), Invalid demon pointer from king"));
	pobDemon->SetDemonEffectStandard();

	return 0;
}


void KingRetreatToPointMovement::NotifyMovementDone()
{
	//If this is the end of our "move to point" transition, queue up our end z-axis targeted transition.
	if(!m_bReachedTarget)
	{
		KingBohan* pobKing = (KingBohan*)m_pobBoss;
		ntError_p(pobKing, ("King pointer should've been stored on retreat to point movement transition on-initialise!"));
		CEntity* pobPlayer = CEntityManager::Get().GetPlayer();
		ntError_p(pobPlayer, ("KingRetreatToPointMovement: Could not get pointer to player from entity manager!"));

		//When we finish retreating, we face the player again (maybe the anim should be a taunt or something?)
		ZAxisAlignTargetedTransitionDef obRetreatEndAlignDef;
		obRetreatEndAlignDef.SetDebugNames("King Retreat End", "ZAxisAlignTargetedTransitionDef");
		obRetreatEndAlignDef.m_bApplyGravity = false;
		obRetreatEndAlignDef.m_pobEntityAlignZTowards = pobPlayer;
		obRetreatEndAlignDef.m_obAnimationName = m_obRetreatStopAnim;

		//Bring in the new controller.
		pobKing->GetMovement()->BringInNewController( obRetreatEndAlignDef, CMovement::DMM_STANDARD, CMovement::MOVEMENT_BLEND );

		//Make sure we're notified when this movement is finished.
		Message obMovementMessage(msg_movementdone);
		pobKing->GetMovement()->SetCompletionMessage(obMovementMessage, pobKing);

		m_bReachedTarget = true;
	}
	//If this is the end overall, then flag this movement as done.
	else
	{
		m_bDone = true;
	}
}

//===================== MOVEMENT SELECTORS ====================

START_STD_INTERFACE(KingRetreatMovementSelector)
	COPY_INTERFACE_FROM(BossMovementSelector)
	DEFINE_INTERFACE_INHERITANCE(BossMovementSelector)

	PUBLISH_VAR_WITH_DEFAULT_AS(m_bIsAttachedToDemon, false, IsAttachedToDemon)

END_STD_INTERFACE


KingRetreatMovementSelector::KingRetreatMovementSelector()
{
	m_bIsAttachedToDemon = false;
}


float KingRetreatMovementSelector::GetPriority(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	//Check that this is on a king!
	ntError_p( pobBoss->GetBossType() == Boss::BT_KING_BOHAN, ("Should not attach a KingRetreatMovementSelector on anyone other than the king!") );
	KingBohan* pobKing = (KingBohan*)pobBoss;

	//Whether we want this one to be selected when in the air (perhaps it has a different set of anims etc) or not should
	//be factored in.
	Demon* pobDemon = (Demon*)pobKing->GetDemon();
	if(pobDemon->IsAttachedToKing() == false)
	{
		return m_fMinPriority;
	}

	if(pobKing->m_iCurrentRetreatPoint < KING_NUM_RETREAT_POINTS - 1)
	{
		float fBossHealth = pobKing->ToCharacter()->GetCurrHealth();
		int iCurrRetreatLevelHealth = pobKing->m_aobRetreatHealths[pobKing->m_iCurrentRetreatPoint];

		if(fBossHealth == iCurrRetreatLevelHealth)
		{
			return m_fMaxPriority;
		}
	}

	if(pobKing->m_bTooFarFromCurrentRetreatPoint)
	{
		return m_fMaxPriority;
	}

	return m_fMinPriority;
}

BossMovement* KingRetreatMovementSelector::GetSelectedMovement()
{
	if (m_obMovements.size() > 0)
	{
		return (*m_obMovements.begin());
	}
	
	return 0; 
}

BossMovement* KingRetreatMovementSelector::DoMovement(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	if (m_obMovements.size() > 0)
	{
		return (*m_obMovements.begin())->DoMovement(fTimeDelta,pobBoss,pobPlayer);
	}
	
	return 0;
}



START_STD_INTERFACE(KingTauntMovementSelector)
	COPY_INTERFACE_FROM(BossTauntMovementSelector)
	DEFINE_INTERFACE_INHERITANCE(BossTauntMovementSelector)

	PUBLISH_VAR_WITH_DEFAULT_AS(m_bAttachedToDemon, false, WhileAttachedToDemon)
END_STD_INTERFACE



KingTauntMovementSelector::KingTauntMovementSelector()
{
	m_bAttachedToDemon = false;
}

float KingTauntMovementSelector::GetPriority(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	ntError_p(pobBoss->GetBossType() == Boss::BT_KING_BOHAN, ("Should only be applying KingTauntMovementSelector to the king!"));
	KingBohan* pobKing = (KingBohan*)pobBoss;
	ntError_p(pobKing->GetDemon() && pobKing->GetDemon()->GetBossType() == Boss::BT_DEMON, ("Demon on king is either missing or wrong-type!"));
	Demon* pobDemon = (Demon*)pobKing->GetDemon();

	//Check that this selector is designed for the current attached-state (different taunts for attached-to-demon and not (flying/not)).
	if(pobDemon->IsAttachedToKing() != m_bAttachedToDemon)
	{
		return m_fMinPriority;
	}

	if(pobBoss->GetTimeSinceLastPlayerStrike() >= m_fTimeSinceAttackBeforeTaunt)
	{
		//Get the boss and player's position to check for range.
		CPoint obBossPos = pobBoss->GetPosition();
		CPoint obPlayerPos = pobPlayer->GetPosition();
		CDirection obBetween = CDirection(obBossPos - obPlayerPos);
		float fLengthSquared = obBetween.LengthSquared();
		if((fLengthSquared >= (m_fMinDistanceToPlayer * m_fMinDistanceToPlayer)) &&
			(fLengthSquared <= (m_fMaxDistanceToPlayer * m_fMaxDistanceToPlayer)))
		{
			//We're going to taunt, so clear our timer!
			pobBoss->ResetTimeSinceLastPlayerStrike();
			//Everything checks out, taunt!
			return m_fMaxPriority;
		}
	}

	return m_fMinPriority;
}



START_STD_INTERFACE(KingMovementSelector)
	COPY_INTERFACE_FROM(BossMovementSelector)
	DEFINE_INTERFACE_INHERITANCE(BossMovementSelector)

	PUBLISH_VAR_WITH_DEFAULT_AS(m_bIsAttachedToDemon, false, IsAttachedToDemon)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bOnlyIfPlayerIsInRangeOfRetreat, true, OnlyIfPlayerIsInRangeOfRetreat)
END_STD_INTERFACE



START_STD_INTERFACE(RandomKingMovementSelector)
	COPY_INTERFACE_FROM(KingMovementSelector)
	DEFINE_INTERFACE_INHERITANCE(KingMovementSelector)
END_STD_INTERFACE


float RandomKingMovementSelector::GetPriority(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer) 
{ 
	UNUSED(fTimeDelta); UNUSED(pobBoss); UNUSED(pobPlayer); 
	
	ntError_p( pobBoss->GetBossType() == Boss::BT_KING_BOHAN, ("RandomKingMovementSelector should only be put on king") );
	KingBohan* pobKing = (KingBohan*)pobBoss;
	Demon* pobDemon = (Demon*)pobKing->GetDemon();

	//If this is only supposed to be selected if the player is in-range of the king's retreat-point/radius
	//then check that the player IS. If not, return 0.0f.
	if(m_bOnlyIfPlayerIsInRangeOfRetreat)
	{
		if(!pobKing->m_bPlayerInRetreatPointRadiusRange)
		{
			return 0.0f;
		}
	}

	if ((m_obMovements.size() > 0) && (pobDemon->IsAttachedToKing() == m_bIsAttachedToDemon))
	{
		m_iSelectedMovement = rand() % m_obMovements.size();
		return m_fMaxPriority;
	}
	else
		return 0.0f;
};


BossMovement* RandomKingMovementSelector::GetSelectedMovement() 
{
	if (m_obMovements.size() > 0)
	{
		ntstd::List<BossMovement*>::iterator obIt = m_obMovements.begin();
		for (int i = 0; i < m_iSelectedMovement; i++)
			obIt++;
		return (*obIt); 
	}
	else
		return 0; 
};


BossMovement* RandomKingMovementSelector::DoMovement(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer) 
{
	ntError_p( pobBoss->GetBossType() == Boss::BT_KING_BOHAN, ("RandomKingMovementSelector should only be put on king") );
	KingBohan* pobKing = (KingBohan*)pobBoss;
	Demon* pobDemon = (Demon*)pobKing->GetDemon();

	if(pobDemon->IsAttachedToKing() != m_bIsAttachedToDemon)
	{
		ntError_p( 0, ("King's demon-attachment does not match demon-attachment state on random movement selector. This should have been checked-against already!") );
	}

	if (m_obMovements.size() > 0)
	{
		ntstd::List<BossMovement*>::iterator obIt = m_obMovements.begin();
		for (int i = 0; i < m_iSelectedMovement; i++)
			obIt++;
		return (*obIt)->DoMovement(fTimeDelta,pobBoss,pobPlayer); 
	}
	else
		return 0;
};


//===================== ATTACK SELECTORS ====================

START_STD_INTERFACE(KingAttackSelector)
	COPY_INTERFACE_FROM(BossAttackSelector)
	DEFINE_INTERFACE_INHERITANCE(BossAttackSelector)

	PUBLISH_VAR_WITH_DEFAULT_AS(m_bIsAttachedToDemon, false, IsAttachedToDemon)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bOnlyIfPlayerIsInRangeOfRetreat, true, OnlyIfPlayerIsInRangeOfRetreat)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bOnlyIfPlayerIsOutsideRangeOfRetreat, false, OnlyIfPlayerIsOutsideRangeOfRetreat)
END_STD_INTERFACE



START_STD_INTERFACE(RandomKingAttackSelector)
	COPY_INTERFACE_FROM(KingAttackSelector)
	DEFINE_INTERFACE_INHERITANCE(KingAttackSelector)
END_STD_INTERFACE


//--------------------------------------------------
//!
//! RandomKingAttackSelector
//!
//--------------------------------------------------
float RandomKingAttackSelector::GetPriority(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	ntError_p( pobBoss->GetBossType() == Boss::BT_KING_BOHAN, ("RandomKingAttackSelector should only be put on king") );
	KingBohan* pobKing = (KingBohan*)pobBoss;
	Demon* pobDemon = (Demon*)pobKing->GetDemon();

	//If this is only supposed to be selected if the player is in-range or out-of-range of the king's retreat-point/radius
	//then check that the player IS. If not, return 0.0f.
	//If both m_bOnlyIfPlayerIsInRangeOfRetreat and m_bOnlyIfPlayerIsOutsideRangeOfRetreat are toggled, in-range takes priority.
	if(m_bOnlyIfPlayerIsInRangeOfRetreat)
	{
		if(!pobKing->m_bPlayerInRetreatPointRadiusRange)
		{
			return 0.0f;
		}
	}
	else if(m_bOnlyIfPlayerIsOutsideRangeOfRetreat)
	{
		if(pobKing->m_bPlayerInRetreatPointRadiusRange)
		{
			return 0.0f;
		}
	}
	
	if ((m_obAttacks.size() > 0) && (pobDemon->IsAttachedToKing() == m_bIsAttachedToDemon))
		return m_fMaxPriority;
	else
		return 0.0f;
}

BossAttack* RandomKingAttackSelector::BeginAttack(Boss* pobBoss, CEntity* pobPlayer)
{
	ntError_p( pobBoss->GetBossType() == Boss::BT_KING_BOHAN, ("RandomKingAttackSelector should only be put on king") );
	KingBohan* pobKing = (KingBohan*)pobBoss;
	Demon* pobDemon = (Demon*)pobKing->GetDemon();

	if (pobDemon->IsAttachedToKing() != m_bIsAttachedToDemon)
	{
		ntError_p( 0, ("King's demon-attachment does not match demon-attachment state on random attack selector. This should have been checked-against already!") );
	}

	if (m_obAttacks.size() > 0)
	{
		return ChooseRandomAttackWithWeighting(pobBoss, pobPlayer);
	}

	return 0;
}


START_STD_INTERFACE(DistanceSuitabilityKingAttackSelector)
	COPY_INTERFACE_FROM(KingAttackSelector)
	DEFINE_INTERFACE_INHERITANCE(KingAttackSelector)

	PUBLISH_VAR_WITH_DEFAULT_AS(m_fTimeBeforeRepeatingSelection, 0.0f, TimeBeforeRepeatingSelection)
END_STD_INTERFACE

//--------------------------------------------------
//!
//! DistanceSuitabilityBossAttackSelector
//!
//--------------------------------------------------
float DistanceSuitabilityKingAttackSelector::GetPriority(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	ntError_p( pobBoss->GetBossType() == Boss::BT_KING_BOHAN, ("DistanceSuitabilityKingAttackSelector should only be put on king.") );
	KingBohan* pobKing = (KingBohan*)pobBoss;
	Demon* pobDemon = (Demon*)pobKing->GetDemon();

	//Don't select this if it has been too soon.
	m_fTimeSinceLastSelection += fTimeDelta;
	if(m_fTimeSinceLastSelection < m_fTimeBeforeRepeatingSelection)
	{
		return 0.0f;
	}

	//If this is only supposed to be selected if the player is in-range or out-of-range of the king's retreat-point/radius
	//then check that the player IS. If not, return 0.0f.
	//If both m_bOnlyIfPlayerIsInRangeOfRetreat and m_bOnlyIfPlayerIsOutsideRangeOfRetreat are toggled, in-range takes priority.
	if(m_bOnlyIfPlayerIsInRangeOfRetreat)
	{
		if(!pobKing->m_bPlayerInRetreatPointRadiusRange)
		{
			return 0.0f;
		}
	}
	else if(m_bOnlyIfPlayerIsOutsideRangeOfRetreat)
	{
		if(pobKing->m_bPlayerInRetreatPointRadiusRange)
		{
			return 0.0f;
		}
	}

	if (pobDemon->IsAttachedToKing() == m_bIsAttachedToDemon)
	{
		// Get the average distance of our attacks, if we're within this average distance, select randomly from our pool
		CPoint obPlayerPosition = pobPlayer->GetPosition();
		CPoint obBossPosition = pobBoss->GetPosition();
		CDirection obBossToPlayer = CDirection( obPlayerPosition - obBossPosition );
		float fDistance = obBossToPlayer.Length();

		// Get average strike proxy for all our attacks
		ntstd::List<BossAttack*>::iterator obIt = m_obAttacks.begin();
		ntstd::List<BossAttack*> obInRangeAttacks;
		while (obIt != m_obAttacks.end())
		{
			if (fDistance < (*obIt)->GetMaxDistance())
			{
				obInRangeAttacks.push_back((*obIt));
			}
			obIt++;
		}

		//If we have any attacks within range, return maximum priority (BeginAttack() then handles picking one and initialising it).
		if(obInRangeAttacks.size() > 0)
		{
			return m_fMaxPriority;
		}
	}

	// Else we can't do anything at the moment
	m_pobSelectedAttack = 0;
	return m_fMinPriority;
}


BossAttack* DistanceSuitabilityKingAttackSelector::BeginAttack(Boss* pobBoss, CEntity* pobPlayer)
{
	// Get the average distance of our attacks, if we're within this average distance, select randomly from our pool
	CPoint obPlayerPosition = pobPlayer->GetPosition();
	CPoint obBossPosition = pobBoss->GetPosition();
	CDirection obBossToPlayer = CDirection( obPlayerPosition - obBossPosition );
	float fDistance = obBossToPlayer.Length();

	// Get average strike proxy for all our attacks
	ntstd::List<BossAttack*>::iterator obIt = m_obAttacks.begin();
	ntstd::List<BossAttack*> obInRangeAttacks;
	while (obIt != m_obAttacks.end())
	{
		if (fDistance < (*obIt)->GetMaxDistance())
		{
			obInRangeAttacks.push_back((*obIt));
		}
		obIt++;
	}

	// If we're in range...
	if (obInRangeAttacks.size() == 1)
	{
		m_pobSelectedAttack = *obInRangeAttacks.begin();
		if(m_pobSelectedAttack && m_pobSelectedAttack->Initialise(pobBoss, pobPlayer))
		{
			m_fTimeSinceLastSelection = 0.0f;
			return m_pobSelectedAttack;
		}
		else
		{
			return 0;
		}
	}
	else if (obInRangeAttacks.size() > 1)
	{
		m_pobSelectedAttack = ChooseRandomAttackWithWeighting(pobBoss,pobPlayer,&obInRangeAttacks);
		m_fTimeSinceLastSelection = 0.0f;
		return m_pobSelectedAttack;
	}

	//Shouldn't get here, max-priority is only returned from the selector if we have 1 or more in-range attacks.
	ntError_p( false, ("DistanceSuitabilityKingAttackSelector: Should not have got here.") );
	return 0;
}


START_STD_INTERFACE(KingDetachDemonAttackSelector)
	COPY_INTERFACE_FROM(KingAttackSelector)
	DEFINE_INTERFACE_INHERITANCE(KingAttackSelector)

	PUBLISH_VAR_WITH_DEFAULT_AS(m_fMinTime, 30.0f, MinTimeTillDetach)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fMaxTime, 50.0f, MaxTimeTillDetach)
END_STD_INTERFACE


KingDetachDemonAttackSelector::KingDetachDemonAttackSelector()
{
	m_bRandomTimeGenerated = false;
	m_fTimeTillDetach = 40.0f;
}


//--------------------------------------------------
//!
//! KingDetachDemonAttackSelector
//!
//--------------------------------------------------
float KingDetachDemonAttackSelector::GetPriority(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	ntError_p( pobBoss->GetBossType() == Boss::BT_KING_BOHAN, ("KingDetachDemonAttackSelector should only be put on king") );
	KingBohan* pobKing = (KingBohan*)pobBoss;
	Demon* pobDemon = (Demon*)pobKing->GetDemon();

	//Each time this has been selected we'll want to re-generate a random time between our min and max times and store it.
	if(m_bRandomTimeGenerated == false)
	{
		//Just to be picky...
		ntError_p(m_fMaxTime >= m_fMinTime, ("KingDetachDemonAttackSelector: MaxTime smaller than MinTime"));

		//Generate our time.
		float fDifference = m_fMaxTime - m_fMinTime;
		float fRandDifference = grandf(fDifference);
		
		//Store it.
		this->m_fTimeTillDetach = m_fMinTime + fRandDifference;
		
		//Flag that we've generated a time.
		m_bRandomTimeGenerated = true;
	}

	//We don't want to detach if we're at our last retreat point. This guarantees that when the king dies he still has the demon
	//attached. We want to ensure this because in the Ninja Sequence he has the demon attached already!
	if(pobKing->m_iCurrentRetreatPoint >= KING_NUM_RETREAT_POINTS - 1)
	{
		return 0.0f;
	}

	//We don't want to detach if the demon is already detached.
	if(pobDemon->IsAttachedToKing() == false)
	{
		return 0.0f;
	}

	//This "attack" takes priority if the demon has been attached for more than the generated amount of time (and there is an attack!)
	if((pobDemon->GetTimeSinceLastAttachStateChange() >= m_fTimeTillDetach) && (m_obAttacks.size() > 0))
	{
		//We don't want to detach if it's not safe too (we might be in the middle of a special high up in the air!)
		//Note: Code-wise this is still safe to do, it won't break or anything, but it looks silly, so we return 0 here instead.
		if(pobKing->IsInSpecialAttack())
		{
			return 0.0f;
		}

		//We don't want to detach if the player is outside of the radius from the king's retreat point. If it did, the king wouldn't
		//have any of his specials to reach the player with, and the demon wouldn't be able to reach the player at that range anyway.
		//So we keep it attached. This is done as a FINAL check before returning m_fMaxPriority in the hope that more often than not
		//these checks can be skipped.
		CPoint obCurrRetreatPointPos = pobKing->m_aobRetreatPoints[pobKing->m_iCurrentRetreatPoint];
		float fRetreatPointRange = pobKing->m_aobRetreatPointDistances[pobKing->m_iCurrentRetreatPoint];
		CPoint obPlayerPos = pobPlayer->GetPosition();
		CDirection obBetween = CDirection(obPlayerPos - obCurrRetreatPointPos);
		float fDistSquared = obBetween.LengthSquared();
		//If the player is too far away, return 0.0f on this.
		if(fDistSquared >= (fRetreatPointRange * fRetreatPointRange))
		{
			return 0.0f;
		}

		return m_fMaxPriority;
	}

	return 0.0f;
}


BossAttack* KingDetachDemonAttackSelector::BeginAttack(Boss* pobBoss, CEntity* pobPlayer)
{
	ntError_p( pobBoss->GetBossType() == Boss::BT_KING_BOHAN, ("KingDetachDemonAttackSelector should only be put on king") );
	KingBohan* pobKing = (KingBohan*)pobBoss;
	Demon* pobDemon = (Demon*)pobKing->GetDemon();

	if(pobDemon->IsAttachedToKing() == false)
	{
		ntError_p( 0, ("KingDetachDemonAttackSelector should never have been selected if the demon wasn't currently attached!") );
	}

	if(m_obAttacks.size() > 0)
	{
		//We're going to select our attack, so flag the selector as needing a new random time for next-time.
		m_bRandomTimeGenerated = false;

		//There should only be the one attack here, so we pick the first one.
		if((*m_obAttacks.begin())->Initialise(pobBoss, pobPlayer))
		{
			return *m_obAttacks.begin();
		}
	}

	return 0;
}



START_STD_INTERFACE(KingAttachToDemonAttackSelector)
	COPY_INTERFACE_FROM(KingAttackSelector)
	DEFINE_INTERFACE_INHERITANCE(KingAttackSelector)

	PUBLISH_VAR_WITH_DEFAULT_AS(m_fMinTime, 30.0f, MinTimeTillAttach)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fMaxTime, 50.0f, MaxTimeTillAttach)
END_STD_INTERFACE


KingAttachToDemonAttackSelector::KingAttachToDemonAttackSelector()
{
	m_bRandomTimeGenerated = false;
	m_fTimeTillAttach = 40.0f;
}


//--------------------------------------------------
//!
//! KingAttachToDemonAttackSelector
//!
//--------------------------------------------------
float KingAttachToDemonAttackSelector::GetPriority(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	ntError_p( pobBoss->GetBossType() == Boss::BT_KING_BOHAN, ("KingAttachToDemonAttackSelector should only be put on king") );
	KingBohan* pobKing = (KingBohan*)pobBoss;
	Demon* pobDemon = (Demon*)pobKing->GetDemon();

	//Each time this has been selected we'll want to re-generate a random time between our min and max times and store it.
	if(m_bRandomTimeGenerated == false)
	{
		//Just to be picky...
		ntError_p(m_fMaxTime >= m_fMinTime, ("KingAttachToDemonAttackSelector: MaxTime smaller than MinTime"));

		//Generate our time.
		float fDifference = m_fMaxTime - m_fMinTime;
		float fRandDifference = grandf(fDifference);
		
		//Store it.
		this->m_fTimeTillAttach = m_fMinTime + fRandDifference;
		
		//Flag that we've generated a time.
		m_bRandomTimeGenerated = true;
	}

	//We don't want to Attach if the demon is already attached.
	if(pobDemon->IsAttachedToKing() == true)
	{
		return 0.0f;
	}

	//This "attack" takes priority if the demon has been detached for more than the generated amount of time (and there is an attack!)
	if((pobDemon->GetTimeSinceLastAttachStateChange() >= m_fTimeTillAttach) && (m_obAttacks.size() > 0))
	{
		//If we're already in the middle of an attack, then don't try to re-attach.
		if(pobKing->IsInSpecialAttack())
		{
			return 0.0f;
		}

		return m_fMaxPriority;
	}

	return 0.0f;
}


BossAttack* KingAttachToDemonAttackSelector::BeginAttack(Boss* pobBoss, CEntity* pobPlayer)
{
	ntError_p( pobBoss->GetBossType() == Boss::BT_KING_BOHAN, ("KingAttachToDemonAttackSelector should only be put on king") );
	KingBohan* pobKing = (KingBohan*)pobBoss;
	Demon* pobDemon = (Demon*)pobKing->GetDemon();

	if(pobDemon->IsAttachedToKing() == true)
	{
		ntError_p( 0, ("KingAttachToDemonAttackSelector should never have been selected if the demon was already attached!") );
	}

	if(m_obAttacks.size() > 0)
	{
		//We're going to select our attack, so flag the selector as needing a new random time for next-time.
		m_bRandomTimeGenerated = false;

		//There should only be the one attack here, so we pick the first one.
		if((*m_obAttacks.begin())->Initialise(pobBoss, pobPlayer))
		{
			return *m_obAttacks.begin();
		}
	}

	return 0;
}


//===================== SELECTOR CONDITIONS (for generic attack/movement selectors) ====================

START_STD_INTERFACE(GeneralBossAttackSelectorKingHasDemonAttached)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bDesiredAttachedState, true, TrueIfAttached)
END_STD_INTERFACE

bool GeneralBossAttackSelectorKingHasDemonAttached::IsSatisfied(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	ntError_p(pobBoss->GetBossType() == Boss::BT_KING_BOHAN, ("GeneralBossAttackSelectorKingHasDemonAttached should only be put on king!"));
	KingBohan* pobKing = (KingBohan*)pobBoss;

	if(pobKing->HasAttachedDemon() == m_bDesiredAttachedState)
	{
		return true;
	}

	//Not what we wanted.
	return false;
}


START_STD_INTERFACE(GeneralBossMovementSelectorKingHasDemonAttached)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bDesiredAttachedState, true, TrueIfAttached)
END_STD_INTERFACE

bool GeneralBossMovementSelectorKingHasDemonAttached::IsSatisfied(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	ntError_p(pobBoss->GetBossType() == Boss::BT_KING_BOHAN, ("GeneralBossMovementSelectorKingHasDemonAttached should only be put on king!"));
	KingBohan* pobKing = (KingBohan*)pobBoss;

	if(pobKing->HasAttachedDemon() == m_bDesiredAttachedState)
	{
		return true;
	}

	//Not what we wanted.
	return false;
}


START_STD_INTERFACE(GeneralBossAttackSelectorKingPlayerInRetreatRadius)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bDesiredInRadiusState, true, TrueIfInRetreatRadius)
END_STD_INTERFACE

bool GeneralBossAttackSelectorKingPlayerInRetreatRadius::IsSatisfied(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	ntError_p(pobBoss->GetBossType() == Boss::BT_KING_BOHAN, ("GeneralBossAttackSelectorKingPlayerInRetreatRadius should only be put on king!"));
	KingBohan* pobKing = (KingBohan*)pobBoss;

	if(pobKing->IsPlayerInRetreatPointRadius() == m_bDesiredInRadiusState)
	{
		return true;
	}

	//Not what we wanted.
	return false;
}


START_STD_INTERFACE(GeneralBossMovementSelectorKingPlayerInRetreatRadius)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bDesiredInRadiusState, true, TrueIfInRetreatRadius)
END_STD_INTERFACE

bool GeneralBossMovementSelectorKingPlayerInRetreatRadius::IsSatisfied(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	ntError_p(pobBoss->GetBossType() == Boss::BT_KING_BOHAN, ("GeneralBossMovementSelectorKingPlayerInRetreatRadius should only be put on king!"));
	KingBohan* pobKing = (KingBohan*)pobBoss;

	if(pobKing->IsPlayerInRetreatPointRadius() == m_bDesiredInRadiusState)
	{
		return true;
	}

	//Not what we wanted.
	return false;
}


//===================== WING-ATTACK ====================

START_STD_INTERFACE(KingWingAttack)
	COPY_INTERFACE_FROM(BossSpecialAttack)
	DEFINE_INTERFACE_INHERITANCE(BossSpecialAttack)

	PUBLISH_VAR_WITH_DEFAULT_AS(m_fCrowSpawnXRange, 10.0f, CrowSpawnXRange);
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fCrowSpawnYRange, 10.0f, CrowSpawnYRange);
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fCrowSpawnZRange, 2.0f, CrowSpawnZRange);
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fCrowSpawnDelay, 0.5f, CrowSpawnDelay);
	PUBLISH_VAR_WITH_DEFAULT_AS(m_iMaxNumberOfWaves, 3, MaxNumberOfWaves)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_iMaxNumberOfWavesAdjust, 1, MaxNumberOfWavesAdjust)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_iMaxCrowsPerSpawn, 15, MaxCrowsPerSpawn)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_iMinCrowsPerSpawn, 8, MinCrowsPerSpawn)
	PUBLISH_PTR_AS(m_pobAttackStart, AttackStart)
	PUBLISH_PTR_AS(m_pobAttackLoop, AttackLoop)
	PUBLISH_PTR_AS(m_pobAttackEnd, AttackEnd)

	PUBLISH_PTR_AS(m_pobProjectileAttributes, CrowProjectileAttributes)
END_STD_INTERFACE


KingWingAttack::KingWingAttack()
{
	m_iWavesRemaining = -1;
	m_bFlaggedFinished = false;
	m_iMaxNumberOfWaves = 3;
	m_iMaxNumberOfWavesAdjust = 1;

	m_pobAttackStart = m_pobAttackLoop = m_pobAttackEnd = 0;
	m_fCrowSpawnDelay = 0.8f;
	m_bSpawnCrows = false;
	m_iLinksProcessed = 0;

	m_pobProjectileAttributes = 0;
}


bool KingWingAttack::Initialise(Boss* pobBoss, CEntity* pobPlayer)
{
	ntError_p( pobBoss->GetBossType() == Boss::BT_KING_BOHAN, ("Should not attach a KingWingAttack on anyone other than the king!") );

	//Make sure that we've got all the attack links we need, if not, bail.
	ntError_p(m_pobAttackStart && m_pobAttackLoop && m_pobAttackEnd, ("One or more attack-links for the wing-hurricane special was NULL"));

	//Error-check our crow-count boundaries.
	ntError_p(m_iMaxCrowsPerSpawn >= m_iMinCrowsPerSpawn, ("Error: You seem to have set min-crows at a value larger than max-crows"));

	KingBohan* pobKing = (KingBohan*)pobBoss;
	//If we're already in the middle of an attack, then don't initialise another... ever.
	if(pobKing->IsInSpecialAttack())
	{
		ntPrintf("***###*** KingWingAttackSpecial: SHOULD NOT HAVE HAPPENED (attempting to initialise an attack whilst already in one [%s]) ***###***\n",
			pobKing->GetCurrentSpecialName());
		return false;
	}

	ntError_p(m_pobProjectileAttributes, ("KingWingAttack: The crow's projectile-attributes are missing"));

	m_bFlaggedFinished = false;
	m_iWavesRemaining = m_iMaxNumberOfWaves;
	m_iWavesRemaining -= grand() % (m_iMaxNumberOfWavesAdjust + 1);
	if(m_iWavesRemaining <= 0)
	{
		m_iWavesRemaining = 1;
	}

	pobBoss->GetBossMovement()->m_obTargetPoint = pobPlayer->GetPosition();
	pobBoss->GetBossMovement()->m_bTargetPointSet = true;

	m_pobBoss = pobBoss;
	m_bSpawnCrows = false;
	m_fCrowSpawnDelayCounter = 0.0f;
	m_iLinksProcessed = 0;

	//Kick off the attack start, which will link into the attack loop (flapping wings and what-not).
	//TODO: Pass 'force' flag to this attack request?
	if(pobBoss->GetAttackComponent()->Boss_Command_RequestDirectAttack(m_pobAttackStart))
	{
		pobBoss->GetMessageHandler()->ReceiveMsg<msg_buttonattack>();

		//Tell the demon to switch effect appropriately.
		KingBohan* pobKing = (KingBohan*)pobBoss;
		Demon* pobDemon = (Demon*)pobKing->GetDemon();
		ntError_p(pobDemon && (pobDemon->GetBossType() == Boss::BT_DEMON), ("KingWingAttack::Initialise(), Invalid demon pointer from king"));
		pobDemon->SetDemonEffectWingAttack();
		pobKing->SetInSpecialAttack(true, "WingAttack");	//Don't detach mid-air!

		return true;
	}
	
	return false;
}

void KingWingAttack::NotifyAttackFinished()
{
	m_iWavesRemaining--;
}

void KingWingAttack::NotifyAttackAutoLinked()
{
	m_iLinksProcessed++;
	if(!m_bSpawnCrows && m_iWavesRemaining > 0)
	{
		if((m_iLinksProcessed < 2) && (m_iLinksProcessed + 1 >= 2))
		{
			m_bSpawnCrows = true;
			m_fCrowSpawnDelayCounter = 0.0f;
		}
	}
	else
	{
		//If we have just had the part of the attack where we spawn crows, spawn no more.
		m_bSpawnCrows = false;
		m_iWavesRemaining--;
		m_iLinksProcessed = 0;
	}
}


BossAttack* KingWingAttack::Update(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	KingBohan* pobKing = (KingBohan*)pobBoss;

	// Keep the king facing the player during autolinked wing-flaps
	if (m_iWavesRemaining >= 0 )
	{
		pobKing->GetBossMovement()->m_obTargetPoint = pobPlayer->GetPosition();
		pobKing->GetBossMovement()->m_bTargetPointSet = true;
	}

	//If we've finished with the looping flap-attack, start the end "attack".
	if((m_iWavesRemaining <= 0) && (m_bFlaggedFinished == false))
	{
		//Queue attack end as next attack.
		if(pobKing->GetAttackComponent()->Boss_Command_RequestDirectAttack(m_pobAttackEnd, true))
		{
			pobKing->GetMessageHandler()->ReceiveMsg<msg_buttonattack>();
		}

		m_iWavesRemaining--;
		m_bFlaggedFinished = true;
		m_bSpawnCrows = false;	//No more crows after this point.
	}

	//If we're ready to spawn crows, then spawn a random amount of crows every m_fSpawnDelay seconds.
	if(m_bSpawnCrows)
	{
		m_fCrowSpawnDelayCounter += fTimeDelta;
		//If enough time has passed since the last crow spawn, then spawn more!
		if(m_fCrowSpawnDelayCounter >= m_fCrowSpawnDelay)
		{
			int iCrowCountRange = (int)(m_iMaxCrowsPerSpawn - m_iMinCrowsPerSpawn);
			int iNumCrowsToSpawn = (grand() % iCrowCountRange) + m_iMinCrowsPerSpawn;
			//For each of those crows...
			for(int i = 0 ; i < iNumCrowsToSpawn ; i++)
			{
				//Generate a random offset for crow.
				float fRandX = grandf(m_fCrowSpawnXRange) - (m_fCrowSpawnXRange / 2.0f);
				float fRandY = grandf(m_fCrowSpawnYRange);
				float fRandZ = grandf(m_fCrowSpawnZRange);
				CPoint obOffset(fRandX, fRandY, fRandZ);
				//We want the first projectile spawned to head right at the player. Any subsequent crows can have a random offset around the
				//player for it's target-location.
				bool bRandomOffsetFromTarget = (i == 0) ? false : true;

				//And we fire.
				Object_Projectile::CreateKingWingAttackCrow(pobBoss, 0, m_pobProjectileAttributes, pobPlayer, obOffset, bRandomOffsetFromTarget);
			}

			//Reset the timer.
			m_fCrowSpawnDelayCounter = 0.0f;
		}
	}

	//Have we finished everything?
	if(m_iWavesRemaining < 0 && pobBoss->CanStartAnAttack() && m_bFlaggedFinished)
	{
		//We've finished.

		//Tell the demon to return to normal effect.
		Demon* pobDemon = (Demon*)pobKing->GetDemon();
		ntError_p(pobDemon && (pobDemon->GetBossType() == Boss::BT_DEMON), ("KingWingAttack::Update(), Invalid demon pointer from king"));
		pobDemon->SetDemonEffectStandard();
		pobKing->SetInSpecialAttack(false, NULL);	//We're clear to detach again if we want.

		return 0;
	}

	return this;
}


void KingWingAttack::DebugRender(CPoint& obScreenLocation, float fXOffset, float fYOffset)
{
#ifndef _GOLD_MASTER
	g_VisualDebug->Printf3D(obScreenLocation,fXOffset,fYOffset,DC_CYAN,0,"KingWingAttack: %i Waves remaining", m_iWavesRemaining);
#endif
}


//===================== LIGHTNING ATTACK 1 ====================

START_STD_INTERFACE(KingLightningAttack1)
	COPY_INTERFACE_FROM(BossSpecialAttack)
	DEFINE_INTERFACE_INHERITANCE(BossSpecialAttack)

	PUBLISH_VAR_WITH_DEFAULT_AS(m_fTimeDelayOnBoltSpawn[0], 2.15f, TimeDelayOnBoltSpawn[0])
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fTimeDelayOnBoltSpawn[1], 2.8f, TimeDelayOnBoltSpawn[1])
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fTimeDelayOnBoltSpawn[2], 3.5f, TimeDelayOnBoltSpawn[2])
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fMaxTimeLightningIdle, 20.0f, MaxLightningIdleTime)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_iNumDazedLoops, 3, MaxDazedLoops)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fDamagePerReturnedBolt, 10.0f, DamagePerReturnedBolt)

	PUBLISH_PTR_AS(m_pobAttackStart, AttackStart)
	PUBLISH_PTR_AS(m_pobAttackFire, AttackFire)
	PUBLISH_PTR_AS(m_pobAttackEnd, AttackEnd)
	PUBLISH_PTR_AS(m_pobBoltsReturnedAndHit, HitByBolt)
	PUBLISH_PTR_AS(m_pobDazedRecovery, DazedRecovery)
	PUBLISH_PTR_AS(m_pobDazedRecoil, DazedRecoil)
	PUBLISH_PTR_AS(m_pobProjectileAttributes, ProjectileAttributes)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obDeflectEffects[0], CHashedString("NULL"), FirstHitEffect1);
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obDeflectEffects[1], CHashedString("NULL"), FirstHitEffect2);
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obDeflectEffects[2], CHashedString("NULL"), FirstHitEffect3);
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obDeflectEffects[3], CHashedString("NULL"), SecondHitEffect1);
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obDeflectEffects[4], CHashedString("NULL"), SecondHitEffect2);
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obDeflectEffects[5], CHashedString("NULL"), SecondHitEffect3);
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obDeflectEffects[6], CHashedString("NULL"), ThirdHitEffect1);
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obDeflectEffects[7], CHashedString("NULL"), ThirdHitEffect2);
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obDeflectEffects[8], CHashedString("NULL"), ThirdHitEffect3);
END_STD_INTERFACE


KingLightningAttack1::KingLightningAttack1()
{
	m_iBoltsFired = 0;
	m_iBoltsDeflected = 0;
	m_bFlaggedFinished = false;
	m_fMaxTimeLightningIdle = 20.0f;
	m_iDazedLoopCount = 0;
	m_bRecoverySet = false;

	m_pobAttackStart = m_pobAttackFire = m_pobAttackEnd = m_pobBoltsReturnedAndHit = m_pobDazedRecovery = m_pobDazedRecoil = 0;
	m_fTimeIdleInAir = 0.0f;
	m_pobProjectileAttributes = 0;
	for(int i = 0 ; i < KING_NUM_LIGHTNINGBOLTS_1 ; i++)
	{
		m_pobLightningBalls[i] = 0;
	}
	m_bStunned = false;
}


bool KingLightningAttack1::Initialise(Boss* pobBoss, CEntity* pobPlayer)
{
	ntError_p( pobBoss->GetBossType() == Boss::BT_KING_BOHAN, ("Should not attach a KingLightningAttack1 on anyone other than the king!") );
	KingBohan* pobKing = (KingBohan*)pobBoss;

	//Make sure that we've got all the attack links we need, if not, bail.
	ntError_p(m_pobAttackStart && m_pobAttackFire && m_pobAttackEnd && m_pobBoltsReturnedAndHit && m_pobDazedRecovery && m_pobDazedRecoil,
		("One or more attack links was NULL"));
	//Make sure we've got our projectile attributes specified or we can't fire anything!
	ntError_p(m_pobProjectileAttributes, ("KingLightningAttack1: No Projectile Attributes specified"));

	//If we're already in the middle of an attack, then don't initialise another... ever.
	if(pobKing->IsInSpecialAttack())
	{
		ntPrintf("***###*** LightningAttack1: SHOULD NOT HAVE HAPPENED (attempting to initialise an attack whilst already in one [%s]) ***###***\n",
			pobKing->GetCurrentSpecialName());
		return false;
	}

	//The attack-selector conditions should stop this from happening. However, in-case they don't, we catch it here and output a message.
	if(pobKing->HasAttachedDemon() == false)
	{
		ntPrintf("***###*** LightningAttack1: SHOULD NOT HAVE HAPPENED (attempting to select and initialise this attack without demon attached). Please check attack-selector conditions ***###***\n");
		return false;
	}


	m_bFlaggedFinished = false;
	m_iBoltsFired = 0;
	m_iBoltsDeflected = 0;
	m_bReadyToFire = false;
	m_fTimeIdleInAir = 0.0f;
	m_iDazedLoopCount = 0;
	m_bRecoverySet = false;
	for(int i = 0 ; i < KING_NUM_LIGHTNINGBOLTS_1 ; i++)
	{
		m_pobLightningBalls[i] = 0;
	}

	//Set that the king isn't currently "hit while stunned".
	pobKing->m_bNotifyIfHitStunned = false;
	pobKing->SetHitWhileStunned(false);

	pobBoss->GetBossMovement()->m_obTargetPoint = pobPlayer->GetPosition();
	pobBoss->GetBossMovement()->m_bTargetPointSet = true;
	m_bStunned = false;

	m_pobBoss = pobBoss;

	//Kick off the attack start, which will link into the attack loop (firing all three bolts in one anim with time-offsets for code).
	//When this attack links, m_bReadyToFire will be set to true. This is to stop it from firing until we've moved past the attack-start phase.
	//TODO: Pass 'force' flag to this attack request?
	if(pobBoss->GetAttackComponent()->Boss_Command_RequestDirectAttack(m_pobAttackStart))
	{
		KingBohan* pobKing = (KingBohan*)pobBoss;
		pobKing->GetMessageHandler()->ReceiveMsg<msg_buttonattack>();
		pobKing->SetInSpecialAttack(true, "LightningAttack1");	//Don't detach mid-air!

		return true;
	}

	return false;
}


void KingLightningAttack1::NotifyAttackFinished()
{
	m_fBoltSpawnDelayCounter = 0.0f;
	m_bReadyToFire = true;
	if(m_bStunned)
	{
		m_iDazedLoopCount++;
	}
}


void KingLightningAttack1::NotifyAttackAutoLinked()
{
	m_fBoltSpawnDelayCounter = 0.0f;
	m_bReadyToFire = true;
	if(m_bStunned)
	{
		m_iDazedLoopCount++;
	}
}


BossAttack* KingLightningAttack1::NotifyAttackInterrupted()
{
	//Output a message for debug-purposes just in-case this attack being interrupted hangs anything etc.
	ntPrintf("*** LightningAttack1 Interrupted ***\n");

	//Just make sure to toggle this flag off so that he can go into another special afterwards.
	//This attack shouldn't be interruptable, but account for it just in-case.
	KingBohan* pobKing = (KingBohan*)m_pobBoss;
	pobKing->SetInSpecialAttack(false, NULL);
	if(m_bStunned)
	{
		Demon* pobDemon = (Demon*)pobKing->GetDemon();
		ntError_p(pobDemon && (pobDemon->GetBossType() == Boss::BT_DEMON), ("Why is the demon on the king missing or not of type BT_DEMON?"));
		pobDemon->SetDemonEffectStandard();	//Change the demon effect back to standard.
	}

	//No-longer in an attack, return 0.
	return 0;
}


BossAttack* KingLightningAttack1::Update(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	KingBohan* pobKing = (KingBohan*)pobBoss;
	ntError_p(pobKing->GetDemon() && (pobKing->GetDemon()->GetBossType() == Boss::BT_DEMON), ("Demon on king is either missing or wrong bosstype"));
	Demon* pobDemon = (Demon*)pobKing->GetDemon();

	//Update whether or not we need to be notified of "hit while stunned"...
	pobKing->m_bNotifyIfHitStunned = m_bStunned;
	if(!m_bStunned)
	{
		//Make sure this hit-while-stunned flag stays false during this attack if the king isn't stunned.
		pobKing->SetHitWhileStunned(false);
	}

	//Keep the king facing the player throughout unless he's stunned.
	if(m_iBoltsDeflected < KING_NUM_LIGHTNINGBOLTS_1)
	{
		pobKing->GetBossMovement()->m_obTargetPoint = pobPlayer->GetPosition();
		pobKing->GetBossMovement()->m_bTargetPointSet = true;
	}
	else
	{
		pobKing->GetBossMovement()->m_obTargetPoint = pobKing->GetPosition() + (pobKing->GetMatrix().GetZAxis() * 1.0f);
		pobKing->GetBossMovement()->m_bTargetPointSet = true;
	}

	if(m_iBoltsFired >= KING_NUM_LIGHTNINGBOLTS_1)
	{
		m_fTimeIdleInAir += fTimeDelta;
	}

	//Handle firing and stuff.
	if((m_iBoltsFired < KING_NUM_LIGHTNINGBOLTS_1) && m_bReadyToFire)
	{
		float fTempCounter = m_fBoltSpawnDelayCounter;
		m_fBoltSpawnDelayCounter += fTimeDelta;
		for(int i = 0 ; i < KING_NUM_LIGHTNINGBOLTS_1 ; i++)
		{
			if((fTempCounter < m_fTimeDelayOnBoltSpawn[i]) && (m_fBoltSpawnDelayCounter >= m_fTimeDelayOnBoltSpawn[i]))
			{
				//Handle firing the projectiles.
				m_pobLightningBalls[i] = Object_Projectile::CreateKingLightningBall(pobBoss, 0, m_pobProjectileAttributes, pobPlayer, true);
				ntAssert_p(m_pobLightningBalls[i] != 0, ("Failed to create king lightning-ball projectile?!?"));
				m_obProjectileNames[i] = CHashedString(m_pobLightningBalls[i]->GetName());

				ntPrintf("!!!FIRING LIGHTNING BALL %d!!!\n", i);
				m_iBoltsFired++;
			}
		}
	}

	//Perform some updates on our projectiles (we could even destroy them here after a certain distance to keep things clean if-necessary).
	int iNumValidToReturnBolts = 0;
	bool bAnyReturned = false;
	for(int i = 0 ; i < m_iBoltsFired ; i++)
	{
		//Check if our projectile entity still exists, so we're free to access it through our still-valid pointer.
		if(CEntityManager::Get().FindEntity(m_obProjectileNames[i]))
		{
			//If the projectile has been countered then we just assume it's good to keep.
			if(m_pobLightningBalls[i]->m_bHasBeenCountered)
			{
				iNumValidToReturnBolts++;
				bAnyReturned = true;
				continue;
			}

			//If the bolt has travelled a certain distance past the player then it can be classed as no-longer-valid.
			//While it's possible that the slow-moving lightningball can be caught up and returned, it's unlikely to happen, particularly
			//for the player to then have returned all three too.
			CPoint obProjectilePos = m_pobLightningBalls[i]->GetPosition();
			CPoint obKingPos = pobBoss->GetPosition();
			CPoint obPlayerPos = pobPlayer->GetPosition();

			CDirection obDirKingToProj = CDirection(obProjectilePos - obKingPos);
			CDirection obDirKingToPlayer = CDirection(obPlayerPos - obKingPos);
			float fDistKingToProj = obDirKingToProj.Length();
			float fDistKingToPlayer = obDirKingToPlayer.Length();

			if(fDistKingToProj < (fDistKingToPlayer + 15.0f))
			{
				iNumValidToReturnBolts++;
				continue;
			}

			//Sooo that covers if they've hit the terrain, if they've been returned, or if they've travelled too far for the player
			//to counter. Is there anything else? Perhaps the king needs a time-out for this bit in-case the player runs a long way then
			//counters or if the lightningball somehow manages to get stuck (unlikely though that is).
		}
	}

	//We put a time-out of m_fMaxTimeLightningIdle seconds here, after which bStayInAir will always become false.
	bool bStayInAir = (((m_iBoltsFired < 3) || iNumValidToReturnBolts > 0 || bAnyReturned) && (m_fTimeIdleInAir < m_fMaxTimeLightningIdle)) ? true : false;

	//If we're supposed to be staying in the air, then we should react-to (and deflect) any returned projectiles that come our way.
	if(bStayInAir == true)
	{
		for(int i = 0 ; i < m_iBoltsFired ; i++)
		{
			//Check if our projectile entity still exists, so we're free to access it through our still-valid pointer.
			if(CEntityManager::Get().FindEntity(m_obProjectileNames[i]))
			{
				//If the projectile has been countered then we just assume it's good to keep.
				if(m_pobLightningBalls[i]->m_bHasBeenCountered)
				{
					//Check if the projectile is now within range of the king, and if so, play an "attack" on the king and deflect the
					//projectile.
					CPoint obProjectilePos = m_pobLightningBalls[i]->GetPosition();
					CPoint obKingPos = pobBoss->GetPosition();
					CDirection obDirKingToProj = CDirection(obProjectilePos - obKingPos);
					float fDistKingToProj = obDirKingToProj.Length();

					if((fDistKingToProj < 3.0f) && (m_iBoltsDeflected < KING_NUM_LIGHTNINGBOLTS_1))	//TODO: Tweak from 3.0 to what looks right.
					{
						ntPrintf("***KING SUPER DEFLECT GOODNESS*** Bolt[%d]\n", i);
						m_pobLightningBalls[i]->m_bHasBeenCountered = false;	//So that we don't try to do this more than once (although it should be destroyed).
						m_pobLightningBalls[i]->m_bEffectOnDestroy = false;		//We toggle the default on-destruct effects off as we play our own.
						m_pobLightningBalls[i]->KingLightningBallDestroy();		//Tell the lightning ball to remove itself.
						int iEffectBase = m_iBoltsDeflected * 3;
						for(int iEffectNum = iEffectBase ; iEffectNum < (iEffectBase + 3) ; iEffectNum++)
						{
							//Play an effect if it's filled-in.
							if(ntStr::IsNull(m_obDeflectEffects[iEffectNum]) == false)
							{
								FXHelper::Pfx_CreateStatic(m_obDeflectEffects[iEffectNum], pobBoss, "spine_00");
							}
						}
						m_iBoltsDeflected++;

						//We want the king to take some damage for every returned bolt (stunning him and getting the hit is just a bonus).
                        pobKing->ChangeHealth(-m_fDamagePerReturnedBolt, "Hit by returned lightningbolt 1");
					}
				}
			}
		}
	}

	if((m_iBoltsDeflected >= KING_NUM_LIGHTNINGBOLTS_1) && (m_bFlaggedFinished == false) && (m_bStunned == false))
	{
		//We've been hit by the third, so we go into our stunned state which should link automatically through all the rest.
		if(pobKing->GetAttackComponent()->Boss_Command_RequestDirectAttack(m_pobBoltsReturnedAndHit, true))
		{
			pobKing->GetMessageHandler()->ReceiveMsg<msg_buttonattack>();
			m_bFlaggedFinished = true;	//We know we've queued up the "stunned" end-attack (so don't queue up the other one!)
			m_bStunned = true;	//Flag as stunned.
			pobDemon->SetDemonEffectStunned();	//Change the demon effect to be stunned.
		}
	}

	//If we've finished with the bolt-firing thing and we don't have three bolts possible to return, start the end "attack".
	if((m_iBoltsFired >= KING_NUM_LIGHTNINGBOLTS_1) && (m_bFlaggedFinished == false) && (bStayInAir == false) && (m_bStunned == false))
	{
		//Queue attack end as next attack.
		if(pobKing->GetAttackComponent()->Boss_Command_RequestDirectAttack(m_pobAttackEnd, true))
		{
			pobKing->GetMessageHandler()->ReceiveMsg<msg_buttonattack>();
			m_bFlaggedFinished = true;	//We know we've queued up the end attack at least.
		}
	}

	//If we were hit whilst stunned, then push on a new attack-link as a specific recoil to get up from stunned state (rather than
	//snapping to a standard anim which will look rubbish!)
	if((m_iBoltsDeflected >= KING_NUM_LIGHTNINGBOLTS_1) && (m_bStunned == true) && (m_bFlaggedFinished == true) &&
		(m_bRecoverySet == false) && (pobKing->IsHitWhileStunned() == true))
	{
		//Queue stunned recoil as next attack.
		ntPrintf("***###*** KING STUNNED HIT RECOIL ANIM WILL PLAY HERE (currently just plays recovery) ***###***\n");
		if(pobKing->GetAttackComponent()->Boss_Command_RequestDirectAttack(m_pobDazedRecoil, true))
		{
			pobKing->GetMessageHandler()->ReceiveMsg<msg_buttonattack>();
			m_bRecoverySet = true;
			m_bStunned = false;
			pobDemon->SetDemonEffectStandard();	//Change the demon effect back to standard.
		}
	}

	//If we're currently looping our dazed 'attack', then after a certain amount of loops, force on the 2fs.
	if((m_iBoltsDeflected >= KING_NUM_LIGHTNINGBOLTS_1) && (m_bStunned == true) && (m_bFlaggedFinished == true) &&
		(m_iDazedLoopCount > m_iNumDazedLoops) && (m_bRecoverySet == false))
	{
		//Queue stunned 2fs as next attack.
		if(pobKing->GetAttackComponent()->Boss_Command_RequestDirectAttack(m_pobDazedRecovery, true))
		{
			pobKing->GetMessageHandler()->ReceiveMsg<msg_buttonattack>();
			m_bRecoverySet = true;
			m_bStunned = false;
			pobDemon->SetDemonEffectStandard();	//Change the demon effect back to standard.
		}
	}

	//Have we finished everything?
	if(m_iBoltsFired >= KING_NUM_LIGHTNINGBOLTS_1 && m_bFlaggedFinished && pobBoss->CanStartAnAttack() && (m_bStunned == false))
	{
		//We've finished.
		pobKing->SetInSpecialAttack(false, NULL);
		if(m_bStunned)
		{
			pobDemon->SetDemonEffectStandard();	//Change the demon effect back to standard.
		}
		return 0;
	}

	return this;
}


void KingLightningAttack1::DebugRender(CPoint& obScreenLocation, float fXOffset, float fYOffset)
{
#ifndef _GOLD_MASTER
	g_VisualDebug->Printf3D(obScreenLocation,fXOffset,fYOffset,DC_CYAN,0,"KingLightningAttack1: %i bolts fired, %i bolts deflected",
		m_iBoltsFired, m_iBoltsDeflected);
#endif
}

bool KingLightningAttack1::IsVulnerableTo(CStrike* pobStrike)
{
	return IsVulnerableTo(pobStrike->GetAttackDataP());
}

bool KingLightningAttack1::IsVulnerableTo(const CAttackData* pobAttackData)
{
	return m_bStunned;	//Also factor in (m_pobBoss->GetAttackComponent()->IsInInvulnerabilityWindow <= 0) ??
}


//===================== LIGHTNING ATTACK 2 ====================

START_STD_INTERFACE(KingLightningAttack2)
	COPY_INTERFACE_FROM(BossSpecialAttack)
	DEFINE_INTERFACE_INHERITANCE(BossSpecialAttack)

	PUBLISH_VAR_WITH_DEFAULT_AS(m_fTimeDelayOnBoltSpawn[0], 1.0f, TimeDelayOnBoltSpawn[0])
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fTimeDelayOnBoltSpawn[1], 1.9f, TimeDelayOnBoltSpawn[1])
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fTimeDelayOnBoltSpawn[2], 2.5f, TimeDelayOnBoltSpawn[2])
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fTimeDelayOnBoltSpawn[3], 3.4f, TimeDelayOnBoltSpawn[3])
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fTimeDelayOnBoltSpawn[4], 4.0f, TimeDelayOnBoltSpawn[4])
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fTimeDelayOnBoltSpawn[5], 4.8f, TimeDelayOnBoltSpawn[5])

	PUBLISH_VAR_WITH_DEFAULT_AS(m_fMaxTimeLightningIdle, 20.0f, MaxLightningIdleTime)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_iNumDazedLoops, 3, MaxDazedLoops)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fDamagePerReturnedBolt, 8.0f, DamagePerReturnedBolt)

	PUBLISH_PTR_AS(m_pobAttackStart, AttackStart)
	PUBLISH_PTR_AS(m_pobAttackFire, AttackFire)
	PUBLISH_PTR_AS(m_pobAttackEnd, AttackEnd)
	PUBLISH_PTR_AS(m_pobBoltsReturnedAndHit, HitByBolt)
	PUBLISH_PTR_AS(m_pobDazedRecovery, DazedRecovery)
	PUBLISH_PTR_AS(m_pobDazedRecoil, DazedRecoil)
	PUBLISH_PTR_AS(m_pobHitInAirRecoil, HitInAirRecoil)
	PUBLISH_PTR_AS(m_pobProjectileAttributes, ProjectileAttributes)

	PUBLISH_VAR_WITH_DEFAULT_AS(m_obDeflectEffects[0], CHashedString("NULL"), FirstHitEffect1);
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obDeflectEffects[1], CHashedString("NULL"), FirstHitEffect2);
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obDeflectEffects[2], CHashedString("NULL"), FirstHitEffect3);
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obDeflectEffects[3], CHashedString("NULL"), SecondHitEffect1);
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obDeflectEffects[4], CHashedString("NULL"), SecondHitEffect2);
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obDeflectEffects[5], CHashedString("NULL"), SecondHitEffect3);
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obDeflectEffects[6], CHashedString("NULL"), ThirdHitEffect1);
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obDeflectEffects[7], CHashedString("NULL"), ThirdHitEffect2);
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obDeflectEffects[8], CHashedString("NULL"), ThirdHitEffect3);
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obDeflectEffects[9], CHashedString("NULL"), FourthHitEffect1);
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obDeflectEffects[10], CHashedString("NULL"), FourthHitEffect2);
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obDeflectEffects[11], CHashedString("NULL"), FourthHitEffect3);
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obDeflectEffects[12], CHashedString("NULL"), FifthHitEffect1);
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obDeflectEffects[13], CHashedString("NULL"), FifthHitEffect2);
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obDeflectEffects[14], CHashedString("NULL"), FifthHitEffect3);
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obDeflectEffects[15], CHashedString("NULL"), SixthHitEffect1);
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obDeflectEffects[16], CHashedString("NULL"), SixthHitEffect2);
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obDeflectEffects[17], CHashedString("NULL"), SixthHitEffect3);
END_STD_INTERFACE


KingLightningAttack2::KingLightningAttack2()
{
	m_iBoltsFired = 0;
	m_iBoltsSpawned = 0;
	m_bFlaggedFinished = false;
	m_fMaxTimeLightningIdle = 20.0f;
	m_iDazedLoopCount = 0;
	m_bRecoverySet = false;
	m_fBoltSpawnDelayCounter = 0.0f;
	m_fBoltFireDelayCounter = 0.0f;

	m_pobAttackStart = m_pobAttackFire = m_pobAttackEnd = m_pobBoltsReturnedAndHit = m_pobDazedRecovery = m_pobDazedRecoil = m_pobHitInAirRecoil = 0;
	m_fTimeIdleInAir = 0.0f;
	m_pobProjectileAttributes = 0;

	for(int i = 0 ; i < KING_NUM_LIGHTNINGBOLTS_2 ; i++)
	{
		m_pobLightningBalls[i] = 0;
	}
	m_bStunned = false;
}


bool KingLightningAttack2::Initialise(Boss* pobBoss, CEntity* pobPlayer)
{
	ntError_p( pobBoss->GetBossType() == Boss::BT_KING_BOHAN, ("Should not attach a KingLightningAttack2 on anyone other than the king!") );

	//Make sure that we've got all the attack links we need, if not, bail.
	ntError_p(m_pobAttackStart && m_pobAttackFire && m_pobAttackEnd && m_pobBoltsReturnedAndHit && m_pobDazedRecovery && m_pobDazedRecoil
		&& m_pobHitInAirRecoil, ("One or more attack links was NULL"));
	//Make sure we've got our projectile attributes specified or we can't fire anything!
	ntError_p(m_pobProjectileAttributes, ("KingLightningAttack2: No Projectile Attributes specified"));

	KingBohan* pobKing = (KingBohan*)pobBoss;
	//If we're already in the middle of an attack, then don't initialise another... ever.
	if(pobKing->IsInSpecialAttack())
	{
		ntPrintf("***###*** LightningAttack2: SHOULD NOT HAVE HAPPENED (attempting to initialise an attack whilst already in one [%s]) ***###***\n",
			pobKing->GetCurrentSpecialName());
		return false;
	}

	//The attack-selector conditions should stop this from happening. However, in-case they don't, we catch it here and output a message.
	if(pobKing->HasAttachedDemon() == false)
	{
		ntPrintf("***###*** LightningAttack2: SHOULD NOT HAVE HAPPENED (attempting to select and initialise this attack without demon attached). Please check attack-selector conditions ***###***\n");
		return false;
	}


	m_bFlaggedFinished = false;
	m_iBoltsFired = 0;
	m_iBoltsSpawned = 0;
	m_bReadyToFire = false;
	m_iBoltsDeflected = 0;
	m_fTimeIdleInAir = 0.0f;
	m_iDazedLoopCount = 0;
	m_bRecoverySet = false;
	m_fBoltSpawnDelayCounter = 0.0f;
	m_fBoltFireDelayCounter = 0.0f;
	m_bStunned = false;

	//Set that the king isn't currently "hit while stunned".
	pobKing->m_bNotifyIfHitStunned = false;
	pobKing->SetHitWhileStunned(false);

	pobBoss->GetBossMovement()->m_obTargetPoint = pobPlayer->GetPosition();
	pobBoss->GetBossMovement()->m_bTargetPointSet = true;

	m_pobBoss = pobBoss;

	//Kick off the attack start, which will link into the attack loop (firing all three bolts in one anim with time-offsets for code).
	//When this attack links, m_bReadyToFire will be set to true. This is to stop it from firing until we've moved past the attack-start phase.
	//TODO: Pass 'force' flag to this attack request?
	if(pobBoss->GetAttackComponent()->Boss_Command_RequestDirectAttack(m_pobAttackStart))
	{
		KingBohan* pobKing = (KingBohan*)pobBoss;
		pobKing->GetMessageHandler()->ReceiveMsg<msg_buttonattack>();
		pobKing->SetInSpecialAttack(true, "LightningAttack2");	//Don't detach mid-air!

		for(int i = 0 ; i < KING_NUM_LIGHTNINGBOLTS_2 ; i++)
		{
			m_pobLightningBalls[i] = 0;
		}

		return true;
	}

	return false;
}


void KingLightningAttack2::NotifyAttackFinished()
{
	m_fBoltSpawnDelayCounter = 0.0f;
	m_fBoltFireDelayCounter = 0.0f;
	m_bReadyToFire = true;
	if(m_bStunned)
	{
		m_iDazedLoopCount++;
	}
}


void KingLightningAttack2::NotifyAttackAutoLinked()
{
	m_bReadyToFire = true;
	if(m_bStunned)
	{
		m_iDazedLoopCount++;
	}
}


BossAttack* KingLightningAttack2::NotifyAttackInterrupted()
{
	//Output a message for debug-purposes just in-case this attack being interrupted hangs anything etc.
	ntPrintf("*** LightningAttack2 Interrupted ***\n");

	//Just make sure to toggle this flag off so that he can go into another special afterwards.
	//This attack shouldn't be interruptable, but account for it just in-case.
	KingBohan* pobKing = (KingBohan*)m_pobBoss;
	pobKing->SetInSpecialAttack(false, NULL);
	if(m_bStunned)
	{
		Demon* pobDemon = (Demon*)pobKing->GetDemon();
		ntError_p(pobDemon && (pobDemon->GetBossType() == Boss::BT_DEMON), ("Why is the demon on the king missing or not of type BT_DEMON?"));
		pobDemon->SetDemonEffectStandard();	//Change the demon effect back to standard.
	}

	//No-longer in an attack, return 0.
	return 0;
}


BossAttack* KingLightningAttack2::Update(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	KingBohan* pobKing = (KingBohan*)pobBoss;
	Demon* pobDemon = (Demon*)pobKing->GetDemon();
	ntError_p(pobDemon && (pobDemon->GetBossType() == Boss::BT_DEMON), ("Demon pointer missing or incorrect boss-type on king"));

	//Update whether or not we need to be notified of "hit while stunned"...
	pobKing->m_bNotifyIfHitStunned = m_bStunned;
	if(!m_bStunned)
	{
		//Make sure this hit-while-stunned flag stays false during this attack if the king isn't stunned.
		pobKing->SetHitWhileStunned(false);
	}

	//Keep the king facing the player throughout unless he's stunned.
	if(!m_bStunned)
	{
		pobKing->GetBossMovement()->m_obTargetPoint = pobPlayer->GetPosition();
		pobKing->GetBossMovement()->m_bTargetPointSet = true;
	}
	else
	{
		pobKing->GetBossMovement()->m_obTargetPoint = pobKing->GetPosition() + (pobKing->GetMatrix().GetZAxis() * 1.0f);
		pobKing->GetBossMovement()->m_bTargetPointSet = true;
	}

	if(m_iBoltsFired >= KING_NUM_LIGHTNINGBOLTS_2)
	{
		m_fTimeIdleInAir += fTimeDelta;
	}

	//Handle spawning the bolts into orbit.
	if((m_iBoltsSpawned < KING_NUM_LIGHTNINGBOLTS_2) && m_bReadyToFire)
	{
		float fTempCounter = m_fBoltSpawnDelayCounter;
		m_fBoltSpawnDelayCounter += fTimeDelta;
		for(int i = 0 ; i < KING_NUM_LIGHTNINGBOLTS_2 ; i++)
		{
			if((fTempCounter < m_fTimeDelayOnBoltSpawn[i]) && (m_fBoltSpawnDelayCounter >= m_fTimeDelayOnBoltSpawn[i]))
			{
				//Handle spawning the projectiles and putting them into orbit around the king.
				m_pobLightningBalls[i] = Object_Projectile::CreateKingLightningBall(pobBoss, 0, m_pobProjectileAttributes, pobPlayer, true,
					true, true);
				m_obProjectileNames[i] = CHashedString(m_pobLightningBalls[i]->GetName());

				ntPrintf("!!!SPAWNING LIGHTNING BALL %d!!!\n", i);
				m_iBoltsSpawned++;
			}
		}
	}
	//Handle firing the bolts from orbit toward the player.
	else if((m_iBoltsFired < KING_NUM_LIGHTNINGBOLTS_2) && m_bReadyToFire)
	{
		//Now fire the bolts one at a time.
		float fTempCounter = m_fBoltFireDelayCounter;
		m_fBoltFireDelayCounter += fTimeDelta;
		for(int i = m_iBoltsFired ; i < KING_NUM_LIGHTNINGBOLTS_2 ; i++)
		{
			//One per second for now.
			if((fTempCounter < i + 1) && (m_fBoltFireDelayCounter >= i + 1))
			{
				//Fire-off each of the projectiles.
				if(m_pobLightningBalls[i])
				{
					m_pobLightningBalls[i]->KingLightningBallLaunchFromOrbit();
					m_iBoltsFired++;
					ntPrintf("!!!FIRING LIGHTNING BALL %d!!!\n", i);
				}
			}
		}
	}

	//Perform some updates on our projectiles (we could even destroy them here after a certain distance to keep things clean if-necessary).
	int iNumValidToReturnBolts = 0;
	bool bAnyReturned = false;
	for(int i = 0 ; i < m_iBoltsFired ; i++)
	{
		//Check if our projectile entity still exists, so we're free to access it through our still-valid pointer.
		if(CEntityManager::Get().FindEntity(m_obProjectileNames[i]))
		{
			//If the projectile has been countered then we just assume it's good to keep.
			if(m_pobLightningBalls[i]->m_bHasBeenCountered)
			{
				iNumValidToReturnBolts++;
				bAnyReturned = true;
				continue;
			}

			//If the bolt has travelled a certain distance past the player then it can be classed as no-longer-valid.
			//While it's possible that the slow-moving lightningball can be caught up and returned, it's unlikely to happen, particularly
			//for the player to then have returned all three too.
			CPoint obProjectilePos = m_pobLightningBalls[i]->GetPosition();
			CPoint obKingPos = pobBoss->GetPosition();
			CPoint obPlayerPos = pobPlayer->GetPosition();

			CDirection obDirKingToProj = CDirection(obProjectilePos - obKingPos);
			CDirection obDirKingToPlayer = CDirection(obPlayerPos - obKingPos);
			float fDistKingToProj = obDirKingToProj.Length();
			float fDistKingToPlayer = obDirKingToPlayer.Length();

			if(fDistKingToProj < (fDistKingToPlayer + 15.0f))
			{
				iNumValidToReturnBolts++;
				continue;
			}

			//Sooo that covers if they've hit the terrain, if they've been returned, or if they've travelled too far for the player
			//to counter. Is there anything else? Perhaps the king needs a time-out for this bit in-case the player runs a long way then
			//counters or if the lightningball somehow manages to get stuck (unlikely though that is).
		}
	}

	//We put a time-out of m_fMaxTimeLightningIdle seconds here, after which bStayInAir will always become false.
	bool bStayInAir = (((m_iBoltsFired < KING_NUM_LIGHTNINGBOLTS_2) || iNumValidToReturnBolts > 0 || bAnyReturned)
		&& (m_fTimeIdleInAir < m_fMaxTimeLightningIdle)) ? true : false;

	//If we're supposed to be staying in the air, then we should react-to (and deflect) any returned projectiles that come our way.
	if(bStayInAir == true)
	{
		for(int i = 0 ; i < m_iBoltsFired ; i++)
		{
			//Check if our projectile entity still exists, so we're free to access it through our still-valid pointer.
			if(CEntityManager::Get().FindEntity(m_obProjectileNames[i]))
			{
				//If the projectile has been countered then we just assume it's good to keep.
				if(m_pobLightningBalls[i]->m_bHasBeenCountered)
				{
					//Check if the projectile is now within range of the king, and if so, play an "attack" on the king and deflect the
					//projectile.
					CPoint obProjectilePos = m_pobLightningBalls[i]->GetPosition();
					CPoint obKingPos = pobBoss->GetPosition();
					CDirection obDirKingToProj = CDirection(obProjectilePos - obKingPos);
					float fDistKingToProj = obDirKingToProj.Length();

					if((fDistKingToProj < 3.0f) && (m_iBoltsDeflected < KING_NUM_LIGHTNINGBOLTS_2))	//TODO: Tweak from 3.0 to what looks right.
					{
						ntPrintf("***KING SUPER DEFLECT GOODNESS*** Heavy Bolt[%d]\n", i);
						m_pobLightningBalls[i]->m_bHasBeenCountered = false;	//So that we don't try to do this more than once (although it should be destroyed).
						m_pobLightningBalls[i]->m_bEffectOnDestroy = false;		//We toggle the default on-destruct effects off as we play our own.
						m_pobLightningBalls[i]->KingLightningBallDestroy();		//Tell the lightning ball to remove itself.
						int iEffectBase = m_iBoltsDeflected * 3;
						for(int iEffectNum = iEffectBase ; iEffectNum < (iEffectBase + 3) ; iEffectNum++)
						{
							//Play an effect if it's filled-in.
							if(ntStr::IsNull(m_obDeflectEffects[iEffectNum]) == false)
							{
								FXHelper::Pfx_CreateStatic(m_obDeflectEffects[iEffectNum], pobBoss, "spine_00");
							}
						}
						m_iBoltsDeflected++;

						//We also push on a recoil animation that will then link back into his idle if this is not the final bolt.
						//(If it is the final bolt then we'll go to stunned instead below).
						if(m_iBoltsDeflected < KING_NUM_LIGHTNINGBOLTS_2)
						{
							pobKing->GetAttackComponent()->Boss_Command_RequestDirectAttack(m_pobHitInAirRecoil, true);
						}

						//We want the king to take some damage for every returned bolt (stunning him and getting the hit is just a bonus).
                        pobKing->ChangeHealth(-m_fDamagePerReturnedBolt, "Hit by returned lightningbolt 2");
					}
				}
			}
		}
	}

	//If we're done with all that, then finish-up the attack.
	if(m_iBoltsFired >= KING_NUM_LIGHTNINGBOLTS_2)
	{
		if((m_iBoltsDeflected >= KING_NUM_LIGHTNINGBOLTS_2) && (m_bFlaggedFinished == false) && (m_bStunned == false))
		{
			//We've been hit by the sixth, so we go into our stunned state which should link automatically through the stunned cycle.
			if(pobKing->GetAttackComponent()->Boss_Command_RequestDirectAttack(m_pobBoltsReturnedAndHit, true))
			{
				pobKing->GetMessageHandler()->ReceiveMsg<msg_buttonattack>();
				m_bFlaggedFinished = true;
				m_bStunned = true;
				pobDemon->SetDemonEffectStunned();
			}
		}

		//If we've finished with the bolt-firing thing, start the end "attack".
		if((m_bFlaggedFinished == false) && (bStayInAir == false) && (m_bStunned == false))
		{
			//Queue attack end as next attack.
			if(pobKing->GetAttackComponent()->Boss_Command_RequestDirectAttack(m_pobAttackEnd, true))
			{
				pobKing->GetMessageHandler()->ReceiveMsg<msg_buttonattack>();
				m_bFlaggedFinished = true;	//We know we've queued up the end attack at least.
			}
		}

		//If we were hit while stunned, then push on a new attack-link as a specific recoil to get up from stunned state (rather than
		//snapping to a standard anim which will look rubbish!)
		if((m_iBoltsDeflected >= KING_NUM_LIGHTNINGBOLTS_2) && (m_bStunned == true) && (m_bFlaggedFinished == true) &&
			(m_bRecoverySet == false) && (pobKing->IsHitWhileStunned() == true))
		{
			//Queue stunned recoil as next attack.
			ntPrintf("***###*** KING STUNNED HIT RECOIL (LightningAttack2) ***###***\n");
			if(pobKing->GetAttackComponent()->Boss_Command_RequestDirectAttack(m_pobDazedRecoil, true))
			{
				pobKing->GetMessageHandler()->ReceiveMsg<msg_buttonattack>();
				m_bRecoverySet = true;
				m_bStunned = false;
				pobDemon->SetDemonEffectStandard();
			}
		}

		//If we're currently looping our dazed 'attack', then after a certain number of loops, force on the 2fs.
		if((m_iBoltsDeflected >= KING_NUM_LIGHTNINGBOLTS_2) && (m_bStunned == true) && (m_bFlaggedFinished == true) &&
			(m_iDazedLoopCount > m_iNumDazedLoops) && (m_bRecoverySet == false))
		{
			//Queue stunned 2fs as next attack.
			if(pobKing->GetAttackComponent()->Boss_Command_RequestDirectAttack(m_pobDazedRecovery, true))
			{
				pobKing->GetMessageHandler()->ReceiveMsg<msg_buttonattack>();
				m_bRecoverySet = true;
				m_bStunned = false;
				pobDemon->SetDemonEffectStandard();
			}
		}

		//Have we finished everything?
		if(m_bFlaggedFinished && pobBoss->CanStartAnAttack() && (m_bStunned == false))
		{
			//We've finished.
			pobKing->SetInSpecialAttack(false, NULL);
			if(m_bStunned)
			{
				pobDemon->SetDemonEffectStandard();	//Change the demon effect back to standard.
			}
			return 0;
		}
	}

	return this;
}


bool KingLightningAttack2::IsVulnerableTo(CStrike* pobStrike)
{
	return IsVulnerableTo(pobStrike->GetAttackDataP());
}

bool KingLightningAttack2::IsVulnerableTo(const CAttackData* pobAttackData)
{
	return m_bStunned;	//Also factor in (m_pobBoss->GetAttackComponent()->IsInInvulnerabilityWindow <= 0) ??
}

void KingLightningAttack2::DebugRender(CPoint& obScreenLocation, float fXOffset, float fYOffset)
{
#ifndef _GOLD_MASTER
	g_VisualDebug->Printf3D(obScreenLocation,fXOffset,fYOffset,DC_CYAN,0,"KingLightningAttack2: %i bolts spawned, %i bolts fired, %i bolts deflected",
		m_iBoltsSpawned, m_iBoltsFired, m_iBoltsDeflected);
#endif
}


//===================== LIGHTNING ATTACK 3 ====================

START_STD_INTERFACE(KingLightningAttack3)
	COPY_INTERFACE_FROM(BossSpecialAttack)
	DEFINE_INTERFACE_INHERITANCE(BossSpecialAttack)

	PUBLISH_VAR_WITH_DEFAULT_AS(m_fTimeDelayOnBoltSpawn[0], 1.0f, TimeDelayOnBoltSpawn[0])
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fTimeDelayOnBoltSpawn[1], 1.9f, TimeDelayOnBoltSpawn[1])
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fTimeDelayOnBoltSpawn[2], 2.5f, TimeDelayOnBoltSpawn[2])
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fTimeDelayOnBoltSpawn[3], 3.4f, TimeDelayOnBoltSpawn[3])
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fTimeDelayOnBoltSpawn[4], 4.0f, TimeDelayOnBoltSpawn[4])
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fTimeDelayOnBoltSpawn[5], 4.8f, TimeDelayOnBoltSpawn[5])
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fTimeDelayOnBoltSpawn[6], 6.0f, TimeDelayOnBoltSpawn[6])
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fTimeDelayOnBoltSpawn[7], 6.0f, TimeDelayOnBoltSpawn[7])
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fTimeDelayOnBoltSpawn[8], 6.0f, TimeDelayOnBoltSpawn[8])

	PUBLISH_VAR_WITH_DEFAULT_AS(m_fMaxTimeLightningIdle, 20.0f, MaxLightningIdleTime)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_iNumDazedLoops, 3, MaxDazedLoops)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fDamagePerReturnedBolt, 6.0f, DamagePerReturnedBolt)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fMinTimeBetweenShots, 0.75f, MinTimeBetweenShots)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fMaxTimeBetweenShots, 1.25f, MaxTimeBetweenShots)

	PUBLISH_PTR_AS(m_pobAttackStart, AttackStart)
	PUBLISH_PTR_AS(m_pobAttackFire, AttackFire)
	PUBLISH_PTR_AS(m_pobAttackEnd, AttackEnd)
	PUBLISH_PTR_AS(m_pobBoltsReturnedAndHit, HitByBolt)
	PUBLISH_PTR_AS(m_pobDazedRecovery, DazedRecovery)
	PUBLISH_PTR_AS(m_pobDazedRecoil, DazedRecoil)
	PUBLISH_PTR_AS(m_pobHitInAirRecoil, HitInAirRecoil)
	PUBLISH_PTR_AS(m_pobProjectileAttributes[0], LargeProjectileAttributes)
	PUBLISH_PTR_AS(m_pobProjectileAttributes[1], SmallProjectileAttributes)

	PUBLISH_VAR_WITH_DEFAULT_AS(m_obDeflectEffects[0], CHashedString("NULL"), FirstHitEffect1);
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obDeflectEffects[1], CHashedString("NULL"), FirstHitEffect2);
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obDeflectEffects[2], CHashedString("NULL"), FirstHitEffect3);
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obDeflectEffects[3], CHashedString("NULL"), SecondHitEffect1);
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obDeflectEffects[4], CHashedString("NULL"), SecondHitEffect2);
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obDeflectEffects[5], CHashedString("NULL"), SecondHitEffect3);
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obDeflectEffects[6], CHashedString("NULL"), ThirdHitEffect1);
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obDeflectEffects[7], CHashedString("NULL"), ThirdHitEffect2);
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obDeflectEffects[8], CHashedString("NULL"), ThirdHitEffect3);
END_STD_INTERFACE


KingLightningAttack3::KingLightningAttack3()
{
	m_iBoltsFired = 0;
	m_iBoltsSpawned = 0;
	m_bFlaggedFinished = false;
	m_fMaxTimeLightningIdle = 20.0f;
	m_iDazedLoopCount = 0;
	m_bRecoverySet = false;
	m_fBoltSpawnDelayCounter = 0.0f;
	m_fBoltFireDelayCounter = 0.0f;

	m_pobAttackStart = m_pobAttackFire = m_pobAttackEnd = m_pobBoltsReturnedAndHit = m_pobDazedRecovery = m_pobDazedRecoil = m_pobHitInAirRecoil = 0;
	m_fTimeIdleInAir = 0.0f;
	m_pobProjectileAttributes[0] = 0;
	m_pobProjectileAttributes[1] = 0;

	m_fMinTimeBetweenShots = 0.75f;
	m_fMaxTimeBetweenShots = 1.25f;
	m_fTimeToFireNextShot = 0.0f;

	for(int i = 0 ; i < (KING_NUM_LIGHTNINGBOLTS_3_LIGHT + KING_NUM_LIGHTNINGBOLTS_3_HEAVY) ; i++)
	{
		m_pobLightningBalls[i] = 0;
		m_obProjectileNames[i] = CHashedString("");
	}

	m_bStunned = false;
	m_vpobProjectilesToFire.clear();
}


bool KingLightningAttack3::Initialise(Boss* pobBoss, CEntity* pobPlayer)
{
	ntError_p( pobBoss->GetBossType() == Boss::BT_KING_BOHAN, ("Should not attach a KingLightningAttack3 on anyone other than the king!") );

	//Make sure that we've got all the attack links we need, if not, bail.
	ntError_p(m_pobAttackStart && m_pobAttackFire && m_pobAttackEnd && m_pobBoltsReturnedAndHit && m_pobDazedRecovery && m_pobDazedRecoil
		&& m_pobHitInAirRecoil, ("One or more attack links was NULL"));
	//Make sure we've got our projectile attributes specified or we can't fire anything!
	ntError_p(m_pobProjectileAttributes[0] && m_pobProjectileAttributes[1], ("KingLightningAttack3: No Projectile Attributes specified"));

	KingBohan* pobKing = (KingBohan*)pobBoss;
	//If we're already in the middle of an attack, then don't initialise another... ever.
	if(pobKing->IsInSpecialAttack())
	{
		ntPrintf("***###*** LightningAttack3: SHOULD NOT HAVE HAPPENED (attempting to initialise an attack whilst already in one [%s]) ***###***\n",
			pobKing->GetCurrentSpecialName());
		return false;
	}

	//The attack-selector conditions should stop this from happening. However, in-case they don't, we catch it here and output a message.
	if(pobKing->HasAttachedDemon() == false)
	{
		ntPrintf("***###*** LightningAttack3: SHOULD NOT HAVE HAPPENED (attempting to select and initialise this attack without demon attached). Please check attack-selector conditions ***###***\n");
		return false;
	}


	m_bFlaggedFinished = false;
	m_iBoltsFired = 0;
	m_iBoltsSpawned = 0;
	m_bReadyToFire = false;
	m_iBoltsDeflected = 0;
	m_fTimeIdleInAir = 0.0f;
	m_iDazedLoopCount = 0;
	m_bRecoverySet = false;
	m_fBoltSpawnDelayCounter = 0.0f;
	m_fBoltFireDelayCounter = 0.0f;
	m_bStunned = false;

	//Set that the king isn't currently "hit while stunned".
	pobKing->m_bNotifyIfHitStunned = false;
	pobKing->SetHitWhileStunned(false);

	pobBoss->GetBossMovement()->m_obTargetPoint = pobPlayer->GetPosition();
	pobBoss->GetBossMovement()->m_bTargetPointSet = true;

	m_pobBoss = pobBoss;

	//Clear our projectile list (filled as projectiles are added).
	m_vpobProjectilesToFire.clear();

	//Choose a time for the first shot to be fired.
	float fDifferenceInShotTime = m_fMaxTimeBetweenShots - m_fMinTimeBetweenShots;
	m_fTimeToFireNextShot = grandf(fDifferenceInShotTime) + m_fMinTimeBetweenShots;

	//Kick off the attack start, which will link into the attack loop (firing all three bolts in one anim with time-offsets for code).
	//When this attack links, m_bReadyToFire will be set to true. This is to stop it from firing until we've moved past the attack-start phase.
	//TODO: Pass 'force' flag to this attack request?
	if(pobBoss->GetAttackComponent()->Boss_Command_RequestDirectAttack(m_pobAttackStart))
	{
		KingBohan* pobKing = (KingBohan*)pobBoss;
		pobKing->GetMessageHandler()->ReceiveMsg<msg_buttonattack>();
		pobKing->SetInSpecialAttack(true, "LightningAttack3");	//Don't detach mid-air!

		for(int i = 0 ; i < (KING_NUM_LIGHTNINGBOLTS_3_LIGHT + KING_NUM_LIGHTNINGBOLTS_3_HEAVY) ; i++)
		{
			m_pobLightningBalls[i] = 0;
			m_obProjectileNames[i] = CHashedString("");
		}

		return true;
	}

	return false;
}


void KingLightningAttack3::NotifyAttackFinished()
{
	m_fBoltSpawnDelayCounter = 0.0f;
	m_fBoltFireDelayCounter = 0.0f;
	m_bReadyToFire = true;
	if(m_bStunned)
	{
		m_iDazedLoopCount++;
	}
}


void KingLightningAttack3::NotifyAttackAutoLinked()
{
	m_bReadyToFire = true;
	if(m_bStunned)
	{
		m_iDazedLoopCount++;
	}
}


BossAttack* KingLightningAttack3::NotifyAttackInterrupted()
{
	//Output a message for debug-purposes just in-case this attack being interrupted hangs anything etc.
	ntPrintf("*** LightningAttack3 Interrupted ***\n");

	//Just make sure to toggle this flag off so that he can go into another special afterwards.
	//This attack shouldn't be interruptable, but account for it just in-case.
	KingBohan* pobKing = (KingBohan*)m_pobBoss;
	pobKing->SetInSpecialAttack(false, NULL);
	if(m_bStunned)
	{
		Demon* pobDemon = (Demon*)pobKing->GetDemon();
		ntError_p(pobDemon && (pobDemon->GetBossType() == Boss::BT_DEMON), ("Why is the demon on the king missing or not of type BT_DEMON?"));
		pobDemon->SetDemonEffectStandard();	//Change the demon effect back to standard.
	}

	//No-longer in an attack, return 0.
	return 0;
}


BossAttack* KingLightningAttack3::Update(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	KingBohan* pobKing = (KingBohan*)pobBoss;
	Demon* pobDemon = (Demon*)pobKing->GetDemon();
	ntError_p(pobDemon && (pobDemon->GetBossType() == Boss::BT_DEMON), ("Demon pointer missing or incorrect boss-type on king"));

	//Update whether or not we need to be notified of "hit while stunned"...
	pobKing->m_bNotifyIfHitStunned = m_bStunned;
	if(!m_bStunned)
	{
		//Make sure this hit-while-stunned flag stays false during this attack if the king isn't stunned.
		pobKing->SetHitWhileStunned(false);
	}

	//Keep the king facing the player throughout unless he's stunned.
	if(!m_bStunned)
	{
		pobKing->GetBossMovement()->m_obTargetPoint = pobPlayer->GetPosition();
		pobKing->GetBossMovement()->m_bTargetPointSet = true;
	}
	else
	{
		pobKing->GetBossMovement()->m_obTargetPoint = pobKing->GetPosition() + (pobKing->GetMatrix().GetZAxis() * 1.0f);
		pobKing->GetBossMovement()->m_bTargetPointSet = true;
	}

	if(m_iBoltsFired >= (KING_NUM_LIGHTNINGBOLTS_3_LIGHT + KING_NUM_LIGHTNINGBOLTS_3_HEAVY))
	{
		m_fTimeIdleInAir += fTimeDelta;
	}

	//Handle spawning the bolts into orbit.
	if((m_iBoltsSpawned < (KING_NUM_LIGHTNINGBOLTS_3_LIGHT + KING_NUM_LIGHTNINGBOLTS_3_HEAVY)) && m_bReadyToFire)
	{
		float fTempCounter = m_fBoltSpawnDelayCounter;
		m_fBoltSpawnDelayCounter += fTimeDelta;
		for(int i = 0 ; i < (KING_NUM_LIGHTNINGBOLTS_3_LIGHT + KING_NUM_LIGHTNINGBOLTS_3_HEAVY) ; i++)
		{
			if((fTempCounter < m_fTimeDelayOnBoltSpawn[i]) && (m_fBoltSpawnDelayCounter >= m_fTimeDelayOnBoltSpawn[i]))
			{
				//Handle spawning the projectiles and putting them into orbit around the king.
				if(i < KING_NUM_LIGHTNINGBOLTS_3_HEAVY)
				{
					m_pobLightningBalls[i] = Object_Projectile::CreateKingLightningBall(pobBoss, 0, m_pobProjectileAttributes[0], pobPlayer, true,
						true, true);
					m_obProjectileNames[i] = CHashedString(m_pobLightningBalls[i]->GetName());
				}
				else
				{
					m_pobLightningBalls[i] = Object_Projectile::CreateKingLightningBall(pobBoss, 0, m_pobProjectileAttributes[1], pobPlayer, true,
						true, false);
					m_obProjectileNames[i] = CHashedString(m_pobLightningBalls[i]->GetName());
				}

				if(m_pobLightningBalls[i])
				{
					m_vpobProjectilesToFire.push_back(m_pobLightningBalls[i]);
				}
				ntPrintf("!!!SPAWNING LIGHTNING BALL %d!!!\n", i);
				m_iBoltsSpawned++;
			}
		}
	}
	//Handle firing the bolts from orbit toward the player.
	else if((m_iBoltsFired < (KING_NUM_LIGHTNINGBOLTS_3_LIGHT + KING_NUM_LIGHTNINGBOLTS_3_HEAVY)) && m_bReadyToFire)
	{
		//Now fire the bolts one at a time.
		float fTempCounter = m_fBoltFireDelayCounter;
		m_fBoltFireDelayCounter += fTimeDelta;

		if((fTempCounter < m_fTimeToFireNextShot) && (m_fBoltFireDelayCounter >= m_fTimeToFireNextShot))
		{
			//Fire a random one.
			int iRandToFire = grand() % m_vpobProjectilesToFire.size();
			if(m_vpobProjectilesToFire[iRandToFire])
			{
				m_vpobProjectilesToFire[iRandToFire]->KingLightningBallLaunchFromOrbit();
				m_iBoltsFired++;
				ntPrintf("!!!FIRING LIGHTNINGBALL %d!!!\n", iRandToFire);
			}

			ntstd::Vector<Object_Projectile*>::iterator obIt = m_vpobProjectilesToFire.begin();
			obIt += iRandToFire;
			m_vpobProjectilesToFire.erase(obIt);

			//Now calculate a new time for the next one to be fired at.
			float fDifferenceInShotTime = m_fMaxTimeBetweenShots - m_fMinTimeBetweenShots;
			m_fTimeToFireNextShot = m_fTimeToFireNextShot + grandf(fDifferenceInShotTime) + m_fMinTimeBetweenShots;
		}
	}

	//Perform some updates on our projectiles (we could even destroy them here after a certain distance to keep things clean if-necessary).
	int iNumValidToReturnBolts = 0;
	bool bAnyReturned = false;
	for(int i = 0 ; i < m_iBoltsSpawned ; i++)	//We have to check ALL spawned bolts, not just up to num-fired, as the fired-order is random.
	{
		//Check if our projectile entity still exists, so we're free to access it through our still-valid pointer.
		if(CEntityManager::Get().FindEntity(m_obProjectileNames[i]))
		{
			//If the projectile has been countered then we just assume it's good to keep.
			if(m_pobLightningBalls[i]->m_bHasBeenCountered)
			{
				iNumValidToReturnBolts++;
				bAnyReturned = true;
				continue;
			}

			//If the bolt has travelled a certain distance past the player then it can be classed as no-longer-valid.
			//While it's possible that the slow-moving lightningball can be caught up and returned, it's unlikely to happen, particularly
			//for the player to then have returned all three too.
			CPoint obProjectilePos = m_pobLightningBalls[i]->GetPosition();
			CPoint obKingPos = pobBoss->GetPosition();
			CPoint obPlayerPos = pobPlayer->GetPosition();

			CDirection obDirKingToProj = CDirection(obProjectilePos - obKingPos);
			CDirection obDirKingToPlayer = CDirection(obPlayerPos - obKingPos);
			float fDistKingToProj = obDirKingToProj.Length();
			float fDistKingToPlayer = obDirKingToPlayer.Length();

			if(fDistKingToProj < (fDistKingToPlayer + 15.0f))
			{
				iNumValidToReturnBolts++;
				continue;
			}

			//Sooo that covers if they've hit the terrain, if they've been returned, or if they've travelled too far for the player
			//to counter. Is there anything else? Perhaps the king needs a time-out for this bit in-case the player runs a long way then
			//counters or if the lightningball somehow manages to get stuck (unlikely though that is).
		}
	}

	//We put a time-out of m_fMaxTimeLightningIdle seconds here, after which bStayInAir will always become false.
	bool bStayInAir = (((m_iBoltsFired < (KING_NUM_LIGHTNINGBOLTS_3_LIGHT + KING_NUM_LIGHTNINGBOLTS_3_HEAVY))
		|| iNumValidToReturnBolts > 0 || bAnyReturned) && (m_fTimeIdleInAir < m_fMaxTimeLightningIdle)) ? true : false;

	//If we're supposed to be staying in the air, then we should react-to (and deflect) any returned projectiles that come our way.
	if(bStayInAir == true)
	{
		for(int i = 0 ; i < m_iBoltsSpawned ; i++)	//Check ALL, not just those fired, as fire-order is random.
		{
			//Check if our projectile entity still exists, so we're free to access it through our still-valid pointer.
			if(CEntityManager::Get().FindEntity(m_obProjectileNames[i]))
			{
				//If the projectile has been countered then we just assume it's good to keep.
				if(m_pobLightningBalls[i]->m_bHasBeenCountered)
				{
					//Check if the projectile is now within range of the king, and if so, play an "attack" on the king and deflect the
					//projectile.
					CPoint obProjectilePos = m_pobLightningBalls[i]->GetPosition();
					CPoint obKingPos = pobBoss->GetPosition();
					CDirection obDirKingToProj = CDirection(obProjectilePos - obKingPos);
					float fDistKingToProj = obDirKingToProj.Length();

					if((fDistKingToProj < 3.0f) && (m_iBoltsDeflected < (KING_NUM_LIGHTNINGBOLTS_3_LIGHT + KING_NUM_LIGHTNINGBOLTS_3_HEAVY)))	//TODO: Tweak from 3.0 to what looks right.
					{
						if(m_pobLightningBalls[i]->m_bLarge)
						{
							ntPrintf("***KING SUPER DEFLECT GOODNESS*** Heavy Bolt[%d]\n", i);
						}
						else
						{
							ntPrintf("***KING SUPER DEFLECT GOODNESS*** Light Bolt[%d]\n", i);
						}
						m_pobLightningBalls[i]->m_bHasBeenCountered = false;	//So that we don't try to do this more than once (although it should be destroyed).
						m_pobLightningBalls[i]->m_bEffectOnDestroy = false;		//We toggle the default on-destruct effects off as we play our own.
						m_pobLightningBalls[i]->KingLightningBallDestroy();		//Tell the lightning ball to remove itself.
						int iEffectBase = (int)(m_iBoltsDeflected / 3) * 3;
						for(int iEffectNum = iEffectBase ; (iEffectNum < (iEffectBase + 3)) && (iEffectNum < 9) ; iEffectNum++)
						{
							//Play an effect if it's filled-in.
							if(ntStr::IsNull(m_obDeflectEffects[iEffectNum]) == false)
							{
								FXHelper::Pfx_CreateStatic(m_obDeflectEffects[iEffectNum], pobBoss, "spine_00");
							}
						}
						m_iBoltsDeflected++;

						//We also push on a recoil animation that will then link back into his idle if this is not the final bolt.
						//(If it is the final bolt then we'll go to stunned instead below).
						if(m_iBoltsDeflected < (KING_NUM_LIGHTNINGBOLTS_3_LIGHT + KING_NUM_LIGHTNINGBOLTS_3_HEAVY))
						{
							pobKing->GetAttackComponent()->Boss_Command_RequestDirectAttack(m_pobHitInAirRecoil, true);
						}

						//We want the king to take some damage for every returned bolt (stunning him and getting the hit is just a bonus).
                        pobKing->ChangeHealth(-m_fDamagePerReturnedBolt, "Hit by returned lightningbolt 3");
					}
				}
			}
		}
	}

	//If we're done with all that, then finish-up the attack.
	if(m_iBoltsFired >= (KING_NUM_LIGHTNINGBOLTS_3_LIGHT + KING_NUM_LIGHTNINGBOLTS_3_HEAVY))
	{
		if((m_iBoltsDeflected >= (KING_NUM_LIGHTNINGBOLTS_3_LIGHT + KING_NUM_LIGHTNINGBOLTS_3_HEAVY))
			&& (m_bFlaggedFinished == false) && (m_bStunned == false))
		{
			//We've been hit by the ninth, so we go into our stunned state which should link automatically through the stunned cycle.
			if(pobKing->GetAttackComponent()->Boss_Command_RequestDirectAttack(m_pobBoltsReturnedAndHit, true))
			{
				pobKing->GetMessageHandler()->ReceiveMsg<msg_buttonattack>();
				m_bFlaggedFinished = true;
				m_bStunned = true;
				pobDemon->SetDemonEffectStunned();
			}
		}

		//If we've finished with the bolt-firing thing, start the end "attack".
		if((m_bFlaggedFinished == false) && (bStayInAir == false) && (m_bStunned == false))
		{
			//Queue attack end as next attack.
			if(pobKing->GetAttackComponent()->Boss_Command_RequestDirectAttack(m_pobAttackEnd, true))
			{
				pobKing->GetMessageHandler()->ReceiveMsg<msg_buttonattack>();
				m_bFlaggedFinished = true;	//We know we've queued up the end attack at least.
			}
		}

		//If we were hit while stunned, then push on a new attack-link as a specific recoil to get up from stunned state (rather than
		//snapping to a standard anim which will look rubbish!)
		if((m_iBoltsDeflected >= (KING_NUM_LIGHTNINGBOLTS_3_LIGHT + KING_NUM_LIGHTNINGBOLTS_3_HEAVY))
			&& (m_bStunned == true) && (m_bFlaggedFinished == true) && (m_bRecoverySet == false) && (pobKing->IsHitWhileStunned() == true))
		{
			//Queue stunned recoil as next attack.
			ntPrintf("***###*** KING STUNNED HIT RECOIL (LightningAttack3) ***###***\n");
			if(pobKing->GetAttackComponent()->Boss_Command_RequestDirectAttack(m_pobDazedRecoil, true))
			{
				pobKing->GetMessageHandler()->ReceiveMsg<msg_buttonattack>();
				m_bRecoverySet = true;
				m_bStunned = false;
				pobDemon->SetDemonEffectStandard();
			}
		}

		//If we're currently looping our dazed 'attack', then after a certain number of loops, force on the 2fs.
		if((m_iBoltsDeflected >= (KING_NUM_LIGHTNINGBOLTS_3_LIGHT + KING_NUM_LIGHTNINGBOLTS_3_HEAVY))
			&& (m_bStunned == true) && (m_bFlaggedFinished == true) && (m_iDazedLoopCount > m_iNumDazedLoops) && (m_bRecoverySet == false))
		{
			//Queue stunned 2fs as next attack.
			if(pobKing->GetAttackComponent()->Boss_Command_RequestDirectAttack(m_pobDazedRecovery, true))
			{
				pobKing->GetMessageHandler()->ReceiveMsg<msg_buttonattack>();
				m_bRecoverySet = true;
				m_bStunned = false;
				pobDemon->SetDemonEffectStandard();
			}
		}

		//Have we finished everything?
		if(m_bFlaggedFinished && pobBoss->CanStartAnAttack() && (m_bStunned == false))
		{
			//We've finished.
			pobKing->SetInSpecialAttack(false, NULL);
			if(m_bStunned)
			{
				pobDemon->SetDemonEffectStandard();	//Change the demon effect back to standard.
			}
			return 0;
		}
	}

	return this;
}


bool KingLightningAttack3::IsVulnerableTo(CStrike* pobStrike)
{
	return IsVulnerableTo(pobStrike->GetAttackDataP());
}

bool KingLightningAttack3::IsVulnerableTo(const CAttackData* pobAttackData)
{
	return m_bStunned;	//Also factor in (m_pobBoss->GetAttackComponent()->IsInInvulnerabilityWindow <= 0) ??
}

void KingLightningAttack3::DebugRender(CPoint& obScreenLocation, float fXOffset, float fYOffset)
{
#ifndef _GOLD_MASTER
	g_VisualDebug->Printf3D(obScreenLocation,fXOffset,fYOffset,DC_CYAN,0,"KingLightningAttack3: %i bolts spawned, %i bolts fired, %i bolts deflected",
		m_iBoltsSpawned, m_iBoltsFired, m_iBoltsDeflected);
#endif
}


//===================== SWOOP ATTACK ====================

START_STD_INTERFACE(KingSwoopAttack)
	COPY_INTERFACE_FROM(BossSpecialAttack)
	DEFINE_INTERFACE_INHERITANCE(BossSpecialAttack)

	PUBLISH_PTR_AS(m_pobAttack, Attack)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fTrackTillDistance, 2.5f, TrackTillMinDistance)
END_STD_INTERFACE


KingSwoopAttack::KingSwoopAttack()
{
	m_pobBoss = 0;
	m_pobAttack = 0;
	m_fTrackTillDistance = 2.5f;
	m_bTrackPlayer = true;
}


bool KingSwoopAttack::Initialise(Boss* pobBoss, CEntity* pobPlayer)
{
	ntError_p( pobBoss->GetBossType() == Boss::BT_KING_BOHAN, ("Should not attach a KingSwoopAttack on anyone other than the king!") );

	//Make sure that we've got all the attack links we need, if not, bail.
	ntError_p(m_pobAttack, ("Attack link was NULL"));

	KingBohan* pobKing = (KingBohan*)pobBoss;
	//If we're already in the middle of an attack, then don't initialise another... ever.
	if(pobKing->IsInSpecialAttack())
	{
		ntPrintf("***###*** KingSwoopAttack: SHOULD NOT HAVE HAPPENED (attempting to detach from demon during a special attack [%s]. Selector should've handled this!) ***###***\n",
			pobKing->GetCurrentSpecialName());
		return false;
	}

	m_pobBoss = pobBoss;
	m_bTrackPlayer = true;	//We assume for frame-1 that the player is far enough away to track.

	//Kick off the start, which will link through the attack automatically (no loop or breaks, so it's just one continuous attack).
	//TODO: Pass 'force' flag to this attack request?
	if(pobBoss->GetAttackComponent()->Boss_Command_RequestDirectAttack(m_pobAttack))
	{
		pobBoss->GetMessageHandler()->ReceiveMsg<msg_buttonattack>();
		pobKing->SetInSpecialAttack(true, "KingSwoopAttack");	//No other specials should attempt to start at the same time... which could be very bad visually!
		return true;
	}

	return false;
}


BossAttack* KingSwoopAttack::Update(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	KingBohan* pobKing = (KingBohan*)pobBoss;

	//We keep the king facing the player until the swoop attacks gets sufficiently close. Have to last-minute evade.
	ntError_p(pobPlayer, ("Could not get pointer to player!"));
	if(pobPlayer)
	{
		CPoint obPlayerPos = pobPlayer->GetPosition();
		obPlayerPos.Y() = 0.0f;
		CPoint obKingPos = pobKing->GetPosition();
		obKingPos.Y() = 0.0f;
		CDirection obBetween(obPlayerPos - obKingPos);
		if(obBetween.LengthSquared() < (m_fTrackTillDistance * m_fTrackTillDistance))
		{
			m_bTrackPlayer = false;
		}
	}
	if(m_bTrackPlayer)
	{
		pobKing->GetBossMovement()->m_obTargetPoint = pobPlayer->GetPosition();
		pobKing->GetBossMovement()->m_bTargetPointSet = true;
	}
	else
	{
		pobKing->GetBossMovement()->m_bTargetPointSet = false;
	}

	//TODO: There'll probably be more to do here such as generating strikes on any nearby army guys and making them fly-off the
	//way we want... unless the combat stuff handles that auto-magically.

	//Have we finished everything?
	if(pobBoss->CanStartAnAttack())
	{
		//We're done.
		pobKing->SetInSpecialAttack(false, NULL);
		return 0;
	}

	return this;
}

void KingSwoopAttack::DebugRender(CPoint& obScreenLocation, float fXOffset, float fYOffset)
{
#ifndef _GOLD_MASTER
	g_VisualDebug->Printf3D(obScreenLocation,fXOffset,fYOffset,DC_CYAN,0,"KingSwoopAttack");
#endif
}


void KingSwoopAttack::NotifyAttackFinished()
{

}


void KingSwoopAttack::NotifyAttackAutoLinked()
{

}

BossAttack* KingSwoopAttack::NotifyAttackInterrupted()
{
	ntPrintf("*** KingSwoopAttack Interrupted ***\n");
	KingBohan* pobKing = (KingBohan*)m_pobBoss;
	pobKing->SetInSpecialAttack(false, NULL);

	return 0;
}


//===================== DEMON DETACH ====================

START_STD_INTERFACE(KingDetachFromDemon)
	COPY_INTERFACE_FROM(BossSpecialAttack)
	DEFINE_INTERFACE_INHERITANCE(BossSpecialAttack)

	PUBLISH_PTR_AS(m_pobStart, Start)
	PUBLISH_PTR_AS(m_pobLoop, Loop)
	PUBLISH_PTR_AS(m_pobEnd, End)
END_STD_INTERFACE


KingDetachFromDemon::KingDetachFromDemon()
{
	m_bFlaggedFinished = false;
	m_pobStart = m_pobLoop = m_pobEnd = 0;
}


bool KingDetachFromDemon::Initialise(Boss* pobBoss, CEntity* pobPlayer)
{
	ntError_p( pobBoss->GetBossType() == Boss::BT_KING_BOHAN, ("Should not attach a KingDetachFromDemon on anyone other than the king!") );

	//Make sure that we've got all the attack links we need, if not, bail.
	ntError_p(m_pobStart && m_pobLoop && m_pobEnd, ("One or more attack links was NULL"));

	KingBohan* pobKing = (KingBohan*)pobBoss;
	//If we're already in the middle of an attack, then don't initialise another... ever.
	if(pobKing->IsInSpecialAttack())
	{
		ntPrintf("***###*** KingDetachFromDemon: SHOULD NOT HAVE HAPPENED (attempting to detach from demon during a special attack [%s]. Selector should've handled this!) ***###***\n",
			pobKing->GetCurrentSpecialName());
		return false;
	}


	//TODO: Do we want the king to update his view to the player at all during this? Or should it be something that he doesn't move during?
/*
	pobBoss->GetBossMovement()->m_obTargetPoint = pobPlayer->GetPosition();
	pobBoss->GetBossMovement()->m_bTargetPointSet = true;
*/

	m_pobBoss = pobBoss;
	m_bFlaggedFinished = false;

	//Kick off the start, which will link to the loop where it'll remain until the demon is flagged as detached (on it's own control).
	//TODO: Pass 'force' flag to this attack request?
	if(pobBoss->GetAttackComponent()->Boss_Command_RequestDirectAttack(m_pobStart))
	{
		pobBoss->GetMessageHandler()->ReceiveMsg<msg_buttonattack>();

		//Force the demon into a specific phase where it will select it's own movement transitions to handle detaching and going about
		//its business.
		KingBohan* pobKing = (KingBohan*)m_pobBoss;
		CHashedString obAscendPhaseName("Demon_AscendPhase_Attack");
		pobKing->GetDemon()->SetNamedAttackPhase(obAscendPhaseName);
		pobKing->SetInSpecialAttack(true, "DetachFromDemon");	//No other specials should attempt to start at the same time... which could be very bad visually!

		return true;
	}

	return false;
}


void KingDetachFromDemon::NotifyAttackFinished()
{
	KingBohan* pobKing = (KingBohan*)m_pobBoss;
	pobKing->SetInSpecialAttack(false, NULL);
}


void KingDetachFromDemon::NotifyAttackAutoLinked()
{

}

BossAttack* KingDetachFromDemon::NotifyAttackInterrupted()
{
	//Output a message in-case interrupting this causes any kind of hang (for debug purposes).
	ntPrintf("*** KingDetachFromDemon Interrupted ***\n");
	//Flag that we're no-longer in a special attack.
	KingBohan* pobKing = (KingBohan*)m_pobBoss;
	pobKing->SetInSpecialAttack(false, NULL);
	return 0;
}


BossAttack* KingDetachFromDemon::Update(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	KingBohan* pobKing = (KingBohan*)pobBoss;

	//TODO: Do we want to keep facing the player throughout or not?
/*
	//Keep the king facing the player throughout.
	pobKing->GetBossMovement()->m_obTargetPoint = pobPlayer->GetPosition();
	pobKing->GetBossMovement()->m_bTargetPointSet = true;

*/

	//If the demon is detached, we can go to our end "attack" to return to normality.
	Demon* pobDemon = (Demon*)pobKing->GetDemon();
	if((pobDemon->IsAttachedToKing() == false) && (m_bFlaggedFinished == false))
	{
		//Queue attack end
		//TODO: Use Boss_Command_RequestDirectNextAttack instead?
		if(pobKing->GetAttackComponent()->Boss_Command_RequestDirectAttack(m_pobEnd, true))
		{
			pobKing->GetMessageHandler()->ReceiveMsg<msg_buttonattack>();
			m_bFlaggedFinished = true;
		}
	}

	//Have we finished everything?
	if(m_bFlaggedFinished && pobBoss->CanStartAnAttack() && m_bFlaggedFinished)
	{
		//We're done.
		pobKing->SetInSpecialAttack(false, NULL);
		return 0;
	}

	return this;
}

void KingDetachFromDemon::DebugRender(CPoint& obScreenLocation, float fXOffset, float fYOffset)
{
#ifndef _GOLD_MASTER
	g_VisualDebug->Printf3D(obScreenLocation,fXOffset,fYOffset,DC_CYAN,0,"KingDetachFromDemon");
#endif
}


//===================== DEMON ATTACH ====================

START_STD_INTERFACE(KingAttachToDemon)
	COPY_INTERFACE_FROM(BossSpecialAttack)
	DEFINE_INTERFACE_INHERITANCE(BossSpecialAttack)

	PUBLISH_PTR_AS(m_pobStart, Start)
	PUBLISH_PTR_AS(m_pobLoop, Loop)
	PUBLISH_PTR_AS(m_pobEnd, End)
END_STD_INTERFACE


KingAttachToDemon::KingAttachToDemon()
{
	m_bFlaggedFinished = false;
	m_pobStart = m_pobLoop = m_pobEnd = 0;
}


bool KingAttachToDemon::Initialise(Boss* pobBoss, CEntity* pobPlayer)
{
	ntError_p( pobBoss->GetBossType() == Boss::BT_KING_BOHAN, ("Should not attach a KingAttachToDemon on anyone other than the king!") );

	//Make sure that we've got all the attack links we need, if not, bail.
	ntError_p(m_pobStart && m_pobLoop && m_pobEnd, ("One or more attack links was NULL"));

	//TODO: Do we want the king to update his view to the player at all during this? Or should it be something that he doesn't move during?
/*
	pobBoss->GetBossMovement()->m_obTargetPoint = pobPlayer->GetPosition();
	pobBoss->GetBossMovement()->m_bTargetPointSet = true;
*/

	KingBohan* pobKing = (KingBohan*)pobBoss;
	//If we're already in the middle of an attack, then don't initialise another... ever.
	if(pobKing->IsInSpecialAttack())
	{
		ntPrintf("***###*** KingAttachToDemon: SHOULD NOT HAVE HAPPENED (attempting to attach to demon during special [%s], selector should've handled this) ***###***\n",
			pobKing->GetCurrentSpecialName());
		return false;
	}

	m_pobBoss = pobBoss;
	m_bFlaggedFinished = false;

	//Kick off the start, which will link to the loop where it'll remain until the demon is flagged as attached (on it's own control).
	//TODO: Pass 'force' flag to this attack request?
	if(pobBoss->GetAttackComponent()->Boss_Command_RequestDirectAttack(m_pobStart))
	{
		pobBoss->GetMessageHandler()->ReceiveMsg<msg_buttonattack>();

		//Force the demon into a specific phase where it will select it's own movement transitions to handle returning and attaching again.
		KingBohan* pobKing = (KingBohan*)m_pobBoss;
		CHashedString obDescendPhaseName("Demon_DescendPhase_Attack");
		pobKing->GetDemon()->SetNamedAttackPhase(obDescendPhaseName);
		pobKing->SetInSpecialAttack(true, "AttachToDemon");

		return true;
	}

	return false;
}


void KingAttachToDemon::NotifyAttackFinished()
{

}


void KingAttachToDemon::NotifyAttackAutoLinked()
{

}

BossAttack* KingAttachToDemon::NotifyAttackInterrupted()
{
	//Output a message in-case interrupting this causes any kind of hang (for debug purposes).
	ntPrintf("*** KingAttachToDemon Interrupted ***\n");
	//Flag that we're no-longer in a special attack.
	KingBohan* pobKing = (KingBohan*)m_pobBoss;
	pobKing->SetInSpecialAttack(false, NULL);
	return 0;
}


BossAttack* KingAttachToDemon::Update(float fTimeDelta, Boss* pobBoss, CEntity* pobPlayer)
{
	KingBohan* pobKing = (KingBohan*)pobBoss;

	//TODO: Do we want to keep facing the player throughout or not?
/*
	//Keep the king facing the player throughout.
	pobKing->GetBossMovement()->m_obTargetPoint = pobPlayer->GetPosition();
	pobKing->GetBossMovement()->m_bTargetPointSet = true;

*/
	//If the demon is attached, we can go to our end "attack" to return to normality.
	Demon* pobDemon = (Demon*)pobKing->GetDemon();
	if((pobDemon->HasJustAttachedToKing() == true) && (m_bFlaggedFinished == false))
	{
		//Queue attack end.
		//TODO: Use Boss_Command_RequestDirectNextAttack instead?
		if(pobKing->GetAttackComponent()->Boss_Command_RequestDirectAttack(m_pobEnd, true))
		{
			pobKing->GetMessageHandler()->ReceiveMsg<msg_buttonattack>();
			m_bFlaggedFinished = true;
		}
	}

	//Have we finished everything?
	//Note: We allow a small grace-period for the demon's 2fs anims to finish by waiting until it has actually set it's m_bAttachedToKing
	//flag to true (this happens after the one-extra-cycle anim and the cloud-to-wings-2fs anim).
	//This ensures that we never select to do anything ground-based just because the wing's haven't finished their 2fs.
	if(m_bFlaggedFinished && pobDemon->IsAttachedToKing() && pobBoss->CanStartAnAttack() && m_bFlaggedFinished)
	{
		//We're done.
		pobKing->SetInSpecialAttack(false, NULL);
		return 0;
	}

	return this;
}

void KingAttachToDemon::DebugRender(CPoint& obScreenLocation, float fXOffset, float fYOffset)
{
#ifndef _GOLD_MASTER
	g_VisualDebug->Printf3D(obScreenLocation,fXOffset,fYOffset,DC_CYAN,0,"KingAttachToDemon");
#endif
}
