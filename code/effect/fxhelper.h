//----------------------------------------------------------------------------------------------------
//!
//!	\file fxhelper.h
//!	FX Helper functions, basically the same as the bind functions without the LUA, and can
//! be called from anywhere that includes this file
//!
//----------------------------------------------------------------------------------------------------

#ifndef _FXHELPER_H
#define _FXHELPER_H

class CEntity;

class FXHelper
{
public:
	static u_int	Pfx_CreateStaticMatrix(CHashedString pcDefName, const CMatrix& obPosMatrix);
	static u_int	Pfx_CreateStatic(CHashedString pcDefName, CEntity* pEnt, CHashedString pcTransformName);
	static u_int	Pfx_CreateAttached (CHashedString pcDefName, CHashedString pcEntityName, CHashedString pcTransformName);
	static void		Pfx_Destroy(u_int uiID, bool bImmediate);

	// TO-DO (when and if required)
	//static int		CreateSwordChainEffect(NinjaLua::LuaState* pobState);
	//static int		CreateChainmanChains(NinjaLua::LuaState* pobState);
};

#endif //_FXHELPER_H
