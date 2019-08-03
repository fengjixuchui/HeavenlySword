//--------------------------------------------------------------------------------------------------
/**
	@file		GpModelDef.inl

	@brief		

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef GP_MODEL_DEF_INL
#define GP_MODEL_DEF_INL

//--------------------------------------------------------------------------------------------------
//  INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

inline int GpModelDef::GetMeshCount() const
{
	return GetResource()->m_meshInstCount;
}

//--------------------------------------------------------------------------------------------------

inline int GpModelDef::GetMeshIndex(FwHashedString hashedName) const
{
	for(int i=0; i<GetMeshCount(); i++)
	{
		if(GetMeshDef(i)->GetNameHash() == hashedName)
			return i;
	}
	return -1;
}

//--------------------------------------------------------------------------------------------------

inline GpMeshDef* GpModelDef::GetMeshDef(int index) const
{
	FW_ASSERT((index >= 0) && (index < GetMeshCount()));
	return &m_pMeshes[index];
}

//--------------------------------------------------------------------------------------------------

inline GpMeshDef* GpModelDef::GetMeshDef(FwHashedString hashedName) const
{
	return GetMeshDef(GetMeshIndex(hashedName));
}

//--------------------------------------------------------------------------------------------------

inline int GpModelDef::GetInverseBindMatrixCount() const
{
	return GetResource()->m_inverseBindMatrixCount;
}

//--------------------------------------------------------------------------------------------------

inline const FwTransform* GpModelDef::GetInverseBindMatrices() const
{
	return GetResource()->m_inverseBindMatrixArray.Get();
}

//--------------------------------------------------------------------------------------------------

inline u32 GpModelDef::GetSkeletonKey() const
{
	return GetResource()->m_skeletonKey;;
}

//--------------------------------------------------------------------------------------------------

inline const ModelResource* GpModelDef::GetResource() const
{
	return m_pResource;
}

//--------------------------------------------------------------------------------------------------

inline int GpModelDef::GetMaterialCount()
{
	return m_materialCount;
}

//--------------------------------------------------------------------------------------------------

inline GpMaterialData* GpModelDef::GetMaterialData(int materialIndex)
{
	FW_ASSERT(materialIndex < m_materialCount);

	return m_pMaterialData + materialIndex;
}

//--------------------------------------------------------------------------------------------------

inline void* GpModelDef::GetData() const
{
	return m_pResource->m_data.Get();
}

//--------------------------------------------------------------------------------------------------

inline FwHashedString GpMeshDef::GetNameHash() const
{
	return GetResource()->m_nameHash;
}

//--------------------------------------------------------------------------------------------------

inline int GpMeshDef::GetSubMeshCount() const
{
	return GetResource()->m_meshResourceOffset.Get()->m_subMeshCount;
}

//--------------------------------------------------------------------------------------------------

inline GpSubMeshDef* GpMeshDef::GetSubMeshDef(int index) const
{
	FW_ASSERT((index >= 0) && (index < GetSubMeshCount()));
	return &m_pSubMeshes[index];
}

//--------------------------------------------------------------------------------------------------

inline u32 GpMeshDef::GetTransformIndex() const
{
	return GetResource()->m_transformIndex;
}

//--------------------------------------------------------------------------------------------------

inline bool GpMeshDef::HasBlendTargets() const
{
	return (GetBlendTargetCount() != 0);
}

//--------------------------------------------------------------------------------------------------

inline int GpMeshDef::GetBlendTargetCount() const
{
	return GetResource()->m_meshResourceOffset.Get()->m_blendTargetCount;
}

//--------------------------------------------------------------------------------------------------

inline const MeshResource::BlendTargetEntry&  GpMeshDef::GetBlendTargetEntry(int index) const
{
	return GetResource()->m_meshResourceOffset.Get()->m_blendTargetArray.Get()[index];
}

//--------------------------------------------------------------------------------------------------

inline const MeshInstResource* GpMeshDef::GetResource() const
{
	return m_pResource;
}

//--------------------------------------------------------------------------------------------------

inline void* GpMeshDef::GetData() const
{
	return m_pResource->m_meshResourceOffset.Get()->m_data.Get();
}

//--------------------------------------------------------------------------------------------------

inline Gc::PrimitiveType GpSubMeshDef::GetPrimitiveType() const
{
	return (Gc::PrimitiveType)GetResource()->m_primitiveType;
}

//--------------------------------------------------------------------------------------------------

inline int GpSubMeshDef::GetIndexCount() const
{
	return GetResource()->m_indexCount;
}

//--------------------------------------------------------------------------------------------------

inline const GcStreamBufferHandle& GpSubMeshDef::GetIndexStream() const
{
	return m_hIndexStream;
}

//--------------------------------------------------------------------------------------------------

inline const GcStreamBufferHandle& GpSubMeshDef::GetVertexStream() const
{
	return m_hVertexStream;
}

//--------------------------------------------------------------------------------------------------

inline int GpSubMeshDef::GetVertexCount() const
{
	return m_hVertexStream->GetCount();
}

//--------------------------------------------------------------------------------------------------

inline const void* GpSubMeshDef::GetVertexData() const
{
	return GetResource()->m_vertexData.Get();
}

//--------------------------------------------------------------------------------------------------

inline const u16* GpSubMeshDef::GetIndexData() const
{
	return GetResource()->m_indexData.Get();
}

//--------------------------------------------------------------------------------------------------

inline GpMaterialData* GpSubMeshDef::GetMaterialData() const
{
	return m_pMaterialData;
}

//--------------------------------------------------------------------------------------------------

inline bool GpSubMeshDef::IsSkinned() const
{
	return GetResource()->m_skinningOffset.Get() != NULL;
}

//--------------------------------------------------------------------------------------------------

inline int GpSubMeshDef::GetSkinningMatrixCount() const
{
	FW_ASSERT(IsSkinned());
	return GetResource()->m_skinningOffset.Get()->m_matrixRemapCount;
}

//--------------------------------------------------------------------------------------------------

inline u16 GpSubMeshDef::GetSkinningMatrixIndex(int localIndex) const
{
	FW_ASSERT(IsSkinned());
	FW_ASSERT((localIndex >= 0) && (localIndex < GetSkinningMatrixCount()));
	return GetResource()->m_skinningOffset.Get()->m_matrixRemapData.Get()[localIndex];
}

//--------------------------------------------------------------------------------------------------

inline void* GpSubMeshDef::GetData() const
{
	return GetResource()->m_data.Get();
}

//--------------------------------------------------------------------------------------------------

inline const GcStreamBufferHandle& GpSubMeshDef::GetSkinningWeightStream() const
{
	return m_hSkinningWeightStream;
}

//--------------------------------------------------------------------------------------------------

inline const GcStreamBufferHandle& GpSubMeshDef::GetSkinningIndexStream() const
{
	return m_hSkinningIndexStream;
}

//--------------------------------------------------------------------------------------------------

inline bool GpSubMeshDef::HasBlendTargets() const
{
	return (GetResource()->m_blendShapingOffset.Get() != NULL);
}

//--------------------------------------------------------------------------------------------------

inline int GpSubMeshDef::GetBlendTargetCount() const
{
	FW_ASSERT(HasBlendTargets());
	return GetResource()->m_blendShapingOffset.Get()->m_streamCount;
}

//--------------------------------------------------------------------------------------------------

inline const SubMeshResource* GpSubMeshDef::GetResource() const
{
	return m_pResource;
}

//--------------------------------------------------------------------------------------------------

inline const char* GpMaterialData::GetMaterialFilename() const
{
	return m_pResource->m_filename.Get();
}

//--------------------------------------------------------------------------------------------------

inline FwHashedString GpMaterialData::GetMaterialHash() const
{
	return m_materialHash;
}

//--------------------------------------------------------------------------------------------------

inline FwHashedString GpMaterialData::GetMaterialInstanceHash() const
{
	return m_materialInstanceHash;
}

//--------------------------------------------------------------------------------------------------

inline int GpMaterialData::GetConstantCount() const
{
	return m_constantCount;
}

//--------------------------------------------------------------------------------------------------

inline FwHashedString GpMaterialData::GetConstantHash(int index) const
{
	FW_ASSERT((index >= 0) && (index < GetConstantCount()));
	return m_pConstantHashedNames[index];
}

//--------------------------------------------------------------------------------------------------

inline int GpMaterialData::GetConstantIndex(FwHashedString hashedName) const
{
	for(int i=0; i<GetConstantCount(); i++)
	{
		if(GetConstantHash(i) == hashedName)
			return i;
	}
	return -1;
}

//--------------------------------------------------------------------------------------------------

inline const FwVector4& GpMaterialData::GetConstant(int index) const
{
	FW_ASSERT((index >= 0) && (index < GetConstantCount()));
	return m_pConstantData[index];
}

//--------------------------------------------------------------------------------------------------

inline const FwVector4& GpMaterialData::GetConstant(FwHashedString hashedName) const
{
	return GetConstant(GetConstantIndex(hashedName));
}

//--------------------------------------------------------------------------------------------------

inline void	GpMaterialData::SetConstant(int index, FwVector4_arg data)
{
	FW_ASSERT((index >= 0) && (index < GetConstantCount()));
	m_pConstantData[index] = data;
}

//--------------------------------------------------------------------------------------------------

inline void	GpMaterialData::SetConstant(FwHashedString hashedName, FwVector4_arg data)
{
	SetConstant(GetConstantIndex(hashedName), data);
}

//--------------------------------------------------------------------------------------------------

inline int GpMaterialData::GetTextureCount() const
{
	return m_textureCount;
}

//--------------------------------------------------------------------------------------------------

inline FwHashedString GpMaterialData::GetTextureHash(int index) const
{
	FW_ASSERT((index >= 0) && (index < GetTextureCount()));
	return m_pTextureHashedNames[index];
}

//--------------------------------------------------------------------------------------------------

inline int GpMaterialData::GetTextureIndex(FwHashedString hashedName) const
{
	for(int i=0; i<GetTextureCount(); i++)
	{
		if(GetTextureHash(i) == hashedName)
			return i;
	}
	return -1;
}

//--------------------------------------------------------------------------------------------------

inline const char* GpMaterialData::GetTextureFilename(FwHashedString hashedName) const
{
	return GetTextureFilename(GetConstantIndex(hashedName));
}

//--------------------------------------------------------------------------------------------------

inline const char* GpMaterialData::GetTextureFilename(int index) const
{
	FW_ASSERT((index >= 0) && (index < GetTextureCount()));

	const MaterialResource::Texture* pTextures = m_pResource->m_textureData.Get();
	return pTextures[index].m_filename.Get();
}

//--------------------------------------------------------------------------------------------------

inline Gc::TexWrapMode GpMaterialData::GetTextureWrapS( int index ) const
{
	FW_ASSERT_MSG((index >= 0) && (index < GetTextureCount()), ("Texture index (%d) is out of range for materials textures [0,%d).", index, GetTextureCount()) );

	const MaterialResource::Texture* pTextures = m_pResource->m_textureData.Get();
	return (Gc::TexWrapMode)( ( pTextures[index].m_wrapBits >> 0 ) & 0x7 );
}

//--------------------------------------------------------------------------------------------------

inline Gc::TexWrapMode	GpMaterialData::GetTextureWrapS( FwHashedString hashedName ) const
{
	return GetTextureWrapS( GetTextureIndex( hashedName ) );
}

//--------------------------------------------------------------------------------------------------

inline Gc::TexWrapMode GpMaterialData::GetTextureWrapT( int index ) const
{
	FW_ASSERT_MSG((index >= 0) && (index < GetTextureCount()), ("Texture index (%d) is out of range for materials textures [0,%d).", index, GetTextureCount()) );

	const MaterialResource::Texture* pTextures = m_pResource->m_textureData.Get();
	return (Gc::TexWrapMode)( ( pTextures[index].m_wrapBits >> 3 ) & 0x7 );
}

//--------------------------------------------------------------------------------------------------

inline Gc::TexWrapMode GpMaterialData::GetTextureWrapT( FwHashedString hashedName ) const
{
	return GetTextureWrapT( GetTextureIndex( hashedName ) );
}

//--------------------------------------------------------------------------------------------------

inline Gc::TexWrapMode GpMaterialData::GetTextureWrapR( int index ) const
{
	FW_ASSERT_MSG((index >= 0) && (index < GetTextureCount()), ("Texture index (%d) is out of range for materials textures [0,%d).", index, GetTextureCount()) );

	const MaterialResource::Texture* pTextures = m_pResource->m_textureData.Get();
	return (Gc::TexWrapMode)( ( pTextures[index].m_wrapBits >> 6 ) & 0x7 );
}

//--------------------------------------------------------------------------------------------------

inline Gc::TexWrapMode GpMaterialData::GetTextureWrapR( FwHashedString hashedName ) const
{
	return GetTextureWrapR( GetTextureIndex( hashedName ) );
}

//--------------------------------------------------------------------------------------------------

inline Gc::TexFilter GpMaterialData::GetTextureMinFilter( int index ) const
{
	FW_ASSERT_MSG((index >= 0) && (index < GetTextureCount()), ("Texture index (%d) is out of range for materials textures [0,%d).", index, GetTextureCount()) );

	const MaterialResource::Texture* pTextures = m_pResource->m_textureData.Get();
	return (Gc::TexFilter)( ( pTextures[index].m_wrapBits >> 9 ) & 0x7 );
}

//--------------------------------------------------------------------------------------------------

inline Gc::TexFilter GpMaterialData::GetTextureMinFilter( FwHashedString hashedName ) const
{
	return GetTextureMinFilter( GetTextureIndex( hashedName ) );
}

//--------------------------------------------------------------------------------------------------

inline Gc::TexFilter GpMaterialData::GetTextureMagFilter( int index ) const
{
	FW_ASSERT_MSG((index >= 0) && (index < GetTextureCount()), ("Texture index (%d) is out of range for materials textures [0,%d).", index, GetTextureCount()) );

	const MaterialResource::Texture* pTextures = m_pResource->m_textureData.Get();
	return (Gc::TexFilter)( ( pTextures[index].m_wrapBits >> 12 ) & 0x7 );
}

//--------------------------------------------------------------------------------------------------

inline Gc::TexFilter GpMaterialData::GetTextureMagFilter( FwHashedString hashedName ) const
{
	return GetTextureMagFilter( GetTextureIndex( hashedName ) );
}

//--------------------------------------------------------------------------------------------------

inline Gc::AnisotropyLevel GpMaterialData::GetTextureAnisotropy( int index ) const
{
	FW_ASSERT_MSG((index >= 0) && (index < GetTextureCount()), ("Texture index (%d) is out of range for materials textures [0,%d).", index, GetTextureCount()) );

	const MaterialResource::Texture* pTextures = m_pResource->m_textureData.Get();
	return (Gc::AnisotropyLevel)( ( pTextures[index].m_wrapBits >> 15 ) & 0x7 );
}

//--------------------------------------------------------------------------------------------------

inline Gc::AnisotropyLevel GpMaterialData::GetTextureAnisotropy( FwHashedString hashedName ) const
{
	return GetTextureAnisotropy( GetTextureIndex( hashedName ) );
}

//--------------------------------------------------------------------------------------------------

inline bool GpMaterialData::IsTextureSrgbCorrected( int index ) const
{
	FW_ASSERT_MSG((index >= 0) && (index < GetTextureCount()), ("Texture index (%d) is out of range for materials textures [0,%d).", index, GetTextureCount()) );

	const MaterialResource::Texture* pTextures = m_pResource->m_textureData.Get();
	return ( ( pTextures[index].m_wrapBits >> 18 ) & 0x1 );
}

//--------------------------------------------------------------------------------------------------

inline bool GpMaterialData::IsTextureSrgbCorrected( FwHashedString hashedName ) const
{
	return IsTextureSrgbCorrected( GetTextureIndex( hashedName ) );
}

//--------------------------------------------------------------------------------------------------

inline bool GpMaterialData::IsTextureSignedExpanded( int index ) const
{
	FW_ASSERT_MSG((index >= 0) && (index < GetTextureCount()), ("Texture index (%d) is out of range for materials textures [0,%d).", index, GetTextureCount()) );

	const MaterialResource::Texture* pTextures = m_pResource->m_textureData.Get();
	return ( ( pTextures[index].m_wrapBits >> 19 ) & 0x1 );
}

//--------------------------------------------------------------------------------------------------

inline bool GpMaterialData::IsTextureSignedExpanded( FwHashedString hashedName ) const
{
	return IsTextureSignedExpanded( GetTextureIndex( hashedName ) );
}

//--------------------------------------------------------------------------------------------------

inline u32 GpMaterialData::GetTextureBorderColour( int index ) const
{
	FW_ASSERT_MSG((index >= 0) && (index < GetTextureCount()), ("Texture index (%d) is out of range for materials textures [0,%d).", index, GetTextureCount()) );

	const MaterialResource::Texture* pTextures = m_pResource->m_textureData.Get();
	return pTextures[index].m_borderColour;
}

//--------------------------------------------------------------------------------------------------

inline u32 GpMaterialData::GetTextureBorderColour( FwHashedString hashedName ) const
{
	return GetTextureBorderColour( GetTextureIndex( hashedName ) );
}

//--------------------------------------------------------------------------------------------------

inline float GpMaterialData::GetTextureLodMin( int index ) const
{
	FW_ASSERT_MSG((index >= 0) && (index < GetTextureCount()), ("Texture index (%d) is out of range for materials textures [0,%d).", index, GetTextureCount()) );

	const MaterialResource::Texture* pTextures = m_pResource->m_textureData.Get();
	return pTextures[index].m_lodMin;
}

//--------------------------------------------------------------------------------------------------

inline float GpMaterialData::GetTextureLodMin( FwHashedString hashedName ) const
{
	return GetTextureLodMin( GetTextureIndex( hashedName ) );
}

//--------------------------------------------------------------------------------------------------

inline float GpMaterialData::GetTextureLodMax( int index ) const
{
	FW_ASSERT_MSG((index >= 0) && (index < GetTextureCount()), ("Texture index (%d) is out of range for materials textures [0,%d).", index, GetTextureCount()) );

	const MaterialResource::Texture* pTextures = m_pResource->m_textureData.Get();
	return pTextures[index].m_lodMax;
}

//--------------------------------------------------------------------------------------------------

inline float GpMaterialData::GetTextureLodMax( FwHashedString hashedName ) const
{
	return GetTextureLodMax( GetTextureIndex( hashedName ) );
}

//--------------------------------------------------------------------------------------------------

inline float GpMaterialData::GetTextureLodBias( int index ) const
{
	FW_ASSERT_MSG((index >= 0) && (index < GetTextureCount()), ("Texture index (%d) is out of range for materials textures [0,%d).", index, GetTextureCount()) );

	const MaterialResource::Texture* pTextures = m_pResource->m_textureData.Get();
	return pTextures[index].m_lodBias;
}

//--------------------------------------------------------------------------------------------------

inline float GpMaterialData::GetTextureLodBias( FwHashedString hashedName ) const
{
	return GetTextureLodBias( GetTextureIndex( hashedName ) );
}

//--------------------------------------------------------------------------------------------------

inline const GcTextureHandle& GpMaterialData::GetTexture(int index) const
{
	FW_ASSERT_MSG((index >= 0) && (index < GetTextureCount()), ("Texture index (%d) is out of range for materials textures [0,%d).", index, GetTextureCount()) );
	return m_pTextures[index];
}

//--------------------------------------------------------------------------------------------------

inline const GcTextureHandle& GpMaterialData::GetTexture(FwHashedString hashedName) const
{
	return GetTexture(GetTextureIndex(hashedName));
}

//--------------------------------------------------------------------------------------------------

inline void GpMaterialData::SetTexture(int index, const GcTextureHandle& hTexture)
{
	FW_ASSERT((index >= 0) && (index < GetTextureCount()));
	m_pTextures[index].Reset();
	m_pTextures[index] = hTexture;
}

//--------------------------------------------------------------------------------------------------

inline void GpMaterialData::SetTexture(FwHashedString hashedName, const GcTextureHandle& hTexture)
{
	return SetTexture(GetTextureIndex(hashedName), hTexture);
}

//--------------------------------------------------------------------------------------------------

inline bool GpMaterialData::IsAlphaBlendEnabled() const
{
	return ( m_pResource->m_renderStateBits & 0x01 ) != 0;
}

//--------------------------------------------------------------------------------------------------

inline Gc::BlendFunc GpMaterialData::GetSourceBlendFunc() const
{
	return ( Gc::BlendFunc )m_pResource->m_sourceBlendFunc;
}

//--------------------------------------------------------------------------------------------------

inline Gc::BlendFunc GpMaterialData::GetDestBlendFunc() const
{
	return ( Gc::BlendFunc )m_pResource->m_destBlendFunc;
}

//--------------------------------------------------------------------------------------------------

inline bool GpMaterialData::IsAlphaTestEnabled() const
{
	return ( m_pResource->m_renderStateBits & 0x02 ) != 0;
}

//--------------------------------------------------------------------------------------------------

inline Gc::CmpFunc GpMaterialData::GetAlphaTestFunc() const
{
	return ( Gc::CmpFunc )m_pResource->m_alphaTestFunc;
}

//--------------------------------------------------------------------------------------------------

inline float GpMaterialData::GetAlphaTestRef() const
{
	return m_pResource->m_alphaTestRef;
}

//--------------------------------------------------------------------------------------------------

inline bool GpMaterialData::IsZWriteEnabled() const
{
	return ( m_pResource->m_renderStateBits & 0x04 ) != 0;
}

//--------------------------------------------------------------------------------------------------

inline bool GpMaterialData::IsBackfaceCullingEnabled() const
{
	return ( m_pResource->m_renderStateBits & 0x08 ) != 0;
}

//--------------------------------------------------------------------------------------------------

inline const MaterialResource* GpMaterialData::GetResource() const
{
	return m_pResource;
}

//--------------------------------------------------------------------------------------------------

#endif // GP_MODEL_DEF_INL
