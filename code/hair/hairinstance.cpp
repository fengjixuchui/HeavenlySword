#include "hairinstance.h"

#include "hair/effectchain.h"
#include "hair/chaindef.h"
#include "gfx/renderer.h"
#include "gfx/hardwarecaps.h"
#include "gfx/graphicsdevice.h"


CHairInstance::CHairInstance(Transform const* pobTransform, CMeshHeader const* pobMeshHeader,bool bRenderOpaque, bool bShadowRecieve, bool bShadowCast):
	CMeshInstance(pobTransform,pobMeshHeader,bRenderOpaque,bShadowRecieve,bShadowCast)
{
#ifdef PLATFORM_PS3
	// create the game material
	CMaterial const* pobMaterial = ShaderManager::Get().FindMaterial( CHashedString("hair_off" ) );
	
	if( !pobMaterial )
		pobMaterial = ShaderManager::Get().FindMaterial( ERROR_MATERIAL );

	ntError_p( pobMaterial, ("No possible material (including lambert) found!") );
#else
	FXMaterial* pLitMaterial = FXMaterialManager::Get().FindMaterial("hair");
	if(!pLitMaterial)
	{
		pLitMaterial = FXMaterialManager::Get().FindMaterial("lambert_debug");
		ntAssert(pLitMaterial);
	}

	this->ForceFXMaterial(pLitMaterial);
#endif // PLATFORM_PC
}


//void CHairInstance::RenderMesh() const
//{
//	if(!ChainRessource::Get().GetGlobalDef().m_bDrawMesh)
//	{
//		return;
//	}
//
//#ifdef PLATFORM_PS3
//	Renderer::Get().m_Platform.SetStream( m_hVertexBuffer );
//	Renderer::Get().m_Platform.DrawIndexedPrimitives( ConvertPRIMTYPEToGCPRIM(m_meshType), 0, m_iIndexCount, m_hIndexBuffer );
//	Renderer::Get().m_Platform.ClearStreams();
//#else // PLATFORM_PC
//
//#endif // PLATFORM_PC
//}



/*
CHairInstance::CHairInstance(Transform const* pobTransform, CMeshHeader const* pobMeshHeader):
	CMeshInstance(pobTransform,pobMeshHeader)
{
	FXMaterial* pLitMaterial = FXMaterialManager::Get().FindMaterial("hair");
	if(!pLitMaterial)
	{
		pLitMaterial = FXMaterialManager::Get().FindMaterial("lambert_debug");
		ntAssert(pLitMaterial);
	}

	this->ForceFXMaterial(pLitMaterial);
}


void CHairInstance::RenderMaterial()
{
	if(!ChainRessource::Get().GetGlobalDef().m_bDrawMesh)
	{
		return;
	}
	
	if(true)
	{
		// need som proper alpha stuff
		
		
		//if(HardwareCapabilities::Get().SupportsPixelShader3())
		//{
		//	// alpha blend config
		//	Renderer::Get().SetZBufferMode( GFX_ZMODE_LESSEQUAL );
		//	GetD3DDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		//	GetD3DDevice()->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
		//	GetD3DDevice()->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
		//}
		//else
		//{
		//	Renderer::Get().SetZBufferMode( GFX_ZMODE_LESSEQUAL );
		//	GetD3DDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		//}
				
		// render mesh
		m_pMaterial->PreRender( m_pobTransform, IsRecievingShadows() );
		RenderMesh();
		m_pMaterial->PostRender();	


		//if(HardwareCapabilities::Get().SupportsPixelShader3())
		//{
		//	 //alpha blend config
		//	GetD3DDevice()->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
		//	GetD3DDevice()->SetRenderState( D3DRS_ALPHATESTENABLE, FALSE );
		//	Renderer::Get().SetZBufferMode( GFX_ZMODE_LESSEQUAL_READONLY );
		//}
	}
	else
	{
		m_pMaterial->PreRender( m_pobTransform, IsRecievingShadows() );
		
		// alpha test config
		Renderer::Get().SetAlphaTestMode(GFX_ALPHATEST_NONE
		GetD3DDevice()->SetRenderState( D3DRS_ALPHATESTENABLE, TRUE );
		float fV = static_cast<float>(0x000000FF) *  0.95f;
		uint32_t ulV = static_cast<uint32_t>(fV);
		GetD3DDevice()->SetRenderState( D3DRS_ALPHAREF, ulV );
		
		// alpha blend config
		GetD3DDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		GetD3DDevice()->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
		GetD3DDevice()->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
		
		// opaque
		GetD3DDevice()->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER );
		Renderer::Get().SetCullMode( GFX_CULLMODE_NONE );
		RenderMesh(); //!!!!!
		
		// alphablended
		GetD3DDevice()->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_LESSEQUAL );
		
		// back
		Renderer::Get().SetCullMode( GFX_CULLMODE_REVERSED );
		RenderMesh(); //!!!!!
		// front
		Renderer::Get().SetCullMode( GFX_CULLMODE_NORMAL );
		RenderMesh(); //!!!!!

		// alpha config
		Renderer::Get().SetBlendMode( GFX_BLENDMODE_NORMAL );
		Renderer::Get().SetAlphaTestMode( GFX_ALPHATEST_NONE );
			
		m_pMaterial->PostRender();		
	}
}

*/
