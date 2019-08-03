
#include "gfx/lenseffects.h"
#include "objectdatabase/dataobject.h"

START_CHUNKED_INTERFACE( CGhostLensEffectSettings, Mem::MC_GFX )

	ICOLOUR( CGhostLensEffectSettings, GhostFinalTint );
	IFLOAT( CGhostLensEffectSettings, GhostScaleSharp );
	IFLOAT( CGhostLensEffectSettings, GhostScaleBlurred );

	ICOLOUR( CGhostLensEffectSettings, GhostModulate[0] );
	IFLOAT( CGhostLensEffectSettings, GhostScale[0] );
	ICOLOUR( CGhostLensEffectSettings, GhostModulate[1] );
	IFLOAT( CGhostLensEffectSettings, GhostScale[1] );
	ICOLOUR( CGhostLensEffectSettings, GhostModulate[2] );
	IFLOAT( CGhostLensEffectSettings, GhostScale[2] );
	ICOLOUR( CGhostLensEffectSettings, GhostModulate[3] );
	IFLOAT( CGhostLensEffectSettings, GhostScale[3] );
	ICOLOUR( CGhostLensEffectSettings, GhostModulate[4] );
	IFLOAT( CGhostLensEffectSettings, GhostScale[4] );
	ICOLOUR( CGhostLensEffectSettings, GhostModulate[5] );
	IFLOAT( CGhostLensEffectSettings, GhostScale[5] );
	ICOLOUR( CGhostLensEffectSettings, GhostModulate[6] );
	IFLOAT( CGhostLensEffectSettings, GhostScale[6] );
	ICOLOUR( CGhostLensEffectSettings, GhostModulate[7] );
	IFLOAT( CGhostLensEffectSettings, GhostScale[7] );

END_STD_INTERFACE

/***************************************************************************************************
*
*	CLASS			CStaticLensConfig
*
*	DESCRIPTION		static class holding all the hacky lens statics.
*					this lets me seperate the config from the actual effect class (i.e. we can have
*					an alternate XENON path)
*
***************************************************************************************************/
/* these values are not used, LevelReset changes to the correct defaults */
float	CStaticLensConfig::m_fMin = 1.5f;
float	CStaticLensConfig::m_fMax = 10.0f;
float	CStaticLensConfig::m_fEffectPower = 1.0f/2.0f;
int		CStaticLensConfig::m_iGaussianLevels = 7;
float	CStaticLensConfig::m_fMinScalar = 1.0f;
float	CStaticLensConfig::m_fMaxScalar = 1.0f;
float	CStaticLensConfig::m_fEffectPowerScalar = 1.0f;
int		CStaticLensConfig::m_iAdditionalGausian = 0;

/***************************************************************************************************
*
*	FUNCTION		CStaticLensConfig::LevelReset
*
*	DESCRIPTION		Reset lens params
*
***************************************************************************************************/
void CStaticLensConfig::LevelReset()
{
	m_fMin = 1.5f;
	m_fMax = 10.0f;
	m_fEffectPower = 1.0f/2.0f;
	m_iGaussianLevels = 7;
	m_fMinScalar = 1.0f;
	m_fMaxScalar = 1.0f;
	m_fEffectPowerScalar = 1.0f;
	m_iAdditionalGausian = 0;
}



/***************************************************************************************************
*
*	FUNCTION		CGhostLensEffectSettings::CGhostLensEffectSettings
*
*	DESCRIPTION		XML based lens congi
*
***************************************************************************************************/
CGhostLensEffectSettings::CGhostLensEffectSettings() :
	m_fGhostScaleSharp( 3.0f ),
	m_fGhostScaleBlurred( -1.5f ),
	m_obGhostFinalTint( 0.2f, 0.2f, 0.2f, 0 )
{
	static const CVector obStartColour(0.3f, 0.3f, 0.5f, 0 );
	static const CVector obEndColour(1.0f, 1.0f, 1.0f, 0 );
	static const float fStartScale = -1.0f;
	static const float fEndScale = 2.f;
	for( unsigned int iIndex = 0;iIndex < 8;iIndex++ )
	{
		m_fGhostScale[iIndex] = CMaths::Lerp( fStartScale, fEndScale, static_cast<float>(iIndex) / 7.f );
		m_obGhostModulate[iIndex] = CVector::Lerp( obStartColour, obEndColour, static_cast<float>(iIndex) / 7.f );
	}
}





