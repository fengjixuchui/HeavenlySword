//------------------------------------------------------
//
// \file army/army_section.h
// This contains a abstract base class the all army section derive from 
//
//------------------------------------------------------

#ifndef ARMY_SECTION
#define ARMY_SECTION

//------------------------------------------------------
//	Includes files.
//------------------------------------------------------

#include "gfx/material.h"

#if !defined( ARMY_PPU_SPU_H_ )
#include "army/army_ppu_spu.h"
#endif
#if !defined( BATTLEFIELD_H_ )
#include "army/battlefield.h"
#endif
//------------------------------------------------------
//	Forward declarations.
//------------------------------------------------------
struct General;
struct Battlefield;
struct BattlefieldHeader;
struct BattlefieldEvent;
struct Battalion;
struct GruntGameState;
struct GruntRenderState;
struct Unit;
struct HeightfieldFileHeader;
class ArmyBattlefield;
class ArmyBattalion;
struct DependencyCounter;
struct ExecSPUJobAdder;
class ArmyUnitParameters;
class ArmyGenericParameters;
class ArmyRenderable;
class WeaponDef;
class Template_Flag;
class DataObject;

//------------------------------------------------------
//!
//! An abstract class that actual army sections derive
//! from, it auto cleans up all the pointer it contains
//! and calls updates etc. It allows each section to 
//! contain just the creation and section level logic in
//! C++
//!
//------------------------------------------------------
class ArmySection
{
public:
	ArmySection();
	virtual ~ArmySection();

	// usually per section just overload Update to do there stuff
	virtual void Update( float fTimeStep );
	// we want this to be done in when we can 
	virtual void UpdateKickSPUs( float fTimeStep );
	virtual void Render() = 0;

	virtual void AddEvent( BATTLEFIELD_EVENT_TYPE type, const CPoint& pnt, float radius, uint32_t iParam0, uint32_t iParam1, uint32_t iUpdateStartDec = 100000, uint32_t iUpdateData = 0 );

	virtual void ProcessGlobalEvent( const CHashedString obGlobalEvent ) = 0;

	void ToggleDebugMode()
	{
		// 0 = no debug
		// 1 = body count + SPU Lines (on for all debug)
		// 2 = obstacles adn event circles (rest of mutally exclusive with each otheer
		// 3 = state text
		m_iDebugMode++;
		if( m_iDebugMode > 3 )
		{
			m_iDebugMode = 0;
		}
	}

	unsigned int GetBodyCount();

	

	virtual void SetCamera( const CPoint& pos, const CDirection& dir );

	void ForceMaterials( CMaterial const* pobMaterial );

	// Interfaces needed for army audio

	CPoint GetPlayerPosition () { return m_PlayerPos; }
    int GetUnitsAlive (const CPoint& obPosition, float fRadius); // Get the number of units that are still alive at a given position
	Battlefield* GetBattlefield () { return m_pBattlefield; }
	const ArmyBattlefield* GetArmyArena() { return m_pArmyArena; }

protected:
	typedef ntstd::Vector<GruntGameState> TempGruntVector;

	const ArmyBattlefield* m_pArmyArena;

	uint32_t m_iDebugMode;

	// grunt must have been sorted before the battlefield update can run
	DependencyCounter*	m_pSortDependency;

	// Grunts must have finished updating before they're rendered.
	DependencyCounter*	m_pRenderDependency;

	// all battalions must have finished before grunt logic
	DependencyCounter*	m_pBattalionDependency;

	// here so we can share the SPU task logic between sections
	Battlefield*	m_pBattlefield;

	ExecSPUJobAdder*	m_pArmyAdder;
	ExecSPUJobAdder*	m_pRenderAdder;

	unsigned int		m_iNumEvents;
	BattlefieldEvent*	m_pEvents;
	BattlefieldEventUpdate*		m_EventUpdates[ MAX_BATTLEFIELD_EVENTS ];	//!< real memory 

	// sound handles
	unsigned int		m_uiSoundRocketFireID;
	unsigned int		m_uiSoundExplosionID;

	void InstallWelderHookup();
	void UpdateFromWelder();
	ArmyUnitParameters* m_pArmyUnitParameters[ MAX_UNIT_TYPES ];

	ArmyGenericParameters* m_pArmyGenericParameters;

	// given a ordered line Battalion parameters this automated all the units and grunt construction
	void FillInOrderedLineBattalion(	const ArmyBattalion* pBattalionDef, 
										const BattlefieldHeader* pBattlefieldHeader, 
										Battalion* pBattalion, 
										TempGruntVector& grunts );
	void FillInUnOrderedSquareBattalion(	const ArmyBattalion* pBattalionDef, 
											const BattlefieldHeader* pBattlefieldHeader, 
											Battalion* pBattalion, 
											TempGruntVector& grunts );

	void CreatePreBattalionDMAStructures( BattlefieldHeader& battlefieldHeader, HeightfieldFileHeader*& pHeightfieldData );
	void CreatePostBattalionDMAStructures( const BattlefieldHeader& battlefieldHeader, HeightfieldFileHeader* pHeightfieldData, const TempGruntVector& grunts );

	void PlayAnimation( GruntRenderState* pRenderGrunt, ArmyRenderable* pRenderable );

	float CalculuteRadiusInBFSpace( const float radius );

	void BatchUpdateAnims( const float fTimeStep );
	void AllocateRenderables( const float fTimeStep );

	// default battalion fallback
	void CreateDefaultBattalion(	const ArmyBattalion* pBattalionDef, 
									const BattlefieldHeader* pBattlefieldHeader, 
									Battalion* pBattalion, TempGruntVector& grunts );


	void UpdatePreviousFrameWork( const float fTimeStep );
	bool m_bFirstFrameDone;

	//! SLOW do at construct time and store the index -1 not found
	int32_t GetObstacleIndexFromName( const CHashedString& str );

	CPoint		m_CameraPos;
	CDirection	m_CameraFacing;
	CPoint		m_PlayerPos;
	float		m_CameraRadius;
	float		m_PlayerRadius;

	void AllocateRenderable( GruntGameState* pGrunt, GruntRenderState* pCurRenderGrunt );
	void AllocateRealDude( ArmyRenderable* pRenderable, GruntGameState* pGrunt, GruntRenderState* pCurRenderGrunt );
	void DeallocateRealDude( ArmyRenderable* pRenderable, GruntGameState* pGrunt, GruntRenderState* pCurRenderGrunt );
	void DeallocateRenderable( ArmyRenderable* pRenderable,  GruntGameState* pGrunt, GruntRenderState* pCurRenderGrunt );

	struct ArmyFlagEntry
	{
		uint16_t		m_GruntID;
		Template_Flag*	m_pFlag;
		ArmyRenderable*	m_pFlagPole;
	};
	typedef ntstd::Vector<ArmyFlagEntry> ArmyFlagContainer;

	ArmyFlagContainer	m_ArmyFlags;
	uint8_t*			m_pFlagArray;

	void CreateFlagForGrunt( uint16_t gruntID );
	void CreateStaticAISegments( DataObject* pConFieldDO );

};


// evil little
extern bool g_bArmyReloadElves;

// a helper function that may be removed eventually prodouces a chunked battlefield from float data
extern void CreateBattlefield( const BattlefieldHeader* pHeader, const float* pFloats, Battlefield* pBattlefield );
extern HeightfieldFileHeader* GenerateHeightfield( const CPoint &origin, const CDirection &diagonal, unsigned int width, unsigned int height );


#endif	// !ARMY_SECTION
