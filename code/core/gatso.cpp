/***************************************************************************************************
*
*   $Header:: /game/gatso.cpp 6     14/08/03 10:48 Dean                                           $
*
*	Implementation for CGatso, a class used for timing routines & rendering
*
*	CHANGES		
*
*	22/02/2001	dean	Created
*
***************************************************************************************************/

#include "core/gatso.h"
#include "core/timer.h"
#include "gfx/graphing.h"
#include "gfx/graphicsdevice.h"
#include "gfx/renderer.h"
#include "core/debug_hud.h"
#include "core/profiling.h"
//#include "core/memreport.h"
#include "core/visualdebugger.h"

#ifdef	_GATSO

CGatso::GATSO_INSTANCE*	CGatso::m_gapstHashList[ uiGATSO_HASH_LIST_SIZE ];
CGatso::GATSO_INSTANCE	CGatso::m_gastInstancePool[ uiGATSO_NUM_POOL_ELEMENTS ];
u_int		 			CGatso::m_guiInstanceIndex;	
uint64_t				CGatso::m_glCPUStartTime;
int						CGatso::m_giTick;
int						CGatso::m_giLastPrintTick;
bool					CGatso::m_gbRequestConsoleDump;
bool					CGatso::m_gbRenderToScreen;
bool					CGatso::m_gbSortDump;
bool					CGatso::m_gbMaxReport;
bool					CGatso::m_gbDisableGatso = false;

/***************************************************************************************************
*	
*	FUNCTION		CGatso::Initialise
*
*	DESCRIPTION		Initialise a gatso for use, constructing graphing objects
*
***************************************************************************************************/

void	CGatso::Initialise( void )
{
	// Initialise internal vars...
	m_giTick 		 		= -1;
	m_giLastPrintTick 		= -1;
	m_gbRequestConsoleDump	= false;
	m_gbRenderToScreen = false;
	m_gbSortDump = false;
	m_gbMaxReport = false;

	// Make sure gatso slots are clear
	Clear();
}	




/***************************************************************************************************
*	
*	FUNCTION		CGatso::Clear
*
*	DESCRIPTION		Clear all gatso records (at start of game frame).
*
***************************************************************************************************/

void	CGatso::Clear( void )
{
	// Reset CPU cycle timer (we use performance counter 0 for this)
	m_glCPUStartTime = CTimer::GetHWTimer();

	// Start clear of gatso records
	m_giTick++;

	if ( !( m_giTick & uiGATSO_TICK_MASK ) )
	{
		// Forget about all previously active instances...
//		m_guiInstanceIndex = 0;
//		memset( m_gapstHashList, 0, sizeof( GATSO_INSTANCE* ) * uiGATSO_HASH_LIST_SIZE );
		for( u_int i = 0;i < uiGATSO_HASH_LIST_SIZE;i++)
		{
			GATSO_INSTANCE* pstInstance = m_gapstHashList[ i ];

			while ( pstInstance != 0 ) 
			{
				pstInstance->m_lAccumTime = 0;
				pstInstance->m_lLastStart = 0;
				pstInstance->m_iAccumCalls = 0;
				pstInstance->m_bActive = false;
				pstInstance = pstInstance->m_pstNext;
			}
		}
	}
}	


/***************************************************************************************************
*	
*	FUNCTION		CGatso::Start
*
*	DESCRIPTION		Start gatso timing for given functionality.
*
*	INPUTS			pcTag		- Text tag describing operation being timed.
*
***************************************************************************************************/

#ifndef	_GATSO_SUMMARY_ONLY
void	CGatso::Start( const char* pcTag )
{
	if( CGatso::m_gbDisableGatso == true )
		return;

	if ( !( m_giTick & uiGATSO_TICK_MASK ) )
	{
		GATSO_INSTANCE* pstInstance;

		if ( ( pstInstance = FindInstance( pcTag ) ) == NULL )
		{
			// Instance not previously registered...
			pstInstance = AddInstance( pcTag );
		}

		#ifndef _GATSO_NO_SAFETY_CHECKS

			// Make sure not trying to start an already active instance...
			ntAssert( !pstInstance->m_bActive );		

			// Make sure not trying to mess with a var display instance...
			ntAssert( pstInstance->m_iAccumCalls >= 0 );		

		#endif	//_GATSO_NO_SAFETY_CHECKS

		// Increase use count for instance...
		pstInstance->m_iAccumCalls++;

		// Log timer count...
		pstInstance->m_lLastStart = CTimer::GetHWTimer();

		// Mark this instance as active
		pstInstance->m_bActive = true;
	}
}	
#endif	//_GATSO_SUMMARY_ONLY


/***************************************************************************************************
*	
*	FUNCTION		CGatso::Stop
*
*	DESCRIPTION		Stop gatso timing for given functionality.
*
*	INPUTS			pcTag		- Text tag for timing being concluded (must match Start() call!)
*
***************************************************************************************************/

#ifndef	_GATSO_SUMMARY_ONLY
void	CGatso::Stop( const char* pcTag )
{
	if( CGatso::m_gbDisableGatso == true )
		return;

	if ( !( m_giTick & uiGATSO_TICK_MASK ) )
	{
		GATSO_INSTANCE* pstInstance = FindInstance( pcTag );

		ntError( pstInstance != NULL );					// Make sure not trying to stop something that was never started...

		#ifndef _GATSO_NO_SAFETY_CHECKS
			ntAssert( pstInstance->m_bActive );			// Make sure not trying to stop an inactive instance...
			ntAssert( pstInstance->m_iAccumCalls >= 0 );	// Make sure not trying to mess with a var display instance...
		#endif	//_GATSO_NO_SAFETY_CHECKS

		int64_t lElapsed;
		lElapsed = CTimer::GetHWTimer() - pstInstance->m_lLastStart;

		// Add elapsed time to accumulating total...
		pstInstance->m_lAccumTime += lElapsed;

		// Flag that instance is 'inactive'...
		pstInstance->m_bActive = false;
	}
}	
#endif	//_GATSO_SUMMARY_ONLY


/***************************************************************************************************
*	
*	FUNCTION		CGatso::float
*
*	DESCRIPTION		Find and return the frame time value for this tag
*
*	INPUTS			pcTag		- Text tag for timing being concluded (must match Start() call!)
*
***************************************************************************************************/

#ifndef	_GATSO_SUMMARY_ONLY
float	CGatso::Retrieve( const char* pcTag )
{
	if( CGatso::m_gbDisableGatso == true )
		return 0.f;

	if ( !( m_giTick & uiGATSO_TICK_MASK ) )
	{
		GATSO_INSTANCE* pstInstance = FindInstance( pcTag );

		if (pstInstance)
		{
            //ntAssert( pstInstance != NULL );					// Make sure not trying to read something that was never started...

			#ifndef _GATSO_NO_SAFETY_CHECKS
				ntAssert( !pstInstance->m_bActive );			// Make sure not trying to read an active instance...
			#endif	//_GATSO_NO_SAFETY_CHECKS

			float fPercentage = ( ( float )( pstInstance->m_lAccumTime ) * CTimer::GetHWTimerPeriodFrame() ) * 100.0f;
	
			return fPercentage;
		}
	}
	return 0.0f;
}	
#endif	//_GATSO_SUMMARY_ONLY

/***************************************************************************************************
*	
*	FUNCTION		AddInstance
*
*	DESCRIPTION		Adds new gatso instance.
*
***************************************************************************************************/

#ifndef	_GATSO_SUMMARY_ONLY
CGatso::GATSO_INSTANCE* CGatso::AddInstance( const char* pcTag )
{
	// Get new management instance from pool...
	ntAssert( m_guiInstanceIndex < uiGATSO_NUM_POOL_ELEMENTS );

	GATSO_INSTANCE* pstInstance = &m_gastInstancePool[ m_guiInstanceIndex++ ];

	// Get hash list index into which instance will be placed...
	u_int uiIndex = GetHashIndex( pcTag );

	// Initialise fields in management instance...
	pstInstance->m_pcTag 	   = pcTag;
	pstInstance->m_pstNext     = m_gapstHashList[ uiIndex ];
	pstInstance->m_bActive	   = false;
	pstInstance->m_iAccumCalls = 0;
	pstInstance->m_lAccumTime  = 0;

	ResetGatsoHistory();

	// new instance becomes first link at hash index position...
	m_gapstHashList[ uiIndex ] = pstInstance;

	return( pstInstance );
}	
#endif	//_GATSO_SUMMARY_ONLY


/***************************************************************************************************
*	
*	FUNCTION		AddVarInstance
*
*	DESCRIPTION		Add gatso instance to display variable contents.
*
***************************************************************************************************/

#ifndef	_GATSO_SUMMARY_ONLY
void	CGatso::AddVarInstance( const char* pcTag, enum VAR_TYPE eType, intptr_t iVal )
{
	if ( !( m_giTick & uiGATSO_TICK_MASK ) )
	{
		GATSO_INSTANCE* pstInstance;

		#ifndef _GATSO_NO_SAFETY_CHECKS
			pstInstance = FindInstance( pcTag );
			ntAssert_p( pstInstance == NULL, ( "%s %d %d\n", pcTag, eType, iVal ) );			// Shouldn't exist yet!
		#endif	//_GATSO_NO_SAFETY_CHECKS

		pstInstance = AddInstance( pcTag );

		*( enum VAR_TYPE* )&pstInstance->m_iAccumCalls = eType;
		pstInstance->m_lAccumTime = iVal;
	}
}	
#endif	//_GATSO_SUMMARY_ONLY


/***************************************************************************************************
*	
*	FUNCTION		AddVarInstanceAcc
*
*	DESCRIPTION		Add gatso instance to display variable contents of accumulating value.
*
***************************************************************************************************/

#ifndef	_GATSO_SUMMARY_ONLY
void	CGatso::AddVarInstanceAcc( const char* pcTag, enum VAR_TYPE eType, intptr_t iVal )
{
	if ( !( m_giTick & uiGATSO_TICK_MASK ) )
	{
		GATSO_INSTANCE* pstInstance;

		if ( ( pstInstance = FindInstance( pcTag ) ) == NULL )
		{
			// First reference of tag, create new instance and clear value...
			pstInstance = AddInstance( pcTag );
			*( enum VAR_TYPE* )&pstInstance->m_iAccumCalls = eType;
			pstInstance->m_lAccumTime = 0;		
		}

		ntAssert( eType == *( enum VAR_TYPE* )&pstInstance->m_iAccumCalls );

		pstInstance->m_lAccumTime += iVal;
	}
}	
#endif	//_GATSO_SUMMARY_ONLY

// gets the first gatso instance
CGatso::GATSO_INSTANCE* CGatso::Begin()
{
	u_int uiIndex = 0;
	while( uiIndex < uiGATSO_HASH_LIST_SIZE )
	{
		if( m_gapstHashList[ uiIndex ] != 0 )
			return m_gapstHashList[ uiIndex ];
		else
			uiIndex++;
	}

	return 0;
}

// this is the last gatso i.e. while( iterator != CGatso::End() )
CGatso::GATSO_INSTANCE* CGatso::End()
{
	return 0;
}

// gets the next gatso, normal use of gatso iterator
// iterator = CGatso::Begin();
// while( iterator != CGatso::End() )
//		iterator = CGatso::Next( iterator );
CGatso::GATSO_INSTANCE* CGatso::Next( CGatso::GATSO_INSTANCE* pCurr )
{
	if( pCurr == 0 )
		return 0;

	// if we are at the end of this hash bucket get next bucket
	if( pCurr->m_pstNext != 0 )
	{
		return pCurr->m_pstNext;
	} else
	{
		u_int uiIndex = GetHashIndex( pCurr->m_pcTag )+1;
		while( uiIndex < uiGATSO_HASH_LIST_SIZE )
		{
			if( m_gapstHashList[ uiIndex ] != 0 )
				return m_gapstHashList[ uiIndex ];
			else
				uiIndex++;
		}
		return 0;
	}
}

void CGatso::ResetGatsoHistory()
{
	CGatso::GATSO_INSTANCE* gatIt = CGatso::Begin();
	while( gatIt != CGatso::End() )
	{
		gatIt->m_fMaxPercent = -FLT_MAX;
		for( int i=0;i < TOTAL_SAMPLES;i++)
		{
			gatIt->m_fHistory[i] = 0.0f;
		}
		gatIt->m_iCurrentIndex = 0;
		gatIt = CGatso::Next( gatIt );
	}

}


/***************************************************************************************************
*	
*	FUNCTION		CGatso::Update
*
*	DESCRIPTION		Update gatso timing results (and optionally spit them out to console)
*
***************************************************************************************************/

void	CGatso::Update( void )
{
	if (m_gbRequestConsoleDump)
	{
		ntPrintf("\n =========GATSO=============\n");
	}

	// If we have things to display, process them...
	if ( m_guiInstanceIndex > 0 )
	{
		u_int			uiCount;
		GATSO_INSTANCE* pstInstance;
		// Check each hash list position for active instances...
		for ( uiCount = 0; uiCount < uiGATSO_HASH_LIST_SIZE; uiCount++ )
		{
			pstInstance = m_gapstHashList[ uiCount ];
			// Display all instances at hash list position...
			while ( pstInstance != NULL )
			{
				if ( pstInstance->m_iAccumCalls > 0 )
				{
					// Show a timing result instance...

					float fPercentage = ( ( float )( pstInstance->m_lAccumTime ) * CTimer::GetHWTimerPeriodFrame() ) * 100.0f;				
					pstInstance->m_fHistory[ pstInstance->m_iCurrentIndex % TOTAL_SAMPLES ] = fPercentage;			
					pstInstance->m_fMaxPercent = max(pstInstance->m_fMaxPercent, fPercentage );
					pstInstance->m_iCurrentIndex++;
					pstInstance->m_iCurrentIndex = pstInstance->m_iCurrentIndex % TOTAL_SAMPLES;

					if (m_gbRequestConsoleDump)
					{
						ntPrintf("%s : %f\n", pstInstance -> m_pcTag, fPercentage);
					}
				}
				// Get next instance in hash slot, or NULL...
				pstInstance = pstInstance->m_pstNext;
			}
		}
	}
	m_gbRequestConsoleDump = false;
}

#endif	// _GATSO
