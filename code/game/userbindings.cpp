#include "userbindings.h"

//--------------------------------------------------
//!
//!	DO NOT USE THAT FILE !!!!!!!!!!!!!!!!!!!!!!!
//! UNLESS YOU HAVE A GOOD REASON
//!
//!	"special" lua binding function
//!	This is the palce where you bind all the special/debug/per-level function.
//!	Everybody who use that MUST be aware that this file will be clean up one day or the other.
//! The bindings function in that file should be here for special and temporary reasons
//!
//! ATTN! this class does nothing on PS3!
//!
//--------------------------------------------------





#include "game/luaglobal.h"
#include "game/luahelp.h"
#include "core/OSDDisplay.h"
#include "hair/forcefield.h"
#include "hair/haircollision.h"
#include "core/profiling.h"
#include "game/entity.h"
#include "game/entity.inl"

//-------------------------------------------------------------------------------------------------
// BINDFUNC:    EnableHairOutput()
// DESCRIPTION: I put that in the "hair level so that Guy don't have to enable the channel each time
//-------------------------------------------------------------------------------------------------
static void EnableHairOutput()
{
	OSD::EnableChannel(OSD::HAIR);
}

// void AddGustOfWind(float fLifeDuration, const CPoint& begin, const CPoint& end, float fPower);
static int AddGustOfWind(NinjaLua::LuaState* pState)
{
	if(ForceFieldManager::Exists())
	{
		NinjaLua::LuaStack args(*pState);
				
		ForceFieldManager::Get().AddGustOfWind(
			args[1].GetFloat(),
			CLuaHelper::PointFromTable(args[2]),
			CLuaHelper::PointFromTable(args[3]),
			args[4].GetFloat());

	}
	else
	{
		OSD::Add( OSD::DEBUG_CHAN, 0xffffffff, "ForceField not created" );
	}
	return 0;
}



// void AddExplosion(float fLifeDuration, const CPoint& center, float fRadius, float fPower);
static int AddExplosion(NinjaLua::LuaState* pState)
{
	if(ForceFieldManager::Exists())
	{
		NinjaLua::LuaStack args(*pState);
		ForceFieldManager::Get().AddExplosion(
			args[1].GetFloat(),
			CLuaHelper::PointFromTable(args[2]),
			args[3].GetFloat(),
			args[4].GetFloat());
	}
	else
	{
		OSD::Add( OSD::DEBUG_CHAN, 0xffffffff, "ForceField not created" );
	}
	return 0;
}





static void SwordCollisionAddToBody(CEntity* pEntity, bool bRight)
{
	UNUSED(bRight);
	// Remove any sword collision for the hair
	if (pEntity->GetCollisionSword())
	{
		pEntity->GetCollisionSword()->Add();
	}

}
static void SwordCollisionRemoveFromBody(CEntity* pEntity, bool bRight)
{
	UNUSED(bRight);
	// Remove any sword collision for the hair
	if (pEntity->GetCollisionSword())
	{
		pEntity->GetCollisionSword()->Remove();
	}
}







//-------------------------------------------------------------------------------------------------
// BINDFUNC:    GatsoFilterName()
// DESCRIPTION: I put that in the "hair level so that Guy don't have to enable the channel each time
//-------------------------------------------------------------------------------------------------
static void GatsoFilterName(const char* pcAnim)
{
	UNUSED( pcAnim );
#ifdef _PROFILING
	if(CProfiler::Exists())
	{
		CProfiler::Get().SetNameFilter(pcAnim);
	}
	else
	{
		OSD::Add( OSD::DEBUG_CHAN, 0xffffffff, "CProfiler not created" );
	}
#endif
}








//--------------------------------------------------
//!
//!	register lua function listed in that file
//!
//--------------------------------------------------

void CUserBindings::Register()
{
	NinjaLua::LuaObject obGlobals = CLuaGlobal::Get().State().GetGlobals();

	obGlobals.Register( "EnableHairOutput", EnableHairOutput );
	obGlobals.RegisterRaw( "AddExplosion", AddExplosion );
	obGlobals.RegisterRaw( "AddGustOfWind", AddGustOfWind );
	obGlobals.Register( "GatsoFilterName", GatsoFilterName );
	
	obGlobals.Register( "SwordCollisionAddToBody", SwordCollisionAddToBody );
	obGlobals.Register( "SwordCollisionRemoveFromBody", SwordCollisionRemoveFromBody );
}
