#ifndef _EXPORTSTRUCT_CLUMP_PS3_H_
#define _EXPORTSTRUCT_CLUMP_PS3_H_


#include "core/semantics.h"
#include "core/VolumeTypes.h"
#include "core/soundprimtypes.h"
#include "gfx/vertexdeclaration.h"
#include "gfx/texture.h"

#ifdef JAM_EXPORTER
#include "makeid.h"
#endif

// CLUMP_VERSION should begin 'CS' on ps3, and 'CC' on PC - both currently being version '01' 
//	for details of spec see JIRA item: TLS-174. [PWC]

#define	OLD_CLUMP_VERSION	MAKE_ID( 'C', 'S', '0', '2' )	
#define	CLUMP_VERSION		MAKE_ID( 'C', 'S', '0', '3' )
#define NEW_CLUMP_VERSION	MAKE_ID( 'C', 'S', '0', '4' )

// clump flags
namespace ClumpFlags
{
	typedef enum
	{
		IS_HAIR_RENDERABLE = (1<<1),
		DEBUG_FLAG = (1<<2),
		POSITIONS_NEED_RECONSTRUCTION = 1 << 3,	// If set, the position coordinates in the mesh are normalized to the [-1.0, 1.0] range, and need to be transformed by the reconstruction matrix to get to object space.

		IS_BLENDSHAPES_CAPABLE = ( 1 << 4 ),	// this mesh can have associated blendshapes
	} MESH_FLAGS;

} // end of namespace ClumpFlags

template< typename type >
class ntDiskPointer
{
public:
	//! normal pointer access
	type* operator->()
	{
		return m_pPointer;
	}
	//! const pointer access
	const type* operator->() const
	{
		return m_pPointer;
	}
	//! array access
	type& operator[]( const unsigned int index )
	{
		return *(m_pPointer + index);
	}
	//! const array access
	const type& operator[]( const unsigned int index ) const
	{
		return *(m_pPointer + index);
	}

	//! allow conversion to base type, so it looks like a normal pointer in every way
	operator type*()
	{
		return m_pPointer;
	}

	//! allow conversion to const base type, so it looks like a normal pointer in every way
	operator type*() const
	{
		return m_pPointer;
	}

	//! perform a fixup from the disk to the normal pointer. Must be called once before use
	void Fixup( const void* pBase )
	{
		ResolveOffset(m_pPointer, pBase );
	}

	//! recover the original file offset of the pointer
	void ReverseFixup( const void* pBase )
	{
		RecoverOffset(m_pPointer, pBase );
	}

private:
	type* m_pPointer;
};

#ifndef JAM_EXPORTER

class	CMeshHeader;
class	CMeshVertexElement;
class	CTransformLinkage;
class	CBindPose;
class	CColprimDesc;
class	CMaterialProperty;
class	ClumpHeader_RuntimeDataPS3;
class	CHierarchy;

/***************************************************************************************************
*	
*	CLASS			CClumpHeader
*
*	DESCRIPTION		This class defines the header of clump data. This header would be the start of
*					loaded data, and is assumed to have been loaded on a page boundary to memory
*					that was allocated as physically contiguous. All of the other classes defined
*					within this file are to be found following this particular class. Where pointers
*					exist, they are stored in the file as offsets from the start of the CClumpHeader
*					object. They are resolved upon load.
*
***************************************************************************************************/

ALIGNTO_PREFIX(16) class CClumpHeader
{
public:
	u_int					m_uiVersionTag;					// Type & Version Tag ( "CL02" )
	int						m_iFlags;						// Flags - exact contents to be confirmed.
	u_int					m_uiHeaderPageSize;				// This is the size of all the data prior to GPU stuff (ie callable pushbuffers & vertex buffers). It should always be a multiple of 4Kb.
	u_int					m_uiHierarchyKey;				// This is a 32-bit encoded value formed from the names of hierarchy components. Zero for now..

	FwHashedString			m_obClumpName;					// A hash of the name of the clump.

	ntDiskPointer<int8_t>	m_pCharacterBoneToIndexArray;	// Offset/pointer to an array of 64 bone->index remap values. We need this so we can get to a transform index
															// from a logical joint/transform number. Unused values are filled with -1... note the signed nature of this
															// array too. This pointer should be NULL for non-character clumps.

	int									m_iNumberOfTransforms;			// How many transforms do we have? 

private:
	// Don't want anything messing with this apart from CHierarchy because
	// CHierarchy changes the format completely so it won't be an array
	// of CTransformLinkage objects anymore.
	// TODO: Needs a clump format change :( - also, see Hierarchy.h for other clump format changes. [ARV].
	friend class CHierarchy;
	friend class CClumpLoader;
	ntDiskPointer<CTransformLinkage>	m_pobTransformLinkageArray;		// Offset/pointer to an array of transform linkage structures

public:
	ntDiskPointer<CBindPose>			m_pobBindPoseArray;				// Offset/pointer to an array of bind pose structures
	ntDiskPointer<CMatrix>				m_pobSkinToBoneArray;			// Offset/pointer to an array of skin->bone transformation matrices (NULL if clump has no skinned meshes)

	int									m_iUnusedInt0;
	void*								m_pUnusedPtr0;

	int									m_iUnusedInt1;
	ClumpHeader_RuntimeDataPS3*			m_pAdditionalData;

	int									m_iUnusedInt2;
	void*								m_pUnusedPtr2;

	int									m_iUnusedInt3;
	void*								m_pUnusedPtr3;

	int									m_iNumberOfMeshes;				// How many meshes do we have?
	ntDiskPointer<CMeshHeader>			m_pobMeshHeaderArray;			// Offset/pointer to an array of mesh headers

private:
	~CClumpHeader();										// Forces proper deallocation.
} ALIGNTO_POSTFIX(16);


/***************************************************************************************************
*	
*	CLASS			CTransformLinkage
*
*	DESCRIPTION		This class defines information that is used to construct the transforms 
*					associated with a hierarchy. It also contains a hashed version of the transform
*					name that can help in the identification of a transform within game code.
*
***************************************************************************************************/

class CTransformLinkage
{
public:
	int						m_iFlags;						// 32 bit flags, defined as a struct/bitfield.
	FwHashedString			m_obNameHash;					// Hash value formed from the name of the transform in Maya..
	short					m_sParentIndex;					// Index of parent in transform array. Must be -1 if this is the root (ie no parent).
	short					m_sNextSiblingIndex;			// Index of next sibling in transform array. Must be -1 if there is no child
	short					m_sFirstChildIndex;				// Index of first child in transform array. Must be -1 if no child.
	short					m_sPad;							// Padding to 32-bits (and also 128-bits!).
};


/***************************************************************************************************
*	
*	CLASS			CBindPose
*
*	DESCRIPTION		This class defines the bind pose associated with a clump, along with offset
*					information used to ensure that differently sized skeletons can share 
*					animations. Each bones offset for a particular character is generated by looking
*					at the difference between the base hierarchy and the character hierarchy. Any
*					translations of shareable bones generated by the exporter would be offset by this
*					difference - effectively animating them based on the base hierarchy. When the 
*					hierarchy is animated in-game, the offset required to move from base to 
*					character hierarchy is added back to any computed translation.
*
*					Having bind pose information available also allows us to have non-animating
*					hierarchies (that are completely under code control), with the transforms for a
*					clump being initially set to the values in the CBindPose array.
*
*	NOTES
*
*		Wha..? How Big?
*		---------------
*
*			Yes... the bone offset is a CPoint object. I know.. it's added another 16 Bytes onto
*			this structure. We support offsets in each axis, so a single floating point value
*			just wouldn't cut it.
*
***************************************************************************************************/

ALIGNTO_PREFIX(16) class CBindPose
{
public:
	CQuat					m_obRotation;					// Rotation from parent
	CPoint					m_obTranslation;				// Translation from parent
	CPoint					m_obBoneOffset;					// Offset applied to translation to cope
															// with different sized characters sharing anims.
} ALIGNTO_POSTFIX(16);

/***************************************************************************************************
*	
*	CLASS			CVertexStream
*
*	DESCRIPTION		A vertex attribute stream in the CMeshHeader.
*
***************************************************************************************************/

class CVertexStream
{
public:
	int							m_iNumberOfVertexElements;		// The number of elements per vertex in this stream.
	ntDiskPointer<CMeshVertexElement> m_pobVertexElements;		// A pointer to the elements table.

	int							m_iVertexStride;				// The total stride per vertex (in bytes).
	int							m_iNumberOfVertices;			// The number of vertices in the VertexStream. Should be the same for all streams in a mesh.
	ntDiskPointer<u_char>		m_pvVertexBufferData;			// The vertex buffer data.

	ntDiskPointer<u_char>		m_pobVertexBuffer;				// This is exported as NULL, and patched up on load.
};

/***************************************************************************************************
*	
*	CLASS			CMeshHeader
*
*	DESCRIPTION		This class defines the header for a mesh. A mesh is essentially a renderable 
*					chunk of geometry that consists of a number of streams. It also has an associated
*					material, but that stuff is coming along later.
*
*	NOTES
*
*		Meshes and Bones
*		----------------
*
*			As described in the 'Transform and Hierarchy.cpp' document, we have a system where we
*			allow up to 44 bones *per mesh* as opposed to per object (as was the case in KFC). To 
*			facilitate this change each mesh has an array of chars that map bone indices to indices
*			within the clump hierarchy array of skin matrices. For all meshes, vertex information
*			relating to skinning must refer to local bone indices (ie from 0 to the number of 
*			bones used). The mesh rendering system will ensure that skin matrices are correctly
*			loaded from their physical location in the hierarchy skin matrix array to the relevant
*			vertex shader constants.
*
*		Axis Aligned Bounds
*		-------------------
*
*			This is a local space aligned bounding box around the entire mesh. Although better 
*			bounds may be available (especially for characters) this will do for getting a scene
*			graph up and running.
*
***************************************************************************************************/

class CMeshHeader
{
public:
	float						m_afReconstructionMatrix[16];	// Matrix used to transform the normalized position coordinates into object space.
	int							m_iFlags;						// Flags.. one of these will say whether the pushbuffer is CPU copyable, or directly callable.
	FwHashedString				m_obNameHash;					// Hash value formed from the name of the mesh in Maya.
	int							m_iTransformIndex;				// Which transform index is used for this mesh. Use zero for any skinned mesh, please, as it uses object space.

	int							m_iNumberOfBonesUsed;			// Holds how many bones this mesh uses if skinned. Zero for non-skinned meshes
	ntDiskPointer<u_char>		m_pucBoneIndices;				// Pointer to an array of chars holding bone indices. NULL if not skinned.

	int							m_iNumberOfVertexStreams;		// The number of vertex attribute streams.
	ntDiskPointer<CVertexStream> m_pobVertexStreams;			// Pointer to the CVertexStreams as a flat array.

	int							m_iNumberOfIndices;				// How many indices are in the indexed strip
	ntDiskPointer<u_short>		m_pusIndices;					// Pointer to the index data

	void*						m_pUnusedPtr2;					// used to be a patched up index buffer handle. Remove at next clump revision

	float						m_afAxisAlignedMin[3];			// The minimum x, y and z of a local-space aligned bounding box.
	float						m_afAxisAlignedMax[3];			// The maximum x, y and z of a local-space aligned bounding box.
	float						m_afBoundingSphere[4];			// Position and radius of bounding sphere (x, y, z, radius)

	FwHashedString				m_obMaterialNameHash;			// A hash code for the game material name.
	
	int							m_iNumberOfProperties;			// The number of exported material properties for this clump.
	ntDiskPointer<CMaterialProperty> m_pobMaterialProperties;		// The material property table.

public:
	bool IsPositionCompressed(void) const 
	{
		return ((m_iFlags & ClumpFlags::POSITIONS_NEED_RECONSTRUCTION) != 0);
	}
};

/***************************************************************************************************
*	
*	CLASS			CMeshVertexElement
*
*	DESCRIPTION		
*
***************************************************************************************************/

class CMeshVertexElement
{
public:
	STREAM_SEMANTIC_TYPE	m_eStreamSemanticTag;	// Tag describing what this element actually IS for the game.
	VERTEX_DECL_STREAM_TYPE	m_eType;				// The type of this element.
	int						m_iOffset;				// The offset of this element in the stream (in Bytes).
	int						m_iStride;				// The stride of this element (in Bytes).
public:
	CMeshVertexElement() {}
	CMeshVertexElement(STREAM_SEMANTIC_TYPE eStreamSemanticTag, VERTEX_DECL_STREAM_TYPE eType, int iOffset, int iStride)
		:m_eStreamSemanticTag(eStreamSemanticTag)
		,m_eType(eType)
		,m_iOffset(iOffset)
		,m_iStride(iStride)
	{
		// nothing
	}
};

/***************************************************************************************************
*	
*	CLASS			CMaterialProperty
*
*	DESCRIPTION		An exported material property.
*
***************************************************************************************************/

struct MATERAL_TEXTURE_DATA
{
	ntDiskPointer<const char>		pcTextureName;		//!< The name of texture used in Maya.
	// Deano line below was IDirect3DTexture9
	Texture::Ptr					pobTexture;			//!< Patched up on load.
};

struct MATERIAL_FLOAT_DATA
{
	float				afFloats[4];		//!< A 4-vector constant.
};

class CMaterialProperty
{
public:
    MATERAL_TEXTURE_DATA&		GetTextureData()
	{
		return *((MATERAL_TEXTURE_DATA*)aData);
	}
    const MATERAL_TEXTURE_DATA&	GetTextureData() const
	{
		return *((MATERAL_TEXTURE_DATA*)aData);
	}

	MATERIAL_FLOAT_DATA&			GetFloatData()
	{
		return *((MATERIAL_FLOAT_DATA*)aData);
	}
	const MATERIAL_FLOAT_DATA&		GetFloatData() const
	{
		return *((MATERIAL_FLOAT_DATA*)aData);
	}

	//!< The game semantic tag for this property.
	int				m_iPropertyTag;
	
	// the data size is always 128 bits
	char			aData[ sizeof(float) *  4 ];
public:
	CMaterialProperty()
		:m_iPropertyTag(0)
	{
		// this should hopefully initialise MATERIAL_FLOAT_DATA or MATERAL_TEXTURE_DATA
		// in a nice state (for procedrual "in-game" use)
		for(uint32_t i = 0 ; i < sizeof(float) *  4 ; ++i ) aData[i]=char(0);
	}
};

/***************************************************************************************************
*	
*	CLASS			CColprimDesc
*
*	DESCRIPTION		Information needed to instantiate a collision primitive.
*
***************************************************************************************************/

ALIGNTO_PREFIX(16) class CColprimDesc
{
public:
	CQuat					m_obRotation;					// Rotation for this collision primitive relative to parent
	CPoint					m_obTranslation;				// Translation for this collision primitive relative to parent
	ntDiskPointer<const char>	m_pcType;
	int						m_iTransform;					// the transform this colprim hangs off
	COLLISION_VOLUME_TYPE	m_eType;						// type of colprim (should be enum!)
	union
	{
		struct
		{
			float	fRadius;
		}	m_obSphereData;									// Data (when m_eType is CV_TYPE_SPHERE)

		struct
		{
			float	fXSize;
			float	fYSize;
			float	fZSize;
		}	m_obBoxData;									// Data (when m_eType is CV_TYPE_OBB)

		struct
		{
			float	fLength;
			float	fRadius;
		}	m_obCapsuleData;								// Data (when m_eType is CV_TYPE_CAPSULE)

		struct
		{
			int		iTriCount;
			ntDiskPointer<char>	pvTriangles;							// NOTE: Pointer fixup required on load
		}	m_obMeshData;									// Data (when m_eType is CV_TYPE_TRIANGLEMESH)
	};
} ALIGNTO_POSTFIX(16);

#endif // !JAM_EXPORTER


#endif //_EXPORTSTRUCT_CLUMP_PS3_H_
