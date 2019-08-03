/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkbase/hkBase.h>
#include <hkbase/memory/hkStackTracer.h>

hkStackTracer::hkStackTracer()
{
}

hkStackTracer::~hkStackTracer()
{
}

void hkStackTracer::dumpStackTrace( const hkUlong* trace, int numtrace, printFunc pfunc, void* context )
{
	for( int i = 0; i < numtrace; ++i )
	{
		char buf[256];
		hkString::snprintf(buf, sizeof(buf), "0x%x\n", trace[i] );
		pfunc(buf, context);
	}
}

int hkStackTracer::getStackTrace( hkUlong* trace, int maxtrace )
{
	hkUlong temp;
	hkUlong numAddresses;
	asm volatile(""
			"mr %0, %%r1\n"
			"sub %1, %1, %1\n"
			".LStackTraceLoop%=:\n"
			"ld %0, 0(%0)\n"
			"stw %0, 0(%2)\n"
			"addi %2, %2, 4\n"
			"addi %1, %1, 1\n"
			"cmpd %1, %3\n"
			"beq .LStackTraceDone%=\n"
			"cmpwi %0, 0\n"
			"bne .LStackTraceLoop%=\n"
			".LStackTraceDone%=:\n"
			: "=&b"(temp),  "=&b"(numAddresses)
			: "r"(trace), "r"(maxtrace)
			: "memory");

	return numAddresses;
}

// nm --numeric-sort --demangle %ELF%.elf | grep -v " ? " | grep -v _compiled > %ELF%.sym


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
