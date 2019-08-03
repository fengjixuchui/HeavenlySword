/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#include <hkbase/hkBase.h>
#include <hkbase/class/hkTypeInfo.h>

HK_REFLECTION_DEFINE_VIRTUAL(hkReferencedObject);

// base object does not define placement new, so implement manually
// a.k.a. HK_REFLECTION_DEFINE_VIRTUAL(hkBaseObject);
static void HK_CALL finishLoadedObjecthkBaseObject(void*)
{
}
static void HK_CALL cleanupLoadedObjecthkBaseObject(void*)
{
}
static const void* HK_CALL getVtablehkBaseObject()
{
	hkBaseObject o;
	return *(void**)(&o);
}
extern const hkTypeInfo hkBaseObjectTypeInfo;
const hkTypeInfo hkBaseObjectTypeInfo( "hkBaseObject", finishLoadedObjecthkBaseObject, cleanupLoadedObjecthkBaseObject, getVtablehkBaseObject() );

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
