/*
 * Copyright (c) 2003-2006 Naughty Dog, Inc. 
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#include "icerender.h"
#include "iceglobalcommandcontext.h"

using namespace Ice::Render;

// The current command context used by the inline helper functions.
CommandContextData *Ice::Render::g_currentCommandContext = nullptr;

#ifndef __SPU__
// Null fragment program location
U32 Ice::Render::g_nullFragmentProgramOffset;
#endif

// Reserve() is called whenever commands are about to be added to a
// command buffer. If it doesn't have enough space, it calls the current
// command context's callback function (if any) to give the application a
// chance to extend the context first.

bool CommandContext::ReserveFailed(U32 minimumSize)
{
	// Called from Reserve() when the current command buffer is full.
	if (m_callback) 
	{
		return (*m_callback)(static_cast<CommandContextData*>(this), minimumSize);
	}
	else 
	{
		ICE_ASSERT(!"ReserveFailed: No callback specified!");
		return false;
	}
}

bool Unsafe::CommandContext::ReserveFailed(U32 minimumSize)
{
	// Called from Reserve() when the current command buffer is full.
	if (m_callback) 
	{
		return (*m_callback)(static_cast<CommandContextData*>(this), minimumSize);
	}
	else 
	{
		ICE_ASSERT(!"ReserveFailed: No callback specified!");
		return false;
	}
}

bool Inline::CommandContext::ReserveFailed(U32 minimumSize)
{
	// Called from Reserve() when the current command buffer is full.
	if (m_callback) 
	{
		return (*m_callback)(static_cast<CommandContextData*>(this), minimumSize);
	}
	else 
	{
		ICE_ASSERT(!"ReserveFailed: No callback specified!");
		return false;
	}
}

bool InlineUnsafe::CommandContext::ReserveFailed(U32 minimumSize)
{
	// Called from Reserve() when the current command buffer is full.
	if (m_callback) 
	{
		return (*m_callback)(static_cast<CommandContextData*>(this), minimumSize);
	}
	else 
	{
		ICE_ASSERT(!"ReserveFailed: No callback specified!");
		return false;
	}
}

#ifndef __SPU__
#if ((ICEDEBUG==0) && (ICERENDER_FAST==1))
#define ICERENDER_ASM 1
#endif
#endif

namespace Ice
{
	namespace Render
	{
		#define ICERENDER_UNSAFE 0
		#define ICERENDER_INLINE 0
		#include "icecommandcontextcore.inl"
		#undef ICERENDER_INLINE
		#undef ICERENDER_UNSAFE

		namespace Unsafe
		{
			#define ICERENDER_UNSAFE 1
			#define ICERENDER_INLINE 0
			#include "icecommandcontextcore.inl"
			#undef ICERENDER_INLINE
			#undef ICERENDER_UNSAFE
		}
	}
}
