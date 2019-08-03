/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */



#ifndef HK_DYNAMICS2_DEFAULT_WORLD_MAINTENANCE_MGR_H
#define HK_DYNAMICS2_DEFAULT_WORLD_MAINTENANCE_MGR_H

#include <hkdynamics/world/maintenance/hkWorldMaintenanceMgr.h>

class hkDefaultWorldMaintenanceMgr : public hkWorldMaintenanceMgr
{
	public:
		hkDefaultWorldMaintenanceMgr();

		virtual void init( hkWorld* world);
		virtual void performMaintenance( hkWorld* world, hkStepInfo& stepInfo );

			// do all maintenance but ignore island split checks. This is used for multithreaded simulation, where
			// the split check is done in parallel to the solve job
		virtual void performMaintenanceNoSplit( hkWorld* world, hkStepInfo& stepInfo );

	private:
		inline void resetWorldTime( hkWorld* world, hkStepInfo& stepInfo);

			// this is the old Havok400 style deactivation checks at the beginning of each frame
		inline void markIslandsForDeactivationDeprecated( hkWorld* world, hkStepInfo& stepInfo);


	protected:
		/// A range within the time variable will be held
		hkReal m_minAllowedTimeValue;
		hkReal m_maxAllowedTimeValue;
};




#endif // HK_DYNAMICS2_DEFAULT_WORLD_MAINTENANCE_MGR_H

/*
* Havok SDK - CLIENT RELEASE, BUILD(#20060902)
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
