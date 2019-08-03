/*
 * Copyright (c) 2003-2005 Naughty Dog, Inc. 
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 * 
 * Revision History:
 *  - Created 7/28/05
 */

#ifndef ICELIGHTS_H
#define ICELIGHTS_H

#include "icerender.h"

namespace Ice
{
	namespace Bucketer
	{
		// A Point light which casts shadow volumes from m_position.
		struct ShadowVolumeCastingPointLight
		{
			// The location of the light in world space.
			struct 
			{
				F32 x,y,z;
			} m_position;
			// The push buffer contexts associated with this light.
			Ice::Render::CommandContext *m_profileContext;
			Ice::Render::CommandContext *m_frontCapContext;
			Ice::Render::CommandContext *m_backCapContext;
		};
		
		// A Directional light casts shadow volumes in the direction opposite to m_direction.
		struct ShadowVolumeCastingDirectionalLight
		{
			// The direction that the light will shine to.
			struct 
			{
				F32 x,y,z;
			} m_direction;
			// The push buffer contexts associated with this light.
			Ice::Render::CommandContext *m_profileContext;
			Ice::Render::CommandContext *m_frontCapContext;
		};
	}
}

#endif // ICELIGHTS_H
