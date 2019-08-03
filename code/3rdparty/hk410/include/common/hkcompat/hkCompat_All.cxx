/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COMPAT_FILE
#	error Please define HK_COMPAT_FILE before including this file
#endif

// The default is to list all classes and updaters

#ifndef HK_LIST_UPDATERS
#	define HK_LIST_UPDATERS 1
#endif

#ifndef HK_LIST_CLASS_VERSIONS
#	define HK_LIST_CLASS_VERSIONS 1
#endif

#include <hkbase/config/hkConfigVersion.h>
#include <hkserialize/version/hkVersionRegistry.h>
#include <hkserialize/util/hkBuiltinTypeRegistry.h>

// Forward declarations of all used classes, versions and updaters
#if HK_LIST_UPDATERS || HK_LIST_CLASS_VERSIONS
#	define HK_CLASS_UPDATE_INFO(FROM,TO) \
		namespace hkHavok##FROM##Classes \
		{ \
			extern const char VersionString[]; \
			extern hkClass* const Classes[]; \
		} \
		namespace hkCompat_hk##FROM##_hk##TO \
		{ \
			extern const hkVersionRegistry::Updater hkVersionUpdater; \
		}
#		include HK_COMPAT_FILE
#	undef HK_CLASS_UPDATE_INFO
#endif

#if HK_LIST_UPDATERS
// List of all updaters

#	define HK_CLASS_UPDATE_INFO(FROM,TO) &hkCompat_hk##FROM##_hk##TO::hkVersionUpdater,

const hkVersionRegistry::Updater* hkVersionRegistry::StaticLinkedUpdaters[] =
{
#		include HK_COMPAT_FILE
	HK_NULL
};
#	undef HK_CLASS_UPDATE_INFO

#else

// empty updaters list
const hkVersionRegistry::Updater* hkVersionRegistry::StaticLinkedUpdaters[] =
{
	HK_NULL
};

#endif

#if HK_LIST_CLASS_VERSIONS
// List of all versions and corresponding classes

#	define HK_CLASS_UPDATE_INFO(FROM,TO) { hkHavok##FROM##Classes::VersionString, hkHavok##FROM##Classes::Classes },
const hkVersionRegistry::ClassList hkVersionRegistry::StaticLinkedClassList[] =
{
#		include HK_COMPAT_FILE
	{ HAVOK_SDK_VERSION_STRING, const_cast<hkClass*const*>(hkBuiltinTypeRegistry::StaticLinkedClasses)},
	{ HK_NULL, HK_NULL }
};
#	undef HK_CLASS_UPDATE_INFO

#else

// empty class list
const hkVersionRegistry::ClassList hkVersionRegistry::StaticLinkedClassList[] =
{
	{ HK_NULL, HK_NULL }
};

#endif

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
