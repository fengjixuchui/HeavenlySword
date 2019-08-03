/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_MOPP_MODIFY_VIRTUAL_MACHINE_H
#define HK_COLLIDE2_MOPP_MODIFY_VIRTUAL_MACHINE_H

#include <hkinternal/collide/mopp/machine/hkMoppObbVirtualMachine.h>
#include <hkinternal/collide/mopp/machine/hkMoppModifier.h>

class hkMoppModifyVirtualMachine : public hkMoppObbVirtualMachine 
{
	public:
		// standard constructor
		inline hkMoppModifyVirtualMachine(){}
		// standard destructor
		inline ~hkMoppModifyVirtualMachine(){}

			/// Read the hkMoppModifyVirtualMachine_queryAabb documentation
		void queryAabb( const hkMoppCode* code, const hkAabb& aabb, hkMoppModifier* modifierOut );

	protected:
		HK_FORCE_INLINE void addHit(unsigned int id, const unsigned int properties[hkMoppCode::MAX_PRIMITIVE_PROPERTIES]);

			/// returns true if this node should be removed
		hkBool queryModicationPointsRecursive	(const hkMoppObbVirtualMachineQuery* query, const unsigned char* commands);

	protected:
		hkBool			m_tempLastShouldTerminalBeRemoved;
		hkMoppModifier* m_modifier;
};

#endif // HK_COLLIDE2_MOPP_MODIFY_VIRTUAL_MACHINE_H

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
