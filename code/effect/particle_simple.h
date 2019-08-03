//--------------------------------------------------
//!
//!	\file particle_simple.h
//!	Declaration of the various types of particle
//! handlers used by the simple particle system
//!
//--------------------------------------------------

#ifndef _PARTICLE_SIMPLE_H
#define _PARTICLE_SIMPLE_H

#include "quadlist.h"

#ifdef PLATFORM_PC
#include "gfx/fxmaterial.h"
#endif

#ifdef PLATFORM_PS3
#include "gfx/shader.h"
#endif

#ifndef _PSYSTEM_UTILS_H
#include "effect/psystem_utils.h"
#endif

class ParticleSpawner;
class PSystemSimple;
class PSystemSimpleDef;

class ParticleDef_Rotating;
class ParticleDef_WorldQuad;
class ParticleDef_AxisQuad;
class ParticleDef_VelScaledRay;
struct pos_vel_PD;
struct CPUParticle_PS;
struct CPUParticle_PS_Rot;

//--------------------------------------------------
//!
//!	ParticleHandlerSimple
//! Class that hides what kind of particle we are
//! from the parent effect object.
//!
//--------------------------------------------------
class ParticleHandlerSimple : public CNonCopyable
{
public:
	static ParticleHandlerSimple* Instantiate( const PSystemSimple* pOwner );
	
	// provided for the majority of anticipated particle handlers that use quads / point sprites
	virtual ~ParticleHandlerSimple();
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
	ParticleHandlerSimple( const PSystemSimple* pOwner );

	const PSystemSimple*	m_pOwner;
	const PSystemSimpleDef*	m_pPSystemDef;
	ParticleSpawner*		m_pSpawner;

	QuadList*	m_pQuads;						//!< alloc in Mem::MC_EFFECTS
	u_int		m_iCurrParticle;
	bool		m_bUsingCPUParticle;
	bool		m_bUsingPointSprites;

#ifdef PLATFORM_PC
	FXHandle*		m_ppFX;
#elif PLATFORM_PS3
	DebugShader*	m_pVertexShader;
	DebugShader*	m_pPixelShader;
	void SetPixelShader();
#endif

private:
	ParticleHandlerSimple(); // no-one can manufacture us without using Instantiate()
};

//--------------------------------------------------
//!
//!	PHS_SimpleSprite
//! Type of particle handler
//!
//--------------------------------------------------
class PHS_SimpleSprite : public ParticleHandlerSimple
{
public:
	PHS_SimpleSprite( const PSystemSimple* pOwner ) : ParticleHandlerSimple(pOwner) { ConstructBody(); }

	virtual pos_vel_PD* EmitParticle( float fCurrTime, float fBirthTime, const CPoint& pos, const CDirection& vel );
	virtual void Update( float fTimeDelta, float fTimeDeltaN );

protected:
	// NULL ctor for derived classes
	PHS_SimpleSprite( const PSystemSimple* pOwner, bool SuppressConstruct ) : ParticleHandlerSimple(pOwner) { UNUSED(SuppressConstruct); }

private:
	void ConstructBody();

protected:
	CPUParticle_PS*		m_pParticlesCPU;

	enum PARTICLE_ELEMENT
	{
		PE_BIRTH_TIME	= PV_ELEMENT_0,
		PE_POSITON		= PV_ELEMENT_POS,
		PE_VELOCITY		= PV_ELEMENT_2,
		PE_SIZESTART	= PV_ELEMENT_3,
		PE_SIZEVEL		= PV_ELEMENT_4,
		PE_TEXTURE0		= PV_ELEMENT_5,
	};

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
		void Set( float t, const CPoint& pos, const CDirection& vel, float fSizeStart,	float fSizeVel )
		{
			birthTime = t;
			posX = pos.X();posY= pos.Y();posZ = pos.Z();
			velX = vel.X();velY= vel.Y();velZ = vel.Z();
			size = fSizeStart;
			sizeVel = fSizeVel;
		}
		
		float birthTime;
		float posX, posY, posZ;
		float velX, velY, velZ;
		float size;
		float sizeVel;
	};
};

//--------------------------------------------------
//!
//!	PHS_RotatingSprite
//! Type of particle handler
//!
//--------------------------------------------------
class PHS_RotatingSprite : public ParticleHandlerSimple
{
public:
	PHS_RotatingSprite( const PSystemSimple* pOwner );

	virtual pos_vel_PD* EmitParticle( float fCurrTime, float fBirthTime, const CPoint& pos, const CDirection& vel );
	virtual void Update( float fTimeDelta, float fTimeDeltaN );
	virtual void PreRender();

private:
	ParticleDef_Rotating*	m_pParticleDef;
	CPUParticle_PS_Rot*		m_pParticlesCPU;

	enum PARTICLE_ELEMENT
	{
		PE_BIRTH_TIME	= PV_ELEMENT_0,
		PE_POSITON		= PV_ELEMENT_POS,
		PE_VELOCITY		= PV_ELEMENT_2,
		PE_SIZESTART	= PV_ELEMENT_3,
		PE_SIZEVEL		= PV_ELEMENT_4,
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
					float fSizeStart, float fSizeVel,
					float fRotStart, float fRotVel )
		{
			birthTime = t;
			posX = pos.X();posY= pos.Y();posZ = pos.Z();
			velX = vel.X();velY= vel.Y();velZ = vel.Z();
			size = fSizeStart; sizeVel = fSizeVel;
			rot = fRotStart; rotVel = fRotVel;
		}
		
		float birthTime;
		float posX, posY, posZ;
		float velX, velY, velZ;
		float size; float sizeVel;
		float rot; float rotVel;
	};
};

//--------------------------------------------------
//!
//!	PHS_OrientedQuad
//! Type of particle handler derived from PHS_SimpleSprite
//! as its basically the same with an alternate shader
//!
//--------------------------------------------------
class PHS_OrientedQuad : public PHS_SimpleSprite
{
public:
	PHS_OrientedQuad( const PSystemSimple* pOwner );
	virtual void PreRender();
private:
	ParticleDef_WorldQuad* m_pParticleDef;
};

//--------------------------------------------------
//!
//!	PHS_OrientedQuad
//! Type of particle handler derived from PHS_SimpleSprite
//! as its basically the same with an alternate shader
//!
//--------------------------------------------------
class PHS_AxisAlignedRay : public PHS_SimpleSprite
{
public:
	PHS_AxisAlignedRay( const PSystemSimple* pOwner );
	virtual void PreRender();
private:
	ParticleDef_AxisQuad* m_pParticleDef;
};

//--------------------------------------------------
//!
//!	PHS_VelScaledRay
//! Type of particle handler
//!
//--------------------------------------------------
class PHS_VelScaledRay : public ParticleHandlerSimple
{
public:
	PHS_VelScaledRay( const PSystemSimple* pOwner );

	virtual pos_vel_PD* EmitParticle( float fCurrTime, float fBirthTime, const CPoint& pos, const CDirection& vel );
	virtual void Update( float fTimeDelta, float fTimeDeltaN );
	virtual void PreRender();

private:
	ParticleDef_VelScaledRay*	m_pParticleDef;
	CPUParticle_PS*				m_pParticlesCPU;

	enum PARTICLE_ELEMENT
	{
		PE_BIRTH_TIME	= PV_ELEMENT_0,
		PE_POSITON		= PV_ELEMENT_POS,
		PE_VELOCITY		= PV_ELEMENT_2,
		PE_SIZESTART	= PV_ELEMENT_3,
		PE_SIZEVEL		= PV_ELEMENT_4,
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
		void Set( float t, const CPoint& pos, const CDirection& vel, float fSizeStart,	float fSizeVel )
		{
			birthTime = t;
			posX = pos.X();posY= pos.Y();posZ = pos.Z();
			velX = vel.X();velY= vel.Y();velZ = vel.Z();
			size = fSizeStart;
			sizeVel = fSizeVel;
		}
		
		float birthTime;
		float posX, posY, posZ;
		float velX, velY, velZ;
		float size;
		float sizeVel;
	};

	float m_fLastTimeInterval;
};

#endif // _PARTICLE_SIMPLE_H
