//------------------------------------------------------------------------------------------
//!
//!	\file audio/ambientsound.h
//!
//------------------------------------------------------------------------------------------

#ifndef AMBIENTSOUND_H
#define AMBIENTSOUND_H

#include "editable/enumlist.h"





//------------------------------------------------------------------------------------------
//!
//! BaseAmbientSoundDefinition
//!	Base type.
//!
//------------------------------------------------------------------------------------------

class BaseAmbientSoundDefinition
{
public:

	BaseAmbientSoundDefinition () :
		m_bActive(true)
	{
	}

	virtual ~BaseAmbientSoundDefinition () {}

	virtual void PostConstruct () = 0;
	virtual bool EditorChangeValue (CallBackParameter pcItem, CallBackParameter pcValue) = 0;

	virtual void Update () = 0;
	virtual void DebugRender () = 0;

	virtual void SetActive (bool bActive) = 0;
	virtual void SetDebug (bool bDebug) = 0;
	virtual void ToggleDebugRender () = 0;
	
	virtual AMBIENT_SOUND_TYPE GetType () = 0;
	virtual bool IsDebugRenderEnabled () = 0;

	bool IsActive () { return m_bActive; }
	const CHashedString& GetName () { return m_obName; }

protected:

	CHashedString			m_obName;

	bool					m_bActive;

};





//------------------------------------------------------------------------------------------
//!
//! AmbientSoundDefinition
//!	An instance of an environment sound.
//!
//------------------------------------------------------------------------------------------

class AmbientSoundDefinition : public BaseAmbientSoundDefinition
{
public:

	AmbientSoundDefinition ();
	~AmbientSoundDefinition ();

	virtual void PostConstruct ();
	virtual bool EditorChangeValue(CallBackParameter pcItem, CallBackParameter pcValue);

	virtual void Update ();
	virtual void DebugRender ();

	virtual void SetActive (bool bActive);
	virtual void SetDebug (bool bDebug) { m_bDebugRender=bDebug; }
	virtual void ToggleDebugRender () { m_bDebugRender=!m_bDebugRender; }

	virtual AMBIENT_SOUND_TYPE GetType () { return m_eType; }
	virtual bool IsDebugRenderEnabled () { return m_bDebugRender; }

	// ----- Serialised data -----

	ntstd::String			m_obSoundBank;
	ntstd::String			m_obSoundCue;

	bool 					m_bEnabled;					// The initial state of the ambient sound
	bool 					m_bDebugRender;				// Toggle for debug info (non release builds only)

	AMBIENT_SOUND_TYPE		m_eType;
	AMBIENT_SOUND_SHAPE		m_eShape;

	CPoint 					m_obTriggerPosition;		// Centre of the trigger volume for this ambient sound
	CDirection				m_obTriggerRotation;		// Orientation of our trigger (if applicable)
	CDirection				m_obTriggerHalfExtents1;	// Inner half extents (if applicable)
	CDirection				m_obTriggerHalfExtents2;	// Outer half extents (if applicable)
	float					m_fTriggerRadius;			// Radius relative to trigger position
	
	CPoint 					m_obRandomOffset;			// Random position relative to the trigger position
	float 					m_fMinDistance;				// Min distance of the actual sound emitter position (used with linear attenuation)
	float 					m_fMaxDistance;				// Max distance of the actual sound emitter position, beyond this the sound will turn off
	
	float 					m_fMinInterval;				// Minimum time in seconds before the next sound is played (intermittent only)
	float 					m_fMaxInterval;				// Maximum time in seconds before the next sound is played (intermittent only)
	int						m_iPlayCount;				// Number of times the ambient sound can be triggered - 0 means infinite (intermittent only)

	float 					m_fFadeInDuration;			// Volume fade in time in seconds
	float 					m_fFadeOutDuration;			// Volume fade out time in seconds

	bool					m_bRespawn;					// Re-occurrence of sounds relative to the first sound (intermittent only)

	int 					m_iRespawnMinInstances;
	int 					m_iRespawnMaxInstances;
	float 					m_fRespawnMinInterval;
	float 					m_fRespawnMaxInterval;

private:

	void SetActualPosition ();
	void FadeIn ();
	void FadeOut ();
	bool IsPaused ();
	void Validate ();
	float CalculateVolumeFromListener ();

	double					m_dTime;

	unsigned int			m_uiSoundHandle;

	int						m_iTimesPlayed;
	float 					m_fVolume;
	float					m_fFadeModifier;
	float 					m_fTimeToPlay;
	CPoint					m_obActualPosition;

	float					m_fNextRespawn;
	int						m_iRespawnCount;

	bool					m_bValidSoundResource;

	CMatrix					m_obTriggerWorldMatrix;
};






//------------------------------------------------------------------------------------------
//!
//! SlaveAmbientSoundDefinition
//!	
//!
//------------------------------------------------------------------------------------------
class SlaveAmbientSoundDefinition
{
public:

	SlaveAmbientSoundDefinition ();
	~SlaveAmbientSoundDefinition ();

	void PostConstruct ();
	bool EditorChangeValue(CallBackParameter pcItem, CallBackParameter pcValue);
	
	void Trigger ();
	void Update (float fTimeDelta);
	void DebugRender ();

	// ----- Serialised members -----

	CKeyString				m_obSound;		// Event group:event

	CPoint					m_obPosition;
	CDirection				m_obRandomOffset;

	float					m_fMinStartOffset;
	float					m_fMaxStartOffset;
	float					m_fProbability;

protected:

	CHashedString			m_obName; // This is only used for debugging

	float					m_fPlayTimer;
};





//------------------------------------------------------------------------------------------
//!
//! MasterAmbientSoundDefinition
//!	
//!
//------------------------------------------------------------------------------------------
class MasterAmbientSoundDefinition : public BaseAmbientSoundDefinition
{
public:

	MasterAmbientSoundDefinition ();
	~MasterAmbientSoundDefinition ();

	virtual void PostConstruct ();
	virtual bool EditorChangeValue(CallBackParameter pcItem, CallBackParameter pcValue);

	virtual void Update ();
	virtual void DebugRender ();

	virtual void SetActive (bool bActive);
	virtual void SetDebug (bool bDebug) { m_bDebugRender=bDebug; }
	virtual void ToggleDebugRender () { m_bDebugRender=!m_bDebugRender; }

	virtual AMBIENT_SOUND_TYPE GetType () { return HRTF_INTERMITTENT; }
	virtual bool IsDebugRenderEnabled () { return m_bDebugRender; }

	// ----- Serialised members -----

	ntstd::List<SlaveAmbientSoundDefinition*>	m_obSlaveList;				// List of slave ambient sounds

	CKeyString									m_obSound;					// The eventgroup:event that gets played

	bool										m_bEnabled;
	bool										m_bDebugRender;

	AMBIENT_SOUND_SHAPE							m_eShape;

	// This represents the region you have to enter in order for this ambient sound to fire off

	CPoint 										m_obTriggerPosition;		// Centre of the trigger volume for this ambient sound
	CDirection									m_obTriggerRotation;		// Orientation of our trigger (if box shape)
	CDirection									m_obTriggerHalfExtents;		// Inner half extents (if box shape)
	float										m_fTriggerRadius;			// Radius relative to trigger position (if sphere shape)
	
	CPoint 										m_obRandomOffset;			// Random position relative to TriggerPosition, where the sound will be positioned
	
	float 										m_fMinInterval;				// Minimum time in seconds before the next sound is played (intermittent only)
	float 										m_fMaxInterval;				// Maximum time in seconds before the next sound is played (intermittent only)

protected:

	CMatrix										m_obTriggerMatrix;

	float										m_fPlayTimer;
};





//------------------------------------------------------------------------------------------
//!
//! AmbientSoundManager
//!	
//!
//------------------------------------------------------------------------------------------

class AmbientSoundManager : public Singleton<AmbientSoundManager>
{
public:

	friend class AudioConsole;
	friend class AmbientSoundDefinition;
	friend class MasterAmbientSoundDefinition;

	AmbientSoundManager ();
	~AmbientSoundManager ();

	void SetActive (bool bActive);

	void Update ();

	void Activate (const CHashedString& obAmbientSound);
	void Deactivate (const CHashedString& obAmbientSound);

	bool IsActive (const CHashedString& obAmbientSound);
	bool IsEnabled () { return m_bActive; }

private:

	void AddAmbientSoundDef (BaseAmbientSoundDefinition* pobAmbientSoundDef);
	void RemoveAmbientSoundDef (BaseAmbientSoundDefinition* pobAmbientSoundDef);

	bool m_bActive;

	ntstd::List<BaseAmbientSoundDefinition*> m_obAmbientSoundDefList;
};


#endif //  AMBIENTSOUND_H
