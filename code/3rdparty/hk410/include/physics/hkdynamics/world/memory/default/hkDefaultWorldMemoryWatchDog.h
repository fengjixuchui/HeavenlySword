/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */



#ifndef HK_DYNAMICS2_DEFAULT_WORLD_MEMORY_WATCH_DOG_H
#define HK_DYNAMICS2_DEFAULT_WORLD_MEMORY_WATCH_DOG_H

#include <hkdynamics/world/memory/hkWorldMemoryWatchDog.h>

class hkWorld;
class hkStepInfo;
class hkSimulationIsland;

	/// The purpose of this class is to remove objects from the physics to
	/// free memory. To do this class simply iterates over all islands, finds the objects 
	/// with the highest autoRemoveLevel and removes those objects until
	/// the memory consumption drops below (hkWorldMemoryWatchDog::m_memoryLimit-m_minAmountOfMemoryToFreeAtATime)
class hkDefaultWorldMemoryWatchDog : public hkWorldMemoryWatchDog
{
	public:
			/// Constructor
			/// \param memoryLimit      The physics checks whether hkWorldMemoryWatchDog::getMemoryUsed() is below this parameter
			///							If not it calls freeMemory
			/// \param minMemoryToFree  The minimum amount of memory which is freed by every call to freeMemory.
			///							This makes sense, as freeing memory takes some CPU and you want to free enough memory
			///							to avoid to rapid calls to this function
		hkDefaultWorldMemoryWatchDog(hkInt32 memoryLimit, hkInt32 minMemoryToFree = 20000);

			/// Free memory
		void freeMemory( hkWorld* world );

	protected:
			/// Remove some entities from one island
		static void removeObjectsFromIsland( hkSimulationIsland* island, int minAutoRemoveLevelToProcess, hkInt32 targetMemoryUsage, int & maxFoundAutoRemoveLevelOut );

			/// The minimum amount of memory which is freed by every call to freeMemory.
			/// This makes sense, as freeing memory takes some CPU and you want to free enough memory
			/// to avoid to rapid calls to this function
		hkInt32       m_minAmountOfMemoryToFreeAtATime;


};


#endif // HK_DYNAMICS2_DEFAULT_WORLD_MEMORY_WATCH_DOG_H

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
