//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file aiformation_circle.cpp                                                               
//!                                                                                         
//------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
// Includes                                                                                 
//------------------------------------------------------------------------------------------
#include "ai/aiformation_circle.h"
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
static const float DEFAULT_RADIUS                 = 3.0f;
static const float DEFAULT_COMPRESSION            = 1.0f;
static const float DEFAULT_RANKSPACING			  = 1.0f;
static const  int  DEFAULT_SLOTCOUNT              = 10;

//------------------------------------------------------------------------------------------
// Declare editable interface for AIFormation
//------------------------------------------------------------------------------------------

START_CHUNKED_INTERFACE(AIFormation_Circle, Mem::MC_AI)
	DEFINE_INTERFACE_INHERITANCE(AIFormation)
	COPY_INTERFACE_FROM(AIFormation)
	PUBLISH_PTR_CONTAINER_AS(m_aXMLRanks, Ranks)
	DECLARE_POSTCONSTRUCT_CALLBACK(PostXmlConstruct)
END_STD_INTERFACE


//------------------------------------------------------------------------------------------
// Declare editable interface for AICircleRank_Formation
//------------------------------------------------------------------------------------------
START_CHUNKED_INTERFACE(AICircleRank_Formation, Mem::MC_AI)
	PUBLISH_VAR_WITH_DEFAULT_AS(fRadius, USE_DEFAULT, Radius)
	PUBLISH_VAR_WITH_DEFAULT_AS(fCompression, DEFAULT_COMPRESSION, Compression)
	PUBLISH_VAR_WITH_DEFAULT_AS(fBaseAngle, USE_DEFAULT, BaseAngle)
	PUBLISH_VAR_WITH_DEFAULT_AS(iSlotsMax, DEFAULT_SLOTCOUNT, SlotCount)
END_STD_INTERFACE


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIFormation_Circle::PostXmlConstruct
//! 
//!                                                                                         
//------------------------------------------------------------------------------------------
void AIFormation_Circle::PostXmlConstruct()
{
	// Call the parent construction too.
	AIFormation::PostXmlConstruct();

	m_aLUARanks = 0;

	m_iRanks = m_aXMLRanks.size();

	ValidateData();
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIFormation_Circle::AIFormation_Circle		                                            
//! Construct a circular formation from a lua definition.                                   
//!                                                                                         
//------------------------------------------------------------------------------------------
AIFormation_Circle::AIFormation_Circle(const NinjaLua::LuaObject& def)
{
	// Formation shape attributes
	ntAssert(def[DEF_RANKS].IsTable());
	NinjaLua::LuaObject lua_ranks = def[DEF_RANKS];

	// Count the number of ranks
	for (m_iRanks = 0; lua_ranks[m_iRanks+1].IsTable(); m_iRanks++)
		;

	// Create the ranks and set attributes
	m_aLUARanks = NT_NEW_ARRAY_CHUNK(Mem::MC_AI) AICircleRank_Formation[m_iRanks];

	AICircleRank_Formation *pRank = &GetRank(0);
	for (int iRank = 1; lua_ranks[iRank].IsTable(); iRank++, pRank++)
	{
		NinjaLua::LuaObject lua_rank = lua_ranks[iRank];

		// Radius of this rank
		if (lua_rank[DEF_RADIUS].IsNumber())
			pRank->fRadius = lua_rank[DEF_RADIUS].GetFloat();
		else
			pRank->fRadius = USE_DEFAULT;

		// Set the compression
		if (lua_rank[DEF_COMPRESS].IsNumber())
			pRank->fCompression = lua_rank[DEF_COMPRESS].GetFloat();
		else
			pRank->fCompression = DEFAULT_COMPRESSION;

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

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIFormation_Circle::AIFormation_Circle		                                            
//! Construct a circular formation from a lua definition.                                   
//!                                                                                         
//------------------------------------------------------------------------------------------
void AIFormation_Circle::ValidateData()
{
	m_iSlotsInFormation = 0;

	for (int iRank = 0; iRank < GetRankCount(); iRank++)
	{
		AICircleRank_Formation* pRank = &GetRank(iRank);

		// Radius of this rank
		if (pRank->fRadius == USE_DEFAULT)
			pRank->fRadius = DEFAULT_RADIUS + (DEFAULT_RANKSPACING * iRank);

		if (iRank == 0 || m_fEntryDistance < pRank->fRadius)
			m_fEntryDistance = pRank->fRadius;

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

		// Check this rank is ok
		ntAssert(pRank->fRadius > 0.0f);
		ntAssert(pRank->iSlotsMax > 0);

		// Store the slot offset for this rank and increment the total slot count
		pRank->iSlotOffset = m_iSlotsInFormation;
		m_iSlotsInFormation += pRank->iSlotsMax;
	}

	// Entry distance is a bit bigger than outer rank radius.
	m_fEntryDistance += ENTRY_DISTANCE_THRESHOLD;

	// Exit distance is a bit bigger than entry distance.
	m_fExitDistance = m_fEntryDistance + EXIT_DISTANCE_THRESHOLD;

	ntAssert(m_iSlotsInFormation > 0);
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//! Calculate the positions of all the slots in a rank of the formation.                    
//! 
//! Slots start at the base angle and spread out alternatively left and right of the first
//! slot. This ensures that the formation is always filled with entities where it should be.
//! 
//------------------------------------------------------------------------------------------
void AIFormation_Circle::CalculateSlotPositions()
{
	for (int iRank = 0; iRank < GetRankCount(); iRank++)
	{
		AICircleRank_Formation* pRank = &GetRank(iRank);
		float fRadius = pRank->fRadius;

		if (AIFormationManager::Get().m_bPlayerPlayingDervish)
			fRadius = 2.5f;

		float fAngle = 0.0f;
		float fBaseAngle = pRank->fBaseAngle * DEG_TO_RAD_VALUE;

		if (m_bCameraRelative)
		{
			const CamView* pView = CamMan::GetPrimaryView();
			CDirection ZView = pView->GetCurrMatrix().GetZAxis();
			fBaseAngle += atan2f(ZView.X(), ZView.Z());
		}

		float fSpacingAngle = (TWO_PI / (float)pRank->iSlotsMax) * pRank->fCompression;

		for (int iSlot = 0; iSlot < pRank->iSlotsMax; iSlot++)
		{
			AIFormationSlot* pSlot = GetSlot(iRank, iSlot);
			ntAssert(pSlot);

			float fSinStore, fCosStore;

			if ((iSlot & 1) == 1)
			{
				fAngle += fSpacingAngle;
				CMaths::SinCos(fBaseAngle + fAngle, fSinStore, fCosStore);
			}
			else
			{
				CMaths::SinCos(fBaseAngle - fAngle, fSinStore, fCosStore);
			}

			pSlot->SetLocalPoint(CPoint(fSinStore*fRadius, 0.0f, fCosStore*fRadius));
		}
	}
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIFormation_Circle::GetSlot                                                           
//! Get the slot in a given rank and index.                                                 
//!                                                                                         
//------------------------------------------------------------------------------------------
inline AIFormationSlot* AIFormation_Circle::GetSlot(int iRank, int iSlot)
{
	ntAssert(iRank >= 0 && iRank < GetRankCount());
	ntAssert(iSlot >= 0 && iSlot < GetRank(iRank).iSlotsMax);

	return m_pSlots[GetRank(iRank).iSlotOffset + iSlot];
}
