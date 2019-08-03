//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file sceneelementcomponent.cpp                                                         
//!                                                                                         
//------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
// Required Includes                                                                        
//------------------------------------------------------------------------------------------
#include "game/entitypoieffector.h"

#include "game/fsm.h"
#include "objectdatabase/dataobject.h"


//------------------------------------------------------------------------------------------
// Definition Interface                                                                     
//------------------------------------------------------------------------------------------
START_STD_INTERFACE(POI_Effector)
	COPY_INTERFACE_FROM(CEntity)
	DEFINE_INTERFACE_INHERITANCE(CEntity)
	OVERRIDE_DEFAULT(ParentTransform, "ROOT")

	PUBLISH_VAR_WITH_DEFAULT_AS(m_def.m_fImportance,         0.5f, Importance)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_def.m_fRadius,             1.0f, Radius)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_def.m_obOffset,            CDirection(0.0f, 0.0f, 0.0f), Offset)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_def.m_fInfluenceRadiusIn,  5.0f, RadiusIn)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_def.m_fInfluenceRadiusOut, 10.0f, RadiusOut)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_def.m_fLookAheadTime,      0.0f, LookAheadTime)

	DECLARE_POSTPOSTCONSTRUCT_CALLBACK(OnPostPostConstruct)
END_STD_INTERFACE


//!----------------------------------------------------------------------------------------------
//!
//! Simple Toggle FSM
//!
//!----------------------------------------------------------------------------------------------
STATEMACHINE(TOGGLE_FSM, POI_Effector)
	BEGIN_EVENTS
		EVENT(msg_deactivate)
			ME->SetActive(false);
		END_EVENT(true)

		EVENT(msg_activate)
			ME->SetActive(true);
		END_EVENT(true)
	END_EVENTS
END_STATEMACHINE



//------------------------------------------------------------------------------------------
//!                                                                                         
//!	POI_Effector::OnPostPostConstruct
//!	Create the scene element...
//!                                                                                         
//------------------------------------------------------------------------------------------
void POI_Effector::OnPostPostConstruct()
{
	m_pSceneElementComponent = NT_NEW_CHUNK(Mem::MC_ENTITY) SceneElementComponent(this, &m_def);

	CEntity::OnPostPostConstruct();

	// Create and attach the statemachine
	TOGGLE_FSM* pFSM = NT_NEW_CHUNK(Mem::MC_ENTITY) TOGGLE_FSM;
	ATTACH_FSM(pFSM);
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	POI_Effector::SetActive
//!	Activate and Deactive the Effector
//!                                                                                         
//------------------------------------------------------------------------------------------
void POI_Effector::SetActive(bool bActive)
{
	if(!bActive)
	{
		m_pSceneElementComponent->SetImportance(0.f);
	}
	else
	{
		m_pSceneElementComponent->SetImportance(m_def.m_fImportance);
	}
}
