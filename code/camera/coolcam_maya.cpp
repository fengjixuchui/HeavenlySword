//------------------------------------------------------------------------------------------
//!
//!	\file CoolCam_Maya.cpp
//!
//------------------------------------------------------------------------------------------


/////////////////////////////////
// Required Includes
/////////////////////////////////
#include "camera/coolcam_maya.h"
#include "camera/camman_public.h"
#include "camera/coolcam_generic.h"
#include "camera/matrixtweakereditor.h"

#include "anim/animator.h"
#include "anim/hierarchy.h"
#include "core/exportstruct_anim.h"
#include "gfx/clump.h"
#include "gui/guimanager.h"
#include "game/entitymanager.h"
#include "game/entitybindings.h"
#include "game/luaattrtable.h"
#include "game/messagehandler.h"
#include "objectdatabase/dataobject.h"


//------------------------------------------------------------------------------------------
//!                                                                                         
//! CoolCam_Maya::CoolCam_MayaAnimator
//! Construct a cool animator for a maya cool cam
//! 
//------------------------------------------------------------------------------------------
CoolCam_MayaAnimator::CoolCam_MayaAnimator(const char* sClump, const char* sAnims)
{
	//ntError_p( sClump.GetLength() > 0, ("CoolCam_MayaAnimator: Clump string is invalid.") );
	ntError_p( !ntStr::IsNull(sClump), ("CoolCam_MayaAnimator: Clump string is invalid.") );
	//ntError_p( sAnims.GetLength() > 0, ("CoolCam_MayaAnimator: anim container name is invalid.") );
	ntError_p( !ntStr::IsNull(sAnims), ("CoolCam_MayaAnimator: anim container name is invalid.") );

	m_pClumpHeader = CClumpLoader::Get().LoadClump_Neutral(sClump,true);
	ntError_p(m_pClumpHeader != 0, ("Could not find clump %s", sClump));
	m_pHierarchy = CHierarchy::Create(m_pClumpHeader);
	ntError_p(m_pHierarchy != 0, ("Could not create hierarchy."));
	CHierarchy::GetWorld()->GetRootTransform()->AddChild(m_pHierarchy->GetRootTransform());
	m_pHierarchy->GetRootTransform()->SetLocalMatrix(CVecMath::GetIdentity());
	m_pAnimator = CAnimator::Create(static_cast< EntityAnimSet * >(this), m_pHierarchy, NULL, true);
	m_pAnimator->SetFlagBits(ANIMATORF_APPLY_DELTAS_EXPLICITY);
	EntityAnimSet::InstallAnimator(m_pAnimator, sAnims);
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//! CoolCam_Maya::CoolCam_MayaAnimator
//! Destruction
//!
//------------------------------------------------------------------------------------------
CoolCam_MayaAnimator::~CoolCam_MayaAnimator()
{
	if(m_pAnimator)
	{
		CAnimator::Destroy(m_pAnimator);
		m_pAnimator = 0;
	}
	
	if(m_pHierarchy)
	{
		if(m_pHierarchy->GetRootTransform()->GetParent())
			m_pHierarchy->GetRootTransform()->RemoveFromParent();

		CHierarchy::Destroy(m_pHierarchy);
		m_pHierarchy = 0;
	}

	if(m_pClumpHeader)
	{
		CClumpLoader::Get().UnloadClump(m_pClumpHeader);
		m_pClumpHeader = 0;
	}
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//! CoolCam_Maya::CoolCam_Maya
//! Construction, not much to see here
//! 
//------------------------------------------------------------------------------------------
/*CoolCam_Maya::CoolCam_Maya(const CamView& view)
 : CoolCamera(view),
   m_pCoolAnimator(0), 
   m_matrix(CONSTRUCT_CLEAR),
   m_pAttacker(0),
   m_pInformEntity(0)
{
	m_bFinished = true;
	m_bAutoFinish = true;
}*/


//------------------------------------------------------------------------------------------
//!                                                                                         
//! CoolCam_Maya::CoolCam_Maya
//! Construction, quite a bit to see here
//! 
//------------------------------------------------------------------------------------------
CoolCam_Maya::CoolCam_Maya(const CamView& view, CoolCam_MayaDef& def)
: CoolCamera(view),
  m_pCoolAnimator(def.pCoolAnimator), 
  m_pAnim(0),
  m_bAutoFinish( false ),
  m_ptTranslation( CONSTRUCT_CLEAR ),
  m_matrix(CONSTRUCT_CLEAR),
  m_pAttacker(0),
  m_pInformEntity(0),
  m_pTweaker(0)
{
	// Prepare the animation
	m_sAnim = def.sAnim;
	m_pCoolAnimator->GetAnimator()->RemoveAllAnimations();
	m_pAnim = m_pCoolAnimator->GetAnimator()->CreateAnimation(def.sAnim);
	one_time_assert_p(0x4201, m_pAnim, ("Could not find the animation '%s' for the maya camera.\n", ntStr::GetString(def.sAnim)));
	if(!m_pAnim)
	{
		return;
	}

	m_pAnim->SetFlags(ANIMF_INHIBIT_AUTO_DESTRUCT);
	m_pCoolAnimator->GetAnimator()->AddAnimation(m_pAnim);

	// Use the original FOV if the animation doesn't include one
	m_fFOV = CamMan_Public::Get().GetFOVAngle() * RAD_TO_DEG_VALUE;

	// Zero the transform
	m_matrix.SetIdentity();

	// Set the time
	if(def.fDuration > EPSILON)
	{
		m_pAnim->SetTimeRemaining(def.fDuration);
		ResetTime(m_pAnim->GetDuration());
	}
	else
	{
		ResetTime(m_pAnim->GetDuration());
	}

	m_pCoolAnimator->GetAnimator()->CreateBlends(0.f);
	m_pCoolAnimator->GetAnimator()->UpdateResults();
	SetTimeCurve(def.pCurve);

	m_iPriority = 1000;

	// Set up the tweaker
	if(def.pTweaker)
	{
		m_pTweaker = def.pTweaker;
	}

	// And we're done
	m_bFinished = false;
	m_bAutoFinish = false;
	return;
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//! CoolCam_Maya::CoolCam_Maya
//! Destruction, nope nothing here either
//!                                                                                         
//------------------------------------------------------------------------------------------
CoolCam_Maya::~CoolCam_Maya() 
{
	// if we have already finished, we have already removed our animations from the
	// animator. If we do this again unnessecarily we will break other cool cams that 
	// share the same animator as us. This happens in ninja sequences as the destructor
	// for cameras is deferred for a frame. 
	if(m_pCoolAnimator && !m_bFinished)
	{
		m_pCoolAnimator->GetAnimator()->RemoveAllAnimations();
	}
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//! CoolCam_Maya::Update
//! Just copy in the entities transform, need a copy for transitions
//! 
//------------------------------------------------------------------------------------------
void CoolCam_Maya::Update(float fTimeDelta)
{
	ntAssert(m_pCoolAnimator);

	if(!HasFinished())
	{
		m_pCoolAnimator->GetAnimator()->CreateBlends( fTimeDelta );
		m_pCoolAnimator->GetAnimator()->UpdateResults();
	}

	// The animation goes wrong for a frame before it finishes, I can't figure out why...
	if((!m_bFinished || !m_pAnim->IsActive()))// && m_pAnim->GetTime() < m_pAnim->GetDuration() - 0.1f)
	{
		CHierarchy* pobH = m_pCoolAnimator->GetHierarchy();
		ntAssert(pobH);

		Transform* pobTransform;
		Transform* pobParent;
		int			nIdx;         // Transform index

		// This timedelta needs to be scaled according to the attackers entities time scalar
		//ntPrintf("%s: %.4f * %.4f = %.4f\n", m_pAttacker->GetName().c_str(), fTimeDelta, m_pAttacker->GetTimeMultiplier(), fTimeDelta * m_pAttacker->GetTimeMultiplier());
		if(m_pAttacker)
		{
			fTimeDelta *= m_pAttacker->GetTimeMultiplier();
			// FIX: Hmmm...
			//m_pEntity->SetTimeMultiplier(m_pAttacker->GetTimeMultiplier());
		}

		pobTransform = pobH->GetRootTransform();
		nIdx = 0;

		pobParent = pobTransform->GetParent();
		ntAssert(pobParent);


		// Possible bug / Export Problem

		// Seems we need to face our camera the opposite way!
		// 
		CMatrix obTurn(CVecMath::GetYAxis(), PI);
		CMatrix obLocal = obTurn * pobTransform->GetLocalMatrix();

		m_obTransform = obLocal * pobParent->GetWorldMatrix();

		// Apply any specified matrix
		m_obTransform *= m_matrix;

		// The FOV is stored in the tracking data as the Y value
		CVector obTracking = m_pCoolAnimator->GetAnimator()->GetTrackingData(nIdx);
		float fFOV = obTracking.X();
		if(fFOV > 0.f)
		{
			//This calculation should really be done on animation export.
			m_fFOV = atanf(9.f/16.f*tanf(fFOV/2.f)) * 2.f * RAD_TO_DEG_VALUE;
		}

		// Set the DoF parameters
		if(obTracking.W() > 0.1f) // should be safe
		{
			m_bUseDoF = true; 
			m_fNearBlurDepth = obTracking.Y();
			m_fFocalDepth    = obTracking.Z();
			m_fFarBlurDepth  = obTracking.W();
		}
		else
		{
			m_bUseDoF = true; 
			m_fFocalDepth    = CDirection(m_obTransform.GetTranslation() ^ CEntityManager::Get().GetPlayer()->GetLocation()).Length();//GenericCoolCamProps::Get().fDOFMayaFocal;
			m_fNearBlurDepth = m_fFocalDepth - GenericCoolCamProps::Get().fDOFMayaNear;
			m_fFarBlurDepth  = m_fFocalDepth + GenericCoolCamProps::Get().fDOFMayaFar;
		}
		//ntPrintf("%s %.2f, %.2f, %.2f\n", m_bUseDoF ? "true":"false", m_fFocalDepth, m_fNearBlurDepth, m_fFarBlurDepth);

		
		// Update the time scalar
		UpdateTime(fTimeDelta);
		//int iGuide = int(CRenderer::Get().GetDefaultViewport()->GetHeight()) - 50;
		//CCamUtil::DebugPrintf(20, iGuide-36, "Cam Time %.2f:(%.2f) / %.2f", m_pAnim->GetTime(), m_fTime, m_pAnim->GetDuration());	

		// We don't need the look at point but it is used by transitions...  We need to fake one up...
		m_obLookAt = m_obTransform.GetTranslation() + m_obTransform.GetZAxis();

		if(m_pAnim->GetTime() >= m_pAnim->GetDuration() && m_bAutoFinish)
		{
			EndCamera();
		}
	}
	else
	{
		//ntPrintf("Cool cam Finished\n");
	}

	if(m_pTweaker)
	{
		m_obTransform = m_pTweaker->ApplyTweak(m_obTransform, fTimeDelta);
	}

	//ntPrintf("%.2f, %.2f, %.2f - > %.2f, %.2f, %.2f", m_obTransform.GetTranslation().X(), m_obTransform.GetTranslation().Y(), m_obTransform.GetTranslation().Z(),
	//											    m_obLookAt.X(), m_obLookAt.Y(), m_obLookAt.Z());
	
	//ntPrintf(" T(%.5f) D(%.5f) S(%.5f)", m_pAnim->GetTime(), m_pAnim->GetDuration(), m_pAnim->GetSpeed());
	
	//if(!m_pAnim->IsActive()) ntPrintf(" ANIMEND ");
	//if(m_pAnim->GetTime() >= m_pAnim->GetDuration())
	//	ntPrintf(" OVERTIME ");
	//if(m_bFinished)          ntPrintf("  DONE   ");
	//if(m_bEndNextFrame)      ntPrintf(" CLOSING ");
	//ntPrintf("\n");
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//! CoolCam_Maya::IsActive
//! Is the animation playing and is the tracking data set to on?
//! 
//------------------------------------------------------------------------------------------
bool CoolCam_Maya::IsActive() const
{
	if(!m_pCoolAnimator || m_bFinished)
		return false;

	return true;
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//! CoolCam_Maya::HasFinished
//! Has the animation we were attached to completed?
//! 
//------------------------------------------------------------------------------------------
bool CoolCam_Maya::HasFinished() const
{
	// Might need to do some transitions sometime...
	return m_bFinished;
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//! CoolCam_Maya::SetFinished
//! Mark the Maya camera as done.
//! 
//------------------------------------------------------------------------------------------
void CoolCam_Maya::SetFinished()
{
	m_bFinished = true;

	if(m_pCoolAnimator)
		m_pCoolAnimator->GetAnimator()->RemoveAllAnimations();

	if(m_pInformEntity)
		CMessageSender::SendEmptyMessage("msg_mayacam_finish", m_pInformEntity->GetMessageHandler());
}

//------------------------------------------------------------------------------------------
//! 
//! CoolCam_Maya::EndCamera
//! Terminate the cool camera.
//! 
//------------------------------------------------------------------------------------------
void CoolCam_Maya::EndCamera()
{
	SetFinished();
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//! CoolCam_Maya::SetAttacker
//! Set the attacking entity for this camera, the time multiplier is inherited from the 
//! attacker...
//! 
//------------------------------------------------------------------------------------------
void CoolCam_Maya::SetAttacker(const CEntity* pEnt) 
{
	m_pAttacker = pEnt; 
	//FIX: Hmmm
	//m_pEntity->SetTimeMultiplier(m_pAttacker->GetTimeMultiplier());
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CoolCam_Maya::SetMatrix                                                                 
//!	A matrix to apply to position the camera correctly.                                     
//!                                                                                         
//------------------------------------------------------------------------------------------
void CoolCam_Maya::SetMatrix(const CMatrix& mat)
{
	m_matrix = mat;
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CoolCam_Maya::SetTransform                                                              
//!	Parent the camera to a transform, camera will move with the transform.                  
//!                                                                                         
//------------------------------------------------------------------------------------------
void CoolCam_Maya::SetParentTransform(Transform& parentTransform)
{
	ntAssert(m_pCoolAnimator);
	parentTransform.AddChild(m_pCoolAnimator->GetHierarchy()->GetRootTransform());
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CoolCam_Maya::GetCameraName                                                             
//!	The maya cool cam returns some extra debugging information in it's name.                
//!                                                                                         
//------------------------------------------------------------------------------------------
CHashedString CoolCam_Maya::GetCameraName() const 
{
	static char buf[256]; 
	
	sprintf(buf, "*Maya Cam* %i(%.2f(%.2f)/%.2f)", ntStr::GetHashKey(m_sAnim), m_fTime, m_pAnim->GetTime(), m_fTotalTime); 
	return CHashedString(buf);
}
