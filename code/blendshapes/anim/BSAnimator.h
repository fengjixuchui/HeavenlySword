//--------------------------------------------------
//!
//!	\file BSAnimator.h
//! Takes care of the animation of a particular
//! blendshapes set
//! 
//--------------------------------------------------

#ifndef _BSANIMATOR_H_
#define _BSANIMATOR_H_


#include "blendshapes/anim/blendshapes_anim.h"
#include "blendshapes/blendshapes_constants.h"
#include "game/luaglobal.h"

class BSAnimShortcutContainer;
class CEntity;
class BSSet;
typedef  BSSet*  BSSetPtr_t;

class BSAnimator;
typedef BSAnimator* BSAnimatorPtr_t;


class BSAnimator : CNonCopyable
{
public:

	HAS_LUA_INTERFACE()

	BSAnimator( CEntity* pEntity, BSSetPtr_t pBlendShapes, BSAnimShortcutContainer* pBSAnimContainer );
	~BSAnimator();

	//! enabling/disabling functions
	bool		IsEnabled( void ) const;
	void		SetEnabled( bool bEnable );
	void		Enable( void );
	void		Disable( void );

	//! update the playing animations and internal blendshape set data
	void		Update( float timeStep );
	

	//! \returns true if bsanim is playing
	bool		IsPlaying ( const CHashedString& shortName );
	//! \returns true if bsanim was added to the playlist
	bool		Play( const CHashedString& shortName, float fSpeed = 1.0f, int iFlags = 0 );
	//! stops the bsanimation from playing and removes it from the playlist if present
	void		Stop( const CHashedString& shortName );
	//! removes every bsanim from the playlist
	void		StopAll( void );
	//! stops all bsanims marked as finished. Called inside Update anyway
	void		StopAllFinished( void );

	//! reset all playing animations  but doesn't remove them from the playlist
	void		Reset( void );	

	void		DebugPrint( void ) const;

public:
	//! Extended Lua functions
	bool		Lua_Play( const char* pcShortName, float fSpeed = 1.0f, bool bLooping = false );
	bool		Lua_PlayAndHold( const char* pcShortName, float fSpeed = 1.0f );
	void		Lua_Stop( const char* pcShortName );
	void		Lua_StopAll( void );
	void		Lua_StopAllFinished( void );

private:
	typedef ntstd::Map< CHashedString, BSAnimation*, ntstd::less< CHashedString >, Mem::MC_PROCEDURAL > BSAnimationPtrCollection_t;

	//! updates the bsanims 
	void		UpdateBSAnims( float timeStep );
	bool		IsCompatibleWithCurrentBSMesh( BSAnimation* pAnim ) const;

	// may return a null ptr
	BSAnimation* GetPlayingBSAnimation( uint32_t nameHash );
	BSAnimation* GetPlayingBSAnimation( const CHashedString& shortName );
	BSAnimationPtrCollection_t::iterator RemoveFromPlayingBSAnimations( BSAnimationPtrCollection_t::iterator& it );
	
private:
	
	BSAnimationPtrCollection_t		m_anims; 
	//! enabled?
	bool							m_bEnabled;
	//! the entity to update
	CEntity*						m_pEntity;
	//! current blendshape set
	BSSetPtr_t						m_pBlendShapes;
	//! bsanims pool
	BSAnimShortcutContainer*		m_pobBSAnimContainer;
};


LV_DECLARE_USERDATA(BSAnimator);


inline
bool BSAnimator::IsEnabled( void ) const
{
	return m_bEnabled;
}

inline 
void BSAnimator::SetEnabled( bool bEnable ) 
{
	m_bEnabled = bEnable;
}

inline
void BSAnimator::Enable( void )
{
	SetEnabled( true );
}

inline
void BSAnimator::Disable( void )
{
	SetEnabled( false );
}

#endif // end of _BSANIMATOR_H_
