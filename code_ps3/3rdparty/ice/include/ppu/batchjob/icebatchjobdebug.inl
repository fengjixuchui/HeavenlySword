/*
 * Copyright (c) 2003-2005 Naughty Dog, Inc.
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

/*!
 * \file icebatchjobdebug.inl
 * \brief Contains implementation of common batch job debugging functionality
 *        including debug printing, assertions, and audits.
 *
 *	icebatchjobdebug.inl expects to be included into a batch job dispatcher
 *	cpp file.  This structure allows the including file to control the
 *	preprocessor defines which control the compilation of this file, including:
 *	BATCHJOB_NAMESPACE - sets the namespace for buffer size constants for this job
 *	BATCHJOB_DISPATCHER_DPRINTS - if not 0, enable debug prints in the batch job
 *		through a SPU LS stack or PPU printf.
 *	BATCHJOB_DISPATCHER_ASSERTS - if not 0, enable debug assertions in the batch job
 *	BATCHJOB_DISPATCHER_AUDITS - if not 0, enable audits in the SPU batch job through
 *		the job manager audit system.  BATCHJOB_NAMESPACE::kAuditSystem will be used
 *		as the system tag for all audits.
 */

#include "icebatchjobdebug.h"
#include "icebatchjob.h"

#if BATCHJOB_DISPATCHER_DPRINTS || BATCHJOB_DISPATCHER_ASSERTS
DEBUG_PtrToLocFunction g_pPtrToLocFunction = NULL;

# if ICE_TARGET_PS3_SPU
extern "C"
{
# endif
void DEBUG_SetPtrToLocFn( DEBUG_PtrToLocFunction pFn ) { g_pPtrToLocFunction = pFn; }
# if ICE_TARGET_PS3_SPU
}
# endif
U32 DEBUG_PtrToLoc(void const *p) { return (*g_pPtrToLocFunction)(p); }
#endif

#if BATCHJOB_DISPATCHER_DPRINTS

#if ICE_TARGET_PS3_SPU
//HACK: spu_printf can't handle more than 15 parameters!
static inline int dprint_hex_byte(char *pBuf, U8 byte)
{
	char *p = pBuf;
	*p++ = '0';
	*p++ = 'x';
	U32 nibble_hi = (byte >> 4) & 0x0000000F;
	U32 nibble_lo = byte & 0x0000000F;
	*p++ = (nibble_hi < 0xA) ? ('0' + nibble_hi) : (('a' - 0xa) + nibble_hi);
	*p++ = (nibble_lo < 0xA) ? ('0' + nibble_lo) : (('a' - 0xa) + nibble_lo);
	return p - pBuf;
}
int dprint_qword_as_hex_bytes(char *pBuf, U8 const* pBytes)
{
	char *p = pBuf;
	for (U32F i = 0; ; i += 4) {
		p += dprint_hex_byte(p, pBytes[i]);
		*p++ = ',';
		p += dprint_hex_byte(p, pBytes[i+1]);
		*p++ = ',';
		p += dprint_hex_byte(p, pBytes[i+2]);
		*p++ = ',';
		p += dprint_hex_byte(p, pBytes[i+3]);
		if (i >= 12) {
			*p++ = 0;
			break;
		}
		*p++ = ',';
		*p++ = ' ';
	}
	return p - pBuf;
}
#endif

void DISPATCHER_DPRINT_Mem(void const *p, U32F size)
{
	U8 const *pU8 = (U8 const*)CONV_TO_PTR(CONV_FROM_PTR(p) & ~0xF);
	U8 const *pU8End = (U8 const*)((U8 const*)p + size);
	for (; pU8 < pU8End; pU8 += 16) {
#if ICE_TARGET_PS3_SPU
		//HACK: spu_printf can't handle more than 15 parameters!
		char buf[96];
		dprint_qword_as_hex_bytes(buf, pU8);
		DISPATCHER_DPRINTF("        .db %s;@ %04x\n", buf, DEBUG_PtrToLoc(pU8));
#else
		DISPATCHER_DPRINTF("        .db 0x%02x,0x%02x,0x%02x,0x%02x, 0x%02x,0x%02x,0x%02x,0x%02x, 0x%02x,0x%02x,0x%02x,0x%02x, 0x%02x,0x%02x,0x%02x,0x%02x ;@ %04x\n",
					 pU8[0],pU8[1],pU8[2],pU8[3], pU8[4],pU8[5],pU8[6],pU8[7], pU8[8],pU8[9],pU8[10],pU8[11], pU8[12],pU8[13],pU8[14],pU8[15], DEBUG_PtrToLoc(pU8));
#endif
	}
}
void DISPATCHER_DPRINT_MemF(void const *p, U32F size)
{
	F32 const *pF32 = (F32 const*)CONV_TO_PTR(CONV_FROM_PTR(p) & ~0xF);
	F32 const *pF32End = (F32 const*)((U8 const*)p + size);
	for (; pF32 < pF32End; pF32 += 4) {
		DISPATCHER_DPRINTF("        .df %10.5f,%10.5f,%10.5f,%10.5f ;@ %04x\n",
					 pF32[0], pF32[1], pF32[2], pF32[3], DEBUG_PtrToLoc(pF32));
	}
}
void DISPATCHER_DPRINT_MemU16(void const *p, U32F size)
{
	U16 const *pU16 = (U16 const*)CONV_TO_PTR(CONV_FROM_PTR(p) & ~0xF);
	U16 const *pU16End = (U16 const*)((U8 const*)p + size);
	for (; pU16 < pU16End; pU16 += 8) {
		DISPATCHER_DPRINTF("        .dh 0x%04x,0x%04x,0x%04x,0x%04x,0x%04x,0x%04x,0x%04x,0x%04x ;@ %04x\n",
					 pU16[0], pU16[1], pU16[2], pU16[3], pU16[4], pU16[5], pU16[6], pU16[7], DEBUG_PtrToLoc(pU16));
	}
}
void DISPATCHER_DPRINT_MemU32(void const *p, U32F size)
{
	U32 const *pU32 = (U32 const*)CONV_TO_PTR(CONV_FROM_PTR(p) & ~0xF);
	U32 const *pU32End = (U32 const*)((U8 const*)p + size);
	for (; pU32 < pU32End; pU32 += 4) {
		DISPATCHER_DPRINTF("        .dw 0x%08x,0x%08x,0x%08x,0x%08x ;@ %04x\n", pU32[0], pU32[1], pU32[2], pU32[3], DEBUG_PtrToLoc(pU32));
	}
}
void DISPATCHER_DPRINT_MemU64(void const *p, U32F size)
{
	U32 const *pU32 = (U32 const*)CONV_TO_PTR(CONV_FROM_PTR(p) & ~0xF);
	U32 const *pU32End = (U32 const*)((U8 const*)p + size);
	for (; pU32 < pU32End; pU32 += 4) {
#if ICE_ENDIAN_BIG	// U64[2] so we need to word swap on little endian machines
		DISPATCHER_DPRINTF("        .dw 0x%08x,0x%08x,0x%08x,0x%08x ;@ %04x\n", pU32[0], pU32[1], pU32[2], pU32[3], DEBUG_PtrToLoc(pU32));
#else
		DISPATCHER_DPRINTF("        .dw 0x%08x,0x%08x,0x%08x,0x%08x ;@ %04x\n", pU32[1], pU32[0], pU32[3], pU32[2], DEBUG_PtrToLoc(pU32));
#endif
	}
}
void DISPATCHER_DPRINT_DmaList(void const *dmaListPtr, U32F dmaListSize, U32F dmaListEAhi, void const *dmaDestLS)
{
	U32 const *dmaList = (U32 const *)dmaListPtr;
	U8 const *dst = (U8 const*)dmaDestLS;

	if (dmaListSize > 8*64) dmaListSize = 8*64;

	for (U32F i = 0; i < dmaListSize / 8; i++) {
		U32F sizeLS = ((dmaList[i*2 + 0] + 0xF) & ~0xF);
		DISPATCHER_DPRINTF("        DMALIST (%05x <- %08x_%08x %04x)\n", DEBUG_Ptr(dst), (U32)dmaListEAhi, dmaList[i*2 + 1], dmaList[i*2 + 0]);
		dst += sizeLS;
	}
}

#endif	//#if BATCHJOB_DISPATCHER_DPRINTS

//------------------------------------------------------------------------------------------------
// Debug output helper functions
#if BATCHJOB_DISPATCHER_DPRINTS || BATCHJOB_DISPATCHER_ASSERTS
# if ICE_TARGET_PS3_SPU
#  if !ICEANIM_SPU_ASSEMBLY_DISPATCHER
// s_memoryMap contains a map of the starts of important locations in memory (currentInputBuffer, workBuffer, currentOutputBufferPos, 0)
extern VU32 	 g_memoryMap;
// s_reverseMemoryMap and s_reverseMemoryMapEnd contain the start and end of the input, work, and output buffers, to allow reverse lookups in DEBUG_PtrToLoc()
extern VU32		 g_reverseMemoryMap;
extern VU32		 g_reverseMemoryMapEnd;

U32 DEBUG_PtrToLoc_Batched(void const *p)
{
	VU32 vP = spu_splats((U32)p & 0x3FFF0);
	// mask_inBuf[i] = ((p >= g_memoryMap[i]) && (p < g_reverseMemoryMap)) ? 0xFFFFFFFF : 0;
	VU32 mask_inBuf = spu_andc( spu_cmpgt(g_reverseMemoryMapEnd, vP), spu_cmpgt(g_reverseMemoryMap, vP) );
	// spu_gather converts mask_inBuf into a 4 nibble bit mask with 0xF or 0x0 in each nibble of the preferred halfword
	// spu_cntlz will then return (due to the 16 leading zeros of the high halfword),
	//   0x10, 0x14, 0x18, or 0x1c, depending on whether the pointer is in each buffer in the order
	//   Input, Work, Output, or Memory.  We subtract 0x10, which converts this into one of the constants:
	//	 kLocInput(0x0), kLocWork(0x4), kLocOutput(0x8), kLocMemory(0xc).
	U32 loc0 = spu_extract( spu_cntlz( spu_gather((VU8)mask_inBuf) ), 0 ) - 0x10;
	// Now calculate the offset from g_memoryMap[loc0] and then or in our buffer tag.
	return (U32)((U8 const*)p - (U8 const*)spu_extract( spu_rlqwbyte( g_reverseMemoryMap, loc0 ), 0)) | loc0;
}
#  endif //if !ICEANIM_SPU_ASSEMBLY_DISPATCHER ... elif BATCHJOB_DISPATCHER_DPRINTS
U32 DEBUG_CommandPtr(U16 const *pCommand, VU32 memoryMap)
{
	return (U32)((U8 const*)pCommand - spu_extract(memoryMap, 0));
}
# else	//if ICE_TARGET_PS3_SPU ...
enum {
	kBufInput = 	Ice::BatchJob::kLocInput  >> 2,
	kBufWork = 		Ice::BatchJob::kLocWork   >> 2,
	kBufOutput = 	Ice::BatchJob::kLocOutput >> 2,
	kBufVolatile = 	Ice::BatchJob::kLocVolatile >> 2,
};
// g_memoryMap contains a map of the starts of important locations in memory (currentInputBuffer, workBuffer, currentOutputBufferPos, volatileBuffer)
extern U8 const *g_memoryMap[4];
extern U8 const *g_outputBuf;
extern U8 const *g_memoryLS;

U32 DEBUG_PtrToLoc_Batched(void const *p)
{
	U32 loc = (U32)((U8 const*)p - g_memoryMap[kBufInput]) & 0x3FFF0;
	if (loc < BATCHJOB_NAMESPACE::kInputBufferSize) {
		return loc | Ice::BatchJob::kLocInput;
	}
	loc = (U32)((U8 const*)p - g_memoryMap[kBufWork]) & 0x3FFF0;
	if (loc < BATCHJOB_NAMESPACE::kWorkBufferSize) {
		return loc | Ice::BatchJob::kLocWork;
	}
	loc = (U32)((U8 const*)p - g_outputBuf /*g_memoryMap[kBufOutput]*/) & 0x3FFF0;
	if (loc < BATCHJOB_NAMESPACE::kOutputBufferSize) {
		return loc | Ice::BatchJob::kLocOutput;
	}
	return ((U32)((U8 const*)p - g_memoryLS) & 0x3FFF0) | Ice::BatchJob::kLocVolatile;
}
U32 DEBUG_CommandPtr(U16 const *pCommand, U8 const*const* memoryMap)
{
	return (U32)((U8 const*)pCommand - memoryMap[kBufInput]);
}
# endif	//if ICE_TARGET_PS3_SPU ... else ...
#endif	//#if BATCHJOB_DISPATCHER_DPRINTS || BATCHJOB_DISPATCHER_ASSERTS ...

#if BATCHJOB_DISPATCHER_DPRINTS
# if ICE_TARGET_PS3_SPU
extern "C" void DISPATCHER_DPRINT_Command(U16 const *command, VU32 memoryMap, void const*const* pCommandTable, U8 const* pNumParamsTable)
# else
extern "C" void DISPATCHER_DPRINT_Command(U16 const *command, U8 const*const* memoryMap, void const*const* pCommandTable, U8 const* pNumParamsTable)
# endif
{
	static char s_szCommandFmt[] = "    CMD @%04x, %02x %s[*0x%05x](%04x, %04x, %04x, %04x, %04x, %04x, %04x, %04x)\n";

	U32F cmd = command[0];
	if (cmd <= BATCHJOB_NAMESPACE::kNumCommands) {
		U8 numParams = pNumParamsTable[cmd];
		if (numParams == 0) {
			DISPATCHER_DPRINTF("    CMD @%04x, %02x %s()\n", DEBUG_CommandPtr(command, memoryMap), command[0], BATCHJOB_NAMESPACE::DEBUG_CommandName(cmd));
		} else {
			U32F iFmtPos = 30 + numParams*6;
			s_szCommandFmt[iFmtPos+0] = ')';
			s_szCommandFmt[iFmtPos+1] = '\n';
			s_szCommandFmt[iFmtPos+2] = 0;
			DISPATCHER_DPRINTF(s_szCommandFmt, DEBUG_CommandPtr(command, memoryMap), command[0], BATCHJOB_NAMESPACE::DEBUG_CommandName(cmd), DEBUG_Ptr(pCommandTable[cmd]), command[1], command[2], command[3], command[4], command[5], command[6], command[7], command[8]);
			s_szCommandFmt[iFmtPos+0] = ',';
			s_szCommandFmt[iFmtPos+1] = ' ';
			s_szCommandFmt[iFmtPos+2] = '%';
		}
	}
}
#endif //if BATCHJOB_DISPATCHER_DPRINTS


