/***************************************************************************************************
*
*	DESCRIPTION		
*
*	NOTES			
*
***************************************************************************************************/

#ifndef _GUISKINMENUIMAGE_H_
#define _GUISKINMENUIMAGE_H_

// Includes
#include "gui/guiunit.h"
#include "effect/screensprite.h"
#include "effect/rotscreensprite.h"
#include "game/guiskinfader.h"

/***************************************************************************************************
*
*	CLASS			CGuiSkinMenuImage
*
*	DESCRIPTION		
*
***************************************************************************************************/

class CGuiSkinMenuImage : public CGuiUnit
{
	typedef CGuiUnit super;
	friend class CGuiSkinFader;
public:

	// Construction Destruction
	CGuiSkinMenuImage( void );
	virtual ~CGuiSkinMenuImage( void );

protected:

	//! To deal with parsing
	virtual bool	ProcessAttribute( const char* pcTitle, const char* pcValue );
	virtual bool	ProcessEnd( void );

	//! System
	virtual bool	Render( void );

	//! Us
	bool			SetImage(const char* pcValue);
	bool			SetImageSize(const char* pcValue);
	bool			SetImageSizeType(const char* pcValue);

	//internal
	void			UpdateImageSize();

	enum IMAGESIZETYPE
	{
		SIZE_RELATIVE,
		SIZE_PIXEL
	};

private:
	
	ScreenSprite m_obImage;
	RotScreenSprite m_obRotImage;

	bool m_bTextureSet;

	float m_fWidth;
	float m_fHeight;
	float m_fRadiansSecond;
	float m_fCurrentAngle;

	bool m_bConstructing;
	bool m_bUseRotSprite;

	IMAGESIZETYPE m_eImageSizeType;
};

#endif // _GUISKINMENUIMAGE_H_
