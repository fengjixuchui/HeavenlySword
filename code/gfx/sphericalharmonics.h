//--------------------------------------------------
//!
//!	\file sphericalharmonics.h
//!	all spherical harmonics related shenningans
//!
//--------------------------------------------------

#ifndef GFX_SPHERICAL_HARMONICS_H
#define GFX_SPHERICAL_HARMONICS_H

#include "gfx/texture.h"

//--------------------------------------------------
//!
//!	Static helper class for SH functionality
//!
//--------------------------------------------------
class SHHelper
{
private:

	static void AddDirectionalSample( CDirection* pDest, const CDirection& toLightSource, const CDirection& intensity );

public:
	
	enum SH_BASIS_TYPE
	{
		SH_Y00 = 0,
		SH_Y1_1,
		SH_Y10,
		SH_Y11,
		SH_Y2_2,
		SH_Y2_1,
		SH_Y20,
		SH_Y21,
		SH_Y22,
		SH_BASIS_NUM
	};

	static void AddSample( CDirection* pDest, const CDirection& toLightSource, const CDirection& intensity, const float area = 0.0f );
	static void AddSample( CDirection* pDest, const CDirection& ambient );
	static void AddEnvironment( CDirection* pDest, Texture::Ptr envMap, float fIntensity );

	static CDirection GetAmbient( const CDirection* pSrc );
	static CDirection GetIrradiance( const CDirection* pSrc, const CDirection& direction );
	static CDirection GetIrradiance( const CMatrix* pSrc, const CDirection& direction );

	static void GenerateChannelMatrices( const CDirection* pSrc, CMatrix* pDest );

private:
	static const unsigned int s_uiSqrtSampleCount;

	static const float s_fBasisConst1;
	static const float s_fBasisConst2;
	static const float s_fBasisConst3;
	static const float s_fBasisConst4;
	static const float s_fBasisConst5;

	static const float s_fAmbientMagicConst;

	static const float s_fExpansionConst1;
	static const float s_fExpansionConst2;
	static const float s_fExpansionConst3;
	static const float s_fExpansionConst4;
	static const float s_fExpansionConst5;
};

//--------------------------------------------------
//!
//! structure that describes an SH light environment
//!
//--------------------------------------------------
class SHEnvironment
{
public:
	friend class SHChannelMatrices;

	SHEnvironment() { Clear(); }

	void Clear()
	{
		memset( m_aCoefficients, 0, SHHelper::SH_BASIS_NUM*sizeof(CDirection) );
	}

	SHEnvironment& operator+=( const SHEnvironment& env )
	{
		for( int i = 0; i < SHHelper::SH_BASIS_NUM; i++ )
			m_aCoefficients[i] += env.m_aCoefficients[i];
		return *this;
	}

	SHEnvironment& operator*=( float fScale )
	{
		for( int i = 0; i < SHHelper::SH_BASIS_NUM; i++ )
			m_aCoefficients[i] *= fScale;
		return *this;
	}

	void AddSample( const CDirection& from, const CDirection& intensity )
	{
		SHHelper::AddSample( m_aCoefficients, from, intensity );
	}

	void AddSample( const CDirection& ambient )
	{
		SHHelper::AddSample( m_aCoefficients, ambient );
	}

	void AddEnvironment( Texture::Ptr envMap, float fIntensity )
	{
		SHHelper::AddEnvironment( m_aCoefficients, envMap, fIntensity );
	}

	void SetCoefficient( SHHelper::SH_BASIS_TYPE eCoeff, const CDirection& coeff )
	{
		m_aCoefficients[ eCoeff ] = coeff;
	}

	CDirection GetCoefficient( SHHelper::SH_BASIS_TYPE eCoeff ) const
	{
		return m_aCoefficients[eCoeff];
	}

private:
	CDirection	m_aCoefficients[ SHHelper::SH_BASIS_NUM ];
};

//--------------------------------------------------
//!
//! structure that describes an evaluation ready SH light environment
//!
//--------------------------------------------------
class SHChannelMatrices
{
public:
	SHChannelMatrices(){};

	SHChannelMatrices( const SHEnvironment& copy )
	{
		SHHelper::GenerateChannelMatrices( copy.m_aCoefficients, m_aChannelMats );
	}

	CDirection GetIrradiance( const CDirection& direction )
	{
		return SHHelper::GetIrradiance( m_aChannelMats, direction );
	}

	CMatrix	m_aChannelMats[3];
};





//--------------------------------------------------
//!
//!	Base class for a single editable SH contributor
//!
//--------------------------------------------------
class SHContributor
{
public:
	virtual ~SHContributor() {};
	virtual void AddContribution( SHEnvironment& environment ) = 0;
};

//--------------------------------------------------
//!
//!	Ambient SH contributor
//!
//--------------------------------------------------
class SHAmbient : public SHContributor
{
public:
	SHAmbient() : m_colour( 1.0f, 1.0f, 1.0f, 1.0f ) {};
	virtual void AddContribution( SHEnvironment& environment );
	
	void PostConstruct() { m_internalcolour = CDirection( m_colour ) * m_colour.W(); }
	bool EditorChangeValue( CallBackParameter, CallBackParameter ) { PostConstruct(); return true; }

	CDirection	GetColour() const { return m_internalcolour; }
	CVector		m_colour;

private:
	CDirection	m_internalcolour;	
};

//--------------------------------------------------
//!
//!	Directional SH contributor
//! Note, the direction presented to the user is
//! the other way round to the direction used by
//! the SH evaluation
//!
//--------------------------------------------------
class SHDirectional : public SHContributor
{
public:
	SHDirectional() :
		m_direction( 0.0f, -1.0f, 0.0f ),
		m_colour( 1.0f, 1.0f, 1.0f, 1.0f )
	{};

	virtual void AddContribution( SHEnvironment& environment );
	
	void PostConstruct()
	{
		m_internalDirection = m_direction;
		m_internalDirection.Normalise();

		m_internalcolour = CDirection( m_colour ) * m_colour.W();
	}
	bool EditorChangeValue( CallBackParameter, CallBackParameter ) { PostConstruct(); return true; }

	CDirection GetDirection()	const { return m_internalDirection; }
	CDirection GetColour()		const { return m_internalcolour; }

	CDirection	m_direction;
	CVector		m_colour;

private:
	CDirection	m_internalDirection;
	CDirection	m_internalcolour;	
};

//--------------------------------------------------
//!
//!	SH coeffeicient SH contributor
//!
//--------------------------------------------------
class SHCoefficients : public SHContributor
{
public:
	SHCoefficients()
	{
		m_aCoeffsWithScalar[ SHHelper::SH_Y00 ] = CVector(1.0f,1.0f,1.0f,1.0f);
		m_aCoeffsWithScalar[ SHHelper::SH_Y1_1 ].Clear();
		m_aCoeffsWithScalar[ SHHelper::SH_Y10  ].Clear();
		m_aCoeffsWithScalar[ SHHelper::SH_Y11  ].Clear();
		m_aCoeffsWithScalar[ SHHelper::SH_Y2_2 ].Clear();
		m_aCoeffsWithScalar[ SHHelper::SH_Y2_1 ].Clear();
		m_aCoeffsWithScalar[ SHHelper::SH_Y20  ].Clear();
		m_aCoeffsWithScalar[ SHHelper::SH_Y21  ].Clear();
		m_aCoeffsWithScalar[ SHHelper::SH_Y22  ].Clear();
	}

	virtual void AddContribution( SHEnvironment& environment );

	void PostConstruct();
	bool EditorChangeValue( CallBackParameter, CallBackParameter ) { PostConstruct(); return true; }

	CVector	m_aCoeffsWithScalar[ SHHelper::SH_BASIS_NUM ];

private:
	SHEnvironment m_environment;
};

//--------------------------------------------------
//!
//!	SH coeffeicients from a cubemap
//! NB, this class is only initialised upon EDITING
//! as its so damn expensive. Again i need a decent
//! export pipe for this.
//!
//--------------------------------------------------
class SHCubeMap : public SHContributor
{
public:
	SHCubeMap()
	{
		m_aCoeffsWithScalar[ SHHelper::SH_Y00 ] = CVector(1.0f,1.0f,1.0f,1.0f);
		m_aCoeffsWithScalar[ SHHelper::SH_Y1_1 ].Clear();
		m_aCoeffsWithScalar[ SHHelper::SH_Y10  ].Clear();
		m_aCoeffsWithScalar[ SHHelper::SH_Y11  ].Clear();
		m_aCoeffsWithScalar[ SHHelper::SH_Y2_2 ].Clear();
		m_aCoeffsWithScalar[ SHHelper::SH_Y2_1 ].Clear();
		m_aCoeffsWithScalar[ SHHelper::SH_Y20  ].Clear();
		m_aCoeffsWithScalar[ SHHelper::SH_Y21  ].Clear();
		m_aCoeffsWithScalar[ SHHelper::SH_Y22  ].Clear();
		m_fCubeIntensity = 1.0f;
	}

	virtual void AddContribution( SHEnvironment& environment );

	void PostConstruct();
	bool EditorChangeValue( CallBackParameter, CallBackParameter );

	CVector	m_aCoeffsWithScalar[ SHHelper::SH_BASIS_NUM ];
	ntstd::String	m_cubeTexure;
	float		m_fCubeIntensity;

private:
	SHEnvironment m_environment;
};

#endif // end GFX_SPHERICAL_HARMONICS_H
