//--------------------------------------------------
//!
//!	\file texture_function.cpp
//!	Objects that collect FunctionCurve_User objects
//! and approximate them into a texture or
//! float array.
//!
//--------------------------------------------------

#include "texture_function.h"
#include "gfx/TextureReader.h"
#include "gfx/renderer.h"
#include "gfx/hardwarecaps.h"
#include "gfx/surfacemanager.h"
#include "functioncurve.h"
#include "effect_manager.h"

//--------------------------------------------------
//!
//! FunctionTableGenerator::GenerateTexture
//! Make a texture called pName containing our
//! functions. If we're sparse, none existant
//! functions end up with zeros in the textre
//!
//--------------------------------------------------
Texture::Ptr FunctionTableGenerator::GenerateTexture( u_int iFunctionWidth, u_int iResolution, bool b8bit, bool bSaveToDisk, const char* pName )
{
	Texture::Ptr result;

	if (EffectManager::AllocsDisabled())
		return result;

	u_int iTexWidth = iFunctionWidth * GetTableSize();

	// create texture of the right format
	GFXFORMAT format = GF_A8;

	if (!b8bit)
	{
		if ( HardwareCapabilities::Get().IsValidTextureFormat(GF_R16F) )
			format = GF_R16F;
		else if ( HardwareCapabilities::Get().IsValidTextureFormat(GF_R32F) )
			format = GF_R32F;
	}

	result = SurfaceManager::Get().CreateTexture( iResolution, iTexWidth, format );

	// lock it and fill it in with our colour values
	uint32_t pitch;
	TextureReader tex_it( result->CPULock2D(pitch), format );
	
	float fSampleStep = 1.0f / _R(iResolution);
	float fSampleStart = fSampleStep / 2.0f;

	// for every function in the list
	for (	ntstd::List<const FunctionCurve_User*>::iterator it = m_functions.begin();
			it != m_functions.end(); ++it )
	{
		// for every repeat of the function
		for ( u_int i = 0; i < iFunctionWidth; i++ )
		{
			// sample the curve and blat it into the texture
			float fU = fSampleStart;
			float fVal = 0.0f;
			for ( u_int j = 0; j < iResolution; j++, tex_it.Next() )
			{
				if (*it)
				{
					fVal = (*it)->EvaluateScaledAndOffset(fU);
					fU += fSampleStep;
				}
				tex_it.Set( CVector(fVal,fVal,fVal,fVal) );
			}
		}
	}
	
	result->CPUUnlock2D();

#ifdef PLATFORM_PC // FIXME_WIL

	// now dump it to file 
	if (bSaveToDisk)
		result->m_Platform.SaveToDisk( pName, D3DXIFF_DDS, true, true );

#endif

	return result;
}

//--------------------------------------------------
//!
//! FunctionTableGenerator::GenerateTexture
//! Make a float array containing our functions.
//! If we're sparse, non-existant
//! functions are NOT output into the array
//!
//--------------------------------------------------
void FunctionTableGenerator::GenerateFunctionTable( u_int iResolution, float** ppTablePtr )
{
	ntAssert( *ppTablePtr == 0 );

	// ATTN! this float array is the size of the actual number
	// of functions, rather than 'table' we represent

	int iTableSize = iResolution * GetNumFunctions();
	*ppTablePtr = NT_NEW_ARRAY_CHUNK( Mem::MC_EFFECTS ) float [iTableSize];
	float* pCurr = *ppTablePtr;

	float fSampleStep = 1.0f / _R(iResolution);
	float fSampleStart = fSampleStep / 2.0f;

	// for every function in the list
	for (	ntstd::List<const FunctionCurve_User*>::iterator it = m_functions.begin();
			it != m_functions.end(); ++it )
	{
		if (*it) // only if this function exists
		{
			// sample the curve and blat it into the table
			float fU = fSampleStart;
			for ( u_int i = 0; i < iResolution; i++, pCurr++, iTableSize-- )
			{
				*pCurr = (*it)->EvaluateScaledAndOffset(fU);
				fU += fSampleStep;
			}
		}
	}

	ntAssert(iTableSize == 0);
}
