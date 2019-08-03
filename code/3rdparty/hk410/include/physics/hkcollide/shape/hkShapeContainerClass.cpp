/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

// WARNING: THIS FILE IS GENERATED. EDITS WILL BE LOST.
// Generated from 'hkcollide/shape/hkShapeContainer.h'

#include <hkcollide/hkCollide.h>
#include <hkbase/class/hkClass.h>
#include <hkbase/class/hkInternalClassMember.h>
#include <hkcollide/shape/hkShapeContainer.h>


// External pointer and enum types
extern const hkClass hkShapeClass;

//
// Class hkShapeContainer
//
extern const hkClass hkShapeContainerClass;
const hkClass hkShapeContainerClass(
	"hkShapeContainer",
	HK_NULL, // parent
	sizeof(hkShapeContainer),
	HK_NULL,
	1, // interfaces
	HK_NULL,
	0, // enums
	HK_NULL,
	0,
	HK_NULL // defaults
	);

//
// Class hkSingleShapeContainer
//
const hkInternalClassMember hkSingleShapeContainer::Members[] =
{
	{ "childShape", &hkShapeClass, HK_NULL, hkClassMember::TYPE_POINTER, hkClassMember::TYPE_STRUCT, 0, 0, HK_OFFSET_OF(hkSingleShapeContainer,m_childShape) }
};

extern const hkClass hkSingleShapeContainerClass;
const hkClass hkSingleShapeContainerClass(
	"hkSingleShapeContainer",
	&hkShapeContainerClass, // parent
	sizeof(hkSingleShapeContainer),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkSingleShapeContainer::Members),
	int(sizeof(hkSingleShapeContainer::Members)/sizeof(hkInternalClassMember)),
	HK_NULL // defaults
	);

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
