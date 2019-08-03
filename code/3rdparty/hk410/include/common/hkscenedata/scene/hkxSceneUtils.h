/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HKX_SCENE_UTILS_H
#define HKX_SCENE_UTILS_H

#include <hkmath/linear/hkMathUtil.h>

/// Scene utilities
class hkxSceneUtils
{
	public:

			/// Options taken by "hkxSceneUtils::transformScene"
		struct SceneTransformOptions
		{
				/// Should we apply this transformation to scene graph nodes
			hkBool m_applyToNodes;

				/// Should we apply this transformation to vertex buffers
			hkBool m_applyToBuffers;

				/// Should we apply this transformation to lights
			hkBool m_applyToLights;

				/// Should we apply this transformation to cameras
			hkBool m_applyToCameras;

				/// Should we flip index buffer winding
			hkBool m_flipWinding;

				/// The transform to apply
			hkMatrix3 m_transformMatrix;
		};

			/// Given a scene and the options specified in the SceneTransformOption struct, it goes
			/// through nodes, attributes, meshes, etc.. applying the specified transform to the scene
			/// Useful for scaling scenes as well as for transformin coordinate systems
		static void HK_CALL transformScene( class hkxScene& scene, const SceneTransformOptions& opts );

			/// Extracts environment data from an hkxScene - used mostly for backwards compatibility as previously
			/// environment information was stored in the hkxScene object. The variables extracted are:
			/// "asset" (ex: "car"),
			/// "assetPath" (ex: "c:/temp/car.max"),
			/// "assetFolder" (ex: "c:/temp/"),
			/// "modeller" (ex: "3ds max 8.0.0"),
			/// "selected" (ex: "chassis")
		static void HK_CALL fillEnvironmentFromScene (const class hkxScene& scene, class hkxEnvironment& environment);

	private:

			// Contains useful information about the transform
		struct TransformInfo
		{
				// The transform as a Matrix4
			hkMatrix3 m_transform;

				// The inverse of the transform
			hkMatrix3 m_inverse;

				// The inverse of the transform, transposed
			hkMatrix3 m_inverseTranspose;

				// The transform decomposed
			hkMathUtil::Decomposition m_decomposition;
		};


			// Transforms a node and its children. It also transform node attributes
		static void transformNode( const TransformInfo& transformInfo, class hkxNode& node);

		static void transformSkinBinding( const TransformInfo& transformInfo, class hkxSkinBinding& skinBinding);

		static void transformVertexBuffer( const TransformInfo& transformInfo, class hkxVertexBuffer& vbuffer);

		static void transformLight( const TransformInfo& transformInfo, class hkxLight& light);

		static void transformCamera( const TransformInfo& transformInfo, class hkxCamera& camera);

		static void flipWinding( class hkxIndexBuffer &ibuffer );

			// Called by transformNode
		static void transformAttributeGroup ( const TransformInfo& transformInfo, struct hkxAttributeGroup& attributeGroup);

			// Called by transformAttributeGroup
		static void transformAnimatedFloat (const TransformInfo& transformInfo, struct hkxAnimatedFloat& animatedFloat);
		static void transformAnimatedQuaternion (const TransformInfo& transformInfo, struct hkxAnimatedQuaternion& animatedQuaternion);
		static void transformAnimatedMatrix (const TransformInfo& transformInfo, struct hkxAnimatedMatrix& animatedMatrix);
		static void transformAnimatedVector (const TransformInfo& transformInfo, struct hkxAnimatedVector& animatedVector);

			// Transforms a fullMatrix4, reused in different places
		static void transformMatrix4 (const TransformInfo& transformInfo, hkMatrix4& matrix4);
};

#endif // HK_SCENE_UTILS_H

/*
* Havok SDK - PUBLIC RELEASE, BUILD(#20060902)
*
* Confidential Information of Havok.  (C) Copyright 1999-2006 
* Telekinesys Research Limited t/a Havok. All Rights Reserved. The Havok
* Logo, and the Havok buzzsaw logo are trademarks of Havok.  Title, ownership
* rights, and intellectual property rights in the Havok software remain in
* Havok and/or its suppliers.
*
* Use of this software for evaluation purposes is subject to and indicates 
* acceptance of the End User licence Agreement for this product. A copy of 
* the license is included with this software and is also available from salesteam@havok.com.
*
*/
