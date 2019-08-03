/*
 * Copyright (c) 2003-2005 Naughty Dog, Inc.
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#ifndef ICE_CAMERAS_H
#define ICE_CAMERAS_H


#include "icerender.h"
#include "shared/math/transform.h"

namespace Ice
{
	namespace Graphics
	{
		union F32_I32 {
			F32 f32;
			I32 i32;

			F32_I32() {}
			F32_I32(F32 f32_value) : f32(f32_value) {}
			F32_I32(I32 i32_value) : i32(i32_value) {}
		};
		const I32 POSITIVE_INFINITY_INT = 0x7F800000;
		const float POSITIVE_INFINITY = F32_I32( POSITIVE_INFINITY_INT ).f32;

		// A camera that renders to the primary render target
		struct RenderCamera
		{
			SMath::Transform        m_transform;    ///< World transform of camera.
			float                   m_aspectRatio;  ///< Ratio of viewport height and width.
			float                   m_focalLength;  ///< Focal length - used to compute projection matrix.
			float                   m_nearDepth;    ///< Near plane distance.
			float                   m_farDepth;     ///< Far plane distance. If +INF, will use an infinite far plane.
		};

		// A camera that renders to a secondary render target
		struct RenderToTextureCamera
		{
			SMath::Transform        m_transform;    ///< World transform of camera.
			float                   m_aspectRatio;  ///< Ratio of viewport height and width.
			float                   m_focalLength;  ///< Focal length - used to compute projection matrix.
			float                   m_nearDepth;    ///< Near plane distance.
			float                   m_farDepth;     ///< Far plane distance. If +INF, will use an infinite far plane.
			U32                     m_left;         ///< Left pixel column of viewport.
			U32                     m_top;          ///< Top pixel row of viewport.
			U32                     m_width;        ///< Width of viewport in pixels.
			U32                     m_height;       ///< Height of viewport in pixels.
			Render::RenderTarget    m_renderTarget; ///< The render target to which the camera renders.
		};
	}
}


#endif
