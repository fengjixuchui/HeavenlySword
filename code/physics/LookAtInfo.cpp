//--------------------------------------------------
//!
//!	\file LookAtInfo.cpp
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#include "LookAtInfo.h"
#include "objectdatabase/dataobject.h"

static const CPoint				DEFAULT_OFFSET( 0, 0, 0 );
static const ntstd::String		DEFAULT_TRANSFORM("root");
static const u_int				DEFAULT_PRIORITY			= 3;

void ForceLinkFunctionLookAtInfo()
{
	ntPrintf("!ATTN! Calling ForceLinkFunctionLookAtInfo() !ATTN!\n");
}


START_STD_INTERFACE( LookAtInfo )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_obTransformName,		DEFAULT_TRANSFORM,		Transform )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_obOffset,			DEFAULT_OFFSET,			Offset )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_iPriority,			DEFAULT_PRIORITY,		Priority )
END_STD_INTERFACE


//eof

