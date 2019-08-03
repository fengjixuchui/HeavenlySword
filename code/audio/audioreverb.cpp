#include "audio/audioreverb.h"
#include "audio/audiosystem.h"

#include "camera/camutils.h"

#include "core/visualdebugger.h" // Debug rendering

#include "objectdatabase/dataobject.h"



//#define _ENABLE_REVERB


#ifndef _RELEASE

#define _REVERB_RESOURCE_MESSAGES
#define _REVERB_ERROR_MESSAGES

#endif // _RELEASE

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------

START_STD_INTERFACE							( AudioReverbSphere )
	PUBLISH_VAR_AS							( m_obReverbDef,			ReverbDef )
	PUBLISH_VAR_AS							( m_obPosition,				Position )
	PUBLISH_VAR_AS							( m_fRadius,				Radius )
	PUBLISH_VAR_AS							( m_iPriority,				Priority )
	PUBLISH_VAR_AS							( m_bEnabled,				Enabled )
	DECLARE_POSTCONSTRUCT_CALLBACK			( OnPostConstruct )
	//DECLARE_EDITORCHANGEVALUE_CALLBACK		( EditorChangeValue )
END_STD_INTERFACE

START_STD_INTERFACE							( AudioReverbBox )
	PUBLISH_VAR_AS							( m_obReverbDef,			ReverbDef )
	PUBLISH_VAR_AS							( m_obPosition,				Position )
	PUBLISH_VAR_AS							( m_obRotation,				Rotation )
	PUBLISH_VAR_AS							( m_obHalfExtents,			HalfExtents )
	PUBLISH_VAR_AS							( m_iPriority,				Priority )
	PUBLISH_VAR_AS							( m_bEnabled,				Enabled )
	DECLARE_POSTCONSTRUCT_CALLBACK			( OnPostConstruct )
	DECLARE_EDITORCHANGEVALUE_CALLBACK		( EditorChangeValue )
END_STD_INTERFACE

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------

AudioReverbZone::AudioReverbZone () :
	m_obPosition(CONSTRUCT_CLEAR),
	m_iPriority(1000),
	m_bEnabled(true),
	m_bActive(false),
	m_bDebugRender(false)
{
	// Add reverb zone to audio system

	AudioReverbManager::Get().AddReverbZone(this);
}

AudioReverbZone::~AudioReverbZone ()
{
	AudioReverbManager::Get().RemoveReverbZone(this);
}

void AudioReverbZone::SetActive (bool bActive)
{
	if (bActive!=m_bActive)
	{
		m_bActive=bActive;

		AudioSystem::Get().Reverb_SetActive(GetReverbDef(),bActive);
	}
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------

AudioReverbSphere::AudioReverbSphere () :
	AudioReverbZone(),
	m_fRadius(1.0f)
{
}

void AudioReverbSphere::OnPostConstruct ()
{
	DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromPointer( this );
	if ( pDO )
		m_obName = CHashedString(pDO->GetName());
}

bool AudioReverbSphere::IsInside (const CPoint& obPoint)
{
	if (m_bEnabled)
	{
		CDirection obDiff(obPoint - m_obPosition);

		if (obDiff.LengthSquared() < (m_fRadius * m_fRadius))
			return true;
	}

	return false;
}

void AudioReverbSphere::DebugRender ()
{
#ifndef _RELEASE

	if (!m_bDebugRender)
		return;

	g_VisualDebug->RenderSphere(CVecMath::GetQuatIdentity(),m_obPosition,m_fRadius,0x77ff00ff);
	g_VisualDebug->RenderSphere(CVecMath::GetQuatIdentity(),m_obPosition,m_fRadius,0xffffffff,DPF_WIREFRAME);

	g_VisualDebug->Printf3D(m_obPosition, 0.0f,-20.0f,0xffffffff,DTF_ALIGN_HCENTRE,ntStr::GetString(m_obName));
	g_VisualDebug->Printf3D(m_obPosition, 0.0f,-10.0f,0xffffffff,DTF_ALIGN_HCENTRE,"Reverb:%s",ntStr::GetString(m_obReverbDef));
	g_VisualDebug->Printf3D(m_obPosition, 0.0f,0.0f, 0xffffffff,DTF_ALIGN_HCENTRE,"Priority:%d",m_iPriority);

#endif // _RELEASE
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------

AudioReverbBox::AudioReverbBox () :
	AudioReverbZone(),
	m_obRotation(CONSTRUCT_CLEAR),
	m_obHalfExtents(CONSTRUCT_CLEAR)
{
}

void AudioReverbBox::OnPostConstruct ()
{
	DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromPointer( this );
	if ( pDO )
		m_obName = CHashedString(pDO->GetName());

	CCamUtil::MatrixFromEuler_XYZ(m_obWorldMatrix,m_obRotation.X() * DEG_TO_RAD_VALUE,m_obRotation.Y() * DEG_TO_RAD_VALUE,m_obRotation.Z() * DEG_TO_RAD_VALUE);
	m_obWorldMatrix.SetTranslation(m_obPosition);
}

bool AudioReverbBox::EditorChangeValue (CallBackParameter/*pcItem*/, CallBackParameter/*pcValue*/)
{
	OnPostConstruct();

	return true;
}

bool AudioReverbBox::IsInside (const CPoint& obPoint)
{
	if (!m_bEnabled)
		return false;

	// Do bounding sphere check first

	CDirection obRelativePosition(obPoint - m_obPosition);
	float fDistSqrd=obRelativePosition.LengthSquared();
	float fOBBRadius=m_obHalfExtents.LengthSquared();

	if (fDistSqrd>fOBBRadius) // Do a sphere check first to make sure we are near the OBB
	{
		return false;
	}

	// Transform the test position so its relative to the OBB
	CMatrix obInverseWorldMatrix(m_obWorldMatrix.GetAffineInverse());

	CDirection obTransformedPosition=obRelativePosition * obInverseWorldMatrix;

	// Check to see if we are outside the AABB
    if (fabsf(obTransformedPosition.X())>m_obHalfExtents.X() ||
		fabsf(obTransformedPosition.Y())>m_obHalfExtents.Y() ||
		fabsf(obTransformedPosition.Z())>m_obHalfExtents.Z())
	{
		return false;
	}

	return true;
}

void AudioReverbBox::DebugRender ()
{
#ifndef _RELEASE

	if (!m_bDebugRender)
		return;
	
	g_VisualDebug->RenderOBB(m_obWorldMatrix,m_obHalfExtents,0xffffffff,DPF_WIREFRAME);
	g_VisualDebug->RenderOBB(m_obWorldMatrix,m_obHalfExtents,0x77ff00ff,0);

	g_VisualDebug->Printf3D(m_obWorldMatrix.GetTranslation(), 0.0f,-20.0f,0xffffffff,DTF_ALIGN_HCENTRE,ntStr::GetString(m_obName));
	g_VisualDebug->Printf3D(m_obWorldMatrix.GetTranslation(), 0.0f,-10.0f,0xffffffff,DTF_ALIGN_HCENTRE,"Reverb:%s",ntStr::GetString(m_obReverbDef));
	g_VisualDebug->Printf3D(m_obWorldMatrix.GetTranslation(), 0.0f,0.0f, 0xffffffff,DTF_ALIGN_HCENTRE,"Priority:%d",m_iPriority);


#endif // _RELEASE
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------

AudioReverbManager::AudioReverbManager () :
#ifdef _ENABLE_REVERB
	m_bEnabled(true)
#else
	m_bEnabled(false)
#endif
{
}

AudioReverbManager::~AudioReverbManager ()
{
	m_obReverbZoneList.clear();
}

void AudioReverbManager::Update ()
{
	if (!m_bEnabled || !AudioSystem::Get().GetFMODEventSystem()) // Only update if manager is enabled and the audio system is correctly initialised
		return;

	CPoint obListenerPosition(AudioSystem::Get().GetListenerPosition());

	AudioReverbZone* pobPreviouslyActiveZone=0;
	AudioReverbZone* pobActiveZone=0;

	for(ntstd::List<AudioReverbZone*>::iterator obIt=m_obReverbZoneList.begin(); obIt!=m_obReverbZoneList.end(); ++obIt)
	{
		if ((*obIt)->IsActive()) // Keep track of the previously active zone
		{
			pobPreviouslyActiveZone=(*obIt); // Note: There can only ever be one zone active at any one time, so this *should* only get set once
		}

		if ((*obIt)->IsInside(obListenerPosition)) // Listener is inside this zone
		{
			if (!pobActiveZone || (*obIt)->GetPriority() > pobActiveZone->GetPriority()) // Check to see if the priority of this active zone is higher than the existing
			{
				pobActiveZone=*obIt;
			}
		}
	}

	if (pobActiveZone!=pobPreviouslyActiveZone) // Zones have changed
	{
		if (pobPreviouslyActiveZone) // Deactivate the previous zone if necessary
			pobPreviouslyActiveZone->SetActive(false);

		if (pobActiveZone) // Activate the new zone if necessary
			pobActiveZone->SetActive(true);
	}
}

void AudioReverbManager::ActivateZone (const CHashedString& obName)
{
	for(ntstd::List<AudioReverbZone*>::iterator obIt=m_obReverbZoneList.begin(); obIt!=m_obReverbZoneList.end(); ++obIt)
	{
		if (obName==(*obIt)->GetName())
		{
			(*obIt)->SetEnabled(true);
			return;
		}
	}
}



void AudioReverbManager::DeactivateZone (const CHashedString& obName)
{
	for(ntstd::List<AudioReverbZone*>::iterator obIt=m_obReverbZoneList.begin(); obIt!=m_obReverbZoneList.end(); ++obIt)
	{
		if (obName==(*obIt)->GetName())
		{
			(*obIt)->SetEnabled(false);
			return;
		}
	}
}


void AudioReverbManager::SetEnable (bool bEnable)
{
	if (bEnable!=m_bEnabled) // State has changed
	{
		m_bEnabled=bEnable;

		if (!bEnable)  // We are disabling, ensure all zones are inactive
		{
			for(ntstd::List<AudioReverbZone*>::iterator obIt=m_obReverbZoneList.begin(); obIt!=m_obReverbZoneList.end(); ++obIt)
				(*obIt)->SetActive(false);
		}
	}
}

void AudioReverbManager::AddReverbZone (AudioReverbZone* pobReverbZone)
{
	#ifdef _REVERB_RESOURCE_MESSAGES
	ntPrintf("AudioReverbManager: Registering %s\n",ntStr::GetString(pobReverbZone->GetName()));
	#endif // _REVERB_RESOURCE_MESSAGES

	m_obReverbZoneList.push_back(pobReverbZone);
}

void AudioReverbManager::RemoveReverbZone (AudioReverbZone* pobReverbZone)
{
	#ifdef _REVERB_RESOURCE_MESSAGES
	ntPrintf("AudioReverbManager: Unregistering %s\n",ntStr::GetString(pobReverbZone->GetName()));
	#endif // _REVERB_RESOURCE_MESSAGES

	m_obReverbZoneList.remove(pobReverbZone);
}






