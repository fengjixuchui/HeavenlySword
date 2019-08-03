//--------------------------------------------------
//!
//!	\file particle_wake.h
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#ifndef _PARTICLE_WAKE_H
#define _PARTICLE_WAKE_H

#include "effect.h"

class Transform;
class EmitterDef;

//--------------------------------------------------
//!
//!	ParticleWake
//!	Class that manages a transform used by a particle
//! system. The transform is mapped to nearest ground
//! collision volume via a ray-check, within a certain
//! tolerance.
//!
//--------------------------------------------------
class ParticleWake : public Effect
{
public:
	ParticleWake(	const Transform* pSource,
					void* pPSystemDef,
					const EmitterDef* pEmitterOveridef,
					float fFullEmitDistance,
					float fNoEmitDistance,
					const CPoint& offset );
	~ParticleWake();

	virtual bool UpdateEffect();
	virtual bool WaitingForResources() const { return false; }

private:
	void UpdateMangedTransform( bool bFirstFrame = false );
	const Transform*	m_pSourceTransform;
	Transform*			m_pManagedTransform;	u_int				m_iManagedEffect;
	float				m_fDistanceToHit;
	float				m_fFullEmitDistance;
	float				m_fNoEmitDistance;
	CPoint				m_offset;
};

#endif // _PARTICLE_WAKE_H
