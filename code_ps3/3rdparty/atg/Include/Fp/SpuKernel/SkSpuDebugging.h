//--------------------------------------------------------------------------------------------------
/**
	@file		SkSpuDebugging.h

	@brief		

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.
**/
//--------------------------------------------------------------------------------------------------

#ifndef SK_SPU_DEBUGGING_H
#define SK_SPU_DEBUGGING_H

//--------------------------------------------------------------------------------------------------
//  INCLUDES
//--------------------------------------------------------------------------------------------------

#include <sys/raw_spu.h>
#include <ppu_thread.h>

//--------------------------------------------------------------------------------------------------
//  SWITCHES
//--------------------------------------------------------------------------------------------------

#define SN_RAW_SPU_SUPPORT

//--------------------------------------------------------------------------------------------------
//  MACRO DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
//  DECLARATIONS
//--------------------------------------------------------------------------------------------------

class SpuStatusRegister;

extern bool HandleStopSignal( sys_raw_spu_t spuId );
extern void HandleHaltSignal( sys_raw_spu_t idSpu );
extern void SetupSpuInterruptHandler( sys_raw_spu_t spuId, sys_interrupt_thread_handle_t* pIntrThreadHandle, sys_interrupt_tag_t* pIntrTag );
extern void RemoveSpuInterruptHandler( sys_raw_spu_t spuId, sys_interrupt_thread_handle_t intrThreadHandle, sys_interrupt_tag_t intrTag );
extern u32 GetStopSignal( sys_raw_spu_t spuId );
extern void PrintSpuStatusRegister( sys_raw_spu_t spuId, const SpuStatusRegister& status );

extern int SpuPrintfServerInitialize( void );
extern int SpuPrintfServerFinalize( void );
extern int SpuPrintfServerRegister( sys_spu_thread_t spu );
extern int SpuPrintfServerUnregister( sys_spu_thread_t spu );

extern int SpuCallbackServerInitialise( void );
extern int SpuCallbackServerFinalize( void );
extern int SpuCallbackServerRegister( sys_spu_thread_t spu );
extern int SpuCallbackServerUnregister( sys_spu_thread_t spu );

//--------------------------------------------------------------------------------------------------
//  CLASS DEFINITIONS
//--------------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------------
//  INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
//  GLOBAL NAMESPACE DECLARATIONS
//--------------------------------------------------------------------------------------------------

#endif // SK_SPU_DEBUGGING_H
