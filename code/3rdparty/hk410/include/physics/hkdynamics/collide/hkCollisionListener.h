/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_DYNAMICS2_COLLISION_LISTENER_H
#define HK_DYNAMICS2_COLLISION_LISTENER_H

#include <hkmath/basetypes/hkContactPoint.h>
#include <hkdynamics/collide/hkDynamicsContactMgr.h>
#include <hkconstraintsolver/constraint/contact/hkContactPointProperties.h>
#include <hkcollide/agent/hkContactMgr.h>
#include <hkcollide/agent/hkProcessCollisionOutput.h>

class hkCollidable;
struct hkCollisionInput;
struct hkProcessCollisionOutput;
class hkEntity;
class hkDynamicsContactMgr;

/// Accept or reject contact points.
enum hkContactPointAccept
{
		/// Accept.
	HK_CONTACT_POINT_ACCEPT = 0,
		/// Reject.
	HK_CONTACT_POINT_REJECT = 1
};

struct hkToiPointAddedEvent;
struct hkManifoldPointAddedEvent;


	/// This collision event is passed to a hkCollisionListener's contactPointAddedCallback() just when
	/// a new contact point is created by the engine
struct hkContactPointAddedEvent
{
		/// This is the hkCdBody that was used (for entity A) to generate the contact point. If the entity's collision 
		/// detection representation is a single shape (for example one hkConvexVerticesShape) this will actually be
		/// a reference to the entity's hkCollidable. However if the entity has a more complicated shape hierarchy
		/// (such as a hkMeshShape) then this will be a stack allocated hkCdBody object, which may also reference a
		/// stack allocated hkShape. Do not store a pointer to this hkCdBody, or the shape it references for this
		/// reason, as the objects may be temporary. You can call getRootCollidable() on the hkCdBody, which will
		/// return a reference to the (permanent) heap created hkCollidable owned by the entity.
		/// The shape this points to is always the "leaf" shape, e.g. the actual triangle
		/// that created the contact point.
	const hkCdBody& m_bodyA;
	
		/// This is the hkCdBody that was used (for entity B) to generate the contact point. See the comments for m_bodyA
		/// for further details.
	const hkCdBody& m_bodyB;

	const enum Type { TYPE_TOI, TYPE_MANIFOLD } m_type;
	
		/// The entity this collision listener was added to
		/// (or HK_NULL if the collision listener was added to the world)
	hkEntity* m_callbackFiredFrom; 

		/// The contact point. This cannot be changed by the user - it needs to be done in process callback.
	const hkContactPoint* m_contactPoint;
	
		/// Optional information about the feature type of that contact.
		/// This provides detailed information about the type of contact, e.g.
		/// point-point, point-edge .., edge-edge or point-triangle.
		/// Only valid if the gsk algorithm is used. However if hkShapeCollection::m_disableWelding
		/// is set to false (default), this pointer might be HK_NULL.
	const hkGskCache* m_gskCache;
	
		/// The contact point properties, including friction and restitution. 
		/// This is changeable (and is the best place to change it).
	hkContactPointMaterial*  m_contactPointMaterial;

		/// The relative velocity of the objects at the point of contact projected onto the collision normal.<br>
		/// Note: this value is negative
		/// You can change its value, but it only has an effect for toi contact points
	hkReal m_projectedVelocity;        


		/// Whether the contact point will actually be added to the constraint solver or not. 
		/// By default this is HK_CONTACT_POINT_ACCEPT. If you set this to HK_CONTACT_POINT_REJECT,
		/// there will be no further action taken for this contact point, and no process or remove 
		/// events will be fired for the contact point.
	hkContactPointAccept m_status;

	inline hkBool isToi();

		/// if the underlying contact point is a toi contact point, this will give you a safe access
		/// to the additional properties
	inline hkToiPointAddedEvent& asToiEvent();

		/// if the underlying contact point is a normal collision point, this will give you a safe access
		/// to the additional properties
	inline hkManifoldPointAddedEvent& asManifoldEvent();

		//
		//	internal section
		//
		// The contact manager.
	hkDynamicsContactMgr& m_internalContactMgr;

	const struct hkProcessCollisionInput& m_collisionInput;

	struct hkProcessCollisionOutput& m_collisionOutput;
	

		// Creates a new hkContactPointAddedEvent.
	inline hkContactPointAddedEvent(	hkDynamicsContactMgr& contactMgr,	
										const hkProcessCollisionInput& m_collisionInput,
										hkProcessCollisionOutput& m_collisionOutput,

										const hkCdBody& a,    const hkCdBody& b, 
										hkContactPoint* cp,   const hkGskCache* gskCache, hkContactPointMaterial* cpp, 
										hkReal projectedVelocity, Type pointType );

};

	/// additional properties, in case hkContactPointAddedEvent is a toi event
struct hkToiPointAddedEvent: public hkContactPointAddedEvent
{
		/// The time of impact, you can modify this value
	hkTime m_toi;

		// Creates a new hkContactPointAddedEvent.
	inline hkToiPointAddedEvent(	hkDynamicsContactMgr& contactMgr,	
									const hkProcessCollisionInput& collisionInput,
									hkProcessCollisionOutput& collisionOutput,
									const hkCdBody& a,    const hkCdBody& b, 
									hkContactPoint* cp,   const hkGskCache* gskCache, hkContactPointMaterial* cpp, 
									hkTime toi, hkReal projectedVelocity );

};

	/// Additional properties if the added event was fired by the normal discrete collision detection
struct hkManifoldPointAddedEvent : public hkContactPointAddedEvent
{
		/// Identifies the contact point in the contact mgr
	const hkContactPointId m_contactPointId;

		/// This lets you specify the delay to the next process callback.
		/// This is by default initialized with 0
	mutable hkUint16	m_nextProcessCallbackDelay;

	inline hkManifoldPointAddedEvent(	hkContactPointId contactPointId, 
										hkDynamicsContactMgr& contactMgr,	
										const hkProcessCollisionInput& collisionInput,
										hkProcessCollisionOutput& collisionOutput,
										const hkCdBody& a,    const hkCdBody& b, 
										hkContactPoint* cp,   const hkGskCache* gskCache, hkContactPointMaterial* cpp, 
										hkReal projectedVelocity);

};

	/// This event is fired once for every new contact point exactly when
	/// its information is used for the very first time.
	/// You should use this:
	///   - to trigger high quality game events
	/// Warning: Due to the way this function is implemented, you cannot use it to
	/// apply forces or impulses at rigid bodies. The reason is that
	/// this callback is fired by the solver and at this stage the rigid
	/// body velocities are copied to internal solver data already.
struct hkContactPointConfirmedEvent
{
		/// The first entity's hkCollidable. Note that this is not the "leaf" hkCdBody (as is the case for the hkContactPointAddedEvent).
		/// This is because these events are fired after collision detection has been performed, and the hkCdBody objects needed
		/// to create the contact points may no longer exist.
	const hkCollidable& m_collidableA;
	
		/// The second entity's hkCollidable. Note that this is not the "leaf" hkCdBody (as is the case for the hkContactPointAddedEvent).
		/// This is because these events are fired after collision detection has been performed, and the hkCdBody objects needed
		/// to create the contact points may no longer exist.
	const hkCollidable& m_collidableB;

		/// The entity this collision listener was added to
		/// (or HK_NULL if the collision listener was added to the world)
	hkEntity* m_callbackFiredFrom; 

		/// returns true if the contact point is a toi contact point
	inline hkBool isToi() const;


		//
		// Changeable attributes:
		//

		/// The contact point. 
	hkContactPoint* m_contactPoint;
	
		/// The contact point properties, including friction and restitution.
	hkContactPointMaterial*  m_contactPointMaterial;

		/// Specifies how much the contact normal should be rotated in toi events. This is only relevant for Toi points.
		/// When set to non-zero value, this parameter reduces number of toi collisions, artificially adding separating velocity to colliding bodies.
		/// The higher the relative velocity of the bodies in the contact's tangent plane, the higher the extra impulse. 
		/// Note: This causes serious artifacts for collisions of e.g. a car riding along a wall and then crashing/sliding into the wall. Similarly,
		///       it may cause artifacts, when a car's chassis collides with landscape. It is advised to zero this parameter in a callback in such cases.
	hkReal m_rotateNormal;

		/// The relative velocity of the objects at the point of contact projected onto the collision normal.
		/// Changing this value influences the restitution. If this event is a toi event, you should
		/// only decrease its value, otherwise objects can sink in at the expense of tons of extra CPU
	const hkReal m_projectedVelocity;     

	const enum hkContactPointAddedEvent::Type m_type;


		/// Gets the contact point id of this point (or -1 for TOIs)
	hkContactPointId getContactPointId() const;

		/// Gets the DynamicsContactMgr: Warning: This function only works if you are not implementing your own
		/// hkDynamicsContactMgr
		// 
	const class hkDynamicsContactMgr* getContactMgr() const;

		// Internal section
		// 

		/// Pointer to the contact constraint data that triggered the callback. This can be used to access the contact manager and 
		/// extract the id of the contact point. This is only valid for in-manifold constact points, and is set to null for Toi contact points.
	const class hkSimpleContactConstraintData* m_contactData;



		// Creates a new hkContactPointAddedEvent.
	inline hkContactPointConfirmedEvent(	hkContactPointAddedEvent::Type type,
											const hkCollidable& a, const hkCollidable& b, 
											const hkSimpleContactConstraintData* data,
											hkContactPoint* cp,	hkContactPointMaterial* cpp, 
											hkReal rotateNormal,
											hkReal projectedVelocity);



};


	/// This collision event is passed to a hkCollisionListener's contactPointRemovedCallback() before a contact point is removed.
struct hkContactPointRemovedEvent
{
		/// Identifies the contact point. Is set to HK_INVALID_CONTACT_POINT if this contact point was a toi
	const hkContactPointId m_contactPointId;
			
		/// The material of the contact point. If this event is a normal event (not a toi event)
		/// you are allowed to upcast m_contactPointMaterial to hkContactPointProperties
	hkContactPointMaterial* m_contactPointMaterial;

		/// The first entity.
	hkEntity* m_entityA;

		/// The second entity.
	hkEntity* m_entityB;
	
		/// The entity this collision listener was added to
		/// (or HK_NULL if the collision listener was added to the world)
	hkEntity* m_callbackFiredFrom;

		/// returns true if the contact point is a toi contact point
	inline hkBool isToi();

		//
		//	Internal section	
		//

		// Creates a new hkContactPointRemovedEvent.
	inline hkContactPointRemovedEvent( 	hkContactPointId contactPointId, hkDynamicsContactMgr& contactMgr,	class hkConstraintOwner& constraintOwner, hkContactPointMaterial* mat, hkEntity* a,	hkEntity* b);

		// The contact manager.
	hkDynamicsContactMgr& m_internalContactMgr;

	hkConstraintOwner& m_constraintOwner;
};

	/// This collision event is passed to a hkCollisionListener's contactProcessCallback() just after
	/// all collision agents between a pair of bodies have been executed.
struct hkContactProcessEvent
{
	
		/// The first entity's hkCollidable. Note that this is not the "leaf" hkCdBody (as is the case for the hkContactPointAddedEvent).
		/// This is because these events are fired after collision detection has been performed, and the hkCdBody objects needed
		/// to create the contact points may no longer exist.
	const hkCollidable& m_collidableA;
	
		/// The second entity's hkCollidable. Note that this is not the "leaf" hkCdBody (as is the case for the hkContactPointAddedEvent).
		/// This is because these events are fired after collision detection has been performed, and the hkCdBody objects needed
		/// to create the contact points may no longer exist.
	const hkCollidable& m_collidableB;
	
		/// The entity this collision listener was added to
		/// (or HK_NULL if the collision listener was added to the world)
	hkEntity* m_callbackFiredFrom;

		/// All contact points including the toi contact point
	hkProcessCollisionData& m_collisionData;
    
		/// An array of additional properties, like friction, one pointer for each contact point in
		/// the m_collisionData.m_contactPoints array
	hkContactPointProperties* m_contactPointProperties[ HK_MAX_CONTACT_POINT ];


		//
		//	Internal section
		//

		// Access to the contact manager
	hkDynamicsContactMgr& m_internalContactMgr;

		/// Creates a new hkContactProcessEvent.
	inline hkContactProcessEvent( 	hkDynamicsContactMgr& contactMgr,	const hkCollidable& a,	const hkCollidable& b,	hkProcessCollisionData& cr);
};

	/// The base interface for listeners that respond to collision events.
	/// There are two types of contacts:
	///   - Normal contacts, which are persistent between two objects and are part of the contact manifold
	///   - Toi contacts, which are just created if two objects hit with high velocity and the interaction
	///     of both objects is flagged as continuous.
class hkCollisionListener
{
	public:
			///  Called after a contact point is created. This contact point is still a potential one.
			///  You can use this callback to:
			///	 - Override the friction and restitution values in the contact point's hkContactPointMaterial.
			///  - Reject the contact point.
			/// In this call you get access to the full information of the new contact point.
		virtual void contactPointAddedCallback(	hkContactPointAddedEvent& event) = 0;

			/// Called when a new contact point is used for the very first time.
			/// At this stage the contact point no longer is a potential one.
			/// So this callback can be used to trigger high quality game events.
			/// This callback is fired:
			///    - If it is a normal (manifold) contact: by the constraint solver
			///    - If it is a toi contact: by the toi handler in hkContinuousSimulation
		virtual void contactPointConfirmedCallback( hkContactPointConfirmedEvent& event) = 0;

			/// Called before a contact point gets removed.
		virtual void contactPointRemovedCallback( hkContactPointRemovedEvent& event ) = 0;

			/// Called just after all collision detection between 2 rigid bodies has been executed.
			/// NB: Callback frequency is related to hkEntity::m_processContactCallbackDelay. 
			/// This callback allows you to change any information in the result. 
			/// You can and should use this callback to:
			///    - do custom welding
			///    - trigger sound effects, especially scratching and rolling
		virtual void contactProcessCallback( hkContactProcessEvent& event ) {}

			/// Virtual destructor for derived objects
		virtual ~hkCollisionListener() {}
};

#include <hkdynamics/collide/hkCollisionListener.inl>


#endif // HK_DYNAMICS2_COLLISION_LISTENER_H


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
