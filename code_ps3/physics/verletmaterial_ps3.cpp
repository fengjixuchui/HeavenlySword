#include "verletmaterial_ps3.h"

#include "verletdef_ps3.h"
#include "gfx/texturemanager.h"
#include "gfx/meshinstance.h"
#include "spu/flags/data.h"

namespace Physics
{

VerletGameLink::VerletGameLink()
{
	Set();
}

void VerletGameLink::Set()
{
	// stream dynamic
	m_streamDynamic.AllocateVertexElem(4);
	m_streamDynamic.m_gameVertexElement[0] = CMeshVertexElement(STREAM_POSITION, VD_STREAM_TYPE_FLOAT3, 0, 12);  //float pos[3];
	m_streamDynamic.m_gameVertexElement[1] = CMeshVertexElement(STREAM_NORMAL, VD_STREAM_TYPE_FLOAT3, 12, 12); //float normal[3];
	m_streamDynamic.m_gameVertexElement[2] = CMeshVertexElement(STREAM_TANGENT, VD_STREAM_TYPE_FLOAT3, 24, 12); //float tangent[3];
	m_streamDynamic.m_gameVertexElement[3] = CMeshVertexElement(STREAM_BINORMAL, VD_STREAM_TYPE_FLOAT3, 36, 12); //float binormal[3];
	m_streamDynamic.GenerateGameVertexElement();
	ProceduralStream::CheckStream(m_streamDynamic.m_gameVertexElement.Get(),m_streamDynamic.m_iNbStreamElem,sizeof(FlagBinding::VertexDynamic));

	// stream static
	m_streamStatic.AllocateVertexElem(2);
	m_streamStatic.m_gameVertexElement[0] = CMeshVertexElement(STREAM_NORMAL_MAP_TEXCOORD, VD_STREAM_TYPE_FLOAT2, 0, 8); //float nmaptex[2];
	m_streamStatic.m_gameVertexElement[1] = CMeshVertexElement(STREAM_DIFFUSE_TEXCOORD0, VD_STREAM_TYPE_FLOAT2, 8, 8); //float surftex[2];
	m_streamStatic.GenerateGameVertexElement();
	ProceduralStream::CheckStream(m_streamStatic.m_gameVertexElement.Get(),m_streamStatic.m_iNbStreamElem,sizeof(FlagBinding::VertexStatic));

	// stream both (the game need it)
	m_streamBoth.AllocateVertexElem(6);
	for(uint32_t iDynamic = 0 ; iDynamic < uint32_t(m_streamDynamic.m_iNbStreamElem) ; ++iDynamic )
	{
		m_streamBoth.m_gameVertexElement[iDynamic+0] = m_streamDynamic.m_gameVertexElement[iDynamic];
	}
	for(uint32_t iStatic = 0 ; iStatic < uint32_t(m_streamStatic.m_iNbStreamElem); ++iStatic )
	{
		m_streamBoth.m_gameVertexElement[iStatic+m_streamDynamic.m_iNbStreamElem] = m_streamStatic.m_gameVertexElement[iStatic];
		m_streamBoth.m_gameVertexElement[iStatic+m_streamDynamic.m_iNbStreamElem].m_iOffset+=sizeof(FlagBinding::VertexDynamic);
	}
	m_streamBoth.GenerateGameVertexElement();
	ProceduralStream::CheckStream(m_streamBoth.m_gameVertexElement.Get(),m_streamBoth.m_iNbStreamElem,
		sizeof(FlagBinding::VertexDynamic) + sizeof(FlagBinding::VertexStatic));
	
	// material
	m_property.AllocateGameProperty(5);
	m_property.m_emptyMaterialProperty[0].m_iPropertyTag = TEXTURE_DIFFUSE0;
	m_property.m_emptyMaterialProperty[1].m_iPropertyTag = TEXTURE_NORMAL_MAP;
	m_property.m_emptyMaterialProperty[2].m_iPropertyTag = PROPERTY_SPECULAR_COLOUR;
	m_property.m_emptyMaterialProperty[3].m_iPropertyTag = PROPERTY_DIFFUSE_COLOUR0;
	m_property.m_emptyMaterialProperty[4].m_iPropertyTag = PROPERTY_SPECULAR_POWER;

	m_property.m_pMaterial = ShaderManager::Get().FindMaterial("jambertDS1n");

	if (m_property.m_pMaterial == NULL)
		m_property.m_pMaterial = ShaderManager::Get().FindMaterial(ERROR_MATERIAL);

	ntError_p(m_property.m_pMaterial, ("Cannot find verlet material"));
}



VerletMaterialInstance::VerletMaterialInstance(const VerletMaterialInstanceDef& materialDef, const VerletGameLink& link)
{
	SetMaterial(materialDef,link);
}

void VerletMaterialInstance::SetMaterial(const VerletMaterialInstanceDef& materialDef, const VerletGameLink& link)
{
	m_gameMaterialProperty = link.m_property.GetEmptyMaterialProperty();

	ntAssert(link.m_property.m_iNbGameProperty==5);
	for(int i = 0 ; i < link.m_property.m_iNbGameProperty ; ++i )
	{
		switch(m_gameMaterialProperty[i].m_iPropertyTag)
		{
		case PROPERTY_DIFFUSE_COLOUR0:
			{
				NT_MEMCPY(reinterpret_cast<MATERIAL_FLOAT_DATA&>(m_gameMaterialProperty[i].aData).afFloats,materialDef.m_diffuseColor,3*sizeof(float));
				break;
			}
		case PROPERTY_SPECULAR_COLOUR:
			{
				NT_MEMCPY(reinterpret_cast<MATERIAL_FLOAT_DATA&>(m_gameMaterialProperty[i].aData).afFloats,materialDef.m_specularColor,3*sizeof(float));
				break;
			}
		case PROPERTY_SPECULAR_POWER:
			{
				NT_MEMCPY(reinterpret_cast<MATERIAL_FLOAT_DATA&>(m_gameMaterialProperty[i].aData).afFloats,&materialDef.m_specularCoef,1*sizeof(float));
				break;
			}
		case TEXTURE_DIFFUSE0:
			{
				Texture::Ptr pTex = TextureManager::Get().LoadTexture_Neutral( materialDef.m_surfaceTextureName.c_str() );
				reinterpret_cast<MATERAL_TEXTURE_DATA&>(m_gameMaterialProperty[i].aData).pobTexture=pTex;
				break;
			}
		case TEXTURE_NORMAL_MAP:
			{
				Texture::Ptr pTex = TextureManager::Get().LoadTexture_Neutral( materialDef.m_normalTextureName.c_str() );
				reinterpret_cast<MATERAL_TEXTURE_DATA&>(m_gameMaterialProperty[i].aData).pobTexture=pTex;
				break;
			}
		default:
			{
				ntError_p(false, ("wrong game property in SpeedTreeGameLink"));
				break;
			}
		}
	}

	m_pMaterialInstance.Reset(NT_NEW CGameMaterialInstance( link.m_property.m_pMaterial,
		link.m_streamBoth.m_gameVertexElement.Get(), link.m_streamBoth.m_iNbStreamElem,
		m_gameMaterialProperty.Get(), link.m_property.m_iNbGameProperty));
}



} //Physics

