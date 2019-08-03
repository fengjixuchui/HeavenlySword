#ifndef GP_ICE_MESH_INL_H
#define GP_ICE_MESH_INL_H

//--------------------------------------------------------------------------------------------------
/**
	@brief			Gets material constant index from hash
**/
//--------------------------------------------------------------------------------------------------

inline u32 GpIceMesh::GetConstantIndex(const MaterialData* pMaterial, FwHashedString hashedName)
{
	for(u32 i=0; i<pMaterial->m_constantCount; i++)
	{
		if(pMaterial->m_pConstantNames[i] == hashedName)
			return i;
	}
	return kInvalidIndex;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Gets material constant by index (fast)
**/
//--------------------------------------------------------------------------------------------------

inline FwVector4& GpIceMesh::GetConstant(const MaterialData* pMaterial, u32 index)
{
	FW_ASSERT(index < pMaterial->m_constantCount);
	return pMaterial->m_pConstantData[index];
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Gets material constant by hash
**/
//--------------------------------------------------------------------------------------------------

inline FwVector4& GpIceMesh::GetConstant(const MaterialData* pMaterial, FwHashedString hashedName)
{
	u32 index = GetConstantIndex(pMaterial, hashedName);
	FW_ASSERT(index != kInvalidIndex);
	return GetConstant(pMaterial, index);
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Gets texture index from hash
**/
//--------------------------------------------------------------------------------------------------

inline u32 GpIceMesh::GetTextureIndex(const MaterialData* pMaterial, FwHashedString hashedName)
{
	for(u32 i=0; i<pMaterial->m_textureCount; i++)
	{
		if(pMaterial->m_pTextureNames[i] == hashedName)
			return i;
	}
	return kInvalidIndex;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Gets texture by index
**/
//--------------------------------------------------------------------------------------------------

inline const GcTextureHandle& GpIceMesh::GetTexture(const MaterialData* pMaterial, u32 index)
{
	FW_ASSERT(index < pMaterial->m_textureCount);
	return pMaterial->m_pTextures[index];
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Gets texture by hash
**/
//--------------------------------------------------------------------------------------------------

inline const GcTextureHandle& GpIceMesh::GetTexture(const MaterialData* pMaterial, FwHashedString hashedName)
{
	u32 index = GetTextureIndex(pMaterial, hashedName);
	FW_ASSERT(index != kInvalidIndex);
	return GetTexture(pMaterial, index);
}

//--------------------------------------------------------------------------------------------------

inline Gc::TexWrapMode GpIceMesh::GetTextureWrapS( const MaterialData* pMaterial, int index )
{
	FW_ASSERT_MSG((index >= 0) && (index < ( int )pMaterial->m_textureCount), ("Texture index (%d) is out of range for materials textures [0,%d).", index, pMaterial->m_textureCount) );
	return (Gc::TexWrapMode)( ( pMaterial->m_pTextureData[index].m_wrapBits >> 0 ) & 0x7 );
}

//--------------------------------------------------------------------------------------------------

inline Gc::TexWrapMode	GpIceMesh::GetTextureWrapS( const MaterialData* pMaterial, FwHashedString hashedName )
{
	return GetTextureWrapS( pMaterial, GetTextureIndex( pMaterial, hashedName ) );
}

//--------------------------------------------------------------------------------------------------

inline Gc::TexWrapMode GpIceMesh::GetTextureWrapT( const MaterialData* pMaterial, int index )
{
	FW_ASSERT_MSG((index >= 0) && (index < ( int )pMaterial->m_textureCount), ("Texture index (%d) is out of range for materials textures [0,%d).", index, pMaterial->m_textureCount) );
	return (Gc::TexWrapMode)( ( pMaterial->m_pTextureData[index].m_wrapBits >> 3 ) & 0x7 );
}

//--------------------------------------------------------------------------------------------------

inline Gc::TexWrapMode GpIceMesh::GetTextureWrapT( const MaterialData* pMaterial, FwHashedString hashedName )
{
	return GetTextureWrapT( pMaterial, GetTextureIndex( pMaterial, hashedName ) );
}

//--------------------------------------------------------------------------------------------------

inline Gc::TexWrapMode GpIceMesh::GetTextureWrapR( const MaterialData* pMaterial, int index )
{
	FW_ASSERT_MSG((index >= 0) && (index < ( int )pMaterial->m_textureCount), ("Texture index (%d) is out of range for materials textures [0,%d).", index, pMaterial->m_textureCount) );
	return (Gc::TexWrapMode)( ( pMaterial->m_pTextureData[index].m_wrapBits >> 6 ) & 0x7 );
}

//--------------------------------------------------------------------------------------------------

inline Gc::TexWrapMode GpIceMesh::GetTextureWrapR( const MaterialData* pMaterial, FwHashedString hashedName )
{
	return GetTextureWrapR( pMaterial, GetTextureIndex( pMaterial, hashedName ) );
}

//--------------------------------------------------------------------------------------------------

inline Gc::TexFilter GpIceMesh::GetTextureMinFilter( const MaterialData* pMaterial, int index )
{
	FW_ASSERT_MSG((index >= 0) && (index < ( int )pMaterial->m_textureCount), ("Texture index (%d) is out of range for materials textures [0,%d).", index, pMaterial->m_textureCount) );
	return (Gc::TexFilter)( ( pMaterial->m_pTextureData[index].m_wrapBits >> 9 ) & 0x7 );
}

//--------------------------------------------------------------------------------------------------

inline Gc::TexFilter GpIceMesh::GetTextureMinFilter( const MaterialData* pMaterial, FwHashedString hashedName )
{
	return GetTextureMinFilter( pMaterial, GetTextureIndex( pMaterial, hashedName ) );
}

//--------------------------------------------------------------------------------------------------

inline Gc::TexFilter GpIceMesh::GetTextureMagFilter( const MaterialData* pMaterial, int index )
{
	FW_ASSERT_MSG((index >= 0) && (index < ( int )pMaterial->m_textureCount), ("Texture index (%d) is out of range for materials textures [0,%d).", index, pMaterial->m_textureCount) );
	return (Gc::TexFilter)( ( pMaterial->m_pTextureData[index].m_wrapBits >> 12 ) & 0x7 );
}

//--------------------------------------------------------------------------------------------------

inline Gc::TexFilter GpIceMesh::GetTextureMagFilter( const MaterialData* pMaterial, FwHashedString hashedName )
{
	return GetTextureMagFilter( pMaterial, GetTextureIndex( pMaterial, hashedName ) );
}

//--------------------------------------------------------------------------------------------------

inline Gc::AnisotropyLevel GpIceMesh::GetTextureAnisotropy( const MaterialData* pMaterial, int index )
{
	FW_ASSERT_MSG((index >= 0) && (index < ( int )pMaterial->m_textureCount), ("Texture index (%d) is out of range for materials textures [0,%d).", index, pMaterial->m_textureCount) );
	return (Gc::AnisotropyLevel)( ( pMaterial->m_pTextureData[index].m_wrapBits >> 15 ) & 0x7 );
}

//--------------------------------------------------------------------------------------------------

inline Gc::AnisotropyLevel GpIceMesh::GetTextureAnisotropy( const MaterialData* pMaterial, FwHashedString hashedName )
{
	return GetTextureAnisotropy( pMaterial, GetTextureIndex( pMaterial, hashedName ) );
}

//--------------------------------------------------------------------------------------------------

inline u32 GpIceMesh::GetTextureBorderColour( const MaterialData* pMaterial, int index )
{
	FW_ASSERT_MSG((index >= 0) && (index < ( int )pMaterial->m_textureCount), ("Texture index (%d) is out of range for materials textures [0,%d).", index, pMaterial->m_textureCount) );
	return pMaterial->m_pTextureData[index].m_borderColour;
}

//--------------------------------------------------------------------------------------------------

inline u32 GpIceMesh::GetTextureBorderColour( const MaterialData* pMaterial, FwHashedString hashedName )
{
	return GetTextureBorderColour( pMaterial, GetTextureIndex( pMaterial, hashedName ) );
}

//--------------------------------------------------------------------------------------------------

inline float GpIceMesh::GetTextureLodMin( const MaterialData* pMaterial, int index )
{
	FW_ASSERT_MSG((index >= 0) && (index < ( int )pMaterial->m_textureCount), ("Texture index (%d) is out of range for materials textures [0,%d).", index, pMaterial->m_textureCount) );
	return pMaterial->m_pTextureData[index].m_lodMin;
}

//--------------------------------------------------------------------------------------------------

inline float GpIceMesh::GetTextureLodMin( const MaterialData* pMaterial, FwHashedString hashedName )
{
	return GetTextureLodMin( pMaterial, GetTextureIndex( pMaterial, hashedName ) );
}

//--------------------------------------------------------------------------------------------------

inline float GpIceMesh::GetTextureLodMax( const MaterialData* pMaterial, int index )
{
	FW_ASSERT_MSG((index >= 0) && (index < ( int )pMaterial->m_textureCount), ("Texture index (%d) is out of range for materials textures [0,%d).", index, pMaterial->m_textureCount) );
	return pMaterial->m_pTextureData[index].m_lodMax;
}

//--------------------------------------------------------------------------------------------------

inline float GpIceMesh::GetTextureLodMax( const MaterialData* pMaterial, FwHashedString hashedName )
{
	return GetTextureLodMax( pMaterial, GetTextureIndex( pMaterial, hashedName ) );
}

//--------------------------------------------------------------------------------------------------

inline float GpIceMesh::GetTextureLodBias( const MaterialData* pMaterial, int index )
{
	FW_ASSERT_MSG((index >= 0) && (index < ( int )pMaterial->m_textureCount), ("Texture index (%d) is out of range for materials textures [0,%d).", index, pMaterial->m_textureCount) );
	return pMaterial->m_pTextureData[index].m_lodBias;
}

//--------------------------------------------------------------------------------------------------

inline float GpIceMesh::GetTextureLodBias( const MaterialData* pMaterial, FwHashedString hashedName )
{
	return GetTextureLodBias( pMaterial, GetTextureIndex( pMaterial, hashedName ) );
}

//--------------------------------------------------------------------------------------------------

inline bool GpIceMesh::IsAlphaBlendEnabled( const MaterialData* pMaterial )
{
	return ( pMaterial->m_renderStateBits & 0x01 ) != 0;
}

//--------------------------------------------------------------------------------------------------

inline Gc::BlendFunc GpIceMesh::GetSourceBlendFunc( const MaterialData* pMaterial )
{
	return ( Gc::BlendFunc )pMaterial->m_sourceBlendFunc;
}

//--------------------------------------------------------------------------------------------------

inline Gc::BlendFunc GpIceMesh::GetDestBlendFunc( const MaterialData* pMaterial )
{
	return ( Gc::BlendFunc )pMaterial->m_destBlendFunc;
}

//--------------------------------------------------------------------------------------------------

inline bool GpIceMesh::IsAlphaTestEnabled( const MaterialData* pMaterial )
{
	return ( pMaterial->m_renderStateBits & 0x02 ) != 0;
}

//--------------------------------------------------------------------------------------------------

inline Gc::CmpFunc GpIceMesh::GetAlphaTestFunc( const MaterialData* pMaterial )
{
	return ( Gc::CmpFunc )pMaterial->m_alphaTestFunc;
}

//--------------------------------------------------------------------------------------------------

inline float GpIceMesh::GetAlphaTestRef( const MaterialData* pMaterial )
{
	return pMaterial->m_alphaTestRef;
}

//--------------------------------------------------------------------------------------------------

inline bool GpIceMesh::IsZWriteEnabled( const MaterialData* pMaterial )
{
	return ( pMaterial->m_renderStateBits & 0x04 ) != 0;
}

//--------------------------------------------------------------------------------------------------

inline bool GpIceMesh::IsBackfaceCullingEnabled( const MaterialData* pMaterial )
{
	return ( pMaterial->m_renderStateBits & 0x08 ) != 0;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns true if renderable is skinned
**/
//--------------------------------------------------------------------------------------------------

inline bool GpIceMesh::RenderableIsSkinned(const Renderable* pRenderable)
{
	return pRenderable->m_pStreamInfo->m_mpi.m_skinningBits;
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns true if renderable is trimmed
**/
//--------------------------------------------------------------------------------------------------

inline bool GpIceMesh::RenderableIsTrimmed(const Renderable* pRenderable)
{
	return (pRenderable->m_procFlags & Ice::Mesh::kDoTrimming);
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			Returns true if renderable has discrete pm
**/
//--------------------------------------------------------------------------------------------------

inline bool GpIceMesh::RenderableHasPm(const Renderable* pRenderable)
{
	return (pRenderable->m_vertexSetType == 1);
}

#endif

