//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		Top-level Graphics Core Header.
	
	Contains minimal set of necessary Gc include files.

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef GC_H
#define GC_H

//--------------------------------------------------------------------------------------------------
//	INCLUDES
//--------------------------------------------------------------------------------------------------

#ifdef _DEBUG
#define ICEDEBUG 1
#else
#define ICEDEBUG 0
#endif

#ifdef GC_TOOLS
#include <icelib/icerender/icerender.h>
#else
#include <icetypes.h>
#include <icebase.h>
#include <render/icerender.h>
#endif

#include <Fw/FwStd/FwStdIntrusivePtr.h>
#include <Gc/GcConstants.h>

//--------------------------------------------------------------------------------------------------
//  FORWARD DECLARATIONS
//--------------------------------------------------------------------------------------------------

class GcInitParams;
class GcRenderBuffer;
class GcResource;
class GcShader;
class GcStreamBuffer;
class GcTexture;

typedef FwStd::IntrusivePtr< GcRenderBuffer >	GcRenderBufferHandle;
typedef FwStd::IntrusivePtr< GcResource >		GcResourceHandle;
typedef FwStd::IntrusivePtr< GcShader >			GcShaderHandle;
typedef FwStd::IntrusivePtr< GcStreamBuffer >	GcStreamBufferHandle;
typedef FwStd::IntrusivePtr< GcTexture >		GcTextureHandle;

//--------------------------------------------------------------------------------------------------

#endif // GC_H
