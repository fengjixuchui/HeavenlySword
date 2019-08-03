/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

// WARNING: THIS FILE IS GENERATED. EDITS WILL BE LOST.
// Generated from 'hkconstraintsolver/constraint/atom/hkConstraintAtom.h'
#include <hkdynamics/hkDynamics.h>

#include <hkbase/class/hkClass.h>
#include <hkbase/class/hkInternalClassMember.h>
#include <hkconstraintsolver/constraint/atom/hkConstraintAtom.h>


// External pointer and enum types
extern const hkClass hkBridgeConstraintAtomClass;
extern const hkClass hkConstraintAtomClass;
extern const hkClass hkConstraintDataClass;
extern const hkClass hkConstraintMotorClass;

//
// Enum hkConstraintAtom::AtomType
//
static const hkInternalClassEnumItem hkConstraintAtomAtomTypeEnumItems[] =
{
	{0, "TYPE_INVALID"},
	{1, "TYPE_BRIDGE"},
	{2, "TYPE_SET_LOCAL_TRANSFORMS"},
	{3, "TYPE_SET_LOCAL_TRANSLATIONS"},
	{4, "TYPE_SET_LOCAL_ROTATIONS"},
	{5, "TYPE_BALL_SOCKET"},
	{6, "TYPE_STIFF_SPRING"},
	{7, "TYPE_LIN"},
	{8, "TYPE_LIN_SOFT"},
	{9, "TYPE_LIN_LIMIT"},
	{10, "TYPE_LIN_FRICTION"},
	{11, "TYPE_LIN_MOTOR"},
	{12, "TYPE_2D_ANG"},
	{13, "TYPE_ANG"},
	{14, "TYPE_ANG_LIMIT"},
	{15, "TYPE_TWIST_LIMIT"},
	{16, "TYPE_CONE_LIMIT"},
	{17, "TYPE_ANG_FRICTION"},
	{18, "TYPE_ANG_MOTOR"},
	{19, "TYPE_RAGDOLL_MOTOR"},
	{20, "TYPE_PULLEY"},
	{21, "TYPE_OVERWRITE_PIVOT"},
	{22, "TYPE_CONTACT"},
	{23, "TYPE_MODIFIER_SOFT_CONTACT"},
	{24, "TYPE_MODIFIER_MASS_CHANGER"},
	{25, "TYPE_MODIFIER_VISCOUS_SURFACE"},
	{26, "TYPE_MODIFIER_MOVING_SURFACE"},
	{27, "TYPE_MAX"},
};

//
// Enum hkConstraintAtom::CallbackRequest
//
static const hkInternalClassEnumItem hkConstraintAtomCallbackRequestEnumItems[] =
{
	{0, "CALLBACK_REQUEST_NONE"},
	{1, "CALLBACK_REQUEST_CONTACT_POINT"},
	{2, "CALLBACK_REQUEST_SETUP_PPU_ONLY"},
};
static const hkInternalClassEnum hkConstraintAtomEnums[] = {
	{"AtomType", hkConstraintAtomAtomTypeEnumItems, 28 },
	{"CallbackRequest", hkConstraintAtomCallbackRequestEnumItems, 3 }
};
extern const hkClassEnum* hkConstraintAtomAtomTypeEnum = reinterpret_cast<const hkClassEnum*>(&hkConstraintAtomEnums[0]);
extern const hkClassEnum* hkConstraintAtomCallbackRequestEnum = reinterpret_cast<const hkClassEnum*>(&hkConstraintAtomEnums[1]);

//
// Class hkConstraintAtom
//
static const hkInternalClassMember hkConstraintAtomClass_Members[] =
{
	{ "type", HK_NULL, hkConstraintAtomAtomTypeEnum, hkClassMember::TYPE_ENUM, hkClassMember::TYPE_VOID, 0, hkClassMember::ENUM_16, HK_OFFSET_OF(hkConstraintAtom,m_type) }
};
const hkClass hkConstraintAtomClass(
	"hkConstraintAtom",
	HK_NULL, // parent
	sizeof(hkConstraintAtom),
	HK_NULL,
	0, // interfaces
	reinterpret_cast<const hkClassEnum*>(hkConstraintAtomEnums),
	2, // enums
	reinterpret_cast<const hkClassMember*>(hkConstraintAtomClass_Members),
	int(sizeof(hkConstraintAtomClass_Members)/sizeof(hkInternalClassMember)),
	HK_NULL // defaults
	);

//
// Class hkBridgeConstraintAtom
//
static const hkInternalClassMember hkBridgeConstraintAtomClass_Members[] =
{
	{ "buildJacobianFunc", HK_NULL, HK_NULL, hkClassMember::TYPE_ZERO, hkClassMember::TYPE_POINTER, 0, 0, HK_OFFSET_OF(hkBridgeConstraintAtom,m_buildJacobianFunc) },
	{ "constraintData", &hkConstraintDataClass, HK_NULL, hkClassMember::TYPE_POINTER, hkClassMember::TYPE_STRUCT, 0, 0, HK_OFFSET_OF(hkBridgeConstraintAtom,m_constraintData) }
};

const hkClass hkBridgeConstraintAtomClass(
	"hkBridgeConstraintAtom",
	&hkConstraintAtomClass, // parent
	sizeof(hkBridgeConstraintAtom),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkBridgeConstraintAtomClass_Members),
	int(sizeof(hkBridgeConstraintAtomClass_Members)/sizeof(hkInternalClassMember)),
	HK_NULL // defaults
	);

//
// Class hkBridgeAtoms
//
static const hkInternalClassMember hkBridgeAtomsClass_Members[] =
{
	{ "bridgeAtom", &hkBridgeConstraintAtomClass, HK_NULL, hkClassMember::TYPE_STRUCT, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkBridgeAtoms,m_bridgeAtom) }
};
extern const hkClass hkBridgeAtomsClass;
const hkClass hkBridgeAtomsClass(
	"hkBridgeAtoms",
	HK_NULL, // parent
	sizeof(hkBridgeAtoms),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkBridgeAtomsClass_Members),
	int(sizeof(hkBridgeAtomsClass_Members)/sizeof(hkInternalClassMember)),
	HK_NULL // defaults
	);

//
// Class hkBallSocketConstraintAtom
//

extern const hkClass hkBallSocketConstraintAtomClass;
const hkClass hkBallSocketConstraintAtomClass(
	"hkBallSocketConstraintAtom",
	&hkConstraintAtomClass, // parent
	sizeof(hkBallSocketConstraintAtom),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	HK_NULL,
	0,
	HK_NULL // defaults
	);

//
// Class hkStiffSpringConstraintAtom
//
static const hkInternalClassMember hkStiffSpringConstraintAtomClass_Members[] =
{
	{ "length", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkStiffSpringConstraintAtom,m_length) }
};

extern const hkClass hkStiffSpringConstraintAtomClass;
const hkClass hkStiffSpringConstraintAtomClass(
	"hkStiffSpringConstraintAtom",
	&hkConstraintAtomClass, // parent
	sizeof(hkStiffSpringConstraintAtom),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkStiffSpringConstraintAtomClass_Members),
	int(sizeof(hkStiffSpringConstraintAtomClass_Members)/sizeof(hkInternalClassMember)),
	HK_NULL // defaults
	);

//
// Class hkSetLocalTransformsConstraintAtom
//
static const hkInternalClassMember hkSetLocalTransformsConstraintAtomClass_Members[] =
{
	{ "transformA", HK_NULL, HK_NULL, hkClassMember::TYPE_TRANSFORM, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkSetLocalTransformsConstraintAtom,m_transformA) },
	{ "transformB", HK_NULL, HK_NULL, hkClassMember::TYPE_TRANSFORM, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkSetLocalTransformsConstraintAtom,m_transformB) }
};

extern const hkClass hkSetLocalTransformsConstraintAtomClass;
const hkClass hkSetLocalTransformsConstraintAtomClass(
	"hkSetLocalTransformsConstraintAtom",
	&hkConstraintAtomClass, // parent
	sizeof(hkSetLocalTransformsConstraintAtom),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkSetLocalTransformsConstraintAtomClass_Members),
	int(sizeof(hkSetLocalTransformsConstraintAtomClass_Members)/sizeof(hkInternalClassMember)),
	HK_NULL // defaults
	);

//
// Class hkSetLocalTranslationsConstraintAtom
//
static const hkInternalClassMember hkSetLocalTranslationsConstraintAtomClass_Members[] =
{
	{ "translationA", HK_NULL, HK_NULL, hkClassMember::TYPE_VECTOR4, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkSetLocalTranslationsConstraintAtom,m_translationA) },
	{ "translationB", HK_NULL, HK_NULL, hkClassMember::TYPE_VECTOR4, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkSetLocalTranslationsConstraintAtom,m_translationB) }
};

extern const hkClass hkSetLocalTranslationsConstraintAtomClass;
const hkClass hkSetLocalTranslationsConstraintAtomClass(
	"hkSetLocalTranslationsConstraintAtom",
	&hkConstraintAtomClass, // parent
	sizeof(hkSetLocalTranslationsConstraintAtom),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkSetLocalTranslationsConstraintAtomClass_Members),
	int(sizeof(hkSetLocalTranslationsConstraintAtomClass_Members)/sizeof(hkInternalClassMember)),
	HK_NULL // defaults
	);

//
// Class hkSetLocalRotationsConstraintAtom
//
static const hkInternalClassMember hkSetLocalRotationsConstraintAtomClass_Members[] =
{
	{ "rotationA", HK_NULL, HK_NULL, hkClassMember::TYPE_ROTATION, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkSetLocalRotationsConstraintAtom,m_rotationA) },
	{ "rotationB", HK_NULL, HK_NULL, hkClassMember::TYPE_ROTATION, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkSetLocalRotationsConstraintAtom,m_rotationB) }
};

extern const hkClass hkSetLocalRotationsConstraintAtomClass;
const hkClass hkSetLocalRotationsConstraintAtomClass(
	"hkSetLocalRotationsConstraintAtom",
	&hkConstraintAtomClass, // parent
	sizeof(hkSetLocalRotationsConstraintAtom),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkSetLocalRotationsConstraintAtomClass_Members),
	int(sizeof(hkSetLocalRotationsConstraintAtomClass_Members)/sizeof(hkInternalClassMember)),
	HK_NULL // defaults
	);

//
// Class hkOverwritePivotConstraintAtom
//
static const hkInternalClassMember hkOverwritePivotConstraintAtomClass_Members[] =
{
	{ "copyToPivotBFromPivotA", HK_NULL, HK_NULL, hkClassMember::TYPE_UINT8, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkOverwritePivotConstraintAtom,m_copyToPivotBFromPivotA) }
};

extern const hkClass hkOverwritePivotConstraintAtomClass;
const hkClass hkOverwritePivotConstraintAtomClass(
	"hkOverwritePivotConstraintAtom",
	&hkConstraintAtomClass, // parent
	sizeof(hkOverwritePivotConstraintAtom),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkOverwritePivotConstraintAtomClass_Members),
	int(sizeof(hkOverwritePivotConstraintAtomClass_Members)/sizeof(hkInternalClassMember)),
	HK_NULL // defaults
	);

//
// Class hkLinConstraintAtom
//
static const hkInternalClassMember hkLinConstraintAtomClass_Members[] =
{
	{ "axisIndex", HK_NULL, HK_NULL, hkClassMember::TYPE_UINT8, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkLinConstraintAtom,m_axisIndex) }
};

extern const hkClass hkLinConstraintAtomClass;
const hkClass hkLinConstraintAtomClass(
	"hkLinConstraintAtom",
	&hkConstraintAtomClass, // parent
	sizeof(hkLinConstraintAtom),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkLinConstraintAtomClass_Members),
	int(sizeof(hkLinConstraintAtomClass_Members)/sizeof(hkInternalClassMember)),
	HK_NULL // defaults
	);

//
// Class hkLinSoftConstraintAtom
//
static const hkInternalClassMember hkLinSoftConstraintAtomClass_Members[] =
{
	{ "axisIndex", HK_NULL, HK_NULL, hkClassMember::TYPE_UINT8, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkLinSoftConstraintAtom,m_axisIndex) },
	{ "tau", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkLinSoftConstraintAtom,m_tau) },
	{ "damping", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkLinSoftConstraintAtom,m_damping) }
};

extern const hkClass hkLinSoftConstraintAtomClass;
const hkClass hkLinSoftConstraintAtomClass(
	"hkLinSoftConstraintAtom",
	&hkConstraintAtomClass, // parent
	sizeof(hkLinSoftConstraintAtom),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkLinSoftConstraintAtomClass_Members),
	int(sizeof(hkLinSoftConstraintAtomClass_Members)/sizeof(hkInternalClassMember)),
	HK_NULL // defaults
	);

//
// Class hkLinLimitConstraintAtom
//
static const hkInternalClassMember hkLinLimitConstraintAtomClass_Members[] =
{
	{ "axisIndex", HK_NULL, HK_NULL, hkClassMember::TYPE_UINT8, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkLinLimitConstraintAtom,m_axisIndex) },
	{ "min", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkLinLimitConstraintAtom,m_min) },
	{ "max", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkLinLimitConstraintAtom,m_max) }
};

extern const hkClass hkLinLimitConstraintAtomClass;
const hkClass hkLinLimitConstraintAtomClass(
	"hkLinLimitConstraintAtom",
	&hkConstraintAtomClass, // parent
	sizeof(hkLinLimitConstraintAtom),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkLinLimitConstraintAtomClass_Members),
	int(sizeof(hkLinLimitConstraintAtomClass_Members)/sizeof(hkInternalClassMember)),
	HK_NULL // defaults
	);

//
// Class hk2dAngConstraintAtom
//
static const hkInternalClassMember hk2dAngConstraintAtomClass_Members[] =
{
	{ "freeRotationAxis", HK_NULL, HK_NULL, hkClassMember::TYPE_UINT8, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hk2dAngConstraintAtom,m_freeRotationAxis) }
};

extern const hkClass hk2dAngConstraintAtomClass;
const hkClass hk2dAngConstraintAtomClass(
	"hk2dAngConstraintAtom",
	&hkConstraintAtomClass, // parent
	sizeof(hk2dAngConstraintAtom),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hk2dAngConstraintAtomClass_Members),
	int(sizeof(hk2dAngConstraintAtomClass_Members)/sizeof(hkInternalClassMember)),
	HK_NULL // defaults
	);

//
// Class hkAngConstraintAtom
//
static const hkInternalClassMember hkAngConstraintAtomClass_Members[] =
{
	{ "firstConstrainedAxis", HK_NULL, HK_NULL, hkClassMember::TYPE_UINT8, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkAngConstraintAtom,m_firstConstrainedAxis) },
	{ "numConstrainedAxes", HK_NULL, HK_NULL, hkClassMember::TYPE_UINT8, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkAngConstraintAtom,m_numConstrainedAxes) }
};

extern const hkClass hkAngConstraintAtomClass;
const hkClass hkAngConstraintAtomClass(
	"hkAngConstraintAtom",
	&hkConstraintAtomClass, // parent
	sizeof(hkAngConstraintAtom),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkAngConstraintAtomClass_Members),
	int(sizeof(hkAngConstraintAtomClass_Members)/sizeof(hkInternalClassMember)),
	HK_NULL // defaults
	);

//
// Class hkAngLimitConstraintAtom
//
static const hkInternalClassMember hkAngLimitConstraintAtomClass_Members[] =
{
	{ "isEnabled", HK_NULL, HK_NULL, hkClassMember::TYPE_UINT8, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkAngLimitConstraintAtom,m_isEnabled) },
	{ "limitAxis", HK_NULL, HK_NULL, hkClassMember::TYPE_UINT8, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkAngLimitConstraintAtom,m_limitAxis) },
	{ "minAngle", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkAngLimitConstraintAtom,m_minAngle) },
	{ "maxAngle", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkAngLimitConstraintAtom,m_maxAngle) },
	{ "angularLimitsTauFactor", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkAngLimitConstraintAtom,m_angularLimitsTauFactor) }
};
namespace
{
	struct hkAngLimitConstraintAtom_DefaultStruct
	{
		int s_defaultOffsets[5];
		typedef hkInt8 _hkBool;
		typedef hkReal _hkVector4[4];
		typedef hkReal _hkQuaternion[4];
		typedef hkReal _hkMatrix3[12];
		typedef hkReal _hkRotation[12];
		typedef hkReal _hkQsTransform[12];
		typedef hkReal _hkMatrix4[16];
		typedef hkReal _hkTransform[16];
		hkReal m_angularLimitsTauFactor;
	};
	const hkAngLimitConstraintAtom_DefaultStruct hkAngLimitConstraintAtom_Default =
	{
		{-1,-1,-1,-1,HK_OFFSET_OF(hkAngLimitConstraintAtom_DefaultStruct,m_angularLimitsTauFactor)},
		1.0
	};
}

extern const hkClass hkAngLimitConstraintAtomClass;
const hkClass hkAngLimitConstraintAtomClass(
	"hkAngLimitConstraintAtom",
	&hkConstraintAtomClass, // parent
	sizeof(hkAngLimitConstraintAtom),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkAngLimitConstraintAtomClass_Members),
	int(sizeof(hkAngLimitConstraintAtomClass_Members)/sizeof(hkInternalClassMember)),
	&hkAngLimitConstraintAtom_Default
	);

//
// Class hkTwistLimitConstraintAtom
//
static const hkInternalClassMember hkTwistLimitConstraintAtomClass_Members[] =
{
	{ "isEnabled", HK_NULL, HK_NULL, hkClassMember::TYPE_UINT8, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkTwistLimitConstraintAtom,m_isEnabled) },
	{ "twistAxis", HK_NULL, HK_NULL, hkClassMember::TYPE_UINT8, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkTwistLimitConstraintAtom,m_twistAxis) },
	{ "refAxis", HK_NULL, HK_NULL, hkClassMember::TYPE_UINT8, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkTwistLimitConstraintAtom,m_refAxis) },
	{ "minAngle", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkTwistLimitConstraintAtom,m_minAngle) },
	{ "maxAngle", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkTwistLimitConstraintAtom,m_maxAngle) },
	{ "angularLimitsTauFactor", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkTwistLimitConstraintAtom,m_angularLimitsTauFactor) }
};
namespace
{
	struct hkTwistLimitConstraintAtom_DefaultStruct
	{
		int s_defaultOffsets[6];
		typedef hkInt8 _hkBool;
		typedef hkReal _hkVector4[4];
		typedef hkReal _hkQuaternion[4];
		typedef hkReal _hkMatrix3[12];
		typedef hkReal _hkRotation[12];
		typedef hkReal _hkQsTransform[12];
		typedef hkReal _hkMatrix4[16];
		typedef hkReal _hkTransform[16];
		hkReal m_angularLimitsTauFactor;
	};
	const hkTwistLimitConstraintAtom_DefaultStruct hkTwistLimitConstraintAtom_Default =
	{
		{-1,-1,-1,-1,-1,HK_OFFSET_OF(hkTwistLimitConstraintAtom_DefaultStruct,m_angularLimitsTauFactor)},
		1.0
	};
}

extern const hkClass hkTwistLimitConstraintAtomClass;
const hkClass hkTwistLimitConstraintAtomClass(
	"hkTwistLimitConstraintAtom",
	&hkConstraintAtomClass, // parent
	sizeof(hkTwistLimitConstraintAtom),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkTwistLimitConstraintAtomClass_Members),
	int(sizeof(hkTwistLimitConstraintAtomClass_Members)/sizeof(hkInternalClassMember)),
	&hkTwistLimitConstraintAtom_Default
	);

//
// Enum hkConeLimitConstraintAtom::MeasurementMode
//
static const hkInternalClassEnumItem hkConeLimitConstraintAtomMeasurementModeEnumItems[] =
{
	{0, "ZERO_WHEN_VECTORS_ALIGNED"},
	{1, "ZERO_WHEN_VECTORS_PERPENDICULAR"},
};
static const hkInternalClassEnum hkConeLimitConstraintAtomEnums[] = {
	{"MeasurementMode", hkConeLimitConstraintAtomMeasurementModeEnumItems, 2 }
};
extern const hkClassEnum* hkConeLimitConstraintAtomMeasurementModeEnum = reinterpret_cast<const hkClassEnum*>(&hkConeLimitConstraintAtomEnums[0]);

//
// Class hkConeLimitConstraintAtom
//
static const hkInternalClassMember hkConeLimitConstraintAtomClass_Members[] =
{
	{ "isEnabled", HK_NULL, HK_NULL, hkClassMember::TYPE_UINT8, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkConeLimitConstraintAtom,m_isEnabled) },
	{ "twistAxisInA", HK_NULL, HK_NULL, hkClassMember::TYPE_UINT8, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkConeLimitConstraintAtom,m_twistAxisInA) },
	{ "refAxisInB", HK_NULL, HK_NULL, hkClassMember::TYPE_UINT8, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkConeLimitConstraintAtom,m_refAxisInB) },
	{ "angleMeasurementMode", HK_NULL, hkConeLimitConstraintAtomMeasurementModeEnum, hkClassMember::TYPE_ENUM, hkClassMember::TYPE_VOID, 0, hkClassMember::ENUM_8, HK_OFFSET_OF(hkConeLimitConstraintAtom,m_angleMeasurementMode) },
	{ "minAngle", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkConeLimitConstraintAtom,m_minAngle) },
	{ "maxAngle", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkConeLimitConstraintAtom,m_maxAngle) },
	{ "angularLimitsTauFactor", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkConeLimitConstraintAtom,m_angularLimitsTauFactor) }
};
namespace
{
	struct hkConeLimitConstraintAtom_DefaultStruct
	{
		int s_defaultOffsets[7];
		typedef hkInt8 _hkBool;
		typedef hkReal _hkVector4[4];
		typedef hkReal _hkQuaternion[4];
		typedef hkReal _hkMatrix3[12];
		typedef hkReal _hkRotation[12];
		typedef hkReal _hkQsTransform[12];
		typedef hkReal _hkMatrix4[16];
		typedef hkReal _hkTransform[16];
		hkReal m_angularLimitsTauFactor;
	};
	const hkConeLimitConstraintAtom_DefaultStruct hkConeLimitConstraintAtom_Default =
	{
		{-1,-1,-1,-1,-1,-1,HK_OFFSET_OF(hkConeLimitConstraintAtom_DefaultStruct,m_angularLimitsTauFactor)},
		1.0
	};
}

extern const hkClass hkConeLimitConstraintAtomClass;
const hkClass hkConeLimitConstraintAtomClass(
	"hkConeLimitConstraintAtom",
	&hkConstraintAtomClass, // parent
	sizeof(hkConeLimitConstraintAtom),
	HK_NULL,
	0, // interfaces
	reinterpret_cast<const hkClassEnum*>(hkConeLimitConstraintAtomEnums),
	1, // enums
	reinterpret_cast<const hkClassMember*>(hkConeLimitConstraintAtomClass_Members),
	int(sizeof(hkConeLimitConstraintAtomClass_Members)/sizeof(hkInternalClassMember)),
	&hkConeLimitConstraintAtom_Default
	);

//
// Class hkAngFrictionConstraintAtom
//
static const hkInternalClassMember hkAngFrictionConstraintAtomClass_Members[] =
{
	{ "isEnabled", HK_NULL, HK_NULL, hkClassMember::TYPE_UINT8, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkAngFrictionConstraintAtom,m_isEnabled) },
	{ "firstFrictionAxis", HK_NULL, HK_NULL, hkClassMember::TYPE_UINT8, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkAngFrictionConstraintAtom,m_firstFrictionAxis) },
	{ "numFrictionAxes", HK_NULL, HK_NULL, hkClassMember::TYPE_UINT8, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkAngFrictionConstraintAtom,m_numFrictionAxes) },
	{ "maxFrictionTorque", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkAngFrictionConstraintAtom,m_maxFrictionTorque) }
};

extern const hkClass hkAngFrictionConstraintAtomClass;
const hkClass hkAngFrictionConstraintAtomClass(
	"hkAngFrictionConstraintAtom",
	&hkConstraintAtomClass, // parent
	sizeof(hkAngFrictionConstraintAtom),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkAngFrictionConstraintAtomClass_Members),
	int(sizeof(hkAngFrictionConstraintAtomClass_Members)/sizeof(hkInternalClassMember)),
	HK_NULL // defaults
	);

//
// Class hkAngMotorConstraintAtom
//
static const hkInternalClassMember hkAngMotorConstraintAtomClass_Members[] =
{
	{ "isEnabled", HK_NULL, HK_NULL, hkClassMember::TYPE_BOOL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkAngMotorConstraintAtom,m_isEnabled) },
	{ "motorAxis", HK_NULL, HK_NULL, hkClassMember::TYPE_UINT8, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkAngMotorConstraintAtom,m_motorAxis) },
	{ "initializedOffset", HK_NULL, HK_NULL, hkClassMember::TYPE_INT16, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkAngMotorConstraintAtom,m_initializedOffset) },
	{ "previousTargetAngleOffset", HK_NULL, HK_NULL, hkClassMember::TYPE_INT16, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkAngMotorConstraintAtom,m_previousTargetAngleOffset) },
	{ "correspondingAngLimitSolverResultOffset", HK_NULL, HK_NULL, hkClassMember::TYPE_INT16, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkAngMotorConstraintAtom,m_correspondingAngLimitSolverResultOffset) },
	{ "targetAngle", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkAngMotorConstraintAtom,m_targetAngle) },
	{ "motor", &hkConstraintMotorClass, HK_NULL, hkClassMember::TYPE_POINTER, hkClassMember::TYPE_STRUCT, 0, 0, HK_OFFSET_OF(hkAngMotorConstraintAtom,m_motor) }
};

extern const hkClass hkAngMotorConstraintAtomClass;
const hkClass hkAngMotorConstraintAtomClass(
	"hkAngMotorConstraintAtom",
	&hkConstraintAtomClass, // parent
	sizeof(hkAngMotorConstraintAtom),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkAngMotorConstraintAtomClass_Members),
	int(sizeof(hkAngMotorConstraintAtomClass_Members)/sizeof(hkInternalClassMember)),
	HK_NULL // defaults
	);

//
// Class hkRagdollMotorConstraintAtom
//
static const hkInternalClassMember hkRagdollMotorConstraintAtomClass_Members[] =
{
	{ "isEnabled", HK_NULL, HK_NULL, hkClassMember::TYPE_BOOL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkRagdollMotorConstraintAtom,m_isEnabled) },
	{ "initializedOffset", HK_NULL, HK_NULL, hkClassMember::TYPE_INT16, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkRagdollMotorConstraintAtom,m_initializedOffset) },
	{ "previousTargetAnglesOffset", HK_NULL, HK_NULL, hkClassMember::TYPE_INT16, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkRagdollMotorConstraintAtom,m_previousTargetAnglesOffset) },
	{ "targetFrameAinB", HK_NULL, HK_NULL, hkClassMember::TYPE_MATRIX3, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkRagdollMotorConstraintAtom,m_targetFrameAinB) },
	{ "motors", &hkConstraintMotorClass, HK_NULL, hkClassMember::TYPE_POINTER, hkClassMember::TYPE_STRUCT, 3, 0, HK_OFFSET_OF(hkRagdollMotorConstraintAtom,m_motors) }
};

extern const hkClass hkRagdollMotorConstraintAtomClass;
const hkClass hkRagdollMotorConstraintAtomClass(
	"hkRagdollMotorConstraintAtom",
	&hkConstraintAtomClass, // parent
	sizeof(hkRagdollMotorConstraintAtom),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkRagdollMotorConstraintAtomClass_Members),
	int(sizeof(hkRagdollMotorConstraintAtomClass_Members)/sizeof(hkInternalClassMember)),
	HK_NULL // defaults
	);

//
// Class hkLinFrictionConstraintAtom
//
static const hkInternalClassMember hkLinFrictionConstraintAtomClass_Members[] =
{
	{ "isEnabled", HK_NULL, HK_NULL, hkClassMember::TYPE_UINT8, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkLinFrictionConstraintAtom,m_isEnabled) },
	{ "frictionAxis", HK_NULL, HK_NULL, hkClassMember::TYPE_UINT8, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkLinFrictionConstraintAtom,m_frictionAxis) },
	{ "maxFrictionForce", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkLinFrictionConstraintAtom,m_maxFrictionForce) }
};

extern const hkClass hkLinFrictionConstraintAtomClass;
const hkClass hkLinFrictionConstraintAtomClass(
	"hkLinFrictionConstraintAtom",
	&hkConstraintAtomClass, // parent
	sizeof(hkLinFrictionConstraintAtom),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkLinFrictionConstraintAtomClass_Members),
	int(sizeof(hkLinFrictionConstraintAtomClass_Members)/sizeof(hkInternalClassMember)),
	HK_NULL // defaults
	);

//
// Class hkLinMotorConstraintAtom
//
static const hkInternalClassMember hkLinMotorConstraintAtomClass_Members[] =
{
	{ "isEnabled", HK_NULL, HK_NULL, hkClassMember::TYPE_BOOL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkLinMotorConstraintAtom,m_isEnabled) },
	{ "motorAxis", HK_NULL, HK_NULL, hkClassMember::TYPE_UINT8, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkLinMotorConstraintAtom,m_motorAxis) },
	{ "initializedOffset", HK_NULL, HK_NULL, hkClassMember::TYPE_INT16, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkLinMotorConstraintAtom,m_initializedOffset) },
	{ "previousTargetPositionOffset", HK_NULL, HK_NULL, hkClassMember::TYPE_INT16, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkLinMotorConstraintAtom,m_previousTargetPositionOffset) },
	{ "targetPosition", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkLinMotorConstraintAtom,m_targetPosition) },
	{ "motor", &hkConstraintMotorClass, HK_NULL, hkClassMember::TYPE_POINTER, hkClassMember::TYPE_STRUCT, 0, 0, HK_OFFSET_OF(hkLinMotorConstraintAtom,m_motor) }
};

extern const hkClass hkLinMotorConstraintAtomClass;
const hkClass hkLinMotorConstraintAtomClass(
	"hkLinMotorConstraintAtom",
	&hkConstraintAtomClass, // parent
	sizeof(hkLinMotorConstraintAtom),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkLinMotorConstraintAtomClass_Members),
	int(sizeof(hkLinMotorConstraintAtomClass_Members)/sizeof(hkInternalClassMember)),
	HK_NULL // defaults
	);

//
// Class hkPulleyConstraintAtom
//
static const hkInternalClassMember hkPulleyConstraintAtomClass_Members[] =
{
	{ "fixedPivotAinWorld", HK_NULL, HK_NULL, hkClassMember::TYPE_VECTOR4, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkPulleyConstraintAtom,m_fixedPivotAinWorld) },
	{ "fixedPivotBinWorld", HK_NULL, HK_NULL, hkClassMember::TYPE_VECTOR4, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkPulleyConstraintAtom,m_fixedPivotBinWorld) },
	{ "ropeLength", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkPulleyConstraintAtom,m_ropeLength) },
	{ "leverageOnBodyB", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkPulleyConstraintAtom,m_leverageOnBodyB) }
};

extern const hkClass hkPulleyConstraintAtomClass;
const hkClass hkPulleyConstraintAtomClass(
	"hkPulleyConstraintAtom",
	&hkConstraintAtomClass, // parent
	sizeof(hkPulleyConstraintAtom),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkPulleyConstraintAtomClass_Members),
	int(sizeof(hkPulleyConstraintAtomClass_Members)/sizeof(hkInternalClassMember)),
	HK_NULL // defaults
	);

//
// Class hkModifierConstraintAtom
//
static const hkInternalClassMember hkModifierConstraintAtomClass_Members[] =
{
	{ "modifierAtomSize", HK_NULL, HK_NULL, hkClassMember::TYPE_UINT16, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkModifierConstraintAtom,m_modifierAtomSize) },
	{ "childSize", HK_NULL, HK_NULL, hkClassMember::TYPE_UINT16, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkModifierConstraintAtom,m_childSize) },
	{ "child", &hkConstraintAtomClass, HK_NULL, hkClassMember::TYPE_POINTER, hkClassMember::TYPE_STRUCT, 0, 0, HK_OFFSET_OF(hkModifierConstraintAtom,m_child) }
};

extern const hkClass hkModifierConstraintAtomClass;
const hkClass hkModifierConstraintAtomClass(
	"hkModifierConstraintAtom",
	&hkConstraintAtomClass, // parent
	sizeof(hkModifierConstraintAtom),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkModifierConstraintAtomClass_Members),
	int(sizeof(hkModifierConstraintAtomClass_Members)/sizeof(hkInternalClassMember)),
	HK_NULL // defaults
	);

//
// Class hkSoftContactModifierConstraintAtom
//
static const hkInternalClassMember hkSoftContactModifierConstraintAtomClass_Members[] =
{
	{ "tau", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkSoftContactModifierConstraintAtom,m_tau) },
	{ "maxAcceleration", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkSoftContactModifierConstraintAtom,m_maxAcceleration) }
};

extern const hkClass hkSoftContactModifierConstraintAtomClass;
const hkClass hkSoftContactModifierConstraintAtomClass(
	"hkSoftContactModifierConstraintAtom",
	&hkModifierConstraintAtomClass, // parent
	sizeof(hkSoftContactModifierConstraintAtom),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkSoftContactModifierConstraintAtomClass_Members),
	int(sizeof(hkSoftContactModifierConstraintAtomClass_Members)/sizeof(hkInternalClassMember)),
	HK_NULL // defaults
	);

//
// Class hkMassChangerModifierConstraintAtom
//
static const hkInternalClassMember hkMassChangerModifierConstraintAtomClass_Members[] =
{
	{ "factorA", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkMassChangerModifierConstraintAtom,m_factorA) },
	{ "factorB", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkMassChangerModifierConstraintAtom,m_factorB) }
};

extern const hkClass hkMassChangerModifierConstraintAtomClass;
const hkClass hkMassChangerModifierConstraintAtomClass(
	"hkMassChangerModifierConstraintAtom",
	&hkModifierConstraintAtomClass, // parent
	sizeof(hkMassChangerModifierConstraintAtom),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkMassChangerModifierConstraintAtomClass_Members),
	int(sizeof(hkMassChangerModifierConstraintAtomClass_Members)/sizeof(hkInternalClassMember)),
	HK_NULL // defaults
	);

//
// Class hkViscousSurfaceModifierConstraintAtom
//

extern const hkClass hkViscousSurfaceModifierConstraintAtomClass;
const hkClass hkViscousSurfaceModifierConstraintAtomClass(
	"hkViscousSurfaceModifierConstraintAtom",
	&hkModifierConstraintAtomClass, // parent
	sizeof(hkViscousSurfaceModifierConstraintAtom),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	HK_NULL,
	0,
	HK_NULL // defaults
	);

//
// Class hkMovingSurfaceModifierConstraintAtom
//
static const hkInternalClassMember hkMovingSurfaceModifierConstraintAtomClass_Members[] =
{
	{ "velocity", HK_NULL, HK_NULL, hkClassMember::TYPE_VECTOR4, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(hkMovingSurfaceModifierConstraintAtom,m_velocity) }
};

extern const hkClass hkMovingSurfaceModifierConstraintAtomClass;
const hkClass hkMovingSurfaceModifierConstraintAtomClass(
	"hkMovingSurfaceModifierConstraintAtom",
	&hkModifierConstraintAtomClass, // parent
	sizeof(hkMovingSurfaceModifierConstraintAtom),
	HK_NULL,
	0, // interfaces
	HK_NULL,
	0, // enums
	reinterpret_cast<const hkClassMember*>(hkMovingSurfaceModifierConstraintAtomClass_Members),
	int(sizeof(hkMovingSurfaceModifierConstraintAtomClass_Members)/sizeof(hkInternalClassMember)),
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
