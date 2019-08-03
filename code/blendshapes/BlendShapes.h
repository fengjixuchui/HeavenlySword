//--------------------------------------------------
//!
//!	\file BlendShapes.h
//!
//--------------------------------------------------

#ifndef _BLENDSHAPES_H_
#define _BLENDSHAPES_H_



//#include "core/smartptr.h"

#include "blendshapes/blendshapes_export.h"
#include "blendshapes/blendshapes_managers.h"
#include "blendshapes/blendshapes_constants.h"
#include "game/entity.h"
#include "game/entity.inl"

class BlendedMeshInstance;
class CClumpHeader;

class MeshBSSet;
typedef MeshBSSet*  MeshBSSetPtr_t;

class BSSet;
typedef  BSSet*  BSSetPtr_t;

class BlendedMeshInstance;
class CEntity;


class BSSet
{
public:
//	static BSSetPtr_t Create( BSClumpHeaderPtr_t pBSClumpHeader );

	BSSet( const BSClumpHeaderPtr_t pBSClumpHeader, CEntity* pEntity );
	BSSet();
	~BSSet();

	//! name of this blendshape set
	uint32_t			GetNameHash( void ) const;
	//! name of the associated clump
	uint32_t			GetClumpNameHash( void ) const;
	//! associated clump's hierarchy key
	uint32_t			GetClumpHierarchyKey( void ) const;
	//! number of meshes affected by this set
	u_int				GetNumOfMeshBSSets( void ) const;
	// ! how many bs targets on this set?
	u_int				GetNumOfBSTargets( void ) const;
	//! gets a mesh blendshape set
	MeshBSSet*			GetMeshBSSetByIndex( u_int index );

	CEntity*			GetEntity( void ) const;



	BSClumpHeaderPtr_t	GetHeader( void ) const { return m_pBSClumpHeader; }


	//! blendshape target accessors
	//! access by target ID has been eliminated as we now have custom targets all the time
	float				GetTargetWeightByIndex( u_int targetIndex ) const;
	const float*		GetTargetWeights( void ) const;
	float*				GetTargetWeights( void );
	void				SetTargetWeightByIndex( u_int targetIndex, float weight );
	void				SetTargetWeights( const float* pWeights );


	//! wrinkle map area weight accessors
	float				GetWrinkleAreaWeight( u_int areaIndex ) const;
	float				GetWrinkleAreaWeight( const CHashedString &wrinkleAreaName ) const; NOT_IMPLEMENTED
	const float*		GetWrinkleAreaWeights( void ) const;
	float*				GetWrinkleAreaWeights( void );
	void				SetWrinkleAreaWeight( u_int areaIndex, float weight );
	void				SetWrinkleAreaWeight( const CHashedString& wrinkleAreaName, float weight );	NOT_IMPLEMENTED


	//! resets all weights ( bstarget and wrinkle area)
	void				ResetWeights( void );

private:

	
	
	//! Santa's little helpers
	//int IDToIndex( BSTargetID id ) const;
	//void InitIDToIndexArray( void );
	void InitBSMeshSetArray( void );
	void DestroyBSMeshSetArray( void );

private:
	CEntity*							m_pEntity;
	//! the actual blendshape data
	BSClumpHeaderPtr_t					m_pBSClumpHeader;
	//! children bsmesh instance array
	MeshBSSet*							m_aMeshBS;
	//! instance-specific wrinkle map area weights
	float								m_aWrinkleWeights[ blendshapes::MAX_WRINKLE_AREAS ];
	//! instance-specific bs target weights
	float*								m_aTargetWeights;	//!< dma'ble ptr
};

//! blendshapes data intance for a given mesh
class MeshBSSet
{
public:

	MeshBSSet( const BSMeshHeader* pHeader, BSSet* pParent  );
	MeshBSSet() : m_pParent( 0 ), m_pHeader(0) {}

	uint32_t			GetMeshNameHash( void ) const;

	
	// get resolved ptrs to arrays
	const uint16_t*		GetTargetIndices( u_int targetIndex ) const;
	const blendshapes::delta_t*		GetTargetDeltas( u_int targetIndex ) const;
	float				GetTargetDeltaScale( u_int targetIndex ) const;
	uint32_t			GetTargetNumOfDeltas( u_int targetIndex ) const;

	
	//! get resolved header ptr to a target 
	const BSTarget*		GetTargetHeader( u_int targetIndex ) const;

	const void*			GetBSTargetHeadersPtr( void ) const;

	//! through parent
	// TODO_OZZ: re-factor this perhaps? It looks kinda funky to have this kind of access. However,
	// having the ability to query this directly from the (blended) mesh instance is a must and, 
	// while having something like GetParent()->GetXXX() would have the same effect, this approach
	// lets me implement target weights per mesh instance should the need arise without changing the 
	// interface. Note that it's still not possible to set weights through children of a BSSet. This 
	// is intentional as the animation system works on a whole set for the character instead of on 
	// a (game) mesh one. 

	//! get the number of blendshape targets in this bsmesh instance/header
	u_int				GetNumOfBSTargets( void ) const;
	//! get the blenshape target weight by index in the mesh header
	float				GetTargetWeightByIndex( u_int targetIndex ) const;
	//! get blendshape target weight array const ptr
	const float*		GetTargetWeights( void ) const;
	//! get blendshape target weight array ptr
	float*				GetTargetWeights( void );
	//! wrinkle map area weight accessors (still through parent)
	float				GetWrinkleAreaWeight( u_int areaIndex ) const;
	const float*		GetWrinkleAreaWeights( void ) const;
	float*				GetWrinkleAreaWeights( void );

	CEntity*			GetEntity( void ) const;


	friend class BSSet;

private:

	// ptrs/offsets/whatever time-saver. For now, BSMesh is a full ptr but...
	const BSMeshHeader* GetHeader( void ) const;
	
private:	
    BSSet* m_pParent;
	// final info starts here
	//! bs mesh header data
	const BSMeshHeader*			m_pHeader;
};




// ----------------------------------------------------------------- //
//						BSSet INLINED
// ----------------------------------------------------------------- //

inline CEntity* BSSet::GetEntity( void ) const
{
	return m_pEntity;
}

inline uint32_t BSSet::GetClumpNameHash( void ) const
{
	return m_pBSClumpHeader->m_clumpNameHash;
}


inline uint32_t BSSet::GetClumpHierarchyKey( void ) const
{
	return m_pBSClumpHeader->m_uiHierarchyKey;
}

inline uint32_t BSSet::GetNameHash( void ) const
{
	return m_pBSClumpHeader->m_nameHash;
}

inline u_int BSSet::GetNumOfMeshBSSets( void ) const
{
	return m_pBSClumpHeader->m_numOfBSMeshHeaders;
}

inline u_int BSSet::GetNumOfBSTargets( void ) const
{
	return m_pBSClumpHeader->m_numOfBSTargets;
}

inline MeshBSSetPtr_t BSSet::GetMeshBSSetByIndex( u_int index )
{
	user_error_p( index < GetNumOfMeshBSSets(), ("BAD DATA! - %s: MeshBSSet %i not present in BSSet %i\nPlease re-export the bsclump and/or bsanims\n", ntStr::GetString( GetEntity()->GetName() ), index, GetNameHash() ) );
	u_int idx = ntstd::Min( index, GetNumOfMeshBSSets() );
	return &m_aMeshBS[ idx ];
}

inline float BSSet::GetTargetWeightByIndex( u_int targetIndex ) const
{
	user_error_p( targetIndex < GetNumOfBSTargets(), ("BAD DATA! - %s: Target weight %i not present in BSSet %i\nPlease re-export the bsclump and/or bsanims\n", ntStr::GetString( GetEntity()->GetName() ),targetIndex, GetNameHash() ) );
	u_int idx = ntstd::Min( targetIndex, GetNumOfBSTargets() );
	return m_aTargetWeights[ idx ];
}

inline const float*	BSSet::GetTargetWeights() const
{
	return &m_aTargetWeights[0];
}

inline float* BSSet::GetTargetWeights( void )
{
	return &m_aTargetWeights[0];
}

inline void	BSSet::SetTargetWeightByIndex( uint32_t targetIndex, float weight )
{
	user_error_p( targetIndex < GetNumOfBSTargets(), ("BAD DATA! - %s: Target weight %i not present in BSSet %i\nPlease re-export the bsclump and/or bsanims\n", ntStr::GetString( GetEntity()->GetName() ),targetIndex, GetNameHash() ) );
	u_int idx = ntstd::Min( targetIndex, GetNumOfBSTargets() );
	m_aTargetWeights[ idx ] = weight;
}


inline void	BSSet::SetTargetWeights( const float* pWeights )
{
	NT_MEMCPY( m_aTargetWeights, pWeights, GetNumOfBSTargets() * sizeof(float) );
}

inline void BSSet::ResetWeights( void ) 
{
	memset( m_aTargetWeights, 0, GetNumOfBSTargets() * sizeof(float) );
	memset( m_aWrinkleWeights, 0, blendshapes::MAX_WRINKLE_AREAS * sizeof(float) );
}


inline float BSSet::GetWrinkleAreaWeight( u_int areaIndex ) const
{
	ntAssert( areaIndex < blendshapes::MAX_WRINKLE_AREAS );
	return m_aWrinkleWeights[ areaIndex ];
}

inline const float* BSSet::GetWrinkleAreaWeights( void ) const
{
	return m_aWrinkleWeights;
}

inline float* BSSet::GetWrinkleAreaWeights( void )
{
	return m_aWrinkleWeights;
}

inline void	BSSet::SetWrinkleAreaWeight( u_int areaIndex, float weight )
{
	ntAssert( areaIndex < blendshapes::MAX_WRINKLE_AREAS );
	m_aWrinkleWeights[ areaIndex ] = weight;
}



// ----------------------------------------------------------------- //
//						MeshBSSet INLINED
// ----------------------------------------------------------------- //

inline const BSMeshHeader* MeshBSSet::GetHeader( void ) const 
{
	return m_pHeader;
}

inline const BSTarget* MeshBSSet::GetTargetHeader( u_int targetIndex ) const
{
	ntAssert( targetIndex < GetNumOfBSTargets() );

	return GetHeader()->m_pBSTargets + targetIndex;
}


inline const void* MeshBSSet::GetBSTargetHeadersPtr( void ) const
{
	return (const void*)(uintptr_t)( m_pHeader->m_pBSTargets );
}


inline uint32_t MeshBSSet::GetMeshNameHash( void ) const
{
	return GetHeader()->m_meshNameHash;
}


inline float MeshBSSet::GetTargetDeltaScale( u_int targetIndex ) const 
{
	const BSTarget* pTarget = GetTargetHeader( targetIndex );
	return pTarget->m_deltaScale;

}

inline const uint16_t*	MeshBSSet::GetTargetIndices( u_int targetIndex ) const
{
	const BSTarget* pTarget = GetTargetHeader( targetIndex );
	return  pTarget->m_pIndices;

}

inline 	const blendshapes::delta_t* MeshBSSet::GetTargetDeltas ( u_int targetIndex ) const
{
	const BSTarget* pTarget = GetTargetHeader( targetIndex );
	return  pTarget->m_pDeltas;
}


inline uint32_t MeshBSSet::GetTargetNumOfDeltas ( u_int targetIndex ) const
{
	const BSTarget* pTarget = GetTargetHeader( targetIndex );
	return pTarget->m_numOfDeltas;

}

inline u_int MeshBSSet::GetNumOfBSTargets( void ) const
{
	// this is a bit paranoid
	ntAssert( m_pParent->GetNumOfBSTargets() == GetHeader()->m_numOfBSTargets );

	return GetHeader()->m_numOfBSTargets;
}

inline float MeshBSSet::GetTargetWeightByIndex( u_int targetIndex ) const
{
	return m_pParent->GetTargetWeightByIndex( targetIndex );
}

inline const float* MeshBSSet::GetTargetWeights( void ) const
{
	return m_pParent->GetTargetWeights();
}

inline float* MeshBSSet::GetTargetWeights( void )
{
	return m_pParent->GetTargetWeights();
}

inline float MeshBSSet::GetWrinkleAreaWeight( u_int areaIndex ) const
{
	return m_pParent->GetWrinkleAreaWeight( areaIndex );
}

inline const float* MeshBSSet::GetWrinkleAreaWeights( void ) const
{
	return m_pParent->GetWrinkleAreaWeights();
}

inline float* MeshBSSet::GetWrinkleAreaWeights( void )
{
	return m_pParent->GetWrinkleAreaWeights();
}

inline CEntity* MeshBSSet::GetEntity( void ) const
{
	return m_pParent->GetEntity();
}


#endif // end of _BLENDSHAPES_H_

