/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_DEMOS_SPU_UTIL_H
#define HK_DEMOS_SPU_UTIL_H

#ifdef HK_PLATFORM_PS3
	#include <sys/spu_thread.h>
	#include <cell/spurs.h>
#elif defined(HK_SIMULATE_SPU_DMA_ON_CPU)
	#include <hkbase/thread/util/hkSpuSimulator.h>
#endif

struct hkSpuParams
{
	void* m_param0;
	void* m_param1;
	void* m_param2;
	void* m_param3;
};

	/// Wrapper class for running spus
class hkSpuUtil
{
	public:

		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_UTILITIES, hkSpuUtil);

		hkSpuUtil();

			// This calls quitSpurs or quitSpuThreadGroup as appropriate
		~hkSpuUtil();

		//
		// Simple Spurs wrapper
		//

			/// creates a single run-complete task the specified elf.
			/// You must specify an elf for the PS3, for the PC simulator this can be left as NULL
		void initSpurs(CellSpurs * spurs, void* elf = HK_NULL );

			/// Start SPURS task
		void startSpursTask( hkSpuParams& args );

			/// Quit spurs
		void quitSpurs();


		//
		// Simple Spu thread wrapper
		//
			
			/// Init thread group with number of required spus
		void initSpuThreadGroup( int numSpus, char* spuProg = HK_NULL );

			/// Start a thread.  This must be called as many times as the numSpus parameter passed into initSpuThreadGroup.
			/// The final time this is called, the thread group is actually started.
		void startSpuThread( hkSpuParams& args );

			/// Quite the thread group
		void quitSpuThreadGroup();



	private:

#ifdef HK_PLATFORM_PS3

		// SPURS
		CellSpurs* m_spurs;
		CellSpursTaskset* m_taskset;
		void* m_spursElf;

		// SPU threads
		sys_spu_thread_group_t m_groupId;	// SPU thread group ID 
		sys_spu_image m_spuThreadImg;
		int m_spuThreadCounter;
		int m_numSpuThreads;

#elif defined( HK_SIMULATE_SPU_DMA_ON_CPU )

		hkSpuSimulator::Server* m_spuSimulatorServer;

#endif

};


#endif // HK_DEMOS_SPU_UTIL_H

/*
* Havok SDK - DEMO RELEASE, BUILD(#20060630)
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
