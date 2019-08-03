//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file combatcamdef.cpp
//!                                                                                         
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
// Required Includes
//------------------------------------------------------------------------------------------
#include "camera/combatcamdef.h"
#include "objectdatabase/dataobject.h"


//------------------------------------------------------------------------------------------
// CCombatCamDef XML Interface
//------------------------------------------------------------------------------------------
START_CHUNKED_INTERFACE(CCombatCamDef, Mem::MC_CAMERA)
	IHELP("Definitions for the combat camera systems")
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fZoomInSpeed, 1.0f, ZoomInSpeed)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fZoomOutSpeed, 2.0f, ZoomOutSpeed)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fZoomSpring, 0.75f, ZoomSpring)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fZoomDamp, 0.5f, ZoomDamp)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fZoomHoldTime, 2.0f, ZoomHoldTime)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fFovExternal, 35.f, FoV)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fFoVBlendTime, 5.f, FoVBlendTime)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fIdealInterestingRatio, 0.5f, IdealInterestingRatio)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fMaxInterestingRatio, 0.75f, MaxInterestingRatio)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fMinInterestingRatio, 0.25f, MinInterestingRatio)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fRotateUpSpeed, 45.0f, RotateUpSpeed)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fRotateDownSpeed, 15.0f, RotateDownSpeed)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fRotateSpring, 0.75f, RotateSpring)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fRotateDamp, 0.5f, RotateDamp)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fMaxRotation, 60.0f, MaxRotation)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fRotationHoldTime, 2.0f, RotationHoldTime)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_ptOffset, CDirection(CONSTRUCT_CLEAR), Offset)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fPOILimit, -1.f, POILimit)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bAutoActivate, false, AutoActivate)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bCanZoomInCombat, true, CanZoomInCombat)

	DECLARE_POSTCONSTRUCT_CALLBACK(PostConstruct)
	DECLARE_EDITORCHANGEVALUE_CALLBACK(EditorChangeValue)
END_STD_INTERFACE



//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CCombatCamDef::CCombatCamDef
//!	Construction
//!                                                                                         
//------------------------------------------------------------------------------------------
CCombatCamDef::CCombatCamDef()
 : m_ptOffset(CONSTRUCT_CLEAR), m_fZoomHoldTime(0.0f),
   m_fZoomInSpeed(0.0f), m_fZoomOutSpeed(0.0f),
   m_fZoomSpring(0.0f), m_fZoomDamp(0.0f),
   m_fRotateUpSpeed(0.0f), m_fRotateDownSpeed(0.0f),
   m_fRotateSpring(0.0f), m_fRotateDamp(0.0f),
   m_fMaxRotation(0.0f), m_fRotationHoldTime(0.0f),
   m_fPOILimit(-1.f), m_bAutoActivate(false)
{
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CCombatCamDef::PostConstruct
//!	PostConstruction
//!                                                                                         
//------------------------------------------------------------------------------------------
void CCombatCamDef::PostConstruct()
{
	m_fFoV = m_fFovExternal*DEG_TO_RAD_VALUE;
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CCombatCamDef::EditorChangeValue
//!	Recompute the FoV when you edit the interface
//!                                                                                         
//------------------------------------------------------------------------------------------
bool CCombatCamDef::EditorChangeValue(CallBackParameter, CallBackParameter)
{
	PostConstruct();
	return true;
}
