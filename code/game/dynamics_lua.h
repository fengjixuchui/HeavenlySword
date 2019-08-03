//------------------------------------------------------------------------------------------
//!
//!	\file dynamics_lua.h
//!
//------------------------------------------------------------------------------------------

#ifndef	_DYNAMICS_LUA_INC
#define	_DYNAMICS_LUA_INC

	HAS_LUA_INTERFACE()

	void				Lua_SetHoldingCapsule(bool bSize);
	void				Lua_ActivateState(const char* pcName);
	void				Lua_DeactivateState(const char* pcName);
	void				Lua_ActivateParticleOnContact();
	void				Lua_SetLookAtEntity(CEntity* pobToWatch);
	void				Lua_ResetLookAt();
	void				Lua_SetLinearVelocity(NinjaLua::LuaObject obTable);
	void				Lua_SetAngularVelocity(NinjaLua::LuaObject obTable);
	void				Lua_SetLinearVelocityFromTarget(CEntity* pobControllingEntity, NinjaLua::LuaObject obInputVelocity);
	void				Lua_SetLinearVelocityFromCamera(NinjaLua::LuaObject obInputVelocity);
	void				Lua_ApplyLinearImpulse(NinjaLua::LuaObject obVelocity);
	NinjaLua::LuaObject Lua_GetLinearVelocity();										// Bad
	void				Lua_Animated_Collapse();
	void				Lua_AttachedFromLeftFoot();
	void				Lua_Ragdoll_Freeze();
	void				Lua_Ragdoll_SetMotionType(int iMotionType);
	void				Lua_Ragdoll_Reparent(CEntity* pobParentEntity);
	void				Lua_Ragdoll_CheckAtRest();
	void				Lua_Ragdoll_SetBoneTransformTrackingMapping(int iBoneFlag, CEntity* pobEntity, const char* pcTransformName);
	void				Lua_Ragdoll_SetAnimatedBones(int iBoneFlags);
	void				Lua_Ragdoll_AddAnimatedBone(int iBoneFlag);
	void				Lua_Ragdoll_SetTurnDynamicOnContact(bool bTurnDynamicOnContact);
	void				Lua_Ragdoll_SetExemptFromCleanup(bool bExemption);
	void				Lua_Ragdoll_SetBeingHeld(bool bHeld);
	void				Lua_Ragdoll_Twitch();
	void				Lua_Ragdoll_ApplyLinearImpulse(NinjaLua::LuaObject obImp);
	void				Lua_Ragdoll_ApplyAngularImpulse(NinjaLua::LuaObject obImp);
	void				Lua_Ragdoll_SetBeingThrown( float fDistanceMultiplier=1.0f, float fAngleMultiplier=1.0f, const char* pcTargetType=NULL );
	void				Lua_Ragdoll_SetMinTumbleDistance( float fDist );
	void				Lua_CharacterController_SetRagdollCollidable(bool bCollide);
	void				Lua_Rigid_CheckAtRest();
	void				Lua_Rigid_ResetCollisionFilters();
	void				Lua_Rigid_CollideWithPlayerOnly();
	void				Lua_Rigid_CollideWithEnemiesOnly();
	void				Lua_Rigid_CollideWithPlayerAndEnemiesOnly();
	void				Lua_Rigid_PushFromPlayer();
	void				Lua_Rigid_CheckIfMoving();
	void				Lua_Rigid_SetKeyframedMotion(bool bEnable);
	void				Lua_Rigid_SetBreakableKeyframedMotion(bool bEnable);
	bool				Lua_Rigid_IsInBreakableKeyframedMotion() const;	
	bool				Lua_Rigid_SetRotationRelativeToEntitySafe (CEntity* pobTarget,float fR1,float fR2,float fR3,float fR4);//!< moves rigid body into rotation only if it does not lead to penetration. 
	bool				Lua_Rigid_MoveRelativeToEntitySafe (CEntity* pobTarget,float fX,float fY,float fZ); //!< moves rigid body into position only if it does not collide on path from its current position to given one
	void				Lua_Rigid_SetMotionType(int iMotionType);
	void				Lua_Rigid_AntiGravity(float fMinVelocity,float fDuration);
	void				Lua_CompoundRigid_CheckAtRest();
	void				Lua_CompoundRigid_SetMotionType(int iMotionType);
	void				Lua_CompoundRigid_SetKeyframedMotion(bool bEnable);
	void				Lua_CompoundRigid_SetBreakableKeyframedMotion(bool bEnable);
	bool				Lua_CompoundRigid_IsInBreakableKeyframedMotion() const;
	void				Lua_CompoundRigid_Collapse();
	bool				Lua_CompoundRigid_MoveRelativeToEntitySafe (CEntity* pobTarget,float fX,float fY,float fZ); //!< moves rigid body into position only if it does not lead to penetration. 
	bool				Lua_CompoundRigid_SetRotationRelativeToEntitySafe (CEntity* pobTarget,float fR1,float fR2,float fR3,float fR4);//!< moves rigid body into rotation only if it does not lead to penetration. 
	void				Lua_CompoundRigid_AntiGravity(float fMinVelocity,float fDuration);
	void				Lua_RemoveChildEntities();
	void				Lua_Rigid_OrientateToVelocity(bool bEnable);
	void				Lua_Rigid_EnableVelocityReflection(bool bEnable);
	void				Lua_Rigid_DeflectionRender(bool bEnable);
	void				Lua_DoHurtOnCollision(bool bEnable);
	void				Lua_CollisionStrike(bool bEnable);
	void				Lua_SetController(CEntity* pEntController, CHashedString pcParamDef);
	void				Lua_SetControllerDef(CEntity* pEntController, AftertouchControlParameters* pobControlDef);
	void				Lua_Rigid_EnableSteering(CEntity* pEntController);
	void				Lua_Rigid_DisableSteering();
	void				Lua_Spear_SetMotionType(int iMotionType);
	void				Lua_Spear_SetController(CEntity* pEntController, const char* pcParamDef);
	void				Lua_Spear_StartThrownBehaviour();
	void				Lua_Spear_StopThrownBehaviour();
	void				Lua_Spear_GenerateAtRestMessage();
	int					Lua_Projectile_Reparent(NinjaLua::LuaState& pobState);			// Bad
	// Should these be here?
	void				Lua_Projectile_EnableSteering(CEntity* pEntController,bool, AftertouchControlParameters* pobControlDef);	
	void				Lua_Projectile_EnableMonoSteering(CEntity* pEntController, AftertouchControlParameters* pobControlDef) { Lua_Projectile_EnableSteering(pEntController,false, pobControlDef); }
	void				Lua_Projectile_EnableDualSteering(CEntity* pEntController, AftertouchControlParameters* pobControlDef) { Lua_Projectile_EnableSteering(pEntController,true, pobControlDef); }
	
	void				Lua_Projectile_EnableEntityTracking(CEntity* pEntityToTrack, float fXOffset=0.0f, float fYOffset=0.0f, float fZOffset=0.0f, float fTurnWeighting = 0.8f);	// Should these be here?
	void				Lua_Projectile_DisableSteering();								// Should these be here?
	void				Lua_Projectile_SetSplineRadius(float fRadius);
	bool				Lua_Projectile_IsMoving();
	float				Lua_Projectile_GetStateTime();
	void				Lua_Character_SetCollidable(bool bCollidable);
	void				Lua_Character_SetSoftParent(CEntity* pSoftEnt);
	void				Lua_SendMessageOnCollision(bool);

	void				Lua_SetCollisionCallback(const char* pcSelfMessage,const char* pcReceiverMessage);
	int					Lua_ParamAttack ( NinjaLua::LuaObject obParamTable );
	void				Lua_SetCharacterControllerDoMovementAbsolutely( bool bDoMovementAbsolutely );


	// Alternative UNREGISTERED functions, mainly to get rid of Lua based parameters
	void				Lua_AltSetLinearVelocity(CDirection &vDirection);
	void				Lua_AltSetLinearVelocityFromTarget(CEntity* pobControllingEntity, const CDirection& InputVelocity);
	void				Lua_AltSetLinearVelocityFromCamera(const CDirection& InputVelocity);
	void				Lua_AltSetAngularVelocity(const CDirection& NewVelocity);
	void				Lua_AltExplosion( const CExplosionParams& params );

#endif //_DYNAMICS_LUA_INC
