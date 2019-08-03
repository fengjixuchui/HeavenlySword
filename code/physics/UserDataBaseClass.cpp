// TKBMS v1.0 -----------------------------------------------------
//
// PLATFORM   : ALL !SPU
// PRODUCT    : HAVOK_2
// VISIBILITY : PUBLIC
//
// ------------------------------------------------------TKBMS v1.0

// WARNING: THIS FILE IS GENERATED. EDITS WILL BE LOST.
// Generated from 'UserData/UserDataBase.h'

#include <hkbase/hkBase.h>
#include <hkbase/class/hkClass.h>
#include <hkbase/class/hkInternalClassMember.h>
#include "UserDataBase.h"


//
// Class UserDataBase
//
static const hkInternalClassMember UserDataBaseClass_Members[] =
{
	{ "uiVersionTag", HK_NULL, HK_NULL, hkClassMember::TYPE_CHAR, hkClassMember::TYPE_VOID, 4, 0, HK_OFFSET_OF(UserDataBase,m_uiVersionTag) }
};
extern const hkClass UserDataBaseClass;
const hkClass UserDataBaseClass(
	"UserDataBase",
	HK_NULL, // parent
	sizeof(UserDataBase),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(UserDataBaseClass_Members),
	int(sizeof(UserDataBaseClass_Members)/sizeof(hkInternalClassMember)),
	HK_NULL // defaults
	);
