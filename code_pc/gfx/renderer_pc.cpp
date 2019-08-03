/***************************************************************************************************
*
*	$Header:: /game/renderer.cpp 41    24/07/03 11:53 Simonb                                       $
*
*	The Renderer 
*
*	CHANGES
*
*	05/03/2003	Simon	Created
*
***************************************************************************************************/

#include "core/visualdebugger.h"
#include "gfx/pictureinpicture.h"
#include "gfx/renderer.h"
#include "gfx/rendercontext.h"
#include "gfx/hardwarecaps.h"
#include "gfx/shader.h"
#include "gfx/surfacemanager.h"
#include "gfx/graphicsdevice.h"

#include "dxerror_pc.h"

//#define ALEXEY_DUMP_SHADERS

// little external helper function to kick off the visual debugger
extern void RegisterPCVisualDebugger(int iDebugDisplayWidth,int iDebugDisplayHeight);

//-----------------------------------------------------
//!
//! Initialises the render. This creates a default device based on the dash
//!	settings, and initialises the default viewport to the entire frame buffer.
//!
//-----------------------------------------------------
Renderer::Renderer() 
{
	int iBBWidth = (int)DisplayManager::Get().GetInternalWidth();
	int iBBHeight = (int)DisplayManager::Get().GetInternalHeight();

	m_Platform.m_pThis = this;

#if !defined(_RELEASE)
    D3DXDebugMute(FALSE);  
#endif

	// create the declaration manager
	NT_NEW_CHUNK(Mem::MC_GFX) CVertexDeclarationManager;

	RegisterPCVisualDebugger(iBBWidth,iBBHeight);

	// create the picture in picture sub system
	m_pPIPManager = NT_NEW_CHUNK(Mem::MC_GFX) PIPManager( iBBWidth, iBBHeight );

	m_eStandardCullMode = GFX_CULLMODE_EXPLICIT_CCW;
	m_Platform.PostPresent();

	m_fGPUFrameTime = 0.0f;
	m_fCPUFrameTime = 0.0f;
}

//-----------------------------------------------------
//!
//! Destroys the renderer.
//!
//-----------------------------------------------------
Renderer::~Renderer()
{
	NT_DELETE_CHUNK(Mem::MC_GFX,  m_pPIPManager );

	// destroy our visual debugger
	NT_DELETE_CHUNK(Mem::MC_GFX,  g_VisualDebug );

	// destroy the shader manager
	DebugShaderCache::Kill();
	ShaderManager::Kill();

	// destroy the declaration manager
	CVertexDeclarationManager::Kill();
}

void Renderer::RequestPresentMode( PRESENT_MODE ){};

//-----------------------------------------------------
//!
//! Vertical blank ICE callback. allows us to swap
//!	vsync modes without tearing
//!
//-----------------------------------------------------
void Renderer::Present()
{
	// swap the surfaces
	PROFILER_PAUSE;
	GetD3DDevice()->Present(0, 0, 0, 0);
	PROFILER_RESUME;

	// reset the state cache
	m_Platform.PostPresent();
}

//-----------------------------------------------------
//!
//! Clears the current viewport.
//!
//-----------------------------------------------------
void Renderer::Clear(uint32_t dwFlags, uint32_t dwColour, float fZValue, uint32_t dwStencil)
{
	// clear the viewport
	GetD3DDevice()->Clear(0, 0, dwFlags, dwColour, fZValue, dwStencil);
}

//-----------------------------------------------------
//!
//! Creates a system memory vertex buffer not for rendering from.
//! 
//-----------------------------------------------------
VBHandle RendererPlatform::CreateSystemVertexBuffer(int iSize) const
{
	IDirect3DVertexBuffer9* pBuffer; 
	dxerror_p(	GetD3DDevice()->CreateVertexBuffer(iSize, 0, 0, D3DPOOL_SYSTEMMEM, &pBuffer, 0),
				("Failed to create scratch vertex buffer") );
	return VBHandle( pBuffer );
}

//-----------------------------------------------------
//!
//! Creates a static write-only vertex buffer.
//!
//-----------------------------------------------------
VBHandle RendererPlatform::CreateStaticVertexBuffer(int iSize) const
{
	IDirect3DVertexBuffer9* pBuffer; 
	dxerror_p(	GetD3DDevice()->CreateVertexBuffer(iSize, D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &pBuffer, 0),
				("Failed to create static vertex buffer") );
	return VBHandle( pBuffer );
}

//-----------------------------------------------------
//!
//! Creates a dynamic write-only vertex buffer.
//!   NOTE : flushes all managed resources
//!
//-----------------------------------------------------
VBHandle RendererPlatform::CreateDynamicVertexBuffer(int iSize) const
{
	IDirect3DVertexBuffer9* pBuffer; 
	dxerror_p(	GetD3DDevice()->CreateVertexBuffer(iSize, D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &pBuffer, 0),
				("Failed to create dynamic vertex buffer") );
	return VBHandle( pBuffer );
}

//-----------------------------------------------------
//!
//! Creates a write-only static index buffer.
//!
//-----------------------------------------------------
IBHandle RendererPlatform::CreateStaticIndexBuffer(int iSize) const
{
	IDirect3DIndexBuffer9* pBuffer; 
	dxerror_p(	GetD3DDevice()->CreateIndexBuffer(iSize, D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &pBuffer, 0),
				("Failed to create static index buffer") );
	return IBHandle( pBuffer );
}

//-----------------------------------------------------
//!
//! Creates a write-only dynamic index buffer.
//!
//-----------------------------------------------------
IBHandle RendererPlatform::CreateDynamicIndexBuffer(int iSize) const
{
	IDirect3DIndexBuffer9* pBuffer; 
	dxerror_p(	GetD3DDevice()->CreateIndexBuffer(iSize, D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &pBuffer, 0),
				("Failed to create dynamic index buffer") );
	return IBHandle( pBuffer );
}

//-----------------------------------------------------
//!
//! Creates a system memory index buffer, not for rendering from.
//!
//-----------------------------------------------------
IBHandle RendererPlatform::CreateSystemIndexBuffer(int iSize) const
{
	IDirect3DIndexBuffer9* pBuffer; 
	dxerror_p(	GetD3DDevice()->CreateIndexBuffer(iSize, 0, D3DFMT_INDEX16, D3DPOOL_SYSTEMMEM, &pBuffer, 0),
				("Failed to create index buffer") );
	return IBHandle( pBuffer );
}

//-----------------------------------------------------
//! 
//! Compiles a vertex shader from assembler source.
//! 
//-----------------------------------------------------
CComPtr<IDirect3DVertexShader9> RendererPlatform::AssembleVertexShader(const char* pcSource) const
{
	// compile the assembler
	CComPtr<ID3DXBuffer> pobCompiledSource, pobErrors;
	HRESULT hr;
	hr = D3DXAssembleShader(pcSource, strlen(pcSource), 0, 0, 0, pobCompiledSource.AddressOf(), pobErrors.AddressOf());

	if(FAILED(hr))
	{
		CScopedArray<char> acText(NT_NEW_CHUNK(Mem::MC_GFX) char[pobErrors->GetBufferSize() + 1]);
		NT_MEMCPY(acText.Get(), pobErrors->GetBufferPointer(), pobErrors->GetBufferSize());
		acText[pobErrors->GetBufferSize()] = 0;
		ntPrintf(acText.Get());
	}
	ntAssert(SUCCEEDED(hr));

	// create a shader from the compiled source
	return CreateVertexShader(reinterpret_cast<const uint32_t*>(pobCompiledSource->GetBufferPointer()));
}

//-----------------------------------------------------
//! 
//! Compiles a pixel shader from assembler source.
//!
//-----------------------------------------------------
CComPtr<IDirect3DPixelShader9> RendererPlatform::AssemblePixelShader(const char* pcSource) const
{
	// compile the assembler
	CComPtr<ID3DXBuffer> pobCompiledSource, pobErrors;
	HRESULT hr; 
	hr = D3DXAssembleShader(pcSource, strlen(pcSource), 0, 0, 0, pobCompiledSource.AddressOf(), pobErrors.AddressOf());
	
	if(FAILED(hr))
	{
		CScopedArray<char> acText(NT_NEW_CHUNK(Mem::MC_GFX) char[pobErrors->GetBufferSize() + 1]);
		NT_MEMCPY(acText.Get(), pobErrors->GetBufferPointer(), pobErrors->GetBufferSize());
		acText[pobErrors->GetBufferSize()] = 0;
		ntPrintf(acText.Get());
	}
	ntAssert(SUCCEEDED(hr));

	// create a shader from the compiled source
	return CreatePixelShader(reinterpret_cast<const uint32_t*>(pobCompiledSource->GetBufferPointer()));
}

//-----------------------------------------------------
//! 
//! Compiles a vertex shader from HLSL source.
//!
//-----------------------------------------------------
CComPtr<IDirect3DVertexShader9> RendererPlatform::CompileVertexShader(const char* pcSource, const char* pcFuncName, const char* pcProfile ) const
{
	// compile the HLSL
	CComPtr<ID3DXBuffer> pobCompiledSource, pobErrors;
	HRESULT hr;
	hr = D3DXCompileShader(pcSource, strlen(pcSource), 0, 0, pcFuncName, pcProfile, 0, pobCompiledSource.AddressOf(), pobErrors.AddressOf(), NULL);

	if(FAILED(hr))
	{
		CScopedArray<char> acText(NT_NEW_CHUNK(Mem::MC_GFX) char[pobErrors->GetBufferSize() + 1]);
		NT_MEMCPY(acText.Get(), pobErrors->GetBufferPointer(), pobErrors->GetBufferSize());
		acText[pobErrors->GetBufferSize()] = 0;
		ntPrintf(acText.Get());
	}
	ntAssert(SUCCEEDED(hr));

	// create a shader from the compiled source
	return CreateVertexShader(reinterpret_cast<const uint32_t*>(pobCompiledSource->GetBufferPointer()));
}

//-----------------------------------------------------
//! 
//! Compiles a pixel shader from assembler source.
//!
//-----------------------------------------------------
CComPtr<IDirect3DPixelShader9> RendererPlatform::CompilePixelShader(const char* pcSource, const char* pcFuncName, const char* pcProfile ) const
{
	// compile the assembler
	CComPtr<ID3DXBuffer> pobCompiledSource, pobErrors;
	HRESULT hr; 
	hr = D3DXCompileShader(pcSource, strlen(pcSource), 0, 0, pcFuncName, pcProfile, 0, pobCompiledSource.AddressOf(), pobErrors.AddressOf(), NULL);

	if(FAILED(hr))
	{
		CScopedArray<char> acText(NT_NEW_CHUNK(Mem::MC_GFX) char[pobErrors->GetBufferSize() + 1]);
		NT_MEMCPY(acText.Get(), pobErrors->GetBufferPointer(), pobErrors->GetBufferSize());
		acText[pobErrors->GetBufferSize()] = 0;
		ntPrintf(acText.Get());
	}
	ntAssert(SUCCEEDED(hr));

	// create a shader from the compiled source
	return CreatePixelShader(reinterpret_cast<const uint32_t*>(pobCompiledSource->GetBufferPointer()));
}

//-----------------------------------------------------
//! 
//! Creates a vertex shader from exported shader data.
//! 
//-----------------------------------------------------
CComPtr<IDirect3DVertexShader9> RendererPlatform::CreateVertexShader(const uint32_t* pdwFunction) const
{
	CComPtr<IDirect3DVertexShader9> pobShader;
	HRESULT hr;
	hr = GetD3DDevice()->CreateVertexShader( (const DWORD*) pdwFunction, pobShader.AddressOf());
	ntAssert(SUCCEEDED(hr));
	return pobShader;
}

//-----------------------------------------------------
//! 
//! Creates a pixel shader from exported shader data.
//!
//-----------------------------------------------------
CComPtr<IDirect3DPixelShader9> RendererPlatform::CreatePixelShader(const uint32_t* pdwFunction) const
{
	CComPtr<IDirect3DPixelShader9> pobShader;
	HRESULT hr;
	hr = GetD3DDevice()->CreatePixelShader( (const DWORD*) pdwFunction, pobShader.AddressOf());
	ntAssert(SUCCEEDED(hr));
	return pobShader;
}

//-----------------------------------------------------
//!
//! Returns a floating point value representing the fraction of the available RAM
//! used by the current Direct3D device. 1.0f represents the amount of RAM defined
//! by the VRAM_LIMIT preprocessor macro.
//!
//-----------------------------------------------------
float	RendererPlatform::GetMemoryUsage() const
{
	float fResult = 0.0f;

#ifdef TRACK_GFX_MEM
	uint32_t iTotal =	Renderer::ms_iVBAllocs +
						Renderer::ms_iIBAllocs +
						Renderer::ms_iDiskTex +
						Renderer::ms_iProcTex + 
						Renderer::ms_iRTAllocs;

	fResult = ( float )( ( double )( iTotal )/ ( double )VRAM_LIMIT );
#endif

	return fResult;
}

//-----------------------------------------------------
//!
//! EvictManagedResources wrapped as not on Xenon
//!
//----------------------------------------------------
void RendererPlatform::EvictManagedResources() const
{
	GetD3DDevice()->EvictManagedResources();
}

/***************************************************************************************************
*
*	FUNCTION		CRendererStateCache::Present
*
*	DESCRIPTION		Presents a surface to the back buffer.
*
***************************************************************************************************/

void RendererPlatform::PostPresent()
{
	// reset the vertex and pixel shader
	m_pThis->m_pCachedVertexShader = 0;
	m_pThis->m_pCachedPixelShader = 0;

	// reset the device states
	m_pThis->SetDefaultRenderStates();

	m_bInScene = false;

	// reset the texture stage states
	ResetTextureStates();

	// reset fill mode
	Renderer::Get().SetFillMode( GFX_FILL_SOLID );
}


//-----------------------------------------------------
//!
//! 
//!
//----------------------------------------------------
void Renderer::SetVertexShader( Shader* pobShader, bool /*bForce NOT USED ON PC*/ )
{
	if( m_pCachedVertexShader != pobShader)
	{
		// load the shader function
		ntAssert(pobShader);
		GetD3DDevice()->SetVertexShader( pobShader->GetVertexFunction() );
		
#ifdef ALEXEY_DUMP_SHADERS
		static std::set<Shader*>	shaders;

		if (std::find(shaders.begin(), shaders.end(), pobShader) == shaders.end())
		{
			//if (pobShader -> GetName() && strstr(pobShader -> GetName(), "phong3np"))
			{
				DWORD data[10000];					   
				unsigned int size;
				pobShader -> GetVertexFunction() -> GetFunction(data, &size);
				LPD3DXBUFFER buffer;
				D3DXDisassembleShader(data, true, pobShader -> GetName(), &buffer);
				char*	disassembly = (char*)buffer -> GetBufferPointer();
				unsigned int disassemblySize = buffer -> GetBufferSize();

				File	outFile;
				char fileName[128];
				sprintf(fileName, "d:/work/disasm_vp_%d.htm", shaders.size());
				outFile.Open(fileName, File::FT_BINARY | File::FT_WRITE);
				outFile.Write(disassembly, disassemblySize);
				
				shaders.insert(pobShader);
			}
		}
#endif
		
		// cache the shader
		m_pCachedVertexShader = pobShader;
	}
}

//-----------------------------------------------------
//!
//!	Setup the current vertex shader declaration
//!
//----------------------------------------------------
void RendererPlatform::SetVertexDeclaration( const CVertexDeclaration& pDecl )
{
	// load the declaration
	ntAssert( pDecl );
	GetD3DDevice()->SetVertexDeclaration( pDecl.Get() );
}

//-----------------------------------------------------
//!
//! 
//!
//----------------------------------------------------
void Renderer::SetPixelShader(Shader* pobShader,bool /*bForce NOT USED ON PC*/)
{
	if(m_pCachedPixelShader != pobShader)
	{
		// load the shader function
		ntAssert( pobShader );
		GetD3DDevice()->SetPixelShader(pobShader->GetPixelFunction());

		// cache the shader
		m_pCachedPixelShader = pobShader;

#ifdef ALEXEY_DUMP_SHADERS
		static std::set<Shader*>	shaders;

		if (std::find(shaders.begin(), shaders.end(), pobShader) == shaders.end())
		{
			//if (pobShader -> GetName() && strstr(pobShader -> GetName(), "phong3np"))
			{
				DWORD data[10000];					   
				unsigned int size;
				pobShader -> GetPixelFunction() -> GetFunction(data, &size);
				LPD3DXBUFFER buffer;
				D3DXDisassembleShader(data, true, pobShader -> GetName(), &buffer);
				char*	disassembly = (char*)buffer -> GetBufferPointer();
				unsigned int disassemblySize = buffer -> GetBufferSize();

				File	outFile;
				char fileName[128];
				sprintf(fileName, "d:/work/disasm_fp_%d.htm", shaders.size());
				outFile.Open(fileName, File::FT_BINARY | File::FT_WRITE);
				outFile.Write(disassembly, disassemblySize);
				
				shaders.insert(pobShader);
			}
		}
#endif

		// reset the texture stage states (filtering etc)
		m_Platform.ResetTextureStates();
	}
}

//-----------------------------------------------------
//!
//! Sets the texture on a given sampler stage.
//!
//----------------------------------------------------
void RendererPlatform::SetTexture(int iStage, IDirect3DBaseTexture9* pobTexture, bool bForce)
{
	ntAssert( 0 <= iStage && iStage < MAX_SAMPLERS );
	if( (pobTexture != m_apobTextures[iStage]) || (bForce == true))
	{
		GetD3DDevice()->SetTexture( iStage, pobTexture );
		m_apobTextures[iStage] = pobTexture;
	}
}


//-----------------------------------------------------
//!
//! Sets the texture on a given sampler stage.
//!
//----------------------------------------------------
void Renderer::SetTexture(int iStage, Texture::Ptr const& pobTexture, bool bForce )
{
	switch( pobTexture->GetType() )
	{
	case TT_2D_TEXTURE:
		m_Platform.SetTexture( iStage, pobTexture->m_Platform.Get2DTexture(), bForce );
		break;

	case TT_3D_TEXTURE:
		m_Platform.SetTexture( iStage, pobTexture->m_Platform.Get3DTexture(), bForce );
		break;

	case TT_CUBE_TEXTURE:
		m_Platform.SetTexture( iStage, pobTexture->m_Platform.GetCubeTexture(), bForce );
		break;
	}
}


//-----------------------------------------------------
//!
//! Clearsthe texture on a given sampler stage
//!
//----------------------------------------------------
void Renderer::SetTexture(int iStage, Texture::NONE_ENUM)
{
	m_Platform.SetTexture( iStage, 0, true );
}




//-----------------------------------------------------
//!
//! Sets the texture address mode for the given sampler stage. Redundant calls to
//! this function have minimal performance penalty.
//!
//----------------------------------------------------
void Renderer::SetSamplerAddressMode(int iStage, int iAddressMode)
{
	ntAssert(0 <= iStage && iStage < RendererPlatform::MAX_SAMPLERS);
	if(iAddressMode != m_Platform.m_aiTextureAddressModes[iStage])
	{
		GetD3DDevice()->SetSamplerState(iStage, D3DSAMP_ADDRESSU, (iAddressMode >> 16) & 0xff);
		GetD3DDevice()->SetSamplerState(iStage, D3DSAMP_ADDRESSV, (iAddressMode >> 8) & 0xff);
		GetD3DDevice()->SetSamplerState(iStage, D3DSAMP_ADDRESSW, iAddressMode & 0xff);
		m_Platform.m_aiTextureAddressModes[iStage] = iAddressMode;
	}
}

//-----------------------------------------------------
//!
//! Sets the texture filter mode for the given sampler stage. Redundant calls to
//! this function have minimal performance penalty.
//!
//----------------------------------------------------
void Renderer::SetSamplerFilterMode(int iStage, int iFilterMode)
{
	ntAssert(0 <= iStage && iStage < RendererPlatform::MAX_SAMPLERS);
	if(iFilterMode != m_Platform.m_aiTextureFilterModes[iStage])
	{
		if( CRendererSettings::bEnableGammaCorrection )
			GetD3DDevice()->SetSamplerState(iStage, D3DSAMP_SRGBTEXTURE, ((iFilterMode & 0xff000000) != 0) ? TRUE : FALSE);
		else
			GetD3DDevice()->SetSamplerState(iStage, D3DSAMP_SRGBTEXTURE, FALSE);
		GetD3DDevice()->SetSamplerState(iStage, D3DSAMP_MAGFILTER, (iFilterMode >> 16) & 0xff );
		GetD3DDevice()->SetSamplerState(iStage, D3DSAMP_MINFILTER, (iFilterMode >> 8) & 0xff );
		GetD3DDevice()->SetSamplerState(iStage, D3DSAMP_MIPFILTER, iFilterMode & 0xff );
		m_Platform.m_aiTextureFilterModes[iStage] = iFilterMode;
	}
}

//-----------------------------------------------------
//!
//!	Send our custom blend mode to the GFX device
//!
//----------------------------------------------------
void Renderer::SetBlendMode( GFX_BLENDMODE_TYPE eMode )
{
	// commit the blend mode
	switch(eMode)
	{
	case GFX_BLENDMODE_OVERWRITE:
		// overwrite all channels
		GetD3DDevice()->SetRenderState( D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ARGB );
		GetD3DDevice()->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
		GetD3DDevice()->SetRenderState( D3DRS_SEPARATEALPHABLENDENABLE, FALSE );
		break;

	case GFX_BLENDMODE_LERP:
		// lerp all channels (on source alpha)
		GetD3DDevice()->SetRenderState( D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RGB );
		GetD3DDevice()->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
		GetD3DDevice()->SetRenderState( D3DRS_SEPARATEALPHABLENDENABLE, FALSE );

		GetD3DDevice()->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
		GetD3DDevice()->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
		GetD3DDevice()->SetRenderState( D3DRS_BLENDOP, D3DBLENDOP_ADD );
		break;

	case GFX_BLENDMODE_ADD:
		// add all channels
		GetD3DDevice()->SetRenderState( D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RGB );
		GetD3DDevice()->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
		GetD3DDevice()->SetRenderState( D3DRS_SEPARATEALPHABLENDENABLE, FALSE );

		GetD3DDevice()->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
		GetD3DDevice()->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );
		GetD3DDevice()->SetRenderState( D3DRS_BLENDOP, D3DBLENDOP_ADD );
		break;

	case GFX_BLENDMODE_SUB:
		// add sub channels
		GetD3DDevice()->SetRenderState( D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RGB );
		GetD3DDevice()->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
		GetD3DDevice()->SetRenderState( D3DRS_SEPARATEALPHABLENDENABLE, FALSE );

		GetD3DDevice()->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
		GetD3DDevice()->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );
		GetD3DDevice()->SetRenderState( D3DRS_BLENDOP, D3DBLENDOP_REVSUBTRACT );
		break;

	case GFX_BLENDMODE_ADD_SRCALPHA:
		// modulate src by src alpha, add to dest
		GetD3DDevice()->SetRenderState( D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RGB );
		GetD3DDevice()->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
		GetD3DDevice()->SetRenderState( D3DRS_SEPARATEALPHABLENDENABLE, FALSE );

		GetD3DDevice()->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
		GetD3DDevice()->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );
		GetD3DDevice()->SetRenderState( D3DRS_BLENDOP, D3DBLENDOP_ADD );
		break;

	case GFX_BLENDMODE_SUB_SRCALPHA:
		// modulate src by src alpha, subract from dest
		GetD3DDevice()->SetRenderState( D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RGB );
		GetD3DDevice()->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
		GetD3DDevice()->SetRenderState( D3DRS_SEPARATEALPHABLENDENABLE, FALSE );

		GetD3DDevice()->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
		GetD3DDevice()->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );
		GetD3DDevice()->SetRenderState( D3DRS_BLENDOP, D3DBLENDOP_REVSUBTRACT );
		break;

	case GFX_BLENDMODE_DISABLED:
		// writes nothing
		GetD3DDevice()->SetRenderState( D3DRS_COLORWRITEENABLE, 0 );
		GetD3DDevice()->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
		GetD3DDevice()->SetRenderState( D3DRS_SEPARATEALPHABLENDENABLE, FALSE );
		break;

	default:
		ntAssert_p( false, ( "Unknown blend mode %d set", eMode ) );
	}

	m_cachedBlendMode = eMode;
}

//-----------------------------------------------------
//!
//!	Send our custom zbuffer mode to the GFX device
//!
//----------------------------------------------------
void Renderer::SetZBufferMode( GFX_ZMODE_TYPE eMode )
{
	switch(eMode)
	{
	case GFX_ZMODE_DISABLED:
		GetD3DDevice()->SetRenderState( D3DRS_ZENABLE, FALSE );
		GetD3DDevice()->SetRenderState( D3DRS_ZFUNC, D3DCMP_ALWAYS );
		GetD3DDevice()->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
		break;

	case GFX_ZMODE_LESSEQUAL:
		GetD3DDevice()->SetRenderState( D3DRS_ZENABLE, TRUE );
		GetD3DDevice()->SetRenderState( D3DRS_ZFUNC, D3DCMP_LESSEQUAL );
		GetD3DDevice()->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
		break;

	case GFX_ZMODE_LESSEQUAL_READONLY:
		GetD3DDevice()->SetRenderState( D3DRS_ZENABLE, TRUE );
		GetD3DDevice()->SetRenderState( D3DRS_ZFUNC, D3DCMP_LESSEQUAL );
		GetD3DDevice()->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
		break;

	default:
		ntAssert_p(false, ("Unknown z mode %d set", eMode));
	}

	m_cachedZMode = eMode;
}

//-----------------------------------------------------
//!
//!	Send our culling mode to the CPU
//!
//----------------------------------------------------
void Renderer::SetCullMode( GFX_CULLMODE_TYPE eMode )
{	
	// pick the cull mode based on the current standard cull mode
	if(eMode == GFX_CULLMODE_NORMAL)
		eMode = m_eStandardCullMode;
	else if(eMode == GFX_CULLMODE_REVERSED)
		eMode = (GFX_CULLMODE_TYPE)(1 - (int)m_eStandardCullMode);

	// commit the cullmode
	switch(eMode)
	{
	case GFX_CULLMODE_NONE:
		GetD3DDevice()->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
		break;

	case GFX_CULLMODE_EXPLICIT_CCW:
		GetD3DDevice()->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
		break;

	case GFX_CULLMODE_EXPLICIT_CW:
		GetD3DDevice()->SetRenderState( D3DRS_CULLMODE, D3DCULL_CW );
		break;

	default:
		ntAssert_p(false, ("Unknown cull mode %d set", eMode));
	}
}

//-----------------------------------------------------
//!
//!	Send our culling mode to the CPU
//!
//----------------------------------------------------
void Renderer::SetAlphaTestModeN( GFX_ALPHA_TEST_MODE eMode, float fRef )
{
	SetAlphaTestMode( eMode, (int)(fRef * 255.0f) );
}

void Renderer::SetAlphaTestMode( GFX_ALPHA_TEST_MODE eMode, int iRef )
{
	switch( eMode )
	{
	case GFX_ALPHATEST_NONE:
		GetD3DDevice()->SetRenderState( D3DRS_ALPHATESTENABLE,	FALSE );
		break;

	case GFX_ALPHATEST_EQUAL:
		GetD3DDevice()->SetRenderState( D3DRS_ALPHATESTENABLE,	TRUE );
		GetD3DDevice()->SetRenderState( D3DRS_ALPHAFUNC,		D3DCMP_EQUAL );
		GetD3DDevice()->SetRenderState( D3DRS_ALPHAREF,			iRef );	
		break;

	case GFX_ALPHATEST_NOTEQUAL:
		GetD3DDevice()->SetRenderState( D3DRS_ALPHATESTENABLE,	TRUE );
		GetD3DDevice()->SetRenderState( D3DRS_ALPHAFUNC,		D3DCMP_NOTEQUAL );
		GetD3DDevice()->SetRenderState( D3DRS_ALPHAREF,			iRef );	
		break;

	case GFX_ALPHATEST_LESS:
		GetD3DDevice()->SetRenderState( D3DRS_ALPHATESTENABLE,	TRUE );
		GetD3DDevice()->SetRenderState( D3DRS_ALPHAFUNC,		D3DCMP_LESS );
		GetD3DDevice()->SetRenderState( D3DRS_ALPHAREF,			iRef );	
		break;

	case GFX_ALPHATEST_LESSEQUAL:
		GetD3DDevice()->SetRenderState( D3DRS_ALPHATESTENABLE,	TRUE );
		GetD3DDevice()->SetRenderState( D3DRS_ALPHAFUNC,		D3DCMP_LESSEQUAL );
		GetD3DDevice()->SetRenderState( D3DRS_ALPHAREF,			iRef );	
		break;

	case GFX_ALPHATEST_GREATER:
		GetD3DDevice()->SetRenderState( D3DRS_ALPHATESTENABLE,	TRUE );
		GetD3DDevice()->SetRenderState( D3DRS_ALPHAFUNC,		D3DCMP_GREATER );
		GetD3DDevice()->SetRenderState( D3DRS_ALPHAREF,			iRef );	
		break;

	case GFX_ALPHATEST_GREATEREQUAL:
		GetD3DDevice()->SetRenderState( D3DRS_ALPHATESTENABLE,	TRUE );
		GetD3DDevice()->SetRenderState( D3DRS_ALPHAFUNC,		D3DCMP_GREATEREQUAL );
		GetD3DDevice()->SetRenderState( D3DRS_ALPHAREF,			iRef );	
		break;

	case GFX_ALPHATEST_ALWAYS:
		GetD3DDevice()->SetRenderState( D3DRS_ALPHATESTENABLE,	TRUE );
		GetD3DDevice()->SetRenderState( D3DRS_ALPHAFUNC,		D3DCMP_ALWAYS );
		GetD3DDevice()->SetRenderState( D3DRS_ALPHAREF,			iRef );	
		break;

	default:
		ntAssert_p(false, ("Unknown alpha test mode %d set", eMode));
	}
}

//-----------------------------------------------------
//!
//!	Send our fill mode
//!
//----------------------------------------------------
void Renderer::SetFillMode( GFX_FILL_MODE eMode )
{
	switch( eMode )
	{
	case GFX_FILL_SOLID:
		GetD3DDevice()->SetRenderState( D3DRS_FILLMODE,	D3DFILL_SOLID );
		break;

	case GFX_FILL_POINT:
		GetD3DDevice()->SetRenderState( D3DRS_FILLMODE,	D3DFILL_POINT );
		break;

	case GFX_FILL_WIREFRAME:
		GetD3DDevice()->SetRenderState( D3DRS_FILLMODE,	D3DFILL_WIREFRAME );
		break;

	default:
		ntAssert_p(false, ("Unknown fill mode %d set", eMode));
	}
}

//-----------------------------------------------------
//!
//!	Toggle pointsprite renderstate
//!
//----------------------------------------------------
void Renderer::SetPointSpriteEnable( bool bEnabled )
{
	GetD3DDevice()->SetRenderState( D3DRS_POINTSPRITEENABLE, bEnabled ? TRUE : FALSE );
}

//-----------------------------------------------------
//!
//! Sets a vertex shader constant into a set of registers.
//!
//----------------------------------------------------
void Renderer::SetVertexShaderConstant(int iRegister, const void* pvValue, int iNumRegisters)
{
	ntAssert( iRegister != 0 );	// reserved for texel offset
	GetD3DDevice()->SetVertexShaderConstantF(iRegister, reinterpret_cast<const float*>(pvValue), iNumRegisters);
}

//-----------------------------------------------------
//!
//! Sets a pixel shader constant into a set of registers.
//!
//----------------------------------------------------
void Renderer::SetPixelShaderConstant(int iRegister, const void* pvValue, int iNumRegisters)
{
	GetD3DDevice()->SetPixelShaderConstantF(iRegister, reinterpret_cast<const float*>(pvValue), iNumRegisters);
}

//-----------------------------------------------------
//!
//! Sets the current scissoring region
//!
//----------------------------------------------------
void Renderer::SetScissorRegion( const ScissorRegion* pRegion )
{
	ntAssert(pRegion);

	RECT scissorRect;

	scissorRect.left	= pRegion->left;
    scissorRect.top		= pRegion->top;
    scissorRect.right	= pRegion->right;
    scissorRect.bottom	= pRegion->bottom;

	GetD3DDevice()->SetScissorRect( &scissorRect );
}

//-----------------------------------------------------
//!
//! Unsets all textures.
//!
//----------------------------------------------------
void RendererPlatform::ResetTextureStates()
{
	// reset all texture stage states
	for(int iStage = 0; iStage < MAX_SAMPLERS; ++iStage)
	{
		m_aiTextureAddressModes[iStage] = 0;
		m_aiTextureFilterModes[iStage] = 0;
	}
}

//-----------------------------------------------------
//!
//! Wrappers for D3D scene delimiters.
//!
//----------------------------------------------------
void RendererPlatform::BeginScene()
{
	ntAssert( !InScene() );

	m_bInScene = true;
	GetD3DDevice()->BeginScene();
}

void RendererPlatform::EndScene()
{
	ntAssert( InScene() );

	m_bInScene = false;
	GetD3DDevice()->EndScene();
}

//-----------------------------------------------------
//!
//!	GetHardwareBackBuffer
//! method that retrieves the device back buffer. Treat with care!
//!
//----------------------------------------------------
RenderTarget::Ptr Renderer::GetHardwareBackBuffer()
{
	IDirect3DSurface9* pDeviceBB;
	
	dxerror_p(	GetD3DDevice()->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pDeviceBB),
				("Unable to retrieve back buffer") );

	return SurfaceManager::Get().CreateRenderTarget( RenderTarget::CreationStruct( pDeviceBB, true ) );
}


