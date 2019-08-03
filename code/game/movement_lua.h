//------------------------------------------------------------------------------------------
//!
//!	\file movement_lua.h
//!
//------------------------------------------------------------------------------------------

#ifndef	_MOVEMENT_LUA_INC
#define	_MOVEMENT_LUA_INC

#include "lua/ninjalua.h"

class CEntity;
class CMovement;
class CoordinationParams;

class Movement_Lua
{
public:
	HAS_LUA_INTERFACE()

	int Lua_SetMovementCompleteMessage(NinjaLua::LuaState& State);
	int Lua_SetMovementInteruptMessage(NinjaLua::LuaState& State);
	bool Lua_StartEmptyMovement();
	bool Lua_StartMovementFromXMLDef(CHashedString pcDefName);
	int  Lua_ClampBones(NinjaLua::LuaState& State);
	void Lua_ActivateVehicle(CHashedString pcDefName, CEntity* pVehicle, CHashedString pcDrivingSeat, bool bAIControlled);
	void Lua_ChainMovementFromXMLDef(CHashedString pcDefinitionName);
	int  Lua_StartSimpleMovement( NinjaLua::LuaState& State );
	void Lua_StartRelativeMovement(CEntity* pobRelativeEntity, CHashedString pcAnimationName, bool bLooping, bool bSoftMovement);
	int  Lua_ChainSimpleMovement(NinjaLua::LuaState& State);
	int  Lua_StartMoveToTransition(CHashedString pcAnim, CEntity* pTarget, float fDist, float fSpeed);
	int  Lua_AltStartMoveToTransition(CHashedString pcAnim, CEntity* pTarget, float fDist, float fSpeed, CPoint* obOffset = 0);
	bool  Lua_StartFacingMoveToTransition(CHashedString pcAnim, CEntity* pTarget, float fDist, float fSpeed, CPoint* obOffset=0, CQuat* obRotOffset=0, CDirection* pobFacingDir = 0);
	int  Lua_StartFacingMovement(NinjaLua::LuaState& State);
	int  Lua_StartTargetedFacingMovement(NinjaLua::LuaState& State);
	int  Lua_StartSnapToMovement(NinjaLua::LuaState& State);
	void Lua_AltStartSnapToMovement(CHashedString pcAnimName, CEntity* pobTarg, CPoint& obPosition, CQuat& obRotation, bool Gravity = false);
	void Lua_AltStartSnapToMovement(CHashedString pcAnimName, CEntity* pobTarg, CPoint& obPosition, CQuat& obRotation, bool Gravity, CHashedString pcTransform);
	int  Lua_StartLinkedMovement(NinjaLua::LuaState& State);
	
	void Lua_StartInteractiveAnimMovement(CHashedString pcAnim, CEntity* pControllerEnt, float fMaxSpeed, float fSpeedAcc, float fSpeedDec, float fButtonPressInterval);
	void Lua_StartFallAftertouch(CEntity* pobFallingEntity,	CHashedString	psAnimName, float fMaxVelocity,	float fAccelFactor,	CEntity* pobControllingEntity);
	

	// These are all replaced by ButtonMash and SimpleDoor objects
	// Can be removed once old lua objects replaced in walkways
	void Lua_CreateCrankLevelController(const char* pcCrankParams);
	void Lua_CreateCrankDoorController(const char* pcCrankParams);
	void Lua_CreateCrankOperatorController(const char* pcCrankParams);
	void Lua_CrankDoorLock(const char* pcCrankParams, bool bLocked);
	void Lua_LadderController(CHashedString LadderParams, CEntity* pLadder, bool bChained = false);
	void Lua_CreateCounterWeightLeverController(const char* pcLeverParams);
	void Lua_CreateCounterWeightOperatorController(const char* pcLeverParams);

	// Alternative UNREGISTERED functions (mainly functions to remove LuaStates being used as parameters)
	// Nasty, please fix this to be nicer...
	void Lua_AltSetMovementCompleteMessage(const char* pcMessage, CEntity* pEnt = 0);
	bool Lua_AltStartFacingMovement(CHashedString pcAnimName, float fAngularSpeed, float fAnimSpeed, float fStartTurnControl, float fEndTurnControl, float fBlendTime = -1.0f);
	bool Lua_AltStartTargetedFacingMovement(CHashedString pcAnimName, float fAngularSpeed, float fAnimSpeed, float fStartTurnControl, float fEndTurnControl, float fBlendTime = -1.0f );
	bool Lua_AltStartSimpleMovement(CHashedString& pcAnimName, bool bLooping = false, bool bRandomOffset = false, bool bGravity = false);
	bool Lua_AltStartLinkedMovement(CHashedString obAnim, CEntity* pobEnt, CHashedString obTransform, CPoint& obTranslation, CPoint& obRotation);
	bool Lua_AltStartFullyLinkedMovement(CHashedString obAnim, CEntity* pobEnt, CHashedString obTransform, CPoint& obTranslation, CPoint& obRotation);
	bool Lua_AltStartCorrectiveMovement(CPoint& obTargetPos, CQuat& obTargetRotation);
	bool Lua_StartCoordinatedMovement(CHashedString& pcAnimName, bool bLooping, bool bGravity, CoordinationParams* pobCoordParams);


};

#endif //_MOVEMENT_LUA_INC
