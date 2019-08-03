/***************************************************************************************************
*
*	DESCRIPTION		The basic unit on which HUD items are based
*
*	NOTES			
*
***************************************************************************************************/

#ifndef	_BLENDABLE_HUDUNIT_H
#define	_BLENDABLE_HUDUNIT_H

// Set up a standard blend time
#define ANIM_BLEND_TIME ( 0.25f )
#define ANIM_BLEND_CYCLE ( 0.5f )

// Includes
#include "gui\guiutil.h"
#include "hud\hudunit.h"

// Forward declarations
class CBlendableHudUnit;

/***************************************************************************************************
*
*	CLASS			CBlendableHudUnitDef
*
*	DESCRIPTION		The basic def unit for a HUD element, with blending functionality
*
***************************************************************************************************/

class CBlendableHudUnitDef : public CHudUnitDef
{
public:
	HAS_INTERFACE( CBlendableHudUnitDef );

	// A pure virtual call to be overridden by specific unit defintions
	virtual CHudUnit* CreateInstance( void ) const;

	//! Construction Destruction - derive if you want one
	CBlendableHudUnitDef( void );
	virtual ~CBlendableHudUnitDef( void );

	float m_fBlendInTime;
	float m_fBlendOutTime;
};

/***************************************************************************************************
*
*	CLASS			CBlendableHudUnit
*
*	DESCRIPTION		The basic unit for a HUD element.
*
***************************************************************************************************/

class CBlendableHudUnit : public CHudUnit
{

public:

	//! The biggies
	virtual bool	Initialise( void );
	virtual bool	Render( void );

	// State Updates
	virtual bool	BeginEnter( bool bForce = false );
	virtual bool	BeginExit( bool bForce = false );

	//! Construction Destruction - derive if you want one
	CBlendableHudUnit( CBlendableHudUnitDef* pobDef );
	virtual ~CBlendableHudUnit( void );

protected:
	// State based updates - should probably be scripted at some point
	virtual void	UpdateEnter( float fTimestep );
	virtual void	UpdateInactive( float fTimestep );
	virtual void	UpdateActive( float fTimestep );
	virtual void	UpdateExit( float fTimestep );

	CBlendableHudUnitDef* m_pobBlendableDef;

	CGuiTimer m_obBlendTimer;

	float m_fOverallAlpha;
};

#endif	//_BLENDABLE_HUDUNIT_H
