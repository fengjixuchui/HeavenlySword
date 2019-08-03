//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file basiccamera.h
//!                                                                                         
//------------------------------------------------------------------------------------------

#ifndef _BASICCAMERA_INC
#define _BASICCAMERA_INC

//------------------------------------------------------------------------------------------
// Forward Declarations
//------------------------------------------------------------------------------------------
class BasicCameraTemplate;
class BasicCamera;
class CamTransitionDef;
class CamView;
class CamVolume;
class MotionControllerDef;
class MotionController;
class LookAtControllerDef;
class LookAtController;
class LensControllerDef;
class LensController;
class CCombatCamDef;
class CombatCam;
class CMatrixTweakerEditor;
class CEntity;

//------------------------------------------------------------------------------------------
// Required Includes
//------------------------------------------------------------------------------------------
#include "camera/camerainterface.h"
#include "lua/ninjalua.h"

//------------------------------------------------------------------------------------------
//!
//!	BasicCameraTemplate
//!	A template to construct a basic game camera from.
//!
//------------------------------------------------------------------------------------------
class BasicCameraTemplate
{
public:
	BasicCameraTemplate();
	virtual ~BasicCameraTemplate() {;}

	HAS_LUA_INTERFACE()

	virtual void         PostConstruct();
	virtual void		 PostPostConstruct();
	CHashedString        GetCameraName() const;
	BasicCamera*         Instance(const CamView& view);
	bool                 IsDebug() const {return m_bDebugCam;}

	// Directly activate this camera - this should 
	// be temporary, need to look at this contruct - GH
	void Activate( void );

	// Transitions
	const CamTransitionDef* GetTransitionTo(const BasicCameraTemplate* pTo) const;
	const CamTransitionDef* GetDefaultTransition() const;

private: 
	// The components
	MotionControllerDef*   m_pMotionControllerDef;
	LookAtControllerDef*   m_pLookAtControllerDef;
	LensControllerDef*	   m_pLensControllerDef;
	CMatrixTweakerEditor*  m_pTweaker;
	CCombatCamDef*		   m_pCombatCamDef;

	// Parent entity for the camera.
	CEntity*               m_pParentEntity;

	// Transitions
	typedef ntstd::List<CamTransitionDef*, Mem::MC_CAMERA> TransitionList;
	TransitionList	                                       m_transitionList;

	// DoF
	CEntity* m_pDoFEnt;
	float    m_fDoFFarDist;
	float    m_fDoFNearDist;


protected:
	bool m_bDebugCam;

	friend class BasicCameraTemplateInterface;
	friend class BasicCamera;
};

class DebugCameraTemplate : public BasicCameraTemplate
{
public:
	DebugCameraTemplate() {m_bDebugCam=true;}
};

LV_DECLARE_USERDATA(BasicCameraTemplate);

//------------------------------------------------------------------------------------------
//!
//!	BasicCamera
//!	A basic normal level camera made up of Lens, Motion and Look At Controllers
//!
//------------------------------------------------------------------------------------------
class BasicCamera : public CameraInterface
{
public:
	BasicCamera(const BasicCameraTemplate& bct, const CamView& view);
	virtual ~BasicCamera();

	// Virtual interface
	virtual CHashedString GetCameraName() const            {return m_template.GetCameraName();}
	virtual void        Update(float fTimeDelta);
	virtual void        Reset();
	virtual bool        IsActive() const                 {return true;}
	virtual CAMTYPE     GetType() const                  {return CT_BASIC;}
	virtual int         GetPriority() const              {return 0;}
	virtual bool        AllowShakes() const              {return true;}
#ifndef _RELEASE
	virtual void RenderDebugInfo();
#endif

	// Classification
	bool IsInstanceOf(BasicCameraTemplate* pTemplate)           {return pTemplate == &m_template;}

	// Transitions
	const CamTransitionDef* GetTransitionTo(BasicCamera* pTo);

	//    scee.sbashow:added                                                                                     
	//		This is useful if you require to hook up a transition under a particular set of circumstances                                                                      
	// 		(See defn of GetTransitionTo), but the scratch should last no longer than the lifetime of obCamDef...
	//		So use with care. (also, only one scratch trans can be set at a time.)
	void  SetScratchTransition(const CamTransitionDef& obCamDef) const 	{	m_pobScratchTransition = &obCamDef;			}
	const CamTransitionDef* GetDefaultTransition()              		{	return m_template.GetDefaultTransition();	}

	// Depth of Field
	void SetDoF(const CEntity* pEntFocal, float fFar, float fNear, float fTransitionTime);

	// Components
	const MotionController* GetMC()        const {return m_pMotionController;}
	const LookAtController* GetLAC()       const {return m_pLookAtController;}
	const LensController*   GetLC()        const {return m_pLensController;}
	CombatCam*              GetCombatCam() const {return m_pCombatCam;}

	// Debug Info
	virtual void DebugRender();

	// Our template
	const BasicCameraTemplate& m_template;

protected:
	// The components
	MotionController*	   m_pMotionController;
	LookAtController*	   m_pLookAtController;
	LensController*		   m_pLensController;
	CMatrixTweakerEditor*  m_pTweaker;
	CCombatCamDef*		   m_pCombatCamDef;
	CombatCam*             m_pCombatCam;

	// Depth of Field Settings
	const CEntity* m_pDoFEnt;
	float          m_fDoFFar;
	float          m_fDoFNear;

	const CEntity* m_pOldDoFEnt;
	float          m_fOldDoFFar;
	float          m_fOldDoFNear;

	float          m_fTransitionTime;
	float          m_fTotalTransitionTime;
	float          m_fTweakTime;

	mutable const CamTransitionDef*	m_pobScratchTransition;
};

#endif  //_BASICCAMERA_INC
