//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file aiformation_mapped.cpp                                                               
//!                                                                                         
//------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
// Includes                                                                                 
//------------------------------------------------------------------------------------------
#include "ai/aiformation_mapped.h"
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
#include "game/luahelp.h"

//------------------------------------------------------------------------------------------
// Declare editable interface for AIFormation
//------------------------------------------------------------------------------------------

START_CHUNKED_INTERFACE(AIFormation_Mapped, Mem::MC_AI)
	DEFINE_INTERFACE_INHERITANCE(AIFormation)
	COPY_INTERFACE_FROM(AIFormation)
	PUBLISH_CONTAINER_AS(m_aXMLPoints, Places)
	DECLARE_POSTCONSTRUCT_CALLBACK(PostXmlConstruct)
END_STD_INTERFACE

///------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIFormation_Mapped::PostXmlConstruct
//! 
//!                                                                                         
//------------------------------------------------------------------------------------------
void AIFormation_Mapped::PostXmlConstruct()
{
	// Call the parent construction too.
	AIFormation::PostXmlConstruct();

	m_aLUAPoints = 0;

	m_iPoints = m_aXMLPoints.size();

	ValidateData();
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIFormation_Mapped::AIFormation_Mapped		                                            
//! Construct a circular formation from a lua definition.                                   
//!                                                                                         
//------------------------------------------------------------------------------------------
AIFormation_Mapped::AIFormation_Mapped(const NinjaLua::LuaObject& def)
{
	// Formation shape attributes
	ntAssert(def[DEF_PLACES].IsTable());
	NinjaLua::LuaObject lua_points = def[DEF_PLACES];

	// Count the number of places
	for (m_iPoints = 0; lua_points[m_iPoints+1].IsTable(); m_iPoints++)
		;

	// Create the positions and set attributes
	// [scee_st] added ARRAY specifier -- deleter uses it correctly already
	m_aLUAPoints = NT_NEW_ARRAY_CHUNK(Mem::MC_AI) CPoint[m_iPoints];

	for (int iPoint = 0; lua_points[iPoint+1].IsTable(); iPoint++)
	{
		m_aLUAPoints[iPoint] = CLuaHelper::PointFromTable(lua_points[iPoint+1]);
	}

	m_bCameraRelative = def.GetOpt(DEF_CAMERARELATIVE, false);

	ValidateData();
}

void AIFormation_Mapped::ValidateData()
{
	m_iSlotsInFormation = GetPoints();

	m_fEntryDistance = 0.0f;

	for (int iPoint = 0; iPoint < GetPoints(); iPoint++)
	{
		CPoint* pPoint = GetPoint(iPoint);

		// Looking for furthest point from centre of formation.
		float fDistanceToTarget = pPoint->LengthSquared();

		if (fDistanceToTarget > m_fEntryDistance)
		{
			m_fEntryDistance = fDistanceToTarget;
		}
	}

	// Entry distance is a bit bigger than outer most point.
	m_fEntryDistance = sqrtf(m_fEntryDistance) + ENTRY_DISTANCE_THRESHOLD;

	// Exit distance is a bit bigger than entry distance.
	m_fExitDistance= m_fEntryDistance + EXIT_DISTANCE_THRESHOLD;

	// Check everything's ok.
	ntAssert(m_iSlotsInFormation > 0);
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIFormation_Mapped::CalculateSlotPositions                                                        
//! Calculate the positions of all the slots in the formation.                    
//!                                                                                         
//------------------------------------------------------------------------------------------
void AIFormation_Mapped::CalculateSlotPositions()
{
	CMatrix obRot;

	if (m_bCameraRelative)
	{
		const CamView* pView = CamMan::GetPrimaryView();
		CDirection ZView = pView->GetCurrMatrix().GetZAxis();
		float fAngle = atan2f(ZView.X(), ZView.Z());

		CQuat obQuatRot = CQuat(CDirection( 0.0f, 1.0f, 0.0f ), fAngle);

		obRot = CMatrix(obQuatRot);
	}

	for (int iPoint = 0; iPoint < GetPoints(); iPoint++)
	{
		CPoint* pPoint = GetPoint(iPoint);

		AIFormationSlot* pSlot = GetSlot(iPoint);
		ntAssert(pSlot);

		if (m_bCameraRelative)
		{
			CMatrix obPos;

			obPos.Clear();
			obPos.SetTranslation(*pPoint);

			obPos *= obRot;

			pSlot->SetLocalPoint(obPos.GetTranslation());
		}
		else
		{
			pSlot->SetLocalPoint(*pPoint);
		}
	}
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIFormation_Mapped::GetSlot                                                           
//! Get the slot in a given positions.                                                 
//!                                                                                         
//------------------------------------------------------------------------------------------
inline AIFormationSlot* AIFormation_Mapped::GetSlot(int iPoint)
{
	ntAssert(iPoint >= 0 && iPoint < GetPoints());

	return m_pSlots[iPoint];
}
