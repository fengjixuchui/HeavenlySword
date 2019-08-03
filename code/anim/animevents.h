//------------------------------------------------------------------------------------------
//!
//!	\file animevents.h
//!
//------------------------------------------------------------------------------------------

#ifndef ANIM_ANIMEVENTS_H
#define ANIM_ANIMEVENTS_H

#ifndef ANIM_ANIMATION_H
#include "anim/animation.h"
#endif

#include "editable/enumlist.h"

#include "core/registered.h"

class CEntity;
class CAttackData; 
class EffectTrigger;
class CAnimEventHandler;

namespace Physics { class CVolumeData; }



//#ifndef _RELEASE

#define _DEBUG_DISABLE_EVENT // Enable this functionality permanently otherwise we break release builds

//#endif // _RELEASE





//------------------------------------------------------------------------------------------
//!
//!	CAnimEvent
//!	Base type for an event that occurs in an anim.
//!
//------------------------------------------------------------------------------------------
class CAnimEvent
{
public:

	CAnimEvent () :
		#ifdef _DEBUG_DISABLE_EVENT
		m_fOffset(0.0f),
		m_fBlendThreshold(0.0f),
		m_bDebugDisable(false)
		#else
		m_fOffset(0.0f),
		m_fBlendThreshold(0.0f)
		#endif // _DEBUG_DISABLE_EVENT
	{
	}

	virtual void PostConstruct( void );

	void OnDelete();

	virtual ~CAnimEvent() {};

	virtual void Trigger (CEntity* /*pobEntity*/) {}

	virtual void Stop (CEntity* /*pobEntity*/) {}// Called when the animation event is destroyed

	// ----- Serialised members -----

	float m_fOffset; // 0.0 - 1.0

	float m_fBlendThreshold; // 0.0 - 1.0

	#ifdef _DEBUG_DISABLE_EVENT
	bool m_bDebugDisable; // This flag is available for non-release builds and allows events to be temporarily disabled
	#endif // _DEBUG_DISABLE_EVENT
};


//------------------------------------------------------------------------------------------
//!
//!	CAnimEventBSAnimPlay
//!	play a bsanimation
//!
//------------------------------------------------------------------------------------------
class CAnimEventBSAnimPlay : public CAnimEvent
{
public:
	CAnimEventBSAnimPlay () :
		m_bHoldLastKeyframe(false),
		m_bLooping(false),
		m_fSpeed(1.0f)
	{
	}

	virtual void PostConstruct ( void );

	virtual void Trigger (CEntity* pobEntity);

	CHashedString	m_obShortName;			//!< bsanim shortname
	bool		m_bHoldLastKeyframe;	//!< for pose-driven events
	bool		m_bLooping;
	float		m_fSpeed;				//!< playback speed
};


//------------------------------------------------------------------------------------------
//!
//!	CAnimEventBSAnimStop
//!	stop a bsanimation
//!
//------------------------------------------------------------------------------------------
class CAnimEventBSAnimStop : public CAnimEvent
{
public:
	virtual void Trigger (CEntity* pobEntity);

	CHashedString	m_obShortName;	// bsanim shortname
};

//------------------------------------------------------------------------------------------
//!
//!	CAnimEventSound
//!	Sound event that plays a sound from an entity (from one of their channels)
//!	at a given offset relative to the entity's root.
//!
//------------------------------------------------------------------------------------------
class CAnimEventSound : public CAnimEvent
{
public:

	CAnimEventSound ();

	virtual void PostConstruct ( void );
	
	virtual bool EditorChangeValue (CallBackParameter, CallBackParameter);

	virtual void Trigger (CEntity* pobEntity);

	virtual void Stop (CEntity* pobEntity);

	// ----- Serialised members -----

	CHashedString			m_obClumpName; // This allows you to restrict this anim event to a particular 

	ENTITY_AUDIO_CHANNEL	m_eChannel; // See enumlist.h

	CKeyString				m_obSoundBank;

	CKeyString				m_obSoundCue;

	float					m_fVolume;

	float					m_fPitch;

	float					m_fProbability; // Probability that this sound cue will fire (default=1.0)

	bool					m_bStopSound; // Stops the sound that was playing on specified channel when anim changes

	bool					m_bSendToChatterBox;

	bool					m_bHasSubtitles;
};

//------------------------------------------------------------------------------------------
//!
//!	CAnimEventAudioChannelPlay
//!	Used exclusively for playing sounds through entity audio channels.
//!
//------------------------------------------------------------------------------------------
/*
class CAnimEventAudioChannelPlay : public CAnimEvent
{
public:

	CAnimEventAudioChannelPlay ();

	virtual void PostConstruct ();
	virtual void EditorChangeValue ();

	virtual void Trigger (CEntity* pobEntity);
	virtual void Stop (CEntity* pobEntity);

	CKeyString				m_obSoundBank;		// Sound bank
	CKeyString				m_obSoundCue;		// Sound cue

	SPATIALIZATION			m_eSpatialization;	// 3d positioning type

	CPoint					m_obPosition;		// Position offset relative to the entity

	float					m_fVolume;			// Volume modifier
	float					m_fPitch;			// Pitch modifier
	float					m_fProbability;		// The probability that this sound will play (default is 1.0 which means it always plays)
};
*/


//------------------------------------------------------------------------------------------
//!
//!	CAnimEventAudioChannelStop
//!	Used exclusively for stopping entity audio channels.
//!
//------------------------------------------------------------------------------------------
class CAnimEventAudioChannelStop : public CAnimEvent
{
public:

	CAnimEventAudioChannelStop ();

	virtual void PostConstruct ();
	virtual void EditorChangeValue ();

	virtual void Trigger (CEntity* pobEntity);
	virtual void Stop (CEntity* pobEntity);

	ENTITY_AUDIO_CHANNEL	m_eChannel;			// The Channel this sound is played in

	bool					m_bDebugDisable;	// Allows you to temporarily disable a channel
};



//------------------------------------------------------------------------------------------
//!
//!	CAnimEventFootstep
//!	Trigger a footstep event on an entity.
//!
//------------------------------------------------------------------------------------------
class CAnimEventFootstep : public CAnimEvent
{
public:

	CAnimEventFootstep ();

	virtual void Trigger (CEntity* pobEntity);

	// ----- Serialised members -----

	float					m_fVolume;

	FOOTSTEP				m_eFootstep;
};


//------------------------------------------------------------------------------------------
//!
//!	AnimEventMixerProfile
//!	Transition to an audio layer.
//!
//------------------------------------------------------------------------------------------
class AnimEventMixerProfile : public CAnimEvent
{
public:

	AnimEventMixerProfile ();

	virtual void Trigger (CEntity* pobEntity);

	// ----- Serialised members -----

	CHashedString				m_obProfile;
	
	float						m_fTransitionDuration;
};



//------------------------------------------------------------------------------------------
//!
//!	CAnimEventScript
//!	Execute a lua script.
//!
//------------------------------------------------------------------------------------------
class CAnimEventScript : public CAnimEvent
{
public:
	CAnimEventScript();
	
	virtual void Trigger( CEntity* pobEntity );

	// ----- Serialised members -----

	CHashedString	m_obFunction;
};

//------------------------------------------------------------------------------------------
//!
//!	CAnimEventMessage
//!	Send a message.
//!
//------------------------------------------------------------------------------------------
class CAnimEventMessage : public CAnimEvent
{
public:
	CAnimEventMessage();
	
	virtual void Trigger( CEntity* pobEntity );

	// ----- Serialised members -----

	CHashedString m_obMessage;
};


//------------------------------------------------------------------------------------------
//!
//!	CAnimEventDemonAnimMessage
//!	Send a message that is intercepted by the demon entity to play an animation on itself.
//!
//------------------------------------------------------------------------------------------
class CAnimEventDemonAnimMessage : public CAnimEvent
{
public:
	CAnimEventDemonAnimMessage();
	
	virtual void Trigger( CEntity* pobEntity );

	// ----- Serialised members -----
	CHashedString m_obAnimName;
};


//------------------------------------------------------------------------------------------
//!
//!	CAnimEventCameraShake
//!	Shakes the camera?
//!
//------------------------------------------------------------------------------------------
class CAnimEventCameraShake : public CAnimEvent
{
public:
	CAnimEventCameraShake();

	virtual void Trigger( CEntity* pobEntity );
	
	// ----- Serialised members -----
	float	m_fTime;
	float	m_fIntensity;
	float	m_fSpeed;
};




//------------------------------------------------------------------------------------------
//!
//!	AnimEventEffect
//!	
//!
//------------------------------------------------------------------------------------------
class AnimEventEffect : public CAnimEvent
{
public: 
	AnimEventEffect();
	virtual ~AnimEventEffect();
	
	virtual void Trigger(CEntity* pEnt);
	virtual void Stop(CEntity* pEnt);

	// ----- Serialised members -----

	bool			m_bAutoDestruct;
	EffectTrigger*	m_pobTriggerDef;

private:
	struct EntIDListNode
	{
		CEntity*	pEnt;
		u_int		iID;
	};

	ntstd::List<EntIDListNode*, Mem::MC_ANIMATION> m_cleanups;
};

//!------------------------------------------------------------------------------
//!  AnimEventCombatAttack
//!  Anim event that allows the triggering of combat wake
//!
//!  Base class CAnimEvent <TODO: insert base class description here>
//!
//!  @author GavB @date 22/11/2006
//!------------------------------------------------------------------------------
class AnimEventCombatAttack : public CAnimEvent
{
public: 

	// 
	AnimEventCombatAttack() : 
		m_pAttackData( NULL ), 
		m_bUseStrike( true ),  
		m_bUseWake( true ), 
		m_fRadius( 10.0f ), 
		m_fDebrisImpulse( 100.0f ) 
	{}

	virtual ~AnimEventCombatAttack() {}
	virtual void Trigger(CEntity* pEnt);

public:

	// Pointer to the attack to trigger. 
	CAttackData*	m_pAttackData;

	// Use the strike defined in the attack
	bool	m_bUseStrike;

	// Use the wake effectg defined in the attack
	bool	m_bUseWake;

	// The radius in which targets will be hit 
	float	m_fRadius;

	// Apply an impulse to the ragdolls
	float	m_fDebrisImpulse;
};


//------------------------------------------------------------------------------------------
//!
//!	CAnimEventList
//!	Stores a list of anim events.
//!
//------------------------------------------------------------------------------------------
class CAnimEventList
{
public:

	// Construction
	CAnimEventList( void );

	// To make sure all is well data wise
#ifndef _RELEASE
	void PostConstruct( void );
#endif

	void OnDelete( void );

	void RemoveEventIfPresent( CAnimEvent *ae ) const;

	// Our only member
	mutable ntstd::List<CAnimEvent*, Mem::MC_ANIMATION> m_obAnimEventList;
};

//------------------------------------------------------------------------------------------
//!
//!	The CAnimEventMonitor needs to have a base-type of Registered<> in non-release
//! modes to enable Welder to delete CAnimEvents and CAnimEventLists.
//!
//------------------------------------------------------------------------------------------
#ifndef _RELEASE
#	define AE_REGISTERED_BASE( type ) : public Registered< type >
#else
#	define AE_REGISTERED_BASE( type )
#endif

//------------------------------------------------------------------------------------------
//!
//!	CAnimEventMonitor
//!	This is linked with a particular animation on an entity. If the animation
//! is played, the monitor is activated and runs through the CAnimEventList.
//!
//------------------------------------------------------------------------------------------
class CAnimEventMonitor AE_REGISTERED_BASE( CAnimEventMonitor )
{
public:

	friend class CAnimEventHandler;

	// Construction
	CAnimEventMonitor( u_int uiAnimNameHash, const CAnimEventList* pobAnimEventList, CEntity* pobEntity );

	void Update( void );

	// Activate or deactive this monitor
	void SetAnimation( const CAnimationPtr &pobAnimation );
	void RemoveAnimation( const CAnimationPtr &pobAnimation );
	void RemoveAllAnimations();

	void ToggleDebugDisable (const CHashedString& obEventName);

	// This needs to be called if SetTime is used on the anim
	void ResetProgress ();

	// Get the name hash of the animation we are a monitor for
	u_int GetAnimNameHash( void ) const { return m_uiAnimNameHash; }

	// Is this monitor active
	bool IsActive( void ) const { return m_bActive; }

	void RemoveEventListIfPresent( CAnimEventList *ael );
	void RemoveEventIfPresent( CAnimEvent *ae )
	{
		if ( m_pobAnimEventList != NULL )
		{
			m_pobAnimEventList->RemoveEventIfPresent( ae );
		}
	}

protected:

	// This value corresponds to the same anim name hash in the animation header which this monitor is linked with
	u_int m_uiAnimNameHash; 

	// Pointer to a serialised anim event list
	const CAnimEventList* m_pobAnimEventList;

	struct HandledAnim
	{
		CAnimationPtr	m_Anim;
		float			m_PreviousProgress;
		float			m_CurrentProgress;

		HandledAnim()
		:	m_Anim				( NULL )
		,	m_PreviousProgress	( 0.0f )
		,	m_CurrentProgress	( 0.0f )
		{}

		~HandledAnim()
		{
			m_Anim = CAnimationPtr( NULL );
		}
	};
	typedef ntstd::Vector< HandledAnim, Mem::MC_ANIMATION >	HandledAnimVector;

	// Array of animations and percentage progress sets we are monitoring
	HandledAnimVector m_Animations;
	
	// Pointer to the entity being used
	CEntity* m_pobEntity; 

	// Are we active?
	bool m_bActive;
};


//------------------------------------------------------------------------------------------
//!
//!	CAnimEventHandler
//!	This is effectively the manager for anim event lists and anim event monitors.
//!	These are linked with animators - therefore there is only one per entity.
//!
//------------------------------------------------------------------------------------------
class CAnimEventHandler
{
public:

	// Construction destruction
	CAnimEventHandler( void );
	~CAnimEventHandler( void );

	// Add event monitors to our full list - or remove them all
	void AddAnimEventMonitor( u_int uiAnimNameHash, const CAnimEventList* pobAnimEventList );
	void RemoveAnimEventMonitor( u_int uiAnimNameHash );
	void RemoveAllMonitors( void );

	// Set up our parent entity
	void SetEntity( CEntity* pobParentEntity );

	// Activate and deactivate animation event monitors
	void SetAnimation (const CAnimationPtr& pobAnimation);
	void RemoveAnimation (const CAnimationPtr& pobAnimation);

	// This needs to be called if SetTime is called on an anim
	void ResetProgress (const CAnimationPtr& pobAnimation);

	// Clear the existing animation events
	void ClearAnimations( void );

	// Debug function for disabling events
	void ToggleDebugDisable (const CHashedString& obAnimName,const CHashedString& obEventName); 

	// Update the active event monitors
	void Update( void );

	// Monitor a particular message to get a message when it is done
	void GenerateAnimDoneMessage( const CHashedString& pcAnimName );
	void GenerateAnimDoneMessage( const CHashedString& pcAnimName, const char* pcMessageString );
	typedef void ( *CompletionCallback )( CEntity*);
	void GenerateAnimDoneMessageAndCallback( const CHashedString& pcAnimName, const char* pcMessageString, CompletionCallback pcCallback );

	// Get a pointer to the parent entity - DEBUG ONLY
	const CEntity* GetEntity( void ) const { return m_pobParentEntity; }

	void SetDebugMode (bool bEnable) { m_bDebugRender=bEnable; }

protected:
	
	void AddToUpdateList (CAnimEventMonitor* pobMonitor);

	// Our complete list of animation event monitors
	ntstd::List<CAnimEventMonitor*, Mem::MC_ANIMATION> m_obAnimEventMonitorList;


	ntstd::List<CAnimEventMonitor*, Mem::MC_ANIMATION> m_obMonitorUpdateList;

	// A pointer to the entity on which the events will take place
	CEntity* m_pobParentEntity;

	// Do we need to send an animation done message for any anims?
	u_int m_uiAnimDoneHash;
	// If so, in the new style, this is the message we need to send
	ntstd::String m_pcAnimDoneMessageString;
	// If so, in the new style, this is the callback we need to call
	CompletionCallback m_pcAnimDoneCallback; 
	

	bool m_bDebugRender;
};

#endif // ANIM_ANIMEVENTS_H

