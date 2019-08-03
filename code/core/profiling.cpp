/***************************************************************************************************
*
*	DESCRIPTION		Profiling instrumentation
*
*	NOTES
*
***************************************************************************************************/


#include "core/timer.h"
#include "core/visualdebugger.h"

#include "gfx/graphing.h"
#include "gfx/graphicsdevice.h"
#include "gfx/renderer.h"
#include "core/gatso.h"
#include "core/debug_hud.h"
#include "core/gfxmem.h"
//#include "core/memreport.h"
#include <time.h>
#include "input/inputhardware.h"
#include "core/profiling.h"

#define BYTES_TO_KB( n )	((n + 1023)/1024)
#define BYTES_TO_MB( m )	((BYTES_TO_KB(m) + 1023)/1024)

#if defined ( _PROFILING )

namespace
{
	static const unsigned int COL_WHITE_IGNORE	= NTCOLOUR_ARGB(0xFF,0x66,0x66,0x00);
	static const unsigned int COL_RED_IGNORE	= NTCOLOUR_ARGB(0xFF,0x66,0x00,0x00);

	static const unsigned int COL_WHITE			= NTCOLOUR_ARGB(0xFF,0xFF,0xFF,0xFF);
	static const unsigned int COL_RED			= NTCOLOUR_ARGB(0xFF,0xFF,0x00,0x00);
	static const unsigned int COL_GREEN			= NTCOLOUR_ARGB(0xFF,0x00,0xFF,0x00);
	static const unsigned int COL_BLUE			= NTCOLOUR_ARGB(0xFF,0x00,0x00,0xFF);
	static const unsigned int COL_BLACK			= NTCOLOUR_ARGB(0xFF,0x00,0x00,0x00);

	static const unsigned int COL_YELLOW		= NTCOLOUR_ARGB(0xFF,0xFF,0xFF,0x00);
	static const unsigned int COL_LBLUE			= NTCOLOUR_ARGB(0xFF,0x00,0xFF,0xFF);
	static const unsigned int COL_MGREEN		= NTCOLOUR_ARGB(0xFF,0x00,0xFF,0x40);
	static const unsigned int COL_ORANGE		= NTCOLOUR_ARGB(0xFF,0xFF,0x80,0x00);
	static const unsigned int COL_INVISIBLE		= NTCOLOUR_ARGB(0x00,0xFF,0xFF,0xFF);


	static const unsigned int MAX_FUNCTIONS = 28;
	static const int GRAPH_SAMPLES = 900;
}


int LoadTimeProfiler::s_iIndentCount = 0;
bool g_bSortGatso = false;



/***************************************************************************************************
*	 
*	CLASS			TimedOperationWarning
*
*	DESCRIPTION		Prints a user-specified warning message to the console if an operation
*					takes unusually long to complete. This assumes pOperationID and pMessage
*					will still be available at destruction time.
*
***************************************************************************************************/
TimedOperationWarning::TimedOperationWarning( const char* pOperationID, const char* pMessage, float fThresholdSeconds )
	: m_iStartTime( 0 )
	, m_pOperationID( pOperationID )
	, m_pMessage( pMessage )
	, m_fThresholdSeconds( fThresholdSeconds )
{
	m_iStartTime = CTimer::GetHWTimer();
}

float TimedOperationWarning::ElapsedSeconds() const
{
	return static_cast< float >( CTimer::GetHWTimer() - m_iStartTime ) * CTimer::GetHWTimerPeriod();
}

void TimedOperationWarning::Stop()
{
	if ( m_iStartTime == 0)
		return;

	float fElapsedSeconds = ElapsedSeconds();
	if ( fElapsedSeconds > m_fThresholdSeconds )
	{
        static char pBuffer[ 256 ];
		snprintf( pBuffer, sizeof( pBuffer ), "TIMED OPERATION WARNING: %s - %s (%f secs)\n", m_pOperationID, m_pMessage, fElapsedSeconds );
		Debug::AlwaysOutputString( pBuffer );
	}

	m_iStartTime = 0;
}


/***************************************************************************************************
*	 
*	CLASS			LoadTimeProfiler
*
*	DESCRIPTION		A simple little helper class (and macros) for doing a timing not via gatso
*					as gatso are frame based
*
***************************************************************************************************/
LoadTimeProfiler::LoadTimeProfiler( )
{
	m_iStart = CTimer::GetHWTimer(); 
	s_iIndentCount++;
}

// the pText should be a printf style with a single %f to take the time in seconds
void LoadTimeProfiler::StopAndPrint( const char* pText)
{
	// this dump to the console the elapsed time in seconds
	int64_t iEnd = CTimer::GetHWTimer();
	float fPeriod = CTimer::GetHWTimerPeriod();
	float fTimeInSecs = ((float)(iEnd - m_iStart)) * fPeriod;
	char pBuffer[1024];
	sprintf( pBuffer, pText, fTimeInSecs );

	for( int i=1;i < s_iIndentCount;++i )
	{
		Debug::AlwaysOutputString( "    " );
	}
	Debug::AlwaysOutputString( pBuffer );
	s_iIndentCount--;
}

//!**************************************************************************************************
//!
//! CProfilerInternal stuff
//!
//!**************************************************************************************************

class CProfilerInternal : public Singleton<CProfilerInternal>
{
public:
	CProfilerInternal() : 
		m_fGraphSizeX( -1.f ),
		m_fGraphSizeY( -1.f ),
		m_fGraphSizeWidth(180.f),
		m_fGraphSizeHeight(150.f),
		m_pobFunctionGraph(0),
		m_StartGatso(0)
	{
		m_pobOverallGraph	= NT_NEW_CHUNK( Mem::MC_DEBUG ) CGraph( GRAPH_TYPE_ROLLING );
		m_pobSummaryGraph	= NT_NEW_CHUNK( Mem::MC_DEBUG ) CGraph( GRAPH_TYPE_ROLLING );
		m_pobTotalMemGraph	= NT_NEW_CHUNK( Mem::MC_DEBUG ) CGraph( GRAPH_TYPE_ROLLING );
		m_pobPerFrameMemGraph	= NT_NEW_CHUNK( Mem::MC_DEBUG) CGraph( GRAPH_TYPE_ROLLING );
		
		m_pRunningTime		= m_pobOverallGraph->AddSampleSet( "FRAME", GRAPH_SAMPLES, COL_YELLOW );
		m_pPresentTime		= m_pobOverallGraph->AddSampleSet( "PRESENT", GRAPH_SAMPLES, COL_LBLUE );
		m_pTotalTime		= m_pobOverallGraph->AddSampleSet( "TOTAL", GRAPH_SAMPLES, COL_WHITE );

		m_pGameTime			= m_pobSummaryGraph->AddSampleSet( "UPDATE", GRAPH_SAMPLES, COL_MGREEN );
		m_pCPURenderTime	= m_pobSummaryGraph->AddSampleSet( "CPU RENDER", GRAPH_SAMPLES, COL_ORANGE );
		m_pGPURenderTime	= m_pobSummaryGraph->AddSampleSet( "GPU RENDER", GRAPH_SAMPLES, COL_RED );

		m_pRAM	= m_pobOverallGraph->AddSampleSet( "MEMORY", GRAPH_SAMPLES, COL_GREEN );
		m_pVRAM	= m_pobOverallGraph->AddSampleSet( "VRAM", GRAPH_SAMPLES, COL_ORANGE );

		m_pPerFrameBytesInUse = m_pobPerFrameMemGraph->AddSampleSet( "PER_FRAME", GRAPH_SAMPLES, COL_YELLOW );

		m_pTotalBytesInUse 				= m_pobTotalMemGraph->AddSampleSet( "TOTAL", GRAPH_SAMPLES, COL_YELLOW );
		m_pTotalMiscBytesInUse 			= m_pobTotalMemGraph->AddSampleSet( "MISC", GRAPH_SAMPLES, COL_LBLUE );
		m_pTotalRSXMainBytesInUse 		= m_pobTotalMemGraph->AddSampleSet( "RSXMAIN", GRAPH_SAMPLES, COL_GREEN );
		m_pTotalXDDRTextureBytesInUse 	= m_pobTotalMemGraph->AddSampleSet( "XDDRTEXTURE", GRAPH_SAMPLES, COL_RED );
		m_pTotalGFXBytesInUse 			= m_pobTotalMemGraph->AddSampleSet( "ATG", GRAPH_SAMPLES, COL_WHITE );
		m_pTotalHavokBytesInUse 		= m_pobTotalMemGraph->AddSampleSet( "HAVOK", GRAPH_SAMPLES, COL_GREEN );
		m_pTotalOverflowBytesInUse	= m_pobTotalMemGraph->AddSampleSet( "DEBUGOVERFLOW", GRAPH_SAMPLES, COL_ORANGE );
		m_pTotalODBBytesInUse			= m_pobTotalMemGraph->AddSampleSet( "ODB", GRAPH_SAMPLES, COL_ORANGE );
		m_pTotalAnimationBytesInUse		= m_pobTotalMemGraph->AddSampleSet( "ANIM", GRAPH_SAMPLES, COL_BLACK );
		m_pTotalLuaBytesInUse			= m_pobTotalMemGraph->AddSampleSet( "LUA", GRAPH_SAMPLES, COL_RED );
		

		m_pPerFrameBytesInUse = m_pobPerFrameMemGraph->AddSampleSet( "PER_FRAME", GRAPH_SAMPLES, COL_YELLOW );
		m_pPerFrameMiscBytesInUse = m_pobPerFrameMemGraph->AddSampleSet( "MISC", GRAPH_SAMPLES, COL_LBLUE );
		m_pPerFrameRSXMainBytesInUse = m_pobPerFrameMemGraph->AddSampleSet( "RSXMAIN", GRAPH_SAMPLES, COL_GREEN );
		m_pPerFrameXDDRTextureBytesInUse = m_pobPerFrameMemGraph->AddSampleSet( "XDDRTEXTURE", GRAPH_SAMPLES, COL_RED );
		m_pPerFrameGFXBytesInUse = m_pobPerFrameMemGraph->AddSampleSet( "ATG", GRAPH_SAMPLES, COL_WHITE );
		m_pPerFrameHavokBytesInUse = m_pobPerFrameMemGraph->AddSampleSet( "HAVOK", GRAPH_SAMPLES, COL_GREEN );
		m_pPerFrameOverflowBytesInUse = m_pobPerFrameMemGraph->AddSampleSet( "DEBUGOVERFLOW", GRAPH_SAMPLES, COL_ORANGE );
		m_pPerFrameODBBytesInUse = m_pobPerFrameMemGraph->AddSampleSet( "ODB", GRAPH_SAMPLES, COL_GREEN );

	}
	~CProfilerInternal()
	{
		NT_DELETE_CHUNK( Mem::MC_DEBUG, m_pobTotalMemGraph );
		NT_DELETE_CHUNK( Mem::MC_DEBUG, m_pobPerFrameMemGraph );
		NT_DELETE_CHUNK( Mem::MC_DEBUG, m_pobOverallGraph );
		NT_DELETE_CHUNK( Mem::MC_DEBUG, m_pobSummaryGraph );
		NT_DELETE_CHUNK( Mem::MC_DEBUG, m_pobFunctionGraph );
	}

	// use to 'zoom' into graphs 
	float				m_fGraphSizeX;
	float				m_fGraphSizeY;
	float				m_fGraphSizeWidth;
	float				m_fGraphSizeHeight;

	CGraph*				m_pobFunctionGraph;
	CGraph*				m_pobOverallGraph;
	CGraph*				m_pobSummaryGraph;
	CGraph*				m_pobTotalMemGraph;
	CGraph*				m_pobPerFrameMemGraph;


	CGraphSampleSet*	m_pobInstantSampleSet;
	CGraphSampleSet*	m_pobMaxSampleSet;
	CGraphSampleSet*	m_pobAvgSampleSet;
	CGraphSampleSet*	m_pobLastSecSampleSet;
	CGatso::GATSO_INSTANCE* m_pGraphedGatso;

	CGraphSampleSet*	m_pRunningTime;
	CGraphSampleSet*	m_pPresentTime;
	CGraphSampleSet*	m_pTotalTime;

	CGraphSampleSet*	m_pGameTime;
	CGraphSampleSet*	m_pCPURenderTime;
	CGraphSampleSet*	m_pGPURenderTime;
	
	CGraphSampleSet*	m_pVRAM;
	CGraphSampleSet*	m_pRAM;

	CGraphSampleSet*	m_pTotalBytesInUse ;
	CGraphSampleSet*	m_pTotalMiscBytesInUse;
	CGraphSampleSet*	m_pTotalRSXMainBytesInUse;
	CGraphSampleSet*	m_pTotalXDDRTextureBytesInUse;
	CGraphSampleSet*	m_pTotalGFXBytesInUse;
	CGraphSampleSet*	m_pTotalHavokBytesInUse;
	CGraphSampleSet*	m_pTotalOverflowBytesInUse;
	CGraphSampleSet*	m_pTotalODBBytesInUse;
	CGraphSampleSet*	m_pTotalAnimationBytesInUse;
	CGraphSampleSet*	m_pTotalLuaBytesInUse;

	CGraphSampleSet*	m_pPerFrameBytesInUse ;
	CGraphSampleSet*	m_pPerFrameMiscBytesInUse;
	CGraphSampleSet*	m_pPerFrameRSXMainBytesInUse;
	CGraphSampleSet*	m_pPerFrameXDDRTextureBytesInUse;
	CGraphSampleSet*	m_pPerFrameGFXBytesInUse;
	CGraphSampleSet*	m_pPerFrameHavokBytesInUse;
	CGraphSampleSet*	m_pPerFrameOverflowBytesInUse;
	CGraphSampleSet*	m_pPerFrameODBBytesInUse;

	int m_StartGatso;

	enum GATSO_INDICES
	{
		// positive are selectable
		GI_MAIN_OVERALL = 0,
		GI_MAIN_SUMMARY = 1,
		GI_MAIN_FUNCTIONS = 2,
		GI_OVERALL_GRAPH = 3,
		GI_SUMMARY_GRAPH = 4,
		GI_RESET = 5,
		GI_FUNCTION_GRAPH = 6,
		GI_MAIN_TOTAL_MEMORY = 7,
		GI_MAIN_PER_FRAME_MEMORY = 8,
		GI_TOTAL_MEM_GRAPH = 9,
		GI_PER_FRAME_MEM_GRAPH = 10,
		GI_MAIN_MEMCHUNK_TOTALS = 11,

		GI_SCROLL_UP = 200,
		GI_SCROLL_DOWN = 201,

		// negative are not selectable
		GI_IGNORE = -1, // -1 is for non selectable non callback types.

		GI_CPU_PERCENT = -2,
		GI_PRESENT_PERCENT = -3,
		GI_TOTAL_TIME = -4,
		GI_MEMORY_PERCENT = -5,

		GI_TOTAL_POLY_COUNT = -11,
		GI_RENDERED_POLY_COUNT = -12,
		GI_POLYS_PER_SEC = -13,
		GI_GAME_PERCENT = -14,
		GI_CPU_RENDER_PERCENT = -15,
		GI_GPU_RENDER_PERCENT = -16,
		GI_RENDER_MEMORY_PERCENT = -17,

		GI_FUNCTION_MAX = -18,
		GI_FUNCTION_AVG_SEC = -19,
		GI_FUNCTION_LAST_SEC = -20,
		GI_FUNCTION_INSTANT = -21,

		// Memory chunk total display
		GI_MEMCHUNK_TOTAL_BASE = -8000,

		GI_MEMCHUNK_TOTAL_MC_RSX_MAIN_INTERNAL		= GI_MEMCHUNK_TOTAL_BASE + Mem::MC_RSX_MAIN_INTERNAL,         
		GI_MEMCHUNK_TOTAL_MC_RSX_MAIN_USER			= GI_MEMCHUNK_TOTAL_BASE + Mem::MC_RSX_MAIN_USER,

		GI_MEMCHUNK_TOTAL_MC_GFX              		= GI_MEMCHUNK_TOTAL_BASE + Mem::MC_GFX,              
		GI_MEMCHUNK_TOTAL_MC_LOADER           		= GI_MEMCHUNK_TOTAL_BASE + Mem::MC_LOADER,           
		GI_MEMCHUNK_TOTAL_MC_ANIMATION        		= GI_MEMCHUNK_TOTAL_BASE + Mem::MC_ANIMATION, 
		GI_MEMCHUNK_TOTAL_MC_HAVOK					= GI_MEMCHUNK_TOTAL_BASE + Mem::MC_HAVOK,         
		GI_MEMCHUNK_TOTAL_MC_AUDIO					= GI_MEMCHUNK_TOTAL_BASE + Mem::MC_AUDIO,

		GI_MEMCHUNK_TOTAL_MC_ODB              		= GI_MEMCHUNK_TOTAL_BASE + Mem::MC_ODB,              

		GI_MEMCHUNK_TOTAL_MC_ARMY             		= GI_MEMCHUNK_TOTAL_BASE + Mem::MC_ARMY,             
		GI_MEMCHUNK_TOTAL_MC_LUA              		= GI_MEMCHUNK_TOTAL_BASE + Mem::MC_LUA,              
		GI_MEMCHUNK_TOTAL_MC_ENTITY           		= GI_MEMCHUNK_TOTAL_BASE + Mem::MC_ENTITY,           
		GI_MEMCHUNK_TOTAL_MC_MISC             		= GI_MEMCHUNK_TOTAL_BASE + Mem::MC_MISC,             

		GI_MEMCHUNK_TOTAL_MC_AI               		= GI_MEMCHUNK_TOTAL_BASE + Mem::MC_AI,               
		GI_MEMCHUNK_TOTAL_MC_CAMERA           		= GI_MEMCHUNK_TOTAL_BASE + Mem::MC_CAMERA,           
		GI_MEMCHUNK_TOTAL_MC_EFFECTS        		= GI_MEMCHUNK_TOTAL_BASE + Mem::MC_EFFECTS,
		GI_MEMCHUNK_TOTAL_MC_PROCEDURAL        		= GI_MEMCHUNK_TOTAL_BASE + Mem::MC_PROCEDURAL,

		GI_MEMCHUNK_TOTAL_MC_OVERFLOW   			= GI_MEMCHUNK_TOTAL_BASE + Mem::MC_OVERFLOW,

#ifdef _HAVE_DEBUG_MEMORY
		GI_MEMCHUNK_TOTAL_MC_DEBUG 					= GI_MEMCHUNK_TOTAL_BASE + Mem::MC_DEBUG,
#endif

#ifdef _HAVE_MEMORY_TRACKING
		GI_MEMCHUNK_TOTAL_MC_DEBUG_MEM_TRACKER 		= GI_MEMCHUNK_TOTAL_BASE + Mem::MC_DEBUG_MEM_TRACKER,
#endif

		GI_MEMCHUNK_TOTAL_XDDR,
	};

	static void DebugHUDGraphCallback(  DebugHudItem* item, float *fInOutX, float *fInOutY, float* pOutWidth, float* pOutHeight )
	{
		if( CProfilerInternal::Get().m_fGraphSizeX > 0 )
		{
			*fInOutX = CProfilerInternal::Get().m_fGraphSizeX;
		}
		if( CProfilerInternal::Get().m_fGraphSizeY > 0 )
		{
			*fInOutY = CProfilerInternal::Get().m_fGraphSizeY;
		}

		float fX = *fInOutX;
		float fY = *fInOutY;

		float fWidth = CProfilerInternal::Get().m_fGraphSizeWidth;
		float fHeight = CProfilerInternal::Get().m_fGraphSizeHeight;

		CPoint obTopLeft( fX, fY , 0.0f );
		CPoint obBottomRight( fX + fWidth, fY + fHeight, 0.0f );

		switch( item->iIndex )
		{
		case GI_OVERALL_GRAPH:
			g_VisualDebug->RenderGraph(  CProfilerInternal::Get().m_pobOverallGraph, obTopLeft, obBottomRight );
			break;
		case GI_SUMMARY_GRAPH:
			g_VisualDebug->RenderGraph(  CProfilerInternal::Get().m_pobSummaryGraph, obTopLeft, obBottomRight );
			break;
		case GI_FUNCTION_GRAPH:
			if(CProfilerInternal::Get().m_pobFunctionGraph != 0 )
				g_VisualDebug->RenderGraph(  CProfilerInternal::Get().m_pobFunctionGraph, obTopLeft, obBottomRight );
			break;
		case GI_TOTAL_MEM_GRAPH:
			g_VisualDebug->RenderGraph(  CProfilerInternal::Get().m_pobTotalMemGraph, obTopLeft, obBottomRight );
			break;
		case GI_PER_FRAME_MEM_GRAPH:
			g_VisualDebug->RenderGraph(  CProfilerInternal::Get().m_pobPerFrameMemGraph, obTopLeft, obBottomRight );
			break;
		}

		*fInOutX = *fInOutX - 2;
		*fInOutY = *fInOutY - 2;
		*pOutWidth = fWidth + 4;
		*pOutHeight = fHeight + 4;

	}
	static bool DebugHUDGraphSelectCallback(  DebugHudItem* pItem, bool bSelect )
	{
		if( bSelect )
		{
			CProfilerInternal::Get().m_fGraphSizeX = 20.f;
			CProfilerInternal::Get().m_fGraphSizeY = 20.f;
			CProfilerInternal::Get().m_fGraphSizeWidth = g_VisualDebug->GetDebugDisplayWidth() - 40.f;
			CProfilerInternal::Get().m_fGraphSizeHeight = g_VisualDebug->GetDebugDisplayHeight() - 40.f;
		} else
		{
			CProfilerInternal::Get().m_fGraphSizeX = -1.f;
			CProfilerInternal::Get().m_fGraphSizeY = -1.f;
			CProfilerInternal::Get().m_fGraphSizeWidth = 240.f;
			CProfilerInternal::Get().m_fGraphSizeHeight = 200.f;

			if( pItem->iIndex == GI_FUNCTION_GRAPH )
			{
				NT_DELETE( CProfilerInternal::Get().m_pobFunctionGraph );
				CProfilerInternal::Get().m_pobFunctionGraph = 0;
	
				// rebuild function list
				return DebugHUDFuncMenuSelectCallback( 0, true );
			}
		}

		return true;
	}
	
	// Note, this should reflect summary operation in DumpSimpleChunkStats()
	static void CreateMemChunkSummary( char* pBuffer )
	{
		const Mem::MemStats& totalStats( Mem::GetMemStats() );
		const Mem::MemSubStats& chunkStats(  );
		
		int iTotalUsed = 0;
		iTotalUsed += totalStats.sChunks[ Mem::MC_RSX_MAIN_INTERNAL ].iOriginalAllocationSize;

		iTotalUsed += totalStats.sChunks[ Mem::MC_GFX ].iOriginalAllocationSize;
		iTotalUsed += totalStats.sChunks[ Mem::MC_LOADER ].iOriginalAllocationSize;
		iTotalUsed += totalStats.sChunks[ Mem::MC_ANIMATION ].iOriginalAllocationSize;
		iTotalUsed += totalStats.sChunks[ Mem::MC_HAVOK ].iOriginalAllocationSize;
		iTotalUsed += totalStats.sChunks[ Mem::MC_AUDIO ].iOriginalAllocationSize;

		int iTotalAlloced = iTotalUsed;

#ifndef _COLLAPSE_SMALL_CHUNKS
		// deliberately ignore overflow, as will be accounted for in the overflow chunk
		iTotalUsed += totalStats.sChunks[ Mem::MC_ODB ].iNumBytesInUse;
		iTotalUsed += totalStats.sChunks[ Mem::MC_ARMY ].iNumBytesInUse;
		iTotalUsed += totalStats.sChunks[ Mem::MC_LUA ].iNumBytesInUse;
		iTotalUsed += totalStats.sChunks[ Mem::MC_ENTITY ].iNumBytesInUse;
		iTotalUsed += totalStats.sChunks[ Mem::MC_MISC ].iNumBytesInUse;
		iTotalUsed += totalStats.sChunks[ Mem::MC_AI ].iNumBytesInUse;
		iTotalUsed += totalStats.sChunks[ Mem::MC_CAMERA ].iNumBytesInUse;
		iTotalUsed += totalStats.sChunks[ Mem::MC_EFFECTS ].iNumBytesInUse;
		iTotalUsed += totalStats.sChunks[ Mem::MC_PROCEDURAL ].iNumBytesInUse;

		iTotalAlloced += totalStats.sChunks[ Mem::MC_ODB ].iOriginalAllocationSize;
		iTotalAlloced += totalStats.sChunks[ Mem::MC_ARMY ].iOriginalAllocationSize;
		iTotalAlloced += totalStats.sChunks[ Mem::MC_LUA ].iOriginalAllocationSize;
		iTotalAlloced += totalStats.sChunks[ Mem::MC_ENTITY ].iOriginalAllocationSize;
		iTotalAlloced += totalStats.sChunks[ Mem::MC_MISC ].iOriginalAllocationSize;
		iTotalAlloced += totalStats.sChunks[ Mem::MC_AI ].iOriginalAllocationSize;
		iTotalAlloced += totalStats.sChunks[ Mem::MC_CAMERA ].iOriginalAllocationSize;
		iTotalAlloced += totalStats.sChunks[ Mem::MC_EFFECTS ].iOriginalAllocationSize;
		iTotalAlloced += totalStats.sChunks[ Mem::MC_PROCEDURAL ].iOriginalAllocationSize;
#endif

		iTotalUsed += totalStats.sChunks[ Mem::MC_OVERFLOW ].iNumBytesInUse;
		iTotalAlloced += totalStats.sChunks[ Mem::MC_OVERFLOW ].iOriginalAllocationSize;

#ifndef _HAVE_MEMORY_TRACKING
		iTotalUsed = 0;
#endif

		float percent_allocated = (_R(iTotalUsed) * 100.0f) / _R(iTotalAlloced);

		sprintf( pBuffer, "                    : %10dK             %10dK [%3.f %%%%]   %3dMb           %3dMb",
					BYTES_TO_KB( iTotalUsed ),
					BYTES_TO_KB( iTotalAlloced ),
					percent_allocated,
					BYTES_TO_MB( iTotalUsed ),
					BYTES_TO_MB( iTotalAlloced ) );
	}
	
	static void CreateMemChunkText( char* pBuffer, Mem::MEMORY_CHUNK chunk, const char* pcName, unsigned int& outColour )
	{
		const Mem::MemStats& totalStats( Mem::GetMemStats() );
		const Mem::MemSubStats& chunkStats( totalStats.sChunks[ chunk ] );
		
		int32_t used_bytes = chunkStats.iNumBytesInUse + chunkStats.iNumBytesOverflowInUse;
		int32_t max_bytes = chunkStats.iNumBytesInUseHighWaterMark + chunkStats.iNumBytesOverflowHighWaterMark;
		int32_t start_bytes = chunkStats.iOriginalAllocationSize;

		float percent_allocated = (_R(used_bytes) * 100.0f) / _R(start_bytes);

		sprintf( pBuffer, "%20s: %10dK %10dK %10dK [%3.f %%%%]   %3dMb   %3dMb   %3dMb",
					pcName,
					BYTES_TO_KB( used_bytes ),
					BYTES_TO_KB( max_bytes ),
					BYTES_TO_KB( start_bytes ),
					percent_allocated,
					BYTES_TO_MB( used_bytes ),
					BYTES_TO_MB( max_bytes ),
					BYTES_TO_MB( start_bytes ) );
	
		if ( used_bytes >= start_bytes )
			outColour = COL_RED;
		else if ( max_bytes >= start_bytes )
			outColour = COL_ORANGE;
		else	
			outColour = COL_WHITE;
	}
	
	// helper macro for repeated memchunk level code
#define CASE_MEMCHUNK( memchunk, name )					\
	case (GI_MEMCHUNK_TOTAL_##memchunk):				\
	{													\
		CreateMemChunkText( outText, Mem::memchunk, name, outColour );	\
		break;					  						\
	}
	
	static void DebugHUDTextCallback( DebugHudItem* item, char outText[256], unsigned int& outColour )
	{
		static char* pcFrame		=	"CPU       : %5.1f %%%%";
		static char* pcBlocked		=	"PRESENT   : %5.1f %%%%";
		static char* pcTotal		=	"TOTAL     : %5.1f %%%% [%.0f FPS]";

		static char* pcUpdate		=	"UPDATE    : %5.1f %%%%";
		static char* pcCPURender	=	"CPU_RENDER: %5.1f %%%%";
		static char* pcGPURender	=	"GPU_RENDER: %5.1f %%%%";

		static char* pcRenderMemory	=	"VRAM      : %5.1f %%%%";
//		static char* pcMemory		=	"SYS RAM   : %5.1f %%%%";

		switch( item->iIndex )
		{
		case GI_CPU_PERCENT:
			sprintf( outText, pcFrame, CProfiler::Get().m_fRunningPercent );
			outColour = COL_YELLOW;
			break;

		case GI_PRESENT_PERCENT:
			sprintf( outText, pcBlocked, CProfiler::Get().m_fPresentPercent );
			outColour = COL_LBLUE;
			break;

		case GI_TOTAL_TIME:
			sprintf( outText, pcTotal, CProfiler::Get().m_fProfileTotal, CProfiler::Get().m_fFPS );
			if( CProfiler::Get().m_fProfileTotal < 100 )
			{
				outColour = COL_WHITE;
			} else
			{
				outColour = COL_RED;
			}
			break;

		case GI_GAME_PERCENT:
			sprintf( outText, pcUpdate, CProfiler::Get().m_fGamePercent );
			outColour = COL_MGREEN;
			break;

		case GI_CPU_RENDER_PERCENT:
			sprintf( outText, pcCPURender, CProfiler::Get().m_fCPURenderPercent );
			outColour = COL_ORANGE;
			break;

		case GI_GPU_RENDER_PERCENT:
			sprintf( outText, pcGPURender, CProfiler::Get().m_fGPURenderPercent );
			outColour = COL_RED;
			break;

		case GI_MEMORY_PERCENT:
//			sprintf( outText, pcMemory, MemTracker::GetMemoryUsage() * 100.f );
			outColour = COL_GREEN;
			break;

		case GI_RENDER_MEMORY_PERCENT:
			sprintf( outText, pcRenderMemory, Renderer::Get().m_Platform.GetMemoryUsage() * 100.f );
			outColour = COL_ORANGE;
			break;

		// all the memory chunk values are effectively boilerplated
		CASE_MEMCHUNK( MC_RSX_MAIN_INTERNAL		   , "RSX Main" )
		CASE_MEMCHUNK( MC_RSX_MAIN_USER     , "RSX Main User" )

		CASE_MEMCHUNK( MC_GFX              , "GFX" )
		CASE_MEMCHUNK( MC_LOADER           , "Loader" )
		CASE_MEMCHUNK( MC_ANIMATION        , "Animation" )
		CASE_MEMCHUNK( MC_HAVOK			   , "Havok" )
		CASE_MEMCHUNK( MC_AUDIO            , "Audio" )

#ifndef _COLLAPSE_SMALL_CHUNKS
		CASE_MEMCHUNK( MC_ODB              , "ODB" )

		CASE_MEMCHUNK( MC_ARMY             , "Army" )
		CASE_MEMCHUNK( MC_LUA              , "Lua" )
		CASE_MEMCHUNK( MC_ENTITY           , "Entity" )
		CASE_MEMCHUNK( MC_MISC             , "Misc" )

		CASE_MEMCHUNK( MC_AI               , "AI" )
		CASE_MEMCHUNK( MC_CAMERA           , "Camera" )
		CASE_MEMCHUNK( MC_EFFECTS          , "Effects" )
		CASE_MEMCHUNK( MC_PROCEDURAL       , "Procedural" )
#endif

		CASE_MEMCHUNK( MC_OVERFLOW		   , "Overflow" )

/*		No point in rendering these
		CASE_MEMCHUNK( MC_DEBUG			   , "Debug" )
		CASE_MEMCHUNK( MC_DEBUG_MEM_TRACKER, "Debug Tracker" )
*/

		case GI_MEMCHUNK_TOTAL_XDDR:
		{
			CreateMemChunkSummary( outText );
			outColour = COL_RED;
		}
		break;

		default:
			outText[0] = 0;
		};
	}

	static bool FindString( const ntstd::String& lookin, const ntstd::String& lookfor )
	{
		int iRes = lookin.find(lookfor);
		return iRes>=0;
	}

	static ntstd::String m_nameFilter;
	
	static void DebugHUDFunctionCallback( DebugHudItem* item, char outText[256], unsigned int& outColour )
	{
#ifdef	_GATSO
		CGatso::GATSO_INSTANCE* gatInst = (CGatso::GATSO_INSTANCE*) item->pUserData;
		if( gatInst == 0 )
		{
			outText[0] = 0;
			outColour = COL_BLACK;
			return;
		}
		static char* pcText		=	"%40.40s : %8.3f %%%%                  \n";
		static char* pcText2	=	"%40.40s : %8.3f %%%%                  \n";
		static char* pcTextInst	=	"%40.40s : %8.3f %%%%(%d)              \n";
		
		// positive are name and instant
		if( item->iIndex >= 0 )
		{
			// name and instant
			sprintf( outText, pcText, gatInst->m_pcTag, gatInst->GetInstantPercent() );
			
			// name filter stuff
			bool bIgnore = false;
			if(!m_nameFilter.empty())
			{
				bIgnore = !FindString(ntstd::String(outText),m_nameFilter);
			}
			
			if( gatInst->GetInstantPercent() < 100 )
			{
				outColour = bIgnore?COL_WHITE_IGNORE:COL_WHITE;
			} else
			{
				outColour = bIgnore?COL_RED_IGNORE:COL_RED;
			}
			if( item->uiColour == COL_INVISIBLE )
			{
				outColour = COL_INVISIBLE;
			}
		} 

		// negative indices are used to display other info in the sub menu
		switch( item->iIndex )
		{
		case GI_FUNCTION_INSTANT:
			sprintf( outText, pcTextInst, gatInst->m_pcTag, gatInst->GetInstantPercent(), gatInst->m_iAccumCalls );
			outColour = COL_WHITE;
			break;
		case GI_FUNCTION_MAX:
			// max 
			sprintf( outText, pcText2, "Max", gatInst->m_fMaxPercent );
			outColour = COL_GREEN;
			break;
		case GI_FUNCTION_AVG_SEC:
			// second average
			sprintf( outText, pcText2, "Average (second)", gatInst->GetFilteredPercent() );
			outColour = COL_YELLOW;
			break;
		case GI_FUNCTION_LAST_SEC:
		{
			// last second average
			int lastSecondIndex = (gatInst->m_iCurrentIndex / CGatso::SAMPLES_PER_SECOND);
			lastSecondIndex = lastSecondIndex * CGatso::SAMPLES_PER_SECOND;
			sprintf( outText, pcText2, "Last second Average", gatInst->GetSecondAverage( lastSecondIndex ) );
			outColour = COL_ORANGE;
			break;
		}
		
		}

#else
    UNUSED(outColour); UNUSED(outText); UNUSED(item);
#endif
	}

	static bool DebugHUDFuncMenuSelectCallback(  DebugHudItem*, bool bSelect );

	static bool DebugHUDEachFunctionSelectCallback( DebugHudItem*, bool bSelect );

	static bool DebugHUDResetHistorySelect( DebugHudItem*, bool bSelect )
	{
#ifdef _GATSO
		if( bSelect )
		{
			CGatso::ResetGatsoHistory();
		}
#else
    UNUSED(bSelect);
#endif
		return false;
	}

	void DumpGatsoToCSV();

	static bool DebugHUDScrollCallback( DebugHudItem* pItem, bool bSelect )
	{
		if( bSelect )
		{
			if( pItem->iIndex == GI_SCROLL_UP )
			{
				CProfilerInternal::Get().m_StartGatso--;
				if( CProfilerInternal::Get().m_StartGatso < 0 )
					CProfilerInternal::Get().m_StartGatso = 0;
			} else
			{
				CProfilerInternal::Get().m_StartGatso++;
			}
		}
		return DebugHUDFuncMenuSelectCallback(pItem, bSelect);
	}
};

ntstd::String CProfilerInternal::m_nameFilter = ntstd::String("");

namespace 
{
#define CPI_ CProfilerInternal::
#define TextCallback (const char*) &CPI_ DebugHUDTextCallback
#define GraphCallback (const char*) &CPI_ DebugHUDGraphCallback
#define SFTxtCallback (const char*) &CPI_ DebugHUDFunctionCallback

DebugHudItem ProfilerOverallMenu[] = 
{
	{ DebugHudItem::DHI_TEXT, {"Overall"},			CPI_ GI_IGNORE,		{COL_WHITE}, 0, 0, 0, 0 },
	{ DebugHudItem::DHI_TEXT, {"--------"},			CPI_ GI_IGNORE,		{COL_BLUE}, 0, 0, 0, 0 },
	{ DebugHudItem::DHI_TEXT_CALLBACK, {TextCallback},CPI_ GI_CPU_PERCENT, {0}, 0, 0, 0, 0 },
	{ DebugHudItem::DHI_TEXT_CALLBACK, {TextCallback},CPI_ GI_PRESENT_PERCENT, {0},  0, 0, 0, 0 },
	{ DebugHudItem::DHI_TEXT_CALLBACK, {TextCallback},CPI_ GI_TOTAL_TIME, {0}, 0, 0, 0, 0 },
	{ DebugHudItem::DHI_TEXT_CALLBACK, {TextCallback},CPI_ GI_MEMORY_PERCENT, {0}, 0, 0, 0 },
	{ DebugHudItem::DHI_TEXT_CALLBACK, {TextCallback},CPI_ GI_RENDER_MEMORY_PERCENT, {0}, 0, 0, 0, 0 },
	{ DebugHudItem::DHI_TEXT, {"--------"},			CPI_ GI_IGNORE,		{COL_BLUE}, 0, 0, 0, 0 },
	{ DebugHudItem::DHI_DRAW_CALLBACK, {GraphCallback},CPI_ GI_OVERALL_GRAPH, {0}, 0, &CPI_ DebugHUDGraphSelectCallback, 0, 0 },
	{ DebugHudItem::DHI_NONE, {0}, 0, {0}, 0, 0, 0 },	
};

DebugHudItem ProfilerSummaryMenu[] = 
{
	{ DebugHudItem::DHI_TEXT, {"Summary"},			CPI_ GI_IGNORE,		{COL_WHITE}, 0, 0, 0, 0 },
	{ DebugHudItem::DHI_TEXT, {"--------"},			CPI_ GI_IGNORE,		{COL_BLUE}, 0, 0, 0, 0 },
	{ DebugHudItem::DHI_TEXT_CALLBACK, {TextCallback},CPI_ GI_GAME_PERCENT, {0}, 0, 0, 0, 0 },
	{ DebugHudItem::DHI_TEXT_CALLBACK, {TextCallback},CPI_ GI_CPU_RENDER_PERCENT, {0},  0, 0, 0, 0 },
	{ DebugHudItem::DHI_TEXT_CALLBACK, {TextCallback},CPI_ GI_GPU_RENDER_PERCENT, {0},  0, 0, 0, 0 },
	{ DebugHudItem::DHI_TEXT, {"--------"},			CPI_ GI_IGNORE,		{COL_BLUE}, 0, 0, 0, 0 },
	{ DebugHudItem::DHI_DRAW_CALLBACK, {GraphCallback},CPI_ GI_SUMMARY_GRAPH, {0}, 0, &CPI_ DebugHUDGraphSelectCallback, 0, 0 },
	{ DebugHudItem::DHI_NONE, {0}, 0, {0}, 0, 0, 0 },	
};

DebugHudItem ProfilerFunctionMenu[ MAX_FUNCTIONS + 5] = 
{
	{ DebugHudItem::DHI_TEXT, {"Functions"},			CPI_ GI_IGNORE,		{COL_WHITE}, 0, 0, 0, 0 },
	{ DebugHudItem::DHI_TEXT, {"--------"},			CPI_ GI_IGNORE,		{COL_BLUE}, 0, 0, 0, 0 },
	{ DebugHudItem::DHI_TEXT, {"Scroll UP"},			CPI_ GI_SCROLL_UP,	{COL_ORANGE}, 0, &CPI_ DebugHUDScrollCallback, 0, 0 },
	{ DebugHudItem::DHI_NONE, {0}, 0, {0}, 0, 0, 0 },	
};
DebugHudItem ProfilerFunctionSubMenu[] = 
{
	{ DebugHudItem::DHI_TEXT_CALLBACK, {SFTxtCallback},CPI_ GI_FUNCTION_INSTANT, {0}, 0, 0, 0, 0 },
	{ DebugHudItem::DHI_TEXT_CALLBACK, {SFTxtCallback},CPI_ GI_FUNCTION_MAX, {0},  0, 0, 0, 0 },
	{ DebugHudItem::DHI_TEXT_CALLBACK, {SFTxtCallback},CPI_ GI_FUNCTION_AVG_SEC, {0},  0, 0, 0, 0 },
	{ DebugHudItem::DHI_TEXT_CALLBACK, {SFTxtCallback},CPI_ GI_FUNCTION_LAST_SEC, {0},  0, 0, 0, 0 },
	{ DebugHudItem::DHI_DRAW_CALLBACK, {GraphCallback},CPI_ GI_FUNCTION_GRAPH, {0}, 0, &CPI_ DebugHUDGraphSelectCallback, 0, 0 },
	{ DebugHudItem::DHI_NONE, {0}, 0, {0}, 0, 0, 0 },	
};

DebugHudItem ProfilerTotalMemoryMenu[] = 
{
	{ DebugHudItem::DHI_TEXT, {"Memory Stats"},		CPI_ GI_IGNORE,		{COL_WHITE}, 0, 0, 0, 0 },
	{ DebugHudItem::DHI_TEXT, {"--------"},			CPI_ GI_IGNORE,		{COL_BLUE}, 0, 0, 0, 0 },
	{ DebugHudItem::DHI_TEXT, {"Total"},			CPI_ GI_IGNORE,		{COL_YELLOW}, 0, 0, 0, 0 },
	{ DebugHudItem::DHI_TEXT, {"Misc"},				CPI_ GI_IGNORE,		{COL_LBLUE}, 0, 0, 0, 0 },
	{ DebugHudItem::DHI_TEXT, {"RSX Main"},			CPI_ GI_IGNORE,		{COL_GREEN}, 0, 0, 0, 0 },
	{ DebugHudItem::DHI_TEXT, {"   RSX Main User"},	CPI_ GI_IGNORE,	{COL_RED}, 0, 0, 0, 0 },
	{ DebugHudItem::DHI_TEXT, {"ATG"},				CPI_ GI_IGNORE,		{COL_WHITE}, 0, 0, 0, 0 },
	{ DebugHudItem::DHI_TEXT, {"Havok"},			CPI_ GI_IGNORE,		{COL_MGREEN}, 0, 0, 0, 0 },
	{ DebugHudItem::DHI_TEXT, {"Debug Overflow"},	CPI_ GI_IGNORE,		{COL_ORANGE}, 0, 0, 0, 0 },
	{ DebugHudItem::DHI_TEXT, {"--------"},			CPI_ GI_IGNORE,		{COL_BLUE}, 0, 0, 0, 0 },
	{ DebugHudItem::DHI_DRAW_CALLBACK, {GraphCallback},CPI_ GI_TOTAL_MEM_GRAPH, {0}, 0, &CPI_ DebugHUDGraphSelectCallback, 0, 0 },
	{ DebugHudItem::DHI_NONE, {0}, 0, {0}, 0, 0, 0 },	
};

DebugHudItem ProfilerPerFrameMemoryMenu[] = 
{
	{ DebugHudItem::DHI_TEXT, {"Memory Stats"},		CPI_ GI_IGNORE,		{COL_WHITE}, 0, 0, 0, 0 },
	{ DebugHudItem::DHI_TEXT, {"--------"},			CPI_ GI_IGNORE,		{COL_BLUE}, 0, 0, 0, 0 },
	{ DebugHudItem::DHI_TEXT, {"Total"},			CPI_ GI_IGNORE,		{COL_YELLOW}, 0, 0, 0, 0 },
	{ DebugHudItem::DHI_TEXT, {"Misc"},				CPI_ GI_IGNORE,		{COL_LBLUE}, 0, 0, 0, 0 },
	{ DebugHudItem::DHI_TEXT, {"RSX Main"},			CPI_ GI_IGNORE,		{COL_GREEN}, 0, 0, 0, 0 },
	{ DebugHudItem::DHI_TEXT, {"   RSX Main User"},	CPI_ GI_IGNORE,	{COL_RED}, 0, 0, 0, 0 },
	{ DebugHudItem::DHI_TEXT, {"ATG"},				CPI_ GI_IGNORE,		{COL_WHITE}, 0, 0, 0, 0 },
	{ DebugHudItem::DHI_TEXT, {"Havok"},			CPI_ GI_IGNORE,		{COL_MGREEN}, 0, 0, 0, 0 },
	{ DebugHudItem::DHI_TEXT, {"Debug Overflow"},	CPI_ GI_IGNORE,		{COL_ORANGE}, 0, 0, 0, 0 },
	{ DebugHudItem::DHI_TEXT, {"--------"},			CPI_ GI_IGNORE,		{COL_BLUE}, 0, 0, 0, 0 },
	{ DebugHudItem::DHI_DRAW_CALLBACK, {GraphCallback},CPI_ GI_PER_FRAME_MEM_GRAPH, {0}, 0, &CPI_ DebugHUDGraphSelectCallback, 0, 0 },
	{ DebugHudItem::DHI_NONE, {0}, 0, {0}, 0, 0, 0 },	
};

// [scee_st] menu to allow display of memory chunk levels as values rather than a graph
DebugHudItem ProfilerTotalMemoryValuesMenu[] = 
{
	{ DebugHudItem::DHI_TEXT, {"Chunk Totals"},				CPI_ GI_IGNORE,							{COL_WHITE}, 0, 0, 0, 0 },
	{ DebugHudItem::DHI_TEXT, {"--------"},					CPI_ GI_IGNORE,							{COL_BLUE}, 0, 0, 0, 0 },
	
	//						    12345678901234567890: 12345678901 12345678901 12345678901
	{ DebugHudItem::DHI_TEXT, {"                Name      Current     Highest      Budget"},			CPI_ GI_IGNORE,		{COL_YELLOW}, 0, 0, 0, 0 },
	{ DebugHudItem::DHI_TEXT, {"                ----      -------     -------      ------"},			CPI_ GI_IGNORE,		{COL_YELLOW}, 0, 0, 0, 0 },
	
	{ DebugHudItem::DHI_TEXT_CALLBACK, {TextCallback},		CPI_ GI_MEMCHUNK_TOTAL_MC_RSX_MAIN_INTERNAL , {COL_MGREEN},  0, 0, 0, 0 },
	{ DebugHudItem::DHI_TEXT_CALLBACK, {TextCallback},		CPI_ GI_MEMCHUNK_TOTAL_MC_RSX_MAIN_USER		, {COL_MGREEN},  0, 0, 0, 0 },

	{ DebugHudItem::DHI_TEXT_CALLBACK, {TextCallback},		CPI_ GI_MEMCHUNK_TOTAL_MC_GFX				, {COL_MGREEN},  0, 0, 0, 0 },
	{ DebugHudItem::DHI_TEXT_CALLBACK, {TextCallback},		CPI_ GI_MEMCHUNK_TOTAL_MC_LOADER			, {COL_MGREEN},  0, 0, 0, 0 },
	{ DebugHudItem::DHI_TEXT_CALLBACK, {TextCallback},		CPI_ GI_MEMCHUNK_TOTAL_MC_ANIMATION			, {COL_MGREEN},  0, 0, 0, 0 },	
	{ DebugHudItem::DHI_TEXT_CALLBACK, {TextCallback},		CPI_ GI_MEMCHUNK_TOTAL_MC_HAVOK				, {COL_MGREEN},  0, 0, 0, 0 },
	{ DebugHudItem::DHI_TEXT_CALLBACK, {TextCallback},		CPI_ GI_MEMCHUNK_TOTAL_MC_AUDIO				, {COL_MGREEN},  0, 0, 0, 0 },

#ifndef _COLLAPSE_SMALL_CHUNKS
	{ DebugHudItem::DHI_TEXT_CALLBACK, {TextCallback},		CPI_ GI_MEMCHUNK_TOTAL_MC_ODB				, {COL_MGREEN},  0, 0, 0, 0 },

	{ DebugHudItem::DHI_TEXT_CALLBACK, {TextCallback},		CPI_ GI_MEMCHUNK_TOTAL_MC_ARMY				, {COL_MGREEN},  0, 0, 0, 0 },
	{ DebugHudItem::DHI_TEXT_CALLBACK, {TextCallback},		CPI_ GI_MEMCHUNK_TOTAL_MC_LUA				, {COL_MGREEN},  0, 0, 0, 0 },
	{ DebugHudItem::DHI_TEXT_CALLBACK, {TextCallback},		CPI_ GI_MEMCHUNK_TOTAL_MC_ENTITY			, {COL_MGREEN},  0, 0, 0, 0 },
	{ DebugHudItem::DHI_TEXT_CALLBACK, {TextCallback},		CPI_ GI_MEMCHUNK_TOTAL_MC_MISC				, {COL_MGREEN},  0, 0, 0, 0 },

	{ DebugHudItem::DHI_TEXT_CALLBACK, {TextCallback},		CPI_ GI_MEMCHUNK_TOTAL_MC_AI				, {COL_MGREEN},  0, 0, 0, 0 },
	{ DebugHudItem::DHI_TEXT_CALLBACK, {TextCallback},		CPI_ GI_MEMCHUNK_TOTAL_MC_CAMERA			, {COL_MGREEN},  0, 0, 0, 0 },
	{ DebugHudItem::DHI_TEXT_CALLBACK, {TextCallback},		CPI_ GI_MEMCHUNK_TOTAL_MC_EFFECTS   		, {COL_MGREEN},  0, 0, 0, 0 },
	{ DebugHudItem::DHI_TEXT_CALLBACK, {TextCallback},		CPI_ GI_MEMCHUNK_TOTAL_MC_PROCEDURAL		, {COL_MGREEN},  0, 0, 0, 0 },
#endif

	{ DebugHudItem::DHI_TEXT_CALLBACK, {TextCallback},		CPI_ GI_MEMCHUNK_TOTAL_MC_OVERFLOW			, {COL_MGREEN},  0, 0, 0, 0 },

	// no point showing these
/*
	{ DebugHudItem::DHI_TEXT_CALLBACK, {TextCallback},		CPI_ GI_MEMCHUNK_TOTAL_MC_DEBUG				, {COL_MGREEN},  0, 0, 0, 0 },
	{ DebugHudItem::DHI_TEXT_CALLBACK, {TextCallback},		CPI_ GI_MEMCHUNK_TOTAL_MC_DEBUG_MEM_TRACKER	, {COL_MGREEN},  0, 0, 0, 0 },
*/
	{ DebugHudItem::DHI_TEXT, {"                ----      -------     -------      ------"},			CPI_ GI_IGNORE,		{COL_YELLOW}, 0, 0, 0, 0 },

	{ DebugHudItem::DHI_TEXT_CALLBACK, {TextCallback},		CPI_ GI_MEMCHUNK_TOTAL_XDDR					, {COL_MGREEN},  0, 0, 0, 0 },

	{ DebugHudItem::DHI_NONE, {0}, 0, {0}, 0, 0, 0 },	
};


DebugHudItem ProfilerMainMenu[] = 
{
	{ DebugHudItem::DHI_TEXT, {"Profiler"},		CPI_ GI_IGNORE,			{COL_GREEN}, 0, 0, 0, 0 },
	{ DebugHudItem::DHI_TEXT, {"--------"},		CPI_ GI_IGNORE,			{COL_BLUE}, 0, 0, 0, 0 },
	{ DebugHudItem::DHI_TEXT, {"Overall"},		CPI_ GI_MAIN_OVERALL,	{COL_WHITE}, 0, 0, 0, &ProfilerOverallMenu[0] },
	{ DebugHudItem::DHI_TEXT, {"Summary"},		CPI_ GI_MAIN_SUMMARY,	{COL_WHITE}, 0, 0, 0, &ProfilerSummaryMenu[0] },
	{ DebugHudItem::DHI_TEXT, {"Functions"},	CPI_ GI_MAIN_FUNCTIONS,	{COL_WHITE}, 0, &CPI_ DebugHUDFuncMenuSelectCallback, 0, &ProfilerFunctionMenu[0]  },
	{ DebugHudItem::DHI_TEXT, {"--------"},			CPI_ GI_IGNORE, 		{COL_BLUE}, 0, 0, 0, 0 },
	{ DebugHudItem::DHI_TEXT, {"Reset History"},	CPI_ GI_RESET,			{COL_RED}, 0, &CPI_ DebugHUDResetHistorySelect, 0, 0  },
	{ DebugHudItem::DHI_TEXT, {"--------"},			CPI_ GI_IGNORE, 		{COL_BLUE}, 0, 0, 0, 0 },
	{ DebugHudItem::DHI_TEXT, {"Total Memory"},		CPI_ GI_MAIN_TOTAL_MEMORY,	{COL_WHITE}, 0, 0, 0, &ProfilerTotalMemoryMenu[0]  },
	{ DebugHudItem::DHI_TEXT, {"Per Frame Memory"},	CPI_ GI_MAIN_PER_FRAME_MEMORY,	{COL_WHITE}, 0, 0, 0, &ProfilerPerFrameMemoryMenu[0]  },
	{ DebugHudItem::DHI_TEXT, {"Chunk Levels"},		CPI_ GI_MAIN_MEMCHUNK_TOTALS,	{COL_WHITE}, 0, 0, 0, &ProfilerTotalMemoryValuesMenu[0]  },
	
	
	{ DebugHudItem::DHI_TEXT, {"--------"},	CPI_ GI_IGNORE, 		{COL_BLUE}, 0, 0, 0, 0 },
	{ DebugHudItem::DHI_NONE, {0}, 0, {0}, 0, 0, 0 },	
};


#undef SFTxtCallback
#undef GraphCallback
#undef TextCallback
#undef CPI_

};

struct GatsoItem 
{ 
	int m_Index;  
	CGatso::GATSO_INSTANCE* m_Inst; 

	bool operator < ( const GatsoItem& Other )
	{
#ifdef _GATSO
		if( !m_Inst ) return true;
		if( !Other.m_Inst ) return false;
		// Change the order to get the top brass at the top
		return Other.m_Inst->GetInstantPercent() < m_Inst->GetInstantPercent();
#else
        UNUSED(Other);
        return true;
#endif
	}
};

bool CProfilerInternal::DebugHUDFuncMenuSelectCallback(  DebugHudItem*, bool bSelect )
{
#ifdef _GATSO
	if( bSelect )
	{
		// when selected build the function list
		DebugHudItem* pHudItem = &ProfilerFunctionMenu[0];
		pHudItem->uiColour = COL_WHITE;
		pHudItem++;
		pHudItem->uiColour = COL_BLUE;
		pHudItem++;
		pHudItem->uiColour = COL_ORANGE;
		pHudItem++;

		// Define and create the list
		ntstd::List<GatsoItem> obList;

		CGatso::GATSO_INSTANCE* gatIt = CGatso::Begin();
		int index = 1;

		while( gatIt != CGatso::End() )
		{
			GatsoItem obItem;
			obItem.m_Index = index;
			obItem.m_Inst = gatIt;
			obList.push_back( obItem );

			gatIt = CGatso::Next( gatIt );
			++index;
		}

		if (g_bSortGatso)
			obList.sort();

		int skipindex = 0;
		index = 1;

		for( ntstd::List<GatsoItem>::iterator obIt( obList.begin() ); obIt != obList.end(); ++obIt )
		{
			if( skipindex < CProfilerInternal::Get().m_StartGatso )
			{
				gatIt = CGatso::Next( gatIt );
				skipindex++;
				continue;
			}

			if( index >= (int)MAX_FUNCTIONS )
			{
				break;
			}

			pHudItem->eType = DebugHudItem::DHI_TEXT_CALLBACK;
			pHudItem->pTextCallback = CProfilerInternal::DebugHUDFunctionCallback;
			pHudItem->iIndex = (*obIt).m_Index; // the callback uses user data this is used to select which data to print
			pHudItem->pUserData = (*obIt).m_Inst;
			pHudItem->pHighLightedFunc = 0;
			pHudItem->pSelectedFunc = CProfilerInternal::DebugHUDEachFunctionSelectCallback;
			pHudItem->pChildMenu = &ProfilerFunctionSubMenu[0];
			++pHudItem;
			++index;
		}

		static const char* s_ScrollDown = "Scroll DOWN";
		pHudItem->eType = DebugHudItem::DHI_TEXT;
		pHudItem->pText = s_ScrollDown;
		pHudItem->iIndex = GI_SCROLL_DOWN;
		pHudItem->uiColour = COL_ORANGE;
		pHudItem->pHighLightedFunc = 0;
		pHudItem->pSelectedFunc = DebugHUDScrollCallback;
		pHudItem->pChildMenu = 0;
		++pHudItem;

		pHudItem->eType = DebugHudItem::DHI_NONE;
	} else
	{
		// do nothing
	}
#else
    UNUSED(bSelect);
#endif

	return true;
}
bool CProfilerInternal::DebugHUDEachFunctionSelectCallback( DebugHudItem* pItem, bool bSelect )
{
#ifdef _GATSO
	if( bSelect )
	{
		CGatso::GATSO_INSTANCE* pGatso = (CGatso::GATSO_INSTANCE*) pItem->pUserData;

		// wipe function list
		DebugHudItem* pHudItem = &ProfilerFunctionMenu[0];
		while( pHudItem->eType != DebugHudItem::DHI_NONE )
		{
			pHudItem->pUserData = 0;
			++pHudItem;
		}

		pHudItem = &ProfilerFunctionSubMenu[0];

		pHudItem->pUserData = pGatso;		// first item is name and instant percentage
		pHudItem++;
		pHudItem->pUserData = pGatso; // max
		pHudItem++;
		pHudItem->pUserData = pGatso; // avg
		pHudItem++;
		pHudItem->pUserData = pGatso; // second
		pHudItem++;
		pHudItem->pUserData = pGatso; // graph

		// create the actual function preload it with samples
		if( CProfilerInternal::Get().m_pobFunctionGraph )
		{
			NT_DELETE( CProfilerInternal::Get().m_pobFunctionGraph );
		}

		CProfilerInternal::Get().m_pobFunctionGraph = NT_NEW CGraph( GRAPH_TYPE_ROLLING );
		CProfilerInternal::Get().m_pobInstantSampleSet	= CProfilerInternal::Get().m_pobFunctionGraph->AddSampleSet( "INST", GRAPH_SAMPLES, COL_WHITE );
		CProfilerInternal::Get().m_pobMaxSampleSet		= CProfilerInternal::Get().m_pobFunctionGraph->AddSampleSet( "MAX", GRAPH_SAMPLES, COL_GREEN );
		CProfilerInternal::Get().m_pobAvgSampleSet		= CProfilerInternal::Get().m_pobFunctionGraph->AddSampleSet( "AVG", GRAPH_SAMPLES, COL_YELLOW );
		CProfilerInternal::Get().m_pobLastSecSampleSet	= CProfilerInternal::Get().m_pobFunctionGraph->AddSampleSet( "LASTSEC", GRAPH_SAMPLES, COL_ORANGE );

		CProfilerInternal::Get().m_pGraphedGatso = pGatso;
		int index = pGatso->m_iCurrentIndex+1;
		for( int i =0;i < CGatso::TOTAL_SAMPLES;i++)
		{
			CProfilerInternal::Get().m_pobInstantSampleSet->AddSample( pGatso->GetSample( index ) * 0.01f );
			CProfilerInternal::Get().m_pobMaxSampleSet->AddSample( pGatso->m_fMaxPercent * 0.01f );
			CProfilerInternal::Get().m_pobAvgSampleSet->AddSample( pGatso->GetSecondAverage(index) * 0.01f );
			int lastSecondIndex = (index / CGatso::SAMPLES_PER_SECOND);
			lastSecondIndex = lastSecondIndex * CGatso::SAMPLES_PER_SECOND;
			CProfilerInternal::Get().m_pobAvgSampleSet->AddSample( pGatso->GetSecondAverage(lastSecondIndex) * 0.01f );
			index++;
		}

		return true; // we aren't a sub menu strangely..
	} else
	{
		NT_DELETE( CProfilerInternal::Get().m_pobFunctionGraph );
		CProfilerInternal::Get().m_pobFunctionGraph = 0;

		// rebuild function list
		return DebugHUDFuncMenuSelectCallback( 0, true );
	}
#else
    UNUSED(bSelect); UNUSED(pItem);
    return true;
#endif
}


void CProfilerInternal::DumpGatsoToCSV()
{
#ifdef _GATSO
#if defined (PLATFORM_PC)

	char tmpbuf[128];
	char tmpbuf2[128];
	_strtime( tmpbuf );
	_strdate( tmpbuf2 );

	char buffer[ 1024];
	sprintf( buffer, "Profile_TIME%s_DATE%s.csv", tmpbuf, tmpbuf2 );
	for( unsigned int i=0;i < strlen(buffer); i++)
	{
		if( buffer[i] == ':' || buffer[i] == '/' )
		{
			buffer[i] = '_';
		}
	}

	// Excel and open office can't handle a file with lots of columns, so 
	// I have to rotate it to rows, this sucks elephant balls.

	int iNumGatso = 0;
	CGatso::GATSO_INSTANCE* countIt = CGatso::Begin();
	while( countIt != CGatso::End() )
	{
		iNumGatso++;
		countIt = CGatso::Next( countIt );
	}

	float* pBuffer = NT_NEW float[ iNumGatso * CGatso::TOTAL_SAMPLES ];
	float* pCurBuffer = pBuffer;

	FILE* fh = fopen( buffer, "w" );
	CGatso::GATSO_INSTANCE* gatIt = CGatso::Begin();
	while( gatIt != CGatso::End() )
	{
		fprintf( fh, "%s", gatIt->m_pcTag );

		for( int i=0;i < CGatso::TOTAL_SAMPLES;i++)
		{
			pCurBuffer[i] = gatIt->GetSample(gatIt->m_iCurrentIndex+i);
		}
		pCurBuffer = pCurBuffer + CGatso::TOTAL_SAMPLES;
		gatIt = CGatso::Next( gatIt );
		if( gatIt != CGatso::End() )
		{
			fprintf( fh, "," );
		} else
		{
			fprintf( fh, "\n" );
		}
	}

	pCurBuffer = pBuffer;

	// write CGatso::TOTAL_SAMPLES rows of data
	for( int i = 0;i < CGatso::TOTAL_SAMPLES;i++)
	{
		int gatNum = 0;
		// write a row of samples per gatso
		CGatso::GATSO_INSTANCE* gatIt = CGatso::Begin();
		while( gatIt != CGatso::End() )
		{
			fprintf( fh, "%f", pCurBuffer[gatNum * CGatso::TOTAL_SAMPLES] );
			gatIt = CGatso::Next( gatIt );
			if( gatIt != CGatso::End() )
			{
				fprintf( fh, "," );
			} else
			{
				fprintf( fh, "\n" );
			}
			gatNum++;
		}
		pCurBuffer++;
	}

	NT_DELETE_ARRAY( pBuffer );

	fclose( fh );

#else

	char buffer[ 1024 * 4 ];
	sprintf( buffer, "\n\n\n\n\n\n\nProfile Data:\n" );

	float highest_mean = 0.0f;
	float highest_standard_deviation = 0.0f;
	CGatso::GATSO_INSTANCE *gatso_with_highest_variance = NULL;

	CGatso::GATSO_INSTANCE *gatso = CGatso::Begin();
	while ( gatso != CGatso::End() )
	{
		float mean = 0.0f, standard_deviation = 0.0f;

		// Work out the mean and the standard deviation.
		for ( int32_t i=0;i<CGatso::TOTAL_SAMPLES;i++ )
		{
			float sample = gatso->GetSample( gatso->m_iCurrentIndex + i );
			mean += sample;
			standard_deviation += sample * sample;
		}
		mean /= float( CGatso::TOTAL_SAMPLES );
		standard_deviation /= float( CGatso::TOTAL_SAMPLES );
		standard_deviation -= mean * mean;
		standard_deviation = fsqrtf( standard_deviation );

		// Work out if this has the highest variance so far.
		if ( gatso_with_highest_variance == NULL || standard_deviation > highest_standard_deviation )
		{
			gatso_with_highest_variance = gatso;
			highest_mean = mean;
			highest_standard_deviation = standard_deviation;
		}

		// Does this gatso have any wildly out of fit samples?
		for ( int32_t i=0;i<CGatso::TOTAL_SAMPLES;i++ )
		{
			float sample = gatso->GetSample( gatso->m_iCurrentIndex + i );
			float delta_from_mean = fabsf( sample - mean );
			float num_sd = delta_from_mean / standard_deviation;

			// Ignore small samples.
			if ( sample > 5.0f && num_sd > 10.0f )
			{
				sprintf( buffer, "%s--------------------\n", buffer );
				sprintf( buffer, "%sGatso [%s] sample outside of confidence range!\n", buffer, gatso->m_pcTag );
				sprintf( buffer, "%ssample value: %f, num sd outside mean: %f\n", buffer, sample, num_sd );
				sprintf( buffer, "%smean: %f, standard deviation: %f\n", buffer, mean, standard_deviation );
			}
		}

		// Next gatso.
		gatso = CGatso::Next( gatso );
	}

	if ( gatso_with_highest_variance == NULL )
	{
		sprintf( buffer, "%sErm... No gatsos are present!\n", buffer );
	}
	else
	{
		sprintf( buffer, "%s--------------------\n", buffer );
		sprintf( buffer, "%sHighest standard deviation was with Gatso %s.\n", buffer, gatso_with_highest_variance->m_pcTag );
		sprintf( buffer, "%sStandard deviation was %f, with mean %f\n\n\n\n\n\n", buffer, highest_standard_deviation, highest_mean );
	}

	Debug::AlwaysOutputString( buffer );

#endif	// PLATFORM_PC
#endif	// _GATSO
}


/***************************************************************************************************
*
*	FUNCTION		CProfiler::CProfiler
*
*	DESCRIPTION		singleton construction
*
***************************************************************************************************/
CProfiler::CProfiler( void ) :
	m_bProfiling( false ),
	m_bShowMenu( false )
{
	NT_NEW CProfilerInternal();
}

/***************************************************************************************************
*
*	FUNCTION		CProfiler::~CProfiler
*
*	DESCRIPTION		cleanup
*
***************************************************************************************************/
CProfiler::~CProfiler( void )
{
	CProfilerInternal::Kill();

}


/***************************************************************************************************
*
*	FUNCTION		CProfiler::StopProfile
*
*	DESCRIPTION		Finish timing this frame
*
***************************************************************************************************/
void	CProfiler::StartProfile( void )
{
	ntAssert( !m_bProfiling );
	m_bProfiling = true;
	m_runningTimer.Start();
	m_pausedTimer.Reset();
}

/***************************************************************************************************
*
*	FUNCTION		CProfiler::StopProfile
*
*	DESCRIPTION		Finish timing this frame
*
***************************************************************************************************/
void	CProfiler::StopProfile( void )
{
	ntAssert( m_bProfiling );
	m_bProfiling = false;
	m_runningTimer.Stop();
	m_pausedTimer.StopNoPause();

	// get our desired refresh rate
	float fFrameIntervalSecs = 1.0f / GraphicsDevice::Get().GetGameRefreshRate();

	// this is the total amount of time covered by the profiler, minus any
	// interval within PROFILER_PAUSE / PROFILER_RESUME pairs.
	m_fRunningPercent = m_runningTimer.GetFrameTime();

	// this is the total amount of time within PROFILER_PAUSE / PROFILER_RESUME pairs.
	// we only pause the profiler around the device present (and hence the vsync call)
	m_fPresentPercent = m_pausedTimer.GetFrameTime();

	// total execution time measured by the profiler
	m_fProfileTotal = m_fRunningPercent + m_fPresentPercent;
	m_fFPS = 1.f / ( fFrameIntervalSecs * (m_fProfileTotal * 0.01f) );

	// this is the gatso'd CPU cost of the game update
	m_fGamePercent = CGatso::Retrieve( "ShellMain::Update" );

	// this is the gatso'd CPU cost of rendering, minus the time spent in our
	// device present call
	m_fCPURenderPercent = CGatso::Retrieve( "ShellMain::Render" );
	m_fCPURenderPercent -= m_fPresentPercent;
	m_fGPURenderPercent = (Renderer::Get().GetGPUFrameTime() / fFrameIntervalSecs) * 100.0f;

	// update graphs
	CProfilerInternal::Get().m_pRunningTime->AddSample( m_fRunningPercent * 0.01f );
	CProfilerInternal::Get().m_pPresentTime->AddSample( m_fPresentPercent * 0.01f );
	CProfilerInternal::Get().m_pTotalTime->AddSample( m_fProfileTotal * 0.01f );

	CProfilerInternal::Get().m_pGameTime->AddSample( m_fGamePercent * 0.01f );
	CProfilerInternal::Get().m_pCPURenderTime->AddSample( m_fCPURenderPercent * 0.01f );
	CProfilerInternal::Get().m_pGPURenderTime->AddSample( m_fGPURenderPercent * 0.01f );

	CProfilerInternal::Get().m_pVRAM->AddSample( Renderer::Get().m_Platform.GetMemoryUsage()  );
//	CProfilerInternal::Get().m_pRAM->AddSample( MemTracker::GetMemoryUsage()  );

	const Mem::MemStats& totalStats = Mem::GetMemStats();
	const Mem::MemStats& frameStats = Mem::GetFrameStats();

	// total uses a 192Mb scale
	CProfilerInternal::Get().m_pTotalBytesInUse->AddSample( totalStats.sTotal.iNumBytesInUse * (1.f/RAM_LIMIT) );
	CProfilerInternal::Get().m_pTotalMiscBytesInUse->AddSample( totalStats.sChunks[ Mem::MC_MISC].iNumBytesInUse * (1.f/RAM_LIMIT) );
	CProfilerInternal::Get().m_pTotalRSXMainBytesInUse->AddSample( totalStats.sChunks[ Mem::MC_RSX_MAIN_INTERNAL].iNumBytesInUse * (1.f/RAM_LIMIT) );
	CProfilerInternal::Get().m_pTotalGFXBytesInUse->AddSample( totalStats.sChunks[ Mem::MC_GFX].iNumBytesInUse * (1.f/RAM_LIMIT) );
	CProfilerInternal::Get().m_pTotalHavokBytesInUse->AddSample( totalStats.sChunks[ Mem::MC_HAVOK].iNumBytesInUse * (1.f/RAM_LIMIT) );
	CProfilerInternal::Get().m_pTotalXDDRTextureBytesInUse->AddSample( totalStats.sChunks[ Mem::MC_RSX_MAIN_USER].iNumBytesInUse * (1.f/RAM_LIMIT) );
	CProfilerInternal::Get().m_pTotalOverflowBytesInUse->AddSample( totalStats.sChunks[ Mem::MC_OVERFLOW].iNumBytesInUse * (1.f/RAM_LIMIT) );
	CProfilerInternal::Get().m_pTotalODBBytesInUse->AddSample( totalStats.sChunks[ Mem::MC_ODB ].iNumBytesInUse * (1.f/RAM_LIMIT) );

	// per frame uses a -0.5 to 0.5 megabyte scale
	CProfilerInternal::Get().m_pPerFrameBytesInUse->AddSample( (frameStats.sTotal.iNumBytesInUse * (1.f/(0.5f*Mem::Mb))) + 0.5f );
	CProfilerInternal::Get().m_pPerFrameMiscBytesInUse->AddSample( (frameStats.sChunks[ Mem::MC_MISC].iNumBytesInUse * (1.f/(0.5f*Mem::Mb))) + 0.5f );
	CProfilerInternal::Get().m_pPerFrameRSXMainBytesInUse->AddSample( (frameStats.sChunks[ Mem::MC_RSX_MAIN_INTERNAL].iNumBytesInUse * (1.f/(0.5f*Mem::Mb))) + 0.5f );
	CProfilerInternal::Get().m_pPerFrameGFXBytesInUse->AddSample( (frameStats.sChunks[ Mem::MC_GFX].iNumBytesInUse * (1.f/(0.5f*Mem::Mb))) + 0.5f );
	CProfilerInternal::Get().m_pPerFrameHavokBytesInUse->AddSample( (frameStats.sChunks[ Mem::MC_HAVOK].iNumBytesInUse * (1.f/(0.5f*Mem::Mb))) + 0.5f );
	CProfilerInternal::Get().m_pPerFrameXDDRTextureBytesInUse->AddSample( (frameStats.sChunks[ Mem::MC_RSX_MAIN_USER].iNumBytesInUse * (1.f/(0.5f*Mem::Mb))) + 0.5f );
	CProfilerInternal::Get().m_pPerFrameOverflowBytesInUse->AddSample( (frameStats.sChunks[ Mem::MC_OVERFLOW].iNumBytesInUse * (1.f/(0.5f*Mem::Mb))) + 0.5f );
	CProfilerInternal::Get().m_pPerFrameODBBytesInUse->AddSample( (frameStats.sChunks[ Mem::MC_ODB].iNumBytesInUse * (1.f/(0.5f*Mem::Mb))) + 0.5f );


#ifdef _GATSO
	if( CProfilerInternal::Get().m_pobFunctionGraph )
	{
		float fVal = CProfilerInternal::Get().m_pGraphedGatso->GetInstantPercent();

		CProfilerInternal::Get().m_pobInstantSampleSet->AddSample( fVal * 0.01f );

		CProfilerInternal::Get().m_pobMaxSampleSet->AddSample( 
			CProfilerInternal::Get().m_pGraphedGatso->m_fMaxPercent * 0.01f );

		CProfilerInternal::Get().m_pobAvgSampleSet->AddSample( 
			CProfilerInternal::Get().m_pGraphedGatso->GetFilteredPercent() * 0.01f );

		int lastSecondIndex = (CProfilerInternal::Get().m_pGraphedGatso->m_iCurrentIndex / CGatso::SAMPLES_PER_SECOND);
		lastSecondIndex = lastSecondIndex * CGatso::SAMPLES_PER_SECOND;
		CProfilerInternal::Get().m_pobLastSecSampleSet->AddSample( 
			CProfilerInternal::Get().m_pGraphedGatso->GetSecondAverage( lastSecondIndex ) * 0.01f );
	}
#endif
}

void	CProfiler::SetNameFilter(const char * pNameFilter) // zero value remove the name filter
{
	if(pNameFilter)
	{
		CProfilerInternal::m_nameFilter = ntstd::String(pNameFilter);
	}
	else
	{
		CProfilerInternal::m_nameFilter = ntstd::String();
	}
}

/***************************************************************************************************
*
*	FUNCTION		CProfiler::PrintScrn
*
*	DESCRIPTION		dump our info
*
***************************************************************************************************/
void	CProfiler::Display()		
{
	CInputKeyboard* pobKeyboard = CInputHardware::Get().GetKeyboardP();

	if ( pobKeyboard->IsKeyPressed( KEYC_G ) )
	{
#ifdef _PROFILING
		m_bShowMenu ^= true;
		if( m_bShowMenu )
		{
			DebugHUD::Get().UseMenu( &ProfilerMainMenu[0] );
		} else
		{
			DebugHUD::Get().UseMenu( 0 );
			return;
		}
#else
		CGatso::ToggleGraph();
		return;
#endif
	}

	if( m_bShowMenu == false )
	{
		return;
	}

#ifdef _GATSO

	if ( pobKeyboard->IsKeyPressed( KEYC_S, KEYM_CTRL | KEYM_ALT ) )
	{
		g_bSortGatso ^= true;
	}

	if( g_bSortGatso )
	{

		// force a resort and rebuild every 10 frames
		static int iSortGatsoCount = 0;
		if( iSortGatsoCount >= 10 )
		{
			CProfilerInternal::Get().DebugHUDFuncMenuSelectCallback( 0, true );
			iSortGatsoCount = 0;
		} else
		{
			++iSortGatsoCount;
		}
	}

	// zoom function graph to 2 * MAX
	if ( pobKeyboard->IsKeyPressed( KEYC_Z, KEYM_CTRL | KEYM_ALT ) )
	{
		if( CProfilerInternal::Get().m_pobFunctionGraph )
		{
			CProfilerInternal::Get().m_pobFunctionGraph->SetYAxis( 0, CProfilerInternal::Get().m_pGraphedGatso->m_fMaxPercent * 2 * 0.01f, 10 );
		}
	}

	// log data to CSV
	if ( pobKeyboard->IsKeyPressed( KEYC_L, KEYM_CTRL | KEYM_ALT ) )
	{
		CProfilerInternal::Get().DumpGatsoToCSV();
	}
	// reset history
	if ( pobKeyboard->IsKeyPressed( KEYC_R, KEYM_CTRL | KEYM_ALT ) )
	{
		CGatso::ResetGatsoHistory();
	}
#endif

	return;
}

#endif

/***************************************************************************************************
*	 
*	CLASS			CTimedBlock
*
*	DESCRIPTION		Hangs in constructor untill input time has passed
*
***************************************************************************************************/
CTimedBlock::CTimedBlock( float fBlockTime )
{
	int64_t lStart, lEnd;
	lStart = CTimer::GetHWTimer(); 
	float fPeriod = CTimer::GetHWTimerPeriod();

	do { lEnd = CTimer::GetHWTimer(); }
	while ( (((float)(lEnd - lStart))*fPeriod) < fBlockTime );		
}


/***************************************************************************************************
*	 
*	CLASS			CMicroTimer
*
*	DESCRIPTION		Useful timer for 1-stop timing needs, ideal for bracketing function calls.
*
***************************************************************************************************/
CMicroTimer::CMicroTimer( void )				
{ 
	m_lStart = _LLONG_MAX; 
	m_lStop = -_LLONG_MAX; 
}

void CMicroTimer::Start( void )	
{ 
	m_lStop = -_LLONG_MAX; 
	m_lStart = CTimer::GetHWTimer(); 
}
void CMicroTimer::Stop( void )	
{ 
	m_lStop = CTimer::GetHWTimer(); 
	Check();  
}

int	CMicroTimer::GetTicks( void ) const 
{ 
	Check(); 
	return (int)(m_lStop - m_lStart); 
}
float	CMicroTimer::GetSecs( void ) const 
{ 
	Check(); 
	return ((float)(m_lStop - m_lStart) * CTimer::GetHWTimerPeriod()); 
}
float	CMicroTimer::GetMilliSecs( void ) const 
{ 
	return (GetSecs() * 1000.0f); 
}
float	CMicroTimer::GetMicroSecs( void ) const 
{ 
	return (GetSecs() * 1000000.0f); 
}
float	CMicroTimer::GetNanoSecs( void ) const 
{ 
	return (GetSecs() * 1000000000.0f); 
}
float	CMicroTimer::GetFramePercent( void ) const 
{ 
	Check(); 
	return ((float)(m_lStop - m_lStart) * CTimer::GetHWTimerPeriodFrame()) * 100.0f; 
}
void CMicroTimer::Check( void ) const 
{ 
	ntAssert(m_lStop >= m_lStart); 
}


//!**************************************************************************************************
//!
//! CEstimateTimer
//!
//!**************************************************************************************************
// create some smoothers, setup scale factor and timers
CEstimateTimer::CEstimateTimer( float fSrcClock, float fTargetClock )
{
	m_fScaleFact = fSrcClock/fTargetClock;
	m_lThisFrame = -_LLONG_MAX;
	m_bRunning = false;
	m_bPaused = false;
	m_fCumFrame = 0.0f;
	m_fCumTime = 0.0f;
}

void	CEstimateTimer::Start( void )
{
	Check();
	ntAssert_p( m_lThisFrame <= CTimer::GetHWTimer(), ("Only use CEstimateTimer::Start() once per frame") );
	m_lThisFrame = CTimer::GetHWTimer();
	m_fCumFrame = 0.0f;
	m_fCumTime = 0.0f;
	m_bRunning = true;
	m_obMTimer.Start();
}

// reset for this frame
void	CEstimateTimer::Reset( void )
{
	Check();
	ntAssert_p( m_lThisFrame <= CTimer::GetHWTimer(), ("Only use CEstimateTimer::Start() once per frame") );
	m_lThisFrame = CTimer::GetHWTimer();
	m_fCumFrame = 0.0f;
	m_fCumTime = 0.0f;
	m_obMTimer.Start();
}

// stop for this frame
void	CEstimateTimer::Stop( void )
{
	Pause();

	m_bRunning = false;
	m_bPaused = false;
}

// pause this frames timing
void	CEstimateTimer::Pause( void )
{
	m_obMTimer.Stop();

	ntAssert_p( m_bRunning, ("CEstimateTimer not running") );
	ntAssert_p( !m_bPaused, ("CEstimateTimer is paused") );

	m_fCumFrame += m_obMTimer.GetFramePercent();
	m_fCumTime += m_obMTimer.GetMicroSecs();

	m_bPaused = true;
}


void	CEstimateTimer::StopNoPause( void )
{
	if (!m_bRunning) return;
	if (!m_bPaused) Pause();

	m_bRunning = false;
	m_bPaused = false;
}

// resume this frames timing
void	CEstimateTimer::Resume( void )
{
	ntAssert_p( m_bPaused, ("CEstimateTimer is not paused") );
	m_bPaused = false;

	m_obMTimer.Start();
}
void	CEstimateTimer::ResumeOrStart( void ) 
{ 
	m_bPaused ? Resume() : Start(); 
}

void	CEstimateTimer::PrintScrn(float fX, float fY, const char* pcString, uint32_t dwCol, bool bShowEstimate ) const
{
#ifndef _GOLD_MASTER
	ntAssert(pcString);
	g_VisualDebug->Printf2D(fX, fY,		dwCol, 0, "%s%.1f%%",	pcString, GetFrameTime() );
	g_VisualDebug->Printf2D(fX, fY+12.0f,	dwCol, 0, "%s%.1fms",	pcString, GetMilliSecs() );
	g_VisualDebug->Printf2D(fX, fY+24.0f,	dwCol, 0, "%s%.1fus",	pcString, GetMicroSecs() );

	if (bShowEstimate)
	{
		g_VisualDebug->Printf2D(fX, fY+40.0f,	dwCol, 0, "%s%.1f%% (EST)",	pcString, GetEstFrameTime() );
		g_VisualDebug->Printf2D(fX, fY+52.0f,	dwCol, 0, "%s%.1fms (EST)",	pcString, GetEstMilliSecs() );
		g_VisualDebug->Printf2D(fX, fY+64.0f,	dwCol, 0, "%s%.1fus (EST)",	pcString, GetEstMicroSecs() );
	}
#endif
}

float	CEstimateTimer::GetFrameTime( void ) const 
{ 
	Check(); 
	return m_fCumFrame; 
}
float	CEstimateTimer::GetMilliSecs( void ) const 
{ 
	Check(); 
	return m_fCumTime / 1000.0f; 
}
float	CEstimateTimer::GetMicroSecs( void ) const 
{ 
	Check(); 
	return m_fCumTime; 
}

float	CEstimateTimer::GetEstFrameTime( void ) const 
{ 
	Check(); 
	return GetFrameTime() * m_fScaleFact; 
}
float	CEstimateTimer::GetEstMilliSecs( void ) const 
{ 
	Check(); 
	return GetMilliSecs() * m_fScaleFact; 
}
float	CEstimateTimer::GetEstMicroSecs( void ) const 
{ 
	Check(); 
	return GetMicroSecs() * m_fScaleFact; 
}

void	CEstimateTimer::Check( void ) const 
{ 
	ntAssert_p(!m_bRunning,("CEstimateTimer still running")); 
	ntAssert_p(!m_bPaused, ("CEstimateTimer still paused")); 
}
