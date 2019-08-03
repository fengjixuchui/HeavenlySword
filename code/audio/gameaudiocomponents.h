//-------------------------------------------------------
//!
//!	\file audio\gameaudiocomponents.h
//!	Game specific audio classes.
//!
//-------------------------------------------------------

#ifndef _GAMEAUDIOCOMPONENTS_H
#define _GAMEAUDIOCOMPONENTS_H

// Necessary includes
#include "editable/enumlist.h"
#include "game/audio_lua.h"
#include "game/LuaGlobal.h"
#include "audio/audiosystem.h"

// Forward declaractions
class CEntity;
class Transform;
class FootstepProperties;
class CombatHitSoundDefinition;

namespace Physics
{
	class psPhysicsMaterial;
}





//------------------------------------------------------------------------------------------
//!
//!	GameAudioConfig
//!	Global game audio configuration
//!
//------------------------------------------------------------------------------------------

class GameAudioConfig
{
public:

	GameAudioConfig ();
	~GameAudioConfig ();

	void PostConstruct ();

	// ----- Serialised members -----

	CHashedString	m_obMixerProfile_Frontend;
	CHashedString	m_obMixerProfile_LevelStart;
	CHashedString	m_obMixerProfile_LevelExit;
	CHashedString	m_obMixerProfile_GamePause;
	CHashedString	m_obMixerProfile_OSDPause;
};









//------------------------------------------------------------------------------------------
//!
//!	GameAudioManager
//!	This class is meant to encapsulate all the various audio components that the game uses.
//! In particular it deals with loading/unloading and update functions.
//!
//------------------------------------------------------------------------------------------

class GameAudioManager : public Singleton<GameAudioManager>
{
public:

	GameAudioManager ();
	~GameAudioManager ();

	void RegisterGameAudioConfig (GameAudioConfig* pGameAudioConfig) { m_pGameAudioConfig=pGameAudioConfig; }
	void LoadResources ();		// Load PC wavebanks

	void Update ();				// Out update function which must be called every frame
	void DebugRender ();

	// ----- GUI transition handlers -----

	void OnFrontend (); // This should be called as soon as we enter the frontend
	void OnLevelStart (); // This should be called as soon as we enter the level
	void OnLevelExit (); // This should be called as soon as the level exit fadeout starts
	void OnPause (bool bActive); // This should be called when we enter/exit the in-game pause menu
	void OnOSD (bool bActive); // This should be called when we enter/exit the ps3 button menu

	// ----- Combat hit sound effects -----

	void TriggerCombatHitSound (const CEntity* pobAttacker,const CEntity* pobReceiver,COMBAT_STATE eCombatState,ATTACK_CLASS eAttackClass);
	void AddCombatHitSoundDef (CombatHitSoundDefinition* pobSoundDef);
	void RemoveCombatHitSoundDef (CombatHitSoundDefinition* pobSoundDef);

private:

	bool m_bFirstFrame;

	GameAudioConfig* m_pGameAudioConfig;

	ntstd::List<CombatHitSoundDefinition*> m_obCombatHitSoundDefList;
};





//------------------------------------------------------------------------------------------
//!
//!	EntityAudioChannel
//!	This is a mechanism for playing sounds through an entity. The channel tracks the sounds
//! that are played through its channels and have rules associated with them.
//!
//------------------------------------------------------------------------------------------

class EntityAudioChannel : public Audio_Lua
{
public:
	EntityAudioChannel (const CEntity* pobParent);

	// ----- Footstep -----
	void RegisterFootstepDefinition (const CHashedString& obName);
	void ProcessFootstepEvent (FOOTSTEP eType,float fVolume);

	// ----- Entity sounds -----
	void SetAudioRadius (float fDistance) { m_fAudioRadius=fDistance; }
	bool Play (ENTITY_AUDIO_CHANNEL eChannel,const CKeyString& obGroup,const CKeyString& obEvent,const CPoint& obPosition=CVecMath::GetZeroPoint(),float fVolume=1.0f,float fPitch=0.0f);
	void SetCallback (AudioCallback pFunc, void* pData) { m_pCallbackFunc=pFunc; m_pCallbackData=pData; } // Note: This will set a callback on the next EntityAudioChannel::Play
	void Stop (ENTITY_AUDIO_CHANNEL eChannel);
	bool IsPlaying (ENTITY_AUDIO_CHANNEL eChannel) const;
	float GetAudioRadius () const { return m_fAudioRadius; }

protected:

	bool DoChannelLogic (ENTITY_AUDIO_CHANNEL eChannel,int iGroupIndex,int iEventIndex);

	struct CHANNEL_INFO
	{
		unsigned int id; // AudioSystem Handle
		int iGroupIndex;
		int iEventIndex;
	};

	CHANNEL_INFO			m_astChannelInfo [TOTAL_CHANNELS]; // An array of channel info that is used to track what sound is playing on each channel

	const CEntity*			m_pobParentEntity; // Pointer to the entity that this audio channel belongs to

	const Transform*		m_pobRootTransform; // Root transform of parent entity

	FootstepProperties*		m_pobFootstepProperties; // Optional footstep properties

	AudioCallback			m_pCallbackFunc; // Function callback that gets triggered when the sound finishes playing
	void*					m_pCallbackData; // Function callback data

	double					m_dFootstepTime; // This time value is used to ensure a minimum interval between footstep triggering

	float					m_fAudioRadius; // Defines an audible radius for sounds played, if the listener is outside this radius - the sound doesn't get played
};






//------------------------------------------------------------------------------------------
//!
//!	FootstepEffect
//!	Association between sounds/particle with a material.
//!
//------------------------------------------------------------------------------------------
class FootstepEffect
{
public:

	FootstepEffect () {}

	// ----- Serialised data -----
	CHashedString	m_obSurfaceMaterial; // The surface material type this effect is associated with
	
	ntstd::String	m_obSingleFootSound;
	ntstd::String	m_obDoubleFootSound;
	ntstd::String	m_obSlideShortSound;
	ntstd::String	m_obSlideMediumSound;
	ntstd::String	m_obSlideLongSound;
	ntstd::String	m_obCustom1Sound;
	ntstd::String	m_obCustom2Sound;

	// Note: Can add particle effects here later I guess
};
 


//------------------------------------------------------------------------------------------
//!
//!	FootstepProperties
//!	Properties for footstep effects for a given entity.
//!
//------------------------------------------------------------------------------------------
class FootstepProperties
{
public:

	FootstepProperties ();

	void PostConstruct(void);
	bool EditorChangeValue(CallBackParameter obItem, CallBackParameter obValue);

	unsigned int ProcessEvent (FOOTSTEP eType,const CEntity* pobEntity,float fVolume); // Returns ID of the sound played

	// ----- Serialised data -----

	ntstd::List<FootstepEffect*> m_obFootstepEffectList;

protected:

	Physics::psPhysicsMaterial* GetSurfaceMaterial (const CPoint& obFootPosition);

	FootstepEffect* m_pobDefaultEffect;
};




//------------------------------------------------------------------------------------------
//!
//!	CombatHitSoundDefinition
//!	Properties for footstep effects for a given entity.
//!
//------------------------------------------------------------------------------------------
class CombatHitSoundDefinition
{
public:

	CombatHitSoundDefinition ();
	~CombatHitSoundDefinition ();

	void TriggerSound (const CEntity* pobReceiver,COMBAT_STATE eCombatState,ATTACK_CLASS eAttackClass);

	const CHashedString& GetAttackerHash () const { return m_obAttackerEntityClump; }
	const CHashedString& GetReceiverHash () const { return m_obReceiverEntityClump; }


	// ----- Serialised data -----

	CHashedString m_obAttackerEntityClump; // The attacker clump
	CHashedString m_obReceiverEntityClump; // The receiver clump

	ntstd::String m_obHit_SpeedFast;
	ntstd::String m_obHit_SpeedMedium;
	ntstd::String m_obHit_PowerFast;
	ntstd::String m_obHit_PowerMedium;
	ntstd::String m_obHit_RangeFast;
	ntstd::String m_obHit_RangeMedium;

	ntstd::String m_obHitStagger_SpeedFast;
	ntstd::String m_obHitStagger_SpeedMedium;
	ntstd::String m_obHitStagger_PowerFast;
	ntstd::String m_obHitStagger_PowerMedium;
	ntstd::String m_obHitStagger_RangeFast;
	ntstd::String m_obHitStagger_RangeMedium;

	ntstd::String m_obBlock_SpeedFast;
	ntstd::String m_obBlock_SpeedMedium;
	ntstd::String m_obBlock_PowerFast;
	ntstd::String m_obBlock_PowerMedium;
	ntstd::String m_obBlock_RangeFast;
	ntstd::String m_obBlock_RangeMedium;

	ntstd::String m_obBlockStagger_SpeedFast;
	ntstd::String m_obBlockStagger_SpeedMedium;
	ntstd::String m_obBlockStagger_PowerFast;
	ntstd::String m_obBlockStagger_PowerMedium;
	ntstd::String m_obBlockStagger_RangeFast;
	ntstd::String m_obBlockStagger_RangeMedium;

	ntstd::String m_obKO_SpeedFast;
	ntstd::String m_obKO_SpeedMedium;
	ntstd::String m_obKO_PowerFast;
	ntstd::String m_obKO_PowerMedium;
	ntstd::String m_obKO_RangeFast;
	ntstd::String m_obKO_RangeMedium;

	ntstd::String m_obDeath_SpeedFast;
	ntstd::String m_obDeath_SpeedMedium;
	ntstd::String m_obDeath_PowerFast;
	ntstd::String m_obDeath_PowerMedium;
	ntstd::String m_obDeath_RangeFast;
	ntstd::String m_obDeath_RangeMedium;

protected:

	void PlaySound (const CEntity* pobEntity,ntstd::String& obSound);
};







#endif // _GAMEAUDIOCOMPONENTS_H
