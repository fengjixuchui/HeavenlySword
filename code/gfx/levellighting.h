//--------------------------------------------------
//!
//!	\file levellighting.h
//!	NT_NEW classes holding xml defs for multiple lighting
//! contexts within a level.
//!
//--------------------------------------------------

#ifndef _LEVEL_LIGHTING_H
#define _LEVEL_LIGHTING_H

class CEffectMoonDef;
class CGhostLensEffectSettings;
class PostProcessingSet;
class FillLightSet;
class KeyLightSet;
class ExposureSet;


//--------------------------------------------------
//!
//!	SolarArc
//! Definies the path of the sun or moon
//!
//--------------------------------------------------
class SolarArc
{
public:
	SolarArc() : 
		m_fHorizonAngle( 0.0f ), 
		m_fTiltAngle( 0.0f )
	{}

	// float fTime is a normalised float value
	CDirection GetDirection( float fTime ) const
	{
		float fTilt = m_fTiltAngle * DEG_TO_RAD_VALUE;
		float fHorizon = m_fHorizonAngle * DEG_TO_RAD_VALUE;

		CDirection obUnrotated( fsinf( fTilt ), fcosf( fTilt )*fsinf( fTime*PI ), fcosf( fTime*PI ) );
		CDirection obResult(	fcosf( fHorizon )*obUnrotated.X() + fsinf( fHorizon )*obUnrotated.Z(), 
								obUnrotated.Y(),
								-fsinf( fHorizon )*obUnrotated.X() + fcosf( fHorizon )*obUnrotated.Z());
		obResult.Normalise();
		return obResult;
	}

    float m_fHorizonAngle;
	float m_fTiltAngle;
};

//--------------------------------------------------
//!
//!	ShadowMapParams
//! Tweakable shadows parameters
//!
//--------------------------------------------------
class ShadowMapParamsDef
{
public:
	ShadowMapParamsDef() : 
		m_fShadowSlope( 2.0f ), 
		m_fShadowBias( 0.0000001f ),
		m_fShadowPlane0( 0.000f ),
		m_fShadowPlane1( 0.055f ),
		m_fShadowPlane2( 0.100f ),
		m_fShadowPlane3( 0.500f ),
		m_fShadowPlane4( 1.500f ),
		m_bUseKeyDirection( true ),
		m_vShadowDirection( 0.707f, -0.707f, 0 )
	{}

	float m_fShadowSlope;
	float m_fShadowBias;
	float m_fShadowPlane0;
	float m_fShadowPlane1;
	float m_fShadowPlane2;
	float m_fShadowPlane3;
	float m_fShadowPlane4;
	bool	m_bUseKeyDirection;
	CDirection m_vShadowDirection;
};

//--------------------------------------------------
//!
//!	LevelLightingCtrl
//! Dummy object that it's being used to destroy the
//! current lighting def and load a new one
//--------------------------------------------------

class LevelLightingCtrl
{
public:

	static int LoadLevelLightingDef(const char* pcNewLeveLightingDef);
	static void SwitchLightingFile(const char* pcNewLeveLightingDef);


private:

	LevelLightingCtrl();
};

//--------------------------------------------------
//!
//!	LevelLightingDef
//! Object that defines all the global lighting info
//! for a level
//!
//--------------------------------------------------
class LevelLightingDef
{
public:
	LevelLightingDef() :
		m_fSunRise( 6.0f ),
		m_fSunSet( 18.0f ),
		m_fPauseExposureFactor( -0.95f ),
		m_bSeperateHazeDir( false ),
		m_vHazeDir( 0.707f, -0.707f, 0 )
	{}

	void PostConstruct();
	~LevelLightingDef();

	float			m_fSunRise;
	float			m_fSunSet;
	float			m_fPauseExposureFactor;
	static float	s_fHoursInDay;

	CEffectMoonDef*				m_pMoonDef;
	SolarArc*					m_pSunArc;
	SolarArc*					m_pMoonArc;
	CGhostLensEffectSettings*	m_pGhostSettings;
	PostProcessingSet*			m_pPostProcessingSet;
	FillLightSet*				m_pFillLightSet;
	KeyLightSet*				m_pKeyLightSet;
	ExposureSet*				m_pExposureSet;
	ShadowMapParamsDef*			m_pShadowMapParams;
	bool						m_bSeperateHazeDir;
	CDirection					m_vHazeDir;

	bool IsDayTime( float fTimeOfDay ) const { return ((fTimeOfDay >= m_fSunRise) && (fTimeOfDay <= m_fSunSet)) ? true : false; }
	float GetDaylightHours() const { return (m_fSunSet - m_fSunRise); }
	float GetMoonlitHours() const { return (s_fHoursInDay - GetDaylightHours()); }

	// get direction to sun and moon at any time
	CDirection GetDirectionToSun( float fTimeOfDay ) const
	{
		float fTimeN = 0.0f;
		
		// remap time to sun solar arc input (0.0f->2.0f with 0.0f at m_fSunRise)
		if (IsDayTime(fTimeOfDay))
			fTimeN = (fTimeOfDay - m_fSunRise) / GetDaylightHours();
		else if (fTimeOfDay > m_fSunSet)
			fTimeN = ((fTimeOfDay - m_fSunSet) / GetMoonlitHours()) + 1.0f;
		else
			fTimeN = ((fTimeOfDay + (s_fHoursInDay - m_fSunSet))/ GetMoonlitHours()) + 1.0f;
		
		return m_pSunArc->GetDirection( fTimeN );
	}

	CDirection GetDirectionToMoon( float fTimeOfDay ) const
	{
		float fTimeN = 0.0f;

		// remap time to moon solar arc input (0.0f->2.0f with 0.0f at m_fSunSet)
		if (fTimeOfDay > m_fSunSet)
			fTimeN = (fTimeOfDay - m_fSunSet) / GetMoonlitHours();
		else if (fTimeOfDay < m_fSunRise)
			fTimeN = (fTimeOfDay + (s_fHoursInDay - m_fSunSet)) / GetMoonlitHours();
		else
			fTimeN = ((fTimeOfDay - m_fSunRise) / GetDaylightHours()) + 1.0f;

		return m_pMoonArc->GetDirection( fTimeN );
	}

	// Assumes moons is key at night, sun during the day.
	CDirection GetDirToKeyLight( float fTimeOfDay ) const
	{
		ntAssert( (fTimeOfDay >= 0.0f) && (fTimeOfDay <= s_fHoursInDay) );
		return IsDayTime(fTimeOfDay) ? GetDirectionToSun(fTimeOfDay) : GetDirectionToMoon(fTimeOfDay);
	}
};

//--------------------------------------------------
//!
//!	LevelLighting 
//! Singleton made by above class
//!
//--------------------------------------------------
class LevelLighting : public Singleton<LevelLighting, Mem::MC_GFX >
{
public:
	friend class LevelLightingCtrl;

	LevelLighting::LevelLighting();
	void RetrieveContextLighting( const CPoint& eyePos );

	//static void InstallDefaults();

	// get a fake light which basically blend the sun position with a directional faciing
	// the camera (a looking down)
	CDirection GetWorldRimLight(float fCoef, const CDirection& worldRealLight, const CDirection& worldCameraDirection);

	// get other light directions
	CDirection	GetDirectionToMoon() const		{ return m_pDef->GetDirectionToMoon(m_fTimeOfDay); }

	// stuff to do with time of day
	void	SetTimeOfDay( float fTimeOfDay )
	{
		m_fTimeOfDay = fTimeOfDay;
		while ( m_fTimeOfDay > LevelLightingDef::s_fHoursInDay )
		{
			m_fTimeOfDay -= LevelLightingDef::s_fHoursInDay;
		}
		while ( m_fTimeOfDay < 0.0f )
		{
			m_fTimeOfDay += LevelLightingDef::s_fHoursInDay;
		}
	}

	float	GetTimeOfDay() const { return m_fTimeOfDay; }
	float	GetTimeOfDayN() const { return m_fTimeOfDay / LevelLightingDef::s_fHoursInDay; }
	void	GetTimeOfDay( float& fHours, float& fMinutes )
	{
		fMinutes = fmodf( m_fTimeOfDay, 1.0f );
		fHours = m_fTimeOfDay - fMinutes;
		fMinutes *= 60.0f;
	}

	const CGhostLensEffectSettings* GetGhostSetting()
	{
		return m_pDef->m_pGhostSettings;
	}

	void RegisterLightingFile( CHashedString pLevelLightName, LevelLightingDef* pDef );
	void UnRegisterLightingFile( CHashedString pLevelLightName );
	void UseLightingFile( CHashedString pLevelLightName );
	int GetNumLightingFilesRegistered();

private:
	// this levels lighting data
	LevelLightingDef* m_pDef;

	static const int MAX_LEVELLIGHT_FILES = 8;

	struct LevelLightStruct
	{
		CHashedString			m_pName;
		LevelLightingDef*		m_pLightDef;
	};

	LevelLightStruct	m_LevelLightDefs[MAX_LEVELLIGHT_FILES];

	float	m_fTimeOfDay;
};

#endif // _LEVEL_LIGHTING_H
