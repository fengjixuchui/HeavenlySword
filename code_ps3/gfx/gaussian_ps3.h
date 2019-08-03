/***************************************************************************************************
*
*	DESCRIPTION
*
*	NOTES
*
***************************************************************************************************/

#ifndef GFX_GAUSSIAN_PC_H
#define GFX_GAUSSIAN_PC_H

#include "gfx/rendertarget.h"
#include "gfx/vertexdeclaration.h"
#include "gfx/shader.h"

//--------------------------------------------------
//!
//!	NewGaussianBlur
//! blur helper class
//!
//--------------------------------------------------
class NewGaussianBlur
{
public:
	NewGaussianBlur();

	void RecursiveBlur( RenderTarget::Ptr& pSource, 
						GFXFORMAT fmt,
						const CVector& weight, 
						int iPassStart, 
						int iPassEnd );

private:
	DebugShader *	m_vertexShader;
	DebugShader	*	m_pixelShader;
	VBHandle		m_hSimpleQuadData;
};

#endif // ndef GFX_GAUSSIAN_PC_H
