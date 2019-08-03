//--------------------------------------------------
//!
//!	\file psystem_debug.h
//!	debug singleton that captures info on particle usage
//!
//--------------------------------------------------

#ifndef _PSYSTEM_DEBUG_H
#define _PSYSTEM_DEBUG_H

#include "core/profiling.h"

#include "psystem_simple.h"
#include "psystem_complex.h"

#define USE_PSYSTEM_DEBUGMAN

#ifdef	_RELEASE
#ifndef _PROFILING
#undef USE_PSYSTEM_DEBUGMAN
#endif
#endif

//--------------------------------------------------
//!
//!	PSystemDebugMan
//! Gathers debug information about particle systems
//!
//--------------------------------------------------
class PSystemDebugMan : public Singleton<PSystemDebugMan>
{
public:
	PSystemDebugMan() : m_bDebugRender(false) {};

	void	Reset()
	{
		m_simpleEmitters.clear();
		m_functionalEmitters.clear();
	}

	void	DebugRender();

	static void	Register(PSystemSimple* pEffect)
	{
		UNUSED(pEffect);
		#ifdef USE_PSYSTEM_DEBUGMAN
		Get().m_simpleEmitters.push_back(pEffect);
		#endif
	}

	static void	Register(PSystemComplex* pEffect)
	{
		UNUSED(pEffect);
		#ifdef USE_PSYSTEM_DEBUGMAN
		Get().m_functionalEmitters.push_back(pEffect);
		#endif
	}

	static void	Release(PSystemSimple* pEffect)
	{
		UNUSED(pEffect);
		#ifdef USE_PSYSTEM_DEBUGMAN
		ntstd::List<PSystemSimple*>::iterator it = ntstd::find( Get().m_simpleEmitters.begin(), Get().m_simpleEmitters.end(), pEffect );
		ntError_p( it != Get().m_simpleEmitters.end(), ("Effect not registered with DebugMan\n") );
		Get().m_simpleEmitters.erase( it );
		#endif
	}

	static void	Release(PSystemComplex* pEffect)
	{
		UNUSED(pEffect);
		#ifdef USE_PSYSTEM_DEBUGMAN
		ntstd::List<PSystemComplex*>::iterator it = ntstd::find( Get().m_functionalEmitters.begin(), Get().m_functionalEmitters.end(), pEffect );
		ntError_p( it != Get().m_functionalEmitters.end(), ("Effect not registered with DebugMan\n") );
		Get().m_functionalEmitters.erase( it );
		#endif
	}

	static void StartSimpleUpdateTimer()
	{
		#ifdef USE_PSYSTEM_DEBUGMAN
		Get().m_simpleUpdateTimer.ResumeOrStart();
		#endif
	}

	static void	StopSimpleUpdateTimer()
	{
		#ifdef USE_PSYSTEM_DEBUGMAN
		Get().m_simpleUpdateTimer.Pause();
		#endif
	}

	static void StartFunctionalUpdateTimer()
	{
		#ifdef USE_PSYSTEM_DEBUGMAN
		Get().m_functionalUpdateTimer.ResumeOrStart();
		#endif
	}

	static void	StopFunctionalUpdateTimer()
	{
		#ifdef USE_PSYSTEM_DEBUGMAN
		Get().m_functionalUpdateTimer.Pause();
		#endif
	}

	static void KillAll(PSystemSimpleDef* simpleDef)
	{
		UNUSED(simpleDef);
		#ifdef USE_PSYSTEM_DEBUGMAN
		for (ntstd::List<PSystemSimple*>::iterator it = Get().m_simpleEmitters.begin(); it != Get().m_simpleEmitters.end(); it++)
		{
			if (&((*it)->GetDefinition()) == simpleDef)
			{
				(*it)->KillMeNow();
			}
		}
		#endif
	}

	static void KillAll(const PSystemComplexDef* complexDef)
	{
		UNUSED(complexDef);
		#ifdef USE_PSYSTEM_DEBUGMAN
		for (ntstd::List<PSystemComplex*>::iterator it = Get().m_functionalEmitters.begin(); it != Get().m_functionalEmitters.end(); it++)
		{
			if (&((*it)->GetDefinition()) == complexDef)
			{
				(*it)->KillMeNow();
			}
		}
		#endif
	}

private:
	ntstd::List<PSystemSimple*>	 m_simpleEmitters;
	ntstd::List<PSystemComplex*> m_functionalEmitters;
	bool	m_bDebugRender;

	CEstimateTimer	m_simpleUpdateTimer;
	CEstimateTimer	m_functionalUpdateTimer;
};

#endif
