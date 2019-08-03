/*
 * Copyright (c) 2003-2006 Naughty Dog, Inc.
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#ifndef __SPU__
#include <cell/gcm.h>
#include <cell/gcm_pm.h>
#endif
#include "icerender.h"

using namespace Ice::Render;

#ifndef __SPU__
void Ice::Render::SetDisplaySwapMode(DisplaySwapMode mode)
{
	g_gpuHardwareConfig.m_displaySwapMode = mode;

	if(mode != kSwapModeImmediate)
	{
		unsigned int flip;
		switch(mode)
		{
		case kSwapModeHSync: flip = CELL_GCM_DISPLAY_HSYNC; break;
		default: case kSwapModeVSync: flip = CELL_GCM_DISPLAY_VSYNC; break;
		}

		cellGcmSetFlipMode(flip);
	}
}

void Ice::Render::RegisterRenderTargetForDisplay(RenderTarget *target, U32 index)
{
	ICE_ASSERT(target != NULL);
	ICE_ASSERT(index < 8);
	ICE_ASSERTF(((target->m_targetFormat >> 12) & 0x000F) == kMultisampleNone, ("The Gpu does not support multisampled render targets for display."));

	target->m_displayIndex = index;

	cellGcmSetDisplayBuffer((uint8_t) index, target->m_colorOffset[0], target->m_colorPitch[0], target->m_targetLeftWidth >> 16, target->m_targetTopHeight >> 16);
}

void Ice::Render::SetTileAndCompression(U32 index, U32 offset, TileContext context, U32 size, U32 pitch, CompressionFormat format, U32 base, U32 bank)
{
	ICE_ASSERT(index < kMaxTiles);
	ICE_ASSERT((U64(offset) & 0xFFFF) == 0); // 64k aligned. This is a limitation of gcm. Should be only 4k aligned?
	ICE_ASSERT((size & 0xFFFF) == 0); // 64k aligned. This is a limitation of gcm. Should be only 4k aligned?
	
#if (ICERENDER_RSXTYPE==ICERENDER_RSX_A01)
	U32 cformat = CELL_GCM_COMPMODE_DISABLED;
#else
	U32 cformat;
	switch(format)
	{
		case kCompDisabled: cformat = CELL_GCM_COMPMODE_DISABLED; break;
		case kCompColor2x1: cformat = CELL_GCM_COMPMODE_C32_2X1; break;
		case kCompColor2x2: cformat = CELL_GCM_COMPMODE_C32_2X2; break;
		case kCompDepthStencil: cformat = CELL_GCM_COMPMODE_Z32_SEPSTENCIL; break;
		case kCompDepthStencil2X: cformat = CELL_GCM_COMPMODE_Z32_SEPSTENCIL_DIAGONAL; break;
		case kCompDepthStencil4XOrdered: cformat = CELL_GCM_COMPMODE_Z32_SEPSTENCIL_REG; break;
		case kCompDepthStencil4XRotated: cformat = CELL_GCM_COMPMODE_Z32_SEPSTENCIL_ROTATED; break;
		default: ICE_ASSERT(!"Unknown compression format!"); return;
	}
#endif
	
	U8 location;
	switch(context)
	{
		case kTileVideoMemory: location = CELL_GCM_LOCATION_LOCAL; break;
		case kTileMainMemory: location = CELL_GCM_LOCATION_MAIN; break;
		default: ICE_ASSERT(!"Unknown context!"); return;
	}
	
	cellGcmSetTile(index, location, offset, size, pitch, cformat, base, bank);
}

void Ice::Render::SetDepthCullArea(U32 index, U32 offset, U32 width, U32 height, U32 start, DepthStencilBufferFormat depthFmt, MultisampleMode ms, DepthCullDirection dir, DepthCullFormat cullFmt, ComparisonFunc func, U32 ref, U32 mask)
{
	ICE_ASSERT(index < 8);
	ICE_ASSERT((U64(offset) & 0xFFF) == 0); // 4k aligned
	ICE_ASSERT((start & 0xFFF) == 0); // 4k aligned

	U32 dfmt;
	switch(depthFmt)
	{
		case kBufferD16S0: dfmt = CELL_GCM_ZCULL_Z16; break;
		case kBufferD24S8: dfmt = CELL_GCM_ZCULL_Z24S8; break;
		default: ICE_ASSERT(!"Unknown depth format!"); return;
	}
	
	U32 cellMs;
	switch(ms)
	{
	case kMultisampleNone: cellMs = CELL_GCM_SURFACE_CENTER_1; break;
	case kMultisample2X: cellMs = CELL_GCM_SURFACE_DIAGONAL_CENTERED_2;	break;
	case kMultisample4XOrdered: cellMs = CELL_GCM_SURFACE_SQUARE_CENTERED_4; break;
	case kMultisample4XRotated: cellMs = CELL_GCM_SURFACE_SQUARE_ROTATED_4; break;
	default: ICE_ASSERT(!"Unknown multisample mode!"); return;
	}
	
	U32 zdir;
	switch(dir)
	{
	case kCullLess: zdir = CELL_GCM_ZCULL_LESS; break;
	case kCullGreater: zdir = CELL_GCM_ZCULL_GREATER; break;
	default: ICE_ASSERT(!"Unknown depth cull direction!"); return;
	}
	
	U32 cfmt;
	switch(cullFmt)
	{
	case kCullMsb: cfmt = CELL_GCM_ZCULL_MSB; break;
	case kCullLones: cfmt = CELL_GCM_ZCULL_LONES; break;
	default: ICE_ASSERT(!"Unknown cull format!"); return;
	}
	
	U32 sfunc;
	switch(func)
	{
	case kFuncNever: sfunc = CELL_GCM_NEVER; break;
	case kFuncLess: sfunc = CELL_GCM_LESS; break;
	case kFuncEqual: sfunc = CELL_GCM_EQUAL; break;
	case kFuncLessEqual: sfunc = CELL_GCM_LEQUAL; break;
	case kFuncGreater: sfunc = CELL_GCM_GREATER; break;
	case kFuncNotEqual: sfunc = CELL_GCM_NOTEQUAL; break;
	case kFuncGreaterEqual: sfunc = CELL_GCM_GEQUAL; break;
	case kFuncAlways: sfunc = CELL_GCM_ALWAYS; break;
	default: ICE_ASSERT(!"Unknown stencil func!"); return;
	}
	
	ICE_ASSERTF((ref & ~0xFF) == 0, ("reference(%02x) must be a byte", ref));
	ICE_ASSERTF((mask & ~0xFF) == 0, ("mask(%02x) must be a byte", mask));
	
	if ( index != 1 )
		cellGcmSetZcull(index, offset, width, height, start, dfmt, cellMs, zdir, cfmt, sfunc, ref, mask);
}

#endif // __SPU__

U32 Ice::Render::GetTiledPitch(U32 pitch)
{
	U32 static const pitches[33] =
	{
		0x200,
		0x300,
		0x400,
		0x500,
		0x600,
		0x700,
		0x800,
		0xa00,
		0xc00,
		0xd00,
		0xe00,
		0x1000,
		0x1400,
		0x1800,
		0x1a00,
		0x1c00,
		0x2000,
		0x2800,
		0x3000,
		0x3400,
		0x3800,
		0x4000,
		0x5000,
		0x6000,
		0x6800,
		0x7000,
		0x8000,
		0xa000,
		0xc000,
		0xd000,
		0xe000,
		0x10000
	};
	
	for (U32F i = 0; i < ARRAY_COUNT(pitches); ++i)
	{
		if (pitch <= pitches[i])
			return pitches[i];
	}
	
	return 0;
}

#ifndef __SPU__
void Ice::Render::SetDebugOutputLevel(DebugOutputLevel level)
{
	I32 lvl;
	switch(level)
	{
	default: case kDebugLevelNone: lvl = CELL_GCM_DEBUG_LEVEL0; break;
	case kDebugLevelSimple: lvl = CELL_GCM_DEBUG_LEVEL1; break;
	case kDebugLevelComplete: lvl = CELL_GCM_DEBUG_LEVEL2; break;
	}

	cellGcmSetDebugOutputLevel(lvl);
}

void Ice::Render::SetGpuExceptionCallback(void (*callback)(U32 const exception))
{
	cellGcmSetGraphicsHandler(callback);
}

void Ice::Render::SetSwapStartCallback(void (*callback)(U32 const head))
{
	cellGcmSetQueueHandler(callback);
}

void Ice::Render::SetSwapCompleteCallback(void (*callback)(U32 const head))
{
	cellGcmSetFlipHandler(callback);
}

void Ice::Render::SetVBlankStartCallback(void (*callback)(U32 const head))
{
	cellGcmSetVBlankHandler(callback);
}

void Ice::Render::SetPpuInterruptCallback(void (*callback)(U32 const parameterValue))
{
	cellGcmSetUserHandler(callback);
}

void Ice::Render::SetPerformanceReportEvents(DomainType domain, U32 events[4])
{
	cellGcmSetPerfMonEvent(domain, events);
}

void Ice::Render::GetPerformanceReportResults(DomainType domain, U32 counters[4], U32 cycles[2])
{
	cellGcmGetPerfMonCounter(domain, counters, cycles);
}

static bool ViewportScissorSanityCheck(U32 *contents)
{
	U32 scissorWL = contents[0x08C0/4]; // Scissor Width/Left
	U32 scissorHT = contents[0x08C4/4]; // Scissor Height/Top
	U32 viewportWL = contents[0x0A00/4]; // Viewport Width/Left
	U32 viewportHT = contents[0x0A04/4]; // Viewport Height/Top

	U32 sl = scissorWL & 0xFFFF;
	U32 sr = sl + (scissorWL >> 16);
	U32 st = scissorHT & 0xFFFF;
	U32 sb = st + (scissorHT >> 16);

	U32 vl = viewportWL & 0xFFFF;
	U32 vr = vl + (viewportWL >> 16);
	U32 vt = viewportHT & 0xFFFF;
	U32 vb = vt + (viewportHT >> 16);

	return (sl < vl) || (st < vt) || (sr > vr) || (sb > vb);
}

enum SanityChecks
{
	kCheckScissor        = 0x0000000000000001ULL,
	kCheckVertexArrays   = 0x0000000000000002ULL,
	kCheckTextures       = 0x0000000000000004ULL,
	kCheckVertexTextures = 0x0000000000000008ULL,
	kCheckRenderTarget   = 0x0000000000000010ULL,
};

ValidationError Ice::Render::ValidatePushBuffer(void *start, void *end, U32* outAddr, U32 *outReg, U32 *outValue, ValidationLevel validationLevel, U64 *outWarnings)
{
	U32 dummyU32[3];
	U64 dummyU64;
	if(!outAddr)
		outAddr = &dummyU32[0];
	if(!outReg)
		outReg = &dummyU32[1];
	if(!outValue)
		outValue = &dummyU32[2];
	if(!outWarnings)
		outWarnings = &dummyU64;

	bool static initialized = false;
	struct
	{
		U32 start, num;
	} static validRegs[] = {
		{0x0000, 1},
		{0x0050, 1},
		{0x0064, 3},
		{0x0100, 4},
		{0x0110, 1},
		{0x0120, 4},
		{0x0130, 4},
		{0x0140, 1},
		{0x0180, 4},
		{0x0190, 4},
		{0x01A0, 3},
		{0x01B0, 3},
		{0x0200, 4},
		{0x0210, 4},
		{0x0220, 4},
		{0x0230, 4},
		{0x0240, 4},
		{0x0250, 1},
		{0x0260, 4},
		{0x0270, 4},
		{0x0280, 4},
		{0x02B8, 2},
		{0x02C0, 2*8},
		{0x0300, 4},
		{0x0310, 4},
		{0x0320, 4},
		{0x0330, 4},
		{0x0340, 4},
		{0x0350, 4},
		{0x0360, 4},
		{0x0370, 4},
		{0x0380, 4},
		{0x0394, 2},
		{0x03B0, 1}, 
		{0x03B8, 2},
		{0x03C0, 48},
		{0x08C0, 2},
		{0x08CC, 1},
		{0x08D0, 3},
		{0x08E0, 2},
		{0x0900, 20},
		{0x0A00, 2},
		{0x0A0C, 1},
		{0x0A1C, 1},
		{0x0A20, 4},
		{0x0A30, 4},
		{0x0A60, 40},
		{0x0B00, 16},
		{0x0B40, 10},
		{0x0B80, 32},
		{0x1428, 2},
		{0x1430, 3},
		{0x1450, 2},
		{0x145C, 1},
		{0x1478, 2},
		{0x1480, 32},
		{0x1500, 3},
		{0x1510, 3},
		{0x1520, 3},
		{0x1530, 3},
		{0x1540, 3},
		{0x1550, 3},
		{0x1560, 3},
		{0x1570, 3},
		{0x1580, 3},
		{0x1590, 3},
		{0x15A0, 3},
		{0x15B0, 3},
		{0x15C0, 3},
		{0x15D0, 3},
		{0x15E0, 3},
		{0x15F0, 3},
		{0x1680, 16},
		{0x1710, 4},
		{0x1738, 2},
		{0x1740, 16},
		{0x17C8, 2},
		{0x17D0, 2},
		{0x17DC, 1},
		{0x1800, 16},
		{0x1840, 16},
		{0x1880, 32},
		{0x1900, 16},
		{0x1940, 16},
		{0x1980, 32},
		{0x1A00, 128},
		{0x1C00, 64},
		{0x1D00, 16},
		{0x1D60, 16},
		{0x1DA0, 12},
		{0x1E40, 16},
		{0x1E94, 3},
		{0x1EA0, 4},
		{0x1EE0, 8},
		{0x1F00, 32},
		{0x1FC0, 15},
		{0x2000, 1},
		{0x2100, 2},
		{0x2180, 3},
		{0x230C, 1},
		{0x2310, 4},
		{0x2320, 3},
		{0x4000, 1},
		{0x4180, 3},
		{0x4300, 4},
		{0x6000, 1},
		{0x6100, 2},
		{0x6180, 3},
		{0x6300, 4},
		{0x8000, 1},
		{0x8100, 2},
		{0x8180, 4},
		{0x8300, 2},
		{0xA000, 1},
		{0xA100, 2},
		{0xA180, 4},
		{0xA190, 4},
		{0xA2F8, 2},
		{0xA300, 4},
		{0xA400, 1792},
		{0xC000, 1},
		{0xC100, 2},
		{0xC140, 1},
		{0xC180, 4},
		{0xC190, 3},
		{0xC2FC, 1},
		{0xC300, 4},
		{0xC310, 4},
		{0xC400, 4},
		{0xE000, 1},
		{0xE100, 2},
		{0xE200, 256},
		{0xE920, 2}, // Flip No OSD
		{0xE940, 4}, // Flip OSD
		{0xEB00, 1},
	};
	bool static validRegList[0x3FFF] = {false};
	U32 static validMaskList[0x3FFF];
	U32 static contents[0x3FFF] = {0};
	U64 sanityCheckFlags = 0;
	if(!initialized)
	{
		//U32 static const tiledPitchMask = 0x200|0x300|0x400|0x500|0x600|0x700|0x800|
		//	0xA00|0xC00|0xD00|0xE00|0x1000|0x1400|0x1800|0x1A00|0x1C00|0x2000|
		//	0x2800|0x3000|0x3800|0x4000|0x5000|0x6000|0x6800|0x7000|0x8000|
		//	0xA000|0xC000|0xD000|0xE000|0x10000;
		U32 static const comparisonCodeMask = 0x200|0x201|0x202|0x203|0x204|0x205|0x206|0x207;
		U32 static const stencilOpMask = 0x0000|0x1E00|0x1E01|0x1E02|0x1E03|0x150A|0x8507|0x8508;

		//
		// Initialize valid register list.
		//

		for(U32F i = 0; i < ARRAY_COUNT(validRegs); ++i)
		{
			U32F start = validRegs[i].start/4;
			U32F end = start+validRegs[i].num;
			for(U32F ii = start; ii < end; ++ii)
				validRegList[ii] = true;
		}

		//
		// Initialize valid masks.
		//

		memset(validMaskList, 0xFF, sizeof(validMaskList));
		validMaskList[0x64/4] = 0x00000FFF;
		validMaskList[0x18C/4] = 0xFEED0000|0xFEED0001;
		validMaskList[0x194/4] = 0xFEED0000|0xFEED0001;
		validMaskList[0x198/4] = 0xFEED0000|0xFEED0001;
		validMaskList[0x1A8/4] = 0xFEED0000|0xFEED0001|0x66626660;
		validMaskList[0x1B4/4] = 0xFEED0000|0xFEED0001;
		validMaskList[0x1B8/4] = 0xFEED0000|0xFEED0001;
		validMaskList[0x200/4] = 0x1FFF1FFF;
		validMaskList[0x204/4] = 0x1FFF1FFF;
		validMaskList[0x208/4] = 0xFFFF737F;
		validMaskList[0x20C/4] = 0x001FFFFF;
		validMaskList[0x210/4] = 0x0FFFFFFF;
		validMaskList[0x214/4] = 0x0FFFFFFF;
		validMaskList[0x218/4] = 0x0FFFFFFF;
		validMaskList[0x218/4] = 0x0FFFFFFF;
		validMaskList[0x21C/4] = 0x001FFFFF;
		validMaskList[0x220/4] = 0x0000001F;
		validMaskList[0x22C/4] = 0x001FFFFF;
		validMaskList[0x234/4] = 0;
		validMaskList[0x280/4] = 0x001FFFFF;
		validMaskList[0x284/4] = 0x001FFFFF;
		validMaskList[0x288/4] = 0x0FFFFFFF;
		validMaskList[0x28C/4] = 0x0FFFFFFF;
		validMaskList[0x2B8/4] = 0x0FFF0FFF;
		validMaskList[0x2BC/4] = 0x00000001;
		for(U32F i = 0; i < 8; ++i)
		{
			validMaskList[0x2C0/4+i*2] = 0x0FFF0FFF;
			validMaskList[0x2C4/4+i*2] = 0x0FFF0FFF;
		}
		validMaskList[0x300/4] = 0x00000001;
		validMaskList[0x304/4] = 0x00000001;
		validMaskList[0x308/4] = comparisonCodeMask;
		validMaskList[0x30C/4] = 0x0000FFFF;
		validMaskList[0x310/4] = 0x00000001;
		validMaskList[0x314/4] = 0x830F830F;
		validMaskList[0x318/4] = 0x830F830F;
		validMaskList[0x320/4] = 0xF00FF00F;
		validMaskList[0x324/4] = 0x01010101;
		validMaskList[0x328/4] = 0x00000001;
		validMaskList[0x32C/4] = 0x000000FF;
		validMaskList[0x330/4] = comparisonCodeMask;
		validMaskList[0x334/4] = 0x000000FF;
		validMaskList[0x338/4] = 0x000000FF;
		validMaskList[0x33C/4] = stencilOpMask;
		validMaskList[0x340/4] = stencilOpMask;
		validMaskList[0x344/4] = stencilOpMask;
		validMaskList[0x348/4] = 0x00000001;
		validMaskList[0x34C/4] = 0x000000FF;
		validMaskList[0x350/4] = comparisonCodeMask;
		validMaskList[0x354/4] = 0x000000FF;
		validMaskList[0x358/4] = 0x000000FF;
		validMaskList[0x35C/4] = stencilOpMask;
		validMaskList[0x360/4] = stencilOpMask;
		validMaskList[0x364/4] = stencilOpMask;
		validMaskList[0x368/4] = 0x1D00|0x1D01;
		validMaskList[0x36C/4] = 0x0000000E;
		validMaskList[0x370/4] = 0x0000FFF0;
		validMaskList[0x374/4] = 0x00000001;
		validMaskList[0x378/4] = 0x150F;
		validMaskList[0x380/4] = 0x00000001;
		validMaskList[0x3B0/4] = 0x00100000;
		// 3b8? // TODO
		validMaskList[0x3BC/4] = 0x00000001;
		validMaskList[0x8C0/4] = 0x1FFF1FFF;
		validMaskList[0x8C4/4] = 0x1FFF1FFF;
		validMaskList[0x8CC/4] = 0x2601|0x0800|0x0801|0x0802;
		validMaskList[0x8DC/4] = 0x00000000;
		validMaskList[0x8E4/4] = 0xFFFFFFC3;
		for(U32F i = 0; i < 4; ++i)
		{
			validMaskList[0x900/4+i*8] = 0x0FFFFFFF;
			validMaskList[0x904/4+i*8] = 0x000FBF33;
			validMaskList[0x908/4+i*8] = 0x00000F0F;
			validMaskList[0x90C/4+i*8] = 0xFFFFFF80; // TODO: This can be tighter?
			validMaskList[0x910/4+i*8] = 0x0001FFFF;
			validMaskList[0x914/4+i*8] = 0x00001FFF;
			validMaskList[0x918/4+i*8] = 0x1FFF1FFF;
			validMaskList[0x91C/4+i*8] = 0xFFFFFFFF;
		}
		validMaskList[0xA00/4] = 0x1FFF1FFF;
		validMaskList[0xA04/4] = 0x1FFF1FFF;
		validMaskList[0xA60/4] = 0x00000001;
		validMaskList[0xA64/4] = 0x00000001;
		validMaskList[0xA68/4] = 0x00000001;
		validMaskList[0xA6C/4] = comparisonCodeMask;
		validMaskList[0xA70/4] = 0x00000001;
		validMaskList[0xA74/4] = 0x00000001;
		for(U32F i = 0; i < 16; ++i)
			validMaskList[0xB00/4+i] = 0x0000FFFF;
		for(U32F i = 0; i < 10; ++i)
			validMaskList[0xB40/4+i] = 0x00000011;
		validMaskList[0x1428/4] = 0x00000001;
		validMaskList[0x142C/4] = 0x00000001;
		validMaskList[0x1438/4] = 0x00000003;
		validMaskList[0x1450/4] = 0x00080007;
		validMaskList[0x1454/4] = 0x00000000;
		validMaskList[0x1478/4] = 0x00333333;
		validMaskList[0x147C/4] = 0x00000001;
		for(U32F i = 0; i < 16; ++i)
			validMaskList[0x1680/4+i] = 0x8FFFFFFF;
		validMaskList[0x1710/4] = 0x00000000;
		validMaskList[0x1714/4] = 0x00000000;
		validMaskList[0x1738/4] = 0x0FFFFFFF;
		validMaskList[0x173C/4] = 0xFFFFFFFF;
		for(U32F i = 0; i < 16; ++i)
			validMaskList[0x1740/4+i] = 0xFFFFFF77;
		validMaskList[0x17C8/4] = 0x00000003;
		validMaskList[0x17CC/4] = 0x00000001;
		validMaskList[0x1800/4] = 0x07FFFFFF;
		validMaskList[0x1804/4] = 0x00000001;
		validMaskList[0x1808/4] = 0x0000000F;
		validMaskList[0x181C/4] = 0x0FFFFFFF;
		validMaskList[0x1820/4] = 0x00000011;
		validMaskList[0x1828/4] = 0x1B00|0x1B01|0x1B02;
		validMaskList[0x182C/4] = 0x1B00|0x1B01|0x1B02;
		validMaskList[0x1830/4] = 0x404|0x405|0x408;
		validMaskList[0x1834/4] = 0x900|0x901;
		validMaskList[0x1838/4] = 0x00000001;
		validMaskList[0x183C/4] = 0x00000001;
		for(U32F i = 0; i < 16; ++i)
			validMaskList[0x1840/4+i] = 0xFFF0FFFF;
		for(U32F i = 0; i < 16; ++i)
		{
			validMaskList[0x1A00/4+i*8] = 0x0FFFFFFF;
			validMaskList[0x1A04/4+i*8] = 0x000FFF3F;
			validMaskList[0x1A08/4+i*8] = 0xF0FF1F3F;
			validMaskList[0x1A0C/4+i*8] = 0xFFFFFFF7;
			validMaskList[0x1A10/4+i*8] = 0x0001FFFF;
			validMaskList[0x1A14/4+i*8] = 0xFF0FFFFF; // TODO: This could be tighter?
			validMaskList[0x1A18/4+i*8] = 0x1FFF1FFF;
			validMaskList[0x1A1C/4+i*8] = 0xFFFFFFFF;
		}
		//validMaskList[0x1D00/4] = ; // TODO
		validMaskList[0x1D60/4] = 0x3F0084CE;
		validMaskList[0x1D64/4] = 0x02FF02FF;
		validMaskList[0x1D6C/4] = 0x00000FF0;
		validMaskList[0x1D78/4] = 0x00000111;
		validMaskList[0x1D7C/4] = 0xFFFF0111;
		validMaskList[0x1D80/4] = 0x00000003;
		validMaskList[0x1D84/4] = 0x00000003;
		validMaskList[0x1D88/4] = 0x00001FFF;
		validMaskList[0x1D94/4] = 0x000000F3;
		validMaskList[0x1D98/4] = 0x0FFF0FFF;
		validMaskList[0x1D9C/4] = 0x0FFF0FFF;
		validMaskList[0x1DA4/4] = 0x00000000;
		validMaskList[0x1DAC/4] = 0x00000001;
		validMaskList[0x1DB4/4] = 0x00000001;
		validMaskList[0x1E94/4] = 0x00000113;
		validMaskList[0x1E98/4] = 0x03FFFFFF;
		validMaskList[0x1E9C/4] = 0x000001FF;
		validMaskList[0x1EA0/4] = 0x000001FF;
		validMaskList[0x1EA4/4] = 0x00000011;
		validMaskList[0x1EAC/4] = 0xFFFF0000 | comparisonCodeMask;
		validMaskList[0x1EE4/4] = 0x00000001;
		validMaskList[0x1EE8/4] = 0x0000FFF7; // TODO
		validMaskList[0x1EF8/4] = 0x00FFFFFF;
		validMaskList[0x1EFC/4] = 0x000001FF;
		validMaskList[0x1FC0/4] = 0x0000FFFF;
		validMaskList[0x1FC4/4] = 0x0FFFFFFF;
		validMaskList[0x1FC8/4] = 0xFFFFFFFF;
		validMaskList[0x1FCC/4] = 0x000000FF;
		validMaskList[0x1FD0/4] = 0x003F3F3F;
		validMaskList[0x1FD4/4] = 0x003F3F3F;
		validMaskList[0x1FD8/4] = 0x00000003;
		validMaskList[0x1FDC/4] = 0x0000FFFF;
		validMaskList[0x1FEC/4] = 0x00000001;
		validMaskList[0x1FF4/4] = 0x003FFFFF;

		initialized = true;
	}
	*outWarnings = 0;
	U32F vmSize = g_gpuHardwareConfig.m_usableVideoMemorySize;
	U32 returnTo = 0xFF1CE1CE;
	U32 putNext = TranslateAddressToIoOffset(start);
	U32 *pPutNext = (U32*)start;
	U32 putEnd = TranslateAddressToIoOffset(end);
	if(putNext == 0xFFFFFFFF) return kErrorStartAtInvalidLocation;
	if(putEnd == 0xFFFFFFFF) return kErrorEndAtInvalidLocation;
	I32 drawCount = 0;
	while (putNext != putEnd)
	{
		U32 getValue = *pPutNext;
		if ((getValue&0xBFFF0003) == 0) // Nop
		{
			putNext += 4;
			++pPutNext;
			if (((putNext&0xFFFFF)==0) && (TranslateIoOffsetToAddress(putNext) == NULL))
			{
				*outReg = 0;
				*outValue = putNext;
				return kErrorSteppedToInvalidLocation;
			}
			*outAddr = putNext;
		}
		else if (getValue & 0x20000000) // Jump Short
		{
			putNext = getValue & ~0x20000000;
			pPutNext = (U32*)TranslateIoOffsetToAddress(putNext);
			if (pPutNext == NULL)
			{
				*outReg = 0;
				*outValue = putNext;
				return kErrorShortJumpToInvalidLocation;
			}
			*outAddr = putNext;
		}
		else if (getValue & 0x00000001) // Jump Long
		{
			putNext = getValue & ~0x00000001;
			pPutNext = (U32*)TranslateIoOffsetToAddress(putNext);
			if (pPutNext == NULL)
			{
				*outReg = 0;
				*outValue = putNext;
				return kErrorLongJumpToInvalidLocation;
			}
			*outAddr = putNext;
		}
		else if (getValue & 0x00000002) // Call
		{
			if (returnTo != 0xFF1CE1CE) return kErrorCallWithoutReturn;
			returnTo = putNext + 4;
			putNext = getValue & ~0x00000002;
			pPutNext = (U32*)TranslateIoOffsetToAddress(putNext);
			if (pPutNext == NULL)
			{
				*outReg = 0;
				*outValue = putNext;
				return kErrorCallToInvalidLocation;
			}
			*outAddr = putNext;
		}
		else if (getValue & 0x00020000) // Return
		{
			if (returnTo == 0xFF1CE1CE) return kErrorReturnWithoutCall;
			putNext = returnTo;
			returnTo = 0xFF1CE1CE;
			pPutNext = (U32*)TranslateIoOffsetToAddress(putNext);
			*outAddr = putNext;
		}
		else
		{
			U32F inc = (getValue & 0x40000000) ? 0 : 4;
			U32F num = (getValue >> 18) & 0x7FF;
			putNext += 4;
			++pPutNext;
			*outAddr = putNext;
			
			U32F reg = getValue & 0xFFFC;
			*outReg = reg;
			while(num--)
			{
				*outAddr = putNext;
				U32 const value = *pPutNext;
				contents[reg/4] = value;
				*outValue = value;
				if (validRegList[reg/4] == false) return kErrorInvalidRegister;
				if ((value & ~validMaskList[reg/4])!=0) return kErrorInvalidRegisterValue; 
				if (reg == 0x1824)
				{
					U32 baseOffset = contents[0x1738/4];
					U32 baseIndex = contents[0x173C/4];
					U32 start = value & 0x00FFFFFF;
					U32 count = value >> 24;
					
					// Index buffer validation
					{
						U32 ioff = contents[0x181C/4];
						U32 fmt = contents[0x1820/4];
						U32F indexSize = ((fmt&0xF0) == 0x10) ? 2 : 4;

						// Valid index offset?
						void *indexes;
						if(!(fmt&0xF))
						{
							if(ioff >= vmSize) 
								return kErrorIndexBufferInvalidOffset;
							indexes = TranslateOffsetToAddress(ioff);
							U32F endOff = ioff + (start + count)*indexSize;
							if(endOff >= vmSize) 
								return kErrorDrawElementsIndexBufferMemoryOverrun;
						}
						else
						{
							indexes = TranslateIoOffsetToAddress(ioff);
							if(!indexes) 
								return kErrorIndexBufferInvalidOffset;
							U32F endOff = ioff + (start + count)*indexSize;
							if(!TranslateIoOffsetToAddress(endOff)) 
								return kErrorDrawElementsIndexBufferMemoryOverrun;
						}

						//
						// Validate Every Index!! 
						// (SLOW VALIDATION OPERATION IF INDEXES ARE IN VIDEO MEMORY!)
						//
						if(validationLevel == kValidateHeavy)
						{
							U32 restartEnable = contents[0x1DAC/4];
							U32 restart = contents[0x1DB0/4];
							U32 frequencyControl = contents[0x1FC0/4];
							for(U32F i = 0; i <= count; ++i)
							{
								U32 index = ((fmt&0xF0) == 0x10) ? *((U16*)indexes+start+i) : *((U32*)indexes+start+i);
								
								if(restartEnable && (index == restart))
									continue;
									
								index += baseIndex;

								// Check Vertex array memory overruns
								for(U32F i = 0; i < 16; ++i)
								{
									U32 fmt = contents[0x1740/4+i];
									// enabled? (stride of non-zero)
									if(fmt&0xFF00)
									{
										U32 divider = fmt >> 16;
										U32 stride = (fmt&0xFF00) >> 8;
										U32 offsetAndContext = contents[0x1680/4+i];
										U32 vaContext = offsetAndContext&0x80000000;
										U32 adjustedIndex = index;
										if(frequencyControl&(1<<i))
											adjustedIndex %= divider;
										else if(divider)
											adjustedIndex /= divider;
										U32 vaOff = (offsetAndContext&0x7FFFFFFF) + baseOffset + adjustedIndex*stride;
										// Valid index offset?
										if(!vaContext)
										{
											if(vaOff >= vmSize) return ValidationError(kErrorDrawElementsVertexBuffer0MemoryOverrun+i);
										}
										else if(!TranslateIoOffsetToAddress(vaOff)) return ValidationError(kErrorDrawElementsVertexBuffer0MemoryOverrun+i);
									}
								}
							}
						}
					}
				}
				else if (reg == 0x1808)
				{
					// Begin drawing only.
					if(value != 0)
					{
						//
						// Primitive Launch Check
						//

						if(sanityCheckFlags & kCheckScissor)
						{
							if(ViewportScissorSanityCheck(contents)) 
								return kErrorScissorMustBeInsideViewport;
							sanityCheckFlags ^= kCheckScissor;
						}

						//
						// Vertex attribute array validation
						//
						if(sanityCheckFlags & kCheckVertexArrays)
						{
							U32 baseVaOffset = contents[0x1738/4];
							for(U32F i = 0; i < 16; ++i)
							{
								U32 fmt = contents[0x1740/4+i];
								// enabled? (stride of non-zero)
								if(fmt&0xFF00)
								{
									U32 vaOff = contents[0x1680/4+i];
									// Valid index offset?
									if((vaOff&0x80000000) == 0x00000000)
									{
										if(vaOff+baseVaOffset >= vmSize) return ValidationError(kErrorVertexAttribArray0InvalidOffset+i);
									}
									else if(!TranslateIoOffsetToAddress((vaOff&0x7FFFFFFF)+baseVaOffset)) return ValidationError(kErrorVertexAttribArray0InvalidOffset+i);
								}
							}
							sanityCheckFlags ^= kCheckVertexArrays;
						}

						//
						// Texture setup validation
						//
						if(sanityCheckFlags & kCheckTextures)
						{
							for(U32F i = 0; i < 16; ++i)
							{
								U32F base = 0x1A00/4+i*8;
								U32 control2 = contents[base+0x0C/4];
								if(control2&0x80000000) // Enabled?
								{
									U32 texoff = contents[base+0x00/4];
									U32 fmt = contents[base+0x04/4];
									U32 control1 = contents[base+0x08/4];
									U32 swizzle = contents[base+0x10/4];
									U32 filter = contents[base+0x14/4];
									U32 size1 = contents[base+0x18/4];
									U32 size2 = contents[0x1840/4+i];
									U32 width = (size1 >> 16) & 0x1FFF;
									U32 height = size1 & 0x1FFF;
									U32 depth = size2 >> 20;

									// Valid offset?
									if((fmt&3)==1)
									{
										if(texoff >= vmSize) return ValidationError(kErrorTexture0InvalidOffset+i);
									}
									else if(!TranslateIoOffsetToAddress(texoff)) return ValidationError(kErrorTexture0InvalidOffset+i);

									// Border texels?
									if((fmt&0x8)==0)
									{
										if((width&(width-1)) != 0) return kErrorTexturesWithBordersMustBePowerOfTwo;
										if((height&(height-1)) != 0) return kErrorTexturesWithBordersMustBePowerOfTwo;
										if((depth&(depth-1)) != 0) return kErrorTexturesWithBordersMustBePowerOfTwo;
									}

									// Unnormalized?
									if(fmt&0x4000)
									{
										U32 swrap = (control1&0x0000F);
										U32 twrap = (control1&0x00F00)>>8;
										U32 rwrap = (control1&0xF0000)>>16;

										// Mirror or Wrap?
										if((swrap != 0x3) && (swrap != 0x4) && (swrap != 0x5) && ((width&(width-1)) != 0))
											return kErrorWidthOnUnormalizedTextureWithWrapOrMirrorMustBePowerOfTwo;
										if((twrap != 0x3) && (twrap != 0x4) && (twrap != 0x5) && ((height&(height-1)) != 0))
											return kErrorHeightOnUnormalizedTextureWithWrapOrMirrorMustBePowerOfTwo;
										if((rwrap != 0x3) && (rwrap != 0x4) && (rwrap != 0x5) && ((depth&(depth-1)) != 0))
											return kErrorDepthOnUnormalizedTextureWithWrapOrMirrorMustBePowerOfTwo;

										// No Mips allowed.
										if((fmt&0x000F0000)!=0x00010000) return kErrorUnnormalizedTexturesCannotHaveMips;
									}

									if(((fmt&0x8)==0) && ((fmt&0x2000)==0)) // Border texels are on and swizzled.
									{
										if((fmt&0x30)==0x10) // 1D
										{
											if(height!=1) return kErrorHeightFor1DTextureIsNotOne;
											if(width>0x0800) return kErrorWidthFor1DTextureWithBorderTexelsIsGreaterThan2048;
										}
										else if((fmt&0x30)==0x20) // 2D
										{
											if(height>0x0800) return kErrorHeightFor2DTextureWithBorderTexelsIsGreaterThan2048;
											if(width>0x0800) return kErrorWidthFor2DTextureWithBorderTexelsIsGreaterThan2048;
										}
										else //if((fmt&0x30)==0x30) // 3D
										{
											if(height>0x0100) return kErrorHeightFor3DTextureWithBorderTexelsIsGreaterThan256;
											if(width>0x0100) return kErrorWidthFor3DTextureWithBorderTexelsIsGreaterThan256;
											if(depth>0x0100) return kErrorDepthFor3DTextureWithBorderTexelsIsGreaterThan256;
										}
									}
									else
									{
										if((fmt&0x30)==0x10) // 1D
										{
											if(height!=1) return kErrorHeightFor1DTextureIsNotOne;
											if(width>0x1000) return kErrorWidthFor1DTextureIsGreaterThan4096;
										}
										else if((fmt&0x30)==0x20) // 2D
										{
											if(height>0x1000) return kErrorHeightFor2DTextureIsGreaterThan4096;
											if(width>0x1000) return kErrorWidthFor2DTextureIsGreaterThan4096;
										}
										else //if((fmt&0x30)==0x30) // 3D
										{
											if(width>0x0200) return kErrorHeightFor3DTextureIsGreaterThan512;
											if(height>0x0200) return kErrorWidthFor3DTextureIsGreaterThan512;
											if(depth>0x0200) return kErrorDepthFor3DTextureIsGreaterThan512;
										}
									}

									if((fmt&0x30)==0x10) // 1D
									{
										if(fmt&0x4) return kErrorCubeMappingWith1DTexture;
										if(depth != 1) return kErrorDepthFor1DTextureIsNotOne;
									}
									else if((fmt&0x30)==0x20) // 2D
									{
										if(((control2&0x70)!=0) && (((filter&0xF0000)==0x70000) || ((filter&0x0F000000)==0x04000000))) 
											return kErrorConvolutionFilterAndAnisotropicFilteringWith2DTexture;
										if(fmt&0x4)
										{
											if((width^height) != 0) return kErrorCubeMappingWithoutEqualWidthHeight;
											if((width&(width-1)) != 0) return kErrorCubeMapTexturesMustBePowerOfTwo;
										}
										if(depth != 1) return kErrorDepthFor2DTextureIsNotOne;
									}
									else //if((fmt&0x30)==0x30) // 3D
									{
										if(fmt&0x4) return kErrorCubeMappingWith3DTexture;
										if(control2&0x70) return kErrorAnisotropicFilteringWith3DTexture;
										if((filter&0xF0000)==0x70000) return kErrorConvolutionFilterWith3DTexture;
										if((filter&0x0F000000)==0x04000000) return kErrorConvolutionFilterWith3DTexture;
										if(   ((fmt&0x1F00)==0x1000)    // Depth24
										|| ((fmt&0x1F00)==0x1100)    // Depth24F
										|| ((fmt&0x1F00)==0x1200)    // Depth16
										|| ((fmt&0x1F00)==0x1300))   // Depth16F
										{
											return kErrorDepthTexturesMustBe1DOr2D;
										}
										if((width&(width-1)) != 0) return kErrorWidthFor3DTexturesMustBePowerOfTwo;
										if((height&(height-1)) != 0) return kErrorHeightFor3DTexturesMustBePowerOfTwo;
										if((depth&(depth-1)) != 0) return kErrorDepthFor3DTexturesMustBePowerOfTwo;
									}

									// Swizzled?
									if((fmt&0x2000)==0)
									{
										if((size2&0x1FFFF) != 0) return kErrorSwizzledTextureWithNonZeroPitch;
										if((width&(width-1)) != 0) return kErrorWidthNotPowerOfTwoWithSwizzledTexture;
										if((height&(height-1)) != 0) return kErrorHeightNotPowerOfTwoWithSwizzledTexture;
										if(   ((fmt&0x1F00)==0x0D00)    // B8R8,G8R8
										|| ((fmt&0x1F00)==0x0E00)    // R8B8,R8G8
										|| ((fmt&0x1F00)==0x0900)    // SB8
										|| ((fmt&0x1F00)==0x0C00)    // SG8SB8
										|| ((fmt&0x1F00)==0x0A00)    // X7SY9
										|| ((fmt&0x1F00)==0x1600))   // A4V6YB6A4U6YA6
										{
											return kErrorInvalidSwizzledTextureFormat;
										}
									}

									// Gamma correction enabled?
									if(control1&0x00F00000) 
									{
										if(((fmt&0x1F00)!=0x0100)    // B8
										&& ((fmt&0x1F00)!=0x0500)    // Rgba8888
										&& ((fmt&0x1F00)!=0x0600)    // Dxt1
										&& ((fmt&0x1F00)!=0x0700)    // Dxt3
										&& ((fmt&0x1F00)!=0x0800)    // Dxt5
										&& ((fmt&0x1F00)!=0x0900)    // SB8
										&& ((fmt&0x1F00)!=0x0B00)    // Gb88
										&& ((fmt&0x1F00)!=0x0C00)    // SG8SB8
										&& ((fmt&0x1F00)!=0x0D00)    // B8R8,G8R8
										&& ((fmt&0x1F00)!=0x0E00)    // R8B8,R8G8
										&& ((fmt&0x1F00)!=0x1800)    // Unsigned HiLo8
										&& ((fmt&0x1F00)!=0x1900)    // Signed HiLo8
										&& ((fmt&0x1F00)!=0x1E00))   // Xrgb8888
										{
											return kErrorGammaCorrectionOnTextureWithout8bitComponents;
										}
									}

									if(    ((fmt&0x1F00)==0x1A00) // Rgba16f
										|| ((fmt&0x1F00)==0x1B00) // Rgba32f
										|| ((fmt&0x1F00)==0x1C00) // R32f
										|| ((fmt&0x1F00)==0x1F00))// Rg16f
									{
										// Anisotropic filtering enabled.
										if(control2&0x70) return kErrorAnisotropicFilteringWithFpTexture;
										// No remapping allowed.
										if((swizzle&0xFF)!=0xE4) return kErrorComponentSwizzlingWithFpTexture;
										// Per-texture alpha test.
										if(control2&0x4) return kErrorPerTextureAlphaTestWithFpTexture;

										// R32f? 
										if((fmt&0x1F00)==0x1C00) 
										{
											// Point Sampling only.
											if((filter&0x000F0000)!=0x00030000) return kErrorFp32TextureWithoutNearestMipmapNearestMinFilter;
											if((filter&0x0F000000)!=0x01000000) return kErrorFp32TextureWithoutNearestMagFilter;
										}
									}
									else if(((fmt&0x1F00)==0x1400) // R16
										||  ((fmt&0x1F00)==0x1500))// Rg16
									{
										if(    ((swizzle&0xFF)!=0xE4)        // Supported 16-bit remaps
											&& ((swizzle&0xFF)!=0x4E)
											&& ((swizzle&0xFF)!=0xEE)
											&& ((swizzle&0xFF)!=0x44))
										{
											return kErrorInvalidSwizzleFor16bitPerComponentTexture;
										}
									}
								}
							}
							sanityCheckFlags ^= kCheckTextures;
						}

						//
						// Vertex Texture Setup Validation
						//
						if(sanityCheckFlags & kCheckVertexTextures)
						{
							for(U32F i = 0; i < 4; ++i)
							{
								U32F base = 0x0900/4+i*8;
								U32 control2 = contents[base+0x0C/4];
								if(control2&0x80000000) // Enabled?
								{
									U32 vtoff = contents[base+0x00/4];
									U32 fmt = contents[base+0x04/4];
									//U32 control1 = contents[base+0x08/4];
									//U32 size2 = contents[base+0x10/4];
									//U32 filter = contents[base+0x14/4];
									//U32 size1 = contents[base+0x18/4];
									//U32 width = (size1 >> 16) & 0x1FFF;
									//U32 height = size1 & 0x1FFF;

									// Valid offset?
									if((fmt&3)==1)
									{
										if(vtoff >= vmSize) return ValidationError(kErrorVertexTexture0InvalidOffset+i);
									}
									else if(!TranslateIoOffsetToAddress(vtoff)) return ValidationError(kErrorVertexTexture0InvalidOffset+i);

									// 3D?
									if((fmt&0x30)==0x30) return kErrorVertexTexturesCannotBe3D;
									// Unnormalized?
									if(fmt&0x4000) return kErrorVertexTexturesMustBeNormalized;
									// Swizzled?
									if((fmt&0x2000)==0) return kErrorVertexTexturesMustBeLinear;
									// R32f, Rgba32f only.
									if(((fmt&0x1F00)!=0x1B00) && ((fmt&0x1F00)!=0x1C00)) return kErrorVertexTexturesMustBeRgba32fOrR32f;
								}
							}
							sanityCheckFlags ^= kCheckVertexTextures;
						}

						//
						// Render target validation
						//
						if(sanityCheckFlags & kCheckRenderTarget)
						{
							U32 rtfmt = contents[0x0208/4];
							U32 fmt = rtfmt & 0x1F;
							U32 dsfmt = rtfmt & 0xE0;

							// Valid color offset?
							U32 dbmask = contents[0x220/4];
							if(dbmask&0x1)
							{
								if(contents[0x0194/4] == 0xFEED0000)
								{
									if(contents[0x0210/4] >= vmSize) return kErrorRenderTargetColorBuffer0InvalidOffset;
								}
								else if(!TranslateIoOffsetToAddress(contents[0x0210/4])) return kErrorRenderTargetColorBuffer0InvalidOffset;
							}

							// Valid depth offset?
							if(contents[0x0198/4] == 0xFEED0000)
							{
								if(contents[0x0214/4] >= vmSize) return kErrorRenderTargetDepthBufferInvalidOffset;
							}
							else if(!TranslateIoOffsetToAddress(contents[0x0214/4])) return kErrorRenderTargetDepthBufferInvalidOffset;

							// Depth Testing Enabled?
							if(contents[0x0A74/4])
							{
								if (fmt == 0x09) return kErrorDepthTestEnabledOnB8RenderTarget; 
								if (fmt == 0x0A) return kErrorDepthTestEnabledOnGb88RenderTarget; 
							}

							// Mrt Enabled?
							if(dbmask&0x10)
							{
								if((contents[0x0194/4] != contents[0x018C/4])
									|| (contents[0x0194/4] != contents[0x01B4/4])
									|| (contents[0x0194/4] != contents[0x01B8/4]))
									return kErrorMrtAndColorBuffersInBothVideoAndMainMemory;
							
								if(contents[0x018C/4] == 0xFEED0000)
								{
									if((dbmask&0x2) && (contents[0x0218/4] >= vmSize)) return kErrorRenderTargetColorBuffer1InvalidOffset;
									if((dbmask&0x4) && (contents[0x0288/4] >= vmSize)) return kErrorRenderTargetColorBuffer2InvalidOffset;
									if((dbmask&0x8) && (contents[0x028C/4] >= vmSize)) return kErrorRenderTargetColorBuffer3InvalidOffset;
								}
								else
								{
									if((dbmask&0x2) && !TranslateIoOffsetToAddress(contents[0x0218/4])) return kErrorRenderTargetColorBuffer1InvalidOffset;
									if((dbmask&0x4) && !TranslateIoOffsetToAddress(contents[0x0288/4])) return kErrorRenderTargetColorBuffer2InvalidOffset;
									if((dbmask&0x8) && !TranslateIoOffsetToAddress(contents[0x028C/4])) return kErrorRenderTargetColorBuffer3InvalidOffset;
								}

								if (contents[0x0300/4] != 0) return kErrorDitherEnabledWithMrt;
								if (contents[0x0304/4] != 0) return kErrorAlphaTestingWithMultipleRenderTargets;
								if ((contents[0x1D7C/4] & 0x10) != 0) return kErrorAlphaToCoverageWithMultipleRenderTargets;
								
								// Fragment Program validation
								U32 regCount = contents[0x1D60/4] >> 24;
								if(((dbmask&1)!=0x0) && (regCount < 1)) return kErrorColorBuffer0EnabledButNotWrittenByFragmentProgram;
								if(((dbmask&2)!=0x0) && (regCount < 3)) return kErrorColorBuffer1EnabledButNotWrittenByFragmentProgram;
								if(((dbmask&4)!=0x0) && (regCount < 4)) return kErrorColorBuffer2EnabledButNotWrittenByFragmentProgram;
								if(((dbmask&8)!=0x0) && (regCount < 5)) return kErrorColorBuffer3EnabledButNotWrittenByFragmentProgram;

								if(fmt == 0x09) return kErrorMrtWithB8RenderTarget;
								if(fmt == 0x0A) return kErrorMrtWithGb88RenderTarget;
							}

							// Multisampling Enabled?
							if(rtfmt&0xF000)
							{
								if ((fmt == 0x0B) || (fmt == 0x0C) || (fmt == 0x0D)) 
								{
									if(contents[0x03BC/4]) return kErrorFpRenderTargetLineSmooth;
									if(contents[0x1838/4]) return kErrorFpRenderTargetPolygonSmooth;
								}
								if((rtfmt&0x0F00)==0x0200) return kErrorMultisampledSwizzledRenderTarget;
							}
							else
							{
								if ((contents[0x1D7C/4] & 0x1) != 0) return kErrorMultisamplingEnabledWithNonMultisampledRenderTarget;
							}

							// Swizzled render target?
							if((rtfmt&0x0F00)==0x0200)
							{
								if((fmt == 0x0B) || (fmt == 0x0C) || (fmt == 0x0D)) return kErrorSwizzledFpRenderTarget;
								if(fmt == 0x09) return kErrorSwizzledB8RenderTarget;
								if(fmt == 0x0A) return kErrorSwizzledGb88RenderTarget;
								U32 color0Pitch = contents[0x20C/4];
								U32 color1Pitch = contents[0x21C/4];
								U32 color2Pitch = contents[0x280/4];
								U32 color3Pitch = contents[0x284/4];
								U32 depthPitch = contents[0x22C/4];
								if(color0Pitch&(color0Pitch-1)) return kErrorSwizzledRenderTargetColorBuffer0PitchMustBePowerOfTwo;
								if(color1Pitch&(color1Pitch-1)) return kErrorSwizzledRenderTargetColorBuffer1PitchMustBePowerOfTwo;
								if(color2Pitch&(color2Pitch-1)) return kErrorSwizzledRenderTargetColorBuffer2PitchMustBePowerOfTwo;
								if(color3Pitch&(color3Pitch-1)) return kErrorSwizzledRenderTargetColorBuffer3PitchMustBePowerOfTwo;
								if(depthPitch&(depthPitch-1)) return kErrorSwizzledRenderTargetDepthBufferPitchMustBePowerOfTwo;
								if(color0Pitch < 64) return kErrorSwizzledRenderTargetColorBuffer0PitchMustBeGreaterThanOrEqualTo64;
								if(color1Pitch < 64) return kErrorSwizzledRenderTargetColorBuffer1PitchMustBeGreaterThanOrEqualTo64;
								if(color2Pitch < 64) return kErrorSwizzledRenderTargetColorBuffer2PitchMustBeGreaterThanOrEqualTo64;
								if(color3Pitch < 64) return kErrorSwizzledRenderTargetColorBuffer3PitchMustBeGreaterThanOrEqualTo64;
								if(depthPitch < 64) return kErrorSwizzledRenderTargetDepthBufferPitchMustBeGreaterThanOrEqualTo64;

								// If Depth Buffer is D24S8
								if(dsfmt == 0x40)
								{
									if(fmt == 0x01) return kErrorSwizzledXrgbZrgb1555WithoutD16S0;
									if(fmt == 0x02) return kErrorSwizzledXrgbOrgb1555WithoutD16S0;
									if(fmt == 0x03) return kErrorSwizzledRgb565WithoutD16S0;
								}
								// If Depth Buffer is D16S0
								else 
								{
									if(fmt == 0x04) return kErrorSwizzledXrgbZrgb8888WithoutD24S8;
									if(fmt == 0x05) return kErrorSwizzledXrgbOrgb8888WithoutD24S8;
									if(fmt == 0x08) return kErrorSwizzledArgb8888WithoutD24S8;
									if(fmt == 0x0E) return kErrorSwizzledXbgrZbgr8888WithoutD24S8;
									if(fmt == 0x0F) return kErrorSwizzledXbgrObgr8888WithoutD24S8;
									if(fmt == 0x10) return kErrorSwizzledAbgr8888WithoutD24S8;
								}
							}

							// Alpha Test Enabled?
							if(contents[0x0304/4])
							{
								if(fmt == 0x0D) return kErrorAlphaTestingWithFp32RenderTarget;
							}

							// Blending Enabled?
							if(contents[0x0310/4] || contents[0x036C/4])
							{
								if((fmt == 0x0C) || (fmt == 0x0D)) return kErrorBlendingWithFp32RenderTarget;
								
								// Signed blend equation?
								U32 equation = contents[0x0320/4];
								if(((equation & 0xF0000000) == 0xF0000000) || ((equation & 0x0000F000) == 0x0000F000))
								{
									if ((fmt == 0x0B) || (fmt == 0x0C) || (fmt == 0x0D)) return kErrorSignedBlendEquationWithFpRenderTarget;
									if ((fmt == 0x03)) return kErrorSignedBlendEquationWith16bitRenderTarget;
								}
							}
							
							// Gamma Corrected Writes Enabled?
							if(contents[0x1FEC/4])
							{
								if(contents[0x1D60/4]&0x40) *outWarnings |= kWarningGammaCorrectedWritesWithoutFp16FragmentProgram;
							}
							
							if(dsfmt != 0x40)
							{
								// Stencil Test Front/Back Enabled?
								if(contents[0x0328/4] || contents[0x0348/4]) return kErrorStencilTestEnabledWithoutStencilBuffer;
								// Is Fp Render Target?
								if ((fmt == 0x0B) || (fmt == 0x0C) || (fmt == 0x0D)) return kErrorDepthBufferMustBeD24S8WithFpRenderTarget;
							}
							
							sanityCheckFlags ^= kCheckRenderTarget;
						}

					}

					drawCount += (value != 0) ? 1 : -1;
					if (drawCount > 1) return kErrorBeginWithoutEnd;
					if (drawCount < 0) return kErrorEndWithoutBegin;
				}
				else if ((reg >= 0x1680) && (reg <= 0x16BC)) sanityCheckFlags |= kCheckVertexArrays;
				else if ((reg == 0x181C) && ((value & 0x7f) != 0)) return kErrorIndexBufferAlignment;
				else if (reg == 0x1820) 
				{
					/* do nothing */
				}
				else if ((reg >= 0xA000) && (reg <= 0xAFFC))
				{
					/* do nothing */
				}
				else if ((reg >= 0x0B80) && (reg <= 0x0BFC))
				{
					/* do nothing */
				}
				else if (reg == 0x1EFC) 
				{
					/* do nothing */
				}
				else if ((reg >= 0x1F00) && (reg <= 0x1F7C))
				{
					/* do nothing */
				}
				else if (reg == 0x0064)
				{
					if ((value & 0x3) != 0) return kErrorSemaphoreAddressAlignment;
					if (value > 0x0FFC) return kErrorSemaphoreAddressOutOfRange;
				}
				else if ((reg >= 0x018C) && (reg <= 0x01B8))
				{
					if (reg == 0x018C) sanityCheckFlags |= kCheckRenderTarget;
					else if (reg == 0x0194) sanityCheckFlags |= kCheckRenderTarget;
					else if (reg == 0x0198) sanityCheckFlags |= kCheckRenderTarget;
					else if (reg == 0x01B4) sanityCheckFlags |= kCheckRenderTarget;
					else if (reg == 0x01B8) sanityCheckFlags |= kCheckRenderTarget;
				}
				else if ((reg >= 0x0208) && (reg <= 0x028C))
				{
					sanityCheckFlags |= kCheckRenderTarget;

					if ((reg == 0x0208) && ((value & 0xF000) != 0))
					{
						U32 fmt = value & 0x1F;
						if ((fmt == 0x0B) || (fmt == 0x0C) || (fmt == 0x0D)) 
							return kErrorMultisampledFpRenderTarget;
					}
					if ((reg == 0x0210) && ((value & 0x7f) != 0)) return kErrorRenderTargetColorBuffer0Alignment;
					if ((reg == 0x0214) && ((value & 0x7f) != 0)) return kErrorRenderTargetDepthBuffer0Alignment;
					if ((reg == 0x0218) && ((value & 0x7f) != 0)) return kErrorRenderTargetColorBuffer1Alignment;
					if ((reg == 0x0288) && ((value & 0x7f) != 0)) return kErrorRenderTargetColorBuffer2Alignment;
					if ((reg == 0x028C) && ((value & 0x7f) != 0)) return kErrorRenderTargetColorBuffer3Alignment;
				}
				else if ((reg >= 0x0304) && (reg <= 0x03BC))
				{
					if (reg == 0x0304) sanityCheckFlags |= kCheckRenderTarget;
					else if (reg == 0x0310) sanityCheckFlags |= kCheckRenderTarget;
					else if (reg == 0x0320) sanityCheckFlags |= kCheckRenderTarget;
					else if (reg == 0x036C) sanityCheckFlags |= kCheckRenderTarget;
					else if (reg == 0x03BC) sanityCheckFlags |= kCheckRenderTarget;
				}
				else if ((reg >= 0x08C0) && (reg <= 0x08C4)) 
				{
					sanityCheckFlags |= kCheckScissor;
					if((value & 0xFFFF) + (value >> 16) > 0x1000) return kErrorInvalidScissor;
				}
				else if (reg == 0x08E4) 
				{
					sanityCheckFlags |= kCheckRenderTarget;
					if (value & 0x3C) return kErrorFragmentProgramAlignment;
				}
				else if ((reg >= 0x0900) && (reg <= 0x097C))
				{
					if ((reg == 0x0900) && ((value & 0x7f) != 0)) return kErrorVertexTexture0Alignment;
					else if ((reg == 0x0920) && ((value & 0x7f) != 0)) return kErrorVertexTexture1Alignment;
					else if ((reg == 0x0940) && ((value & 0x7f) != 0)) return kErrorVertexTexture2Alignment;
					else if ((reg == 0x0960) && ((value & 0x7f) != 0)) return kErrorVertexTexture3Alignment;
					sanityCheckFlags |= kCheckVertexTextures;
				}
				else if ((reg >= 0x0A00) && (reg <= 0x0A04))
				{
					sanityCheckFlags |= kCheckScissor;
					if((value & 0xFFFF) + (value >> 16) > 0x1000) return kErrorInvalidViewport;
				}
				else if (reg == 0x0A74) sanityCheckFlags |= kCheckRenderTarget;
				else if (reg == 0x1738) sanityCheckFlags |= kCheckVertexArrays;
				else if ((reg >= 0x1740) && (reg <= 0x177C))
				{
					U32 type = value & 0xF;
					if((type == 0x6) && ((value&0xF0)!=0x10)) return kErrorAttributeFormat111110RequiresCountOfOne;
					if((type == 0x7) && ((value&0xF0)!=0x40)) return kErrorAttributeFormatUByteRequiresCountOfFour;
					sanityCheckFlags |= kCheckVertexArrays;
				}
				else if ((reg == 0x1800) && ((value & 0x3) != 0)) return kErrorWriteReportAlignment;
				else if (reg == 0x1814)
				{
					U32 baseOffset = contents[0x1738/4];
					U32 start = value & 0x00FFFFFF;
					U32 count = value >> 24;

					// Check Vertex array memory overruns
					for(U32F i = 0; i < 16; ++i)
					{
						U32 fmt = contents[0x1740/4+i];
						// enabled? (stride of non-zero)
						if(fmt&0xFF00)
						{
							//U32 divider = fmt >> 16;
							U32 stride = (fmt&0xFF00) >> 8;
							U32 offsetAndContext = contents[0x1680/4+i];
							U32 vaContext = offsetAndContext&0x80000000;
							U32 vaOff = (offsetAndContext&0x7FFFFFFF) + baseOffset + (start+count)*stride;
							// Valid index offset?
							if(!vaContext)
							{
								if(vaOff >= vmSize) return ValidationError(kErrorDrawArraysVertexBuffer0MemoryOverrun+i);
							}
							else if(!TranslateIoOffsetToAddress(vaOff)) return ValidationError(kErrorDrawArraysVertexBuffer0MemoryOverrun+i);
						}
					}
				}
				else if (reg == 0x1838) sanityCheckFlags |= kCheckRenderTarget;
				else if ((reg >= 0x1840) && (reg <= 0x187C)) sanityCheckFlags |= kCheckTextures;
				else if ((reg >= 0x1A00) && (reg <= 0x1BFC))
				{
					if((reg & 0xF)==0)
					{
						if ((reg == 0x1A00) && ((value & 0x7f) != 0)) return kErrorTexture0Alignment;
						if ((reg == 0x1A20) && ((value & 0x7f) != 0)) return kErrorTexture1Alignment;
						if ((reg == 0x1A40) && ((value & 0x7f) != 0)) return kErrorTexture2Alignment;
						if ((reg == 0x1A60) && ((value & 0x7f) != 0)) return kErrorTexture3Alignment;
						if ((reg == 0x1A80) && ((value & 0x7f) != 0)) return kErrorTexture4Alignment;
						if ((reg == 0x1AA0) && ((value & 0x7f) != 0)) return kErrorTexture5Alignment;
						if ((reg == 0x1AC0) && ((value & 0x7f) != 0)) return kErrorTexture6Alignment;
						if ((reg == 0x1AE0) && ((value & 0x7f) != 0)) return kErrorTexture7Alignment;
						if ((reg == 0x1B00) && ((value & 0x7f) != 0)) return kErrorTexture8Alignment;
						if ((reg == 0x1B20) && ((value & 0x7f) != 0)) return kErrorTexture9Alignment;
						if ((reg == 0x1B40) && ((value & 0x7f) != 0)) return kErrorTexture10Alignment;
						if ((reg == 0x1B60) && ((value & 0x7f) != 0)) return kErrorTexture11Alignment;
						if ((reg == 0x1B80) && ((value & 0x7f) != 0)) return kErrorTexture12Alignment;
						if ((reg == 0x1BA0) && ((value & 0x7f) != 0)) return kErrorTexture13Alignment;
						if ((reg == 0x1BC0) && ((value & 0x7f) != 0)) return kErrorTexture14Alignment;
						if ((reg == 0x1BE0) && ((value & 0x7f) != 0)) return kErrorTexture15Alignment;
					}
					sanityCheckFlags |= kCheckTextures;
				}
				else if (reg == 0x1D60) sanityCheckFlags |= kCheckRenderTarget;
				else if (reg == 0x1D6C)
				{
					if ((value & 0xF) != 0) return kErrorSemaphoreSignalAddressAlignment;
					if (value > 0x0FF0) return kErrorSemaphoreSignalAddressOutOfRange;
				}
				else if (reg == 0x1D7C) sanityCheckFlags |= kCheckRenderTarget;
				else if (reg == 0x1D94)
				{
					if(ViewportScissorSanityCheck(contents)) 
						return kErrorScissorMustBeInsideViewport;

					U32 fmt = contents[0x0208/4] & 0x1F;
					if (fmt == 0x09) // B8
					{
						if((contents[0x1D90/4] & 0xFFFFFF00)!=0) *outWarnings |= kWarningClearColorDoesNotMatchRenderTargetFormat;
						if((value & 0x0F)!=0) return kErrorClearDepthBufferOnB8RenderTarget; 
					}
					else if(fmt == 0x0A) // Gb88
					{
						if((contents[0x1D90/4] & 0xFFFF0000)!=0) *outWarnings |= kWarningClearColorDoesNotMatchRenderTargetFormat;
						if((value & 0x0F)!=0) return kErrorClearDepthBufferOnGb88RenderTarget; 
					}
					else if(fmt == kBufferRgb565) // Rgb565
					{
						if((contents[0x1D90/4] & 0xFFFF0000)!=0) *outWarnings |= kWarningClearColorDoesNotMatchRenderTargetFormat;
					}

					if ((value & 0xF0) != 0)
					{
						if ((fmt == 0x0B) || (fmt == 0x0C) || (fmt == 0x0D)) 
							return kErrorClearFloatingPointRenderTarget;
					}
				}
				else if ((reg == 0x1E98) && ((value & 0x3) != 0)) return kErrorConditionalRenderAlignment;
				else if (reg == 0x1FEC) sanityCheckFlags |= kCheckRenderTarget;
				reg += inc;
				*outReg = reg;
				putNext += 4;
				++pPutNext;
			}
		}
	}

	return kErrorNone;
}

void Ice::Render::SingleStepPusher(unsigned int putEnd)
{
	volatile Ice::Render::GpuControl *control = g_gpuControl;

	// WARNING: bar0 should be 0x62000000 on RSX! Please double check!
	//unsigned int bar0 = (unsigned int)((unsigned long)(Ice::Render::TranslateOffsetToAddress(0))) + 0x0E000000;

	unsigned int returnTo = 0;
	unsigned int putNext;
	do
	{
		unsigned int get = control->m_getOffset;
		unsigned int getValue = *(unsigned int *)Ice::Render::TranslateIoOffsetToAddress(get);
		if (returnTo)
			printf("-Reference %08x: g=%08x *g=%08x", Ice::Render::GetReference(), get, getValue);
		else
			printf("Reference %08x: g=%08x *g=%08x", Ice::Render::GetReference(), get, getValue);

		// Single Step the GPU
		if (getValue & 0x20000000) // Jump Short
		{
			putNext = (getValue & ~0x20000000);
			printf("\nJumped To: %08x\n", putNext);
		}
		else if (getValue & 0x00000001) // Jump Long
		{
			putNext = (getValue & ~0x00000001);
			printf("\nJumped To: %08x\n", putNext);
		}
		else if (getValue & 0x00000002) // Call
		{
			putNext = (getValue & ~0x00000002);
			returnTo = get + 4;
			printf("\nCalled: %08x\n", putNext);
		}
		else if (getValue & 0x00020000) // Return
		{
			putNext = returnTo;
			returnTo = 0;
			printf("\nReturned: %08x\n", putNext);
		}
		else
		{
			U32F const numWords = (getValue >> 18) & 0x7FF;
			for (unsigned long ii = 1; ii <= numWords; ++ii)
			{
				U32 v = *(U32 *)Ice::Render::TranslateIoOffsetToAddress(get + ii*4);
				printf(" %08x", v);
			}
			printf("\n");
			putNext = get + (numWords * 4) + 4;
		}
		control->m_putOffset = putNext;

		//int numWait = 0;
		while (control->m_putOffset != control->m_getOffset)
		{
			asm __volatile__ ("sync");
			/*
			if ((++numWait == 0x1000) && (*(unsigned int*)(0x00400104L + bar0) != 0))
			{
				printf("Gpu Exception Raised!\n");
				printf("%08x\n", *(unsigned int*)(0x00400104L + bar0)); // 0x04000000 == protection fault
				printf("%08x\n", *(unsigned int*)(0x00400108L + bar0));
				printf("%08x\n", *(unsigned int*)(0x0040010CL + bar0));
				printf("%08x\n", *(unsigned int*)(0x00400110L + bar0));
				printf("%08x\n", *(unsigned int*)(0x0040080CL + bar0)); // Address of pixel being written
				printf("%08x\n", *(unsigned int*)(0x00400810L + bar0)); // descriptor?
				printf("%08x\n", *(unsigned int*)(0x00400800L + bar0));
			}
			*/
		}
		/*
		if (*(unsigned int*)(0x00400104L + bar0) != 0)
		{
			printf("Gpu Exception Raised!\n");
			printf("%08x\n", *(unsigned int*)(0x00400104L + bar0)); // 0x04000000 == protection fault, 0x01000000 == ???
			printf("%08x\n", *(unsigned int*)(0x00400108L + bar0));
			printf("%08x\n", *(unsigned int*)(0x0040010CL + bar0));
			printf("%08x\n", *(unsigned int*)(0x00400110L + bar0));
			printf("%08x\n", *(unsigned int*)(0x0040080CL + bar0)); // Address of pixel being written
			printf("%08x\n", *(unsigned int*)(0x00400810L + bar0)); // descriptor?
			printf("%08x\n", *(unsigned int*)(0x00400800L + bar0));
		}
		*/

		asm __volatile__ ("sync");
	} while (putNext != putEnd);
}

#endif // __SPU__
