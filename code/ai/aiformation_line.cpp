//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file aiformation_line.cpp                                                               
//!                                                                                         
//------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
// Includes                                                                                 
//------------------------------------------------------------------------------------------
#include "ai/aiformation_line.h"
#include "ai/ainavgraphmanager.h"
#include "ai/airepulsion.h"
#include "ai/aiformationmanager.h"
#include "ai/aiformationattack.h"
#include "ai/aiformationcomponent.h"
#include "camera/camman.h"
#include "camera/camview.h"
#include "core/timer.h"
#include "core/gatso.h"
#include "game/aicomponent.h"
#include "game/attacks.h"
#include "game/entityinfo.h"
#include "game/messagehandler.h"
#include "objectdatabase/dataobject.h"
#include "ai/ainavigationsystem/ainavigsystemman.h"
#include "core/visualdebugger.h"

//------------------------------------------------------------------------------------------
// Constants                                                                                
//------------------------------------------------------------------------------------------
static const float DEFAULT_BASE_ANGLE             = 0.0f;
static const float DEFAULT_DISTANCE               = 3.0f;
static const float DEFAULT_DISTANCE_SPACING		  = 1.0f;
static const float DEFAULT_SPACING				  = 2.0f;
static const  int  DEFAULT_SLOTCOUNT              = 10;

//------------------------------------------------------------------------------------------
// Declare editable interface for AIFormation
//------------------------------------------------------------------------------------------

START_CHUNKED_INTERFACE(AIFormation_Line, Mem::MC_AI)
	DEFINE_INTERFACE_INHERITANCE(AIFormation)
	COPY_INTERFACE_FROM(AIFormation)
	PUBLISH_PTR_CONTAINER_AS(m_aXMLRanks, Ranks)
	DECLARE_POSTCONSTRUCT_CALLBACK(PostXmlConstruct)
END_STD_INTERFACE


//------------------------------------------------------------------------------------------
// Declare editable interface for AILineRank_Formation
//------------------------------------------------------------------------------------------
START_CHUNKED_INTERFACE(AILineRank_Formation, Mem::MC_AI)
	PUBLISH_VAR_WITH_DEFAULT_AS(fDistance, USE_DEFAULT, Distance)
	PUBLISH_VAR_WITH_DEFAULT_AS(fSpacing, USE_DEFAULT, Spacing)			
	PUBLISH_VAR_WITH_DEFAULT_AS(fBaseAngle, USE_DEFAULT, BaseAngle)			
	PUBLISH_VAR_WITH_DEFAULT_AS(iSlotsMax, DEFAULT_SLOTCOUNT, SlotCount)			
END_STD_INTERFACE

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIFormation_Line::PostXmlConstruct
//! 
//!                                                                                         
//------------------------------------------------------------------------------------------
void AIFormation_Line::PostXmlConstruct()
{
	// Call the parent construction too.
	AIFormation::PostXmlConstruct();

	m_aLUARanks = 0;

	m_iRanks = m_aXMLRanks.size();

	ValidateData();
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIFormation_Line::AIFormation_Line		                                            
//! Construct a circular formation from a lua definition.                                   
//!                                                                                         
//------------------------------------------------------------------------------------------
AIFormation_Line::AIFormation_Line(const NinjaLua::LuaObject& def)
{
	// Formation shape attributes
	ntAssert(def[DEF_RANKS].IsTable());
	NinjaLua::LuaObject lua_ranks = def[DEF_RANKS];

	// Count the number of ranks
	for (m_iRanks = 0; lua_ranks[m_iRanks+1].IsTable(); m_iRanks++)
		;

	// Create the ranks and set attributes
	m_aLUARanks = NT_NEW_ARRAY_CHUNK(Mem::MC_AI) AILineRank_Formation[m_iRanks];

	AILineRank_Formation *pRank = &GetRank(0);
	for (int iRank = 1; lua_ranks[iRank].IsTable(); iRank++, pRank++)
	{
		NinjaLua::LuaObject lua_rank = lua_ranks[iRank];

		// Radius of this rank
		if (lua_rank[DEF_DISTANCE].IsNumber())
			pRank->fDistance = lua_rank[DEF_DISTANCE].GetFloat();
		else
			pRank->fDistance = USE_DEFAULT;

		// Set the compression
		if (lua_rank[DEF_SPACING].IsNumber())
			pRank->fSpacing = lua_rank[DEF_SPACING].GetFloat();
		else
			pRank->fSpacing = USE_DEFAULT;

		// Set the base angle (radians)
		if (lua_rank[DEF_BASE_ANGLE].IsNumber())
			pRank->fBaseAngle = lua_rank[DEF_BASE_ANGLE].GetFloat() * RAD_TO_DEG_VALUE;
		// Set the base angle (degrees)
		else if (lua_rank[DEF_ANGLE].IsNumber())
			pRank->fBaseAngle = lua_rank[DEF_ANGLE].GetFloat();
		else
			pRank->fBaseAngle = USE_DEFAULT;

		// Slot count for this rank
		if (lua_rank[DEF_SLOTCOUNT].IsNumber())
			pRank->iSlotsMax = lua_rank[DEF_SLOTCOUNT].GetInteger();
		else
			pRank->iSlotsMax = DEFAULT_SLOTCOUNT;
	}

	ValidateData();
}

void AIFormation_Line::ValidateData()
{
	m_iSlotsInFormation = 0;
	
	m_fEntryDistance = 0.0f;

	for (int iRank = 0; iRank < GetRankCount(); iRank++)
	{
		AILineRank_Formation* pRank = &GetRank(iRank);

		// Radius of this rank
		if (pRank->fDistance == USE_DEFAULT)
			pRank->fDistance = DEFAULT_DISTANCE + (DEFAULT_DISTANCE_SPACING * iRank);

		// Sensible defaults: no value for inner rank, default to 0; no value for subsequent ranks, default to inner rank value.
		if (pRank->fSpacing == USE_DEFAULT)
		{
			if (iRank == 0)
			{
				pRank->fSpacing = DEFAULT_SPACING;
			}
			else
			{
				pRank->fSpacing = GetRank(0).fSpacing;
			}
		}

		// Sensible defaults: no value for inner rank, default to 0; no value for subsequent ranks, default to inner rank value.
		if (pRank->fBaseAngle == USE_DEFAULT)
		{
			if (iRank == 0)
			{
				pRank->fBaseAngle = DEFAULT_BASE_ANGLE;
			}
			else
			{
				pRank->fBaseAngle = GetRank(0).fBaseAngle;
			}
		}

		// Looking for furthest point from centre of formation.
		float fDistanceToTarget = sqr(pRank->fDistance) + sqr(pRank->iSlotsMax / 2.0f * pRank->fSpacing);

		if (fDistanceToTarget > m_fEntryDistance)
		{
			m_fEntryDistance = fDistanceToTarget;
		}

		// Check this rank is ok
		ntAssert(pRank->fDistance > 0.0f);
		ntAssert(pRank->iSlotsMax > 0);

		// Store the slot offset for this rank and increment the total slot count
		pRank->iSlotOffset = m_iSlotsInFormation;
		m_iSlotsInFormation += pRank->iSlotsMax;
	}

	// Entry distance is a bit bigger than outermost point.
	m_fEntryDistance = sqrtf(m_fEntryDistance) + ENTRY_DISTANCE_THRESHOLD;

	// Exit distance is a bit bigger than entry distance.
	m_fExitDistance= m_fEntryDistance + EXIT_DISTANCE_THRESHOLD;

	// Check everything's ok.
	ntAssert(m_iSlotsInFormation > 0);
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIFormation_Line::CalculateSlotPositions                                                        
//! Calculate the positions of all the slots in a rank of the formation.                    
//!                                                                                         
//------------------------------------------------------------------------------------------
void AIFormation_Line::CalculateSlotPositions()
{
	for (int iRank = 0; iRank < GetRankCount(); iRank++)
	{
		AILineRank_Formation* pRank = &GetRank(iRank);
		float fDistance = pRank->fDistance;

		if (AIFormationManager::Get().m_bPlayerPlayingDervish)
			fDistance = 2.5f;

		// Calculate the spacing for this rank

		float fAngle = pRank->fBaseAngle * DEG_TO_RAD_VALUE;

		if (m_bCameraRelative)
		{
			const CamView* pView = CamMan::GetPrimaryView();
			CDirection ZView = pView->GetCurrMatrix().GetZAxis();
			fAngle += atan2f(ZView.X(), ZView.Z());
		}

		CDirection FirstSlotPosition(0.0f, 0.0f, 0.0f);
		CDirection FirstSlotPositionNormalised(0.0f, 0.0f, 0.0f);
		float fSpacing = 0.0f;

		for (int iSlot = 0; iSlot < pRank->iSlotsMax; iSlot++)
		{
			AIFormationSlot* pSlot = GetSlot(iRank, iSlot);
			ntAssert(pSlot);

			CDirection SlotPosition;

			if (iSlot == 0)
			{
				float fSinStore, fCosStore;
				CMaths::SinCos(fAngle, fSinStore, fCosStore);
				FirstSlotPositionNormalised = FirstSlotPosition = CDirection(fSinStore*fDistance, 0.0f, fCosStore*fDistance);
				FirstSlotPositionNormalised.Normalise();
				SlotPosition.Clear();
			}
			else
			{
				// Is the slot on the left or right side of the formation
				if ((iSlot & 1) == 1)
				{
					fSpacing += pRank->fSpacing;
					SlotPosition = CDirection(FirstSlotPositionNormalised.Z(), FirstSlotPositionNormalised.Y(), -FirstSlotPositionNormalised.X());
				}
				else
				{
					SlotPosition = CDirection(-FirstSlotPositionNormalised.Z(), FirstSlotPositionNormalised.Y(), FirstSlotPositionNormalised.X());
				}

				SlotPosition *= fSpacing;
			}

			pSlot->SetLocalPoint(CPoint(FirstSlotPosition + SlotPosition));
		}
	}
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIFormation_Line::GetSlot                                                           
//! Get the slot in a given rank and index.                                                 
//!                                                                                         
//------------------------------------------------------------------------------------------
inline AIFormationSlot* AIFormation_Line::GetSlot(int iRank, int iSlot)
{
	ntAssert(iRank >= 0 && iRank < GetRankCount());
	ntAssert(iSlot >= 0 && iSlot < GetRank(iRank).iSlotsMax);

	return m_pSlots[GetRank(iRank).iSlotOffset + iSlot];
}
