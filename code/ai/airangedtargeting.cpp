//! -------------------------------------------
//! airangedtargeting.cpp
//!
//! AI Ranged Targeting
//!
//! Author: Gavin Costello
//!
//!--------------------------------------------

#include "airangedtargeting.h"
#include "core/visualdebugger.h"

#include "ai/aivision.h"
#include "ai/ainavigationsystem/ainavigsystemman.h"
#include "editable/enums_ai.h"
#include "game/movementcontrollerinterface.h"		// For MovementControllerUtilities::RotationAboutY
#include "game/entitymanager.h"
#include "game/entitybrowser.h"
#include "game/entityinfo.h"						// For IsDead()
#include "game/query.h"

#include "camera/camman.h"
#include "camera/camview.h"

#include "game/shellconfig.h"						// For shell-options about debug-rendering.
#include "gfx/display.h"
#include "game/entityarcher.h"

//For aiming purposes (predicting position requires knowledge of projectile attributes)
#include "game/entityrangedweapon.h"
#include "game/entityprojectile.h"
#ifndef _PS3_RUN_WITHOUT_HAVOK_BUILD
#include "Physics/advancedcharactercontroller.h"
#include "physics/system.h"
#include "physics/projectilelg.h"
#endif


//--------------------------------------------------
//!
//! Constructor, sets default values.
//!
//--------------------------------------------------
CAIRangedTargeting::CAIRangedTargeting()
{
	m_obTargetPoint = CPoint(CONSTRUCT_CLEAR);
	m_pParentEntity = 0;
	m_fCurrentTimeTillCooldownBegins = m_obParams.m_fContractionCooldownTime;
	m_fCurrentExpansion = m_obParams.m_fMaxExpansion;
	m_fCurrentExpansionScalar = 1.0f;
#ifdef AI_RANGEDTARGETING_PERIODIC_AIM_UPDATE
	m_fCurrentTimeSinceLastAimUpdate = 0.0f;
#endif
	m_fAmountOfTimeTargetIsMovingContinuously = 0.0f;
	m_fLastAmountOfTimeTargetIsMovingContinuously = 0.0f;
	m_bInFrustumLastFrame = false;
	m_bArcherTargetAimingLastFrame = false;
}


//--------------------------------------------------
//!
//! Destructor.
//!
//--------------------------------------------------
CAIRangedTargeting::~CAIRangedTargeting()
{
}


//--------------------------------------------------
//!
//! Sets the parent-entity for this object and checks it's a valid AI.
//!
//--------------------------------------------------
void CAIRangedTargeting::SetParent(CEntity* pParentEntity)
{
	if(pParentEntity)
	{
		//Make sure it's only attached to AI entities.
		ntError_p(pParentEntity->IsAI(), ("Ranged Targetting components should only be added to AI, %s is not an AI", pParentEntity->GetName().c_str()));

		m_pParentEntity = static_cast<AI*>(pParentEntity);
	}
}


//--------------------------------------------------
//!
//! Sets a specific targeting parameter (called from LUA) based on enum-value.
//!
//--------------------------------------------------
void CAIRangedTargeting::SetRangedTargetingParam(unsigned int uiParam, float fValue)
{
	if (fValue < 0.0f)
	{
		ntPrintf("WARNING: Negative value passed into SetRangedTargetingParam for param-value. Not using\n");
		return;
	}
	if (uiParam >= RTP_NUM_PARAMS)
	{
		ntError_p(false, ("Attempting to pass invalid (out of enum-range) value %d to SetRangedTargetingParam", uiParam));
		return;
	}

	switch (uiParam)
	{
	case RTP_MAX_EXPANSION:
		ntPrintf("Setting max-expansion to %f\n", fValue);
		if(fValue > 20.0f)
		{
			ntPrintf("Warning: Setting max-expansion to a value over 20 (%f). Is this intentional or just garbage getting in? Tell GavC\n",
				fValue);
		}
		m_obParams.m_fMaxExpansion = (fValue > 20.0f) ? 15.0f : fValue;	//Just protect against garbage a little without limiting too much.
		break;
	case RTP_MAX_EXPANSION_DISTANCE:
		ntPrintf("Setting max-expansion-distance to %f\n", fValue);
		if(fValue > 200.0f)
		{
			ntPrintf("Warning: Setting max-expansion-distance to a value over 200 (%f). Is this intentional or just garbage getting in? Tell GavC\n",
				fValue);
		}
		m_obParams.m_fMaxExpansionDistance = (fValue > 200.0f) ? 200.0f : fValue;
		break;
	case RTP_MIN_CONTRACTION:
		ntPrintf("Setting min-contraction to %f\n", fValue);
		if(fValue > 20.0f)
		{
			ntPrintf("Warning: Setting min-contraction to a value over 20 (%f). Is this intentional or just garbage getting in? Tell GavC\n",
				fValue);
		}
		m_obParams.m_fMinContraction = (fValue > 20.0f) ? 20.0f : fValue;	//Just protect against garbage a little without limiting too much.
		break;
	case RTP_CONTRACTION_COOLDOWN:
		ntPrintf("Setting contraction-cooldown to %f\n", fValue);
		m_obParams.m_fContractionCooldownTime = fValue;
		break;
	case RTP_CONTRACTION_TIME:
		ntPrintf("Setting contraction-time to %f\n", fValue);
		m_obParams.m_fContractionTime = fValue;
		break;
	case RTP_MOVINGEXPANSION_TIME:
		ntPrintf("Setting moving-expansion time to %f\n", fValue);
		m_obParams.m_fTimeRunningContinuouslyForExpansionEvent = fValue;
		break;
	case RTP_STOPPINGEXPANSION_MINTIME:
		ntPrintf("Setting minimum-time moving before target-stopping expansion takes effect to %f\n", fValue);
		m_obParams.m_fMinTimeRunningBeforeStopForExpansionEvent = fValue;
	default:
		break;
	}

	//For now we'll just warp people if they've done something silly.
	if(m_obParams.m_fMaxExpansion < m_obParams.m_fMinContraction)
	{
		ntError_p(false, ("***Max-expansion [%f] is actually smaller than min-contraction [%f], check setting order!!!***\n",
			m_obParams.m_fMaxExpansion, m_obParams.m_fMinContraction));
		ntPrintf("Setting min-contraction to be max-expansion so that everything else works fine still. Be sure to fix your setting-order!\n");
		m_obParams.m_fMinContraction = m_obParams.m_fMaxExpansion;
	}
}


//--------------------------------------------------
//!
//! Checks for expansion events, where the aiming reticule should expand, reducing
//! the accuracy of the AI temporarily (until the reticule shrinks again).
//!
//--------------------------------------------------
bool CAIRangedTargeting::CheckForExpansionEvents(const Character* pTargetEntity)
{
	//These only apply if our target is Nariko or Kai, so we can skip all the checks if that's not the case.
	if(pTargetEntity && pTargetEntity->IsPlayer())
	{
		//If our target is moving for enough time (value can be set via lua) then our aiming reticule resets.
		if((m_fAmountOfTimeTargetIsMovingContinuously >= m_obParams.m_fTimeRunningContinuouslyForExpansionEvent)
			&& (m_fLastAmountOfTimeTargetIsMovingContinuously < m_obParams.m_fTimeRunningContinuouslyForExpansionEvent))
		{
			ntPrintf("***Expansion event*** 'Target moving for more than %f seconds' triggered\n",
				m_obParams.m_fTimeRunningContinuouslyForExpansionEvent);
			//Reset our timers so that this happens over and over every m_fTimeRunningContinuouslyForExpansionEvent seconds.
			m_fAmountOfTimeTargetIsMovingContinuously = 0.0f;
			m_fLastAmountOfTimeTargetIsMovingContinuously = 0.0f;
			return true;
		}

		//If the player stops moving, and was previously moving for over a specified minimum amount of time (set from lua)
		//then our aiming reticule resets.
		if((m_fLastAmountOfTimeTargetIsMovingContinuously > m_fAmountOfTimeTargetIsMovingContinuously)
			&& (m_fLastAmountOfTimeTargetIsMovingContinuously > m_obParams.m_fMinTimeRunningBeforeStopForExpansionEvent))
		{
			ntPrintf("***Expansion event*** 'Target stopped after moving for at least %f seconds' triggered\n",
				m_obParams.m_fMinTimeRunningBeforeStopForExpansionEvent);
			return true;
		}

		//If our target is the archer and they've just gone into aiming mode, then we trigger an expansion event to give them
		//time to aim a bit.
		if(pTargetEntity->IsPlayer() && pTargetEntity->ToPlayer()->IsArcher())
		{
			const Archer* pArcher = pTargetEntity->ToPlayer()->ToArcher();
			bool bArcherTargetIsAiming = pArcher->IsIn1stPersonState();
			if(bArcherTargetIsAiming && !m_bArcherTargetAimingLastFrame)
			{
				ntPrintf("***Expansion event*** 'Archer target started aiming' triggered\n");
				m_bArcherTargetAimingLastFrame = bArcherTargetIsAiming;
				return true;
			}
			m_bArcherTargetAimingLastFrame = bArcherTargetIsAiming;
		}

		//If this AI has just left the view frustum then trigger an expansion event. If the player swings the camera to focus on
		//another AI then they shouldn't immediately be shot from offscreen by this AI. We trigger the expansion event so that the
		//next few shots will miss but be notification-enough that they're being fired at before being hit.
		//To check whether we're in the frustum we have to manually create a little box around ourselves and check each point to see
		//if it's on-screen. This isn't precise, and is quite expensive, but there doesn't seem to be any nice "Box in frustum" check
		//provided.
		bool bInFrustum = false;

		//Get where we are on screen.
		CPoint obCurrentPosition = m_pParentEntity->GetPosition();
		// Use a bounding box and test each of its verts
		CPoint obCullBoxHalfExtents(1.0f,1.0f,1.0f);
		CPoint obScreenPoints[8];
		obScreenPoints[0] = obCurrentPosition;
		obScreenPoints[0].Y() -= obCullBoxHalfExtents.Y();
		obScreenPoints[0].X() -= obCullBoxHalfExtents.X();
		obScreenPoints[1] = obCurrentPosition;
		obScreenPoints[1].Y() -= obCullBoxHalfExtents.Y();
		obScreenPoints[1].X() += obCullBoxHalfExtents.X();
		obScreenPoints[2] = obCurrentPosition;
		obScreenPoints[2].Y() -= obCullBoxHalfExtents.Y();
		obScreenPoints[2].Z() -= obCullBoxHalfExtents.Z();
		obScreenPoints[3] = obCurrentPosition;
		obScreenPoints[3].Y() -= obCullBoxHalfExtents.Y();
		obScreenPoints[3].Z() += obCullBoxHalfExtents.Z();
		obScreenPoints[4] = obCurrentPosition;
		obScreenPoints[4].Y() += obCullBoxHalfExtents.Y();
		obScreenPoints[4].X() -= obCullBoxHalfExtents.X();
		obScreenPoints[5] = obCurrentPosition;
		obScreenPoints[5].Y() += obCullBoxHalfExtents.Y();
		obScreenPoints[5].X() += obCullBoxHalfExtents.X();
		obScreenPoints[6] = obCurrentPosition;
		obScreenPoints[6].Y() += obCullBoxHalfExtents.Y();
		obScreenPoints[6].Z() -= obCullBoxHalfExtents.Z();
		obScreenPoints[7] = obCurrentPosition;
		obScreenPoints[7].Y() += obCullBoxHalfExtents.Y();
		obScreenPoints[7].Z() += obCullBoxHalfExtents.Z();

		const CCamera* pCurrCamera = CamMan::GetPrimaryView();
		for(int i = 0 ; i < 8 ; i++)
		{
			if (pCurrCamera->CanSeePoint(obScreenPoints[i]))
			{
				//At least one of our points is on-screen, we're in-frustum. Notify and break.
				bInFrustum = true;
				break;
			}
		}

		if(!bInFrustum && m_bInFrustumLastFrame)
		{
			ntPrintf("***Expansion event*** 'AI Left View Frustum' triggered\n");
			m_bInFrustumLastFrame = bInFrustum;
			return true;
		}

		m_bInFrustumLastFrame = bInFrustum;
	}

	//Otherwise we definitely don't need any kind of expansion events.
	return false;
}


//--------------------------------------------------
//!
//! Handles expanding the aiming reticule on-expansion-event. Expands to a
//! maximum value scaled down with distance.
//! Also scales to a gradually smaller size if repeated expansion-events occur
//! in rapid-succession (to stop the player from abusing the system).
//!
//--------------------------------------------------
void CAIRangedTargeting::ForceReticuleExpansion()
{
	//To stop this happening over and over and over we gradually shrink the scalar and only set to scalar * max-expansion.
	//The scalar is only ever set to 1.0f if we were currently at min-contraction when the call was made.
	//This stops people from abusing the expansion triggers to attempt to force ranged-aiming to stay inaccurate.
	//(E.G. Archer going into aimed forces expansion, could theoretically just keep going into to stop them becoming accurate otherwise!)
	if(m_fCurrentExpansion > m_obParams.m_fMinContraction)
	{
		m_fCurrentExpansionScalar -= 0.1f;
		if(m_fCurrentExpansionScalar < 0.0f)
		{
			m_fCurrentExpansionScalar = 0.0f;
		}
	}
	else
	{
		m_fCurrentExpansionScalar = 1.0f;
	}

	//Unaltered (non-scaled) expansion-range used when calculating distance-scaled max-expansion
	float fExpansionRange = m_obParams.m_fMaxExpansion - m_obParams.m_fMinContraction;
	float fScaledMaxExpansion = m_obParams.m_fMaxExpansion;

	//The MaxExpansion value should change depending on how far away the target is. It should only ever actually be m_fMaxExpansion
	//when the target is at (or over) m_fMaxExpansionDistance away. As the target gets closer, we scale down linearly between m_fMaxExpansion
	//and m_fMinContraction. This way their worst-case aim gets gradually better as the target gets closer. They're soldiers after all!
	CDirection obToTarget = CDirection(m_obTargetPoint - m_pParentEntity->GetPosition());
	float fDistanceToTarget = obToTarget.Length();
	if(fDistanceToTarget < m_obParams.m_fMaxExpansionDistance)
	{
		fScaledMaxExpansion = fExpansionRange * (fDistanceToTarget / m_obParams.m_fMaxExpansionDistance);
	}
	
	//Scale between MaxExpansion and MinContraction... taking into account the scaled max-expansion (depending on distance to target).
	float fScaledExpansionRange = fScaledMaxExpansion - m_obParams.m_fMinContraction;
	m_fCurrentExpansion = (fScaledExpansionRange * m_fCurrentExpansionScalar) + m_obParams.m_fMinContraction;
	//Remember to reset our cooldown timer (before contraction begins).
	m_fCurrentTimeTillCooldownBegins = m_obParams.m_fContractionCooldownTime;
}


//--------------------------------------------------
//!
//! Updates the aiming for ranged AI, including predictive aiming, and stores
//! the new target position interally to be accessed when-firing.
//!
//--------------------------------------------------
void CAIRangedTargeting::Update(float fTimeChange)
{
	if(!m_pParentEntity || !m_pParentEntity->GetRangedWeapon())
	{
		return;
	}

	//Get a pointer to our current target, if we don't have one, we don't need to do anything here (and we reset expansion etc for new
	//target when we get one).
	const CEntity* pTargetEntity = 0;
	CAIComponent* pCAIComp = m_pParentEntity->GetAIComponent();

	//We only go through this whole updating process if we can currently see our enemy within our shooting cone... this should reduce
	//the number of calculations for levels like Temple considerably! (as it's per-frame on all AIs with ranged-weapons otherwise!!!).
	if(!pCAIComp->GetAIVision()->IsTargetInShootingRange())
	{
		return;
	}

	if(pCAIComp->GetCAIMovement())
	{
		pTargetEntity = pCAIComp->GetCAIMovement()->GetEntityToAttack();
	}

	//If we have no target, or the target isn't a character, then return.
	//TODO: Will we ever want to target non-character objects? Perhaps, then this function will need changing to only try
	//to do position-prediction if it's a character (can store result of IsCharacter() in boolean early-on and just use later),
	//but do everything else (practically) regardless...
	if(!pTargetEntity || !pTargetEntity->IsCharacter())
	{
		m_fCurrentExpansionScalar = 1.0f;
		m_fCurrentExpansion = m_obParams.m_fMaxExpansion;
		m_fCurrentTimeTillCooldownBegins = m_obParams.m_fContractionCooldownTime;
		return;
	}

	const Character* pTargetCharacter = pTargetEntity->ToCharacter();
	if(!pTargetCharacter)
	{
		ntError_p(false, ("Could not get character pointer to valid entity where IsCharacter returned true... oddness"));
		return;
	}

	/*
	TODO: Should we bother to check whether the current distance from the target should scale down our current expansion if it's over the
	max-expansion?
	Reasons to do this:
	- 1. As the target runs towards this AI (or vice-versa) the aim will get better, as it will be scaling down the max-scaling, which
		COULD potentially make the new scaled max-expansion smaller than current-expansion.
	- 2. If you manage to somehow sneak up on an AI with a ranged weapon, then their initial expansion would still be m_fMaxExpansion when
		at this closer distance it should've perhaps scaled down immediately.
	Reasons not to do this:
	- 1. Extra distance-checks (although can be left as squared values to avoid sqrt)
	- 2. The accuracy would be improving anyway, just on the standard over-time basis.
	- 3. Aside from the sneaking-up case above, it probably won't make any visible difference, and at the next expansion-event the distance-scale
		would have kicked in anyway.

	... Left out for now, perhaps add later (done the same way as it's done in ForceReticuleExpansion()).
	*/

	//Adjust the size of the aiming reticule's expansion by the appropriate amount if we're not in our 'wait' stage.
	if(m_fCurrentTimeTillCooldownBegins > 0.0f)
	{
		m_fCurrentTimeTillCooldownBegins -= fTimeChange;
		if(m_fCurrentTimeTillCooldownBegins < 0.0f)
		{
			m_fCurrentTimeTillCooldownBegins = 0.0f;
		}
	}

	if(m_fCurrentTimeTillCooldownBegins <= 0.0f)
	{
		if(m_obParams.m_fContractionTime == 0.0f)
		{
			ntPrintf("WARNING: m_fContractionTime is 0, avoiding divide by 0, cannot use this value as-is!\n");
			m_obParams.m_fContractionTime = 1.0f;
		}
		if(m_fCurrentExpansion > m_obParams.m_fMinContraction)
		{
			m_fCurrentExpansion -= (fTimeChange / m_obParams.m_fContractionTime) * (m_obParams.m_fMaxExpansion - m_obParams.m_fMinContraction);
		}
		if(m_fCurrentExpansion < m_obParams.m_fMinContraction)
		{
			m_fCurrentExpansion = m_obParams.m_fMinContraction;
		}
	}


	//Get the current speed of our target (debug render line so we can check all is well).
	CDirection obTargetVelocity(CONSTRUCT_CLEAR);
	if(pTargetCharacter->GetPhysicsSystem())
	{
		Physics::AdvancedCharacterController* pobCC =
			(Physics::AdvancedCharacterController*)pTargetCharacter->GetPhysicsSystem()->GetFirstGroupByType( Physics::LogicGroup::ADVANCED_CHARACTER_CONTROLLER );

		//We ignore the movement if our target is currently soft-parented (such as on moving platforms).
		//This stops the expansion-event "target has been moving for 'x' amount of time" from triggering when the target (e.g. archer) is
		//on a moving platform.
		if(pobCC && !pobCC->IsSoftParented())	//Should always be the case... but just to be sure.
		{
			obTargetVelocity = pobCC->GetLinearVelocity();
		}
	}


	float fProjectileSpeed = 20.0f;	//We replace this below... unless we couldn't get a pointer to the projectile properties for any reason.
	//Only do all this for moving target.
	float fTargetSpeed = obTargetVelocity.Length();
	//Any character parented to an object (e.g. moving platform?) will never be classed as moving, as presumably they'll have a very limited
	//range of movement, and any movement done by the parent entity shouldn't cause an expansion event.
	bool bTargetMoving = (fTargetSpeed > 0.3f);
	m_fAmountOfTimeTargetIsMovingContinuously = (bTargetMoving == true) ? m_fAmountOfTimeTargetIsMovingContinuously + fTimeChange : 0.0f;

	//See if any events have happened with us or our archer target to see if we need to maximise our current reticule expansion.
	if(CheckForExpansionEvents(pTargetCharacter))
	{
		ForceReticuleExpansion();
	}

	if(bTargetMoving)
	{
		//NOTE: We have to directly access the projectile attributes off of the ranged weapon to know what speed the projectile is going
		//to move at for predictive aiming that's anywhere near accurate.
		Object_Ranged_Weapon* pRangedWeapon = static_cast<Object_Ranged_Weapon*>(m_pParentEntity->GetRangedWeapon());
		if(!pRangedWeapon)
		{
			ntError_p(false, ("Ranged-weapon retrieval wierdness"));	//We already checked GetRangedWeapon, so this shouldn't happen.
			return;
		}
		Projectile_Attributes* pProjAttrs = pRangedWeapon->GetProjectileAttributes();
		if(!pProjAttrs)
		{
			return;
		}

		ProjectileProperties* pobProperties = ObjectDatabase::Get().GetPointerFromName<ProjectileProperties*>(pProjAttrs->m_Properties);
		if(!pobProperties)
		{
			ntPrintf("Warning: Could not get projectile properties from ranged-weapon to extract speed. Predictive aiming may be WAY off. Tell GavC\n");
		}
		else
		{
			//Extract speed here... this is taken as the initial speed on the projectile... we can always factor-in acceleration below.
			fProjectileSpeed = pobProperties->m_fInitialSpeed;
		}
	}

	//The centre of the circle reticule is aimed at the player's position but offset from the ground.
	//The amount we offset from their root (feet) depends on whether or not our character is the archer and is crouching.
	//We also take into account the fact that they'll still be notified as 'crouching' if they've switched to 1st person,
	//so we make sure they're not in aiming mode from a crouched-location too, otherwise we want to aim higher.
	CPoint obTargetExactPosition(CONSTRUCT_CLEAR);
	if(pTargetEntity->IsPlayer() && pTargetEntity->ToPlayer()->IsArcher()
		&& pTargetEntity->ToPlayer()->ToArcher()->IsCrouching()
		&& !pTargetEntity->ToPlayer()->ToArcher()->IsIn1stPersonState())
	{
		obTargetExactPosition = pTargetCharacter->GetPosition() + CPoint(0.0f, 0.2f, 0.0f);
	}
	else
	{
		obTargetExactPosition = pTargetCharacter->GetPosition() + CPoint(0.0f, 1.3f, 0.0f);
	}

	//Render the current velocity of the target ON the target we're aiming at.
#ifndef _GOLD_MASTER
	if(g_ShellOptions->m_bRangedTargetDebug)
	{
		g_VisualDebug->RenderLine(obTargetExactPosition, obTargetExactPosition + CPoint(obTargetVelocity), 0xffffffff);
	}
#endif

	//Account for a moving target... we still want our standard circle-based offset, but then we shift it to an offset around a
	//predicted position instead... (instead of just using GetPosition() above).
	CPoint obPredictedTargetPosition = obTargetExactPosition;
	float fTimeToHitTarget = 0.0f;
	float fLastTimeToHitTarget = 0.0f;

	//We solve this with an iterative approximation. Don't get me wrong, I'm all for a "proper" solution, but any added factors later would
	//make it increasingly more complex, where a nice iterative approximation just needs to run some "microsimulations" to adjust. Easier to
	//factor in extra bits such as projectile-acceleration without having to go back to the drawing-board with the equation.
	//We iterate a maximum of 3 times to narrow down our error to an acceptable level.
	//We also only bother to do this if our target is currently moving of course.
	//FIXME: On second thoughts, this is a pretty expensive way of doing it, though it'll only apply to active crossbowmen that can currently
	//see the player (see check early-on in this function)... hmmm... likely not a problem.
	if(bTargetMoving)
	{
		for(int i = 0 ; i < 3 ; i++)
		{
			fTimeToHitTarget = CDirection(obPredictedTargetPosition - m_pParentEntity->GetPosition()).Length() / fProjectileSpeed;
			obPredictedTargetPosition = obPredictedTargetPosition + CDirection(obTargetVelocity * (fTimeToHitTarget - fLastTimeToHitTarget));
			fLastTimeToHitTarget = fTimeToHitTarget;
		}
	}


	//Calculate where our random offset (on the edge of the aiming reticule) is going to be.
	CDirection obLine2Enemy = CDirection(pTargetCharacter->GetPosition() - m_pParentEntity->GetPosition());
	obLine2Enemy.Y() = 0;
	obLine2Enemy.Normalise();

	float fRandAngle = grandf(1.0f) > 0.5f ? grandf(PI) : -grandf(PI);
	float fSin, fCos;
	CMaths::SinCos(fRandAngle, fSin, fCos);

	CDirection obOffsetDirection = CDirection(obLine2Enemy.X()*fCos - obLine2Enemy.Z()*fSin,
											  obLine2Enemy.X()*fSin + obLine2Enemy.Z()*fCos,
											  obLine2Enemy.Z());
	obOffsetDirection.Normalise();
	obOffsetDirection *= m_fCurrentExpansion;

	//TODO: Account for if our target is the archer and is crouching in a cover position. If so, change our aim ever so slightly lower
	//so that we hit whatever they're covering behind more often. Don't lower it too much though, as if they're in a crouching volume
	//but not BEHIND a crouching volume, we want to be able to hit them still! We could always trace, but that'll be more expensive.

#ifdef AI_RANGEDTARGETING_PERIODIC_AIM_UPDATE
	//Only update our actual aim-target periodically (mainly useful during debug, so there's time for the arrow to pass-through the target).
	m_fCurrentTimeSinceLastAimUpdate += fTimeChange;
	if(m_fCurrentTimeSinceLastAimUpdate > 0.2f)	//Only update our actual aim 5 times per second.	//TODO: Finalise value, if used.
	{
		m_obTargetPoint = obPredictedTargetPosition + CPoint(obOffsetDirection);
		m_fCurrentTimeSinceLastAimUpdate = 0.0f;
	}
#else
	//Just update every frame.
	m_obTargetPoint = obPredictedTargetPosition + CPoint(obOffsetDirection);
#endif

	//Debug-render our target point.
#ifndef _GOLD_MASTER
	if(g_ShellOptions->m_bRangedTargetDebug)
	{
	g_VisualDebug->RenderPoint(m_obTargetPoint, 40, DC_YELLOW);
	}
#endif
	//Store the last-known amount of time that our target is continuously moving for.
	m_fLastAmountOfTimeTargetIsMovingContinuously = m_fAmountOfTimeTargetIsMovingContinuously;

	return;
}
