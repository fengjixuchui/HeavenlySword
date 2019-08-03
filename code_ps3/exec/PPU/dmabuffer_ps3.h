//----------------------------------------------------------------------------------------
//! 
//! \filename exec\ppu\dmabuffer_ps3.h
//! 
//----------------------------------------------------------------------------------------
#if !defined( EXEC_PPU_DMABUFFER_PS3_H )
#define EXEC_PPU_DMABUFFER_PS3_H

#include "core/mem.h"

//!
//!	DO NOT MAKE THIS A BASE CLASS OR A DERIVED CLASS OF ANYTHING, IT MUST BE A POD CLASS.
//!
class DMABuffer
{
	public:
		//! Slightly dirty hack:
		//!		The passed in pointer below is marked as const
		//!		because then we can pass in const-pointers and
		//!		non-const-pointers without hastle. However, we
		//!		const-cast away the constness when storing. We
		//!		could get around this by using separate DMA
		//!		buffer classes for "input to SPU task" buffers
		//!		and "(input and) output from SPU task" buffers,
		//!		but this seemed quicker for now.
		DMABuffer( const void *pPtr, uint32_t iSize );

#		ifdef __SPU__
		//! ctor for SPU with an EA address
		DMABuffer( const uint32_t iEA, uint32_t iSize );
#		endif

		//!	Default ctor - just creates a zero sizes buffer pointing at NULL.
		DMABuffer();

		//! Returns the memory this DMA buffer represents.
#		ifndef __SPU__
			void *Get() const
			{
				return m_pAddr;
			}
			//! returns the original size (the buffer may be bigger due to alignment) 
			uint32_t GetSize() const
			{
				return m_iSize;
			}
#		else
			void *GetLS() const
			{
				return (void *)( (uintptr_t)m_pAddr & 0x07ffffff );
			}

			int32_t GetDmaID() const
			{
				return ( (uintptr_t)m_pAddr >> ( 32 - 5 ) ) & 31;
			}

			//! note isn't valid for post-fixup DMABUFFER 
			//! for PPU generated DMABuffer
			//!		pre fixup its EA 
			//!		post fixup its LS | DmaID
			//! for SPU created DMAbuffer 
			//!		it will be EA
			uint32_t GetEA() const { return (uint32_t) m_pAddr; }

			//! returns the original size (the buffer may be bigger due to alignment) 
			uint32_t GetSize() const
			{
				// top bit tell us where pAddr is EA or LS | DmaID
				return m_iSize & 0x7fffffff;
			}
			//! will GetEA get you a EA addr or a munged LS one...
			//! note in our standard system g_DMAEffectiveAddresses will have 
			//! the original EA which you can get if this return false
			bool IsAddrEA()
			{
				return !(m_iSize & 0x80000000);
			}
#		endif

#		ifdef __SPU__
			void Set( void *addr, int32_t dma_id )
			{
				ntError( dma_id >= 0 && dma_id < 32 );
				ntError( ( (uintptr_t)addr & 0x07ffffff ) == (uintptr_t)addr );		// Make sure we can stuff the dma_id into the top 5 bits of the address.
				m_pAddr = (void *)( (uintptr_t)addr | ( dma_id << (32 - 5) ) );		// Put the dma_id into the top 5 bits.
				m_iSize |= 0x80000000;
			}
#		endif



		//! returns the required number of 1KB pages to store this buffer into in LS.
		uint32_t GetRequiredNumPages() const
		{
			return ROUND_POW2( m_iSize, 1024 ) >> 10;
		}

	public:
		//!
		//!	Static allocation function - memory for DMAs can use this to give correct alignment.
		//!	If num_bytes is not a multiple of DefaultDMASizeAlignment, then it will be rounded up.
		//!
		static void *	DMAAlloc	( size_t num_bytes, size_t *actual_allocated_size = NULL, Mem::MEMORY_CHUNK chunk = Mem::MC_MISC, uint32_t iAlignment = DefaultDMAAlignment );
		static void		DMAFree		( const void *mem_ptr, Mem::MEMORY_CHUNK chunk = Mem::MC_MISC );

		//----------------------------------------------------------------------------------------
		//! 
		//! returns the number of bytes that will by a particular DMAAlloc(N)
		//! 
		//----------------------------------------------------------------------------------------
		static size_t	DMAAllocSize( size_t num_bytes, uint32_t iAlignment = DefaultDMASizeAlignment  )
		{
			return Util::Align( num_bytes, iAlignment );
		}

	private:
		void *					m_pAddr;	//!< address that hold this DMA buffer
		mutable uint32_t		m_iSize;	//!< original (unadjusted for alignment size)

		static const size_t		DefaultDMAAlignment = 0x10;
		static const size_t		DefaultDMASizeAlignment = 0x10;

		static_assert_in_class( ( DefaultDMAAlignment != 0 ) && ( DefaultDMAAlignment & ( DefaultDMAAlignment - 1 ) ) == 0, DefaultDMAAlignment_must_be_a_non_zero_power_of_two );
		static_assert_in_class( ( DefaultDMASizeAlignment != 0 ) && ( DefaultDMASizeAlignment & ( DefaultDMASizeAlignment - 1 ) ) == 0, DefaultDMASizeAlignment_must_be_a_non_zero_power_of_two );
};

//**************************************************************************************
//
// INLINEd basically cos its a pain in the arse to show cpp between ppu and spu 
//	
//**************************************************************************************

//----------------------------------------------------------------------------------------
//! 
//! ctor
//! 
//----------------------------------------------------------------------------------------
inline DMABuffer::DMABuffer()
:	m_pAddr( NULL )
,	m_iSize( 0 )
{}

//----------------------------------------------------------------------------------------
//! 
//! ctor
//! 
//----------------------------------------------------------------------------------------
inline DMABuffer::DMABuffer( const void* pPtr, uint32_t iSize )
:	m_pAddr( const_cast< void * >( pPtr ) )
,	m_iSize( iSize )
{
	ntError_p( ( (intptr_t)m_pAddr & (DefaultDMASizeAlignment-1) ) == 0, ("Invalid DMA alignment - must be a power of 2 >= DefaultDMAAlignment.") );
}

#		ifdef __SPU__
//----------------------------------------------------------------------------------------
//! 
//! ctor
//! 
//----------------------------------------------------------------------------------------
inline DMABuffer::DMABuffer( const uint32_t iEA, uint32_t iSize )
:	m_pAddr( reinterpret_cast< void* >(iEA ) )
,	m_iSize( iSize )
{
	ntError_p( ( (intptr_t)m_pAddr & (DefaultDMASizeAlignment-1) ) == 0, ("Invalid DMA alignment - must be a power of 2 >= DefaultDMAAlignment.") );
}
#	endif

#endif // EXEC_PPU_DMABUFFER_PS3_H
