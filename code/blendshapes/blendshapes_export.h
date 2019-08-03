//--------------------------------------------------
//!
//!	\file blendshapes/blendshapes_export.h
//!
//--------------------------------------------------

#ifndef BLEND_SHAPE_EXPORT_PS3_H
#define BLEND_SHAPE_EXPORT_PS3_H


#ifdef _FINALISER 
	typedef u32 uint32_t;
	typedef u16 uint16_t;
	typedef s8	int8_t;
	typedef u8	uint8_t;
	typedef s16	int16_t;	
	//typedef	FwQuat CQuat;
#endif //_FINALISER


//------------------------------------------------------
//!
//! delta_t will be used to pack float displacements
//!	into this type. Signed only please.
//! NOTE: this *MUST* be reflected in the Finaliser
//! 
//------------------------------------------------------
namespace blendshapes
{
#ifdef BS_DO_NOT_PACK_DELTAS
	typedef float delta_t;
#else
	typedef int8_t delta_t;
#endif //BS_DO_NOT_PACK_DELTAS
}




//--------------------------------------------------
//!
//!	BlendShape Target info
//!	Contains the target vertex displacement infor for a given mesh.
//! \NOTE: This struct is DMAble so it MUST have the same size/alignment on PPU/SPU
//!
//--------------------------------------------------
ALIGNTO_PREFIX(16)
struct BSTarget
{
	uint16_t*						m_pIndices;					// delta indices	[m_numOfDeltas]
	blendshapes::delta_t*			m_pDeltas;					// delta array		[m_numOfDeltas*3]
	uint32_t						m_numOfDeltas;				// number of deltas ( not components! )
	float							m_deltaScale;				// delta scale after unpack

	BSTarget( void )
		: m_pIndices( 0 ),
		m_pDeltas( 0 ),
		m_numOfDeltas( 0 ),
		m_deltaScale( 1.0f )
	{
		// nothing
	}
}
ALIGNTO_POSTFIX(16); 


//--------------------------------------------------
//!
//!	Blendshape data for a single mesh
//!	Note the the alignment is just to ensure the bstargets
//! ptr is aligned and therefore, dma'ble. 
//!
//--------------------------------------------------
ALIGNTO_PREFIX(16)
struct BSMeshHeader
{
	BSTarget*							m_pBSTargets;					//! [ m_numOfBSTargets ] 
	uint32_t							m_meshNameHash;					//! associated mesh hash
	uint32_t							m_numOfBSTargets;			

	BSMeshHeader( void ) 
		: m_pBSTargets( 0 ),
		m_meshNameHash( 0 ),
		m_numOfBSTargets( 0 )
		
	{
		// nope
	}
}
ALIGNTO_POSTFIX(16);



struct BSClumpHeader
{
	BSMeshHeader*					m_pBSMeshHeaders;				//!< bsmesh headers
	uint32_t						m_nameHash;						//!< name hash of this blendshape header/set
	uint32_t						m_clumpNameHash;				//!< associated clump name hash
	uint32_t						m_uiHierarchyKey;				//!< associated hierarchy key
	
	uint32_t						m_numOfBSMeshHeaders;			//!< number of bsmesh headers
	uint32_t						m_numOfBSTargets;

	BSClumpHeader( void )
		: m_pBSMeshHeaders( 0 ),
		m_nameHash( 0 ),
		m_clumpNameHash( 0 ),
		m_uiHierarchyKey( 0 ),
		m_numOfBSMeshHeaders( 0 ),
		m_numOfBSTargets( 0 )
	{
		// nothing to see here. Move along!
	}
};



#endif //BLEND_SHAPE_EXPORT_PS3_H
