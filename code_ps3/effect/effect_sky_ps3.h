/***************************************************************************************************
*
*	DESCRIPTION
*
*	NOTES
*
***************************************************************************************************/

#ifndef EFFECT_SKY_PS3_H
#define EFFECT_SKY_PS3_H

//#include "gfx/texture.h"

#ifndef GFX_VERTEXDECLARATION_H
#include "gfx/vertexdeclaration.h"
#endif

#ifndef GFX_SHADER_H
#include "gfx/shader.h"
#endif

//--------------------------------------------------
//!
//!	new SkyEffect class
//!
//--------------------------------------------------
class SkyEffect
{
public:
	SkyEffect(int iGridWidth, int iGridHeight);
	void Render();

private:
	struct vertex
	{
		float position[2];
	};

	int m_iGridWidth, m_iGridHeight;
	int m_iNumVertices, m_iNumIndices;

	VBHandle m_pVertices;
	IBHandle m_pIndices;

	DebugShader *m_vertexShader;
	DebugShader *m_pixelShader;
};

#endif // ndef EFFECT_SKY_PS3_H
