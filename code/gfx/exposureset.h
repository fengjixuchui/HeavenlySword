//--------------------------------------------------
//!
//!	\file exposureset.h
//!	Level lighting data defineing the exposure function
//!
//--------------------------------------------------

#ifndef GFX_EXPOSURE_SET_H
#define GFX_EXPOSURE_SET_H

#include "gfx/lightingtemplates.h"
#include "gfx/spatialinterpolator.h"

//--------------------------------------------------
//!
//!	ExposureSettings 
//! Settings for current exposure function
//!
//--------------------------------------------------
class ExposureSettings
{
public:
	ExposureSettings() :
		m_fSamplingArea( 1.0f ), 
		m_fKeyValueMapping( 0.18f ), 
		m_fLuminanceBurnout( 8.0f ), 
		m_fErrorReduction( 5.0f ), 
		m_fBloomFilterMin( 1.5f ), 
		m_fBloomFilterMax( 10.0f ),
		m_fBloomFilterEffectPower( 1.0f/2.0f ),
		m_fBloomGaussianLevels( 4.0f ),
		m_fKeyLuminanceMin( 0.0001f ),
		m_fKeyLuminanceMax( 10000.0f )
	{}
	
	void Clear() { memset( this, 0, sizeof(ExposureSettings) ); }

	float m_fSamplingArea;
	float m_fKeyValueMapping;
	float m_fLuminanceBurnout;
	float m_fErrorReduction;

	float m_fBloomFilterMin;
	float m_fBloomFilterMax;
	float m_fBloomFilterEffectPower;
	float m_fBloomGaussianLevels;
	float m_fKeyLuminanceMin;
	float m_fKeyLuminanceMax;
		
	//! opereators to aide lerping between nodes
	ExposureSettings& operator+=( const ExposureSettings& otherSettings )
	{
		m_fSamplingArea				+= otherSettings.m_fSamplingArea;
		m_fKeyValueMapping			+= otherSettings.m_fKeyValueMapping;
		m_fLuminanceBurnout			+= otherSettings.m_fLuminanceBurnout;
		m_fErrorReduction			+= otherSettings.m_fErrorReduction;
		
		m_fBloomFilterMin			+= otherSettings.m_fBloomFilterMin;
		m_fBloomFilterMax			+= otherSettings.m_fBloomFilterMax;
		m_fBloomFilterEffectPower	+= otherSettings.m_fBloomFilterEffectPower;
		m_fBloomGaussianLevels		+= otherSettings.m_fBloomGaussianLevels;

		m_fKeyLuminanceMin			+= otherSettings.m_fKeyLuminanceMin;
		m_fKeyLuminanceMax			+= otherSettings.m_fKeyLuminanceMax;

		return *this;
	}

	ExposureSettings& operator*=( float fScale )
	{
		m_fSamplingArea				*= fScale;
		m_fKeyValueMapping			*= fScale;
		m_fLuminanceBurnout			*= fScale;
		m_fErrorReduction			*= fScale;
		
		m_fBloomFilterMin			*= fScale;
		m_fBloomFilterMax			*= fScale;
		m_fBloomFilterEffectPower	*= fScale;
		m_fBloomGaussianLevels		*= fScale;

		m_fKeyLuminanceMin			*= fScale;
		m_fKeyLuminanceMax			*= fScale;

		return *this;
	}
};

//--------------------------------------------------
//!
//!	ExposureSettings_TOD
//! Settings for current exposure function at a particular time of day
//!
//--------------------------------------------------
class ExposureSettings_TOD
{
public:
	ExposureSettings_TOD() : m_fTOD( 0.0f ) {};
	float GetSortingValue() const { return m_fTOD; }

	float				m_fTOD;
	ExposureSettings	m_settings;
};




//--------------------------------------------------
//!
//!	Spatial_ExposureSettings 
//! Virtual base class for exposure setting nodes
//!
//--------------------------------------------------
class Spatial_ExposureSettings
{
public:
	Spatial_ExposureSettings() : m_centerOfInfluence( CONSTRUCT_CLEAR ) {};
	virtual ~Spatial_ExposureSettings() {};

	virtual void GetSettings( ExposureSettings& settings ) const = 0;
	CPoint	m_centerOfInfluence;
};

//--------------------------------------------------
//!
//!	Spatial_ExposureSettings_Simple
//! Basic spatial exposure settings, a wrapper around a
//! ExposureSettings object
//!
//--------------------------------------------------
class Spatial_ExposureSettings_Simple : public Spatial_ExposureSettings
{
public:
	virtual void GetSettings( ExposureSettings& settings ) const { settings = m_settings; }
	ExposureSettings m_settings;
};

//--------------------------------------------------
//!
//!	Spatial_ExposureSettings_TOD
//! Has a sub list of nodes based on time of day
//!
//--------------------------------------------------
class Spatial_ExposureSettings_TOD : public Spatial_ExposureSettings
{
public:
	void PostConstruct();
	void SanityCheck() const;

	mutable SortableList< ExposureSettings_TOD, Mem::MC_GFX > m_expSettings;
	virtual void GetSettings( ExposureSettings& settings ) const;
};



//--------------------------------------------------
//!
//!	ExposureSet 
//! Set of Key light nodes, linked by position
//!
//--------------------------------------------------
class ExposureSet
{
public:
	ExposureSet();
	~ExposureSet();
	void PostConstruct();
	bool EditorChangeValue(CallBackParameter,CallBackParameter);

	// get the current exposure settings 
	void GetSettings( const CPoint& position, ExposureSettings& settings ) const;

	typedef ntstd::List< Spatial_ExposureSettings*, Mem::MC_GFX > SpatialExposureSettingsList;
	// the list of Key light nodes
	SpatialExposureSettingsList m_nodes;

private:
	SpatialInterpolator* m_pSpatialInterpolator;

	class comparatorMoreThan
	{
	public:
		bool operator()( const PointIDandFraction* pFirst, const PointIDandFraction* pSecond ) const
		{
			return ( pFirst->fraction > pSecond->fraction );
		}
	};
};

#endif // GFX_EXPOSURE_SET_H
