/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef INC_HKCONSTRAINEDSYSTEML_FILTER
#define INC_HKCONSTRAINEDSYSTEML_FILTER

#include <hkcollide/filter/hkCollisionFilter.h>
#include <hkdynamics/constraint/hkConstraintListener.h>

	/// This filter disables collisions between two rigid bodies if they are connected by a constraint (other than a contact constraint);
	/// otherwise, it forwards to another (optional) filter.
	///
	/// This filter is also a world constraint listener, and it removes agents immediately
	/// upon addition of constraints. Therefore it is not necessary to call 
	/// updateCollisionFilter explicitly. You will need to call updateCollisionFilter explicitly
	/// when removing a constraint and expecting collision detection to be reenabled between linked bodies.
class hkConstrainedSystemFilter : public hkCollisionFilter, public hkConstraintListener
{
	public:

		HK_DECLARE_REFLECTION();

			/// Constructor
		hkConstrainedSystemFilter (const hkCollisionFilter* otherFilter = HK_NULL);

			// Destructor
		~hkConstrainedSystemFilter();

			// Checks two collidables
		virtual hkBool isCollisionEnabled( const hkCollidable& a, const hkCollidable& b ) const;

		virtual	hkBool isCollisionEnabled( const hkCollisionInput& input, const hkCdBody& a, const hkCdBody& b, const hkShapeContainer& bContainer, hkShapeKey bKey  ) const;

		virtual hkBool isCollisionEnabled( const hkShapeRayCastInput& aInput, const hkShape& shape, const hkShapeContainer& bContainer, hkShapeKey bKey ) const;

		virtual hkBool isCollisionEnabled( const hkWorldRayCastInput& a, const hkCollidable& collidableB ) const;


		//
		// Implementation of the hkConstraintListener interface
		//

			// Called when a constraint is added to the world.
		virtual void constraintAddedCallback( hkConstraintInstance* constraint );

			// Called when a constraint is removed from the world.
		virtual void constraintRemovedCallback( hkConstraintInstance* constraint );


	public:

		hkConstrainedSystemFilter(hkFinishLoadedObjectFlag f) {}

	private:

		const hkCollisionFilter* m_otherFilter;
};


#endif //INC_HKCONSTRAINEDSYSTEML_FILTER

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
