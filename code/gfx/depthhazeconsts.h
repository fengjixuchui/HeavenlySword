//--------------------------------------------------
//!
//!	\file depthhazeconsts.h
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#ifndef _DH_CONSTS_H
#define _DH_CONSTS_H

#ifndef GFX_RENDERCONTEXT_H
#include "gfx/rendercontext.h"
#endif

//--------------------------------------------------
//!
//!	CDepthHazeSetting
//!
//--------------------------------------------------
class CDepthHazeSetting
{
public:

	static inline CVector GetAConsts()
	{
		// CONSTS_A = 1 < log2 *e, termMultiplier >
		return CVector(1.0f, 0.818284367f, RenderingContext::Get()->m_keyLight.m_fInscatterMultiplier, 0.0f);
	}

	static inline CVector GetGConsts()
	{
		// g == Henley/Greenstein phase function eccentricity (Probability a photon has been scattered)
		const float g = RenderingContext::Get()->m_keyLight.m_fHenleyGreensteinEccentricity;

		// CONSTS_G = <(1-g)^2, WorldSunPower, WorldSunMultiplier> 
		return CVector( (1-g)*(1-g), RenderingContext::Get()->m_keyLight.m_fWorldSunPower, RenderingContext::Get()->m_keyLight.m_fWorldSunMultiplier, 0.0f);
	}

	// beta_1 = Br = Rayleigh coefficient<RGB> beta_2 = Mie coefficient<RGB>
	static inline CVector GetBeta1PlusBeta2()
	{
		return	RenderingContext::Get()->m_keyLight.m_rayleighScattering
				+ RenderingContext::Get()->m_keyLight.m_mieScattering;
	}

	static inline CVector GetOneOverBeta1PlusBeta2()
	{
		const CVector obBeta1PlusBeta2 = GetBeta1PlusBeta2();

		CVector temp(CONSTRUCT_CLEAR);
		if( obBeta1PlusBeta2.X() > 1e-5f )
			temp.X() = 1.f / obBeta1PlusBeta2.X();
		if( obBeta1PlusBeta2.Y() > 1e-5f )
			temp.Y() = 1.f / obBeta1PlusBeta2.Y();
		if( obBeta1PlusBeta2.Z() > 1e-5f )
			temp.Z() = 1.f / obBeta1PlusBeta2.Z();
		return temp;
	}

	static inline CVector GetBetaDash1()
	{
		return RenderingContext::Get()->m_keyLight.m_rayleighScattering * 3.f/16.f*PI;
	}

	static inline CVector GetBetaDash2()
	{
		return RenderingContext::Get()->m_keyLight.m_mieScattering * 1.f/4.f*PI;
	}

	static inline CDirection GetSunDir()
	{
		return( RenderingContext::Get()->m_sunDirection );
	}

	static inline CVector GetSunColour()
	{
		CVector result( RenderingContext::Get()->m_keyColour );
		float fSunLen = RenderingContext::Get()->m_keyColour.Length();
		if( fSunLen > 1e-5f )
			result *= 1.0f / fSunLen;
		result.W() = fSunLen;
		return result;
	}
};

#endif // _DH_CONST_H
