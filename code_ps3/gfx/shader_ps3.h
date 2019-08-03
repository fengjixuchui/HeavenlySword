#ifndef GFX_SHADER_PS3_H_
#define GFX_SHADER_PS3_H_

#include <Gc/GcShader.h>
#include "gfx/renderer_debug_settings_ps3.h"
#include "blendshapes/shading/bsskin_semantic_names.h"

/***************************************************************************************************
*
*	DESCRIPTION		This file is the PS3 specific shader class, always include
* 					shader.h and not this.
*
*	NOTES
*
***************************************************************************************************/

/***************************************************************************************************
*
*	CLASS			STREAM_INPUT
*
*	DESCRIPTION		Simple structure used to store the setup information for a given stream source.
*
***************************************************************************************************/

//! Simple structure used to store the setup information for a given stream source.
struct STREAM_INPUT
{
	// Deano below was IDirect3DVertexBuffer9
    void* pobVertexBuffer;
    int iStride;
	int iOffset;
};

/***************************************************************************************************
*
*	FUNCTION		BindStreams
*
*	DESCRIPTION		Attempts to create a declaration that binds the given stream elements to 
*					the given shader. Returns true if successful.
*
***************************************************************************************************/

bool BindStreams( SHADER_STREAM_BINDING const* pstShaderBindings, int iNumShaderBindings, 
				  CMeshVertexElement const* pobElements, int iNumElements, 
				  unsigned int* puiStream, //< which stream each element is in, must be iNumElements long 
					// Deano below was D3DVERTEXELEMENT9*
				  void* pstDeclaration );

/***************************************************************************************************
*
*	CLASS			CVertexShader
*
*	DESCRIPTION		A named vertex shader resource. This shader has a set of numerical constants
*					associated with the code, and knows all of its stream and property bindings.
*
***************************************************************************************************/

class Shader
{
public:
	Shader();
	virtual ~Shader() {}
	
	void Load(const void* pMemory, unsigned int size, SHADERTYPE shaderType);

	//! Unloads the shader from DirectX.
	void Unload();
	
	//! Gets the shader name.
	const char* GetName() const { return m_pcName; }

	//! Gets the shader function. 
	GcShaderHandle GetVertexHandle() const
	{
		ntAssert(m_eType == SHADERTYPE_VERTEX);
#ifdef DELAY_SHADER_CREATION
		if (!m_hShaderHandle)
			LoadInternal();
#endif
		return m_hShaderHandle;
	}
	
	//! Gets the shader function.
	GcShaderHandle GetPixelHandle() const
	{
		ntAssert(m_eType == SHADERTYPE_PIXEL);
#ifdef DELAY_SHADER_CREATION
		if (!m_hShaderHandle)
			LoadInternal();
#endif
		return m_hShaderHandle;
	}

	//! Gets the number of stream bindings.
	int GetNumStreamBindings() const { return m_iNumStreamBindings; }

	//! Gets the stream binding for a given stream.
	const SHADER_STREAM_BINDING* GetStreamBinding(int iIndex) const { return &m_pstStreamBindings[iIndex]; }


	//! Gets the number of property bindings.
	int GetNumPropertyBindings() const { return m_iNumPropertyBindings; }

	//! Gets the stream binding for a given property.
	const SHADER_PROPERTY_BINDING* GetPropertyBinding(int iIndex) const { return &m_pstPropertyBindings[iIndex]; }


	//! Gets the number of texture bindings.
	int GetNumTextureBindings() const { return m_iNumTextureBindings; }

	//! Gets the stream binding for a given property.
	const SHADER_TEXTURE_BINDING* GetTextureBinding(int iIndex) const { return &m_pstTextureBindings[iIndex]; }

	SHADERTYPE GetType() const { return m_eType; };

	//! NOTE! these 'index' args MUST be the same as that returned by GcShader::GetConstantIndex()
	inline u32 GetConstantIndex( const char* pConstantName ) const
	{
		ntAssert_p( m_hShaderHandle, ("Invalid shader to call GetConstantIndex() upon") );
		return m_hShaderHandle->GetConstantIndex( FwHashedString( pConstantName ) );
	}

	// Old style Gc shader constant setting.
	// NOTE! index is NOT a real register number, but rather
	// an index into an array of bound constants in the shader. It MUST
	// come from  GcShader::GetConstantIndex( FwHashedString("some_constant" ) );

	//! Sets a vertex shader constant by an arbitary index allocated by 
	void SetVSConstant( u32 index, const void* pvValue, int iNumRegisters );
	void SetVSConstant( u32 index, const CVectorBase& value )						{ SetVSConstant( index, &value, 1 ); }
	void SetVSConstant( u32 index, const CMatrix& value, int iNumRegisters = 4 )	{ SetVSConstant( index, &value, iNumRegisters ); }

	//! Sets a vertex shader constant by a name held within the shader
	inline void SetVSConstantByName( const char* pName, const void* pvValue, int iNumRegisters );
	inline void SetVSConstantByName( const char* pName, const CVectorBase& value );
	inline void SetVSConstantByName( const char* pName, const CMatrix& value, int iNumRegisters = 4 );

	// new style Gc vertex shader constant setting.
	//! This is the same as direct X explict register setting. you can use the 
	//! ': register(c5)' semantic in cg to mark your constants, or
	//! GcShader::GetConstantResourceIndex( FwHashedString("some_constant" ) );
	//! NOTE! does not exist for pixel shaders. sigh.

	//! Sets a vertex shader constant by the register number in the shader
	void SetVSConstantByRegister( u32 registerIndex, const void* pvValue, int iNumRegisters );
	void SetVSConstantByRegister( u32 registerIndex, const CVectorBase& value )						{ SetVSConstantByRegister( registerIndex, &value, 1 ); }
	void SetVSConstantByRegister( u32 registerIndex, const CMatrix& value, int iNumRegisters = 4 )	{ SetVSConstantByRegister( registerIndex, &value, iNumRegisters ); }

	//! Sets a pixel shader constant.
	void SetPSConstant( u32 index, const void* pvValue, int iNumRegisters );
	void SetPSConstant( u32 index, const CVectorBase& value )						{ SetPSConstant( index, &value, 1 ); }
	void SetPSConstant( u32 index, const CMatrix& value, int iNumRegisters = 4 )	{ SetPSConstant( index, &value, iNumRegisters ); }

	//! Sets a pixel shader constant by a name held within the shader
	inline void SetPSConstantByName( const char* pName, const void* pvValue, int iNumRegisters );
	inline void SetPSConstantByName( const char* pName, const CVectorBase& value );
	inline void SetPSConstantByName( const char* pName, const CMatrix& value, int iNumRegisters = 4 );

	//! Bindings generatios methods..
	void GenerateStreamBindings(SHADER_STREAM_BINDING* pStreamBind);
	void GeneratePropertyBindings(SHADER_PROPERTY_BINDING* pPropBind);
	void GenerateTextureBindings(SHADER_TEXTURE_BINDING* pTextureBind);

	void SetAnisotropicTextureFilteringOverride( bool state ) { m_bAnisotropicTextureFilterOverride = state; }

	GcShaderHandle m_hShaderHandle;	//!< PS3 implementation of shader

	bool		m_bNonHeresyFragmentCached;		//!< part of the evil to save ram when not using heresy
protected:
	void LoadInternal( void ) const;

	SHADERTYPE m_eType;							//!< The shader type.

	const char* m_pcName;			//!< The shader name.	

	int m_iNumStreamBindings;							//!< The number of stream bindings.
	const SHADER_STREAM_BINDING* m_pstStreamBindings;	//!< The stream bindings.

	int m_iNumPropertyBindings;								//!< The number of property bindings.
	const SHADER_PROPERTY_BINDING* m_pstPropertyBindings;	//!< The property bindings.

	int m_iNumTextureBindings;								//!< The number of texture bindings.
	const SHADER_TEXTURE_BINDING* m_pstTextureBindings;		//!< The texture bindings.

	bool m_bAnisotropicTextureFilterOverride;			
#ifdef COLLECT_SHADER_STATS
public:
    CMaterial*      m_material;
    unsigned int    m_index;
    unsigned int    m_slot;
#endif

};

//-----------------------------------------------------
//!
//!	Shader::SetVSConstantByName / SetPSConstantByName
//!	Wrappers that retrieve constant inidices 
//!
//-----------------------------------------------------
inline void Shader::SetVSConstantByName( const char* pName, const void* pvValue, int iNumRegisters )
{
	u32 index = GetConstantIndex( pName );
//	ntAssert_p( index != 0xffffffff, ("Constant %s not present in vertexshader.",pName));
	if (index == 0xffffffff) return;
	SetVSConstant( index, pvValue, iNumRegisters );
}

inline void Shader::SetVSConstantByName( const char* pName, const CVectorBase& value )
{
	u32 index = GetConstantIndex( pName );
	//ntAssert_p( index != 0xffffffff, ("Constant %s not present in vertexshader.",pName));
	if (index == 0xffffffff) return;
	SetVSConstant( index, value );
}

inline void Shader::SetVSConstantByName( const char* pName, const CMatrix& value, int iNumRegisters )
{
	u32 index = GetConstantIndex( pName );
	//ntAssert_p( index != 0xffffffff, ("Constant %s not present in vertexshader.",pName));
	if (index == 0xffffffff) return;
	SetVSConstant( index, value, iNumRegisters );
}

inline void Shader::SetPSConstantByName( const char* pName, const void* pvValue, int iNumRegisters )
{
	u32 index = GetConstantIndex( pName );
	//ntAssert_p( index != 0xffffffff, ("Constant %s not present in pixelshader.",pName));
	if (index != 0xffffffff)
	{
		SetPSConstant( index, pvValue, iNumRegisters );
	}
}

inline void Shader::SetPSConstantByName( const char* pName, const CVectorBase& value )
{
	u32 index = GetConstantIndex( pName );
	//ntAssert_p( index != 0xffffffff, ("Constant %s not present in pixelshader.",pName));
	if (index != 0xffffffff)
	{
	SetPSConstant( index, value );
	}
}

inline void Shader::SetPSConstantByName( const char* pName, const CMatrix& value, int iNumRegisters )
{
	u32 index = GetConstantIndex( pName );
	//ntAssert_p( index != 0xffffffff, ("Constant %s not present in pixelshader.",pName));
	if (index != 0xffffffff)
	{
		SetPSConstant( index, value, iNumRegisters );
	}
}

const unsigned int aiLinkCgStreamWithStreamSemantic[] = 
{
	CHashedString("tangent_space_basis").Get(),			STREAM_NORMAL_MAP_COORD_SYSTEM,
	CHashedString("binormal").Get(),					STREAM_BINORMAL,							
	CHashedString("indices").Get(),						STREAM_BLENDINDICES,
	CHashedString("weights").Get(),						STREAM_BLENDWEIGHTS,
	CHashedString("diffuse_texcoord0").Get(),			STREAM_DIFFUSE_TEXCOORD0,
	CHashedString("diffuse_texcoord1").Get(),			STREAM_DIFFUSE_TEXCOORD1,
	CHashedString("diffuse_texcoord2").Get(),			STREAM_DIFFUSE_TEXCOORD2,
	CHashedString("normal").Get(),						STREAM_NORMAL,
	CHashedString("normal_map_texcoord").Get(),			STREAM_NORMAL_MAP_TEXCOORD,
	CHashedString("position").Get(),					STREAM_POSITION,
	CHashedString("tangent").Get(),						STREAM_TANGENT,
	CHashedString("specular_colour_map_texcoord").Get(),STREAM_SPECULAR_COLOUR_METALLICITY_TEXCOORD,
	CHashedString("generic_texcoord0").Get(),			STREAM_GENERIC_TEXCOORD0,		
	CHashedString("generic_texcoord1").Get(),			STREAM_GENERIC_TEXCOORD1,		
	CHashedString("generic_texcoord2").Get(),			STREAM_GENERIC_TEXCOORD2,		
	CHashedString("generic_texcoord3").Get(),			STREAM_GENERIC_TEXCOORD3,		
	CHashedString("generic_texcoord4").Get(),			STREAM_GENERIC_TEXCOORD4,		
	CHashedString("generic_texcoord5").Get(),			STREAM_GENERIC_TEXCOORD5,		
	CHashedString("generic_texcoord6").Get(),			STREAM_GENERIC_TEXCOORD6,		
	CHashedString("generic_texcoord7").Get(),			STREAM_GENERIC_TEXCOORD7,		
	CHashedString("generic_texcoord8").Get(),			STREAM_GENERIC_TEXCOORD8,		
	CHashedString("generic_texcoord9").Get(),			STREAM_GENERIC_TEXCOORD9,		
	CHashedString("generic_texcoord10").Get(),			STREAM_GENERIC_TEXCOORD10,		
	CHashedString("generic_texcoord11").Get(),			STREAM_GENERIC_TEXCOORD11,		
	CHashedString("generic_texcoord12").Get(),			STREAM_GENERIC_TEXCOORD12,		
	CHashedString("generic_texcoord13").Get(),			STREAM_GENERIC_TEXCOORD13,		
	CHashedString("generic_texcoord14").Get(),			STREAM_GENERIC_TEXCOORD14,		
	CHashedString("generic_texcoord15").Get(),			STREAM_GENERIC_TEXCOORD15,		
};

const unsigned int aiLinkCgUniformWithPropertySemantic[] =
{
	// FIXME_WIL temp attemp to fix USB init bug (dont ask). Need updating if ATG CRC stuff changes
	0xe6166c4b, /*CHashedString("specular_colour").Get(),				*/ PROPERTY_SPECULAR_COLOUR,						1,
	0x268896d7, /*CHashedString("specular_power").Get(),				*/ PROPERTY_SPECULAR_POWER,							1,
	0x1f35a276, /*CHashedString("reflectance_colour").Get(),			*/ PROPERTY_REFLECTANCE_MAP_COLOUR,					1,
	0x80fa87d8, /*CHashedString("reflected_colour").Get(),				*/ PROPERTY_REFLECTANCE_COLOUR,						1,
	0xed451b7e, /*CHashedString("fresnel_strength").Get(),				*/ PROPERTY_FRESNEL_EFFECT,							1,
	0xc68373df, /*CHashedString("blend_transforms").Get(),				*/ PROPERTY_BLEND_TRANSFORMS,						192,
	0x3d2188e2, /*CHashedString("g_depthOfFieldParams").Get(),			*/ PROPERTY_DEPTHOFFIELD_PARAMS,					1,
	0xa6d04368, /*CHashedString("depth_haze_b1_plus_b2").Get(),			*/ PROPERTY_DEPTH_HAZE_BETA1PLUSBETA2,				1,
	0x6615ba71, /*CHashedString("depth_haze_bdash1").Get(),				*/ PROPERTY_DEPTH_HAZE_BETADASH1,					1,	
	0xff1cebcb, /*CHashedString("depth_haze_bdash2").Get(),				*/ PROPERTY_DEPTH_HAZE_BETADASH2,					1,
	0x77421395, /*CHashedString("depth_haze_consts_A").Get(),			*/ PROPERTY_DEPTH_HAZE_CONSTS_A,					1,
	0x9e21b6a0, /*CHashedString("depth_haze_consts_G").Get(),			*/ PROPERTY_DEPTH_HAZE_CONSTS_G,					1,
	0x63e48a77, /*CHashedString("depth_haze_recip_b1_plus_b2").Get(),	*/ PROPERTY_DEPTH_HAZE_RECIP_BETA1PLUSBETA2,		1,
	
	CHashedString("haze_kill_power").Get(),									PROPERTY_DEPTH_HAZE_KILL_POWER,					1,

	0xb6f492bc, /*CHashedString("diffuse_colour0").Get(),				*/ PROPERTY_DIFFUSE_COLOUR0,						1,
	0xc1f3a22a, /*CHashedString("diffuse_colour1").Get(),				*/ PROPERTY_DIFFUSE_COLOUR1,						1,
	0x58faf390, /*CHashedString("diffuse_colour2").Get(),				*/ PROPERTY_DIFFUSE_COLOUR2,						1,
	
	//CHashedString("fill_sh").Get(),						PROPERTY_FILL_SH,	// this should be PROPERTY_FILL_SH but I can't find this property in lambert and jambert

	0xa8a77fe9, /*CHashedString("fill_sh").Get(),						*/ PROPERTY_FILL_SH_MATRICES,						12,
	0xf4c6911d, /*CHashedString("key_dir_colour").Get(),				*/ PROPERTY_KEY_DIR_COLOUR,							1,
	0x915d983f, /*CHashedString("key_dir").Get(),						*/ PROPERTY_KEY_DIR_OBJECTSPACE,		 			1,
	0x417b50bb, /*CHashedString("g_parallaxScaleAndBias").Get(),		*/ PROPERTY_PARALLAX_SCALE_AND_BIAS,				1,
	0x7ffb37d9, /*CHashedString("projection").Get(),					*/ PROPERTY_PROJECTION,								4,
	0x7cdf3b5b, /*CHashedString("shadow_map_transform").Get(),			*/ PROPERTY_SHADOW_MAP_TRANSFORM,					4,
	0xad1b577d, /*CHashedString("shadow_map_transform1").Get(),			*/ PROPERTY_SHADOW_MAP_TRANSFORM1,					4,
	0x341206c7, /*CHashedString("shadow_map_transform2").Get(),			*/ PROPERTY_SHADOW_MAP_TRANSFORM2,					4,
	0x43153651, /*CHashedString("shadow_map_transform3").Get(),			*/ PROPERTY_SHADOW_MAP_TRANSFORM3,					4,
	0xc80f5f1e, /*CHashedString("shadow_plane0").Get(),					*/ PROPERTY_SHADOW_PLANE0,							1,
	0xbf086f88, /*CHashedString("shadow_plane1").Get(),					*/ PROPERTY_SHADOW_PLANE1,							1,
	0x26013e32, /*CHashedString("shadow_plane2").Get(),					*/ PROPERTY_SHADOW_PLANE2,							1,
	0x51060ea4, /*CHashedString("shadow_plane3").Get(),					*/ PROPERTY_SHADOW_PLANE3,							1,
	0xcf629b07, /*CHashedString("shadow_plane4").Get(),					*/ PROPERTY_SHADOW_PLANE4,							1,
	0x10980db8, /*CHashedString("shadow_radii").Get(),					*/ PROPERTY_SHADOW_RADII,							1,
	0x9474a389, /*CHashedString("shadow_radii1").Get(),					*/ PROPERTY_SHADOW_RADII1,							1,
	0x0d7df233, /*CHashedString("shadow_radii2").Get(),					*/ PROPERTY_SHADOW_RADII2,							1,
	0x7a7ac2a5, /*CHashedString("shadow_radii3").Get(),					*/ PROPERTY_SHADOW_RADII3,							1,
	0xe0f9c032, /*CHashedString("sun_colour").Get(),					*/ PROPERTY_SUN_COLOUR,								1,
	0x147a039f, /*CHashedString("sun_dir").Get(),						*/ PROPERTY_SUN_DIRECTION_OBJECTSPACE,				1,
	0xe4925d1d, /*CHashedString("view_position").Get(),					*/ PROPERTY_VIEW_POSITION_OBJECTSPACE,				1,
	0x0bc3bad3, /*CHashedString("view_transform").Get(),				*/ PROPERTY_VIEW_TRANSFORM,							4,
	0xd6c73f9e, /*CHashedString("vPosToUV").Get(),						*/ PROPERTY_VPOS_TO_UV,								1,
	0xa161574b, /*CHashedString("world_transform").Get(),				*/ PROPERTY_WORLD_TRANSFORM,						4,
	0x718c25b2, /*CHashedString("alphatest_ref").Get(),					*/ PROPERTY_ALPHATEST_THRESHOLD,					1,

	CHashedString("key_dir_worldspace").Get(),								PROPERTY_KEY_DIR_WORLDSPACE,					1,				

	CHashedString( BSSKIN_DIFFUSE_WRAP ).Get(),								PROPERTY_BSSKIN_DIFFUSE_WRAP,					1,
	CHashedString( BSSKIN_SPECULAR_FACING ).Get(),							PROPERTY_BSSKIN_SPECULAR_FACING,				1,
	CHashedString( BSSKIN_SPECULAR_GLANCING ).Get(),						PROPERTY_BSSKIN_SPECULAR_GLANCING,				1,
	CHashedString( BSSKIN_FUZZ_COLOUR ).Get(),								PROPERTY_BSSKIN_FUZZ_COLOUR,					1,
	CHashedString( BSSKIN_FUZZ_TIGHTNESS ).Get(),							PROPERTY_BSSKIN_FUZZ_TIGHTNESS,					1,
	CHashedString( BSSKIN_SUBCOLOUR ).Get(),								PROPERTY_BSSKIN_SUBCOLOUR,						1,
	CHashedString( BSSKIN_WRINKLE_REGIONS0_WEIGHTS ).Get(),					PROPERTY_BSSKIN_WRINKLE_REGIONS0_WEIGHTS,		1,
	CHashedString( BSSKIN_WRINKLE_REGIONS1_WEIGHTS ).Get(),					PROPERTY_BSSKIN_WRINKLE_REGIONS1_WEIGHTS,		1,
	CHashedString( BSSKIN_NORMAL_MAP_STRENGTH ).Get(),						PROPERTY_BSSKIN_NORMAL_MAP_STRENGTH,			1,
	CHashedString( BSSKIN_SPECULAR_STRENGTH ).Get(),						PROPERTY_BSSKIN_SPECULAR_STRENGTH,				1,

	CHashedString("baseFrequency").Get(),									PROPERTY_HAIR_BASETEX_FREQ,						1,
	CHashedString("shiftFrequency").Get(),									PROPERTY_HAIR_SHIFTTEX_FREQ,					1,
	CHashedString("maskFrequency").Get(),									PROPERTY_HAIR_MASKTEX_FREQ,						1,
	CHashedString("hairSpecularShift").Get(),								PROPERTY_SPECULAR_SHIFT,						1,
	CHashedString("hairSpecularShift2").Get(),								PROPERTY_SPECULAR_SHIFT2,						1,
	CHashedString("specular_colour2").Get(),								PROPERTY_SPECULAR_COLOUR2,						1,
	CHashedString("specular_power2").Get(),									PROPERTY_SPECULAR_POWER2,						1,

	CHashedString("shadow_planes").Get(),									PROPERTY_SHADOW_PLANES,							1,

	CHashedString("pos_reconstruction_mat").Get(),							PROPERTY_POSITION_RECONST_MAT,					4,
	CHashedString("projection_noreconst_mat").Get(),						PROPERTY_PROJECTION_NORECONST_MAT,				4,

	CHashedString("scaleandbias_texcoord0").Get(),							PROPERTY_GENERIC_TEXCOORD0_SCALE_BIAS,			1,
	CHashedString("scaleandbias_texcoord1").Get(),							PROPERTY_GENERIC_TEXCOORD1_SCALE_BIAS,			1,
	CHashedString("scaleandbias_texcoord2").Get(),							PROPERTY_GENERIC_TEXCOORD2_SCALE_BIAS,			1,
	CHashedString("scaleandbias_texcoord3").Get(),							PROPERTY_GENERIC_TEXCOORD3_SCALE_BIAS,			1,
	CHashedString("scaleandbias_texcoord4").Get(),							PROPERTY_GENERIC_TEXCOORD4_SCALE_BIAS,			1,
	CHashedString("scaleandbias_texcoord5").Get(),							PROPERTY_GENERIC_TEXCOORD5_SCALE_BIAS,			1,
	CHashedString("scaleandbias_texcoord6").Get(),							PROPERTY_GENERIC_TEXCOORD6_SCALE_BIAS,			1,
	CHashedString("scaleandbias_texcoord7").Get(),							PROPERTY_GENERIC_TEXCOORD7_SCALE_BIAS,			1,
	CHashedString("scaleandbias_texcoord8").Get(),							PROPERTY_GENERIC_TEXCOORD8_SCALE_BIAS,			1,
	CHashedString("scaleandbias_texcoord9").Get(),							PROPERTY_GENERIC_TEXCOORD9_SCALE_BIAS,			1,
	CHashedString("scaleandbias_texcoord10").Get(),							PROPERTY_GENERIC_TEXCOORD10_SCALE_BIAS,			1,
	CHashedString("scaleandbias_texcoord11").Get(),							PROPERTY_GENERIC_TEXCOORD11_SCALE_BIAS,			1,
	CHashedString("scaleandbias_texcoord12").Get(),							PROPERTY_GENERIC_TEXCOORD12_SCALE_BIAS,			1,
	CHashedString("scaleandbias_texcoord13").Get(),							PROPERTY_GENERIC_TEXCOORD13_SCALE_BIAS,			1,
	CHashedString("scaleandbias_texcoord14").Get(),							PROPERTY_GENERIC_TEXCOORD14_SCALE_BIAS,			1,
	CHashedString("scaleandbias_texcoord15").Get(),							PROPERTY_GENERIC_TEXCOORD15_SCALE_BIAS,			1,

	CHashedString("g_amWindMatrices").Get(),								PROPERTY_SPEEDTREE_WINDMATRICES,				16,
	CHashedString("g_avLeafClusters").Get(),								PROPERTY_SPEEDTREE_LEAFCLUSTERS,				48,
	CHashedString("g_amLeafAngleMatrices").Get(),							PROPERTY_SPEEDTREE_LEAFANGLES,					16,
	CHashedString("g_speedtreeAlphaTreshold").Get(),						PROPERTY_SPEEDTREE_ALPHATRESHOLD,				1,

	CHashedString("snow_diffuse_colour").Get(),								PROPERTY_SNOW_DIFFUSE_COLOUR,					1,
	CHashedString("snow_specular_colour").Get(),							PROPERTY_SNOW_SPECULAR_COLOUR,					1,
	CHashedString("snow_intensity").Get(),									PROPERTY_SNOW_INTENSITY,						1,
	CHashedString("facing_colour").Get(),									PROPERTY_FACINGTERM_COLOUR,						1,
	CHashedString("facing_power").Get(),									PROPERTY_FACINGTERM_POWER,						1,

	CHashedString("ice_frost").Get(),										PROPERTY_ICE_FROST,								1,

	CHashedString("refraction_warp").Get(),									PROPERTY_REFRACTION_WARP,						1,

	CHashedString("uvanim_texcoord0").Get(),								PROPERTY_UVSCROLL0,								1,
	CHashedString("uvanim_texcoord1").Get(),								PROPERTY_UVSCROLL1,								1,
	CHashedString("uvanim_texcoord2").Get(),								PROPERTY_UVSCROLL2,								1,
	CHashedString("uvanim_texcoord3").Get(),								PROPERTY_UVSCROLL3,								1,
	CHashedString("uvanim_texcoord4").Get(),								PROPERTY_UVSCROLL4,								1,
	CHashedString("uvanim_texcoord5").Get(),								PROPERTY_UVSCROLL5,								1,

	CHashedString("game_time").Get(),										PROPERTY_GAME_TIME,								1,

	CHashedString("velvet_roughness").Get(),								PROPERTY_VELVET_ROUGHNESS,						1,
	CHashedString("velvet_backscatter").Get(),								PROPERTY_VELVET_BACKSCATTER,					1,
	CHashedString("velvet_retroreflective_colour").Get(),					PROPERTY_VELVET_RETROREFLECTIVE_COLOUR,			1,
	CHashedString("velvet_silhouette_color").Get(),							PROPERTY_VELVET_SILHOUETTE_COLOUR,				1,

	CHashedString("haze_extinction").Get(),									PROPERTY_HAZE_FRAGMENT_EXTINCTION,				1,
	CHashedString("haze_scatter").Get(),									PROPERTY_HAZE_FRAGMENT_SCATTER,					1,

	CHashedString("exposure_and_tonemapping").Get(),						PROPERTY_EXPOSURE_AND_TONEMAP,					1,

	CHashedString("NotUsed").Get(),											PROPERTY_ANISOTROPIC_FILTERING,					1,
};


const unsigned int aiLinkCgSamplerWithTextureSemantic[] =
{
	CHashedString("custom_reflectance_map").Get(),		TEXTURE_CUSTOM_REFLECTANCE_MAP,
	CHashedString("reflectance").Get(),					TEXTURE_REFLECTANCE_MAP,
	CHashedString("diffuse0").Get(),					TEXTURE_DIFFUSE0,
	CHashedString("diffuse1").Get(),					TEXTURE_DIFFUSE1,
	CHashedString("diffuse2").Get(),					TEXTURE_DIFFUSE2,
	CHashedString("hdr_diffuse0").Get(),				TEXTURE_HDR_DIFFUSE0,
	CHashedString("normal_map").Get(),					TEXTURE_NORMAL_MAP,
	CHashedString("shadow_map").Get(),					TEXTURE_SHADOW_MAP,
	CHashedString("shadow_map1").Get(),					TEXTURE_SHADOW_MAP1,
	CHashedString("shadow_map2").Get(),					TEXTURE_SHADOW_MAP2,	
	CHashedString("shadow_map3").Get(),					TEXTURE_SHADOW_MAP3,
	CHashedString("stencil_map").Get(),					TEXTURE_STENCIL_MAP,
	CHashedString("specular_colour_map").Get(),			TEXTURE_SPECULAR_COLOUR_METALLICITY,
	CHashedString("mask").Get(),						TEXTURE_MASK,
	CHashedString("emissivity_map").Get(),				TEXTURE_EMISSIVITY_MAP,
	CHashedString("glittering_map").Get(),				TEXTURE_GLITTERING_MAP,
	CHashedString("irradiance_cache_map").Get(),		TEXTURE_IRRADIANCE_CACHE,
	CHashedString("velvet_fill_spec_map").Get(),		TEXTURE_VELVET_FILL_SPECULARITY,
	CHashedString("velvet_silhouette_spec_map").Get(),	TEXTURE_VELVET_SILHOUETTE_SPECULARITY,


	CHashedString( BSSKIN_DIFFUSE_SPECULAR_S ).Get(),						TEXTURE_DIFFUSE0,
	CHashedString( BSSKIN_BASE_NORMAL_OCCLUSION_S ).Get(),					TEXTURE_BSSKIN_BASE_NORMAL_OCCLUSION,	
	CHashedString( BSSKIN_WRINKLE_NORMAL_OCCLUSION_S ).Get(),				TEXTURE_BSSKIN_WRINKLE_NORMAL_OCCLUSION,	
	CHashedString( BSSKIN_WRINKLE_REGIONS0_S ).Get(),						TEXTURE_BSSKIN_WRINKLE_REGIONS0,	
	CHashedString( BSSKIN_WRINKLE_REGIONS1_S ).Get(),						TEXTURE_BSSKIN_WRINKLE_REGIONS1,	
	CHashedString( BSSKIN_SPECULAR_GAIN_S ).Get(),							TEXTURE_BSSKIN_SPECULAR_GAIN,	
	CHashedString( BSSKIN_DIFFUSE_GAIN_S ).Get(),							TEXTURE_BSSKIN_DIFFUSE_GAIN,	
};



#endif //GFX_SHADER_PS3_H_
