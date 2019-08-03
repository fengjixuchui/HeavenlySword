//! -------------------------------------------
//! aidiving.cpp
//!
//! Diving functionality for AIs avoiding bolts
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!--------------------------------------------

#include "aidiving.h"
#include "ainavigsystemman.h"
#include "game/shellconfig.h"
#include "game/attacks.h"
#include "core/visualdebugger.h"

#define MAX_NUMBER_OF_DIVERS 50

//#define BOLT_CONE_LENGTH		(20.0f)
//#define BOLT_CONE_HALF_RADIUS	(2.0f)

#define DEBUG_AI_DIVING_ON (g_ShellOptions->m_bDivingDebug)
#define DEBUG_PRINT_AI_DIVING(condition_msg) if (g_ShellOptions->m_bDivingDebug) { Debug::Printf("DIVE: "); Debug::Printf condition_msg; }


// -------------------------------------------
// Constructor
// -------------------------------------------
CAIDivingMan::CAIDivingMan() :  m_fDebugConeLength(20.0f), m_fDebugConeHalfRadius(2.0f)
{
	m_vDiversData.reserve(MAX_NUMBER_OF_DIVERS);
}

// -------------------------------------------
// Destructor
// -------------------------------------------
CAIDivingMan::~CAIDivingMan()
{
	// Clean the DiversData
	ntstd::Vector<SDiversData*, Mem::MC_AI>::iterator obIt		= m_vDiversData.begin();
	//ntstd::Vector<SDiversData*, Mem::MC_AI>::iterator obItEnd	= m_vDiversData.end();
	for ( ; obIt != m_vDiversData.end(); )
	{
		SDiversData* pDD = *obIt;
		NT_DELETE_CHUNK(Mem::MC_AI,pDD);
		//obIt = m_vDiversData.erase(obIt);
	}
	m_vDiversData.clear();
}

// -------------------------------------------
// RemoveBolt
// -------------------------------------------
void CAIDivingMan::RemoveBolt (Object_Projectile* pBolt)
{
	if (!pBolt || m_listBoltsData.empty())
		return;

	ListBoltsData::iterator obIt	= m_listBoltsData.begin();
	ListBoltsData::iterator obEnd	= m_listBoltsData.end();

	for ( ; obIt != obEnd; ++obIt )
	{
		SBoltData* pBD = (*obIt);

		if (!pBD || !pBD->pBolt)
			continue;

		Object_Projectile* pThisBolt = pBD->pBolt;
		
		if (pBolt == pThisBolt)
		{
			DEBUG_PRINT_AI_DIVING(("Removing Bolt [%s] from BoltList (list size: %d)\n",ntStr::GetString(((CEntity*)pThisBolt)->GetName()), m_listBoltsData.size()));
			ClearDiversDataWithBolt(pThisBolt);
			NT_DELETE_CHUNK(Mem::MC_AI, pBD);
			m_listBoltsData.erase(obIt);
			return;
		}
	}
	DEBUG_PRINT_AI_DIVING(("RemoveBolt FAILED. Bolt [%s] was not found in BoltList (list size: %d)\n",ntStr::GetString(((CEntity*)pBolt)->GetName()), m_listBoltsData.size()));
}

// -------------------------------------------
// RemoveBolt
// -------------------------------------------
void CAIDivingMan::AddBolt	(Object_Projectile* pBolt)
{ 
	if (pBolt)
	{
		SBoltData* pBD = NT_NEW_CHUNK(Mem::MC_AI) _SBoltData( pBolt );
		m_listBoltsData.push_back(pBD); 
		DEBUG_PRINT_AI_DIVING(("Adding Bolt [%s] to BoltList (list size: %d)\n",ntStr::GetString(((CEntity*)pBolt)->GetName()), m_listBoltsData.size()));
	}
}

// -------------------------------------------
// Update
// -------------------------------------------
void CAIDivingMan::Update (float fTimeChange)
{
	static const float fMaxBoltFlyingTime = 10.0f;
	//static const float fBoltInfluenceRadiusSQR = BOLT_CONE_LENGTH_SQR;
	//static const float fBoltInfluenceRadius	   = BOLT_CONE_LENGTH;
	static const float fPlayerHight = 2.0f;

	if (m_listBoltsData.empty())
		return;

	// ===================================================================
	// Remove of Old Flying Bolts
	// ===================================================================

	ListBoltsData::iterator obIt	= m_listBoltsData.begin();
	//ListBoltsData::iterator obEnd	= m_listBoltsData.end();

	for ( ; obIt != m_listBoltsData.end(); )
	{
		SBoltData* pBD = (*obIt);

		if (!pBD || !pBD->pBolt)
		{
			obIt = m_listBoltsData.erase(obIt);
			DEBUG_PRINT_AI_DIVING(("(Error!) Erased NULL Bolt from m_listBoltsData (list size : %d)\n", m_listBoltsData.size() ));
		}
		else
		{
			Object_Projectile* pThisBolt = pBD->pBolt;
			// Update Flying Time
			pBD->fTime += fTimeChange;

			if ( pBD->fTime > fMaxBoltFlyingTime )
			{
				// Time-out. Remove Bolt and Divers' Data
				ClearDiversDataWithBolt(pThisBolt);
				NT_DELETE_CHUNK(Mem::MC_AI, pBD);
				obIt = m_listBoltsData.erase(obIt);
				DEBUG_PRINT_AI_DIVING(("Removed Time-Up Bolt [%s] from BoltList (list size: %d)\n",ntStr::GetString(((CEntity*)pThisBolt)->GetName()), m_listBoltsData.size()));
			}
			else
			{
				++obIt;
			}
		}
	}

	// ===================================================================
	// Loop through the list of Flying Bolts
	// ===================================================================

	if (m_listBoltsData.empty())
		return;

	obIt	= m_listBoltsData.begin();
	//obEnd	= m_listBoltsData.end();

	for ( ; obIt != m_listBoltsData.end(); ++obIt )
	{
		SBoltData* pBD = (*obIt);
		if (!pBD || !pBD->pBolt)
			continue;

		Object_Projectile* pThisBolt = pBD->pBolt;
		CPoint obBoltPos = ((CEntity*)pThisBolt)->GetPosition();
		CDirection obBoltDir = ((CEntity*)pThisBolt)->GetMatrix().GetZAxis();

		// ===================================================================
		// Loop through the list of AIs and check if there is any one inside
		// ===================================================================

		QueryResultsContainerType* pLocalAIList = CAINavigationSystemMan::Get().GetAIList();

		if (!pLocalAIList)
			return;

		QueryResultsContainerType::iterator obIt	= pLocalAIList->begin();
		QueryResultsContainerType::iterator obEnd	= pLocalAIList->end();
		for ( ; obIt != obEnd; ++obIt )
		{
			CEntity* pEntAI = (*obIt);
			CAIMovement* pMov = ((AI*)pEntAI)->GetAIComponent()->GetCAIMovement();
			// Don't count PAUSED, DEAD (maybe NOT in CS_STANDARD...)
			if (	!pEntAI								||
					pEntAI->ToCharacter()->IsPaused()	||
					pEntAI->ToCharacter()->IsDead()		||
					!pMov->IsSimpleActionComplete()		||
					!pEntAI->GetAttackComponent()->AI_Access_IsInCSStandard()
				)
			{
				continue;
			}

			// -------------------------------------------
			// Check if the AI is inside the Bolt's cone
			// -------------------------------------------

			CPoint EntAIPos = pEntAI->GetPosition();
			CDirection	LineBoltAI(obBoltPos-EntAIPos);
			float		fDistSQR = LineBoltAI.LengthSquared();
			bool		bBoltInfluencesAI = fDistSQR < pMov->GetBoltConeLengthSQR();
			
			CPoint obBoltPerpend	= CPoint(pMov->GetBoltConeHalfRadius()*CAINavigationSystemMan::Get().GetPerpendicular(obBoltDir));
			CPoint obBoltFarCentre	= obBoltPos + CPoint( pMov->GetBoltConeLength()*obBoltDir );

			bool		bValidY =	( ( obBoltFarCentre.Y() > EntAIPos.Y() ) &&
									  ( obBoltFarCentre.Y() < EntAIPos.Y() + fPlayerHight ) );

			bBoltInfluencesAI = bBoltInfluencesAI && bValidY;

            if ( bBoltInfluencesAI )
			{
                CDirection LineBoltAI(obBoltPos-EntAIPos);

				m_BoltCone = _SBoltCone(	EntAIPos,
											obBoltPos,
											obBoltFarCentre + obBoltPerpend,
											obBoltFarCentre - obBoltPerpend
										);

				if (	m_BoltCone.P0_AI.Dot(m_BoltCone.Normal0) < 0.0f &&
						m_BoltCone.P1_AI.Dot(m_BoltCone.Normal1) < 0.0f &&
						m_BoltCone.P2_AI.Dot(m_BoltCone.Normal2) < 0.0f
					)
					bBoltInfluencesAI = true;
				else
					bBoltInfluencesAI = false;
			}

			ManageDiversData( pEntAI, pThisBolt, bBoltInfluencesAI);
			
		} // AI List Loop
	} // Bolt List Loop
}

// -------------------------------------------
// IsInTheDiversList
// -------------------------------------------
bool CAIDivingMan::IsInTheDiversList ( CEntity* pEntAI, bool* bDiscarded )
{
	*bDiscarded = false;

	if (!pEntAI || m_vDiversData.empty())
		return false;

	unsigned int uiVectorSize = m_vDiversData.size();

	for ( unsigned int i = 0; i < uiVectorSize; i++)
	{
		SDiversData* pDD = m_vDiversData[i];
		if (pDD->pEntAI == pEntAI)
		{
		//	DEBUG_PRINT_AI_DIVING(("Entity AI: [%s] found in Diver's Data List. List Size: (%d)\n",ntStr::GetString(pDD->pEntAI->GetName()),uiVectorSize ));
			*bDiscarded = pDD->bDiscarded;
			return true;
		}
	}
	return false;
}

// -------------------------------------------
// IsInTheDiversList
// -------------------------------------------
//unsigned int  CAIDivingMan::GetDivingSideToAvoidBolt ( CEntity* pEntAI )
//{
//	
//	Go through the list and find all the bolts related to the AI.
//	Then take the closest and calculate the proper direction to avoid the arrow.
//
//}

// -------------------------------------------
// GetDivingSideToAvoidBolt
// -------------------------------------------
unsigned int  CAIDivingMan::GetDivingSideToAvoidBolt ( CEntity* pEntAI )
{
	if (!pEntAI || m_listBoltsData.empty())
		return E_DONT_DIVE;

	CEntity* pEntityBolt = NULL;

	float fMinDistSQR = 999999.9f; // Big....
	CPoint AIPos = pEntAI->GetPosition();

	// ===================================================================
	// Find the closest Bolt
	// ===================================================================

	ListBoltsData::iterator obIt	= m_listBoltsData.begin();
	ListBoltsData::iterator obEnd	= m_listBoltsData.end();

	for ( ; obIt != obEnd; ++obIt)
	{
		SBoltData* pBD = (*obIt);

		if (!pBD || !pBD->pBolt)
			continue;
		
		CDirection LineBolt2AI(AIPos - ((CEntity*)(pBD->pBolt))->GetPosition());

		float fDistSQR = LineBolt2AI.LengthSquared();

		if ( fDistSQR < 99999.9f && fDistSQR < fMinDistSQR )
		{
			pEntityBolt = (CEntity*)(pBD->pBolt);
			fMinDistSQR = fDistSQR;
		}
	}

	if (!pEntityBolt)
		return E_DONT_DIVE;

	// ===================================================================
	// Decide how to dive
	// ===================================================================

	CDirection LineBolt2AI(AIPos - pEntityBolt->GetPosition());
	LineBolt2AI.Normalise();
	CDirection AIHeading = pEntAI->GetMatrix().GetZAxis();
//	CDirection AISideDir = pEntAI->GetMatrix().GetXAxis();
	CDirection BoltSideDir = pEntityBolt->GetMatrix().GetXAxis();

	float fLineDotHeading = LineBolt2AI.Dot(AIHeading);
//	float fLineDotSideDir = LineBolt2AI.Dot(AISideDir);
	float fLineDotBoltSide= LineBolt2AI.Dot(BoltSideDir);

	if ( fLineDotHeading > 0.0f )
		return E_DONT_DIVE;

	if ( fLineDotBoltSide < 0.0f )
			return (E_DIVE_LONG_LEFT | E_DIVE_SHORT_LEFT | E_DIVE_PANIC | E_DIVE_CROUCH);
		else
			return (E_DIVE_LONG_RIGHT | E_DIVE_SHORT_RIGHT | E_DIVE_PANIC | E_DIVE_CROUCH);

	//if ( fLineDotSideDir < 0.0f )
	//{
	//	// Shooting from the Right
	//	if ( fLineDotBoltSide < 0.0f )
	//		return (E_DIVE_LONG_LEFT | E_DIVE_SHORT_LEFT | E_DIVE_PANIC | E_DIVE_CROUCH);
	//	else
	//		return (E_DIVE_LONG_RIGHT | E_DIVE_SHORT_RIGHT | E_DIVE_PANIC | E_DIVE_CROUCH);
	//}
	//else
	//{
	//	// Shooting from the Left
	//	if ( fLineDotBoltSide < 0.0f )
	//		return (E_DIVE_LONG_LEFT | E_DIVE_SHORT_LEFT | E_DIVE_PANIC | E_DIVE_CROUCH);
	//	else
	//		return (E_DIVE_LONG_RIGHT | E_DIVE_SHORT_RIGHT | E_DIVE_PANIC | E_DIVE_CROUCH);
	//}

}

// -------------------------------------------
// IsInTheDiversList
// -------------------------------------------
void CAIDivingMan::ManageDiversData ( CEntity* pEntAI, Object_Projectile* pBolt, bool bInfluences )
{
	if (!pEntAI)
		return;

	if (!m_vDiversData.empty())
	{
		ntstd::Vector<SDiversData*, Mem::MC_AI>::iterator obIt		= m_vDiversData.begin();
		ntstd::Vector<SDiversData*, Mem::MC_AI>::iterator obItEnd	= m_vDiversData.end();
		
		for ( ; obIt != obItEnd; ++obIt)
		{
			SDiversData* pDD = *obIt;
			if (pDD->pEntAI == pEntAI && pDD->pBolt == pBolt)
			{
				if (!bInfluences)
				{
					// Remove Diver
					NT_DELETE_CHUNK(Mem::MC_AI,pDD);
					m_vDiversData.erase(obIt);
					DEBUG_PRINT_AI_DIVING(("ManageDiversData: Removing Data for Diver [%s]. Diver's Data List Size [%d]\n",ntStr::GetString(pEntAI->GetName()), m_vDiversData.size()  ));
					return;
				}
				else
				{
					// Leave Diver
					return;
				}
			}
		}
	}

	// It does not exist yet
	if (bInfluences)
	{
		// Add Diver
		SDiversData* pDD = NT_NEW_CHUNK(Mem::MC_AI) _SDiversData(pEntAI, pBolt);
		m_vDiversData.push_back(pDD);
		CAINavigationSystemMan::Get().ResetBoltCoverTimer();
		DEBUG_PRINT_AI_DIVING(("Entity AI: [%s] Added to the Diver's Data List. List Size: (%d)\n",ntStr::GetString(pEntAI->GetName()),m_vDiversData.size() ));
	}
	return;
}

// -------------------------------------------
// DisregardCurrentBolt
// -------------------------------------------
void CAIDivingMan::DisregardCurrentBolt ( CEntity* pEntAI )
{
	if (!pEntAI || m_vDiversData.empty())
		return;

	unsigned int uiVectorSize = m_vDiversData.size();

	for ( unsigned int i = 0; i < uiVectorSize; i++)
	{
		SDiversData* pDD = m_vDiversData[i];
		if (pDD->pEntAI == pEntAI)
		{
			pDD->bDiscarded = true;
			DEBUG_PRINT_AI_DIVING(("Entity AI: [%s] discarded Bolt: [%s]\n",ntStr::GetString(pDD->pEntAI->GetName()),ntStr::GetString(((CEntity*)pDD->pBolt)->GetName()) ));
			return;
		}
	}
	return;
}


// -------------------------------------------
// RemoveDiversWithBolt
// -------------------------------------------
void CAIDivingMan::ClearDiversDataWithBolt ( Object_Projectile* pBolt )
{
	if (!pBolt || m_vDiversData.empty())
		return;

	DEBUG_PRINT_AI_DIVING(("ClearDiversDataWithBolt: Clearing DiversData With Bolt [%s] from Diver's Data Vector (vector size: %d)\n",ntStr::GetString(((CEntity*)pBolt)->GetName()), m_vDiversData.size()));
	ntstd::Vector<SDiversData*, Mem::MC_AI>::iterator obIt		= m_vDiversData.begin();
	//ntstd::Vector<SDiversData*, Mem::MC_AI>::iterator obItEnd	= m_vDiversData.end();
	for ( ; obIt !=  m_vDiversData.end(); )
	{
		SDiversData* pDD = *obIt;

		if (pDD || pDD->pBolt == pBolt)
		{
			DEBUG_PRINT_AI_DIVING(("ClearDiversDataWithBolt: Removed Data for Diver [%s] (vector size: %d)\n",ntStr::GetString(pDD->pEntAI->GetName()), m_vDiversData.size()-1 ));
			NT_DELETE_CHUNK(Mem::MC_AI,pDD);
			obIt = m_vDiversData.erase(obIt);
		}
		else
		{
			++obIt;
		}
	}
}

// -------------------------------------------
// DebugRender
// -------------------------------------------
void CAIDivingMan::RemoveDiver( CEntity* pEnt )
{
	if (!pEnt || m_vDiversData.empty() )
		return;

	ntstd::Vector<SDiversData*, Mem::MC_AI>::iterator obIt		= m_vDiversData.begin();
//	ntstd::Vector<SDiversData*, Mem::MC_AI>::iterator obItEnd	= m_vDiversData.end();
	for ( ; obIt != m_vDiversData.end(); )
	{
		SDiversData* pDD = *obIt;

		if (pDD->pEntAI == pEnt)
		{
			DEBUG_PRINT_AI_DIVING(("RemoveDiver: Removed Data for Diver [%s]\n",ntStr::GetString(pDD->pEntAI->GetName()) ));
			NT_DELETE_CHUNK(Mem::MC_AI,pDD);
			obIt = m_vDiversData.erase(obIt);
		}
		else
		{
			++obIt;
		}
	}
}

// -------------------------------------------
// DebugRender
// -------------------------------------------
void CAIDivingMan::DebugRender( void )
{
#ifndef _GOLD_MASTER

	static const CQuat obOrientation( CONSTRUCT_IDENTITY );
	static const float fAIRadius = 1.5f;

	// Render BOLT info
	if (!m_listBoltsData.empty())
	{
		ListBoltsData::iterator obIt	= m_listBoltsData.begin();
		ListBoltsData::iterator obEnd	= m_listBoltsData.end();

		for ( ; obIt != obEnd; ++obIt )
		{
			SBoltData* pBD = (*obIt);
			Object_Projectile* pThisBolt = pBD->pBolt;
			CPoint obBoltPos = ((CEntity*)pThisBolt)->GetPosition();
			CDirection obBoltDir = ((CEntity*)pThisBolt)->GetMatrix().GetZAxis();
			pBD->fDebugRadius = pBD->fDebugRadius > 1.0f ? 0.1f : pBD->fDebugRadius+=0.1f;

			// Render Bolt
			g_VisualDebug->RenderSphere( obOrientation, obBoltPos, pBD->fDebugRadius, DC_RED, 4096 );
			g_VisualDebug->Printf3D( obBoltPos, DC_YELLOW, 0, "%s - %.3f", 	ntStr::GetString(((CEntity*)pThisBolt)->GetName()),
																			pBD->fTime 
																			);
			// Render Sphere of influence
				
			CPoint obBoltPerpend	= CPoint(m_fDebugConeHalfRadius*CAINavigationSystemMan::Get().GetPerpendicular(obBoltDir));
			CPoint obBoltFarCentre	= obBoltPos + CPoint(m_fDebugConeLength*obBoltDir);

		
			g_VisualDebug->RenderLine( obBoltPos, obBoltFarCentre + obBoltPerpend, DC_CYAN );
			g_VisualDebug->RenderLine( obBoltFarCentre + obBoltPerpend, obBoltFarCentre - obBoltPerpend, DC_CYAN );
			g_VisualDebug->RenderLine( obBoltFarCentre - obBoltPerpend, obBoltPos, DC_CYAN );
			
		}
	}

	// Render Divers' Info

	if (!m_vDiversData.empty())
	{
		ntstd::Vector<SDiversData*, Mem::MC_AI>::iterator obIt		= m_vDiversData.begin();
		ntstd::Vector<SDiversData*, Mem::MC_AI>::iterator obItEnd	= m_vDiversData.end();
		for ( ; obIt != obItEnd; ++obIt )
		{
			SDiversData* pDD = *obIt;
			
			if (!pDD->bDiscarded)
			{
				// Highlight Diver
				g_VisualDebug->RenderSphere( obOrientation, pDD->pEntAI->GetPosition(), fAIRadius, DC_CYAN, 4096 );
				g_VisualDebug->Printf3D( pDD->pEntAI->GetPosition(), DC_YELLOW, 0, "Diver: %s", 	ntStr::GetString(pDD->pEntAI->GetName()) );
			}
		}
	}
#endif
}



