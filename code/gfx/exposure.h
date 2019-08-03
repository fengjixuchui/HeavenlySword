//--------------------------------------------------
//!
//!	\file exposure.h
//!	Evalute a HDR buffer to find its log luminance
//!
//--------------------------------------------------

#ifndef GFX_EXPOSURE_H
#define GFX_EXPOSURE_H

#ifndef GFX_RENDERTARGET_H
#include "gfx/rendertarget.h"
#endif

class EXPControllerCPU_impl;
class EXPControllerGPU_impl;
class LevelsGraph_impl;

namespace EXPController
{
	GFXFORMAT GetInternalFormat();
};

//--------------------------------------------------
//!
//!	EXPControllerCPU
//! Public interface to our CPU based exposure
//!
//--------------------------------------------------
class EXPControllerCPU
{
public:
	EXPControllerCPU();
	~EXPControllerCPU();

	void SampleCurrBackBuffer( RenderTarget::Ptr& pBackBuffer );
	
private:
	EXPControllerCPU_impl*	m_pImpl;
};

//--------------------------------------------------
//!
//!	EXPControllerGPU
//! Public interface to our GPU based exposure
//!
//--------------------------------------------------
class EXPControllerGPU
{
public:
	EXPControllerGPU();
	~EXPControllerGPU();

	void SampleCurrBackBuffer( RenderTarget::Ptr& pBackBuffer );
	
private:
	EXPControllerGPU_impl*	m_pImpl;
};

//--------------------------------------------------
//!
//!	LevelsGraph
//! Public interface to our levels graph
//!
//--------------------------------------------------
#ifdef PLATFORM_PC

class LevelsGraph
{
public:
	LevelsGraph();
	~LevelsGraph();

	void Update( int iNumLevels, const Surface::Ptr& pRenderTarget );
	void RenderGraph( const CPoint& topLeft, const CPoint& bottomRight );

private:
	LevelsGraph_impl* m_pImpl;
};

#endif // PLATFORM_PC

#endif // ndef GFX_EXPOSURE_H
