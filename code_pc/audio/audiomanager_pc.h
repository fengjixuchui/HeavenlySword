/***************************************************************************************************
*
*   $Header:: /game/audiomanager_pc.h 6     19/08/03 13:24 Harvey                                  $
*
*	Header file for CAudioManager and related classes.
*
*	CHANGES		
*
*	02/07/2003	Harvey	Created
*
***************************************************************************************************/

#ifndef _AUDIOMANAGER_PC_H
#define _AUDIOMANAGER_PC_H

#include "audio/audioresource.h"
#include "audio/audioengine.h"
#include "audio/audiocue.h"
#include "editable/enumlist.h"

// Forward declaration

class Transform;
class CAudioLayer;

// Finished Callback Hack - JML Added 09-12-05
#include "game/LuaGlobal.h"
// New call backs migrated to C++
typedef void (*AudioCallback)(void* pData);



/*
// This is already defined in enumlist.h

enum SPATIALIZATION
{
	SPATIALIZATION_NONE			= 0,	// Ignore 3d positioning
	SPATIALIZATION_ATTENUATED	= 1,	// Linear volume attenuation based on distance from listener
	SPATIALIZATION_HRTF			= 2,	// Full volume attenuation and panning
};
*/






/***************************************************************************************************
*
*	CLASS			CI3DL2Reverb
*
*	DESCRIPTION		I3DL2 reverb parameters.
*
***************************************************************************************************/
class CI3DL2Reverb
{
public:

	CI3DL2Reverb ();

	virtual void PostConstruct ();

	virtual bool EditorChangeValue (CallBackParameter pcItem, CallBackParameter pcValue);

	void Set (AUDIO_EFFECT_I3DL2REVERB& stI3DL2Reverb);

	void Validate ();

	// ---- Serialised data -----

	int 	m_iRoom;
	int 	m_iRoomHF; 
	float	m_fRoomRolloffFactor;
	float	m_fDecayTime;
	float	m_fDecayHFRatio;
	int		m_iReflections;
	float	m_fReflectionsDelay;
	int		m_iReverb;
	float	m_fReverbDelay; 
	float	m_fDiffusion;
	float	m_fDensity;
	float	m_fHFReference;
};

/***************************************************************************************************
*
*	CLASS			CAudioObjectID
*
*	DESCRIPTION		An allocated ID for an audio object.
*
***************************************************************************************************/

class CAudioObjectID
{
public:

	CAudioObjectID ()
	{
		m_iID=0;
	}

	void Invalidate ();

	bool IsNull () const;
	
	int		operator = (const CAudioObjectID& obID);
	
	bool	operator == (const CAudioObjectID& obID) const;

	int m_iID;
};

inline void CAudioObjectID::Invalidate ()
{
	m_iID=0;
}

inline bool CAudioObjectID::IsNull () const
{
	return (m_iID==0 ? true : false);
}

inline	int CAudioObjectID::operator = ( const CAudioObjectID& obID )
{
	m_iID=obID.m_iID;

	return m_iID;
}

inline	bool CAudioObjectID::operator == ( const CAudioObjectID& obID ) const
{
	return (m_iID==obID.m_iID ? true : false);
}













/***************************************************************************************************
*
*	CLASS			CAudioObject
*
*	DESCRIPTION		This is an instance of a sound playing using the properties specified by an
*					audio cue.
*
***************************************************************************************************/

class CAudioObject
{
public:
	enum AUDIO_OBJECT_FLAGS
	{
		FLAG_LOOPING		= 1, // Does this sound loop?
		FLAG_REVERB			= 2, // Directsound reverb EQ
		FLAG_PARAMEQ		= 4, // Directsound param EQ
	};

	enum AUDIO_OBJECT_STATE
	{
		STATE_UNINITIALISED,	// Audio object has just been created
		STATE_INITIALISED,		// Audio object has been successfully created
		STATE_WAITING,			// Audio object is waiting to start
		STATE_FADING_IN,		// Audio object is playing and fading in
		STATE_PLAYING,			// Audio object is playing
		STATE_FADING_OUT,		// Audio object is playing and fading out
		STATE_PENDING_STOP,		// Audio object has already requested a stream stop and is waiting for confirmation
		STATE_FINISHED,			// Audio object is finished playing
	};

	CAudioObject (const CAudioObjectID& obID,CAudioCue* pobCue,CAudioLayer* pobAudioLayer,SPATIALIZATION eSpatialType);
	~CAudioObject ();

	bool Update ();

	void Play ();
	void Stop ();

	void Pause ();
	void Resume ();

	void SetPosition (const CPoint& obPosition); // Used if we wish to position the sound statically (or manually)
	void SetPosition (const Transform* pobTransform,const CPoint& obPosition); // Used if we wish to connect the sound to something

	void SetVolume (float fVolume) { m_fVolume=fVolume; }
	void SetPitch (float fPitch) { m_fPitch=fPitch; }

	void SetI3DL2Reverb (AUDIO_EFFECT_I3DL2REVERB* pstI3DL2Reverb);
	void SetParamEq (AUDIO_EFFECT_PARAMEQ* pstParamEq);

	void SetDelay (float fTime) { m_fStartTime=fTime; } // Temporary until delay variation is implemented

	void SetCategory (unsigned int uiCategoryID) { m_uiCategoryID=uiCategoryID; } // Override the category ID

	void SetName (const CKeyString& obSoundCue,const CKeyString& obSoundBank);
	
	const CAudioObjectID& GetID () const { return m_obID; }

	u_int GetCueID () const { return m_uiCueID; }
	u_int GetCategory () const { return m_uiCategoryID; }
	u_short GetPriority () const { return m_usPriority; }

	float GetStartTime () const { return m_fStartTime; }
	float GetActualVolume () const { return m_pobAudioStream->GetVolume(); }
	float GetActualPitch () const { return m_pobAudioStream->GetPitch(); }


	bool Is3d () const { return (m_eSpatialization!=SPATIALIZATION_NONE ? true : false); }
	bool IsFinished () const { return (m_eState==STATE_FINISHED ? true : false); }
	bool IsInitialised () const { return (m_eState==STATE_UNINITIALISED ? false : true); }

	
	const CKeyString& GetSoundCueName () { return m_obSoundCue; }
	const CKeyString& GetSoundBankName () { return m_obSoundBank; }

	// Finished Callback Hack - JML Added 09-12-05
	void SetFinishCallBack(NinjaLua::LuaObject& lobjFinishMsg) {m_lobjFinishMsg = lobjFinishMsg;}
	void SetFinishCallBack(AudioCallback pFunc, void* pData)   {m_pCallbackFunc = pFunc; m_pCallbackData = pData;}

private:
	// Finished Callback Hack - JML Added 09-12-05
	void SetFinished();

private:
	CAudioStream* m_pobAudioStream; // Pointer to our stream
	CAudioLayer* m_pobAudioLayer; // Pointer to category which this audio object uses
	
	const Transform* m_pobTransform;
	CMatrix m_obOffsetMatrix;
	CPoint m_obWorldPosition;

	CKeyString m_obSoundCue;
	CKeyString m_obSoundBank;

	CAudioObjectID m_obID;
	
	int m_iSoundID; // Allocation ID of CSound object

	AUDIO_OBJECT_STATE m_eState;

	SPATIALIZATION m_eSpatialization;

	u_int m_uiFlags; // Object status flags

	float m_fStartTime; // The time this is actually supposed to start playing
	
	float m_fBaseVolume; // Volume as defined in the cue properties
	float m_fBasePitch; // Pitch as defined in the cue properties

	float m_fVolume; // Volume set externally (relative to the base volume)
	float m_fPitch; // Pitch set externally (relative to the base pitch)
	
	float m_fFadeVolume; // Fade multiplier

	float m_fMinDist;
	float m_fMaxDist;

	float m_fFadeTime;
	float m_fFadeInTime;
	float m_fFadeOutTime;
	VOLUME_CURVE m_eFadeInCurve;
	VOLUME_CURVE m_eFadeOutCurve;

	unsigned char m_ucPlayCount;
	float m_fDelayTime;

	unsigned int m_uiCueID;
	unsigned int m_uiCategoryID;
	unsigned short m_usPriority;

	// Finished Callback Hack - JML Added 09-12-05
	NinjaLua::LuaObject m_lobjFinishMsg;
	// Migrated to C++
	AudioCallback  m_pCallbackFunc;
	void*          m_pCallbackData;
};
















/***************************************************************************************************
*
*	CLASS			CAudioManager
*
*	DESCRIPTION		This is the main interface used by the game to play and manage audio cues.
*
***************************************************************************************************/
class CAudioManager : public Singleton<CAudioManager> // Audio data lookup and storage
{
public:

	CAudioManager ();
	~CAudioManager ();

	void SetConfig (u_int uiMax2dSounds,u_int uiMax3dSounds);

    bool RegisterSoundBank (const char* pcPath);
	bool ReleaseSoundBank (const char* pcFriendlyName);
	void ReleaseAll ();

	void SetListener (const CMatrix& obMatrix);

	void SetI3DL2Reverb (CI3DL2Reverb* pobI3DL2Reverb);

	void Update (float fTime);

	bool Create (CAudioObjectID& obID,const char* pcString,SPATIALIZATION eSpatialType);
	bool Create (CAudioObjectID& obID,const CKeyString& obBank,const CKeyString& obCue,SPATIALIZATION eSpatialType);
	bool Play (const CAudioObjectID& obID);
	// Finished Callback Hack - JML Added 09-12-05
	bool SetCallback(const CAudioObjectID& obID, NinjaLua::LuaObject& pMsg);
	bool SetCallback(const CAudioObjectID& obID, AudioCallback pFunc, void* pData);
	bool Stop (const CAudioObjectID& obID);

	bool SetVolume (const CAudioObjectID& obID,float fVolume);
	bool SetPitch (const CAudioObjectID& obID,float fPitch);
	bool SetPosition (const CAudioObjectID& obID,const CPoint& obPosition);
	bool SetPosition (const CAudioObjectID& obID,const Transform* pobTransform,const CPoint& obPosition);
	bool SetVolumeControl (const CAudioObjectID& obID,const char* pcBank,const char* pcParamName,float fValue);
	bool SetPitchControl (const CAudioObjectID& obID,const char* pcBank,const char* pcParamName,float fValue);
	bool SetParamEqControl (const CAudioObjectID& obID,const char* pcBank,const char* pcParamName,float fValue);
	bool SetDelay (const CAudioObjectID& obID,float fTime); // Temporary until delay variation is implemented
	bool SetCategory (const CAudioObjectID& obID,const char* pcCategory);

	void StopAll (); // Kill all sounds that are playing

	bool IsPlaying (const CAudioObjectID& obID);
	float GetTime () const { return m_fTime; }
	const CPoint& GetListenerPosition () const { return m_obListenerMatrix.GetTranslation(); }

	bool Verify (const CHashedString& obBank,const CHashedString& obCue);

	// easy to use audio trigger system, events throughout the codebase call the following
	// function allowing registered functions to trigger sound FX and music from within Lua
	static void GlobalTrigger( const char* pcEvent );
	static void GlobalTriggerF( const char* pcFormat, ... );

	// ----- Debugging -----
	
	// play a debug sound, i in [0..2] (clamped)
	void PlayDebugSound(int i = 0);

private:

	CI3DL2Reverb* m_pobI3DL2Reverb; // NOTE: This is temporary for the prototype

	AUDIO_EFFECT_I3DL2REVERB m_stI3DL2Reverb;

	ntstd::List<CAudioObject*> m_obAudioObjectList; // Sound instances

	ntstd::List<CSoundBank*> m_obSoundBankList; // Wave banks

	CAudioObjectID m_obGeneratedID; // Variable used to generate IDs for audio objects

	CMatrix m_obListenerMatrix; // Matrix representing the listener

	bool m_bDebugRender;

	bool m_bEnabled; // Is the audiomanager enabled?

	u_int	m_iNextSoundBankToUpdate;

	float m_fTime; // Current time

	u_int m_uiMaximum2dSounds; // Maximum allowable 2d instances
	u_int m_uiMaximum3dSounds; // Maximum allowable 3d instances

	CSoundBank* FindSoundBankP (u_int uiID);
	CAudioObject* GetAudioObjectP (const CAudioObjectID& obID);
	bool CanPlayAudioCue (CAudioCue* pobCue,bool b3d);
};




#endif // _AUDIOMANAGER_PC_H
