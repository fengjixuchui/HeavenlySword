//------------------------------------------------------------------------------------------
//!
//!	\file combat_lua.h
//!
//------------------------------------------------------------------------------------------

#ifndef	_COMBAT_LUA_INC
#define	_COMBAT_LUA_INC

class CEntity;

class Combat_Lua
{
public:
	void Lua_SendRecoilMessage();
	void Lua_ResetStylePoints();
	bool Lua_GenerateStrike ( CEntity* pobAttacker, CEntity* pobTarget, const char* pcAttackDef );
	void Lua_ChangeLeadClusterTo( const char* pcName );
	void Lua_SetDefaultLeadClusterTo( const char* pcName );
	void Lua_ResetLeadCluster();
	void Lua_SetEnableStrikeVolumeCreation( bool bEnable );

	void Lua_SetInvulnerableToNormalStrike( bool bInv );
	void Lua_SetInvulnerableToSyncStrike( bool bInv );
	void Lua_SetInvulnerableToProjectileStrike( bool bInv );
	void Lua_SetInvulnerableToCounterStrike( bool bInv );

	bool Lua_IsInvolvedInASynchronisedAttack();

	void Lua_SetCanHeadshotThisEntity( bool bHead );
};

#endif //_COMBAT_LUA_INC
