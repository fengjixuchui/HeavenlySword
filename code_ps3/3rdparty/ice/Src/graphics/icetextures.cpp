/*
 * Copyright (c) 2003-2005 Naughty Dog, Inc.
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#include "icefiles.h"
#include "iceutils.h"
#include "icetextures.h"
#include "iceloader.h"



using namespace Ice::Graphics;
using namespace Ice::Render;
using namespace Ice;


struct TargaHeader
{
    unsigned char       idLength;
    unsigned char       colorMapType;
    unsigned char       imageType;
    unsigned char       colorMapStartL;
    unsigned char       colorMapStartH;
    unsigned char       colorMapLengthL;
    unsigned char       colorMapLengthH;
    unsigned char       colorMapDepth;
    short               xOffset;
    short               yOffset;
    short               width;
    short               height;
    unsigned char       pixelDepth;
    unsigned char       imageDescriptor;
};


static inline U32 MakeRgbaColor(U32 red, U32 green, U32 blue, U32 alpha)
{
    return ((red << 24) | (green << 16) | (blue << 8) | alpha);
}

static inline U32 GetRedComponent(U32 color)
{
    return (color >> 24);
}

static inline U32 GetGreenComponent(U32 color)
{
    return ((color >> 16) & 255);
}

static inline U32 GetBlueComponent(U32 color)
{
    return ((color >> 8) & 255);
}

static inline U32 GetAlphaComponent(U32 color)
{
    return (color & 255);
}

static unsigned char *DecompressImage(U32 width, U32 height, const unsigned char *imageData, U32 allocSize)
{
    unsigned char *output = new unsigned char[allocSize];
    const unsigned char *c = imageData;

    U32 *image = reinterpret_cast<U32 *>(output);
    U32F area = width * height;
    for (U32F a = 0; a < area;)
    {
        unsigned char d = *c++;
        long count = (d & 0x7F) + 1;

        if ((d & 0x80) != 0)
        {
            unsigned long red = c[0];
            unsigned long green = c[1];
            unsigned long blue = c[2];
            unsigned long alpha = c[3];
            c += 4;

            U32 p = MakeRgbaColor(red, green, blue, alpha);
            do
            {
                image[a++] = p;
            } while (--count > 0);
        }
        else
        {
            do
            {
                unsigned long red = c[0];
                unsigned long green = c[1];
                unsigned long blue = c[2];
                unsigned long alpha = c[3];
                c += 4;

                image[a++] = MakeRgbaColor(red, green, blue, alpha);
            } while (--count > 0);
        }
    }

    return (output);
}

static U32 GetLevelCount(U32 width, U32 height, U32 *pixelCount)
{
    U32 levelCount = 0;
    U32 size = 0;

    U32 mipWidth = width;
    U32 mipHeight = height;

    for (;;)
    {
        levelCount++;
        size += mipWidth * mipHeight;

        if ((mipWidth == 1) && (mipHeight == 1)) break;
        if (mipWidth != 1) mipWidth >>= 1;
        if (mipHeight != 1) mipHeight >>= 1;
    }

    *pixelCount = size;
    return (levelCount);
}

static void GenerateMipmapLevels(U32 *image, U32 width, U32 height)
{
    U32 xsize = 1;
    U32 ysize = 1;
    U32 shift = 0;

    U32 *mipmap = image;

    U32 level = 1;
    for (U32 mipWidth = width, mipHeight = height; (mipWidth != 1) || (mipHeight != 1);)
    {
        mipmap += mipWidth * mipHeight;

        if (mipWidth != 1)
        {
            mipWidth >>= 1;
            xsize <<= 1;
            shift++;
        }

        if (mipHeight != 1)
        {
            mipHeight >>= 1;
            ysize <<= 1;
            shift++;
        }

        for (U32 y = 0; y < mipHeight; y++)
        {
            const U32 *src = image + y * ysize * width;
            U32 *dst = mipmap + y * mipWidth;

            for (U32 x = 0; x < mipWidth; x++)
            {
                U32 red = 0;
                U32 green = 0;
                U32 blue = 0;
                U32 alpha = 0;

                for (U32 j = 0; j < ysize; j++)
                {
                    for (U32 i = 0; i < xsize; i++)
                    {
                        U32 p = src[j * width + x * xsize + i];
                        red += GetRedComponent(p);
                        green += GetGreenComponent(p);
                        blue += GetBlueComponent(p);
                        alpha += GetAlphaComponent(p);
                    }
                }

                red >>= shift;
                green >>= shift;
                blue >>= shift;
                alpha >>= shift;

                dst[x] = MakeRgbaColor(red, green, blue, alpha);
            }
        }

        level++;
    }
}

// This does a swizzle on a single rectangle of memory (not necessarily square, but power of two in both dims)
static void Swizzle(void *dest, void *src, U32 width, U32 height)
{
	for (U32 x=0; x<width; x++)
	{
		for (U32 y=0; y<height; y++)
		{
			// this is the color we want to place
			U32 const color = ((U32*)src)[x + y*width];

			U32 destAddress = 0;

			// compute the destination address by sliding a bit up until we get higher than x's address, copying the x address bits into place as we go
			U32 cnt = 0;
			U32 i = 1;
			for (; i<width && i<height; i<<=1, cnt++)
			{
				destAddress |= (x & i) << cnt;     // this puts all x bits into place (***if this is a square texture***)
				destAddress |= (y & i) << (cnt+1); // this puts all y bits into place
			}
			// copy the rest of the x bits, if x is larger than y (if it's not, this loop will not enter)
			for (; i<width; i<<=1)
			{
				destAddress |= (x & i) << cnt;
			}
			// copy the rest of the y bits, if y is larger than x (if it's not, this loop will not enter either)
			for (; i<height; i<<=1)
			{
				destAddress |= (y & i) << cnt;
			}

			// store it
			((U32*)dest)[destAddress] = color;
		}
	}
}

static unsigned char *SwizzleTexture(U32 width, U32 height, unsigned char *image, U32 *totalSize)
{
	// NOTE: We don't currently know how to correctly align the 1x1 mipmap.
	// There's a nvTexParameterf() call in the Bind() function that clamps
	// the max lod to the 2x2 mipmap.

	U32 mipWidth = width;
	U32 mipHeight = height;
	U32 size = 0;
	do
	{
		size += (mipWidth * mipHeight * 4 + 31) & ~31;

		if (mipWidth != 1) mipWidth >>= 1;
		if (mipHeight != 1) mipHeight >>= 1;

	} while ((mipWidth != 1) || (mipHeight != 1));

	U8 *data = new U8[size];

	mipWidth = width;
	mipHeight = height;
	U32 srcOffset = 0;
	U32 dstOffset = 0;
	do
	{
		Swizzle(data + dstOffset, image + srcOffset, mipWidth, mipHeight);

		srcOffset += mipWidth * mipHeight * 4;
		dstOffset += (mipWidth * mipHeight * 4 + 31) & ~31;

		if (mipWidth != 1) mipWidth >>= 1;
		if (mipHeight != 1) mipHeight >>= 1;

	} while ((mipWidth != 1) || (mipHeight != 1));

	*totalSize = size;
	return (data);
}

static void CopyMemory(const void *src, void *dst, size_t size)
{
    const unsigned int *source = static_cast<const unsigned int *>(src) - 1;
    unsigned int *destin = static_cast<unsigned int *>(dst) - 1;

    size >>= 2;
    while (size != 0)
    {
        *++destin = *++source;
        size--;
    }
}

static void CopyBytes(const void *src, void *dst, size_t size)
{
    const U8 *s = static_cast<const U8 *>(src) - 1;
    U8 *d = static_cast<U8 *>(dst) - 1;

    for (size_t i = 0; i < size; i++) *++d = *++s;
}

Render::Texture *Ice::Graphics::LoadTextureFromFile(const char *filename)
{
	TargaHeader     header;
	U32             pixelCount;
	void			*baseAddress;
	unsigned char   *decompImage = nullptr;

	printf("Loading %s\n", filename);

	File file(filename);
	unsigned long fileSize = file.GetSize();
	unsigned char *imageData = new unsigned char[fileSize];
	file.Read(0, fileSize, imageData);

	memcpy(&header, imageData, sizeof(TargaHeader));
	Reverse(&header.width);
	Reverse(&header.height);

	U32 width = header.width;
	U32 height = header.height;

	U32 levelCount = GetLevelCount(width, height, &pixelCount);
    U32 imageSize = pixelCount * 4;
    U32 allocSize = (imageSize > fileSize) ? imageSize : fileSize;
	unsigned char *sourceImage = imageData + sizeof(TargaHeader);

	if (header.imageType != 2)
	{
		decompImage = DecompressImage(width, height, imageData + sizeof(TargaHeader), allocSize);
		sourceImage = decompImage;
	}
	else
	{
		decompImage = new unsigned char[imageSize];
		CopyBytes(sourceImage, decompImage, width * height * 4);
	}

	GenerateMipmapLevels((U32 *) decompImage, width, height);
	unsigned char *swizzledData = SwizzleTexture(width, height, decompImage, &imageSize);

	baseAddress = Render::AllocateLinearVideoMemory(imageSize);

	CopyMemory(swizzledData, baseAddress, imageSize);

	delete[] swizzledData;
	delete[] decompImage;
	delete[] imageData;

	Render::Texture *texture = new Render::Texture;
	Render::InitTexture(texture);
	texture->SetImage2D(Render::TranslateAddressToOffset(baseAddress), kTextureVideoMemory, levelCount, width, height, kTextureBgra8888);
	texture->SetFilterMode(kFilterLinear, kFilterLinearMipmapLinear);
	return texture;
}

