//--------------------------------------------------
//!
//!	\file psystem_simple.h
//!	declaration of simple particle effect and its
//! definition object
//!
//--------------------------------------------------

#ifndef _PSYSTEM_SIMPLE_H
#define _PSYSTEM_SIMPLE_H

#include "psystem.h"
#include "particle_simple.h"

//--------------------------------------------------
//!
//!	PSystemSimpleDef
//!
//--------------------------------------------------
class PSystemSimpleDef : public ParticleSystemStdParams
{
public:
	// extra params
	//----------------------------------------
	float	m_fSizeStartMin;
	float	m_fSizeStartMax;
	float	m_fSizeEndMin;
	float	m_fSizeEndMax;

	CVector	m_colStart;
	CVector	m_colEnd;
	float	m_fAlphaStart;
	float	m_fAlphaEnd;

	CDirection	GetColourStart() const { return CDirection(	m_colStart.X() * m_colStart.W(),
															m_colStart.Y() * m_colStart.W(),
															m_colStart.Z() * m_colStart.W() ); }

	CDirection	GetColourEnd() const { return CDirection(	m_colEnd.X() * m_colEnd.W(),
															m_colEnd.Y() * m_colEnd.W(),
															m_colEnd.Z() * m_colEnd.W() ); }
	// functions
	//----------------------------------------
	PSystemSimpleDef();
	virtual void PostConstruct();
	virtual bool EditorChangeValue( CallBackParameter, CallBackParameter );
	virtual const char* GetDebugName() const;
};

//--------------------------------------------------
//!
//!	PSystemSimple
//!
//--------------------------------------------------
class PSystemSimple : public ParticleSystem
{
public:
	PSystemSimple( const PSystemSimpleDef& def, const CMatrix& frame, const EmitterDef* pOverideEmit = 0 );
	PSystemSimple( const PSystemSimpleDef& def, const Transform& transform, const EmitterDef* pOverideEmit = 0 );
	virtual ~PSystemSimple();

	// virtuals from Effect interface
	//--------------------------------------------------
	virtual bool UpdateEffect();
	virtual void RenderEffect();
	virtual bool WaitingForResources() const { return false; }

	// virtuals from ParticleSystem interface
	//--------------------------------------------------
	virtual void Reset( bool bInDestructor );
	
	// wrapper around our particle handler update function
	virtual void UpdateParticles( float fTimeChange )
	{
		m_pParticles->Update( fTimeChange, fTimeChange / m_pDefinition->m_fParticleLifetime );
	}

	// wrapper around particle hander emission function
	virtual pos_vel_PD* SpawnNewParticle( float fCurrTime, float fBirthTime, const CPoint& pos, const CDirection& vel )
	{
		return m_pParticles->EmitParticle( fCurrTime, fBirthTime, pos, vel );
	}

	// accesors
	const ParticleHandlerSimple& GetParticles() const { return *m_pParticles; }
	const PSystemSimpleDef& GetDefinition() const { return *m_pDefinition; }

	// for debug info only
	void DebugRender( bool bEditing, bool bText );

private:
	const PSystemSimpleDef*	m_pDefinition;
	ParticleHandlerSimple*	m_pParticles;
};

#endif //_PSYSTEM_SIMPLE_H
