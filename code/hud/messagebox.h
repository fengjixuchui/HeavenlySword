#ifndef _CMessageBox_H
#define _CMessageBox_H

// Necessary includes
#include "hud/blendablehudunit.h"

#include "effect/screensprite.h"
#include "game/entitymanager.h"
#include "game/entityinfo.h"
#include "editable/enumlist.h"

class HudTextRenderer;
class HudTextRenderDef;

//------------------------------------------------------------------------------------------
//!
//!	CMessageBoxDef
//!
//------------------------------------------------------------------------------------------
class CMessageBoxDef
{
	HAS_INTERFACE( CMessageBoxDef );

public:
	// Construction destruction
	CMessageBoxDef( void ) {};
	~CMessageBoxDef( void ) {};

	// Layout
	float		m_fPositionX;				// Position for message box
	float		m_fPositionY;				// Position for message box

	float		m_fBoxMinHeight;			// Min size of message box 
	float		m_fBoxMinWidth;				// Min size of message box 

	float		m_fBoxMaxHeight;			// Max size of message box 
	float		m_fBoxMaxWidth;				// Max size of message box 
};

//------------------------------------------------------------------------------------------
//!
//!	CMessageBox
//!
//------------------------------------------------------------------------------------------
class CMessageBox
{
	//HAS_INTERFACE( CMessageBox );

public:
	// Construction destruction
	CMessageBox( CMessageBoxDef* pobDef );
	CMessageBox( void );
	~CMessageBox( void );

	// Layout
	float		m_fPositionX;				// Position for message box
	float		m_fPositionY;				// Position for message box

	float		m_fBoxMinHeight;			// Min size of message box 
	float		m_fBoxMinWidth;				// Min size of message box 

	float		m_fBoxMaxHeight;			// Max size of message box 
	float		m_fBoxMaxWidth;				// Max size of message box 

	ntstd::Vector<HudTextRenderDef*>   m_aobHudTextDefList;
};

typedef ntstd::Vector<HudTextRenderDef*>::iterator TextDefIter;

//------------------------------------------------------------------------------------------
//!
//!	CMessageBoxRenderDef
//!
//------------------------------------------------------------------------------------------

enum {
	MBI_TOP_LEFT,
	MBI_TOP,
	MBI_TOP_RIGHT,
	MBI_LEFT,
	MBI_CENTRE,
	MBI_RIGHT,
	MBI_BOTTOM_LEFT,
	MBI_BOTTOM,
	MBI_BOTTOM_RIGHT,
	MBI_WATERMARK,
	MSG_BOX_IMAGES,
};


class CMessageBoxRenderDef : public CBlendableHudUnitDef
{
	HAS_INTERFACE( CMessageBoxRenderDef );

public:
	// Construction destruction
	CMessageBoxRenderDef( void );
	~CMessageBoxRenderDef( void );

	virtual CHudUnit* CreateInstance( void ) const;

	void PostConstruct( void );

	// Layout
	float			m_fBoxImageHeight;			// Size of message box images
	float			m_fBoxImageWidth;			// Size of message box images

	float			m_fBoxWatermarkHeight;		// Size of message box images
	float	 		m_fBoxWatermarkWidth;		// Size of message box images

	float			m_fAlphaMessageBox;
	float			m_fAlphaWatermark;
	float			m_fAlphaText;

	float			m_fVerticalMargin;			// Vertical Margin
	float	 		m_fHorizontalMargin;		// Horizontal Margin

										
	CKeyString m_aobBoxTexture[ MSG_BOX_IMAGES ];

	ntstd::String m_obFont;

	HORIZONTAL_JUSTFICATION	m_eJustifyHorizontal;
	VERTICAL_JUSTIFICATION	m_eJustifyVertical;

	EFFECT_BLENDMODE m_eBlendMode;
};

//------------------------------------------------------------------------------------------
//!
//!	CMessageBoxRenderer
//!
//------------------------------------------------------------------------------------------
class CMessageBoxRenderer : public CBlendableHudUnit
{
public:
	// Construction destruction
	CMessageBoxRenderer( CMessageBoxRenderDef*  pobRenderDef );
	~CMessageBoxRenderer( void );

	// Render our current settings
	virtual bool Render( void );
	virtual bool Update( float fTimeStep );
	virtual bool Initialise( void );
	bool Initialise( const CSharedPtr<CMessageBox,Mem::MC_MISC>& obMsgBox);

	virtual bool	BeginExit( bool bForce = false );
	virtual bool	BeginEnter( bool bForce = false );

private:
	virtual void	UpdateInactive( float fTimestep );
	virtual void	UpdateActive( float fTimestep );

	CMessageBoxRenderDef* m_pobDef;

	ScreenSprite m_aobBoxSprite[ MSG_BOX_IMAGES ];

	CSharedPtr<CMessageBox,Mem::MC_MISC> m_obMessageBox;

	ntstd::Vector<HudTextRenderer*> m_aobHudTextRendererList;

	float		m_fBoxHeight;			
	float		m_fBoxWidth;			

};

typedef ntstd::Vector<HudTextRenderer*>::iterator TextRenderIter;
#endif // _CMessageBox_H
