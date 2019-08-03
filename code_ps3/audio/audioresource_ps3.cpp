/***************************************************************************************************
*
*	$Header:: /game/audioresource_pc.cpp 6     1/08/03 10:24 Harvey                                $
*
*	Source file for CAudioResourceManager and related classes.
*
*	CHANGES
*
*	20/06/2003	Harvey	Created
*
***************************************************************************************************/


#include "audio/audioresource.h"
#include "audio/soundutil.h"

#ifndef _RELEASE

//#define _SHOW_WARNING_MESSAGES  - Harvey, can you make warning messages enabled from a .config file flag please? Thanks! :)
//#define _SHOW_WAVE_FILES_LOADED

#endif // _RELEASE

///***************************************************************************************************
//*
//*	FUNCTION		CWaveBank::CWaveBank
//*
//*	DESCRIPTION		
//*
//***************************************************************************************************/
//
//CWaveBank::CWaveBank () :
//	m_pstWaveInfo(0),
//	m_pvData(0),
//	m_hFileHandle(INVALID_HANDLE_VALUE)
//{
//}
//
///***************************************************************************************************
//*
//*	FUNCTION		CWaveBank::~CWaveBank
//*
//*	DESCRIPTION		
//*
//***************************************************************************************************/
//
//CWaveBank::~CWaveBank ()
//{
//	while(m_obWaveFileList.size()>0)
//	{
//		NT_DELETE( m_obWaveFileList.back() );
//		m_obWaveFileList.pop_back();
//	}
//
//	NT_DELETE_ARRAY( m_pstWaveInfo );
//
//	NT_DELETE_ARRAY( m_pvData );
//	
//	if (m_hFileHandle!=INVALID_HANDLE_VALUE)
//		CloseHandle(m_hFileHandle);
//}
//
///***************************************************************************************************
//*
//*	FUNCTION		CWaveBank::Open
//*
//*	DESCRIPTION		
//*
//***************************************************************************************************/
//
//bool CWaveBank::Open (const char* pcFriendlyName,const char* pcPath,WAVE_BANK_TYPE eType,int iMillisec,bool bZeroLatency)
//{
//	u_long ulBytesRead;
//
//	m_obID=CHashedString(pcFriendlyName);
//	m_iMillisecBuffer=iMillisec;
//	m_bZeroLatency=bZeroLatency;
//
//	// Set type
//
//	m_eType=eType;
//
//	// Create file handle to the wavebank
//	
//	m_hFileHandle=CreateFile(pcPath,GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_FLAG_RANDOM_ACCESS,NULL);
//
//	if (m_hFileHandle==INVALID_HANDLE_VALUE)
//	{
//		return false;
//	}
//
//	// Read the header
//
//	ReadFile(m_hFileHandle,&m_stHeader.usWaveCount,HEADER_SIZE,&ulBytesRead,NULL);
//
//	// Read the waveinfo structures
//
//	m_pstWaveInfo=NT_NEW WAVEINFO [m_stHeader.usWaveCount];
//
//	ntAssert(m_pstWaveInfo);
//
//	ReadFile(m_hFileHandle,m_pstWaveInfo,INFO_SIZE*m_stHeader.usWaveCount,&ulBytesRead,NULL);
//
//	// Calculate various sizes
//
//	m_ulFileSize=GetFileSize(m_hFileHandle,NULL);
//	m_ulInfoChunkSize=HEADER_SIZE+(INFO_SIZE*m_stHeader.usWaveCount);
//	m_ulDataChunkSize=m_ulFileSize-m_ulInfoChunkSize;
//
//	// If this wavebank is streamed from main memory, write the data chunk to memory
//
//	if (m_eType==WAVE_BANK_RAM)
//	{
//		// Calculate the sizes of the various portions of the wavebank
//
//		m_pvData=NT_NEW uint8_t [m_ulDataChunkSize];
//
//		ntAssert(m_pvData);
//
//		SetFilePointer(m_hFileHandle,m_ulInfoChunkSize,NULL,FILE_BEGIN);
//
//		if (!ReadFile(m_hFileHandle,m_pvData,m_ulDataChunkSize,&ulBytesRead,NULL))
//			return false;
//	}
//
//	// Create wavefile object for each entry in the list
//
//	for(int iCount=0; iCount<m_stHeader.usWaveCount; ++iCount)
//	{
//		CWaveFile* pobWaveFile=NT_NEW CWaveFile(this,&m_pstWaveInfo[iCount]);
//		ntAssert(pobWaveFile);
//		m_obWaveFileList.push_back(pobWaveFile);
//
//		#ifdef _SHOW_WAVE_FILES_LOADED
//		ntPrintf("CWaveBank: WaveFile %d (Format=%d  Channels=%d  SampPerSec=%d  BytePerSec=%d  Bits=%d  Offset=%d  Size=%d  Duration=%0.3f)\n",
//            m_pstWaveInfo[iCount].obHash,
//			m_pstWaveInfo[iCount].usFormatTag,
//			m_pstWaveInfo[iCount].usChannels,
//			m_pstWaveInfo[iCount].ulSamplesPerSec,
//			m_pstWaveInfo[iCount].ulBytePerSec,
//			m_pstWaveInfo[iCount].usBitsPerSample,
//			m_pstWaveInfo[iCount].ulDataOffset,
//			m_pstWaveInfo[iCount].ulDataSize,
//			(float)m_pstWaveInfo[iCount].ulDuration/1000.0f);
//		#endif
//	}
//
//	return true;
//}
//
///***************************************************************************************************
//*
//*	FUNCTION		CWaveBank::FindWaveFileP
//*
//*	DESCRIPTION		
//*
//***************************************************************************************************/
//
//CWaveFile* CWaveBank::FindWaveFileP (const u_int uiWaveFile)
//{
//	for(ntstd::List<CWaveFile*>::iterator obWaveFileIt=m_obWaveFileList.begin(); obWaveFileIt!=m_obWaveFileList.end(); ++obWaveFileIt)
//	{
//		if ((*obWaveFileIt)->GetID()==uiWaveFile)
//			return (*obWaveFileIt);
//	}
//	
//	return 0; // Failed to find wavefile
//}
//
///***************************************************************************************************
//*
//*	FUNCTION		CWaveBank::FindWaveFileP
//*
//*	DESCRIPTION		
//*
//***************************************************************************************************/
//
//CWaveFile* CWaveBank::FindWaveFileP (const CHashedString& obWaveFile)
//{
//	for(ntstd::List<CWaveFile*>::iterator obWaveFileIt=m_obWaveFileList.begin(); obWaveFileIt!=m_obWaveFileList.end(); ++obWaveFileIt)
//	{
//		if ((*obWaveFileIt)->GetID()==obWaveFile.GetHash())
//			return (*obWaveFileIt);
//	}
//
//	return 0; // Failed to find wavefile
//}
//
///***************************************************************************************************
//*
//*	FUNCTION		CWaveBank::GetWaveData
//*
//*	DESCRIPTION		
//*
//***************************************************************************************************/
//
//void CWaveBank::GetWaveData (WAVEINFO* pstWaveInfo,void* pvDestination,u_long ulOffset,u_long ulSize)
//{
//	ntAssert(pstWaveInfo);
//	ntAssert(pvDestination);
//
//	if (m_pvData) // We are streaming this data from main memory
//	{
//		const u_long ulPosition=pstWaveInfo->ulDataOffset-m_ulInfoChunkSize; // Since data chunk is in memory, we disregard the info chunk
//		
//		NT_MEMCPY(pvDestination,(uint8_t*)m_pvData+ulPosition+ulOffset,ulSize);
//	}
//	else // We are streaming this data from disk
//	{
//		u_long ulBytesRead;
//
//		SetFilePointer(m_hFileHandle,pstWaveInfo->ulDataOffset+ulOffset,0,FILE_BEGIN);
//
//		ReadFile(m_hFileHandle,pvDestination,ulSize,&ulBytesRead,NULL);
//	}
//}
//
///***************************************************************************************************
//*
//*	FUNCTION		CWaveBank::GetMarkerData
//*
//*	DESCRIPTION		
//*
//***************************************************************************************************/
//
//void CWaveBank::GetMarkerData (WAVEINFO* pstWaveInfo,MARKER* pstMarkerList)
//{
//	ntAssert(pstWaveInfo);
//	ntAssert(pstMarkerList);
//	ntAssert(pstWaveInfo->ulMarkerSize>0);
//
//	// Copy marker information to address
//
//	if (m_pvData)
//	{
//		u_long ulOffset=(pstWaveInfo->ulMarkerOffset-m_ulInfoChunkSize);
//
//		NT_MEMCPY(pstMarkerList,(uint8_t*)m_pvData+ulOffset,pstWaveInfo->ulMarkerSize);
//	}
//	else
//	{
//		u_long ulBytesRead;
//
//		SetFilePointer(m_hFileHandle,pstWaveInfo->ulMarkerOffset,0,FILE_BEGIN);
//
//		ReadFile(m_hFileHandle,pstMarkerList,pstWaveInfo->ulMarkerSize,&ulBytesRead,NULL);
//	}
//}
//
//
//
//
//






//
//
//
///***************************************************************************************************
//*
//*	FUNCTION		CWaveFile::CWaveFile
//*
//*	DESCRIPTION		
//*
//***************************************************************************************************/
//
//CWaveFile::CWaveFile (CWaveBank* pobParent,WAVEINFO* pstWaveInfo) :
//	m_pobParentWaveBank(pobParent),
//	m_pstWaveInfo(pstWaveInfo),
//	m_pvCache(0),
//	m_pstMarkerList(0),
//	m_uiMarkerCount(0)
//{
//	ntAssert(pobParent);
//	ntAssert(pstWaveInfo);
//
//	// Setup the WAVEFORMATEX structure
//	switch(pstWaveInfo->usFormatTag)
//	{
//		case WAVE_FORMAT_PCM :
//		{
//			m_stFormat.wFormatTag=pstWaveInfo->usFormatTag;
//			m_stFormat.wBitsPerSample=pstWaveInfo->usBitsPerSample;
//			m_stFormat.nChannels=pstWaveInfo->usChannels;
//			m_stFormat.nBlockAlign=pstWaveInfo->usChannels * pstWaveInfo->usBitsPerSample/8;
//			m_stFormat.nSamplesPerSec=pstWaveInfo->ulSamplesPerSec;
//			m_stFormat.nAvgBytesPerSec=pstWaveInfo->ulBytePerSec;
//			m_stFormat.cbSize=0;
//			break;
//		}
//
////		case WAVE_FORMAT_XBOX_ADPCM :
////		{
////			m_stFormat.wFormatTag=pstWaveInfo->usFormatTag;
////			m_stFormat.wBitsPerSample=pstWaveInfo->usBitsPerSample;
////			m_stFormat.nChannels=pstWaveInfo->usChannels;
////			m_stFormat.nBlockAlign=36 * pstWaveInfo->usChannels;
////			m_stFormat.nSamplesPerSec=pstWaveInfo->ulSamplesPerSec;
////			m_stFormat.nAvgBytesPerSec=pstWaveInfo->ulBytePerSec;
////			m_stFormat.cbSize=0;
////			break;
////		}
//
//		default :
//		{
//			ZeroMemory(&m_stFormat,sizeof(WAVEFORMATEX));
//
//			break;
//		}
//	}
//
//	// Get marker data
//
//	if (m_pstWaveInfo->ulMarkerSize>0)
//	{
//		m_uiMarkerCount=m_pstWaveInfo->ulMarkerSize/MARKER_SIZE;
//
//		m_pstMarkerList=NT_NEW MARKER [m_uiMarkerCount];
//
//		m_pobParentWaveBank->GetMarkerData(m_pstWaveInfo,m_pstMarkerList);
//	}
//
//	// Calculate the buffersize
//
//	m_ulBufferSize=CalculateBufferSize(pobParent->m_eType,&m_stFormat,pobParent->m_iMillisecBuffer);
//
//	//ntPrintf("CWaveFile: Allocating %d bytes for buffer\n",m_ulBufferSize);
//
//	// Prime the wave (if applicable)
//
//	if (pobParent->m_bZeroLatency && pobParent->m_eType!=WAVE_BANK_RAM) // This wave isn't stored in sound memory, therefore it is streamed
//	{
//		// Cache the first packet
//
//		const u_long ulPacketSize=m_ulBufferSize>>1;
//
//		m_pvCache=NT_NEW uint8_t [ulPacketSize];
//
//		GetWaveData(m_pvCache,0,ulPacketSize);
//	}
//}
//
///***************************************************************************************************
//*
//*	FUNCTION		CWaveFile::~CWaveFile
//*
//*	DESCRIPTION		
//*
//***************************************************************************************************/
//
//CWaveFile::~CWaveFile ()
//{
//	NT_DELETE_ARRAY( m_pvCache );
//
//	NT_DELETE_ARRAY( m_pstMarkerList );
//}
//
///***************************************************************************************************
//*
//*	FUNCTION		CWaveFile::GetCachedData
//*
//*	DESCRIPTION		
//*
//***************************************************************************************************/
//
//bool CWaveFile::GetCachedData (void* pvDestination)
//{
//	ntAssert(pvDestination);
//
//	if (m_pvCache)
//	{
//		const u_long ulPacketSize=m_ulBufferSize>>1;
//
//		if (GetDataSize()<ulPacketSize) // Wave file is actually smaller than a packet
//		{
//			const u_char ucSilence=(m_stFormat.wBitsPerSample==8 ? 128 : 0);
//
//			NT_MEMCPY(pvDestination,m_pvCache,GetDataSize()); // Copy the first packet to destination address
//
//			memset((u_char*)pvDestination+GetDataSize(),ucSilence,ulPacketSize-GetDataSize()); // Fill remainder with silence
//		}
//		else
//		{
//			NT_MEMCPY(pvDestination,m_pvCache,ulPacketSize);
//		}
//
//		return true;
//	}
//
//	return false;
//}
//
///***************************************************************************************************
//*
//*	FUNCTION		CWaveFile::GetWaveData
//*
//*	DESCRIPTION		
//*
//***************************************************************************************************/
//
//void CWaveFile::GetWaveData (void* pvDestination,u_long ulOffset,u_long ulSize)
//{
//	ntAssert(pvDestination);
//	ntAssert(m_pobParentWaveBank);
//
//	m_pobParentWaveBank->GetWaveData(m_pstWaveInfo,pvDestination,ulOffset,ulSize);
//}
//
///***************************************************************************************************
//*
//*	FUNCTION		CWaveFile::GetNextMarker
//*
//*	DESCRIPTION		Return the offset of the next marker relative to a specified offset.	
//*
//***************************************************************************************************/
//
//void CWaveFile::GetNextMarker(u_long ulCurrentPosition,u_long &ulOffset,u_int &uiIndex)
//{
//	if (m_uiMarkerCount==0)
//	{
//		ulOffset=GetDataSize();
//		uiIndex=0;
//	}
//	else
//	{
//		ntAssert(m_pstMarkerList);
//
//		for(u_int uiCount=0; uiCount<m_uiMarkerCount; ++uiCount)
//		{
//			if (ulCurrentPosition<m_pstMarkerList[uiCount].ulOffset)
//			{
//				ulOffset=m_pstMarkerList[uiCount].ulOffset;
//				uiIndex=uiCount;
//				return;
//			}
//		}
//
//		ulOffset=GetDataSize();
//		uiIndex=m_uiMarkerCount+1;
//	}
//}
//
///***************************************************************************************************
//*
//*	FUNCTION		CWaveFile::GetMarkerOffset
//*
//*	DESCRIPTION		Get the offset for a particular marker.
//*
//***************************************************************************************************/
//
//u_long CWaveFile::GetMarkerOffset (u_int uiIndex)
//{
//	ntAssert(m_pstMarkerList);
//	
//	if (uiIndex<m_uiMarkerCount)
//		return m_pstMarkerList[uiIndex].ulOffset;
//
//	return GetDataSize();
//}
//
//
//
//
//
//






/***************************************************************************************************
*
*	FUNCTION		CAudioResourceManager::CAudioResourceManager
*
*	DESCRIPTION		
*
***************************************************************************************************/

CAudioResourceManager::CAudioResourceManager ()
{
}

/***************************************************************************************************
*
*	FUNCTION		CAudioResourceManager::~CAudioResourceManager
*
*	DESCRIPTION		
*
***************************************************************************************************/

CAudioResourceManager::~CAudioResourceManager ()
{
	Release();
}

/***************************************************************************************************
*
*	FUNCTION		CAudioResourceManager::RegisterWaveBank
*
*	DESCRIPTION		Register a wavebank, if the wavebank is already loaded, do nothing.
*
***************************************************************************************************/

bool CAudioResourceManager::RegisterWaveBank (const char* pcFriendlyName,const char* pcPath,WAVE_BANK_TYPE eType,int iMillisec,bool bPrime)
{
	CWaveBank* pobWaveBank=NT_NEW CWaveBank();

	ntAssert(pobWaveBank);

	if (pobWaveBank->Open(pcFriendlyName,pcPath,eType,iMillisec,bPrime))
	{
		m_obWaveBankList.push_back(pobWaveBank);

		return true;
	}

#ifdef _SHOW_WARNING_MESSAGES
	ntPrintf("CAudioResourceManager: Failed to register wavebank %s\n",pcPath);
#endif // _SHOW_WARNING_MESSAGES

	NT_DELETE( pobWaveBank );

	return false;
}

/***************************************************************************************************
*
*	FUNCTION		CAudioResourceManager::ReleaseWaveBank
*
*	DESCRIPTION		
*
***************************************************************************************************/

bool CAudioResourceManager::ReleaseWaveBank (const char* pcFriendlyName) // Release a wavebank, if its loaded
{
	CHashedString obHash(pcFriendlyName);

	for(ntstd::List<CWaveBank*>::iterator obWaveBankIt=m_obWaveBankList.begin(); obWaveBankIt!=m_obWaveBankList.end(); ++obWaveBankIt)
	{
		if ((*obWaveBankIt)->m_obID==obHash)
		{
			NT_DELETE( *obWaveBankIt );
			obWaveBankIt=m_obWaveBankList.erase(obWaveBankIt);

			return true;
		}
	}

	return false;
}

/***************************************************************************************************
*
*	FUNCTION		CAudioResourceManager::Release
*
*	DESCRIPTION		
*
***************************************************************************************************/

void CAudioResourceManager::Release ()
{
	while(m_obWaveBankList.size()>0)
	{
		NT_DELETE( m_obWaveBankList.back() );
		m_obWaveBankList.pop_back();
	}
}

/***************************************************************************************************
*
*	FUNCTION		CAudioResourceManager::FindWaveFileP
*
*	DESCRIPTION		
*
***************************************************************************************************/

CWaveFile* CAudioResourceManager::FindWaveFileP (const u_int uiWaveBank,const u_int uiWaveFile)
{
	for(ntstd::List<CWaveBank*>::iterator obWaveBankIt=m_obWaveBankList.begin(); obWaveBankIt!=m_obWaveBankList.end(); ++obWaveBankIt)
	{
		if ((*obWaveBankIt)->GetID()==uiWaveBank)
		{
			return (*obWaveBankIt)->FindWaveFileP(uiWaveFile);
		}
	}

#ifdef _SHOW_WARNING_MESSAGES
	ntPrintf("CAudioResourceManager: Unable to find wavefile #%d in wavebank #%d\n",uiWaveFile,uiWaveBank);
#endif // _SHOW_WARNING_MESSAGES

	return 0; // Unable to find this wave file
}

/***************************************************************************************************
*
*	FUNCTION		CAudioResourceManager::FindWaveFileP
*
*	DESCRIPTION		
*
***************************************************************************************************/

CWaveFile* CAudioResourceManager::FindWaveFileP (const CHashedString& obWaveBank,const CHashedString& obWaveFile)
{
	for(ntstd::List<CWaveBank*>::iterator obWaveBankIt=m_obWaveBankList.begin(); obWaveBankIt!=m_obWaveBankList.end(); ++obWaveBankIt)
	{
		if ((*obWaveBankIt)->GetID()==obWaveBank.GetHash())
		{
			return (*obWaveBankIt)->FindWaveFileP(obWaveFile.GetHash());
		}
	}

#ifdef _SHOW_WARNING_MESSAGES
	ntPrintf("CAudioResourceManager: Unable to find wavefile %s in wavebank %s\n", ntStr::GetString(obWaveFile), ntStr::GetString(obWaveBank));
#endif // _SHOW_WARNING_MESSAGES

	return 0; // Unable to find this wave file
}

