//--------------------------------------------------------------------------------------------------
/**
	@file		SkSpuCollection.h

	@brief		An SPU Collection is a container for multiple SPUs

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.
**/
//--------------------------------------------------------------------------------------------------

#ifndef SK_SPU_COLLECTION_H
#define SK_SPU_COLLECTION_H

//--------------------------------------------------------------------------------------------------
//  INCLUDES
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
//  SWITCHES
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
//  MACRO DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
//  DECLARATIONS
//--------------------------------------------------------------------------------------------------

class SkSpuInterface;
class SkSharedArea;
typedef struct sys_spu_segment sys_spu_segment_t;

//--------------------------------------------------------------------------------------------------
//  CLASS DEFINITIONS
//--------------------------------------------------------------------------------------------------

#ifndef KERNEL_SPU_THREADS
//We only need this class for Raw SPUs
#include <sys/event.h>

class EventNotifier
{
public:
	void CreateEventNotifier( uint64_t portName, sys_ipc_key_t queueKey );
	void SendEvent( void );
	void WaitForEvent( void );
	~EventNotifier();
private:
	sys_event_port_t	m_port;
	sys_event_queue_t	m_queue;
};
#endif
//--------------------------------------------------------------------------------------------------
/**
	@class			SpuCollection

	@brief			Container of multiple SPUs
**/
//--------------------------------------------------------------------------------------------------

class SkSpuCollection
{
public:
						SkSpuCollection( u8 numSpus, const char* spuKernelFile = NULL );
						~SkSpuCollection();

	void				SetupKernel( SkSharedArea& rSharedArea );
	void				Run( void );

	void				StallForAllSpusToEnd( void );

private:
	SkSpuInterface**	m_spuList;
	u8					m_numSpus;

#ifdef KERNEL_SPU_THREADS
	sys_spu_thread_group_t	m_threadGroup;
	void*				m_pElfImg;		//Need to check the lifetime on these
	sys_spu_segment_t*	m_pSegments;	//Need to check the lifetime on these
#else
	EventNotifier		m_spusEnded;
#endif

	volatile u32		m_numRunningSpus;			//The number of currently running SPUs
};

//--------------------------------------------------------------------------------------------------
//  INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
//  GLOBAL NAMESPACE DECLARATIONS
//--------------------------------------------------------------------------------------------------

#endif // SK_SPU_COLLECTION_H
