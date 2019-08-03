//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file camman_public.cpp                                                               
//!                                                                                         
//------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
// Required Includes                                                                        
//------------------------------------------------------------------------------------------
#include "camera/camman_public.h"
#include "camera/camman.h"
#include "camera/camview.h"


//------------------------------------------------------------------------------------------
//!
//!	CamMan_Public::Get
//!	Get the real CamMan Singleton
//!
//------------------------------------------------------------------------------------------
CCamera& CamMan_Public::Get()
{
	ntAssert(CamMan::GetPrimaryView());
	return *CamMan::GetPrimaryView();
}


//------------------------------------------------------------------------------------------
//!
//!	CamMan_Public::GetP
//!	Get a pointer to the real CamMan Singleton
//!
//------------------------------------------------------------------------------------------
CCamera* CamMan_Public::GetP()
{
	return static_cast<CCamera*>( CamMan::GetPrimaryView() );
}


//------------------------------------------------------------------------------------------
//!
//!	CamMan_Public::GetCurrMatrix
//!	Get the primary viewing matrix
//!
//------------------------------------------------------------------------------------------
CMatrix	CamMan_Public::GetCurrMatrix()
{
	return CamMan::GetPrimaryView()->GetCurrMatrix();
}


//------------------------------------------------------------------------------------------
//!
//!	CamMan_Public::GetTransform
//!	Get the primary viewing transform
//!
//------------------------------------------------------------------------------------------
Transform* CamMan_Public::GetTransform()
{
	return const_cast<Transform*>(&CamMan::GetPrimaryView()->GetTransform());
}


//------------------------------------------------------------------------------------------
//!
//!	CamMan_Public::GetTransform
//!	Get the primary viewing transform
//!
//------------------------------------------------------------------------------------------
const CamView* CamMan_Public::GetPrimaryView()
{
	return CamMan::GetPrimaryView();
}


//------------------------------------------------------------------------------------------
//!
//!	CamMan_Public::GetDebugCameraPadNumber
//!	Get the debug camera pad id.s
//!
//------------------------------------------------------------------------------------------
PAD_NUMBER CamMan_Public::GetDebugCameraPadNumber()
{
	return CamMan::GetDebugCameraPadNumber();
}


