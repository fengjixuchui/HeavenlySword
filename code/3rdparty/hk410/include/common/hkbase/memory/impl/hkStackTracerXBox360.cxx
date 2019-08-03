/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

//#include <hkbase/fwd/hkcstdio.h> // Needed for printfs
#include <xtl.h>
#include <Xbdm.h>

/*
 * hkStackTracer has no member variables, so I am storing the address as a
 * static variable.
 *
 */
static hkUint32 module_address = HK_NULL;

hkStackTracer::hkStackTracer ( void )
{
	HRESULT				error;
	DMN_MODLOAD			modLoad;
	PDM_WALK_MODULES	walkModules	= HK_NULL;

	/* Find the address that the last module loaded was loaded at */
	while( XBDM_NOERR == (error = DmWalkLoadedModules(&walkModules, &modLoad)) )
	{
		//printf("Module %s loaded at 0x%08x\n", modLoad.Name, modLoad.BaseAddress);
		module_address	= (hkUint32)modLoad.BaseAddress;
	}

	/* DmWalkLoadedModules failed. */
	if (error != XBDM_ENDOFLIST)
	{
		HK_BREAKPOINT();
	}

	/* Free memory allocated by DmWalkLoadedModules */
	DmCloseLoadedModules(walkModules);
}

hkStackTracer::~hkStackTracer( void )
{
}

void hkStackTracer::dumpStackTrace( const hkUlong* trace, int maxtrace, printFunc pfunc, void* context )
{
	/* Allocate some storage to print the callstack to */
	char buffer[16];

	while (maxtrace--)
	{
		hkString::snprintf(buffer, sizeof(buffer), "0x%08X\n", *(trace++));
		pfunc(buffer, context);
	}
}

int hkStackTracer::getStackTrace( hkUlong* trace, int maxtrace )
{
	if (trace == HK_NULL)
	{
		/* Null trace pointer passed to hkStackTracer::getStackTrace. */
		HK_BREAKPOINT(); 
		return 0;
	}

	/*
	 * The preferred address is ignored on Xbox360. Modules are all loaded with an
	 * offset that is shown at the top of the map file. The address seems to be
	 * always 0x00400000. If this stack tracer stops working in the future, it
	 * could be caused by this magic number being changed.
	 */
	const unsigned int	MAGIC_ADDRESS = 0x00400000;
	HRESULT				error;

	error = DmCaptureStackBackTrace(maxtrace, (PVOID*)trace);

	if (error != XBDM_NOERR)
	{
		// DmCaptureStackBackTrace failed
		HK_BREAKPOINT();
		return 0;
	}

	/*
	 * We need to pre-process the back-trace to add a standard offset to every
	 * entry. This is because the modules are not stored in memory at address 0.
	 *
	 */

	int i;

	for (i=0; trace[i] && i < maxtrace; i++ )
	{
		trace[i] += MAGIC_ADDRESS;
		trace[i] -= module_address;
	}

	return i;
}

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
