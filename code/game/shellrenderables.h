//------------------------------------------------------------------------------------------
//!
//!	\file shellrenderables.h
//! helper classes for boot up display
//!
//------------------------------------------------------------------------------------------

#ifndef GAME_SHELLRENDERABLES_H
#define GAME_SHELLRENDERABLES_H

class TextureXDRAM;
class ScreenSprite;

//----------------------------------------------------------------------------------------
//!
//!	ShellImageDef
//!	Config struct for shell image
//!
//----------------------------------------------------------------------------------------
struct ShellImageDef
{		
	// dimensions and position are unit size, 0,0 is top left of screen
	float m_width, m_height;
	float m_x, m_y;

	// must be a string literal for this to work
	const char* m_pTexName;

	// time in seconds
	float m_fadeUp, m_fadeDown;

	// colour
	CVector m_rgba;

	ShellImageDef() :
		m_width(1.0f),
		m_height(1.0f),
		m_x(0.5f),
		m_y(0.5f),
		m_pTexName("missing.dds"),
		m_fadeUp(1.0f),
		m_fadeDown(1.0f),
		m_rgba(1.0f,1.0f,1.0f,1.0f)
	{}
};

//----------------------------------------------------------------------------------------
//!
//!	ShellImage
//!	Combines XDRAM texture, sprite and fade code
//!
//----------------------------------------------------------------------------------------
class ShellImage
{
public:
	ShellImage( const ShellImageDef& def );
	~ShellImage();

	enum IMAGE_STATE
	{
		INVISIBLE,
		FADING_UP,
		VISIBLE,
		FADING_DOWN,
	};

	IMAGE_STATE	GetStatus() const { return m_eStatus; }
	float GetAlpha() const { return m_fAlpha; }

	void FadeUp();
	void FadeDown();
	void Update( float fTimeChange );
	void Render();

private:
	ShellImageDef	m_def;
	TextureXDRAM*	m_image;
	ScreenSprite*	m_pSprite;
	IMAGE_STATE		m_eStatus;
	float			m_fTransTime;
	float			m_fAlpha;
};

//----------------------------------------------------------------------------------------
//!
//!	ShellIconDef
//!	Config struct for shell animated icons
//!
//----------------------------------------------------------------------------------------
struct ShellIconDef
{		
	// dimensions and position are unit size, 0,0 is top left of screen
	float m_width, m_height;
	float m_x, m_y;

	// time in seconds
	float m_loopTime;

	// colour
	CVector m_rgba;

	ShellIconDef() :
		m_width(1.0f),
		m_height(1.0f),
		m_x(0.5f),
		m_y(0.5f),
		m_loopTime(1.0f),
		m_rgba(1.0f,1.0f,1.0f,1.0f)
	{}
};

//----------------------------------------------------------------------------------------
//!
//!	ShellAnimIcon
//!	Combines XDRAM textures, sprite and blend code
//!
//----------------------------------------------------------------------------------------
class ShellAnimIcon
{
public:
	ShellAnimIcon( const ShellIconDef& def );
	~ShellAnimIcon();

	void SetAlpha( float fAlpha ) { m_fAlpha = fAlpha; }

	// add an image to the animation
	void AddImage( const char* pImageName );

	// render the anim sprite at a normalised point in the animation
	void RenderNormalised( float fTimeN, bool bBlend = true );

	// render the anim sprite given some arbitary time, will loop
	void RenderTime( float fTime, bool bBlend = true );

	// poke variables required for rendering
	ShellIconDef& GetDefinition() { return m_def; }

private:
	typedef ntstd::Vector< TextureXDRAM* > ImageList;

	ShellIconDef	m_def;
	ImageList		m_images;
	ScreenSprite*	m_pSprite;
	float			m_fAlpha;
};

#endif // GAME_SHELLRENDERABLES_H
