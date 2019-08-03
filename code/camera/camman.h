//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file camman.h
//!                                                                                         
//------------------------------------------------------------------------------------------


#ifndef _CAMMAN_INC
#define _CAMMAN_INC


//------------------------------------------------------------------------------------------
// Required Includes
//------------------------------------------------------------------------------------------
#include "gfx/camera.h"
#include "input/inputhardware.h"

//------------------------------------------------------------------------------------------
// External Declarations
//------------------------------------------------------------------------------------------
class BasicCameraTemplate;
class CamView;
class CamViewInfo;
class TimeScalarCurve;
namespace NinjaLua { class LuaState; class LuaObject; }


//------------------------------------------------------------------------------------------
//!
//!	CamMan
//!	The new camera manager
//!
//------------------------------------------------------------------------------------------
class CamMan : public Singleton<CamMan>
{
public:
	// Construction / Destruction
	// --------------------------
	CamMan();
	~CamMan();

	// Init / Update
	//---------------------
	void Init();
	void Update();

	// Basic Camera Methods
	// --------------------
	void AddLevelCameraTemplate(BasicCameraTemplate* pTemplate);
	void RemoveLevelCameraTemplate(BasicCameraTemplate* pTemplate);

	// View Management
	// --------------------
	int             CreateView(CamViewInfo& info);
	void			KillView(int iView);
	CamView*        GetView(int iView)                   {ntAssert(iView >= 0 && iView < MAX_VIEWS); return m_views[iView];}
	static CamView* GetPrimaryView()                     {return CamMan::Get().GetView(CamMan::Get().m_iPrimaryView);}

	// Shake Methods
	// --------------------
	void            Shake(float fTime, float fAmp, float fFreq, CPoint* pPt = 0, float fRadiusSqrd = -1.f);
	static void		Lua_Shake(float fTime, float fAmp, float fFreq) {CamMan::Get().Shake(fTime, fAmp, fFreq);}
	static int		Lua_ShakePositioned(NinjaLua::LuaState* State);

	// Combat Aware Cameras
	// --------------------
	bool IsCombatAware()               {return m_bCombatAware;}
	static void SetCombatAware(bool b) {Get().m_bCombatAware = b;} // Static for lua exposure.


// ---------------------------------------------------------------------------------------------------
// Helper Functions
// ---------------------------------------------------------------------------------------------------
protected:
	void CleanUp();
	BasicCameraTemplate* FindLevelCamera(const CHashedString& sName);
	static int GetLevelCameraForLua(NinjaLua::LuaState& State);
	static int GetViewForLua(NinjaLua::LuaState& State);
	static int CreatePrimaryViewFromDef(CHashedString pcViewInfo);
	static int CreateViewFromDef(CHashedString pcViewInfo);


// ---------------------------------------------------------------------------------------------------
// Statics
// ---------------------------------------------------------------------------------------------------
private:
	static const int MAX_VIEWS = 8;

// ---------------------------------------------------------------------------------------------------
// Members
// ---------------------------------------------------------------------------------------------------
private:
	bool                                m_bReady;
	ntstd::Vector<BasicCameraTemplate*, Mem::MC_CAMERA> m_levelCameraTemplates;
	CamView*                            m_views[MAX_VIEWS];
	int                                 m_iPrimaryView;
	bool                                m_bCombatAware;

// ---------------------------------------------------------------------------------------------------
// Debug Camera Methods
// ---------------------------------------------------------------------------------------------------
public:
	static PAD_NUMBER GetDebugCameraPadNumber() { return m_eDebugCameraPad; }
	static void SetDebugCameraPadNumber(PAD_NUMBER ePad) { m_eDebugCameraPad = ePad; }


// ---------------------------------------------------------------------------------------------------
// Debug Camera Members
// ---------------------------------------------------------------------------------------------------
private:
	// The number of the pad used to control the debug camera
	static PAD_NUMBER		m_eDebugCameraPad;


// ---------------------------------------------------------------------------------------------------
// Debug Visualisation
// ---------------------------------------------------------------------------------------------------
#ifndef _RELEASE
public:
	void SetSelectedCurve(TimeScalarCurve* pTSCCurve) {m_pTSCCurve = pTSCCurve;}
	void RenderTimeCurve();

private:
	TimeScalarCurve* m_pTSCCurve;
#else
public:
	void SetSelectedCurve(TimeScalarCurve*) {;}
	void RenderTimeCurve() {;}
#endif
};


#endif  //_CAMMAN_INC
