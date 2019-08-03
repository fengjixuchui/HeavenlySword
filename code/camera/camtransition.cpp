//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file camtransition.cpp
//!                                                                                         
//------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
// Required Includes
//------------------------------------------------------------------------------------------
#include "camera/camtransition.h"
#include "camera/camerainterface.h"
#include "camera/camutils.h"

#include "objectdatabase/dataobject.h"

//------------------------------------------------------------------------------------------
//!
//!	CamTransitionDef::SetDestCamera
//!	Sets Destination camera if one does not exist
//!
//------------------------------------------------------------------------------------------
bool CamTransitionDef::SetDestCamera( BasicCameraTemplate* pobDst )
{
	if (!m_pDst)
	{
		m_pDst = pobDst;
		return true;
	}

	return false;
}


//------------------------------------------------------------------------------------------
//!
//!	CamTransition::~CamTransition
//!	Destruction
//!
//------------------------------------------------------------------------------------------
CamTransition::~CamTransition()
{
	if(m_pSrc->IsType(CT_TRANSITION))
	{
		NT_DELETE_CHUNK(Mem::MC_CAMERA, m_pSrc );
	}

	if(m_pDst->IsType(CT_TRANSITION))
	{
		NT_DELETE_CHUNK(Mem::MC_CAMERA, m_pDst );
	}
}


//------------------------------------------------------------------------------------------
//!
//!	CamTransition::UpdateTransition
//!	Info about this transition.
//!
//------------------------------------------------------------------------------------------
void CamTransition::UpdateTransition(float fTimeDelta)
{
	ntAssert(m_pSrc);
	ntAssert(m_pDst);

	// Update child transitions
	if(m_pSrc->IsType(CT_TRANSITION))
	{
		( const_cast<CamTransition*>( static_cast< const CamTransition*>( m_pSrc ) ) )->UpdateTransition(fTimeDelta);

		// If the transition is done then swap out to the transitions destination.
		if(!m_pSrc->IsActive())
		{
			const CameraInterface* pNewSrc = ( const_cast<CamTransition*>( static_cast< const CamTransition*>( m_pSrc ) ) )->m_pDst;
			NT_DELETE_CHUNK(Mem::MC_CAMERA,  m_pSrc );
			m_pSrc = pNewSrc;
		}
	}

	if(m_pDst->IsType(CT_TRANSITION))
	{
		( const_cast<CamTransition*>( static_cast< const CamTransition*>( m_pDst ) ) )->UpdateTransition(fTimeDelta);

		// If the transition is done then swap out to the transitions destination.
		if(!m_pDst->IsActive())
		{
			const CameraInterface* pNewDst = ( const_cast<CamTransition*>( static_cast< const CamTransition*>( m_pDst ) ) )->m_pDst;
			NT_DELETE_CHUNK(Mem::MC_CAMERA,  m_pDst );
			m_pDst = pNewDst;
		}
	}

	// Now we can update ourselves...
	Update(fTimeDelta);

	// update control transition handling
	ControlTransitionUpdate( fTimeDelta );
}


//------------------------------------------------------------------------------------------
//!
//!	CamTransition::GetCameraName
//!	Info about this transition.
//!
//------------------------------------------------------------------------------------------
CHashedString CamTransition::GetCameraName() const
{
	static char buf[1024];
	sprintf(buf, "%s (%s -> %s)", GetTypeName(), ntStr::GetString(m_pSrc->GetCameraName()), ntStr::GetString(m_pDst->GetCameraName()));

	return CHashedString(buf);
}


//------------------------------------------------------------------------------------------
//!
//!	CamTransition::GetPriority
//!	Our priority is the same as our destination camera.
//!
//------------------------------------------------------------------------------------------
int CamTransition::GetPriority() const
{
	return m_pDst->GetPriority();
}


//------------------------------------------------------------------------------------------
//!
//!	CamTransition::GetTimeScalar
//!	Our time scalar is the same as our destination camera.
//!
//------------------------------------------------------------------------------------------
float CamTransition::GetTimeScalar() const
{
	return m_pDst->GetTimeScalar();
}


//------------------------------------------------------------------------------------------
//!
//!	CamTransition::ControlTransitionUpdate
//!	Update the data used for interpolating the control direction during camera transitions.
//!
//------------------------------------------------------------------------------------------
void CamTransition::ControlTransitionUpdate( float fTimeDelta )
{
	m_fControlTransitionTime += fTimeDelta;

	float fControlInterpValue = CCamUtil::Sigmoid( m_fControlTransitionTime, m_fControlTransitionTotalTime );
	if( fControlInterpValue > 1.f || fControlInterpValue < 0.f)
	{
		ntPrintf("Camera - Bad Sigmoid control interpolation value.\n");
		fControlInterpValue = clamp( fControlInterpValue, 0.f, 1.f );
	}

	m_obControlInterpLookatPoint = m_pSrc->GetLookAt() + ((m_pDst->GetLookAt() - m_pSrc->GetLookAt()) * fControlInterpValue);

	//m_bActive = true;
	if( m_bActive==false && m_fControlTransitionTime<m_fControlTransitionTotalTime )
	{
		m_bActive = true;
	}
}


const CameraInterface* CamTransition::GetDestination() const
{
	if(m_pDst->GetType() == CT_TRANSITION)
		return ( const_cast<CamTransition*>( static_cast< const CamTransition*>( m_pDst ) ) )->GetDestination();

	return m_pDst;
}

//

CamTransition* CamTrans_NullDef::Create(const CameraInterface* pSrc, const CameraInterface* pDst) const
{
	return NT_NEW_CHUNK( Mem::MC_CAMERA ) CamTrans_Null(pSrc, pDst);
}

void CamTrans_Null::Update(float fTimeDelta)
{
	UNUSED(fTimeDelta);

	m_obTransform = m_pSrc->GetTransform();
	m_obLookAt    = m_pSrc->GetLookAt();
	m_fFOV        = m_pSrc->GetFOV();

	m_bActive = m_pSrc->IsActive() && m_pDst->IsActive();
}
