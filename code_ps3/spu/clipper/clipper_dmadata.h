//---------------------------------------------------------------
//!
//! \file clipper_spu\clipper_dmadata.h
//!  Data structures for dma transfer clipper data to/from SPU
//!
//---------------------------------------------------------------

#ifndef CLIPPER_DMADATA_H
#define CLIPPER_DMADATA_H

#define CLIPPER_DMA_FROM_SPU

#define NUM_FRUSTUMS 8

namespace ClipperData
{
	//typedef uint32_t PPU_Ptr;

	typedef uint32_t RenderableID;

	const unsigned int s_renderablesPerTask = 32;
	const unsigned int INPUT_SLOT = 0;
	const unsigned int OUTPUT_SLOT = 1;
    const unsigned int NUM_SHADOW_PLANES = 4;
    const unsigned int NUM_CASTER_AABB  = NUM_SHADOW_PLANES - 1;
    const unsigned int NUM_FRUSTUM_AABB = NUM_SHADOW_PLANES - 1;
    const unsigned int NUM_AABBS        = NUM_CASTER_AABB + NUM_FRUSTUM_AABB;

    enum RenderableInputFlags
    {
        rifNoFlags,
        rifCastShadow,
        rifReceiveShadow,
		rifRendering
    };

	struct RenderableIn
	{
#ifndef CLIPPER_DMA_FROM_SPU
		CAABB			m_AABB;
		uint16_t		m_renderableID;
#else
		uint32_t		m_renderableID;		// it is actually a ponter to AABB member of a renderable
		uint32_t		m_transformAddr;
#endif
		
        unsigned short  m_flags;
	};

	struct VisibleOut
	{
#ifndef CLIPPER_DMA_FROM_SPU
		uint16_t		m_renderableID;
#else
		uint32_t		m_renderableID;		// it is actually a ponter to AABB member of a renderable
#endif
        unsigned short  m_flags;
	};

	struct ClipperDataOut
	{
		CAABB			m_frustums[NUM_AABBS];
		uint32_t		m_sizeVisible;
		uint32_t		m_sizeShadowcaster;

	};

	/*ALIGNTO_PREFIX(16)*/ struct ClipperDataIn
	{
		CMatrix		m_worldToView;
		CMatrix		m_viewToScreen;
		CMatrix		m_worldToScreen;
        CDirection  m_shadowDirection;
		float		m_ZNear;
		float		m_ZFar;
		float		m_FOV;
		float		m_aspectRatio;
		float		m_shadowPercents[8];
		uint32_t	m_numRenderables;
        uint32_t    m_shadowQuality;
	} /*ALIGNTO_POSTFIX(16)*/;

	struct DMA_In
	{
		ClipperDataIn	m_controlData;
		RenderableIn	m_renderables[s_renderablesPerTask];
	};

	struct DMA_Out
	{
		ClipperDataOut	m_header;
		VisibleOut		m_visibles[s_renderablesPerTask];
        VisibleOut      m_shadowCasters[s_renderablesPerTask];

	};

}

#endif
