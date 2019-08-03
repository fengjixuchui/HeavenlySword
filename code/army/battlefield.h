/***************************************************************************************************
*
*	DESCRIPTION		A Battlefield holds a heightfield and collision objects, that represent the
*					playfield an army works in
*
*	NOTES
*
***************************************************************************************************/

#ifndef BATTLEFIELD_H_
#define BATTLEFIELD_H_

//**************************************************************************************
//	Includes files.
//**************************************************************************************
#if !defined( ARMY_PPU_SPU_H_ )
#include "army/army_ppu_spu.h"
#endif

//**************************************************************************************
//	Forward declarations.
//**************************************************************************************
class DMABuffer;
struct Unit;
struct Battlefield;
struct Battalion;
struct BattalionArray;
struct BattlefieldChunk;
class ArmyRenderable;
struct GruntRenderState;
class ArmyRenderPool;

class ArmyBattalionSoundDef;
class SpawnDef;

//**************************************************************************************
//	Typedefs.
//**************************************************************************************



// 3 qwords (48 bytes) per segment
struct ArmyStaticAISegment
{
	BF_Position P0, P1;
//	float Nx, Nz, Dx, Dz;
//	float Cx, Cz, dummy0, dummy1;
};


struct BattlefieldEvent
{
	uint32_t m_iType : NUM_BFE_BITS;						// type (4)
	uint32_t m_iRadius : MAX_BFE_RADIUS_BITS;				// radius (15)
															// total 4 bytes

	BF_Position	m_EventLocation;							// total 8 bytes
	uint32_t	m_iParam0;									// total 12 bytes
	uint32_t	m_iParam1;									// total 16 bytes.
};

//! each frame this set to say one, than an SPU can atomicly decrement
//! alignment both to stop redudement atomic clashes and dma speed
struct BattlefieldEventUpdate
{
	int32_t		m_iDec;										// some form of counter
	uint32_t	m_iData;								// some payload (for bazooka hash name of shot)
	int32_t		m_iOrigDec;								// simple bit of double buffering 
	uint32_t	m_iOrigData;							// simple bit of double buffering
}  __attribute__ ( (aligned( 128 )) );

struct BattlefieldObstacle
{
	BF_Position	m_Location;								// total 8 bytes
	uint16_t	m_Radius	: MAX_BFE_RADIUS_BITS;
	uint16_t	m_Attract	: 1;						// total 12 bytes
};

struct GruntArray
{
	uint32_t	m_iSize;

	uint16_t	m_iNumGrunts;
	uint16_t	m_padd[5]; // the end of the structure must be 16 byte aligned 

}; // followed by m_iNumGrunts * GruntGameState + m_iNumGrunts * uint16_t

#if !defined(_RELEASE )
struct SPU_DebugLine
{
	CPoint	a;
	CPoint	b;

	uint32_t col;
	uint32_t m_padd32[3];
};
#define MAX_ARMY_SPU_DEBUG_LINES 1000
#endif

// this has to 128 bytes long, no more, no less
struct BattlefieldInfo
{
	uint32_t	m_iTotalDeadCount;	//!< how many dead people how over the whole battle (body count)
	uint32_t	m_iCurDead;			//!< how many dead people at the moment (respawn makes it less than total)
	uint32_t	m_iExplodeCount;	//!< how many people have 'exploded' (bombermen logic)

	uint8_t		m_padd[116];
} __attribute__( (aligned( 128 )) );


// okay the reason for the odd crazy bad shit mad seeming random numbers
// is that the structure must be exactly 2K the rest works backwords from there
#define MAX_ARMY_REAL_DUDE_ALLOCS_PER_FRAME			32
#define MAX_ARMY_REAL_DUDE_DEALLOCS_PER_FRAME		32
#define MAX_ARMY_RENDERABLE_ALLOCS_PER_FRAME		476
#define MAX_ARMY_RENDERABLE_DEALLOCS_PER_FRAME		476
struct ArmyGruntRenderAllocator
{
	// how many people should be changed into a real dude this frame
	uint16_t	m_iNumRealDudeToAlloc;
	// how many people should be changed from a real dude this frame
	uint16_t	m_iNumRealDudeToDeAlloc;
	// how many people should be changed into a renderable this frame
	uint16_t	m_iNumRenderablesToAlloc;
	// how many people should be changed from a renderable this frame
	uint16_t	m_iNumRenderablesToDeAlloc;

	// the render grunt index (NOT grunt ID, recover that from, the render grunt!!)
	// of the people references in the counts above
	uint16_t	m_RealDudeAllocIds[ MAX_ARMY_REAL_DUDE_ALLOCS_PER_FRAME ];
	uint16_t	m_RealDudeDeAllocIds[ MAX_ARMY_REAL_DUDE_DEALLOCS_PER_FRAME ];
	uint16_t	m_RenderableAllocIds[ MAX_ARMY_RENDERABLE_ALLOCS_PER_FRAME ];
	uint16_t	m_RenderableDeAllocIds[ MAX_ARMY_RENDERABLE_DEALLOCS_PER_FRAME ];

} __attribute__( (aligned( 128 )) );

//------------------------------------------------------
//!
//! The main data about the battlefield, is global data
//! available to all SPU army code.
//! must be 1KB or less
//!
//------------------------------------------------------
struct BattlefieldHeader
{
	float		m_accumTime;		//!< how long has passed on the battlefield in 30hz frames (so 30x more than the main game!!!)
	float		m_deltaTime;		//!< how long is this frame in 30hz frames (so 30x more than the main game!!!)

	// how big is the whole battlefield 
	uint16_t	m_TotalWidth;		//!< width in pixels of the heightfield
	uint16_t	m_TotalHeight;		//!< height in pixels of the heightfield
	uint16_t	m_WidthInChunks;	//!< X chunkId = N * 1.f / (65535 / iWidthInChunks )
	uint16_t	m_HeightInChunks;	//!< size in chunks

	uint8_t		m_iNumStaticSegments[MAX_STATIC_AI_POLYGONS];	//!< number of segments to the static AI fence

	uint8_t		m_iNumStaticPolygons;	//! must be < than MAX_STATIC_AI_POLYGONS
	uint8_t		m_iNumEvents;			//!< number of events on the field
	uint8_t		m_iNumObstacles;		//!< how many obstacles to avoid
	uint8_t		m_iDummyS[2];			//!< dummy

	uint16_t	m_iGlobalEventFlags;	//!< bit flags that can be read and altered externally

	CPoint		m_WorldTopLeft;			//!< the top left point in world space of the heightfield
	float		m_WorldHoriz;				//!< the horizontal extent of the hf (only needs to be a float...)
	float 		m_WorldVert;				//!< the vertical extent of the hf
	float		m_WorldHeight;				//!< height extents of the battlefield

	uint16_t	m_iStagingPostLine;		//!< currently we assume one staging post per level
	uint16_t	m_iPlayerTrackWallInset;	//!< how much to inset off the hard wall for battlion player tracking
	uint16_t	m_iPlayerTrackWallMultipler;	//!< how far player tracking should walk (the global multiplier)
	uint16_t	m_iPlayerTrackWallRandomishFactor;	//!< used to add some randomidity to the player tracking

	// static AI segments
	static const unsigned int MAX_STATIC_SEGMENTS = 48; // must be a even number for alignment purposes...
	ArmyStaticAISegment m_StaticSegments[MAX_STATIC_SEGMENTS];

	// events on the field
	BattlefieldEvent	m_Events[ MAX_BATTLEFIELD_EVENTS ];
	// ea address of where the associated update structure is (for spu to ppu atomic response)
	BattlefieldEventUpdate*	m_eaEventUpdates[ MAX_BATTLEFIELD_EVENTS ];

	CPoint		m_CameraPos;
	CDirection	m_CameraFacing;
	CPoint		m_PlayerPos;
	float		m_CameraRadius;
	float		m_PlayerRadius;
	BF_Position	m_PlayerBFPos;
	uint16_t	m_PlayerBFRadius;

	// an atomically updateble structure for game play stats..
	BattlefieldInfo*	m_eaInfo;

	// I really need to chunk these up NOW!!!!
	BattlefieldObstacle	m_Obstacles[ MAX_BATTLEFIELD_OBSTACLES ];

	ArmyGruntRenderAllocator*	m_eaGruntRenderAllocator;	//!< DMA up and down
	uint16_t					m_iMaxPeeps;
	uint16_t					m_iMaxAIDudes;

#if !defined(_RELEASE)
	uint32_t*			m_iNumDebugLines;			//!< point that must be atomically updated
	SPU_DebugLine*		m_pDebugLineBuffer;			//!< 
#endif
};

// note this is a lie, the pChunk array will be allocated ONLY enough for m_iNumChunk and 
// not MAX_BATTLEFIELD_CHUNKS!!!!
struct BattlefieldChunksHeader
{
	uint16_t			m_iNumChunks;
	BattlefieldChunk*	m_pChunks[MAX_BATTLEFIELD_CHUNKS];
};

#if !defined( __SPU__ )





//------------------------------------------------------
//!
//! The battlefield subdives the army section, it is
//! both a ai heirachy and the actual collision, path
//! test system. 
//! Never goes onto an SPU
//!
//------------------------------------------------------
struct Battlefield
{
	BattlefieldHeader*			m_pHeader;				//!< must be DMA-able
	BattlefieldChunksHeader*	m_pChunksHeader;		//!< must be DMA-able

	GruntArray*					m_pGrunts;						//!< all the grunts in the battlefield live here, must be DMA-able!
	BattalionArray*				m_pBattalions;

	GruntRenderState*			m_pRenderDestBuffer;						//!< must be DMA-able
	ArmyRenderable**			m_pRenderables;								//!< for each grunt either null or renderable render type
	ArmyRenderPool*				m_pRenderPools[ MAX_UNIT_TYPES ];			//!< pools of actual renderables

};

#endif


// bring in the inline functions
#include "army/battlefield.inl"

#endif	// !BATTLEFIELD_H_
