//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef GP_ICE_MESH_H
#define GP_ICE_MESH_H

//--------------------------------------------------------------------------------------------------
//  INCLUDES
//--------------------------------------------------------------------------------------------------

#ifdef _DEBUG
#define ICEDEBUG 1
#else
#define ICEDEBUG 0
#endif

#include <Fw/Fw.h>
#include <Fw/FwMaths/FwTransform.h>
#include <Fw/FwStd/FwHashedString.h>
#include <Gc/GcKernel.h>
#include <Gc/GcShader.h>
#include <render/icerender.h>
#include <mesh/icemesh.h>
#include <mesh/icemeshfrontend.h>

//--------------------------------------------------------------------------------------------------
//  CLASS DEFINITIONS
//--------------------------------------------------------------------------------------------------

class GpIceMesh
{
public:

	enum VertexSetType
	{
		kBasicVertexSet	= 0,
		kPmVertexSet	= 1
	};

	enum MeshFlags
	{
		kDynamicTransform = 1
	};

	// pointers work like FwOffset
	struct PointerPatchTable
	{
		u32		m_count;
		u32		m_patchOffsets[];			// offsets within file of pointers to patch
	};

	// 4 byte aligned
	struct MaterialData
	{
		struct Texture
		{
			const char*					m_pFilename;
			u32							m_wrapBits;	
			u32							m_borderColour;
			float						m_lodMin;
			float						m_lodMax;
			float						m_lodBias;
		};

		const char*							m_pFilename;
		FwHashedString						m_materialHash;
		FwHashedString						m_materialInstanceHash;

		u32									m_constantCount;
		u32									m_textureCount;
		FwHashedString*				 		m_pConstantNames;
		FwVector4*							m_pConstantData;
		FwHashedString*						m_pTextureNames;
		Texture*							m_pTextureData;
		GcTextureHandle*					m_pTextures;				// this array gets filled at runtime

		u32									m_renderStateBits;		//!< Enabled/disabled bits for alpha blend/alpha test/z write/backface culling
		u16									m_sourceBlendFunc;
		u16									m_destBlendFunc;
		u32									m_alphaTestFunc;
		f32									m_alphaTestRef;
	};
	
	// 4 byte aligned
	struct AttributeInfo
	{
		FwHashedString	m_name;
		u8				m_semanticId;		// semantic id, or stream + 0xF0 if attribute is pass through
		u8				m_stride;
		u8				m_count : 4;
		u8				m_type : 4;
		u8				m_offset;
	};

	// 16 byte aligned
	struct StreamInfo
	{
		Ice::Mesh::MeshProgramInfo				m_mpi;
		Ice::Mesh::FixedFormatInfo				m_ffi;
		AttributeInfo*							m_pAi;
		u8										m_meshProgramType;
		u8										m_attributeInfoCount;
		u8										m_pad[2];
		u32										m_deltaStreamUsageBits;
	};

	// 4 byte aligned
	struct Renderable
	{
		StreamInfo*							m_pStreamInfo;
		MaterialData*						m_pMaterialData;
		u16									m_vertexSetCount;
		u8									m_vertexSetType;		// uses VertexSetType enum
		u8									m_procFlags;			// uses Ice::Mesh::ProcessingFlags enum
		union
		{
			Ice::Mesh::PmVertexSet**		m_pPmVertexSets;
			Ice::Mesh::BasicVertexSet**		m_pVertexSets;
		};
		Ice::Mesh::VertexSetBlendShapes**	m_pBlendShapes;			// pointer is NULL if no vertex set has blendshapes. table may have NULLs in it, if some do and some don't.
	};

	// 4 byte aligned
	struct LodGroup
	{
		u8				m_startLod;
		u8				m_endLod;
		u16				m_renderableCount;
		Renderable**	m_pRenderables;
	};

	// 4 byte aligned
	struct BlendTarget
	{
		FwHashedString	m_name;
		s32				m_targetIndex;								// -1 if this mesh has no target for this weight name
	};

	// 4 byte aligned
	struct CpmNode
	{
		float						m_boundingSphere[4];
		Ice::Mesh::PmVertexSet*		m_pVertexSet;
		CpmNode**					m_pChildNodes;
		u32							m_childCount;
	};

	// 4 byte aligned
	struct CpmRenderable
	{
		StreamInfo*				m_pStreamInfo;
		MaterialData*			m_pMaterialData;
		u16						m_treeCount;
		u8						m_procFlags;			// uses Ice::Mesh::ProcessingFlags enum
		u8						m_pad0;
		CpmNode**				m_pTreeRoots;
	};

	// 4 byte aligned
	struct DpmMesh
	{
		u16				m_lodGroupCount;
		u16				m_blendTargetCount;
		LodGroup**		m_pLodGroups;
		BlendTarget*	m_pBlendTargets;
	};

	// 4 byte aligned
	struct CpmMesh
	{
		u8							m_lodGroupCount;		
		u8							m_lodCount;				
		u16							m_renderableCount;		
		u32*						m_pLodGroupStartLods;	
		CpmRenderable**				m_pRenderables;			
	};

	// 4 byte aligned
	struct Mesh
	{
		u8						m_isCpm;
		u8						m_pad[3];
		u32						m_extraDataSize;
		u8*						m_pExtraData;
		union
		{
			CpmMesh				m_cpmMesh;
			DpmMesh				m_dpmMesh;
		};
	};

	// 4 byte aligned
	struct MeshInstance
	{
		FwHashedString	m_name;
		u32				m_flags;				// uses MeshFlags enum
		u16				m_transformIndex;		// 0xffff if mesh has no transform
		u16				m_pad0;
		Mesh*			m_pMesh;
	};

	// 16 byte aligned
	struct FileHeader
	{
		u32				m_tag;
		FwOffset<u32>	m_pointerPatchupTable;
		u32				m_skeletonKey;
		u16				m_instanceCount;
		u16				m_inverseBindMatrixCount;
		u16				m_instanceTransformCount;
		u16				m_materialCount;
		u32				m_extraDataSize;
		MeshInstance*	m_pInstances;
		FwTransform*	m_pInverseBindMatrices;
		FwTransform*	m_pInstanceTransforms;
		MaterialData*	m_pMaterialData;
		u8*				m_pExtraData;
	};

	static const u32 kInvalidIndex = (u32)-1;

	static void PatchPointers(FileHeader* pHeader);
	static void SetStreamIoOffsets(FileHeader* pHeader);

	static u32 GetConstantIndex(const MaterialData* pMaterial, FwHashedString hashedName);
	static FwVector4& GetConstant(const MaterialData* pMaterial, u32 index);
	static FwVector4& GetConstant(const MaterialData* pMaterial, FwHashedString hashedName);
	
	static u32 GetTextureIndex(const MaterialData* pMaterial, FwHashedString hashedName);
	static const GcTextureHandle& GetTexture(const MaterialData* pMaterial, u32 index);
	static const GcTextureHandle& GetTexture(const MaterialData* pMaterial, FwHashedString hashedName);

	static Gc::TexWrapMode GetTextureWrapS( const MaterialData* pMaterial, int index );
	static Gc::TexWrapMode GetTextureWrapS( const MaterialData* pMaterial, FwHashedString hashedName );
	static Gc::TexWrapMode GetTextureWrapT( const MaterialData* pMaterial, int index );
	static Gc::TexWrapMode GetTextureWrapT( const MaterialData* pMaterial, FwHashedString hashedName );
	static Gc::TexWrapMode GetTextureWrapR( const MaterialData* pMaterial, int index );
	static Gc::TexWrapMode GetTextureWrapR( const MaterialData* pMaterial, FwHashedString hashedName );

	static Gc::TexFilter GetTextureMinFilter( const MaterialData* pMaterial, int index );
	static Gc::TexFilter GetTextureMinFilter( const MaterialData* pMaterial, FwHashedString hashedName );
	static Gc::TexFilter GetTextureMagFilter( const MaterialData* pMaterial, int index );
	static Gc::TexFilter GetTextureMagFilter( const MaterialData* pMaterial, FwHashedString hashedName );

	static Gc::AnisotropyLevel GetTextureAnisotropy( const MaterialData* pMaterial, int index );
	static Gc::AnisotropyLevel GetTextureAnisotropy( const MaterialData* pMaterial, FwHashedString hashedName );

	static u32 GetTextureBorderColour( const MaterialData* pMaterial, int index );
	static u32 GetTextureBorderColour( const MaterialData* pMaterial, FwHashedString hashedName );

	static float GetTextureLodMin( const MaterialData* pMaterial, int index );
	static float GetTextureLodMin( const MaterialData* pMaterial, FwHashedString hashedName );
	static float GetTextureLodMax( const MaterialData* pMaterial, int index );
	static float GetTextureLodMax( const MaterialData* pMaterial, FwHashedString hashedName );
	static float GetTextureLodBias( const MaterialData* pMaterial, int index );
	static float GetTextureLodBias( const MaterialData* pMaterial, FwHashedString hashedName );

	static void ResetTextureHandles(FileHeader* pHeader);

	static bool IsAlphaBlendEnabled( const MaterialData* pMaterial );
	static Gc::BlendFunc GetSourceBlendFunc( const MaterialData* pMaterial );
	static Gc::BlendFunc GetDestBlendFunc( const MaterialData* pMaterial );
	static bool IsAlphaTestEnabled( const MaterialData* pMaterial );
	static Gc::CmpFunc GetAlphaTestFunc( const MaterialData* pMaterial );
	static float GetAlphaTestRef( const MaterialData* pMaterial );
	static bool IsZWriteEnabled( const MaterialData* pMaterial );
	static bool IsBackfaceCullingEnabled( const MaterialData* pMaterial );

	static bool RenderableIsSkinned(const Renderable* pRenderable);
	static bool RenderableIsTrimmed(const Renderable* pRenderable);
	static bool RenderableHasPm(const Renderable* pRenderable);

	static void SetStreamAttributes(
		const StreamInfo* pStreamInfo,
		const GcShaderHandle& hShader,
		Ice::Render::CommandContext* pContext,
		Ice::Mesh::VertexInputInfo* pVii);

	static void SetStreamProcessing(
		const StreamInfo* pStreamInfo,
		const GcShaderHandle& hShader,
		Ice::Mesh::MeshProgramInfo* pInfo,
		u32* pDeltaStreamUsageBits);

private:

	static void SetCpmStreamIoOffsets(const CpmNode* pCpmNode);
};

//--------------------------------------------------------------------------------------------------
//	INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

#include "GpIceMesh.inl"

#endif
