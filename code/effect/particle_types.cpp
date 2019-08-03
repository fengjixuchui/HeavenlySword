//--------------------------------------------------
//!
//!	\file particle_types.cpp
//!	Definitions for particles types exposed in welder
//!
//--------------------------------------------------

#include "particle_types.h"
#include "camera/camutils.h"
#include "effect_util.h"
#include "objectdatabase/dataobject.h"

START_STD_INTERFACE( ParticleDef_Rotating )

	I2FLOAT		( m_fRotationStartMin,	StartRotationMin(deg) )
	I2FLOAT		( m_fRotationStartMax,	StartRotationMax(deg) )
	I2FLOAT		( m_fRotationVelMin,	RotationVelocityMin(deg/s) )
	I2FLOAT		( m_fRotationVelMax,	RotationVelocityMax(deg/s) )
	I2FLOAT		( m_fRotationAcc,		RotationAcceleration(deg/s/s) )
	I2FLOAT		( m_fVelocitySidedness,	VelocitySidedness(0--1) )
	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
	DECLARE_EDITORCHANGEVALUE_CALLBACK( EditorChangeValue )
	DECLARE_EDITORCHANGEPARENT_CALLBACK( EditorChangeParent )
END_STD_INTERFACE

START_STD_INTERFACE( ParticleDef_WorldQuad )
	I2VECTOR	( m_YPR, YawPitchRoll(deg) )
	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
	DECLARE_EDITORCHANGEVALUE_CALLBACK( EditorChangeValue )
	DECLARE_EDITORCHANGEPARENT_CALLBACK( EditorChangeParent )
END_STD_INTERFACE

START_STD_INTERFACE( ParticleDef_AxisQuad )
	I2VECTOR	( m_dir, RotationAxis )
	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
	DECLARE_EDITORCHANGEVALUE_CALLBACK( EditorChangeValue )
	DECLARE_EDITORCHANGEPARENT_CALLBACK( EditorChangeParent )
END_STD_INTERFACE

START_STD_INTERFACE( ParticleDef_VelScaledRay )
	I2BOOL	( m_bFixedTime, UseFixedTimeInterval )
	I2FLOAT	( m_fXScale, XScaleFactor )
	I2FLOAT	( m_fYScale, YScaleFactor )
END_STD_INTERFACE

void ForceLinkFunction14()
{
	ntPrintf("!ATTN! Calling ForceLinkFunction14() !ATTN!\n");
}

//--------------------------------------------------
//!
//!	ParticleDef_Rotating::ParticleDef_Rotating
//!
//--------------------------------------------------
ParticleDef_Rotating::ParticleDef_Rotating() :
	m_fRotationStartMin(0.0f),
	m_fRotationStartMax(0.0f),
	m_fRotationVelMin(0.0f),
	m_fRotationVelMax(0.0f),
	m_fRotationAcc(0.0f),
	m_fVelocitySidedness(0.5f)
{
}

void ParticleDef_Rotating::PostConstruct()
{
	m_fRotationStartMinRad =	m_fRotationStartMin * DEG_TO_RAD_VALUE;
	m_fRotationStartMaxRad =	m_fRotationStartMax * DEG_TO_RAD_VALUE;
	m_fRotationVelMinRad =		m_fRotationVelMin * DEG_TO_RAD_VALUE;
	m_fRotationVelMaxRad =		m_fRotationVelMax * DEG_TO_RAD_VALUE;
	m_fRotationAccRad =			m_fRotationAcc * DEG_TO_RAD_VALUE;
	m_fVelocitySidedness = ntstd::Clamp( m_fVelocitySidedness, 0.0f, 1.0f );
}

//--------------------------------------------------
//!
//!	ParticleDef_WorldQuad::ParticleDef_WorldQuad
//!
//--------------------------------------------------
ParticleDef_WorldQuad::ParticleDef_WorldQuad() :
	m_YPR(CONSTRUCT_CLEAR)
{
}

void ParticleDef_WorldQuad::PostConstruct()
{
	CCamUtil::MatrixFromYawPitchRoll(	m_orientation,
										m_YPR.X() * DEG_TO_RAD_VALUE,
										m_YPR.Y() * DEG_TO_RAD_VALUE,
										m_YPR.Z() * DEG_TO_RAD_VALUE );
}

//--------------------------------------------------
//!
//!	ParticleDef_AxisQuad::ParticleDef_AxisQuad
//!
//--------------------------------------------------
ParticleDef_AxisQuad::ParticleDef_AxisQuad() :
	m_dir( 0.0f, 1.0f, 0.0f ),
	m_directionN( 0.0f, 1.0f, 0.0f )
{
}

//--------------------------------------------------
//!
//!	ParticleDef_VelScaledRay::ParticleDef_VelScaledRay
//!
//--------------------------------------------------
ParticleDef_VelScaledRay::ParticleDef_VelScaledRay() :
	m_bFixedTime(true),
	m_fXScale(0.0f),
	m_fYScale(1.0f)
{
}

