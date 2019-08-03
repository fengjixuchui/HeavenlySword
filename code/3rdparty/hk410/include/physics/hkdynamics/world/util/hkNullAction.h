/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_DYNAMICS2_NULL_ACTION_H
#define HK_DYNAMICS2_NULL_ACTION_H


#include <hkmath/hkMath.h>
#include <hkmath/basetypes/hkStepInfo.h>
#include <hkdynamics/entity/hkEntityListener.h>
#include <hkdynamics/action/hkAction.h>
//#include <hkdynamics/action/hkNullActionCinfo.h>

class hkEntity;
class hkPhantom;
class hkSimulationIsland;
class hkStepInfo;
class hkWorld;

/// This is the base class from which user actions (or controllers) are derived. Actions 
/// are the interface between user controllable behavior of the physical simulation and the Havok core. 
class hkNullAction : public hkAction
{
	public:

		HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_ACTION);

		inline hkNullAction() : hkAction(0) {}

		virtual void applyAction( const hkStepInfo& stepInfo )      {}

		virtual void getEntities( hkArray<hkEntity*>& entitiesOut ) {}

		virtual void entityRemovedCallback(hkEntity* entity)        {}

		virtual hkAction* clone( const hkArray<hkEntity*>& entitiesIn, const hkArray<hkPhantom*>& newPhantoms  ) const  { return HK_NULL; }

		static inline hkNullAction* HK_CALL getNullAction(){ return HK_NULL; }

};

#endif // HK_DYNAMICS2_NULL_ACTION_H

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
