//! -------------------------------------------
//! aihearingman.cpp
//!
//! AI Hearing Manager
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!--------------------------------------------

#include "aihearingman.h"
#include "aihearing.h"
#include "game/aicomponent.h"
//#include "game/entitymanager.h"
//#include "game/query.h"
#include "game/entity.h"
#include "game/entity.inl"
#include "game/messagehandler.h"
#include "ai/ainavigationsystem/ainavigsystemman.h"

#include "core/visualdebugger.h"


//------------------------------------------------------------------------------------------
// Lua Interface
//------------------------------------------------------------------------------------------
LUA_EXPOSED_START(CAIHearingMan)
	LUA_EXPOSED_METHOD(TriggerSound, TriggerSound, "Adds a Sound to AI the Hearing Manager", "Entity, float, float, bool", "Sound Source, radius, volume Constant volume")
	LUA_EXPOSED_METHOD(GetSoundSourceEntity, GetSoundSourceEntity, "Returns the source of sound heard by a particular AI", "Entity", "AI")
LUA_EXPOSED_END(CAIHearingMan)

//!--------------------------------------------
//! Constructor
//!--------------------------------------------
CAIHearingMan::CAIHearingMan() : m_bSoundInfoRender(false)
{
	ATTACH_LUA_INTERFACE(CAIHearingMan);
	CLuaGlobal::Get().State().GetGlobals().Set("CAIHearingMan", this);
}

//!--------------------------------------------
//! AddSound
//!--------------------------------------------
void CAIHearingMan::TriggerSound ( CEntity* pSourceEnt, float fRadius, float fVolume, bool bConstantVol )
{
	if (!pSourceEnt) return;

	// Check that the parameters make sense
	if (fRadius<0 || fVolume<0) return;

	// Get Sound source position
	CPoint obSourcePos = pSourceEnt->GetPosition();

	//float fMinDistSQR	= FLT_MAX-2;
	float fRadiusSQR	= fRadius*fRadius;
	bool  bAlerted		= false;
	bool  bAnyAlerted	= false;

	// Clear Debug Info 
	m_SDebugInfo = SDebugSoundInfo();

	// =================================================================
	// ========       LOOP THROUGH THE LIST OF AIs             =========
	// =================================================================

	QueryResultsContainerType* pLocalAIList = CAINavigationSystemMan::Get().GetAIList();
	if (!pLocalAIList)
		return;

	QueryResultsContainerType::iterator obIt	= pLocalAIList->begin();
	QueryResultsContainerType::iterator obEnd	= pLocalAIList->end();

	for ( ; obIt != obEnd; ++obIt )
	{
		bAlerted = false;
		AI* pEnt = (AI*)(*obIt);
		CAIHearing* pHear = pEnt->GetAIComponent()->GetAIHearing();

		// Only consider active, non deaf entities

		if ( pEnt->IsPaused() || pEnt->IsDead() || !pHear || pHear->IsDeaf() ) 
 			continue;

		// Find AIs in Range

		float	fDistSQR = (pEnt->GetPosition() - obSourcePos).LengthSquared();
		
		if ( fRadiusSQR > fDistSQR )
		{
			// Is the volume high enough

			float fVolumeThreshold = pHear->GetVolumeThreshold();

			if ( bConstantVol &&  ( fVolume > fVolumeThreshold ) )
			{
				//// Is this the closest sound?
				//if ( fDistSQR < fMinDistSQR )
				//{
				//	bAlerted		= true;
				//	fMinDistSQR		= fDistSQR;
				//}
					bAlerted		= true;
				bAnyAlerted		= true;
			} 
			else
			{
				// Apply the distance factor
				float fLocalVolume = (fVolume/fRadiusSQR)*(fRadiusSQR-fDistSQR);

				if ( fLocalVolume > fVolumeThreshold )
				{
					// Is this the closest sound?
					/*if ( fDistSQR < fMinDistSQR )
					{
						bAlerted		= true;
						fMinDistSQR		= fDistSQR;
					}*/
					bAlerted		= true;
					bAnyAlerted		= true;
				}
			}

			if (bAlerted)
			{
				//pHear->SetSoundSourceLocation(obSourcePos);
				pHear->SetSoundSourceEntity(pSourceEnt);
				pEnt->GetMessageHandler()->Receive(CMessageHandler::Make(pEnt, "MSG_ALERTED_BY_SOUND"));
			}
		}
	}
	// Add Debug Info
	if (m_bSoundInfoRender)
		m_SDebugInfo = SDebugSoundInfo(pSourceEnt,fRadius,fVolume, bAnyAlerted);
}

//!--------------------------------------------
//! GetSoundSourceEntity
//!--------------------------------------------
CEntity* CAIHearingMan::GetSoundSourceEntity ( AI* pAI )
{
	if (!pAI) return NULL;
	
	return pAI->GetAIComponent()->GetAIHearing()->GetSoundSourceEntity();
}

//!--------------------------------------------
//! Update
//!--------------------------------------------
void CAIHearingMan::Update ( float fTimeChange )
{
	static const CQuat obOrientation( CONSTRUCT_IDENTITY );

	m_SDebugInfo.m_fTimer+=fTimeChange;
	if (m_SDebugInfo.m_fTimer > 10.0f) 
	{
		m_SDebugInfo.m_fTimer=0.0f;
		m_SDebugInfo = SDebugSoundInfo();

	}

#ifndef _GOLD_MASTER
	unsigned int uiColor = m_SDebugInfo.m_bWasItHeard ? DC_BLUE : DC_WHITE;
	if (m_bSoundInfoRender && m_SDebugInfo.m_fDbgSoundVolume > 0.0f && m_SDebugInfo.m_fDbgSoundRadius > 0.0f )
	{
		m_SDebugInfo.m_fDbgEntityRadius+=0.1f;
		if (m_SDebugInfo.m_fDbgEntityRadius>1.5f) m_SDebugInfo.m_fDbgEntityRadius=0.0f;
		
		// Render Sound Radius
		g_VisualDebug->RenderSphere( obOrientation, m_SDebugInfo.m_pEnt->GetPosition(), m_SDebugInfo.m_fDbgSoundRadius, uiColor, 4096 );
		g_VisualDebug->Printf3D( m_SDebugInfo.m_pEnt->GetPosition(), DC_WHITE, 0, "%s - Sound Volume: [%.3f] - %s", 
																				m_SDebugInfo.m_bWasItHeard ? "Heard" : "Unnoticed",
																				m_SDebugInfo.m_fDbgSoundVolume,
																				m_SDebugInfo.m_bDbgSoundConstant ? "Constant" : "Decreasing" );

		// Render Sound Source
		CMatrix ArcMatrix(CONSTRUCT_IDENTITY);
		ArcMatrix.SetTranslation(m_SDebugInfo.m_pEnt->GetPosition()+CPoint(0.0f,0.1f,0.0f));
		g_VisualDebug->RenderArc(ArcMatrix, m_SDebugInfo.m_fDbgEntityRadius , TWO_PI,  DC_RED);
	}
#endif


}

