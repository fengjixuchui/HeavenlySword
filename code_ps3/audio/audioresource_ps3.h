/***************************************************************************************************
*
*	$Header:: /game/audioresource_pc.h 5     1/08/03 10:24 Harvey                                  $
*
*	Header file for CAudioResourceManager and related classes.
*
*	CHANGES
*
*	20/06/2003	Harvey	Created
*
***************************************************************************************************/

#ifndef AUDIORESOURCE_PS3_H
#define AUDIORESOURCE_PS3_H





// Forward declarations

class CWaveFile;








enum WAVE_BANK_TYPE
{
	WAVE_BANK_UNKNOWN,
	WAVE_BANK_SRAM,		// Wavebank is stored directly in sound memory (not currently supported)
	WAVE_BANK_RAM,		// Streamed from main memory to sound memory
	WAVE_BANK_DVD,		// Streamed from DVD to sound memory
	WAVE_BANK_HD,		// Streamed from HD to sound memory
};



struct WAVEBANKHEADER
{
	unsigned short usWaveCount;			// Number of wave files in this bank
};

struct WAVEINFO
{
	CHashedString obHash;				// Hash ID 
	unsigned short usFormatTag;			// Format ID
	unsigned short usChannels;			// Channels
	unsigned long ulSamplesPerSec;		// Sample rate
	unsigned long ulBytePerSec;			// Byte rate
	unsigned short usBitsPerSample;		// Bits per sample
	unsigned long ulDuration;			// Duration of wave file in milliseconds
	unsigned long ulDataOffset;			// Offset into file where wave data begins
	unsigned long ulDataSize;			// Size of data chunk in bytes
	unsigned long ulMarkerOffset;		// Offset into file where marker data begins
	unsigned long ulMarkerSize;			// Size of marker data in bytes
	unsigned long ulLipSyncOffset;		// Offset into file where lipsync data begins
	unsigned long ulLipSyncSize;		// Size of lipsync data in bytes
};

struct MARKER
{
	unsigned long ulOffset;				// Offset into data where this marker is located
};

static const unsigned long HEADER_SIZE = sizeof(WAVEBANKHEADER);
static const unsigned long INFO_SIZE = sizeof(WAVEINFO);
static const unsigned long MARKER_SIZE = sizeof(MARKER);







/***************************************************************************************************
*
*	CLASS			CWaveBank
*
*	DESCRIPTION		
*
***************************************************************************************************/

class CWaveBank
{
public:
//
//	CWaveBank ();
//
//	~CWaveBank ();

	bool Open (const char*,const char*,WAVE_BANK_TYPE,int,bool) { return false; }

	CWaveFile* FindWaveFileP (const u_int) { return 0; }

	CWaveFile* FindWaveFileP (const CHashedString&) { return 0; }

	void GetWaveData (WAVEINFO*,void*,u_long,u_long) {}

	void GetMarkerData (WAVEINFO*,MARKER*) {}

	u_int GetID () const { return m_obID.GetValue(); }

	CHashedString		m_obID; // ID of this wavebank
	
	WAVE_BANK_TYPE		m_eType; // Type of wave bank

	WAVEBANKHEADER		m_stHeader; // Wavebank header

	WAVEINFO*			m_pstWaveInfo; // Array of waveinfo structures

	ntstd::List<CWaveFile*>	m_obWaveFileList; // Individual wave files

//	HANDLE				m_hFileHandle; // Handle to file (if applicable)

	bool				m_bZeroLatency;
	int					m_iMillisecBuffer; // Size of the cached data in milliseconds (if this is 0, then there is no zero latency streaming)

	void*				m_pvData; // Wave data

	u_long				m_ulFileSize;
	u_long				m_ulInfoChunkSize;
	u_long				m_ulDataChunkSize;
};









/***************************************************************************************************
*
*	CLASS			CWaveFile
*
*	DESCRIPTION		
*
***************************************************************************************************/

class CWaveFile
{
public:

//	CWaveFile (CWaveBank* pobParent,WAVEINFO* pstWaveInfo);
//
//	~CWaveFile ();
//
	bool GetCachedData (void* pvDestination) { return false; }

	void GetWaveData (void* pvDestination,u_long ulOffset,u_long ulSize) {}

	u_int GetID () const { return m_pstWaveInfo->obHash.GetValue(); }

//	WAVEFORMATEX* GetWaveFormatP () { return &m_stFormat; }
	WAVE_BANK_TYPE GetWaveBankType () const { return m_pobParentWaveBank->m_eType; }

	u_long GetBufferSize () const { return m_ulBufferSize; }
	u_long GetDuration () const { return m_pstWaveInfo->ulDuration; }
	u_long GetDataSize () const { return m_pstWaveInfo->ulDataSize; }
	u_long GetBytesPerSec () const { return m_pstWaveInfo->ulBytePerSec; }
	
	u_int GetMarkerCount () const { return m_uiMarkerCount; }
	u_long GetMarkerOffset (u_int) { return 0; }
	void GetNextMarker (u_long,u_long &,u_int &) {} 

private:

	CWaveBank*		m_pobParentWaveBank;

//	WAVEFORMATEX	m_stFormat;

	WAVEINFO*		m_pstWaveInfo;

	MARKER*			m_pstMarkerList;

	void*			m_pvCache;

	u_long			m_ulBufferSize;

	u_int			m_uiMarkerCount;
};








/***************************************************************************************************
*
*	CLASS			CAudioResourceManager
*
*	DESCRIPTION		
*
***************************************************************************************************/

class CAudioResourceManager : public Singleton<CAudioResourceManager> // Audio data lookup and storage
{
public:

	CAudioResourceManager ();

	~CAudioResourceManager ();

	void Release ();

    bool RegisterWaveBank (const char* pcFriendlyName,const char* pcPath,WAVE_BANK_TYPE eType,int iMillisec,bool bPrime);

	bool ReleaseWaveBank (const char* pcFriendlyName);

	CWaveFile* FindWaveFileP (const u_int uiWaveBank,const u_int uiWaveFile);

	CWaveFile* FindWaveFileP (const CHashedString& obWaveBank,const CHashedString& obWaveFile);

	int GetWaveBankCount () { return m_obWaveBankList.size(); }

private:

	ntstd::List<CWaveBank*> m_obWaveBankList; // Wave banks
};










#endif // AUDIORESOURCE_PS3_H
