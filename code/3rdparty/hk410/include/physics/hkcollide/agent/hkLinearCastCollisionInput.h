/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_LINEAR_CAST_COLLISION_INPUT_H
#define HK_COLLIDE2_LINEAR_CAST_COLLISION_INPUT_H

#include <hkcollide/agent/hkProcessCollisionInput.h>

	/// This structure is used for all linear cast queries.
struct hkLinearCastCollisionInput : public hkCollisionInput
{
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_AGENT, hkLinearCastCollisionInput);

		/// Constructor initialises m_maxExtraPenetration to HK_REAL_EPSILON
	inline hkLinearCastCollisionInput();

		/// Constructor copies everything it can from the hkProcessCollisionInput
	inline void set( const hkProcessCollisionInput& input );

		///	Sets the path, and the cached Path length.
	inline void setPathAndTolerance( const hkVector4& path, hkReal tolerance );

		/// The path from in world space for object A relative to object B.
		/// It is calculated from the destination position - the original position of object A
		/// Note: make sure you set the m_cachedPathLength to be the length of this vector.
	hkVector4	m_path;

		/// If the cast is parallel to a given surface,  it does not report a hit
		/// as long it does not move closer by more than m_maxExtraPenetration distance. 
	    /// Note: Every call the caster is allowed to get m_maxExtraPenetration closer, 
	    /// so the caller must prevent the objects of sinking in.
	hkReal m_maxExtraPenetration;

		/// This value is used for optimisation purposes. It is the length of the m_path vector.
	hkReal m_cachedPathLength;

		/// A pointer to a structure containing internal collision tolerances etc.
	hkCollisionAgentConfig* m_config;

};

#include <hkcollide/agent/hkLinearCastCollisionInput.inl>

#endif // HK_COLLIDE2_LINEAR_CAST_COLLISION_INPUT_H

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
