//--------------------------------------------------
//!
//!	\file game/entityparticleemitter.h
//!	Definition of the basic Particle Emitter object
//!
//--------------------------------------------------

#ifndef	_ENTITY_PARTICLE_EMITTER_H
#define	_ENTITY_PARTICLE_EMITTER_H

#include "game/entity.h"
#include "game/entity.inl"

#include "fsm.h"


//--------------------------------------------------
//!
//! Class Object_ParticleEmitter.
//! A particle emitter object type
//!
//--------------------------------------------------
class Object_ParticleEmitter : public CEntity
{
	// Declare dataobject interface
	HAS_INTERFACE(Object_ParticleEmitter)

public:
	// Constructor
	Object_ParticleEmitter();

	// Destructor
	~Object_ParticleEmitter();

	// Post Construct
	void OnPostConstruct();

	// Public variables (so they can be accessed easily by the FSM
	u_int		m_uFXID;	// ID of the particle effect instance

	// Particle effect for the emitter
	CHashedString	m_ParticleFX;
	CMatrix			m_PosMatrix;

protected:
	// Object description
	CKeyString	m_Description;

	// Initial State
	bool		m_bActive;

};


#endif // _ENTITY_PARTICLE_EMITTER_H

