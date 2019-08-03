/***************************************************************************************************
*
*	$Header:: /game/debugtext.cpp 22    13/08/03 10:39 Simonb                                      $
*
*	Debug font and text rendering classes. 
*
*	CHANGES
*
*	05/03/2003	Simon	Created
*
***************************************************************************************************/

#include "gfx/renderer.h"
#include "gfx/shader.h"
#include "gfx/texturemanager.h"
#include "core/visualdebugger.h"
#include "gfx/graphicsdevice.h"
#include "gfx/debugtext_pc.h"

/***************************************************************************************************
*
*	FUNCTION		CDebugFont::CDebugFont
*
*	DESCRIPTION		Creates an empty font.
*
***************************************************************************************************/

DebugFont::DebugFont() : m_usMaxGlyph(0), m_ulNumGlyphs(0), m_fHeight(0.0f) {}

/***************************************************************************************************
*
*	FUNCTION		CDebugFont::Load
*
*	DESCRIPTION		Loads a font from a texture and a widths data file.
*
***************************************************************************************************/

void DebugFont::Load(Texture::Ptr pTexture, const void* pABCData)
{
	// store the texture
	m_pobTexture = pTexture;

	/*
	// corresponds to newer font maker code
	// Check version of file (to make sure it matches up with the FontMaker tool)
    const uint8_t* pData = (uint8_t*)pABCData;
    uint32_t dwFileVersion = *((uint32_t*)pData); pData += sizeof(uint32_t);

	if( dwFileVersion == 0x00000005 )
    {
		struct GLYPH_ATTR
		{
			WORD  tu1, tv1, tu2, tv2; // Texture coordinates for the image
			SHORT wOffset;            // Pixel offset for glyph start
			SHORT wWidth;             // Pixel width of the glyph
			SHORT wAdvance;           // Pixels to advance after the glyph
			WORD  wMask;              // Channel mask
		};

		// Parse the font data
		float m_fFontHeight        = *((FLOAT*)pData); pData += sizeof(FLOAT);
		float m_fFontTopPadding    = *((FLOAT*)pData); pData += sizeof(FLOAT);
		float m_fFontBottomPadding = *((FLOAT*)pData); pData += sizeof(FLOAT);
		float m_fFontYAdvance      = *((FLOAT*)pData); pData += sizeof(FLOAT);

		UNUSED(m_fFontTopPadding);
		UNUSED(m_fFontBottomPadding);
		UNUSED(m_fFontYAdvance);

		// Point to the translator string
		WCHAR_T m_cMaxGlyph			= ((WORD*)pData)[0];   pData += sizeof(WORD);
		SHORT* m_TranslatorTable	= (SHORT*)pData;       pData += sizeof(WCHAR_T)*(m_cMaxGlyph+1);

		// Read the glyph attributes from the file
		uint32_t         m_dwNumGlyphs = *((uint32_t*)pData);  pData += sizeof(uint32_t);
		GLYPH_ATTR*   m_Glyphs      = (GLYPH_ATTR*)pData;

		// patch up simons ropy old code
		D3DSURFACE_DESC stDesc;
		m_pobTexture->GetLevelDesc(0, &stDesc);

		m_fHeight = m_fFontHeight;
		m_usMaxGlyph = m_cMaxGlyph;

		m_ausTranslatorTable.Reset(NT_NEW_CHUNK(Mem::MC_GFX) WORD[m_usMaxGlyph + 1]);
		NT_MEMCPY(m_ausTranslatorTable.Get(), m_TranslatorTable, (m_usMaxGlyph + 1)*sizeof(WORD));

		m_ulNumGlyphs = m_dwNumGlyphs;
		GLYPH_ATTR*	pstGlyphs = m_Glyphs;

		m_astGlyphs.Reset(NT_NEW_CHUNK(Mem::MC_GFX) GLYPH_DATA[m_ulNumGlyphs]);
		for(uint32_t ulGlyph = 0; ulGlyph < m_ulNumGlyphs; ++ulGlyph, ++pstGlyphs)
		{
			m_astGlyphs[ulGlyph].fTexU0 = pstGlyphs->tu1 / (float)stDesc.Width;
			m_astGlyphs[ulGlyph].fTexV0 = pstGlyphs->tv1 / (float)stDesc.Height;
			m_astGlyphs[ulGlyph].fTexU1 = pstGlyphs->tu2 / (float)stDesc.Width;
			m_astGlyphs[ulGlyph].fTexV1 = pstGlyphs->tv2 / (float)stDesc.Height;
			m_astGlyphs[ulGlyph].fOffset = _R(pstGlyphs->wOffset);
			m_astGlyphs[ulGlyph].fWidth = _R(pstGlyphs->wWidth);
			m_astGlyphs[ulGlyph].fAdvance = _R(pstGlyphs->wAdvance);
		}
    }
    else
    {
		ntAssert(0);
    }
	*/

	// skip to the font height
	const char* pData = reinterpret_cast<const char*>(pABCData);
	pData += sizeof(uint32_t);

	// store the font height
	NT_MEMCPY(&m_fHeight, pData, sizeof(float));
	pData += 4*sizeof(uint32_t);

	// store the max glyph value
	m_usMaxGlyph = *reinterpret_cast<const WORD*>(pData);
	pData += sizeof(WORD);

	m_ausTranslatorTable.Reset(NT_NEW_CHUNK(Mem::MC_GFX) WORD[m_usMaxGlyph + 1]);
	NT_MEMCPY(m_ausTranslatorTable.Get(), pData, (m_usMaxGlyph + 1)*sizeof(WORD));
	pData += (m_usMaxGlyph + 1)*sizeof(WORD);

	// skip to the start of the glyph data
	m_ulNumGlyphs = *reinterpret_cast<const uint32_t*>(pData);
	pData += sizeof(uint32_t);

	struct FILE_GLYPH_ATTR
	{
		float fLeft, fTop, fRight, fBottom;
		short wOffset;
		short wWidth;
		short wAdvance;
		short wPad;
	};
	const FILE_GLYPH_ATTR* pstGlyphs = reinterpret_cast<const FILE_GLYPH_ATTR*>(pData);

	// convert each glyph
	m_astGlyphs.Reset(NT_NEW_CHUNK(Mem::MC_GFX) GLYPH_DATA[m_ulNumGlyphs]);
	for(uint32_t ulGlyph = 0; ulGlyph < m_ulNumGlyphs; ++ulGlyph, ++pstGlyphs)
	{
		m_astGlyphs[ulGlyph].fTexU0 = pstGlyphs->fLeft;
		m_astGlyphs[ulGlyph].fTexV0 = pstGlyphs->fTop;
		m_astGlyphs[ulGlyph].fTexU1 = pstGlyphs->fRight;
		m_astGlyphs[ulGlyph].fTexV1 = pstGlyphs->fBottom;
		m_astGlyphs[ulGlyph].fOffset = _R(pstGlyphs->wOffset);
		m_astGlyphs[ulGlyph].fWidth = _R(pstGlyphs->wWidth);
		m_astGlyphs[ulGlyph].fAdvance = _R(pstGlyphs->wAdvance);
	}

	// validate the data
	Validate();
}

/***************************************************************************************************
*
*	FUNCTION		CDebugFont::GetGlyphData
*
*	DESCRIPTION		Gets the glyph data for a given character.
*
***************************************************************************************************/

DebugFont::GLYPH_DATA const& DebugFont::GetGlyphData(short sChar) const
{
	// return a default glpyh for stuff out-of-range
	if(sChar < 0 || static_cast<u_short>(sChar) > m_usMaxGlyph)
		return m_astGlyphs[0];

	if (sChar == '\n')
		sChar = ' ';

	// otherwise translate to the correct glyph directly
	return m_astGlyphs[m_ausTranslatorTable[sChar]];
}

/***************************************************************************************************
*
*	FUNCTION		CDebugFont::Validate
*
*	DESCRIPTION		Validates the loaded glyph data.
*
***************************************************************************************************/

void DebugFont::Validate()
{
	// check each glyph index is in range
	for(uint32_t usChar = 0; usChar < m_usMaxGlyph; ++usChar)
		ntAssert(0 <= m_ausTranslatorTable[usChar] && m_ausTranslatorTable[usChar] < m_ulNumGlyphs);
}

/***************************************************************************************************
*
*	FUNCTION		CDebugText::CDebugText
*
*	DESCRIPTION		Creates the debug text manager, which will load the normal font off disk
*					and manually create its own shaders for rendering.
*
***************************************************************************************************/

DebugText::DebugText(int iBufferLength) 
  : m_iMaxNumVertices(4*iBufferLength), 
	m_iNumVertices(0)
{
	// load the debug font
	Texture::Ptr pobFontTexture = TextureManager::Get().LoadTexture_Neutral("pc_font.dds", true);

	// load the font data (in a horribly inefficient manner)
	static char acFileName[ MAX_PATH ];
	Util::GetFiosFilePath( TEXTURE_ROOT_PATH"pc_font.abc", acFileName );

	FILE* pData = fopen(acFileName, "rb");
	ntAssert_p(pData, ("failed to load the font data 'pc_font.abc'"));

	fseek(pData, 0, SEEK_END);
	int iNumBytes = static_cast<int>(ftell(pData));
	fseek(pData, 0, SEEK_SET);
	CScopedArray<char> acData(NT_NEW_CHUNK(Mem::MC_GFX) char[iNumBytes]);
	fread(acData.Get(), 1, iNumBytes, pData);
	fclose(pData);

	// create the font
	m_obDebugFont.Load(pobFontTexture, acData.Get());

	// load the shaders
	m_obVertexShader.SetASMFunction(
		SHADERTYPE_VERTEX, 
		"vs_1_1 \n"
		"dcl_position0 v0 \n"
		"dcl_texcoord0 v1 \n"
		"dcl_color0 v2 \n"
		"mul r0, v0, c1 \n"
		"add r0, r0, c2 \n"
		"add oPos, r0, c0 \n"
		"mov oT0.xy, v1 \n"
		"mov oD0, v2 \n"
	);
	m_obPixelShader.SetASMFunction(
		SHADERTYPE_PIXEL, 
		"ps_1_1 \n"
		"tex t0 \n"
		"mul r0, t0, v0 \n"
	);

	D3DVERTEXELEMENT9 stElements[] = 
	{
		{ 0, 0,					D3DDECLTYPE_FLOAT2,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,	0 }, 
		{ 0, 2*sizeof(float),	D3DDECLTYPE_FLOAT2,		D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,	0 }, 
		{ 0, 4*sizeof(float),	D3DDECLTYPE_D3DCOLOR,	D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,		0 }, 
		D3DDECL_END()
	};
	HRESULT hr;
	hr = GetD3DDevice()->CreateVertexDeclaration(&stElements[0], m_pobVertexDeclaration.AddressOf());
	ntAssert(SUCCEEDED(hr));

	// create the text buffer
	m_astDebugText.Reset(NT_NEW_CHUNK(Mem::MC_GFX) TEXT_VERTEX[m_iMaxNumVertices]);
}

/***************************************************************************************************
*
*	FUNCTION		CDebugText::Printf
*
*	DESCRIPTION		Prints a string to the buffer at the given location in viewport space.
*
***************************************************************************************************/

void DebugText::Printf(float fX, float fY, uint32_t dwColour, int iFlags, const char* pcBuffer, int iLength)
{
	// get the text dimensions if necessary
	const float fHeight = m_obDebugFont.GetGlyphHeight();
	float fWidth = 0.0f;
	if(iFlags != 0)
	{
		for(int iChar = 0; iChar < iLength; ++iChar)
			fWidth += m_obDebugFont.GetGlyphData(pcBuffer[iChar]).fAdvance;
	}

	// set the horizontal alignment
	if((iFlags & DTF_ALIGN_HCENTRE) != 0)
		fX -= fWidth/2.0f;
	else if((iFlags & DTF_ALIGN_RIGHT) != 0)
		fX -= fWidth;

	// set the vertical alignment
	if((iFlags & DTF_ALIGN_VCENTRE) != 0)
		fY -= fHeight/2.0f;
	else if((iFlags & DTF_ALIGN_BOTTOM) != 0)
		fY -= fHeight;

	// set the start position for the text
	float fLeft = fX, fTop = fY;

	// queue up quads to render the text
	for(int iChar = 0; iChar < iLength; ++iChar)
	{
		DebugFont::GLYPH_DATA const& stGlyph = m_obDebugFont.GetGlyphData(pcBuffer[iChar]);

		// set the positions
		fLeft += stGlyph.fOffset;
		CDirection obTopLeft(fLeft, fTop, 0.0f);
		CDirection obBottomRight(fLeft + stGlyph.fWidth, fTop + fHeight, 0.0f);
		fLeft += stGlyph.fAdvance;

		// check we have space for the next character
		if(m_iNumVertices + 6 > m_iMaxNumVertices)
			break;

		// set the positions
		m_astDebugText[m_iNumVertices + 0].fX 
			= m_astDebugText[m_iNumVertices + 1].fX
			= m_astDebugText[m_iNumVertices + 5].fX = obTopLeft.X();
		m_astDebugText[m_iNumVertices + 2].fX 
			= m_astDebugText[m_iNumVertices + 3].fX 
			= m_astDebugText[m_iNumVertices + 4].fX = obBottomRight.X();

		m_astDebugText[m_iNumVertices + 1].fY 
			= m_astDebugText[m_iNumVertices + 2].fY 
			= m_astDebugText[m_iNumVertices + 3].fY = obTopLeft.Y();
		m_astDebugText[m_iNumVertices + 0].fY 
			= m_astDebugText[m_iNumVertices + 4].fY 
			= m_astDebugText[m_iNumVertices + 5].fY = obBottomRight.Y();

		// set the texture coordinates
		m_astDebugText[m_iNumVertices + 0].fU = stGlyph.fTexU0;
		m_astDebugText[m_iNumVertices + 1].fU = stGlyph.fTexU0;
		m_astDebugText[m_iNumVertices + 2].fU = stGlyph.fTexU1;
		m_astDebugText[m_iNumVertices + 3].fU = stGlyph.fTexU1;
		m_astDebugText[m_iNumVertices + 4].fU = stGlyph.fTexU1;
		m_astDebugText[m_iNumVertices + 5].fU = stGlyph.fTexU0;

		m_astDebugText[m_iNumVertices + 0].fV = stGlyph.fTexV1;
		m_astDebugText[m_iNumVertices + 1].fV = stGlyph.fTexV0;
		m_astDebugText[m_iNumVertices + 2].fV = stGlyph.fTexV0;
		m_astDebugText[m_iNumVertices + 3].fV = stGlyph.fTexV0;
		m_astDebugText[m_iNumVertices + 4].fV = stGlyph.fTexV1;
		m_astDebugText[m_iNumVertices + 5].fV = stGlyph.fTexV1;

		// set the colour
		m_astDebugText[m_iNumVertices + 0].dwColour = dwColour;
		m_astDebugText[m_iNumVertices + 1].dwColour = dwColour;
		m_astDebugText[m_iNumVertices + 2].dwColour = dwColour;
		m_astDebugText[m_iNumVertices + 3].dwColour = dwColour;
		m_astDebugText[m_iNumVertices + 4].dwColour = dwColour;
		m_astDebugText[m_iNumVertices + 5].dwColour = dwColour;

		// advance
		m_iNumVertices += 6;
	}
}

/***************************************************************************************************
*
*	FUNCTION		CDebugText::Flush
*
*	DESCRIPTION		Renders the current buffer to the viewport.
*
***************************************************************************************************/

void DebugText::Flush(CDirection const& obScale, CDirection const& obOffset)
{
	if(m_iNumVertices != 0)
	{
		// set the state
		Renderer::Get().SetBlendMode( GFX_BLENDMODE_LERP );
		Renderer::Get().SetZBufferMode( GFX_ZMODE_DISABLED );

		Renderer::Get().SetVertexShader(&m_obVertexShader);
		Renderer::Get().SetPixelShader(&m_obPixelShader);
		Renderer::Get().m_Platform.SetVertexDeclaration( m_pobVertexDeclaration );

		CVector obScaleW(obScale);
		obScaleW.W() = 1.0f;
		Renderer::Get().SetVertexShaderConstant(1, &obScaleW, 1);
		Renderer::Get().SetVertexShaderConstant(2, &obOffset, 1);
		
		Renderer::Get().SetTexture( 0, m_obDebugFont.GetTexture() );
		Renderer::Get().SetSamplerAddressMode(0, TEXTUREADDRESS_CLAMPALL);
		Renderer::Get().SetSamplerFilterMode(0, TEXTUREFILTER_BILINEAR);

		GetD3DDevice()->DrawPrimitiveUP(D3DPT_TRIANGLELIST, m_iNumVertices/3, m_astDebugText.Get(), sizeof(TEXT_VERTEX));

		// reset the state
		Renderer::Get().SetTexture( 0, Texture::NONE );
		Renderer::Get().SetBlendMode( GFX_BLENDMODE_NORMAL );
		Renderer::Get().SetZBufferMode( GFX_ZMODE_NORMAL );

		// reset the text
		m_iNumVertices = 0;
	}
}
