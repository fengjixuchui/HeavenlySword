//--------------------------------------------------
//!
//!	\file entityanimset.h
//!	Class that glues together global EntityAnimContainer
//! classes with entitys
//!
//--------------------------------------------------

#ifndef ENTITY_ANIM_SET_H
#define ENTITY_ANIM_SET_H

#ifndef	ANIM_ANIMATION_HEADER_H
#include "anim/AnimationHeader.h"
#endif

class CAnimator;

class EntityAnimContainer;
class AnimShortCut;

template < typename RetType > class Functor;

//typedef Functor< CHashedString >	StringFunctor;
typedef Functor< ntstd::String >	StringFunctor;

//------------------------------------------------------------------------------------------
//!
//!	EntityAnimSet
//!	Poorly concieved class that joins shared anim event lists to individual entity animators
//! and collects EntityAnimContainers together for a unified place to find animations. Bobbins.
//!
//------------------------------------------------------------------------------------------
class EntityAnimSet 
{
protected:
	// Gash. why does the CEntityBrowser not use a well defined interface like the rest of us have to?
	friend class CEntityBrowser;

	//	Protected ctor, dtor - this means only deriving classes can construct/destroy us.
	EntityAnimSet();
	~EntityAnimSet();

public:
	void InstallAnimator(CAnimator* pAnimator, const CHashedString& animContainerName);

	void InstallNSAnims( const CHashedString& animContainerName);
	void UninstallNSAnims( const CHashedString& animContainerName);
	bool AreNSAnimsAdded( const CHashedString& animContainerName) const;

	const AnimShortCut*		FindAnimShortCut( const CHashedString& shortName, bool bSearchPooled )	const;
	const CAnimationHeader* FindAnimHeader( const CHashedString& shortName, bool bSearchPooled )	const;

	void InstallGetName(StringFunctor* pGetNameFunc )
	{
		ntError_p( pGetNameFunc != NULL, ("get_name_func must be a valid function.") );
		m_pGetNameFunc = pGetNameFunc;
	}
	ntstd::String GetName() const;

	#ifndef _RELEASE
	void RebuildAnimEventLists( const CHashedString& containerName );
	const char*	GetAnimName( const CAnimationHeader *pHeader ) const;
	#endif

private:
	void RegisterAnimEventLists( EntityAnimContainer* pAnimContainer );
	void UnRegisterAnimEventLists( EntityAnimContainer* pAnimContainer );

	#ifndef _RELEASE
	// debug slow method based on headers, used only for name lookups
	const AnimShortCut*	FindAnimShortCut( const CAnimationHeader* pHeader ) const;
	#endif

	typedef ntstd::List<EntityAnimContainer*,Mem::MC_ENTITY> AnimMapList;

	// global shared containers
	EntityAnimContainer*	g_pAnimationMap;
	AnimMapList				m_gNSAnimationMaps;

	// I don't really like the fact that this is here...  It's used for anim event monitors.
	CAnimator*				m_pAnimator;		
	StringFunctor*			m_pGetNameFunc;
};

#endif	// !ENTITY_ANIM_SET_H

