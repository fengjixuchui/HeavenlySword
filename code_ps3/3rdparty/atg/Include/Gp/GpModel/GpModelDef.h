//--------------------------------------------------------------------------------------------------
/**
	@file		GpModelDef.h

	@brief		

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef GP_MODEL_DEF_H
#define GP_MODEL_DEF_H

//--------------------------------------------------------------------------------------------------
//  INCLUDES
//--------------------------------------------------------------------------------------------------

#include <Fw/FwStd/FwStdIntrusivePtr.h>
#include <Fw/FwResource.h>
#include <Gc/GcStreamBuffer.h>
#include <Gc/GcTexture.h>
#include <Gp/GpModel/GpModelResource.h>

//--------------------------------------------------------------------------------------------------
//  FORWARD DECLARATIONS
//--------------------------------------------------------------------------------------------------

class GpModelDef;
class GpMeshDef;
class GpSubMeshDef;
class GpMaterialData;
class GpSpuBlendData;

//--------------------------------------------------------------------------------------------------
//  TYPE DEFINITIONS
//-------------------------------------------------------------------------------------------------

typedef FwStd::IntrusivePtr<GpModelDef> GpModelDefHandle;

//--------------------------------------------------------------------------------------------------
//  CLASS DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@class			GpModelDef

	@brief			
**/
//--------------------------------------------------------------------------------------------------

class GpModelDef : public FwNonCopyable
{
public:
	static GpModelDefHandle				Create(const void* pResource, void* pMem = NULL);
	static GpModelDefHandle				Create(const FwResourceHandle& hResource, void* pMem = NULL);

	static int							QuerySizeInBytes(const void* pResource);

	int									GetMeshCount() const;
	int									GetMeshIndex(FwHashedString hashedName) const;
	GpMeshDef*							GetMeshDef(int index) const;
	GpMeshDef*							GetMeshDef(FwHashedString hashedName) const;

	int									GetInverseBindMatrixCount() const;
	const FwTransform*					GetInverseBindMatrices() const;

	u32									GetSkeletonKey() const;

	const ModelResource*				GetResource() const;

	int									GetMaterialCount();
	GpMaterialData*						GetMaterialData(int materialIndex);

	void*								GetData() const;

private:
	GpModelDef() {};
	~GpModelDef();

	bool								m_ownsMemory;
	int									m_materialCount;
	int									m_refCount;
	GpMeshDef*							m_pMeshes;
	GpSubMeshDef*						m_pSubMeshes;
	GpMaterialData*						m_pMaterialData;
	const ModelResource*				m_pResource;
	FwResourceHandle					m_hResource;

	friend void	IntrusivePtrAddRef( GpModelDef* p )
	{
		++p->m_refCount;
	}
	friend void	IntrusivePtrRelease( GpModelDef* p )
	{
		if ( --p->m_refCount == 0 )
		{
			bool	ownsMemory = p->m_ownsMemory;
			
			p->~GpModelDef();
			
			if (ownsMemory)
				FW_DELETE_ARRAY( (u8*) p );
		}
	}
	friend u32 IntrusivePtrGetRefCount( GpModelDef* p )
	{
		return (u32)( p->m_refCount );
	}
};

//--------------------------------------------------------------------------------------------------
/**
	@class			GpMeshDef

	@brief			
**/
//--------------------------------------------------------------------------------------------------

class GpMeshDef : public FwNonCopyable
{
public:
	FwHashedString						GetNameHash() const;

	int									GetSubMeshCount() const;
	GpSubMeshDef*						GetSubMeshDef(int index) const;

	u32									GetTransformIndex() const;

	bool								HasBlendTargets() const;
	int									GetBlendTargetCount() const;
	int									GetBlendTargetIndex(FwHashedString hashedName) const;
	FwHashedString						GetBlendTargetHash(int index) const;
	const MeshResource::BlendTargetEntry&	GetBlendTargetEntry(int index) const;

	const MeshInstResource*				GetResource() const;

	void*								GetData() const;

private:
	GpMeshDef() {};
	~GpMeshDef();

	GpSubMeshDef*						m_pSubMeshes;
	const MeshInstResource*				m_pResource;

	friend class GpModelDef;
};

//--------------------------------------------------------------------------------------------------
/**
	@class			GpSubMeshDef

	@brief			
**/
//--------------------------------------------------------------------------------------------------

class GpSubMeshDef : public FwNonCopyable
{
public:
	Gc::PrimitiveType					GetPrimitiveType() const;

	int									GetIndexCount() const;
	const GcStreamBufferHandle&			GetIndexStream() const;
	const u16*							GetIndexData() const;

	int									GetVertexCount() const;
	const GcStreamBufferHandle&			GetVertexStream() const;
	const void*							GetVertexData() const;

	GpMaterialData*						GetMaterialData() const;

	bool								IsSkinned() const;
	int									GetSkinningMatrixCount() const;
	u16									GetSkinningMatrixIndex(int localIndex) const;
	const GcStreamBufferHandle&			GetSkinningWeightStream() const;
	const GcStreamBufferHandle&			GetSkinningIndexStream() const;

	bool								HasBlendTargets() const;
	int									GetBlendTargetCount() const;

	const SubMeshResource*				GetResource() const;

	void*								GetData() const;

private:
	GpSubMeshDef() {};
	~GpSubMeshDef();

	GcStreamBufferHandle				m_hIndexStream;
	GcStreamBufferHandle				m_hVertexStream;
	GcStreamBufferHandle				m_hSkinningWeightStream;
	GcStreamBufferHandle				m_hSkinningIndexStream;

	GpMaterialData*						m_pMaterialData;

	const SubMeshResource*				m_pResource;

	GpSpuBlendData*						m_pSpuBlendData;

	friend class GpModelDef;
	friend class GpMeshDef;
	friend class GpSubMeshInstance;
};

//--------------------------------------------------------------------------------------------------
/**
	@class			GpMaterialData

	@brief			
**/
//--------------------------------------------------------------------------------------------------

class GpMaterialData : public FwNonCopyable
{
public:
	const char*							GetMaterialFilename() const;
	FwHashedString						GetMaterialHash() const;				//!< @brief The name of the material.
	FwHashedString						GetMaterialInstanceHash() const;		//!< @brief The name of this usage of the material (typically the name of the shader in Maya/atgi).

	int									GetConstantCount() const;
	FwHashedString						GetConstantHash(int index) const;
	int									GetConstantIndex(FwHashedString hashedName) const;
	const FwVector4&					GetConstant(int index) const;
	const FwVector4&					GetConstant(FwHashedString hashedName) const;
	void								SetConstant(int index, FwVector4_arg data);
	void								SetConstant(FwHashedString hashedName, FwVector4_arg data);

	int									GetTextureCount() const;
	FwHashedString						GetTextureHash(int index) const;
	int									GetTextureIndex(FwHashedString hashedName) const;
	const char*							GetTextureFilename(int index) const;
	const char*							GetTextureFilename(FwHashedString hashedName) const;

	Gc::TexWrapMode						GetTextureWrapS( int index ) const;
	Gc::TexWrapMode						GetTextureWrapS( FwHashedString hashedName ) const;
	Gc::TexWrapMode						GetTextureWrapT( int index ) const;
	Gc::TexWrapMode						GetTextureWrapT( FwHashedString hashedName ) const;
	Gc::TexWrapMode						GetTextureWrapR( int index ) const;
	Gc::TexWrapMode						GetTextureWrapR( FwHashedString hashedName ) const;

	Gc::TexFilter						GetTextureMinFilter( int index ) const;
	Gc::TexFilter						GetTextureMinFilter( FwHashedString hashedName ) const;
	Gc::TexFilter						GetTextureMagFilter( int index ) const;
	Gc::TexFilter						GetTextureMagFilter( FwHashedString hashedName ) const;

	Gc::AnisotropyLevel					GetTextureAnisotropy( int index ) const;
	Gc::AnisotropyLevel					GetTextureAnisotropy( FwHashedString hashedName ) const;

	bool								IsTextureSrgbCorrected( int index ) const;
	bool								IsTextureSrgbCorrected( FwHashedString hashedName ) const;

	bool								IsTextureSignedExpanded( int index ) const;
	bool								IsTextureSignedExpanded( FwHashedString hashedName ) const;

	u32									GetTextureBorderColour( int index ) const;
	u32									GetTextureBorderColour( FwHashedString hashedName ) const;

	float								GetTextureLodMin( int index ) const;
	float								GetTextureLodMin( FwHashedString hashedName ) const;
	float								GetTextureLodMax( int index ) const;
	float								GetTextureLodMax( FwHashedString hashedName ) const;
	float								GetTextureLodBias( int index ) const;
	float								GetTextureLodBias( FwHashedString hashedName ) const;

	void								SetTexture(int index, const GcTextureHandle& hTexture);
	void								SetTexture(FwHashedString hashedName, const GcTextureHandle& hTexture);
	const GcTextureHandle&				GetTexture(int index) const;
	const GcTextureHandle&				GetTexture(FwHashedString hashedName) const;

	bool								IsAlphaBlendEnabled() const;
	Gc::BlendFunc						GetSourceBlendFunc() const;
	Gc::BlendFunc						GetDestBlendFunc() const;
	bool								IsAlphaTestEnabled() const;
	Gc::CmpFunc							GetAlphaTestFunc() const;
	float								GetAlphaTestRef() const;
	bool								IsZWriteEnabled() const;
	bool								IsBackfaceCullingEnabled() const;

	const MaterialResource*				GetResource() const;

private:
	struct Texture
	{
		
		GcTextureHandle hObject;
	};

	GpMaterialData() {};
	~GpMaterialData();

	FwHashedString						m_materialHash;
	FwHashedString						m_materialInstanceHash;

	int									m_constantCount;
	const FwHashedString*				m_pConstantHashedNames;
	FwVector4*							m_pConstantData;

	int									m_textureCount;
	const FwHashedString*				m_pTextureHashedNames;
	GcTextureHandle*					m_pTextures;

	const MaterialResource*				m_pResource;

	friend	class	GpModelDef;
	friend	class	GpModelInstance;
	FW_MEM_FRIEND();
};

//--------------------------------------------------------------------------------------------------
//  INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

#include <Gp/GpModel/GpModelDef.inl>

//--------------------------------------------------------------------------------------------------

#endif // GP_MODEL_DEF_H
