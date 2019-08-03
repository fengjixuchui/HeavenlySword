/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		Helper functions to run dmas in an interrupt safe manner
**/
//--------------------------------------------------------------------------------------------------

#ifndef WWS_JOB_JOB_SPU_DMA_H
#define WWS_JOB_JOB_SPU_DMA_H

//--------------------------------------------------------------------------------------------------

#include <wwsbase/types.h>
#include <jobsystem/helpers.h>
#include <spu_mfcio.h>
#include <jobapi/spuinterrupts.h>

//--------------------------------------------------------------------------------------------------

typedef struct JobDmaListElement
{
	U32	m_size;
	U32	m_eaLo;
} JobDmaListElement;

//--------------------------------------------------------------------------------------------------

enum
{
	//User dma tag are all in the range 8 to 31 inclusive
	kMinValidUserTag	= 8,
	kMaxValidUserTag	= 31,

	//User should never work with any of these tags
	kInvalidTagMask		= 0x000000FF,
};

//--------------------------------------------------------------------------------------------------

#define JobDmaNormalAssert(ls,ea,size,tag) \
	WWSJOB_ASSERT((((uintptr_t)(ls) & 0xf) == 0)	& \
					(((uintptr_t)(ea) & 0xf) == 0)	& \
					(((size) & 0xf) == 0)			& \
					((size) <= (16<<10))			& \
					((tag) <= kMaxValidUserTag)		& \
					((tag) >= kMinValidUserTag))

#define JobDmaListAssert(ls,la,lsize,tag) \
	WWSJOB_ASSERT((((uintptr_t)(ls) & 0xf) == 0)	& \
					(((uintptr_t)(la) &   7) == 0)	& \
					(((lsize) & 7) == 0)			& \
					((lsize) <= (16<<10))			& \
					((tag) <= kMaxValidUserTag)		& \
					((tag) >= kMinValidUserTag))

#define JobDmaAtomicAssert(ls,ea) \
	WWSJOB_ASSERT((((uintptr_t)(ls) & 0x7f) == 0)	& \
					(((uintptr_t)(ea) & 0x7f) == 0))

#define JobDmaPutqllucAssert(ls,ea,tag) \
	WWSJOB_ASSERT((((uintptr_t)(ls) & 0x7f) == 0)	& \
					(((uintptr_t)(ea) & 0x7f) == 0)	& \
					((tag) <= kMaxValidUserTag)		& \
					((tag) >= kMinValidUserTag))

#define JobDmaTagMaskAssert( tagmask ) \
	WWSJOB_ASSERT( ((tagmask) & kInvalidTagMask) == 0 )

//--------------------------------------------------------------------------------------------------

extern void JobDmaLargeCmd( uintptr_t ls, U32 ea, U32 size, U32 tag, U32 cmd );

//--------------------------------------------------------------------------------------------------

inline void JobDmaCmd( volatile void* ls, U32 ea, U32 size, U32 tag, U32 cmd )
{
#ifdef PRESERVE_INTERRUPT_STATUS
	Bool32 enabled = AreInterruptsEnabled();
	DisableInterrupts();
	spu_mfcdma32( ls, ea, size, tag, cmd );
	if(enabled)
		EnableInterrupts();
#else
	WWSJOB_ASSERT( AreInterruptsEnabled() );
	DisableInterrupts();
	spu_mfcdma32( ls, ea, size, tag, cmd );
	EnableInterrupts();
#endif
}

//--------------------------------------------------------------------------------------------------

inline void JobDmaPut( const volatile void* ls, U32 ea, U32 size, U32 tag )
{
	JobDmaNormalAssert( ls, ea, size, tag );
	JobDmaCmd( const_cast<volatile void*>(ls), ea, size, tag, MFC_CMD_WORD(0,0,MFC_PUT_CMD) );
}

//--------------------------------------------------------------------------------------------------

inline void JobDmaPutf( const volatile void* ls, U32 ea, U32 size, U32 tag )
{
	JobDmaNormalAssert( ls, ea, size, tag );
	JobDmaCmd( const_cast<volatile void*>(ls), ea, size, tag, MFC_CMD_WORD(0,0,MFC_PUTF_CMD) );
}

//--------------------------------------------------------------------------------------------------

inline void JobDmaPutb( const volatile void* ls, U32 ea, U32 size, U32 tag )
{
	JobDmaNormalAssert( ls, ea, size, tag );
	JobDmaCmd( const_cast<volatile void*>(ls), ea, size, tag, MFC_CMD_WORD(0,0,MFC_PUTB_CMD) );
}

//--------------------------------------------------------------------------------------------------

inline void JobDmaGet( volatile void* ls, U32 ea, U32 size, U32 tag )
{
	JobDmaNormalAssert( ls, ea, size, tag );
	JobDmaCmd( ls, ea, size, tag, MFC_CMD_WORD(0,0,MFC_GET_CMD) );
}

//--------------------------------------------------------------------------------------------------

inline void JobDmaGetf( volatile void* ls, U32 ea, U32 size, U32 tag )
{
	JobDmaNormalAssert( ls, ea, size, tag );
	JobDmaCmd( ls, ea, size, tag, MFC_CMD_WORD(0,0,MFC_GETF_CMD) );
}

//--------------------------------------------------------------------------------------------------

inline void JobDmaGetb( volatile void* ls, U32 ea, U32 size, U32 tag )
{
	JobDmaNormalAssert( ls, ea, size, tag );
	JobDmaCmd( ls, ea, size, tag, MFC_CMD_WORD(0,0,MFC_GETB_CMD) );
}

//--------------------------------------------------------------------------------------------------

inline void JobDmaListPut( const volatile void* ls, const JobDmaListElement* list, U32 lsize, U32 tag )
{
	JobDmaListAssert( ls, list, lsize, tag );
	JobDmaCmd( const_cast<volatile void*>(ls), (U32)list, lsize, tag, MFC_CMD_WORD(0,0,MFC_PUTL_CMD) );
}

//--------------------------------------------------------------------------------------------------

inline void JobDmaListPutf( const volatile void* ls, const JobDmaListElement* list, U32 lsize, U32 tag )
{
	JobDmaListAssert( ls, list, lsize, tag );
	JobDmaCmd( const_cast<volatile void*>(ls), (U32)list, lsize, tag, MFC_CMD_WORD(0,0,MFC_PUTLF_CMD) );
}

//--------------------------------------------------------------------------------------------------

inline void JobDmaListPutb( const volatile void* ls, const JobDmaListElement* list, U32 lsize, U32 tag )
{
	JobDmaListAssert( ls, list, lsize, tag );
	JobDmaCmd( const_cast<volatile void*>(ls), (U32)list, lsize, tag, MFC_CMD_WORD(0,0,MFC_PUTLB_CMD) );
}

//--------------------------------------------------------------------------------------------------

inline void JobDmaListGet( volatile void* ls, const JobDmaListElement* list, U32 lsize, U32 tag )
{
	JobDmaListAssert( ls, list, lsize, tag );
	JobDmaCmd( ls, (U32)list, lsize, tag, MFC_CMD_WORD(0,0,MFC_GETL_CMD) );
}

//--------------------------------------------------------------------------------------------------

inline void JobDmaListGetf( volatile void* ls, const JobDmaListElement* list, U32 lsize, U32 tag )
{
	JobDmaListAssert( ls, list, lsize, tag );
	JobDmaCmd( ls, (U32)list, lsize, tag, MFC_CMD_WORD(0,0,MFC_GETLF_CMD) );
}

//--------------------------------------------------------------------------------------------------

inline void JobDmaListGetb( volatile void* ls, const JobDmaListElement* list, U32 lsize, U32 tag )
{
	JobDmaListAssert( ls, list, lsize, tag );
	JobDmaCmd( ls, (U32)list, lsize, tag, MFC_CMD_WORD(0,0,MFC_GETLB_CMD) );
}

//--------------------------------------------------------------------------------------------------

inline void JobDmaGetllar( volatile void* ls, U32 ea )
{
	JobDmaAtomicAssert( ls, ea );
	JobDmaCmd( ls, ea, 128, 0, MFC_CMD_WORD(0,0,MFC_GETLLAR_CMD) );
}

//--------------------------------------------------------------------------------------------------

inline void JobDmaPutllc( const volatile void* ls, U32 ea )
{
	JobDmaAtomicAssert( ls, ea );
	JobDmaCmd( const_cast<volatile void*>(ls), ea, 128, 0, MFC_CMD_WORD(0,0,MFC_PUTLLC_CMD) );
}

//--------------------------------------------------------------------------------------------------

inline void JobDmaPutlluc( const volatile void* ls, U32 ea )
{
	JobDmaAtomicAssert( ls, ea );
	JobDmaCmd( const_cast<volatile void*>(ls), ea, 128, 0, MFC_CMD_WORD(0,0,MFC_PUTLLUC_CMD) );
}

//--------------------------------------------------------------------------------------------------

inline void JobDmaPutqlluc( const volatile void* ls, U32 ea, U32 tag )
{
	JobDmaPutqllucAssert( ls, ea, tag );
	JobDmaCmd( const_cast<volatile void*>(ls), ea, 128, tag, MFC_CMD_WORD(0,0,MFC_PUTQLLUC_CMD) );
}

//--------------------------------------------------------------------------------------------------

inline U32 JobDmaWaitTagStatusImmediate( U32 tagmask )
{
	JobDmaTagMaskAssert( tagmask );
  #ifdef PRESERVE_INTERRUPT_STATUS
	Bool32 enabled = AreInterruptsEnabled();
	DisableInterrupts();
	mfc_write_tag_mask(tagmask);
	U32 retVal = mfc_read_tag_status_immediate();
	if(enabled)
		EnableInterrupts();
	return retVal;
  #else
	WWSJOB_ASSERT( AreInterruptsEnabled() );
	DisableInterrupts();
	mfc_write_tag_mask(tagmask);
	U32 retVal = mfc_read_tag_status_immediate();
	EnableInterrupts();
	return retVal;
  #endif
}

//--------------------------------------------------------------------------------------------------

inline U32 JobDmaWaitTagStatusAny( U32 tagmask )
{
	JobDmaTagMaskAssert( tagmask );
  #ifdef PRESERVE_INTERRUPT_STATUS
	Bool32 enabled = AreInterruptsEnabled();
	DisableInterrupts();
	mfc_write_tag_mask(tagmask);
	U32 retVal =  mfc_read_tag_status_any();
	if(enabled)
		EnableInterrupts();
	return retVal;
  #else
  	WWSJOB_ASSERT( AreInterruptsEnabled() );
	DisableInterrupts();
	mfc_write_tag_mask(tagmask);
	U32 retVal =  mfc_read_tag_status_any();
	EnableInterrupts();
	return retVal;
  #endif
}

//--------------------------------------------------------------------------------------------------

inline U32 JobDmaWaitTagStatusAll( U32 tagmask )
{
	JobDmaTagMaskAssert( tagmask );
  #ifdef PRESERVE_INTERRUPT_STATUS
	Bool32 enabled = AreInterruptsEnabled();
	DisableInterrupts();
	mfc_write_tag_mask(tagmask);
	U32 retVal =  mfc_read_tag_status_all();
	if(enabled)
		EnableInterrupts();
	return retVal;
  #else
  	WWSJOB_ASSERT( AreInterruptsEnabled() );
	DisableInterrupts();
	mfc_write_tag_mask(tagmask);
	U32 retVal =  mfc_read_tag_status_all();
	EnableInterrupts();
	return retVal;
  #endif
}

//--------------------------------------------------------------------------------------------------

inline void JobDmaLargePut( const void *ls, U32 ea, U32 size, U32 tag )
{
	JobDmaLargeCmd( (uintptr_t)ls,ea,size,tag,MFC_CMD_WORD(0,0,MFC_PUT_CMD) );
}

//--------------------------------------------------------------------------------------------------

inline void JobDmaLargePutf( const void *ls, U32 ea, U32 size, U32 tag )
{
	JobDmaLargeCmd( (uintptr_t)ls,ea,size,tag,MFC_CMD_WORD(0,0,MFC_PUTF_CMD) );
}

//--------------------------------------------------------------------------------------------------

inline void JobDmaLargePutb( const void *ls, U32 ea, U32 size, U32 tag )
{
	JobDmaLargeCmd( (uintptr_t)ls,ea,size,tag,MFC_CMD_WORD(0,0,MFC_PUTB_CMD) );
}

//--------------------------------------------------------------------------------------------------

inline void JobDmaLargeGet( void* ls, U32 ea, U32 size, U32 tag )
{
	JobDmaLargeCmd( (uintptr_t)ls,ea,size,tag,MFC_CMD_WORD(0,0,MFC_GET_CMD) );
}

//--------------------------------------------------------------------------------------------------

inline void JobDmaLargeGetf( void *ls, U32 ea, U32 size, U32 tag )
{
	JobDmaLargeCmd( (uintptr_t)ls,ea,size,tag,MFC_CMD_WORD(0,0,MFC_GETF_CMD) );
}

//--------------------------------------------------------------------------------------------------

inline void JobDmaLargeGetb( void *ls, U32 ea, U32 size, U32 tag )
{
	JobDmaLargeCmd( (uintptr_t)ls,ea,size,tag,MFC_CMD_WORD(0,0,MFC_GETB_CMD) );
}

//--------------------------------------------------------------------------------------------------

inline U32 JobDmaWaitAtomicStatus( void )
{
	return mfc_read_atomic_status();
}

//--------------------------------------------------------------------------------------------------

#endif /* WWS_JOB_JOB_SPU_DMA_H */
