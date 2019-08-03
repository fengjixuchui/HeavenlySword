/*
 * Copyright (c) 2003-2006 Naughty Dog, Inc. 
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#ifndef ICE_FRAGMENTPROGRAM_H
#define ICE_FRAGMENTPROGRAM_H



#if ICE_COMPILER_VISUALC
	#pragma warning(disable:4200)	// zero-sized array
#endif

namespace Ice
{
	namespace Render 
	{
		//! Data for patching pixel shader constants.
		struct PatchData
		{
			//! The number of locations that need to be patched.
			U16 m_count;
			//! A table of instruction indexes pointing to the patch locations.
			U16 m_index[0];
		};

		/*! Patch data follows, then microcode (on 16-byte boundary) */
#ifdef __SPU__
		struct ICE_ALIGN(16) FragmentProgram
#else
		struct FragmentProgram
#endif
		{
			//! The microcode size, in bytes.
			/*! Offset: 0x00 */
			U32 m_microcodeSize;
			//! The mask of texcoords that have centroid sampling enabled. Coordinate 0 is the LSB.
			/*! Offset: 0x04 */
			U16 m_centroidMask;
			//! The mask of texcoords that are 2D only. Coordinate 0 is the LSB.
			/*! Offset: 0x06 */
			U16 m_texcoordMask;
			//! Packed information about various state.
			/*! Offset: 0x08 */
			U32 m_control;
			//! The offset of the microcode in the context. LSB is the context Ice::Render::FragmentProgramContext.
			/*! Offset: 0x0C */
			U32 m_offsetAndContext;
			//! Offset to microcode, in bytes, relative to beginning of this struct.
			/*! Offset: 0x10 */
			U32 m_microcodeOffset;
			//! Number of immediate patches ('fragment program constants').
			/*! Offset: 0x14 */
			U32 m_patchCount;
			//! Array of byte offsets to patch data, relative to beginning of this struct.
			/*! Offset: 0x18 */
			U32 m_patchData[0];
			
			const U32 *GetMicrocode() const
			{
				return (reinterpret_cast<const U32 *>((U32F)this + m_microcodeOffset));
			}
			
			const PatchData *GetPatchData(U32 index) const
			{
				return (reinterpret_cast<const PatchData *>((U32F)this + m_patchData[index]));
			}
			
			FragmentProgramContext GetContext() const
			{
				return FragmentProgramContext(m_offsetAndContext & 0x3);
			}
			
			U32 GetRegisterCount() const
			{
				return m_control >> 24;
			}

			void SetRegisterCount(U32 registerCount)
			{
				m_control = (m_control & 0x00FFFFFF) | (registerCount << 24);
			}
		};
	}
}

#endif // ICE_FRAGMENTPROGRAM_H
