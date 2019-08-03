/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_DYNAMICS2_FAKE_RIGIDBODY_DEACTIVATOR_H
#define HK_DYNAMICS2_FAKE_RIGIDBODY_DEACTIVATOR_H

#include <hkdynamics/entity/hkRigidBodyDeactivator.h>

extern const hkClass hkFakeRigidBodyDeactivatorClass;

	/// This class is deprecated.
	/// Class for all objects that are never deactivated.
	/// Note: Do not add any actual havok mem manged data
	/// to this class or super classes, as it is a static
	/// global instance.
	/// It has a specific class for serialization so that it
	/// can be identified (and thus not streamed etc.)
class hkFakeRigidBodyDeactivator : public hkRigidBodyDeactivator
{
	public:

		HK_DECLARE_REFLECTION();

		inline hkFakeRigidBodyDeactivator();

			/// Returns the deactivator type.
		virtual enum DeactivatorType getRigidBodyDeactivatorType() const;

			/// High frequency deactivation is used to quickly deactivate entities that have come to a definite halt.
		virtual hkBool shouldDeactivateHighFrequency( const hkEntity* entity );

			/// Low frequency deactivation checks less frequently if the object needs to be deactivated.
			/// This makes it less likely that the system will deactivate an object that might quickly need to move again.
		virtual hkBool shouldDeactivateLowFrequency( const hkEntity* entity );

			/// Returns the world hkFakeRigBodyDeactivator that is used by all entities that wish never to deactivate.
		inline static hkFakeRigidBodyDeactivator* getFakeRigidBodyDeactivator();

	protected:

		static hkFakeRigidBodyDeactivator m_fakeRigidBodyDeactivator;

	public:

		hkFakeRigidBodyDeactivator( class hkFinishLoadedObjectFlag flag ) {}

};

#include <hkdynamics/entity/hkFakeRigidBodyDeactivator.inl>

#endif // HK_DYNAMICS2_FAKE_RIGIDBODY_DEACTIVATOR_H

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
