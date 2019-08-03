//--------------------------------------------------
//!
//!	\file vertexdecleration_ps3.h
//! 
//!
//--------------------------------------------------
#if !defined( GFX_VERTEXDECLERATION_PS3_H )
#define GFX_VERTEXDECLERATION_PS3_H

#include	<Gc/GcStreamBuffer.h>

// FIXME_WIL
struct fakeThing
{
	void AddRef() {}
	void Release() {}
};

typedef fakeThing	ID3DXEffect;
typedef fakeThing	ID3DXEffectPool;

//--------------------------------------------------
//!
//!	convert VERTEX_DECL_STREAM_TYPE to StreamType
//!
//--------------------------------------------------
Gc::StreamType GetStreamType( VERTEX_DECL_STREAM_TYPE ntType );

//--------------------------------------------------
//!
//!	convert whats the count of fields used in this type
//!
//--------------------------------------------------
int GetStreamElements( VERTEX_DECL_STREAM_TYPE ntType );

//--------------------------------------------------
//!
//!	convert VERTEX_DECL_STREAM_TYPE to a byte size
//!
//--------------------------------------------------
size_t GetSizeOfElement( VERTEX_DECL_STREAM_TYPE ntType );

//--------------------------------------------------
//!
//! is VERTEX_DECL_STREAM_TYPE normalised?
//!
//--------------------------------------------------
bool IsTypeNormalised( VERTEX_DECL_STREAM_TYPE ntType );

#include "core/semantics.h"

// this probably doesn't belong here but we need to convert from the exported
// STREAM_SEMANTIC_TYPE to a hashed name that Gc uses, this does it 
FwHashedString GetSemanticName( STREAM_SEMANTIC_TYPE ntType );

/***************************************************************************************************
*
*	CLASS			VBHandle
*
*	DESCRIPTION		A handle round GcStreamBufferHandle like a com pointer, allows us to track memory
*
***************************************************************************************************/
class VBHandle
{
public:
	friend class RendererPlatform;

	VBHandle(){}
#ifdef TRACK_GFX_MEM
	~VBHandle() { Release(); }
#endif

	VBHandle(const VBHandle& other) :
		m_hResource(other.m_hResource)
#ifdef TRACK_GFX_MEM
		,m_iAllocSize(other.m_iAllocSize),
		m_iNumVerts(other.m_iNumVerts)
#endif
	{}

	VBHandle& operator=(const VBHandle& other)
	{
#ifdef TRACK_GFX_MEM
		Release();
		m_iAllocSize = other.m_iAllocSize;
		m_iNumVerts = other.m_iNumVerts;
#endif
		m_hResource = other.m_hResource;
		return *this;
	}

	const GcStreamBufferHandle&	GetHandle()	const { return m_hResource; }
	
	GcStreamBuffer*	Get()			const { return m_hResource.Get(); }
	GcStreamBuffer*	operator->()	const { return m_hResource.Get(); }

	operator bool() const { return m_hResource.IsValid(); }

private:

	// only RendererPlatform can create us
#ifdef TRACK_GFX_MEM
	VBHandle(GcStreamBufferHandle hResource, uint32_t vertexCount, uint32_t vramStart );
#else
	VBHandle(GcStreamBufferHandle hResource) : m_hResource(hResource) {};
#endif

	GcStreamBufferHandle m_hResource;

#ifdef TRACK_GFX_MEM
	void Release();
	uint32_t m_iAllocSize;
	uint32_t m_iNumVerts;
#endif
};

/***************************************************************************************************
*
*	CLASS			IBHandle
*
*	DESCRIPTION		A handle round GcStreamBufferHandle like a com pointer, allows us to track memory
*
***************************************************************************************************/
class IBHandle
{
public:
	friend class RendererPlatform;

	IBHandle(){}
#ifdef TRACK_GFX_MEM
	~IBHandle() { Release(); }
#endif

	IBHandle(const IBHandle& other) :
		m_hResource(other.m_hResource)
#ifdef TRACK_GFX_MEM
		,m_iAllocSize(other.m_iAllocSize)
#endif
	{}

	IBHandle& operator=(const IBHandle& other)
	{
#ifdef TRACK_GFX_MEM
		Release();
		m_iAllocSize = other.m_iAllocSize;
#endif
		m_hResource = other.m_hResource;
		return *this;
	}

	const GcStreamBufferHandle&	GetHandle()	const { return m_hResource; }
	
	GcStreamBuffer*	Get()			const { return m_hResource.Get(); }
	GcStreamBuffer*	operator->()	const { return m_hResource.Get(); }

	operator bool() const { return m_hResource.IsValid(); }

private:

	// only RendererPlatform can create us
#ifdef TRACK_GFX_MEM
	IBHandle(GcStreamBufferHandle hResource, uint32_t vramStart);
#else
	IBHandle(GcStreamBufferHandle hResource) : m_hResource(hResource) {};
#endif

	GcStreamBufferHandle m_hResource;

#ifdef TRACK_GFX_MEM
	void Release();
	uint32_t m_iAllocSize;
#endif
};

#endif // GFX_VERTEXDECLERATION_PS3_H
