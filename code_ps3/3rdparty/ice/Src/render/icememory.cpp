/*
 * Copyright (c) 2003-2006 Naughty Dog, Inc.
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#include <cell/gcm.h>
#include "icerender.h"

using namespace Ice;
using namespace Ice::Render;

Region *s_regions = NULL;
U32 s_numRegions = 0;
Region *s_freeVideoRegionStart = NULL;
Region *s_usedVideoRegionStart = NULL;
Region *s_freeIoRegionStart = NULL;
Region *s_usedIoRegionStart = NULL;
Region *s_freeCullRegionStart = NULL;
Region *s_usedCullRegionStart = NULL;
Region *s_freeTagRegionStart = NULL;
Region *s_usedTagRegionStart = NULL;
Region *s_dirtyRegion = NULL;

namespace Ice
{
	namespace Render
	{
		Tile g_tiles[kMaxTiles] = {0,{0,0,kTileVideoMemory,0,0,kCompDisabled,0,0}};
		CompressionTags g_compressionTags[kMaxTiles] = {0,0};
		DepthCullArea g_depthCullAreas[kMaxDepthCullAreas] = {0,0,{0,0,0,0,0,(DepthStencilBufferFormat)0,(MultisampleMode)0,(DepthCullDirection)0,(DepthCullFormat)0,(ComparisonFunc)0,0,0}};
	}
}

static Region *FindUsedRegion(Region *usedRegionStart, U32 start)
{
	Region *r;
	for (r = usedRegionStart; r; r = r->m_next)
	{
		if ((r->m_typeAndStart & kRegionStartMask) == start)
			return r;
	}
	return NULL;
}

static Region *FindUnusedRegion()
{
	for (U32F i = 0; i < s_numRegions; ++i)
	{
		Region *r = &s_regions[i];
		if ((r->m_typeAndStart & kRegionTypeMask) == kUnused)
			return r;
	}
	return NULL;
}

static bool QueryAllocateRegion(Region const *freeRegionStart, U32 size, U32 alignment)
{
	// must be power of 2
	ICE_ASSERTF((alignment & (alignment-1)) == 0, ("%i", alignment));

	for (Region const *r = freeRegionStart; r; r = r->m_next)
	{
		ICE_ASSERT((alignment-1) <= kRegionStartMask); // optimization check
		U32 miss = r->m_typeAndStart & (alignment-1);

		if (miss != 0)
			miss = alignment - miss;

		if (r->m_size >= size + miss)
			return true;
	}

	return false;
}

static Region *AllocateRegion(Region **freeRegionStart, Region **usedRegionStart, U32 size, RegionType type, U32 alignment)
{
	// must be power of 2
	ICE_ASSERTF((alignment & (alignment-1)) == 0, ("%i", alignment));

	Region *r, *rp = NULL;
	for (r = *freeRegionStart; r; rp = r, r = r->m_next)
	{
		ICE_ASSERT((alignment-1) <= kRegionStartMask); // optimization check
		U32 miss = r->m_typeAndStart & (alignment-1);
		if (miss != 0) miss = alignment - miss;
		if (r->m_size >= size + miss)
		{
			if (miss != 0)
			{
				// Align the region start
				Region *n = FindUnusedRegion();
				n->m_size = miss;
				n->m_typeAndStart = r->m_typeAndStart;
				n->m_next = r;
				ICE_ASSERT((r->m_typeAndStart & kRegionStartMask) + miss <= kRegionStartMask); // optimization check
				r->m_typeAndStart += miss;
				r->m_size -= miss;
				if (rp)
					rp->m_next = n;
				else
					*freeRegionStart = n;
				rp = n;
			}

			U32 off = r->m_typeAndStart & kRegionStartMask;
			ICE_ASSERT((r->m_typeAndStart & kRegionStartMask) + size <= kRegionStartMask); // optimization check
			r->m_typeAndStart += size;
			r->m_size -= size;
			ICE_ASSERTF((off & (alignment-1)) == 0, ("(%x & (%x-1)", off, alignment));

			if (r->m_size == 0)
			{
				// Unlink me
				if (rp)
					rp->m_next = r->m_next;
				else
					*freeRegionStart = r->m_next;

				// Re-link as used region
				Region *u = r;
				u->m_next = *usedRegionStart;
				u->m_size = size;
				u->m_typeAndStart = off | type;
				*usedRegionStart = u;
				//printf("a %08x %08x %08x\n", u->m_typeAndStart, size, alignment);
				return u;
			}
			else
			{
				// Link as used region
				Region *u = FindUnusedRegion();
				u->m_next = *usedRegionStart;
				u->m_size = size;
				u->m_typeAndStart = off | type;
				*usedRegionStart = u;
				//printf("b %08x %08x %08x\n", u->m_typeAndStart, size, alignment);
				return u;
			}
		}
	}

	return NULL;
}

static void ReleaseRegion(Region **freeRegionStart, Region **usedRegionStart, U32 start, RegionType type)
{
	// Find the region in the used list
	Region *r, *rp = NULL;
	for (r = *usedRegionStart; r; rp = r, r = r->m_next)
	{
		if ((r->m_typeAndStart & kRegionStartMask) == start)
		{
			ICE_ASSERTF((r->m_typeAndStart & kRegionTypeMask) == type, ("Released with different type than allocated with! %08x %08x", r->m_typeAndStart & kRegionTypeMask, type));
			//printf("f: %08x %08x\n", r->m_typeAndStart, r->m_size);

			// Unlink
			if (rp)
				rp->m_next = r->m_next;
			else
				*usedRegionStart = r->m_next;

			U32 re = (r->m_typeAndStart & kRegionStartMask) + r->m_size;

			// Add to free list, find insertion point
			Region *f, *fp = NULL;
			for (f = *freeRegionStart; f; fp = f, f = f->m_next)
			{
				if ((f->m_typeAndStart & kRegionStartMask) > start)
				{
					// Merge nearby blocks?
					if (fp && ((fp->m_typeAndStart & kRegionStartMask) + fp->m_size == (r->m_typeAndStart & kRegionStartMask)))
					{
						//printf("p\n");
						// Merge with previous
						fp->m_size += r->m_size;
						r->m_typeAndStart &= kRegionStartMask;

						// Merge prev with next?
						if ((fp->m_typeAndStart & kRegionStartMask) + fp->m_size == (f->m_typeAndStart & kRegionStartMask))
						{
							//printf("pn\n");
							fp->m_size += f->m_size;
							fp->m_next = f->m_next;
							f->m_typeAndStart &= kRegionStartMask;
						}
					}
					else if (re == (f->m_typeAndStart & kRegionStartMask))
					{
						//printf("n\n");
						// Merge with next
						f->m_typeAndStart -= r->m_size;
						f->m_size += r->m_size;
						r->m_typeAndStart &= kRegionStartMask;
					}
					else
					{
						// Insert
						r->m_next = f;
						if (fp)
							fp->m_next = r;
						else
							*freeRegionStart = r;
					}
					break;
				}
			}

			if (f == NULL)
			{
				// Add to end
				r->m_next = NULL;
				if (fp)
					fp->m_next = r;
				else
					*freeRegionStart = r;
			}

			return;
		}
	}

	ICE_ASSERT(!"Pointer Not Allocated With IceRender");
}

static U64 AllocateTile(Region *r)
{
	for (U32F i = 0; i < ARRAY_COUNT(g_tiles); ++i)
	{
		if (g_tiles[i].m_owner == NULL)
		{
			g_tiles[i].m_owner = r;
			return i;
		}
	}

	return 0xFFFFFFFFFFFFFFFFULL;
}

static U64 ReleaseTile(Region *r)
{
	for (U32F i = 0; i < ARRAY_COUNT(g_tiles); ++i)
	{
		if (g_tiles[i].m_owner == r)
		{
			g_tiles[i].m_owner = NULL;
			return i;
		}
	}

	return 0xFFFFFFFFFFFFFFFFULL;
}

static U64 AllocateDepthCullAreaInternal(Region *r, U64 size, U64 *start)
{
	for (U32F i = 0; i < ARRAY_COUNT(g_depthCullAreas); ++i)
	{
		if (g_depthCullAreas[i].m_owner == NULL)
		{
			// Attempt to allocate a Depth-Cull region.
			Region *area = AllocateRegion(&s_freeCullRegionStart, &s_usedCullRegionStart, size, kCull, 0x1000);
			if (area != NULL)
			{
				g_depthCullAreas[i].m_owner = r;
				g_depthCullAreas[i].m_cullArea = area;
				*start = area->m_typeAndStart & kRegionStartMask;
				return i;
			}
			else
			{
				return 0xFFFFFFFFFFFFFFFFULL;
			}
		}
	}

	return 0xFFFFFFFFFFFFFFFFULL;
}


static U64 ReleaseDepthCullAreaInternal(Region *r)
{
	for (U32F i = 0; i < ARRAY_COUNT(g_depthCullAreas); ++i)
	{
		if (g_depthCullAreas[i].m_owner == r)
		{
			ReleaseRegion(&s_freeCullRegionStart, &s_usedCullRegionStart, g_depthCullAreas[i].m_cullArea->m_typeAndStart & kRegionStartMask, kCull);
			g_depthCullAreas[i].m_owner = NULL;
			g_depthCullAreas[i].m_cullArea = NULL;
			return i;
		}
	}

	return 0xFFFFFFFFFFFFFFFFULL;
}

static U64 AllocateCompressionTags(Region *r, U64 size, U64 *start)
{
	for (U32F i = 0; i < ARRAY_COUNT(g_compressionTags); ++i)
	{
		if (g_compressionTags[i].m_owner == NULL)
		{
			// Attempt to allocate some compression tags.
			Region *area = AllocateRegion(&s_freeTagRegionStart, &s_usedTagRegionStart, size, kTag, 1);
			if (area != NULL)
			{
				g_compressionTags[i].m_owner = r;
				g_compressionTags[i].m_tagArea = area;
				*start = area->m_typeAndStart & kRegionStartMask;
				return i;
			}
			else
			{
				return 0xFFFFFFFFFFFFFFFFULL;
			}
		}
	}

	return 0xFFFFFFFFFFFFFFFFULL;
}

static U64 ReleaseCompressionTags(Region *r)
{
	for (U32F i = 0; i < ARRAY_COUNT(g_compressionTags); ++i)
	{
		if (g_compressionTags[i].m_owner == r)
		{
			ReleaseRegion(&s_freeTagRegionStart, &s_usedTagRegionStart, g_compressionTags[i].m_tagArea->m_typeAndStart & kRegionStartMask, kTag);
			g_compressionTags[i].m_owner = NULL;
			g_compressionTags[i].m_tagArea = NULL;
			return i;
		}
	}

	return 0xFFFFFFFFFFFFFFFFULL;
}

void Ice::Render::InitializeMemoryAllocators(void *ioBase, U32 ioSize, void *vmBase, U32 vmSize, Region *regions, U32 numRegions)
{
	memset(regions, 0, sizeof(Region) * numRegions);
	s_regions = regions;
	s_numRegions = numRegions;

	if(ioSize)
	{
		// Setup io memory allocations
		Region *sr = FindUnusedRegion();
		sr->m_next = NULL;
		sr->m_typeAndStart = TranslateAddressToIoOffset(ioBase) | kLinearIo;
		sr->m_size = ioSize;
		s_freeIoRegionStart = sr;
		s_usedIoRegionStart = NULL;
	}

	if(vmSize)
	{
		ICE_ASSERT(vmSize >= 0x10000);	// minimum size due to dirty region
		
		// Setup video memory allocations
		Region *sr = FindUnusedRegion();
		sr->m_next = NULL;
		sr->m_typeAndStart = TranslateAddressToOffset(vmBase) | kLinear;
		sr->m_size = vmSize;
		s_freeVideoRegionStart = sr;
		s_usedVideoRegionStart = NULL;
		
		// Setup Cull memory allocations
		Region *cr = FindUnusedRegion();
		cr->m_next = NULL;
		cr->m_typeAndStart = 0 | kCull;
		cr->m_size = kDepthCullSize;
		s_freeCullRegionStart = cr;
		s_usedCullRegionStart = NULL;
		
		// Setup Tag memory allocations
		Region *tr = FindUnusedRegion();
		tr->m_next = NULL;
		tr->m_typeAndStart = 1 | kTag; // 0th tag used for dirty region
		tr->m_size = kCompressionTagSize-1;
		s_freeTagRegionStart = tr;
		s_usedTagRegionStart = NULL;
		
		// Allocate the dirty region that all depth-cull/tile regions will be set to when they are not used.
		s_dirtyRegion = AllocateRegion(&s_freeVideoRegionStart, &s_usedVideoRegionStart, 0x10000, kDirty, 0x10000);
	}
}


bool Ice::Render::QueryAllocateIoMemory(U32 size, U32 alignment)
{
	return QueryAllocateRegion(s_freeIoRegionStart, size, alignment);
}

void *Ice::Render::AllocateIoMemory(U32 size, U32 alignment)
{
	Region *r = AllocateRegion(&s_freeIoRegionStart, &s_usedIoRegionStart, size, kLinearIo, alignment);
	if (r != NULL)
		return TranslateIoOffsetToAddress(r->m_typeAndStart & kRegionStartMask);
	else
		return NULL;
}

void Ice::Render::ReleaseIoMemory(void *ptr)
{
	ReleaseRegion(&s_freeIoRegionStart, &s_usedIoRegionStart, TranslateAddressToIoOffset(ptr), kLinearIo);
}

void *Ice::Render::AllocateTiledIoMemory(U32 size, U32 pitch, U32 bank)
{
	size = (size + 0xFFFF) & ~0xFFFF;
	Region *r = AllocateRegion(&s_freeIoRegionStart, &s_usedIoRegionStart, size, kTiledIo, 0x10000);
	if (r != NULL)
	{
		U32 off = r->m_typeAndStart & kRegionStartMask;
		U64 tile = AllocateTile(r);
		if (tile != 0xFFFFFFFFFFFFFFFFULL)
		{
			ICE_ASSERT((GetTiledPitch(pitch) == pitch) && "Pitch must be a tileable pitch. Use GetTiledPitch() to convert to the next largest tilable pitch.");
			g_tiles[tile].m_parms.Set(tile, off, kTileMainMemory, size, pitch, kCompDisabled, 0, bank);
			SetTileAndCompression(tile, off, kTileMainMemory, size, pitch, kCompDisabled, 0, bank);
		}
		
		return TranslateIoOffsetToAddress(r->m_typeAndStart & kRegionStartMask);
	}
	else
		return NULL;
}

void Ice::Render::ReleaseTiledIoMemory(void *ptr)
{
	Region *r = FindUsedRegion(s_usedIoRegionStart, TranslateAddressToIoOffset(ptr));

	if (r != NULL)
	{
		U64 tile = ReleaseTile(r);
		if (tile != 0xFFFFFFFFFFFFFFFFULL)
		{
			U32 off = s_dirtyRegion->m_typeAndStart & kRegionStartMask;
			g_tiles[tile].m_parms.Set(tile, off, kTileVideoMemory, 0x10000, 0x200, kCompDisabled, 0, 0);
			SetTileAndCompression(tile, off, kTileVideoMemory, 0x10000, 0x200, kCompDisabled, 0, 0);
		}

		ReleaseRegion(&s_freeIoRegionStart, &s_usedIoRegionStart, TranslateAddressToIoOffset(ptr), kTiledIo);
	}
	else
	{
		ICE_ASSERT(!"Unable to find region associated with ptr!");
	}
}

bool Ice::Render::QueryAllocateLinearVideoMemory(U32 size, U32 alignment)
{
	return QueryAllocateRegion(s_freeVideoRegionStart, size, alignment);
}

void *Ice::Render::AllocateLinearVideoMemory(U32 size, U32 alignment)
{
	Region *r = AllocateRegion(&s_freeVideoRegionStart, &s_usedVideoRegionStart, size, kLinear, alignment);
	if (r != NULL)
		return TranslateOffsetToAddress(r->m_typeAndStart & kRegionStartMask);
	else
		return NULL;
}

void Ice::Render::ReleaseLinearVideoMemory(void *ptr)
{
	ReleaseRegion(&s_freeVideoRegionStart, &s_usedVideoRegionStart, TranslateAddressToOffset(ptr), kLinear);
}

void *Ice::Render::AllocateTiledVideoMemory(U32 size, U32 pitch, CompressionFormat cfmt, U32 bank)
{
	ICE_ASSERTF((size & 0xFFFF) == 0, ("Size must be a multiple of 64k."));
	Region *r = AllocateRegion(&s_freeVideoRegionStart, &s_usedVideoRegionStart, size, kTiled, 0x10000);
	if (r != NULL)
	{
		U32 off = r->m_typeAndStart & kRegionStartMask;
		U64 tile = AllocateTile(r);
		if (tile != 0xFFFFFFFFFFFFFFFFULL)
		{
			U64 tagStart = 0;
			if(cfmt != kCompDisabled)
			{
				U64 tagId = AllocateCompressionTags(r, size/0x10000, &tagStart);
				if(tagId == 0xFFFFFFFFFFFFFFFFULL)
				{
					cfmt = kCompDisabled;
				}
			}
		
			ICE_ASSERT((GetTiledPitch(pitch) == pitch) && "Pitch must be a tileable pitch. Use GetTiledPitch() to convert to the next largest tilable pitch.");
			g_tiles[tile].m_parms.Set(tile, off, kTileVideoMemory, size, pitch, cfmt, tagStart, bank);
			SetTileAndCompression(tile, off, kTileVideoMemory, size, pitch, cfmt, tagStart, bank);
		}
		
		return TranslateOffsetToAddress(off);
	}
	else
		return NULL;
}

void Ice::Render::ReleaseTiledVideoMemory(void *ptr)
{
	U32 off = TranslateAddressToOffset(ptr);
	Region *r = FindUsedRegion(s_usedVideoRegionStart, off);
	if (r != NULL)
	{
		U64 tile = ReleaseTile(r);
		if (tile != 0xFFFFFFFFFFFFFFFFULL)
		{
			ReleaseCompressionTags(r);
			U32 dirtyOff = s_dirtyRegion->m_typeAndStart & kRegionStartMask;
			g_tiles[tile].m_parms.Set(tile, dirtyOff, kTileVideoMemory, 0x10000, 0x200, kCompDisabled, 0, 0);
			SetTileAndCompression(tile, dirtyOff, kTileVideoMemory, 0x10000, 0x200, kCompDisabled, 0, 0);
		}

		ReleaseRegion(&s_freeVideoRegionStart, &s_usedVideoRegionStart, off, kTiled);
	}
	else
	{
		ICE_ASSERT(!"Unable to find region associated with ptr!");
	}
}

void Ice::Render::GetColorSurfaceTileParameters(U32 width, U32 height, ColorBufferFormat format, MultisampleMode multisample, U32 *size, U32 *pitch, CompressionFormat *cfmt)
{
	unsigned int multiplier0, multiplier1;
	switch (multisample)
	{
	case kMultisample2X:
		multiplier0 = 2;
		multiplier1 = 1;
		*cfmt = kCompColor2x1;
		break;
	case kMultisample4XOrdered:
		multiplier0 = 2;
		multiplier1 = 2;
		*cfmt = kCompColor2x2;
		break;
	case kMultisample4XRotated:
		multiplier0 = 2;
		multiplier1 = 2;
		*cfmt = kCompColor2x2;
		break;
	default:
		multiplier0 = 1;
		multiplier1 = 1;
		*cfmt = kCompColor2x1;
		break;
	}

	// Only 32-bit 4 component formats support compression.
	if(format != kBufferArgb8888)
		*cfmt = kCompDisabled;

	unsigned int pixelSize = 4;
	switch (format)
	{
	case kBufferRgb565:   pixelSize = 2; break;
	case kBufferArgb8888: pixelSize = 4; break;
	case kBufferB8:	      pixelSize = 1; break;
	case kBufferGb88:     pixelSize = 2; break;
	case kBufferRgba16f:  pixelSize = 8; break;
	case kBufferRgba32f:  pixelSize = 16; break;
	case kBufferR32f:     pixelSize = 4; break;
	default:
		ICE_ASSERT(!"Invalid color buffer format specified.");
		break;
	}

	width = (width + 0x3f) & ~0x3f;
	height = (height + 0x3f) & ~0x3f;
	U32 p = *pitch = GetTiledPitch(width * pixelSize * multiplier0);
	ICE_ASSERTF(p != 0, ("Pitch of %x is too large.", p));
	*size = ((p * height * multiplier1) + 0xFFFF) & ~0xFFFF;
}

void Ice::Render::GetDepthStencilSurfaceTileParameters(U32 width, U32 height, DepthStencilBufferFormat format, MultisampleMode multisample, U32 *size, U32 *pitch, CompressionFormat *cfmt, U32 *cullWidth, U32 *cullHeight)
{
	U32 tileWidth, tileHeight;
	U32 multiplier[2];
	switch (multisample)
	{
	case kMultisample2X:
		multiplier[0] = 2;
		multiplier[1] = 1;
		*cfmt = kCompDepthStencil2X;
		*cullWidth = width;
		*cullHeight = height;
		tileWidth = (width + 0x3f) & ~0x3f;
		tileHeight = (height + 0x3f) & ~0x3f;
		break;
	case kMultisample4XOrdered:
		multiplier[0] = 2;
		multiplier[1] = 2;
		*cfmt = kCompDepthStencil4XOrdered;
		*cullWidth = width;
		*cullHeight = height;
		tileWidth = (width + 0x3f) & ~0x3f;
		tileHeight = (height + 0x3f) & ~0x3f;
		break;
	case kMultisample4XRotated:
		multiplier[0] = 2;
		multiplier[1] = 2;
		*cfmt = kCompDepthStencil4XRotated;
		*cullWidth = width;
		*cullHeight = height;
		tileWidth = (width + 0x3f) & ~0x3f;
		tileHeight = (height + 0x3f) & ~0x3f;
		break;
	case kMultisampleNone:
	default:
		multiplier[0] = 1;
		multiplier[1] = 1;

//		*cfmt = kCompDepthStencil2X;
//		*cullWidth = width/2;

		*cfmt = kCompDepthStencil;
		*cullWidth = width;

		*cullHeight = height;
		tileWidth = (width + 0x7f) & ~0x7f;
		tileHeight = (height + 0x3f) & ~0x3f;
		break;				
	}

	unsigned int pixelSize;
	switch (format)
	{
	case kBufferD16S0:
		pixelSize = 2;
		*cfmt = kCompDisabled;
		*cullWidth = width;
		*cullHeight = height;
		break;
	case kBufferD24S8:
	default:
		pixelSize = 4;
		break;
	}

	U32 p = *pitch = GetTiledPitch(tileWidth * pixelSize * multiplier[0]);
	ICE_ASSERTF(p != 0, ("%x", p));
	*size = ((p * tileHeight * multiplier[1]) + 0xFFFF) & ~0xFFFF;
}

bool Ice::Render::AllocateDepthCullArea(const void *vmBasePtr, U32F width, U32F height, DepthStencilBufferFormat format, MultisampleMode multisample)
{
	U32 off = TranslateAddressToOffset(vmBasePtr);
	Region *r = FindUsedRegion(s_usedVideoRegionStart, off);
	U64 areaStart;
	U64 area = AllocateDepthCullAreaInternal(r, width*height, &areaStart);
	if (area == 0xFFFFFFFFFFFFFFFFULL)
		return false;
		
	g_depthCullAreas[area].m_parms.Set(area, off, width, height, areaStart, format, multisample, kCullLess, kCullLones, kFuncLess, 0x80, 0xFF);
	SetDepthCullArea(area, off, width, height, areaStart, format, multisample, kCullLess, kCullLones, kFuncLess, 0x80, 0xFF);
	return true;
}

bool Ice::Render::ReleaseDepthCullArea(const void *vmBasePtr)
{
	U32 off = TranslateAddressToOffset(vmBasePtr);
	Region *r = FindUsedRegion(s_usedVideoRegionStart, off);
	U64 area = ReleaseDepthCullAreaInternal(r);
	if (area == 0xFFFFFFFFFFFFFFFFULL)
		return false;
		
	U32 dirtyOff = s_dirtyRegion->m_typeAndStart & kRegionStartMask;
	g_depthCullAreas[area].m_parms.Set(area, dirtyOff, 128, 128, 0, kBufferD24S8, kMultisampleNone, kCullLess, kCullLones, kFuncLess, 0x80, 0xFF);
	SetDepthCullArea(area, dirtyOff, 128, 128, 0, kBufferD24S8, kMultisampleNone, kCullLess, kCullLones, kFuncLess, 0x80, 0xFF);
	return true;
}

U32 Ice::Render::GetAllocationSize(void *ptr, RegionType type)
{
	Region	*usedRegionStart;
	U32		start;
	
	if ((type == kLinearIo) || (type == kTiledIo))
	{
		usedRegionStart = s_usedIoRegionStart;
		start = TranslateAddressToIoOffset(ptr);
		ICE_ASSERTF(start != 0xFFFFFFFF, ("%p is an invalid IO address.", ptr));
	}
	else
	{
		ICE_ASSERTF((type == kLinear) || (type == kTiled), ("Invalid region type."));
		usedRegionStart = s_usedVideoRegionStart;
		start = TranslateAddressToOffset(ptr);
	}
	
	Region *r = FindUsedRegion(usedRegionStart, start);
	return (r) ? r->m_size : 0;
}
