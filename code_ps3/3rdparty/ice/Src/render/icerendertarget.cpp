/*
 * Copyright (c) 2003-2006 Naughty Dog, Inc.
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#include "icerender.h"

using namespace Ice::Render;

void RenderTarget::Init(U32 width, U32 height, ColorBufferFormat colorFormat, DepthStencilBufferFormat depthFormat, MultisampleMode multisample, U32 flags)
{
	U32 targetFormat = depthFormat | colorFormat | (multisample << 12);

	if (flags & kTargetSwizzled)
	{
		ICE_ASSERTF(multisample == kMultisampleNone, ("Swizzled render targets cannot be multisampled."));
		ICE_ASSERTF(colorFormat != kBufferRgba16f, ("Swizzled render targets cannot have a floating point color buffer."));
		ICE_ASSERTF(colorFormat != kBufferRgba32f, ("Swizzled render targets cannot have a floating point color buffer."));
		ICE_ASSERTF(colorFormat != kBufferR32f, ("Swizzled render targets cannot have a floating point color buffer."));

		U32 logWidth = 31 - Cntlzw(width);
		U32 logHeight = 31 - Cntlzw(height);	

		targetFormat |= 0x00000200 | (logWidth << 16) | (logHeight << 24);
	}
	else
	{
		targetFormat |= 0x00000100;
	}

	m_targetFormat = targetFormat;
	m_targetLeftWidth = width << 16;
	m_targetTopHeight = height << 16;
	m_targetAuxHeight = height | 0x00001000;
	m_targetOrigin = 0;

	m_colorOffset[0] = 0;
	m_colorOffset[1] = 0;
	m_colorOffset[2] = 0;
	m_colorOffset[3] = 0;
	m_depthOffset = 0;

	m_colorPitch[0] = 64;
	m_colorPitch[1] = 64;
	m_colorPitch[2] = 64;
	m_colorPitch[3] = 64;
	m_depthPitch = 64;

	m_displayIndex = 0xFFFFFFFF;
}

void RenderTarget::SetColorAndDepthBuffers(
	U32 colorCount, U32 const *colorOffsets, RenderTargetContext const *colorContexts, U32 const *colorPitches,
	U32 depthOffset, RenderTargetContext depthContext, U32 depthPitch)
{
	ICE_ASSERTF(colorCount > 0, ("At least one color buffer must be specified."));

	switch(colorCount)
	{
		case 4:
			ICE_ASSERTF((colorOffsets[3] & 0x7F) == 0, ("Misaligned color buffer."));
			ICE_ASSERTF(!(((m_targetFormat&0x0F00)==0x0200) && (colorPitches[3] < 64)), ("Swizzled Render Targets require a pitch >= 64."));
			ICE_ASSERTF(!(((m_targetFormat&0x0F00)==0x0200) && (colorPitches[3]&(colorPitches[3]-1))), ("Swizzled Render Targets require a power of two pitch."));
			U32 const offset3 = colorOffsets[3];
			U32 const pitch3 = colorPitches[3];
			m_colorOffset[3] = offset3;
			RenderTargetContext const context3 = colorContexts[3];
			m_colorPitch[3] = pitch3;
			m_colorContext[3] = context3;
		case 3:
			ICE_ASSERTF((colorOffsets[2] & 0x7F) == 0, ("Misaligned color buffer."));
			ICE_ASSERTF(!(((m_targetFormat&0x0F00)==0x0200) && (colorPitches[2] < 64)), ("Swizzled Render Targets require a pitch >= 64."));
			ICE_ASSERTF(!(((m_targetFormat&0x0F00)==0x0200) && (colorPitches[2]&(colorPitches[2]-1))), ("Swizzled Render Targets require a power of two pitch."));
			U32 const offset2 = colorOffsets[2];
			U32 const pitch2 = colorPitches[2];
			m_colorOffset[2] = offset2;
			RenderTargetContext const context2 = colorContexts[2];
			m_colorPitch[2] = pitch2;
			m_colorContext[2] = context2;
		case 2:
			ICE_ASSERTF((colorOffsets[1] & 0x7F) == 0, ("Misaligned color buffer."));
			ICE_ASSERTF(!(((m_targetFormat&0x0F00)==0x0200) && (colorPitches[1] < 64)), ("Swizzled Render Targets require a pitch >= 64."));
			ICE_ASSERTF(!(((m_targetFormat&0x0F00)==0x0200) && (colorPitches[1]&(colorPitches[1]-1))), ("Swizzled Render Targets require a power of two pitch."));
			U32 const offset1 = colorOffsets[1];
			U32 const pitch1 = colorPitches[1];
			m_colorOffset[1] = offset1;
			RenderTargetContext const context1 = colorContexts[1];
			m_colorPitch[1] = pitch1;
			m_colorContext[1] = context1;
		case 1:
		default:
			ICE_ASSERTF((colorOffsets[0] & 0x7F) == 0, ("Misaligned color buffer."));
			ICE_ASSERTF(!(((m_targetFormat&0x0F00)==0x0200) && (colorPitches[0] < 64)), ("Swizzled Render Targets require a pitch >= 64."));
			ICE_ASSERTF(!(((m_targetFormat&0x0F00)==0x0200) && (colorPitches[0]&(colorPitches[0]-1))), ("Swizzled Render Targets require a power of two pitch."));
			U32 const offset = colorOffsets[0];
			U32 const pitch = colorPitches[0];
			m_colorOffset[0] = offset;
			RenderTargetContext const context = colorContexts[0];
			m_colorPitch[0] = pitch;
			m_colorContext[0] = context;

			switch(colorCount)
			{
				case 1:
					m_colorOffset[1] = offset;
					m_colorPitch[1] = pitch;
					m_colorContext[1] = context;
				case 2:
					m_colorOffset[2] = offset;
					m_colorPitch[2] = pitch;
					m_colorContext[2] = context;
				case 3:
					m_colorOffset[3] = offset;
					m_colorPitch[3] = pitch;
					m_colorContext[3] = context;
			}

			ICE_ASSERTF((depthOffset & 0x7F) == 0, ("Misaligned depth buffer."));
			ICE_ASSERTF(!(((m_targetFormat&0x0F00)==0x0200) && (depthPitch < 64)), ("Swizzled Render Targets require a pitch >= 64."));
			ICE_ASSERTF(!(((m_targetFormat&0x0F00)==0x0200) && (depthPitch&(depthPitch-1))), ("Swizzled Render Targets require a power of two pitch."));
			m_depthOffset = depthOffset;
			m_depthPitch = depthPitch;
			m_depthContext = depthContext;
	}
}

void RenderTarget::SetColorAndDepthBuffers(
	U32 colorOffset, RenderTargetContext colorContext, U32 colorPitch, 
	U32 depthOffset, RenderTargetContext depthContext, U32 depthPitch)
{
	ICE_ASSERTF((colorOffset & 0x7F) == 0, ("Misaligned color buffer."));
	ICE_ASSERTF(!(((m_targetFormat&0x0F00)==0x0200) && (colorPitch < 64)), ("Swizzled Render Targets require a pitch >= 64."));
	ICE_ASSERTF(!(((m_targetFormat&0x0F00)==0x0200) && (colorPitch&(colorPitch-1))), ("Swizzled Render Targets require a power of two pitch."));
	m_colorOffset[0] = colorOffset;
	m_colorOffset[1] = colorOffset;
	m_colorOffset[2] = colorOffset;
	m_colorOffset[3] = colorOffset;
	m_colorPitch[0] = colorPitch;
	m_colorPitch[1] = colorPitch;
	m_colorPitch[2] = colorPitch;
	m_colorPitch[3] = colorPitch;
	m_colorContext[0] = colorContext;
	m_colorContext[1] = colorContext;
	m_colorContext[2] = colorContext;
	m_colorContext[3] = colorContext;

	ICE_ASSERTF((depthOffset & 0x7F) == 0, ("Misaligned depth buffer."));
	ICE_ASSERTF(!(((m_targetFormat&0x0F00)==0x0200) && (depthPitch < 64)), ("Swizzled Render Targets require a pitch >= 64."));
	ICE_ASSERTF(!(((m_targetFormat&0x0F00)==0x0200) && (depthPitch&(depthPitch-1))), ("Swizzled Render Targets require a power of two pitch."));
	m_depthOffset = depthOffset;
	m_depthPitch = depthPitch;
	m_depthContext = depthContext;
}

void RenderTarget::SetColorNoDepthBuffers(U32 colorCount, U32 const *colorOffsets, RenderTargetContext const *colorContexts, U32 const *colorPitches)
{
	ICE_ASSERTF(colorCount > 0, ("At least one color buffer must be specified."));

	switch(colorCount)
	{
		case 4:
			ICE_ASSERTF((colorOffsets[3] & 0x7F) == 0, ("Misaligned color buffer."));
			ICE_ASSERTF(!(((m_targetFormat&0x0F00)==0x0200) && (colorPitches[3] < 64)), ("Swizzled Render Targets require a pitch >= 64."));
			ICE_ASSERTF(!(((m_targetFormat&0x0F00)==0x0200) && (colorPitches[3]&(colorPitches[3]-1))), ("Swizzled Render Targets require a power of two pitch."));
			U32 const offset3 = colorOffsets[3];
			U32 const pitch3 = colorPitches[3];
			m_colorOffset[3] = offset3;
			RenderTargetContext const context3 = colorContexts[3];
			m_colorPitch[3] = pitch3;
			m_colorContext[3] = context3;
		case 3:
			ICE_ASSERTF((colorOffsets[2] & 0x7F) == 0, ("Misaligned color buffer."));
			ICE_ASSERTF(!(((m_targetFormat&0x0F00)==0x0200) && (colorPitches[2] < 64)), ("Swizzled Render Targets require a pitch >= 64."));
			ICE_ASSERTF(!(((m_targetFormat&0x0F00)==0x0200) && (colorPitches[2]&(colorPitches[2]-1))), ("Swizzled Render Targets require a power of two pitch."));
			U32 const offset2 = colorOffsets[2];
			U32 const pitch2 = colorPitches[2];
			m_colorOffset[2] = offset2;
			RenderTargetContext const context2 = colorContexts[2];
			m_colorPitch[2] = pitch2;
			m_colorContext[2] = context2;
		case 2:
			ICE_ASSERTF((colorOffsets[1] & 0x7F) == 0, ("Misaligned color buffer."));
			ICE_ASSERTF(!(((m_targetFormat&0x0F00)==0x0200) && (colorPitches[1] < 64)), ("Swizzled Render Targets require a pitch >= 64."));
			ICE_ASSERTF(!(((m_targetFormat&0x0F00)==0x0200) && (colorPitches[1]&(colorPitches[1]-1))), ("Swizzled Render Targets require a power of two pitch."));
			U32 const offset1 = colorOffsets[1];
			U32 const pitch1 = colorPitches[1];
			m_colorOffset[1] = offset1;
			RenderTargetContext const context1 = colorContexts[1];
			m_colorPitch[1] = pitch1;
			m_colorContext[1] = context1;
		case 1:
		default:
			ICE_ASSERTF((colorOffsets[0] & 0x7F) == 0, ("Misaligned color buffer."));
			ICE_ASSERTF(!(((m_targetFormat&0x0F00)==0x0200) && (colorPitches[0] < 64)), ("Swizzled Render Targets require a pitch >= 64."));
			ICE_ASSERTF(!(((m_targetFormat&0x0F00)==0x0200) && (colorPitches[0]&(colorPitches[0]-1))), ("Swizzled Render Targets require a power of two pitch."));
			U32 const offset = colorOffsets[0];
			U32 const pitch = colorPitches[0];
			m_colorOffset[0] = offset;
			RenderTargetContext const context = colorContexts[0];
			m_colorPitch[0] = pitch;
			m_colorContext[0] = context;

			switch(colorCount)
			{
				case 1:
					m_colorOffset[1] = offset;
					m_colorPitch[1] = pitch;
					m_colorContext[1] = context;
				case 2:
					m_colorOffset[2] = offset;
					m_colorPitch[2] = pitch;
					m_colorContext[2] = context;
				case 3:
					m_colorOffset[3] = offset;
					m_colorPitch[3] = pitch;
					m_colorContext[3] = context;
			}

			m_depthOffset = offset;
			m_depthPitch = pitch;
			m_depthContext = context;
	}
}

void RenderTarget::SetColorNoDepthBuffers(U32 colorOffset, RenderTargetContext colorContext, U32 colorPitch)
{
	ICE_ASSERTF((colorOffset & 0x7F) == 0, ("Misaligned color buffer."));
	ICE_ASSERTF(!(((m_targetFormat&0x0F00)==0x0200) && (colorPitch < 64)), ("Swizzled Render Targets require a pitch >= 64."));
	ICE_ASSERTF(!(((m_targetFormat&0x0F00)==0x0200) && (colorPitch&(colorPitch-1))), ("Swizzled Render Targets require a power of two pitch."));
	RenderTargetContext const context = colorContext;
	m_colorOffset[0] = colorOffset;
	m_colorOffset[1] = colorOffset;
	m_colorOffset[2] = colorOffset;
	m_colorOffset[3] = colorOffset;
	m_depthOffset = colorOffset;
	m_colorPitch[0] = colorPitch;
	m_colorPitch[1] = colorPitch;
	m_colorPitch[2] = colorPitch;
	m_colorPitch[3] = colorPitch;
	m_depthPitch = colorPitch;
	m_colorContext[0] = context;
	m_colorContext[1] = context;
	m_colorContext[2] = context;
	m_colorContext[3] = context;
	m_depthContext = context;
}

void RenderTarget::SetDepthNoColorBuffers(U32 depthOffset, RenderTargetContext depthContext, U32 depthPitch)
{
	ICE_ASSERTF((depthOffset & 0x7F) == 0, ("Misaligned depth buffer."));
	ICE_ASSERTF(!(((m_targetFormat&0x0F00)==0x0200) && (depthPitch < 64)), ("Swizzled Render Targets require a pitch >= 64."));
	ICE_ASSERTF(!(((m_targetFormat&0x0F00)==0x0200) && (depthPitch&(depthPitch-1))), ("Swizzled Render Targets require a power of two pitch."));
	RenderTargetContext const context = depthContext;
	m_colorOffset[0] = depthOffset;
	m_colorOffset[1] = depthOffset;
	m_colorOffset[2] = depthOffset;
	m_colorOffset[3] = depthOffset;
	m_depthOffset = depthOffset;
	m_colorPitch[0] = depthPitch;
	m_colorPitch[1] = depthPitch;
	m_colorPitch[2] = depthPitch;
	m_colorPitch[3] = depthPitch;
	m_depthPitch = depthPitch;
	m_colorContext[0] = context;
	m_colorContext[1] = context;
	m_colorContext[2] = context;
	m_colorContext[3] = context;
	m_depthContext = context;
}
