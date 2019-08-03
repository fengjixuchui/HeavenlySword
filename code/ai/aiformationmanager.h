//------------------------------------------------------------------------------------------
//!
//!	\file aiformationmanager.h
//!
//------------------------------------------------------------------------------------------

#ifndef _AIFORMATIONMANAGER_INC
#define _AIFORMATIONMANAGER_INC

//------------------------------------------------------------------------------------------
// Includes for inheritance or for members  
//------------------------------------------------------------------------------------------
#include "game/luaglobal.h"
#include "ai/aiformationslot.h"
#include "editable/enums_formations.h"

#ifndef _COMMAND_RESULT_H
#include "game/commandresult.h"
#endif

//------------------------------------------------------------------------------------------
// External Decls.                           
//------------------------------------------------------------------------------------------

class AIFormation;
namespace NinjaLua { class LuaState; };

//------------------------------------------------------------------------------------------
//!
//!	AIFormationManager
//!	Manages all the formations.
//!
//------------------------------------------------------------------------------------------
class AIFormationManager : public Singleton<AIFormationManager>
{
public:
	AIFormationManager();
	~AIFormationManager();

	HAS_LUA_INTERFACE()


	AIFormation* CreateFormation(const NinjaLua::LuaObject& def);

	// Upate the global instance..
	void Update(float fTimeDelta);

	// Return the commander entity
	const CEntity* GetCommander(const CEntity* pEnt) const;

	// Add and remove formation entities from the internal list. 
	void AddFormationEntity( CEntity* );
	void RemoveFormationEntity( CEntity* );
	int DisableAllFormationEntities( NinjaLua::LuaState& );

	// Are there any entities in a formation?
	bool IsAttackingPlayer( void );

	// Return a debug context for the formation attacks
	int GetDebugContext() const { return m_DebugAttackContext; }

	// LuaFile was reloaded, use the excuse to setup the formations again.
#ifndef _RELEASE
	void LuaFileReloaded();
#endif

	// Functions to be invoked by keyboard/script
	COMMAND_RESULT AdvanceDebugAttackContext();

	void PlayerPlayingDervish( bool bState ) { m_bPlayerPlayingDervish = bState; }

	// Exposed member var, who cares if a a few short cuts are taken?
	bool	m_bPlayerPlayingDervish;

// Members
private:

	ntstd::List<CEntity*>	m_FormationEntities;

	int	m_DebugAttackContext;

// Debug Info
#ifndef _RELEASE
public:
	static bool DisplayDebugInfo() {return m_bDebug;}
	static bool m_bDebug;
	bool m_bKillAllEntities;
	uintptr_t m_MemoryTracker;
#endif
};

LV_DECLARE_USERDATA(AIFormationManager);

#endif //_AIFORMATIONMANAGER_INC
