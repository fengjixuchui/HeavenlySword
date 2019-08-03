//--------------------------------------------------
//!
//!	\file particle_spawning.h
//!	Particles seeding other particle systems
//!
//--------------------------------------------------

#ifndef _PARTICLE_SPAWNING_H
#define _PARTICLE_SPAWNING_H

#include "anim/transform.h"

#include "effect_util.h"
#include "effect_manager.h"

struct pos_vel_PD;

//--------------------------------------------------
//!
//!	ParticleSpawnDef
//!	Class that defines external interface to spawn
//! functionality
//!
//--------------------------------------------------
class ParticleSpawnDef
{
public:
	ParticleSpawnDef();
	virtual ~ParticleSpawnDef() {};
	virtual void PostConstruct ();
	virtual bool EditorChangeValue( CallBackParameter, CallBackParameter ) { PostConstruct(); return true; }

	bool m_bAttatchStartEffects;
	bool m_bCleanupStartEffects;
	ntstd::List<void*>	m_toSpawnAtStart;

	bool m_bAttatchBounceEffects;
	bool m_bCleanupBounceEffects;
	ntstd::List<void*>	m_toSpawnAtBounce;

	bool m_bAttatchDeathEffects;
	bool m_bCleanupDeathEffects;
	ntstd::List<void*>	m_toSpawnAtDeath;

	float m_fDeathEffectMinTime;
	float m_fDeathEffectMaxTime;

	int	m_iBounceCount;
	bool m_bKillParticleOnBounce;

	bool HasStartEffects() const { return !m_toSpawnAtStart.empty(); }
	bool HasBounceEffects() const { return !m_toSpawnAtBounce.empty(); }
	bool HasDeathEffects() const { return !m_toSpawnAtDeath.empty(); }
	bool HasAttachedEffects() const { return (m_bAttatchStartEffects || m_bAttatchBounceEffects || m_bAttatchDeathEffects); }
};

//--------------------------------------------------
//!
//!	ParticleSpawner
//!	Class that handles mechanics of spawning new
//! particle systems from within a given particle system
//!
//--------------------------------------------------
class ParticleSpawner
{
public:
	ParticleSpawner( const ParticleSpawnDef* pDef, u_int iMaxParticles );
	~ParticleSpawner()
	{
		Reset();
		NT_DELETE_ARRAY_CHUNK( Mem::MC_EFFECTS, m_pAdditionalInfo );
	}

	void Reset( bool bCleanNow = false );
	void NewParticleToTrack( const pos_vel_PD* pToTrack );
	void BounceNotification( pos_vel_PD* pJustBounced, const CPoint& intersect, const CDirection& normal );
	void Update();

private:
	static void MatrixFromParticle( const pos_vel_PD* pParticle, CMatrix& result );
	static void MatrixFromPointUpAndForwards(	const CPoint& point, const CDirection& up,
												const CDirection& forwards, CMatrix& result );
	const ParticleSpawnDef* m_pDef;
	u_int m_iMaxParticles;

	struct ParticleInfo
	{
		// setup death time
		inline void Init( const ParticleSpawnDef* pDef )
		{
			fDeathtime = erandf(	pDef->m_fDeathEffectMaxTime -
								pDef->m_fDeathEffectMinTime ) + pDef->m_fDeathEffectMinTime;
			
			iBounceCount = pDef->m_iBounceCount;
			bTriggeredDeath = false;

			ntAssert( effectsToClean.empty() );
		}

		// free allocated effects
		inline void Clean( bool bCleanNow = false )
		{
			while (!effectsToClean.empty())
			{
				if (bCleanNow)
					EffectManager::Get().KillEffectNow( effectsToClean.back() );
				else
					EffectManager::Get().KillEffectWhenReady( effectsToClean.back() );

				effectsToClean.pop_back();
			}
		}

		Transform		transform;
		float			fDeathtime;
		int				iBounceCount;
		bool			bTriggeredDeath;
		ntstd::List<u_int>	effectsToClean;
	};

	ParticleInfo*	m_pAdditionalInfo;

	typedef ntstd::Vector<u_int, Mem::MC_EFFECTS> SlotList;
	SlotList m_freeSlots;

	typedef ntstd::Map<const pos_vel_PD*,u_int,ntstd::less<const pos_vel_PD*>, Mem::MC_EFFECTS > ParticleTable;
	ParticleTable m_particles;

	// mark particles slot as free, clean its effects and remove from the list
	inline u_int AllocateParticle()
	{
		ntAssert( !m_freeSlots.empty() );
		u_int iSlot = m_freeSlots.back();
		m_freeSlots.pop_back();
		return iSlot;
	}

	// mark particles slot as free, clean its effects and remove from the list
	inline ParticleTable::iterator ReleaseParticle( ParticleTable::iterator it, bool bCleanNow = false )
	{
		ntAssert( it != m_particles.end() );
		m_pAdditionalInfo[it->second].Clean(bCleanNow);
		m_freeSlots.push_back(it->second);
		return m_particles.erase( it );
	}
	
	static void FireEffectList( bool bAttach, bool bClean, const ntstd::List<void*>& effects,
								const CMatrix& frame, ParticleInfo& recipient );
};

#endif // _PARTICLE_SPAWNING_H
