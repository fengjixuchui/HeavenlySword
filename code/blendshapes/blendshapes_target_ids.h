//--------------------------------------------------
//!
//!	\file blendshapes_target_ids.h
//!	Target IDs and common names
//!
//--------------------------------------------------

#ifndef _BLENDSHAPES_TARGET_IDS_H_
#define _BLENDSHAPES_TARGET_IDS_H_

#include "blendshapes_constants.h"

// kinda ugly but enough for testing purposes I guess...

enum BSTargetID
{
	kBST_Invalid						= -1,			


	// ---- Common IDs start Here ---- //
	kBST_Common							= 0,

	
	// ---- In Game ---- //				// usually refer to whole expressions instead of just a visime
	kBST_InGame							= kBST_Common,

	kBST_Idle							= kBST_InGame,
	kBST_Angry,
	kBST_Surprised,
	kBST_InPain,
	kBST_Smiling,

	kBST_LastInGameTarget,
	kBST_NumOfInGameTargets				= kBST_LastInGameTarget - kBST_InGame,



	// ---- CutScene ---- //			// now we have localised deformations
	kBST_CutScene,

	kBST_MouthNarrow					= kBST_CutScene,		
	KBST_MouthWide,
	kBST_LeftMouthSmile,
	kBST_RightMouthSmile,

	kBST_LeftEyeWink,
	kBST_RightEyeWink,
	kBST_LeftBrowRaise,
	kBST_RightBrowRaise,

	kBST_LastCutSceneTarget,
	kBST_NumOfCutSceneTargets			= kBST_LastCutSceneTarget - kBST_CutScene,

	// ---- End of Common IDs ---- //
	kBST_NumOfCommonTargets				= kBST_LastCutSceneTarget,
	
	// ---- Custom IDs ---- //
	kBST_Custom							= kBST_NumOfCommonTargets,



	// ---- End of All IDs --- //
	kBST_TotalNumOfTargets				= blendshapes::MAX_TARGETS,						
};


//---------------------------------------------------------------------
//							COMMON NAMES
//---------------------------------------------------------------------
//static const char* CommonBlendShapeNames[kBST_NumOfCommonTargets] =
//{
//	//---- InGame ----//
//	"bs_idle",
//	"bs_anger",
//	"bs_surprise",
//	"bs_pain",
//	"bs_smile",
//
//	
//	//---- CutScene----//
//	"bs_lips_narrow",
//	"bs_lips_wide",
//	"bs_left_lips_smile",
//	"bs_right_lips_smile",
//
//	"bs_left_eyelid_close",
//	"bs_right_eyelid_close",
//	"bs_left_brow_rise",
//	"bs_right_brow_rise",
//};

#endif // end of _BLENDSHAPES_TARGET_IDS_H_
