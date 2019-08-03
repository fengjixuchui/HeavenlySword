/***************************************************************************************************
*
*   $Header:: /game/gatso.h 5     15/08/03 13:06 Giles                                             $
*
*	Header file for CGatso, a timing class for routines & rendering performance
*
*	CHANGES		
*
*	22/02/2001	dean	Created
*
***************************************************************************************************/

#ifndef _GATSO_H
#define _GATSO_H

class	CRendererStateCache;
class	CGraph;
class	CGraphSampleSet;
struct DebugHudItem;

#ifndef	_GATSO

// This handy stub class means we don't need to conditionalise calls to CGatso member functions,
// making for neater code..

class CGatso
{
	friend class Renderer;

public:

	static	void	Initialise( void ) {};
	static	void	Shutdown( void ) {};
	static	void	Start( const char* ) {};
	static	void	Stop( const char* ) {};
	static	float	Retrieve( const char* ) { return 0.0f; };
	static	void	ShowVar( const char*, float ) {};
	static	void	ShowVar( const char*, int ) {};
	static	void	ShowVar( const char*, uint32_t ) {};
	static	void	ShowVar( const char*, const char* ) {};
	static	void	ShowHex( const char*, int ) {};
	static	void	ShowHex( const char*, short ) {};
	static	void	ShowVarAcc( const char*, int ) {};

	static	void	DoGUI(){};
	
	static	void	Dump( void )	{};
	static	void	Clear( void )	{};
	static	void	Update( void )	{};

    struct GATSO_INSTANCE
    {
    };
};

#endif // _GATSO

//--------------------------------------------------------------------------------------------------

#ifdef	_GATSO


class CGatso
{
	friend class Renderer;

public:
	static bool					m_gbDisableGatso;

	// Initialisation & shutdown
	static	void	Initialise( void );


#ifdef	_GATSO_SUMMARY_ONLY
	static	void	Start( const char* pcTag ) {};
	static	void	Stop( const char* pcTag ) {};
	static	float	Retrieve( const char* pcTag ) {};
	static	void	ShowVar( const char* pcTag, float fVal ) {};
	static	void	ShowVar( const char* pcTag, int iVal ) {};
	static	void	ShowVar( const char* pcTag, uint32_t uiVal ) {};
	static	void	ShowVar( const char* pcTag, const char* pcString ) {};
	static	void	ShowHex( const char* pcTag, int iVal ) {};
	static	void	ShowHex( const char* pcTag, short sVal ) {};
	static	void	ShowVarAcc( const char* pcTag, int iVal ) {};
#else

	// Toggle graph rendering state, returning new state..
	static	bool	ToggleScreenRender( void )		{ m_gbRenderToScreen = !m_gbRenderToScreen; return m_gbRenderToScreen; }
	static	bool	ToggleSortDump( void )			{ m_gbSortDump = !m_gbSortDump; return m_gbSortDump; }
	static	bool	ToggleMaxReporting( void )		{ m_gbMaxReport = !m_gbMaxReport; return m_gbMaxReport; }

	// Request a dump of information to the console
	
	static	void	Dump( void )		{ m_gbRequestConsoleDump = true; };

	// Start timing a function...

	static	void	Start( const char* pcTag );

	// Stop timing a function...

	static	void	Stop( const char* pcTag );

	// retrieve timing for a function...

	static	float	Retrieve( const char* pcTag );

	// Enable display of variable contents...

	static	void	ShowVar( const char* pcTag, float fVal )
	{
		AddVarInstance( pcTag, VAR_FLOAT, *( reinterpret_cast<int*>( &fVal ) ) );
	}	

	static	void	ShowVar( const char* pcTag, int iVal )
	{
		AddVarInstance( pcTag, VAR_INT, iVal );
	}	

	static	void	ShowVar( const char* pcTag, uint32_t uiVal )
	{
		AddVarInstance( pcTag, VAR_UINT, uiVal );
	}	

	static	void	ShowVar( const char* pcTag, const char* pcString )
	{
		AddVarInstance( pcTag, VAR_STRING, reinterpret_cast<intptr_t>( pcString ) );		
	}

	static	void	ShowHex( const char* pcTag, int iVal )
	{
		AddVarInstance( pcTag, VAR_HEX_INT, static_cast<u_int>( iVal ) );
	}	

	static	void	ShowHex( const char* pcTag, short sVal )
	{
		AddVarInstance( pcTag, VAR_HEX_SHORT, static_cast<u_short>( sVal ) );		
	}

	// Enable display of an accumulating integer value...

	static	void	ShowVarAcc( const char* pcTag, int iVal )
	{
		AddVarInstanceAcc( pcTag, VAR_INT, iVal );
	}

#endif	//_GATSO_SUMMARY_ONLY

	// Update gatso results...
	static	void	Update( void );

	// Clear all gatso records...
	static	void	Clear( void );

	static void ResetGatsoHistory();

	static const int NUM_SECONDS = 30;
	static const int SAMPLES_PER_SECOND = 30;
	static const int TOTAL_SAMPLES = NUM_SECONDS * SAMPLES_PER_SECOND;

	struct GATSO_INSTANCE
	{
		const char* m_pcTag;
		uint64_t	m_lAccumTime;	
		uint64_t	m_lLastStart;
		int  		m_iAccumCalls;
		bool		m_bActive;
		float		GetInstantPercent()
		{
			return GetSample( m_iCurrentIndex-1 );
		};

		float		GetFilteredPercent()
		{
			return GetSecondAverage( m_iCurrentIndex-1 );
		}

		float		GetSample( int index )
		{
			if( index < 0 )
				return m_fHistory[TOTAL_SAMPLES + index];
			return m_fHistory[ index % TOTAL_SAMPLES ];
		}

		float GetSecondAverage( int index )
		{
			int startIndex = index - (SAMPLES_PER_SECOND-1);
			float avg = 0.f;
			for( int i =0;i < SAMPLES_PER_SECOND;i++)
			{
				avg += GetSample( startIndex + i );
			}
			return avg / SAMPLES_PER_SECOND;
		}

		int			m_iCurrentIndex;
		float		m_fMaxPercent;
	
		GATSO_INSTANCE* m_pstNext;
		float		m_fHistory[TOTAL_SAMPLES];
	};

	// iterate across all the gatso (for the GUI);
	static GATSO_INSTANCE* Begin();
	static GATSO_INSTANCE* End();
	static GATSO_INSTANCE* Next( GATSO_INSTANCE* pCurr );

private:

	static	const	uint32_t	uiGATSO_TICK_MASK		= 0;
	static	const	int		iGATSO_PRINT_DELAY		= 60;

	static	const	int		iGATSO_MAX_STRING_LEN	= 128;

	// NOTE: Gatso only refreshes on frames where 'm_iTick & GATSO_TICK_MASK == 0'...

	static	const	uint32_t uiGATSO_HASH_LIST_SIZE	= 128;
	static	const	uint32_t uiGATSO_HASH_LIST_AND		= uiGATSO_HASH_LIST_SIZE - 1;
	static	const	uint32_t uiGATSO_NUM_POOL_ELEMENTS	= 256;
		
	enum VAR_TYPE
	{
		VAR_FLOAT = -10,
		VAR_INT,
		VAR_UINT,
		VAR_HEX_SHORT,
		VAR_HEX_INT,
		VAR_STRING,
	};

	static	GATSO_INSTANCE* 	m_gapstHashList[ uiGATSO_HASH_LIST_SIZE ];
	static	GATSO_INSTANCE  	m_gastInstancePool[ uiGATSO_NUM_POOL_ELEMENTS ];
	static	uint64_t			m_glCPUStartTime;
	static	uint32_t	    	m_guiInstanceIndex;	

	static	bool				m_gbRequestConsoleDump;
	static	bool				m_gbRenderToScreen;
	static	bool				m_gbSortDump;
	static	bool				m_gbMaxReport;

	static	int					m_giTick;
	static	int					m_giLastPrintTick;


	// -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - 

	// Functions to return/add specific gatso instances using names...

	static	CGatso::GATSO_INSTANCE*	FindInstance( const char* pcTag );
	static	CGatso::GATSO_INSTANCE*	AddInstance( const char* pcTag );
	static	void					AddVarInstance( const char* pcTag, enum VAR_TYPE eType, intptr_t iVal );
	static	void					AddVarInstanceAcc( const char* pcTag, enum VAR_TYPE eType, intptr_t iVal );
	

	// Hash key generator...

	static	uint32_t	GetHashIndex( const char* pcTag )
	{
		unsigned int uiTag = 0;

		while( *pcTag )
		{
			uiTag = ( uiTag << 7 ) + uiTag;
			uiTag = uiTag + *pcTag++;
		}
		return( uiTag & uiGATSO_HASH_LIST_AND );
	}


};

/***************************************************************************************************
*	
*	FUNCTION		CGatso::FindInstance
*
*	DESCRIPTION		Attempt to locate a timing record for the given operation.
*
*	INPUTS			pcTag		- Tag of operation being timed.
*
***************************************************************************************************/

inline	CGatso::GATSO_INSTANCE* CGatso::FindInstance( const char* pcTag )
{
	uint32_t uiIndex = GetHashIndex( pcTag );

	GATSO_INSTANCE* pstInstance = m_gapstHashList[ uiIndex ];

	// Find within slot...		

	while ( ( pstInstance != NULL ) &&
			( CStringUtil::StrCmp( pstInstance->m_pcTag, pcTag ) == false ) )
	{
		pstInstance = pstInstance->m_pstNext;
	}

	return( pstInstance );
}	

#endif // _GATSO

#endif // _GATSO_H
