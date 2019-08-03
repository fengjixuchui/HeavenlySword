//--------------------------------------------------
//!
//!	\file effect_manager.h
//!	Singleton that owns, updates and renders all
//! Effect derived objects in the game.
//!
//--------------------------------------------------

#ifndef _EFFECT_MANAGER_H
#define _EFFECT_MANAGER_H

#include "sortable_effect.h"

class CFrustum;

//--------------------------------------------------
//!
//!	EffectManager
//! Effect managing singleton
//!
//--------------------------------------------------
class EffectManager : public Singleton<EffectManager>
{
public:
	EffectManager();
	~EffectManager();
	void Reset();
	
	/// pNewEffect should be allocated in the Mem::MC_EFFECTS chunk
	u_int AddEffect( Effect* pNewEffect );
	u_int AddSortableEffect( SortableEffect* pNewEffect );

	Effect*	GetEffect( u_int iEffectID );
	bool KillEffectNow( u_int iEffectID );
	bool KillEffectWhenReady( u_int iEffectID );

	void UpdateManager();
	void PreRender( const CFrustum* pFrustum );
	void RenderHDR();
	void RenderLDR();

	typedef ntstd::List<Effect*, Mem::MC_EFFECTS>	EffectPtrList;
	typedef ntstd::List<u_int, Mem::MC_EFFECTS>		EffectIDList;

	// for debug managers and the like to work outwhat has been rendered 
	const EffectPtrList& GetHDRRenderList() const { return m_sortedHDR; }
	const EffectPtrList& GetLDRRenderList() const { return m_sortedLDR; }

	static bool RenderDisabled() { return (s_bAllocationsDisabled || s_bRenderDisabled); }
	static bool AllocsDisabled() { return (s_bAllocationsDisabled); }

	static void ToggleRender() { s_bRenderDisabled = !s_bRenderDisabled; }
	static void HaltAllocs() { s_bAllocationsDisabled = true; }

private:
	typedef ntstd::Map<u_int, Effect*, ntstd::less<u_int>, Mem::MC_EFFECTS>		EffectTable;
	typedef ntstd::List<SortableEffect*, Mem::MC_EFFECTS>						SortablePtrList;

	// for book keeping
	u_int	AssignEffectID( Effect* pEffect );
	EffectTable		m_effects;
	u_int			m_iCurrID;

	// these lists are for rendering only
	EffectIDList	m_worldEffects;
	EffectIDList	m_miscEffects;
	EffectPtrList	m_sortedHDR;
	EffectPtrList	m_sortedLDR;

	// these are members so we can permanetly grow them to a good size
	ntstd::Vector<SortableEffect*, Mem::MC_EFFECTS>	m_transientOpaques;
	ntstd::Vector<SortableEffect*, Mem::MC_EFFECTS>	m_transientAlphas;

	static bool s_bAllocationsDisabled;
	static bool s_bRenderDisabled;

	// as we update and render on the main thread while loading
	CriticalSection m_CriticalSection;
};

namespace Effects
{
	//--------------------------------------------------
	//!
	//!	comparatorCloserThan
	//!
	//--------------------------------------------------
	class comparatorCloserThan
	{
	public:
		bool operator()( const SortableEffect* pFirst, const SortableEffect* pSecond ) const
		{
			return (pFirst->GetSortingDistance() < pSecond->GetSortingDistance());
		}
	};

	//--------------------------------------------------
	//!
	//!	comparatorFurtherThan
	//!
	//--------------------------------------------------
	class comparatorFurtherThan
	{
	public:
		bool operator()( const SortableEffect* pFirst, const SortableEffect* pSecond ) const
		{
			return (pFirst->GetSortingDistance() > pSecond->GetSortingDistance());
		}
	};
}

#endif //_EFFECT_MANAGER_H
