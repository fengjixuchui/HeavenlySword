//--------------------------------------------------
//!
//!	\file listspace.h
//!	A simple space that internally just holds unsorted lists.
//!
//--------------------------------------------------

#ifndef GFX_LISTSPACE_H
#define GFX_LISTSPACE_H

#include "gfx/rendercontext.h"

// forward declarations
class CRenderable;
class CCamera;

struct IFrustumClipper;
struct IFrameFlagsUpdateCallback;

//--------------------------------------------------
//!
//!	A simple implementation of space, with no internal spatial structure.
//!
//--------------------------------------------------
class ListSpace : CNonCopyable
{
public:

	typedef ntstd::Vector<CRenderable*, Mem::MC_GFX> RenderableListType;
    typedef ntstd::Vector<IFrameFlagsUpdateCallback*, Mem::MC_GFX> CallbackListType;

	ListSpace();

	//! Destructs an empty space.
	~ListSpace();
	
	//! Adds a renderable to the space.
	void AddRenderable( CRenderable* pobRenderable );

	//! Removes a renderable from the space.
	void RemoveRenderable( CRenderable* pobRenderable );

	//! Builds the visible renderable list using the given frustum. if the pShadowPercents array is non-null computes shadow visibility as well
	void SetVisibleFrustum( const float* pShadowPercents = 0 ) const;

	//!! Updates this render context visbible opaque and alpha list
	void UpdateVisibleOpaqueAndAlphaLists() const;

    RenderableListType const& GetRenderableList() const;

private:

    void SetVisibleFrustumImpl( const float* pShadowPercents = 0 ) const;
	void SetVisibleFrustumImplPPU( const float* pShadowPercents = 0 ) const;

	void ListSpace::BatchRenderables(RenderingContext::CONTEXT_DATA::RenderableVector &OldRenderables,
									 RenderingContext::CONTEXT_DATA::RenderableVector &NewRenderables, 
									 RenderingContext::CONTEXT_DATA::BatchableVector  &BatchableRenderables,
									 RenderingContext::CONTEXT_DATA::RenderableVector &BatchedRenderables ) const;

    RenderableListType          m_obRenderables;

    CallbackListType            m_obCallbackList;

	IFrustumClipper*			m_clipperImpl;      // Instance of a platform-specific clipper

	static int QsortRenderableComparator( const void* a, const void* b );
	static int QsortBatchRenderableComparator( const void* a, const void* b );
};

#endif // ndef GFX_LISTSPACE_H
