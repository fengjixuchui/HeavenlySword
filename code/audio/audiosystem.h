#ifndef _AUDIOSYSTEM_H
#define _AUDIOSYSTEM_H


// Forward declare the FMOD classes
namespace FMOD
{
	class EventSystem;
	class EventCategory;
	class EventGroup;
	class Event;
	class EventParameter;
	class EventProject;
};

typedef void (*AudioCallback)(void* pData);



enum AUDIO_SYSTEM_DEBUG_FLAGS
{
	DEBUG_SYSTEM_INFO			=	0x0001, // FMOD system information
	DEBUG_RENDER_SOUND_POS		=	0x0002, // Render 3d positioned sounds
	DEBUG_TTY_SYSTEM			=	0x0004, // System messages messages
	DEBUG_TTY_RESOURCE			=	0x0008, // Resource messages
	DEBUG_TTY_EVENT_PLAY		=	0x0010, // Event play events
	DEBUG_TTY_EVENT_STOP		=	0x0020, // Event stop events
	DEBUG_TTY_EVENT_CALLBACK	=	0x0040, // Event callbacks
	DEBUG_TTY_EVENT_ERROR		=	0x0080, // Event error events
	DEBUG_TTY_TRIGGER_STATS		=	0x0100, // Show Trigger Statistics
	DEBUG_TTY_HIDE_PAUSED_ENT	=	0x0200, // Hide paused entities in the participants' list
};


#ifdef PLATFORM_PS3

#define MAX_EVENT_INSTANCES		48
#define MAX_FMOD_CHANNELS		48
#define MAX_FMOD_PROJECTS		32

#else // PLATFORM_PC

#define MAX_EVENT_INSTANCES		64
#define MAX_FMOD_CHANNELS		64
#define MAX_FMOD_PROJECTS		32

#endif



//------------------------------------------------------------------------------------------
//!
//! AudioEventInstance
//!	Class for handling an event instance.
//!
//------------------------------------------------------------------------------------------

class AudioEventInstance
{
public:

	AudioEventInstance ();

	bool Init (unsigned long ulID,FMOD::Event* pobEvent);

	void Process ();

	void SetVolume (float fVolume) { m_fUserVolume=fVolume; }
	void SetPitch (float fPitch) { m_fUserPitch=fPitch * 0.25f; }
	void SetPosition (const CPoint& obPosition,float fMaxDistance=0.0f);
	void SetParameterValue (const char* pcParameterName,float fValue);
	void SetParameterVelocity (const char* pcParameterName,float fVelocity);
	void SetParameterSeekSpeed (const char* pcParameterName,float fSeekSpeed);
	void SetCallBack (AudioCallback pFunc, void* pData) { m_pCallbackFunc=pFunc; m_pCallbackData=pData; }

	bool Play ();
	void Stop (bool bImmediate=false);

	void DebugRender ();

	unsigned int GetID () const { return m_uiID; }
	
	bool IsFree () { return (m_pobEvent ? false : true); }
	bool IsPaused ();
	bool Is3d ();

	float GetVolume ();
	float GetPitch ();
	const CPoint& GetPosition () { return m_obPosition; }
	bool GetParameterRange (const char* pcParameterName,float& fMin,float& fMax);

	unsigned int GetState () { return m_uiState; }

	const char* GetEventGroupName ();
	const char* GetEventName ();

protected:

	void UpdateEventVolumePitch ();
	void UpdateEvent3DProperties ();

	unsigned int 		m_uiID;

	FMOD::Event* 		m_pobEvent;
	bool				m_bWaitingForStart;

	unsigned int		m_uiState;

	float				m_fBaseVolume;
	float				m_fBasePitch;
	float 				m_fUserVolume;
	float 				m_fUserPitch;
	float 				m_fFinalVolume;
	float 				m_fFinalPitch;

	CPoint				m_obPosition;
	bool				m_bUpdate3DProperties;
	float				m_fRadiusSqrd;

	AudioCallback		m_pCallbackFunc;
	void*				m_pCallbackData;
};

//------------------------------------------------------------------------------------------
//!
//! AudioSystem
//!	Manager class for FMOD.
//!
//------------------------------------------------------------------------------------------

class AudioProject
{
public:

	AudioProject () :
		m_pobEventProject(0),
		m_iGroupID(0)
	{
	}

	void Unload ();

	const char* GetName ();
	int GetNumEvents ();
	int GetNumGroups ();

	CHashedString			m_obProjectHash;
	FMOD::EventProject*		m_pobEventProject;
	int						m_iGroupID;
};


//------------------------------------------------------------------------------------------
//!
//! AudioSystem
//!	Manager class for FMOD.
//!
//------------------------------------------------------------------------------------------

class AudioSystem : public Singleton<AudioSystem>
{
public:

	friend class AudioConsole;

	AudioSystem ();
	~AudioSystem ();

	// ----- Resource management -----

	void SetMediaPath (const char* pcPath);

	bool LoadProject (const char* pcPath,int iGroupID=0);
	void UnloadProject (const char* pcPath);
	void UnloadProjectGroup (int iGroupID);
	void UnloadAllProjects ();

	void LoadEventGroup (const char* pcGroupName,bool bAsync=false);
	void UnloadEventGroup (const char* pcGroupName);

	// ----- Run-time -----

	void DoWork (); // This should be called in the main-thread with each game update

	void SetListener (int iListener);
	void SetListenerProperties (int iListener,const CMatrix& obWorldMatrix);
	const CPoint& GetListenerPosition ();
	float GetDistanceSqrdFromListener (const CPoint& obPosition);

	bool Sound_Prepare (unsigned int& id,const char* pcGroupName,const char* pcEventName);
	bool Sound_Prepare (unsigned int& id,const char* pcSoundName);
	void Sound_SetVolume (unsigned int id,float fVolume);
	void Sound_SetPitch (unsigned int id,float fPitch);
	void Sound_SetPosition (unsigned int id,const CPoint& obPosition,float fAudioRadius=0.0f);
	void Sound_SetCallback (unsigned int id,AudioCallback pFunc, void* pData);
	void Sound_SetParameterValue (unsigned int id,const char* pcParameterName,float fValue);
	void Sound_SetParameterVelocity (unsigned int id,const char* pcParameterName,float fVelocity);
	void Sound_SetParameterSeekSpeed (unsigned int id,const char* pcParameterName,float fSeekSpeed);
	bool Sound_Play (unsigned int id);
	void Sound_Stop (unsigned int id);
	bool Sound_IsPlaying (unsigned int id);
	bool Sound_IsPaused (unsigned int id);
	bool Sound_GetParameterRange (unsigned int id,const char* pcParameterName,float& fMin,float& fMax);

	void Sound_ResetCallbacks ();
	void Sound_StopAll (bool bImmediate=false);

	bool Reverb_SetActive (const char* pcName,bool bActive);

	// ----- System -----

	FMOD::EventSystem* GetFMODEventSystem () { return m_pobEventSystem; }
	int32_t GetSpeakerCount();
	static int GetSPUThreadCount (); // Return the number of SPU threads that need to be reserved for FMOD

	// ----- Debugging ------

	bool IsInitialised () const { return m_bInitialised; }
	unsigned int GetFMODVersion ();
	unsigned int GetFMODEventVersion ();
	void GetMemoryUsage (int& iCurrent,int& iHigh,int& iMax);
	int GetPlayingChannels ();
	int GetMaxChannels () const { return MAX_FMOD_CHANNELS; }
	void ToggleDebugOption (unsigned int uiFlag);
	bool IsDebugOptionEnabled (unsigned int uiFlag) { return (m_uiDebugFlags & uiFlag ? true : false); }
	void SimpleTest ();

protected:

	AudioEventInstance* FindFreeInstance ();
	AudioEventInstance* FindInstance (unsigned int id);
	unsigned int GenerateHandleID ();

	bool							m_bInitialised;;

	uint8_t*						m_pMemoryPool;

	FMOD::EventSystem*				m_pobEventSystem;

	AudioEventInstance				m_obInstanceList [MAX_EVENT_INSTANCES];

	AudioProject					m_obProject [MAX_FMOD_PROJECTS];

	CMatrix							m_obListenerMatrix;

	CPoint							m_obPreviousListenerPosition;

	bool							m_bUpdateListener;

	unsigned int					m_uiLastID;
	AudioEventInstance*				m_uiLastAudioInstance;

	// ----- Debugging -----
	
	unsigned int					m_uiDebugFlags;
};


#endif // _AUDIOSYSTEM_H

