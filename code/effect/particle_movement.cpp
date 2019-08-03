//--------------------------------------------------
//!
//!	\file particle_movment.cpp
//!	XML movement interface implementations
//!
//--------------------------------------------------

#include "particle_movement.h"
#include "functioncurve.h"
#include "objectdatabase/dataobject.h"

START_STD_INTERFACE( Particle_AdvancedMovement )

	I2INT		( m_resources.m_iNumSamples,		TableResolution )
	I2REFERENCE	( m_resources.m_pVelScale,			VelocityScaleCurve )
	I2REFERENCE	( m_resources.m_pRocketAccMin,		RocketAccMin )
	I2REFERENCE	( m_resources.m_pRocketAccMax,		RocketAccMax )
	I2REFERENCE	( m_resources.m_pAccelerationX,		AccelerationX )
	I2REFERENCE	( m_resources.m_pAccelerationY,		AccelerationY )
	I2REFERENCE	( m_resources.m_pAccelerationZ,		AccelerationZ )
	I2REFERENCE	( m_resources.m_pYaw,				SteerYaw )
	I2REFERENCE	( m_resources.m_pPitch,				SteerPitch )

	PUBLISH_PTR_CONTAINER_AS( m_obObjects, Objects )

	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
	DECLARE_EDITORCHANGEVALUE_CALLBACK( EditorChangeValue )
	DECLARE_EDITORCHANGEPARENT_CALLBACK( EditorChangeParent )
END_STD_INTERFACE

void ForceLinkFunction13()
{
	ntPrintf("!ATTN! Calling ForceLinkFunction13() !ATTN!\n");
}

//--------------------------------------------------
//!
//!	ParticleAdvMoveResources::ctor
//!
//--------------------------------------------------
ParticleAdvMoveResources::ParticleAdvMoveResources() :
	m_iNumSamples(256),
	m_pVelScale(0),
	m_pRocketAccMin(0),
	m_pRocketAccMax(0),
	m_pAccelerationX(0),
	m_pAccelerationY(0),
	m_pAccelerationZ(0),
	m_pYaw(0),
	m_pPitch(0)
{
	EffectResourceMan::Get().RegisterResource( *this );
}

ParticleAdvMoveResources::~ParticleAdvMoveResources()
{
	EffectResourceMan::Get().ReleaseResource( *this );
}

//--------------------------------------------------
//!
//!	ParticleAdvMoveResources::GenerateResources
//!
//--------------------------------------------------
void ParticleAdvMoveResources::GenerateResources()
{
	m_functions.Reset( FunctionObject::FTT_FLOAT_SPARSE, m_iNumSamples );

	// NB, functions must be added in same order as ADV_MOVE_FUNC_ID
	// btw, these functions can be null, as we're using a sparse table
	m_functions.AddFunction( m_pVelScale );			// VELOCITY_SCALING

	m_functions.AddFunction( m_pRocketAccMin );		// ROCKET_ACCELERATION_MIN
	m_functions.AddFunction( m_pRocketAccMax );		// ROCKET_ACCELERATION_MAX

	m_functions.AddFunction( m_pAccelerationX );	// WORLD_ACCELERATION_X
	m_functions.AddFunction( m_pAccelerationY );	// WORLD_ACCELERATION_Y
	m_functions.AddFunction( m_pAccelerationZ );	// WORLD_ACCELERATION_Z
	
	m_functions.AddFunction( m_pYaw );				// ROCKET_YAW
	m_functions.AddFunction( m_pPitch );			// ROCKET_PITCH

	m_functions.FlushCreation();

	ResourcesOutOfDate(); // this flushes any erronious refresh detects
	m_bRequireRefresh = false;
}

//--------------------------------------------------
//!
//!	ParticleAdvMoveResources::ResourcesOutOfDate
//! per frame callback to see if we need regenerating
//!
//--------------------------------------------------
bool ParticleAdvMoveResources::ResourcesOutOfDate() const
{
	// we intentionally check all of these
	if ((m_pVelScale) && (m_pVelScale->DetectCurveChanged()))			m_bRequireRefresh = true;
	if ((m_pRocketAccMin) && (m_pRocketAccMin->DetectCurveChanged()))	m_bRequireRefresh = true;
	if ((m_pRocketAccMax) && (m_pRocketAccMax->DetectCurveChanged()))	m_bRequireRefresh = true;
	if ((m_pAccelerationX) && (m_pAccelerationX->DetectCurveChanged())) m_bRequireRefresh = true;
	if ((m_pAccelerationY) && (m_pAccelerationY->DetectCurveChanged())) m_bRequireRefresh = true;
	if ((m_pAccelerationZ) && (m_pAccelerationZ->DetectCurveChanged())) m_bRequireRefresh = true;
	if ((m_pYaw) && (m_pYaw->DetectCurveChanged()))						m_bRequireRefresh = true;
	if ((m_pPitch) && (m_pPitch->DetectCurveChanged()))					m_bRequireRefresh = true;

	return m_bRequireRefresh;
}





//--------------------------------------------------
//!
//!	fill in any missing fields we have...
//!
//--------------------------------------------------
void Particle_AdvancedMovement::PostConstruct()
{
	m_resources.m_iNumSamples = ntstd::Max( m_resources.m_iNumSamples, 16 );
	m_bUseVelocityScaling = m_resources.m_pVelScale ? true : false;
	m_bUseRocketAcc = (m_resources.m_pRocketAccMin || m_resources.m_pRocketAccMax) ? true : false;
	m_bUseWorldAcc  = (m_resources.m_pAccelerationX || m_resources.m_pAccelerationY || m_resources.m_pAccelerationZ) ? true : false;
	m_bUseSteering = (m_resources.m_pYaw) || (m_resources.m_pPitch);
}

//--------------------------------------------------
//!
//!	refresh our internal state after editing
//!
//--------------------------------------------------
bool Particle_AdvancedMovement::EditorChangeValue( CallBackParameter param, CallBackParameter )
{
	CHashedString pName(param);

	if (HASH_STRING_VELOCITYSCALECURVE == pName || HASH_STRING_ROCKETACCMIN == pName || HASH_STRING_ROCKETACCMAX == pName ||
		HASH_STRING_ACCELERATIONX == pName || HASH_STRING_ACCELERATIONY == pName || HASH_STRING_ACCELERATIONZ == pName ||
		HASH_STRING_STEERYAW == pName || HASH_STRING_STEERPITCH == pName)
	{
		m_resources.GenerateResources();
	}

	PostConstruct();
	return true;
}



