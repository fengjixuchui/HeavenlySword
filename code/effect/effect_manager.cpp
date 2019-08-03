//--------------------------------------------------
//!
//!	\file effect_manager.cpp
//!	Singleton that owns, updates and renders all
//! Effect derived objects in the game.
//!
//--------------------------------------------------

#include "effect/effect_manager.h"
#include "effect/effect_resourceman.h"
#include "psystem_debug.h"
#include "gfx/textureatlas.h"
#include "gfx/rendercontext.h"
#include "gfx/renderer.h"
#include "game/shellmain.h"
#include "core/frustum.h"
#include "core/gatso.h"
#include "core/visualdebugger.h"

bool EffectManager::s_bAllocationsDisabled = false;
bool EffectManager::s_bRenderDisabled = false;

//--------------------------------------------------
//!
//!	EffectManager ctor
//!
//--------------------------------------------------
EffectManager::EffectManager()
{
	m_iCurrID = 1;
	
	// [scee_st] Strictly we can't chunk these unless we change Singleton,
	// but since NT_DELETE ignores the chunk type, I can cheat.
	NT_NEW_CHUNK ( Mem::MC_EFFECTS ) EffectResourceMan;
	NT_NEW_CHUNK ( Mem::MC_EFFECTS ) PSystemDebugMan;
	NT_NEW_CHUNK ( Mem::MC_EFFECTS ) TextureAtlasManager;
}

//--------------------------------------------------
//!
//!	EffectManager dtor
//!
//--------------------------------------------------
EffectManager::~EffectManager()
{
	Reset();
	EffectResourceMan::Kill();
	PSystemDebugMan::Kill();
	TextureAtlasManager::Kill();
}

//--------------------------------------------------
//!
//!	EffectManager::Reset()
//! clear out our lists
//!
//--------------------------------------------------
void EffectManager::Reset()
{
	ScopedCritical crit( m_CriticalSection );

	for( EffectTable::iterator curr = m_effects.begin(); curr != m_effects.end(); )
	{
		NT_DELETE_CHUNK( Mem::MC_EFFECTS, curr->second );
		curr = m_effects.erase( curr );
	}

	PSystemDebugMan::Get().Reset();
	TextureAtlasManager::Get().Reset();

	m_transientOpaques.clear();
	m_transientAlphas.clear();

	m_sortedHDR.clear();
	m_sortedLDR.clear();
}

//--------------------------------------------------
//!
//!	EffectManager::AddEffect
//! Add a regular effect to the manager
//!
//--------------------------------------------------
u_int EffectManager::AddEffect( Effect* pNewEffect )
{
	ScopedCritical crit( m_CriticalSection );

	ntError(pNewEffect);
	u_int ID = AssignEffectID(pNewEffect);
	m_miscEffects.push_back(ID);
	return ID;
}

//--------------------------------------------------
//!
//!	EffectManager::AddSortableEffect
//! Add a world space effect that needs sorting to the manager
//!
//--------------------------------------------------
u_int EffectManager::AddSortableEffect( SortableEffect* pNewEffect )
{
	ScopedCritical crit( m_CriticalSection );

	ntError(pNewEffect);
	u_int ID = AssignEffectID(pNewEffect);
	m_worldEffects.push_back(ID);
	return ID;
}



//--------------------------------------------------
//!
//!	Update effects.
//!
//--------------------------------------------------
void EffectManager::UpdateManager()
{
	ScopedCritical crit( m_CriticalSection );

	// debug update resources
	if ( ShellMain::Get().IsLoadingAsync() == false )	
		EffectResourceMan::Get().RefreshResources();

	CGatso::Start( "EffectManager::UpdateManager" );

	// update effects
	for	(EffectTable::iterator curr = m_effects.begin(); curr != m_effects.end(); )
	{
		if	(
			( curr->second->WaitingForResources() == false ) &&
			( curr->second->UpdateEffect() )
			)
		{
			NT_DELETE_CHUNK( Mem::MC_EFFECTS, curr->second );
			curr = m_effects.erase( curr );
		}
		else
		{
			++curr;
		}
	}

	CGatso::Stop( "EffectManager::UpdateManager" );
}

//--------------------------------------------------
//!
//!	GetEffect
//! Find an effect if it exists
//!
//--------------------------------------------------
Effect*	EffectManager::GetEffect( u_int iEffectID )
{
	ScopedCritical crit( m_CriticalSection );

	EffectTable::iterator it = m_effects.find( iEffectID );
	if (it != m_effects.end())
		return it->second;
	
	return NULL;
}

//--------------------------------------------------
//!
//!	KillEffectNow
//! Find an effect and kill it, if it exists
//!
//--------------------------------------------------
bool EffectManager::KillEffectNow( u_int iEffectID )
{
	ScopedCritical crit( m_CriticalSection );

	Effect* pEffect = GetEffect(iEffectID);

	if (pEffect)
	{
		m_effects.erase( iEffectID );
		NT_DELETE_CHUNK( Mem::MC_EFFECTS, pEffect );
		return true;
	}

	return false;
}

//--------------------------------------------------
//!
//!	KillEffectWhenReady
//! Find an effect and tell it to die when its ready
//!
//--------------------------------------------------
bool EffectManager::KillEffectWhenReady( u_int iEffectID )
{
	ScopedCritical crit( m_CriticalSection );

	Effect* pEffect = GetEffect(iEffectID);

	if (pEffect)
	{
		pEffect->KillMeWhenReady();
		return true;
	}

	return false;
}

//--------------------------------------------------
//!
//!	AssignEffectID
//! Store pointer to effect and return it's ID
//!
//--------------------------------------------------
u_int	EffectManager::AssignEffectID( Effect* pEffect )
{
	u_int iReturn = m_iCurrID;
	m_effects[m_iCurrID] = pEffect;
	m_iCurrID++;
	return iReturn;
}

//--------------------------------------------------
//!
//!	PreRender
//! Generates renderable lists based on bounding volumes
//! and sort considerations such as z writing and 
//! render type
//!
//--------------------------------------------------
void EffectManager::PreRender( const CFrustum* pFrustum )
{
	ScopedCritical crit( m_CriticalSection );

	if (RenderDisabled())
		return;

	CGatso::Start( "EffectManager::PreRender" );

	m_transientOpaques.clear();
	m_transientAlphas.clear();

	m_sortedHDR.clear();
	m_sortedLDR.clear();

	SortablePtrList	visibles;
	
	// build a list of all visible world effects
	//------------------------------------------------------------
	CMatrix transform( CONSTRUCT_IDENTITY );
	CMatrix cullTransform( RenderingContext::Get()->m_worldToView );
	CPoint camPos = RenderingContext::Get()->GetEyePos();

	for	(EffectIDList::iterator curr = m_worldEffects.begin(); curr != m_worldEffects.end(); )
	{
		Effect* pEffect = GetEffect(*curr);

		if (pEffect)
		{
			// we know that this effect is a SortableEffect, so this upcast is legit
			SortableEffect* pWorldEffect = static_cast<SortableEffect*>(pEffect);

			if	(
				(pWorldEffect->Invisible() == false) && 
				(pWorldEffect->WaitingForResources() == false)
				)
			{
				transform.SetTranslation( pWorldEffect->GetCullingOrigin() );
				CDirection halfLengths( pWorldEffect->GetCullingRadius(), 
										pWorldEffect->GetCullingRadius(), 
										pWorldEffect->GetCullingRadius() );

				if ( pFrustum->IntersectsOBB( transform * cullTransform, halfLengths ) )
				{
					pWorldEffect->CalcSortingDistanceTo( camPos ); // cache distance to camera
					visibles.push_back( pWorldEffect );
				}
			}

			++curr;
		}
		else
		{
			curr = m_worldEffects.erase( curr );
		}
	}

	// now split into opaques and alphas
	//-----------------------------------------------------------
	for	(SortablePtrList::iterator curr = visibles.begin();
		curr != visibles.end(); ++curr )
	{
		if ((*curr)->Opaque())
			m_transientOpaques.push_back( (*curr) );
		else
			m_transientAlphas.push_back( (*curr) );
	}

	// then sort opaques front to back
	//-----------------------------------------------------------
	if (m_transientOpaques.size())
	{
		ntstd::sort(	&m_transientOpaques[0], &m_transientOpaques[0] + m_transientOpaques.size(),
					Effects::comparatorCloserThan() );
	}

	// then sort alphas back to front
	//-----------------------------------------------------------
	if (m_transientAlphas.size())
	{
		ntstd::sort(	&m_transientAlphas[0], &m_transientAlphas[0] + m_transientAlphas.size(),
					Effects::comparatorFurtherThan() );
	}

	// then split into hdr and ldr effects
	//-----------------------------------------------------------
	for ( u_int i = 0; i < m_transientOpaques.size(); i++ )
	{
		if ( m_transientOpaques[i]->HighDynamicRange() )
			m_sortedHDR.push_back( m_transientOpaques[i] );
		else
			m_sortedLDR.push_back( m_transientOpaques[i] );
	}

	for ( u_int i = 0; i < m_transientAlphas.size(); i++ )
	{
		if ( m_transientAlphas[i]->HighDynamicRange() )
			m_sortedHDR.push_back( m_transientAlphas[i] );
		else
			m_sortedLDR.push_back( m_transientAlphas[i] );
	}
	
	// finally add our misc effects to the equation
	//-----------------------------------------------------------
	for	(EffectIDList::iterator curr = m_miscEffects.begin(); curr != m_miscEffects.end(); )
	{
		Effect* pEffect = GetEffect(*curr);

		if (pEffect)
		{
			if (pEffect->WaitingForResources() == false)
			{
				if (pEffect->HighDynamicRange())
					m_sortedHDR.push_back( pEffect );
				else
					m_sortedLDR.push_back( pEffect );
			}
			curr++;
		}
		else
		{
			curr = m_miscEffects.erase( curr );
		}
	}

	CGatso::Stop( "EffectManager::PreRender" );
}

//--------------------------------------------------
//!
//!	Render effects
//! Assumes current viewport is where we want to be rendered to.
//!
//--------------------------------------------------
void EffectManager::RenderHDR()
{
	ScopedCritical crit( m_CriticalSection );

	if (RenderDisabled())
		return;

	CGatso::Start( "EffectManager::RenderHDR" );

	for	(EffectPtrList::iterator curr = m_sortedHDR.begin(); curr != m_sortedHDR.end(); ++curr )
	{
		(*curr)->RenderEffect();
	}

	CGatso::Stop( "EffectManager::RenderHDR" );
}

//--------------------------------------------------
//!
//!	Render effects
//! Assumes current viewport is where we want to be rendered to.
//!
//--------------------------------------------------
void EffectManager::RenderLDR()
{
	ScopedCritical crit( m_CriticalSection );

	if (RenderDisabled())
		return;

	CGatso::Start( "EffectManager::RenderLDR" );

	for	(EffectPtrList::iterator curr = m_sortedLDR.begin(); curr != m_sortedLDR.end(); ++curr )
	{
		(*curr)->RenderEffect();
	}

	CGatso::Stop( "EffectManager::RenderLDR" );

	PSystemDebugMan::Get().DebugRender();
}

//--------------------------------------------------
//!
//!	SortableEffect::DebugRenderCullVolume
//! debug display of our culling parameters
//!
//--------------------------------------------------
void SortableEffect::DebugRenderCullVolume() const
{
#ifndef _GOLD_MASTER
	g_VisualDebug->RenderSphere( CVecMath::GetQuatIdentity(), GetCullingOrigin(), GetCullingRadius(), 0xffff0000, DPF_WIREFRAME );
#endif
}

