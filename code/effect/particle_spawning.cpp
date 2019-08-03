//--------------------------------------------------
//!
//!	\file particle_spawning.cpp
//!	Particles seeding other particle systems
//!
//--------------------------------------------------

#include "particle_spawning.h"
#include "particle_structs.h"
#include "effect_error.h"
#include "effect/psystem_utils.h"
#include "camera/camutils.h"
#include "objectdatabase/dataobject.h"

START_STD_INTERFACE( ParticleSpawnDef )

	I2BOOL		( m_bAttatchStartEffects,	AttatchStartEffects )
	I2BOOL		( m_bCleanupStartEffects,	CleanupStartEffects )
	PUBLISH_PTR_CONTAINER_AS		( m_toSpawnAtStart,			ToSpawnOnStart )

	I2BOOL		( m_bAttatchBounceEffects,	AttatchBounceEffects )
	I2BOOL		( m_bCleanupBounceEffects,	CleanupBounceEffects )
	PUBLISH_PTR_CONTAINER_AS		( m_toSpawnAtBounce,		ToSpawnOnBounce )

	I2BOOL		( m_bAttatchDeathEffects,	AttatchDeathEffects )
	I2BOOL		( m_bCleanupDeathEffects,	CleanupDeathEffects )
	PUBLISH_PTR_CONTAINER_AS		( m_toSpawnAtDeath,			ToSpawnOnDeath )

	I2FLOAT		( m_fDeathEffectMinTime,	DeathEffectMinTime(0--1) )
	I2FLOAT		( m_fDeathEffectMaxTime,	DeathEffectMaxTime(0--1) )

	I2INT		( m_iBounceCount,			NumBouncesThatSpawn )
	I2BOOL		( m_bKillParticleOnBounce,	KillParticleOnBounce )

	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
	DECLARE_EDITORCHANGEVALUE_CALLBACK( EditorChangeValue )
END_STD_INTERFACE

//--------------------------------------------------
//!
//!	ParticleSpawnDef ctor
//!
//--------------------------------------------------
ParticleSpawnDef::ParticleSpawnDef() :
	m_bAttatchStartEffects(true),
	m_bCleanupStartEffects(true),

	m_bAttatchBounceEffects(false),
	m_bCleanupBounceEffects(false),

	m_bAttatchDeathEffects(true),
	m_bCleanupDeathEffects(true),

	m_fDeathEffectMinTime(1.0f),
	m_fDeathEffectMaxTime(1.0f),

	m_iBounceCount(1),
	m_bKillParticleOnBounce(false)
{
}

//--------------------------------------------------
//!
//!	ParticleSpawnDef::PostConstruct
//!
//--------------------------------------------------
void ParticleSpawnDef::PostConstruct()
{
	m_fDeathEffectMinTime = ntstd::Clamp( m_fDeathEffectMinTime, 0.0f, 1.0f );
	m_fDeathEffectMaxTime = ntstd::Clamp( m_fDeathEffectMaxTime, 0.0f, 1.0f );

	if (m_bAttatchStartEffects)
		m_bCleanupStartEffects = true;

	if (m_bAttatchBounceEffects)
		m_bCleanupBounceEffects = true;

	if (m_bAttatchDeathEffects)
		m_bCleanupDeathEffects = true;
}




//--------------------------------------------------
//!
//!	ParticleSpawner::ctor
//!
//--------------------------------------------------
ParticleSpawner::ParticleSpawner( const ParticleSpawnDef* pDef, u_int iMaxParticles ) :
	m_pDef(pDef),
	m_iMaxParticles(iMaxParticles)
{
	ntError( m_pDef );
	ntError( m_iMaxParticles > 0 );

	m_pAdditionalInfo = NT_NEW_ARRAY_CHUNK ( Mem::MC_EFFECTS ) ParticleInfo[m_iMaxParticles];
	m_freeSlots.reserve(m_iMaxParticles);

	u_int iValue = iMaxParticles;
	while ( iValue > 0 )
	{
		m_freeSlots.push_back( --iValue );
	}
}

//--------------------------------------------------
//!
//!	ParticleSpawner::dtor
//!
//--------------------------------------------------
void ParticleSpawner::Reset( bool bCleanNow )
{
	for ( ParticleTable::iterator it = m_particles.begin(); it != m_particles.end();  )
	{
		it = ReleaseParticle( it, bCleanNow);
	}

	ntAssert( m_freeSlots.size() == m_iMaxParticles );
}

//--------------------------------------------------
//!
//!	ParticleSpawner::FireEffectList
//! Handy wrapper for effect triggering code
//!
//--------------------------------------------------
void ParticleSpawner::FireEffectList(	bool bAttach, bool bClean, const ntstd::List<void*>& effects,
										const CMatrix& frame, ParticleInfo& recipient )
{
	for ( ntstd::List<void*>::const_iterator it = effects.begin(); it != effects.end(); ++it )
	{
		u_int iGUID;
		
		if ( bAttach )
			iGUID = PSystemUtils::ConstructParticleEffect( *it, &recipient.transform );
		else
			iGUID = PSystemUtils::ConstructParticleEffect( *it, frame );				

		if (( bClean ) && ( iGUID ))
			recipient.effectsToClean.push_back( iGUID );
	}
}

//--------------------------------------------------
//!
//!	ParticleSpawner::NewParticleToTrack
//! register particle to track
//!
//--------------------------------------------------
void ParticleSpawner::NewParticleToTrack( const pos_vel_PD* pToTrack )
{
	ntAssert( pToTrack );

	if (m_particles.find(pToTrack) != m_particles.end())
	{
		// hmm, we already have this one, so we dont need to add it 
		// however, we still need to cleanup whatever was going on with its previous life

		m_pAdditionalInfo[ m_particles[pToTrack] ].Clean();
	}
	else
	{
		// dont have this one alread, so assign it a new info block and add it to the list
		m_particles[pToTrack] = AllocateParticle();	
	}
	
	// setup the info block
	u_int iSlot = m_particles[pToTrack];
	m_pAdditionalInfo[iSlot].Init(m_pDef);

	if (m_pDef->HasAttachedEffects())
	{
		CMatrix frame;
		MatrixFromParticle( pToTrack, frame );
		m_pAdditionalInfo[iSlot].transform.SetLocalMatrix(frame);
	}

	// now create startup effects if any
	if (m_pDef->HasStartEffects())
	{
		CMatrix frame;
		MatrixFromParticle( pToTrack, frame );
		FireEffectList( m_pDef->m_bAttatchStartEffects, m_pDef->m_bCleanupStartEffects,
						m_pDef->m_toSpawnAtStart, frame, m_pAdditionalInfo[iSlot] );						
	}
}

//--------------------------------------------------
//!
//!	ParticleSpawner::MatrixFromParticle
//! convert particle info into a matrix
//!
//--------------------------------------------------
void ParticleSpawner::MatrixFromParticle( const pos_vel_PD* pParticle, CMatrix& result )
{
	CCamUtil::CreateFromPoints( result, pParticle->pos, pParticle->vel + pParticle->pos );
}

//--------------------------------------------------
//!
//!	ParticleSpawner::MatrixFromPointUpAndForwards
//! construct a matrix from 3 inputs
//!
//--------------------------------------------------
void ParticleSpawner::MatrixFromPointUpAndForwards( const CPoint& point, const CDirection& up,
													const CDirection& forwards, CMatrix& result )
{
	result.SetIdentity();
	CDirection xaxis = up.Cross( forwards );

	if( xaxis.Normalise() )
		result.SetXAxis( CVecMath::GetXAxis() ); 
	else
		result.SetXAxis( xaxis );

	result.SetYAxis( up );
	result.BuildZAxis();		
	result.SetTranslation( point );
}

//--------------------------------------------------
//!
//!	ParticleSpawner::BounceNotification
//! informed we have a bounce
//!
//--------------------------------------------------
void ParticleSpawner::BounceNotification( pos_vel_PD* pJustBounced, const CPoint& intersect, const CDirection& normal )
{
	ntAssert(m_particles.find(pJustBounced) != m_particles.end());
	ParticleInfo& extraInfo = m_pAdditionalInfo[ m_particles[pJustBounced] ];

	if (!m_pDef->m_bKillParticleOnBounce)
	{
		// normal bounce behaviour
		if	(
			(m_pDef->HasBounceEffects()) && 
			(extraInfo.iBounceCount > 0) &&
			(!extraInfo.bTriggeredDeath)
			)
		{
			CMatrix frame;
			MatrixFromPointUpAndForwards( intersect, normal, pJustBounced->vel, frame );

			FireEffectList( m_pDef->m_bAttatchBounceEffects, m_pDef->m_bCleanupBounceEffects,
							m_pDef->m_toSpawnAtBounce, frame, extraInfo );

			extraInfo.iBounceCount--;
		}
	}
	else 
	{
		// kill the particle
		pJustBounced->ageN = 1.0f;
		
		// trigger death effect instead
		if	(
			(m_pDef->HasDeathEffects()) &&
			(!extraInfo.bTriggeredDeath)
			)
		{
			CMatrix frame( CONSTRUCT_IDENTITY );
			MatrixFromPointUpAndForwards( intersect, normal, pJustBounced->vel, frame );

			FireEffectList( m_pDef->m_bAttatchDeathEffects, false,
							m_pDef->m_toSpawnAtDeath, frame, extraInfo );

			extraInfo.bTriggeredDeath = true;
		}

		// remove us from the list of valid particles
		ReleaseParticle( m_particles.find(pJustBounced) );

		// make sure we have the correct setup here
		if ( m_pDef->m_bCleanupDeathEffects )
		{
			#ifndef _RELEASE
			static char aErrors[MAX_PATH];
			sprintf( aErrors, "Cannot cleanup death effect if KillParticleOnBounce is true. See %s.",  ntStr::GetString(ObjectDatabase::Get().GetNameFromPointer( m_pDef )) );
			EffectErrorMSG::AddDebugError( aErrors );
			#endif
		}
	}
}

//--------------------------------------------------
//!
//!	ParticleSpawner::Update
//! Track death of particles, update relevant transforms.
//!
//--------------------------------------------------
void ParticleSpawner::Update()
{
	for ( ParticleTable::iterator it = m_particles.begin(); it != m_particles.end();  )
	{
		ParticleInfo& extraInfo = m_pAdditionalInfo[it->second];
		const pos_vel_PD* pParticle = it->first;

		if	(
			( pParticle->ageN >= extraInfo.fDeathtime ) &&
			( !extraInfo.bTriggeredDeath )
			)
		{
			// first kill any that are still attached
			extraInfo.Clean();

			// now trigger death effects
			CMatrix frame( CONSTRUCT_IDENTITY );
			frame.SetTranslation( pParticle->pos );

			FireEffectList( m_pDef->m_bAttatchDeathEffects, m_pDef->m_bCleanupDeathEffects,
							m_pDef->m_toSpawnAtDeath, frame, extraInfo );

			extraInfo.bTriggeredDeath = true;
		}

		if ( pParticle->ageN >= 1.0f )
		{
			// just died completely, kill any attached effects and remove from the list
			it = ReleaseParticle( it );
		}
		else
		{
			// update the transform for this frame
			if ( m_pDef->HasAttachedEffects() )
			{
				CMatrix frame;
				MatrixFromParticle( pParticle, frame );
				extraInfo.transform.SetLocalMatrix(frame);
			}
			
			++it;
		}
	}
}
