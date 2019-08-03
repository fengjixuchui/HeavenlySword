/***************************************************************************************************
*
*	$Header:: /game/material.cpp 8     13/08/03 10:39 Simonb                                       $
*
*
*
*	CHANGES
*
*	2/7/2003	SimonB	Created
*
***************************************************************************************************/

#include "gfx/material.h"
#include "gfx/shader.h"
#include "gfx/renderer.h"
#include "gfx/hardwarecaps.h"
#include "core/file.h"



/***************************************************************************************************
*
*	FUNCTION		CShaderDictionary::FindVertexShader
*
*	DESCRIPTION		Finds a vertex shader by name. If no shader is found we return 0.
*
***************************************************************************************************/

Shader* CShaderDictionary::FindShader(const char* pcName) const
{
#ifdef PLATFORM_PS3
	if ( m_pDictionaryData != NULL )
#endif
	{
		// do a linear search for the shader
		for(int iShader = 0; iShader < m_iNumShaders; ++iShader)
		{
			if(strcmp(pcName, m_pobShaders[iShader].GetName()) == 0)
				return &m_pobShaders[iShader];
		}

		// shader not found
		
		return 0;
	}
#ifdef PLATFORM_PS3
	else return 0;
#endif
}


