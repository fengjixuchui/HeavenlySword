//--------------------------------------------------
//!
//!	\file blendedmeshinstance_ps3.h
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------


#ifndef _BlendedMeshInstance_PS3_H_
#define _BlendedMeshInstance_PS3_H_

#include "blendshapes/BlendShapes.h"

#include "gfx/meshinstance.h"
#include "core/rotationnalindex.h"


class XPUShapeBlendingImpl;
struct BSSpuAdditionalInfo;

class BlendedMeshInstance : public CMeshInstance
{
public:
	BlendedMeshInstance(	Transform const* pobTransform, CMeshHeader const* pobMeshHeader,
							bool bRenderOpaque, bool bShadowRecieve, bool bShadowCast );

	virtual ~BlendedMeshInstance( void );

	//! overriden from CMeshInstance
	virtual void	ForceMaterial( CMaterial const* pobMaterial );

	bool			HasBlendShapes( void ) const { return m_pBlendShapes ? true : false; }
	//! get current blendshape instance. May return null
	MeshBSSetPtr_t	GetBlendShapes( void ) { return m_pBlendShapes; }
	//! sets current blendshapes ptr and returns previous one. It does not free the prev ptr. 
	MeshBSSetPtr_t	SetBlendShapes( MeshBSSetPtr_t pMeshBlendShapes );
	//! \returns removed blendshapes ptr. It does NOT delete the resource itself. It's up the user to do this
	MeshBSSetPtr_t	RemoveBlendShapes( void );
	//! \returns true if blendshapes data is compatible with this particular mesh
	bool			IsCompatible( MeshBSSetPtr_t pBlendShapes );


//! overriden render method from CMeshInstance
public: 
	virtual void ReleaseAreaResources( void );
	virtual void CreateAreaResources( void );
protected:
	
	virtual void RenderMesh() const;
	virtual void GetVertexAndIndexBufferHandles( void );

private:
	
	//! copies the original mesh data into the vbuffer. Also asks for scratch vram if necessary
	void			ResetVertexBuffer( void );
	//! \returns the vbuffer write address for the current frame
	void*			GetVertexBufferWriteAddress( void ); 
	//! finds the wrinkle weights material properties if present
	void			CacheWrinkleWeightsPropertiesPtrs( void ) const;
	//! updates the corresponding material properties in the mesh header with the current animated result from its blendshapes
	void			PatchWrinkleWeights( void ) const;


	void			CacheVertexStreamReconstructionMatrices( void );
	void			BuildVertexBuffers( void );

	//! some helper functions for vram init and cleaning 
	void* InitUserVram( size_t bufferSize );
	void FreeUserVram( void );

	// compressed streams are currently ignored. 
	bool NeedsUpdating( void ) const { return IsRendering() && HasBlendShapes() && m_bHasAreaResources; } 
	
	BSSpuAdditionalInfo* GetSpuAdditionalInfo( void );


	//! make this guy our friend so it can play around with the vbuffer
	friend class XPUShapeBlendingImpl;

private:
	//! the blendshape data. can be invalid at any given time if removed
	MeshBSSetPtr_t m_pBlendShapes;
	//! wrinkleweigh material property cached ptrs so we don't have to loop through all of them each frame
	mutable CMaterialProperty* m_apWrinkleWeightsPropertiesPtrs[2];
	//! number of work buffers
	static const int m_cNumOfBuffers = 2;
	//! the work buffer index
	mutable RotationnalIndex< int, m_cNumOfBuffers > m_bufferIndex;
	//! the buffers vram ptrs 
	void* m_aVramPtrs[ m_cNumOfBuffers ];

	mutable CMatrix	m_obStreamMatrices[2];	//!< reconstruction and inverse

	bool m_bHasAreaResources;
	BSSpuAdditionalInfo*	m_pSpuAdditionalInfo;
};




#endif // end of _BlendedMeshInstance_PS3_H_


//eof
