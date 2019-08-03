/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef INC_HK_LOOK_AT_IK_SOLVER_H
#define INC_HK_LOOK_AT_IK_SOLVER_H

/// This IK solver operates on a single bone (specified as a transform in model space). It rotates it so the forward axis (m_fwdLS) points towards the
/// specified target (targetMS). The movement is clamped around a cone specified by m_limitAxisMS and m_limitAngle. Finally, the rotation applied by the IK
/// can be weighted by a gain value (gain), in the range [0..1]. The solver returns false if the target is over limits.
class hkLookAtIkSolver : public hkReferencedObject
{
	public:

		HK_DECLARE_CLASS_ALLOCATOR( HK_MEMORY_CLASS_ANIM_RUNTIME );

		/// Setup information passed to hkLookAtIkSolver::solve()
		struct Setup
		{
				/// Forward vector in the local space of the bone
			hkVector4 m_fwdLS;

				/// Axis of the limiting cone, specified in model space
			hkVector4 m_limitAxisMS;

				/// Angle of the limiting cone
			hkReal m_limitAngle;
		};

			/// Solves the specified lookAt constraint. Check the documentation for hkLookAtIkSolver for details.
		static hkBool HK_CALL solve ( const Setup& setup, const hkVector4& targetMS, hkReal gain, hkQsTransform& boneModelSpaceInOut );

};


#endif // INC_HK_LOOK_AT_IK_SOLVER_H

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
