//------------------------------------------------------------------------------------------
//!
//!	\file hudsound.cpp
//!
//------------------------------------------------------------------------------------------

// Necessary includes
#include "hud/hudsound.h"
#include "objectdatabase/dataobject.h"
#include "hud/hudmanager.h"

// ---- Forward references ----

// ---- Interfaces ----
START_CHUNKED_INTERFACE( SoundDef, Mem::MC_MISC  )

	PUBLISH_VAR_WITH_DEFAULT_AS( m_obName,	"",		Name ) 
	PUBLISH_VAR_WITH_DEFAULT_AS( m_obBank,	"",		Bank ) 
	PUBLISH_VAR_WITH_DEFAULT_AS( m_obQue,	"",		Que ) 

	DECLARE_POSTCONSTRUCT_CALLBACK		(	PostConstruct )
END_STD_INTERFACE

void ForceLinkFunctionHudSound()
{
	ntPrintf("!ATTN! Calling ForceLinkFunctionHudSound() !ATTN!\n");
}

/////////////////////////////////////////
//
//	SoundDef::PostConstruct
//
/////////////////////////////////////////
void SoundDef::PostConstruct( void )
{
	if ( CHud::Exists() && CHud::Get().GetHudSoundManager() )
	{
		CHud::Get().GetHudSoundManager()->RegisterSoundDef( this );
	}
}

/////////////////////////////////////////
//
//	HudSoundManager
//	Construct/Destruct
//
/////////////////////////////////////////
HudSoundManager::~HudSoundManager()
{
	Reset();
}

/////////////////////////////////////////
//
//	HudSoundManager::RegisterSoundDef()
//
/////////////////////////////////////////
void HudSoundManager::RegisterSoundDef ( SoundDef* pobSoundDef )
{
#ifdef _DEBUG
	iSoundMapIter obIt = m_aobSoundList.find ( pobSoundDef->m_obName );
	ntAssert_p ( obIt == m_aobSoundList.end(), ("Sound already in map\n") );
#endif // _DEBUG

	m_aobSoundList[ pobSoundDef->m_obName ] = ntstd::pair < SoundDef*, unsigned int >( pobSoundDef, 0 );
}

/////////////////////////////////////////
//
//	HudSoundManager::Reset()
//
/////////////////////////////////////////
void HudSoundManager::Reset( void )
{
	m_aobSoundList.clear();
}

/////////////////////////////////////////
//
//	HudSoundManager::PlaySound()
//
/////////////////////////////////////////
bool HudSoundManager::PlaySound ( CHashedString obName )
{
	iSoundMapIter obIt = m_aobSoundList.find ( obName );

	if ( obIt != m_aobSoundList.end() )
	{
		ntstd::pair < SoundDef*, unsigned int >* pobSoundRef = &(obIt->second);

		if ( AudioSystem::Get().Sound_Prepare( pobSoundRef->second, 
			ntStr::GetString( pobSoundRef->first->m_obBank ), ntStr::GetString( pobSoundRef->first->m_obQue ) ) )
		{
			AudioSystem::Get().Sound_Play( pobSoundRef->second );
			return true;
		}
	}

	return false;
}

/////////////////////////////////////////
//
//	HudSoundManager::StopSound()
//
/////////////////////////////////////////
void HudSoundManager::StopSound ( CHashedString obName )
{
	iSoundMapIter obIt = m_aobSoundList.find ( obName );

	if ( obIt != m_aobSoundList.end() )
	{
		ntstd::pair < SoundDef*, unsigned int >* pobSoundRef = &(obIt->second);

		AudioSystem::Get().Sound_Stop( pobSoundRef->second );
	}	
}


