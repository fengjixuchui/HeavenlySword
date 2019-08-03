//------------------------------------------------------------------------------------------
//!
//!	\file RandManager.cpp
//!
//------------------------------------------------------------------------------------------

//------------------------
// Includes
//------------------------

#include "game/randmanager.h"


//---------------------------
// Constants
//---------------------------


//---------------------------
// Statics
//---------------------------

// Seeds for the different systems
u_int	RandManager::m_guiDSeed;
u_int	RandManager::m_guiGSeed;
u_int	RandManager::m_guiESeed;

RandCapture RandManager::m_gobUsage;

#ifdef ENABLE_RAND_USAGE_COUNT
ntstd::Map<ntstd::String, int> *RandManager::m_gpobFileAndLineUsage;
#endif

//---------------------------
// Functions
//---------------------------



//------------------------------------------------------------------------------------------
//!
//!	RandManager::RandManager
//!	Construction - Default
//!
//------------------------------------------------------------------------------------------
RandManager::RandManager()
{
#ifdef ENABLE_RAND_USAGE_COUNT
	// allocate a new map
	m_gpobFileAndLineUsage = NT_NEW ntstd::Map<ntstd::String, int>;
#endif
}


//------------------------------------------------------------------------------------------
//!
//!	RandManager::~RandManager
//!	Destruction
//!
//------------------------------------------------------------------------------------------
RandManager::~RandManager()
{
#ifdef ENABLE_RAND_USAGE_COUNT

	// Empty the map
	m_gpobFileAndLineUsage->clear();

	// Delete it
	NT_DELETE( m_gpobFileAndLineUsage );
#endif
}

// Place breakpoints below and set the frame number to match the frame you want to examine rand usage on
#define RAND_FRAME_STOP -2L


// Get a debug rand
u_int RandManager::DRand(char* pcFile, int iLine)
{
	UNUSED(pcFile);
	UNUSED(iLine);

	#ifdef ENABLE_RAND_USAGE_COUNT
	m_gobUsage.m_uiDUsageCount++;
	#endif
	return RndGenerator(m_guiDSeed);
}

// Get a game rand
u_int RandManager::GRand(char* pcFile, int iLine)
{
	UNUSED(pcFile);
	UNUSED(iLine);

	#ifdef ENABLE_RAND_USAGE_COUNT
	m_gobUsage.m_uiGUsageCount++;
	#endif
	return RndGenerator(m_guiGSeed);
}

// Get an effect rand
u_int RandManager::ERand(char* pcFile, int iLine)
{
	UNUSED(pcFile);
	UNUSED(iLine);

	#ifdef ENABLE_RAND_USAGE_COUNT
	m_gobUsage.m_uiEUsageCount++;
	#endif
	return RndGenerator(m_guiESeed);
}


#ifdef ENABLE_RAND_USAGE_COUNT

void RandManager::DebugTrackFileAndLine(char* pcFile, int iLine)
{
	// Build name and line string
	char cBuffer[256];
	snprintf(cBuffer, 256, "%s:%d", pcFile, iLine);
	ntstd::String obCombined = cBuffer;

	ntstd::Map<ntstd::String, int>::iterator itFind = m_gpobFileAndLineUsage->find(obCombined);
	if (itFind != m_gpobFileAndLineUsage->end())
	{
		// Increase the usage count
		(*itFind).second++;
	}
	else
	{
		// Insert a new entry
		m_gpobFileAndLineUsage->insert(ntstd::Map<ntstd::String, int>::value_type(obCombined, 1));
	}
}

void RandManager::DumpTrackFileAndLine()
{
	//
	ntstd::Map<ntstd::String, int>::iterator obIt = m_gpobFileAndLineUsage->begin();

	ntPrintf("--- Start: Dumping Rand usage on failed frame ---");

	while (obIt != m_gpobFileAndLineUsage->end())
	{
		// ntPrintf out the file and line details
		ntPrintf("%s x %d\n", obIt->first.data(), obIt->second);

		// Move to next
		obIt++;
	}

	ntPrintf("--- End: Dumping Rand usage on failed frame ---");
}


#endif

