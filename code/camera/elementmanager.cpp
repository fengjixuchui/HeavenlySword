//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file elementmanager.cpp                                                         
//!                                                                                         
//------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
// Required Includes                                                                        
//------------------------------------------------------------------------------------------
#include "camera/elementmanager.h"
#include "camera/sceneelementcomponent.h"
#include "camera/camtools.h"
#include "camera/camutils.h"

#include "game/entity.h"
#include "game/entity.inl"


// I would like to get rid of the need for these and tidy up the CalcPOI function too
#include "ai/aistates.h"
#include "game/aicomponent.h"

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CamSceneElementList::AddSceneElement                                                    
//!	Add a scene element to the list.                                                        
//!                                                                                         
//------------------------------------------------------------------------------------------
void CamSceneElementList::AddSceneElement(SceneElementComponent* pElement)
{
	ntAssert(pElement);
	m_elements.push_back(pElement);
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CamSceneElementList::RemoveSceneElement                                                 
//!	Remove a scene element from the list.                                                   
//!                                                                                         
//------------------------------------------------------------------------------------------
void CamSceneElementList::RemoveSceneElement(SceneElementComponent* pElement)
{
	ntAssert(pElement);
	m_elements.remove(pElement);
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CamSceneElementList::RemoveAll                                                          
//!	Remove all the scene elements from the list.                                            
//!                                                                                         
//------------------------------------------------------------------------------------------
void CamSceneElementList::RemoveAll()
{
	while(!m_elements.empty())
	{
		m_elements.pop_back();
	}
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CamSceneElementMan::CamSceneElementMan                                                  
//!	Construction.                                                                           
//!                                                                                         
//------------------------------------------------------------------------------------------
CamSceneElementMan::CamSceneElementMan(const CEntity* pPrimaryEntity, bool bIsPrimaryLocked): 
	m_ptPOI(CONSTRUCT_CLEAR),
	m_fFrustFOV(0.f),
	m_elements(CamSceneElementList::Get().m_elements),
	m_bPrimaryLocked(bIsPrimaryLocked),
	m_bRenderPOI(false),
	m_bRenderFrust(false),
	m_fIntDepth(0.f),
	m_fIntOffset(0.f),
	m_obZoomConstraintPos(CONSTRUCT_CLEAR),
	m_fZoomConstraintRadius(0.f),
	m_bZoomX(false)
{

	m_pPrimaryElement = pPrimaryEntity ? pPrimaryEntity->GetSceneElement() : 0;

	if(!m_pPrimaryElement)
		ntPrintf("No primary element for CamView.");
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CamSceneElementMan::CalcPointOfInterest                                                 
//!	Calculate the Point of Interest.                                                        
//!                                                                                         
//------------------------------------------------------------------------------------------
CPoint CamSceneElementMan::CalcPointOfInterest()
{
	CPoint ptPrimary = m_pPrimaryElement ? m_pPrimaryElement->GetPosition() : CPoint(0.f, 0.f, 0.f);

	if(m_elements.empty() || m_bPrimaryLocked)
	{
		m_ptPOI = ptPrimary;
		return ptPrimary;
	}

	// calculate weighted point of Interest
	CPoint obLowPos(CONSTRUCT_CLEAR);
	float  fLowTotalWeight = 0.0f;
	CPoint obMedPos(CONSTRUCT_CLEAR);
	float  fMedTotalWeight = 0.0f;
	CPoint obHighPos(CONSTRUCT_CLEAR);
	float  fHighTotalWeight = 0.0f;

	for(ntstd::List<SceneElementComponent*>::const_iterator it(m_elements.begin());
		it != m_elements.end(); ++it)
	{
		if(!(*it)->GetEntity())
		{
			LOG_WARNING("CCamSceneElementMan::CalcPointOfInterest: Element with no associated entity.");
			continue;
		}

		(*it)->SetRGB(0.0f, 0.0f, 0.0f);

		// Check Influence Radius - JML         // Look Ahead?
		if((*it)->IsInfluencing(ptPrimary) > SceneElementComponent::INFLUENCE_NONE ||
		   (*it) == m_pPrimaryElement)
		{
			// Please find a better solution for this! - JML
			AIStates::eSTATE eAIState = AIStates::STATE_START;
			if((*it)->GetEntity()->IsAI() && ((AI*)(*it)->GetEntity())->GetAIComponent() && ((AI*)(*it)->GetEntity())->GetAIComponent()->GetAIState())
				eAIState = ((AI*)(*it)->GetEntity())->GetAIComponent()->GetAIState()->GetCurrState();

			if((*it) == m_pPrimaryElement)
			{
				obHighPos += (*it)->GetPosition();
				fHighTotalWeight += 1.f;
				(*it)->SetRGB(1.0f, 1.0f, 1.0f);
			}
			else if(eAIState >= AIStates::ATTACK_SELECTION && eAIState <= AIStates::BLOCKING)
			{   // This AI is in an attacking mode
				obMedPos += ((*it)->GetPosition()) * (*it)->GetVariableImportance();
				fMedTotalWeight += (*it)->GetVariableImportance();
				(*it)->SetRGB(1.0f, 0.0f, 0.0f);
			}
			else
			{
				obLowPos += ((*it)->GetPosition()) * (*it)->GetVariableImportance();
				fLowTotalWeight += (*it)->GetVariableImportance();
				(*it)->SetRGB(0.0f, 1.0f, 0.0f);
			}
		}
	}

	// Consolidate the three groups
	obMedPos        += obHighPos;
	fMedTotalWeight += fHighTotalWeight;
	obLowPos        += obMedPos;
	fLowTotalWeight += fMedTotalWeight;

	if(fHighTotalWeight)
	{
		obHighPos /= fHighTotalWeight;
	}

	CDirection obMedDir;
	if(fMedTotalWeight)
	{
		obMedPos /= fMedTotalWeight;
		obMedDir = obMedPos ^ obHighPos;
	}
	else
	{
		obMedDir = CDirection(CONSTRUCT_CLEAR);
	}

	CDirection obLowDir;
	if(fLowTotalWeight)
	{
		obLowPos /= fLowTotalWeight;
		obLowDir = obLowPos ^ obMedPos;
	}
	else
	{
		obLowDir = CDirection(CONSTRUCT_CLEAR);
	}
	
	// And Done
	m_ptPOI = obHighPos + obMedDir + obLowDir;
	return m_ptPOI;
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CamSceneElementMan::SetPrimaryEntity                                                          
//!	Change the primary element for this view.                                              
//!                                                                                         
//------------------------------------------------------------------------------------------
void CamSceneElementMan::SetPrimaryEntity(CEntity* pEnt)
{
	m_pPrimaryElement = pEnt->GetSceneElement();
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CamSceneElementMan::CalcIdealZoom                                                       
//!	Calculate the ideal zoom for the view.                                                  
//!                                                                                         
//------------------------------------------------------------------------------------------
float CamSceneElementMan::CalcIdealZoom(const CPoint& obSrc, const CPoint& obTarg, float fAspect, float fYFOV) const
{
	if(m_bPrimaryLocked)
		return CalcPlayerZoom(obSrc, obTarg, fAspect, fYFOV);

	if(m_elements.empty()) 
		return 0;

	if(fYFOV <= 0.f || fAspect <= 0.f || fYFOV >= HALF_PI)
	{
		if(fYFOV <= 0.f)        ntPrintf("CalcZoom - FoV <= zero.\n");
		else if(fAspect <= 0.f) ntPrintf("CalcZoom - Aspect <= zero.\n");
		else                    ntPrintf("CalcZoom - FoV >= PI/2.\n");
		return 0;
	}

	// These values for calculating ideal zoom
	float fX = 0.0f; // Offset from camera in rads
	float fY = 0.0f;
	float fXZ = 0.0f; // Depth relative to camera
	float fYZ = 0.0f;
	float fXl = 0.0f; // Offset relative to camera
	float fYl = 0.0f;

#ifdef _DEBUG
	CPoint obZoomConstraintPosX(CONSTRUCT_CLEAR);
	float fZoomConstraintRadiusX = 0.0f;
	CPoint obZoomConstraintPosY(CONSTRUCT_CLEAR);
	float fZoomConstraintRadiusY = 0.0f;
#endif

	// Keep a track of the shallowest element, don't want to zoom past it...
	float fShallowDepth = MAX_POS_FLOAT;

	// start with our current view transform
	CMatrix matFrust;
	CCamUtil::CreateFromPoints(matFrust, obSrc, obTarg);
	matFrust.SetTranslation(CVecMath::GetZeroPoint());
	
	// get the inverse of this matrix
	CMatrix obInvPOIMat(matFrust.GetAffineInverse());
	
	// Get the player entity's position so we can check influence radii - JML
	CPoint ptPrimary = m_pPrimaryElement ? m_pPrimaryElement->GetPosition() : CPoint(0.f, 0.f, 0.f);

	// for all scene elements
	int iElementCount = 0;
	for(ntstd::List<SceneElementComponent*>::const_iterator it = m_elements.begin();
		it != m_elements.end(); ++it)
	{
		// Only want influencing zoom elements...
		if((*it)->IsInfluencing(ptPrimary) != SceneElementComponent::INFLUENCE_FULL)
			continue;

		iElementCount++;

		// back translate
		CDirection obDir = ((*it)->GetPosition()) ^ obSrc;

		// get the visible width of the entity
		float fDistance = obDir.Length();
		float fRadius   = (*it)->GetRadius();
		float fArcLen   = 0.0f;
					
		if(fDistance)
			fArcLen = fabsf(fatanf(fRadius / fDistance));

		// transform the entity direction to camera space
		obDir = obDir * obInvPOIMat;
		
		// Consider it as being closer to the camera to account for bounding sphere
		//obDir -= CDirection(0.0f, 0.0f, fRadius);
		
		CDirection obNormalisedDir = obDir;
		obNormalisedDir.Normalise();

		// get its position in spherical coords
		float fEntLong, fEntLat;
		CCamUtil::SphericalFromCartesian(obNormalisedDir, fEntLat, fEntLong);

		// get the distance from the center
		float fVert   = ntstd::Max(fEntLat + fArcLen, fabsf(fEntLat - fArcLen));
		float fHorz   = ntstd::Max(fabsf(fEntLong - fArcLen), fEntLong + fArcLen);

		if(obDir.Z() < fShallowDepth)
			fShallowDepth = obDir.Z();

		if(fVert > fY)
		{
			fY = fVert;
			fYZ = obDir.Z();
			fYl = fabsf(obDir.Y()) + fRadius;
#ifdef _DEBUG
			obZoomConstraintPosY   = (*it)->GetPosition();
			fZoomConstraintRadiusY = fRadius;
#endif
		}
		if(fHorz > fX)
		{
			fX = fHorz;
			fXZ = obDir.Z();
			fXl = fabsf(obDir.X()) + fRadius;
#ifdef _DEBUG
			obZoomConstraintPosX   = (*it)->GetPosition();
			fZoomConstraintRadiusX = fRadius;
#endif
		}
	}

	// Will have to account for aspect ratio...  // WRONG JML
	fYFOV /= 2.f;
	float fXFOV = fatanf(ftanf(fYFOV*.5f)*fAspect)*2.f;
	ntAssert(fXFOV > 0.f);

	float fXZoom = fXZ - fXl * ftanf(HALF_PI - fXFOV);
	float fYZoom = fYZ - fYl * ftanf(HALF_PI - fYFOV);

	float fZoom  = ntstd::Min(fXZoom, fYZoom);
	fZoom        = ntstd::Min(fShallowDepth, fZoom);

	return fZoom;
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CamSceneElementMan::CalcPlayerZoom                                                      
//!	Calculate the zoom required to make the player the optimum size in the view.            
//!                                                                                         
//------------------------------------------------------------------------------------------
float CamSceneElementMan::CalcPlayerZoom(const CPoint& obSrc, const CPoint &obTarg, 
										 float fAspect, float fYFOV) const
{
	UNUSED(obTarg);

	if(m_elements.empty() || !m_pPrimaryElement) 
		return 0;

	if(fYFOV <= 0.f)
	{
		ntPrintf("CalcZoom FoV < zero.\n");
		return 0;
	}

	// Get the player entity's position so we can check influence radii - JML
	CPoint ptPrimary = m_pPrimaryElement ? m_pPrimaryElement->GetPosition() : CPoint(0.f, 0.f, 0.f);

	// These values for calculating ideal zoom
	float fX = 0.0f; // Offset from camera in radians
	float fY = 0.0f;
	float fXZ = 0.0f; // Depth relative to camera
	float fYZ = 0.0f;
	float fXl = 0.0f; // Offset relative to camera
	float fYl = 0.0f;

	// start with our current view transform
	CMatrix matFrust;
	CCamUtil::CreateFromPoints(matFrust, obSrc, ptPrimary);
	matFrust.SetTranslation(CVecMath::GetZeroPoint());
	
	// get the inverse of this matrix
	CMatrix obInvPOIMat(matFrust.GetAffineInverse());

	// back translate
	CDirection obDir = (m_pPrimaryElement->GetPosition()) ^ obSrc;

	// get the visible width of the entity
	float fDistance = obDir.Length();
	float fRadius   = m_pPrimaryElement->GetRadius();
	float fArcLen   = 0.0f;
				
	if(fDistance)
		fArcLen = fabsf(fatanf(fRadius / fDistance));

	// transform the entity direction to camera space
	obDir = obDir * obInvPOIMat;
	
	CDirection obNormalisedDir = obDir;
	obNormalisedDir.Normalise();

	// get its position in spherical coords
	float fEntLong, fEntLat;
	CCamUtil::SphericalFromCartesian(obNormalisedDir, fEntLat, fEntLong);

	// get the distance from the center
	float fVert   = ntstd::Max(fEntLat + fArcLen, fabsf(fEntLat - fArcLen));
	float fHorz   = ntstd::Max(fabsf(fEntLong - fArcLen), fEntLong + fArcLen);

	if(fVert > fY)
	{
		fY = fVert;
		fYZ = obDir.Z();
		fYl = fabsf(obDir.Y()) + fRadius;
	}
	if(fHorz > fX)
	{
		fX = fHorz;
		fXZ = obDir.Z();
		fXl = fabsf(obDir.X()) + fRadius;
	}

	// Will have to account for aspect ratio... // WRONG JML
	fYFOV /= 2.f;
	float fXFOV = fatanf(ftanf(fYFOV*.5f)*fAspect)*2.f;

	float fXZoom = fXZ - fXl * ftanf(HALF_PI - fXFOV);
	float fYZoom = fYZ - fYl * ftanf(HALF_PI - fYFOV);

	return ntstd::Min(fXZoom, fYZoom);
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CamSceneElementMan::CalcCoolCamZoom                                                       
//!	Calculate the zoom required to optimally frame 2 combatants in the view.                
//!                                                                                         
//------------------------------------------------------------------------------------------
float CamSceneElementMan::CalcCoolCamZoom(const CPoint &obSrc, const CPoint &obTarg, 
										  float fAspect, float fYFOV,
                                          const CEntity *pobAttacker, const CEntity *pobAttackee) const
{
	if(m_elements.empty()) 
		return 0;

	if(fYFOV <= 0.f)
	{
		ntPrintf("CalcZoom FoV < zero.\n");
		return 0;
	}

	// These values for calculating ideal zoom
	float fX = 0.0f; // Offset from camera in rads
	float fY = 0.0f;
	float fXZ = 0.0f; // Depth relative to camera
	float fYZ = 0.0f;
	float fXl = 0.0f; // Offset relative to camera
	float fYl = 0.0f;
	float fXr = 0.0f; // Radius of the camera sphere for the entity;
	float fYr = 0.0f;	
	float fShallowDepth = MAX_POS_FLOAT;

	// start with our current view transform
	CMatrix matFrust;
	CCamUtil::CreateFromPoints(matFrust, obSrc, obTarg);
	matFrust.SetTranslation(CVecMath::GetZeroPoint());
	
	// get the inverse of this matrix
	CMatrix obInvPOIMat(matFrust.GetAffineInverse());

	// for all scene elements
	for(ntstd::List<SceneElementComponent*>::const_iterator it(m_elements.begin());
		it != m_elements.end(); ++it)
	{
		// Only want influencing zoom elements...
		if((*it)->GetEntity() != pobAttackee && 
		   (*it)->GetEntity() != pobAttacker)
			continue;

		// back translate
		CDirection obDir = ((*it)->GetPosition()) ^ obSrc;

		// get the visible width of the entity
		float fDistance = obDir.Length();
		float fRadius   = (*it)->GetRadius();
		float fArcLen   = 0.0f;
					
		if(fDistance)
			fArcLen = fabsf(fatanf(fRadius / fDistance));

		// transform the entity direction to camera space
		obDir = obDir * obInvPOIMat;
		CDirection obNormalisedDir = obDir;
		obNormalisedDir.Normalise();

		// get its position in spherical coords
		float fEntLong, fEntLat;
		CCamUtil::SphericalFromCartesian(obNormalisedDir, fEntLat, fEntLong);

		// get the distance from the center
		float fVert   = fabsf(fEntLat)  + fabsf(fArcLen);
		float fHorz   = fabsf(fEntLong) + fabsf(fArcLen);

		if(obDir.Z() < fShallowDepth)
			fShallowDepth = obDir.Z();

		if(fVert > fY)
		{
			fY = fVert;
			fYZ = obDir.Z();
			fYl = fabsf(obDir.Y()) + fRadius;
			fYr = fRadius;
		}
		if(fHorz > fX)
		{
			fX = fHorz;
			fXZ = obDir.Z();
			fXl = fabsf(obDir.X()) + fRadius;
			fXr = fRadius;
		}
	}

	// Will have to account for aspect ratio...  // WRONG JML
	fYFOV /= 2.f;
	float fXFOV = fatanf(ftanf(fYFOV*.5f)*fAspect)*2.f;

	float fXZoom = fXZ - fXl / ftanf(fXFOV) - fXr;
	float fYZoom = fYZ - fYl / ftanf(fYFOV) - fYr;

	return ntstd::Max(fXZoom, fYZoom);
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CamSceneElementMan::CountEnemiesInfluencing                                                       
//!	                                                                                        
//!                                                                                         
//------------------------------------------------------------------------------------------
int CamSceneElementMan::CountEnemiesInfluencing() const
{
	if(m_elements.empty()) 
		return 0;

	
	int iElementCount = 0;
	CPoint ptPrimary = m_pPrimaryElement ? m_pPrimaryElement->GetPosition() : CPoint(0.f, 0.f, 0.f);

	for(ntstd::List<SceneElementComponent*>::const_iterator it = m_elements.begin();
		it != m_elements.end(); ++it)
	{
		// Only want influencing zoom elements...
		if((*it)->IsInfluencing(ptPrimary) != SceneElementComponent::INFLUENCE_FULL)
			continue;

		if((*it)->GetEntity()->IsAI())
			iElementCount++;

	}

	return iElementCount;
}


/***************************************************************************************************
*	
*	FUNCTION		CCamSceneElementMan::Render
*
*	DESCRIPTION		Show what we're Interested in
*
***************************************************************************************************/
#ifndef _GOLD_MASTER
void CamSceneElementMan::RenderDebugInfo()
{

	//if(m_bRenderPOI)
	//{
		CCamUtil::Render_Sphere(m_ptPOI, 0.2f, 1.0f, 0.0f, 0.0f, 1.0f);

		for(ntstd::List<SceneElementComponent*>::const_iterator it(m_elements.begin());
			it != m_elements.end(); ++it)
		{
			CCamUtil::Render_Sphere((*it)->GetPosition(), (*it)->GetRadius(),
									(*it)->m_fR, (*it)->m_fG, (*it)->m_fB, 1.0f);
		}
	//	m_bRenderPOI = false;
	//}

	//if(m_bRenderFrust)
	//{
	//	CCamUtil::Render_Frustum(matFrust, m_fFrustFOV, CamMan_Public::Get().GetAspectRatio(), 1.0f, 0.0f, 0.0f, 1.0f);
	//	m_bRenderFrust = false;
	//}

	//int iGuide = 400;
	//CCamUtil::DebugPrintf(20, iGuide   , "   Zoom(%.2f)", m_fZoom);	
	//CCamUtil::DebugPrintf(20, iGuide+12, "   IntDepth(%.2f)", m_fIntDepth);	
	//CCamUtil::DebugPrintf(20, iGuide+24, "   IntOffset(%.2f)", m_fIntOffset);	
	//CCamUtil::DebugPrintf(20, iGuide+36, "   ZoomConst(%s)", m_bZoomX ? "X" : "Y");	

	//CCamUtil::Render_Sphere(m_obZoomConstraintPos, m_fZoomConstraintRadius, 1.0f, 0.0f, 0.0f, 1.0f);
}
#endif

