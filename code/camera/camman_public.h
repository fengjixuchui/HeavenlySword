//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file camman_public.h
//!                                                                                         
//------------------------------------------------------------------------------------------

// This file indirects common functions from the CameraManager, please include this file
// rather than camman.h if you just want to use the below functions as it prevents the
// rebuilding of lots of files when other functions in camman.h are modified.  Thanks.

#ifndef CAMMAN_PUBLIC_INC
#define CAMMAN_PUBLIC_INC

#include "gfx/camera.h"
#include "input/inputhardware.h"

class CCamSceneElementMan;
class Transform;
class CamView;

class CamMan_Public
{
public:
	static CCamera& Get();                      // Returns the primary view CCamera
	static CCamera* GetP();                     // Returns a pointer to primary view CCamera
	static CMatrix GetCurrMatrix();				// Returns the primary view matrix
	static Transform* GetTransform();			// Returns the primary view transform
	static const CamView* GetPrimaryView();		// Returns the primary view without casting.

	static PAD_NUMBER GetDebugCameraPadNumber();

	// Element Manager, will be removed soon
	static CCamSceneElementMan& GetElementManager();
};

#endif//CAMMAN_PUBLIC_INC
