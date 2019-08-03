//--------------------------------------------------
//!
//!	\file particle_structs.h
//!	Common particle structures used by particle handlers
//!
//--------------------------------------------------

#ifndef _PARTICLE_STRUCTS_H
#define _PARTICLE_STRUCTS_H

//--------------------------------------------------
//!
//! Basic Data Type
//!
//--------------------------------------------------
struct pos_vel_PD
{
	CPoint pos;
	CDirection vel;
	float ageN;

	// please remember the order here is important (vel update before pos)
	// as this must compliment Iterative_ParticleMover::Update (pos before vel)
	void OffsetInitialConditions(	float fTimeStep,
									float fTimeStepN,
									const CDirection& acc )
	{
		vel -= acc * fTimeStep;
		pos -= vel * fTimeStep;
		ageN -= fTimeStepN;
	}
};

//--------------------------------------------------
//!
//! CPUParticle_PS
//! CPU side representation of a CPU based particle
//!
//--------------------------------------------------
struct CPUParticle_PS : public pos_vel_PD
{
	CPUParticle_PS(){}
	CPUParticle_PS( float t ) { ageN = t; }
	inline void Init( const CPoint& posIn, const CDirection& velIn, float fSizeStart, float fSizeVel )
	{
		pos = posIn;
		vel = velIn;
		ageN = 0.0f;
		sizeStart = fSizeStart;
		sizeVel = fSizeVel;
	}

	float GetSize() const { return (ageN * sizeVel) + sizeStart; }
	float sizeStart, sizeVel;
};

//--------------------------------------------------
//!
//! CPUParticle_PF
//! CPU side representation of a CPU based particle
//!
//--------------------------------------------------
struct CPUParticle_PF : public pos_vel_PD
{
	CPUParticle_PF(){}
	CPUParticle_PF( float t ) { ageN = t; }
	inline void Init( const CPoint& posIn, const CDirection& velIn )
	{
		pos = posIn;
		vel = velIn;
		bUp = true;
		ageN = 0.0f;
		lerpStart = erandf( 1.0f );
		lerpRange = erandf( 1.0f ) - lerpStart;
	}

	float GetUniqueLerp() const { return (ageN * lerpRange) + lerpStart; }
	float lerpStart, lerpRange;
	bool bUp;
};

//--------------------------------------------------
//!
//! CPUParticle_PS_Rot
//! CPU side representation of a CPU based particle
//!
//--------------------------------------------------
struct CPUParticle_PS_Rot : public CPUParticle_PS
{
	CPUParticle_PS_Rot(){}
	CPUParticle_PS_Rot( float t ) { ageN = t; }
	inline void Init( const CPoint& posIn, const CDirection& velIn, float fSizeStart, float fSizeVel, float fRotStart, float fRotVel  )
	{
		CPUParticle_PS::Init( posIn, velIn, fSizeStart, fSizeVel );
		rot = fRotStart;
		rotVel = fRotVel;
	}

	float rot, rotVel;
};

//--------------------------------------------------
//!
//! CPUParticle_PF_Rot
//! CPU side representation of a CPU based particle
//!
//--------------------------------------------------
struct CPUParticle_PF_Rot : public CPUParticle_PF
{
	CPUParticle_PF_Rot(){}
	CPUParticle_PF_Rot( float t ) { ageN = t; }
	inline void Init( const CPoint& posIn, const CDirection& velIn, float fRotStart, float fRotVel )
	{
		CPUParticle_PF::Init( posIn, velIn );
		rot = fRotStart;
		rotVel = fRotVel;
	}
	float rot, rotVel;
};

#endif // _PARTICLE_STRUCTS_H
