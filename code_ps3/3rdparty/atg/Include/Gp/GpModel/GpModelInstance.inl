//--------------------------------------------------------------------------------------------------
/**
	@file		GpModelInstance.inl

	@brief		

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef GP_MODEL_INSTANCE_INL
#define GP_MODEL_INSTANCE_INL

//--------------------------------------------------------------------------------------------------
//  INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

inline int GpModelInstance::GetMeshCount() const
{
	return GetDef()->GetMeshCount();
}

//--------------------------------------------------------------------------------------------------

inline int GpModelInstance::GetMeshIndex(FwHashedString hashedName) const
{
	return GetDef()->GetMeshIndex(hashedName);
}

//--------------------------------------------------------------------------------------------------

inline const GpMeshInstanceHandle& GpModelInstance::GetMesh(int index) const
{
	FW_ASSERT((index >= 0) && (index < GetMeshCount()));
	return m_hMeshes[index];
}

//--------------------------------------------------------------------------------------------------

inline const GpMeshInstanceHandle& GpModelInstance::GetMesh(FwHashedString hashedName) const
{
	return GetMesh(GetMeshIndex(hashedName));
}

//--------------------------------------------------------------------------------------------------

inline u32 GpModelInstance::GetSkeletonKey() const
{
	return GetDef()->GetSkeletonKey();
}

//--------------------------------------------------------------------------------------------------

inline const GpModelDefHandle& GpModelInstance::GetDef() const
{
	return m_hDef;
}

//--------------------------------------------------------------------------------------------------

inline FwHashedString GpMeshInstance::GetNameHash() const
{
	return GetDef()->GetNameHash();
}

//--------------------------------------------------------------------------------------------------

inline int GpMeshInstance::GetSubMeshCount() const
{
	return GetDef()->GetSubMeshCount();
}

//--------------------------------------------------------------------------------------------------

inline u32 GpMeshInstance::GetTransformIndex() const
{
	return GetDef()->GetTransformIndex();
}

//--------------------------------------------------------------------------------------------------

inline const GpSubMeshInstanceHandle& GpMeshInstance::GetSubMesh(int index) const
{
	FW_ASSERT((index >= 0) && (index < GetSubMeshCount()));
	return m_hSubMeshes[index];
}

//--------------------------------------------------------------------------------------------------

inline void GpMeshInstance::SetVisible(bool visible)
{
	m_isVisible = visible;
}

//--------------------------------------------------------------------------------------------------

inline bool GpMeshInstance::IsVisible() const
{
	return m_isVisible;
}

//--------------------------------------------------------------------------------------------------

inline bool GpMeshInstance::HasBlendTargets() const
{
	return GetDef()->HasBlendTargets();
}

//--------------------------------------------------------------------------------------------------

inline int GpMeshInstance::GetBlendTargetCount() const
{
	return GetDef()->GetBlendTargetCount();
}

//--------------------------------------------------------------------------------------------------

inline int GpMeshInstance::GetBlendTargetIndex( FwHashedString name ) const
{
	return GetDef()->GetBlendTargetIndex(name);
}

//--------------------------------------------------------------------------------------------------

inline FwHashedString GpMeshInstance::GetBlendTargetHash( int index ) const
{
	return GetDef()->GetBlendTargetHash(index);
}

//--------------------------------------------------------------------------------------------------

inline float GpMeshInstance::GetBlendTargetWeight(int index) const
{
	FW_ASSERT((index >= 0) && (index < GetBlendTargetCount()));
	return m_pBlendTargetWeights[index];
}

//--------------------------------------------------------------------------------------------------

inline float GpMeshInstance::GetBlendTargetWeight(FwHashedString name) const
{
	return GetBlendTargetWeight(GetBlendTargetIndex(name));
}

//--------------------------------------------------------------------------------------------------

inline const MeshResource::BlendTargetEntry& GpMeshInstance::GetBlendTargetEntry(int index) const
{
	return GetDef()->GetBlendTargetEntry(index);
}

//--------------------------------------------------------------------------------------------------

inline void GpMeshInstance::SetBlendTargetWeight(int index, float weight)
{
	FW_ASSERT((index >= 0) && (index < GetBlendTargetCount()));
	m_pBlendTargetWeights[index] = weight;
}

//--------------------------------------------------------------------------------------------------

inline void GpMeshInstance::SetBlendTargetWeight(FwHashedString name, float weight)
{
	SetBlendTargetWeight(GetBlendTargetIndex(name), weight);
}

//--------------------------------------------------------------------------------------------------

inline const GpMeshDef* GpMeshInstance::GetDef() const
{
	return m_pDef;
}

//--------------------------------------------------------------------------------------------------

inline Gc::PrimitiveType GpSubMeshInstance::GetPrimitiveType() const
{
	return GetDef()->GetPrimitiveType();
}

//--------------------------------------------------------------------------------------------------

inline int GpSubMeshInstance::GetIndexCount() const
{
	return GetDef()->GetIndexCount();
}

//--------------------------------------------------------------------------------------------------

inline const GcStreamBufferHandle& GpSubMeshInstance::GetIndexStream() const
{
	return GetDef()->GetIndexStream();
}

//--------------------------------------------------------------------------------------------------

inline const GcStreamBufferHandle& GpSubMeshInstance::GetVertexStream() const
{
	return GetDef()->GetVertexStream();
}
//--------------------------------------------------------------------------------------------------

inline const GcStreamBufferHandle& GpSubMeshInstance::GetDynamicStream() const
{
	return m_hDynamicStream;
}

//--------------------------------------------------------------------------------------------------

inline bool GpSubMeshInstance::HasDynamicStream() const
{
	return m_hDynamicStream.IsValid();
}

//--------------------------------------------------------------------------------------------------

inline GpMaterialData* GpSubMeshInstance::GetMaterialData() const
{
	return m_pMaterialData;
}

//--------------------------------------------------------------------------------------------------

inline bool GpSubMeshInstance::IsSkinned() const
{
	return GetDef()->IsSkinned();
}

//--------------------------------------------------------------------------------------------------

inline int GpSubMeshInstance::GetSkinningMatrixCount() const
{
	return GetDef()->GetSkinningMatrixCount();
}

//--------------------------------------------------------------------------------------------------

inline const void* GpSubMeshInstance::GetSkinningMatrix(int index) const
{
	FW_ASSERT(IsSkinned());
	FW_ASSERT((index >= 0) && (index < GetSkinningMatrixCount()));
	int globalIndex = GetDef()->GetSkinningMatrixIndex(index);
	return (u8*)m_pMatrix + globalIndex*16*3;
}

//--------------------------------------------------------------------------------------------------

inline const GcStreamBufferHandle& GpSubMeshInstance::GetSkinningWeightStream() const
{
	return GetDef()->GetSkinningWeightStream();
}

//--------------------------------------------------------------------------------------------------

inline const GcStreamBufferHandle& GpSubMeshInstance::GetSkinningIndexStream() const
{
	return GetDef()->GetSkinningIndexStream();
}

//--------------------------------------------------------------------------------------------------

inline bool GpSubMeshInstance::HasTransform() const
{
	return (!IsSkinned()) && (m_pMatrix != NULL);
}

//--------------------------------------------------------------------------------------------------

inline const FwTransform& GpSubMeshInstance::GetTransform() const
{
	FW_ASSERT(HasTransform());
	return *(FwTransform*)m_pMatrix;
}

//--------------------------------------------------------------------------------------------------

inline bool GpSubMeshInstance::HasBlendTargets() const
{
	return GetDef()->HasBlendTargets();
}

//--------------------------------------------------------------------------------------------------

inline int GpSubMeshInstance::GetBlendTargetCount() const
{
	return GetDef()->GetBlendTargetCount();
}

//--------------------------------------------------------------------------------------------------

inline const GpSubMeshDef* GpSubMeshInstance::GetDef() const
{
	return m_pDef;
}

//--------------------------------------------------------------------------------------------------

#endif // GP_MODEL_INSTANCE_INL
