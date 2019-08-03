/***************************************************************************************************
*
*	DESCRIPTION
*
*	NOTES
*
***************************************************************************************************/

#ifndef _SKYBOX_H
#define _SKYBOX_H

#include "renderable.h"

class CShader;

class CSkyBoxEffect : public CRenderable
{
public:
	explicit CSkyBoxEffect(CCubeTexture const& pobTexture);

private:
	const CShader *m_pobVertexShader, *m_pobPixelShader;	//!< The shaders.
	CVertexDeclaration m_pobDeclaration;					//!< The vertex declaration for this effect.
	CCubeTexture m_pobTexture;								//!< The skybox cube texture.
};

#endif // ndef _SKYBOX_H
