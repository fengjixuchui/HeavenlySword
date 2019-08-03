//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file camview.h
//!                                                                                         
//------------------------------------------------------------------------------------------


#ifndef _CAMVIEW_INC
#define _CAMVIEW_INC


//------------------------------------------------------------------------------------------
// Required Includes
//------------------------------------------------------------------------------------------
#include "anim/transform.h"
#include "gfx/camera.h"
#include "game/transformcontroller.h"
#include "camera/camerainterface.h"
#include "camera/camtransitiontree.h"
#include "camera/elementmanager.h"
#include "lua/ninjalua.h"


//------------------------------------------------------------------------------------------
// External Declarations
//------------------------------------------------------------------------------------------
class CEntity;
class Boss;
class CoolCamera;
class CamTransitionDef;
class BasicCamera;
class BasicCameraTemplate;
class DebugChaseCamera;
class CoolCam_MayaAnimator;

class CoolCam_AfterTouchDef;
class CoolCam_ChaseDef;
class CoolCam_AimDef;

struct CamShake
{
	CamShake(float fTime, float fAmp, float fFreq) 
		: m_fTimeTotal(fTime), m_fTime(fTime), m_fAmplitude(fAmp), m_fFrequency(fFreq), m_fRadiusSqrd(-1.f) {;}

	float  GetCurrentShake() const
	{
		if(m_fTime < m_fTimeTotal * 0.5f)
			return m_fAmplitude * m_fTime / (0.5f*m_fTimeTotal);
		else
			return m_fAmplitude;
	}

	float GetRelativeShake(const CPoint& pt) const
	{
		if(m_fRadiusSqrd > 0.f)
		{
			CDirection dDelta = pt ^ m_pt;
			float fDistSqrd = dDelta.LengthSquared();
			float fFactor = clamp(1.f - (fDistSqrd / m_fRadiusSqrd), 0.f, 1.f);

			return fFactor * GetCurrentShake();
		}
		else
		{
			return GetCurrentShake();
		}

	}

	float  m_fTimeTotal;
	float  m_fTime;
	float  m_fAmplitude;
	float  m_fFrequency;
	CPoint m_pt;
	float  m_fRadiusSqrd;

};



//------------------------------------------------------------------------------------------
//!
//!	CamViewInfo                                                                          
//!	The extents of the camera view.
//!
//------------------------------------------------------------------------------------------
class CamViewInfo
{
public:
	CamViewInfo() : m_pPrimaryEntity(0), m_bPrimaryLock(false), m_iID(-1), m_iPriority(0),
		            m_fTop(0.f), m_fLeft(0.f), m_fWidth(1.f), m_fHeight(1.f),
					m_fFinalTop(0.f), m_fFinalLeft(0.f), m_fFinalRight(1.f), m_fFinalBottom(1.f),
	                m_fInitialTop(0.f), m_fInitialLeft(0.f), m_fInitialRight(1.f), m_fInitialBottom(1.f),
					m_fTotalTransTime(0.f), m_fTransTime(0.f), m_bTransitioning(true) {;}

	CamViewInfo(int iID, float fTop, float fLeft, float fWidth, float fHeight, CEntity* pEnt);

//Accessors
public:
	int	     GetID() const                   {return m_iID;}

	// Primary Entity
	CEntity* GetPrimaryEntity() const        {return m_pPrimaryEntity;}
	void     SetPrimaryEntity(CEntity* pEnt) {m_pPrimaryEntity = pEnt;}
	bool     IsPrimaryLocked() const         {return m_bPrimaryLock;}   // Do we only look at the primary entity

	// View Priority
	int      GetPriority() const             {return m_iPriority;}
	void     SetPriority(int iPriority)      {m_iPriority = iPriority;}

	// Screen Co-ords
	float    GetTop() const           {return m_fTop;}
	float    GetLeft() const          {return m_fLeft;}
	float    GetWidth() const         {return m_fWidth;}
	float    GetHeight() const        {return m_fHeight;}

	void     FadeTo(uint32_t iColour, float fTime)   {m_fFadeTime=0.f; m_fFadeTotalTime=fTime; m_iFadeColour = iColour; m_bFadeTo = true; m_bFading = true;}
	void     FadeFrom(uint32_t iColour, float fTime) {m_fFadeTime=0.f; m_fFadeTotalTime=fTime; m_iFadeColour = iColour; m_bFadeTo = false; m_bFading = true;}

	bool     AllowShakes() const {return m_bAllowShakes;}

// Methods
public:
	void     Update(float fTimeDelta);
	void     MoveView(float fTop, float fLeft, float fBottom, float fRight, float fTime);

private:
	// The primary focus entity for the views cameras
	CEntity* m_pPrimaryEntity;
	bool     m_bPrimaryLock;

	// The id and priority of this view
	int      m_iID;
	int      m_iPriority;

	// The position and dimensions of the view
	float    m_fTop;
	float    m_fLeft;
	float    m_fWidth;
	float    m_fHeight;

	// The final position and dimensions of the view
	float    m_fFinalTop;
	float    m_fFinalLeft;
	float    m_fFinalRight;
	float    m_fFinalBottom;

	// The initial position and dimensions of the view
	float    m_fInitialTop;
	float    m_fInitialLeft;
	float    m_fInitialRight;
	float    m_fInitialBottom;

	// Fade Data
	float    m_fFadeTime;
	float    m_fFadeTotalTime;
	uint32_t m_iFadeColour;
	bool     m_bFadeTo;
	bool     m_bFading;

	// The time to perform the lerp from initial to final
	float    m_fTotalTransTime;

	// Lerp Data
	float    m_fTransTime;
	bool     m_bTransitioning;

	// Allow Camera Shakes?
	bool     m_bAllowShakes;


	friend class CamMan;
	friend class CamViewInfoInterface;
};



//------------------------------------------------------------------------------------------
//!
//!	CamView                                                                                 
//!	A camera view.  A view displays a camera (or a transition between two cameras) there
//! be multiple views on the manager at any time.
//!
//------------------------------------------------------------------------------------------
class CamView : public CCamera
{
public:
	CamView(const CamViewInfo& info);
	~CamView();

	HAS_LUA_INTERFACE()

	// ----------------------------------------------------------------------------------------------
	// Init/Update/Cleanup
	// ----------------------------------------------------------------------------------------------
	void Init();
	void Update();
	void DestroyView();

	bool IsActive() const        {return m_bActive;}
	void SetActive(bool bActive);

	const CamViewInfo& GetViewInfo() const {return m_info;}

	// NOTE, this is hard wired at the moment to 16/9. this should be a genuine function of a view's width and height, something not 
	// currently implemented.  Also, this is unrelated to the aspect ratio used to build the the view->screen space transform in the 
	// renderer, as that is only available within a given rendering context, and NOT within regular game code.
	float GetAspectRatio() const { return (16.f*m_info.GetWidth()/9.f*m_info.GetHeight()); }
	
	// ----------------------------------------------------------------------------------------------
	// Cool Cameras
	// ----------------------------------------------------------------------------------------------
	void        AddCoolCamera(CoolCamera* pCoolCamera, CamTransitionDef* pTransition = 0);
	void        RemoveCoolCamera(int iCamID);
	void        RemoveAllCoolCameras(bool bIncludingMayaCams = false);
	void        Lua_RemoveAllCoolCameras() {RemoveAllCoolCameras(false);}
	bool        IsCoolCamActive()  const   {return m_pCurrCamera->GetType() > CT_DEBUG;}
	bool        IsDebugCamActive() const   {return m_pCurrCamera->GetType() == CT_DEBUG;}

	CoolCamera* GetCoolCam(int iID);
	CoolCamera* FindCoolCam(CAMTYPE eType) const;
	int			GetNumCoolCams() const;
	// ----------------------------------------------------------------------------------------------
	// Level Cameras
	// ----------------------------------------------------------------------------------------------
	void                   ActivateLevelCamera(BasicCameraTemplate* pCamTemplate);
	const CameraInterface* GetLevelCamera() const;
	bool                   IsLevelCameraInUse(BasicCamera* pCam) const;

	// ----------------------------------------------------------------------------------------------
	// Manipulate the view on screen
	// ----------------------------------------------------------------------------------------------
	void MoveView(float fTop, float fLeft, float fBottom, float fRight, float fTime);
	void FadeTo(uint32_t iColour, float fTime)   {m_info.FadeTo(iColour, fTime);}
	void FadeFrom(uint32_t iColour, float fTime) {m_info.FadeFrom(iColour, fTime);}

	// ----------------------------------------------------------------------------------------------
	// Camera Shakes
	// ----------------------------------------------------------------------------------------------
	void ShakeView(const CamShake& shake);
	void UpdateShake();

	// ----------------------------------------------------------------------------------------------
	// Accessors
	// ----------------------------------------------------------------------------------------------
	int                    GetActiveCameraID()   {return m_pCurrCamera ? m_pCurrCamera->GetID() : 0;}
	bool                   IsTransitionActive();
	const CamTransition*   ActiveTransition() { return m_transMan.GetActiveTransition(); }

	// Camera View 
	CMatrix				   GetCurrMatrix() const {return m_transform.GetLocalMatrix();}
	const Transform&       GetTransform()  const {return m_transform;}
	bool                   IsInView(const CPoint& pt, bool bObscureCheck = false) {return m_pCurrCamera->IsInView(pt, bObscureCheck);}

	// Camera Meta Data
	int                    GetPriority()            const;
	float                  GetTimeScalar()          const;
	bool                   IsUsingCombatCameras()   const {return m_bUseCombatCameras;}
	void                   UseCombatCameras(bool b)       {m_bUseCombatCameras = b;}

	// DoF Stuff
	const CEntity*         GetDoFEntity()     const {return m_pDoFEntity;}
	float                  GetDofFar()        const {return m_fDoFFar;}
	float                  GetDoFNear()       const {return m_fDoFNear;}
	float                  GetConfusionHigh() const {return m_fConfusionHigh;}
	float                  GetConfusionLow()  const {return m_fConfusionLow;}

	const CamSceneElementMan& GetElementManager() const {return m_elementMan;}
	void SetPrimaryEntity(CEntity* pEnt);


//-----------------------------------------------------------------------------------------------------------------
// Lua Bind Methods - Cool camera ones shouldn't really be here, would be architecturally nicer to have
//                    the cool cameras created somewhere else and then applied to the view here.
//-----------------------------------------------------------------------------------------------------------------
public:
	int  ActivateAfterTouchCoolCam(CEntity* pThrownItem, CHashedString pcAftertouchDef);
	int  ActivateAfterTouchCoolCamDef(CEntity* pentTarget, CoolCam_AfterTouchDef* pobDefinition);
	void AfterTouchCoolCamLookAt(CEntity* pLookAtEnt, int iHitType);
	int	 ActivateRotatingCamera(CEntity* pEnt1, CEntity* pEnt2, float fTime, float fAngle);
	int  ActivateChaseAimCam(CEntity* pEnt, CHashedString pcChaseDef, CHashedString pcAimDef);
	int  ActivateChaseAimCamDef(CEntity* pEnt, CoolCam_ChaseDef* pChaseDef, CoolCam_AimDef* pAimDef);
	bool AddFixedEndTrans(int iCamID, float fTransTime );
	int  ActivateChaseCam(CEntity* pEnt, const char* pcChaseDef);
	int  ActivateAimCam(CEntity* pEnt, const char* pcAimDef);
	int  ActivateMayaCam(const char* pcAnim, CEntity* pInformEntity, CEntity* pParentEntity);
	int  ActivateTurretCamera(CEntity* pEnt, CHashedString pszTransform, const CDirection& dirCamOffset );
	int  ActivateBossCamera(CEntity* pobPlayer, CEntity* pobBoss);

//	int  ActivateGladiatorGeneralCamera( CEntity* pobGladiatorGeneral );
	int  ActivateMappedGladiatorGeneralCamera( const char* pcCamDefName );
	int  ActivateMappedKingBossFightCamera( const char* pcCamDefName );
	int  ActivateMappedAerialGeneralCamera( const char* pcCamDefName );
	int  ActivateMappedWaterGeneralCamera( const char* pcCamDefName );

	int  ActivateMappedKingBossFightCamerav2( const char* pcCamDefName );
	void ActivateBossCameraReverseAngle( const char* pcCamDefName );
	void DeactivateBossCameraReverseAngle( const char* pcCamDefName );
	void ActivateReverseAngleLeadIn( const char* pcCamDefName );
	void DeactivateReverseAngleLeadIn( const char* pcCamDefName );

	void SetBossCameraFOV( const char* pcCamDefName, float fFOV );
	void SetBossCameraFOVBlendTime( const char* pcCamDefName, float fFOVBlendTime );

	void ActivateAerialGeneralOrbitMode( const char* pcCamDefName );
	void DeactivateAerialGeneralOrbitMode( const char* pcCamDefName );
	void ActivateAerialGeneralOrbitAcceleration( const char* pcCamDefName );
	void DeactivateAerialGeneralOrbitAcceleration( const char* pcCamDefName );

	void SetAerialGeneralOrbitAcceleration( const char* pcCamDefName, float fAcceleration );
	void SetAerialGeneralOrbitMaxSpeed( const char* pcCamDefName, float fMaxSpeed );

	ntstd::String GetName() const {return "CamView";}

	void SetDoF(const CEntity* pEntFocal, float fFar, float fNear, float fConfusionHigh, float fConfusionLow, float fTransitionTime);

// ----------------------------------------------------------------------------------------------
// Helper Functions
// ----------------------------------------------------------------------------------------------
protected:
	void             ActivateTransition(const CameraInterface* pSrc, const CameraInterface* pDst, const CamTransitionDef* pDef);
	void             ActivateEndingTransition(CoolCamera* pCam, CamTransitionDef* pDef);

// ----------------------------------------------------------------------------------------------
// Members
// ----------------------------------------------------------------------------------------------
private:
	CamViewInfo           m_info;
	bool                  m_bActive;

	// The camera to play scripted animations on for this view
	CoolCam_MayaAnimator* m_pCoolAnimator;

	// Active Cameras on this view
	ntstd::List<CoolCamera*, Mem::MC_CAMERA>    m_coolCameras;
	ntstd::List<BasicCamera*, Mem::MC_CAMERA>   m_levelCameras;

	// Current Cameras
	CameraInterface*      m_pCurrCamera;
	BasicCamera*          m_pCurrLevelCamera;
	CameraInterface*      m_pViewCamera;

	// Transitions and Element Manager
	CamTransitionTree     m_transMan;
	CamSceneElementMan    m_elementMan;

	// View Attributes
	Transform            m_transform;

	// Camera Shake
	CamShake             m_shake;

	// Depth of Field Defaults.
	const CEntity*        m_pDoFEntity;
	float                 m_fDoFFar;
	float                 m_fDoFNear;
	float                 m_fConfusionHigh;
	float                 m_fConfusionLow;

	// Flags
	bool                  m_bUseDoF;
	bool                  m_bUseCombatCameras;


// ----------------------------------------------------------------------------------------------
// Debug Enumerations
// ----------------------------------------------------------------------------------------------
public:
	enum eDEBUG_MODE
	{
		DM_NORMAL = 0,		// all running normally
		DM_FREE,			// debug free floating camera
		DM_REL,				// debug tracking cam where we control look at angle, target dist + matrix tweaker
		DM_TOPDOWN
	};


// ----------------------------------------------------------------------------------------------
// Debug Camera Interface
// ----------------------------------------------------------------------------------------------
public:
	void               UpdateDebugCamera(float fTimeDelta);
	void               SetDebugCameraMode(eDEBUG_MODE eMode);
	eDEBUG_MODE		   GetDebugCameraMode() { return m_eDebugMode; };
	void               SwitchDebugCameraMode();
	void               SwitchDebugCameraStyle();
	DebugChaseCamera*  GetDebugCamera();
	void               SetDebugCameraSpeed(float f)    {m_fDebugCameraSpeed = f;}
	void               SetDebugCameraPlacement(const CPoint& ptPos, const CPoint& ptTarg);
	CMatrix			   GetDebugControllerMatrix()      {return m_debugControl.GetTransform().GetLocalMatrix();}
	void               ToggleDebugRender() {;}


// ----------------------------------------------------------------------------------------------
// Debug Camera Members
// ----------------------------------------------------------------------------------------------
private:
	eDEBUG_MODE           m_eDebugMode;
	DebugChaseCamera*     m_pDebugCamera;
	float			      m_fDebugFOV;
	float                 m_fDebugDist;
	CTransformController  m_debugControl;
	float                 m_fDebugCameraSpeed;
#ifndef _GOLD_MASTER
	void             DebugRender();
#endif
};

#include "lua\ninjalua.h"
LV_DECLARE_USERDATA(CamView);

#endif //_CAMVIEW_INC
