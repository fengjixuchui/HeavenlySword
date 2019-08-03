/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_UTILITIES2_PHYSICS_CONTEXT_H
#define HK_UTILITIES2_PHYSICS_CONTEXT_H

#include <hkvisualize/hkProcessContext.h>
#include <hkdynamics/world/listener/hkWorldDeletionListener.h>
#include <hkdynamics/action/hkActionListener.h>
#include <hkdynamics/constraint/hkConstraintListener.h>
#include <hkdynamics/phantom/hkPhantomListener.h>
#include <hkdynamics/entity/hkEntityListener.h>
class hkWorld;
class hkEntity;
class hkPhantom;

/// A simple interface that viewers that want to know
/// when hkWorlds are added and removed from the physics
/// context can impliment.
class hkPhysicsContextWorldListener
{
    public:

        virtual ~hkPhysicsContextWorldListener() { }
        virtual void worldAddedCallback( hkWorld* newWorld ) = 0;
        virtual void worldRemovedCallback( hkWorld* newWorld ) = 0;
};

#define HK_PHYSICS_CONTEXT_TYPE_STRING "Physics"

/// This is the context that processes (here called the older name
/// of 'viewers') can use if they understand physics Worlds. It listens on all 
/// added worlds and can trigger deletion and addition callbacks 
/// to processes (viewers) if requested. A context itself is just a data store
/// for any information that a viewer will need. In this case it is pointers to 
/// hkWorlds, as from that any of the physics viewers can find the parts they are 
/// interested in and then register themselves with the world to get the appropriate 
/// callbacks to keep their state up to date.
class hkPhysicsContext : 
	public hkReferencedObject, public hkProcessContext, 
	public hkWorldDeletionListener, 
	public hkEntityListener, 
	public hkPhantomListener,
	public hkConstraintListener,
	public hkActionListener
{
	public:

		HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_VDB);

		hkPhysicsContext();

			/// Register all Processes (Physics Viewers) 
			/// in the Utilities. If you don't call this
			/// or a subset of what it calls, all you will get
			/// is the common viewers in hkVisualize (debug display and stats).
			/// If you don't see the hkShapeDisplayViewer for instance in the 
			/// list of available viewers in the VDB client, it is beacuse 
			/// this has not been called.
		static void HK_CALL registerAllPhysicsProcesses();

			/// As there can be any number of different user types
			/// of data contexts, the type is simply identified by string.
		virtual const char* getType() { return HK_PHYSICS_CONTEXT_TYPE_STRING; }

			/// Register a world. This context is a deletion listener, so will 
			/// remove the world when it sees it deleted. It does not reference
			/// the world, just keeps track of it by way of the listeners.
		void addWorld( hkWorld* newWorld );

			/// Explicitly remove a world from the context. If 
			/// you delete a world it will remove itself from
			/// the context anyway as this context is a deletion
			/// listener.
		void removeWorld( hkWorld* newWorld ); 

			/// Find the index of the given world, returns -1
			/// if not found in this context.
		int findWorld(hkWorld* world); 

			/// Get number of worlds registered in this context.
		inline int getNumWorlds() { return m_worlds.getSize(); }

			/// Get the i-th world registered in this context.
		inline hkWorld* getWorld(int i) { return m_worlds[i]; }

			/// So that processes can see all the worlds dynamically, 
			/// they can be hkPhysicsContextWorldListener which simply
			/// get world added to this context events. As such they would
			/// be able to then register themselves as rigidbody added
			/// listeners and so on and for instance be able to create 
			/// bodies to display upon those further callbacks.
		void addWorldAddedListener( hkPhysicsContextWorldListener* cb );

			/// So that processes can see all the worlds dynamically, 
			/// they can be hkPhysicsContextWorldListener which simply
			/// can get world removed from this context, and can update
			/// their state accordingly (remove some display geoms etc).
		void removeWorldAddedListener( hkPhysicsContextWorldListener* cb );

	public:
		
			/// Set the VDB that owns this context. This is callled by the VDB iteslf.
		virtual void setOwner(hkVisualDebugger* vdb);

			// Physics callbacks we want to track

			/// Raised when the given world has been deleted, default action is 
			/// to remove it from the list of registered worlds and notify 
			/// any interested viewers that the world has been removed.
		virtual void worldDeletedCallback( hkWorld* world );

			/// Inspection (tweaking on the client side affecting this server side)
			/// requires that at least high level objects get removed from the 
			/// list of available tweakable objects. Tweakables are registered here
			/// from the physics contexts worlds through this context owner, the VDB itself.
		virtual void entityAddedCallback( hkEntity* entity );
		virtual void entityRemovedCallback( hkEntity* entity );
		virtual void phantomAddedCallback( hkPhantom* phantom );
		virtual void phantomRemovedCallback( hkPhantom* phantom );
		virtual void constraintAddedCallback( hkConstraintInstance* constraint );
		virtual void constraintRemovedCallback( hkConstraintInstance* constraint );
		virtual void actionAddedCallback( hkAction* action );
		virtual void actionRemovedCallback( hkAction* action );

	protected:

			/// As a context must exist at least as long as the VDB, we explicitly
			/// do not allow local variables of it to force the use of 'new' and removeRefernce().
			/// The VDB itself can't add a reference a context is just a abstract interface that
			/// any user data item can impliment for their own viewers.
		virtual ~hkPhysicsContext();

			/// Iterates through existing objects (entities, phantoms etc)
			/// and adds them for Inspection (tweaking) and then adds 
			/// listeners to pick up when that state changes and objects are
			/// added or removed etc. Called upon addWorld.
			/// Non const as has to add listeners
		void addForInspection( hkWorld* w );

			/// Iterates through existing objects (entities, phantoms etc)
			/// and removes them for Inspection (tweaking). Called upon removeWorld.
			/// Non const as has to remove listeners
		void removeFromInspection( hkWorld* w);

		hkArray<hkWorld*> m_worlds;
		hkArray<hkPhysicsContextWorldListener*> m_addListeners;
};

#endif // HK_VISUALIZE_PROCESS_CONTEXT_H

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
