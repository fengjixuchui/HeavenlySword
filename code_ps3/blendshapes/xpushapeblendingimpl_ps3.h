//--------------------------------------------------
//!
//!	\file xpushapeblendingimpl_ps3.h
//!
//--------------------------------------------------

#ifndef SHAPE_BLENDING_IMPL_PS3_H
#define SHAPE_BLENDING_IMPL_PS3_H


//#include "blendshapes/blendshapes_constants.h"

class BlendedMeshInstance;
class SPUProgram;


class XPUShapeBlendingImpl : public Singleton< XPUShapeBlendingImpl >
{
public:

	XPUShapeBlendingImpl( uint32_t vertsPerBatch = 1024 );
	~XPUShapeBlendingImpl();

	//! register a blended mesh so it can be updated
	void Register( BlendedMeshInstance* pMesh );
	void Unregister( BlendedMeshInstance* pMesh );
	bool IsRegistered( BlendedMeshInstance* pMesh ) const;

	//! begin update (since it's not immediate) of all registered meshes
	void BeginUpdate();

	//! process all queued meshes 
	void Flush();	
	//! empty blend queue without processing
	void DiscardQueuedBatches() {/* empty body */}

	//! enabling/disabling blenders
	void Enable();
	void Disable( bool bDiscardCurrentBatches = false );
	void Reset( bool bDeleteRegisteredInstances = false );

	//! is shapeblending enabled?
	bool IsEnabled() const;


	void SetBlendWeightThreshold( float threshold );
	float GetBlendWeightThreshold( void );

private:

	void BatchAndSendToSPU( BlendedMeshInstance* pMesh );
	void BatchAndSendToSPU_NEW( BlendedMeshInstance* pMesh );

private:
	typedef ntstd::List< BlendedMeshInstance*, Mem::MC_PROCEDURAL >	BSMeshRegistry_t;

	//! the meshes that will get updated. Must provide a mechanism to unregister...
	BSMeshRegistry_t	m_meshRegistry;
	//! our little elf
	const SPUProgram*	m_pSpuBlender;
	//! enabled?
	bool				m_bEnabled;

	//! num of vertices to process per batch
	u_int				m_vertsPerBatch;
	
	//! blend weight threshold
	float				m_blendWeightThreshold;
	//! max blend distance(from camera)
	float				m_maxBlendDistance;

	int*				m_pSpuTaskCounter; 
};


inline bool XPUShapeBlendingImpl::IsEnabled( void )  const
{
	return m_bEnabled;
}

inline void XPUShapeBlendingImpl::Enable( void )
{
	m_bEnabled = true;
}

inline void XPUShapeBlendingImpl::Disable( bool bDiscardCurrentBatches )
{
	if ( bDiscardCurrentBatches )
		DiscardQueuedBatches();

	m_bEnabled = false;
}

inline void XPUShapeBlendingImpl::SetBlendWeightThreshold( float threshold )
{
	m_blendWeightThreshold = threshold;
}

inline float XPUShapeBlendingImpl::GetBlendWeightThreshold( void )
{
	return m_blendWeightThreshold;
}

#endif // SHAPE_BLENDING_IMPL_PS3_H
