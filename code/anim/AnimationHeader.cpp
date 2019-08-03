//------------------------------------------------------------------------------------------
//!
//!	\file AnimationHeader.h
//!
//------------------------------------------------------------------------------------------

#include "anim/AnimationHeader.h"

#include "Gp/GpAnimator/GpAnimator.h"

namespace AnimNames
{
	// Define the root node name and the built-in channel-type names.
	FwHashedString	root		( "root" );

	FwHashedString	rotate		( "rotate" );
	FwHashedString	translate	( "translate" );
	FwHashedString	scale		( "scale" );

	// Define the camera node name and our user-defined channel-type names.
	FwHashedString	camera_root	( "clump_thecamera" );		// Need to work this out properly.

	FwHashedString	tracking( "depthoffield" );				// The tracking channel for cameras is called "depthoffield"...

	// Add more user-channel type names here and remember to add them to
	// "InternalUserChannelNamePairs" below and also to increment "NumUserChannelPairs"
	// in the the header-file. Remember to extern everything you define in the header.
}

namespace AnimData
{
	static const uint32_t	NumUserChannelPairs = 1;

	GpAnimNamePair			InternalUserChannelNamePairs[ NumUserChannelPairs ] =
	{
		GpAnimNamePair( AnimNames::camera_root, AnimNames::tracking ),
	};

	GpAnimUserBindingDef *	UserBindingDef = NULL;
	
	void Create()
	{
		ntError_p( UserBindingDef == NULL, ("You should only call AnimData::Create once!") );
		UserBindingDef = GpAnimUserBindingDef::Create( &( InternalUserChannelNamePairs[ 0 ] ), NumUserChannelPairs );
	}

	void Destroy()
	{
		ntError( UserBindingDef != NULL );
		GpAnimUserBindingDef::Destroy( UserBindingDef );
		UserBindingDef = NULL;
	}
}
