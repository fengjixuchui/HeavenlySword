//----------------------------------------------------------------------------------------------------
//!
//!	\file audiohelper.cpp
//!	Audio Helper functions
//!
//----------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include "audiohelper.h"

#include "game/entity.h"
#include "game/entity.inl"

#include "audio/gameaudiocomponents.h"
#include "audio/audiosystem.h"


//-------------------------------------------------------------------------------------------------
// FUNC: AudioHelper::PlaySound( string sound )
// DESCRIPTION: Play a sound from the target entity
//-------------------------------------------------------------------------------------------------
void AudioHelper::PlaySound (const char* pcEventGroup,const char* pcEvent,CEntity* pobEntity)
{
	if (!pobEntity)
		pobEntity = CLuaGlobal::Get().GetTarg();

	unsigned int id;

	if (AudioSystem::Get().Sound_Prepare(id,pcEventGroup,pcEvent))
	{
		if (pobEntity)
			AudioSystem::Get().Sound_SetPosition(id,pobEntity->GetPosition());

		AudioSystem::Get().Sound_Play(id);
	}
}

//-------------------------------------------------------------------------------------------------
// FUNC: AudioHelper::PlaySound( string sound )
// DESCRIPTION: Play a sound from the target entity
//-------------------------------------------------------------------------------------------------
void AudioHelper::PlaySound (const char* pcSound,const CPoint& obPosition)
{
	unsigned int id;

	if (AudioSystem::Get().Sound_Prepare(id,pcSound))
	{
		AudioSystem::Get().Sound_SetPosition(id,obPosition);
		AudioSystem::Get().Sound_Play(id);
	}
}

