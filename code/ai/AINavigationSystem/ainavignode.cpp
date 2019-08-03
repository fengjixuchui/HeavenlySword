//! -------------------------------------------
//! AINavigNode.cpp
//!
//! Node element for the AIs navigation graph 
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!--------------------------------------------

#include "ainavignode.h"
#include "ainavigsystemman.h"
#include "objectdatabase/dataobject.h"
#include "game/randmanager.h"
#include "aimovement.h"

#define SHORT_COVER_DIST (1.5f)

START_CHUNKED_INTERFACE(STCAnimSets,Mem::MC_AI)
	PUBLISH_CONTAINER_AS(listhsBreakLeftTCAnims		, BreakLeftAnims)
	PUBLISH_CONTAINER_AS(listhsBreakRightTCAnims	, BreakRightAnims)
	PUBLISH_CONTAINER_AS(listhsReturnLeftTCAnims	, ReturnLeftAnims)
	PUBLISH_CONTAINER_AS(listhsReturnRightTCAnims	, ReturnRightAnims)
	PUBLISH_CONTAINER_AS(listhsHideCrouchingTCAnims	, HideCrouchingAnims)
END_STD_INTERFACE

START_CHUNKED_INTERFACE(SCoverPointAnimSets,Mem::MC_AI)
	PUBLISH_CONTAINER_AS(listhsPeekFromCoverAnims	, PeekFromCoverAnims)
	PUBLISH_CONTAINER_AS(listhsBreakCoverAnims		, BreakCoverAnims)
	PUBLISH_CONTAINER_AS(listhsTakeCoverAnims		, TakeCoverAnims)
	PUBLISH_CONTAINER_AS(listhsInCoverAnims			, InCoverAnims)

	PUBLISH_CONTAINER_AS(listhsLongLeftTakeCoverAnims		, EnterCoverLongLeftAnims)
	PUBLISH_CONTAINER_AS(listhsShortLeftTakeCoverAnims		, EnterCoverShortLeftAnims)

	PUBLISH_CONTAINER_AS(listhsLongRearTakeCoverAnims		, EnterCoverLongRearAnims)
	PUBLISH_CONTAINER_AS(listhsShortRearTakeCoverAnims		, EnterCoverShortRearAnims)
	
	PUBLISH_CONTAINER_AS(listhsLongRightTakeCoverAnims		, EnterCoverLongRightAnims)
	PUBLISH_CONTAINER_AS(listhsShortRightTakeCoverAnims		, EnterCoverShortRightAnims)
END_STD_INTERFACE

START_CHUNKED_INTERFACE(CAINavigCoverPoint,Mem::MC_AI)
	PUBLISH_PTR_AS				(m_pSCoverPointAnimSets	, CoverAnimSet)
	PUBLISH_VAR_WITH_DEFAULT_AS	(m_obValidDir		, CDirection(-1.0f, 0.0f, 0.0f), CoverDirection)
	PUBLISH_VAR_WITH_DEFAULT_AS	(m_bUseValidDir		, true, UseCoverDirection)
	PUBLISH_VAR_WITH_DEFAULT_AS	(m_obPos			, CPoint(0.0f, 0.0f, 0.0f), Position)
	PUBLISH_VAR_WITH_DEFAULT_AS	(m_fRadiusSQR		, 0.5f, Radius)
	PUBLISH_VAR_WITH_DEFAULT_AS	(m_fChanceOfUse		, 0.5f, ChanceOfUse)
	PUBLISH_PTR_AS				(m_pobNavigGraph	, NavigRegion)
	PUBLISH_VAR_WITH_DEFAULT_AS	(m_bJumpOver		, false, JumpOver)
	DECLARE_POSTCONSTRUCT_CALLBACK(PostConstruct)
END_STD_INTERFACE

START_CHUNKED_INTERFACE(CAINavigNode,Mem::MC_AI)
	PUBLISH_PTR_CONTAINER_AS(m_listTargetArrows	, TargetArrows)
	PUBLISH_PTR_CONTAINER_AS(m_listSourceArrows	, SourceArrows)
	PUBLISH_PTR_CONTAINER_AS(m_listCoverArrows	, CoverArrows)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_obPos			, CPoint(0.0f, 0.0f, 0.0f), Position)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fNodeRadius	, 1.0f, Radius)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_iQueueIndex	, 0, QueueIndex)
//	PUBLISH_VAR_WITH_DEFAULT_AS(m_bIsQueueNode	, false, IsQueueNode)
	PUBLISH_PTR_AS			(m_pobNavigGraph	, NavigRegion)
	PUBLISH_PTR_AS			(m_pobGameEvent		, PatrolEvent)
	PUBLISH_PTR_AS			(m_pobEntityToUse	, UseObject)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fNoWallDetectRadiusSQR, 1.0f, NoWallRadiusSQR)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bDisableWallDetFlag, false, NoWallCollision)
	PUBLISH_PTR_AS			(m_pSTCAnimSets		, WhackAMoleAnimSets)
	PUBLISH_VAR_WITH_DEFAULT_AS	(m_obWhackAMoleDir	, CDirection(-1.0f, 0.0f, 0.0f), WhackAMoleDirection)
	PUBLISH_VAR_WITH_DEFAULT_AS	(m_fNodeWithPercentage	, -1.0f , NodeWidthPercentage)
	DECLARE_POSTCONSTRUCT_CALLBACK(PostConstruct)
END_STD_INTERFACE

START_CHUNKED_INTERFACE(CAINavigArrow,Mem::MC_AI)
	PUBLISH_PTR_AS			(m_pobSrcNode				, SourceNode)
	PUBLISH_PTR_AS			(m_pobTgtNode				, TargetNode)
	PUBLISH_PTR_CONTAINER_AS(m_listAcceptEntityTypes	, AcceptEntities)
	PUBLISH_PTR_CONTAINER_AS(m_listRejectEntityTypes	, RejectEntities)
	PUBLISH_PTR_CONTAINER_AS(m_listAcceptEntity			, AcceptEntity)
	PUBLISH_PTR_CONTAINER_AS(m_listRejectEntity			, RejectEntity)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fDistance				, 0.0f, Distance)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fWidth				, 0.0f, Width)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bIsEnabled			, true, IsEnabled)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bRejectTypeOn			, true, RejectOn)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bRejectEntityOn		, true, RejectTypeOn)
	PUBLISH_PTR_AS			(m_pobEntityToUse			, UseObject)

	PUBLISH_PTR_AS			(m_pobCoverPoint			, TargetCoverPoint)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bToCoverPoint			, false, LeadsToCover)
	DECLARE_POSTCONSTRUCT_CALLBACK(PostConstruct)
END_STD_INTERFACE

CAINavigNode::~CAINavigNode()
{
#ifndef _RELEASE
	ntPrintf("~CAINavigNode() : Node [%s]\n", ntStr::GetString(m_ksName));
#endif
}

CAINavigArrow::~CAINavigArrow()
{
#ifndef _RELEASE
	ntPrintf("~CAINavigArrow() : Arrow [%s]\n", ntStr::GetString(m_ksName));
#endif
}

void CAINavigArrow::PostConstruct()
{
	// Get the Arrows Name
	DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromPointer(this);
	if(pDO) m_ksName = CHashedString((pDO->GetName()).GetString());
}

//------------------------------------------------------------------------------------------
//	CAINavigNode::PostConstruct()
//------------------------------------------------------------------------------------------
void CAINavigNode::PostConstruct()
{
	// Get the Node Name
	DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromPointer(this);
	if(pDO) m_ksName = CHashedString((pDO->GetName()).GetString());
	
	// Calculate the RadiusSQR
	m_fNodeRadiusSQR = m_fNodeRadius*m_fNodeRadius;

	// Verify the Node Width Percentage 

	if (m_fNodeWithPercentage < 0.0f)
		m_fNodeWithPercentage = -1.0f;
	else if ( m_fNodeWithPercentage > 0.9f )
		m_fNodeWithPercentage = 0.9f;
}

//------------------------------------------------------------------------------------------
//	CAINavigCoverPoint::PostConstruct()
//------------------------------------------------------------------------------------------
void CAINavigCoverPoint::PostConstruct()
{
	// Get the Node Name
	DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromPointer(this);
	if(pDO) m_ksName = CHashedString((pDO->GetName()).GetString());
	
	// Calculate the RadiusSQR
	m_fRadiusSQR *=m_fRadiusSQR;
}

//! ------------------------------------------------------------------------------------------
//! void CAINavigNode::DetachSourceLinks( void )
//!
//! Detach Source Links so the caller class (AINavigGraph) can delete permanently this node
//! ------------------------------------------------------------------------------------------
void CAINavigNode::DetachSourceLinks( void )  
{
	// Go through the list of source nodes and delete their links to me

	AINavigArrowList::iterator obIt		= m_listSourceArrows.begin();
	AINavigArrowList::iterator obEndIt	= m_listSourceArrows.end();
	for( ; obIt != obEndIt; ++obIt )
	{
		CAINavigArrow* pobSrcArrow = (*obIt);
		if (!pobSrcArrow)
		{
			ntAssert_p (0,("DetachSourceLinks: NULL source arrow found in m_listSourceArrows\n"));
		}
		pobSrcArrow->GetSrcNode()->m_listTargetArrows.remove(pobSrcArrow);
	}
}

//! ----------------------------------------------------------
//! GetDistanceToPoint
//! !!! - May be ussing Direction.Length() ?
//!	Returns the node distance (selectable type) to a point  
//! ----------------------------------------------------------
float CAINavigNode::GetDistanceToPoint (const CPoint &obTo, ENUM_DISTANCE_TYPE eDT) const
{
	float deltaX = m_obPos.X() - obTo.X();
	float deltaY = m_obPos.Y() - obTo.Y();
	float deltaZ = m_obPos.Z() - obTo.Z();

	switch (eDT)
	{
	case DT_MANHATTAN:
		return ( fabs ( deltaX ) +  fabs ( deltaY ) + fabs ( deltaZ ) );

	case DT_EUCLIDES_SQR:
		return ( sqr ( deltaX ) +  sqr( deltaY ) + sqr( deltaZ ) );

	case DT_EUCLIDES:
		return ( sqrt( sqr ( deltaX ) +  sqr( deltaY ) + sqr( deltaZ ) ) );
	default:
		ntPrintf("CAINavigNode::GetDistanceToPoint -> Unsuported Distance Type. Returning -1\n");
		return (-1.f);
	}
}

//! ----------------------------------------------------------
//! GetDistanceToSourceNode
//! !!! - May be ussing Direction.Length() ?
//!	Returns the node distance (selectable type) to a point  
//! ----------------------------------------------------------
float CAINavigNode::GetDistanceToSourceNode (const CAINavigNode *obSrcNode) const
{
	
	AINavigArrowList::const_iterator obIt		= obSrcNode->GetTgtArrows()->begin();
	AINavigArrowList::const_iterator obEndIt	= obSrcNode->GetTgtArrows()->end();
	for( ; obIt != obEndIt; ++obIt )
	{
		if ( (*obIt)->GetTgtNode() == this )
		{
			return ((*obIt)->GetDistance());
		}
	}
	return (0.f);
}

//! ----------------------------------------------------------
//! GetTargetArrow
//! ----------------------------------------------------------
CAINavigArrow* CAINavigNode::GetTargetArrow	( const CAINavigNode* pNode) const
{
	if (!pNode)
		return NULL;
	
	AINavigArrowList::const_iterator obIt		= m_listTargetArrows.begin();
	AINavigArrowList::const_iterator obEndIt	= m_listTargetArrows.end();
	for( ; obIt != obEndIt; ++obIt )
	{
		CAINavigArrow* pArrow = (*obIt);
		if ( pArrow->GetTgtNode() == pNode )
		{
			return pArrow;
		}
	}
	return NULL;	

}

//! ----------------------------------------------------------
//! GetUseEntities
//! ----------------------------------------------------------
void CAINavigNode::GetUseEntities ( ntstd::List<CEntity*>* pList) const
{
	if (!pList)
		return;
	
	pList->clear();

	AINavigArrowList::const_iterator obIt		= m_listTargetArrows.begin();
	AINavigArrowList::const_iterator obEndIt	= m_listTargetArrows.end();
	for( ; obIt != obEndIt; ++obIt )
	{
		CAINavigArrow* pArrow = (*obIt);
		CEntity* pUseEntity = pArrow->GetEntityToUse();
		if ( pUseEntity )
		{
			pList->push_back(pUseEntity);
		}
	}	
}

//! ----------------------------------------------------------
//! GetCoverPoints
//! ----------------------------------------------------------
void CAINavigNode::GetCoverPoints ( AINavigCoverPointList* plistCP ) 
{ 
	AINavigArrowList::const_iterator obIt		= m_listCoverArrows.begin();
	AINavigArrowList::const_iterator obEndIt	= m_listCoverArrows.end();
	for( ; obIt != obEndIt; ++obIt )
	{
		CAINavigArrow* pArrow = (*obIt);
		if (pArrow)
		{
			CAINavigCoverPoint* pCoverPoint = pArrow->GetCoverPoint();
			if ( pCoverPoint )
			{
				plistCP->push_back(pCoverPoint);
			}
		}
	}
}

//! ----------------------------------------------------------
//! IsLinkedToCoverPoint
//! ----------------------------------------------------------
bool CAINavigNode::IsLinkedToCoverPoint ( CAINavigCoverPoint* pCP )
{
	AINavigArrowList::const_iterator obIt		= m_listCoverArrows.begin();
	AINavigArrowList::const_iterator obEndIt	= m_listCoverArrows.end();
	for( ; obIt != obEndIt; ++obIt )
	{
		CAINavigArrow* pArrow = (*obIt);
		if (pArrow)
		{
			CAINavigCoverPoint* pCoverPoint = pArrow->GetCoverPoint();
			if (pCoverPoint == pCP )
			{
				return true;
			}
		}
	}
	return false;
}

//! ----------------------------------------------------------
//! GetAvailableCoverPoint
//! ----------------------------------------------------------
CAINavigCoverPoint* CAINavigNode::GetAvailableCoverPoint ( CAIMovement* pMov ) 
{ 
	if ( !pMov || !pMov->GetEntityToAttack() ) return NULL;

	CEntity* pEnt			= pMov->GetParent();
	const CEntity* pEnemy	= pMov->GetEntityToAttack();

	AINavigArrowList::const_iterator obIt		= m_listCoverArrows.begin();
	AINavigArrowList::const_iterator obEndIt	= m_listCoverArrows.end();
	for( ; obIt != obEndIt; ++obIt )
	{
		CAINavigArrow* pArrow = (*obIt);
		if (pArrow)
		{
			CAINavigCoverPoint* pCoverPoint = pArrow->GetCoverPoint();
			
			if ( !pCoverPoint ) continue;

			// Get Random Chance of covering
			float fChaceOfUse = pCoverPoint->GetChanceOfUse()*pMov->GetCoverAttitudeMultiplier();
			float fRand = grandf(1.0f);
			
			if (	!pCoverPoint->IsDestroyed() && pCoverPoint->IsAvailabe() &&							// There is a CP and is available
				    pMov->IsPointWithinMinMaxRange(pCoverPoint->GetPos()) &&							// Cover Point within min-max range
					(fChaceOfUse>0.99 || (fChaceOfUse > fRand)) &&										// The odds say "Take it"
					(pCoverPoint->GetClaimer() == NULL || pCoverPoint->GetClaimer() == pEnt) &&			// If Booked, this is the right one
					pCoverPoint->IsCoverInEnemysDirection(pEnemy->GetPosition()) &&									// It covers in the right direction
					(pMov->AllowToReuseCoverPoints() || pMov->IsDifferentFromPreviousCoverPoint(pCoverPoint)) &&	// AI can reuse CPs or this is a new one
					!CAINavigationSystemMan::Get().HasLineOfSight(pMov->GetPosition(), pEnemy->GetPosition())		// There is not LOS with enemy
				)
			{
				return pCoverPoint;
			}
		}
	}
	return NULL;
}

//! ----------------------------------------------------------
//! GetAnimation
//! ----------------------------------------------------------
const CHashedString CAINavigCoverPoint::GetAnimation ( ENUM_COVER_ANIM_TYPE eType, bool * bFound, CEntity* pAI )
{ 
	*bFound = false;
	if (!m_pSCoverPointAnimSets)
	{
		// COmplain only in not crossbowman or if cbm playing peek or in cover anims -- TEMPORARY!!!!
		if (pAI && pAI->ToCharacter()->GetRangedWeapon() == NULL)
			return CHashedString("");
		if (eType != ANIM_BREAK_COVER && eType != ANIM_TO_COVER)
			return CHashedString("");
	}

	HashedStringVector* pAnimList = NULL;
	UNUSED(pAI);

	switch (eType)
	{
		case ANIM_IN_COVER		: pAnimList = &(m_pSCoverPointAnimSets->listhsInCoverAnims); break;
		case ANIM_BREAK_COVER	: 
								{
									if (pAI && pAI->ToCharacter()->GetRangedWeapon() != NULL)
									{
										// For Crossbowman

										if ( IsJumpOver() == true )
										{
											*bFound = true;
											return (CHashedString("crossbowman_cover_break_jump"));
										}

   										CDirection AIHeading = pAI->GetMatrix().GetZAxis();
										CDirection AISideDir = pAI->GetMatrix().GetXAxis();
										CDirection CoverDir  = this->GetValidDir();
										CDirection LineAI2CP(this->GetPos() - pAI->GetPosition());
							
										*bFound = CAINavigationSystemMan::Get().TestDive_NoWallCollision(pAI,E_DIVE_LONG_RIGHT);
										if (*bFound)
											return (CHashedString("crossbowman_cover_break_right"));
                                        *bFound	= CAINavigationSystemMan::Get().TestDive_NoWallCollision(pAI,E_DIVE_LONG_LEFT);
										if (*bFound)
											return (CHashedString("crossbowman_cover_break_left"));
										*bFound = CAINavigationSystemMan::Get().TestDive_NoWallCollision(pAI,E_DIVE_SHORT_RIGHT);
										if (*bFound)
											return (CHashedString("crossbowman_cover_break_right"));
										*bFound = CAINavigationSystemMan::Get().TestDive_NoWallCollision(pAI,E_DIVE_SHORT_LEFT);
										if (*bFound)
											return (CHashedString("crossbowman_cover_break_left"));

										CHashedString("");
									}
									else
									{						
										pAnimList = &(m_pSCoverPointAnimSets->listhsBreakCoverAnims); break;
									}
									break;
								}
		case ANIM_PEEK_COVER	: pAnimList = &(m_pSCoverPointAnimSets->listhsPeekFromCoverAnims); break;
		case ANIM_TO_COVER		: 
								{

									if (pAI && pAI->ToCharacter()->GetRangedWeapon() != NULL)
									{
										// For Crossbowman
								
										if ( IsJumpOver() == true )
										{
											*bFound = true;
											return (CHashedString("crossbowman_cover_enter_jump"));
										}

										// Find the cover animation

										CDirection AIHeading = pAI->GetMatrix().GetZAxis();
										CDirection AISideDir = pAI->GetMatrix().GetXAxis();
										CDirection CoverDir  = this->GetValidDir();
										CDirection LineAI2CP(this->GetPos() - pAI->GetPosition());

										float fDotCheckZAxis = AIHeading.Dot(CoverDir);

										float fDist2Cover = LineAI2CP.Length();
										float fDot = AISideDir.Dot(CoverDir);

										bool bLongCover = fDist2Cover > SHORT_COVER_DIST;
										bool bLeft = fDot > 0.0f;
										bool bRear = fDotCheckZAxis > 0.9f; // less than 25deg.

										//if (fDotCheckZAxis < 0.25f )
										//{
										//	// The AI is facing more than 75deg. away from the cover direction
										//	return CHashedString("");
										//}

										if (fDotCheckZAxis < -0.9f)
										{
											bLeft = !bLeft;
										}
										
										*bFound = true;
										if (bRear && bLongCover)
											return (CHashedString("crossbowman_cover_enter_long_rear"));
										else if (bRear && !bLongCover)
											return (CHashedString("crossbowman_cover_enter_short_rear"));
										else if (bLeft && bLongCover)
											return (CHashedString("crossbowman_cover_enter_long_left"));
										else if (bLeft && !bLongCover)
											return (CHashedString("crossbowman_cover_enter_short_left"));
										else if (!bLeft && bLongCover)
											return (CHashedString("crossbowman_cover_enter_long_right"));
										else if (!bLeft && !bLongCover)
											return (CHashedString("crossbowman_cover_enter_short_right"));
										else
										{
											*bFound = true;
											CHashedString("");
										}
									}
									else
									{
										// Check which one is available
										pAnimList = &(m_pSCoverPointAnimSets->listhsLongRearTakeCoverAnims);
										if (pAnimList->empty())
											pAnimList = &(m_pSCoverPointAnimSets->listhsShortRearTakeCoverAnims);
										if (pAnimList->empty())
											pAnimList = &(m_pSCoverPointAnimSets->listhsLongLeftTakeCoverAnims);
										if (pAnimList->empty())
											pAnimList = &(m_pSCoverPointAnimSets->listhsShortLeftTakeCoverAnims);
										if (pAnimList->empty())
											pAnimList = &(m_pSCoverPointAnimSets->listhsLongRightTakeCoverAnims);
										if (pAnimList->empty())
											pAnimList = &(m_pSCoverPointAnimSets->listhsShortRightTakeCoverAnims);
										
										// For crossbowman
										if (pAnimList->empty())
											pAnimList = &(m_pSCoverPointAnimSets->listhsTakeCoverAnims);
									}
									//if (pAnimList != pSuggestedAnimList)
									//{
									//	ntPrintf("CAINavigCoverPoint::GetAnimation SUGESTED take cover animation NOT USED!\n");
									//}
								}
								break;

		default : return CHashedString("");
	}

	if (pAnimList->empty())
		return CHashedString("");

	unsigned int sz		= pAnimList->size();
	unsigned int uiRand = ((grand() % sz*100)/100);
	unsigned int uiIndex= 0;

	ntAssert( uiRand < sz );

	ntstd::List<CHashedString>::const_iterator obIt		= pAnimList->begin();
	ntstd::List<CHashedString>::const_iterator obEndIt	= pAnimList->end();
	
	for( ; uiIndex < uiRand; ++obIt, ++uiIndex )
	{
		if (obIt == obEndIt) 
			return CHashedString("");
	}

	*bFound = true;

	const CHashedString hsRet = (*obIt);
	return (hsRet);
}

//! ----------------------------------------------------------
//! IsCoverInEnemysDirection
//! ----------------------------------------------------------
bool CAINavigCoverPoint::IsCoverInEnemysDirection( const CPoint & obEnemysPos )
{
	CDirection Line(obEnemysPos - m_obPos);
	float fDotPorduct = Line.Dot(m_obValidDir);
	
	bool bValid = (m_obValidDir.Compare(CDirection(0,0,0),0.01f) || fDotPorduct > 0.0f )? true : false;

	return bValid;
}

//! ----------------------------------------------------------
//! GetTCBreakCoverAnimation
//! ----------------------------------------------------------
const CHashedString CAINavigNode::GetTCBreakCoverAnimation ( bool * bFound )
{ 
	*bFound = false;

	if (!m_pSTCAnimSets ||
		(m_pSTCAnimSets->listhsBreakLeftTCAnims.empty() && m_pSTCAnimSets->listhsBreakRightTCAnims.empty()) 
		)
		return CHashedString("");

	HashedStringVector* pAnimList = NULL;

	// Choose Left or Right
	ENUM_TC_ANIM_TYPE eType = grandf(1.0f) > 0.5f ? ANIM_TC_BREAK_COVER_LEFT : ANIM_TC_BREAK_COVER_RIGHT;
	if (eType == ANIM_TC_BREAK_COVER_LEFT && m_pSTCAnimSets->listhsBreakLeftTCAnims.empty())
	{
		eType = ANIM_TC_BREAK_COVER_RIGHT;
	}
	else if (eType == ANIM_TC_BREAK_COVER_RIGHT && m_pSTCAnimSets->listhsBreakRightTCAnims.empty())
	{
		eType = ANIM_TC_BREAK_COVER_LEFT;
	}

	switch (eType)
	{
		case ANIM_TC_BREAK_COVER_LEFT		: pAnimList = &(m_pSTCAnimSets->listhsBreakLeftTCAnims); m_pSTCAnimSets->eReturnToCoverSide = ANIM_TC_ENTER_COVER_RIGHT; break;
		case ANIM_TC_BREAK_COVER_RIGHT		: pAnimList = &(m_pSTCAnimSets->listhsBreakRightTCAnims); m_pSTCAnimSets->eReturnToCoverSide = ANIM_TC_ENTER_COVER_LEFT; break;

		default : return CHashedString("");
	}

	unsigned int sz		= pAnimList->size();
	unsigned int uiRand = ((grand() % sz*100)/100);
	unsigned int uiIndex= 0;

	ntAssert( uiRand < sz );

	ntstd::List<CHashedString>::const_iterator obIt		= pAnimList->begin();
	ntstd::List<CHashedString>::const_iterator obEndIt	= pAnimList->end();
	
	for( ; uiIndex < uiRand; ++obIt, ++uiIndex )
	{
		if (obIt == obEndIt) 
			return CHashedString("");
	}

	*bFound = true;

	const CHashedString hsRet = (*obIt);
	return (hsRet);
}

//! ----------------------------------------------------------
//! GetTCReturnToCoverAnimation
//! ----------------------------------------------------------
const CHashedString CAINavigNode::GetTCReturnToCoverAnimation ( bool * bFound )
{ 
	*bFound = false;

	if (!m_pSTCAnimSets) 
		return CHashedString("");

	HashedStringVector* pAnimList = NULL;

	switch (m_pSTCAnimSets->eReturnToCoverSide)
	{
		case ANIM_TC_ENTER_COVER_RIGHT		: pAnimList = &(m_pSTCAnimSets->listhsReturnRightTCAnims); break;
		case ANIM_TC_ENTER_COVER_LEFT		: pAnimList = &(m_pSTCAnimSets->listhsReturnLeftTCAnims); break;

		default : return CHashedString("");
	}

	unsigned int sz		= pAnimList->size();
	unsigned int uiRand = ((grand() % sz*100)/100);
	unsigned int uiIndex= 0;

	ntAssert( uiRand < sz );

	ntstd::List<CHashedString>::const_iterator obIt		= pAnimList->begin();
	ntstd::List<CHashedString>::const_iterator obEndIt	= pAnimList->end();
	
	for( ; uiIndex < uiRand; ++obIt, ++uiIndex )
	{
		if (obIt == obEndIt) 
			return CHashedString("");
	}

	*bFound = true;

	const CHashedString hsRet = (*obIt);
	return (hsRet);
}




												
