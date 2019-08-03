//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file camman.cpp                                                               
//!                                                                                         
//------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
// Required Includes                                                                        
//------------------------------------------------------------------------------------------
#include "camera/camman.h"
#include "camera/camview.h"
#include "camera/basiccamera.h"
#include "camera/timescalarcurve.h" // For Debug Rendering of the curve...
#include "core/OSDDisplay.h"
#include "core/timer.h"
#include "game/entitymanager.h"
#include "game/luaglobal.h"
#include "game/luahelp.h"
#include "lua/ninjalua.h"
#include "objectdatabase/dataobject.h"


#ifdef USER_JohnL
 // #define CAMERA_DEBUG
#endif

//------------------------------------------------------------------------------------------
// Statics
//------------------------------------------------------------------------------------------
PAD_NUMBER		CamMan::m_eDebugCameraPad = PAD_0;


//------------------------------------------------------------------------------------------
//!
//!	CamMan::CamMan
//!	Construction
//!
//------------------------------------------------------------------------------------------
CamMan::CamMan() 
:	m_bReady(false),
	m_iPrimaryView(0),
	m_bCombatAware(false)
{

	for(int i = 0; i < MAX_VIEWS; i++)
		m_views[i] = 0;

	#ifndef _RELEASE
		#ifdef _CAM_CURVE_DEBUG
			m_pTSCCurve = 0;
		#endif
	#endif
}


//------------------------------------------------------------------------------------------
//!
//!	CamMan::CamMan
//!	Destruction
//!
//------------------------------------------------------------------------------------------
CamMan::~CamMan()
{
	if(m_bReady)
		CleanUp();
}


//------------------------------------------------------------------------------------------
//!
//!	CamMan::Init
//! Initialise the camera manager.
//!
//------------------------------------------------------------------------------------------
void CamMan::Init()
{
	if(m_bReady)
	{
		ntAssert(false);
		return;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////
	// Create the Cameras meta-table for lua.  This lists all of the game camera templates for the level.
	///////////////////////////////////////////////////////////////////////////////////////////////////////
	NinjaLua::LuaState& luaState(CLuaGlobal::Get().State());
	NinjaLua::LuaObject luaGlobs = luaState.GetGlobals();

	NinjaLua::LuaObject luaCamsTbl = NinjaLua::LuaObject::CreateTable(luaState);
	luaGlobs.Set("Cameras", luaCamsTbl);

	// Create the metatable
	NinjaLua::LuaObject luaCamsMetaTbl = NinjaLua::LuaObject::CreateTable(luaState);
	luaCamsMetaTbl.Set("__index", NinjaLua::LuaObject(luaState, &CamMan::GetLevelCameraForLua));

	luaCamsTbl.SetMetaTable(luaCamsMetaTbl);

	///////////////////////////////////////////////////////////////////////////////////////////////////////
	// Create the Views meta-table for lua.  This lists all of the views currently used by the game.
	///////////////////////////////////////////////////////////////////////////////////////////////////////
	NinjaLua::LuaObject luaViewsTbl = NinjaLua::LuaObject::CreateTable(luaState);
	luaGlobs.Set("Views", luaViewsTbl);

	// Create and assign the metatable
	NinjaLua::LuaObject luaViewsMetaTbl = NinjaLua::LuaObject::CreateTable(luaState);
	luaViewsMetaTbl.Set("__index", NinjaLua::LuaObject(luaState, &CamMan::GetViewForLua));
	luaViewsTbl.SetMetaTable(luaViewsMetaTbl);

	// Add Function for creating new views
	luaViewsTbl.Register("CreatePrimary",      CamMan::CreatePrimaryViewFromDef);
	luaViewsTbl.Register("Create",             CamMan::CreateViewFromDef);
	luaViewsTbl.Register("GetPrimary",         CamMan::GetPrimaryView);
	luaViewsTbl.Register("Shake",              CamMan::Lua_Shake);
	luaViewsTbl.RegisterRaw("ShakePositioned", CamMan::Lua_ShakePositioned);
	luaViewsTbl.Register("SetCombatAware",     CamMan::SetCombatAware);


	///////////////////////////////////////////////////////////////////////////////////////////////////////
	// Create the primary view.
	///////////////////////////////////////////////////////////////////////////////////////////////////////
	CamViewInfo caminfo(0, 0.f, 0.f, 1.f, 1.f, CEntityManager::Get().GetPlayer());
	CreateView(caminfo);

	// Ready to go now.
	m_bReady = true;
}


//------------------------------------------------------------------------------------------
//!
//!	CamMan::CleanUp
//! Tidy up after ourselves.
//!
//------------------------------------------------------------------------------------------
void CamMan::CleanUp()
{
	// Delete all the views
	for(int i = 0; i < MAX_VIEWS; i++)
	{
		NT_DELETE_CHUNK(Mem::MC_CAMERA, m_views[i] );
		m_views[i] = 0;
	}

	// Empty the Level Camera Templates list
	m_levelCameraTemplates.clear();

	// Not ready anymore.
	m_bReady = false;
}


//------------------------------------------------------------------------------------------
//!
//!	CamMan::AddLevelCameraTemplate
//!	Add a new level camera template to the list
//!
//------------------------------------------------------------------------------------------
void CamMan::AddLevelCameraTemplate(BasicCameraTemplate* pTemplate)
{
	ntAssert(pTemplate);

	m_levelCameraTemplates.push_back(pTemplate);
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CamMan::RemoveLevelCameraTemplate                                                             
//!	Remove a level cameras template from the list                                           
//!                                                                                         
//------------------------------------------------------------------------------------------
void CamMan::RemoveLevelCameraTemplate(BasicCameraTemplate* pTemplate)
{
	ntstd::Vector<BasicCameraTemplate*, Mem::MC_CAMERA>::iterator it = ntstd::find(m_levelCameraTemplates.begin(), m_levelCameraTemplates.end(), pTemplate);

	if(it != m_levelCameraTemplates.end())
		m_levelCameraTemplates.erase(it);
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CamMan::CreateView                                                                      
//!	Create a new view.  Returns the id of the new view, -1 if creation fails.                                       
//!                                                                                         
//------------------------------------------------------------------------------------------
int CamMan::CreateView(CamViewInfo& info)
{
	if(info.m_iID == -1)
	{
		for(int i = 0; i < MAX_VIEWS; i++)
			if(!m_views[i])
			{
				info.m_iID = i;
				break;
			}
	}

	if(info.m_iID == -1)
	{
		ntPrintf("Cannot create another CamView.\n");
		return -1;
	}
	else if(m_views[info.m_iID] != 0)
	{
		ntPrintf("View %d already in use.  Failed to create requested view %d.\n", info.m_iID);
		return -1;
	}
	
	// Kill the old view if necessary
	KillView(info.m_iID);
	
	// Make the new view
	m_views[info.m_iID] = NT_NEW_CHUNK( Mem::MC_CAMERA ) CamView(info);
	return info.m_iID;
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CamMan::KillView                                                                        
//!	Destroy a view.                                                                         
//!                                                                                         
//------------------------------------------------------------------------------------------
void CamMan::KillView(int iView)
{
	NT_DELETE_CHUNK(Mem::MC_CAMERA, m_views[iView] );
	m_views[iView] = 0;
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CamMan::Shake                                                                        
//!	Shake the cameras.                                                                         
//!                                                                                         
//------------------------------------------------------------------------------------------
void CamMan::Shake(float fTime, float fAmp, float fFreq, CPoint* pPt, float fRadiusSqrd)
{
	// Set up the shake
	CamShake shake(fTime, fAmp*DEG_TO_RAD_VALUE, fFreq);
	
	if(pPt)
	{
		shake.m_pt          = *pPt;
		shake.m_fRadiusSqrd = fRadiusSqrd;
	}

	for(int i = 0; i < MAX_VIEWS; i++)
	{
		if(!m_views[i])
			continue;

		m_views[i]->ShakeView(shake);
	}
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CamMan::Lua_ShakePositioned                                                             
//!	Apply a position relative shake.                                                           
//!                                                                                         
//------------------------------------------------------------------------------------------
int CamMan::Lua_ShakePositioned(NinjaLua::LuaState* pState)
{
	ntAssert(pState);
	NinjaLua::LuaStack args(*pState);

	float fDuration = (args[1].IsNumber() ? args[1].GetFloat() : 0.f);
	float fAmp      = (args[2].IsNumber() ? args[2].GetFloat() : 0.f) * DEG_TO_RAD_VALUE;
	float fFreq     = (args[3].IsNumber() ? args[3].GetFloat() : 0.f);
	
	CPoint pt(CONSTRUCT_CLEAR);
	if(args[4].IsTable())
	{
		// Note this has to be this way due to differences in the way pts and dirs are done! doh!
		pt = CPoint(CLuaHelper::DirectionFromTable(args[4]));
	}

	float fRadiusSqrd = args[5].IsNumber() ? args[5].GetFloat() : 0.f;
	fRadiusSqrd *= fRadiusSqrd;

	CamMan::Get().Shake(fDuration, fAmp, fFreq, &pt, fRadiusSqrd);

	return 0;
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CamMan::Update                                                                          
//!	The per frame update loop                                                               
//!                                                                                         
//------------------------------------------------------------------------------------------
void CamMan::Update()
{
	// Update all our views
	for(int iView = 0; iView < MAX_VIEWS; iView++)
	{
		if(!m_views[iView])
			continue;


		// Update the View
		m_views[iView]->Update();
	}

#ifndef _RELEASE
	if(OSD::IsChannelEnabled(OSD::CAMERA))
		RenderTimeCurve();
#endif
}


//------------------------------------------------------------------------------------------
//!
//!	CamMan::FindLevelCamera
//!	Find a named level camera.
//!
//------------------------------------------------------------------------------------------
BasicCameraTemplate* CamMan::FindLevelCamera(const CHashedString& sName)
{
	for(ntstd::Vector<BasicCameraTemplate*, Mem::MC_CAMERA>::const_iterator it = m_levelCameraTemplates.begin(); 
		it != m_levelCameraTemplates.end(); it++)
	{
		if(sName==CHashedString((*it)->GetCameraName()))
			return *it;
	}

	return 0;
}


//------------------------------------------------------------------------------------------
//!
//!	CamMan::GetLevelCameraForLua
//!	Lua call back to provide data for the Cameras[] metatable
//!
//------------------------------------------------------------------------------------------
int CamMan::GetLevelCameraForLua(NinjaLua::LuaState& State)
{
	lua_checkstack(State, 2);
	const char* pcCamName = lua_tostring(State, 2);

	BasicCameraTemplate* pCamTemplate = Get().FindLevelCamera(pcCamName);

	if(pCamTemplate)
		return (NinjaLua::LuaValue::Push(State, pCamTemplate));

	return 0;
}


//------------------------------------------------------------------------------------------
//!
//!	CamMan::GetViewForLua
//!	Lua call back to provide data for the Views[] metatable
//!
//------------------------------------------------------------------------------------------
int CamMan::GetViewForLua(NinjaLua::LuaState& State)
{
	lua_checkstack(State, 2);
	int iView = (int)lua_tonumber(State, 2);

	if(iView >= 0 && iView < MAX_VIEWS)
	{
		CamView* pView = Get().GetView(iView);

		if(pView)
			return (NinjaLua::LuaValue::Push(State, pView));
		else
		{
			ntPrintf("%s - CAMERA: No view exists with id %d.\n", State.FileAndLine(), iView);
		}
	}
	else
	{
		ntPrintf("%s - CAMERA: id %d is invalid.\n", State.FileAndLine(), iView);
	}

	return 0;
}


//------------------------------------------------------------------------------------------
//!
//!	CamMan::CreateViewFromDef
//!	Create a new camera view
//!
//------------------------------------------------------------------------------------------
int CamMan::CreateViewFromDef(CHashedString pcViewInfo)
{
	CamViewInfo* pInfo = ObjectDatabase::Get().GetPointerFromName<CamViewInfo*>(pcViewInfo);

	if(!pInfo)
	{
		ntError_p(true, ("Could not find a CamViewInfo definition named '%s'\n", ntStr::GetString(pcViewInfo)));
		return -1;
	}
	
	pInfo->m_iID = -1;
	return  Get().CreateView(*pInfo);
}



//------------------------------------------------------------------------------------------
//!
//!	CamMan::CreatePrimaryViewFromDef
//!	Create a new primary camera view
//!
//------------------------------------------------------------------------------------------
int CamMan::CreatePrimaryViewFromDef(CHashedString pcViewInfo)
{
	CamViewInfo* pInfo = ObjectDatabase::Get().GetPointerFromName<CamViewInfo*>(pcViewInfo);

	if(!pInfo)
	{
		ntError_p(true, ("Could not find a CamViewInfo definition named '%s'\n", ntStr::GetString(pcViewInfo)));
		return -1;
	}

	pInfo->m_iID = 0;
	return  Get().CreateView(*pInfo);
}

#ifndef _RELEASE


//------------------------------------------------------------------------------------------
//!
//!	CamMan::DebugRender
//!	Render some camera debug info
//!
//------------------------------------------------------------------------------------------
void CamMan::RenderTimeCurve()
{
	if(m_pTSCCurve)
		m_pTSCCurve->RenderTimeCurve();
}

#endif

