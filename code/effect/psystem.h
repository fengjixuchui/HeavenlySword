//--------------------------------------------------
//!
//!	\file psystem.h
//!	core particle system classes
//!
//--------------------------------------------------

#ifndef _PSYS_H
#define _PSYS_H

#include "effect/particle_types.h"
#include "effect/psystem_utils.h"
#include "effect/renderstate_block.h"
#include "effect/sortable_effect.h"
#include "effect/effect_resetable.h"

#include "gfx/texture.h"
#include "gfx/hardwarecaps.h"

#ifdef PLATFORM_PS3
#include "gfx/shader.h"
#endif

class ParticleSpawnDef;
class EmitterDef;
class PlaneDef;
class ParticleSpawner;
class Emitter;
class TextureAtlas;
class Transform;
struct pos_vel_PD;

//--------------------------------------------------
//!
//!	ParticleSystemStdParams
//!
//--------------------------------------------------
class ParticleSystemStdParams
{
public:
	ParticleSystemStdParams();
	virtual ~ParticleSystemStdParams(){};
	void	EstablishParticleType();
	void	ResolveTextureMode();
	void	LoadTexture();

	// misc
	int						m_iRandomSeed;
	CDirection				m_acceleration;
	float					m_fParticleLifetime;
	PARTICLE_TYPE			m_particleType;

	PARTICLE_TEXTURE_MODE	m_eTexMode;
	Texture::Ptr			m_pTex;
	const TextureAtlas*		m_pAtlas;

	// sub XML objects
	ParticleSpawnDef*	m_pSpawnDef;
	EmitterDef*			m_pDefaultEmitterDef;
	void*				m_pParticleDef;
	PlaneDef*			m_pBouncePlaneDef;

	// render setup
	ntstd::String		m_texName;
	RenderStateDef		m_renderStateDef;
	bool				m_bUseRandAtlasTex;
	bool				m_bSortedParticles;
	float				m_fCullRadius;
	float				m_fSortingPush;

	// bounce setup
	float				m_fRestitution;
	bool				m_bUseRayCast;

	bool	RequiresBounceCode()	const;
	bool	RequiresSorting()		const;
	bool	CanUsePointSprite()		const;
	virtual bool RequiresCPUParticle()	const;

	// for housekeeping
	ResetSet<Resetable> m_resetSet;
	virtual const char* GetDebugName() const = 0;
};

//--------------------------------------------------
//!
//!	ParticleSystem
//! Base class that provides common functionality 
//! for simple and complex particle systems
//!
//--------------------------------------------------
class ParticleSystem : public SortableEffect, public Resetable
{
public:
	void EmitParticle( float fNormalisedT ); // method provided for m_pEmitter to call
	u_int GetMaxParticles() const { return m_iMaxParticles; }

	CDirection& GetProceduralVel() { return m_proceduralVel; }
	CVector&	GetProceduralCol() { return m_proceduralCol; }
	void		SetEmitMultiplier( float fMult ) { m_fEmitMultiplier = fMult; }

protected:
	// members intentionally accessable by derived classes
	virtual ~ParticleSystem();
	virtual void UpdateParticles( float fTimeChange ) = 0;
	virtual pos_vel_PD* SpawnNewParticle( float fCurrTime, float fBirthTime, const CPoint& pos, const CDirection& vel ) = 0;

	ParticleSystem( const ParticleSystemStdParams& baseDef,
					const CMatrix& frame, const EmitterDef* pOverideEmit )
	{
		ConstructInternal( &baseDef, &frame, 0, pOverideEmit );
	}
	
	ParticleSystem( const ParticleSystemStdParams& baseDef,
					const Transform& transform, const EmitterDef* pOverideEmit )
	{
		ConstructInternal( &baseDef, 0, &transform, pOverideEmit );
	}

	void ResetBase( bool bDestructor = false );

	ParticleSpawner* m_pSpawner;						//!< alloc in Mem::MC_EFFECTS

	bool UpdateParticleSystem( float fTimeDelta );
	void RetriveNewPosAndVel( CPoint& pos, CDirection& vel, float fNormalisedT );
	void DebugRenderEmitter( bool bEditing );
	void DebugRenderInfo(	const char* pCPUCost,
							const char* pVSCost,
							const char* pPSCost,
							const char* pTechnique,
							const char* pType );

#ifdef PLATFORM_PC
	void UploadStdParameters( ID3DXEffect* pFX );
#else
	void UploadStdParameters(	Shader& vertexShader,
								Shader& pixelShader,
								bool bPointSprite,
								bool bCPUParticles );
#endif

	bool EmitterEdited() const;

private:
	void ConstructInternal( const ParticleSystemStdParams* pBaseDef,
							const CMatrix*		pFrame,
							const Transform*	pTransform,
							const EmitterDef*	pOverideEmit );

	void RetrieveWorldInfo( float fTimeDelta );

	const ParticleSystemStdParams*	m_pBaseDef;
	
	const Transform*	m_pTransform;
	CMatrix				m_emitterFrameCurr;
	const EmitterDef*	m_pEmitterInitInfo;
	Emitter*			m_pEmitter;					//!< alloc in Mem::MC_EFFECTS

	u_int				m_iMaxParticles;
	int					m_iLastRand;
	float				m_fLastEmitTime;

	CDirection			m_proceduralVel;
	CVector				m_proceduralCol;
	float				m_fEmitMultiplier;

	// these are only updated if we have a parent transform
	CPoint				m_emitterPosCurr;
	CPoint				m_emitterPosOld;
	CVector				m_posSplineCoeffs[4];

	CQuat				m_startQuat;
	CQuat				m_endQuat;

	// as above. these are used specifically for interpolation
	CDirection			m_emitterVelCurr;
	CDirection			m_emitterVelOld;
	CVector				m_velSplineCoeffs[4];

	inline CPoint GetSmoothPos( float fTime )
	{
		CVector pos = m_posSplineCoeffs[0];
		for (int j = 1; j < 4; j++)
		{
			pos *= fTime;
			pos += m_posSplineCoeffs[j];
		}
		pos.W() = 0.0f;
		return CPoint( pos );
	}

	inline CDirection GetSmoothVel( float fTime )
	{
		CVector vel = m_velSplineCoeffs[0];
		for (int j = 1; j < 4; j++)
		{
			vel *= fTime;
			vel += m_velSplineCoeffs[j];
		}
		vel.W() = 0.0f;
		return CDirection( vel );
	}
};

#endif // _PSYS_H
