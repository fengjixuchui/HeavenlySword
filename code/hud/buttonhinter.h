#ifndef _BUTTONHINTER_H
#define _BUTTONHINTER_H

// Necessary includes
#include "hud/hudunit.h"

#include "effect/screensprite.h"
#include "editable/enumlist.h"
#include "effect/screensprite.h"
#include "game/entity.h"
#include "game/entity.inl"

// Forward declarations


//------------------------------------------------------------------------------------------
//!
//!	ButtonHinter
//!	Gets bools to find out if we need to render any buttons right now
//!
//------------------------------------------------------------------------------------------
class ButtonHinter
{
public:
	ButtonHinter();
	~ButtonHinter();

	void SetHintForButton(VIRTUAL_BUTTON_TYPE eButton, bool bHint);
	bool GetHintForButton(VIRTUAL_BUTTON_TYPE eButton) { return m_abHint[eButton]; };

	void Update (void);

	void RegisterHintEntity ( CEntity* pobEnt );
	void RemoveHintEntity ( CEntity* pobEnt );
	void ClearHintEntities ( void );

private:
	//ButtonHinterDef* m_pobDef;
	//bool m_bOwnsDef;

	bool m_abHint[AB_NUM];
	bool m_abLastHint[AB_NUM];

	// Any entities that an interaction hint should be done on
	ntstd::Vector<CEntity*> m_aobEntList;

	friend class ButtonHinterRenderer;
};

//------------------------------------------------------------------------------------------
//!
//!	ButtonHinterRenderDef
//!	Defines sprites and sizes for ButtonHintRenderer to use
//!
//------------------------------------------------------------------------------------------
class ButtonHinterRenderDef : public CHudUnitDef
{
	HAS_INTERFACE( ButtonHinterRenderDef );

public:
	ButtonHinterRenderDef();
	~ButtonHinterRenderDef();

	virtual CHudUnit* CreateInstance( void ) const;

	CKeyString m_aobButtonSprites[AB_NUM];

	float m_fTopLeftX;
	float m_fTopLeftY;
	float m_fButtonSize;

	float m_fAlphaHigh;
	float m_fAlphaLow;
	float m_fAlphaTime;

	float m_fBlendTime;

	friend class ButtonHinterRenderer;
};

//------------------------------------------------------------------------------------------
//!
//!	ButtonHinterRenderer
//!	Renders the ButtonHinter
//!
//------------------------------------------------------------------------------------------
class ButtonHinterRenderer : public CHudUnit
{
public:
	ButtonHinterRenderer(ButtonHinterRenderDef*  pobRenderDef) 
	:	m_pobRenderDef( pobRenderDef )
	,	m_fPulseTime( 0.0f )
	,	m_fBlendTime( 0.0f )
	,	m_bPulseIn( false )
	,	m_bBlendIn( false ) 
	{};

	~ButtonHinterRenderer() {};

	virtual bool Render( void );
	virtual bool Update( float fTimeChange);
	virtual bool Initialise( void );

	ButtonHinter* m_pobHinter;
	ButtonHinterRenderDef* m_pobRenderDef;

	float m_fPulseTime;
	float m_fBlendTime;

	bool m_bPulseIn;
	bool m_bBlendIn;

	ScreenSprite m_obSprite;
};

#endif // _BUTTONHINTER_H
