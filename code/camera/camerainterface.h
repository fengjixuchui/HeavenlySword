//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file camerainterface.h
//!                                                                                         
//------------------------------------------------------------------------------------------

#ifndef _CAMERAINTERFACE_INC
#define _CAMERAINTERFACE_INC

class CamView;
class CamSceneElementMan;

//------------------------------------------------------------------------------------------
// Enums
//------------------------------------------------------------------------------------------
enum CAMTYPE
{
	CT_TRANSITION = -2,
	CT_VOID = -1,
	CT_BASIC,
	CT_DEBUG,
	CT_ROTATING,
	CT_AFTERTOUCH,
	CT_MAYA,
	CT_TURRET,
	CT_AERIAL,
	CT_LADDER,
	CT_NULL,
	CT_CHASE,
	CT_AIM,
	CT_BOSS,
	CT_CHASEAIMCOMBO
};


//------------------------------------------------------------------------------------------
//!
//!	CameraInterface
//!	The abstract base class for all managed cameras
//!
//------------------------------------------------------------------------------------------
class CameraInterface
{
public:
	CameraInterface(const CamView& view);
	virtual ~CameraInterface() {;}

	// Meta
	// -----------------------------
	virtual int  GetPriority() const                 = 0;
	bool IsType(CAMTYPE eType) const                 {return eType == GetType();}

	// Virtual Interface
	// -----------------------------
	virtual int           GetID() const                    {return 0;}
	virtual void          Update(float fTimeDelta)         = 0;
	virtual void          Reset()                          {;}
	virtual bool	      IsActive() const                 = 0;
	virtual CHashedString GetCameraName() const            {return CHashedString(HASH_STRING_THE_CAMERA_WITH_NO_NAME);}
	virtual CAMTYPE       GetType() const                  = 0;
	virtual bool          Is(CameraInterface* pCam) const  {return this == pCam;}
	virtual bool          AllowShakes() const              {return false;}
#ifndef _RELEASE
	virtual void          RenderDebugInfo()                {;}
#endif

	// Camera properties
	// -----------------------------
	const CPoint&  GetLookAt()    const              {return m_obLookAt;}
	const CMatrix& GetTransform() const              {return m_obTransform;}
	float          GetFOV()       const              {return m_fFOV;}

	// Depth of Field
	// -----------------------------
	bool  UsingDoF()         const                   {return m_bUseDoF;}
	float GetFocalDepth()    const                   {return m_fFocalDepth;}
	float GetNearBlurDepth() const                   {return m_fNearBlurDepth;}
	float GetFarBlurDepth()  const                   {return m_fFarBlurDepth;}
	float GetConfusionHigh() const                   {return m_fConfusionHigh;}
	float GetConfusionLow()  const                   {return m_fConfusionLow;}

	// Motion Blurring
	// -----------------------------
	bool  UsingMotionBlur() const                    {return m_bUseMotionBlur;}
	float GetMotionBlur()   const                    {return m_fMotionBlur;}

	// Time Scaling
	// -----------------------------
	virtual float GetTimeScalar() const              {return 1.0f;}

	// Parent View
	// -----------------------------
	const CamView&            GetView()           const {return m_view;}
	const CamSceneElementMan& GetElementManager() const;

	// Misc Methods.
	// ----------------------------
	bool CheckTransformValid() const;
	
	bool IsInView(const CPoint& pt, bool bObscureCheck = false);

// Members for use by derived classes
protected:
	CPoint         m_obLookAt;
	CMatrix        m_obTransform;
	float          m_fFOV;

	// Depth of Field
	bool           m_bUseDoF;
	float          m_fFocalDepth;
	float          m_fNearBlurDepth;
	float          m_fFarBlurDepth;
	float          m_fConfusionHigh;
	float          m_fConfusionLow;

	// Radial Blur
	bool          m_bUseMotionBlur;
	float         m_fMotionBlur;
	
	// Owning view.
	const CamView& m_view;
	friend class CamView;
};

#endif  //_CAMERAINTERFACE_INC
