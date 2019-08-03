//--------------------------------------------------------------------------------------------------
/**
	@file		GpModelInstance.h

	@brief		

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef GP_MODEL_INSTANCE_H
#define GP_MODEL_INSTANCE_H

//--------------------------------------------------------------------------------------------------
//  INCLUDES
//--------------------------------------------------------------------------------------------------

#include <Gp/GpModel/GpModelDef.h>

//--------------------------------------------------------------------------------------------------
//  FORWARD DECLARATIONS
//--------------------------------------------------------------------------------------------------

class GpModelInstance;
class GpMeshInstance;
class GpSubMeshInstance;
class MultiThreadSafeJobList;

//--------------------------------------------------------------------------------------------------
//  TYPE DEFINITIONS
//-------------------------------------------------------------------------------------------------

typedef FwStd::IntrusivePtr<GpModelInstance> GpModelInstanceHandle;
typedef FwStd::IntrusivePtr<GpMeshInstance> GpMeshInstanceHandle;
typedef FwStd::IntrusivePtr<GpSubMeshInstance> GpSubMeshInstanceHandle;

//--------------------------------------------------------------------------------------------------
//  CLASS DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@class			GpModelInstance

	@brief			
**/
//--------------------------------------------------------------------------------------------------


class GpModelInstance : public FwNonCopyable
{
public:
	static GpModelInstanceHandle		Create(const		GpModelDefHandle& hModelDef,
											   const int*	pMatOverrideMeshes = NULL,
											   int			matOverrideMeshCount = 0,
											   void*		pMem = NULL);

	static u32							QuerySizeInBytes(const		GpModelDefHandle& hModelDef,
														 const int*	pMatOverrideMeshes = NULL,
														 int		matOverrideMeshCount = 0);

	int									GetMeshCount() const;
	int									GetMeshIndex(FwHashedString hashedName) const;
	const GpMeshInstanceHandle&			GetMesh(int index) const;
	const GpMeshInstanceHandle&			GetMesh(FwHashedString hashedName) const;

	void								SetupSkinning(const FwTransform* pMatrices);

	void								SetMeshTransforms(const FwTransform* pTransforms);
	void								ClearMeshTransforms();

	u32									GetSkeletonKey() const;

	const GpModelDefHandle&				GetDef() const;

private:
	GpModelInstance() {};
	~GpModelInstance();

	int									m_refCount;
	int									m_overrideMaterialCount;

	GpMeshInstanceHandle*				m_hMeshes;
	void*								m_pSkinningMatrices;
	GpModelDefHandle					m_hDef;
	GpMaterialData*						m_pOverrideMaterialData;

	bool								m_ownsMemory;

	friend void	IntrusivePtrAddRef( GpModelInstance* p )
	{
		++p->m_refCount;
	}
	friend void	IntrusivePtrRelease( GpModelInstance* p )
	{
		if ( --p->m_refCount == 0 )
		{
			bool ownsMemory = p->m_ownsMemory;
			p->~GpModelInstance();
			if(ownsMemory)
				FW_DELETE_ARRAY( (u8*)p );
		}
	}
	friend u32 IntrusivePtrGetRefCount( GpModelInstance* p )
	{
		return (u32)( p->m_refCount );
	}
};

//--------------------------------------------------------------------------------------------------
/**
	@class			GpMeshInstance

	@brief			
**/
//--------------------------------------------------------------------------------------------------

class GpMeshInstance : public FwNonCopyable
{
public:
	FwHashedString						GetNameHash() const;

	int									GetSubMeshCount() const;
	const GpSubMeshInstanceHandle&		GetSubMesh(int index) const;

	void								SetVisible(bool visible);
	bool								IsVisible() const;

	u32									GetTransformIndex() const;

	bool								HasBlendTargets() const;
	int									GetBlendTargetCount() const;
	int									GetBlendTargetIndex(FwHashedString hashedName) const;
	FwHashedString						GetBlendTargetHash(int index) const;
	float								GetBlendTargetWeight(int index) const;
	float								GetBlendTargetWeight(FwHashedString name) const;
	const MeshResource::BlendTargetEntry&	GetBlendTargetEntry(int index) const;
	void								SetBlendTargetWeight(int index, float weight);
	void								SetBlendTargetWeight(FwHashedString name, float weight);
	void								ApplyBlendShaping();
	// temporary interface to spu blendshaping
	void								ApplyBlendShapingSpu(MultiThreadSafeJobList& jobList);

	const GpMeshDef*					GetDef() const;

private:
	GpMeshInstance() {};
	~GpMeshInstance();

	int									m_refCount;
	bool								m_isVisible;
	GpSubMeshInstanceHandle*			m_hSubMeshes;
	const GpMeshDef*					m_pDef;
	float*								m_pBlendTargetWeights;

	friend void	IntrusivePtrAddRef( GpMeshInstance* p )
	{
		++p->m_refCount;
	}
	friend void	IntrusivePtrRelease( GpMeshInstance* p )
	{
		if ( --p->m_refCount == 0 )
			p->~GpMeshInstance();
	}
	friend u32 IntrusivePtrGetRefCount( GpMeshInstance* p )
	{
		return (u32)( p->m_refCount );
	}

	friend class GpModelInstance;
};

//--------------------------------------------------------------------------------------------------
/**
	@class			GpSubMeshInstance

	@brief			
**/
//--------------------------------------------------------------------------------------------------

class GpSubMeshInstance : public FwNonCopyable
{
public:
	Gc::PrimitiveType					GetPrimitiveType() const;

	int									GetIndexCount() const;
	const GcStreamBufferHandle&			GetIndexStream() const;
	const GcStreamBufferHandle&			GetVertexStream() const;

	bool								HasDynamicStream() const;
	const GcStreamBufferHandle&			GetDynamicStream() const;

	GpMaterialData*						GetMaterialData() const;

	bool								IsSkinned() const;
	int									GetSkinningMatrixCount() const;
	const void*							GetSkinningMatrix(int index) const;
	const GcStreamBufferHandle&			GetSkinningWeightStream() const;
	const GcStreamBufferHandle&			GetSkinningIndexStream() const;

	bool								HasTransform() const;
	const FwTransform&					GetTransform() const;

	bool								HasBlendTargets() const;
	int									GetBlendTargetCount() const;
	void								ApplyBlendShaping(const float* pWeights);
	// temporary interface to spu blendshaping
	void								ApplyBlendShapingSpu(const float* pWeights, MultiThreadSafeJobList& jobList);

	const GpSubMeshDef*					GetDef() const;

private:
	GpSubMeshInstance() {};
	~GpSubMeshInstance();

	u32									m_flags;
	int									m_refCount;
	const void*							m_pMatrix;					///< Points to rigid Matrix44, or array of 3x4 skinning matrices
	GpMaterialData*						m_pMaterialData;
	const GpSubMeshDef*					m_pDef;
	GcStreamBufferHandle				m_hDynamicStream;

	friend void	IntrusivePtrAddRef( GpSubMeshInstance* p )
	{
		++p->m_refCount;
	}
	friend void	IntrusivePtrRelease( GpSubMeshInstance* p )
	{
		if ( --p->m_refCount == 0 )
			p->~GpSubMeshInstance();
	}
	friend u32 IntrusivePtrGetRefCount( GpSubMeshInstance* p )
	{
		return (u32)( p->m_refCount );
	}

	friend class GpModelInstance;
};

//--------------------------------------------------------------------------------------------------
//  INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

#include <Gp/GpModel/GpModelInstance.inl>

//--------------------------------------------------------------------------------------------------

#endif // GP_MODEL_INSTANCE_H
