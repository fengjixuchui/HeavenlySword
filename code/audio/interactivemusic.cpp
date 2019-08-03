#include "audio/interactivemusic.h"
#include "audio/audiosystem.h"

#include "fmod_event.h"
#include "fmod_errors.h"


InteractiveMusic::InteractiveMusic () :
	m_pobFMODSystem(0)
{
	if (AudioSystem::Get().GetFMODEventSystem())
	{
		FMOD_RESULT eResult=AudioSystem::Get().GetFMODEventSystem()->getSystemObject(&m_pobFMODSystem);

		if (eResult!=FMOD_OK)
		{
		}
	}
}

InteractiveMusic::~InteractiveMusic ()
{
	if (m_pcBuffer)
	{
		NT_DELETE( m_pcBuffer );
	}
}

void InteractiveMusic::Init ()
{
	if (!m_pobFMODSystem)
		return;

	// Allocate memory for the streaming buffers
	m_pcBuffer = NT_NEW char [MUSIC_BUFFER_SIZE * MUSIC_NUM_CHANNELS];
	memset(m_pcBuffer,0,MUSIC_BUFFER_SIZE * MUSIC_NUM_CHANNELS);

	FMOD_CREATESOUNDEXINFO stSoundInfo;
	stSoundInfo.cbsize=sizeof(FMOD_CREATESOUNDEXINFO);
	stSoundInfo.length=0;
	stSoundInfo.fileoffset=0;
	stSoundInfo.numchannels=2;
	stSoundInfo.defaultfrequency=44100;
	stSoundInfo.format=FMOD_SOUND_FORMAT_PCM16;
	stSoundInfo.decodebuffersize=0;
	stSoundInfo.initialsubsound=0;
	stSoundInfo.numsubsounds=0;
	stSoundInfo.inclusionlist=0;
	stSoundInfo.inclusionlistnum=0;
	stSoundInfo.pcmreadcallback=0;
	stSoundInfo.pcmsetposcallback=0;
	stSoundInfo.nonblockcallback=0;
	stSoundInfo.dlsname=0;
	stSoundInfo.encryptionkey=0;
	stSoundInfo.maxpolyphony=0;
	stSoundInfo.userdata=0;
	stSoundInfo.suggestedsoundtype=FMOD_SOUND_TYPE_WAV;
	
	// Create our streams
	for(int i=0; i<MUSIC_NUM_CHANNELS; ++i)
	{
		m_pobFMODSystem->createSound(
			m_pcBuffer + (MUSIC_BUFFER_SIZE * i),
			FMOD_OPENMEMORY,
			&stSoundInfo,
			&m_pobSound[i]);
	}

	// Kick off all the streams in a paused state
	for(int i=0; i<MUSIC_NUM_CHANNELS; ++i)
	{
		m_pobFMODSystem->playSound(FMOD_CHANNEL_FREE,m_pobSound[i],true,&m_pobChannel[i]);
	}

	// Begin playback
	for(int i=0; i<MUSIC_NUM_CHANNELS; ++i)
	{
		m_pobChannel[i]->setPaused(false);
	}
}




