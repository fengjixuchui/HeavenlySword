#ifndef GFX_SHADER_PC_H
#define GFX_SHADER_PC_H


/***************************************************************************************************
*
*	DESCRIPTION		This file is the PC specific shader class, always include
* 					shader.h and not this.
*
*	NOTES
*
***************************************************************************************************/
enum SHADERMODEL
{
	SHADERMODEL_UNKNOWN = -1, 
	SHADERMODEL_1_1		= 0x11,
	SHADERMODEL_1_4		= 0x14,
	SHADERMODEL_2_0		= 0x20,
	SHADERMODEL_2_A		= 0x2A,
	SHADERMODEL_2_B		= 0x2B,
	SHADERMODEL_3_0		= 0x30,
};

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
    IDirect3DVertexBuffer9* pobVertexBuffer;
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
				  D3DVERTEXELEMENT9* pstDeclaration );

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
	//! Resolves pointer offsets after loading and creates the shader itself.
	void Load(const CShaderDictionary* pobHead);

	//! Unloads the shader from DirectX.
	void Unload();

	
	//! Gets the shader name.
	const char* GetName() const { return m_pcName; }

	
	//! Gets the shader function.
	IDirect3DVertexShader9* GetVertexFunction() const
	{
		ntAssert(m_eType == SHADERTYPE_VERTEX);
#ifdef DELAY_SHADER_CREATION
		if (!m_pobVertexShader)
			LoadInternal();
#endif
		return m_pobVertexShader.Get();
	}
	
	//! Gets the shader function.
	IDirect3DPixelShader9* GetPixelFunction() const
	{
		ntAssert(m_eType == SHADERTYPE_PIXEL);
#ifdef DELAY_SHADER_CREATION
		if (!m_pobPixelShader)
			LoadInternal();
#endif
		return m_pobPixelShader.Get();
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
	SHADERMODEL GetModel() const { return m_eModel; }

protected:
	Shader();	//!< Private to force construction using the CShaderDictionary::Load function.
	void LoadInternal( void ) const;

	SHADERTYPE m_eType;				//!< The shader type.
	SHADERMODEL	m_eModel;			//!< The shader model.

	const char* m_pcName;			//!< The shader name.
	const uint32_t* m_pdwFunction;		//!< The compiled shader function.
	
	// xenon shaders don't derive off IUnknown so instead we have two seperate objects
	// this is horrible and nasty
	mutable CComPtr<IDirect3DPixelShader9> m_pobPixelShader;	//!< The run-time pixel shader object.
	mutable CComPtr<IDirect3DVertexShader9> m_pobVertexShader;	//!< The run-time vertex shader object.

	int m_iNumStreamBindings;							//!< The number of stream bindings.
	const SHADER_STREAM_BINDING* m_pstStreamBindings;	//!< The stream bindings.

	int m_iNumPropertyBindings;								//!< The number of property bindings.
	const SHADER_PROPERTY_BINDING* m_pstPropertyBindings;	//!< The property bindings.

	int m_iNumTextureBindings;								//!< The number of texture bindings.
	const SHADER_TEXTURE_BINDING* m_pstTextureBindings;		//!< The texture bindings.
};

#endif // end GFX_SHADER_PC_H