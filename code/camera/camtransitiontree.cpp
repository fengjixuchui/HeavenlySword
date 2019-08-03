//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file camtransitiontree.cpp
//!                                                                                         
//------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
// Required Includes
//------------------------------------------------------------------------------------------
#include "camera/camtransitiontree.h"
#include "camera/camtransition.h"
#include "camera/camerainterface.h"
#include "camera/camutils.h"

#include "game/entitymanager.h"
#include "game/inputcomponent.h"

//------------------------------------------------------------------------------------------
//!
//!	CamTransitionTree::Reset
//!	Kill the current tree structure
//!
//------------------------------------------------------------------------------------------
void CamTransitionTree::Reset()
{
	if(m_pTreeTop)
	{
		NT_DELETE_CHUNK(Mem::MC_CAMERA, m_pTreeTop );
		m_pTreeTop = 0;

//		CEntity* pobPlayer = CEntityManager::Get().GetPlayer();
//		if (pobPlayer)
//		{
//			CInputComponent* pInputComponent = pobPlayer->GetInputComponent();
//			// set active transition - slightly icky; could add a GetTransition member to the trans tree manager
//			// would eliminate the casts: trans->cam interface->trans.
//			pInputComponent->SetCameraTransition( 0 );
//		}
	}
}


//------------------------------------------------------------------------------------------
//!
//!	CamTransitionTree::ActivateTransition
//!	Create a new transition and add it to the tree.
//!
//------------------------------------------------------------------------------------------
void CamTransitionTree::ActivateTransition(const CameraInterface* pOldCam, const CameraInterface* pNewCam,
										   const CamTransitionDef* pDef)
{
#ifdef CAMERA_DEBUG	
	ntPrintf("Moving From Camera %s to Camera %s via %s\n.",	pOldCam->GetCameraName(),
															pNewCam->GetCameraName(),
															pDef->GetTransTypeName());
#endif

	if(IsTransitionActive())
	{
		// We're already transitioning, this new transition must fit into our tree.
		// ------------------------------------------------------------------------
		if(m_pTreeTop->GetDestination() == pOldCam)
		{
			// This transition fits in at the top of the tree.
			m_pTreeTop = pDef->Create(m_pTreeTop, pNewCam);
		}
		else
		{
			ntAssert(true);
			// This transition might go somewhere in the tree.
		}
	}
	else
	{
		// No transition is currently active, so just plant a new tree.
		// ------------------------------------------------------------
		m_pTreeTop = pDef->Create(pOldCam, pNewCam);
	}
}


//------------------------------------------------------------------------------------------
//!
//!	CamTransitionTree::Update
//!	Update the transition tree, filters down the nodes, returns true if transition is active
//!
//------------------------------------------------------------------------------------------
bool CamTransitionTree::Update(float fTimeDelta)
{
	if(!m_pTreeTop)
		return false;

	// Has our top transition completed?
	if(!m_pTreeTop->IsActive())
	{
		NT_DELETE_CHUNK(Mem::MC_CAMERA, m_pTreeTop );
		m_pTreeTop = 0;

//		CEntity* pobPlayer = CEntityManager::Get().GetPlayer();
//		if (pobPlayer)
//		{
//			CInputComponent* pInputComponent = pobPlayer->GetInputComponent();
//			// set active transition - slightly icky; could add a GetTransition member to the trans tree manager
//			// would eliminate the casts: trans->cam interface->trans.
//			pInputComponent->SetCameraTransition( 0 );
//		}
		return false;
	}

	// Update our transition tree.
	m_pTreeTop->UpdateTransition(fTimeDelta);
	return true;
}



//------------------------------------------------------------------------------------------
//!	
//!	CamTransitionTree::Using
//!	Are we using a particular camera?
//!
//------------------------------------------------------------------------------------------
bool CamTransitionTree::IsUsing(CameraInterface* pCam) const
{
	return m_pTreeTop ? m_pTreeTop->Is(pCam) : false;
}


//------------------------------------------------------------------------------------------
//!	
//!	CamTransitionTree::GetDestination
//!	What's our destination camera?
//!
//------------------------------------------------------------------------------------------
const CameraInterface* CamTransitionTree::GetDestination() const
{
	if(!m_pTreeTop)
		return 0;

	return m_pTreeTop->GetDestination();
}


//------------------------------------------------------------------------------------------
//!	
//!	CamTransitionTree::GetCameraInterface
//!
//------------------------------------------------------------------------------------------
const CameraInterface* CamTransitionTree::GetCameraInterface() const  
{
	return static_cast<CameraInterface*>(m_pTreeTop);
}


//------------------------------------------------------------------------------------------
//!	
//!	CamTransitionTree::GetCameraInterface
//!
//------------------------------------------------------------------------------------------
CameraInterface* CamTransitionTree::GetCameraInterface()
{
	return static_cast<CameraInterface*>(m_pTreeTop);
}
