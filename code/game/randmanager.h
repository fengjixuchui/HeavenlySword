//------------------------------------------------------------------------------------------
//!
//!	\file capturemanager.h
//!
//------------------------------------------------------------------------------------------

#ifndef _RAND_MANAGER_H
#define _RAND_MANAGER_H

// Define to enable/disable usage count
#ifndef _RELEASE
#define ENABLE_RAND_USAGE_COUNT 
#endif

class RandManager;

//------------------------------------------------------------------------------------------
//!
//!	RandManager
//!
//! Storage class for the capture of random usage counters
//!	
//------------------------------------------------------------------------------------------

class RandCapture
{
public:
	// Constructor - default
	RandCapture()
	{
		m_uiDUsageCount = 0;
		m_uiDUsageCount = 0;
		m_uiDUsageCount = 0;
	}

	// Usage count for the various systems
	u_int	m_uiDUsageCount;
	u_int	m_uiGUsageCount;
	u_int	m_uiEUsageCount;
};

//------------------------
// Legacy
//------------------------

// RAND_MAX is not coherent across different platforms (PS3 RAND_MAX is 0x3fffffff, PC RAND_MAX is 0x7fff)
#define NT_RAND_MAX 0x7fff


// Non deterministic debug rand
#define drands( s )					( RandManager::DSeed( s ) )
#define dseed()						( RandManager::DSeed() )
#define drand( )					( RandManager::DRand( __FILE__, __LINE__) )
#define drandf( f )					( (f)*RandManager::DRand( __FILE__, __LINE__)/( float )NT_RAND_MAX )

// Deterministic game rand
#define grands( s )					( RandManager::GSeed( s ) )
#define gseed()						( RandManager::GSeed() )
#define grand( )					( RandManager::GRand( __FILE__, __LINE__) )
#define grandf( f )					( (f)*RandManager::GRand( __FILE__, __LINE__)/( float )NT_RAND_MAX )

// Non deterministic effects rand
#define erands( s )					( RandManager::ESeed( s ) )
#define eseed()						( RandManager::ESeed() )
#define erand( )					( RandManager::ERand( __FILE__, __LINE__) )
#define erandf( f )					( (f)*RandManager::ERand( __FILE__, __LINE__)/( float )NT_RAND_MAX )



//------------------------------------------------------------------------------------------
//!
//!	RandManager
//!
//! Manager class to handle all random numbers within the game
//!	
//------------------------------------------------------------------------------------------

class RandManager : public Singleton<RandManager>
{
private:
	// Work function to generate a random number
	// This rand functions is rubbish and not by me... due for an update MB
	static u_int RndGenerator(u_int& uiSeed)
	{
		uiSeed = ( uiSeed * 1103515245 + 12345 );
		return ( uiSeed & NT_RAND_MAX ); 
	}

public:
	// Constructor - default
	RandManager();
	// Destructor - default
	~RandManager();


	// Set the debug seed
	static u_int DSeed( u_int uiSeed)	
	{ 
		m_guiDSeed = uiSeed;
		#ifdef ENABLE_RAND_USAGE_COUNT
		m_gobUsage.m_uiDUsageCount = 0;
		#endif
		return m_guiDSeed;
	};
	// Get the debug seed
	static u_int DSeed() {return m_guiDSeed;}

	// Set the game seed
	static u_int GSeed( u_int uiSeed)	
	{ 
		m_guiGSeed = uiSeed;
		#ifdef ENABLE_RAND_USAGE_COUNT
		m_gobUsage.m_uiGUsageCount = 0;
		#endif
		return m_guiGSeed;
	};
	// Get the game seed
	static u_int GSeed() {return m_guiGSeed;}

	// Set the effect seed
	static u_int ESeed( u_int uiSeed)	
	{ 
		m_guiESeed = uiSeed;
		#ifdef ENABLE_RAND_USAGE_COUNT
		m_gobUsage.m_uiEUsageCount = 0;
		#endif
		return m_guiESeed;
	};
	// Get the effect seed
	static u_int ESeed() {return m_guiESeed;}

	// Get a debug rand
	static u_int DRand(char* pcFile, int iLine);
	
	// Get a game rand
	static u_int GRand(char* pcFile, int iLine);

	// Get an effect rand
	static u_int ERand(char* pcFile, int iLine);

	// Get a pointer to the randusage structure
	static const RandCapture* GetRandUsage()
	{
		return &m_gobUsage; 
	}

private:

	// Seeds for the different systems
	static	u_int	m_guiDSeed;
	static	u_int	m_guiGSeed;
	static	u_int	m_guiESeed;

#ifdef ENABLE_RAND_USAGE_COUNT
public:
	static void DumpTrackFileAndLine();
private:
	static void DebugTrackFileAndLine(char* pcFile, int iLine);
	static ntstd::Map<ntstd::String, int>* m_gpobFileAndLineUsage;
#endif

	// Usage counters
	static RandCapture m_gobUsage;

};


#endif //_RAND_MANAGER_H
