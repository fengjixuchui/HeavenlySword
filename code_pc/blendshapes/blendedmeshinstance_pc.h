//--------------------------------------------------
//!
//!	\file blendshapes/blendedmeshinstance.h
//!
//--------------------------------------------------

#ifndef BLENDED_MESH_INSTANCE_PC
#define BLENDED_MESH_INSTANCE_PC

//#include "blendshapes_target_ids.h"
#include "blendshapes/BlendShapes.h"
//#include "anim/BSAnimator.h"
//#include "blendshapes/BSVertexBuffer.h"

#include "gfx/meshinstance.h"
//#include "core/smartptr.h"


//#include "BSVBuffer.h"


class BlendedMeshInstance : public CMeshInstance
{
public:
	BlendedMeshInstance(	Transform const* pobTransform, 
							CMeshHeader const* pobMeshHeader, 
							bool bRenderOpaque, 
							bool bShadowRecieve, 
							bool bShadowCast );

	virtual ~BlendedMeshInstance( void );

	VBHandle		GetVBHandle( void ) { return m_hVertexBuffer[0]; }

	bool			HasBlendShapes( void ) const { return m_pBlendShapes ? true : false; }
	//! get current blendshape instance. May return null
	MeshBSSetPtr_t	GetBlendShapes( void ) { return m_pBlendShapes; }
	//! sets current blendshapes ptr and returns previous one. It does not free the prev ptr. 
	MeshBSSetPtr_t	SetBlendShapes( MeshBSSetPtr_t pMeshBlendShapes );
	//! \return removed blendshapes ptr. It does NOT delete the resource itself. It's up the user to do this
	MeshBSSetPtr_t	RemoveBlendShapes( void );


private:

	MeshBSSetPtr_t m_pBlendShapes;
};



#endif // BLENDED_MESH_INSTANCE_PC
