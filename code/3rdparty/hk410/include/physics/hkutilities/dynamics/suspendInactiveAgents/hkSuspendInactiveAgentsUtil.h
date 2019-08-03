/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_UTILITIES2_SUSPEND_INACTIVE_AGENTS_H
#define HK_UTILITIES2_SUSPEND_INACTIVE_AGENTS_H

#include <hkbase/hkBase.h>
#include <hkdynamics/world/listener/hkIslandActivationListener.h>
#include <hkdynamics/world/listener/hkWorldDeletionListener.h>


	/// A class which tries to free collision cache memory for inactive islands
	/// Note: this class works because of the current implementation of hkBvTreeStreamAgent::updateShapeCollectionFilter
class hkSuspendInactiveAgentsUtil : public hkReferencedObject, hkIslandActivationListener, hkWorldDeletionListener
{
	public:
		enum OperationMode
		{
				/// Remove all collision agents which are a subagent of the bvtreestream agent.
				/// Typically this are the agents between moving bodies and the triangles of the
				/// landscape. Note: This type of agents account for most collision agent memory in most games.
				/// Using this mode you will see small artifacts when objects wake 
			SUSPEND_1N_AGENT_TRACKS,

				/// Removes all child agents for shape collections. This performs another explicit call to updateCollisionFilter on collections
				/// when the island is activated. 
				/// Note:This call is not safe if that update call would remove agents, or in multithreaded simulation.
			SUSPEND_ALL_COLLECTION_AGENTS,

		};

			/// This deterimines how contacts are found when an island activates
		enum InitContactsMode
		{
				/// This is the default. When an island activates, it does extra work to try to find more contact points between colliding
				/// pairs whose agents were deleted.
			INIT_CONTACTS_FIND,

				/// This setting disables the extra work on activation. It is faster, but may result in some extra initial jitter as objects
				/// activate.
			INIT_CONTACTS_DO_NOT_FIND
		};

			//
			// Public functions.
			//

			/// Constructor takes a pointer to an hkWorld and a flag indicating
			/// how aggressively you want to remove agents.
			/// It automatically adds itself to the world and increases its own reference count.
			/// When the world is deleted, it decreases its own reference count.
		hkSuspendInactiveAgentsUtil(hkWorld* world, OperationMode mode = SUSPEND_1N_AGENT_TRACKS, InitContactsMode initContactsMode = INIT_CONTACTS_FIND );

		~hkSuspendInactiveAgentsUtil();

	public:
			/// Called when an island is activated. Simply does nothing.
		virtual void islandActivatedCallback( hkSimulationIsland* island );

			/// Called when an island is deactivated
		virtual void islandDeactivatedCallback( hkSimulationIsland* island );

			/// Called when the hkWorld is deleted.
		virtual void worldDeletedCallback( hkWorld* world);

	public:
		hkWorld* m_world;
		OperationMode m_mode;
		InitContactsMode m_initContactsMode;
};

#endif // HK_UTILITIES2_SUSPEND_INACTIVE_AGENTS_H

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
