/*
 * Copyright (c) 2003-2006 Naughty Dog, Inc. 
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#ifndef ICE_GLOBAL_COMMANDCONTEXT_H
#define ICE_GLOBAL_COMMANDCONTEXT_H

namespace Ice
{
	namespace Render
	{
		//! The current command context for the inlined helper functions.
		extern CommandContextData *g_currentCommandContext;
	
		// COMMAND BUFFER FUNCTIONS
		
		
		//! Sets the current command context.
		/*! \param context  A pointer to the command context object.
		*/
		static inline void BindCommandContext(CommandContextData *context)
		{
			g_currentCommandContext = context;
		}

		#define ICERENDER_TYPE Ice::Render::CommandContext*
		#include "iceglobalcommandcontextcore.inl"
		#undef ICERENDER_TYPE

		namespace Unsafe
		{
			#define ICERENDER_TYPE Ice::Render::Unsafe::CommandContext*
			#include "iceglobalcommandcontextcore.inl"
			#undef ICERENDER_TYPE
		}

		namespace Inline
		{
			#define ICERENDER_TYPE Ice::Render::Inline::CommandContext*
			#include "iceglobalcommandcontextcore.inl"
			#undef ICERENDER_TYPE
		}

		namespace InlineUnsafe
		{
			#define ICERENDER_TYPE Ice::Render::InlineUnsafe::CommandContext*
			#include "iceglobalcommandcontextcore.inl"
			#undef ICERENDER_TYPE
		}
	}
}

#endif
