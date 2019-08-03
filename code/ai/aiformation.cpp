//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file aiformation.cpp                                                               
//!                                                                                         
//------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
// Required Includes                                                                        
//------------------------------------------------------------------------------------------

#include "ai/aiformation.h"
#include "ai/aiformationmanager.h"
#include "ai/aiformationattack.h"
#include "ai/aiformationcomponent.h"

#include "game/attacks.h"
#include "game/entitymanager.h"
#include "game/aicomponent.h"
#include "game/messagehandler.h"
#include "core/visualdebugger.h"
#include "core/gatso.h"
#include "ai/ainavigationsystem/ainavigsystemman.h"
#include "objectdatabase/dataobject.h"

//------------------------------------------------------------------------------------------
// Constants                                                                                
//------------------------------------------------------------------------------------------

#define SLOT_RADIUS		0.75f

#define DEFAULT_FORMATION_RUN_DIST_SQRD  25.0f
#define DEFAULT_FORMATION_WALK_DIST_SQRD  4.0f
#define DEFAULT_FORMATION_STRAFE_DIST_SQRD  9.0f
#define DEFAULT_FORMATION_DEFENCE_RANGE  1.5f
#define DEFAULT_FORMATION_DEFENCE_TIMER  0.1f

#define COLLISION_Y_OFFSET (0.1f)
#define DBG_Y_OFFSET CPoint(0,COLLISION_Y_OFFSET,0)

START_CHUNKED_INTERFACE(AIFormation, Mem::MC_AI)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fPriority, 1.0f, Priority)
	PUBLISH_PTR_AS(m_pTargetEntity, TargetEntity)
	PUBLISH_PTR_AS(m_pLockonEntity, LockonEntity)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bAggressive, false, Aggressive)			
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bCameraRelative, false, CameraRelative)			
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bClosestSlot, false, ClosestSlot)			
	PUBLISH_VAR_WITH_DEFAULT_AS(m_ptOffset, CPoint(CONSTRUCT_CLEAR), TargetOffset)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fConstraintAngle, USE_DEFAULT, ConstraintAngle)			
END_STD_INTERFACE

//------------------------------------------------------------------------------------------
// Event Names                                                                              
//------------------------------------------------------------------------------------------

//static const char* SETUP_LUAFUNC_NAME = "Setup";                                          // Set up the callbacks.

// GCC says this is an unused static variable - which we can't have cos it generates a warning.
//static const char* ON_READYFORFORMATIONATTACK_LUAFUNC_NAME = "OnReadyForFormationAttack"; // Trigger a formation attack.



//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIFormation::AIFormation                                                                
//! Construction                                                                            
//!                                                                                         
//------------------------------------------------------------------------------------------
AIFormation::AIFormation() : m_pOwner(0), m_ptOffset(CONSTRUCT_CLEAR)
{
	ATTACH_LUA_INTERFACE(AIFormation);

	m_bActive = false;
	m_bIsXMLConstructed = false;
	m_pTargetEntity = 0;   
	m_pLockonEntity = 0;   
	m_iEntsInFormation = 0;
	m_iSlotsInFormation = 0; 
	m_pSlots = 0;
	m_iSlotsInFormation = 0;
	m_ptOffset = CPoint(0.0f, 0.0f, 0.0f); 
	m_fConstraintAngle = USE_DEFAULT;
	m_bAggressive = false;
	m_fPriority = 1.0f;
	m_fIdleAnimSpeed = 1.5f;
	m_bOverlapChecked = false;
	m_bCameraRelative = false;
	m_bClosestSlot = false;

	// Set the default formation idle anim
	m_IdleAnim = "formationclose_inc_strafe";

	
}
//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIFormation::~AIFormation
//! Destructor
//!                                                                                         
//------------------------------------------------------------------------------------------
AIFormation::~AIFormation()
{
	// Formation must be clear of entities before destruction
	ntAssert(m_iEntsInFormation == 0);

//	ClearAttackPatterns();
	ClearSlots(true);
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIFormation::PostXmlConstruct
//! 
//!                                                                                         
//------------------------------------------------------------------------------------------
void AIFormation::PostXmlConstruct()
{
	// Set the name of the formation
	DataObject* pDataObject = ObjectDatabase::Get().GetDataObjectFromPointer(this);
	m_Name = ntStr::GetString(pDataObject->GetName());
}

void AIFormation::SetActive(bool bActive, FormationComponent* pFormationComponent)
{
	m_bActive = bActive;

	pFormationComponent->ResetOverlapCheckedStatus();
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIFormation::ClearSlots
//! Empty out the slot list
//!                                                                                         
//------------------------------------------------------------------------------------------
void AIFormation::ClearSlots(bool bDestructing)
{
	UNUSED(bDestructing);

	if (m_pSlots)
	{
		for (int i = 0; i < m_iSlotsInFormation; i++)
		{
			if (m_pSlots[i])
			{
				if (m_pSlots[i]->GetEntity())
				{
					RemoveEntity(m_pSlots[i]->GetEntity());
				}
				
				if (m_pSlots[i]->CanDelete())
				{
					m_pSlots[i]->m_pFormation = 0;
					NT_DELETE_CHUNK(Mem::MC_AI, m_pSlots[i]);
				}
			}
		}

		NT_DELETE_ARRAY_CHUNK(Mem::MC_AI, m_pSlots);
		m_pSlots = 0;
		m_iSlotsInFormation = 0;
		m_iEntsInFormation = 0;
	}
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIFormation::GetEntsReadyForAttack                                                      
//! Fill the m_entsReadyForAttack and return the number of entities ready to attack.        
//!                                                                                         
//------------------------------------------------------------------------------------------

int AIFormation::GetEntsReadyForAttack(ntstd::List<AI*>& rEntityList) const
{
	int Count = 0;

	for (int i = 0; i < m_iSlotsInFormation; i++)
	{
		AI* pEnt = m_pSlots[i]->GetEntity();

		if (!pEnt || pEnt->ToCharacter()->IsDead())
			continue;

		// Don't allow entities already attacking
		if (pEnt->GetAIComponent()->GetAIFormationComponent()->GetFormationAttack())
			continue;

		// Passed all the checks.. add the entity to the list. 
		rEntityList.push_back(pEnt);
		++Count;
	}

	return Count;
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIFormation::GetTarget                                                                  
//! Get the target point for the entire formation.                                          
//!                                                                                         
//------------------------------------------------------------------------------------------
const CPoint AIFormation::GetTarget() const
{
	CPoint TargetPoint(m_ptOffset);

	if (m_pTargetEntity)
	{
		if (m_fConstraintAngle != USE_DEFAULT && !m_bCameraRelative)
		{
			float fSinStore, fCosStore;

			CMaths::SinCos(m_fConstraintAngle * DEG_TO_RAD_VALUE, fSinStore, fCosStore);

			CDirection DirectionBaseAngle(fSinStore, 0.0f, fCosStore);

			DirectionBaseAngle.Normalise();

			CDirection DirectionToTargetFromOffset((CDirection)m_pTargetEntity->GetPosition() - (CDirection)m_ptOffset);

			CDirection DirectionToTargetFromOffsetNormalised = DirectionToTargetFromOffset;

			DirectionToTargetFromOffsetNormalised.Normalise();

			float fDot = DirectionBaseAngle.Dot(DirectionToTargetFromOffsetNormalised);

			CPoint ConstrainedTarget(DirectionBaseAngle * (DirectionToTargetFromOffset.Length() * fDot));

			TargetPoint += ConstrainedTarget;
		}
		else
		{
			TargetPoint += m_pTargetEntity->GetPosition();
		}
	}

	return TargetPoint;
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIFormation::GetLockonTarget                                                            
//! Get the lockon target for the formation.                                                
//!                                                                                         
//------------------------------------------------------------------------------------------
const CPoint AIFormation::GetLockonTarget() const
{
	if (m_pLockonEntity)
		return m_pLockonEntity->GetPosition();
	else
		return m_ptOffset;
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIFormation::FindSlot                                                                   
//! Find the slot associated with a given entity.                                           
//!                                                                                         
//------------------------------------------------------------------------------------------
AIFormationSlot* AIFormation::FindSlot(const AI& entity)
{
	for (int i = 0; i < m_iSlotsInFormation; i++)
	{
		if (m_pSlots[i]->GetEntity() == &entity)
			return m_pSlots[i];
	}
	
	return 0;
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIFormation::GetEntity                                                                   
//! Return the entity for a given index
//!                                                                                         
//------------------------------------------------------------------------------------------
AI* AIFormation::GetEntity(int Index)
{
	for (int i = 0; i < m_iSlotsInFormation; i++)
	{
		if (m_pSlots[i]->GetEntity())
		{
			if (--Index < 0)
				return m_pSlots[i]->GetEntity();
		}
	}
	
	return NULL;
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIFormation::RemoveEntity
//! Remove this entity from the formation.                                                  
//!                                                                                         
//------------------------------------------------------------------------------------------
bool AIFormation::RemoveEntity(AI* pEntity)
{
	// Sanity checks. 
	if (!pEntity 
		|| !pEntity->GetAIComponent() 
		|| !pEntity->GetAIComponent()->GetAIFormationComponent()
		|| !pEntity->GetAIComponent()->GetAIFormationComponent()->GetFormation()
		|| pEntity->GetAIComponent()->GetAIFormationComponent()->GetFormation() != this)
	{
#ifndef _RELEASE
		ntPrintf("failed (not in formation %p)", this );
#endif 
		return false;
	}

	// Find the slot for the entity
	AIFormationSlot* pSlot = FindSlot(*pEntity);

	// Empty the slot if it's valid.
	if (pSlot)
	{
		UnassignSlot(*pSlot);
	}

	// Cache the formation component for the entity
	AIFormationComponent* pEntFormComp = pEntity->GetAIComponent()->GetAIFormationComponent();

	// Is the entity currently in an attack
	if ( pEntFormComp->GetFormationAttack() )
	{
		pEntFormComp->GetFormationAttack()->RemoveEntity(pEntity);
	}

	// Before removing the entity from the squad, get the squad name.
	const ntstd::String& rSquadName = pEntFormComp->GetSquadName();

	/// If there arn't any more entities in the formation, send a messaage. 
	if (!m_iEntsInFormation && m_pOwner && m_pOwner->GetMessageHandler())
	{
		m_pOwner->GetMessageHandler()->Receive(CMessageHandler::Make(m_pOwner, "msg_formation_no_more_entities", m_Name.c_str(), rSquadName.c_str()));
	}

	// Clear out the squad name for entity
	pEntFormComp->NullSquadName();

	return true;
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIFormation::FindSlot                                                                   
//! Find the slot associated with a given entity.                                           
//!                                                                                         
//------------------------------------------------------------------------------------------
const AIFormationSlot* AIFormation::FindSlot(const AI& entity) const
{
	for (int i = 0; i < m_iSlotsInFormation; i++)
	{
		if (m_pSlots[i]->GetEntity() == &entity)
			return m_pSlots[i];
	}
	
	return 0;
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIFormation::HealthChanged
//! 
//!                                                                                         
//------------------------------------------------------------------------------------------

void AIFormation::HealthChanged(CEntity* pEnt, float fNewHealth, float fBaseHealth)
{
	// Find the nearest commander entity to the player
	const CEntity* pCmdEnt = AIFormationManager::Get().GetCommander( 0 );

	// If there is a commander entity
	if( pCmdEnt )
	{
		// Send a health changed message to the formation commander. 
		pCmdEnt->GetMessageHandler()->Receive( CMessageHandler::Make(	pEnt, 
																		"msg_health_changed", 
																		fNewHealth / fBaseHealth, 
																		fBaseHealth) );
	}
	
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIFormation::RemoveSlot
//! AIFormationSlot& slot
//!                                                                                         
//------------------------------------------------------------------------------------------

void AIFormation::RemoveSlot(AIFormationSlot& slot)
{
	// Make sure that the slot belongs to the formation.
	int Index = -1;

	for (int Count = 0; Count < m_iSlotsInFormation; ++Count)
	{
		if (m_pSlots[Count] == &slot)
		{
			Index = Count;
			break;
		}
	}

	// Nope - slot not ours.
	if (Index < 0)
		return;

	// Remove reference to the slot. 
	m_pSlots[Index] = 0;

	// Sanity check
	ntAssert( !slot.GetEntity() && "Removing a slot whilst there is still an entity? Bad order of processing?" );
	
	// Make sure that no entity is assigned to the slot
	UnassignSlot(slot);

	// Clear the formation the slot is assigned to. 
	slot.m_pFormation = 0;
}

//------------------------------------------------------------------------------------------
//!  public  SwitchSlotsToReduceTravel
//!
//!
//!  @author GavB @date 14/12/2006
//------------------------------------------------------------------------------------------
void AIFormation::SwitchSlotsToReduceTravel()
{
	// No point doing any swaps if there's only 1 entity in the formation!
	if (m_iSlotsInFormation < 2 || m_iEntsInFormation < 2) 
		return;

	AI* pEnts[2];
	AIFormationSlot* pSlot[2];
	float fDistanceToTarget[2] = { 0.0f, 0.0f };
	float fDistanceToOtherTarget[2] = { 0.0f, 0.0f };
	CPoint EntityPosition[2];
	CPoint TargetPosition[2];

	for (int iThisSlot = 0; iThisSlot < m_iSlotsInFormation; iThisSlot++)
	{
		if (m_pSlots[iThisSlot]->GetEntity())
		{
			pSlot[0] = m_pSlots[iThisSlot];
			pEnts[0] = pSlot[0]->GetEntity();
			EntityPosition[0] = pEnts[0]->GetPosition();
			TargetPosition[0] = pSlot[0]->GetWorldPoint();

			for (int iOtherSlot = 0; iOtherSlot < m_iSlotsInFormation; iOtherSlot++)
			{
				if (m_pSlots[iOtherSlot]->GetEntity())
				{
					// Avoid duplications, i.e check entity 2 against 4 but not 4 against 2. Also, obviously, don't check 3 against 3.
					if (iOtherSlot >= iThisSlot)
						break;

					pSlot[1] = m_pSlots[iOtherSlot];
					pEnts[1] = pSlot[1]->GetEntity();
					EntityPosition[1] = pEnts[1]->GetPosition();
					TargetPosition[1] = pSlot[1]->GetWorldPoint();

					// See if their swapping their target slots will reduce the biggest distance.
					for (int i = 0; i < 2; i++)
					{
						fDistanceToTarget[i] = (EntityPosition[i] - TargetPosition[i]).LengthSquared();
						fDistanceToOtherTarget[i] = (EntityPosition[i] - TargetPosition[1-i]).LengthSquared();
					}

					float fBiggestDistanceToTarget = fDistanceToTarget[0] > fDistanceToTarget[1] ? fDistanceToTarget[0] : fDistanceToTarget[1];
					float fBiggestDistanceToOtherTarget = fDistanceToOtherTarget[0] > fDistanceToOtherTarget[1] ? fDistanceToOtherTarget[0] : fDistanceToOtherTarget[1];

					// If so swap them.
					if (fBiggestDistanceToOtherTarget < fBiggestDistanceToTarget)
					{
						// Exchange slots.

						pSlot[0]->SetEntity(pEnts[1]);
						pSlot[1]->SetEntity(pEnts[0]);

						pSlot[0]->SetInPosition(false);
						pSlot[1]->SetInPosition(false);

						// We have to break out now, since the pEnts[] are now rubbished through the above swap.
						break;
					}
				}
			}
		}
	}
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIFormation::AllocateSlots
//! Create memory to hold formation slots.
//!                                                                                         
//------------------------------------------------------------------------------------------
void AIFormation::AllocateSlots()
{
	ClearSlots();

	// Allocate the slot array...
	m_pSlots = NT_NEW_ARRAY_CHUNK(Mem::MC_AI) AIFormationSlot*[m_iSlotsInFormation];

	// ...and fill it.
	for (int i = 0; i < m_iSlotsInFormation; i++)
	{
		m_pSlots[i] = NT_NEW_CHUNK(Mem::MC_AI) AIFormationSlot(this);
	}
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIFormation::UnassignSlot                                                           
//! Remove the entity from this slot.                                                       
//!                                                                                         
//------------------------------------------------------------------------------------------
void AIFormation::UnassignSlot(AIFormationSlot& rSlot)
{
	if (rSlot.GetEntity())
	{
		rSlot.GetEntity()->GetAIComponent()->GetAIFormationComponent()->SetFormation(0);
		rSlot.SetEntity(0);
		m_iEntsInFormation--;
	}
}

//------------------------------------------------------------------------------------------
//!  private  AssignSlot
//!
//!  @param [in]       bTestOnly bool     <TODO: insert parameter description here>
//!
//!  @return AIFormationSlot *  Pointer to a valid slot
//!
//!  @author Nick
//------------------------------------------------------------------------------------------
AIFormationSlot* AIFormation::AssignSlot(AI* pEntity, bool bTestOnly)
{
	// Do we have space for another entity in this formation
	if (!pEntity || !IsActive() || !IsSlotAvailable() || pEntity->ToCharacter()->IsDead())
		return 0;

	AIFormationSlot* pBestSlot = 0;

	CPoint EntityPoint = pEntity->GetPosition();

	// Find nearest slot.
	if (m_bClosestSlot)
	{
		float fBestDistanceSquared = 0.0f;

		for (int iSlot = 0; iSlot < m_iSlotsInFormation; iSlot++)
		{
			AIFormationSlot* pSlot = m_pSlots[iSlot];

			// If valid and free.
			if (pSlot->IsValid() && !pSlot->GetEntity())
			{
				float fDistanceSquared = (pSlot->GetWorldPoint() - EntityPoint).LengthSquared();

				if (!pBestSlot || fDistanceSquared < fBestDistanceSquared)
				{
					fBestDistanceSquared = fDistanceSquared;
					pBestSlot = pSlot;
				}
			}
		}
	}
	// Find slot based on slot order
	else
	{
		for (int iSlot = 0; iSlot < m_iSlotsInFormation; iSlot++)
		{
			AIFormationSlot* pSlot = m_pSlots[iSlot];

			// If valid and free.
			if (pSlot->IsValid() && !pSlot->GetEntity())
			{
				pBestSlot = pSlot;
				break;
			}
		}
	}

	// If we're only testing for a slot, return that slot. 
	if ( bTestOnly )
		return pBestSlot;

	// If there is a great slot - then use that. 
	if (pBestSlot)
	{
		pBestSlot->SetEntity(pEntity);
		++m_iEntsInFormation;		
		pEntity->GetAIComponent()->GetAIFormationComponent()->SetFormation(this);
	}

	return pBestSlot;
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIFormation_Line::Update
//! Maintain the entities in a line
//!                                                                                         
//------------------------------------------------------------------------------------------
void AIFormation::Update(float fTimeDelta, unsigned int uiFormationColour, FormationComponent* pFormationComponent)
{
	UNUSED(fTimeDelta);
#ifdef _RELEASE
	UNUSED(uiFormationColour);
	UNUSED(pFormationComponent);
#endif
	CGatso::Start("AIFormation::Update()");

	// Force validation of ALL slots!
	for (int iSlot = 0; iSlot < m_iSlotsInFormation; iSlot++)
	{
		AIFormationSlot* pSlot = m_pSlots[iSlot];

		// Check the to see if slot has clear line of sight from target.
		pSlot->SetNavigable(CAINavigationSystemMan::Get().IsPosValidForFormation(GetLockonTarget(), pSlot->GetWorldPoint(), SLOT_RADIUS));
	}

	// Make sure space is not already occupied by another slot in a higher priority formation.
	pFormationComponent->UpdateClearOfOtherSlots(this, SLOT_RADIUS * 2.0f);

	m_bOverlapChecked = true;

	// Reassign entities to slots where required.
	for (int iSlot = 0; iSlot < m_iSlotsInFormation; iSlot++)
	{
		AIFormationSlot* pSlot = m_pSlots[iSlot];
		AI* pEntity = pSlot->GetEntity();

		// If our slot has been deactivated then find entity another one.
		if (pEntity)
		{
			UnassignSlot(*pSlot);

			if ( (pSlot == AssignSlot(pEntity)) )
			{
				bool bInPosition = pSlot->IsInPosition() && pSlot->IsValid();
				pSlot->SetInPosition( bInPosition );
			}
			else
			{
				// Couldn't find a slot for the entity - so remove it completly from the formation.
				pEntity->GetAIComponent()->GetAIFormationComponent()->SetFormation( this );
				pFormationComponent->RemoveEntity( pEntity );
			}
		}
	}

	SwitchSlotsToReduceTravel();

	for (int iSlot = 0; iSlot < m_iSlotsInFormation; iSlot++)
	{
		AIFormationSlot* pSlot = m_pSlots[iSlot];

		if (pSlot->GetEntity())
			pSlot->UpdateInRange();
	}

	CGatso::Stop("AIFormation::Update()");

	// Render debug information.
#ifndef _RELEASE
	if (AIFormationManager::DisplayDebugInfo())
	{
		RenderDebugInfo(uiFormationColour);

		for (int iSlot = 0; iSlot < m_iSlotsInFormation; iSlot++)
		{
			AIFormationSlot* pSlot = m_pSlots[iSlot];

			// Render the debugging information for the slot.
			pSlot->RenderDebugInfo(uiFormationColour);
		}
	}

	int iEntCount = 0;
	for (int i = 0; i < m_iSlotsInFormation; i++)
	{
		if (m_pSlots[i]->GetEntity())
			++iEntCount;
	}

	ntAssert(iEntCount == m_iEntsInFormation);
#endif
}

#ifndef _RELEASE
void AIFormation::RenderDebugInfo(unsigned int uiFormationColour)
{
	CMatrix ArcMatrix(CONSTRUCT_IDENTITY);
	ArcMatrix.SetTranslation(GetTarget() + DBG_Y_OFFSET);

	g_VisualDebug->RenderArc(ArcMatrix, GetExitDistance(), TWO_PI,  uiFormationColour);
	g_VisualDebug->RenderArc(ArcMatrix, GetEntryDistance(), TWO_PI,  uiFormationColour);

	CPoint TargetPoint(GetTarget() + DBG_Y_OFFSET);

	RenderPlus(TargetPoint, 0.5f, uiFormationColour);

	if (m_pTargetEntity && m_fConstraintAngle != USE_DEFAULT && !m_bCameraRelative)
	{
		float fSinStore, fCosStore;

		CMaths::SinCos(m_fConstraintAngle * DEG_TO_RAD_VALUE, fSinStore, fCosStore);

		CPoint Offset = CPoint(fSinStore * GetExitDistance(), 0.0f, fCosStore * GetExitDistance());

		g_VisualDebug->RenderLine(TargetPoint + Offset, TargetPoint - Offset, uiFormationColour);
	}
}

void AIFormation::RenderPlus(CPoint const& Point, float fSize, unsigned int uiFormationColour)
{
	g_VisualDebug->RenderLine(Point + CPoint(0.0f, 0.0f, fSize), Point + CPoint(0.0f, 0.0f, -fSize), uiFormationColour);
	g_VisualDebug->RenderLine(Point + CPoint(-fSize, 0.0f, 0.0f), Point + CPoint(fSize, 0.0f, 0.0f), uiFormationColour);
}

#endif
