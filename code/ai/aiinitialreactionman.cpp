//! ---------------------------------------------
//! aiinitialreactionman.h
//!
//! AI Initial Reactions (on player seen) Manager
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!----------------------------------------------

#include "aiinitialreactionman.h"
#include "game/entity.inl"
#include "game/entity.h"
#include "aihearingman.h"
#include "ai/ainavigationsystem/ainavigsystemman.h"

#include "core/visualdebugger.h"

//------------------------------------------------------------------------------------------
// Lua Interface
//------------------------------------------------------------------------------------------
LUA_EXPOSED_START(CAIInitialReactionMan)
	LUA_EXPOSED_METHOD(AddAI, AddAI, "Adds an AI the InitialReaction Manager", "AI*", "AI to be added")
	LUA_EXPOSED_METHOD(ClearReporter, ClearReporter, "Clears the InitialReaction Manager", "", "")
	LUA_EXPOSED_METHOD(SetAnims, SetAnims, "Set the InitialReaction Anims", "", "")	
LUA_EXPOSED_END(CAIInitialReactionMan)

//!--------------------------------------------
//! Constructor / Destructor
//!--------------------------------------------
CAIInitialReactionMan::CAIInitialReactionMan() :	m_bInitialReactionRender(false), m_pSAIFirstReporter(NULL), m_pAI_FirstReporter(NULL), 
													m_hsReportEnemySeenAnim("ShieldmanOrderAttack"), m_hsResponseEnemySeenAnim("ShieldmanOrderObey"), 
													m_hsSoundAlerted("ShieldmanHearStand"), m_bReporterAnimFinished(false)
{
	ATTACH_LUA_INTERFACE(CAIInitialReactionMan);
	CLuaGlobal::Get().State().GetGlobals().Set("CAIInitialReactionMan", this);
}

CAIInitialReactionMan::~CAIInitialReactionMan()
{
	m_vectorAIReactionInfo.clear();
}

//!--------------------------------------------
//! IsFirstReporterAlive
//!--------------------------------------------
bool CAIInitialReactionMan::IsFirstReporterAlive( void )	const
{ 
	return ( m_pAI_FirstReporter && !m_pAI_FirstReporter->IsDead() );
	//	return ( m_pSAIFirstReporter && m_pSAIFirstReporter->pAI && !m_pSAIFirstReporter->pAI->IsDead() ); 
}

//!--------------------------------------------
//! AddAI
//!--------------------------------------------
void CAIInitialReactionMan::AddAI ( CEntity* pNewEnt )
{
	if (!pNewEnt || !pNewEnt->IsAI() ) 
		return;

	AI* pNewAI = pNewEnt->ToAI();
	
	ntstd::Vector<SAIReactionInfo, Mem::MC_AI>::const_iterator obIt		= m_vectorAIReactionInfo.begin();
	ntstd::Vector<SAIReactionInfo, Mem::MC_AI>::const_iterator obEndIt	= m_vectorAIReactionInfo.end();
	
	for ( ; obIt != obEndIt; ++obIt )
	{
		SAIReactionInfo sAIInfo = (*obIt);
		
		if (sAIInfo.pAI == pNewAI)
		{
			// This AI has already been asigned as a participant
			ntPrintf("CAIInitialReactionMan: AI [%s] has already been asigned as a participant\n",ntStr::GetString(pNewAI->GetName()));
			return;
		}
	}

	// New AI
	SAIReactionInfo	sNewAIInfo = SAIReactionInfo(pNewAI);
	m_vectorAIReactionInfo.push_back(sNewAIInfo);

	// Clear The First Reporter Field 
	m_pAI_FirstReporter = NULL;
	m_pSAIFirstReporter = NULL;
}

//!--------------------------------------------
//! RemoveAI
//!--------------------------------------------
void CAIInitialReactionMan::RemoveAI ( CEntity* pEnt )
{
	if (!pEnt || !pEnt->IsAI() || m_vectorAIReactionInfo.empty() ) 
		return;

	AI* pAI = pEnt->ToAI();
	
	ntstd::Vector<SAIReactionInfo, Mem::MC_AI>::iterator obIt		= m_vectorAIReactionInfo.begin();
	ntstd::Vector<SAIReactionInfo, Mem::MC_AI>::iterator obEndIt	= m_vectorAIReactionInfo.end();

	for ( ; obIt != obEndIt; ++obIt )
	{
		SAIReactionInfo sAIInfo = (*obIt);
		
		if (sAIInfo.pAI == pAI)
		{
			// Remove AI
			m_vectorAIReactionInfo.erase(obIt);

			// book keeping the first reporter
			if (m_vectorAIReactionInfo.empty())
			{
				m_pAI_FirstReporter = NULL;
				m_pSAIFirstReporter = NULL;
			}
			return;
		}
	}

	// This AI is not a participant
	ntPrintf("CAIInitialReactionMan: AI [%s] is not a particpant\n",ntStr::GetString(pAI->GetName()));
	return;
	
}

//!--------------------------------------------
//! Report
//!--------------------------------------------
void CAIInitialReactionMan::Report ( AI* pAIReporter )
{
	if (!pAIReporter) return;

	ntstd::Vector<SAIReactionInfo, Mem::MC_AI>::const_iterator obIt		= m_vectorAIReactionInfo.begin();
	ntstd::Vector<SAIReactionInfo, Mem::MC_AI>::const_iterator obEndIt	= m_vectorAIReactionInfo.end();
	
	for ( ; obIt != obEndIt; ++obIt )
	{
		SAIReactionInfo sAIInfo = (*obIt);
		
		if (sAIInfo.pAI == pAIReporter)
		{
			// Is the first reporter?
			if (m_pAI_FirstReporter == NULL)
			{
				sAIInfo.bFirstReporter = true;
				m_pAI_FirstReporter = pAIReporter;
				m_pSAIFirstReporter = &sAIInfo;
			}
			return;
		}
	}

	// This AI has already been asigned as a participant
	ntPrintf("CAIInitialReactionMan: AI [%s] has reported but does not belong to the participant's list\n", ntStr::GetString(pAIReporter->GetName()));
}

//!--------------------------------------------
//! IsParticipant
//!--------------------------------------------
bool CAIInitialReactionMan::IsParticipant ( AI* pAI )
{
	if (!pAI) return false;

	ntstd::Vector<SAIReactionInfo, Mem::MC_AI>::const_iterator obIt		= m_vectorAIReactionInfo.begin();
	ntstd::Vector<SAIReactionInfo, Mem::MC_AI>::const_iterator obEndIt	= m_vectorAIReactionInfo.end();
	
	for ( ; obIt != obEndIt; ++obIt )
	{
		SAIReactionInfo sAIInfo = (*obIt);
		
		if (sAIInfo.pAI == pAI)
		{
			return true;
		}
	}
	return false;
}

//!--------------------------------------------
//! UpdateClosestAIToSound
//!--------------------------------------------
CEntity* CAIInitialReactionMan::UpdateClosestAIToSound	( const CPoint & obPos )
{
	if ( m_pClosestEntToSound )
		return m_pClosestEntToSound;

	float fMinDistSQR = FLT_MAX;
	SAIReactionInfo obRefAIClosest;

	ntstd::Vector<SAIReactionInfo, Mem::MC_AI>::const_iterator obIt		= m_vectorAIReactionInfo.begin();
	ntstd::Vector<SAIReactionInfo, Mem::MC_AI>::const_iterator obEndIt	= m_vectorAIReactionInfo.end();
	
	for ( ; obIt != obEndIt; ++obIt )
	{
		SAIReactionInfo sAIInfo = (*obIt);
		
		AI* pEnt = sAIInfo.pAI;
		unsigned int eNavInt = CAINavigationSystemMan::Get().GetNavigationIntentions(pEnt);
		if ( pEnt->IsPaused() || !pEnt->IsAI() || eNavInt != NAVINT_INITIAL_REACTION || pEnt->IsDead() )
			continue;

		CAIHearing* pHear = pEnt->GetAIComponent()->GetAIHearing();
		
		if ( !pHear || pHear->IsDeaf() ) 
 			continue;

		CDirection Line(obPos - pEnt->GetPosition());
		float fDistSQR = Line.LengthSquared();

		if (fMinDistSQR > fDistSQR)
		{
			fMinDistSQR = fDistSQR;
			obRefAIClosest = sAIInfo;
		}
	}

	obRefAIClosest.bClosestToSound = true;
	m_pClosestEntToSound = obRefAIClosest.pAI;
	return m_pClosestEntToSound;
}

//!--------------------------------------------
//! PropagateAlertSound
//!--------------------------------------------
void CAIInitialReactionMan::PropagateAlertSound( CEntity* pSoundSource )
{
	if (!pSoundSource)
		return;

	CPoint obSoundPos = pSoundSource->GetPosition();
	
	ntstd::Vector<SAIReactionInfo, Mem::MC_AI>::const_iterator obIt		= m_vectorAIReactionInfo.begin();
	ntstd::Vector<SAIReactionInfo, Mem::MC_AI>::const_iterator obEndIt	= m_vectorAIReactionInfo.end();
	
	for ( ; obIt != obEndIt; ++obIt )
	{
		SAIReactionInfo sAIInfo = (*obIt);
		AI* pEnt = sAIInfo.pAI;

		unsigned int eNavInt = CAINavigationSystemMan::Get().GetNavigationIntentions(pEnt);
		if ( pEnt->IsPaused() || !pEnt->IsAI() || eNavInt != NAVINT_INITIAL_REACTION || pEnt->ToAI()->IsDead() )
			continue;

		CAIHearing* pHear = pEnt->ToAI()->GetAIComponent()->GetAIHearing();
		
		if ( !pHear || pHear->IsDeaf() ) 
 			continue;

		pHear->SetSoundSourceEntity(pSoundSource);
		pHear->SetSoundSourceLocation(obSoundPos);
	}
}


//!--------------------------------------------
//! DebugRender
//!--------------------------------------------
void CAIInitialReactionMan::DebugRender ( void )
{
#ifndef _GOLD_MASTER
	#if 0
		if (!m_bInitialReactionRender || m_listAIParticipants.empty()) return;

		static float fRadius = 0.0f;
		
		fRadius+=0.1f;
		if (fRadius>1.5f) fRadius = 0.0f;

		ntstd::List<AI*, Mem::MC_AI>::const_iterator obIt		= m_listAIParticipants.begin();
		ntstd::List<AI*, Mem::MC_AI>::const_iterator obEndIt	= m_listAIParticipants.end();

		for ( ; obIt != obEndIt; ++obIt )
		{
			AI* pAI = *obIt;

			// Render AI 
			unsigned int uiColor = (pAI == m_pAI_FirstReporter) ? DC_RED : DC_CYAN;
			CMatrix ArcMatrix(CONSTRUCT_IDENTITY);
			ArcMatrix.SetTranslation(pAI->GetPosition()+CPoint(0.0f,0.1f,0.0f));
			g_VisualDebug->RenderArc(ArcMatrix, fRadius , TWO_PI,  uiColor);
		}
	#endif

	if (!m_bInitialReactionRender || !m_pAI_FirstReporter) 
		return;

	static float fRadius = 0.0f;
		
	fRadius+=0.1f;
	if (fRadius>1.5f) fRadius = 0.0f;

	CMatrix ArcMatrix(CONSTRUCT_IDENTITY);
	ArcMatrix.SetTranslation(m_pAI_FirstReporter->GetPosition()+CPoint(0.0f,0.1f,0.0f));
	g_VisualDebug->RenderArc(ArcMatrix, fRadius , TWO_PI,  DC_RED);

#endif
}

