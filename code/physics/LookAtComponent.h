//--------------------------------------------------
//!
//!	\file LookAtComponent.h
//!	This is the look-at component. It can handle
//! all point-at/look-at/aim-at type of constraints for a character
//!
//--------------------------------------------------

#ifndef _LOOKATCOMPONENT_H_
#define _LOOKATCOMPONENT_H_

#include "game/luaglobal.h"
#include "anim/characterboneid.h"



class CEntity;
class Transform;
class LookAtConstraint;
struct LookAtConstraintDef;
struct LookAtInfo;



struct LookAtCopyRotation;

//--------------------------------------------------
//!
//!	Look-at component soft definition
//!	Long Class Description
//!	Exciting class with lots of stuff to describe
//!
//--------------------------------------------------
struct LookAtComponentDef
{
	HAS_INTERFACE(LookAtConstraintDef)

	ntstd::List<LookAtConstraintDef*>			m_obConstraintDefs;			//!< the constraint defition list 
	ntstd::List<LookAtCopyRotation*>			m_obCopyRotationNodes;

	ntstd::String								m_obTransformName;			//!< affected transform start
	ntstd::String								m_obDefaultTargetName;		//!< if not active, use this
	float										m_fFOV;						//! the FOV inside which this constraint is effective
	float										m_fDistance;				//! the effective distance for this constraint
	bool										m_bEnabled;
	bool										m_bIsActive;	

	bool m_bDebugRender;
	bool m_bDebugRenderTarget;
	bool m_bRefreshConstraints;
};

struct LookAtCopyRotation
{
	HAS_INTERFACE( LookAtCopyRotation )

	CHashedString					m_obFrom;
	CHashedString					m_obTo;
};

class LookAtComponent : CNonCopyable
{
public: 
	typedef ntstd::List<LookAtConstraint*>	ConstraintList_t;

public:
	HAS_LUA_INTERFACE()

	LookAtComponent( CEntity* pobEntity,  LookAtComponentDef* pobDef );
	virtual ~LookAtComponent();
	void PostConstruct(); NOT_IMPLEMENTED
	void PostPostConstruct();

	void			LookAt( const CEntity* pobEntity, const CHashedString& obTransformName = m_obInvalidTransName );
	virtual void	Update( float fTimeStep );
	void			Reset( void );

	bool			IsActive( void ) const;

	bool 			IsEnabled( void ) const;
	void			Enable( void );		
	void			Disable( void );

	// ugh...
	ConstraintList_t& GetConstraints( void ) { return m_obConstraints; }

	//
	//	Extended Lua functions
	//
	void			Lua_LookAt( CHashedString obEntityName, CHashedString obTransformName );

	void			DebugRender( void );

private:

	CEntity*	SelectNewTarget( void );
	void		ApplyConstraintChain( float fTimeStep );
	void		RefreshConstraintChain( void );
	void		DestroyConstraintChain( void );

	void		ProcessCopyRotationNodes( bool bResync = false );

	static const Transform* SelectTransform( const CEntity* pobEntity );

	//! \return the named transform or root if not found
	static const Transform* GetTransform( const CEntity* pobEntity, const CHashedString& obTransformName );

protected:

	ConstraintList_t		m_obConstraints;				//!< the (hard) constraint list

	CEntity*				m_pobEntity;					//!< owner entity
	LookAtComponentDef*		m_pobDef;						//!< soft definition
	
	const CEntity*			m_pobTargetEntity;				//!< look-at that! can be set to NULL
	const Transform*		m_pobTargetTrans;				//!< what to look at exactly
	int						m_iFlags;

private:
	static const CHashedString m_obInvalidTransName;
};

LV_DECLARE_USERDATA(LookAtComponent);



inline bool LookAtComponent::IsEnabled( void ) const	
{ 
	return m_pobDef->m_bEnabled;
}

inline void LookAtComponent::Enable( void )
{ 
	m_pobDef->m_bEnabled = true;
}

inline void LookAtComponent::Disable( void )
{
	m_pobDef->m_bEnabled = false;
}


inline bool LookAtComponent::IsActive( void ) const
{
	return m_pobDef->m_bIsActive;
}

#endif // end of _LOOKATCOMPONENT_H_


