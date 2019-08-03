/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

/*interrupthandlerasm.spu.s*/

	.section .fillsect.text,"ax"
	.global	SpuInterruptHandlerAsm
	.global InterruptHandlerCallback

	.extern g_BisledRegisterStore

.equ kStackOffset,	(125)*16
	/* Leave at least 48*16 spare because we don't know if the main thread could have already used this much */
	/* We're going to preserve all registers in this function */
.equ kRegStoreSize,	(80*16)

.equ kWwsJob_InterruptHandler_real_begin,	5
.equ kWwsJob_InterruptHandler_real_end,		6

/*
 *	Warning this code is complex and intricate.  It generates code on the stack and then jumps to it.
 *
 *	The layout on the stack goes like this:
 *		0x3FFF0
 *			Area in use by main thread
 *		$1 at entry point of SpuInterruptHandlerAsm
 *			Area reserved because of ABI ( kStackOffset )
 *		Followed by
 *			Register store area ( kRegStoreSize )
 *		Followed by
 *			stack used by interrupt handler
 *
 *	Note that the "stack used by interrupt handler" is first used for writing the store reg instructions into.
 *	It is later used for writing the load reg instructions into.
 *
 *	Coding like this means that:
 *	a) The code is much smaller than if we just had a linear list of store instructions.
 *	b) Is better than doing a self-modifying loop because we'd have to sync in that loop so is *very* expensive
 *	This solution optimizes for both space and speed.
 *	Sadly it's very intricate due to its economic use of registers.
 */

/* Note that this function starts one instruction before qword alignment */
SpuInterruptHandlerAsm:
#ifdef ENABLE_TIMING_PA_BOOKMARKS
	stqa	$78, g_BisledRegisterStore
	il		$78, kWwsJob_InterruptHandler_real_begin
	wrch	$ch69, $78
	il		$78, 0
#else
	lnop
	nop
	stqa	$78, g_BisledRegisterStore
	il		$78, 0
#endif

	/* Note that support for using bisled is work in progress and subject to change */
	/* In particular, the bisled register "bisled $78, &SpuBisledHandlerAsm" may well change */

SpuBisledHandlerAsm:
	hbrr	StoresBranchInst, GenerateStoresLoop

	/* IMPORTANT: The "FourStoreInstructions" label *must* be qword aligned */
FourStoreInstructions:
	stqd	$0, (0x00-kStackOffset-kRegStoreSize)($1)
	stqd	$1, (0x10-kStackOffset-kRegStoreSize)($1)
	stqd	$2, (0x20-kStackOffset-kRegStoreSize)($1)
	stqd	$3, (0x30-kStackOffset-kRegStoreSize)($1)

	ila		$2, 0x10004
	lqa		$0, FourStoreInstructions
	ila		$3, 80-4

GenerateStoresLoop:	
	a		$0, $0, $2										/* Modify the instructions */
	ai		$3, $3, -4										/* loop counter */
	stqd	$0, -kStackOffset-kRegStoreSize-0x20($1)		/* Store the instructions (Note that an extra 0x10 is left clear for the bi $0 */
	ai		$1, $1, -0x10									/* Move where the instructions are stored to */
StoresBranchInst:
	brnz	$3, GenerateStoresLoop

	ai		$1, $1, 19*16									/* Reset the corrupted stack pointer to what the stqd's expect */
	ilhu	$0, 0x3500										/* Create bi $0 opcode */
	ila		$2, (kStackOffset+kRegStoreSize+0x10+(19*16))	/* Offset value to generated function entry point */
	stqd	$0, -kStackOffset-kRegStoreSize-0x10($1)		/* Store bi $0 opcode */
	sf		$2, $2, $1										/* Get pointer to generated function */
	sync
	il		$3, -(kStackOffset + kRegStoreSize + 0x20)		/* 0x20 to allow for backchain pointer - Wrong by one qword? */
	bisl	$0, $2											/* Call generated function */



	/* We've finished storing off all the registers, now it's time to call the function */
	a		$1,	$1,	$3										/* Set new stack pointer */
	brsl	$0, InterruptHandlerCallback				/* And call funcion */



	il		$3, (kStackOffset + kRegStoreSize + 0x20)		/* 0x20 to allow for backchain pointer - Wrong by one qword? */
	hbrr	LoadsBranchInst, GenerateLoadsLoop

	ila		$2, 0x10004
	lqa		$0, FourLoadInstructions

	a		$1,	$1,	$3
	ila		$3, 80-4

GenerateLoadsLoop:	
	a		$0, $0, $2										/* Modify the instructions */
	ai		$1, $1, -0x10									/* Move where the instructions are stored to */
	ai		$3, $3, -4										/* loop counter */
	stqd	$0, -kStackOffset-kRegStoreSize-0x10($1)		/* Store the instructions (Note that an extra 0x10 is left clear for the bi $0 */
LoadsBranchInst:
	brnz	$3, GenerateLoadsLoop

	ai		$1, $1, 19*16									/* Reset the corrupted stack pointer to what the stqd's expect */
	ilhu	$0, 0x3500										/* Create bi $0 opcode */
	ila		$2, (kStackOffset+kRegStoreSize+0x10+(19*16))	/* Offset value to generated function entry point */
	stqd	$0, -kStackOffset-kRegStoreSize-0x10($1)		/* Store bi $0 opcode */
	sf		$2, $2, $1										/* Get pointer to generated function */
	sync
	bisl	$0, $2											/* Call generated function */

	/* IMPORTANT: The "FourLoadInstructions" label *must* be qword aligned */
FourLoadInstructions:
	lqd		$3, (0x30-kStackOffset-kRegStoreSize)($1)
	lqd		$2, (0x20-kStackOffset-kRegStoreSize)($1)
	lqd		$1, (0x10-kStackOffset-kRegStoreSize)($1)
	lqd		$0, (0x00-kStackOffset-kRegStoreSize)($1)




	/* If $78 has a value, we clearly "bisled"ed to SpuBisledHandlerAsm so we should return to $78 */
	binz	$78, $78

#ifdef ENABLE_TIMING_PA_BOOKMARKS
	il		$78, kWwsJob_InterruptHandler_real_end
	wrch	$ch69, $78
#endif

	/* Whereas if $78 is zero, then we must have come through the interrupt handler, so return from the interrupt handler */
	lqa		$78, g_BisledRegisterStore

	/* Return from the interrupt */
	syncc
	irete
