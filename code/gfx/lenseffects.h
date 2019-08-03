//--------------------------------------------------
//!
//!	\file lenseffects.h
//!	The shared portion of the lenseffect object,
//! include the platform specific bits as well
//!
//--------------------------------------------------

#ifndef GFX_LENSEFFECTS_H
#define GFX_LENSEFFECTS_H

/***************************************************************************************************
*
*	CLASS			CGhostLensEffect
*
*	DESCRIPTION		simple static XML def for the ghost lens effects
*
***************************************************************************************************/
class CGhostLensEffectSettings
{
public:
	CGhostLensEffectSettings( void );

	float m_fGhostScaleSharp;
	float m_fGhostScaleBlurred;
	float m_fGhostScale[8];
	CVector m_obGhostModulate[8];
	CVector m_obGhostFinalTint;
};

/***************************************************************************************************
*
*	CLASS			CStaticLensConfig
*
*	DESCRIPTION		static class holding all the hacky lens statics.
*
***************************************************************************************************/
class CStaticLensConfig
{
public:
	static void LevelReset();

	static void SetParameters( float fMin, float fMax, float fEffectPower, int iGaussianLevels )
	{
		m_fMin = ntstd::Min( fMin, fMax );
		m_fMax = ntstd::Max( fMin, fMax );
		m_fEffectPower = fEffectPower;
		m_iGaussianLevels = iGaussianLevels;
	}
	
	static void GetParametersRaw( float& fMin, float& fMax, float& fEffectPower, int& iGaussianLevels )
	{
		fMin = m_fMin;
		fMax = m_fMax;
		fEffectPower = m_fEffectPower;
		iGaussianLevels = m_iGaussianLevels;
	}

	static void	SetOverideParameters(	float fMinScalar, float fMaxScalar, 
										float fEffectPowerScalar, int iAdditionalGausian )
	{ 
		m_fMinScalar = fMinScalar;
		m_fMaxScalar = fMaxScalar;
		m_fEffectPowerScalar = fEffectPowerScalar;
		m_iAdditionalGausian = iAdditionalGausian;
	}

	static float	GetMin()			{ return (m_fMin * m_fMinScalar); }
	static float	GetMax()			{ return (m_fMax * m_fMaxScalar); }
	static float	GetEffectPower()	{ return (m_fEffectPower * m_fEffectPowerScalar); }
	static int		GetGausianLevels()	{ return ntstd::Clamp(m_iGaussianLevels + m_iAdditionalGausian, 1, 10); }

private:
	static float m_fMin;
	static float m_fMax;
	static float m_fEffectPower;
	static int m_iGaussianLevels;

	static float	m_fMinScalar;
	static float	m_fMaxScalar;
	static float	m_fEffectPowerScalar;
	static int		m_iAdditionalGausian;
};

#if defined( PLATFORM_PC )
#	include "gfx/lenseffects_pc.h"
#elif defined( PLATFORM_PS3 )
#	include "gfx/lenseffects_ps3.h"
#endif

#endif // end GFX_LENSEFFECTS_H
