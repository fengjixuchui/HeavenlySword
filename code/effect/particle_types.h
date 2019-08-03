//--------------------------------------------------
//!
//!	\file particle_types.h
//!	Definitions for particles types exposed in welder
//!
//--------------------------------------------------

#ifndef _PARTICLE_TYPES_H
#define _PARTICLE_TYPES_H

// this can disapear when we uprade to new XML
#include "game/randmanager.h"


//--------------------------------------------------
//!
//!	PARTICLE_TYPE
//!
//--------------------------------------------------
enum PARTICLE_TYPE
{
	// view plane aligned primitives
	PT_SIMPLE_SPRITE = 0,
	PT_ROTATING_SPRITE,
	
	// abitarily aligned quads
	PT_WORLD_ALIGNED_QUAD,
	PT_AXIS_ALIGNED_RAY,
	PT_VELOCITY_ALIGNED_RAY,

	// other types
//	PT_CLUMP,
};

//--------------------------------------------------
//!
//!	ParticleDef_Rotating
//! Definintion Structure for rotating particles
//!
//--------------------------------------------------
class ParticleDef_Rotating
{
public:
	ParticleDef_Rotating();
	virtual ~ParticleDef_Rotating(){};

	virtual void PostConstruct();
	virtual bool EditorChangeValue( CallBackParameter, CallBackParameter ) { PostConstruct(); return true; }
	virtual void EditorChangeParent() { PostConstruct(); }

	float		m_fRotationStartMin;
	float		m_fRotationStartMax;
	float		m_fRotationVelMin;
	float		m_fRotationVelMax;
	float		m_fRotationAcc;
	float		m_fVelocitySidedness;

	// so we dont have to multiply out into radians all the time
	float		GetNewRotation( void ) const { return erandf( m_fRotationStartMaxRad - m_fRotationStartMinRad ) + m_fRotationStartMinRad; }
	float		GetNewRotationVel( void ) const
	{
		float fRot = erandf( m_fRotationVelMaxRad - m_fRotationVelMinRad ) + m_fRotationVelMinRad;
		float fInvert = (erandf(1.0f) >= m_fVelocitySidedness) ? 1.0f : -1.0f;
		return fRot * fInvert;
	}
	float		GetRotationAcc( void ) const { return m_fRotationAccRad; }

private:
	float		m_fRotationStartMinRad;
	float		m_fRotationStartMaxRad;
	float		m_fRotationVelMinRad;
	float		m_fRotationVelMaxRad;
	float		m_fRotationAccRad;
};

//--------------------------------------------------
//!
//!	ParticleDef_WorldQuad
//! Definintion Structure for world oriented quads
//!
//--------------------------------------------------
class ParticleDef_WorldQuad
{
public:
	ParticleDef_WorldQuad();
	virtual ~ParticleDef_WorldQuad(){};

	virtual void PostConstruct();
	virtual bool EditorChangeValue( CallBackParameter, CallBackParameter ) { PostConstruct(); return true; }
	virtual void EditorChangeParent() { PostConstruct(); }

	CDirection	m_YPR;
	const CMatrix&	GetOrientation( void ) const { return m_orientation; }

private:
	CMatrix	m_orientation;
};

//--------------------------------------------------
//!
//!	ParticleDef_AxisQuad
//! Definintion Structure for axis rotated quads
//!
//--------------------------------------------------
class ParticleDef_AxisQuad
{
public:
	ParticleDef_AxisQuad();
	virtual ~ParticleDef_AxisQuad(){};


	virtual void PostConstruct() { m_directionN = m_dir; m_directionN.Normalise(); }
	virtual bool EditorChangeValue( CallBackParameter, CallBackParameter ) { PostConstruct(); return true; }
	virtual void EditorChangeParent() { PostConstruct(); }

	CDirection	m_dir;
	CDirection	GetDirection( void ) const { return m_directionN; }

private:
	CDirection	m_directionN;
};

//--------------------------------------------------
//!
//!	ParticleDef_VelScaledRay
//! Definintion Structure for velocity scaled quads
//!
//--------------------------------------------------
class ParticleDef_VelScaledRay
{
public:
	ParticleDef_VelScaledRay();
	CVector	GetScales( void ) const { return CVector( m_fXScale, m_fYScale, 0.0f, 0.0f); }

	bool m_bFixedTime;
	float m_fXScale;
	float m_fYScale;
};

#endif // _PARTICLE_TYPES_H
