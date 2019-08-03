//------------------------------------------------------------------------------------------
//!
//!	\file AnimationHeader.h
//!
//------------------------------------------------------------------------------------------

#ifndef	ANIM_ANIMATION_HEADER_H
#define	ANIM_ANIMATION_HEADER_H

#include "Fp/FpAnim/FpAnimClip.h"

class	GpAnimUserBindingDef;

typedef	FpAnimClipDef CAnimationHeader;

namespace AnimNames
{
	// Define the root node name and the built-in channel-type names.
	extern	FwHashedString	root;

	extern	FwHashedString	rotate;
	extern	FwHashedString	translate;
	extern	FwHashedString	scale;

	// Define the camera node name and our user-defined channel-type names.
	extern	FwHashedString	camera_root;

	extern	FwHashedString	tracking;
}

namespace AnimData
{
	extern GpAnimUserBindingDef *	UserBindingDef;

	void Create();
	void Destroy();
}

#endif // !ANIM_ANIMATION_HEADER_H



