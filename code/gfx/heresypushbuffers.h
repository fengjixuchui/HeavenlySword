#ifndef HERESYPUSHBUFFERS_H
#define HERESYPUSHBUFFERS_H

class CHeresyPushBuffers
{
public:
	CHeresyPushBuffers(CMeshInstance* meshInstance, bool registerInMap = true);
	~CHeresyPushBuffers();

	static uint32_t ComputePBTag( const CMeshHeader* pMeshHeader, bool bIsReceivingShadows ) 
	{ 
		uint32_t tag = ((uint32_t)( const_cast<CMeshHeader*>(pMeshHeader) )) & ~0x3;
		tag |= static_cast<uint32_t>( bIsReceivingShadows );
		return tag;
	}

	static CMeshHeader* RetrieveMeshHeaderFromTag( uint32_t tag ) 
	{
		return (CMeshHeader*)( tag & ~0x3 );
	}

	friend void IntrusivePtrAddRef(CHeresyPushBuffers* obj)
	{
		ntAssert(obj -> m_refCount < 200000);
		++ obj -> m_refCount;
	}

	friend void IntrusivePtrRelease(CHeresyPushBuffers* obj)
	{
		ntAssert(obj -> m_refCount != 0);
		if ( -- obj -> m_refCount == 0)
		{
			NT_DELETE_CHUNK(Mem::MC_GFX, obj);
		}
	}

	uint32_t			m_ui32PBTag;
	bool				m_registerInMap;
	unsigned int		m_refCount;
	CMeshHeader const*	m_pMeshHeader;

	Heresy_PushBuffer* m_pDepthPushBufferHeader;
	Heresy_PushBuffer* m_pRenderPushBufferHeader;
	Heresy_PushBuffer* m_pShadowMapPushBufferHeader;

};


#endif
