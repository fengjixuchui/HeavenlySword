/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HKVEHICLE_COLLISIONDETECTION_DEFAULT_hkVehicleRaycastWheelCollide_XML_H
#define HKVEHICLE_COLLISIONDETECTION_DEFAULT_hkVehicleRaycastWheelCollide_XML_H

#include <hkvehicle/hkVehicleInstance.h>
#include <hkvehicle/wheelcollide/hkVehicleWheelCollide.h>
#include <hkdynamics/phantom/hkPhantomOverlapListener.h>
#include <hkdynamics/world/hkWorld.h>

class hkAabbPhantom;
struct hkWorldRayCastOutput;
class hkAabb;

/// An hkPhantomOverlapListener used to ignore the chassis when doing collision detection.
/// (This inherits from base object so that it can be serialize without
/// the listener interface declared as reflected.)
class hkRejectRayChassisListener : public hkReferencedObject, public hkPhantomOverlapListener
{
	public:
	
		HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_VEHICLE);
		HK_DECLARE_REFLECTION();

			/// Default constructor
		hkRejectRayChassisListener();

		virtual ~hkRejectRayChassisListener();
		
		//
		// Methods
		//
		
			/// 
		void collidableAddedCallback(const hkCollidableAddedEvent& event);
		
			/// 
		void collidableRemovedCallback(const hkCollidableRemovedEvent& event);
		
		//
		// Members
		//
	public:
		
			/// 
		const hkCollidable* m_chassis; //+nosave

	public:
		hkRejectRayChassisListener(hkFinishLoadedObjectFlag f) { }
};


/// Default implementation of hkVehicleWheelCollide - performs a raycast
/// from the wheels to the ground.  This component cannot be shared between vehicles.
class hkVehicleRaycastWheelCollide : public hkVehicleWheelCollide
{
	public:

		HK_DECLARE_REFLECTION();

			/// Default constructor
		hkVehicleRaycastWheelCollide();

		~hkVehicleRaycastWheelCollide();
		
		//
		// Methods
		//
		
			///
		virtual void init( const hkVehicleInstance* vehicle );

			/// Calculates information about the effects of colliding the wheels with the ground, on the vehicle.
		virtual void collideWheels(const hkReal deltaTime, hkVehicleInstance* vehicle, CollisionDetectionWheelOutput* cdInfoOut);
		
			/// Passes back an AABB that encompasses the raycasts. It uses the hardpoints and
			/// suspension length to determine minimum and maximum extents.
		virtual void calcWheelsAABB( const hkVehicleInstance* vehicle, hkAabb& aabbOut);
		
			/// Use this method to override the default friction value set by the raycast
			/// vehicle updateBodies loop. The default value is the friction of the rigidbody
			/// that the wheel raycast hits Use the hkWorldRayCastOutput to obtain access to the
			/// shape hit by the raycast
		virtual void calcSingleWheelGroundFriction(hkVehicleInstance* vehicle, hkInt8 wheelInfoNum, const hkWorldRayCastOutput& worldRayCastOutput, hkReal& frictionOut) const;
		
			/// Perform raycast for a single wheel. This implementation performs a castRay call
			/// on the hkAabbPhantom.
		virtual void castSingleWheel(const hkVehicleInstance::WheelInfo& wheelInfo, hkVehicleInstance* vehicle, hkWorldRayCastOutput& output);
		
			/// 
		virtual void updatePhantom(hkVehicleInstance* vehicle);
		
			///
		virtual hkVehicleWheelCollide* clone( const hkArray<hkPhantom*>& newPhantoms ) const;

			///
		virtual void getPhantoms( hkArray<hkPhantom*>& phantomsOut );

		//
		// Members
		//
	public:
		
		/// Use to disable collision detection with the chassis.
		hkUint32 m_wheelCollisionFilterInfo;
		
		/// The phantom must be added to the world by the user.
		hkAabbPhantom* m_phantom;
		
		/// This hkPhantomOverlapListener is added to the phantom to ignore the chassis.
		class hkRejectRayChassisListener m_rejectRayChassisListener;

	public: 
		hkVehicleRaycastWheelCollide(hkFinishLoadedObjectFlag f) { }

};

#endif // HKVEHICLE_COLLISIONDETECTION_DEFAULT_hkVehicleRaycastWheelCollide_XML_H

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
