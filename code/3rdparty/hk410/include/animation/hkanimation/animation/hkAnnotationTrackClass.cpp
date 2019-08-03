/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

// WARNING: THIS FILE IS GENERATED. EDITS WILL BE LOST.
// Generated from 'hkanimation/animation/hkAnnotationTrack.h'

#include <hkanimation/hkAnimation.h>
#include <hkbase/class/hkClass.h>
#include <hkbase/class/hkInternalClassMember.h>
#include <hkanimation/animation/hkAnnotationTrack.h>


// External pointer and enum types
extern const hkClass hkAnnotationTrackAnnotationClass;

//
// Class hkAnnotationTrack::Annotation
//
static const hkInternalClassMember hkAnnotationTrack_AnnotationClass_Members[] =
{
	{ "time", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkAnnotationTrack::Annotation,m_time) },
	{ "text", HK_NULL, HK_NULL, hkClassMember::TYPE_CSTRING, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkAnnotationTrack::Annotation,m_text) }
};
const hkClass hkAnnotationTrackAnnotationClass(
	"hkAnnotationTrackAnnotation",
	HK_NULL, // parent
	sizeof(hkAnnotationTrack::Annotation),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkAnnotationTrack_AnnotationClass_Members),
	int(sizeof(hkAnnotationTrack_AnnotationClass_Members)/sizeof(hkInternalClassMember)),
	HK_NULL // defaults
	);

//
// Class hkAnnotationTrack
//
static const hkInternalClassMember hkAnnotationTrackClass_Members[] =
{
	{ "name", HK_NULL, HK_NULL, hkClassMember::TYPE_CSTRING, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkAnnotationTrack,m_name) },
	{ "annotations", &hkAnnotationTrackAnnotationClass, HK_NULL, hkClassMember::TYPE_SIMPLEARRAY, hkClassMember::TYPE_STRUCT, 0, 0, HK_OFFSET_OF(hkAnnotationTrack,m_annotations) }
};
extern const hkClass hkAnnotationTrackClass;
const hkClass hkAnnotationTrackClass(
	"hkAnnotationTrack",
	HK_NULL, // parent
	sizeof(hkAnnotationTrack),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkAnnotationTrackClass_Members),
	int(sizeof(hkAnnotationTrackClass_Members)/sizeof(hkInternalClassMember)),
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
