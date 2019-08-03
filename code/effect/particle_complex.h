//--------------------------------------------------
//!
//!	\file particle_complex.h
//!	Declaration of the various types of particle
//! handlers used by the complex particle system
//!
//--------------------------------------------------

#ifndef _PARTICLE_COMPLEX_H
#define _PARTICLE_COMPLEX_H

#include "quadlist.h"
#include "texture_function.h"
#include "game/randmanager.h"

#ifndef _PSYSTEM_UTILS_H
#include "effect/psystem_utils.h"
#endif

#ifdef PLATFORM_PC
#include "gfx/fxmaterial.h"
#endif

#ifdef PLATFORM_PS3
#include "gfx/shader.h"
#endif

class ParticleSpawner;
class PSystemComplex;
class PSystemComplexDef;

class ParticleDef_Rotating;
class ParticleDef_WorldQuad;
class ParticleDef_AxisQuad;
class ParticleDef_VelScaledRay;

struct pos_vel_PD;
struct CPUParticle_PF;
struct CPUParticle_PF_Rot;

//--------------------------------------------------
//!
//!	ParticleHandlerComplex
//! Class that hides what kind of particle we are
//! from the parent effect object.
//!
//--------------------------------------------------
class ParticleHandlerComplex : public CNonCopyable
{
public:
	static ParticleHandlerComplex* Instantiate( const PSystemComplex* pOwner );
	
	// provided for the majority of anticipated particle handlers that use quads / point sprites
	virtual ~ParticleHandlerComplex();
	virtual void PreRender();
	virtual void Render();

	// pure virtuals that must be implemented by derived classes
	virtual pos_vel_PD* EmitParticle( float fCurrTime, float fBirthTime, const CPoint& pos, const CDirection& vel ) = 0;
	virtual void Update( float fTimeDelta, float fTimeDeltaN ) = 0;
	
#ifdef PLATFORM_PC
	ID3DXEffect* GetEffect()	{ return m_ppFX->Get(); }
#elif PLATFORM_PS3
	Shader& GetVertexShader()	{ return *m_pVertexShader; }
	Shader& GetPixelShader()	{ return *m_pPixelShader; }
#endif

	bool UsingCPUParticles() const { return m_bUsingCPUParticle; }
	bool UsingPointSprites() const { return m_bUsingPointSprites; }

	void SetSpawner( ParticleSpawner* pSpawner ) { m_pSpawner = pSpawner; }

protected:
	ParticleHandlerComplex( const PSystemComplex* pOwner );

	const PSystemComplex*		m_pOwner;
	const PSystemComplexDef*	m_pPSystemDef;
	ParticleSpawner*			m_pSpawner;
	
	QuadList*	m_pQuads;					//!< alloc in Mem::MC_EFFECTS
	u_int		m_iCurrParticle;
	bool		m_bUsingCPUParticle;
	bool		m_bUsingPointSprites;

	static inline float CalcSizeResult( float fNormalisedAge, float fUniqueLerp, const FunctionSampler& sizeFuncs )
	{
		float sizeCurve1 = sizeFuncs.Sample_linearFilter( fNormalisedAge, 0 );
		float sizeCurve2 = sizeFuncs.Sample_linearFilter( fNormalisedAge, 1 );
		return (sizeCurve1 * (1.0f - fUniqueLerp)) + (sizeCurve2 * fUniqueLerp);
	}

#ifdef PLATFORM_PC
	FXHandle*		m_ppFX;
#elif PLATFORM_PS3
	DebugShader*	m_pVertexShader;
	DebugShader*	m_pPixelShader;
	void SetPixelShader();
#endif

private:
	ParticleHandlerComplex(); // no-one can manufacture us without using Instantiate()
};

//--------------------------------------------------
//!
//!	PHC_SimpleSprite
//! Type of particle handler
//!
//--------------------------------------------------
class PHC_SimpleSprite : public ParticleHandlerComplex
{
public:
	PHC_SimpleSprite( const PSystemComplex* pOwner ) : ParticleHandlerComplex(pOwner) { ConstructBody(); }

	virtual pos_vel_PD* EmitParticle( float fCurrTime, float fBirthTime, const CPoint& pos, const CDirection& vel );
	virtual void Update( float fTimeDelta, float fTimeDeltaN );

protected:
	// NULL ctor for derived classes
	PHC_SimpleSprite( const PSystemComplex* pOwner, bool SuppressConstruct ) : ParticleHandlerComplex(pOwner) { UNUSED(SuppressConstruct); }

private:
	void ConstructBody();
	void SimpleMovementUpdate( float fTimeDelta, float fTimeDeltaN );
	void AdvancedMovementUpdate( float fTimeDelta, float fTimeDeltaN );

protected:
	enum PARTICLE_ELEMENT
	{
		PE_BIRTH_TIME	= PV_ELEMENT_0,
		PE_POSITON		= PV_ELEMENT_POS,
		PE_VELOCITY		= PV_ELEMENT_2,
		PE_SIZE1		= PV_ELEMENT_3,
		PE_SIZE2		= PV_ELEMENT_4,
		PE_TEXTURE0		= PV_ELEMENT_5,
	};

	CPUParticle_PF* m_pParticlesCPU;

	// Plain Old Data Particle
	// data sent to GPU from a CPU based particle
	struct PODParticle 
	{
		PODParticle( float fAgeN, const CPoint& pos, float fsize ) :
			ageN(fAgeN),
			posX(pos.X()), posY(pos.Y()), posZ(pos.Z()),
			size(fsize) {}

		float ageN;
		float posX, posY, posZ;
		float size;
	};

	// GPU Particle
	// Data sent to the GPU corresponding to a full GPU based particle
	struct GPUParticle
	{
		void Set( float t, const CPoint& pos, const CDirection& vel )
		{
			birthTime = t;
			posX = pos.X();posY= pos.Y();posZ = pos.Z();
			velX = vel.X();velY= vel.Y();velZ = vel.Z();
			sizeLerpStart = erandf( 1.0f );
			sizeLerpRange = erandf( 1.0f ) - sizeLerpStart;
		}
		
		float birthTime;
		float posX, posY, posZ;
		float velX, velY, velZ;
		float sizeLerpStart;
		float sizeLerpRange;
	};
};

//--------------------------------------------------
//!
//!	PHC_RotatingSprite
//! Type of particle handler
//!
//--------------------------------------------------
class PHC_RotatingSprite : public ParticleHandlerComplex
{
public:
	PHC_RotatingSprite( const PSystemComplex* pOwner );

	virtual pos_vel_PD* EmitParticle( float fCurrTime, float fBirthTime, const CPoint& pos, const CDirection& vel );
	virtual void Update( float fTimeDelta, float fTimeDeltaN );
	virtual void PreRender();

private:
	CPUParticle_PF_Rot*		m_pParticlesCPU;
	ParticleDef_Rotating*	m_pParticleDef;
	
	void SimpleMovementUpdate( float fTimeDelta, float fTimeDeltaN );
	void AdvancedMovementUpdate( float fTimeDelta, float fTimeDeltaN );

	enum PARTICLE_ELEMENT
	{
		PE_BIRTH_TIME	= PV_ELEMENT_0,
		PE_POSITON		= PV_ELEMENT_POS,
		PE_VELOCITY		= PV_ELEMENT_2,
		PE_SIZE1		= PV_ELEMENT_3,
		PE_SIZE2		= PV_ELEMENT_4,
		PE_ROTSTART		= PV_ELEMENT_5,
		PE_ROTVEL		= PV_ELEMENT_6,
		PE_TEXTURE0		= PV_ELEMENT_7,
	};

	// Plain Old Data Particle
	// data sent to GPU from a CPU based particle
	struct PODParticle 
	{
		PODParticle( float fAgeN, const CPoint& pos, float fsize, float frot ) :
			ageN(fAgeN),
			posX(pos.X()), posY(pos.Y()), posZ(pos.Z()),
			size(fsize),
			rot(frot) {}

		float ageN;
		float posX, posY, posZ;
		float size;
		float rot;
	};

	// GPU Particle
	// Data sent to the GPU corresponding to a full GPU based particle
	struct GPUParticle
	{
		void Set(	float t, const CPoint& pos, const CDirection& vel,
					float fRotStart, float fRotVel )
		{
			birthTime = t;
			posX = pos.X();posY= pos.Y();posZ = pos.Z();
			velX = vel.X();velY= vel.Y();velZ = vel.Z();
			sizeLerpStart = erandf( 1.0f );
			sizeLerpRange = erandf( 1.0f ) - sizeLerpStart;
			rot = fRotStart; rotVel = fRotVel;
		}
		
		float birthTime;
		float posX, posY, posZ;
		float velX, velY, velZ;
		float sizeLerpStart; float sizeLerpRange;
		float rot; float rotVel;
	};
};

//--------------------------------------------------
//!
//!	PHC_OrientedQuad
//! Type of particle handler
//!
//--------------------------------------------------
class PHC_OrientedQuad : public PHC_SimpleSprite
{
public:
	PHC_OrientedQuad( const PSystemComplex* pOwner );
	virtual void PreRender();
private:
	ParticleDef_WorldQuad* m_pParticleDef;
};

//--------------------------------------------------
//!
//!	PHC_AxisAlignedRay
//! Type of particle handler
//!
//--------------------------------------------------
class PHC_AxisAlignedRay : public PHC_SimpleSprite
{
public:
	PHC_AxisAlignedRay( const PSystemComplex* pOwner );
	virtual void PreRender();
private:
	ParticleDef_AxisQuad* m_pParticleDef;
};

//--------------------------------------------------
//!
//!	PHC_VelScaledRay
//! Type of particle handler
//!
//--------------------------------------------------
class PHC_VelScaledRay : public ParticleHandlerComplex
{
public:
	PHC_VelScaledRay( const PSystemComplex* pOwner );

	virtual pos_vel_PD* EmitParticle( float fCurrTime, float fBirthTime, const CPoint& pos, const CDirection& vel );
	virtual void Update( float fTimeDelta, float fTimeDeltaN );
	virtual void PreRender();

private:
	void SimpleMovementUpdate( float fTimeDelta, float fTimeDeltaN );
	void AdvancedMovementUpdate( float fTimeDelta, float fTimeDeltaN );

	ParticleDef_VelScaledRay* m_pParticleDef;
	CPUParticle_PF*	m_pParticlesCPU;

	enum PARTICLE_ELEMENT
	{
		PE_BIRTH_TIME	= PV_ELEMENT_0,
		PE_POSITON		= PV_ELEMENT_POS,
		PE_VELOCITY		= PV_ELEMENT_2,
		PE_SIZE1		= PV_ELEMENT_3,
		PE_SIZE2		= PV_ELEMENT_4,
		PE_VELINTERVAL	= PV_ELEMENT_5,
		PE_TEXTURE0		= PV_ELEMENT_6,
	};

	// Plain Old Data Particle
	// data sent to GPU from a CPU based particle
	struct PODParticle 
	{
		PODParticle( float fAgeN, const CPoint& pos, float fsize, const CDirection& vel ) :
			ageN(fAgeN),
			posX(pos.X()), posY(pos.Y()), posZ(pos.Z()),
			size(fsize),
			velX(vel.X()), velY(vel.Y()), velZ(vel.Z()) {}

		float ageN;
		float posX, posY, posZ;
		float size;
		float velX, velY, velZ;
	};

	// GPU Particle
	// Data sent to the GPU corresponding to a full GPU based particle
	struct GPUParticle
	{
		void Set( float t, const CPoint& pos, const CDirection& vel )
		{
			birthTime = t;
			posX = pos.X();posY= pos.Y();posZ = pos.Z();
			velX = vel.X();velY= vel.Y();velZ = vel.Z();
			sizeLerpStart = erandf( 1.0f );
			sizeLerpRange = erandf( 1.0f ) - sizeLerpStart;
		}
		
		float birthTime;
		float posX, posY, posZ;
		float velX, velY, velZ;
		float sizeLerpStart;
		float sizeLerpRange;
	};

	float m_fLastTimeInterval;
};

#endif // _PARTICLE_COMPLEX_H
