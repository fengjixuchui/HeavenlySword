//--------------------------------------------------
//!
//!	\file surface.h
//!	The main surface class. This only exists on PC
//!
//--------------------------------------------------

#ifndef GFX_SURFACE_H
#define GFX_SURFACE_H

#ifdef PLATFORM_PC

#if !defined( FW_STD_INTRUSIVE_PTR_H )
#include <Fw/FwStd/FwStdIntrusivePtr.h>
#endif

#if !defined( GFX_GFXFORMAT_H )
#include "gfx/gfxformat.h"
#endif

#if !defined(GFX_SURFACE_PC_H)
#include "gfx/surface_pc.h"
#endif

class Surface : public FwStd::IntrusivePtrCountedBase< Surface >
{
public:
	friend class SurfaceManager;
	friend class SurfaceManagerPlatform;

	typedef SurfacePlatform::CreationStruct CreationStruct; 
	typedef	FwStd::IntrusivePtr<Surface>	Ptr;

	enum NONE_ENUM{ NONE };

	//! width accessor
	uint32_t GetWidth() const
	{ 
		return m_iWidth; 
	}
	//! height accessor
	uint32_t GetHeight() const
	{ 
		return m_iHeight; 
	}

	//! returns a generic format you may get a platform specific marker
	GFXFORMAT GetFormat() const;

	//! CPU Lock, allow access to the bits.
	void* CPULock( uint32_t& pitch );

	//! CPU unlock, tell the system you finished fiddling or reading
	void CPUUnlock();

	//! platform dependent stuff
	SurfacePlatform m_Platform;

	//! dtor
	~Surface();

	//! debug method to help estimate our VRAM usage
	inline uint32_t CalculateVRAMFootprint()
	{
		return GFXFormat::CalculateVRAMFootprint( GetFormat(), m_iWidth, m_iHeight );
	}

private:
	//! cross platform ctor (not optimised for the platform particular)
	Surface( uint32_t iWidth, uint32_t iHeight, GFXFORMAT format );

	//! Platform specific creation method
	Surface( const CreationStruct& creationStruct );

	Surface() {};

	uint32_t m_iWidth;		//!< width of the top level
	uint32_t m_iHeight;		//!< height of the top level

};

#endif // PLATFORM_PC

#endif // end GFX_SURFACE_H
