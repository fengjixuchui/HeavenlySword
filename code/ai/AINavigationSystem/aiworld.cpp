//! -------------------------------------------
//! AIWorld.h
//!
//! World description for AIs
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!--------------------------------------------

#include "aiworld.h"
#include "aiworldvolumes.h"
#include "core/visualdebugger.h"
#include "game/shellconfig.h"
#include "ainavigsystemman.h" // for debugrender

#define LONG_DIVE_LENGTH (3.0f)
#define SHORT_DIVE_LENGTH (1.5f)

#define DEBUG_AI_DIVING_ON (g_ShellOptions->m_bDivingDebug)
#define DEBUG_PRINT_AI_DIVING(condition_msg) if (g_ShellOptions->m_bDivingDebug) { Debug::Printf("DIVE: "); Debug::Printf condition_msg; }

//!--------------------------------------------------
//! IntersectsSegments
//! -- Used for Wall detection by CAISteeringLibrary
//!--------------------------------------------------
void CAIWorldMan::RemoveVolume (CAIWorldVolume* pWV)
{
	if (!pWV)
		return;
	
	if (pWV->IsGoAroundVolume())
        m_listpGoAroundVolumes.remove(pWV);	

	m_listpVolumes.remove(pWV);

	// Currently containment regions are stored in m_listpVolumes. This may change
}

//!--------------------------------------------------
//! IntersectsSegments
//! -- Used for Wall detection by CAISteeringLibrary
//!--------------------------------------------------
bool CAIWorldMan::IntersectsAIVolume( const CPoint& obFrom, const CPoint& obTo, CPoint* pPoint, CDirection* pNormal ) const
{
	bool bRet = false;

	// Calculate Ckeck-line length
	CDirection LineFromTo = CDirection(obFrom-obTo);
	float fCheckRadius = LineFromTo.Length();

	AIWorldVolumeList::const_iterator obIt		= m_listpVolumes.begin();
	AIWorldVolumeList::const_iterator obEndIt	= m_listpVolumes.end();

	for( ; obIt != obEndIt; ++obIt )
	{
		if (!(*obIt)->IsInActiveSector())
			continue;
		if (!(*obIt)->IsInProcessingDistance(obFrom,fCheckRadius))
			continue;
		bRet = (*obIt)->IntersectsAIVolume(obFrom,obTo,pPoint,pNormal);
		
		if (bRet) return true; // !!! - (Dario) It returns ONLY the First AI vol.
	}
	return false;
}

//!--------------------------------------------------
//! IntersectsVaultingVolume
//! -- 
//!--------------------------------------------------
const CAIWorldVolume* CAIWorldMan::IntersectsVaultingVolume( const CPoint& obFrom, const CPoint& obTo, CPoint* pPoint, CDirection* pNormal ) const
{
	bool bRet = false;

	// Calculate Ckeck-line length
	CDirection LineFromTo = CDirection(obFrom-obTo);
	float fCheckRadius = LineFromTo.Length();

	AIWorldVolumeList::const_iterator obIt		= m_listpVaultingVolumes.begin();
	AIWorldVolumeList::const_iterator obEndIt	= m_listpVaultingVolumes.end();

	for( ; obIt != obEndIt; ++obIt )
	{
		CAIWorldVolume* pVV = (*obIt);
		if (!pVV->IsInActiveSector())
			continue;
		if (!pVV->IsInProcessingDistance(obFrom,fCheckRadius))
			continue;
		bRet = pVV->IntersectsAIVolume(obFrom,obTo,pPoint,pNormal);
		
		if (bRet) 
			return pVV; // !!! - (Dario) It returns ONLY the First AI vol.
	}
	return NULL;
}

//!--------------------------------------------------
//! CloseToVaultingVolume
//! -- 
//!--------------------------------------------------
const CAIWorldVolume* CAIWorldMan::CloseToVaultingVolume( const CPoint& obFrom, float fRadius ) const
{
	// Create some iterators...
	AIWorldVolumeList::const_iterator obIt		= m_listpVaultingVolumes.begin();
	AIWorldVolumeList::const_iterator obEndIt	= m_listpVaultingVolumes.end();

	for( ; obIt != obEndIt; ++obIt )
	{
		CAIWorldVolume* pVV = (*obIt);
		if ( pVV->IsInProcessingDistance(obFrom, fRadius) )
			return pVV;
	}

	return NULL;
}

////!--------------------------------------------
////! HasLineOfSight
////!--------------------------------------------
//bool CAIWorldMan::HasLineOfSight( const CPoint& obFrom, const CPoint& obTo ) const
//{
//	bool bRet = false;
//
//	CPoint pPoint;
//	CDirection pNormal;
//
//	AIWorldVolumeList::const_iterator obIt		= m_listpVolumes.begin();
//	AIWorldVolumeList::const_iterator obEndIt	= m_listpVolumes.end();
//
//	for( ; obIt != obEndIt; ++obIt )
//	{
//		if ((*obIt)->IsTransparent()) continue;
//
//		bRet = (*obIt)->PreventsLineofSight(obFrom,obTo);
//		if (bRet) return false;
//	}
//	return true;
//} 

//!--------------------------------------------
//! HasLineOfSight
//!--------------------------------------------
bool CAIWorldMan::HasLineOfSight( const CPoint& obFrom, const CPoint& obTo, float fThreshold ) const
{
	bool bRet = false;

	CPoint		pPoint;
	CDirection	pNormal;
	CPoint		obSafeTo;

	if (fThreshold>0.1f)
	{
		// A valid threshold is selected
		CDirection	Dir(obTo - obFrom);
		float		fDirLength = Dir.Length();
		Dir.Normalise();
		Dir *= (fDirLength + fThreshold);

		obSafeTo = obFrom+Dir;
		
#ifndef _GOLD_MASTER
		if (CAINavigationSystemMan::Get().m_bRenderWallAvoidance)
		{
 			g_VisualDebug->RenderLine(obTo, obSafeTo , DC_BLUE);
			g_VisualDebug->RenderPoint(obTo, 10 , DC_WHITE);
			g_VisualDebug->RenderPoint(obSafeTo, 10 , DC_GREEN);
		}
#endif
	}
	else
	{
		// No threshold
		obSafeTo = obTo;
	}

	// Calculate Ckeck-line length
	CDirection LineFromTo = CDirection(obFrom-obSafeTo);
	float fCheckRadius = LineFromTo.Length();

	AIWorldVolumeList::const_iterator obIt		= m_listpVolumes.begin();
	AIWorldVolumeList::const_iterator obEndIt	= m_listpVolumes.end();

	for( ; obIt != obEndIt; ++obIt )
	{
		//if ((*obIt)->IsTransparent()) continue;
		if (!(*obIt)->IsInActiveSector())
			continue;
		if (!(*obIt)->IsInProcessingDistance(obFrom,fCheckRadius))
			continue;
		
		bRet = (*obIt)->PreventsLineofSight(obFrom,obSafeTo);
		if (bRet == true) return false;
	}
	return true;
}

//!--------------------------------------------
//! HasShootingLineOfSight
//!--------------------------------------------
bool CAIWorldMan::HasShootingLineOfSight( const CPoint& obFrom, const CPoint& obTo, float fThreshold ) const
{
	bool bRet = false;

	CPoint		pPoint;
	CDirection	pNormal;
	CPoint		obSafeTo;

	if (fThreshold>0.1f)
	{
		// A valid threshold is selected
		CDirection	Dir(obTo - obFrom);
		float		fDirLength = Dir.Length();
		Dir.Normalise();
		Dir *= (fDirLength + fThreshold);

		obSafeTo = obFrom+Dir;
		
#ifndef _GOLD_MASTER
		if (CAINavigationSystemMan::Get().m_bRenderWallAvoidance)
		{
 			g_VisualDebug->RenderLine(obTo, obSafeTo , DC_BLUE);
			g_VisualDebug->RenderPoint(obTo, 10 , DC_WHITE);
			g_VisualDebug->RenderPoint(obSafeTo, 10 , DC_GREEN);
		}
#endif
	}
	else
	{
		// No threshold
		obSafeTo = obTo;
	}

	// Calculate Ckeck-line length
	CDirection LineFromTo = CDirection(obFrom-obSafeTo);
	float fCheckRadius = LineFromTo.Length();

	AIWorldVolumeList::const_iterator obIt		= m_listpVolumes.begin();
	AIWorldVolumeList::const_iterator obEndIt	= m_listpVolumes.end();

	for( ; obIt != obEndIt; ++obIt )
	{
		if (!(*obIt)->IsInActiveSector())
			continue;
		if ((*obIt)->IsTransparent()) 
			continue;
		if (!(*obIt)->IsInProcessingDistance(obFrom,fCheckRadius))
			continue;

		bRet = (*obIt)->PreventsLineofSight(obFrom,obSafeTo);
		if (bRet == true) return false;
	}
	return true;
}

//!------------------------------------------------
//! HasLineOfSightExcludingGoAroundVolumes
//!		Normal LineOfSight Checking BUT EXCLUDING
//!		the GoAround Volumes
//!------------------------------------------------
bool CAIWorldMan::HasLineOfSightExcludingGoAroundVolumes( const CPoint& obFrom, const CPoint& obTo ) const
{
	bool bRet = false;

	// Calculate Ckeck-line length
	CDirection LineFromTo = CDirection(obFrom-obTo);
	float fCheckRadius = LineFromTo.Length();

	AIWorldVolumeList::const_iterator obIt		= m_listpVolumes.begin();
	AIWorldVolumeList::const_iterator obEndIt	= m_listpVolumes.end();

	for( ; obIt != obEndIt; ++obIt )
	{
		if (!(*obIt)->IsInActiveSector())
			continue;
		if ((*obIt)->IsGoAroundVolume()) 
			continue;
		if (!(*obIt)->IsInProcessingDistance(obFrom,fCheckRadius))
			continue;
		bRet = (*obIt)->PreventsLineofSight(obFrom,obTo);
		if (bRet == true) return false;
	}
	return true;
}

//!--------------------------------------------
//! HasLineOfSightThroughGoAroundVolumes
//!		Checks if the line of sight between 
//!		obFrom and obTo intersects any
//!		GoAround Volume.
//!--------------------------------------------
bool CAIWorldMan::HasLineOfSightThroughGoAroundVolumes( const CPoint& obFrom, const CPoint& obTo ) const
{
	bool bRet = false;

	AIWorldVolumeList::const_iterator obIt		= m_listpGoAroundVolumes.begin();
	AIWorldVolumeList::const_iterator obEndIt	= m_listpGoAroundVolumes.end();

	// Calculate Ckeck-line length
	CDirection LineFromTo = CDirection(obFrom-obTo);
	float fCheckRadius = LineFromTo.Length();

	for( ; obIt != obEndIt; ++obIt )
	{
		CAIWorldVolume* pWV = (*obIt);
		if (!pWV->IsInActiveSector())
			continue;
		if (!(*obIt)->IsInProcessingDistance(obFrom,fCheckRadius))
			continue;
		bRet = pWV->PreventsLineofSight(obFrom,obTo);
		if (bRet == true) return true;
	}
	return false;
}

//!--------------------------------------------
//! GetClosestGoAroundVolumeOnTheWay
//!		Returns the closest GoAround Volume 
//!		in the line of sight (obFrom - obTo)
//!--------------------------------------------
CAIWorldVolume* CAIWorldMan::GetClosestGoAroundVolumeOnTheWay( const CPoint& obFrom, const CPoint& obTo ) const
{
	CPoint		obClosestVolumePos;
	float		fMinDist = 99999.9f; // Very biiiiigggggg
	CAIWorldVolume* pClosestWV = NULL;

	// Calculate Ckeck-line length
	CDirection LineFromTo = CDirection(obFrom-obTo);
	float fCheckRadius = LineFromTo.Length();

	AIWorldVolumeList::const_iterator obIt		= m_listpGoAroundVolumes.begin();
	AIWorldVolumeList::const_iterator obEndIt	= m_listpGoAroundVolumes.end();

	for( ; obIt != obEndIt; ++obIt )
	{
		CAIWorldVolume* pWV = (*obIt);
		ntAssert_p(pWV,("CAIWorldMan:GetClosestGoAroundVolumeOnTheWay -> NULL Volume at pos in m_listpGoAroundVolumes LIST!!!!"));

		if (!(*obIt)->IsInActiveSector())
			continue;
		if (!(*obIt)->IsInProcessingDistance(obFrom,fCheckRadius))
			continue;

		if (pWV->PreventsLineofSight(obFrom,obTo))
		{
			if (!pClosestWV)
			{
				// This is the first one
				obClosestVolumePos = pWV->GetCentre();
				CPoint p3d = obFrom - obClosestVolumePos;
				fMinDist = p3d.LengthSquared();
				pClosestWV = pWV;
			}
			else
			{
				CPoint p3d = obFrom - pWV->GetCentre();
				float fDist = p3d.LengthSquared();
				if (fDist < fMinDist)
				{
					fMinDist = fDist;
					obClosestVolumePos = pWV->GetCentre();
					pClosestWV = pWV;
				}
			}
		}
	}
	return pClosestWV;
}

//!--------------------------------------------
//! DebugRender
//!--------------------------------------------
void CAIWorldMan::DebugRender(void)
{
	AIWorldVolumeList::const_iterator obIt		= m_listpVolumes.begin();
	AIWorldVolumeList::const_iterator obEndIt	= m_listpVolumes.end();

	for( ; obIt != obEndIt; ++obIt )
	{
		if (!(*obIt)->IsInActiveSector())
			continue;
		(*obIt)->DebugRender();
	}
}

//!--------------------------------------------
//! GetVolumeByName
//!--------------------------------------------
CAIWorldVolume*	CAIWorldMan::GetVolumeByName( const char* pcName )
{
	if (!pcName)
		return NULL;

	AIWorldVolumeList::const_iterator obIt		= m_listpVolumes.begin();
	AIWorldVolumeList::const_iterator obEndIt	= m_listpVolumes.end();

	for( ; obIt != obEndIt; ++obIt )
	{
		CAIWorldVolume* pVol = (*obIt);
		if ( strcmp(pcName, pVol->GetName()) == 0 )
			return pVol;
	}

	return NULL;
}

//!--------------------------------------------
//! Constructor
//!--------------------------------------------
CAIWorldMan::CAIWorldMan()
{
	;
}

//!--------------------------------------------
//! TestDive_AICollision
//!--------------------------------------------
CEntity* CAIWorldMan::TestDive_AICollision( CEntity* pEnt, ENUM_DIVE_FULL_DIRECTION eDiveDir )
{
	if (!pEnt)
		return NULL;

	float fDiveLength = 0.0f;
	float fSign = 1.0f;

	switch (eDiveDir)
	{
		case E_DIVE_LONG_LEFT	: fDiveLength = LONG_DIVE_LENGTH; fSign = -1.0f; break;
		case E_DIVE_LONG_RIGHT	: fDiveLength = -LONG_DIVE_LENGTH; fSign = 1.0f; break;
		case E_DIVE_SHORT_LEFT	: fDiveLength = SHORT_DIVE_LENGTH; fSign = -1.0f; break;
		case E_DIVE_SHORT_RIGHT	: fDiveLength = -SHORT_DIVE_LENGTH; fSign = 1.0f; break;
		
		default : user_warn_msg (( "(Warning!) TestDive_AICollision: invalid eDiveDir : [%d]. Returning NULL.\n",eDiveDir  )); return NULL;
	}

	static const float fHalfFrontDistance = 0.8f;
	static const float fPlayerHight = 2.0f;
	static const float fSafetyY = fPlayerHight*0.5f;
	CPoint obPoint[4];
	int	   iIndex = 0;

	CDirection	Heading = pEnt->GetMatrix().GetZAxis();
	CDirection	SideDir = pEnt->GetMatrix().GetXAxis();
	CPoint		AIPos	= pEnt->GetPosition();

	// Create Diving Segments

	obPoint[0] = CPoint(AIPos +fSign*Heading*fHalfFrontDistance + CPoint(0,fSafetyY,0));
	obPoint[3] = CPoint(AIPos -fSign*Heading*fHalfFrontDistance + CPoint(0,fSafetyY,0));
	obPoint[1] = CPoint(obPoint[0] + SideDir*fDiveLength);
	obPoint[2] = CPoint(obPoint[3] + SideDir*fDiveLength);

	CPoint PointA(CONSTRUCT_CLEAR);
	CDirection	Normal;
	CDirection	Face;

	for (int i = 1; i<5; i++)
	{
		PointA = obPoint[i-1];

		iIndex = i<4 ? i : 0;

		Face	= CDirection (obPoint[iIndex]-PointA);
		Normal	= CAINavigationSystemMan::Get().GetPerpendicular(Face);
		m_Seg[i-1].P0 = PointA;
		m_Seg[i-1].P1 = obPoint[iIndex];
		m_Seg[i-1].Normal = Normal;
		m_Seg[i-1].Dir = Face;

#ifndef _GOLD_MASTER
		if (DEBUG_AI_DIVING_ON)
		{
			g_VisualDebug->RenderLine(obPoint[iIndex], PointA , DC_BLUE);
		}
#endif
	}

	// ===================================================================
	// Loop through the list of AIs and check if there is any one inside
	// ===================================================================

	QueryResultsContainerType* pLocalAIList = CAINavigationSystemMan::Get().GetAIList();

	if (!pLocalAIList)
		return NULL;

	QueryResultsContainerType::iterator obIt	= pLocalAIList->begin();
	QueryResultsContainerType::iterator obEnd	= pLocalAIList->end();
	for ( ; obIt != obEnd; ++obIt )
	{
		CEntity* pEntAI = (*obIt);
		CPoint EntAIPos = pEntAI->GetPosition();
 
		// Don't count PAUSED, DEAD or MYSELF
		if (	pEntAI->ToCharacter()->IsPaused() ||
				pEntAI->ToCharacter()->IsDead() ||
				pEntAI == pEnt
			)
		{
			continue;
		}

		// ===================================================================
		// Check if it is inside ...
		// ===================================================================

		bool bInside = false;

		for ( unsigned int i = 0; i<4; ++i )
		{
			// Check first the Y axis
			if ( (EntAIPos.Y() < (m_Seg[i].P0.Y() - fPlayerHight) ) || ( EntAIPos.Y() > (m_Seg[i].P0.Y()+fPlayerHight ) ) ) 
			{
				bInside = false;
				break;
			}

			// Check the dot product sign
			CDirection	Line2Point	(EntAIPos - m_Seg[i].P0);
			
			if ( Line2Point.Dot(m_Seg[i].Normal)>0 )	
			{
				bInside = false;
				break;
			}

			// As far as this vertex is concerned, the point is inside. Keep on looping.
			bInside = true;
		}

		if (bInside)
			return pEntAI;

		// Not inside... try the next AI
	}

	return NULL;
}

//!--------------------------------------------
//! TestDive_AICollision
//!--------------------------------------------
bool CAIWorldMan::TestDive_NoWallCollision( CEntity* pEnt, ENUM_DIVE_FULL_DIRECTION eDiveDir )
{
	if (!pEnt)
		return NULL;

	CDirection	Heading = pEnt->GetMatrix().GetZAxis();
	CDirection	SideDir = pEnt->GetMatrix().GetXAxis();
	CPoint		AIPos	= pEnt->GetPosition();

	float fDiveLength = 0.0f;
	float fSign = 1.0f;

	switch (eDiveDir)
	{
		case E_DIVE_LONG_LEFT	: fDiveLength = LONG_DIVE_LENGTH; fSign = -1.0f; break;
		case E_DIVE_LONG_RIGHT	: fDiveLength = -LONG_DIVE_LENGTH; fSign = 1.0f; break;
		case E_DIVE_SHORT_LEFT	: fDiveLength = SHORT_DIVE_LENGTH; fSign = -1.0f; break;
		case E_DIVE_SHORT_RIGHT	: fDiveLength = -SHORT_DIVE_LENGTH; fSign = 1.0f; break;
		
		default : user_warn_msg (( "(Warning!) TestDive_AICollision: invalid eDiveDir : [%d]. Returning NULL.\n",eDiveDir  )); return NULL;
	}

	CPoint obDiveEndPos	= CPoint(AIPos+SideDir*fDiveLength);
	bool bRet = HasLineOfSight(AIPos,obDiveEndPos);

	if ( DEBUG_AI_DIVING_ON && bRet )
	{
		DEBUG_PRINT_AI_DIVING(( "Diver AI: [%s] has (wall) espace for diving: [%s]\n",ntStr::GetString(pEnt->GetName()), eDiveDir==E_DIVE_LONG_LEFT ? "Long Left" : eDiveDir==E_DIVE_LONG_RIGHT ? "Long Right" : eDiveDir==E_DIVE_SHORT_LEFT ? "Short Left" : "Sort Right"  ));
	}

	return ( bRet );
}






