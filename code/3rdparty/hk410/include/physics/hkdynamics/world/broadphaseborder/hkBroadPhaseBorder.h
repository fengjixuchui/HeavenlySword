/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_UTILITIES2_BROAD_PHASE_BORDER_H
#define HK_UTILITIES2_BROAD_PHASE_BORDER_H

#include <hkmath/hkMath.h>
#include <hkdynamics/world/hkWorldCinfo.h>
#include <hkdynamics/phantom/hkPhantomOverlapListener.h>
#include <hkdynamics/world/listener/hkWorldDeletionListener.h>

class hkWorld;
class hkEntity;

/// This class monitors objects leaving the broadphase.
class hkBroadPhaseBorder : public hkReferencedObject, protected hkWorldDeletionListener, protected hkPhantomOverlapListener
{
	public:

		HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_UTILITIES);

			/// Creates an instance and attaches it to the world. It also adds a second reference to it that gets removed when the world gets deleted
			/// The positions of the border phantoms are derived from the broadphase extents of the hkWorld
		hkBroadPhaseBorder( hkWorld* world, hkWorldCinfo::BroadPhaseBorderBehaviour type  = hkWorldCinfo::BROADPHASE_BORDER_ASSERT );
			
		virtual ~hkBroadPhaseBorder();

			/// This function is called when an object goes beyond the scope of the broadphase.
			/// By default, it removes the entity from the world, you may implement your own version.
		virtual void maxPositionExceededCallback( hkEntity* body );

			/// Deactivate this class 
		virtual void deactivate();

		virtual void collidableAddedCallback( const hkCollidableAddedEvent& event	);

			/// Callback implementation 
		virtual void collidableRemovedCallback( const hkCollidableRemovedEvent& event );

	public:

		virtual void worldDeletedCallback( hkWorld* world );

		hkWorld* m_world;

		class hkPhantom* m_phantoms[6];

		hkWorldCinfo::BroadPhaseBorderBehaviour m_type;

};

#endif // HK_UTILITIES2_BROAD_PHASE_BORDER_H

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
