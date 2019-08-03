/***************************************************************************************************
*
*	DESCRIPTION
*
*	NOTES
*
***************************************************************************************************/

#include "gfx/rendercontext.h"
#include "gfx/renderer.h"
#include "core/timer.h"

#define RENDERER_MAX_CONTEXTS		1

void RenderingContext::Initialise()
{
	ms_astContexts.Reset( NT_NEW_ARRAY_CHUNK( Mem::MC_GFX ) CONTEXT_DATA[RENDERER_MAX_CONTEXTS] );
}

void RenderingContext::Destroy()
{
	ms_astContexts.Reset();
}

void RenderingContext::PushContext()
{
	++ms_iCurrentContext;
	ntAssert( ms_iCurrentContext < RENDERER_MAX_CONTEXTS );
	ms_pstCurrentContext = &ms_astContexts[ms_iCurrentContext];

	ms_pstCurrentContext->m_pCullCamera = 0;
	ms_pstCurrentContext->m_pViewCamera = 0;

	ms_uiRenderContextTick++;
}

void RenderingContext::PopContext()
{
	ntAssert( ms_iCurrentContext >= 0 );
	--ms_iCurrentContext;
	ms_pstCurrentContext = ( ms_iCurrentContext >= 0 ) ? &ms_astContexts[ms_iCurrentContext] : 0;
}

RenderingContext::CONTEXT_DATA* RenderingContext::ms_pstCurrentContext = 0;

int RenderingContext::ms_iCurrentContext = -1;
unsigned int RenderingContext::ms_uiRenderContextTick = 0;
Heresy_GlobalDataT* RenderingContext::ms_pHeresyGlobal = 0;
CScopedArray<RenderingContext::CONTEXT_DATA> RenderingContext::ms_astContexts;
