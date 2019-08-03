/***************************************************************************************************
*
*	DESCRIPTION		
*
*	NOTES			
*
***************************************************************************************************/

#ifndef _GUISKINMENUSLIDER_H_
#define _GUISKINMENUSLIDER_H_

// Includes
#include "gui/menu/guiskinmenutext.h"
#include "game/guiskinfader.h"

// Forward Declarations


/***************************************************************************************************
*
*	CLASS			CGuiSkinMenuSlider
*
*	DESCRIPTION		
*
***************************************************************************************************/

class CGuiSkinMenuSlider : public CGuiSkinMenuText
{
	typedef CGuiSkinMenuText super;
	friend class CGuiSkinFader;
public:

	// Construction Destruction
	CGuiSkinMenuSlider( void );
	virtual ~CGuiSkinMenuSlider( void );

	// Gui slider styles.
	typedef enum eGS_STYLE_BITS
	{
		GS_VALUE = 1,
		GS_GFX = 2,
	} GS_STYLE_BITS;

protected:

	//! To deal with parsing
	virtual bool	ProcessAttribute( const char* pcTitle, const char* pcValue );
	virtual bool	ProcessEnd( void );
	virtual bool	ProcessGfxBasePositionValue( const char* pcValue );
	virtual	bool	ProcessStyleValue( const char* pcValue );

	virtual bool	MoveLeftAction( int iPads );
	virtual bool	MoveRightAction( int iPads );
	virtual bool	StartAction( int iPads );
	virtual bool	SelectAction( int iPads );
	virtual void	ActivationChange( void );

	// us
	virtual bool	Render();

	void TriggerActivationChangeCallback( const bool bActive );
	void TriggerCallback();
	
	// Slider visuals.
	void UpdateSliderSprites( const bool bSmooth );
	void InitialiseSliderSprites( void );
	void RenderSliderSprites( void );
	void FadeSliderSprites( void );

	virtual void	CalculateExtents();

private:
	int m_iMinValue;
	int m_iMaxValue;
	int m_iStep;
	const char* m_pcCallback;
	const char* m_pcCallbackActivationChange;
	int m_iCurrentValue;
	float m_fExternalInitialiser;	//! Used to store an initialiser value from an external source.
									//! Will be used to set current value when the params have been
									//! processed.
	void		CreateValueString();
	CString*	m_pobValueString;

	// Slider visuals
	enum
	{
		NUM_SLIDER_BARS = 2
	};

	enum
	{
		WHITE_SLIDER_BAR = 0,
		BEIGE_SLIDER_BAR,
	};

	ScreenSprite m_obSlider;
	ScreenSprite m_obStaticBar;
	ScreenSprite m_obStaticRightCap;
	ScreenSprite m_obStaticLeftCap;	

	ScreenSprite m_obBar;
	ScreenSprite m_obRightCap;
	ScreenSprite m_obLeftCap;	

	float m_fSliderXPos;
	float m_fSliderYPos;
	float m_fSliderBarDivisionWidth;
	float m_fGfxPositionX;
	float m_fLeftCapX;
	float m_fHalfCapWidth;
	float m_fPixelsShowing;
	float m_fPixelsToShowTarget;
	//

	int	m_iStyleBits;

	bool DisplayGraphic( void ) { return (m_iStyleBits & GS_GFX); }
	bool DisplayValue( void ) { return (m_iStyleBits & GS_VALUE); }
	const bool CGuiSkinMenuSlider::SliderBarNearTargetPosition( void );
};

#endif // _GUISKINMENUSLIDER_H_
