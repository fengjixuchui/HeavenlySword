//--------------------------------------------------------------------------------------------------
/**
	@file		GpModelInstance.h

	@brief		

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef GP_MODEL_RESOURCE_H
#define GP_MODEL_RESOURCE_H

//--------------------------------------------------------------------------------------------------
//  INCLUDES
//--------------------------------------------------------------------------------------------------

#include <Fw/FwMaths/FwTransform.h>
#include <Fp/FpGeom/FpSphere.h>
#include <Gc/GcStreamField.h>

//--------------------------------------------------------------------------------------------------
//  FORWARD DECLARATIONS
//--------------------------------------------------------------------------------------------------

struct ModelResource;
struct MeshResource;
struct MeshInstResource;
struct SubMeshResource;
struct PMResource;
struct BlendShapingResource;
struct SkinningResource;
struct MaterialResource;

//--------------------------------------------------------------------------------------------------

struct ModelResource
{
	u32									m_tag;

	u32									m_skeletonKey;

	s32									m_inverseBindMatrixCount;
	FwOffset<const FwTransform>			m_inverseBindMatrixArray;

	s32									m_meshInstCount;
	FwOffset<const MeshInstResource>	m_meshInstArray;

	s32									m_subMeshInstanceCount;
	s32									m_subMeshCount;
	FwOffset<const SubMeshResource>		m_subMeshArray;

	s32									m_materialCount;
	FwOffset<const MaterialResource>	m_materialArray;

	s32									m_staticInstanceCount;
	FwOffset<const FwTransform>			m_staticInstanceArray;

	FwOffset<void>						m_data;
};

struct MeshResource
{
	struct BlendTargetEntry
	{
		FwHashedString	m_nameHash;
		u32				m_index;
	};

	s32									m_blendTargetCount;
	FwOffset<const BlendTargetEntry>	m_blendTargetArray;

	s32									m_subMeshCount;
	FwOffset<const SubMeshResource>		m_subMeshArray;

	FwOffset<void>						m_data;
};

struct MeshInstResource
{
	enum	FlagBits
	{
		kDynamic_Bit,																				//!< @brief Indicates that this instances transform is in a skeleton.
	};

	enum	FlagMask
	{
		kDynamic	= 1 << kDynamic_Bit,
	};

	FwHashedString						m_nameHash;
	u32									m_flags;
	s32									m_transformIndex;
	FwOffset<const MeshResource>		m_meshResourceOffset;
};

struct SubMeshResource
{
	u32									m_primitiveType;

	s32									m_indexCount;
	FwOffset<const u16>					m_indexData;

	s32									m_vertexCount;
	s32									m_vertexStride;
	FwOffset<const u8>					m_vertexData;

	s32									m_vertexFieldCount;
	FwOffset<const GcStreamField>		m_vertexFieldArray;

	FwOffset<const MaterialResource>	m_materialOffset;

	FwOffset<const BlendShapingResource> m_blendShapingOffset;
	FwOffset<const SkinningResource>	m_skinningOffset;
	FwOffset<const PMResource>			m_PMOffset;

	FwOffset<void>						m_data;
};

struct PMResource
{
	struct Split
	{
		u16		m_splitVert;
		u8		m_numNewTris;
		u8		m_numFixFaces;
		float	m_cost;
	};

	typedef s32 FixFacesType;

	u32								m_numSplits;
	u32								m_numFixFaces;
	FwOffset<const Split>			m_splitsOffset;
	FwOffset<const FixFacesType>	m_fixFacesOffset;
};

struct BlendShapingResource
{
	struct BlendElement
	{
		u32					m_index;
		f32					m_delta[3];
	};

	struct BlendStream
	{
		u32								m_count;
		FwOffset<const BlendElement>	m_pElements;
	};

	s32									m_streamCount;
	FwOffset<const BlendStream>			m_streamArray;

	FwHashedString						m_positionHash;
};

struct SkinningResource
{
	s32									m_matrixRemapCount;
	FwOffset<const u16>					m_matrixRemapData;

	FwOffset<const s16>					m_weightData;
	FwOffset<const u8>					m_indexData;

	GcStreamField						m_weightField;
	GcStreamField						m_indexField;
};

struct MaterialResource
{
	struct Texture
	{
		FwOffset< const char >		m_filename;
		u32							m_wrapBits;	
		u32							m_borderColour;
		float						m_lodMin;
		float						m_lodMax;
		float						m_lodBias;
	};

	FwOffset< const char >				m_filename;
	u32									m_materialHash;				//!< @brief The name of the material.
	u32									m_materialInstanceHash;		//!< @brief The name of this usage of the material (typically the name of the shader in Maya/atgi).

	s32									m_constantCount;
	FwOffset<const FwHashedString> 		m_constantNameHashArray;
	FwOffset<FwVector4>					m_constantData;

	s32									m_textureCount;
	FwOffset<const FwHashedString>		m_textureNameHashArray;
	FwOffset< Texture >					m_textureData;

	u32									m_renderStateBits;		//!< Enabled/disabled bits for alpha blend/alpha test/z write/backface culling
	u16									m_sourceBlendFunc;
	u16									m_destBlendFunc;
	u32									m_alphaTestFunc;
	f32									m_alphaTestRef;
};

//--------------------------------------------------------------------------------------------------
/**
 Tests whether a mesh instance is baked in world space. I.e. has no instance transform and is
 not an "instance" in the "single resource multiple render" sense of the word. If the instance
 is baked then it's transform index is invalid and should not be used.

 @param pMeshInstance Mesh instance to test.

 @return True if the meshes vertices are baked in world space. False otherwise, indicating
   that the vertices are in some local space.
**/
//--------------------------------------------------------------------------------------------------

inline bool IsBaked( const MeshInstResource* pMeshInstance )
{
	return pMeshInstance->m_transformIndex < 0;
}

//--------------------------------------------------------------------------------------------------
/**
 Tests whether a mesh instances transform is dynamic. If so then its transform index
 refers to a dyanamic transform outside of the model. If not then the transform index refers to
 a static transform defined in the model.

 @param pMeshInstance Mesh instance to test.

 @return True if the instance transform index refers to some external transform array. False
   if the instance transform index refers to a transform inside the model.
**/
//--------------------------------------------------------------------------------------------------

inline bool IsDynamic( const MeshInstResource* pMeshInstance )
{
	return pMeshInstance->m_flags & MeshInstResource::kDynamic;
}

//--------------------------------------------------------------------------------------------------

#endif // GP_MODEL_RESOURCE_H
