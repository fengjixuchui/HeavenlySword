//--------------------------------------------------
//!
//!	\file particle_wake.cpp
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#include "particle_wake.h"
#include "effect/psystem_utils.h"
#include "psystem.h"
#include "anim/transform.h"
#include "effect_manager.h"

#include "physics/world.h"

//--------------------------------------------------
//!
//!	ParticleWake::ctor
//!
//--------------------------------------------------
ParticleWake::ParticleWake(	const Transform* pSource,
							void* pPSystemDef,
							const EmitterDef* pEmitterOveridef,
							float fFullEmitDistance,
							float fNoEmitDistance,
							const CPoint& offset ) :
	m_pSourceTransform( pSource ),
	m_pManagedTransform( 0 ),
	m_iManagedEffect( 0xffffffff ),
	m_offset( offset )
{
	ntAssert(m_pSourceTransform);
	m_pManagedTransform = NT_NEW_CHUNK ( Mem::MC_EFFECTS ) Transform;

	m_fFullEmitDistance = fFullEmitDistance;
	m_fNoEmitDistance = fNoEmitDistance;

	UpdateMangedTransform( true );
	m_iManagedEffect = PSystemUtils::ConstructParticleEffect( pPSystemDef, m_pManagedTransform, pEmitterOveridef );
}

//--------------------------------------------------
//!
//!	ParticleWake::dtor
//!
//--------------------------------------------------
ParticleWake::~ParticleWake()
{
	NT_DELETE_CHUNK( Mem::MC_EFFECTS, m_pManagedTransform );
}

//--------------------------------------------------
//!
//!	ParticleWake::UpdateEffect
//!
//--------------------------------------------------
bool ParticleWake::UpdateEffect()
{
	Effect* pEffect = EffectManager::Get().GetEffect( m_iManagedEffect );

	// If I'm meant to be killed, kill me
	if (m_bKillMeNow)
	{
		if (pEffect)
			pEffect->KillMeNow();
		return true;
	}
	
	UpdateMangedTransform();

	// at this point we may need to modulate the alpha of
	// our managed effect according to the distance our
	// src transform is from the ground
	if (pEffect)
	{
		float fEmitMultiplier = 0.0f;
		if (m_fDistanceToHit < m_fFullEmitDistance)
			fEmitMultiplier = 1.0f;

		if (m_fDistanceToHit < m_fNoEmitDistance)
		{
			float fDist = m_fNoEmitDistance - m_fFullEmitDistance;
			if (fabsf(fDist) > EPSILON)
				fEmitMultiplier = 1.0f - ((m_fDistanceToHit - m_fFullEmitDistance) / fDist);
		}

		ParticleSystem* pPSystem = static_cast<ParticleSystem*>(pEffect);
		ntAssert(pPSystem);

		pPSystem->SetEmitMultiplier(fEmitMultiplier);
	}

	if ((m_bKillMeRequested) && (pEffect))
		pEffect->KillMeWhenReady();

	if ((!pEffect) || (m_bKillMeNow) || (m_bKillMeRequested))
		return true;

	return false;
}

//--------------------------------------------------
//!
//!	ParticleWake::UpdateMangedTransform
//!
//--------------------------------------------------
void ParticleWake::UpdateMangedTransform( bool bFirstFrame )
{
	float fRayLength = m_fNoEmitDistance * 2.0f;
	CPoint srcPos = m_offset * m_pSourceTransform->GetWorldMatrix();
	CDirection dir( 0.0f, -fRayLength, 0.0f );

	CDirection	hitNormal;
	CPoint		hitPos;

	float fHitFraction;

	Physics::RaycastCollisionFlag flag;
	flag.base = 0;

	// [Mus] - What settings for this cast ?
	flag.flags.i_am = Physics::STATIC_ENVIRONMENT_BIT;
	flag.flags.i_collide_with = ( Physics::LARGE_INTERACTABLE_BIT );

	if ( Physics::CPhysicsWorld::Get().GetClosestIntersectingSurfaceDetails( srcPos, srcPos+dir, fHitFraction, hitNormal, flag ))
	{
		// hitNormal has been filled in
		hitPos = srcPos + (dir * fHitFraction);
		m_fDistanceToHit = m_fNoEmitDistance * fHitFraction;
	}
	else 
	{
		if (bFirstFrame)
		{
			hitNormal = CDirection( 0.0f, 1.0f, 0.0f );
			hitPos = srcPos + dir;
			m_fDistanceToHit = fRayLength;
		}
		else
		{
			hitNormal = m_pManagedTransform->GetLocalMatrix().GetYAxis();
			hitPos = srcPos;
			hitPos.Y() = m_pManagedTransform->GetLocalMatrix().GetTranslation().Y();
			m_fDistanceToHit = fRayLength;
		}
	}

	// now contsruct new local matrix for managed transform	
	CMatrix mat( CONSTRUCT_CLEAR );
	mat.SetYAxis( hitNormal );
	mat.SetZAxis( m_pSourceTransform->GetWorldMatrix().GetZAxis() );
	mat.BuildXAxis();
	mat.BuildZAxis();
	mat.SetTranslation( hitPos );

	m_pManagedTransform->SetLocalMatrix( mat );
}
