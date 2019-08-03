// TKBMS v1.0 -----------------------------------------------------
//
// PLATFORM   : ALL !SPU
// PRODUCT    : HAVOK_2
// VISIBILITY : PUBLIC
//
// ------------------------------------------------------TKBMS v1.0

// WARNING: THIS FILE IS GENERATED. EDITS WILL BE LOST.
// Generated from 'UserData/ShapeUserData.h'

#include <hkbase/hkBase.h>
#include <hkbase/class/hkClass.h>
#include <hkbase/class/hkInternalClassMember.h>
#include "ShapeUserData.h"


//
// Class ShapeUserData
//
static const hkInternalClassMember ShapeUserDataClass_Members[] =
{
	{ "uiTransformHash", HK_NULL, HK_NULL, hkClassMember::TYPE_UINT32, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(ShapeUserData,m_uiTransformHash) },
	{ "uiMaterialID", HK_NULL, HK_NULL, hkClassMember::TYPE_UINT32, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(ShapeUserData,m_uiMaterialID) }
};
extern const hkClass UserDataBaseClass;

extern const hkClass ShapeUserDataClass;
const hkClass ShapeUserDataClass(
	"ShapeUserData",
	&UserDataBaseClass, // parent
	sizeof(ShapeUserData),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(ShapeUserDataClass_Members),
	int(sizeof(ShapeUserDataClass_Members)/sizeof(hkInternalClassMember)),
	HK_NULL // defaults
	);
