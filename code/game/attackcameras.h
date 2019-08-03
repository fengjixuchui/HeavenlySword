//------------------------------------------------------------------------------------------
//!
//!	\file attackcameras.h
//!
//------------------------------------------------------------------------------------------


#ifndef _AttackCameras_INC
#define _AttackCameras_INC


//------------------------------------------------------------------------------------------
// Referenced classes and forward declarations
//------------------------------------------------------------------------------------------
class CAttackData;
class CStrike;
class CEntity;
class Transform;
class TimeScalarCurve;
class CoolCam_MayaAnimator;

//------------------------------------------------------------------------------------------
//!
//!	CCombatCamProps
//!
//! General properties of Combat Cameras
//!	
//------------------------------------------------------------------------------------------
class CCombatCamProps
{
public:
	HAS_INTERFACE(CCombatCamProps)
	CCombatCamProps();
	virtual ~CCombatCamProps();
	virtual void PostConstruct();

	bool  AreCombatCamsEnabled() const {return m_bEnableCombatCams;}
	bool  IsTimeScalingEnabled() const {return m_bEnableTimeScaling;}
	float GetEarlyExitMinimum()  const {return m_fEarlyExitTime;}

private:
	friend class CombatCamPropsI;

	bool  m_bEnableCombatCams;
	bool  m_bEnableTimeScaling;
	float m_fEarlyExitTime;
};

//------------------------------------------------------------------------------------------
//!
//!	CombatCamProperties
//!
//! General properties of Combat Cameras, derived from CSingleton.
//!	
//------------------------------------------------------------------------------------------
class CombatCamProperties : public Singleton<CombatCamProperties>
{
public:
	CombatCamProperties(const CCombatCamProps& props);
	virtual ~CombatCamProperties() {;}

	bool  AreCombatCamsEnabled()	const {return m_data.AreCombatCamsEnabled();}
	bool  IsTimeScalingEnabled()	const {return m_data.IsTimeScalingEnabled();}
	float GetEarlyExitMinimum()     const {return m_data.GetEarlyExitMinimum();}

private:
	const CCombatCamProps& m_data;
};


//------------------------------------------------------------------------------------------
//!
//!	CAttackCamera
//!
//! Associate a camera animation with a particular attack.
//!	
//------------------------------------------------------------------------------------------
class CAttackCamera
{
public:
	HAS_INTERFACE(CAttackCamera)
	CAttackCamera();
	virtual ~CAttackCamera() {;}

private:
	friend class CAttackCameraI;
	friend class CAttackCameraList;

	//CAttackData*	  m_pobAttack;
	CHashedString		  m_obCamera;
	TimeScalarCurve*  m_pobTSCurve;
	float			  m_fDelay;
	bool			  m_bAllowEarlyExit;
};

//------------------------------------------------------------------------------------------
//!
//!	CAttackCameraList
//!
//! Associate camera animations with attacks.
//!	
//------------------------------------------------------------------------------------------
class CAttackCameraList
{
public:
	HAS_INTERFACE(CAttackCameraList)
	CAttackCameraList();
	virtual ~CAttackCameraList();

	int ActivateCombatCam(const CStrike* pStrike, float fButtonHeldTime, const Transform* pTransform);
	int DeactivateCombatCam(int iID, bool bForce = false);
	bool IsActive() const;
	ntstd::String GetName() const;

	void PostPostConstruct();

// Members
private:
	CAttackCamera* m_pCurrentCamera;
	int            m_iCurrentCameraID;

	static const int      MAX_CAMS = 4;
	CoolCam_MayaAnimator* m_pCoolAnimator[MAX_CAMS];

// Welded Members
private:
	friend class CCAttackCameraListI;

	ntstd::String					m_sCameraClump;
	ntstd::String					m_sAnims;
	ntstd::List<CAttackCamera*>	m_obCameras;
	float                       m_fTransitionOutTime;
};

#endif //_AttackCameras_INC
