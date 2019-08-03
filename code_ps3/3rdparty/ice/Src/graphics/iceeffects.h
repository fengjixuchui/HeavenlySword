/*
 * Copyright (c) 2003-2005 Naughty Dog, Inc. 
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#ifndef ICE_EFFECTS_H
#define ICE_EFFECTS_H


namespace Ice
{
	// Forward declaration
	namespace Fx
	{
		struct Effect;
	}

	namespace Render
	{
		struct VertexProgram;
		struct FragmentProgram;
	}

	namespace Graphics
	{
		/// Loads an Fx::Effect from a file.
		Fx::Effect *LoadEffect(const char *fname);

		/// Loads a Render::VertexProgram from a file.
		Render::VertexProgram *LoadVertexProgram(const char *fname);

		/// Loads a Render::FragmentProgram from a file.
		Render::FragmentProgram *LoadFragmentProgram(const char *fname);
	}
}


#endif
