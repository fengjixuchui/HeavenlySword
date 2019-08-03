//--------------------------------------------------
//!
//!	\file BlendShapesComponent.h
//!	
//! TODO_OZZ: write some nice explanation here
//!
//--------------------------------------------------



#ifndef _BLENDSHAPESCOMPONENT_H_
#define _BLENDSHAPESCOMPONENT_H_

#include "blendshapes/anim/BSAnimContainer.h"
#include "blendshapes/anim/bsanimator.h"

// stolen from entity.h
#ifndef COMPONENT_ACCESS
#	define COMPONENT_ACCESS( TYPE, FUNCTION, VAR ) \
		virtual TYPE*		FUNCTION( void ) { return VAR; } \
		virtual const TYPE*	FUNCTION( void ) const { return VAR; } \
		virtual TYPE*		FUNCTION##_nonconst( void ) { return VAR; } \
		virtual const TYPE*	FUNCTION##_const( void ) const { return VAR; }
#endif

class BSSet;
typedef BSSet*  BSSetPtr_t;

class CHierarchy;
class CClumpHeader;
class CEntity;
class BlendedMeshInstance;



class BlendShapesComponent : public CNonCopyable, public BSAnimShortcutContainer
{
public:

	HAS_LUA_INTERFACE()

	BlendShapesComponent( CEntity* pParent ); 
	virtual ~BlendShapesComponent();

	bool			IsCompatible( const BSSetPtr_t pBlendShapes ) const;

	//! cascading update of the entire component
	void			Update( float timeStep );

	//! enabling/disabling stuff
	bool			IsEnabled( void ) const;
	void			SetEnabled( bool bEnabled );
	void			Enable( void );
	void			Disable( void );

	//! installs the bsclump as the current blendshape set, checks for compatibility and
	//! creates the corresponding BSAnimator for this set. Note that the previous animator
	//! will be removed. Thus, all playing animations stopped. 
	//! However, the bsanims in are not removed from the container. It's up to the 
	//! caller to ensure proper container removal/restore
	bool			PushBSSet( const char* fileName );
	//! gets rid of the last used bsset and restores the previous one (or none if this is the last one)
	void			PopBSSet( void );

	//! gets the active blendshape set
	BSSetPtr_t		GetCurrentBSSet( void );

	COMPONENT_ACCESS( BSAnimator, GetBSAnimator, m_pBSAnimator );

	void			DebugPrint( void );

	static bool		IsBlendShapeCapable( CEntity* pEntity );

protected:
	// overriden from BSAnimShortcutContainer so it stops the clip in the bsanimator
	// before removal
	virtual BSAnimShortcutContainer::BSAnimShortcutCollection::iterator RemoveBSAnim( BSAnimShortcutCollection::iterator it );

private:
	void		SetBSSet( BSSetPtr_t pBS );
	void		DestroyBSAnimator( void );
	void		CreateBSAnimator( void );

private:
	//! used for keeping track of the current blendshape set for this entity
	typedef ntstd::Vector<BSSetPtr_t> BSSetCollection_t;
	//BSSetCollection_t			m_obBSSets;
	BSSet*						m_pobBSSet;

	//! shorcut list of parent entity's blended mesh instances 
	typedef ntstd::List< BlendedMeshInstance* > BlendedMeshCollection_t;
	BlendedMeshCollection_t		m_obBSMeshes;

	//! owner of this component
	CEntity*					m_pEntity;
	//! memory for the animator placement new. It has to be created post-construct since 
	//! it needs a ptr to this
	char						m_pBSAnimatorMemory[ sizeof(BSAnimator) ];
	//! current bsanimator
	BSAnimator*					m_pBSAnimator;
	//! enabled?
	bool						m_bEnabled;
};


LV_DECLARE_USERDATA(BlendShapesComponent);



inline bool BlendShapesComponent::IsEnabled( void ) const
{
	return m_bEnabled;
}

inline void BlendShapesComponent::SetEnabled( bool bEnabled ) 
{
	m_bEnabled = bEnabled;
}

inline void BlendShapesComponent::Enable( void )
{
	SetEnabled( true );
}

inline void BlendShapesComponent::Disable( void )
{
	SetEnabled( false );
}



#endif // end of _BLENDSHAPESCOMPONENT_H_
