/*
 * Copyright (c) 2003-2006 Naughty Dog, Inc. 
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#ifndef ICE_VERTEXPROGRAM_H
#define ICE_VERTEXPROGRAM_H

namespace Ice
{
	namespace Render
	{
#ifdef __SPU__
		struct ICE_ALIGN(16) VertexProgram
#else
		struct VertexProgram
#endif
		{
			//! The number of 16-byte instruction words in the program.
			/*! Offset: 0x00 */
			U16 m_instructionCount;
			//! The index of the hardware slot at which the first instruction is stored.
			/*! Offset: 0x02 */
			U16 m_instructionSlot;
			//! Defines the used inputs
			/*! Offset: 0x04 */
			U32 m_vertexAttributeMask;
			//! Defines the used outputs
			/*! Offset: 0x08 */
			U32 m_vertexResultMask;
			//! Defines instruction execution limits (infinite loop protection)
			/*! Offset: 0x0C*/
			U32 m_vertexLimits;
			//! Offset to branch instruction patch table, in bytes, relative to the beginning of this struct.			
			/*! Offset: 0x10*/
			U16 m_patchTableOffset;
			//! Offset to microcode, in bytes, relative to beginning of this struct.
			/*! Offset: 0x12*/
			U16 m_microcodeOffset;
			//! The number of branch instructions that need to be patched when the program is bound.
			/*! Offset: 0x14*/
			U16 m_patchCount;
			//! The number of compile-time constants that need to be set when the program is bound.
			/*! Offset: 0x16*/
			U16 m_constantCount;
			//! Variable-sized literal constant index table.
			/*! Offset: 0x18*/
			U32 m_index[1];
			
			//! Literal constant vectors follow on 16-byte boundary.
			const float *GetConstantTable() const
			{
				U32 offset = (sizeof(VertexProgram) + 11 + m_constantCount * 4) & ~15;
				return (reinterpret_cast<const float *>(reinterpret_cast<const char *>(this) + offset));
			}
			
			//! Microcode follows on 16-byte boundary.
			const U32 *GetMicrocode() const
			{
				return (reinterpret_cast<const U32 *>(reinterpret_cast<const char *>(this) + m_microcodeOffset));
			}
			
			//! Branch patch table follows on 16-byte boundary.
			const U16 *GetPatchTable() const
			{
				return (reinterpret_cast<const U16 *>(reinterpret_cast<const char *>(this) + m_patchTableOffset));
			}

			//! Moves a vertex program from the current m_instructionSlot to the specified m_instructionSlot. 
			/*! This can be used in conjunction with CommandContext::SelectVertexProgram() to have multiple
			    programs resident in memory at a time allowing a quick switch between resident programs.
			    WARNING: Programs with branches must exist below slot 512! This is a hardware limitation.
			    \param toSlot  This is the hardware instruction slot to re-org the program to. 0-544. 
			*/
			void Move(U32F toSlot);
		};
	}
}

#endif // ICE_VERTEXPROGRAM_H
