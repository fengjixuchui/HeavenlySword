/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_DYNAMICS2_PHYSICS_COMMAND_QUEUE_H
#define HK_DYNAMICS2_PHYSICS_COMMAND_QUEUE_H

#include <hkdynamics/world/commandqueue/hkPhysicsCommand.h>

class hkPhysicsCommandQueue
{
	public:
		HK_FORCE_INLINE hkPhysicsCommandQueue( hkPhysicsCommand* bufferStart, hkPhysicsCommand* bufferEnd );

	public:
		template<typename COMMAND_STRUCT> 
		HK_FORCE_INLINE void addCommand(COMMAND_STRUCT command);

		template<typename COMMAND_STRUCT> 
		HK_FORCE_INLINE COMMAND_STRUCT& newCommand();

	public:
		hkPhysicsCommand* m_start;
		hkPhysicsCommand* m_current;
		hkPhysicsCommand* m_end;
};

template<int NUM_BYTES>
class hkFixedSizePhysicsCommandQueue: public hkPhysicsCommandQueue
{
	public:
		hkFixedSizePhysicsCommandQueue(): hkPhysicsCommandQueue( (hkPhysicsCommand*)&m_buffer[0], (hkPhysicsCommand*)&m_buffer[NUM_BYTES] ){}

	public:
		HK_ALIGN16( hkUchar m_buffer[NUM_BYTES] );
};

#include <hkdynamics/world/commandqueue/hkPhysicsCommandQueue.inl>


#endif // HK_DYNAMICS2_PHYSICS_COMMAND_QUEUE_H

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
