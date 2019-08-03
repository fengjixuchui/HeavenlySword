//------------------------------------------------------------------------------------------
//!
//!	\file aiformation.cpp
//!
//------------------------------------------------------------------------------------------

#include "ai/aiformationslot.h"
#include "ai/aiformation.h"
#include "objectdatabase/dataobject.h"
#include "game/entityai.h"
#include "game/entity.inl"
#include "core/timer.h"
#include "core/visualdebugger.h"

#define COLLISION_Y_OFFSET (0.1f)
#define DBG_Y_OFFSET CPoint(0,COLLISION_Y_OFFSET,0)

START_STD_INTERFACE( AIFormationSlot )
	PUBLISH_VAR_AS( m_pt, Position )
END_STD_INTERFACE

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIFormationSlot::~AIFormationSlot
//! 
//!                                                                                         
//------------------------------------------------------------------------------------------
AIFormationSlot::~AIFormationSlot()
{
	if( m_pFormation )
	{
		if( m_pEnt )
		{
			m_pFormation->RemoveEntity( m_pEnt );
		}

		m_pFormation->RemoveSlot( *this );
	}

	// Check that the entiyt 
	ntAssert( m_pEnt == NULL );
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIFormationSlot::GetWorldPoint                                                          
//! Return the world space co-ordinates of this slot.                                       
//!                                                                                         
//------------------------------------------------------------------------------------------
CPoint AIFormationSlot::GetWorldPoint() const 
{
	return m_pFormation ? m_pFormation->GetTarget() + m_pt : m_pt;
}

//------------------------------------------------------------------------------------------
//!  public virtual constant  UpdateInRange
//!
//!  @return bool Whether the entity assigned to the slot is in range of the formation.
//!
//!  @remarks <TODO: insert remarks here>
//!
//!  @author GavB @date 14/08/2006
//------------------------------------------------------------------------------------------
void AIFormationSlot::UpdateInRange()
{ 
	// Find the distance from the entity to the target for the formation
	float fTargDistSqrd	= (m_pFormation->GetTarget() - GetEntity()->GetPosition()).LengthSquared();

	SetInRange(fTargDistSqrd < sqr(m_bInRange ? m_pFormation->GetExitDistance() : m_pFormation->GetEntryDistance()));
}

#ifndef _RELEASE
void AIFormationSlot::RenderSquare(CPoint const& Point, float fSize, unsigned int uiFormationColour)
{
	g_VisualDebug->RenderLine(Point + CPoint(-fSize, 0.0f, fSize), Point + CPoint(fSize, 0.0f, fSize), uiFormationColour);
	g_VisualDebug->RenderLine(Point + CPoint(fSize, 0.0f, fSize), Point + CPoint(fSize, 0.0f, -fSize), uiFormationColour);
	g_VisualDebug->RenderLine(Point + CPoint(fSize, 0.0f, -fSize), Point + CPoint(-fSize, 0.0f, -fSize), uiFormationColour);
	g_VisualDebug->RenderLine(Point + CPoint(-fSize, 0.0f, -fSize), Point + CPoint(-fSize, 0.0f, fSize), uiFormationColour);
}

void AIFormationSlot::RenderPlus(CPoint const& Point, float fSize, unsigned int uiFormationColour)
{
	g_VisualDebug->RenderLine(Point + CPoint(0.0f, 0.0f, fSize), Point + CPoint(0.0f, 0.0f, -fSize), uiFormationColour);
	g_VisualDebug->RenderLine(Point + CPoint(-fSize, 0.0f, 0.0f), Point + CPoint(fSize, 0.0f, 0.0f), uiFormationColour);
}

void AIFormationSlot::RenderCross(CPoint const& Point, float fSize, unsigned int uiFormationColour)
{
	g_VisualDebug->RenderLine(Point + CPoint(-fSize, 0.0f, fSize), Point + CPoint(fSize, 0.0f, -fSize), uiFormationColour);
	g_VisualDebug->RenderLine(Point + CPoint(fSize, 0.0f, fSize), Point + CPoint(-fSize, 0.0f, -fSize), uiFormationColour);
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	AIFormation::RenderDebugInfo
//! Display positions of all slots, with information as to if they are valid and which entities are using them.
//!                                                                                         
//------------------------------------------------------------------------------------------
void AIFormationSlot::RenderDebugInfo(unsigned int uiFormationColour)
{
	AI* pAI = GetEntity();
	CPoint SlotPoint = GetWorldPoint() + DBG_Y_OFFSET;
	float fSize = 0.25f;

	// A formation point is available.
	if (pAI)
	{
		// Draw a line from entity to its slot.
		g_VisualDebug->RenderLine(GetWorldPoint() + DBG_Y_OFFSET, pAI->GetPosition() + DBG_Y_OFFSET, uiFormationColour);
	}
	else
	{
		// Make smaller for non enabled (not currently needed) slots.
		fSize *= 0.5f;
	}

	// Show if slot invalid.
	if (!m_bNavigable)
		RenderPlus(SlotPoint, fSize, uiFormationColour);

	if (!m_bClearOfOtherSlots)
		RenderCross(SlotPoint, fSize, uiFormationColour);

	RenderSquare(SlotPoint, fSize, uiFormationColour);
}
#endif
