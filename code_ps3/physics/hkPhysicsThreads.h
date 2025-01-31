/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

// This is a utility header used to emulate the running of spu physics threads on
// the pc

#if defined HK_PLATFORM_PS3
#define SPU_THREAD_FUNC "/app_home/hksputhreadconstraint.elf"
#else
#define SPU_THREAD_FUNC hkConstraintSolveThreadMain
#endif

void* HK_CALL hkConstraintSolveThreadMain( void* params );
void* HK_CALL hkConstraintSolveSpursTask( void* params );

void* HK_CALL hkCpuThreadMain( void* params);

/*
* Havok SDK - DEMO RELEASE, BUILD(#20060116)
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
