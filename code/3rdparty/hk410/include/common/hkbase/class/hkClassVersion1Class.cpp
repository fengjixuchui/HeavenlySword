/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkbase/hkBase.h>
#include <hkbase/class/hkClass.h>
#include <hkbase/class/hkInternalClassMember.h>

class hkClassVersion1
{
	public:

		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_HKCLASS, hkClassVersion1 );
		HK_DECLARE_REFLECTION();

	public:

		hkClassVersion1() { }

	protected:

		const char* m_name;
		const hkClass* m_parent;
		int m_objectSize;
		//const hkClass** m_implementedInterfaces;
		int m_numImplementedInterfaces;
		const class hkClassEnum* m_declaredEnums;
		int m_numDeclaredEnums;
		const class hkClassMember* m_declaredMembers;
		int m_numDeclaredMembers;
		hkBool m_hasVtable;
		char m_padToSizeOfClass[sizeof(void*) - sizeof(hkBool)];
};

HK_COMPILE_TIME_ASSERT( sizeof(hkClassVersion1) == sizeof(hkClass) );

class hkClassMemberVersion1
{
	public:

		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_HKCLASS, hkClassMemberVersion1);
		HK_DECLARE_REFLECTION();

	public:

		hkClassMemberVersion1() { }

		enum Type
		{
			TYPE_VOID = 0,
			TYPE_BOOL,
			TYPE_CHAR,
			TYPE_INT8,
			TYPE_UINT8,
			TYPE_INT16,
			TYPE_UINT16,
			TYPE_INT32,
			TYPE_UINT32,
			TYPE_INT64,
			TYPE_UINT64,
			TYPE_REAL,
			TYPE_VECTOR4,
			TYPE_QUATERNION,
			TYPE_MATRIX3,
			TYPE_ROTATION,
			TYPE_QSTRANSFORM,
			TYPE_MATRIX4,
			TYPE_TRANSFORM,
			TYPE_ZERO,
			TYPE_POINTER,
			TYPE_FUNCTIONPOINTER,
			TYPE_ARRAY,
			TYPE_INPLACEARRAY,
			TYPE_ENUM,
			TYPE_STRUCT,
			TYPE_SIMPLEARRAY,
			TYPE_HOMOGENEOUSARRAY,
			TYPE_VARIANT,
			TYPE_MAX
		};

		enum Flags
		{
			POINTER_OPTIONAL = 1,
			POINTER_VOIDSTAR = 2,
			ENUM_8 = 8,
			ENUM_16 = 16,
			ENUM_32 = 32,
			ARRAY_RAWDATA = 64
		};

		enum Range
		{
			INVALID = 0,
			DEFAULT = 1,
			ABS_MIN = 2,
			ABS_MAX = 4,
			SOFT_MIN = 8,
			SOFT_MAX = 16,
			RANGE_MAX = 32
		};

		const char* m_name;
		const hkClass* m_class;
		const hkClassEnum* m_enum;
		hkEnum<Type,hkUint8> m_type;
		hkEnum<Type,hkUint8> m_subtype;
		hkInt16 m_cArraySize;
		hkUint16 m_flags;
		hkUint16 m_offset;
};

// External pointer and enum types
extern const hkClass hkClassClass;
extern const hkClass hkClassEnumClass;
extern const hkClass hkClassMemberVersion1Class;

//
// Class hkClass
//
const hkInternalClassMember hkClassVersion1::Members[] =
{
	{ "name", HK_NULL, HK_NULL, hkClassMember::TYPE_CSTRING, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkClassVersion1,m_name) },
	{ "parent", &hkClassClass, HK_NULL, hkClassMember::TYPE_POINTER, hkClassMember::TYPE_STRUCT, 0, 0, HK_OFFSET_OF(hkClassVersion1,m_parent) },
	{ "objectSize", HK_NULL, HK_NULL, hkClassMember::TYPE_INT32, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkClassVersion1,m_objectSize) },
	{ "numImplementedInterfaces", HK_NULL, HK_NULL, hkClassMember::TYPE_INT32, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkClassVersion1,m_numImplementedInterfaces) },
	{ "declaredEnums", &hkClassEnumClass, HK_NULL, hkClassMember::TYPE_SIMPLEARRAY, hkClassMember::TYPE_STRUCT, 0, 0, HK_OFFSET_OF(hkClassVersion1,m_declaredEnums) },
	{ "declaredMembers", &hkClassMemberVersion1Class, HK_NULL, hkClassMember::TYPE_SIMPLEARRAY, hkClassMember::TYPE_STRUCT, 0, 0, HK_OFFSET_OF(hkClassVersion1,m_declaredMembers) },
	{ "hasVtable", HK_NULL, HK_NULL, hkClassMember::TYPE_BOOL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkClassVersion1,m_hasVtable) }
};
extern const hkClass hkClassVersion1Class;
const hkClass hkClassVersion1Class(
	"hkClass",
	HK_NULL, // parent
	sizeof(hkClassVersion1),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkClassVersion1::Members),
	7, // members
	HK_NULL // defaults
	);

static const hkInternalClassEnumItem hkClassMemberVersion1TypeEnumItems[] =
{
	{0, "TYPE_VOID"},
	{1, "TYPE_BOOL"},
	{2, "TYPE_CHAR"},
	{3, "TYPE_INT8"},
	{4, "TYPE_UINT8"},
	{5, "TYPE_INT16"},
	{6, "TYPE_UINT16"},
	{7, "TYPE_INT32"},
	{8, "TYPE_UINT32"},
	{9, "TYPE_INT64"},
	{10, "TYPE_UINT64"},
	{11, "TYPE_REAL"},
	{12, "TYPE_VECTOR4"},
	{13, "TYPE_QUATERNION"},
	{14, "TYPE_MATRIX3"},
	{15, "TYPE_ROTATION"},
	{16, "TYPE_QSTRANSFORM"},
	{17, "TYPE_MATRIX4"},
	{18, "TYPE_TRANSFORM"},
	{19, "TYPE_ZERO"},
	{20, "TYPE_POINTER"},
	{21, "TYPE_FUNCTIONPOINTER"},
	{22, "TYPE_ARRAY"},
	{23, "TYPE_INPLACEARRAY"},
	{24, "TYPE_ENUM"},
	{25, "TYPE_STRUCT"},
	{26, "TYPE_SIMPLEARRAY"},
	{27, "TYPE_HOMOGENEOUSARRAY"},
	{28, "TYPE_VARIANT"},
	{29, "TYPE_MAX"},
};
static const hkInternalClassEnumItem hkClassMemberVersion1FlagsEnumItems[] =
{
	{1, "POINTER_OPTIONAL"},
	{2, "POINTER_VOIDSTAR"},
	{8, "ENUM_8"},
	{16, "ENUM_16"},
	{32, "ENUM_32"},
	{64, "ARRAY_RAWDATA"},
};
static const hkInternalClassEnumItem hkClassMemberVersion1RangeEnumItems[] =
{
	{0, "INVALID"},
	{1, "DEFAULT"},
	{2, "ABS_MIN"},
	{4, "ABS_MAX"},
	{8, "SOFT_MIN"},
	{16, "SOFT_MAX"},
	{32, "RANGE_MAX"},
};
static const hkInternalClassEnum hkClassMemberVersion1Enums[] = {
	{"Type", hkClassMemberVersion1TypeEnumItems, 30 },
	{"Flags", hkClassMemberVersion1FlagsEnumItems, 6 },
	{"Range", hkClassMemberVersion1RangeEnumItems, 7 }
};
static const hkClassEnum* hkClassMemberVersion1TypeEnum = reinterpret_cast<const hkClassEnum*>(&hkClassMemberVersion1Enums[0]);
static const hkClassEnum* hkClassMemberVersion1FlagsEnum = reinterpret_cast<const hkClassEnum*>(&hkClassMemberVersion1Enums[1]);
static const hkClassEnum* hkClassMemberVersion1RangeEnum = reinterpret_cast<const hkClassEnum*>(&hkClassMemberVersion1Enums[2]);

static hkInternalClassMember hkClassMemberVersion1Class_Members[] =
{
	{ "name", HK_NULL, HK_NULL, hkClassMember::TYPE_CSTRING, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkClassMemberVersion1,m_name) },
	{ "class", &hkClassVersion1Class, HK_NULL, hkClassMember::TYPE_POINTER, hkClassMember::TYPE_STRUCT, 0, 0, HK_OFFSET_OF(hkClassMemberVersion1,m_class) },
	{ "enum", &hkClassEnumClass, HK_NULL, hkClassMember::TYPE_POINTER, hkClassMember::TYPE_STRUCT, 0, 0, HK_OFFSET_OF(hkClassMemberVersion1,m_enum) },
	{ "type", HK_NULL, hkClassMemberVersion1TypeEnum, hkClassMember::TYPE_ENUM, hkClassMember::TYPE_VOID, 0, hkClassMember::ENUM_8, HK_OFFSET_OF(hkClassMemberVersion1,m_type) },
	{ "subtype", HK_NULL, hkClassMemberVersion1TypeEnum, hkClassMember::TYPE_ENUM, hkClassMember::TYPE_VOID, 0, hkClassMember::ENUM_8, HK_OFFSET_OF(hkClassMemberVersion1,m_subtype) },
	{ "cArraySize", HK_NULL, HK_NULL, hkClassMember::TYPE_INT16, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkClassMemberVersion1,m_cArraySize) },
	{ "flags", HK_NULL, HK_NULL, hkClassMember::TYPE_UINT16, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkClassMemberVersion1,m_flags) },
	{ "offset", HK_NULL, HK_NULL, hkClassMember::TYPE_UINT16, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkClassMemberVersion1,m_offset) }
};

const hkClass hkClassMemberVersion1Class(
	"hkClassMember",
	HK_NULL, // parent
	sizeof(hkClassMemberVersion1),
	HK_NULL,
	0, // interfaces
	reinterpret_cast<const hkClassEnum*>(hkClassMemberVersion1Enums),
	3, // enums
	reinterpret_cast<const hkClassMember*>(hkClassMemberVersion1Class_Members),
	8, // members
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
