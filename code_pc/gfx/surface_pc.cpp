//------------------------------------------------------------
//!
//! \file gfx\surface_pc.cpp
//! PC implementation of the surface
//!
//------------------------------------------------------------

#include "gfx/surface.h"
#include "gfx/texturemanager.h"
#include "gfx/dxerror_pc.h"

//--------------------------------------------------
//!
//! destroy, free any allocated memory
//!
//--------------------------------------------------
Surface::~Surface()
{
	if (!m_Platform.m_bImplicit)
		m_Platform.m_pSurface->Release();
}

//! Get neutral format
GFXFORMAT Surface::GetFormat() const
{
	return ConvertD3DFORMATToGFXFORMAT( m_Platform.GetDXFormat() );
}

void* Surface::CPULock( uint32_t& pitch )
{
	D3DLOCKED_RECT d3dRect;
	dxerror( m_Platform.GetSurface()->LockRect( &d3dRect, 0, 0 ) ); 
	pitch = d3dRect.Pitch;
	return d3dRect.pBits;
}

void Surface::CPUUnlock()
{
	dxerror( m_Platform.GetSurface()->UnlockRect() );
}

void SurfacePlatform::SaveToDisk( const char* pName, D3DXIMAGE_FILEFORMAT fmt, bool bAppendType, bool bInDataDir )
{
	char aName[MAX_PATH];

	if (bInDataDir)
	{
		Util::GetFiosFilePath( TEXTURE_ROOT_PATH, aName );
		strcat( aName, pName );
	}
	else
	{
		Util::GetFiosFilePath( pName, aName );
	}

	if (bAppendType)
	{
		switch( fmt )
		{
		case D3DXIFF_BMP:	strcat( aName, ".bmp" );	break;
		case D3DXIFF_JPG:	strcat( aName, ".jpg" );	break;
		case D3DXIFF_TGA:	strcat( aName, ".tga" );	break;
		case D3DXIFF_PNG:	strcat( aName, ".png" );	break;
		case D3DXIFF_DDS:	strcat( aName, ".dds" );	break;
		default:
			ntError_p(0, ("unrecognised texture format")); return;
		}
	}

	dxerror( D3DXSaveSurfaceToFile( aName, fmt, m_pSurface, 0, 0 ) );
}