//--------------------------------------------------
//!
//!	\file blendshapes/xpushapeblending.h
//! Shape blending interface class
//!
//--------------------------------------------------

#ifndef XPU_SHAPE_BLENDING_H
#define XPU_SHAPE_BLENDING_H


#if defined( PLATFORM_PC )
#	include "blendshapes/xpushapeblendingimpl_pc.h"
#elif defined( PLATFORM_PS3 )
#	include "blendshapes/xpushapeblendingimpl_ps3.h"
#else
#	error unknown_platform
#endif


class BlendedMeshInstance;

//--------------------------------------------------
//!
//! Blendshapes batcher and manager
//! just a trivial pimpl interface to the platform-specific
//! blending manager(CPU or SPU)
//!
//--------------------------------------------------
class XPUShapeBlending : public Singleton< XPUShapeBlending >
{
public:

	XPUShapeBlending( uint32_t vertsPerBatch = 0 );
	~XPUShapeBlending();

	//! register/unregister a blended mesh so it can be updated
	void Register( BlendedMeshInstance* pMesh );
	void Unregister( BlendedMeshInstance* pMesh );

	//! begin update (since it's not immediate) of all registered meshes
	void BeginUpdate();

	//! process all queued batches
	void Flush();
	//! empty blend queue without processing
	void DiscardQueuedBatches();

	//! enabling/disabling blenders
	void Enable();
	void Disable( bool bDiscardCurrentBatches = false );

	//! is shapeblending enabled?
	bool IsEnabled() const;

	void Reset();
};

inline
void XPUShapeBlending::Register( BlendedMeshInstance* pMesh )
{
	XPUShapeBlendingImpl::Get().Register( pMesh );
}

inline
void XPUShapeBlending::Unregister( BlendedMeshInstance* pMesh )
{
	XPUShapeBlendingImpl::Get().Unregister( pMesh );
}

inline
void XPUShapeBlending::BeginUpdate()
{
	XPUShapeBlendingImpl::Get().BeginUpdate();
}

inline 
void XPUShapeBlending::Flush()
{
	XPUShapeBlendingImpl::Get().Flush();
}

inline
void XPUShapeBlending::DiscardQueuedBatches()
{
	XPUShapeBlendingImpl::Get().DiscardQueuedBatches();
}

inline 
void XPUShapeBlending::Enable()
{
	XPUShapeBlendingImpl::Get().Enable();
}

inline
void XPUShapeBlending::Disable( bool bDiscardCurrentBatches )
{
	XPUShapeBlendingImpl::Get().Disable( bDiscardCurrentBatches );
}

inline
bool XPUShapeBlending::IsEnabled() const
{
	return XPUShapeBlendingImpl::Get().IsEnabled();
}

inline
void  XPUShapeBlending::Reset()
{
	XPUShapeBlendingImpl::Get().Reset();
}

inline
XPUShapeBlending::XPUShapeBlending( uint32_t vertsPerBatch )
{
	//! implementation should never be instantiated before interface!
	ntAssert( !XPUShapeBlendingImpl::Exists() );

	NT_NEW_CHUNK (Mem::MC_PROCEDURAL) XPUShapeBlendingImpl( vertsPerBatch );
}

inline
XPUShapeBlending::~XPUShapeBlending()
{
	if ( XPUShapeBlendingImpl::Exists() )
		XPUShapeBlendingImpl::Kill();
}


#endif //XPU_SHAPE_BLENDING_H
