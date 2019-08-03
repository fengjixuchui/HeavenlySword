/***************************************************************************************************
*
*	DESCRIPTION		The most basic unit of an army is the Grunt.
*
*	NOTES
*
***************************************************************************************************/

#ifndef GRUNT_H_
#define GRUNT_H_

//**************************************************************************************
//	Includes files.
//**************************************************************************************
#if !defined( ARMY_PPU_SPU_H_ )
#include "army/army_ppu_spu.h"
#endif

//**************************************************************************************
//	Forward declarations.
//**************************************************************************************

//**************************************************************************************
//	Typedefs.
//**************************************************************************************

// 16 states max (4 whole bits!!!)
enum GruntMajorState
{
	GMS_NORMAL		= 0,
	GMS_SCARED		= 1,
	GMS_DEAD		= 2,
	GMS_DYING		= 3,
	GMS_DIVING		= 4,
	GMS_REAL_DUDE	= 5,
	GMS_GETTING_UP	= 6,
	GMS_KOED		= 7,
	GMS_BLOCK		= 8,
	GMS_RECOIL		= 9,

	MAX_GRUNT_STATES	= (1 << MAX_GRUNT_MAJOR_STATE_BITS)
};

// 2 bits
enum GruntRenderType
{
	GRT_SPRITE		= 0,
	GRT_REAL_DUDE	= 1,
	GRT_RENDERABLE	= 2,
	GRT_NOCHANGE	= 3, // not a real state used as a marker 
	MAX_GRUNT_RENDER_TYPES = (1 << MAX_GRUNT_RENDER_TYPE_BITS),
};

// 16 anim states (4 bits)
enum GruntAnimState
{
	GAM_IDLE		= 0,
	GAM_WALK		= 1,
	GAM_RUN			= 2,
	GAM_DIVE_LEFT	= 3,
	GAM_DIVE_RIGHT	= 4,
	GAM_KO_BLUNT	= 5,
	GAM_STAGGER		= 6,
	GAM_DEAD		= 7,
	GAM_GETTING_UP	= 8,
	GAM_KO_WAKE		= 9,
	GAM_TAUNT		= 10,
	GAM_BLOCK		= 11,
	GAM_RECOIL		= 12,


	MAX_GRUNT_ANIM	= (1 << MAX_GRUNT_ANIM_STATE_BITS),
};

//------------------------------------------------------
//!
//! Grunt are split into 2 bits, game state and render
//! state, this is game state. A compressed 2D position
//! and minimal other info, as we potentially require
//! a fair few of these in SPU ram at the same time
//!
//------------------------------------------------------
struct GruntGameState
{
	uint32_t		m_ChunkIndex		: MAX_BATTLEFIELD_CHUNKS_BITS;	//!< which chunk this grunt is in				10 total 10 bits
	uint32_t		m_GruntID			: MAX_GRUNTS_BITS;				//!< unique idenfifier for this grunt			12 total 22 bits
	uint32_t		m_GruntMajorState	: MAX_GRUNT_MAJOR_STATE_BITS;	//!< what this grunt is doing					4 total 26 bits
	uint32_t		m_UnitID			: MAX_UNITS_PER_BATTALION_BITS;	//!< which unit of the battalion				3 total 29 bits 
	uint32_t		m_RenderType		: MAX_GRUNT_RENDER_TYPE_BITS;	//!< which type we currently are				2 total 31 bits
	uint32_t		m_SelfExplode		: 1;							//!< have we just exploded a bomb				1 bits total 4 bytes

	int16_t			m_CurHeight;										//!< this is updated by the heightfield system				total 8 bytes

	BF_Position		m_Position;										//!< 2D position											total 12 bytes
	BF_Orientation	m_Orientation;									//!< orientation											total 14 bytes					

	uint16_t		m_AnimPlayingBits;								//!< one bit per anim state if currently playing
	uint16_t		m_AnimToPlayBits;								//!< one bit per anim to start the anim playing on the PPU

	uint16_t		m_BattalionID		: MAX_BATTALIONS_BITS;			//!< which battalion we belong to					6 total 6 bits
	uint16_t		m_RenderStateChangeCounter : MAX_RENDERSTATECHANGECOUNTER_BITS;	//!< disallow a render state change for a few frames 5 total 11
	uint16_t		m_dummy : 16 - MAX_BATTALIONS_BITS - MAX_RENDERSTATECHANGECOUNTER_BITS;										//!<												5 total 16 bits

	BF_Position		m_POI;		//!< a point of interest

	int16_t			m_BattalionOffsetX;	// and offset (so not a BF_Position)
	int16_t			m_BattalionOffsetY;

	BF_Position		m_IntendedPos;		//!< 

	uint16_t		m_EventRadius;
};

//------------------------------------------------------
//!
//! Just the state required to animated and render a grunt
//! usually much large than the game state and fewer are
//! needed in SPU ram at the same time
//! Note we copy state to ease cache pressure on the PPU bits
//!
//------------------------------------------------------
struct GruntRenderState
{
	CPoint		m_Position;					// 16 bytes compress?
	CQuat		m_Orientation;				// 16 bytes compress

	uint32_t			m_UnitType			: 8;							//!< 8 total 8bits 
	uint32_t			m_GruntID			: MAX_GRUNTS_BITS;				//!< 12 total 20
	uint32_t			m_iDistToCam		: 8;							//!< to improve the mesh/sprite distrubut (0xFF = far)8 total 28 bits
	uint32_t			m_RenderType		: MAX_GRUNT_RENDER_TYPE_BITS;	//!< which type we currently are					  2 total 30 bits
	uint32_t			m_DesiredRenderType	: MAX_GRUNT_RENDER_TYPE_BITS;

	uint32_t			m_iDistToPlayer		: 8;							//<! distance for this grunt from the player		  8 total 8 bits
	uint32_t			m_GruntMajorState	: MAX_GRUNT_MAJOR_STATE_BITS;	//!< what this grunt is doing copied here for speed   4 total 12 bits
	uint32_t			m_bRealDudeAllocFail : 1;							//!< used to not remove a renderable				  1 total 13 bits
	uint32_t			m_bDontRealDude		 : 1;
	uint32_t			m_dummy2			: 18;
	
	uint32_t			m_dummy[2];	// alignment space

}; // total 48 bytes





// prototype for a function used on SPU
int QsortGruntComparator( const void* a, const void* b );


#endif	// !GRUNT_H_

