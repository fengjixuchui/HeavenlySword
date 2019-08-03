//--------------------------------------------------------------------------------------------------
/**
	@file
	
	@brief		FpInput interface (PS3-specific implementation)

	@warning	Temporary PC port. will be rewritten

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.
**/
//--------------------------------------------------------------------------------------------------

#ifndef FP_INPUT_H
#define FP_INPUT_H

//--------------------------------------------------------------------------------------------------
//	SCE LIBRARY INCLUDES
//--------------------------------------------------------------------------------------------------

#include <sys/synchronization.h>
#include <sys/ppu_thread.h>
#include <sys/event.h>

#include <cell/keyboard.h>
#include <cell/pad.h>
#include <cell/mouse.h>

//--------------------------------------------------------------------------------------------------
//	INCLUDES
//--------------------------------------------------------------------------------------------------

#include <Fp/FpInput/FpPad.h>
#include <Fp/FpInput/FpMouse.h>
#include <Fp/FpInput/FpKeyboard.h>

//--------------------------------------------------------------------------------------------------
//	CLASS DEFINITIONS
//--------------------------------------------------------------------------------------------------

class FpInput : public FwSingleton<FpInput>
{
public:
	// Constants
	enum
	{
		kDefaultThreadPriority		= 255,		///< Default input thread priority
		kDefaultThreadFrequency		= 100		///< Default input thread frequency (in Hz)
	};
	enum
	{
		kUseFpPad		= 0x01,
		kUseFpMouse		= 0x02,
		kUseFpKeyboard	= 0x04
	};

	// Destruction
    ~FpInput(void);

	// Singleton init&shutdown
	static void 			Initialise(unsigned int flags = kUseFpPad | kUseFpMouse | kUseFpKeyboard,
									   unsigned int threadPriority = kDefaultThreadPriority, 
									   unsigned int threadFrequency = kDefaultThreadFrequency);
	static void 			Shutdown(void);

	// Operations
	bool					Update(void);
	inline FpPad*			GetPad(unsigned int numPad);
	inline FpKeyboard* 		GetKeyboard(unsigned int numKeyboard=0);
	inline FpMouse*			GetMouse(unsigned int numMouse=0);

private:	
	// Constants
	enum 
	{
		kMaxPads		= 7,		
		kMaxKeyboards	= 7,		
		kMaxMice		= 7,		
	};

	// Event queue (used to signal the thread)
	enum
	{
		kFpInputThreadEventQueueSize = 8
	};
	enum FpInputEvent 
	{	
		kShutdown	
	};

	// Construction 
	FpInput(unsigned int flags, unsigned int threadPriority, unsigned int threadFrequency);

	// Operations
	bool 					Init(unsigned int flags, unsigned int threadPriority, unsigned int threadFrequency);
	bool 					Close(void);

	bool 					UpdateKeyboards(void);
	bool 					UpdatePads(void);
	bool					UpdateMice(void);

	// Input thread
	static void				ThreadProc(uint64_t arg);
	bool					ThreadMain(void);
	sys_ppu_thread_t		m_thread;					///< Input thread
	sys_event_port_t		m_idEventPort;				///< Event port (to communicate to with the thread)
	sys_event_queue_t		m_idEventQueue;				///< Event queue (to communicate to with the thread)
	int						m_threadUsecInterval;		///< Microsecond interval (=comes from threadFrequency parameter)

	// Attributes
	unsigned int			m_flags;					///< General FpInput internal flags 
	FpPad					m_pads[kMaxPads];			///< Internal array of pad(s)
	FpKeyboard		 		m_keyboards[kMaxKeyboards];	///< Internal array of keyboard(s)
	FpMouse					m_mice[kMaxMice];			///< Internal array of mice

	// Pad buffered data 
	sys_lwmutex_t			m_padRingBufferMutex;		///< Mutex guarding access to the data structures updated by the thread
	enum
	{
		kPadRingBufferMaxSamples = 20					///< Up to 20 samples / user update ( = game running a 5FPS in debug ar 100Hz)
	};
	struct
	{
		int32_t				m_resCellPadGetInfo;				///< Return code of cellPadGetInfo
		int32_t				m_resCellPadGetData[kMaxPads];		///< Return code of cellPadGetData (for up to kMaxPads, in connection order in CellPadInfo)
		CellPadInfo			m_padInfo;							///< Pad connection info
		CellPadData			m_padData[kMaxPads];				///< Pad data(for up to kMaxPads, in connection order in CellPadInfo)
	} m_padRingBuffer[ kPadRingBufferMaxSamples ];				///< Ring buffer for threaded pad acquisition
	int m_padRingBufferFirst;									///< Ring buffer for threaded pad acquisition : first
	int	m_padRingBufferCount;									///< Ring buffer for threaded pad acquisition : num elements
};

//--------------------------------------------------------------------------------------------------
//	INLINES
//--------------------------------------------------------------------------------------------------

#include <Fp/FpInput/FpInput.inl>

//--------------------------------------------------------------------------------------------------

#endif	//FP_INPUT_H
