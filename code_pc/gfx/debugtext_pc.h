/***************************************************************************************************
*
*	$Header:: /game/debugtext.h 8     13/08/03 10:39 Simonb                                        $
*
*	Debug font and text rendering classes. 
*
*	CHANGES
*
*	05/03/2003	Simon	Created
*
***************************************************************************************************/

#ifndef _DEBUGTEXT_PC_H
#define _DEBUGTEXT_PC_H

#include "gfx/graphicsdevice.h"
#include "gfx/shader.h"
#include "gfx/vertexdeclaration.h"
#include "gfx/texture.h"

/***************************************************************************************************
*
*	CLASS			CDebugFont
*
*	DESCRIPTION		A simple container for a font texture.
*
***************************************************************************************************/

//! A debug font class for rendering debug text.
class DebugFont
{
public:
	//! Creates an empty font.
	DebugFont();

	//! Sets the font texture and loads the glyph data.
	void Load(Texture::Ptr pTexture, const void* pABCData);

	//! The data type for a glyph.
	struct GLYPH_DATA
	{
		float fTexU0, fTexV0;
		float fTexU1, fTexV1;
		float fOffset, fWidth, fAdvance;
	};

	//! Gets the glyph data for a given character.
	GLYPH_DATA const& GetGlyphData(short sChar) const;

	//! Gets the height of each glyph in the font.
	float GetGlyphHeight() const { return m_fHeight; }

	//! Gets the font texture.
	Texture::Ptr GetTexture() const { return m_pobTexture; }

private:
	//! Validates the data structures in the font.
	void Validate();

	Texture::Ptr m_pobTexture;						//!< The font texture.
	
	uint16_t m_usMaxGlyph;							//!< The highest character value in the font.
	CScopedArray<uint16_t> m_ausTranslatorTable;	//!< A lookup table to translate character values into glyph values.

	uint32_t m_ulNumGlyphs;						//!< The number of glyphs in the font texture.
	CScopedArray<GLYPH_DATA> m_astGlyphs;		//!< The data for each glyph in the font texture.
	float m_fHeight;							//!< The font height.
};

/***************************************************************************************************
*
*	CLASS			CDebugText
*
*	DESCRIPTION		A simple container for a text quadlist.
*
***************************************************************************************************/

//! A debug text class for caching and rendering debug text.
class DebugText
{
public:
	//! Creates a new text buffer of the given maximum length in characters.
	explicit DebugText(int iBufferLength);

	//! Renders debug text into the current viewport at the given position.
	void Printf(float fX, float fY, uint32_t dwColour, int iFlags, const char* pcBuffer, int iLength);

	//! Flushes the debug text cache out to the frame buffer.
	void Flush(CDirection const& obScale, CDirection const& obOffset);

private:
	DebugFont m_obDebugFont;					//!< The debug font, used for rendering debug text.

	DebugShader m_obVertexShader;				//!< The vertex shader used for rendering.
	DebugShader m_obPixelShader;				//!< The pixel shader used for rendering.
	CVertexDeclaration m_pobVertexDeclaration;	//!< The vertex declaration used for rendering.
	
	//! The data structure for debug text.
	struct TEXT_VERTEX 
	{ 
		float fX, fY;	//!< Vertex position.
		float fU, fV;	//!< Texture coordinate.
		uint32_t dwColour;	//!< Text colour.
	};

    CScopedArray<TEXT_VERTEX> m_astDebugText;	//!< The buffer for debug text.
	int m_iMaxNumVertices;						//!< The length of the buffer in vertices.
	int m_iNumVertices;							//!< The current number of text vertices in the buffer.
};

#endif // ndef _DEBUGTEXT_H
