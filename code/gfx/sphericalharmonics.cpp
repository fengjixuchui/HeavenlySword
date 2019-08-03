//--------------------------------------------------
//!
//!	\file sphericalharmonics.cpp
//!	all spherical harmonics related shenningans
//!
//--------------------------------------------------

#include "sphericalharmonics.h"
#include "texturemanager.h"

#include "gfx/texturereader.h"

#include "objectdatabase/dataobject.h"
#include "renderer.h"

START_CHUNKED_INTERFACE( SHAmbient, Mem::MC_GFX )

	PUBLISH_VAR_AS(	m_colour, Colour )
	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct ) 
	DECLARE_EDITORCHANGEVALUE_CALLBACK( EditorChangeValue )

END_STD_INTERFACE

START_CHUNKED_INTERFACE( SHDirectional, Mem::MC_GFX )

	PUBLISH_VAR_AS(	m_direction,	Direction )
	PUBLISH_VAR_AS(	m_colour,		Colour )
	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct ) 
	DECLARE_EDITORCHANGEVALUE_CALLBACK( EditorChangeValue )

END_STD_INTERFACE

START_CHUNKED_INTERFACE( SHCoefficients, Mem::MC_GFX )

	PUBLISH_VAR_AS(	m_aCoeffsWithScalar[0],	Y00 )
	PUBLISH_VAR_AS(	m_aCoeffsWithScalar[1],	Y1_1 )
	PUBLISH_VAR_AS(	m_aCoeffsWithScalar[2],	Y10 )
	PUBLISH_VAR_AS(	m_aCoeffsWithScalar[3],	Y11 )
	PUBLISH_VAR_AS(	m_aCoeffsWithScalar[4],	Y2_2 )
	PUBLISH_VAR_AS(	m_aCoeffsWithScalar[5],	Y2_1 )
	PUBLISH_VAR_AS(	m_aCoeffsWithScalar[6],	Y20 )
	PUBLISH_VAR_AS(	m_aCoeffsWithScalar[7],	Y21 )
	PUBLISH_VAR_AS(	m_aCoeffsWithScalar[8],	Y22 )

	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct ) 
	DECLARE_EDITORCHANGEVALUE_CALLBACK( EditorChangeValue )

END_STD_INTERFACE

START_CHUNKED_INTERFACE( SHCubeMap, Mem::MC_GFX )

	PUBLISH_VAR_AS(	m_aCoeffsWithScalar[0],	Y00 )
	PUBLISH_VAR_AS(	m_aCoeffsWithScalar[1],	Y1_1 )
	PUBLISH_VAR_AS(	m_aCoeffsWithScalar[2],	Y10 )
	PUBLISH_VAR_AS(	m_aCoeffsWithScalar[3],	Y11 )
	PUBLISH_VAR_AS(	m_aCoeffsWithScalar[4],	Y2_2 )
	PUBLISH_VAR_AS(	m_aCoeffsWithScalar[5],	Y2_1 )
	PUBLISH_VAR_AS(	m_aCoeffsWithScalar[6],	Y20 )
	PUBLISH_VAR_AS(	m_aCoeffsWithScalar[7],	Y21 )
	PUBLISH_VAR_AS(	m_aCoeffsWithScalar[8],	Y22 )
	
	PUBLISH_VAR_AS(	m_cubeTexure,		DiffuseIrradiance )
	PUBLISH_VAR_AS(	m_fCubeIntensity,	IrradianceIntensity )

	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct ) 
	DECLARE_EDITORCHANGEVALUE_CALLBACK( EditorChangeValue )

END_STD_INTERFACE

const unsigned int SHHelper::s_uiSqrtSampleCount = 256;

const float SHHelper::s_fBasisConst1 = 0.28209479177387814347403972578039f;	// 1 / (2*sqrt(PI))
const float SHHelper::s_fBasisConst2 = 0.48860251190291992158638462283835f;	// sqrt(3) / (2*sqrt(PI))
const float SHHelper::s_fBasisConst3 = 1.0925484305920790705433857058027f;	// sqrt(15) / (2*sqrt(PI))
const float SHHelper::s_fBasisConst4 = 0.31539156525252000603089369029571f;	// sqrt(5) / (4*sqrt(PI))
const float SHHelper::s_fBasisConst5 = 0.54627421529603953527169285290134f;	// sqrt(15) / (4*sqrt(PI))

const float SHHelper::s_fAmbientMagicConst = 1.128379f;	// hmmm, where is this from?

const float SHHelper::s_fExpansionConst1 = 0.429043f;
const float SHHelper::s_fExpansionConst2 = 0.511664f;
const float SHHelper::s_fExpansionConst3 = 0.743125f;
const float SHHelper::s_fExpansionConst4 = 0.886227f;
const float SHHelper::s_fExpansionConst5 = 0.247708f;


//--------------------------------------------------
//!
//!	incorporate a single sample into an SH environment
//!
//--------------------------------------------------
void SHHelper::AddDirectionalSample( CDirection* pDest, const CDirection& toLightSource, const CDirection& intensity )
{
	pDest[SH_Y00]	+= s_fBasisConst1 * intensity;

	pDest[SH_Y1_1]	+= (s_fBasisConst2 * toLightSource.Y()) * intensity;
	pDest[SH_Y10]	+= (s_fBasisConst2 * toLightSource.Z()) * intensity;
	pDest[SH_Y11]	+= (s_fBasisConst2 * toLightSource.X()) * intensity;
	
	pDest[SH_Y2_2]	+= (s_fBasisConst3 * toLightSource.X() * toLightSource.Y() ) * intensity;
	pDest[SH_Y2_1]	+= (s_fBasisConst3 * toLightSource.Y() * toLightSource.Z() ) * intensity;
	pDest[SH_Y21]	+= (s_fBasisConst3 * toLightSource.X() * toLightSource.Z() ) * intensity;
	pDest[SH_Y20]	+= (s_fBasisConst4 * (3.0f * toLightSource.Z() * toLightSource.Z() - 1.0f)) * intensity;
	pDest[SH_Y22]	+= (s_fBasisConst5 * (toLightSource.X() * toLightSource.X() - toLightSource.Y() * toLightSource.Y())) * intensity;
}

//--------------------------------------------------
//!
//!	incorporate a directional area light (sun anyone? :) ) into an SH environment
//!
//--------------------------------------------------
void SHHelper::AddSample( CDirection* pDest, const CDirection& toLightSource, const CDirection& intensity,  float area )
{
	// if our area light has no area then just ad a single point sized directional light
	if ( area == 0.0f )
	{
		AddDirectionalSample( pDest, toLightSource, intensity );
	}
	// otherwise we can simulate an area light adding many point sized directional lights
	else
	{
		// clamp light stereo angle/area
		area = ntstd::Max( area, 0.0f );
		area = ntstd::Min( area, 1.0f );
		area = sqrtf( area );

		float ro = sqrtf( toLightSource.Dot( toLightSource ) );
		if ( ro > 0.0f )
		{
			// from 3D coordinates to spherical coordinates
			float S = sqrtf( toLightSource.X() * toLightSource.X()+ toLightSource.Y() * toLightSource.Y() );
			float phi = acosf( toLightSource.Z() / ro );
			float theta = asinf( toLightSource.Y() / S );;
			
			if ( toLightSource.X() < 0.0f )
				theta = PI - theta;

			// from spherical coordinated to homogeneously distributed and normalized 2D coordinates
			float x = 1.0f - powf ( cosf ( theta * 0.5f ), 2.0f );
			float y = phi / TWO_PI;

			for (unsigned int i = 0; i < s_uiSqrtSampleCount; i++)
			{
				for (unsigned int j = 0; j < s_uiSqrtSampleCount; j++)
				{
					float sample_x = x + (i / s_uiSqrtSampleCount - 0.5f) * area;
					float sample_y = y + (j / s_uiSqrtSampleCount - 0.5f) * area;

					float sample_theta = 2.0f * acosf( sqrtf( 1.0f - sample_x ) );
					float sample_phi = TWO_PI * sample_y;
					
					AddDirectionalSample( pDest, CDirection( sample_phi, sample_theta ), intensity / ( s_uiSqrtSampleCount * s_uiSqrtSampleCount) );
				}
			}
		}
	}
}

//--------------------------------------------------
//!
//!	incorporate a single ambient sample into an SH environment
//!
//--------------------------------------------------
void SHHelper::AddSample( CDirection* pDest, const CDirection& ambient )
{
	pDest[SH_Y00]	+= s_fAmbientMagicConst * ambient;
}

//--------------------------------------------------
//!
//!	incorporate a cube map as as an SH environment
//! Warning! this is slow and should only be used
//! as an offline tool
//!
//--------------------------------------------------

static float GetLuminance( CDirection const& obColour )
{
	static CDirection const obConversion( 0.212671f, 0.715160f, 0.072169f );
	return obColour.Dot( obConversion );
}

void SHHelper::AddEnvironment( CDirection* pDest, Texture::Ptr envMap, float fIntensity )
{
	// make it error texture safe
	if( envMap->GetType() != TT_CUBE_TEXTURE )
		return;

	ntAssert( envMap->GetType() == TT_CUBE_TEXTURE );

	// static data for the cube map face bases
	static const CDirection aobCubeTextureBases[] = 
	{
		CDirection(1.0f, 1.0f, 1.0f), 
		CDirection(0.0f, 0.0f, -2.0f),   
		CDirection(0.0f, -2.0f, 0.0f), 

		CDirection(-1.0f, 1.0f, -1.0f), 
		CDirection(0.0f, 0.0f, 2.0f),   
		CDirection(0.0f, -2.0f, 0.0f), 

		CDirection(-1.0f, 1.0f, -1.0f), 
		CDirection(2.0f, 0.0f, 0.0f),   
		CDirection(0.0f, 0.0f, 2.0f), 

		CDirection(-1.0f, -1.0f, 1.0f), 
		CDirection(2.0f, 0.0f, 0.0f),   
		CDirection(0.0f, 0.0f, -2.0f), 

		CDirection(-1.0f, 1.0f, 1.0f), 
		CDirection(2.0f, 0.0f, 0.0f),   
		CDirection(0.0f, -2.0f, 0.0f), 

		CDirection(1.0f, 1.0f, -1.0f), 
		CDirection(-2.0f, 0.0f, 0.0f),   
		CDirection(0.0f, -2.0f, 0.0f)
	};
	
	// lock each cubemap face and add to the environment
	float fTotalIntensity = 0;

	for( int iFace = 0; iFace < 6; ++iFace )
	{
		uint32_t pitch;        
		TextureReader it( envMap->CPULockCube( iFace, pitch ), envMap->GetFormat() );

//		ntAssert(pitch == 4*envMap->GetWidth());
//		const u_int* puiData = reinterpret_cast<u_int*>(stLock.pBits);

		for(u_int iV = 0; iV < envMap->GetHeight(); ++iV)
		{
			for(u_int iU = 0; iU < envMap->GetWidth(); ++iU)
			{
//				u_int iIndex = (iV * stDesc.Width) + iU;

				float fU = (_R(iU) + 0.5f)/_R(envMap->GetWidth());
				float fV = (_R(iV) + 0.5f)/_R(envMap->GetHeight());

				CDirection normal = aobCubeTextureBases[3*iFace]
					+ fU*aobCubeTextureBases[3*iFace + 1]
					+ fV*aobCubeTextureBases[3*iFace + 2];
				normal.Normalise();

				CDirection intensity = CDirection( it.Current() );
				fTotalIntensity += GetLuminance( intensity );
/*
				CDirection intensity(
					_R((puiData[iIndex] >> 16) & 0xff), 
					_R((puiData[iIndex] >> 8) & 0xff), 
					_R((puiData[iIndex] >> 0) & 0xff)
//					_R((puiData[iIndex] >> 24) & 0xff) // dont care about the alpha channel
				);
*/

				AddSample( pDest, normal, intensity );
				it.Next();
			}
		}

		envMap->CPUUnlockCube( iFace );
	}

	// normalise the result
//	float fNormalisation = (fIntensity * 4.0f) / _R( 255 * 6 * stDesc.Width*stDesc.Height );
	float fNormalisation = fIntensity / fTotalIntensity;
	for(int iHarmonic = 0; iHarmonic < SH_BASIS_NUM; ++iHarmonic)
		pDest[iHarmonic] *= fNormalisation;
}

//--------------------------------------------------
//!
//!	retrive the ambient light in this environment
//!
//--------------------------------------------------
CDirection SHHelper::GetAmbient( const CDirection* pSrc )
{
	static const float fConst = 1.0f / s_fAmbientMagicConst;
	return pSrc[SH_Y00] * fConst;
}

//--------------------------------------------------
//!
//!	retrive the irradiance in this direction in this environment
//!
//--------------------------------------------------
CDirection SHHelper::GetIrradiance( const CDirection* pSrc, const CDirection& direction )
{
	static CMatrix sharmonics[3];
	
	GenerateChannelMatrices( pSrc, sharmonics );
	return GetIrradiance( sharmonics, direction );
}

//--------------------------------------------------
//!
//!	retrive the irradiance in this direction in this environment
//!
//--------------------------------------------------
CDirection SHHelper::GetIrradiance( const CMatrix* pSrc, const CDirection& direction )
{
	CVector temp( direction.X(), direction.Y(), direction.Z(), 1.0f );

	CDirection irradiance;
	irradiance.X() = CVector(temp * pSrc[0]).Dot( temp );
	irradiance.Y() = CVector(temp * pSrc[1]).Dot( temp );
	irradiance.Z() = CVector(temp * pSrc[2]).Dot( temp );
	return irradiance;
}

//--------------------------------------------------
//!
//!	get coeeficients in colour chanel form
//!
//--------------------------------------------------
void SHHelper::GenerateChannelMatrices( const CDirection* pSrc, CMatrix* pDest )
{
	for(int iChannel = 0; iChannel < 3; ++iChannel)
	{
		pDest[iChannel][0].X() = s_fExpansionConst1 *	pSrc[	SH_Y22	][iChannel];
		pDest[iChannel][0].Y() = s_fExpansionConst1 *	pSrc[	SH_Y2_2	][iChannel];
		pDest[iChannel][0].Z() = s_fExpansionConst1 *	pSrc[	SH_Y21	][iChannel];
		pDest[iChannel][0].W() = s_fExpansionConst2 *	pSrc[	SH_Y11	][iChannel];

		pDest[iChannel][1].X() = s_fExpansionConst1 *	pSrc[	SH_Y2_2	][iChannel];
		pDest[iChannel][1].Y() = -s_fExpansionConst1 *	pSrc[	SH_Y22	][iChannel];
		pDest[iChannel][1].Z() = s_fExpansionConst1 *	pSrc[	SH_Y2_1	][iChannel];
		pDest[iChannel][1].W() = s_fExpansionConst2 *	pSrc[	SH_Y1_1	][iChannel];

		pDest[iChannel][2].X() = s_fExpansionConst1 *	pSrc[	SH_Y21	][iChannel];
		pDest[iChannel][2].Y() = s_fExpansionConst1 *	pSrc[	SH_Y2_1	][iChannel];
		pDest[iChannel][2].Z() = s_fExpansionConst3 *	pSrc[	SH_Y20	][iChannel];
		pDest[iChannel][2].W() = s_fExpansionConst2 *	pSrc[	SH_Y10	][iChannel];

		pDest[iChannel][3].X() = s_fExpansionConst2 *	pSrc[	SH_Y11	][iChannel]; 
		pDest[iChannel][3].Y() = s_fExpansionConst2 *	pSrc[	SH_Y1_1	][iChannel];
		pDest[iChannel][3].Z() = s_fExpansionConst2 *	pSrc[	SH_Y10	][iChannel];
		pDest[iChannel][3].W() = s_fExpansionConst4 *	pSrc[	SH_Y00	][iChannel] 
								- s_fExpansionConst5 *	pSrc[	SH_Y20	][iChannel];
	}
}

//--------------------------------------------------
//!
//!	SHAmbient::AddContribution
//!
//--------------------------------------------------
void SHAmbient::AddContribution( SHEnvironment& environment )
{
	environment.AddSample( GetColour() );
}

//--------------------------------------------------
//!
//!	SHDirectional::AddContribution
//!
//--------------------------------------------------
void SHDirectional::AddContribution( SHEnvironment& environment )
{
	// Note, the direction presented to the user is
	// the other way round to the direction used by
	// the SH evaluation
	environment.AddSample( -GetDirection(), GetColour() );
}

//--------------------------------------------------
//!
//!	SHCoefficients::PostConstruct
//!
//--------------------------------------------------
void SHCoefficients::PostConstruct()
{
	m_environment.Clear();
	for ( int i = SHHelper::SH_Y00; i < SHHelper::SH_BASIS_NUM; i++ )
	{
		CDirection val = CDirection( m_aCoeffsWithScalar[i] ) * m_aCoeffsWithScalar[i].W();
		m_environment.SetCoefficient( (SHHelper::SH_BASIS_TYPE)i, val );
	}
}

//--------------------------------------------------
//!
//!	SHCoefficients::AddContribution
//!
//--------------------------------------------------
void SHCoefficients::AddContribution( SHEnvironment& environment )
{
	environment += m_environment;
}


//--------------------------------------------------
//!
//!	SHCubeMap::PostConstruct
//!
//--------------------------------------------------
void SHCubeMap::PostConstruct()
{
	m_environment.Clear();
	for ( int i = SHHelper::SH_Y00; i < SHHelper::SH_BASIS_NUM; i++ )
	{
		CDirection val = CDirection( m_aCoeffsWithScalar[i] ) * m_aCoeffsWithScalar[i].W();
		m_environment.SetCoefficient( (SHHelper::SH_BASIS_TYPE)i, val );
	}
}

//--------------------------------------------------
//!
//!	SHCubeMap::EditorChangeValue
//!
//--------------------------------------------------
bool SHCubeMap::EditorChangeValue( CallBackParameter param, CallBackParameter )
{
	// if we've been edited and we have a valid cube map, read it in,
	// convert it to a set of SH coeffs, then config our serialisable 
	// values. That way, we only do this during editing. NB, if the 
	// texture changes, your screwed unless you edit the base ref again...

	CHashedString pName(param);

	if (pName == CHashedString(HASH_STRING_DIFFUSEIRRADIANCE))
	{
		// load the texture and do explicit format conversion
		Texture::Ptr temp = TextureManager::Get().LoadTexture_Neutral( m_cubeTexure.c_str(), true );
		m_environment.AddEnvironment( temp, m_fCubeIntensity );
		for ( int i = SHHelper::SH_Y00; i < SHHelper::SH_BASIS_NUM; i++ )
		{
			m_aCoeffsWithScalar[i] = CVector( m_environment.GetCoefficient( (SHHelper::SH_BASIS_TYPE) i ) );
			m_aCoeffsWithScalar[i].W() = 1.0f;
		}

	}
	return true;
}

//--------------------------------------------------
//!
//!	SHCubeMap::AddContribution
//!
//--------------------------------------------------
void SHCubeMap::AddContribution( SHEnvironment& environment )
{
	environment += m_environment;
}

