//------------------------------------------------------------------------------------------
//!
//!	\file movieinstance.cpp
//!
//------------------------------------------------------------------------------------------

#include "movies/movieinstance.h"
#include "movies/moviememory.h"
#include "movies/moviehelpers.h"

#include "core/util.h"

#include "game/languages.h"
#include "game/playeroptions.h"

#include "audio/audiosystem.h"

#ifdef PLATFORM_PS3
#	include "3rdparty/bink/bink.h"
#endif

MovieID MovieInstance::m_MasterMovieID = 0;

namespace
{
	static uint32_t GetTrackFromLanguage( AudioLanguage lang, bool stereo )
	{
		if ( stereo )
		{
			return 2*NT_AUDIO_NUM_LANGUAGES + 5 + uint32_t( lang );
		}
		else
		{
			return ( uint32_t( lang ) * 2 ) + 4;
		}
	}
}

//------------------------------------------------------------------------------------------
//!
//!	Create a movie object from the filename.
//!
//------------------------------------------------------------------------------------------
MovieInstancePtr MovieInstance::Create( const char *filename, int32_t pip_debug_id, uint32_t flags/* = 0*/ )
{
	char extended_filename[ 1024 ];
#	ifdef PLATFORM_PS3
		Util::GetFullBluRayFilePath( filename, extended_filename );
#	else
		Util::GetFiosFilePath( filename, extended_filename );
#	endif

	MovieInstance *movie( NT_NEW_CHUNK( Movie::MemChunk ) MovieInstance( extended_filename, pip_debug_id, flags ) );

	return MovieInstancePtr( movie );
}

//------------------------------------------------------------------------------------------
//!
//!	Setup the volumes for each channel's mix-bin within Bink.
//!
//------------------------------------------------------------------------------------------
void MovieInstance::InitMixBinVolumes( bool stereo )
{
	UNUSED( stereo );
#	ifdef PLATFORM_PS3
	{
		if ( stereo )
		{
			static const uint32_t MaxNumTracks = 1 + NT_AUDIO_NUM_LANGUAGES;	// 1 stereo music channel + 1 stereo-channel per language.
    		int32_t volumes[ MaxNumTracks ];

    		// Left and right.
    		memset( volumes, 0, sizeof( volumes ) );
    		volumes[ 0 ] = 32768;
    		volumes[ 1 ] = 32768;
			uint32_t stereo_track_id = 4 + 2*NT_AUDIO_NUM_LANGUAGES;			// The stereo track comes after all the 5.1 music and per-language tracks.
    		BinkSetMixBinVolumes( m_Bink, stereo_track_id, 0, volumes, MaxNumTracks );
	    	
			// Route the correct audio language channel to the front LR speakers.
			uint32_t lang_track_id = GetTrackFromLanguage( CPlayerOptions::Get().GetAudioLanguage(), true );
			memset( volumes, 0, sizeof( volumes ) );
			volumes[ 0 ] = 32768;
			volumes[ 1 ] = 32768;
			BinkSetMixBinVolumes( m_Bink, lang_track_id, 0, volumes, MaxNumTracks );
		}
		else
		{
			static const uint32_t MaxNumTracks = 4 + 2*NT_AUDIO_NUM_LANGUAGES;	// 5.1 audio (4ch) + 1 stereo-channel and 1 mono-channel per language.
																				// 5.1 surround has 4ch:
																				//		1 stereo Front LR,
																				//		1 mono Centre,
																				//		1 mono Sub-woofer,
																				//		1 stereo Rear LR.
    		int32_t volumes[ MaxNumTracks ];

    		// front LR
    		memset( volumes, 0, sizeof( volumes ) );
    		volumes[ 0 ] = 32768;
    		volumes[ 1 ] = 32768;
    		BinkSetMixBinVolumes( m_Bink, 0, 0, volumes, MaxNumTracks );
	    	
    		// center
    		memset( volumes,  0, sizeof( volumes ) );
    		volumes[ 2 ] = 32768;
    		BinkSetMixBinVolumes( m_Bink, 1, 0, volumes, MaxNumTracks );

    		// sub
    		memset( volumes,  0, sizeof( volumes ) );
    		volumes[ 3 ] = 32768;
    		BinkSetMixBinVolumes( m_Bink, 2, 0, volumes, MaxNumTracks );

			// rear LR.
			memset( volumes, 0, sizeof( volumes ) );
			volumes[ 4 ] = 32768;
			volumes[ 5 ] = 32768;
			BinkSetMixBinVolumes( m_Bink, 3, 0, volumes, MaxNumTracks );

			// Route the correct audio language channel to the front LR speakers.
			uint32_t lang_track_id = GetTrackFromLanguage( CPlayerOptions::Get().GetAudioLanguage(), false );
			memset( volumes, 0, sizeof( volumes ) );
			volumes[ 0 ] = 32768;
			volumes[ 1 ] = 32768;
			BinkSetMixBinVolumes( m_Bink, lang_track_id, 0, volumes, MaxNumTracks );

			// Route the second audio language track to the centre speakers.
			memset( volumes, 0, sizeof( volumes ) );
			volumes[ 2 ] = 32768;
			BinkSetMixBinVolumes( m_Bink, lang_track_id+1, 0, volumes, MaxNumTracks );
		}
	}
#	endif
}

//------------------------------------------------------------------------------------------
//!
//!	Ctor, dtor.
//!
//------------------------------------------------------------------------------------------
MovieInstance::MovieInstance( const char *filename, int32_t pip_debug_id, uint32_t flags )
:	m_Position		( 0.5f, 0.5f, 0.0f )
,	m_Size			( 1.0f, 1.0f, 0.0f )
,	m_ScriptEntity	( NULL )
,	m_Bink			( NULL )
,	m_TextureSet	( NULL )
,	m_Flags			( flags )
,	m_PIPDebugID	( pip_debug_id )
,	m_MovieID		( m_MasterMovieID++ )
,	m_RefCount		( 1 )
,	m_FirstFrame	( true )
,	m_IsFadingOut	( false )
,	m_pobMovieSprite( NULL )
{
#	ifdef PLATFORM_PS3
	{
		// Use stereo audio playback is we don't support 5.1 or better.
		bool stereo = ( AudioSystem::Get().GetSpeakerCount() < 6 );
		ntPrintf( "MOVIE: Num speakers reported by AudioSystem = %i\n", AudioSystem::Get().GetSpeakerCount() );

		if ( m_Flags & MOVIE_NO_SOUND )
		{
			// Turn off all the sound tracks.
			BinkSetSoundTrack( 0, NULL );
		}
		else
		{
			uint32_t lang_track_id = GetTrackFromLanguage( CPlayerOptions::Get().GetAudioLanguage(), stereo );

			if ( stereo )
			{
				// Just play the stereo music track and the stereo language track.
				uint32_t TrackIDsToPlay[ 2 ] = { 2*NT_AUDIO_NUM_LANGUAGES + 4, lang_track_id };
				ntPrintf( "MOVIE: Stereo sound. Tracks playing - music: %i, vo: %i\n", TrackIDsToPlay[ 0 ], TrackIDsToPlay[ 1 ] );

				BinkSetSoundTrack( 2, TrackIDsToPlay );
			}
			else
			{
				// We'd like to play 5.1 surround audio + 2 language tracks.
				uint32_t TrackIDsToPlay[ 6 ] = { 0, 1, 2, 3, lang_track_id, lang_track_id+1 };
				ntPrintf(	"MOVIE: 5.1 sound. Tracks playing - music: %i, %i, %i, %i, vo: %i, %i\n",
							TrackIDsToPlay[ 0 ], TrackIDsToPlay[ 1 ], TrackIDsToPlay[ 2 ], TrackIDsToPlay[ 3 ],
							TrackIDsToPlay[ 4 ], TrackIDsToPlay[ 5 ] );

				BinkSetSoundTrack( 6, TrackIDsToPlay );
			}
		}

		ntPrintf( "MOVIE: Playing: %s\n", filename );
		m_Bink = BinkOpen( filename, BINKNOFRAMEBUFFERS | BINKSNDTRACK );
		ntError_p( m_Bink != NULL, ("Failed to open movie at %s", filename) );

		Movie::Helpers::CreateTextureSet( m_Bink, &m_TextureSet );

		if ( ( m_Flags & MOVIE_NO_SOUND ) == 0 )
		{
			InitMixBinVolumes( stereo );

			BinkSetVolume( m_Bink, 0, 32768 );
			BinkSetVolume( m_Bink, 1, 32768 );

			if ( !stereo )
			{
				BinkSetVolume( m_Bink, 2, 32768 );
				BinkSetVolume( m_Bink, 3, 32768 );
				BinkSetVolume( m_Bink, 4, 32768 );
			}
		}
	}
#	endif

	snprintf( m_Filename, MAX_PATH, "%s", filename );

	if ( m_MasterMovieID == 0xffffffff )
	{
		m_MasterMovieID = 0;
	}
}

MovieInstance::~MovieInstance()
{
#	ifdef PLATFORM_PS3
	{
		Movie::Helpers::DestroyTextureSet( m_TextureSet );
		m_TextureSet = NULL;

		if ( m_Bink != NULL )
		{
			BinkClose( m_Bink );
		}
	}
#	endif
}



