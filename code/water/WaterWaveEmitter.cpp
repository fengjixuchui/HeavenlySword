//--------------------------------------------------
//!
//!	\file WaveEmitter.cpp
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#include "water/waterwaveemitter.h"
#include "water/waterdmadata.h"
#include "water/waterinstance.h"
#include "water/waterinstancedef.h"
#include "game/randmanager.h"
#include "editable/enumlist.h"
#include "objectdatabase/dataobject.h"

//
// Common
//
static const CPoint		DEF_POSITION		( 0.0f, 0.0f, 0.0f );
static const CDirection	DEF_DIRECTION		( 0.0f, 0.0f, 1.0f );
static const float		DEF_AMPLITUDE		= 0.1f;
static const float		DEF_WAVELENGTH		= 2.0f;
static const float		DEF_MAX_AGE			= 20.0f;
static const float		DEF_EMISSIONRATE	= 30.0f;
static const float		DEF_RANDOMNESS		= 0.01f;
static const float		DEF_SPEED			= 0.2f;
static const float		DEF_ATT_L			= 0.0f;
static const float		DEF_ATT_Q			= 0.0f;
static const float		DEF_SHARPNESS		= 0.1f;
static const float		DEF_FADE_IN			= 0.0f;
static const float		DEF_FADE_OUT		= 0.0f;

//
// Attack
//
static const float		DEF_WIDTH			= 10.0f;
static const float		DEF_FALLOFF			= 1.0f;
static const float		DEF_TRAIL			= 5.0f;
static const float		DEF_RADIUS			= 7.0f;


inline float Vary( float x, float p )
{
	return x * ( 1.0f - 0.5f * p + erandf( p ) );
}



START_STD_INTERFACE( WaveEmitter )
	PUBLISH_VAR_WITH_DEFAULT_AS	( m_obPos,								DEF_POSITION,			Position )
	PUBLISH_VAR_WITH_DEFAULT_AS	( m_fEmissionRate,						DEF_EMISSIONRATE,		EmissionRate )
	PUBLISH_VAR_WITH_DEFAULT_AS	( m_obAvgWaveDir,						DEF_DIRECTION,			AvgWaveDirection )
	PUBLISH_VAR_WITH_DEFAULT_AS	( m_fAvgWaveSpeed,						DEF_SPEED,				AvgWaveSpeed )
	PUBLISH_VAR_WITH_DEFAULT_AS	( m_fAvgWaveAmplitude,					DEF_AMPLITUDE,			AvgWaveAmplitude )
	PUBLISH_VAR_WITH_DEFAULT_AS	( m_fAvgWavelength,						DEF_WAVELENGTH,			AvgWaveWavelength )
	PUBLISH_VAR_WITH_DEFAULT_AS	( m_fAvgWaveSharpness,					DEF_SHARPNESS,			AvgWaveSharpness )
	PUBLISH_VAR_WITH_DEFAULT_AS	( m_fAvgWaveLifespan,					DEF_MAX_AGE,			AvgWaveLifespan )
	PUBLISH_VAR_WITH_DEFAULT_AS	( m_fRandomness,						DEF_RANDOMNESS,			Randomness )
	PUBLISH_VAR_WITH_DEFAULT_AS	( m_fAttLinear,							DEF_ATT_L,				LinearAttenuation )
	PUBLISH_VAR_WITH_DEFAULT_AS	( m_fAttQuadratic,						DEF_ATT_Q,				QuadraticAttenuation )
	PUBLISH_VAR_WITH_DEFAULT_AS	( m_fFadeInTime,						DEF_FADE_IN,			FadeInTime )
	PUBLISH_VAR_WITH_DEFAULT_AS	( m_fFadeOutTime,						DEF_FADE_OUT,			FadeOutTime )
	PUBLISH_VAR_WITH_DEFAULT_AS	( m_bAutoEmit,							false,					AutoEmit )
	PUBLISH_VAR_WITH_DEFAULT_AS ( m_iPreferredSlot,						-1,						PreferredSlot )
	PUBLISH_GLOBAL_ENUM_WITH_DEFAULT_AS( m_eType, WaveType, WAVE_TYPE, WAVETYPE_CIRCULAR )
	PUBLISH_VAR_WITH_DEFAULT_AS	( m_Attack0_fWidth,						DEF_WIDTH,				Attack0_Width )
	PUBLISH_VAR_WITH_DEFAULT_AS	( m_Attack0_fFalloff,					DEF_FALLOFF,			Attack0_Falloff )
	PUBLISH_VAR_WITH_DEFAULT_AS	( m_Attack1_fBackTrail,					DEF_TRAIL,				Attack1_BackTrail )
	PUBLISH_VAR_WITH_DEFAULT_AS	( m_Attack1_fFrontTrail,				DEF_TRAIL,				Attack1_FrontTrail )
	PUBLISH_VAR_WITH_DEFAULT_AS	( m_Attack2_fRadius,					DEF_RADIUS,				Attack2_Radius )
	DECLARE_EDITORCHANGEVALUE_CALLBACK( EditorChangeValue );
	DECLARE_POSTPOSTCONSTRUCT_CALLBACK( PostPostConstruct );
END_STD_INTERFACE



bool WaveEmitter::Update( float fTimeStep )
{
	bool bOverwrite = false;
	if ( m_bAutoEmit )
	{
		m_fTimeSinceLastEmission += fTimeStep;

		if ( m_fTimeSinceLastEmission > m_fEmissionRate )
		{
			m_fTimeSinceLastEmission = 0.0f;
			WaveDma* pWave = 0;
			if ( m_iPreferredSlot >= 0 && m_iPreferredSlot < static_cast<int>(m_pobWaterInstance->GetNumOfWaveSlots()) )
			{
				pWave = m_pobWaterInstance->GetWaveSlot( m_iPreferredSlot );
				bOverwrite = true;
			}
			else
			{
				pWave = m_pobWaterInstance->GetFirstAvailableWaveSlot();
			}

			user_warn_p( pWave, ("WATER - wave emitter couldn't find a free wave slot. Please decrease emission frequency\n") );
			if ( pWave )
			{
				CreateWave( pWave, bOverwrite );
				return true;
			}
		}
	}

	return false;
}


void WaveEmitter::PostPostConstruct( void )
{
	ClampValues();
}

bool  WaveEmitter::EditorChangeValue(CallBackParameter,CallBackParameter)
{
	PostPostConstruct();
	return true;
}

void WaveEmitter::InstallParentWaterInstance( WaterInstance* pobWaterInstance )
{
	ntAssert_p( pobWaterInstance, ("WATER - WaveEmitter cannot install a NULL waterinstance\n") );
	
	m_pobWaterInstance = pobWaterInstance;
}

bool WaveEmitter::CreateWave( WaveDma* pWave, bool bForceOverwrite )
{
	ntError( pWave );
	ntAssert_p( m_pobWaterInstance, ("WATER - WaveEmitter has no water instance installed\n") );

	// clamp values to our installed definition
	ClampValues();

	float resolution = m_pobWaterInstance->GetDefinition().m_fResolution;

	if ( (pWave->m_iFlags & kWF_Control_Invalid) || bForceOverwrite )
	{	
		//memset( pWave, 0, sizeof(WaveDma) );
		// no negative amplitudes allowed here
		float amplitude = ntstd::Max( Vary( m_fAvgWaveAmplitude, m_fRandomness ), EPSILON );
		// respect Nyquist law please
		float minWavelength = ntstd::Max( 2.0f * resolution, 2.2f * amplitude );
		float wavelength = ntstd::Max( Vary( m_fAvgWavelength, m_fRandomness ), minWavelength);
		// negative sharpness is a no-no too!
		float sharpness = ntstd::Max( Vary( m_fAvgWaveSharpness, m_fRandomness ), EPSILON );
		float speed = Vary( m_fAvgWaveSpeed, m_fRandomness );

		pWave->m_fAmplitude = amplitude;
		pWave->m_fFrequency = TWO_PI / wavelength;
		pWave->m_fPhase = speed * TWO_PI / wavelength;
		pWave->m_fSharpness = sharpness;

		pWave->m_fAge = 0;
		pWave->m_fMaxAge = ntstd::Max( Vary( m_fAvgWaveLifespan, m_fRandomness ), 0.0f );

		pWave->m_fAttLinear = m_fAttLinear;
		pWave->m_fAttQuadratic = m_fAttQuadratic;
		pWave->m_fFadeInTime = ntstd::Min( m_fFadeInTime,  pWave->m_fMaxAge ); 
		pWave->m_fFadeOutTime = ntstd::Min( m_fFadeOutTime, pWave->m_fMaxAge );


		// note that circular waves will overwrite this as their direction is relative to each vertex being evaluated
		// However, we still require all waves to have a normalised  horizontal direction just to keep things consistent
		pWave->m_obDirection = m_obAvgWaveDir * CMatrix( CVecMath::GetYAxis(), (1.0f-Vary(1.0f,m_fRandomness)) * 30.0f * DEG_TO_RAD_VALUE );
		pWave->m_obDirection.Y() = 0.0f;
		pWave->m_obDirection.Normalise();

		pWave->m_obOrigin = m_obPos;
		pWave->m_obOrigin.Y() = 0.0f;

		switch ( m_eType )
		{
			case WAVETYPE_DIRECTIONAL:
				pWave->m_iFlags = kWF_Type_Directional;
				break;
			case WAVETYPE_CIRCULAR:
				pWave->m_iFlags = kWF_Type_Circular;
				break;
			case WAVETYPE_ATTACK0:
				pWave->m_iFlags = kWF_Type_Attack0;
				pWave->m_TypeSpecific.m_Attack0.m_fFalloff = m_Attack0_fFalloff;
				pWave->m_TypeSpecific.m_Attack0.m_fWidth = m_Attack0_fWidth;
				break;
			case WAVETYPE_ATTACK1:
				pWave->m_iFlags = kWF_Type_Attack1;
				pWave->m_TypeSpecific.m_Attack1.m_fBackTrail = m_Attack1_fBackTrail;
				pWave->m_TypeSpecific.m_Attack1.m_fFrontTrail = m_Attack1_fFrontTrail;
				break;
			case WAVETYPE_ATTACK2:
				pWave->m_iFlags = kWF_Type_Attack2;
				pWave->m_TypeSpecific.m_Attack2.m_fRadius = m_Attack2_fRadius;
				break;
		}

		return true;
	}
	else
	{
		user_warn_msg(("Trying to emit onto a valid wave structure\n"));
		return false;
	}
}

void WaveEmitter::ClampValues( void )
{
	if ( m_pobWaterInstance )
	{
		m_obPos.X() = ntstd::Clamp( m_obPos.X(), -0.5f * m_pobWaterInstance->GetDefinition().m_fWidth, 0.5f * m_pobWaterInstance->GetDefinition().m_fWidth );
		m_obPos.Y() = 0;
		m_obPos.Z() = ntstd::Clamp( m_obPos.Z(), -0.5f * m_pobWaterInstance->GetDefinition().m_fLength, 0.5f * m_pobWaterInstance->GetDefinition().m_fLength );

		m_obAvgWaveDir.Y() = 0;
		m_obAvgWaveDir.Normalise();
	}
}	

void WaveEmitter::ClampValues( const WaterInstance* pobWaterInstance )
{
	ntError( pobWaterInstance );

	m_obPos.X() = ntstd::Clamp( m_obPos.X(), -0.5f * pobWaterInstance->GetDefinition().m_fWidth, 0.5f * pobWaterInstance->GetDefinition().m_fWidth );
	m_obPos.Y() = 0;
	m_obPos.Z() = ntstd::Clamp( m_obPos.Z(), -0.5f * pobWaterInstance->GetDefinition().m_fLength, 0.5f * pobWaterInstance->GetDefinition().m_fLength );

	m_obAvgWaveDir.Y() = 0;
	m_obAvgWaveDir.Normalise();
}



//eof


