//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file camerainterface.cpp                                                               
//!                                                                                         
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
// Required Includes                                                                        
//------------------------------------------------------------------------------------------
#include "camera/camerainterface.h"
#include "camera/camview.h"

#include "physics/world.h"


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CameraInterface::CameraInterface                                                      
//!	                                                                                        
//!                                                                                         
//------------------------------------------------------------------------------------------
CameraInterface::CameraInterface(const CamView& view) : 
	m_obLookAt(CONSTRUCT_CLEAR),
	m_obTransform(CONSTRUCT_IDENTITY),
	m_fFOV(35.0f),
	m_bUseDoF(false),
	m_fFocalDepth(1.0f),
	m_fNearBlurDepth(0.0f),
	m_fFarBlurDepth(100.0f),
	m_fConfusionHigh(5.0f),
	m_fConfusionLow(2.5f),
	m_bUseMotionBlur(false),
	m_fMotionBlur(0.f),
	m_view(view)
{}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CameraInterface::GetElementManager                                                      
//!	Get the element manager associated with this cameras view                               
//!                                                                                         
//------------------------------------------------------------------------------------------
const CamSceneElementMan& CameraInterface::GetElementManager() const
{
	return m_view.GetElementManager();
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CameraInterface::CheckTransformValid                                                             
//!	Check that the camera transform is valid.                                                                  
//!                                                                                         
//------------------------------------------------------------------------------------------
bool CameraInterface::CheckTransformValid() const
{
#ifdef PLATFORM_PS3
	return true;
#else
	if(_finite(m_obTransform[0][0]) && _finite(m_obTransform[0][1]) && _finite(m_obTransform[0][2]) && _finite(m_obTransform[0][3]) &&
	   _finite(m_obTransform[1][0]) && _finite(m_obTransform[1][1]) && _finite(m_obTransform[1][2]) && _finite(m_obTransform[1][3]) &&
	   _finite(m_obTransform[2][0]) && _finite(m_obTransform[2][1]) && _finite(m_obTransform[2][2]) && _finite(m_obTransform[2][3]) &&
	   _finite(m_obTransform[3][0]) && _finite(m_obTransform[3][1]) && _finite(m_obTransform[3][2]) && _finite(m_obTransform[3][3]))
	   return true;

	return false;
#endif
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CameraInterface::IsInView                                                             
//!	Check if a give point is in view for this camera.                                                                  
//!                                                                                         
//------------------------------------------------------------------------------------------
bool CameraInterface::IsInView(const CPoint& pt, bool bObscureCheck)
{
	CMatrix mat = GetTransform();
	CPoint ptTranslated = pt - mat.GetTranslation();

	if(ptTranslated.Dot(CPoint(mat.GetZAxis())) <= 0.0f)
	{
		// Behind the camera...
		return false;
	}

	// Transform the pt into projection space
	CMatrix matProj;
	matProj.Perspective(m_fFOV, GetView().GetAspectRatio(), GetView().GetZNear(), GetView().GetZFar());

	// Get Cam Space
	CVector vec(pt * mat.GetAffineInverse());

	// Get Projection Space
	vec.W() = 0.f;
	vec = vec * matProj;

	// Deal with points at infinity
	if(vec.W() < EPSILON)
	{
		vec = CVector(0.f,0.f,-1.f,1.f);
	}
	else
	{
		vec /= fabsf(vec.W());
	}

	// Now Check Projection Bounds
	if(vec.X() < -1.f || vec.X() > 1.f || vec.Y() < -1.f || vec.Y() > 1.f)
		return false;

	// It's in the frustum
	if(bObscureCheck)
	{
		// Check for Obscuring Geometry...
		Physics::RaycastCollisionFlag obFlag; obFlag.base = 0;

		obFlag.flags.i_am = Physics::LINE_SIGHT_BIT;
		obFlag.flags.i_collide_with = (Physics::CHARACTER_CONTROLLER_PLAYER_BIT | 
									   Physics::CHARACTER_CONTROLLER_ENEMY_BIT	|
									   Physics::SMALL_INTERACTABLE_BIT			|
									   Physics::LARGE_INTERACTABLE_BIT			|
									   Physics::RAGDOLL_BIT						);

		const CEntity *pobEntity = Physics::CPhysicsWorld::Get().CastRay(GetTransform().GetTranslation(), pt, obFlag);
		if(pobEntity)
		{
			//We're obscured...
			return false;
		}
	}
	
	return true;
}
