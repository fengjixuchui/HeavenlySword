//--------------------------------------------------------------------------------------------------
/**
	@file		SkDma.h

	@brief		

	@note		
**/
//--------------------------------------------------------------------------------------------------

#ifndef SK_DMA_H
#define SK_DMA_H

#include <spu_mfcio.h>

#ifdef KERNEL_SPU_THREADS

	#include <sys/spu_thread.h>

	#define LS_BASE_ADDR( spu_num )	\
		( (SYS_SPU_THREAD_OFFSET * spu_num) + SYS_SPU_THREAD_BASE_LOW + SYS_SPU_THREAD_LS_BASE )

//	#define PROB_BASE_ADDR(spu_num)
//		( (SYS_SPU_THREAD_OFFSET * spu_num) + SYS_SPU_THREAD_BASE_LOW + SYS_SPU_THREAD_PROB_BASE)

	#define GetSnr1Addr(spu_num)	((SYS_SPU_THREAD_OFFSET * spu_num) + SYS_SPU_THREAD_BASE_LOW + SYS_SPU_THREAD_SNR1)
	#define GetSnr2Addr(spu_num)	((SYS_SPU_THREAD_OFFSET * spu_num) + SYS_SPU_THREAD_BASE_LOW + SYS_SPU_THREAD_SNR2)

#else

	//From raw_spu_mmio.h

	/* Statically defined base addresses and offsets */
	#define  RAW_SPU_OFFSET       0x0000000000100000
	#define  RAW_SPU_BASE_ADDR    0x00000000E0000000
	#define  RAW_SPU_LS_OFFSET    0x0000000000000000
	#define  RAW_SPU_PROB_OFFSET  0x0000000000040000

	/* Macros to calculate base addresses with the given Raw SPU ID */
	#define LS_BASE_ADDR(id) \
		(RAW_SPU_OFFSET * id + RAW_SPU_BASE_ADDR + RAW_SPU_LS_OFFSET)
	#define PROB_BASE_ADDR(id) \
		(RAW_SPU_OFFSET * id + RAW_SPU_BASE_ADDR + RAW_SPU_PROB_OFFSET)


	#define Sig_Notify_1     0x1400C
	#define Sig_Notify_2     0x1C00C

	#define get_reg_addr(id, offset) \
		(volatile u64)(PROB_BASE_ADDR(id) + offset)
	//#define get_reg_addr(id, offset)
	//	(volatile uint64_t)(PROB_BASE_ADDR(id) + offset)

	#define GetSnr1Addr(spu_num)	get_reg_addr( spu_num, Sig_Notify_1 )
	#define GetSnr2Addr(spu_num)	get_reg_addr( spu_num, Sig_Notify_2 )
#endif


#define SPU_ADDR_UPPER_32BITS(addr)	((addr)>>32)

#define DMA_TAG_IN_OUT							( 29 )
#define DMA_TAG_COMMAND_STACK_LOADING			( 30 )
#define DMA_TAG_PROB_REG_WRITE					( 31 )

const bool gbChickenSwitch = false;

//--------------------------------------------------------------------------------------------------
/**
	@namespace		SkDma

	@brief			DMA operations
**/
//--------------------------------------------------------------------------------------------------

namespace SkDma
{
	void	SyncAllDma( void );
	void	BeginDmaIn( void* spuAddr, u64 ydramAddr, u32 length, u32 id );
	void	BeginDmaOut( const void* spuAddr, u64 ydramAddr, u32 length, u32 id );
	void	DmaLoadAndReserve( void* spuAddr, u64 ydramAddr );
	bool	DmaStoreWithReservation( const void* spuAddr, u64 ydramAddr );

	void	RunCode( u32 programCounter, u32 paramsAddr, u32 constantData, bool verbose = false );

	void	DmaAddBarrier( void );
	void	SyncChannel( u32 id );

	void	SetSpuSignalNotificationRegister( u32 spuId, u32* pValue, u32 dmaTagId );
	void	StallForSpuSignalNotificationRegisterSetting( u32 dmaTagId );
	u32		GetThisSpuSignalNotificationRegister( void );
	u32		GetThisSpuSignalNotificationRegisterChannelCount( void );

	void	BeginBigDmaIn( void* spuAddr, u64 ydramAddr, u32 length, u32 id );
	void	BeginBigDmaOut( const void* spuAddr, u64 ydramAddr, u32 length, u32 id );

	enum
	{
		kMaxDmaSize = 16*1024,	//maximum size in a single dma
	};
};

extern "C" void SkDma_BeginBigDma( void* spuAddr, u64 ydramAddr, u32 length, u32 id, u32 command );
extern "C" u32 SkDma_DmaLoadAndReserve( void* spuAddr, u64 ydramAddr );
extern "C" bool SkDma_DmaStoreWithReservation( const void* spuAddr, u64 ydramAddr );

//--------------------------------------------------------------------------------------------------
/**
	@brief			issue a row-aligned DMA into the SPU

	@param			spuAddr			-	SPU address
	@param			ydramAddr		-	YDRAM address
	@param			length			-	length in bytes
	@param			id				-	sync id
**/
//--------------------------------------------------------------------------------------------------

inline void	SkDma::BeginDmaIn( void* spuAddr, u64 ydramAddr, u32 length, u32 id )
{
	TRACE(( "In: spuAddr = 0x%08X, ydramAddr = 0x%08X, length = 0x%08X\n", spuAddr, (u32)ydramAddr, length ));

	asm __volatile__("" : : : "memory");
	spu_mfcdma64( spuAddr, SPU_ADDR_UPPER_32BITS(ydramAddr), (u32)ydramAddr, length, id, MFC_GET_CMD );
	asm __volatile__("" : : : "memory");

	if ( gbChickenSwitch )
		SyncAllDma();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			issue a row-aligned DMA out of the SPU

	@param			spuAddr			-	SPU address
	@param			ydramAddr		-	YDRAM address
	@param			length			-	length in bytes
	@param			id				-	sync id
**/
//--------------------------------------------------------------------------------------------------

inline void	SkDma::BeginDmaOut( const void* spuAddr, u64 ydramAddr, u32 length, u32 id )
{
	TRACE(( "In: spuAddr = 0x%08X, ydramAddr = 0x%08X, length = 0x%08X\n", spuAddr, (u32)ydramAddr, length ));

	asm __volatile__("" : : : "memory");
	spu_mfcdma64( (volatile void*)spuAddr, SPU_ADDR_UPPER_32BITS(ydramAddr), (u32)ydramAddr, length, id, MFC_PUT_CMD );
	asm __volatile__("" : : : "memory");

	if ( gbChickenSwitch )
		SyncAllDma();
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			issue a DMA load-and-reserve on a row

	@param			spuAddr			-	SPU address of row
	@param			ydramAddr		-	YDRAM address of row
**/
//--------------------------------------------------------------------------------------------------

inline void	SkDma::DmaLoadAndReserve( void* spuAddr, u64 ydramAddr )
{
	u32 status = SkDma_DmaLoadAndReserve( spuAddr, ydramAddr );
	FW_UNUSED( status );
	FW_ASSERT( (status & 0x4) != 0 );	//Should I have this assert in?  Can it happen?
	FW_UNUSED( status );
}

//--------------------------------------------------------------------------------------------------
/**
	@brief			issue a DMA store-with-reservation on a row

	@param			spuAddr			-	SPU address of row
	@param			ydramAddr		-	YDRAM address of row

	@return			true if the store succeeded, else false
**/
//--------------------------------------------------------------------------------------------------

inline bool	SkDma::DmaStoreWithReservation( const void* spuAddr, u64 ydramAddr )
{
	return SkDma_DmaStoreWithReservation( spuAddr, ydramAddr );
}

//--------------------------------------------------------------------------------------------------

inline void SkDma::BeginBigDmaIn( void* spuAddr, u64 ydramAddr, u32 length, u32 id )
{
	SkDma_BeginBigDma( spuAddr, ydramAddr, length, id, MFC_GET_CMD );

	if ( gbChickenSwitch )
		SyncAllDma();
}

//--------------------------------------------------------------------------------------------------

inline void SkDma::BeginBigDmaOut( const void* spuAddr, u64 ydramAddr, u32 length, u32 id )
{
	SkDma_BeginBigDma( (void*) spuAddr, ydramAddr, length, id, MFC_PUT_CMD );

	if ( gbChickenSwitch )
		SyncAllDma();
}

//--------------------------------------------------------------------------------------------------

inline void SkDma::SyncAllDma( void )
{
	TRACE(( "Syncing\n" ));

	spu_writech(MFC_WrTagMask, 0xFFFFFFFF);
	asm __volatile__("" : : : "memory");
	spu_mfcstat(2);
	asm __volatile__("" : : : "memory");

	TRACE(( "Synced\n" ));
}

//--------------------------------------------------------------------------------------------------

inline void SkDma::SyncChannel( u32 id )
{
	TRACE(( "Syncing%d\n", id ));

	spu_writech(MFC_WrTagMask, 1 << id);
	asm __volatile__("" : : : "memory");
	spu_mfcstat(2);
	asm __volatile__("" : : : "memory");

	TRACE(( "Synced%d\n", id ));
}

//--------------------------------------------------------------------------------------------------

inline void SkDma::DmaAddBarrier( void )
{
	si_wrch( MFC_Cmd, si_from_uint(MFC_CMD_WORD(0, 0, MFC_BARRIER_CMD)) );

	if ( gbChickenSwitch )
		SyncAllDma();
}

//--------------------------------------------------------------------------------------------------

inline void SkDma::SetSpuSignalNotificationRegister( u32 spuId, u32* pValue, u32 dmaTagId )
{
	u64 addr = GetSnr1Addr( spuId );	//get_reg_addr( spuId, Sig_Notify_1 );

	asm __volatile__("" : : : "memory");
	spu_mfcdma64( pValue, SPU_ADDR_UPPER_32BITS(addr), (u32)addr, 4, dmaTagId, MFC_SNDSIG_CMD );
	asm __volatile__("" : : : "memory");

	if ( gbChickenSwitch )
		SyncAllDma();
}

//--------------------------------------------------------------------------------------------------

inline void SkDma::StallForSpuSignalNotificationRegisterSetting( u32 dmaTagId )
{
	SkDma::SyncChannel( dmaTagId );
}

//--------------------------------------------------------------------------------------------------

inline u32 SkDma::GetThisSpuSignalNotificationRegister( void )
{
	asm __volatile__("" : : : "memory");
	u32 ret = spu_readch(SPU_RdSigNotify1);
	asm __volatile__("" : : : "memory");
	return ret;
}

//--------------------------------------------------------------------------------------------------

inline u32 SkDma::GetThisSpuSignalNotificationRegisterChannelCount( void )
{
	asm __volatile__("" : : : "memory");
	u32 ret = spu_readchcnt(SPU_RdSigNotify1);
	asm __volatile__("" : : : "memory");
	return ret;
}

//--------------------------------------------------------------------------------------------------

#endif // SK_DMA_H
