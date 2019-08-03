//--------------------------------------------------------------------------------------------------
/**
	@file		GpBasicText.h

	@brief		debug text stuff

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef GP_BASIC_TEXT_H
#define GP_BASIC_TEXT_H

//--------------------------------------------------------------------------------------------------
//  INCLUDES
//--------------------------------------------------------------------------------------------------

#include	<Fw/FwMaths/FwMatrix44.h>
#include	<Fw/FwMaths/FwVector4.h>
#include	<Fw/FwMaths/FwPoint.h>

#include	<Fw/FwResource.h>

#include	<Fw/FwStd/FwHashedString.h>

#include	<Gc/GcColour.h>
#include	<Gc/GcTexture.h>
#include	<Gc/GcShader.h>
#include	<Gc/GcStreamBuffer.h>

//--------------------------------------------------------------------------------------------------
//  SWITCHES
//--------------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------------
//  FORWARD DECLARATIONS
//--------------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------------
//  NAMESPACE DEFINITIONS
//--------------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------------
//  CLASS DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@class			GpBasicText

	@brief			texture-based debug font
**/
//--------------------------------------------------------------------------------------------------

class GpBasicText
{
public:

	// Enumerations
	
	enum Font
	{
		kFontSmall,
		kFontRegular,
		kFontLarge,
	
		kFontMonoSmall,
		kFontMonoRegular,
		kFontMonoLarge,

		kNumFonts,
	};

	
	//  Constants
	
	/// Default maximum number of characters.
	static const int	kDefaultMaxChars = 512;
	
	
	// Initialisation and Shutdown Functions
	
	static void			Initialise( int maxChars = kDefaultMaxChars );
	static void			Shutdown();

	
	// Draw Functions
	
	static void			Draw2D( const char* pString, float x, float y, GcColour_arg rgba=Gc::kColourWhite, Font font=kFontRegular, float width=1.0f, float height=1.0f );

	static void			Draw2D( const char* pString, FwPoint_arg position, GcColour_arg rgba=Gc::kColourWhite, Font font=kFontRegular, float width=1.0f, float height=1.0f );

	
	// Flush
	
	static void			Flush();
	
	
	// Miscellaneous
	
	static float		GetDraw2DWidth( const char* pString, Font font=kFontRegular, float width=1.0f );
	static float		GetDraw2DHeight( Font font=kFontRegular, float height=1.0f );
	
	
	// Accessors
	
	static void			SetViewportSize( int width, int height );

	static void			SetWorldMatrix(const FwMatrix44& worldMtx);
	static void			SetViewMatrix(const FwMatrix44& viewMtx);
	static void			SetProjectionMatrix(const FwMatrix44& projMtx);

	static FwMatrix44	GetWorldMatrix();
	static FwMatrix44	GetViewMatrix();
	static FwMatrix44	GetProjectionMatrix();
	
	static void			SetAllTransforms(	const FwMatrix44&	worldMtx,
											const FwMatrix44&	viewMtx,
											const FwMatrix44&	projMtx);
	
	static void			GetAllTransforms(	FwMatrix44&	worldMtx,
											FwMatrix44&	viewMtx,
											FwMatrix44&	projMtx);

	static void			SetScale2D(float sx, float sy);
	static void			GetScale2D(float& sx, float& sy);
	
	static float		GetFontHeight(Font font = kFontRegular);


protected:

	// Constants

	static const uint	kMaxStringLen	= 256;
	static const uint	kMaxPathLen		= 256;
	
	
	// Structure Definitions
	
	struct FontDef
	{
		u8*		m_pData;						///< Pointer to the font data
		unsigned long	m_dataSize;						///< Size of the font data
		float	m_subTexPixelHeight;			///< Height of font sub-texture in pixels.
	};

	struct FontHeader
	{
		u32		m_tag;
		float	m_height;
		float	m_topPad;
		float	m_bottomPad;
		float	m_yAdvance;
		u16		m_maxGlyph;
		u16		m_glyphs[1];
	};

	struct GlyphDef
	{
		float	m_left, m_top, m_right, m_bottom;		///< texture coords
		s16		m_offset;								///< space before glyph
		s16		m_width;								///< width of glyph
		s16		m_advance;								///< width of glyph plus space after glyph
		s16		m_pad;
	};

	struct FontData
	{
		u8*					m_pResourceData;	///< Font resource data.
		
		FontHeader*			m_pHeader;			///< Font header.
		GlyphDef*			m_pGlyphs;			///< Font glyph definitions array.
	};

	struct Vertex	///< :NOTE: Definition must match the vertex GcStreamBuffer format descriptor!
	{
		float		m_position[4];				///< Clip space vertex position.
		u32			m_colour;					///< Packed RGBA8 colour.
		float		m_texCoord[2];	   			///< Texture coordinate.
	};
	
	
	// Attributes
	
	static int							ms_maxChars;
	
	static bool							ms_matricesInvalid;
	
	static FwMatrix44					ms_worldMtx;
	static FwMatrix44					ms_viewMtx;
	static FwMatrix44					ms_projMtx;
	static FwMatrix44					ms_worldViewProjMtx;

	static float						ms_xScale2d;				///< 2D text (x, y) global scale.
	static float						ms_yScale2d;
	
	static float						ms_pixelW;					///< Pixel dimensions in clip space.
	static float						ms_pixelH;
    
	static const FontDef				ms_fontDefs[kNumFonts];
	static FontData						ms_fontData[kNumFonts];
	
	static u64							ms_lastFlushFrame;			//!< The last frame we flushed on.
	static Vertex*						ms_pVertexBuffer;			///< The current vertex buffer.
	static Vertex*						ms_pTempBuffer;				///< The temp vertex buffer.
	static int							ms_startChar;
	static int							ms_currentChar;
	static GcStreamBufferHandle			ms_hVertexStream;
	
	static GcShaderHandle				ms_hTexturedTextVertexShader;
    static GcShaderHandle				ms_hTexturedTextFragmentShader;
	
	static GcTextureHandle				ms_hTexture;				///< Font texture (contains all fonts - in order).
	static uint							ms_fontSamplerIndex;

	// Operations
	
	static void		UpdateTransforms();

	static void		CheckVertexBuffer();
	
	static void		DrawInternal(	const FwMatrix44&	worldViewProjMtx,
									const char*			pString,
									float				width,
									float				height,
									GcColour_arg		rgba,
									Font				font );
};

//--------------------------------------------------------------------------------------------------
//  INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

inline void	GpBasicText::SetViewportSize( int width, int height )
{
	// :NOTE: Is used to calculate the pixel size in clip space for 2D text.
	ms_pixelW = 2.0f / float( width );
	ms_pixelH = 2.0f / float( height );
}

//--------------------------------------------------------------------------------------------------

inline void GpBasicText::SetWorldMatrix(const FwMatrix44& worldMtx)
{
	ms_worldMtx = worldMtx;
	ms_matricesInvalid = true;
}

//--------------------------------------------------------------------------------------------------

inline void GpBasicText::SetViewMatrix(const FwMatrix44& viewMtx)
{
	ms_viewMtx = viewMtx;
	ms_matricesInvalid = true;
}

//--------------------------------------------------------------------------------------------------

inline void GpBasicText::SetProjectionMatrix(const FwMatrix44& projMtx)
{
	ms_projMtx = projMtx;
	ms_matricesInvalid = true;
}

//--------------------------------------------------------------------------------------------------

inline FwMatrix44 GpBasicText::GetWorldMatrix()
{
	return ms_worldMtx;
}

//--------------------------------------------------------------------------------------------------

inline FwMatrix44 GpBasicText::GetViewMatrix()
{
	return ms_viewMtx;
}

//--------------------------------------------------------------------------------------------------

inline FwMatrix44 GpBasicText::GetProjectionMatrix()
{
	return ms_projMtx;
}

//--------------------------------------------------------------------------------------------------

inline void GpBasicText::SetAllTransforms(	const FwMatrix44&	worldMtx,
											const FwMatrix44&	viewMtx,
											const FwMatrix44&	projMtx)
{
	ms_worldMtx = worldMtx;
	ms_viewMtx = viewMtx;
	ms_projMtx = projMtx;

	ms_matricesInvalid = true;
}
	
//--------------------------------------------------------------------------------------------------

inline void GpBasicText::GetAllTransforms(	FwMatrix44&	worldMtx,
											FwMatrix44&	viewMtx,
											FwMatrix44&	projMtx)
{
	worldMtx = ms_worldMtx;
	viewMtx = ms_viewMtx;
	projMtx = ms_projMtx;
}

//--------------------------------------------------------------------------------------------------

inline void GpBasicText::SetScale2D(float sx, float sy)
{
	// Set global scale for 2D text.
	//
	// This can be useful for rendering text at the correct size when using a super-sampled Render Buffer
	// that is then down-sampled for display.
	
	FW_ASSERT((sx > 0.0f) && (sy > 0.0f)); 
	
	ms_xScale2d = sx;
	ms_yScale2d = sy;
}

//--------------------------------------------------------------------------------------------------

inline void GpBasicText::GetScale2D(float& sx, float& sy)
{
	sx = ms_xScale2d;
	sy = ms_yScale2d;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Retrieve the height in pixels of the specified font.

	@note			This is currently the height between rows in the font texture. Ideally it would 
					actually reflect more closely the height of the font characters.
**/
//--------------------------------------------------------------------------------------------------

inline float GpBasicText::GetFontHeight( Font font )
{

	FontHeader*		pFontDef = ms_fontData[font].m_pHeader;
	
	return pFontDef->m_yAdvance;
}

//--------------------------------------------------------------------------------------------------

#endif // GP_BASIC_TEXT_H
