//------------------------------------------------------------------------------------------
//!
//!	\file CoolCam_Chase.cpp
//!
//------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
// Required Includes
//------------------------------------------------------------------------------------------
#include "camera/coolcam_chaseaimcombo.h"
#include "camera/coolcam_chase.h"
#include "camera/coolcam_aim.h"

#include "camera/camman.h"
#include "camera/camview.h"
#include "camera/camutils.h"
#include "camera/timescalarcurve.h"

#include "core/visualdebugger.h"
#include "game/vkey.h"
#include "game/entity.h"
#include "game/entity.inl"
#include "game/entityinfo.h"
#include "game/messagehandler.h"
#include "game/inputcomponent.h"

//------------------------------------------------------------------------------------------
//!                                                                  
//!	CoolCam_ChaseAimCombo::CoolCam_ChaseAimCombo
//!	Construction                                                                      
//!                                                                  
//------------------------------------------------------------------------------------------
CoolCam_ChaseAimCombo::CoolCam_ChaseAimCombo(const CamView& view, const CEntity& ent, 
											 CoolCam_ChaseDef& chaseDef, CoolCam_AimDef& aimDef) : 
	CoolCamera(view),
	m_pEntity( &ent ),
	m_pChaseCam( 0 ),
	m_pAimCam( 0 ),
	m_eMode( AIM ),
	m_bActive( false ),
	m_fTransitionTime( 0.f ),
	m_fTotalTransitionTimeIn( 0.f ),
	m_fTotalTransitionTimeOut( 0.f ),
	m_fInOutRatio( 0.f )
{
	m_pChaseCam = NT_NEW_CHUNK( Mem::MC_CAMERA ) CoolCam_Chase(view, ent, chaseDef);
	m_pAimCam   = NT_NEW_CHUNK( Mem::MC_CAMERA ) CoolCam_Aim(view, ent, aimDef);

	m_fTotalTransitionTimeIn  = aimDef.m_fTransitionTimeIn;
	m_fTotalTransitionTimeOut = aimDef.m_fTransitionTimeOut;
	m_fInOutRatio             = m_fTotalTransitionTimeIn / m_fTotalTransitionTimeOut; // Transition in and out at different speeds?

	m_iPriority = 5;
}


//------------------------------------------------------------------------------------------
//!                                                                  
//!	CoolCam_Chase::~CoolCam_Chase                                                                  
//!	Destruction                                                                      
//!                                                                  
//------------------------------------------------------------------------------------------
CoolCam_ChaseAimCombo::~CoolCam_ChaseAimCombo()
{
	NT_DELETE_CHUNK(Mem::MC_CAMERA, m_pChaseCam );
	NT_DELETE_CHUNK(Mem::MC_CAMERA,  m_pAimCam );
}


//------------------------------------------------------------------------------------------
//!                                                                  
//!	CoolCam_ChaseAimCombo::Update                                                                  
//!	Update the Camera                                                   
//!                                                                  
//------------------------------------------------------------------------------------------
void CoolCam_ChaseAimCombo::Update(float fTimeDelta)
{
	// Update the child cameras
	// ------------------------
	m_pChaseCam->Update(fTimeDelta);
	m_pAimCam->Update(fTimeDelta);

	// Handle entering and exiting aiming mode...  (This is all a little nasty and should probably be moved out into a game script - quick hack for VIP demo)
	// ------------------------------------------
	if(m_pEntity->GetInputComponent()->GetVHeld() & (1<<AB_RSTANCE))
	{
		m_bActive = true;
	}
	else
	{
		m_bActive = false;
		if(m_eMode == AIM && m_pChaseCam->Enabled() )
		{
			m_eMode = CHASE;
			m_fTransitionTime = 0.f;
			CMessageSender::SendEmptyMessage( "msg_aim_off", m_pEntity->GetMessageHandler() );
		}
	}

	if(!m_bActive)
		return;

	if(m_eMode != AIM && m_pEntity->GetInputComponent()->GetInputSpeed() < 0.3f)
	{
		m_eMode = AIM;
		CMessageSender::SendEmptyMessage( "msg_aim_on", m_pEntity->GetMessageHandler() );
	}
	else if(m_eMode != CHASE && m_pEntity->GetInputComponent()->GetInputSpeed() > 0.9f && m_pChaseCam->Enabled())
	{
		CDirection dir = m_pEntity->GetInputComponent()->GetInputDir() * GetTransform().GetAffineInverse();
		if(dir.Z() > 0.85f)
		{
			m_eMode = CHASE;
			CMessageSender::SendEmptyMessage( "msg_aim_off", m_pEntity->GetMessageHandler() );
		}
	}

	// Update the transition into and out of aiming mode.
	// --------------------------------------------------
	if(m_eMode == CHASE)
	{
		if(m_fTransitionTime > 0.f)
			m_fTransitionTime -= fTimeDelta * m_fInOutRatio;

		// Make sure the vaule isn't too small. 
		if(m_fTransitionTime < fTimeDelta)
			m_fTransitionTime = fTimeDelta;
	}
	else
	{
		if( m_pAimCam->GetTimeCurve() )
		{
			TimeScalarCurve* pTimeCurve = const_cast<TimeScalarCurve*>(m_pAimCam->GetTimeCurve());
			m_fTimeScalar = pTimeCurve->GetScalar( m_fTransitionTime / m_fTotalTransitionTimeIn );
		}

		if(m_fTransitionTime < m_fTotalTransitionTimeIn)
			m_fTransitionTime += fTimeDelta;
	}

	// Ease in and out using a smooth sigmoid function
	float fRatio = CCamUtil::Sigmoid(m_fTransitionTime, m_fTotalTransitionTimeIn);

	CPoint pos = m_pChaseCam->GetTransform().GetTranslation() + 
		         ((m_pAimCam->GetTransform().GetTranslation() - m_pChaseCam->GetTransform().GetTranslation()) * fRatio);
	m_fFOV     = m_pChaseCam->GetFOV() + ((m_pAimCam->GetFOV() - m_pChaseCam->GetFOV()) * fRatio);

	CQuat qSrc(m_pChaseCam->GetTransform());
	CQuat qDst(m_pAimCam->GetTransform());
	CQuat qFinal = CQuat::Slerp(qSrc, qDst, fRatio);

	m_obTransform = CMatrix(qFinal, pos);
	m_obLookAt = m_pChaseCam->GetLookAt() + ((m_pAimCam->GetLookAt() - m_pChaseCam->GetLookAt()) * fRatio);

	// Rander the Aiming sight
	// -----------------------
	if(m_eMode == AIM && m_fTransitionTime >= m_fTotalTransitionTimeIn)
	{
		if(CamMan::GetPrimaryView()->GetActiveCameraID() == GetID() &&
		  !CamMan::GetPrimaryView()->IsTransitionActive())
		{
			/*
			g_VisualDebug->Printf2D(	g_VisualDebug->GetDebugDisplayWidth() * 0.5f,
									g_VisualDebug->GetDebugDisplayHeight() * 0.5f-6.0f,
										NTCOLOUR_ARGB(0xff,0xff,0x77,0x77), DTF_ALIGN_HCENTRE, "X");
			*/
		}

		m_bUseDoF        = m_pAimCam->UsingDoF();
		m_fFocalDepth    = m_pAimCam->GetFocalDepth();
		m_fNearBlurDepth = m_pAimCam->GetNearBlurDepth();
		m_fFarBlurDepth  = m_pAimCam->GetFarBlurDepth();
	}
	else
	{
		m_bUseDoF        = m_pChaseCam->UsingDoF();
		m_fFocalDepth    = m_pChaseCam->GetFocalDepth();
		m_fNearBlurDepth = m_pChaseCam->GetNearBlurDepth();
		m_fFarBlurDepth  = m_pChaseCam->GetFarBlurDepth();
	}
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CoolCam_ChaseAimCombo::SetPitch                                                   
//!	Update the Camera                                                                       
//!                                                                                         
//------------------------------------------------------------------------------------------
void CoolCam_ChaseAimCombo::SetPitch(float fPitch)
{
	m_pAimCam->SetPitch(fPitch);
}
