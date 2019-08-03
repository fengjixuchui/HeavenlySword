/*
 * Copyright (c) 2003-2005 Naughty Dog, Inc.
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#ifndef ICE_BATCHJOBDEBUG_H
#define ICE_BATCHJOBDEBUG_H

#include "icebase.h"

//External defines that control the compile of the dispatcher code:
//#define BATCHJOB_DISPATCHER_AUDITS			// 1 to enable audits in job
//#define BATCHJOB_DISPATCHER_ASSERTS			// 1 to enable assertions in job
//#define BATCHJOB_DISPATCHER_DPRINTS			// 1 to enable basic DPRINTs in job

#if ICE_TARGET_PS3_SPU && BATCHJOB_DISPATCHER_AUDITS
# define DISPATCHER_AUDIT(id, args...)			WwsJob_JobApiStoreAudit((BATCHJOB_NAMESPACE::kAuditSystem<<10) | id, ## args) 
#else
# if ICE_COMPILER_GCC
#  define DISPATCHER_AUDIT(id, args...)			do {} while (0)
# else
#  define DISPATCHER_AUDIT						__noop
# endif
#endif

#if BATCHJOB_DISPATCHER_ASSERTS || BATCHJOB_DISPATCHER_DPRINTS
# if ICE_TARGET_PS3_SPU
#  include "jobapi/jobprintf.h"	// include from WwsJobManager for interrupt-safe printf functionality (via LS stack with PPU doing the print logic)
#  define DISPATCHER_PRINTF JobBasePrintf
# else
#  include <stdio.h>
#  define DISPATCHER_PRINTF ::printf
# endif
#else
# if ICE_COMPILER_GCC
#  define DISPATCHER_PRINTF(format, args...)
# else
#  define DISPATCHER_PRINTF	__noop
# endif
#endif

#define DISPATCHER_HALT()								ICE_HALT()
#if BATCHJOB_DISPATCHER_ASSERTS
# define DISPATCHER_ASSERT(cond)						if (!(cond)) { DISPATCHER_PRINTF("%s:%d: DISPATCHER_ASSERT(%s)\n", __FILE__, __LINE__, ""#cond); DISPATCHER_HALT(); }
# if ICE_COMPILER_GCC
#  define DISPATCHER_ASSERTF(cond, format, args...)		if (!(cond)) { DISPATCHER_PRINTF("%s:%d: DISPATCHER_ASSERT: "#format, __FILE__, __LINE__, ## args); DISPATCHER_HALT(); }
#  define DISPATCHER_ASSERTM(format, args...)			{ DISPATCHER_PRINTF("%s:%d: DISPATCHER_ASSERT: "#format, __FILE__, __LINE__, ## args); DISPATCHER_HALT(); }
# else
#  include <stdarg.h>
static inline void DISPATCHER_ASSERTF(bool cond, const char *format, ...) { va_list va; va_start(va, format); if (!cond) { printf("%s:%d: DISPATCHER_ASSERT: ", __FILE__, __LINE__); vprintf(format, va); DISPATCHER_HALT(); } va_end(va); }
static inline void DISPATCHER_ASSERTM(const char *format, ...) { va_list va; va_start(va, format); { printf("%s:%d: DISPATCHER_ASSERT: ", __FILE__, __LINE__); vprintf(format, va); DISPATCHER_HALT(); } va_end(va); }
# endif
#else
# define DISPATCHER_ASSERT(cond)						do {} while(0)
# if ICE_COMPILER_GCC
#  define DISPATCHER_ASSERTF(cond, format, args...)		do {} while(0)
#  define DISPATCHER_ASSERTM(format, args...)			do {} while(0)
# else
#  define DISPATCHER_ASSERTF								__noop
#  define DISPATCHER_ASSERTM								__noop
# endif
#endif

#if BATCHJOB_DISPATCHER_DPRINTS
# if ICE_COMPILER_GCC
#  define DISPATCHER_DPRINTF(format, args...)	DISPATCHER_PRINTF(format, ## args)
# else
#  define DISPATCHER_DPRINTF	DISPATCHER_PRINTF
# endif
#else
# if ICE_COMPILER_GCC
#  define DISPATCHER_DPRINTF(format, args...)
# else
#  define DISPATCHER_DPRINTF	__noop
# endif
#endif

#if BATCHJOB_DISPATCHER_DPRINTS || BATCHJOB_DISPATCHER_ASSERTS
typedef U32 (*DEBUG_PtrToLocFunction)(void const*);
# if ICE_TARGET_PS3_SPU
extern "C"
{
# endif
void DEBUG_SetPtrToLocFn(DEBUG_PtrToLocFunction pFn);
# if ICE_TARGET_PS3_SPU
}
# endif

U32 DEBUG_PtrToLoc(void const *p);
U32 DEBUG_PtrToLoc_Batched(void const *p);

# if ICE_TARGET_PS3_SPU
#  ifndef CONV_FROM_PTR
#   define CONV_FROM_PTR(p) 	((U32)(p))
#  endif
U32 DEBUG_CommandPtr(U16 const *pCommand, VU32 memoryMap);
static inline U32 DEBUG_Ptr_(void const *p) { return (U32)CONV_FROM_PTR(p) & 0x3FFF0; }
# else
#  ifndef CONV_FROM_PTR
#   define CONV_FROM_PTR(p) 	((U32)(p))
#  endif
U32 DEBUG_CommandPtr(U16 const *pCommand, U8 const*const* memoryMap);
static inline U32 DEBUG_Ptr_(void const *p) { return (U32)CONV_FROM_PTR(p); }
# endif
// So we can DEBUG_Ptr to anything, including function pointers without a warning:
# define DEBUG_Ptr(p) DEBUG_Ptr_((void const*)(p))
#else
# define DEBUG_SetPtrToLocFn(pFn)
# define DEBUG_PtrToLoc(p)						(0)
# define DEBUG_Ptr(p)							((void const*)NULL)
# define DEBUG_CommandPtr(pCommand, memoryMap)	(0)
#endif

#if BATCHJOB_DISPATCHER_DPRINTS

void DISPATCHER_DPRINT_Mem(void const *p, U32F size);
void DISPATCHER_DPRINT_MemF(void const *p, U32F size);
void DISPATCHER_DPRINT_MemU16(void const *p, U32F size);
void DISPATCHER_DPRINT_MemU32(void const *p, U32F size);
void DISPATCHER_DPRINT_MemU64(void const *p, U32F size);
void DISPATCHER_DPRINT_DmaList(void const *dmaListPtr, U32F dmaListSize, U32F dmaListEAhi, void const *dmaDestLS);

static inline void DISPATCHER_DPRINT_Mem(char const *label, void const *p, U32F size)
{
	DISPATCHER_DPRINTF("    %s:\n", label);
	DISPATCHER_DPRINT_Mem(p, size);
}
static inline void DISPATCHER_DPRINT_MemF(char const *label, void const *p, U32F size)
{
	DISPATCHER_DPRINTF("    %s:\n", label);
	DISPATCHER_DPRINT_MemF(p, size);
}
static inline void DISPATCHER_DPRINT_MemU16(char const *label, void const *p, U32F size)
{
	DISPATCHER_DPRINTF("    %s:\n", label);
	DISPATCHER_DPRINT_MemU16(p, size);
}
static inline void DISPATCHER_DPRINT_MemU32(char const *label, void const *p, U32F size)
{
	DISPATCHER_DPRINTF("    %s:\n", label);
	DISPATCHER_DPRINT_MemU32(p, size);
}
static inline void DISPATCHER_DPRINT_MemU64(char const *label, void const *p, U32F size)
{
	DISPATCHER_DPRINTF("    %s:\n", label);
	DISPATCHER_DPRINT_MemU64(p, size);
}
static inline void DISPATCHER_DPRINT_DmaList(char const *label, void const *dmaListPtr, U32F dmaListSize, U32F dmaListEAhi, void const *dmaDestLS)
{
	DISPATCHER_DPRINTF("    %s:\n", label);
	DISPATCHER_DPRINT_DmaList(dmaListPtr, dmaListSize, dmaListEAhi, dmaDestLS);
}

# if ICE_TARGET_PS3_SPU
extern "C" void DISPATCHER_DPRINT_Command(U16 const *command, VU32 memoryMap, void const*const* pCommandTable, U8 const* pNumParamsTable);
extern "C" void DISPATCHER_DPRINT_CommandInputData(U16 const *command, VU32 memoryMap);
extern "C" void DISPATCHER_DPRINT_CommandOutputData(U16 const *command, VU32 memoryMap);
# else
extern "C" void DISPATCHER_DPRINT_Command(U16 const *command, U8 const*const* memoryMap, void const*const* pCommandTable, U8 const* pNumParamsTable);
#  define DISPATCHER_DPRINT_CommandInputData(command, memoryMap)
#  define DISPATCHER_DPRINT_CommandOutputData(command, memoryMap)
# endif

#else	//#if BATCHJOB_DISPATCHER_DPRINTS

# if ICE_COMPILER_GCC
#  define DISPATCHER_DPRINT_Mem(...)
#  define DISPATCHER_DPRINT_MemF(...)
#  define DISPATCHER_DPRINT_MemU16(...)
#  define DISPATCHER_DPRINT_MemU32(...)
#  define DISPATCHER_DPRINT_MemU64(...)
#  define DISPATCHER_DPRINT_DmaList(...)
# else
#  define DISPATCHER_DPRINT_Mem					__noop
#  define DISPATCHER_DPRINT_MemF				__noop
#  define DISPATCHER_DPRINT_MemU16				__noop
#  define DISPATCHER_DPRINT_MemU32				__noop
#  define DISPATCHER_DPRINT_MemU64				__noop
#  define DISPATCHER_DPRINT_DmaList				__noop
# endif

# define DISPATCHER_DPRINT_Command(command, memoryMap, pCommandTable, pNumParamsTable)
# define DISPATCHER_DPRINT_CommandInputData(command, memoryMap)
# define DISPATCHER_DPRINT_CommandOutputData(command, memoryMap)

#endif	//#if BATCHJOB_DISPATCHER_DPRINTS ... #else

#endif // ICE_BATCHJOBDEBUG_H

