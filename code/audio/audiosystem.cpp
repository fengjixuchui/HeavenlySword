#include "audio/audiosystem.h"

#include "core/timer.h"

#include "core/visualdebugger.h" // For debugging
#include "input/inputhardware.h" // For debugging
#include "game/shellmain.h" // For debugging

#include "game/shellconfig.h" // Need for game config options

#include "core/gatso.h" // Performance profiling


// Uncomment this line to disable the audio system. Using 'audio_engine = false' in game.config will do the same job.
// If you wish to build the game without FMOD libraries linked, you can comment out _AUDIO_SYSTEM_ENABLE in switches.h.

//#define _DISABLE_AUDIO_SYSTEM 





// FMOD includes
#include "fmod_event.h"
#include "fmod_event_net.h"
#include "fmod_errors.h"



#ifdef PLATFORM_PS3 // -------------------- PS3 PLATFORM --------------------

	#ifdef USE_FIOS_FOR_SYNCH_IO
	#define _FMOD_USE_FIOS // Comment out this line to disable FIOS on ps3 builds
	#endif // USE_FIOS_FOR_SYNCH_IO

	#define _FMOD_USE_SPU_VERSION

	#include "core\fileio_ps3.h" // Needed for FIOS

	#ifdef _FMOD_USE_SPU_VERSION

	#define _FMOD_SPU_THREADS				0 // Number of threads required by FMOD

	#define _FMOD_SPU_MIXER_PRIORITY		201
	#define _FMOD_SPU_MPEG_PRIORITY			250
	#define _FMOD_SPU_ATRAC3_PRIORITY		250

	#define SPU_MIXER_ELF					"fmodex_spu.self"
	#define SPU_MPEG_ELF					"fmodex_spu_mpeg.self"
	//#define SPU_MIXER_ELF					"fmodex_spurs.self"
	//#define SPU_MPEG_ELF					"fmodex_spurs_mpeg.self"

	#include "fmodps3.h" // Required for SPU version
	#include "exec\ppu\exec_ps3.h"
	#include "exec\ppu\elfmanager.h"
	#include "exec\ppu\spuprogram_ps3.h"
	//CellSpurs sFmodSpurs;

	#endif // _FMOD_USE_SPU_VERSION

	#define _FMOD_MEMORY_ALLOCATION			(15 * 1024 * 1024)

#else // -------------------- PC PLATFORM --------------------

	#define _FMOD_MEMORY_ALLOCATION			(32 * 1024 * 1024)

#endif




#ifndef _GOLD_MASTER

	#define _FMOD_NETWORK_ENABLE // No live editing on gold master builds

#endif // _GOLD_MASTER






#define _FMOD_MAX_MPEG_DECODERS		16			// Default: 16
#define _FMOD_MAX_ADPCM_DECODERS	12			// Default: 32

//#define _FMOD_DSP_BUFFERSIZE		1024		// Default: 1024
//#define _FMOD_DSP_NUMBUFFERS		4			// Default: 4

//#define _FMOD_STREAM_BUFFERSIZE		(32*1024)	// Default: 16384

#define _FMOD_EVENT_MODE			EVENT_NONBLOCKING

#define _DEFAULT_DEBUG_FLAGS		(DEBUG_TTY_SYSTEM | DEBUG_TTY_RESOURCE | DEBUG_TTY_HIDE_PAUSED_ENT)
#define _FMOD_CACHE_EVENTS			false






/*
#ifndef _RELEASE

// Debug functions for displaying debug messages for FMOD results

void FMOD_DebugSuccess (FMOD_RESULT eResult,unsigned int uiDebugFlag,const char* restrict pcFormat, ...)
{
	if (eResult==FMOD_OK && AudioSystem::Get().IsDebugOptionEnabled(uiDebugFlag))
	{
		char acBuffer[ 256 ];
		acBuffer[ sizeof( acBuffer ) - 1 ] = 0;
		int iResult;
		va_list	stArgList;
		va_start( stArgList, pcFormat );
		iResult = vsnprintf( acBuffer, sizeof(acBuffer) - 1, pcFormat, stArgList );
		va_end( stArgList );

		ntPrintf(acBuffer);
	}
}

void FMOD_DebugFail (FMOD_RESULT eResult,unsigned int uiDebugFlag,const char* restrict pcFormat, ...)
{
	if (eResult!=FMOD_OK && AudioSystem::Get().IsDebugOptionEnabled(uiDebugFlag))
	{
		char acBuffer[ 256 ];
		acBuffer[ sizeof( acBuffer ) - 1 ] = 0;
		int iResult;
		va_list	stArgList;
		va_start( stArgList, pcFormat );
		iResult = vsnprintf( acBuffer, sizeof(acBuffer) - 1, pcFormat, stArgList );
		va_end( stArgList );

		ntPrintf(acBuffer);
	}
}

#endif // _RELEASE
*/













#ifdef _FMOD_USE_FIOS

#define _FMOD_FILESYSTEM_BLOCKSIZE		(16*1024) //(48*1024) // Transfer block size (from my tests 48kb/s seems fairly optimal from hostfs or hard disk)

// ---------- FMOD file system hooks ----------

FMOD_RESULT F_CALLBACK FMOD_FileOpen( const char *  name, int  unicode, unsigned int *  filesize, void **  handle, void **  /*userdata*/ )
{
	int iHandle=FileManager::open(name,O_RDONLY, BLU_RAY_MEDIA);

	if (iHandle!=-1)
	{
		*handle=(void*)iHandle;
		*filesize=FileManager::getFileSize(iHandle);

		//ntPrintf("FMOD_FileOpen: %s - unicode=%d  filesize=%d  handle=%d\n",name,unicode,*filesize,iHandle);

		return FMOD_OK;
	}

	return FMOD_ERR_FILE_NOTFOUND;
}

FMOD_RESULT F_CALLBACK  FMOD_FileClose( void *  handle, void *  /*userdata*/ )
{
	FileManager::close((int)handle);

	return FMOD_OK;
}

FMOD_RESULT F_CALLBACK  FMOD_FileRead( void *  handle, void *  buffer, unsigned int  sizebytes, unsigned int *  bytesread, void *  /*userdata*/ )
{
	ssize_t readBytes=FileManager::read((int)handle,buffer,sizebytes);

	if (readBytes==-1)
	{
		*bytesread=0;
		return FMOD_ERR_FILE_EOF;
	}

	*bytesread=(unsigned int)readBytes;

	if (*bytesread < sizebytes)
		return FMOD_ERR_FILE_EOF;

	return FMOD_OK;
}

FMOD_RESULT F_CALLBACK  FMOD_FileSeek( void *  handle, unsigned int  pos, void *  /*userdata*/ )
{
	off_t seekPos=FileManager::lseek((int)handle,pos,0);

	if (seekPos==((off_t)-1))
		return FMOD_ERR_FILE_COULDNOTSEEK;

	return FMOD_OK;
}

#endif // _FMOD_USE_FIOS















//----------------------------------------------------------------------------------------------------------------------------------------------------------------------

AudioEventInstance::AudioEventInstance () :
	m_uiID(0),
	m_pobEvent(0),
	m_uiState(0),
	m_fBaseVolume(1.0f),
	m_fBasePitch(0.0f),
	m_fUserVolume(1.0f),
	m_fUserPitch(0.0f),
	m_fFinalVolume(1.0f),
	m_fFinalPitch(0.0f),
	m_obPosition(CONSTRUCT_CLEAR),
	m_bUpdate3DProperties(false),
	m_fRadiusSqrd(0.0f),
	m_pCallbackFunc(0),
	m_pCallbackData(0)
{
}
 
bool AudioEventInstance::Init (unsigned long id,FMOD::Event* pobEvent)
{
#ifdef _AUDIO_SYSTEM_ENABLE

	if (m_uiID || m_pobEvent) // Event instance is already in use... but this shouldn't ever be the case
		return false;

	m_uiID=id;
	m_pobEvent=pobEvent;

	m_bWaitingForStart=true;

	m_pobEvent->getVolume(&m_fBaseVolume);
	m_pobEvent->getPitch(&m_fBasePitch);
	m_fUserVolume=1.0f;
	m_fUserPitch=0.0f;
	m_fFinalVolume=m_fBaseVolume;
	m_fFinalPitch=m_fBasePitch;
	m_obPosition.Clear();
	m_bUpdate3DProperties=true;
	m_fRadiusSqrd=0.0f;
	m_pCallbackFunc=0;
	m_pCallbackData=0;
	m_uiState=0;

	return true;

#else

	UNUSED(id);
	UNUSED(pobEvent);

	return false;

#endif // _AUDIO_SYSTEM_ENABLE
}

void AudioEventInstance::Process ()
{
#ifdef _AUDIO_SYSTEM_ENABLE

	if (!m_pobEvent)
		return;

	if (m_pobEvent->getState(&m_uiState)!=FMOD_OK || // We are unable to get the state
		(m_uiState & EVENT_STATE_ERROR) || // There has been an error in the state
		(!m_bWaitingForStart && !(m_uiState & EVENT_STATE_CHANNELSACTIVE))) // The event has finished playing
	{
		// This event has finished

		if (m_pCallbackFunc) // Trigger complete callback if one has been set
		{
			#ifndef _RELEASE
			if (AudioSystem::Get().IsDebugOptionEnabled(DEBUG_TTY_EVENT_CALLBACK))
			{
				ntPrintf("AudioEventInstance: Firing callback for %s (ID:%d)\n",GetEventName(),GetID());
			}
			#endif // _RELEASE

			m_pCallbackFunc(m_pCallbackData);
			m_pCallbackFunc=0;
			m_pCallbackData=0;
		}

		// This event is no longer valid
		m_uiID=0;
		m_pobEvent=0; 

		return;
	}
	else if (m_bWaitingForStart && (m_uiState & EVENT_STATE_CHANNELSACTIVE)) // Check to see if the event has started playing
	{
		m_bWaitingForStart=false;
	}


	UpdateEventVolumePitch();
	UpdateEvent3DProperties();

#endif // _AUDIO_SYSTEM_ENABLE
}

void AudioEventInstance::SetPosition (const CPoint& obPosition,float fMaxDistance)
{
#ifdef _AUDIO_SYSTEM_ENABLE

	if (m_pobEvent && Is3d())
	{
		m_obPosition=obPosition;
		m_bUpdate3DProperties=true;	
		m_fRadiusSqrd=fMaxDistance * fMaxDistance;
	}

#else

	UNUSED(obPosition);

#endif // _AUDIO_SYSTEM_ENABLE
}

void AudioEventInstance::SetParameterValue (const char* pcParameterName,float fValue)
{
#ifdef _AUDIO_SYSTEM_ENABLE

	if (m_pobEvent)
	{
		FMOD::EventParameter* pobParameter;

		if (m_pobEvent->getParameter(pcParameterName,&pobParameter)==FMOD_OK)
		{
			pobParameter->setValue(fValue);
		}
	}

#else

	UNUSED(pcParameterName);
	UNUSED(fValue);

#endif // _AUDIO_SYSTEM_ENABLE
}

void AudioEventInstance::SetParameterVelocity (const char* pcParameterName,float fVelocity)
{
#ifdef _AUDIO_SYSTEM_ENABLE

	if (m_pobEvent)
	{
		FMOD::EventParameter* pobParameter;

		if (m_pobEvent->getParameter(pcParameterName,&pobParameter)==FMOD_OK)
		{
			pobParameter->setVelocity(fVelocity);
		}
	}

#else

	UNUSED(pcParameterName);
	UNUSED(fValue);

#endif // _AUDIO_SYSTEM_ENABLE
}

void AudioEventInstance::SetParameterSeekSpeed (const char* pcParameterName,float fSeekSpeed)
{
#ifdef _AUDIO_SYSTEM_ENABLE

	if (m_pobEvent)
	{
		FMOD::EventParameter* pobParameter;

		if (m_pobEvent->getParameter(pcParameterName,&pobParameter)==FMOD_OK)
		{
			pobParameter->setSeekSpeed(fSeekSpeed);
		}
	}

#else

	UNUSED(pcParameterName);
	UNUSED(fValue);

#endif // _AUDIO_SYSTEM_ENABLE
}

bool AudioEventInstance::Play ()
{
#ifdef _AUDIO_SYSTEM_ENABLE

	if (!m_pobEvent) // We don't have a valid event
		return false;

	// Check to see if this event is 3d
	if (Is3d())
	{
		float fRadius;
		float fDistSqrd=AudioSystem::Get().GetDistanceSqrdFromListener(m_obPosition); // Get the squared distance of this sound from listener

		if (m_pobEvent->getProperty("radius",&fRadius)==FMOD_OK) // Check to see if we have a radius user property on the event
		{
			if (fDistSqrd > (fRadius * fRadius)) // The listener is outside the user property radius, so don't play it
			{
				#ifndef _RELEASE
				if (AudioSystem::Get().IsDebugOptionEnabled(DEBUG_TTY_EVENT_PLAY))
				{
					ntPrintf("AudioEventInstance: Skipping playback of event %s (ID:%d), listener is outside its radius %.2f\n",GetEventName(),GetID(),fRadius);
				}
				#endif // _RELEASE

				// This event is now finished
				m_pobEvent=0;
				m_uiID=0;
				m_pCallbackFunc=0;
				m_pCallbackData=0;
				return false;
			}
		}
		else if (m_fRadiusSqrd!=0.0f && fDistSqrd>m_fRadiusSqrd) // If there is no user property radius, use RadiusSqrd value instead
		{
			#ifndef _RELEASE
			if (AudioSystem::Get().IsDebugOptionEnabled(DEBUG_TTY_EVENT_PLAY))
			{
				ntPrintf("AudioEventInstance: Skipping playback of event %s (ID:%d), listener is outside its radius %.2f\n",GetEventName(),GetID(),fsqrtf(m_fRadiusSqrd));
			}
			#endif // _RELEASE

			// This event is now finished
			m_pobEvent=0;
			m_uiID=0;
			m_pCallbackFunc=0;
			m_pCallbackData=0;
			return false;	
		}
	}

	UpdateEventVolumePitch();
	UpdateEvent3DProperties();

	FMOD_RESULT eResult;

	CGatso::Start("FMOD:Event:start");
	eResult=m_pobEvent->start();
	CGatso::Stop("FMOD:Event:start");

	#ifndef _RELEASE
	if (eResult==FMOD_OK && AudioSystem::Get().IsDebugOptionEnabled(DEBUG_TTY_EVENT_PLAY))
	{
		ntPrintf("AudioEventInstance: Playing event %s (ID:%d)\n",GetEventName(),GetID());
	}
	else if (eResult!=FMOD_OK && AudioSystem::Get().IsDebugOptionEnabled(DEBUG_TTY_EVENT_ERROR))
	{
		ntPrintf("AudioEventInstance: Failed to play event %s (ID:%d) - %s\n",GetEventName(),GetID(),FMOD_ErrorString(eResult));
	}
	#endif // _RELEASE

	if (eResult!=FMOD_OK) // Oh dear, somethings gone wrong...
	{
		m_pobEvent=0;
		m_uiID=0;
		m_pCallbackFunc=0;
		m_pCallbackData=0;
		return false;
	}

	return true;

#endif // _AUDIO_SYSTEM_ENABLE
}

void AudioEventInstance::Stop (bool bImmediate)
{
#ifdef _AUDIO_SYSTEM_ENABLE

	if (m_pobEvent)
	{
		FMOD_RESULT eResult;

		eResult=m_pobEvent->stop(bImmediate);

		#ifndef _RELEASE

		if (eResult==FMOD_OK && AudioSystem::Get().IsDebugOptionEnabled(DEBUG_TTY_EVENT_STOP))
		{
			ntPrintf("AudioEventInstance: Stopping event %s (ID:%d)\n",GetEventName(),GetID());
		}
		else if (eResult!=FMOD_OK && AudioSystem::Get().IsDebugOptionEnabled(DEBUG_TTY_EVENT_ERROR))
		{
			ntPrintf("AudioEventInstance: Failed to stop event %s (ID:%d) - %s\n",GetEventName(),GetID(),FMOD_ErrorString(eResult));
		}
		#endif // _RELEASE
	}

#endif // _AUDIO_SYSTEM_ENABLE
}

void AudioEventInstance::DebugRender ()
{
#ifdef _AUDIO_SYSTEM_ENABLE
#ifndef _GOLD_MASTER
//#ifndef _RELEASE

	if (m_pobEvent && ShellMain::Get().HaveLoadedLevel() && Is3d())
	{
		FMOD_VECTOR position;

		if (m_pobEvent->get3DAttributes(&position,0,0)==FMOD_OK) // Attempt to get the position from FMOD for this event
		{
			CPoint obPosition(position.x,position.y,position.z);

			g_VisualDebug->Printf3D(obPosition, 0.0f,-28.0f,0xffffffff,DTF_ALIGN_HCENTRE,GetEventName()); // Display the name of this event

			CDirection obDiff(obPosition - AudioSystem::Get().GetListenerPosition());

			g_VisualDebug->Printf3D(obPosition, 0.0f,-15.0f,0xffffffff,DTF_ALIGN_HCENTRE,"Dist:%0.2f\0",obDiff.Length()); // Display the distance of this event from the listener

			const float fLINE_HALFLENGTH = 0.1f;

			// Display axis which represent the position of this event instance
			g_VisualDebug->RenderLine(CPoint(obPosition.X()-fLINE_HALFLENGTH,obPosition.Y(),obPosition.Z()),CPoint(obPosition.X()+fLINE_HALFLENGTH,obPosition.Y(),obPosition.Z()),0xffffffff);
			g_VisualDebug->RenderLine(CPoint(obPosition.X(),obPosition.Y()-fLINE_HALFLENGTH,obPosition.Z()),CPoint(obPosition.X(),obPosition.Y()+fLINE_HALFLENGTH,obPosition.Z()),0xffffffff);
			g_VisualDebug->RenderLine(CPoint(obPosition.X(),obPosition.Y(),obPosition.Z()-fLINE_HALFLENGTH),CPoint(obPosition.X(),obPosition.Y(),obPosition.Z()+fLINE_HALFLENGTH),0xffffffff);

			// Display min distance sphere around the event position
			float fMinDistance;
			
			if (m_pobEvent->getPropertyByIndex(FMOD::EVENTPROPERTY_3D_MINDISTANCE,&fMinDistance)==FMOD_OK)
			{
				#ifdef PLATFORM_PS3
				g_VisualDebug->RenderSphere(CVecMath::GetQuatIdentity(),obPosition,fMinDistance,0xffffff33,DPF_NOCULLING);
				#else // PLATFORM_PC
				g_VisualDebug->RenderSphere(CVecMath::GetQuatIdentity(),obPosition,fMinDistance,0x33ffffff,DPF_NOCULLING);
				#endif
			}
		}
	}

//#endif // _RELEASE
#endif
#endif // _AUDIO_SYSTEM_ENABLE
}

const char* AudioEventInstance::GetEventGroupName ()
{
#ifdef _AUDIO_SYSTEM_ENABLE
//#ifndef _RELEASE

	if (m_pobEvent)
	{
		FMOD::EventGroup* pobGroup;
		
		if (m_pobEvent->getParentGroup(&pobGroup)==FMOD_OK)
		{
			char* pcGroupName=0;
			pobGroup->getInfo(0,&pcGroupName);

			return pcGroupName;
		}
	}

	return 0;

//#endif // _RELEASE
#endif // _AUDIO_SYSTEM_ENABLE

	return 0;
}

const char* AudioEventInstance::GetEventName ()
{
#ifdef _AUDIO_SYSTEM_ENABLE
//#ifndef _RELEASE

	if (m_pobEvent)
	{
		char* pcEventName=0;

		if (m_pobEvent->getInfo(0,&pcEventName,0)==FMOD_OK)
		{
			return pcEventName;
		}
	}

//#endif // _RELEASE
#endif // _AUDIO_SYSTEM_ENABLE

	return 0;
}

float AudioEventInstance::GetVolume ()
{
#ifdef _AUDIO_SYSTEM_ENABLE
	if (m_pobEvent)
	{
		float fVolume;

		if (m_pobEvent->getVolume(&fVolume)==FMOD_OK)
			return fVolume;
	}
#endif // _AUDIO_SYSTEM_ENABLE

	return 0.0f;
}

float AudioEventInstance::GetPitch ()
{
#ifdef _AUDIO_SYSTEM_ENABLE

	if (m_pobEvent)
	{
		float fPitch;

		if (m_pobEvent->getPitch(&fPitch)==FMOD_OK)
			return fPitch * 4.0f; // Convert result to octaves
	}
#endif // _AUDIO_SYSTEM_ENABLE

	return 0.0f;
}

bool AudioEventInstance::GetParameterRange (const char* pcParameterName,float& fMin,float &fMax)
{
#ifdef _AUDIO_SYSTEM_ENABLE

	if (m_pobEvent)
	{
		FMOD::EventParameter* pobParameter;

		if (m_pobEvent->getParameter(pcParameterName,&pobParameter)==FMOD_OK)
		{
			if (pobParameter->getRange(&fMin,&fMax)==FMOD_OK)
			{
				return true;
			}
		}
	}

#else

	UNUSED(pcParameterName);
	UNUSED(fMin);
	UNUSED(fValuefMax;

#endif // _AUDIO_SYSTEM_ENABLE

	return false;
}

void AudioEventInstance::UpdateEventVolumePitch ()
{
#ifdef _AUDIO_SYSTEM_ENABLE

	// Update the volume and pitch if necessary
	float fVolume=m_fBaseVolume * m_fUserVolume;
	float fPitch=m_fBasePitch + m_fUserPitch;

	if (fVolume!=m_fFinalVolume)
	{
		m_fFinalVolume=fVolume;
		m_pobEvent->setVolume(fVolume);
	}

	if (fPitch!=m_fFinalPitch)
	{
		m_fFinalPitch=fPitch;
		m_pobEvent->setPitch(fPitch);
	}

#endif // _AUDIO_SYSTEM_ENABLE
}

void AudioEventInstance::UpdateEvent3DProperties ()
{
#ifdef _AUDIO_SYSTEM_ENABLE

	if (m_bUpdate3DProperties)
	{
		FMOD_VECTOR position;
		position.x=m_obPosition.X();
		position.y=m_obPosition.Y();
		position.z=m_obPosition.Z();

		FMOD_VECTOR velocity;
		velocity.x=0.0f;
		velocity.y=0.0f;
		velocity.z=0.0f;
		
		#ifdef _RELEASE

		m_pobEvent->set3DAttributes(&position,&velocity,0);

		#else

		FMOD_RESULT eResult=m_pobEvent->set3DAttributes(&position,0,0);

		if (eResult!=FMOD_OK)
		{
			if (AudioSystem::Get().IsDebugOptionEnabled(DEBUG_TTY_EVENT_ERROR))
				ntPrintf("AudioEventInstance: Failed to set 3d properties on ID %d - %s\n",GetID(),FMOD_ErrorString(eResult));
			
		}

		#endif // _RELEASE

		m_bUpdate3DProperties=false;
	}

#endif // _AUDIO_SYSTEM_ENABLE
}

bool AudioEventInstance::IsPaused ()
{
#ifdef _AUDIO_SYSTEM_ENABLE

	if (m_pobEvent)
	{
		bool bPaused;

		if (m_pobEvent->getPaused(&bPaused)==FMOD_OK)
		{
			return bPaused;
		}
	}

#endif // _AUDIO_SYSTEM_ENABLE
	
	return false;
}

bool AudioEventInstance::Is3d ()
{
#ifdef _AUDIO_SYSTEM_ENABLE
	if (m_pobEvent)
	{
		FMOD_MODE eMode;

		m_pobEvent->getPropertyByIndex(FMOD::EVENTPROPERTY_MODE,&eMode);

		if (eMode==FMOD_3D)
			return true;
	}
#endif // _AUDIO_SYSTEM_ENABLE

	return false;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------------

void AudioProject::Unload ()
{
	if (m_pobEventProject)
	{
		m_pobEventProject->release();
		m_pobEventProject=0;
	}
}

const char* AudioProject::GetName ()
{
	if (m_pobEventProject)
	{
		int index;
		char* name;

		if (m_pobEventProject->getInfo(&index,&name)==FMOD_OK)
		{
			return name;
		}
	}

	return 0;
}

int AudioProject::GetNumEvents ()
{
	if (m_pobEventProject)
	{
		int events;

		if (m_pobEventProject->getNumEvents(&events)==FMOD_OK)
			return events;
	}

	return 0;
}

int AudioProject::GetNumGroups ()
{
	if (m_pobEventProject)
	{
		int groups;

		if (m_pobEventProject->getNumGroups(&groups)==FMOD_OK)
			return groups;
	}

	return 0;
}


 //----------------------------------------------------------------------------------------------------------------------------------------------------------------------

AudioSystem::AudioSystem () :
	m_bInitialised(false),
	m_pMemoryPool(0),
	m_pobEventSystem(0),
	m_obListenerMatrix(CONSTRUCT_IDENTITY),
	m_obPreviousListenerPosition(CONSTRUCT_CLEAR),
	m_uiLastID(0),
	m_uiLastAudioInstance(0),
	m_uiDebugFlags(_DEFAULT_DEBUG_FLAGS)
{
#ifndef _DISABLE_AUDIO_SYSTEM
#ifdef _AUDIO_SYSTEM_ENABLE

	ntPrintf("-------------------- Audio System --------------------\n");

	if (!g_ShellOptions->m_bAudioEngine) // Check to see if the audio engine had been disabled from the config
	{
		ntPrintf("AudioSystem: Disabled from game config\n");
		ntPrintf("------------------------------------------------------\n");
		return;
	}

	FMOD_RESULT eResult;

	// ----- Setup memory allocator -----
	m_pMemoryPool = (uint8_t*)NT_MEMALIGN_CHUNK(Mem::MC_AUDIO, _FMOD_MEMORY_ALLOCATION, 128);

	if (m_pMemoryPool)
	{
		ntPrintf("AudioSystem: Created %dkb memory pool for FMOD\n",_FMOD_MEMORY_ALLOCATION / 1024);
	}
	else
	{
		ntPrintf("AudioSystem: Failed to allocate %dkb for memory pool\n",_FMOD_MEMORY_ALLOCATION / 1024);
		return;
	}

	eResult=FMOD::Memory_Initialize(m_pMemoryPool, _FMOD_MEMORY_ALLOCATION, 0, 0, 0);

	if (eResult==FMOD_OK)
	{
		ntPrintf("AudioSystem: Memory allocator for FMOD initialised\n");
	}
	else
	{
		ntPrintf("AudioSystem: Failed to initialize FMOD memory - %s\n",FMOD_ErrorString(eResult));
		return;
	}

	// ----- Create the FMOD event system -----
	eResult=FMOD::EventSystem_Create(&m_pobEventSystem);

	if (eResult==FMOD_OK)
	{
		ntPrintf("AudioSystem: Created FMOD event system - version %x\n",FMOD_EVENT_VERSION);
	}
	else
	{
		ntPrintf("AudioSystem: Failed to create FMOD event system - %s\n",FMOD_ErrorString(eResult));
		return;
	}

#ifdef PLATFORM_PS3


	//*
	#ifdef _FMOD_USE_SPU_VERSION

	// ----- FMOD SPU VERSION -----
	char fmodSpuPath [MAX_PATH];
	Util::GetSpuProgramPath( SPU_MIXER_ELF, fmodSpuPath, MAX_PATH );

	char fmodMpegSpuPath [MAX_PATH];
	Util::GetSpuProgramPath( SPU_MPEG_ELF, fmodMpegSpuPath, MAX_PATH );

	//printf("FMOD spu mixer=%s\n",fmodSpuPath);
	//printf("FMOD spu mpeg decoder=%s\n",fmodMpegSpuPath);

	//*
	FMOD_PS3_EXTRADRIVERDATA extradriverdata;
	memset(&extradriverdata, 0, sizeof(FMOD_PS3_EXTRADRIVERDATA));
	extradriverdata.spu_mixer_elfname_or_spursdata      = fmodSpuPath;
	extradriverdata.spu_streamer_elfname_or_spursdata   = fmodMpegSpuPath;
	extradriverdata.spu_priority_mixer                  = _FMOD_SPU_MIXER_PRIORITY;
	extradriverdata.spu_priority_streamer               = _FMOD_SPU_MPEG_PRIORITY;
	extradriverdata.spu_priority_at3                    = _FMOD_SPU_ATRAC3_PRIORITY;
	extradriverdata.spurs 								= 0;
    extradriverdata.force5point1     = 1;   /* This should always be set to 1 */
    extradriverdata.attenuateDDLFE   = 1;   /* This should always be set to 1 */

	eResult=m_pobEventSystem->init(MAX_FMOD_CHANNELS, FMOD_INIT_NORMAL | FMOD_INIT_3D_RIGHTHANDED, (void *)&extradriverdata, EVENT_INIT_NORMAL);
	//*/

	/*
	// FMOD SPURS Implementation

	if (!ElfManager::Get().Load(SPU_MIXER_ELF)) // Load mixer spu image
	{
		ntPrintf("AudioSystem: Failed to load %s\n",SPU_MIXER_ELF);
		return;
	}

	if (!ElfManager::Get().Load(SPU_MPEG_ELF)) // Load mpeg decoder image
	{
		ntPrintf("AudioSystem: Failed to load %s\n",SPU_MPEG_ELF);
		return;
	}

	const SPUProgram* pobMixerProgram=ElfManager::Get().GetProgram(SPU_MIXER_ELF);
	const SPUProgram* pobMpegProgram=ElfManager::Get().GetProgram(SPU_MPEG_ELF);

	ntError(pobMixerProgram);
	ntError(pobMpegProgram);

	//CellSpurs* pobFmodSpurs=(CellSpurs*)NT_MEMALIGN_CHUNK(Mem::MC_MISC, sizeof(CellSpurs), 128);

	//ntError(pobFmodSpurs);

	cellSpursInitialize( // Initialise our FMOD spurs
		&sFmodSpurs, //pobFmodSpurs, // Spurs object
		2, // 2 SPU threads
		250,//250, // SPU priority
		999, // PPU priority
		false); // Exit if no work

	FMOD_PS3_EXTRADRIVERDATA extradriverdata;
	memset(&extradriverdata, 0, sizeof(FMOD_PS3_EXTRADRIVERDATA));
	extradriverdata.spu_mixer_elfname_or_spursdata      = (void*)pobMixerProgram->GetModule().GetAddress();
	extradriverdata.spu_streamer_elfname_or_spursdata   = (void*)pobMpegProgram->GetModule().GetAddress();
	extradriverdata.spu_priority_mixer                  = 16;  // Default - THIS WILL BE IGNORED
	extradriverdata.spu_priority_streamer               = 200; // Default - THIS WILL BE IGNORED
	extradriverdata.spu_priority_at3                    = 200;
	extradriverdata.spurs 								= &sFmodSpurs; //pobFmodSpurs;

	eResult=m_pobEventSystem->init(MAX_FMOD_CHANNELS, FMOD_INIT_NORMAL | FMOD_INIT_3D_RIGHTHANDED, (void *)&extradriverdata, EVENT_INIT_NORMAL);
	//*/


	/*
	CKeyString obMpegDecoderSPU(SPU_MPEG_ELF);

	if (!ElfManager::Get().Load(obMpegDecoderSPU))
	{
		ntPrintf("AudioSystem: Failed to load %s\n",obMpegDecoderSPU.GetString());
		return;
	}

	const SPUProgram* pobSPUProgram=ElfManager::Get().GetProgram(obMpegDecoderSPU);

	if (!pobSPUProgram || !pobSPUProgram->GetModule().GetAddress())
	{
		ntPrintf("AudioSystem: Failed to get SPU program %s from Elfmanager\n",obMpegDecoderSPU.GetString());
		return;
	}

	int result = cellSpursInitialize(
								&sFmodSpurs, 
								1,  // 1 SPU
								250, // SPU Thread group priority
								2000, // PPU Thread priority
								true // Exit if no work
								);

	ntAssert( result == CELL_OK );
	ntPrintf( " result = %d\n", result );


	FMOD_PS3_EXTRADRIVERDATA extradriverdata;
	memset(&extradriverdata, 0, sizeof(FMOD_PS3_EXTRADRIVERDATA));

    extradriverdata.spu_mixer_elfname_or_spursdata = fmodSpuPath;
    extradriverdata.spu_streamer_elfname_or_spursdata = (void*)pobSPUProgram->GetModule().GetAddress(); // SPURS
	//extradriverdata.spu_streamer_elfname_or_spursdata = fmodMpegSpuPath; // SPU Thread
    extradriverdata.spu_priority_mixer = 16;
    extradriverdata.spu_priority_at3 = 202;
    extradriverdata.spu_priority_streamer = 201;
	extradriverdata.spurs = &sFmodSpurs; // SPURS
	//extradriverdata.spurs = 0; // SPU Thread

    eResult = m_pobEventSystem->init(MAX_FMOD_CHANNELS, FMOD_INIT_NORMAL | FMOD_INIT_3D_RIGHTHANDED, (void *)&extradriverdata, EVENT_INIT_NORMAL);
	//*/

	#else // We are using PPU version of FMOD
	//*/

	// ----- FMOD PPU VERSION -----

	eResult=m_pobEventSystem->init(MAX_FMOD_CHANNELS, FMOD_INIT_NORMAL | FMOD_INIT_3D_RIGHTHANDED, 0, EVENT_INIT_NORMAL);

	#endif // _FMOD_USE_SPU_VERSION
	

#else // PLATFORM_PC

	// Initialize the event system
	eResult=m_pobEventSystem->init(MAX_FMOD_CHANNELS, FMOD_INIT_NORMAL | FMOD_INIT_3D_RIGHTHANDED, 0, EVENT_INIT_NORMAL);

#endif


	if (eResult==FMOD_OK)
	{
		ntPrintf("AudioSystem: Successfully initialised FMOD event system (max %d instances)\n",MAX_EVENT_INSTANCES);
	}
	else
	{
		ntPrintf("AudioSystem: Failed to initialise FMOD event system - %s\n",FMOD_ErrorString(eResult));
		return;
	}

	// ----- Get FMOD system object -----

	FMOD::System* pobSystem=0;

	eResult=m_pobEventSystem->getSystemObject(&pobSystem);

	if (eResult!=FMOD_OK)
	{
		ntPrintf("AudioSystem: Failed to acquire system object - %s\n",FMOD_ErrorString(eResult));
		return;
	}

	// ----- Set advanced settings -----

#ifdef _FMOD_MAX_ADPCM_DECODERS 

	FMOD_ADVANCEDSETTINGS advancedsettings;
	advancedsettings.maxADPCMcodecs=_FMOD_MAX_ADPCM_DECODERS;
	advancedsettings.maxMPEGcodecs=_FMOD_MAX_MPEG_DECODERS;
	advancedsettings.maxXMAcodecs=0;
	advancedsettings.ASIOChannelList=0;
	advancedsettings.ASIONumChannels=0;
	advancedsettings.cbsize=sizeof(FMOD_ADVANCEDSETTINGS);

	eResult=pobSystem->setAdvancedSettings(&advancedsettings);

	if (eResult!=FMOD_OK)
	{
		ntPrintf("AudioSystem: Failed to configure advanced settings - %s\n",FMOD_ErrorString(eResult));
		return;
	}

#endif // _FMOD_MAX_ADPCM_DECODERS

#ifdef _FMOD_DSP_BUFFERSIZE

	eResult=pobSystem->setDSPBufferSize(_FMOD_DSP_BUFFERSIZE,_FMOD_DSP_NUMBUFFERS);

	if (eResult!=FMOD_OK)
	{
		ntPrintf("AudioSystem: Failed to set DSP buffersize to %d x %d - %s\n",_FMOD_DSP_BUFFERSIZE,_FMOD_DSP_NUMBUFFERS,FMOD_ErrorString(eResult));
		return;
	}

#endif // _FMOD_DSP_BUFFERSIZE

#ifdef _FMOD_STREAM_BUFFERSIZE

	eResult=pobSystem->setStreamBufferSize(_FMOD_STREAM_BUFFERSIZE,FMOD_TIMEUNIT_RAWBYTES);

	if (eResult!=FMOD_OK)
	{
		ntPrintf("AudioSystem: Failed to set stream buffersize to %d - %s\n",_FMOD_STREAM_BUFFERSIZE,FMOD_ErrorString(eResult));
		return;
	}

#endif // _FMOD_STREAM_BUFFERSIZE

#ifdef _FMOD_USE_FIOS

	// Configure file system

	eResult=pobSystem->setFileSystem(FMOD_FileOpen,FMOD_FileClose,FMOD_FileRead,FMOD_FileSeek,_FMOD_FILESYSTEM_BLOCKSIZE);

	if (eResult==FMOD_OK)
	{
		ntPrintf("AudioSystem: FMOD file system successfully configured to use FIOS\n");
	}
	else
	{
		ntPrintf("AudioSystem: Failed to configure FMOD file system to use FIOS - %s\n",FMOD_ErrorString(eResult));
		return;
	}

#endif // _FMOD_USE_FIOS

	

#ifdef _FMOD_NETWORK_ENABLE

	if (g_ShellOptions->m_bLiveAudioEditing)
	{
		// Initialize FMOD networking
		eResult=FMOD::NetEventSystem_Init(m_pobEventSystem,0);

		if (eResult==FMOD_OK)
		{
			ntPrintf("AudioSystem: NetEventSystem initialised\n");
		}
		else
		{
			ntPrintf("AudioSystem: Failed to initialise NetEventSystem - %s\n",FMOD_ErrorString(eResult));
		}
	}

#endif // _FMOD_NETWORK_ENABLE


	m_bInitialised=true;
	ntPrintf("AudioSystem: Successfully initialised\n");
	ntPrintf("------------------------------------------------------\n");

#else

	ntPrintf("AudioSystem: System is disabled\n");
	ntPrintf("------------------------------------------------------\n");

#endif // _AUDIO_SYSTEM_ENABLE
#endif // _DISABLE_AUDIO_SYSTEM
}

AudioSystem::~AudioSystem ()
{
#ifdef _AUDIO_SYSTEM_ENABLE

	if (m_pobEventSystem)
	{
		#ifdef _FMOD_NETWORK_ENABLE
		if (g_ShellOptions->m_bLiveAudioEditing)
			FMOD::NetEventSystem_Shutdown();
		#endif // _FMOD_NETWORK_ENABLE

		m_pobEventSystem->unload(); // Probably not necessary since this might already be handled in release
		m_pobEventSystem->release();
		m_pobEventSystem=0;
	}

	if (m_pMemoryPool) // Delete our memory pool
	{
		NT_DELETE_ARRAY( m_pMemoryPool );
	}

#endif // _AUDIO_SYSTEM_ENABLE
}

int32_t AudioSystem::GetSpeakerCount()
{
	if ( m_pobEventSystem == NULL )
	{
		return 0;
	}

	FMOD::System *system = NULL;
	FMOD_RESULT res = m_pobEventSystem->getSystemObject( &system );
	if ( res != FMOD_OK || system == NULL )
	{
		return 0;
	}

	FMOD_SPEAKERMODE speakermode;
	res = system->getSpeakerMode( &speakermode );
	if ( res != FMOD_OK )
	{
		return 0;
	}

	switch ( speakermode )
	{
		case FMOD_SPEAKERMODE_STEREO:
			return 2;

		case FMOD_SPEAKERMODE_QUAD:
		case FMOD_SPEAKERMODE_SURROUND:
			return 4;

		case FMOD_SPEAKERMODE_5POINT1:
			return 6;

		case FMOD_SPEAKERMODE_7POINT1:
			return 8;

		default:
			break;
	}

	return 0;
}

void AudioSystem::SetMediaPath (const char* pcPath)
{
#ifdef _AUDIO_SYSTEM_ENABLE

	if (!m_bInitialised)
		return;

	FMOD_RESULT eResult;

	eResult=m_pobEventSystem->setMediaPath(pcPath);

	#ifndef _RELEASE
	if (eResult!=FMOD_OK && IsDebugOptionEnabled(DEBUG_TTY_RESOURCE))
	{
		ntPrintf("AudioSystem: Failed to set media path %s - %s\n",pcPath,FMOD_ErrorString(eResult));
	}
	#endif // _RELEASE

#else

	UNUSED(pcPath);

#endif // _AUDIO_SYSTEM_ENABLE
}


bool AudioSystem::LoadProject (const char* pcPath,int iGroupID)
{
	if (!m_bInitialised)
		return false;

	CHashedString obProjectHash(pcPath);

	// Check to see if the project is already loaded
	for(int i=0; i<MAX_FMOD_PROJECTS; ++i)
	{
		if (m_obProject[i].m_pobEventProject && obProjectHash==m_obProject[i].m_obProjectHash)
			return false;
	}

	// Find a free slot for our new project
	for(int i=0; i<MAX_FMOD_PROJECTS; ++i)
	{
		if (m_obProject[i].m_pobEventProject==0) // Found a free slot
		{
			FMOD::EventProject* pobFMODProject;
			FMOD_RESULT eResult;
			eResult=m_pobEventSystem->load(pcPath, 0, &pobFMODProject); // Attempt to load the project

			if (eResult==FMOD_OK) // Project was loaded successfully
			{
				#ifndef _RELEASE
				if (IsDebugOptionEnabled(DEBUG_TTY_RESOURCE))
					ntPrintf("AudioSystem: Successfully loaded %s\n",pcPath);
				#endif // _RELEASE

				m_obProject[i].m_iGroupID=iGroupID;
				m_obProject[i].m_obProjectHash=obProjectHash;
				m_obProject[i].m_pobEventProject=pobFMODProject;
				return true;
			}
			else
			{
				#ifndef _RELEASE
				if (IsDebugOptionEnabled(DEBUG_TTY_RESOURCE))
					ntPrintf("AudioSystem: Failed to load event file %s - %s\n",pcPath,FMOD_ErrorString(eResult));
				#endif // _RELEASE

				return false; // Project failed to load
			}
		}
	}

	#ifndef _RELEASE
	if (IsDebugOptionEnabled(DEBUG_TTY_RESOURCE))
		ntPrintf("AudioSystem: Failed to load event file %s - project limit %d reached\n",pcPath,MAX_FMOD_PROJECTS);
	#endif // _RELEASE

	return false; // We have reached our max projects limit
}

void AudioSystem::UnloadProject (const char* pcPath) // Unload a particular project
{
	CHashedString obProjectHash(pcPath);

	for(int i=0; i<MAX_FMOD_PROJECTS; ++i)
	{
		if (m_obProject[i].m_pobEventProject && obProjectHash==m_obProject[i].m_obProjectHash) // Find matching project
		{
			m_obProject[i].Unload(); // Unload it
			return;
		}
	}
}

void AudioSystem::UnloadProjectGroup (int iGroupID) // Unload certain type of project
{
	// Unload all projects of a particular type
	for(int i=0; i<MAX_FMOD_PROJECTS; ++i)
	{
		if (iGroupID==m_obProject[i].m_iGroupID && m_obProject[i].m_pobEventProject)
		{
			m_obProject[i].Unload();	
		}
	}
}

void AudioSystem::UnloadAllProjects ()
{
#ifdef _AUDIO_SYSTEM_ENABLE

	if (!m_bInitialised)
		return;

	// Clear off all callbacks
	Sound_ResetCallbacks();

	// Unload all data from the event system
	m_pobEventSystem->unload();

	// Clear all project slots
	for(int i=0; i<MAX_FMOD_PROJECTS; ++i)
	{
		m_obProject[i].m_pobEventProject=0;
	}

#endif // _AUDIO_SYSTEM_ENABLE
}

void AudioSystem::LoadEventGroup (const char* pcGroupName,bool bAsync)
{
	// Event group pre-loading is disabled for time being
	UNUSED(pcGroupName);
	UNUSED(bAsync);

/*
#ifdef _AUDIO_SYSTEM_ENABLE

	if (!m_bInitialised)
		return;

	FMOD_RESULT eResult;

	FMOD::EventGroup* pobGroup;

	eResult=m_pobEventSystem->getGroup(pcGroupName,_FMOD_CACHE_EVENTS,&pobGroup);

	if (eResult!=FMOD_OK)
	{
		#ifndef _RELEASE
		if (IsDebugOptionEnabled(DEBUG_TTY_RESOURCE))
			ntPrintf("AudioSystem: Failed to load event group %s - %s\n",pcGroupName,FMOD_ErrorString(eResult));
		#endif // _RELEASE

		return; // Invalid group, so we can't continue
	}
	
	eResult=pobGroup->loadEventData(FMOD::EVENT_RESOURCE_STREAMS_AND_SAMPLES,(bAsync ? EVENT_NONBLOCKING : EVENT_DEFAULT));

	#ifndef _RELEASE
	if (IsDebugOptionEnabled(DEBUG_TTY_RESOURCE))
	{        if (eResult==FMOD_OK)
			ntPrintf("AudioSystem: Loaded event group %s\n",pcGroupName);
		else
			ntPrintf("AudioSystem: Failed to load event data %s - %s\n",pcGroupName,FMOD_ErrorString(eResult));
	}
	#endif // _RELEASE

#else

	UNUSED(pcGroupName);
	UNUSED(bAsync);

#endif // _AUDIO_SYSTEM_ENABLE
*/
}

void AudioSystem::UnloadEventGroup (const char* pcGroupName)
{
#ifdef _AUDIO_SYSTEM_ENABLE

	if (!m_bInitialised)
		return;

	FMOD_RESULT eResult;

	FMOD::EventGroup* pobGroup;

	eResult=m_pobEventSystem->getGroup(pcGroupName,false,&pobGroup);

	if (eResult!=FMOD_OK)
	{
		#ifndef _RELEASE
		if (IsDebugOptionEnabled(DEBUG_TTY_RESOURCE))
			ntPrintf("AudioSystem: Failed to find event group %s - %s\n",pcGroupName,FMOD_ErrorString(eResult));
		#endif // _RELEASE

		return; // Invalid group, so we can't continue
	}

	eResult=pobGroup->freeEventData();

	#ifndef _RELEASE
	if (IsDebugOptionEnabled(DEBUG_TTY_RESOURCE))
	{
        if (eResult==FMOD_OK)
			ntPrintf("AudioSystem: Unloaded event group %s\n",pcGroupName);
		else
			ntPrintf("AudioSystem: Failed to unload event group %s - %s\n",pcGroupName,FMOD_ErrorString(eResult));
	}
	#endif // _RELEASE

#else

	UNUSED(pcGroupName);

#endif // _AUDIO_SYSTEM_ENABLE
}

void AudioSystem::DoWork ()
{
#ifdef _AUDIO_SYSTEM_ENABLE

	if (!m_bInitialised)
		return;

	// ----- Networking update -----

	#ifdef _FMOD_NETWORK_ENABLE
	if (g_ShellOptions->m_bLiveAudioEditing)
	{
		FMOD::NetEventSystem_Update();
	}
	#endif // _FMOD_NETWORK_ENABLE

	
	// ----- Update the listener -----

	if (m_bUpdateListener)
	{
		float fTimeStep=CTimer::Get().GetGameTimeChange();

		CDirection obVelocity;

		if (fTimeStep>EPSILON)
		{
			obVelocity=CDirection(m_obListenerMatrix.GetTranslation()-m_obPreviousListenerPosition);
			obVelocity/=fTimeStep;

			m_obPreviousListenerPosition=m_obListenerMatrix.GetTranslation();
		}
		else
		{
			obVelocity.Clear();
		}

		CPoint obPosition(m_obListenerMatrix.GetTranslation());
		CDirection obForward=CDirection(0.0f,0.0f,1.0f) * m_obListenerMatrix;
		CDirection obUp=CDirection(0.0f,1.0f,0.0f) * m_obListenerMatrix;

//		CPoint obPosition(0.0f,0.25f,0.0f);
//		CDirection obForward(1.0f,0.0f,0.0f);
//		CDirection obUp(0.0f,1.0f,0.0f);

		FMOD_VECTOR position;
		position.x=obPosition.X();
		position.y=obPosition.Y();
		position.z=obPosition.Z();

		FMOD_VECTOR velocity;
		//velocity.x=obVelocity.X();
		//velocity.y=obVelocity.Y();
		//velocity.z=obVelocity.Z();
		velocity.x=0.0f;
		velocity.y=0.0f;
		velocity.z=0.0f;

		FMOD_VECTOR forward;
		forward.x=obForward.X();
		forward.y=obForward.Y();
		forward.z=obForward.Z();

		FMOD_VECTOR up;
		up.x=obUp.X();
		up.y=obUp.Y();
		up.z=obUp.Z();
		
		m_pobEventSystem->set3DListenerAttributes(0,&position,&velocity,&forward,&up);

		m_bUpdateListener=false;
	}

	// ----- Update our instances -----
	for(int i=0; i<MAX_EVENT_INSTANCES; ++i)
	{
		m_obInstanceList[i].Process();
	}

	// ----- Update FMOD -----
	CGatso::Start("FMOD:EventSystem:Update");
	m_pobEventSystem->update();
	CGatso::Stop("FMOD:EventSystem:Update");

#endif // _AUDIO_SYSTEM_ENABLE
}

void AudioSystem::SetListener (int iListener)
{
#ifdef _AUDIO_SYSTEM_ENABLE

	if (!m_bInitialised)
		return;

	m_pobEventSystem->set3DNumListeners(iListener);

#else

	UNUSED(iListener);

#endif // _AUDIO_SYSTEM_ENABLE
}

void AudioSystem::SetListenerProperties (int iListener,const CMatrix& obWorldMatrix)
{
#ifdef _AUDIO_SYSTEM_ENABLE

	if (!m_bInitialised)
		return;

	UNUSED(iListener);

	m_obListenerMatrix=obWorldMatrix;
	m_bUpdateListener=true;

	/*
	//CPoint obPosition(obWorldMatrix.GetTranslation() + obWorldMatrix.GetZAxis());

	CPoint obPosition(5.0f,0.25f,5.0f);

	CDirection obForward=CDirection(0.0f,0.0f,1.0f) * obWorldMatrix;
	obForward.Y()=0.0f;
	obForward.Normalise();

	//CDirection obUp=CDirection(0.0f,1.0f,0.0f) * obWorldMatrix;
	CDirection obUp(0.0f,1.0f,0.0f);

	FMOD_VECTOR position;
	position.x=obPosition.X();
	position.y=obPosition.Y();
	position.z=obPosition.Z();

	FMOD_VECTOR forward;
	forward.x=obForward.X();
	forward.y=obForward.Y();
	forward.z=obForward.Z();

	FMOD_VECTOR up;
	up.x=obUp.X();
	up.y=obUp.Y();
	up.z=obUp.Z();
	
	if (m_pobEventSystem->set3DListenerAttributes(iListener,&position,0,&forward,&up)==FMOD_OK)
	{
		m_obListenerMatrix=obWorldMatrix;
	}
	else
	{
		ntPrintf("AudioSystem: Failed to update listener properties\n");
	}
	*/

#else

	UNUSED(iListener);
	UNUSED(obWorldMatrix);

#endif // _AUDIO_SYSTEM_ENABLE
}

const CPoint& AudioSystem::GetListenerPosition ()
{
	return m_obListenerMatrix.GetTranslation();
}

float AudioSystem::GetDistanceSqrdFromListener (const CPoint& obPosition)
{
	CDirection obDiff(obPosition - m_obListenerMatrix.GetTranslation());

	return obDiff.LengthSquared();
}

bool AudioSystem::Sound_Prepare (unsigned int& id,const char* pcGroupName,const char* pcEventName)
{
#ifdef _AUDIO_SYSTEM_ENABLE

	if (!m_bInitialised)
		return false;

	// Early reject
	if (*pcEventName=='\0' || *pcGroupName=='\0' ||
		(pcEventName[0]=='N' && pcEventName[1]=='U' && pcEventName[2]=='L') ||
		(pcGroupName[0]=='N' && pcGroupName[1]=='U' && pcGroupName[2]=='L'))
		return false;


	FMOD::EventGroup* pobGroup=0;
	FMOD_RESULT eResult=m_pobEventSystem->getGroup(pcGroupName,_FMOD_CACHE_EVENTS,&pobGroup);

	if (eResult!=FMOD_OK)
	{
		#ifndef _RELEASE
		if (IsDebugOptionEnabled(DEBUG_TTY_EVENT_ERROR))
			ntPrintf("AudioSystem: Sound_Prepare %s/%s failed - invalid group (%s)\n",pcGroupName,pcEventName,FMOD_ErrorString(eResult));
		#endif // _RELEASE
		return false;
	}

	FMOD::Event* pobEvent=0;

	eResult=pobGroup->getEvent(pcEventName,_FMOD_EVENT_MODE,&pobEvent);

	if (eResult!=FMOD_OK)
	{
		#ifndef _RELEASE
		if (IsDebugOptionEnabled(DEBUG_TTY_EVENT_ERROR))
			ntPrintf("AudioSystem: Sound_Prepare %s/%s failed - invalid event (%s)\n",pcGroupName,pcEventName,FMOD_ErrorString(eResult));
		#endif // _RELEASE
		return false;
	}

	AudioEventInstance* pobEventHandle=FindFreeInstance();

	if (pobEventHandle)
	{
		id=GenerateHandleID();

		if (pobEventHandle->Init(id,pobEvent))
		{
			m_uiLastID=id;
			m_uiLastAudioInstance=pobEventHandle;
			return true;
		}
	}

	// Unable to find a free instance to play this sound

	#ifndef _RELEASE
	if (IsDebugOptionEnabled(DEBUG_TTY_EVENT_ERROR))
		ntPrintf("AudioSystem: Sound_Prepare %s/%s failed - no free instance slots available\n");
	#endif // _RELEASE

	return false;

#else

	UNUSED(id);
	UNUSED(pcGroupName);
	UNUSED(pcEventName);

	return false;

#endif // _AUDIO_SYSTEM_ENABLE
}

bool AudioSystem::Sound_Prepare (unsigned int& id,const char* pcSoundName)
{
	if (pcSoundName)
	{
		if (pcSoundName[0]=='N' && pcSoundName[1]=='U' && pcSoundName[2]=='L') // Early reject
			return false;

		const int iMAX_GROUPNAME_SIZE = 64;

		char acGroupName [iMAX_GROUPNAME_SIZE];

		int i=0;

		while(pcSoundName[i]!='\0' && i<iMAX_GROUPNAME_SIZE)
		{
			acGroupName[i]=pcSoundName[i];

			if (pcSoundName[i]==':')
			{
				acGroupName[i]='\0';

				return Sound_Prepare(id,acGroupName,pcSoundName+i+1);
			}

			++i;
		}
	}

	#ifndef _RELEASE
	if (IsDebugOptionEnabled(DEBUG_TTY_EVENT_ERROR))
		ntPrintf("AudioSystem: Sound_Prepare with invalid group or event - %s\n",pcSoundName);
	#endif // _RELEASE

	return false;
}

void AudioSystem::Sound_SetVolume (unsigned int id,float fVolume)
{
#ifdef _AUDIO_SYSTEM_ENABLE

	AudioEventInstance* pobEventHandle=FindInstance(id);

	if (pobEventHandle)
		pobEventHandle->SetVolume(fVolume);

#else

	UNUSED(id);
	UNUSED(fVolume);

#endif // _AUDIO_SYSTEM_ENABLE
}

void AudioSystem::Sound_SetPitch (unsigned int id,float fPitch)
{
#ifdef _AUDIO_SYSTEM_ENABLE

	AudioEventInstance* pobEventHandle=FindInstance(id);

	if (pobEventHandle)
		pobEventHandle->SetPitch(fPitch);

#else

	UNUSED(id);
	UNUSED(fPitch);

#endif // _AUDIO_SYSTEM_ENABLE
}

void AudioSystem::Sound_SetPosition (unsigned int id,const CPoint& obPosition,float fAudioRadius)
{
#ifdef _AUDIO_SYSTEM_ENABLE

	AudioEventInstance* pobEventHandle=FindInstance(id);

	if (pobEventHandle)
	{
		pobEventHandle->SetPosition(obPosition,fAudioRadius);
	}

#else

	UNUSED(id);
	UNUSED(obPosition);

#endif // _AUDIO_SYSTEM_ENABLE
}

void AudioSystem::Sound_SetCallback (unsigned int id,AudioCallback pFunc, void* pData)
{
#ifdef _AUDIO_SYSTEM_ENABLE

	AudioEventInstance* pobEventHandle=FindInstance(id);

	if (pobEventHandle)
		pobEventHandle->SetCallBack(pFunc,pData);

#else

	UNUSED(id);
	UNUSED(pFunc);
	UNUSED(pData);

#endif // _AUDIO_SYSTEM_ENABLE
}

void AudioSystem::Sound_SetParameterValue (unsigned int id,const char* pcParameterName,float fValue)
{
#ifdef _AUDIO_SYSTEM_ENABLE

	AudioEventInstance* pobEventHandle=FindInstance(id);

	if (pobEventHandle)
		pobEventHandle->SetParameterValue(pcParameterName,fValue);

#else

	UNUSED(id);
	UNUSED(pcParameterName);
	UNUSED(fValue);

#endif // _AUDIO_SYSTEM_ENABLE
}

void AudioSystem::Sound_SetParameterVelocity (unsigned int id,const char* pcParameterName,float fVelocity)
{
#ifdef _AUDIO_SYSTEM_ENABLE

	AudioEventInstance* pobEventHandle=FindInstance(id);

	if (pobEventHandle)
		pobEventHandle->SetParameterVelocity(pcParameterName,fVelocity);

#else

	UNUSED(id);
	UNUSED(pcParameterName);
	UNUSED(fVelocity);

#endif // _AUDIO_SYSTEM_ENABLE
}

void AudioSystem::Sound_SetParameterSeekSpeed (unsigned int id,const char* pcParameterName,float fSeekSpeed)
{
#ifdef _AUDIO_SYSTEM_ENABLE

	AudioEventInstance* pobEventHandle=FindInstance(id);

	if (pobEventHandle)
		pobEventHandle->SetParameterSeekSpeed(pcParameterName,fSeekSpeed);

#else

	UNUSED(id);
	UNUSED(pcParameterName);
	UNUSED(fSeekSpeed);

#endif // _AUDIO_SYSTEM_ENABLE
}

bool AudioSystem::Sound_Play (unsigned int id)
{
#ifdef _AUDIO_SYSTEM_ENABLE

	AudioEventInstance* pobEventHandle=FindInstance(id);

	if (pobEventHandle)
		return pobEventHandle->Play();

#else

	UNUSED(id);

#endif // _AUDIO_SYSTEM_ENABLE

	return false;
}


void AudioSystem::Sound_Stop (unsigned int id)
{
#ifdef _AUDIO_SYSTEM_ENABLE

	AudioEventInstance* pobEventHandle=FindInstance(id);

	if (pobEventHandle)
		pobEventHandle->Stop();

#else

	UNUSED(id);

#endif // _AUDIO_SYSTEM_ENABLE
}

void AudioSystem::Sound_ResetCallbacks ()
{
	for(int i=0; i<MAX_EVENT_INSTANCES; ++i)
	{
		m_obInstanceList[i].SetCallBack(0,0);
	}
}

void AudioSystem::Sound_StopAll (bool bImmediate)
{
	for(int i=0; i<MAX_EVENT_INSTANCES; ++i)
	{
		m_obInstanceList[i].Stop(bImmediate);
	}
}

bool AudioSystem::Sound_IsPlaying (unsigned int id)
{
#ifdef _AUDIO_SYSTEM_ENABLE

	if (FindInstance(id))
		return true;

#else

	UNUSED(id);

#endif // _AUDIO_SYSTEM_ENABLE

	return false;
}

bool AudioSystem::Sound_IsPaused (unsigned id)
{
#ifdef _AUDIO_SYSTEM_ENABLE

	AudioEventInstance* pobEventHandle=FindInstance(id);

	if (pobEventHandle)
		return pobEventHandle->IsPaused();

#else

	UNUSED(id);

#endif // _AUDIO_SYSTEM_ENABLE

	return false;
}

bool AudioSystem::Sound_GetParameterRange (unsigned int id,const char* pcParameterName,float& fMin,float& fMax)
{
#ifdef _AUDIO_SYSTEM_ENABLE

	AudioEventInstance* pobEventHandle=FindInstance(id);

	if (pobEventHandle)
		return pobEventHandle->GetParameterRange(pcParameterName,fMin,fMax);

#else

	UNUSED(id);
	UNUSED(pcParameterName);
	UNUSED(fMin);
	UNUSED(fMax);

#endif // _AUDIO_SYSTEM_ENABLE

	return false;
}

bool AudioSystem::Reverb_SetActive (const char* pcName,bool bActive)
{
#ifdef _AUDIO_SYSTEM_ENABLE

	if (!m_bInitialised)
		return false;

	FMOD::EventReverb* pobEventReverb=0;

	FMOD_RESULT eResult=m_pobEventSystem->getReverb(pcName,&pobEventReverb);
		
	if (eResult!=FMOD_OK)
	{
		#ifndef _RELEASE
		if (IsDebugOptionEnabled(DEBUG_TTY_EVENT_ERROR)) // We have failed to get this reverb definition from FMOD
			ntPrintf("AudioSystem: Failed to set reverb %s to %d - %s\n",pcName,bActive,FMOD_ErrorString(eResult));
		#endif // _RELEASE

		return false;
	}

	pobEventReverb->setActive(bActive); // Activate this reverb

#else

	UNUSED(pcName);
	UNUSED(bActive);

#endif // _AUDIO_SYSTEM_ENABLE

	return true;
}

AudioEventInstance* AudioSystem::FindFreeInstance ()
{
#ifdef _AUDIO_SYSTEM_ENABLE

	if (m_bInitialised)
	{
		for(int i=0; i<MAX_EVENT_INSTANCES; ++i)
		{
			if (m_obInstanceList[i].IsFree())
				return &m_obInstanceList[i];
		}
	}

#endif // _AUDIO_SYSTEM_ENABLE

	return 0;
}

AudioEventInstance* AudioSystem::FindInstance (unsigned int id)
{
#ifdef _AUDIO_SYSTEM_ENABLE

	if (m_bInitialised && id)
	{
		if (id==m_uiLastID) // Check with cached ID
		{
			return m_uiLastAudioInstance;
		}

		for(int i=0; i<MAX_EVENT_INSTANCES; ++i)
		{
			if (id==m_obInstanceList[i].GetID())
				return &m_obInstanceList[i];
		}
	}

#else

	UNUSED(id);

#endif // _AUDIO_SYSTEM_ENABLE

	return 0;
}

unsigned int AudioSystem::GenerateHandleID ()
{
	static unsigned int id=0;

	++id;

	if (id==0)
		++id;

	return id;
}

int AudioSystem::GetSPUThreadCount ()
{
#ifdef _AUDIO_SYSTEM_ENABLE
#ifdef _FMOD_SPU_THREADS

	if (g_ShellOptions->m_bAudioEngine)
		return _FMOD_SPU_THREADS;

#endif // _FMOD_SPU_THREADS
#endif // _AUDIO_SYSTEM_ENABLE

	return 0; // Return 0 threads required by default
}

//----- Debugging ------------------------------------------------------------------------------------------------------------------------------------------------------

unsigned int AudioSystem::GetFMODVersion ()
{
#ifdef _AUDIO_SYSTEM_ENABLE

	if (m_bInitialised)
	{
		return FMOD_VERSION;
	}

#endif // _AUDIO_SYSTEM_ENABLE

	return 0;
}

unsigned int AudioSystem::GetFMODEventVersion ()
{
#ifdef _AUDIO_SYSTEM_ENABLE

	if (m_bInitialised)
	{
		return FMOD_EVENT_VERSION;
	}

#endif // _AUDIO_SYSTEM_ENABLE

	return 0;
}

void AudioSystem::GetMemoryUsage (int& iCurrent,int& iHigh,int& iMax)
{
#ifdef _AUDIO_SYSTEM_ENABLE

	if (m_bInitialised)
	{
		FMOD::Memory_GetStats(&iCurrent,&iHigh);

		iMax=_FMOD_MEMORY_ALLOCATION;

		return;
	}

#endif // _AUDIO_SYSTEM_ENABLE

	iCurrent=0;
	iHigh=0;
	iMax=0;
}

int AudioSystem::GetPlayingChannels ()
{
#ifdef _AUDIO_SYSTEM_ENABLE

	FMOD::System* pobSystem=0;

	if (m_pobEventSystem->getSystemObject(&pobSystem)==FMOD_OK)
	{
		int iChannels=0;

		if (pobSystem->getChannelsPlaying(&iChannels)==FMOD_OK)
			return iChannels;
	}

#endif // _AUDIO_SYSTEM_ENABLE

	return 0;
}

void AudioSystem::ToggleDebugOption (unsigned int uiFlag)
{
	if (m_uiDebugFlags & uiFlag)
		m_uiDebugFlags&=~uiFlag;
	else
		m_uiDebugFlags|=uiFlag;
}


void AudioSystem::SimpleTest ()
{
#ifndef _RELEASE
/*
    FMOD::EventGroup       *eventgroup;
    FMOD::Event            *car;
    FMOD::EventParameter   *rpm;
    FMOD::EventParameter   *load;

    float loadRangeMin, loadRangeMax;

	FMOD_RESULT eResult;

	eResult=m_pobEventSystem->getGroup("examples/examples/car", EVENT_DEFAULT, &eventgroup);

	if (eResult!=FMOD_OK)
	{
		ntPrintf("Simple Example: getGroup failed - %s\n",FMOD_ErrorString(eResult));
		return;
	}

	eResult=eventgroup->getEvent("car", EVENT_DEFAULT, &car);

	if (eResult!=FMOD_OK)
	{
		ntPrintf("Simple Example: getEvent failed - %s\n",FMOD_ErrorString(eResult));
		return;
	}

	eResult=car->getParameter("load", &load);

	if (eResult!=FMOD_OK)
	{
		ntPrintf("Simple Example: getParameter load failed - %s\n",FMOD_ErrorString(eResult));
		return;
	}


	load->getRange(&loadRangeMin, &loadRangeMax);
	load->setValue(loadRangeMax);

	car->getParameterByIndex(0, &rpm);
	rpm->setValue(1000.0f);

	eResult=car->start();

	if (eResult!=FMOD_OK)
	{
		ntPrintf("Simple Example: start() failed - %s\n",FMOD_ErrorString(eResult));
		return;
	}

	ntPrintf("Playing simple example event - Load=%f  RPM=%f\n",loadRangeMax,rpm);
//*/
#endif // _RELEASE
}



