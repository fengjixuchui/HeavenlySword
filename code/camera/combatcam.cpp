//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file combatcam.cpp
//!                                                                                         
//------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
// Required Includes
//------------------------------------------------------------------------------------------
#include "camera/combatcam.h"

#include "camera/camman.h"
#include "camera/camview.h"
#include "camera/converger.h"
#include "game/entitymanager.h"
#include "camera/combatcamdef.h"
#include "camera/basiccamera.h"
#include "camera/motioncontroller.h"
#include "camera/lookatcontroller.h"


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CombatCam::CombatCam                                                                    
//!	                                                                                        
//!                                                                                         
//------------------------------------------------------------------------------------------
CombatCam::CombatCam(const BasicCamera* parent, CCombatCamDef& def)
:	m_def(def), 
	m_pParent(parent),
	m_pobZoomConverger(0),
	m_fLastZoomModifier(0.f), 
	m_fZoomHoldTimer(0.f), 
	m_fFoVBlendTime(def.m_fFoVBlendTime),
	m_bCombatAware(false)
{
	if(def.m_fZoomOutSpeed > 0.0f)
	{
		CConvergerDef convergerDef("_ZoomConverger");
		convergerDef.SetSpeed(def.m_fZoomOutSpeed, def.m_fZoomInSpeed);
		convergerDef.SetSpring(m_def.m_fZoomSpring);
		convergerDef.SetDamp(m_def.m_fZoomDamp);
		m_pobZoomConverger = NT_NEW_CHUNK(Mem::MC_CAMERA) CConverger(convergerDef);
	}
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CombatCam::~CombatCam                                                                   
//!	                                                                                        
//!                                                                                         
//------------------------------------------------------------------------------------------
CombatCam::~CombatCam() 
{
	if(m_pobZoomConverger)
	{
		NT_DELETE_CHUNK(Mem::MC_CAMERA, m_pobZoomConverger );
	}
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CombatCam::ApplyZoomConvergence : 
//!                                                                                         
//------------------------------------------------------------------------------------------
float CombatCam::ApplyZoomConvergence( float fZoom, float fPlayerMaxZoom, float fTimeDelta )
{
	float fZoomModifier = fPlayerMaxZoom - fZoom;

	//OSD::Log(OSD::CAMERA, "(Z:%.2f - M:%.2f)\n", fZoom, fMaxZoom);

	// Zoom Hold...
	if(m_fLastZoomModifier == -MAX_POS_FLOAT)
		m_fLastZoomModifier = fZoomModifier;

	// Converge Zoom
	if(m_pobZoomConverger)
	{
		if(fZoomModifier > m_fLastZoomModifier ||
		   m_fZoomHoldTimer < 0.0f)
		{
			// Stay zoomed out for a while before zooming in again
			if(fZoomModifier > m_fLastZoomModifier)
			{
				m_fZoomHoldTimer = m_def.m_fZoomHoldTime;
			}

			m_pobZoomConverger->Update(fZoomModifier, fTimeDelta);
			m_fLastZoomModifier = fZoomModifier = m_pobZoomConverger->GetDamped();
		}
		else
			fZoomModifier = m_fLastZoomModifier;

		m_fZoomHoldTimer -= fTimeDelta;
	}

	// Calculate the final zoom
	fZoom = fPlayerMaxZoom - fZoomModifier;

	// Do not go beyond the max or we'll lose the player
	if(fZoom > fPlayerMaxZoom)
		fZoom = fPlayerMaxZoom;

	return fZoom;
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CombatCam::UpdateFOV                                                                     
//!	Updates FOV (and also general states of combatcam, currently m_bCombatAware), 
//! 		and returns curr value for use.                          
//!                                                                                         
//------------------------------------------------------------------------------------------
float CombatCam::UpdateFOV(float fExplorationFoV, float fTimeDelta)
{
	// Check for changes in Combat Awareness
	if(CamMan::Get().IsCombatAware() != m_bCombatAware)
	{
		m_bCombatAware = CamMan::Get().IsCombatAware();

		if(m_fFoVBlendTime >= m_def.m_fFoVBlendTime)
		{
			m_fFoVBlendTime = 0.f;
		}
		else
		{
			m_fFoVBlendTime = m_def.m_fFoVBlendTime - m_fFoVBlendTime;
		}
	}

	// Get the current FoV
	float fCurrFov(m_bCombatAware ? m_def.m_fFoV : fExplorationFoV);
	if(m_fFoVBlendTime < m_def.m_fFoVBlendTime)
	{
		float fSigmoidFactor = CCamUtil::Sigmoid(m_fFoVBlendTime, m_def.m_fFoVBlendTime);
		fCurrFov = (fCurrFov * fSigmoidFactor) + ((m_bCombatAware ? fExplorationFoV : m_def.m_fFoV) * (1.f-fSigmoidFactor));

		m_fFoVBlendTime += fTimeDelta;
	}

	return fCurrFov;
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CombatCam::CalcZoom                                                                     
//!	Calculate the zoom required for this combat situation.                                  
//!                                                                                         
//------------------------------------------------------------------------------------------
float CombatCam::CalcZoom(float fTimeDelta, 
						  const CPoint& ptPos, 
						  const CPoint& ptLookAt, 
						  float fCurrFov, 
						  float fExplorationMinRatio, 
						  float fExplorationMaxRatio)
{
	float fZoom, fMaxZoom;

	// Determine the Ideal Zoom Value
	if(m_bCombatAware)
	{
		if (m_def.m_bCanZoomInCombat)
		{
			if(m_pParent->GetElementManager().CountEnemiesInfluencing() == 0)
			{
				float fTargetFOV = fatanf(ftanf(fCurrFov) * m_def.m_fIdealInterestingRatio);
				fZoom = m_pParent->GetElementManager().CalcPlayerZoom(ptPos, ptLookAt, CamMan::GetPrimaryView()->GetAspectRatio(), fTargetFOV);
			}
			else
			{
				fZoom = m_pParent->GetElementManager().CalcIdealZoom(ptPos, ptLookAt, CamMan::GetPrimaryView()->GetAspectRatio(), fCurrFov);

				float fTargetFOV = fatanf(ftanf(fCurrFov) * m_def.m_fMaxInterestingRatio);
				float fZoomLimitHigh = m_pParent->GetElementManager().CalcPlayerZoom(ptPos, ptLookAt, CamMan::GetPrimaryView()->GetAspectRatio(), fTargetFOV);
				fTargetFOV = fatanf(ftanf(fCurrFov) * m_def.m_fMinInterestingRatio);
				float fZoomLimitLow = m_pParent->GetElementManager().CalcPlayerZoom(ptPos, ptLookAt, CamMan::GetPrimaryView()->GetAspectRatio(), fTargetFOV);
				fZoom = clamp(fZoom, fZoomLimitLow, fZoomLimitHigh);
			}

			// Determine the Maximum Zoom Value
			fMaxZoom = m_pParent->GetElementManager().CalcPlayerZoom(ptPos, ptLookAt, CamMan::GetPrimaryView()->GetAspectRatio(),fCurrFov);
		}
		else
		{
			fZoom = fMaxZoom  =0.f;
		}
	}
	else
	{
		if(fExplorationMaxRatio < 0)
		{
			fZoom = fMaxZoom  =0.f;
		}
		else
		{
			fZoom = m_pParent->GetElementManager().CalcIdealZoom(ptPos, ptLookAt, CamMan::GetPrimaryView()->GetAspectRatio(), fCurrFov);

			float fTargetFOV     = fatanf(ftanf(fCurrFov) * fExplorationMaxRatio);
			float fZoomLimitHigh = m_pParent->GetElementManager().CalcPlayerZoom(ptPos, ptLookAt, CamMan::GetPrimaryView()->GetAspectRatio(), fTargetFOV);

				fTargetFOV     = fatanf(ftanf(fCurrFov) * fExplorationMinRatio);
			float fZoomLimitLow  = m_pParent->GetElementManager().CalcPlayerZoom(ptPos, ptLookAt, CamMan::GetPrimaryView()->GetAspectRatio(), fTargetFOV);

			fZoom          = clamp(fZoom, fZoomLimitLow, fZoomLimitHigh);

			// Determine the Maximum Zoom Value
			fMaxZoom = m_pParent->GetElementManager().CalcPlayerZoom(ptPos, ptLookAt, CamMan::GetPrimaryView()->GetAspectRatio(),fCurrFov);
		}
	}

	return ApplyZoomConvergence( fZoom, fMaxZoom, fTimeDelta );
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CombatCam::AutoActivate                                                                 
//!	Automatically activate the combat camera when enemies are nearby?                       
//!                                                                                         
//------------------------------------------------------------------------------------------
bool CombatCam::AutoActivate() const
{
	return m_def.m_bAutoActivate;
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CombatCam::GetCombatOffset                                                                 
//!	What is the offset for this combat camera?                                              
//!                                                                                         
//------------------------------------------------------------------------------------------
const CDirection& CombatCam::GetCombatOffset() const
{
	return m_def.m_ptOffset;
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CombatCam::GetPOILimit                                                                     
//!	What is the POI limit for this combat camera?                                           
//!                                                                                         
//------------------------------------------------------------------------------------------
float CombatCam::GetPOILimit() const
{
	return m_def.m_fPOILimit;
}
