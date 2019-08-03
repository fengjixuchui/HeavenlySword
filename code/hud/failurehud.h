#ifndef _FAILUREHUD_H
#define _FAILUREHUD_H

// Necessary includes
#include "hud/blendablehudunit.h"

#include "effect/screensprite.h"
#include "game/entitymanager.h"
#include "game/entityinfo.h"
#include "effect/renderstate_block.h"
#include "editable/enumlist.h"

// Forward declarations
class FailureHudRenderer;

//------------------------------------------------------------------------------------------
//!
//!	FailureHud
//!	Stuff needed for FailureHud
//!
//------------------------------------------------------------------------------------------
class FailureHud
{
public:
	FailureHud();
	~FailureHud();

	void SetRenderer( FailureHudRenderer* pobFailureHudRenerer );

	void SetFailureStringID( ntstd::String obFailureStringID );

	ntstd::String GetFailureStringID( void ) { return m_obFailureStringID; };

	void NotifyFailure( void );
	void NotifyFailure( ntstd::String obFailureStringID );

private:
	FailureHudRenderer* m_pobFailureHudRenerer;

	ntstd::String m_obFailureStringID;

	friend class FailureHudRenderer;
};

//------------------------------------------------------------------------------------------
//!
//!	FailureHudRenderDef
//!	Defines for FailureHudRender HUD element
//!
//------------------------------------------------------------------------------------------
class FailureHudRenderDef : public CBlendableHudUnitDef
{
	HAS_INTERFACE( FailureHudRenderDef );

public:
	FailureHudRenderDef();
	~FailureHudRenderDef() {};

	float m_fSlowTimeFactor;
	float m_fSlowBlendTime;

	float m_fMoveOnTime;

	float m_obFadeTime;
	uint32_t m_iFadeColour;

	virtual CHudUnit* CreateInstance( void ) const;
};

//------------------------------------------------------------------------------------------
//!
//!	FailureHudRenderer
//!	Code for drawing lifeclock
//!
//------------------------------------------------------------------------------------------
class FailureHudRenderer : public CBlendableHudUnit
{
public:
	FailureHudRenderer( FailureHudRenderDef*  pobRenderDef) 
	:	CBlendableHudUnit ( pobRenderDef )
	,	m_pobRenderDef ( pobRenderDef ) 
	,	m_pobFailureHud ( 0 )
	{};

	virtual ~FailureHudRenderer() {};

	virtual bool Render( void );
	virtual bool Update( float fTimestep );
	virtual bool Initialise( void );

	virtual bool BeginEnter( bool bForce );
	virtual bool BeginExit( bool bForce );

private:
	virtual void UpdateActive( float fTimestep );
	virtual void UpdateInactive( float fTimestep );

	FailureHudRenderDef*	m_pobRenderDef;

	FailureHud*				m_pobFailureHud;

	float m_fCurrSlowBlendTime;
	float m_fCurrMoveOnTime;
};

#endif // _FAILUREHUD_H
