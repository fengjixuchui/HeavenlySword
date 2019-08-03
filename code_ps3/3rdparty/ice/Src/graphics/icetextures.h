/*
 * Copyright (c) 2003-2005 Naughty Dog, Inc. 
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#ifndef ICE_TEXTURES_H
#define ICE_TEXTURES_H


#include "icerender.h"

namespace Ice
{
	namespace Loader
	{
		struct TextureLoaderInfo;
	}

	namespace Graphics
	{
		Render::Texture *LoadTextureFromFile(const char *filename);
	}
}


#endif
