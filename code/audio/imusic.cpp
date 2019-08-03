#include "audio/imusic.h"
#include "audio/imusicresource.h"
#include "audio/audiosystem.h"

#include "game/shellconfig.h" // Need for game config options

#include "editable/enumlist.h"

#include "game/randmanager.h"

#include "fmod_event.h"
#include "fmod_errors.h"


#ifndef _RELEASE

#define _STREAM_DEBUG

#endif // _RELEASE


// Introduces a slight tolerance to music intensity comparisons (for transition matching etc.)
#define MUSIC_INTENSITY_MATCH(first, second)	\
	((first) > ((second) - 0.0005f) && (first) < ((second) + 0.0005f))



// Function for converting a time value to bytes
unsigned long ConvertTimeToBytes (float fTime)
{
	if (fTime<=0.0f)
		return 0;

	unsigned long ulBytes=(unsigned long)((float)MUSIC_BYTES_PER_SECOND * fTime);

	ulBytes&=-MUSIC_BLOCK_ALIGNMENT; // Ensure the results are lower block aligned

	//ulBytes=(ulBytes + 3) & -MUSIC_BLOCK_ALIGNMENT; // Upper block align - I've opted not to use this to ensure we never get overflow errors

	return ulBytes;
}

unsigned long ConvertSamplesToBytes (unsigned long ulSamples)
{
	return (ulSamples << 2);
}








// ---------------------------------------------------- Callbacks ----------------------------------------------------

FMOD_RESULT F_CALLBACK pcmreadcallback(FMOD_SOUND* sound, void *data, unsigned int datalen)
{
	UNUSED(sound);

	InteractiveMusicManager::Get().ThreadUpdate(data,datalen);

    return FMOD_OK;
}












// ---------------------------------------------------- AudioDecoder ----------------------------------------------------

AudioDecoder::AudioDecoder () :
	m_pobSound(0),
	m_uiPosition(0),
	m_uiLength(0),
	m_uiStartPos(0),
	m_uiEndPos(0)
{
	m_acFileName[0]='\0';
}

AudioDecoder::~AudioDecoder ()
{
	Release();
}

void AudioDecoder::Release ()
{
#ifdef _AUDIO_SYSTEM_ENABLE

	m_acFileName[0]='\0';

	if (m_pobSound)
	{
		m_pobSound->release();
		m_pobSound=0;
	}

#endif // _AUDIO_SYSTEM_ENABLE
}

bool AudioDecoder::LoadFile (const char* pcFileName,bool bAccurateTime)
{
#ifdef _AUDIO_SYSTEM_ENABLE

	if (strcmp(pcFileName,m_acFileName)==0) // Check to see if we are trying to reload the same file
	{
		//ntPrintf("AudioDecoder::LoadFile - Seeking to beginning for %s (%d bytes)\n",pcFileName,m_uiLength);

		return Seek(m_uiStartPos); // In which case, just seek back to the beginning
	}
	else
	{
		Release(); // This is a new file, so release the sound object
	}

	FMOD_RESULT eResult;

	FMOD::System* pobSystem;
	
	eResult=AudioSystem::Get().GetFMODEventSystem()->getSystemObject(&pobSystem); // Get a pointer to the FMOD System object

	#ifndef _RELEASE
	if (eResult!=FMOD_OK)
	{
		ntPrintf("AudioDecoder: Failed to get FMOD system object for %s - %s\n",pcFileName,FMOD_ErrorString(eResult));
		return false;
	}
	#endif // _RELEASE

	char acPath [256];

	InteractiveMusicManager::Get().ExtendFilePath(pcFileName,acPath);

	if (bAccurateTime) // We want the indexing to be accurate
	{
		eResult=pobSystem->createStream(acPath, FMOD_OPENONLY | FMOD_LOOP_NORMAL | FMOD_ACCURATETIME, 0, &m_pobSound); // Create an FMOD stream
	}
	else
	{
		eResult=pobSystem->createStream(acPath, FMOD_OPENONLY | FMOD_LOOP_NORMAL, 0, &m_pobSound); // Create an FMOD stream
	}

	#ifndef _RELEASE
	if (eResult!=FMOD_OK)
	{
		ntPrintf("AudioDecoder: Failed to create stream for %s - %s\n",pcFileName,FMOD_ErrorString(eResult));
		return false;
	}
	#endif // _RELEASE

	eResult=m_pobSound->getLength(&m_uiLength,FMOD_TIMEUNIT_PCMBYTES); // Make a note of the PCM length of the sound (i.e. the byte length of the file in its decoded form)

	m_uiPosition=0;
	m_uiStartPos=0;
	m_uiEndPos=m_uiLength;

	#ifndef _RELEASE
	if (eResult!=FMOD_OK)
	{
		ntPrintf("AudioDecoder: Failed to get length of stream %s - %s\n",pcFileName,FMOD_ErrorString(eResult));
		return false;
	}
	#endif // _RELEASE

	strcpy(m_acFileName,pcFileName);

	//ntPrintf("AudioDecoder::LoadFile - Loading %s (%d bytes)\n",pcFileName,m_uiLength);

	return true; // Success

#else

	UNUSED(pcPath);

	return false;

#endif // _AUDIO_SYSTEM_ENABLE
}

void AudioDecoder::SetLoopPoints (unsigned int uiStart,unsigned int uiEnd)
{
	if (uiStart<uiEnd) // Make sure the start is lower than the end
	{
		m_uiStartPos=uiStart;

		if (uiEnd==0 || uiEnd>m_uiLength)
		{
			m_uiEndPos=m_uiLength; // End position will be the length of the file
		}
		else
		{
			m_uiEndPos=uiEnd;
		}

		// If the current seek position is lower than the start position, then seek forwards
		if (m_uiPosition<m_uiStartPos)
		{
			Seek(m_uiStartPos);
		}
	}
}

unsigned int AudioDecoder::GetData (uint8_t* pDestination,unsigned int uiReadSize)
{
#ifdef _AUDIO_SYSTEM_ENABLE

	if (!m_pobSound)
		return 0;

	//*
	unsigned int uiRequestedBytes;

	m_uiPosition+=uiReadSize;

	if (m_uiPosition>m_uiEndPos)
	{
		uiRequestedBytes=uiReadSize-(m_uiEndPos-m_uiPosition);

		m_uiPosition=m_uiEndPos;

		//ntPrintf("AudioDecoder::GetData - Total bytes read=%d\n",m_uiPosition);
	}
	else
	{
		uiRequestedBytes=uiReadSize;
	}

	unsigned int uiBytesRead=0;

	m_pobSound->readData(pDestination,uiRequestedBytes,&uiBytesRead);

	return uiBytesRead;
	//*/

	/*
	unsigned int uiBytesRead=0;

	m_pobSound->readData(pDestination,uiReadSize,&uiBytesRead);

	m_uiPosition+=uiBytesRead;

	if (uiBytesRead<uiReadSize)
	{
		ntPrintf("AudioDecoder::GetData - Total bytes read=%d\n",m_uiPosition);
	}

	return uiBytesRead;
	//*/

	/*
	if (m_pobSound->readData(pDestination,ulReadSize,&uiBytesRead)==FMOD_OK) // Try to read the requested amount to the destination buffer
	{
		m_ulPosition+=uiBytesRead; // Update the seek position

		if (uiBytesRead<ulReadSize)
		{
			ntPrintf("AudioDecoder::GetData - Total bytes read=%d\n",m_ulPosition);
		}

		//ntPrintf("AudioDecoder::GetData - Dest:%x  Req:%d  Read:%d  Pos:%d\n",pDestination,ulReadSize,uiBytesRead,m_ulPosition);

		return uiBytesRead;
	}

	return 0; // Failed to read data, so return 0
	*/


#else

	UNUSED(pDestination);
	UNUSED(ulReadSize);

	return 0;

#endif // _AUDIO_SYSTEM_ENABLE
}

bool AudioDecoder::Seek (unsigned int uiPosition)
{
#ifdef _AUDIO_SYSTEM_ENABLE

	if (m_pobSound)
	{
		if (m_pobSound->seekData(uiPosition)==FMOD_OK) // Try to seek to the requested position (position specified in PCM bytes)
		{
			m_uiPosition=uiPosition;

			return true;
		}
	}

	return false;

#else

	UNUSED(uiPosition);

	return false;

#endif // _AUDIO_SYSTEM_ENABLE
	
}















// ---------------------------------------------------- InteractiveMusicStream ----------------------------------------------------

InteractiveMusicStream::InteractiveMusicStream () :
	m_bFree(true),
	m_pBuffer(0),
	m_ulStartPosition1(0),
	m_ulStartPosition2(0),
	m_ulEndPosition1(0),
	m_ulEndPosition2(0),
	m_pobMusicSample(0),
	m_iNumWaves(0),
	m_iWaveIndex(0),
	m_eSelectionOrder(MUSIC_RANDOM_NOREPEAT),
	m_iPlayCount(0),
	m_fVolume(1.0f)
{
	m_acTargetFileName[0]='\0';
}

InteractiveMusicStream::~InteractiveMusicStream ()
{
}

void InteractiveMusicStream::SetStream (const char* pcFileName,SELECTION_ORDER eSelectionOrder,int iPlayCount,float fVolume)
{
	if (!m_bFree) // This stream is currently in use
		return;

	// We are using a single file

	m_pobMusicSample=0;
	strcpy(m_acTargetFileName,pcFileName);
	m_iNumWaves=1;
	m_eSelectionOrder=eSelectionOrder;
	m_iWaveIndex=-1;

	m_fVolume=fVolume;

	if (iPlayCount<=0)
		m_iPlayCount=-1; // This stream is supposed to loop indefinitely
	else
		m_iPlayCount=iPlayCount; // This stream loops a limited number of times

	m_ulStartPosition1=0;
	m_ulStartPosition2=0;
	m_ulEndPosition1=0;
	m_ulEndPosition2=0;

	if (SetNextWaveFile()) // Make sure the starting audio file is actually valid
	{
		m_bFree=false;
	}	
}

void InteractiveMusicStream::SetStream (InteractiveMusicSample* pobSample,SELECTION_ORDER eSelectionOrder,int iPlayCount,float fVolume)
{
	if (!m_bFree || !pobSample) // This stream is currently in use
		return;

	// We are using an InteractiveMusicSample

	m_pobMusicSample=pobSample;
	m_acTargetFileName[0]='\0';
	m_iNumWaves=pobSample->GetNumWaves();
	m_eSelectionOrder=eSelectionOrder;
	m_iWaveIndex=-1;

	m_fVolume=fVolume;

	if (iPlayCount<=0)
		m_iPlayCount=-1; // This stream is supposed to loop indefinitely
	else
		m_iPlayCount=iPlayCount; // This stream loops a limited number of times

	m_ulStartPosition1=0;
	m_ulStartPosition2=0;
	m_ulEndPosition1=0;
	m_ulEndPosition2=0;

	if (SetNextWaveFile()) // Make sure the starting audio file is actually valid
	{
		m_bFree=false;
	}
}

void InteractiveMusicStream::SetStart (unsigned long ulStartPosition1,unsigned long ulStartPosition2)
{
	#ifdef _STREAM_DEBUG
	//ntPrintf("SetStart (%x): %d -> %d\n",this,ulStartPosition1,ulStartPosition2);
	#endif // _STREAM_DEBUG

	m_ulStartPosition1=ulStartPosition1; // Fade in start offset (the audio file actually starts playing here)
	m_ulStartPosition2=ulStartPosition2; // Fade in finish offset
}

void InteractiveMusicStream::SetEnd (unsigned long ulEndPosition1,unsigned long ulEndPosition2)
{
	if (!m_bFree)
	{
		#ifdef _STREAM_DEBUG
		//ntPrintf("SetEnd (%x): %d -> %d\n",this,ulEndPosition1,ulEndPosition2);
		#endif // _STREAM_DEBUG

		if (m_ulEndPosition2>0 && m_ulEndPosition2<=m_ulStartPosition1) // This stream has been set to end before it begins, therefore it is finished
		{
			m_bFree=true;
		}
		else
		{
			m_ulEndPosition1=ulEndPosition1; // Fade out start offset
			m_ulEndPosition2=ulEndPosition2; // Fade out finish offset (the audio file finishes playing here)
		}
	}
}

void InteractiveMusicStream::Process (unsigned long ulPosition,void* pDestination,unsigned long ulBufferSize)
{
	if (m_bFree) // This stream is not in use
		return;

	unsigned long ulStartingBuffer=m_ulStartPosition1 & (-(long)ulBufferSize); // Calculate which buffer block the stream begins in

	if (ulPosition<ulStartingBuffer) // Do not process this stream if the position is less than the starting buffer of this stream
		return;

	unsigned long ulEndingBuffer=m_ulEndPosition2 & (-(long)ulBufferSize); // Calculate which buffer block the stream ends in

	/*
	// Note: This shouldn't be needed...

	if (m_ulEndPosition2>0 && ulPosition>=(ulEndingBuffer+ulBufferSize)) // Stream has already finished
	{
		m_bFree=true;
		return;
	}
	*/

	if (ulPosition==ulStartingBuffer) // ----- Check to see if the stream is starting -----
	{
		//unsigned long ulSilenceOffset=0;
		unsigned long ulSilenceLength=m_ulStartPosition1-ulPosition;
		unsigned long ulDataOffset=ulSilenceLength;
		unsigned long ulDataLength=ulBufferSize-ulSilenceLength;

		//if (ulSilenceLength>0)
		//{
			//memset(m_pBuffer+ulSilenceOffset,0,ulSilenceLength); // Insert silence at the start of the buffer if required
		//}

		if (ulDataLength>0)
		{
			ReadWaveData(m_pBuffer+ulDataOffset,ulDataLength); // Read wave data into remainder of the buffer

			//ntPrintf("%x starts at %d\n",this,ulPosition+ulDataOffset);
		}

		//#ifdef _STREAM_DEBUG
		//ntPrintf("Process (%x): Starting at %d (%d)\n",this,ulPosition+ulDataOffset,m_ulStartPosition1);
		//#endif // _STREAM_DEBUG
	}
	else if (m_ulEndPosition2>0 && ulPosition==ulEndingBuffer) // ----- Check to see if the stream is finishing -----
	{
		unsigned long ulDataOffset=0;
		unsigned long ulDataLength=m_ulEndPosition2-ulPosition;
		//unsigned long ulSilenceOffset=ulDataLength;
		//unsigned long ulSilenceLength=ulBufferSize-ulDataLength;

		if (ulDataLength>0)
		{
			ReadWaveData(m_pBuffer+ulDataOffset,ulDataLength); // Read wave data into buffer
		}

		//if (ulSilenceLength>0)
		//{
			//memset(m_pBuffer+ulSilenceOffset,0,ulSilenceLength); // Put silence into remainder of buffer
		//}

		m_bFree=true; // That's all folks

		//#ifdef _STREAM_DEBUG
		//ntPrintf("Process (%x): Ending at %d (%d)\n",this,ulPosition+ulSilenceOffset,m_ulEndPosition2);
		//#endif // _STREAM_DEBUG
	}
	else // ----- The stream is active between the start and end positions -----
	{
        ReadWaveData(m_pBuffer,ulBufferSize);

		//ntPrintf("Process (%d): %d\n",this,ulPosition);
	}

	// ----- Perform mixing -----

	signed short* pSourceBuffer=(signed short*)m_pBuffer; // The buffer we are reading from
	signed short* pDestinationBuffer=(signed short*)pDestination; // The buffer we are reading from and writing to

	unsigned long ulChannelPairsRemaining=ulBufferSize >> 2; // 4 bytes per channel pair

	long lBaseVolume=(long)(m_fVolume * 32768.0f);
	long lVolumeModifier;

	unsigned long ulStartDuration=(m_ulStartPosition2-m_ulStartPosition1) >> 2;
	unsigned long ulEndDuration=(m_ulEndPosition2-m_ulEndPosition1) >> 2;

	if (m_ulEndPosition2>0) // This stream has an end position
	{
		while(ulChannelPairsRemaining--)
		{
			if (ulPosition<m_ulStartPosition1 || ulPosition>=m_ulEndPosition2) // Is not yet started or has finished
			{
				pDestinationBuffer+=2;
				pSourceBuffer+=2;
				ulPosition+=4;
				continue;
			}
			else if (ulPosition<m_ulStartPosition2) // Stream is fading in
			{
				lVolumeModifier=(ulPosition-m_ulStartPosition1)<<10;
				lVolumeModifier/=ulStartDuration;
				lVolumeModifier=lVolumeModifier<<3;
				lVolumeModifier=(lVolumeModifier * lBaseVolume) >> 15;
			}
			else if (ulPosition>=m_ulEndPosition1) // Stream is fading out
			{
				lVolumeModifier=(m_ulEndPosition2-ulPosition)<<10;
				lVolumeModifier/=ulEndDuration;
				lVolumeModifier=lVolumeModifier<<3;
				lVolumeModifier=(lVolumeModifier * lBaseVolume) >> 15;
			}
			else // Stream is playing normally
			{
				lVolumeModifier=lBaseVolume;
			}

			ulPosition+=4;

			// Mix left channel
			if (*pSourceBuffer) // Only mix if the source contains something to mix
			{
				long result = (((long)*pSourceBuffer) * lVolumeModifier) >> 15;

				result += (long)*pDestinationBuffer; // Mix source with destination

				// Ensure the result is clipped
				if (result < -32768)
					result=-32768;
				else if (result > 32767)
					result=32767;

				// Write result to destination
				*pDestinationBuffer=(short)result;
			}

			// Go to next channel
			++pDestinationBuffer;
			++pSourceBuffer;

			// Mix right channel
			if (*pSourceBuffer) // Only mix if the source contains something to mix
			{
				long result = (((long)*pSourceBuffer) * lVolumeModifier) >> 15;

				result += (long)*pDestinationBuffer; // Mix source with destination

				// Ensure the result is clipped
				if (result < -32768)
					result=-32768;
				else if (result > 32767)
					result=32767;

				// Write result to destination
				*pDestinationBuffer=(short)result;
			}

			// Go to next channel
			++pDestinationBuffer;
			++pSourceBuffer;
		}

	}
	else // No end position has been specified
	{
		while(ulChannelPairsRemaining--)
		{
			if (ulPosition<m_ulStartPosition1) // Is not yet started or has finished
			{
				++pDestinationBuffer;
				++pDestinationBuffer;
				++pSourceBuffer;
				++pSourceBuffer;
				ulPosition+=4;
				continue;
			}
			else if (ulPosition<m_ulStartPosition2) // Stream is fading in
			{
				lVolumeModifier=(ulPosition-m_ulStartPosition1)<<10;
				lVolumeModifier/=(m_ulStartPosition2-m_ulStartPosition1)>>2;
				lVolumeModifier=lVolumeModifier<<3;
				lVolumeModifier=(lVolumeModifier * lBaseVolume) >> 15;
			}
			else // Stream is playing normally
			{
				lVolumeModifier=lBaseVolume;
			}

			ulPosition+=4;

			// Mix left channel
			if (*pSourceBuffer) // Only mix if the source contains something to mix
			{
				long result = (((long)*pSourceBuffer) * lVolumeModifier) >>15;

				result += (long)*pDestinationBuffer; // Mix source with destination

				// Ensure the result is clipped
				if (result < -32768)
					result=-32768;
				else if (result > 32767)
					result=32767;

				// Write result to destination
				*pDestinationBuffer=(short)result;
			}

			// Go to next channel
			++pDestinationBuffer;
			++pSourceBuffer;

			// Mix right channel
			if (*pSourceBuffer) // Only mix if the source contains something to mix
			{
				long result = (((long)*pSourceBuffer) * lVolumeModifier) >>15;

				result += (long)*pDestinationBuffer; // Mix source with destination

				// Ensure the result is clipped
				if (result < -32768)
					result=-32768;
				else if (result > 32767)
					result=32767;

				// Write result to destination
				*pDestinationBuffer=(short)result;
			}

			// Go to next channel
			++pDestinationBuffer;
			++pSourceBuffer;
		}
	}
}

void InteractiveMusicStream::ReadWaveData (uint8_t* pBuffer,unsigned long ulSize)
{
	const unsigned int uiBytesRead=m_obAudioDecoder.GetData(pBuffer,ulSize);

	if (uiBytesRead<ulSize) // We have reached the end of this file we are decoding
	{
		const unsigned int uiRemainingBuffer=ulSize-uiBytesRead; // Calculate the amount of buffer remaining that we need to fill		

		if (m_iPlayCount>0)
			--m_iPlayCount; // Decrement our play count if necessary

		if (m_iPlayCount==0) // Check to see if this stream has naturally expired
		{
			if (uiRemainingBuffer>0)
			{
				memset(pBuffer+uiBytesRead,0,uiRemainingBuffer); // Stream has finished, fill the remaining buffer with silence
			}

			m_bFree=true; // This stream is now free

			//ntPrintf("InteractiveMusicStream::ReadWaveData - Stream has finished playing\n");
		}
		else // Otherwise we want to go to the next file
		{
			if (SetNextWaveFile()) // Get the next wave file
			{
				m_obAudioDecoder.GetData(pBuffer+uiBytesRead,uiRemainingBuffer); // Feed the beginning of the next file into whatever space is remaining in the buffer
			}
			else // There has been a problem getting the next file, so finish now
			{
				m_bFree=true; 

				//ntPrintf("InteractiveMusicStream::ReadWaveData - Unable to get next wave file, to finishing\n");
			}
		}
	}

	/*
	// Calculate the number of bytes remaining in this current wave file
	const unsigned long ulBytesRemaining=m_obAudioDecoder.GetLength() - m_obAudioDecoder.GetSeekPosition();

	if (ulBytesRemaining<ulSize) // We have reached the end of the wave data
	{
		if (ulBytesRemaining>0) // Read remaining chunk from file into buffer (if there is any left to read)
		{
			m_obAudioDecoder.GetData(pBuffer,ulBytesRemaining); 
		}

		const unsigned long ulRemainingBuffer=ulSize - ulBytesRemaining; // Calculate how much of the buffer is left over after we have read the remainder of the current file

		if (m_iPlayCount>0)
			--m_iPlayCount; // Decrement our play count

		if (m_iPlayCount==0) // Check to see if this stream has naturally expired
		{
			if (ulRemainingBuffer>0)
			{
				memset(pBuffer+ulBytesRemaining,0,ulRemainingBuffer); // Stream has finished, fill the remaining buffer with silence
			}

			m_bFree=true; // This stream is now free

			ntPrintf("InteractiveMusicStream::ReadWaveData - Stream has finished playing\n");
		}
		else
		{
			if (SetNextWaveFile()) // Get the next wave file
			{
				m_obAudioDecoder.GetData(pBuffer+ulBytesRemaining,ulRemainingBuffer); // Feed the beginning of the next file into whatever space is remaining in the buffer
			}
			else
			{
				m_bFree=true; // There has been a problem getting the next file, so finish now

				ntPrintf("InteractiveMusicStream::ReadWaveData - Unable to get next wave file, to finishing\n");
			}
		}
	}
	else // We are in the middle of the wave data
	{
		m_obAudioDecoder.GetData(pBuffer,ulSize);
	}
	*/

	//ntPrintf("Read: %d (%d)\n",m_ulSeekPosition,ulSize);
}

bool InteractiveMusicStream::SetNextWaveFile ()
{
	// If we are drawing from a pool of sound files, then pick one.
	// Otherwise we will assume we are playing a single file, in which case use the existing file path.

	if (m_pobMusicSample)
	{
		// Set the wave file index
		switch(m_eSelectionOrder)
		{
			case MUSIC_SEQUENTIAL:
			{
				++m_iWaveIndex;

				if (m_iWaveIndex>=m_iNumWaves)
					m_iWaveIndex=0;

				break;
			}

			default:
			//case MUSIC_SHUFFLED:
			//case MUSIC_RANDOM_NOREPEAT:
			{
				// Randomly choose a wave file but make sure its not the same as the last one

				if (m_iNumWaves<2)
				{
					m_iWaveIndex=0;
				}
				else
				{
				int i;

				do
				{
					i = (drand() % (m_iNumWaves*100))/100; // Make the number bigger, then divide back down - to give better randomness...

				}
					while(i==m_iWaveIndex); // If the new index is the same as the old wave index, try and find a new random one
				
				m_iWaveIndex=i;
			}
		}
		}

		const char* pcFileName=m_pobMusicSample->GetSoundFile(m_iWaveIndex);

		if (pcFileName) // Make sure we have a valid file name!
		{
			strcpy(m_acTargetFileName,pcFileName);
		}
	}
	
	//ntPrintf("InteractiveMusicStream::SetNextWaveFile -> %s  (index=%d/%d)\n",m_acFilePath,m_iWaveIndex,m_iNumWaves);

	return m_obAudioDecoder.LoadFile(m_acTargetFileName,false);
}

unsigned int InteractiveMusicStream::GetSampleID ()
{
	if (m_pobMusicSample)
	{
		return m_pobMusicSample->GetName().GetHash(); // Get the hash value of the InteractiveMusicSample thats currently playing on this stream
	}

	return 0; // We are not currently using an InteractiveMusicSample on this stream
}











// ---------------------------------------------------- InteractiveMusicManager ----------------------------------------------------

InteractiveMusicManager::InteractiveMusicManager () :
	m_pStreamBuffer(0),
	m_pobSystem(0),
	m_pobSound(0),
	m_pobChannel(0),
	m_fGlobalIntensity(0.0f),
	m_fStopFade(-1.0f),
	m_ulPosition(0),
	m_ulNextBeatMarker(0),
	m_ulNextUserMarker(0),
	m_ulBeatInterval(0),
	m_iNumMarkers(0),
	m_iCurrentMarker(0),
	m_pobTransition(0),
	m_pobTargetMusicState(0)
{
	m_acMediaPath[0]='\0';
}

InteractiveMusicManager::~InteractiveMusicManager ()
{
	if (m_pobSound)
	{
		m_pobSound->release();
	}

	if (m_pStreamBuffer)
	{
		NT_DELETE_ARRAY( m_pStreamBuffer );
	}
}

void InteractiveMusicManager::Init ()
{
#ifdef _AUDIO_SYSTEM_ENABLE

	if (!g_ShellOptions->m_bEnableMusic || !AudioSystem::Get().IsInitialised() || AudioSystem::Get().GetFMODEventSystem()->getSystemObject(&m_pobSystem)!=FMOD_OK)
	{
		return;
	}

	// Allocate space for a work buffer which will be used by all the audio stream
	m_pStreamBuffer = NT_NEW uint8_t [MUSIC_STREAM_BUFFER_SIZE];
	
	memset(m_pStreamBuffer,0,MUSIC_STREAM_BUFFER_SIZE); // Zero it out, we don't want to start with any noise in it!

	for(int i=0; i<MUSIC_MAX_STREAMS; ++i)
	{
		m_obStream[i].SetWorkBuffer(m_pStreamBuffer); // Assign this buffer to each stream
	}

	// Configure our sound channel
	FMOD_CREATESOUNDEXINFO stSoundInfo;
	memset(&stSoundInfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));

	stSoundInfo.cbsize=sizeof(FMOD_CREATESOUNDEXINFO);
	stSoundInfo.numchannels=2;
	stSoundInfo.format=FMOD_SOUND_FORMAT_PCM16;
	stSoundInfo.defaultfrequency=44100;
	stSoundInfo.decodebuffersize = MUSIC_DECODE_BUFFER_SIZE;
	stSoundInfo.length=MUSIC_BUFFER_SIZE;
	stSoundInfo.pcmreadcallback=pcmreadcallback;
	stSoundInfo.pcmsetposcallback=0;//pcmsetposcallback;

	FMOD_RESULT eResult=m_pobSystem->createSound(0,	FMOD_CREATESTREAM | FMOD_2D | FMOD_OPENUSER | FMOD_SOFTWARE | FMOD_LOOP_NORMAL, &stSoundInfo, &m_pobSound);

	if (eResult!=FMOD_OK)
	{
		#ifndef _RELEASE
		ntPrintf("InteractiveMusicManager: Failed to create sound - %s\n",FMOD_ErrorString(eResult));
		#endif // _RELEASE

		return;
	}

	// Initialise the sound
	eResult=m_pobSystem->playSound(FMOD_CHANNEL_FREE,m_pobSound,true,&m_pobChannel); // Note the FMOD channel starts off in a paused state, must be unpaused before it will start playing

	if (eResult!=FMOD_OK)
	{
		#ifndef _RELEASE
		ntPrintf("InteractiveMusicManager: Failed to play sound - %s\n",FMOD_ErrorString(eResult));
		#endif // _RELEASE

		return;
	}

	// Disable reverb on this channel
	/*
	if (m_pobChannel)
	{
		FMOD_REVERB_CHANNELPROPERTIES stReverb;
//		stReverb.Direct=-10000;
//		stReverb.DirectHF=-10000;
//		stReverb.Room=-10000;
//		stReverb.RoomHF=-10000;
//		stReverb.Obstruction=-10000;
//		stReverb.ObstructionLFRatio=0.0f;
//		stReverb.Occlusion=0;
//		stReverb.OcclusionLFRatio=0.25f;
//		stReverb.OcclusionRoomRatio=1.5f;
//		stReverb.OcclusionDirectRatio=1.0f;
//      stReverb.Exclusion=0;
//		stReverb.ExclusionLFRatio=1.0f;
//		stReverb.OutsideVolumeHF=0;
//		stReverb.DopplerFactor=0.0f;
//		stReverb.RolloffFactor=0.0f;
//		stReverb.RoomRolloffFactor=0.0f;
//		stReverb.AirAbsorptionFactor=1.0f;
//		stReverb.Flags=FMOD_REVERB_CHANNELFLAGS_DEFAULT;

		if (m_pobChannel->getReverbProperties(&stReverb)!=FMOD_OK)
		{
			ntPrintf("InteractiveMusicManager: Failed to get reverb properties for channel\n");
		}

		stReverb.Room=-10000;

		if (m_pobChannel->setReverbProperties(&stReverb)!=FMOD_OK)
		{
			ntPrintf("InteractiveMusicManager: Failed to set reverb properties for channel\n");
		}
	}
	*/

	// Kick off the channel
	//m_pobChannel->setPaused(false);



	#ifndef _RELEASE
	ntPrintf("InteractiveMusicManager: Successfully initialized\n");
	#endif // _RELEASE

#endif // _AUDIO_SYSTEM_ENABLE
}

void InteractiveMusicManager::SetMediaPath (const char* pcPath)
{
	const int iPathLength=strlen(pcPath);

	strcpy(m_acMediaPath,pcPath);

	if (m_acMediaPath[iPathLength-1]!='/' && m_acMediaPath[iPathLength-1]!='\\') // Ensure a slash operator is at the end of the path string
	{
		m_acMediaPath[iPathLength]='/';
		m_acMediaPath[iPathLength+1]='\0';
	}
	
	#ifndef _RELEASE
	ntPrintf("InteractiveMusicManager::SetMediaPath -> %s -> %s\n",pcPath,m_acMediaPath);
	#endif // _RELEASE
}

void InteractiveMusicManager::SetGlobalVolume (float fVolume)
{
	if (m_pobChannel)
	{
		m_pobChannel->setVolume(fVolume);
	}
}

void InteractiveMusicManager::SetPause (bool bPause)
{
#ifdef _AUDIO_SYSTEM_ENABLE

	m_pobChannel->setPaused(bPause);

#else

	UNUSED(bPause);

#endif // _AUDIO_SYSTEM_ENABLE
}

void InteractiveMusicManager::SetIntensity (float fIntensity)
{
#ifdef _AUDIO_SYSTEM_ENABLE

	if (!m_pobSystem)
		return;

	// First do a search through the transitions to find one that matches the from/to intensities
	for(ntstd::List<InteractiveMusicTransition*>::iterator obIt=m_obMusicTransitionList.begin(); obIt!=m_obMusicTransitionList.end(); ++obIt)
	{
		if (MUSIC_INTENSITY_MATCH((*obIt)->m_fFromIntensity, m_fGlobalIntensity) && MUSIC_INTENSITY_MATCH((*obIt)->m_fToIntensity, fIntensity)) // We have found one with a matching from intensity
		{
			if ((*obIt)->m_fToIntensity==0.0f) // We are transitioning to silence
			{
				#ifndef _RELEASE
				//ntPrintf("InteractiveMusicManager: Setting intensity %.2f to %.2f\n", m_fGlobalIntensity,fIntensity);
				#endif // _RELEASE

				m_obCriticalSection.Enter();
				m_pobTransition=(*obIt); // Set the transition
				m_pobTargetMusicState=0; // Since we are transitioning to silence, no target state is set
				m_obCriticalSection.Leave();
				
				m_fGlobalIntensity=fIntensity; // Update the intensity

				return;
			}
			else // We are transitioning to another state
			{
				// Search through the states to find one with a matching intensity
				for(ntstd::List<InteractiveMusicState*>::iterator obIt2=m_obMusicStateList.begin(); obIt2!=m_obMusicStateList.end(); ++obIt2)
				{
					if (MUSIC_INTENSITY_MATCH(fIntensity, (*obIt2)->m_fIntensity)) // Found one
					{
						#ifndef _RELEASE
						//ntPrintf("InteractiveMusicManager: Setting intensity %.2f to %.2f\n", m_fGlobalIntensity,fIntensity);
						#endif // _RELEASE

						m_obCriticalSection.Enter();
						m_pobTransition=(*obIt); // Set the transition
						m_pobTargetMusicState=(*obIt2); // Set the target state
						m_obCriticalSection.Leave();

						m_fGlobalIntensity=fIntensity; // Update the intensity

						return;
					}
				}

				#ifndef _RELEASE
				ntPrintf("InteractiveMusicManager: Failed to perform transition from %0.2f to %0.2f, unable to find state with intensity of %0.2f\n",m_fGlobalIntensity,fIntensity,fIntensity);
				#endif // _RELEASE

				return;
			}
		}
	}

	// Failed to fine a transition
	#ifndef _RELEASE
	ntPrintf("InteractiveMusicManager: No transition defined for %0.2f to %0.2f intensity change\n",m_fGlobalIntensity,fIntensity);
	#endif // _RELEASE

#else

	UNUSED(fIntensity);

#endif // _AUDIO_SYSTEM_ENABLE
}

void InteractiveMusicManager::StopAll (float fFadeTime)
{
#ifdef _AUDIO_SYSTEM_ENABLE

	if (!m_pobSystem)
		return;

	m_fGlobalIntensity=0.0f;

	m_obCriticalSection.Enter();
	m_fStopFade=fFadeTime;
	m_pobTransition=0;
	m_pobTargetMusicState=0;
	m_obCriticalSection.Leave();

#else

	UNUSED(fFadeTime);

#endif // _AUDIO_SYSTEM_ENABLE
}


// This function is called in the FMOD sound callback
void InteractiveMusicManager::ThreadUpdate (void* pBuffer,unsigned int uiSize)
{
	unsigned long ulPosition;
	float fStopFade;
	InteractiveMusicTransition* pobTransition;
	InteractiveMusicState* pobTargetState;
	
	// Get shared data
	m_obCriticalSection.Enter();
	ulPosition=m_ulPosition; // Get the current position
	m_ulPosition+=(unsigned long)uiSize; // Increment our position counter
	pobTransition=m_pobTransition;
	pobTargetState=m_pobTargetMusicState;
	fStopFade=m_fStopFade;
	m_obCriticalSection.Leave();

	//ntPrintf("ThreadUpdate: %d + %d\n",ulPosition,uiSize);

	// Update marker stuff here
	UpdateMarkers(ulPosition,uiSize);

	// We want to stop the music
	if (fStopFade>=0.0f)
	{
		unsigned long ulFadeOut1=ulPosition;
		unsigned long ulFadeOut2=ulPosition+ConvertTimeToBytes(fStopFade);

		//ntPrintf("Source finishes: %d -> %d\n",ulFadeOut1,ulFadeOut2);

		// Fade out all existing streams as soon as possible
		for(int i=0; i<MUSIC_MAX_STREAMS; ++i)
		{
			m_obStream[i].SetEnd(ulFadeOut1,ulFadeOut2);
		}
	}
	else if (pobTransition) // It's time to do a transition
	{
		const unsigned long ulMarkerPosition=GetNextMarker(ulPosition); // Get the next marker position

		//ntPrintf("Transition: Position=%d  Marker position=%d\n",ulPosition,ulMarkerPosition);

		unsigned long ulFadeOut1=ulMarkerPosition+ConvertTimeToBytes(pobTransition->m_fSourceEnd1);
		unsigned long ulFadeOut2=ulMarkerPosition+ConvertTimeToBytes(pobTransition->m_fSourceEnd2);

		//ntPrintf("Source finishes: %d -> %d\n",ulFadeOut1,ulFadeOut2);

		// Stop all existing streams at the next marker position with fadeout (if required)
		for(int i=0; i<MUSIC_MAX_STREAMS; ++i)
		{
			if (!pobTransition->IsExempt(m_obStream[i].GetSampleID())) // Check to see if this sample should ignore the transition
				m_obStream[i].SetEnd(ulFadeOut1,ulFadeOut2);
		}

		// ----- Add an intermediate wave if one has been defined -----
		
		const char* pcSoundFile=pobTransition->GetIntermediateSoundFile();

		if (pobTransition->m_fIntermediateVolume>0.0f && pcSoundFile) // Check to see if an intermediate wave has been defined for this transition
		{
			// Get a free stream for this
			InteractiveMusicStream* pobStream=FindFreeStream();

			if (pobStream)
			{
				unsigned long ulStart1=ulMarkerPosition+ConvertTimeToBytes(pobTransition->m_fIntermediateStart1);
				unsigned long ulStart2=ulMarkerPosition+ConvertTimeToBytes(pobTransition->m_fIntermediateStart2);

				unsigned long ulEnd1=ulMarkerPosition+ConvertTimeToBytes(pobTransition->m_fIntermediateEnd1);
				unsigned long ulEnd2=ulMarkerPosition+ConvertTimeToBytes(pobTransition->m_fIntermediateEnd2);

				pobStream->SetStream(pcSoundFile,MUSIC_SEQUENTIAL,1,pobTransition->m_fIntermediateVolume);
				pobStream->SetStart(ulStart1,ulStart2);
				pobStream->SetEnd(ulEnd1,ulEnd2);

				//ntPrintf("Intermediate: Start %d -> %d  Finish %d -> %d\n",ulStart1,ulStart2,ulEnd1,ulEnd2);
			}
		}

		// Add more streams if there is a target state
		if (pobTargetState)
		{
			// ----- Add state streams -----

			unsigned long ulStart1=ulMarkerPosition+ConvertTimeToBytes(pobTransition->m_fDestinationStart1);
			unsigned long ulStart2=ulMarkerPosition+ConvertTimeToBytes(pobTransition->m_fDestinationStart2);

			for(ntstd::List<InteractiveMusicSample*>::iterator obIt=pobTargetState->m_obSampleList.begin(); obIt!=pobTargetState->m_obSampleList.end(); ++obIt)
			{
				if (!IsPlaying((*obIt)->GetName().GetHash())) // Make sure this sample isn't already playing
				{
					InteractiveMusicStream* pobStream=FindFreeStream();
				
					if (pobStream)
					{
						InteractiveMusicSample* pobSample=(*obIt);

						unsigned long ulStartOffset=ConvertTimeToBytes(pobSample->m_fStartTime); // Specifies an additional start offset from the point where the stream is supposed to begin
						unsigned long ulNewStart1=ulStart1+ulStartOffset; // FadeInStart offset + any additional specified start offset
						unsigned long ulNewStart2=(ulStart2 < ulNewStart1 ? ulNewStart1 : ulStart2); // This is the FadeInEnd offset, unless its lower than the newstart. If its lower, then it becomes the same

						// Setup the stream
						pobStream->SetStream(pobSample,pobSample->m_eSelectionOrder,pobSample->m_iPlayCount,pobSample->m_fVolume);

						//ntPrintf("Destination begins: %d -> %d\n",ulNewStart1,ulNewStart2);

						// Set just the start point for the stream, since it will either loop indefinitely or play and expire naturally
						pobStream->SetStart(ulNewStart1,ulNewStart2);
					}
				}
			}

			//ntPrintf("Set markerinfo from %d\n",ulStart1);

			// Reconfigure the markers for this new state - these begin from where the next state begins
			SetMarkerInformation(pobTargetState,ulStart1);
		}

		m_obCriticalSection.Enter();
		m_pobTransition=0;
		m_pobTargetMusicState=0;
		m_obCriticalSection.Leave();
	}
	
	// ----- Process all streams -----

	memset(pBuffer,0,uiSize); // Clear the buffer first, before we mix in data from the streams

	for(int i=0; i<MUSIC_MAX_STREAMS; ++i)
	{
		m_obStream[i].Process(ulPosition,pBuffer,uiSize);
	}

	//ntPrintf("Time=%.2f\n",(double)ulPosition / 176400.0);
}






// This should be called when performing a transition. The input position is the current offset we are at, so that we know where the markers are supposed to start.
void InteractiveMusicManager::SetMarkerInformation (InteractiveMusicState* pobMusicState,unsigned long ulPosition)
{
	if (!pobMusicState) // No music state has been specified
	{
		m_ulNextBeatMarker=0;
		m_ulNextUserMarker=0;
		m_ulBeatInterval=0;
		m_iNumMarkers=0;
		m_iCurrentMarker=0;
		return;	
	}

	// ----- Calculate beat marker -----

	if (pobMusicState->m_fBeatInterval==0.0f)
	{
		m_ulBeatInterval=0; // We have no beat interval defined
	}
	else
	{
		m_ulBeatInterval = ConvertTimeToBytes(pobMusicState->m_fBeatInterval);
	}

	m_ulNextBeatMarker=ulPosition+m_ulBeatInterval;

	// ----- Calculate user marker -----

	if (pobMusicState->m_obMarkerList.size()>0)
	{
		float fLastOffset=0.0f;

		m_iCurrentMarker=1;
		m_iNumMarkers=0;

		for(ntstd::List<InteractiveMusicMarker*>::iterator obIt=pobMusicState->m_obMarkerList.begin(); obIt!=pobMusicState->m_obMarkerList.end(); ++obIt)
		{
			float fThisOffset=(*obIt)->GetTime();
			m_ulMarkerDelta[m_iNumMarkers]=ConvertTimeToBytes(fThisOffset-fLastOffset);
			fLastOffset=fThisOffset;
			++m_iNumMarkers;

			if (m_iNumMarkers>=MUSIC_MAX_MARKERS) // We have reached our limit
			{
				break;
			}
		}

		m_ulNextUserMarker=ulPosition+m_ulMarkerDelta[0];
	}
	else
	{
		m_ulNextUserMarker=ulPosition;
	}
}

void InteractiveMusicManager::UpdateMarkers (unsigned long ulPosition,unsigned long ulBufferSize)
{
	if (m_iNumMarkers==0 && m_ulBeatInterval==0) // There are no markers defined
	{
		// The current position will be are next marker point
		m_ulNextUserMarker=ulPosition;
		m_ulNextBeatMarker=ulPosition;
		return;
	}

	// Update the beat interval marker
	if (m_ulBeatInterval>0)
	{
		unsigned long ulNextBeatMarker=m_ulNextBeatMarker & (-(long)ulBufferSize); // Calculate which buffer this marker resides in

		if (ulPosition>=ulNextBeatMarker) // The position has entered or passed the marker's buffer position
		{
			m_ulNextBeatMarker+=m_ulBeatInterval;
		}
	}

	// Update the user marker
	if (m_iNumMarkers>0)
	{
		unsigned long ulNextUserMarker=m_ulNextUserMarker & (-(long)ulBufferSize); // Calculate which buffer this marker resides in

		if (ulPosition>=ulNextUserMarker)
		{
			m_ulNextUserMarker+=m_ulMarkerDelta[m_iCurrentMarker];

			++m_iCurrentMarker;

			if (m_iCurrentMarker>=m_iNumMarkers)
				m_iCurrentMarker=0;
		}
	}
}

unsigned long InteractiveMusicManager::GetNextMarker (unsigned long ulPosition)
{
	//*
	m_obCriticalSection.Enter();
	unsigned long ulNextUserMarker=m_ulNextUserMarker;
	unsigned long ulNextBeatMarker=m_ulNextBeatMarker;
	unsigned long ulBeatInterval=m_ulBeatInterval;
	int iNumMarkers=m_iNumMarkers;
	m_obCriticalSection.Leave();

	if (iNumMarkers==0 && ulBeatInterval==0) // There are no markers defined, so the next marker is the current position
		return ulPosition;

	unsigned long ulNextMarker=0xffffffff;

	if (iNumMarkers>0 && ulNextUserMarker<ulNextMarker)
		ulNextMarker=ulNextUserMarker;

	if (ulBeatInterval>0 && ulNextBeatMarker<ulNextMarker)
		ulNextMarker=ulNextBeatMarker;

	return ulNextMarker; // Return whichever marker is the soonest
}


/*
float GetTimeUntilNextMarker ()
{
	m_obCriticalSection.Enter();
	unsigned long ulPosition=m_ulPosition;
	m_obCriticalSection.Leave();
	
	unsigned long ulNextMarker=GetNextMarker();
	unsigned long ulDuration=ulNextMarker-ulPosition;

	
	if (ulDuration>0)
	{
		return ((float)(ulDuration/MUSIC_BYTES_PER_SECOND));
	}
	else
	{
		return 0.0f;
	}
}
*/

void InteractiveMusicManager::RegisterMusicState (InteractiveMusicState* pobMusicState)
{
	if (pobMusicState && !FindMusicState(pobMusicState->GetName())) // Prevent duplicates
	{
		m_obMusicStateList.push_back(pobMusicState);
	}
}

void InteractiveMusicManager::RegisterMusicTransition (InteractiveMusicTransition* pobMusicTransition)
{
	if (pobMusicTransition && !FindMusicTransition(pobMusicTransition->GetName())) // Prevent duplicates
	{
		m_obMusicTransitionList.push_back(pobMusicTransition);
	}
}

void InteractiveMusicManager::UnregisterMusicState (InteractiveMusicState* pobMusicState)
{
	m_obMusicStateList.remove(pobMusicState);
}

void InteractiveMusicManager::UnregisterMusicTransition (InteractiveMusicTransition* pobMusicTransition)
{
	m_obMusicTransitionList.remove(pobMusicTransition);
}

InteractiveMusicState* InteractiveMusicManager::FindMusicState (const CHashedString& obName)
{
	for(ntstd::List<InteractiveMusicState*>::iterator obIt=m_obMusicStateList.begin(); obIt!=m_obMusicStateList.end(); ++obIt)
	{
		if (obName==(*obIt)->GetName())
			return (*obIt);
	}

	return 0;
}

InteractiveMusicTransition* InteractiveMusicManager::FindMusicTransition (const CHashedString& obName)
{
	for(ntstd::List<InteractiveMusicTransition*>::iterator obIt=m_obMusicTransitionList.begin(); obIt!=m_obMusicTransitionList.end(); ++obIt)
	{
		if (obName==(*obIt)->GetName())
			return (*obIt);
	}

	return 0;
}

InteractiveMusicStream* InteractiveMusicManager::FindFreeStream ()
{
	for(int i=0; i<MUSIC_MAX_STREAMS; ++i)
	{
		if (m_obStream[i].IsFree())
			return &m_obStream[i];
	}

	return 0;
}

bool InteractiveMusicManager::IsPlaying (unsigned int uiSampleID)
{
	for(int i=0; i<MUSIC_MAX_STREAMS; ++i)
	{
		if (!m_obStream[i].IsFree() && uiSampleID==m_obStream[i].GetSampleID())
			return true;
	}

	return false;
}

float InteractiveMusicManager::GetGlobalVolume ()
{
	if (m_pobChannel)
	{
		float fVolume;

		if (m_pobChannel->getVolume(&fVolume)==FMOD_OK)
			return fVolume;
	}

	return 0.0f;
}

float InteractiveMusicManager::GetPosition ()
{
	unsigned long ulPosition;

	m_obCriticalSection.Enter();
	ulPosition=m_ulPosition;
	m_obCriticalSection.Leave();

	float fPosition=(float)(ulPosition / MUSIC_BYTES_PER_SECOND);

	return fPosition;
}

void InteractiveMusicManager::ExtendFilePath (const char* pcFileName,char* pcOutputPath)
{
	sprintf(pcOutputPath,"%s%s",m_acMediaPath,pcFileName);
}





