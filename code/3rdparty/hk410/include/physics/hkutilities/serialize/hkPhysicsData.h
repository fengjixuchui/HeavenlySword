/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HKUTILITIES_PHYSICSDATA_H
#define HKUTILITIES_PHYSICSDATA_H

#include <hkdynamics/world/hkWorldCinfo.h>
#include <hkdynamics/world/hkPhysicsSystem.h>

class hkWorld;

extern const class hkClass hkPhysicsDataClass;

/// This is the top level container class for physics systems
class hkPhysicsData : public hkReferencedObject
{
	public:
	
		HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_UTILITIES);
		HK_DECLARE_REFLECTION();

			/// Default constructor
		hkPhysicsData();

			/// Default destructor - dereferences any data
		virtual ~hkPhysicsData();

		//
		// Accessors. 
		//

			/// Get the world cinfo of the data
		inline hkWorldCinfo* getWorldCinfo();

			/// Set the world cinfo of the data
		inline void setWorldCinfo( hkWorldCinfo* info );

			/// Add a physics system to the data
			/// This adds a reference to the system
		inline void addPhysicsSystem( hkPhysicsSystem* system );

			/// Remove a physics system from the data
			/// This removes a reference from the system
		inline void removePhysicsSystem( int i );

			/// Get a const list of physics systems
		inline const hkArray<hkPhysicsSystem*>& getPhysicsSystems() const;

			/// Look for a physics system by name (case insensitive)
		hkPhysicsSystem* findPhysicsSystemByName (const char* name) const;

			/// Look for a rigid body by name (case insensitive)
		hkRigidBody* findRigidBodyByName (const char* name) const;

		//
		// Utility Functions
		//


			/// Fill in systems from a live hkWorld object
		void populateFromWorld( const hkWorld* world );

			/// Utility function to convert the cinfo and systems	
			/// into an actual world.
			/// Before the bodies are added to the world, it is usually
			/// desired that all agents are registered (or overlapping pairs
			/// will not be dealt with correctly at the start). If you wish
			/// to register only your own subset (to reduce code size) 
			/// then copy the code from this func and make your own create util func.
		hkWorld* createWorld(hkBool registerAllAgents = true);

			/// Structure returned by splitPhysicsSystems()
		struct SplitPhysicsSystemsOutput
		{
			/// One system for unconstrained fixed rigid bodies
			hkPhysicsSystem* m_unconstrainedFixedBodies;
			/// One system for unconstrained keyframed bodies
			hkPhysicsSystem* m_unconstrainedKeyframedBodies;
			/// One system for unconstrained moving bodies
			hkPhysicsSystem* m_unconstrainedMovingBodies;
			/// One system to contain all the phantoms in the input system
			hkPhysicsSystem* m_phantoms;
			/// One system per group of constrained bodies (such as a ragdoll)
			hkArray<hkPhysicsSystem*> m_constrainedSystems;
		};

			/// Utility function to split one physics system into a group of systems.
			/// The user data pointer of the input system is set to the user data for all systems created.
			///	
			/// This converts the system into:
			/// One system for unconstrained fixed rigid bodies
			/// One system for unconstrained keyframed bodies
			/// One system for unconstrained moving bodies
			/// One system per group of constrained bodies (such as a ragdoll)
			/// One system to contain all the phantoms in the input system
			/// The user data pointer of the input system is set to the user data for all systems created
		static void splitPhysicsSystems(const hkPhysicsSystem* inputSystemConst, SplitPhysicsSystemsOutput& output );

	protected:
		
			/// The world data. Reference counted.
		class hkWorldCinfo* m_worldCinfo;

			/// A list of physics systems.
		hkArray<class hkPhysicsSystem*> m_systems;


	public: 
		hkPhysicsData( hkFinishLoadedObjectFlag f) : m_systems(f) { }
};

#include <hkutilities/serialize/hkPhysicsData.inl>

#endif // HKUTILITIES_PHYSICSDATA_H

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
