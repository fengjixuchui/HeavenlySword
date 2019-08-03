//------------------------------------------------------------------------------------------
//!
//!	\file movementbindings.cpp
//! Contains the interface to the movement system that is available to the scripting
//!
//------------------------------------------------------------------------------------------

// Necessary includes
#include "movementbindings.h"
#include "game/luaglobal.h"
#include "game/entity.h"
#include "game/entity.inl"

// Pad tests
#include "game/inputcomponent.h"


//-------------------------------------------------------------------------------------------------
// BINDFUNC:	IsPowerHeld()
// DESCRIPTION:	Determine if the power stance button is held - Added JML
//-------------------------------------------------------------------------------------------------
static int IsPowerHeld(NinjaLua::LuaState* pobState)
{
	NinjaLua::LuaStack args( pobState );

	CEntity* pobSelf = CLuaGlobal::Get().GetTarg();
	ntAssert(pobSelf);

	if(pobSelf->GetInputComponent()) // Note: AI characters don't have an input component
	{
		float fInputTime=0.0f;

		if (args[1].IsNumber())
			fInputTime=args[1].GetFloat();

		float fHeldTime=pobSelf->GetInputComponent()->GetVHeldTime(AB_PSTANCE);

		if (fHeldTime>fInputTime)
		{
			pobState->Push(true);
			return 1;
		}
	}

	pobState->Push(false);
	return 1;
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC:	IsRangeHeld()
// DESCRIPTION:	Determine if the range stance button is held - Added JML
//-------------------------------------------------------------------------------------------------
static int IsRangeHeld(NinjaLua::LuaState* pobState)
{
	NinjaLua::LuaStack args( pobState );

	CEntity* pobSelf = CLuaGlobal::Get().GetTarg();
	ntAssert(pobSelf);

	if(pobSelf->GetInputComponent()) // Note: AI characters don't have an input component
	{
		float fInputTime=0.0f;

		if (args[1].IsNumber())
			fInputTime=args[1].GetFloat();

		float fHeldTime=pobSelf->GetInputComponent()->GetVHeldTime(AB_RSTANCE);

		if (fHeldTime>fInputTime)
		{
			pobState->Push(true);
			return 1;
		}
	}

	pobState->Push(false);
	return 1;
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:	IsStanceHeld()
// DESCRIPTION:	Determine if the a stance button is held - Added JML
//-------------------------------------------------------------------------------------------------
static int IsStanceHeld(NinjaLua::LuaState* pobState)
{
	NinjaLua::LuaStack args( pobState );

	CEntity* pobSelf = CLuaGlobal::Get().GetTarg();
	ntAssert(pobSelf);

	if(pobSelf->GetInputComponent()) // Note: AI characters don't have an input component
	{
		float fInputTime=0.0f;

		if (args[1].IsNumber())
			fInputTime=args[1].GetFloat();

		float fPStanceTime=pobSelf->GetInputComponent()->GetVHeldTime(AB_PSTANCE);
		float fRStanceTime=pobSelf->GetInputComponent()->GetVHeldTime(AB_RSTANCE);
		
		float fHeldTime=( fPStanceTime > fRStanceTime ? fPStanceTime : fRStanceTime );

		if (fHeldTime>fInputTime)
		{
			pobState->Push(true);
			return 1;
		}
	}

	pobState->Push(false);
	return 1;

	return false;
}


//-------------------------------------------------------------------------------------------------
// BINDFUNC:	IsAttackHeld()
// DESCRIPTION:	Determine if an attack button is held. This is currently used to determine if
//				whether or not to enter aftertouch after a thrown state has completed.
//-------------------------------------------------------------------------------------------------
static int IsAttackHeld (NinjaLua::LuaState* pobState)
{
	NinjaLua::LuaStack args( pobState );

	CEntity* pobSelf = CLuaGlobal::Get().GetTarg();
	ntAssert(pobSelf);

	if(pobSelf->GetInputComponent()) // Note: AI characters don't have an input component
	{
		float fInputTime=0.0f;

		if (args[1].IsNumber())
			fInputTime=args[1].GetFloat();

		float fAttackFastTime=pobSelf->GetInputComponent()->GetVHeldTime(AB_ATTACK_FAST);
		float fAttackMediumTime=pobSelf->GetInputComponent()->GetVHeldTime(AB_ATTACK_MEDIUM);
		
		float fHeldTime=( fAttackFastTime > fAttackMediumTime ? fAttackFastTime : fAttackMediumTime );

		if (fHeldTime>fInputTime)
		{
			pobState->Push(true);
			return 1;
		}
	}

	pobState->Push(false);
	return 1;

	return false;
}

//-------------------------------------------------------------------------------------------------
// BINDFUNC:	IsActionHeld()
// DESCRIPTION:	Determine if an grab button is held. This is currently used to determine if
//				whether or not to enter aftertouch after a thrown state has completed.
//-------------------------------------------------------------------------------------------------
static int IsActionHeld (NinjaLua::LuaState* pobState)
{
	NinjaLua::LuaStack args( pobState );

	CEntity* pobSelf = CLuaGlobal::Get().GetTarg();
	ntAssert(pobSelf);

	if(pobSelf->GetInputComponent()) // Note: AI characters don't have an input component
	{
		float fInputTime=0.0f;

		if (args[1].IsNumber())
			fInputTime=args[1].GetFloat();

		float fHeldTime=pobSelf->GetInputComponent()->GetVHeldTime(AB_ACTION);

		if (fHeldTime>fInputTime)
		{
			pobState->Push(true);
			return 1;
		}
	}

	pobState->Push(false);
	return 1;
}


//------------------------------------------------------------------------------------------
//!
//!	MovementBindings::Register
//!	Register the available movement bindings with the Lua environment
//!
//------------------------------------------------------------------------------------------
void MovementBindings::Register()
{
	// Get the location of the Lua global objects
	NinjaLua::LuaObject obLuaGlobals = CLuaGlobal::Get().State().GetGlobals();

	// Register our functionality
	obLuaGlobals.RegisterRaw( "IsPowerHeld",  IsPowerHeld  );
	obLuaGlobals.RegisterRaw( "IsRangeHeld",  IsRangeHeld  );
	obLuaGlobals.RegisterRaw( "IsStanceHeld", IsStanceHeld );
	obLuaGlobals.RegisterRaw( "IsAttackHeld", IsAttackHeld );
	obLuaGlobals.RegisterRaw( "IsActionHeld", IsActionHeld );
}
