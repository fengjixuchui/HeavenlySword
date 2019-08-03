/***************************************************************************************************
*
*	DESCRIPTION		The basic unit on which all other GUI items are based - including screens
*
*	NOTES			
*
***************************************************************************************************/

#ifndef	_GUIUNIT_H
#define	_GUIUNIT_H

// Set up a standard blend time
#define ANIM_BLEND_TIME ( 0.25f )
#define ANIM_BLEND_CYCLE ( 0.5f )

// Includes
#include "guiparse.h"
#include "guiinput.h"
#include "guiutil.h"
#include "anim/animation.h"
#include "anim/AnimationHeader.h"
#include "editable/enumlist.h"

// Forward declarations
class	CClumpHeader;
class	CHierarchy;
class	CRenderableComponent;
class	CAnimator;
class	Transform;
class	CGuiScreen;

//#define _DEBUG_GUI_UNIT_STATE 1

/***************************************************************************************************
*
*	CLASS			CGuiUnit
*
*	DESCRIPTION		The basic unit for a UI element.
*
*					This is an abstract class and should be maintained as a base library for front
*					end development on any number of games.
*
*					This should contain no 'visual' elements such as strings or sprites.  It will
*					deal with aspects common to all gui items such as positioning and movement.
*
***************************************************************************************************/

class CGuiUnit : public CXMLElement
{
protected:
	
	// Unit States
	enum UNIT_STATE
	{
		STATE_BORN,
		STATE_ENTER,
		STATE_IDLE,
		STATE_FOCUS,
		STATE_FOCUSIN,
		STATE_FOCUSOUT,
		STATE_EXIT,
		STATE_DEAD,

		STATE_FADEIN,		// these two occur STATE_ENTER -> STATE_IDLE and STATE_IDLE -> STATE_EXIT
		STATE_FADEOUT,		//  The GuiScreen will calculate a fade value and expect elements to respond to it

		UNIT_STATE_SIZE,
	}	m_eUnitState;

public:

	//! The biggies
	virtual bool	Update( void );
	virtual bool	Render( void );

	//! Movement Commands - these should remain empty in this abstract class
	virtual bool	MoveLeftAction( int iPads )			{ UNUSED(iPads); return false; }
	virtual bool	MoveRightAction( int iPads )		{ UNUSED(iPads); return false; }
	virtual bool	MoveDownAction( int iPads )			{ UNUSED(iPads); return false; }
	virtual bool	MoveUpAction( int iPads )			{ UNUSED(iPads); return false; }

	//! Positive Actions - these should remain empty in this abstract class
	virtual bool	StartAction( int iPads )			{ UNUSED(iPads); return false; }
	virtual bool	SelectAction( int iPads )			{ UNUSED(iPads); return false; }

	//! Negative Actions - these should remain empty in this abstract class
	virtual bool	BackAction( int iPads )				{ UNUSED(iPads); return false; }
	virtual bool	DeleteAction( int iPads )			{ UNUSED(iPads); return false; }

	//! Indiscriminate Actions - these should remain empty in this abstract class
	virtual bool	AnyAction( int iPads )				{ UNUSED(iPads); return false; }

	// State Updates
	virtual bool	BeginEnter( bool bForce = false );
	virtual bool	BeginIdle( bool bForce = false );
	virtual bool	BeginFocus( bool bForce = false );
	virtual bool	BeginExit( bool bForce = false );

//	virtual bool	BeginFadeIn( bool bForce = false );
//	virtual bool	BeginFadeOut( bool bForce = false );

	// Basic checks on the units state
	bool			IsInteractive( void )	{ return ( ( m_eUnitState == STATE_IDLE ) || ( m_eUnitState == STATE_FOCUS ) ); }

	virtual bool	IsFading( void );

	UNIT_STATE		GetState() const	{ return m_eUnitState;  }

	enum RENDER_SPACE
	{
		RENDER_WORLDSPACE,
		RENDER_CAMERASPACE,
		RENDER_SCREENSPACE,
	};

	CGuiScreen*		GetParentScreen() { return m_pobParentScreen; }

	const CHashedString&	GetUnitID() const { return m_obUnitID; }

	//! Generic data set/getting
	virtual bool	SetData(const char* pcName, void* pvData);
	virtual bool	GetData(const char* pcName, void* pvData);

	CGuiUnit*		FindChildUnit(const char* pcUnitID, bool bRecursive = false);

	bool			AllowRender() { return m_bAllowRender; }
	void			AllowRender(bool bRender) { m_bAllowRender = bRender; }

	virtual float	ScreenFade();
	virtual float	ScreenFadeLength() { return 1.0f/m_fScreenFadeScale; }
	
	void			SetFade(float fFade);
	void			SetFadeComplete();
	virtual bool	AutoFade();

	bool			IsDead() const { return m_eUnitState == STATE_DEAD; }

protected:

	//! Construction Destruction - derive if you want one
	CGuiUnit( void );
	virtual ~CGuiUnit( void );

	//! To deal with parsing
	virtual bool	ProcessAttribute( const char* pcName, const char* pcValue );
	virtual bool	ProcessChild( CXMLElement* pobChild );
	virtual bool	ProcessEnd( void );

	//! Setting Unit Paraments from Strings - return false on failure
	bool	ProcessClumpNameValue( const char* pcValue );
	bool	ProcessBasePositionValue( const char* pcValue );
	bool	ProcessMovePositionValue( const char* pcValue );
	bool	ProcessVerticalJustificationValue( const char* pcValue );
	bool	ProcessHorizontalJustificationValue( const char* pcValue );
	bool	ProcessRenderSpaceValue( const char* pcValue );
	bool	ProcessAnimationNameValue( const char* pcValue, UNIT_STATE eUnitState );

	// State based updates - should probably be scripted at some point
	virtual void	UpdateEnter( void );
	virtual void	UpdateIdle( void );
	virtual void	UpdateFocus( void );
	virtual void	UpdateFocusIn( void );
	virtual void	UpdateFocusOut( void );
	virtual void	UpdateExit( void );

	virtual void	UpdateFadeIn( void );
	virtual void	UpdateFadeOut( void );

	// State Setting code
	virtual void	SetStateEnter( void );
	virtual void	SetStateExit( void );
	virtual void	SetStateIdle( void );
	virtual void	SetStateFocus( void );
	virtual void	SetStateFocusIn( void );
	virtual void	SetStateFocusOut( void );
	virtual void	SetStateDead( void );

	virtual void	SetStateFadeIn( void );
	virtual void	SetStateFadeOut( void );

	// Helpers to cut down on includes
	void	UpdateAnimations( void );
	bool	IsAnimating( void );

	// does the actual work of the public method
	CGuiUnit*		FindChildUnit(CHashedString& obUnitID, bool bRecursive);

	// Animations for states
	// This is changing which is why it's a struct with one thing at the moement
	struct UNIT_ANIMATIONS
	{
		const CAnimationHeader *pobAnimationHeader;
	}	m_astrAnimations[UNIT_STATE_SIZE];

	//! Horizontal justification possiblities
	HORIZONTAL_JUSTFICATION m_eHorizontalJustification;

	//! Vertical justification possibilities
	VERTICAL_JUSTIFICATION m_eVerticalJustification;

	//! Space possibilities
	
	enum RENDER_SPACE m_eRenderSpace;

	//! The base transform for this element
	Transform*			m_pobBaseTransform;
	CPoint				m_BasePosition;

	//! This is what we create the item from
	CClumpHeader*		m_pobClumpHeader;

	//! The elements physical representation
	CHierarchy*			m_pobHierarchy;

	//! The renderables in the element.
	CRenderableComponent*	m_pobRenderable;

	//! The last animation we set playing
	CAnimationPtr		m_pobPreviousAnimation;

	//! This allows us to move stuff about
	CAnimator*			m_pobAnimator;

	//! To Regulate Input
	PAD_NUMBER			m_ePadNumber;
	float				m_fPadTime;
	CGuiTimer			m_obPadTimer;

	CGuiScreen*			m_pobParentScreen;
	
	bool				m_bAllowRender;

	CHashedString		m_obUnitID;

	float m_fScreenFadeScale;
	float m_fScreenFade;
	bool m_bFadeRunning;
	bool m_bAutoFade;
};

#endif	//_GUIUNIT_H
