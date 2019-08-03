/*
 * Copyright (c) 2003-2006 Naughty Dog, Inc.
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#include "icebase.h"
#include "icebitextract.h"
#include "icevertexprogram.h"

using namespace Ice::Render;

void VertexProgram::Move(U32F toSlot)
{
	U32F fromSlot = m_instructionSlot;
	m_instructionSlot = toSlot;
	U32F slotDelta = toSlot - fromSlot;
	U32 instrs = (U32)this + m_microcodeOffset;
	
	// Patch the vertex program branches.
	U16 const *__restrict patchTable = GetPatchTable();
	U32F patchCount = m_patchCount;
	for(U32F i = 0; i < patchCount; ++i)
	{
		U16 patchOffset = patchTable[i];
		U32 d0 = *(U32*)(instrs + patchOffset + 8);
		U32 d1 = *(U32*)(instrs + patchOffset + 12);
		U32F slot = ((d0 & 0x3F) << 3) | (d1 >> 29);
		slot += slotDelta;
		ICE_ASSERTF(slot < 512, ("Vertex Program branch targets must be less than slot 512."));
#ifdef __SPU__
		d0 = (d0 & ~0x3F) | ((slot << 29) & 0x3F);
		d1 = (d1 & ~0xE0000000) | ((slot << 29) & 0xE0000000);
#else
		d0 = Rlwimi(d0, slot, 29, 26, 31);
		d1 = Rlwimi(d1, slot, 29, 0, 2);
#endif
		*(U32*)(instrs + patchOffset + 8) = d0;
		*(U32*)(instrs + patchOffset + 12) = d1;
	}

	ICE_ASSERT(m_patchCount || (m_instructionSlot + m_instructionCount <= 544));
	ICE_ASSERTF(!m_patchCount || (m_instructionSlot + m_instructionCount <= 512), ("Vertex programs with branching must exist below 512 instruction slots."));
}

