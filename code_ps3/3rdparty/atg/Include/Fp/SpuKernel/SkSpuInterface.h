//--------------------------------------------------------------------------------------------------
/**
	@file		SkSpuInterface.h

	@brief		This class interfaces to a single SPU

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.
**/
//--------------------------------------------------------------------------------------------------

#ifndef SK_SPU_INTERFACE_H
#define SK_SPU_INTERFACE_H

//--------------------------------------------------------------------------------------------------
//  INCLUDES
//--------------------------------------------------------------------------------------------------

#include <sys/raw_spu.h>
#include <ppu_thread.h>

//--------------------------------------------------------------------------------------------------
//  SWITCHES
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
//  MACRO DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
//  DECLARATIONS
//--------------------------------------------------------------------------------------------------
class SkElf;
class SkSharedArea;
typedef struct sys_spu_thread_attribute sys_spu_thread_attribute_t;
class EventNotifier;

//--------------------------------------------------------------------------------------------------
/**
	@class			SkSpuInterface

	@brief			interface class to a single SPU
**/
//--------------------------------------------------------------------------------------------------

class SkSpuInterface
{
public:
							SkSpuInterface( const char* spuKernelFile, sys_spu_thread_group_t threadGroup, int spuNum, volatile u32* pNumSpusRunning, EventNotifier* pSpusEndedEventNotifier, sys_spu_thread_attribute_t* pThreadAttr );
							~SkSpuInterface();

	void					SetupKernel( SkSharedArea& rSharedArea );
	void					Run( void );

	//void					DmaSendToSpu( u32 localStoreAddr, const void* mainMemBuffer, u32 size );
	//void					DmaSendFromSpu( u32 localStoreAddr, void* mainMemBuffer, u32 size );

#ifdef KERNEL_SPU_THREADS
	int						GetThreadExitStatus( void );
#endif

private:

	void					SetupSpuInterruptHandler( void );
	void					RemoveSpuInterruptHandler( void );

#ifdef KERNEL_SPU_THREADS
	int						m_spuNum;
	sys_spu_thread_t		m_thread;
#else
	sys_raw_spu_t			m_spuId;

	sys_interrupt_thread_handle_t	m_intrThreadHandle;	//Needed for the interrupt handler setup for this spu
	sys_interrupt_tag_t				m_intrTag;			//Needed for the interrupt handler setup for this spu

	u32						m_entryPoint;

	bool					m_isRunning;
	volatile uint32_t*		m_pNumSpusRunning;
	EventNotifier*			m_pSpusEndedEventNotifier;
#endif

	bool					m_firstRunForThisSpu;

	static bool				ms_firstInit;
	static u32				ms_totalNumCurrentSpus;	//Track the total SPUs presently allocated across all SkSpuInterfaces

	static void				InterruptHandlerCallback( uint64_t arg );

};

//--------------------------------------------------------------------------------------------------
//  INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
//  GLOBAL NAMESPACE DECLARATIONS
//--------------------------------------------------------------------------------------------------

#endif // SK_SPU_INTERFACE_H

