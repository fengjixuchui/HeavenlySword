#ifndef _EFFECTCHAIN_H_
#define _EFFECTCHAIN_H_

#include "hair/onetomanylink.h"
#include "core/valueandmax.h"
#include "core/rotationnalindex.h"
#include "core/bitmask.h"
#include "hair/lifeanddeath.h"
#include "game/commandresult.h"



class ChainGlobalDef;
class ChainRessource;
class HairStyleFromWelder;
class HairStyleFromMaya;
class OneChain;
class ChainElem;
class HairSphereSetDef;
class ObjectContainer;
class XmlFileInterface;
class CEntity;
class LuaAttributeTable;

namespace HairConstant
{
	static const int HAIR_MEMORY_SIZE = 3;
	static const int HAIR_CURRENT = HAIR_MEMORY_SIZE-1;
	static const int HAIR_BEFORE = HAIR_MEMORY_SIZE-2;
	static const int HAIR_EVENBEFORE = HAIR_MEMORY_SIZE-3;
} // end of namespace HairConstant



//--------------------------------------------------
//!
//!	chain ressource, managing style, collision and hair style instance.
//!
//--------------------------------------------------

class ChainRessource: public Singleton<ChainRessource>
{
public:
	
	// rotationnal index (just an easy way to use a modulo index on an array)
	typedef RotationnalIndex<int,HairConstant::HAIR_MEMORY_SIZE> RotIndex;
	
	// mask def
	typedef enum
	{
		F_INITME = BITFLAG(0),     // first draw, init me first
		F_RESET = BITFLAG(1),  // for debug purpose
	} GameState;

	// mask def
	typedef enum
	{
		F_DRAWWIND = BITFLAG(2),
		F_USESPU = BITFLAG(3),
		F_DRAWCOLSPHERE = BITFLAG(4),
		F_DRAWANTICOLSPHERE = BITFLAG(5),

		// no memory
		F_ADDWIND = BITFLAG(10),
		F_SWAPDEBUGMODE = BITFLAG(11),
		F_RESTARTSIM = BITFLAG(12),

		// spu flags
		F_SPU_DEBUG_PERFORMANCE = BITFLAG(15),
		F_SPU_DEBUG_BREAKPOINT = BITFLAG(16),
		F_SPU_DEBUG_BREAKPOINT_AUX = BITFLAG(17),
		F_SPU_DEBUG_BEGINEND = BITFLAG(18),

	} DebugState;   // for debug purpose

protected:
	// mask need by thegame
	typedef BitMask<GameState,uint8_t> GameMask;
	GameMask m_gameMask;

	// mask needed by debug
	typedef BitMask<DebugState,uint32_t> DebugMask;
	DebugMask m_debugMask;
	
	// various sequence stuff (reload file every n frames)
	TimeSequenceManager m_sequence;
	
	// chain container typedef
	// register every chain/hair in the game
	RegisterContainer<OneChain> m_chainRegisters;
	
	// global def
	ChainGlobalDef* m_pGlobalWelderDef;
	
	// dumb value for debug purposes
	ValueAndMax<int> m_debugMode;
	
	// hair material hashed name
	CHashedString m_hairMaterial;
	
public:
	// detection
	int m_iNbDetection;
	
	// on the fly 
	ntstd::Set<XmlFileInterface*> m_onthefly;

public:
	
	//! scope: begin
	ChainRessource();
	
	//! scope: per frame update
	~ChainRessource();
	
	// toggle flag
	COMMAND_RESULT CommandToggleFlags(const int& kind);

	// register key
	void RegisterKey();
	
	// reset everything
	void Reset();

	// reset everything
	void LevelReset();

	// path to get all the information from
	ntstd::String HairPath();
	
	//! get mask
	DebugMask& GetBitMask() {return m_debugMask;}
	const DebugMask& GetBitMask() const {return m_debugMask;}

	//! per frame update
	bool Update();
	
	//! low freq update (eg: about every 30 frames)
	void LowFreqUpdate();
	
	//! register a new chain
	void Register(OneChain*);

	//! register a chain
	void Unregister(OneChain*);

	// get a collision def (create on-the-fly if needed)
	const HairSphereSetDef* GetCollisionSet(const ntstd::String& pcName);

	// get a maya hair style (create on-the-fly if needed)
	const HairStyleFromMaya* GetMayaHairStyle(const ntstd::String& pcName);
	
	// get a welder hair style (create on-the-fly if needed)
	const HairStyleFromWelder* GetWelderHairStyle(const ntstd::String& pcName);

	// get welder global def
	const ChainGlobalDef& GetGlobalDef() {ntAssert(m_pGlobalWelderDef); return *m_pGlobalWelderDef;}
	
	// get hair material hashed id
	CHashedString GetHairMaterialHashedId() {return m_hairMaterial;}

	// draw debug
	void DrawDebug();
	
	// create a hair component
	static OneChain* CreateComponent_Hair(CEntity* pobEnt, LuaAttributeTable* pAttrTable);
		
protected:
	
	// play all chain
	void PlayAll(bool bToggle);
		
	// draw collision sphere
	void DrawAntiCollisionSpheres();
	void DrawCollisionSpheres();
	
	// draw debug collision
	void DrawDebugCollision();
	
	// draw default pose of each chain
	void DrawDefaultPosition();

	// draw chain with debug line
	void DrawChainDebug();
	
	// draw collision sphere
	void DrawMainFrame();
	
	// load collision sphere
	void LoadCollisionSpheres();
	
	// load xml coming from maya
	void LoadMayaHairCut();

	// create the singleton
	void LoadDefFile();
	
	// init function (called once at first rendering)
	void Init();
	
}; // end of class ChainRessource












#endif // end of _EFFECTCHAIN_H_
