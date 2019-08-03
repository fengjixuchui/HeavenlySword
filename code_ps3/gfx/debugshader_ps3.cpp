//------------------------------------------------------------------------------------------
//!
//!	\file debugshader_ps3.h
//!
//------------------------------------------------------------------------------------------

#include "gfx/debugshader_ps3.h"
#include "gfx/graphicsdevice.h"
#include "core/fileattribute.h"

//------------------------------------------------------------------------------------------
//!
//!	DebugShaderCache::DebugShaderCache
//! Prime the cache with commonly used shaders
//!
//------------------------------------------------------------------------------------------
DebugShaderCache::DebugShaderCache()
{
	DebugShaderCache::Get().LoadShader( "worldsprite_vp.sho" );
	DebugShaderCache::Get().LoadShader( "screensprite_vp.sho" );
	DebugShaderCache::Get().LoadShader( "screensprite_fp.sho" );
	DebugShaderCache::Get().LoadShader( "rotscreensprite_vp.sho" );
	DebugShaderCache::Get().LoadShader( "moviesprite_vp.sho" );
	DebugShaderCache::Get().LoadShader( "moviesprite_fp.sho" );
	
	DebugShaderCache::Get().LoadShader( "effecttrail_line_vp.sho" );
	DebugShaderCache::Get().LoadShader( "effecttrail_simple_vp.sho" );
	DebugShaderCache::Get().LoadShader( "effecttrail_simple_notex_fp.sho" );
	DebugShaderCache::Get().LoadShader( "effecttrail_simple_tex_fp.sho" );
	DebugShaderCache::Get().LoadShader( "effecttrail_simple_animtex_fp.sho" );

	DebugShaderCache::Get().LoadShader( "weaponchains_shadow_vp.sho" );
	DebugShaderCache::Get().LoadShader( "weaponchains_vp.sho" );
	DebugShaderCache::Get().LoadShader( "weaponchains_fp.sho" );
	DebugShaderCache::Get().LoadShader( "weaponchains_depth_vp.sho" );
	DebugShaderCache::Get().LoadShader( "weaponchains_depth_fp.sho" );

	// particle system vertex shaders
	//---------------------------------------------------------
	DebugShaderCache::Get().LoadShader( "psystem_simple_cpu_point_vp.sho" );
	DebugShaderCache::Get().LoadShader( "psystem_simple_gpu_point_vp.sho" );
	DebugShaderCache::Get().LoadShader( "psystem_simple_cpu_quad_vp.sho" );
	DebugShaderCache::Get().LoadShader( "psystem_simple_gpu_quad_vp.sho" );
	DebugShaderCache::Get().LoadShader( "psystem_simple_rot_cpu_quad_vp.sho" );
	DebugShaderCache::Get().LoadShader( "psystem_simple_rot_gpu_quad_vp.sho" );
	DebugShaderCache::Get().LoadShader( "psystem_simple_orient_cpu_quad_vp.sho" );
	DebugShaderCache::Get().LoadShader( "psystem_simple_orient_gpu_quad_vp.sho" );
	DebugShaderCache::Get().LoadShader( "psystem_simple_axisray_cpu_vp.sho" );
	DebugShaderCache::Get().LoadShader( "psystem_simple_axisray_gpu_vp.sho" );
	DebugShaderCache::Get().LoadShader( "psystem_simple_velscaleray_cpu_vp.sho" );
	DebugShaderCache::Get().LoadShader( "psystem_simple_velscaleray_gpu_vp.sho" );

	DebugShaderCache::Get().LoadShader( "psystem_simple_point_notex_fp.sho" );
	DebugShaderCache::Get().LoadShader( "psystem_simple_point_tex_fp.sho" );
	DebugShaderCache::Get().LoadShader( "psystem_simple_point_animtex_fp.sho" );
	DebugShaderCache::Get().LoadShader( "psystem_simple_point_randtex_fp.sho" );
	DebugShaderCache::Get().LoadShader( "psystem_simple_quad_notex_fp.sho" );
	DebugShaderCache::Get().LoadShader( "psystem_simple_quad_tex_fp.sho" );
	DebugShaderCache::Get().LoadShader( "psystem_simple_quad_animtex_fp.sho" );
	DebugShaderCache::Get().LoadShader( "psystem_simple_quad_randtex_fp.sho" );

	// particle system pixel shaders
	//---------------------------------------------------------
	DebugShaderCache::Get().LoadShader( "psystem_complex_cpu_point_vp.sho" );
	DebugShaderCache::Get().LoadShader( "psystem_complex_gpu_point_vp.sho" );
	DebugShaderCache::Get().LoadShader( "psystem_complex_cpu_quad_vp.sho" );
	DebugShaderCache::Get().LoadShader( "psystem_complex_gpu_quad_vp.sho" );
	DebugShaderCache::Get().LoadShader( "psystem_complex_rot_cpu_quad_vp.sho" );
	DebugShaderCache::Get().LoadShader( "psystem_complex_rot_gpu_quad_vp.sho" );
	DebugShaderCache::Get().LoadShader( "psystem_complex_orient_cpu_quad_vp.sho" );
	DebugShaderCache::Get().LoadShader( "psystem_complex_orient_gpu_quad_vp.sho" );
	DebugShaderCache::Get().LoadShader( "psystem_complex_axisray_cpu_vp.sho" );
	DebugShaderCache::Get().LoadShader( "psystem_complex_axisray_gpu_vp.sho" );
	DebugShaderCache::Get().LoadShader( "psystem_complex_velscaleray_cpu_vp.sho" );
	DebugShaderCache::Get().LoadShader( "psystem_complex_velscaleray_gpu_vp.sho" );

	DebugShaderCache::Get().LoadShader( "psystem_complex_point_notex_fp.sho" );
	DebugShaderCache::Get().LoadShader( "psystem_complex_point_tex_fp.sho" );
	DebugShaderCache::Get().LoadShader( "psystem_complex_point_animtex_fp.sho" );
	DebugShaderCache::Get().LoadShader( "psystem_complex_point_randtex_fp.sho" );
	DebugShaderCache::Get().LoadShader( "psystem_complex_quad_notex_fp.sho" );
	DebugShaderCache::Get().LoadShader( "psystem_complex_quad_tex_fp.sho" );
	DebugShaderCache::Get().LoadShader( "psystem_complex_quad_animtex_fp.sho" );
	DebugShaderCache::Get().LoadShader( "psystem_complex_quad_randtex_fp.sho" );

	// Misc' shaders.
	//---------------------------------------------------------
	DebugShaderCache::Get().LoadShader( "passthrough_pos_tex_vp.sho" );
	DebugShaderCache::Get().LoadShader( "passthrough_pos_vp.sho" );
	DebugShaderCache::Get().LoadShader( "simpletexture_fp.sho" );
	DebugShaderCache::Get().LoadShader( "radialblurdownsample_fp.sho" );
	DebugShaderCache::Get().LoadShader( "resample_4tap_vp.sho" );
	DebugShaderCache::Get().LoadShader( "simpletexture4_lerp_fp.sho" );
	DebugShaderCache::Get().LoadShader( "simpletexcol_lerp_fp.sho" );
	DebugShaderCache::Get().LoadShader( "black_fp.sho" );
	DebugShaderCache::Get().LoadShader( "white_fp.sho" );
	DebugShaderCache::Get().LoadShader( "fullscreen_vp.sho" );
	DebugShaderCache::Get().LoadShader( "aaresolver_vp.sho" );
	DebugShaderCache::Get().LoadShader( "aaresolver_bilinear_fp.sho" );
	DebugShaderCache::Get().LoadShader( "aaresolver_gaussian_fp.sho" );
	DebugShaderCache::Get().LoadShader( "skylight_vp.sho" );
	DebugShaderCache::Get().LoadShader( "skylight_fp.sho" );

	DebugShaderCache::Get().LoadShader( "exposure_recursivesum_vp.sho" );
	DebugShaderCache::Get().LoadShader( "exposure_recursivesum_fp.sho" );
	DebugShaderCache::Get().LoadShader( "exposure_logfilter_fp.sho" );
	DebugShaderCache::Get().LoadShader( "exposure_interpolation_fp.sho" );
	DebugShaderCache::Get().LoadShader( "exposure_srgbconv_fp.sho" );
	
	DebugShaderCache::Get().LoadShader( "irradiance_vp.sho" );
	DebugShaderCache::Get().LoadShader( "irradiance_fp.sho" );
	DebugShaderCache::Get().LoadShader( "irradiance_mipmaps_vp.sho" );
	DebugShaderCache::Get().LoadShader( "irradiance_mipmaps_fp.sho" );

	DebugShaderCache::Get().LoadShader( "point_impostor_rand_vp.sho" );
	DebugShaderCache::Get().LoadShader( "point_impostor_randdepth_vp.sho" );
	DebugShaderCache::Get().LoadShader( "point_impostor_simple_fp.sho" );
	DebugShaderCache::Get().LoadShader( "point_impostor_depth_fp.sho" );
	DebugShaderCache::Get().LoadShader( "point_impostor_anim_vp.sho" );
	DebugShaderCache::Get().LoadShader( "point_impostor_animdepth_vp.sho" );
	DebugShaderCache::Get().LoadShader( "point_impostor_anim_fp.sho" );
	DebugShaderCache::Get().LoadShader( "point_impostor_animdepth_fp.sho" );

	DebugShaderCache::Get().LoadShader( "impostor_rand_vp.sho" );
	DebugShaderCache::Get().LoadShader( "impostor_randdepth_vp.sho" );
	DebugShaderCache::Get().LoadShader( "impostor_simple_fp.sho" );
	DebugShaderCache::Get().LoadShader( "impostor_depth_fp.sho" );
	DebugShaderCache::Get().LoadShader( "impostor_anim_vp.sho" );
	DebugShaderCache::Get().LoadShader( "impostor_animdepth_vp.sho" );
	DebugShaderCache::Get().LoadShader( "impostor_anim_fp.sho" );
	DebugShaderCache::Get().LoadShader( "impostor_animdepth_fp.sho" );

	DebugShaderCache::Get().LoadShader( "logYUV_to_RGB_fp.sho" );
	DebugShaderCache::Get().LoadShader( "hdrresolver_vp.sho" );
	DebugShaderCache::Get().LoadShader( "hdrresolver_normal_fp.sho" );
	DebugShaderCache::Get().LoadShader( "hdrresolver_cpuexp_fp.sho" );
	DebugShaderCache::Get().LoadShader( "occlusiontermgathering_vp.sho" );
	DebugShaderCache::Get().LoadShader( "occlusiontermgathering_fp.sho" );
	DebugShaderCache::Get().LoadShader( "lens_capture_fp.sho" );
	DebugShaderCache::Get().LoadShader( "lens_ghostpass1_fp.sho" );
	DebugShaderCache::Get().LoadShader( "lens_ghostpass2_fp.sho" );
}

//------------------------------------------------------------------------------------------
//!
//!	DebugShaderCache
//! Cache singleton for Debug shaders
//!
//------------------------------------------------------------------------------------------
DebugShaderCache::~DebugShaderCache()
{
	for( ShaderMap::iterator curr = m_shaderMap.begin(); curr != m_shaderMap.end(); )
	{
		NT_DELETE_CHUNK(Mem::MC_GFX, curr->second );
		curr = m_shaderMap.erase( curr );
	}
}

//------------------------------------------------------------------------------------------
//!
//!	DebugShaderCache
//! Cache singleton for Debug shaders
//!
//------------------------------------------------------------------------------------------
DebugShader* DebugShaderCache::LoadShader( const char* pName, bool bExitGameOnFailure )
{
	uint32_t key = CHashedString( pName ).GetValue();

	ShaderMap::iterator it = m_shaderMap.find( key );
	if (it != m_shaderMap.end())
		return it->second;

	// need to load and cache it
	DebugShader* pResult = NT_NEW_CHUNK( Mem::MC_GFX ) DebugShader( bExitGameOnFailure );
	ntAssert_p( pResult, ("Failed to create new DebugShader") );

	// we dont care if the actual load failed, as this is valid behavior. just
	// stick it in the cache and return it.
	pResult->SetCGFile( pName );
	m_shaderMap[key] = pResult;

	return pResult;
}

//------------------------------------------------------------------------------------------
//!
//!	DebugShader::GenerateResources
//! reload the shader
//!
//------------------------------------------------------------------------------------------
void DebugShader::GenerateResources()
{
#ifdef _DEBUG
	SetCGFile(m_ShaderFileName.c_str());
#endif
}

//------------------------------------------------------------------------------------------
//!
//!	DebugShader::ResourcesOutOfDate
//! test to see if our resources are out of date
//!
//------------------------------------------------------------------------------------------
bool DebugShader::ResourcesOutOfDate() const
{
#ifdef _DEBUG
	CFileAttribute oTempStat(m_ShaderFileNameFull.c_str());
	if (oTempStat.GetModifyTime() > m_iShaderTimestamp)
		return true;
#endif

	return false;
}

//------------------------------------------------------------------------------------------
//!
//!	DebugShader::ctor
//! test to see if our resources are out of date
//!
//------------------------------------------------------------------------------------------
DebugShader::DebugShader(bool bExitGameOnFailure):
	m_bExitGameOnFailure(bExitGameOnFailure)
{}

DebugShader::~DebugShader()
{
	m_hShaderHandle.Reset();
}

//------------------------------------------------------------------------------------------
//!
//!	DebugShader::IsValid
//! test to see if we're valid
//!
//------------------------------------------------------------------------------------------
bool DebugShader::IsValid()
{
	switch(m_eType)
	{
		case SHADERTYPE_PIXEL:
		{
			return m_hShaderHandle.IsValid() && (m_hShaderHandle->GetType() == Gc::kVertexProgram);
			break;
		}
		case SHADERTYPE_VERTEX:
		{
			return m_hShaderHandle.IsValid() && (m_hShaderHandle->GetType() == Gc::kFragmentProgram);
			break;
		}
		default:
		{
			ntAssert(false);
			return false;
			break;
		}
	}
}

//-----------------------------------------------------
//!
//!	DebugShader::SetCGFile
//! Set a shader from a compiled ATG shader object file
//! NOTE! this function is smart and will append the 
//! correct directory based on what type of card we have
//!
//-----------------------------------------------------
bool DebugShader::SetCGFile(const char* pSHOFileName)
{
#ifdef _DEBUG
	// is shader file of the right type
	ntAssert(pSHOFileName);
	if ( strstr(pSHOFileName,".sho") == 0 )
	{
		ntPrintf("Shader %s must be of type '.sho'", pSHOFileName);
		ntAssert(!m_bExitGameOnFailure);
		return false;
	}
#endif

	const char* pShaderDir = "cg/rsx/";

	char pTemp[1024];
	char pFullFileName[1024];
	strcpy( pTemp, pShaderDir );
	strcat( pTemp, pSHOFileName );

	Util::GetFiosFilePath_Platform( pTemp, pFullFileName );

#ifdef _DEBUG
	// does the file exist?
	if ( !File::Exists(pFullFileName) )
	{
		ntPrintf("Error: can't load debug shader: %s \n", pFullFileName);
		ntAssert(!m_bExitGameOnFailure);
		return false;
	}

	m_eType = SHADERTYPE_UNKNOWN;
	Unload();
#endif

	{
		File shader_file;
		uint8_t *shader_data = NULL;
		LoadFile( pFullFileName, File::FT_READ | File::FT_BINARY, shader_file, &shader_data );
		uint32_t shader_length = shader_file.GetFileSize();
		m_ShaderMemory.Reset( shader_data );

		// Load the shader into memory
		m_hShaderHandle = GcShader::Create( shader_data, shader_length );
	}

	if (!m_hShaderHandle)
	{
		ntPrintf("Error: Failed to create shader %s!\n", pFullFileName);
		ntAssert(!m_bExitGameOnFailure);
		return false;
	}

	// establish type based on internal handle
	m_eType =	(m_hShaderHandle->GetType() == Gc::kVertexProgram) ?
				SHADERTYPE_VERTEX : SHADERTYPE_PIXEL;

#ifdef _DEBUG
	// Get file stats
	CFileAttribute shaderStats(pFullFileName);

	// save shader name and timestamp
	m_ShaderFileName = ntstd::String(pSHOFileName);
	m_ShaderFileNameFull = ntstd::String(pFullFileName);
	m_iShaderTimestamp = shaderStats.GetModifyTime();
#endif
	
    m_pcName = pSHOFileName;
	
	return true;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////
// Probably all these bindings methods are meant to be changed in the future, dunno if they make
// any sense at all on the PS3 at the moment..(Marco)
//////////////////////////////////////////////////////////////////////////////////////////////////////

/***************************************************************************************************
*
*	FUNCTION		CDebugVertexShader::SetStreamBindings
*
*	DESCRIPTION		Sets the stream bindings for this shader. The bindings are copied locally.
*
***************************************************************************************************/

void DebugShader::SetStreamBindings(const SHADER_STREAM_BINDING* pstBindings, int iNumBindings)
{
	// cache the bindings
	if(iNumBindings > 0)
	{
		m_aobStreamBindings.Reset(NT_NEW_CHUNK( Mem::MC_GFX ) SHADER_STREAM_BINDING[iNumBindings]);
		NT_MEMCPY(m_aobStreamBindings.Get(), pstBindings, iNumBindings*sizeof(SHADER_STREAM_BINDING));
	}
	else
		m_aobStreamBindings.Reset();

	// set up the base class to use them
	m_pstStreamBindings = m_aobStreamBindings.Get();
	m_iNumStreamBindings = iNumBindings;
}

/***************************************************************************************************
*
*	FUNCTION		CDebugVertexShader::SetPropertyBindings
*
*	DESCRIPTION		Sets the property bindings for this shader. The bindings are copied locally.
*
***************************************************************************************************/

void DebugShader::SetPropertyBindings(const SHADER_PROPERTY_BINDING* pstBindings, int iNumBindings)
{
	// cache the bindings
	if(iNumBindings > 0)
	{
		m_aobPropertyBindings.Reset(NT_NEW_CHUNK( Mem::MC_GFX ) SHADER_PROPERTY_BINDING[iNumBindings]);
		NT_MEMCPY(m_aobPropertyBindings.Get(), pstBindings, iNumBindings*sizeof(SHADER_PROPERTY_BINDING));
	}
	else
		m_aobPropertyBindings.Reset();
	
	// set up the base class to use them
	m_pstPropertyBindings = m_aobPropertyBindings.Get();
	m_iNumPropertyBindings = iNumBindings;
}

/***************************************************************************************************
*
*	FUNCTION		CDebugVertexShader::SetTextureBindings
*
*	DESCRIPTION		Sets the texture bindings for this shader. The bindings are copied locally.
*
***************************************************************************************************/

void DebugShader::SetTextureBindings(const SHADER_TEXTURE_BINDING* pstBindings, int iNumBindings)
{
	// cache the bindings
	if(iNumBindings > 0)
	{
		m_aobTextureBindings.Reset(NT_NEW_CHUNK( Mem::MC_GFX ) SHADER_TEXTURE_BINDING[iNumBindings]);
		NT_MEMCPY(m_aobTextureBindings.Get(), pstBindings, iNumBindings*sizeof(SHADER_TEXTURE_BINDING));
	}
	else
		m_aobTextureBindings.Reset();

	// set up the base class to use them
	m_pstTextureBindings = m_aobTextureBindings.Get();
	m_iNumTextureBindings = iNumBindings;
}

