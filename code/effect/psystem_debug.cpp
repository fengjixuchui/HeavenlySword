//--------------------------------------------------
//!
//!	\file psystem_debug.cpp
//!	debug singleton that captures info on particle usage
//!
//--------------------------------------------------

#include "psystem_debug.h"
#include "effect_manager.h"
#include "psystem_simple.h"
#include "psystem_complex.h"
#include "input/inputhardware.h"
#include "core/visualdebugger.h"

//--------------------------------------------------
//!
//!	PSystemDebugMan::DebugRender
//! Gather profiling info and render it
//!
//--------------------------------------------------
void PSystemDebugMan::DebugRender()
{
#ifdef USE_PSYSTEM_DEBUGMAN

	if (CInputHardware::Get().GetKeyboard().IsKeyPressed( KEYC_TAB, KEYM_CTRL ))
	{
		m_bDebugRender = !m_bDebugRender;
	}

	m_simpleUpdateTimer.StopNoPause();
	m_functionalUpdateTimer.StopNoPause();

	if (m_bDebugRender)
	{
		// build up profiling info
		//--------------------------------
		class particleProfile
		{
		public:
			particleProfile() :
				iNumParticles(0),
				iNumEmitters(0)
			{}	
			int iNumParticles;
			int iNumEmitters;
		};

		int iNumCPUbased_Simple = 0;
		int iNumGPUbased_Simple = 0;
		int iNumCPUbased_Functional = 0;
		int iNumGPUbased_Functional = 0;

		const EffectManager::EffectPtrList& HDRRendered = EffectManager::Get().GetHDRRenderList();
		const EffectManager::EffectPtrList& LDRRendered = EffectManager::Get().GetLDRRenderList();

		// get info on simple particle systems
		//-----------------------------------
		particleProfile	simpleRendered;
		particleProfile	simpleCulled;
		ntstd::Vector<PSystemSimple*> renderedSimple;

		for (	ntstd::List<PSystemSimple*>::iterator it = m_simpleEmitters.begin();
				it != m_simpleEmitters.end(); ++it )
		{
			if ((*it)->GetParticles().UsingCPUParticles())
				iNumCPUbased_Simple += (*it)->GetMaxParticles();
			else
				iNumGPUbased_Simple += (*it)->GetMaxParticles();

			Effect* pBaseEffect = static_cast<Effect*>(*it);
			
			if	(
				( HDRRendered.end() == ntstd::find( HDRRendered.begin(), HDRRendered.end(), pBaseEffect ) )&&
				( LDRRendered.end() == ntstd::find( LDRRendered.begin(), LDRRendered.end(), pBaseEffect ) )
				)
			{
				// wasnt renderd last frame
				simpleCulled.iNumParticles += (*it)->GetMaxParticles();
				simpleCulled.iNumEmitters++;
			}
			else
			{
				// was renderd last frame
				simpleRendered.iNumParticles += (*it)->GetMaxParticles();
				simpleRendered.iNumEmitters++;
				renderedSimple.push_back( *it );
			}
		}

		// get info on complex particle systems
		//-----------------------------------
		particleProfile	functionalRendered;
		particleProfile	functionalCulled;
		ntstd::Vector<PSystemComplex*> renderedFunctional;

		for (	ntstd::List<PSystemComplex*>::iterator it = m_functionalEmitters.begin();
				it != m_functionalEmitters.end(); ++it )
		{
			if ((*it)->GetParticles().UsingCPUParticles())
				iNumCPUbased_Functional += (*it)->GetMaxParticles();
			else
				iNumGPUbased_Functional += (*it)->GetMaxParticles();

			Effect* pBaseEffect = static_cast<Effect*>(*it);
			
			if	(
				( HDRRendered.end() == ntstd::find( HDRRendered.begin(), HDRRendered.end(), pBaseEffect ) ) &&
				( LDRRendered.end() == ntstd::find( LDRRendered.begin(), LDRRendered.end(), pBaseEffect ) )
				)
			{
				// wasnt renderd last frame
				functionalCulled.iNumParticles += (*it)->GetMaxParticles();
				functionalCulled.iNumEmitters++;
			}
			else
			{
				// was renderd last frame
				functionalRendered.iNumParticles += (*it)->GetMaxParticles();
				functionalRendered.iNumEmitters++;
				renderedFunctional.push_back( *it );
			}
		}

		// force the debug render of all visible particle systems
		//----------------------------------------------------------
		if (renderedSimple.size())
		{
			ntstd::sort(	&renderedSimple[0], &renderedSimple[0] + renderedSimple.size(),
				Effects::comparatorFurtherThan() );
		}

		if (renderedFunctional.size())
		{
			ntstd::sort(	&renderedFunctional[0], &renderedFunctional[0] + renderedFunctional.size(),
				Effects::comparatorFurtherThan() );
		}

		u_int iCurrSimple = 0;
		u_int iCurrFunctional = 0;
		int iMaxTextRender = 20;
		int iNoTextRender = ntstd::Max( 0, (int)((renderedSimple.size() + renderedFunctional.size())) - iMaxTextRender );

		while	(
				(iCurrSimple < renderedSimple.size()) ||
				(iCurrFunctional < renderedFunctional.size())
				)
		{
			bool bSimpleLeft = (iCurrSimple < renderedSimple.size()) ? true : false;
			bool bFunctionalLeft = (iCurrFunctional < renderedFunctional.size()) ? true : false;

			if (iNoTextRender)
			{
				// render all the furthest ones away without text
				if (bSimpleLeft && bFunctionalLeft)
				{
					if (renderedSimple[iCurrSimple]->GetSortingDistance() >
						renderedFunctional[iCurrFunctional]->GetSortingDistance())
						renderedSimple[iCurrSimple++]->DebugRender( false, false );
					else
						renderedFunctional[iCurrFunctional++]->DebugRender( false, false );
				}
				else if (bSimpleLeft)
					renderedSimple[iCurrSimple++]->DebugRender( false, false );
				else
					renderedFunctional[iCurrFunctional++]->DebugRender( false, false );

				iNoTextRender--;
			}
			else if (bSimpleLeft)
				renderedSimple[iCurrSimple++]->DebugRender( false, true );
			else
				renderedFunctional[iCurrFunctional++]->DebugRender( false, true );
		}

		// wack up some totals
		//----------------------------------------------------------
		float fStartX = 10.0f;
		float fStartY = 100.0f;

		int iTotalRenderedParticles = functionalRendered.iNumParticles + simpleRendered.iNumParticles;
		int iTotalActiveParticles = iTotalRenderedParticles + functionalCulled.iNumParticles + simpleCulled.iNumParticles;

		int iTotalCPUParticles = iNumCPUbased_Simple + iNumCPUbased_Functional;
		int iTotalGPUParticles = iNumGPUbased_Simple + iNumGPUbased_Functional;

		ntError((iTotalGPUParticles+iTotalCPUParticles) == iTotalActiveParticles);

		int iTotalRenderedEmitters = functionalRendered.iNumEmitters + simpleRendered.iNumEmitters;
		int iTotalActiveEmitters = iTotalRenderedEmitters + functionalCulled.iNumEmitters + simpleCulled.iNumEmitters;

		g_VisualDebug->Printf2D( fStartX, fStartY, 0xffffffff, 0,
										"Total Rendered Particles:  %d (simple:%d, functional:%d)",
										iTotalRenderedParticles, 
										simpleRendered.iNumParticles,
										functionalRendered.iNumParticles );
		fStartY += 12.0f;
		
		g_VisualDebug->Printf2D( fStartX, fStartY, 0xffffffff, 0,
										"Total Active Particles:    %d (simple:%d, functional:%d)",
										iTotalActiveParticles, 
										simpleRendered.iNumParticles + simpleCulled.iNumParticles,
										functionalRendered.iNumParticles + functionalCulled.iNumParticles );		
		fStartY += 12.0f;

		g_VisualDebug->Printf2D( fStartX, fStartY, 0xffffffff, 0,
										"Total CPU based Particles: %d (simple:%d, functional:%d)",
										iTotalCPUParticles, 
										iNumCPUbased_Simple,
										iNumCPUbased_Functional );		
		fStartY += 12.0f;
	
		g_VisualDebug->Printf2D( fStartX, fStartY, 0xffffffff, 0,
										"Total GPU based Particles: %d (simple:%d, functional:%d)",
										iTotalGPUParticles, 
										iNumGPUbased_Simple,
										iNumGPUbased_Functional );		
		fStartY += 12.0f;	
		g_VisualDebug->Printf2D( fStartX, fStartY, 0xffffffff, 0,
										"Total visible psystems:    %d ", iTotalRenderedEmitters );

		fStartY += 12.0f;
		g_VisualDebug->Printf2D( fStartX, fStartY, 0xffffffff, 0,
										"Total active psystems:     %d ", iTotalActiveEmitters );

		float fSimpleUpdate = m_simpleUpdateTimer.GetFrameTime();
		float fFunctionalUpdate = m_functionalUpdateTimer.GetFrameTime();

		fStartY += 12.0f;
		g_VisualDebug->Printf2D( fStartX, fStartY, 0xffffffff, 0,
										"Total simple update cost:     %5.1f%%", fSimpleUpdate );

		fStartY += 12.0f;
		g_VisualDebug->Printf2D( fStartX, fStartY, 0xffffffff, 0,
										"Total functional update cost: %5.1f%%", fFunctionalUpdate );

	}
#endif
}
