//------------------------------------------------------------------------------------------
//!
//!	\file entity_lua.h
//!
//------------------------------------------------------------------------------------------

#ifndef	_ENTITY_LUA_INC
#define	_ENTITY_LUA_INC

	// Horrible - used in warp player stuff which should be redundant. - Actually used much more than just warp, but that only makes it more horrible - JML.
	NinjaLua::LuaObject		Lua_GetPosition();
	void					Lua_SetPosition(float x, float y, float z);
	void					Lua_SetPositionToLocator(const char* pcLocatorName);
	NinjaLua::LuaObject		Lua_GetPositionNamed(const char* pcTransform);
	void					Lua_SetTranslationRelativeToEntity (CEntity* pobTarget,float fX,float fY,float fZ);
	void					Lua_SetRotationRelativeToEntity (CEntity* pobTarget,float fR1,float fR2,float fR3,float fR4);

	void					Lua_AnimPlay (CHashedString pcAnim, float fSpeed, bool bLocomoting, bool bLooping);

	void					Lua_AnimStopAll();
	void					Lua_AnimMessageOnCompletion (const CHashedString& pcAnim);


	void					Lua_SetMeshVisibility(const char* pcMeshName, bool bVisibility);
	bool					Lua_CheckIfCharacterEntity();
	void					Lua_RemoveSelfFromWorld();
	void					Lua_SetIdentity();
	void					Lua_SetParentEntity(CEntity* pobParent);

	void					Lua_ResetAimingComponent();
	void					Lua_SetAimRange(float fMinYaw, float fMaxYaw, float fMinPitch, float fMaxPitch);
	void					Lua_ResetAimRange();

	bool					Lua_IsPowerHeld(float fDuration);
	bool					Lua_IsAttackHeld(float fDuration);
	bool					Lua_IsActionHeld(float fDuration);

	void					Lua_CreatePhysicsSystem();
	void					Lua_SetLocalTransform( float x, float y, float z, float ax, float ay, float az );
	void					Lua_MakeDead();

	bool					Lua_Reparent(CEntity* pNewParent, CHashedString pcTransform);
	void					Lua_ReparentToWorld();

	void					Lua_ConstructProjectile(LuaAttributeTable* pAttrTable);
	void					Lua_SpawnExplodingKegExplosion();

	//Extremely hacky bits put in for specific purposes
	void					Lua_AttachRoachShellToEntity(); //So we can have a controllable player-type entity roach for sound-implementation.

	void					Lua_Pause(bool bPause);
	void					Lua_Deactivate(bool bDeactivate);
#endif //_ENTITY_LUA_INC
