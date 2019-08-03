/***************************************************************************************************
*
*	$Header:: /game/effects.h 3     13/08/03 10:39 Simonb                                          $
*
*	Includes the PC effects if we're on the PC.
*
*	CHANGES
*
*	22/5/2003	SimonB	Created
*
***************************************************************************************************/

#ifndef _EFFECT_SKY_PC_H
#define _EFFECT_SKY_PC_H

#include "gfx/texture.h"
#include "gfx/vertexdeclaration.h"

class Shader;
class DebugShader;

/***************************************************************************************************
*
*	CLASS			SkyEffect
*
*	DESCRIPTION		Temporary CIE sky model effect.
*
***************************************************************************************************/

class SkyEffect
{
public:
	SkyEffect(int iGridWidth, int iGridHeight);
	~SkyEffect();

	void Render();

private:
	int m_iGridWidth, m_iGridHeight;
	int m_iNumVertices, m_iNumIndices;

	VBHandle m_pobVertices;
	IBHandle m_pobIndices;

	Shader* m_pobVertexShader;
	Shader* m_pobPixelShader;
	CVertexDeclaration m_pobDecl;

	DebugShader* m_pDebugVertexShader;
	DebugShader* m_pDebugPixelShader;
};

#endif // ndef _EFFECT_SKY_PC_H