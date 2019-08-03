//--------------------------------------------------
//!
//!	\file depthoffield.h
//!	The shared portion of the depth of field object,
//! include the platform specific bits as well
//!
//--------------------------------------------------

#ifndef GFX_DEPTHOFFIELD_H
#define GFX_DEPTHOFFIELD_H

//--------------------------------------------------
//!
//!	DepthOfFieldXML
//!
//--------------------------------------------------
class DepthOfFieldXML
{
public:
	DepthOfFieldXML()
	{
		m_nearBlurDepth = 0.0f;
		m_focalPlaneDepth = 2.0f;
		m_farBlurDepth = 100.0f;
		m_circleOfConfusion = 5.0f;
		m_circleOfConfusionLow = 2.5f;
	}

	float m_nearBlurDepth;
	float m_focalPlaneDepth;
	float m_farBlurDepth;
	float m_circleOfConfusion;
	float m_circleOfConfusionLow;

	void PostConstruct();
	bool EditorChangeValue(const char*, const char*)
	{
		PostConstruct();
		return true;
	}
};

//--------------------------------------------------
//!
//!	DepthOfFieldSettings 
//! Global config object
//!
//--------------------------------------------------
class DepthOfFieldSettings
{
public:

friend class DepthOfField;

	DepthOfFieldSettings()
	{
		m_bApplyDepthOfField = false;
		m_nearBlurDepth = 0.0f;
		m_focalPlaneDepth = 2.0f;
		m_farBlurDepth = 100.0f;
		m_circleOfConfusion = 5.0f;
		m_circleOfConfusionLow = 2.5f;
	}
	
	float GetNearBlurDepth()	const { return m_nearBlurDepth; }
	float GetFarBlurDepth()		const { return m_farBlurDepth; }
	float GetFocalPlaneDepth()	const { return m_focalPlaneDepth; }
	float GetCircleMaxHiRez()	const { return m_circleOfConfusion; }
	float GetCircleMaxLoRez()	const { return m_circleOfConfusionLow; }

	void SetNearBlurDepth( float nearBlurDepth )		{ m_nearBlurDepth = ntstd::Min( nearBlurDepth, m_focalPlaneDepth ); }
	void SetFarBlurDepth( float farBlurDepth )			{ m_farBlurDepth = ntstd::Max( farBlurDepth, m_focalPlaneDepth ); }
	void SetFocalPlaneDepth( float focalPlaneDepth )	{ m_focalPlaneDepth = ntstd::Clamp( focalPlaneDepth, m_nearBlurDepth, m_farBlurDepth ); }
	void SetCircleMaxHiRez( float fCOC )				{ m_circleOfConfusion = fCOC; }
	void SetCircleMaxLoRez( float fCOC )				{ m_circleOfConfusionLow = fCOC; }

	CVector GetVSParameters() const
	{
		return CVector( m_nearBlurDepth, m_focalPlaneDepth, m_farBlurDepth, 0.0f );
	}

	bool m_bApplyDepthOfField;

private:
	float m_nearBlurDepth;
	float m_focalPlaneDepth;
	float m_farBlurDepth;
	float m_circleOfConfusion;
	float m_circleOfConfusionLow;
};

#if defined( PLATFORM_PC )
#	include "gfx/depthoffield_pc.h"
#elif defined( PLATFORM_PS3 )
#	include "gfx/depthoffield_ps3.h"
#endif

#endif // end GFX_LENSEFFECTS_H
