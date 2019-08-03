#ifndef DDS_H
#define DDS_H

#include <Gc/GcTexture.h>

namespace DDS
{
    // surface description flags
    static const u32 DDSD_CAPS           = 0x00000001l;
    static const u32 DDSD_HEIGHT         = 0x00000002l;
    static const u32 DDSD_WIDTH          = 0x00000004l;
    static const u32 DDSD_PITCH          = 0x00000008l;
    static const u32 DDSD_PIXELFORMAT    = 0x00001000l;
    static const u32 DDSD_MIPMAPCOUNT    = 0x00020000l;
    static const u32 DDSD_LINEARSIZE     = 0x00080000l;
    static const u32 DDSD_DEPTH          = 0x00800000l;

    // pixel format flags
    static const u32 DDPF_ALPHAPIXELS    = 0x00000001l;
    static const u32 DDPF_ALPHA			 = 0x00000002l;
    static const u32 DDPF_FOURCC         = 0x00000004l;
    static const u32 DDPF_RGB            = 0x00000040l;
    static const u32 DDPF_LUMINANCE      = 0x00020000l;

    // dwCaps1 flags
    static const u32 DDSCAPS_COMPLEX         = 0x00000008l;
    static const u32 DDSCAPS_TEXTURE         = 0x00001000l;
    static const u32 DDSCAPS_MIPMAP          = 0x00400000l;

    // dwCaps2 flags
    static const u32 DDSCAPS2_CUBEMAP			= 0x00000200l;
	static const u32 DDSCAPS2_CUBEMAP_POSITIVEX = 0x00000400l;
    static const u32 DDSCAPS2_CUBEMAP_NEGATIVEX = 0x00000800l;
    static const u32 DDSCAPS2_CUBEMAP_POSITIVEY = 0x00001000l;
    static const u32 DDSCAPS2_CUBEMAP_NEGATIVEY = 0x00002000l;
    static const u32 DDSCAPS2_CUBEMAP_POSITIVEZ = 0x00004000l;
    static const u32 DDSCAPS2_CUBEMAP_NEGATIVEZ = 0x00008000l;
    static const u32 DDSCAPS2_CUBEMAP_ALL_FACES = 0x0000FC00l;
    static const u32 DDSCAPS2_VOLUME			= 0x00200000l;

    // compressed texture types
    static const u32 FOURCC_DXT1			= 0x31545844l;	//(MAKEFOURCC('D','X','T','1'))
    static const u32 FOURCC_DXT3			= 0x33545844l;	//(MAKEFOURCC('D','X','T','3'))
    static const u32 FOURCC_DXT5			= 0x35545844l;	//(MAKEFOURCC('D','X','T','5'))
	static const u32 FOURCC_R16G16F			= 112;
	static const u32 FOURCC_A16R16G16B16F	= 113;
	static const u32 FOURCC_R32F			= 114;
	static const u32 FOURCC_A32R32G32B32F	= 116;

    struct DDS_PIXELFORMAT
    {
        u32 dwSize;
        u32 dwFlags;
        u32 dwFourCC;
        u32 dwRGBBitCount;
        u32 dwRBitMask;
        u32 dwGBitMask;
        u32 dwBBitMask;
        u32 dwABitMask;
    };

    struct DDS_HEADER
    {
        u32 tag;
        u32 dwSize;
        u32 dwFlags;
        u32 dwHeight;
        u32 dwWidth;
        u32 dwPitchOrLinearSize;
        u32 dwDepth;
        u32 dwMipMapCount;
        u32 dwReserved1[11];
        DDS_PIXELFORMAT ddspf;
        u32 dwCaps1;
        u32 dwCaps2;
        u32 dwReserved2[3];
    };
}

#endif // DDS_H
