//--------------------------------------------------
//!
//!	\file psystem_complex.h
//!	declaration of complex particle effect and its
//! definition object
//!
//--------------------------------------------------

#ifndef _PSYSTEM_COMPLEX_H
#define _PSYSTEM_COMPLEX_H

#include "psystem.h"
#include "particle_complex.h"
#include "effect/effect_resourceman.h"

class Particle_AdvancedMovement;
class DataInterfaceField;

//--------------------------------------------------
//!
//!	PSystemComplexResources
//!
//--------------------------------------------------
class PSystemComplexResources : public EffectResource
{
public:
	PSystemComplexResources();
	virtual ~PSystemComplexResources();
	
	virtual void GenerateResources();
	virtual bool ResourcesOutOfDate() const;

	// these are all exposed to welder via the parent PSystemComplexDef object
	FunctionCurve_User*		m_pSizeMin;
	FunctionCurve_User*		m_pSizeMax;
	ColourFunction*			m_pPalette;

	const Texture::Ptr&		GetPalette() const				{ return m_texPalette; }
	const Texture::Ptr&		GetSizeFunctionTex() const		{ return m_sizeFunctions.GetFunctionTex(); }
	const FunctionSampler&	GetSizeFunctionTable() const	{ return m_sizeFunctions.GetFunctionTable(); }

	void SetParent( const PSystemComplexDef* pParent ) { m_pParent = pParent; }

	bool HasCPUParticleResources() const { return m_bCPUResources; }

private:
	const PSystemComplexDef*	m_pParent;
	Texture::Ptr				m_texPalette;			// auto gen'd palette
	FunctionObject				m_sizeFunctions;		// auto gen'd size funcs
	bool						m_bCPUResources;
};

//--------------------------------------------------
//!
//!	PSystemComplexDef
//!
//--------------------------------------------------
class PSystemComplexDef : public ParticleSystemStdParams
{
public:
	// extra params
	//----------------------------------------
	PSystemComplexResources		m_resources;
	Particle_AdvancedMovement*	m_pAdvMovement;

	ntstd::List<void*>	m_obObjects;	// storage for auto created thingumys

	// functions
	//----------------------------------------
	PSystemComplexDef();
	virtual void PostConstruct( void );
	virtual bool EditorChangeValue( CallBackParameter, CallBackParameter );
	virtual const char* GetDebugName() const;

	void AutoConstruct( const DataInterfaceField* pField );

	// Do we require advanced movement code?
	bool RequiresAdvMovement() const { return (m_pAdvMovement != NULL); }

	// Do we need to be updated on the CPU or GPU
	virtual bool RequiresCPUParticle() const
	{
		if	(
			(ParticleSystemStdParams::RequiresCPUParticle()) ||
			(RequiresAdvMovement())
			)
			return true;
		return false;
	}
};

//--------------------------------------------------
//!
//!	PSystemComplex
//!
//--------------------------------------------------
class PSystemComplex : public ParticleSystem
{
public:
	PSystemComplex( const PSystemComplexDef& def, const CMatrix& frame, const EmitterDef* pOverideEmit = 0 );
	PSystemComplex( const PSystemComplexDef& def, const Transform& transform, const EmitterDef* pOverideEmit = 0 );
	virtual ~PSystemComplex();

	// virtuals from Effect interface
	//--------------------------------------------------
	virtual bool UpdateEffect();
	virtual void RenderEffect();
	virtual bool WaitingForResources() const;

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
	const ParticleHandlerComplex& GetParticles() const { return *m_pParticles; }
	const PSystemComplexDef& GetDefinition() const { return *m_pDefinition; }

	// for debug info only
	void DebugRender( bool bEditing, bool bText );

private:
	const PSystemComplexDef*	m_pDefinition;
	ParticleHandlerComplex*		m_pParticles;
};

#endif //_PSYSTEM_COMPLEX_H
