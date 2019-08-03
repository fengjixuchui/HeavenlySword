//------------------------------------------------------------------------------------------
//!
//!	\file CoolCam_Maya.h
//!
//------------------------------------------------------------------------------------------

#ifndef	_CoolCam_Maya_INC
#define	_CoolCam_Maya_INC


//------------------------------------------------------------------------------------------
// Required Includes
//------------------------------------------------------------------------------------------
#include "camera/camcool.h"
#include "game/entityanimset.h"
#include "anim/animation.h"


//------------------------------------------------------------------------------------------
// External Classes
//------------------------------------------------------------------------------------------

class Transform;
class CClumpHeader;
class CHierarchy;
class CAnimator;
class CMatrixTweakerEditor;


//------------------------------------------------------------------------------------------
//!
//!	CoolCam_MayaAnimator
//!	A simple animation player for maya cameras
//!
//------------------------------------------------------------------------------------------
class CoolCam_MayaAnimator : private EntityAnimSet
{
public:
	CoolCam_MayaAnimator(const char* sClump, const char* sAnims);
	~CoolCam_MayaAnimator();

	CAnimator*  GetAnimator()  {return m_pAnimator;}
	CHierarchy* GetHierarchy() {return m_pHierarchy;}
	void        InstallGetName(StringFunctor *get_name_func) {EntityAnimSet::InstallGetName(get_name_func);}

public:
	CClumpHeader* m_pClumpHeader;
	CHierarchy*   m_pHierarchy;
	CAnimator*    m_pAnimator;
};


//------------------------------------------------------------------------------------------
//!
//!	CoolCam_MayaDef
//!	Definition for a Maya Cool Camera
//!
//------------------------------------------------------------------------------------------
struct CoolCam_MayaDef
{
	CoolCam_MayaDef() : pCoolAnimator(0), pCurve(0), fDuration(0.f), pTweaker(0) {;}

	CoolCam_MayaAnimator* pCoolAnimator;
	CHashedString         sAnim;
	TimeScalarCurve*      pCurve;
	float                 fDuration;
	CMatrixTweakerEditor* pTweaker;
};


//------------------------------------------------------------------------------------------
//!
//!	CoolCam_Maya
//!	A camera which follows an animation exported from Maya.
//!
//------------------------------------------------------------------------------------------
class CoolCam_Maya : public CoolCamera
{
public:
	CoolCam_Maya(const CamView& view, CoolCam_MayaDef& def);
	//CoolCam_Maya(const CamView& view);
	virtual ~CoolCam_Maya();

	// Init will be removed asap and replaced with a single constructor...
	//bool Init(const CHashedString& sAnim, const char* pcAnimContainerName = 0);             // Used by cutscene cameras
	void SetAttacker(const CEntity* pEnt);
	void SetFinished();

	// Set a matrix or a transform to parent this camera from.
	void SetMatrix(const CMatrix& mat);
	void SetParentTransform(Transform& transform);
	
	// I will try to remove these...
	void SetAutoFinish(bool bAuto)                     {m_bAutoFinish = bAuto;}
	float GetAnimDuration(void) const                  {ntAssert(m_pAnim); return m_pAnim->GetDuration();}

	// Set an event handler for the camera
	void SetEventHandler(const CEntity* pInformEntity) {m_pInformEntity = pInformEntity;}


	virtual void        Update(float fTimeDelta);
	virtual bool        IsActive() const;
	virtual bool        HasFinished() const;
	virtual void        EndCamera();
	virtual CHashedString GetCameraName() const;
	virtual CAMTYPE     GetType() const		      {return CT_MAYA;}

	// Return the entity assigned to the camera
	CoolCam_MayaAnimator* GetCoolAnimator() const {return m_pCoolAnimator;}

// Helper Functions
private:
	//void				CreateCameraEntity(const char* pcAnimContainerName);

// Members
protected:
	CoolCam_MayaAnimator* m_pCoolAnimator;
	CAnimationPtr	      m_pAnim;
	bool                  m_bAutoFinish;
	CPoint                m_ptTranslation;
	CMatrix               m_matrix;
	const CEntity*        m_pAttacker;
	const CEntity*	      m_pInformEntity;
	CMatrixTweakerEditor* m_pTweaker;

	// Debug Info
	CHashedString      m_sAnim;
};

#endif //_ CoolCam_Maya_INC
