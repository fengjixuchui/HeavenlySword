//--------------------------------------------------
//!
//!	\file entityanimcontainer.h
//!	classes that assocaiate animation headers with
//! entitys
//!
//--------------------------------------------------

#ifndef ENTITY_ANIM_CONT_H
#define ENTITY_ANIM_CONT_H

#ifndef	ANIM_ANIMATION_HEADER_H
#include "anim/AnimationHeader.h"
#endif

class CAnimEventList;

class Anim;
class AnimContainer;

//------------------------------------------------------------------------------------------
//!
//!	AnimShortCut
//!	Tie together a short name with an animation data object that will be fixed up at runtime
//!
//------------------------------------------------------------------------------------------
class AnimShortCut
{
public:
	friend class EntityAnimSet;

	AnimShortCut( const Anim* pXMLDef );
	~AnimShortCut();

	void InstallAnim( const char* pDebugTag );
	void UninstallAnim();
	bool AnimInstalled() const { return (m_pAnimationHeader != 0); }

	const CHashedString&	GetShortNameHash() const { return m_shortNameHash; }
	const CAnimationHeader*	GetHeader() const
	{
		ntError_p( m_pAnimationHeader != 0, ("Animation Header has not been installed") );
		return m_pAnimationHeader;
	}

	typedef ntstd::List<const CAnimEventList*, Mem::MC_ENTITY> AnimEventLists;

	#ifndef _RELEASE
	void RebuildAnimEventLists();
	#endif

private:
	// think we can remove this, should be stored as our index in the parent map
	CHashedString	m_shortNameHash;

	// our lookup into the animation header cache, generated on construction
	uint32_t		m_animCacheKey;

	// Pointer to R/O animation data, may be invalid!
	const CAnimationHeader* m_pAnimationHeader;

	// Pointer to R/O anim event list
	AnimEventLists m_animEventLists;

	void InsertAnimEventList(const CHashedString& name);
	
	// debug functionality to allow missing animations
	ntstd::String	m_debugLoadName;

	#ifndef _RELEASE
	// debug functionality to allow the rebuilding of anim event lists
	const Anim*		m_pXMLDef;
	#endif
};

//------------------------------------------------------------------------------------------
//!
//!	EntityAnimContainer
//!	Collection of AnimShortCut objects, corresponds to a code side version of
//! AnimContainer, but is not a serialised object. (should really be one and the same thing)
//!
//------------------------------------------------------------------------------------------
class EntityAnimContainer
{
public:
	friend class EntityAnimSet;
	friend class CEntityBrowser;

	EntityAnimContainer( AnimContainer* pXMLData, const CHashedString& contName );
	~EntityAnimContainer();

	void	InstallAnimations();
	void	UninstallAnimations();
	
	bool	AnimationsInstalled() const		{ return (m_installCount > 0); }
	const CHashedString& GetName() const	{ return m_name; }

#ifndef _RELEASE
	void			RebuildAnimEventLists();
	const AnimShortCut*	FindAnimShortCut( const CAnimationHeader* pHeader ) const;
#endif

	const AnimShortCut*	FindAnimShortCut( uint32_t cacheKey, bool bSearchPooled ) const;

private:	
	typedef ntstd::Map<uint32_t,AnimShortCut*, ntstd::less<uint32_t>, Mem::MC_ENTITY> ShortcutMap;
	typedef ntstd::Vector<AnimShortCut*, Mem::MC_ENTITY> AnimList;
	typedef ntstd::Map<uint32_t,AnimList*, ntstd::less<uint32_t>, Mem::MC_ENTITY> PoolMap;

	int32_t			m_installCount;
	ShortcutMap		m_shortcutAnims;
	PoolMap			m_pooledAnims;
	AnimList		m_totalAnims;
	AnimContainer*	m_pXMLData;
	CHashedString	m_name;
};

//------------------------------------------------------------------------------------------
//!
//! AnimContainerManager
//! Singleton that maintains animation containers
//! Has global rather than level scope
//!
//------------------------------------------------------------------------------------------
class AnimContainerManager : public Singleton<AnimContainerManager>
{
public:
	void AddContainer( AnimContainer* pCont );
	void RemoveContainer( AnimContainer* pCont );

	// we only expose game animation containers, not the original
	// XML serialised object
	EntityAnimContainer* GetAnimContainer( uint32_t cacheKey );

private:
	typedef ntstd::Map<uint32_t, AnimContainer*, ntstd::less<uint32_t>, Mem::MC_ENTITY>			XMLMap;
	typedef ntstd::Map<uint32_t, EntityAnimContainer*, ntstd::less<uint32_t>, Mem::MC_ENTITY>	GameMap;

	XMLMap	m_XMLContMap;
	GameMap	m_gameContMap;
};

#endif // ENTITY_ANIM_CONT_H

