## Copyright (c) 2003-2006 Naughty Dog, Inc. 
## A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
## Use and distribution without consent strictly prohibited
## 
## Jon Olick


.data

#define r0 0
#define r1 1
#define r2 2
#define r3 3
#define r4 4
#define r5 5
#define r6 6
#define r7 7
#define r8 8
#define r9 9
#define r10 10
#define r11 11
#define r12 12
#define r13 13
#define r14 14
#define r15 15
#define r16 16
#define r17 17
#define r18 18
#define r19 19
#define r20 20
#define r21 21
#define r22 22
#define r23 23
#define r24 24
#define r25 25
#define r26 26
#define r27 27
#define r28 28
#define r29 29
#define r30 30
#define r31 31

#define f0 0
#define f1 1
#define f2 2
#define f3 3
#define f4 4
#define f5 5
#define f6 6
#define f7 7
#define f8 8
#define f9 9
#define f10 10
#define f11 11
#define f12 12
#define f13 13
#define f14 14
#define f15 15
#define f16 16
#define f17 17
#define f18 18
#define f19 19
#define f20 20
#define f21 21
#define f22 22
#define f23 23
#define f24 24
#define f25 25
#define f26 26
#define f27 27
#define f28 28
#define f29 29
#define f30 30
#define f31 31

#define cr0 0
#define cr1 1
#define cr2 2
#define cr3 3
#define cr4 4
#define cr5 5
#define cr6 6
#define cr7 7

#define FUNC(name) \
    .section        ".text"; \
    .align 4; \
    .globl name; \
    .section        ".opd","aw"; \
    .align 2; \
	name: \
    .long   .name,.TOC.@tocbase32; \
    .size   name,.-name; \
    .previous; \
    .type   .name,@function; \
    .globl  .name; \
	.name:

#define SIMPLE_OUTOFSPACE_1(name, size, save0) \
	name##_OutOfSpace: \
		stdu r1, -128(r1); \
		std save0, 0x78(r1); \
		mflr r4; \
		std r4, 144(r1); \
		li r4, size; \
		bl ._ZN3Ice6Render14CommandContext13ReserveFailedEj; \
		cmpdi cr7, r3, 0; \
		ld save0, 144(r1); \
		mtlr save0; \
		ld save0, 0x78(r1); \
		addi r1, r1, 128; \
		bne+ cr7, .name; \
		blr;

#define SIMPLE_OUTOFSPACE_2(name, size, save0, save1) \
	name##_OutOfSpace: \
		stdu r1, -128(r1); \
		std save0, 0x78(r1); \
		std save1, 0x70(r1); \
		mflr r4; \
		std r4, 144(r1); \
		li r4, size; \
		bl ._ZN3Ice6Render14CommandContext13ReserveFailedEj; \
		cmpdi cr7, r3, 0; \
		ld save0, 144(r1); \
		mtlr save0; \
		ld save0, 0x78(r1); \
		ld save1, 0x70(r1); \
		addi r1, r1, 128; \
		bne+ cr7, .name; \
		blr;

#define SIMPLE_OUTOFSPACE_CALC_2(name, begin,cmd, save0, save1) \
	name##_OutOfSpace: \
		stdu r1, -128(r1); \
		std save0, 0x78(r1); \
		std save1, 0x70(r1); \
		mflr r4; \
		std r4, 144(r1); \
		sub r4, cmd, begin; \
		srdi r4, r4, 2; \
		bl ._ZN3Ice6Render14CommandContext13ReserveFailedEj; \
		cmpdi cr7, r3, 0; \
		ld save0, 144(r1); \
		mtlr save0; \
		ld save0, 0x78(r1); \
		ld save1, 0x70(r1); \
		addi r1, r1, 128; \
		bne+ cr7, .name; \
		blr;

#define SIMPLE_OUTOFSPACE_3(name, size, save0, save1, save2) \
	name##_OutOfSpace: \
		stdu r1, -128(r1); \
		std save0, 0x78(r1); \
		std save1, 0x70(r1); \
		std save2, 0x68(r1); \
		mflr r4; \
		std r4, 144(r1); \
		li r4, size; \
		bl ._ZN3Ice6Render14CommandContext13ReserveFailedEj; \
		cmpdi cr7, r3, 0; \
		ld save0, 144(r1); \
		mtlr save0; \
		ld save0, 0x78(r1); \
		ld save1, 0x70(r1); \
		ld save2, 0x68(r1); \
		addi r1, r1, 128; \
		bne+ cr7, .name; \
		blr;

#define SIMPLE_OUTOFSPACE_4(name, size, save0, save1, save2, save3) \
	name##_OutOfSpace: \
		stdu r1, -128(r1); \
		std save0, 0x78(r1); \
		std save1, 0x70(r1); \
		std save2, 0x68(r1); \
		std save3, 0x60(r1); \
		mflr r4; \
		std r4, 144(r1); \
		li r4, size; \
		bl ._ZN3Ice6Render14CommandContext13ReserveFailedEj; \
		cmpdi cr7, r3, 0; \
		ld save0, 144(r1); \
		mtlr save0; \
		ld save0, 0x78(r1); \
		ld save1, 0x70(r1); \
		ld save2, 0x68(r1); \
		ld save3, 0x60(r1); \
		addi r1, r1, 128; \
		bne+ cr7, .name; \
		blr;

#define SIMPLE_OUTOFSPACE_CALC_4(name, begin,cmd, save0,save1,save2,save3) \
	name##_OutOfSpace: \
		stdu r1, -128(r1); \
		std save0, 0x78(r1); \
		std save1, 0x70(r1); \
		std save2, 0x68(r1); \
		std save3, 0x60(r1); \
		mflr r4; \
		std r4, 144(r1); \
		sub r4, cmd, begin; \
		srdi r4, r4, 2; \
		bl ._ZN3Ice6Render14CommandContext13ReserveFailedEj; \
		cmpdi cr7, r3, 0; \
		ld save0, 144(r1); \
		mtlr save0; \
		ld save0, 0x78(r1); \
		ld save1, 0x70(r1); \
		ld save2, 0x68(r1); \
		ld save3, 0x60(r1); \
		addi r1, r1, 128; \
		bne+ cr7, .name; \
		blr;

#define SIMPLE_OUTOFSPACE_5(name, size, save0, save1, save2, save3, save4) \
	name##_OutOfSpace: \
		stdu r1, -128(r1); \
		std save0, 0x78(r1); \
		std save1, 0x70(r1); \
		std save2, 0x68(r1); \
		std save3, 0x60(r1); \
		std save4, 0x58(r1); \
		mflr r4; \
		std r4, 144(r1); \
		li r4, size; \
		bl ._ZN3Ice6Render14CommandContext13ReserveFailedEj; \
		cmpdi cr7, r3, 0; \
		ld save0, 144(r1); \
		mtlr save0; \
		ld save0, 0x78(r1); \
		ld save1, 0x70(r1); \
		ld save2, 0x68(r1); \
		ld save3, 0x60(r1); \
		ld save4, 0x58(r1); \
		addi r1, r1, 128; \
		bne+ cr7, .name; \
		blr;

#define SIMPLE_OUTOFSPACE_CALC_5(name, begin,cmd, save0, save1, save2, save3, save4) \
	name##_OutOfSpace: \
		stdu r1, -128(r1); \
		std save0, 0x78(r1); \
		std save1, 0x70(r1); \
		std save2, 0x68(r1); \
		std save3, 0x60(r1); \
		std save4, 0x58(r1); \
		mflr r4; \
		std r4, 144(r1); \
		sub r4, cmd, begin; \
		srdi r4, r4, 2; \
		bl ._ZN3Ice6Render14CommandContext13ReserveFailedEj; \
		cmpdi cr7, r3, 0; \
		ld save0, 144(r1); \
		mtlr save0; \
		ld save0, 0x78(r1); \
		ld save1, 0x70(r1); \
		ld save2, 0x68(r1); \
		ld save3, 0x60(r1); \
		ld save4, 0x58(r1); \
		addi r1, r1, 128; \
		bne+ cr7, .name; \
		blr;

#define SIMPLE_OUTOFSPACE_6(name, size, save0, save1, save2, save3, save4, save5) \
	name##_OutOfSpace: \
		stdu r1, -128(r1); \
		std save0, 0x78(r1); \
		std save1, 0x70(r1); \
		std save2, 0x68(r1); \
		std save3, 0x60(r1); \
		std save4, 0x58(r1); \
		std save5, 0x50(r1); \
		mflr r4; \
		std r4, 144(r1); \
		li r4, size; \
		bl ._ZN3Ice6Render14CommandContext13ReserveFailedEj; \
		cmpdi cr7, r3, 0; \
		ld save0, 144(r1); \
		mtlr save0; \
		ld save0, 0x78(r1); \
		ld save1, 0x70(r1); \
		ld save2, 0x68(r1); \
		ld save3, 0x60(r1); \
		ld save4, 0x58(r1); \
		ld save5, 0x50(r1); \
		addi r1, r1, 128; \
		bne+ cr7, .name; \
		blr;

#define SIMPLE_OUTOFSPACE_7(name, size, save0, save1, save2, save3, save4, save5, save6) \
	name##_OutOfSpace: \
		stdu r1, -128(r1); \
		std save0, 0x78(r1); \
		std save1, 0x70(r1); \
		std save2, 0x68(r1); \
		std save3, 0x60(r1); \
		std save4, 0x58(r1); \
		std save5, 0x50(r1); \
		std save6, 0x48(r1); \
 		mflr r4; \
		std r4, 144(r1); \
		li r4, size; \
		bl ._ZN3Ice6Render14CommandContext13ReserveFailedEj; \
		cmpdi cr7, r3, 0; \
		ld save0, 144(r1); \
		mtlr save0; \
		ld save0, 0x78(r1); \
		ld save1, 0x70(r1); \
		ld save2, 0x68(r1); \
		ld save3, 0x60(r1); \
		ld save4, 0x58(r1); \
		ld save5, 0x50(r1); \
		ld save6, 0x48(r1); \
		addi r1, r1, 128; \
		bne+ cr7, .name; \
		blr;

#define SIMPLE_OUTOFSPACE_CALC_7(name, begin,cmd, save0, save1, save2, save3, save4, save5, save6) \
	name##_OutOfSpace: \
		stdu r1, -128(r1); \
		std save0, 0x78(r1); \
		std save1, 0x70(r1); \
		std save2, 0x68(r1); \
		std save3, 0x60(r1); \
		std save4, 0x58(r1); \
		std save5, 0x50(r1); \
		std save6, 0x48(r1); \
		mflr r4; \
		std r4, 144(r1); \
		sub r4, cmd, begin; \
		srdi r4, r4, 2; \
		bl ._ZN3Ice6Render14CommandContext13ReserveFailedEj; \
		cmpdi cr7, r3, 0; \
		ld save0, 144(r1); \
		mtlr save0; \
		ld save0, 0x78(r1); \
		ld save1, 0x70(r1); \
		ld save2, 0x68(r1); \
		ld save3, 0x60(r1); \
		ld save4, 0x58(r1); \
		ld save5, 0x50(r1); \
		ld save6, 0x48(r1); \
		addi r1, r1, 128; \
		bne+ cr7, .name; \
		blr;

#define SIMPLE_OUTOFSPACE_1R_1F(name, size, save0, savef0) \
	name##_OutOfSpace: \
		stdu r1, -128(r1); \
		std  save0, 0x78(r1); \
		stfd savef0, 0x70(r1); \
		mflr r4; \
		std r4, 144(r1); \
		li r4, size; \
		bl ._ZN3Ice6Render14CommandContext13ReserveFailedEj; \
		cmpdi cr7, r3, 0; \
		ld save0, 144(r1); \
		mtlr save0; \
		ld  save0, 0x78(r1); \
		lfd savef0, 0x70(r1); \
		addi r1, r1, 128; \
		bne+ cr7, .name; \
		blr;

#define SIMPLE_OUTOFSPACE_1R_2F(name, size, save0, savef0, savef1) \
	name##_OutOfSpace: \
		stdu r1, -128(r1); \
		std  save0, 0x78(r1); \
		stfd savef0, 0x70(r1); \
		stfd savef1, 0x68(r1); \
		mflr r4; \
		std r4, 144(r1); \
		li r4, size; \
		bl ._ZN3Ice6Render14CommandContext13ReserveFailedEj; \
		cmpdi cr7, r3, 0; \
		ld save0, 144(r1); \
		mtlr save0; \
		ld  save0, 0x78(r1); \
		lfd savef0, 0x70(r1); \
		lfd savef1, 0x68(r1); \
		addi r1, r1, 128; \
		bne+ cr7, .name; \
		blr;

#define SIMPLE_OUTOFSPACE_2R_1F(name, size, save0, save1, savef0) \
	name##_OutOfSpace: \
		stdu r1, -128(r1); \
		std  save0, 0x78(r1); \
		std  save1, 0x70(r1); \
		stfd savef0, 0x68(r1); \
		mflr r4; \
		std r4, 144(r1); \
		li r4, size; \
		bl ._ZN3Ice6Render14CommandContext13ReserveFailedEj; \
		cmpdi cr7, r3, 0; \
		ld save0, 144(r1); \
		mtlr save0; \
		ld  save0, 0x78(r1); \
		ld  save1, 0x70(r1); \
		lfd savef0, 0x68(r1); \
		addi r1, r1, 128; \
		bne+ cr7, .name; \
		blr;

#define SIMPLE_OUTOFSPACE_2R_2F(name, size, save0, save1, savef0, savef1) \
	name##_OutOfSpace: \
		stdu r1, -128(r1); \
		std  save0, 0x78(r1); \
		std  save1, 0x70(r1); \
		stfd savef0, 0x68(r1); \
		stfd savef1, 0x60(r1); \
		mflr r4; \
		std r4, 144(r1); \
		li r4, size; \
		bl ._ZN3Ice6Render14CommandContext13ReserveFailedEj; \
		cmpdi cr7, r3, 0; \
		ld save0, 144(r1); \
		mtlr save0; \
		ld  save0, 0x78(r1); \
		ld  save1, 0x70(r1); \
		lfd savef0, 0x68(r1); \
		lfd savef1, 0x60(r1); \
		addi r1, r1, 128; \
		bne+ cr7, .name; \
		blr;

#define SIMPLE_OUTOFSPACE_2R_3F(name, size, save0, save1, savef0, savef1, savef2) \
	name##_OutOfSpace: \
		stdu r1, -128(r1); \
		std  save0, 0x78(r1); \
		std  save1, 0x70(r1); \
		stfd savef0, 0x68(r1); \
		stfd savef1, 0x60(r1); \
		stfd savef2, 0x58(r1); \
		mflr r4; \
		std r4, 144(r1); \
		li r4, size; \
		bl ._ZN3Ice6Render14CommandContext13ReserveFailedEj; \
		cmpdi cr7, r3, 0; \
		ld save0, 144(r1); \
		mtlr save0; \
		ld  save0, 0x78(r1); \
		ld  save1, 0x70(r1); \
		lfd savef0, 0x68(r1); \
		lfd savef1, 0x60(r1); \
		lfd savef2, 0x58(r1); \
		addi r1, r1, 128; \
		bne+ cr7, .name; \
		blr;

#define SIMPLE_OUTOFSPACE_2R_4F(name, size, save0, save1, savef0, savef1, savef2, savef3) \
	name##_OutOfSpace: \
		stdu r1, -128(r1); \
		std  save0, 0x78(r1); \
		std  save1, 0x70(r1); \
		stfd savef0, 0x68(r1); \
		stfd savef1, 0x60(r1); \
		stfd savef2, 0x58(r1); \
		stfd savef3, 0x50(r1); \
		mflr r4; \
		std r4, 144(r1); \
		li r4, size; \
		bl ._ZN3Ice6Render14CommandContext13ReserveFailedEj; \
		cmpdi cr7, r3, 0; \
		ld save0, 144(r1); \
		mtlr save0; \
		ld  save0, 0x78(r1); \
		ld  save1, 0x70(r1); \
		lfd savef0, 0x68(r1); \
		lfd savef1, 0x60(r1); \
		lfd savef2, 0x58(r1); \
		lfd savef3, 0x50(r1); \
		addi r1, r1, 128; \
		bne+ cr7, .name; \
		blr;

#define SIMPLE_OUTOFSPACE_5R_2F(name, size, save0, save1, save2, save3, save4, savef0, savef1) \
	name##_OutOfSpace: \
		stdu r1, -128(r1); \
		std  save0, 0x78(r1); \
		std  save1, 0x70(r1); \
		std  save2, 0x68(r1); \
		std  save3, 0x60(r1); \
		std  save4, 0x58(r1); \
		stfd savef0, 0x50(r1); \
		stfd savef1, 0x48(r1); \
		mflr r4; \
		std r4, 144(r1); \
		li r4, size; \
		bl ._ZN3Ice6Render14CommandContext13ReserveFailedEj; \
		cmpdi cr7, r3, 0; \
		ld save0, 144(r1); \
		mtlr save0; \
		ld  save0, 0x78(r1); \
		ld  save1, 0x70(r1); \
		ld  save2, 0x68(r1); \
		ld  save3, 0x60(r1); \
		ld  save4, 0x58(r1); \
		lfd savef0, 0x50(r1); \
		lfd savef1, 0x48(r1); \
		addi r1, r1, 128; \
		bne+ cr7, .name; \
		blr;

#define SIMPLE_OUTOFSPACE_6R_2F(name, size, save0, save1, save2, save3, save4, save5, savef0, savef1) \
	name##_OutOfSpace: \
		stdu r1, -128(r1); \
		std  save0, 0x78(r1); \
		std  save1, 0x70(r1); \
		std  save2, 0x68(r1); \
		std  save3, 0x60(r1); \
		std  save4, 0x58(r1); \
		std  save5, 0x50(r1); \
		stfd savef0, 0x48(r1); \
		stfd savef1, 0x40(r1); \
		mflr r4; \
		std r4, 144(r1); \
		li r4, size; \
		bl ._ZN3Ice6Render14CommandContext13ReserveFailedEj; \
		cmpdi cr7, r3, 0; \
		ld save0, 144(r1); \
		mtlr save0; \
		ld  save0, 0x78(r1); \
		ld  save1, 0x70(r1); \
		ld  save2, 0x68(r1); \
		ld  save3, 0x60(r1); \
		ld  save4, 0x58(r1); \
		ld  save5, 0x50(r1); \
		lfd savef0, 0x48(r1); \
		lfd savef1, 0x40(r1); \
		addi r1, r1, 128; \
		bne+ cr7, .name; \
		blr;

## 7 cycles, 8 instructions, 32 bytes
#define SIMPLE_STUFF_ARG_1W_FUNC(name, register) \
	FUNC(name) \
		lwz r9, 8(r3);						lis r0, 4; \
		lwz r10, 4(r3);	\
		addi r11, r9, 8; \
		ori r0, r0, register; \
		cmplw cr7, r11, r10; \
		bgt- cr7, name##_OutOfSpace;		stw r4, 4(r9); \
		stw r11, 8(r3); \
		stw r0, 0(r9); \
		blr; \
	SIMPLE_OUTOFSPACE_2(name, 2, r3, r4) \
	.size   .name,.-.name

## 8 cycles, 9 instructions, 36 bytes	
#define SIMPLE_STUFF_2ARG_2W_FUNC(name, register, arg0, arg1) \
	FUNC(name) \
		lis r0, 8;							lwz r9, 8(r3); \
		lwz r10, 4(r3);	\
		addi r11, r9, 12; \
		ori r0, r0, register; \
		cmplw cr7, r11, r10; \
		bgt- cr7, name##_OutOfSpace;		stw arg0, 4(r9); \
		stw arg1, 8(r9); \
		stw r11, 8(r3); \
		stw r0, 0(r9);						blr; \
	SIMPLE_OUTOFSPACE_3(name, 3, r3, arg0, arg1) \
	.size   .name,.-.name

## 7 cycles, 9 instructions, 36 bytes
#define SIMPLE_STUFF_CONST_1W_FUNC(name, register, constant) \
	FUNC(name) \
		lis r0, 4;							lwz r9, 8(r3); \
		li r4, constant;					lwz r10, 4(r3);	\
		addi r11, r9, 8; \
		ori r0, r0, register; \
		cmplw cr7, r11, r10; \
		bgt- cr7, name##_OutOfSpace;		stw r4, 4(r9); \
		stw r11, 8(r3); \
		stw r0, 0(r9);						blr; \
	SIMPLE_OUTOFSPACE_1(name, 2, r3) \
	.size   .name,.-.name

.section        ".toc","aw"

## 1.0f
fOne:		.tc32 FS_3f800000[TC], 0x3f800000

## -0.0901681f
fDensity:	.tc32 FS_bdb8aa0e[TC], 0xbdb8aa0e

## 0.5f
fHalf:		.tc32 FS_3f000000[TC], 0x3f000000

## 16777215.f
fDepthMul:	.tc32 FS_4b7fffff[TC], 0x4b7fffff

g_nullFragmentProgramOffset:	.tc32 _ZN3Ice6Render27g_nullFragmentProgramOffsetE[TC], _ZN3Ice6Render27g_nullFragmentProgramOffsetE

.section        ".text"


## void Ice::Render::CommandContext::WriteReport(ReportType type, U32 ioOffset)
FUNC(_ZN3Ice6Render14CommandContext11WriteReportENS0_10ReportTypeEj)
	rlwimi r5, r4, 24, 0, 7;			lwz	r9, 8(r3);
	lis r0, 0x4;						lwz r10, 4(r3);
	addi r11, r9, 8;						
	ori r0, r0, 0x1800;
	cmplw cr7, r11, r10;
	bgt- cr7, _ZN3Ice6Render14CommandContext11WriteReportENS0_10ReportTypeEj_OutOfSpace;	
	stw r5, 4(r9);						
	stw r0, 0(r9);
	stw r11, 8(r3);						blr;
SIMPLE_OUTOFSPACE_3(_ZN3Ice6Render14CommandContext11WriteReportENS0_10ReportTypeEj, 2, r3, r4, r5)
        .size   ._ZN3Ice6Render14CommandContext11WriteReportENS0_10ReportTypeEj,.-._ZN3Ice6Render14CommandContext11WriteReportENS0_10ReportTypeEj

## void Ice::Render::CommandContext::ResetReport(ReportType type)
SIMPLE_STUFF_ARG_1W_FUNC(_ZN3Ice6Render14CommandContext11ResetReportENS0_10ReportTypeE, 0x17C8)

## void Ice::Render::CommandContext::SetDrawBufferMask(U32 mask)
## ~8 cycles, 11 instructions, 44 bytes
FUNC(_ZN3Ice6Render14CommandContext17SetDrawBufferMaskEj)
	cmplwi cr7, r4, 1;					lwz r9, 8(r3);
	lis r11, 4;							lwz r10, 4(r3);
	addi r0, r9, 8;						ble+ cr7, irsSdbmNoMrt;				
	ori r4, r4, 0x10;
irsSdbmNoMrt:
	ori r11, r11, 0x220;														
	cmplw cr7, r0, r10;				
	bgt- cr7, _ZN3Ice6Render14CommandContext17SetDrawBufferMaskEj_OutOfSpace;	stw r4, 4(r9);						
	stw r11, 0(r9);
	stw r0, 8(r3);						blr;
SIMPLE_OUTOFSPACE_2(_ZN3Ice6Render14CommandContext17SetDrawBufferMaskEj, 2, r3, r4)
        .size   ._ZN3Ice6Render14CommandContext17SetDrawBufferMaskEj,.-._ZN3Ice6Render14CommandContext17SetDrawBufferMaskEj

## void Ice::Render::CommandContext::SetAlphaFunc(ComparisonFunc func, U32 ref)
## 6 cycles, 9 instructions, 36 bytes
SIMPLE_STUFF_2ARG_2W_FUNC(_ZN3Ice6Render14CommandContext12SetAlphaFuncENS0_14ComparisonFuncEj, 0x308, 4, 5)

## void Ice::Render::CommandContext::SetMrtBlendEnable(U32 mask)
## 12 cycles, 14 instructions, 44 bytes
FUNC(_ZN3Ice6Render14CommandContext17SetMrtBlendEnableEj)
	lis r0, 4;							lwz r9, 8(r3);
	lis r10, 4;							lwz r12, 4(r3);
	ori r0, r0, 0x310;
	addi r11, r9, 16;						
	ori r10, r10, 0x36C;
	cmplw cr7, r11, r12;
	rlwinm r8, r4, 0, 28, 30;			bgt- cr7, _ZN3Ice6Render14CommandContext17SetMrtBlendEnableEj_OutOfSpace;				
	stw r0, 0(r9);
	clrlwi r4, r4, 31;					
	stw r10, 8(r9);
	stw r8, 12(r9);
	stw r4, 4(r9);
	stw r11, 8(r3);						blr;
SIMPLE_OUTOFSPACE_2(_ZN3Ice6Render14CommandContext17SetMrtBlendEnableEj, 4, r3, r4)
        .size   ._ZN3Ice6Render14CommandContext17SetMrtBlendEnableEj,.-._ZN3Ice6Render14CommandContext17SetMrtBlendEnableEj

## void Ice::Render::CommandContext::SetBlendFunc(BlendFactor src, BlendFactor dst)
## 9 cycles, 11 instructions, 44 bytes
FUNC(_ZN3Ice6Render14CommandContext12SetBlendFuncENS0_11BlendFactorES2_)
	lis r0, 8;							lwz r9, 8(r3);
	rlwimi r4, r4, 16, 0, 15;			lwz r12, 4(r3);
	addi r11, r9, 12;						
	rlwimi r5, r5, 16, 0, 15;				
	cmplw cr7, r11, r12;
	ori r0, r0, 0x314;					
	bgt- cr7, _ZN3Ice6Render14CommandContext12SetBlendFuncENS0_11BlendFactorES2__OutOfSpace;	stw r4, 4(r9);
	stw r5, 8(r9);
	stw r0, 0(r9);
	stw r11, 8(r3);						blr;
SIMPLE_OUTOFSPACE_3(_ZN3Ice6Render14CommandContext12SetBlendFuncENS0_11BlendFactorES2_, 3, r3, r4, r5)
        .size   ._ZN3Ice6Render14CommandContext12SetBlendFuncENS0_11BlendFactorES2_,.-._ZN3Ice6Render14CommandContext12SetBlendFuncENS0_11BlendFactorES2_
	
## @@@ This could potentially be combined with SetBlendFunc for an additional 1 cycle cost.
## void Ice::Render::CommandContext::SetBlendFuncSeparate(BlendFactor srcRGB, BlendFactor dstRGB, BlendFactor srcAlpha, BlendFactor dstAlpha)
## 9 cycles, 11 instructions, 44 bytes
FUNC(_ZN3Ice6Render14CommandContext20SetBlendFuncSeparateENS0_11BlendFactorES2_S2_S2_)
	lis r0, 8;							lwz r9, 8(r3);
	rlwimi r4, r6, 16, 0, 15;			lwz r12, 4(r3);
	addi r11, r9, 12;						
	rlwimi r5, r7, 16, 0, 15;				
	cmplw cr7, r11, r12;
	ori r0, r0, 0x314;					
	bgt- cr7, _ZN3Ice6Render14CommandContext20SetBlendFuncSeparateENS0_11BlendFactorES2_S2_S2__OutOfSpace;	stw r4, 4(r9);					
	stw r5, 8(r9);
	stw r0, 0(r9);
	stw r11, 8(r3);						blr;
SIMPLE_OUTOFSPACE_5(_ZN3Ice6Render14CommandContext20SetBlendFuncSeparateENS0_11BlendFactorES2_S2_S2_, 3, r3, r4, r5, r6, r7)
        .size   ._ZN3Ice6Render14CommandContext20SetBlendFuncSeparateENS0_11BlendFactorES2_S2_S2_,.-._ZN3Ice6Render14CommandContext20SetBlendFuncSeparateENS0_11BlendFactorES2_S2_S2_
										
## void Ice::Render::CommandContext::SetBlendEquation(BlendEquation equation)
## 7 cycles, 9 instructions, 36 bytes
FUNC(_ZN3Ice6Render14CommandContext16SetBlendEquationENS0_13BlendEquationE)
	lis r0, 4;							lwz r9, 8(r3);
	rlwimi r4, r4, 16, 0, 15;			lwz r12, 4(r3);
	addi r11, r9, 8;						
	ori r0, r0, 0x320;
	cmplw cr7, r11, r12;
	bgt- cr7, _ZN3Ice6Render14CommandContext16SetBlendEquationENS0_13BlendEquationE_OutOfSpace;		
	stw r4, 4(r9);
	stw r0, 0(r9);
	stw r11, 8(r3);						blr;
SIMPLE_OUTOFSPACE_2(_ZN3Ice6Render14CommandContext16SetBlendEquationENS0_13BlendEquationE, 2, r3, r4)
        .size   ._ZN3Ice6Render14CommandContext16SetBlendEquationENS0_13BlendEquationE,.-._ZN3Ice6Render14CommandContext16SetBlendEquationENS0_13BlendEquationE
	
## @@@ This could potentially be combined with SetBlendEquation for an additional 1 cycle cost.
## void Ice::Render::CommandContext::SetBlendEquationSeparate(BlendEquation equationRGB, BlendEquation equationAlpha)
## 7 cycles, 9 instructions, 36 bytes
FUNC(_ZN3Ice6Render14CommandContext24SetBlendEquationSeparateENS0_13BlendEquationES2_)
	lis r0, 4;							lwz r9, 8(r3);
	rlwimi r4, r5, 16, 0, 15;			lwz r12, 4(r3);
	addi r11, r9, 8;						
	ori r0, r0, 0x320;
	cmplw cr7, r11, r12;
	bgt- cr7, _ZN3Ice6Render14CommandContext24SetBlendEquationSeparateENS0_13BlendEquationES2__OutOfSpace;	
	stw r4, 4(r9);
	stw r0, 0(r9);
	stw r11, 8(r3);						blr;
SIMPLE_OUTOFSPACE_3(_ZN3Ice6Render14CommandContext24SetBlendEquationSeparateENS0_13BlendEquationES2_, 2, r3, r4, r5)
        .size   ._ZN3Ice6Render14CommandContext24SetBlendEquationSeparateENS0_13BlendEquationES2_,.-._ZN3Ice6Render14CommandContext24SetBlendEquationSeparateENS0_13BlendEquationES2_

## void Ice::Render::CommandContext::SetBlendColor(ArgbColor color)
SIMPLE_STUFF_ARG_1W_FUNC(_ZN3Ice6Render14CommandContext13SetBlendColorEj, 0x31C)

## void Ice::Render::CommandContext::SetFloatBlendColor(half red, half green, half blue, half alpha)
## 11 cycles, 13 instructions, 52 bytes
FUNC(_ZN3Ice6Render14CommandContext18SetFloatBlendColorEtttt)
	lis r0, 4;							lwz r9, 8(r3);
	rlwimi r4, r5, 16, 0, 15;			lwz r12, 4(r3);
	addi r11, r9, 16;						
	ori r10, r0, 0x37C;
	cmplw cr7, r11, r12;
	ori r0, r0, 0x31C;					
	bgt- cr7, _ZN3Ice6Render14CommandContext18SetFloatBlendColorEtttt_OutOfSpace;	stw r4, 4(r9);
	rlwimi r6, r7, 16, 0, 15;				
	stw r10, 8(r9);
	stw r0, 0(r9);
	stw r6, 12(r9);
	stw r11, 8(r3);						blr;
SIMPLE_OUTOFSPACE_5(_ZN3Ice6Render14CommandContext18SetFloatBlendColorEtttt, 4, r3,r4,r5,r6,r7)
        .size   ._ZN3Ice6Render14CommandContext18SetFloatBlendColorEtttt,.-._ZN3Ice6Render14CommandContext18SetFloatBlendColorEtttt

## void Ice::Render::CommandContext::SetColorMask(bool red, bool green, bool blue, bool alpha)
## 18 cycles, 20 instructions, 80 bytes
FUNC(_ZN3Ice6Render14CommandContext12SetColorMaskEbbbb)
	rlwinm r0, r7, 4, 27, 27;				lwz r11, 8(r3);
	rlwimi r0, r4, 5, 26, 26;				lwz r12, 4(r3);
	addi r8, r11, 16;						
	rlwimi r0, r5, 6, 25, 25;
	cmplw cr7, r8, r12;
	rlwimi r0, r6, 7, 24, 24;
	lis r9, 4;								bgt- cr7, _ZN3Ice6Render14CommandContext12SetColorMaskEbbbb_OutOfSpace
	rlwimi r0, r0, 4, 20, 23;
	rlwimi r6, r5, 8, 0, 23;
	rlwimi r0, r0, 4, 16, 19;
	rlwimi r6, r4, 16, 0, 15;
	ori r10, r9, 0x370;
	rlwimi r6, r7, 24, 0, 7;				
	stw r0, 12(r11);
	ori r9, r9, 0x324;					
	stw r10, 8(r11);
	stw r6, 4(r11);
	stw r9, 0(r11);
	stw r8, 8(r3);						blr;
SIMPLE_OUTOFSPACE_5(_ZN3Ice6Render14CommandContext12SetColorMaskEbbbb, 4, r3,r4,r5,r6,r7)
        .size   ._ZN3Ice6Render14CommandContext12SetColorMaskEbbbb,.-._ZN3Ice6Render14CommandContext12SetColorMaskEbbbb

## void Ice::Render::CommandContext::SetMrtColorMask(U16 mask)
## 14 cycles, 16 instructions, 64 bytes
FUNC(_ZN3Ice6Render14CommandContext15SetMrtColorMaskEt)
	lis r9, 4;							lwz r8, 8(r3);
	rlwinm r0, r4, 29, 31, 31;			lwz r12, 4(r3);
	addi r11, r8, 16;						
	rlwimi r0, r4, 6, 23, 23;
	cmplw cr7, r11, r12;
	rlwimi r0, r4, 15, 15, 15;
	ori r10, r9, 0x370;					bgt- cr7, _ZN3Ice6Render14CommandContext15SetMrtColorMaskEt_OutOfSpace
	rlwimi r0, r4, 24, 7, 7;
	ori r9, r9, 0x324;					
	stw r0, 4(r8);
	rlwinm r4, r4, 0, 16, 27;				
	stw r10, 8(r8);
	stw r9, 0(r8);
	stw r4, 12(r8);
	stw r11, 8(r3);						blr;
SIMPLE_OUTOFSPACE_2(_ZN3Ice6Render14CommandContext15SetMrtColorMaskEt, 4, r3,r4)
        .size   ._ZN3Ice6Render14CommandContext15SetMrtColorMaskEt,.-._ZN3Ice6Render14CommandContext15SetMrtColorMaskEt

## void Ice::Render::CommandContext::SetDepthMask(bool mask)
SIMPLE_STUFF_ARG_1W_FUNC(_ZN3Ice6Render14CommandContext12SetDepthMaskEb, 0xA70)

## void Ice::Render::CommandContext::SetStencilMask(U32 mask)
## 10 cycles, 11 instructions, 44 bytes
FUNC(_ZN3Ice6Render14CommandContext14SetStencilMaskEj)
	lis r0, 4;							lwz r9, 8(r3);
	lwz r12, 4(r3);
	addi r10, r9, 16;						
	ori r11, r0, 0x34C;					
	cmplw cr7, r10, r12;
	bgt- cr7, _ZN3Ice6Render14CommandContext14SetStencilMaskEj_OutOfSpace;	stw r4, 12(r9);
	ori r0, r0, 0x32C;					
	stw r4, 4(r9);
	stw r11, 8(r9);
	stw r0, 0(r9);
	stw r10, 8(r3);						blr;
SIMPLE_OUTOFSPACE_2(_ZN3Ice6Render14CommandContext14SetStencilMaskEj, 4, r3,r4)
        .size   ._ZN3Ice6Render14CommandContext14SetStencilMaskEj,.-._ZN3Ice6Render14CommandContext14SetStencilMaskEj

## @@@ This could potentially be combined with SetStencilMask for an additional 1 cycle cost.
## void Ice::Render::CommandContext::SetStencilMaskSeparate(U32 maskFront, U32 maskBack)
## 10 cycles, 11 instructions, 44 bytes
FUNC(_ZN3Ice6Render14CommandContext22SetStencilMaskSeparateEjj)
	lis r0, 4;							lwz r9, 8(r3);
	lwz r12, 4(r3);
	addi r10, r9, 16;						
	ori r11, r0, 0x34C;					
	cmplw cr7, r10, r12;
	bgt- cr7, _ZN3Ice6Render14CommandContext22SetStencilMaskSeparateEjj_OutOfSpace;	stw r5, 12(r9);
	ori r0, r0, 0x32C;					
	stw r4, 4(r9);
	stw r11, 8(r9);
	stw r0, 0(r9);
	stw r10, 8(r3);						blr;
SIMPLE_OUTOFSPACE_3(_ZN3Ice6Render14CommandContext22SetStencilMaskSeparateEjj, 4, r3,r4,r5)
        .size   ._ZN3Ice6Render14CommandContext22SetStencilMaskSeparateEjj,.-._ZN3Ice6Render14CommandContext22SetStencilMaskSeparateEjj

## void Ice::Render::CommandContext::SetStencilFunc(ComparisonFunc func, U32 ref, U32 mask)
## 14 cycles, 15 instructions, 60 bytes
FUNC(_ZN3Ice6Render14CommandContext14SetStencilFuncENS0_14ComparisonFuncEjj)
	lis r10, 0xC;						lwz r9, 8(r3);
	lwz r12, 4(r3);
	addi r0, r9, 32;						
	ori r11, r10, 0x330;					
	cmplw cr7, r0, r12;
	ori r10, r10, 0x350;				bgt- cr7, _ZN3Ice6Render14CommandContext14SetStencilFuncENS0_14ComparisonFuncEjj_OutOfSpace;
	stw r4, 4(r9);
	stw r5, 8(r9);
	stw r6, 12(r9);
	stw r4, 20(r9);
	stw r5, 24(r9);
	stw r6, 28(r9);
	stw r11, 0(r9);
	stw r10, 16(r9);
	stw r0, 8(r3);						blr;
SIMPLE_OUTOFSPACE_4(_ZN3Ice6Render14CommandContext14SetStencilFuncENS0_14ComparisonFuncEjj, 8, r3,r4,r5,r6)
        .size   ._ZN3Ice6Render14CommandContext14SetStencilFuncENS0_14ComparisonFuncEjj,.-._ZN3Ice6Render14CommandContext14SetStencilFuncENS0_14ComparisonFuncEjj

## @@@ This could potentially be combined with SetStencilFunc for an additional 3 cycle cost.
## void Ice::Render::CommandContext::SetStencilFuncSeparate(ComparisonFunc funcFront, U32 refFront, U32 maskFront, ComparisonFunc funcBack, U32 refBack, U32 maskBack)
## 14 cycles, 15 instructions, 60 bytes
FUNC(_ZN3Ice6Render14CommandContext22SetStencilFuncSeparateENS0_14ComparisonFuncEjjS2_jj)
	lis r10, 0xC;						lwz r12, 8(r3);
	lwz r0, 4(r3);
	addi r12, r12, 32;						
	ori r11, r10, 0x330;					
	cmplw cr7, r12, r0;
	ori r10, r10, 0x350;					
	bgt- cr7, _ZN3Ice6Render14CommandContext22SetStencilFuncSeparateENS0_14ComparisonFuncEjjS2_jj_OutOfSpace;	stw r5, 8-32(r12);
	stw r4, 4-32(r12);
	stw r6, 12-32(r12);
	stw r7, 20-32(r12);
	stw r8, 24-32(r12);
	stw r9, 28-32(r12);
	stw r11, 0-32(r12);
	stw r10, 16-32(r12);
	stw r12, 8(r3);						blr;
SIMPLE_OUTOFSPACE_7(_ZN3Ice6Render14CommandContext22SetStencilFuncSeparateENS0_14ComparisonFuncEjjS2_jj, 8, r3,r4,r5,r6,r7,r8,r9)
        .size   ._ZN3Ice6Render14CommandContext22SetStencilFuncSeparateENS0_14ComparisonFuncEjjS2_jj,.-._ZN3Ice6Render14CommandContext22SetStencilFuncSeparateENS0_14ComparisonFuncEjjS2_jj

## void Ice::Render::CommandContext::SetStencilOp(StencilOp sfail, StencilOp dfail, StencilOp dpass)
## 14 cycles, 15 instructions, 60 bytes
FUNC(_ZN3Ice6Render14CommandContext12SetStencilOpENS0_9StencilOpES2_S2_)
	lis r10, 0xC;						lwz r12, 8(r3);
	lwz r0, 4(r3);
	addi r12, r12, 32;
	ori r11, r10, 0x33C;					
	cmplw cr7, r12, r0;
	ori r10, r10, 0x35C;					
	bgt- cr7, _ZN3Ice6Render14CommandContext12SetStencilOpENS0_9StencilOpES2_S2__OutOfSpace;	stw r5, 8-32(r12);
	stw r4, 4-32(r12);
	stw r6, 12-32(r12);
	stw r4, 20-32(r12);
	stw r5, 24-32(r12);
	stw r6, 28-32(r12);
	stw r11, 0-32(r12);
	stw r10, 16-32(r12);
	stw r12, 8(r3);						blr;
SIMPLE_OUTOFSPACE_4(_ZN3Ice6Render14CommandContext12SetStencilOpENS0_9StencilOpES2_S2_, 8, r3,r4,r5,r6)
        .size   ._ZN3Ice6Render14CommandContext12SetStencilOpENS0_9StencilOpES2_S2_,.-._ZN3Ice6Render14CommandContext12SetStencilOpENS0_9StencilOpES2_S2_


## void Ice::Render::CommandContext::SetStencilOpSeparate(StencilOp sfailFront, StencilOp dfailFront, StencilOp dpassFront, StencilOp sfailBack, StencilOp dfailBack, StencilOp dpassBack)
## 14 cycles, 15 instructions, 60 bytes
FUNC(_ZN3Ice6Render14CommandContext20SetStencilOpSeparateENS0_9StencilOpES2_S2_S2_S2_S2_)
	lis r10, 0xC;						lwz r12, 8(r3);
	lwz r0, 4(r3);
	addi r12, r12, 32;
	ori r11, r10, 0x33C;					
	cmplw cr7, r12, r0;
	ori r10, r10, 0x35C;					
	bgt- cr7, _ZN3Ice6Render14CommandContext20SetStencilOpSeparateENS0_9StencilOpES2_S2_S2_S2_S2__OutOfSpace;	stw r5, 8-32(r12);
	stw r4, 4-32(r12);
	stw r6, 12-32(r12);
	stw r7, 20-32(r12);
	stw r8, 24-32(r12);
	stw r9, 28-32(r12);
	stw r11, 0-32(r12);
	stw r10, 16-32(r12);
	stw r12, 8(r3);						blr;
SIMPLE_OUTOFSPACE_7(_ZN3Ice6Render14CommandContext20SetStencilOpSeparateENS0_9StencilOpES2_S2_S2_S2_S2_, 8, r3,r4,r5,r6,r7,r8,r9)
        .size   ._ZN3Ice6Render14CommandContext20SetStencilOpSeparateENS0_9StencilOpES2_S2_S2_S2_S2_,.-._ZN3Ice6Render14CommandContext20SetStencilOpSeparateENS0_9StencilOpES2_S2_S2_S2_S2_

## void Ice::Render::CommandContext::SetShadeModel(ShadeModel model)
SIMPLE_STUFF_ARG_1W_FUNC(_ZN3Ice6Render14CommandContext13SetShadeModelENS0_10ShadeModelE, 0x368)
										
## void Ice::Render::CommandContext::SetLogicOp(LogicOp op)
SIMPLE_STUFF_ARG_1W_FUNC(_ZN3Ice6Render14CommandContext10SetLogicOpENS0_7LogicOpE, 0x378)

## void Ice::Render::CommandContext::SetFogMode(FogMode mode)
SIMPLE_STUFF_ARG_1W_FUNC(_ZN3Ice6Render14CommandContext10SetFogModeENS0_7FogModeE, 0x8CC)

## void Ice::Render::CommandContext::SetFogRange(float fmin, float fmax)
## 31 cycles, 14 instructions, 56 bytes
FUNC(_ZN3Ice6Render14CommandContext11SetFogRangeEff)
	fsubs f3, f2, f1;					lfs f0, fOne@toc(r2);
	lis r0, 8;							lwz r9, 8(r3);
	lwz r12, 4(r3);
	addi r11, r9, 12;
	ori r0, r0, 0x8D0;
	cmplw cr7, r11, r12;	
	bgt- cr7, _ZN3Ice6Render14CommandContext11SetFogRangeEff_OutOfSpace;
	## stall f3 9
	## stall f3 10
	fdivs f3, f0, f3;
	## stall f3 2
	## stall f3 3
	## stall f3 4
	## stall f3 5
	## stall f3 6
	## stall f3 7
	## stall f3 8
	## stall f3 9
	## stall f3 10
	fmadds f2, f2, f3, f0;
	fneg f3, f3;
	## stall f2 3
	## stall f2 4
	## stall f2 5
	## stall f2 6
	stw r11, 8(r3);
	stw r0, 0(r9);
	stfs f2, 4(r9);
	stfs f3, 8(r9);						blr;
SIMPLE_OUTOFSPACE_1R_2F(_ZN3Ice6Render14CommandContext11SetFogRangeEff, 3, r3, f1,f2)
        .size   ._ZN3Ice6Render14CommandContext11SetFogRangeEff,.-._ZN3Ice6Render14CommandContext11SetFogRangeEff

## void Ice::Render::CommandContext::SetFogDensity(float density)
## cycles, instructions, bytes
FUNC(_ZN3Ice6Render14CommandContext13SetFogDensityEf)
	lis r0, 0x8;						lfs f0, fDensity@toc(r2);
	lis r4, 0x3fc0;						lwz r9, 8(r3);
	fmuls f0, f1, f0;					lwz r12, 4(r3);
	addi r11, r9, 12;				
	cmplw cr7, r11, r12;
	ori r0, r0, 0x8D0;					bgt- cr7, _ZN3Ice6Render14CommandContext13SetFogDensityEf_OutOfSpace;
	stw r0, 0(r9);
	stw r11, 8(r3);
	stw r4, 4(r9);
	stfs f0, 8(r9);						
	blr;
SIMPLE_OUTOFSPACE_1R_1F(_ZN3Ice6Render14CommandContext13SetFogDensityEf, 3, r3, f1)
        .size   ._ZN3Ice6Render14CommandContext13SetFogDensityEf,.-._ZN3Ice6Render14CommandContext13SetFogDensityEf
 
## void Ice::Render::CommandContext::SetDepthBounds(float dmin, float dmax)
## 6 cycles, 9 instructions, 36 bytes
FUNC(_ZN3Ice6Render14CommandContext14SetDepthBoundsEff)
	lis r0, 0x8;						lwz r9, 8(3);
	lwz r12, 4(r3);
	addi r11, r9, 12;
	ori r0, r0, 0x384;
	cmplw cr7, r11, r12;
	bgt- cr7, _ZN3Ice6Render14CommandContext14SetDepthBoundsEff_OutOfSpace;	stfs f1, 4(r9);						
	stfs f2, 8(r9);						
	stw r11, 8(r3);
	stw r0, 0(r9);						blr;
SIMPLE_OUTOFSPACE_1R_2F(_ZN3Ice6Render14CommandContext14SetDepthBoundsEff, 3, r3, f1,f2)
        .size   ._ZN3Ice6Render14CommandContext14SetDepthBoundsEff,.-._ZN3Ice6Render14CommandContext14SetDepthBoundsEff

## void Ice::Render::CommandContext::SetViewport(U32 left, U32 top, U32 width, U32 height, float dmin, float dmax)
## 55 cycles, 54 instructions, 216 bytes
FUNC(_ZN3Ice6Render14CommandContext11SetViewportEjjjjff)
	lis r8, 8;							lwz r9, 8(r3);
	lis r11, 8;							lwz r12, 4(r3);
	addi r0, r9, 0x3C;					
	std r6, -16(r1);
	cmplw cr7, r0, r12;
	ori r8, r8, 0xa00;					
	bgt- cr7, _ZN3Ice6Render14CommandContext11SetViewportEjjjjff_OutOfSpace;	std r7, -24(r1);
	ori r11, r11, 0x394;					
	std r4, -32(r1);
	rlwimi r4, r6, 16, 0, 15;				
	std r5, -40(r1);
	rlwimi r5, r7, 16, 0, 15;				
	stw r8, 0(r9);
	stw r11, 12(r9);
	stw r5, 8(r9);
	stw r4, 4(r9);
	lis r10, 0x20;						
	li r5, 0;
	ori r10, r10, 0xa20;					
	stw r5, 56(r9);
	fsubs f8, f2, f1;					lfd f0, -16(r1);
	fcfid f0, f0;						lfd f7, -24(r1);
	fcfid f7, f7;						lfd f11, -32(r1);
	fadds f10, f1, f2;					stfs f1, 16(r9);
	fcfid f11, f11;					
	## stall f0 5
	## stall f0 6
	## stall f0 7
	## stall f0 8
	## stall f0 9
	## stall f0 10
	frsp f9, f0;						lfs f0, fHalf@toc(r2);
	frsp f13, f7;						lfd f7, -40(r1);
	fcfid f7, f7;						stfs f2, 20(r9);
	fmuls f8, f8, f0;					
	frsp f12, f11;
	fmuls f10, f10, f0;
	## stall f9 7
	## stall f9 8
	## stall f9 9
	## stall f9 10
	fmuls f9, f9, f0;
	fmuls f13, f13, f0;				
	frsp f11, f7;					
	fadds f12, f9, f12;				
	fadds f11, f13, f11;			
	fneg f13, f13;					
	stfs f10, 36(r9);
	stfs f9, 44(r9);
	stfs f8, 52(r9);
	stw f5, 40(r9);
	stw f10, 24(r9);
	stw r0, 8(r3);
	stfs f12, 28(r9);
	stfs f11, 32(r9);
	stfs f13, 48(r9);					blr;
SIMPLE_OUTOFSPACE_5R_2F(_ZN3Ice6Render14CommandContext11SetViewportEjjjjff, 15, r3,r4,r5,r6,r7, f1,f2)
        .size   ._ZN3Ice6Render14CommandContext11SetViewportEjjjjff,.-._ZN3Ice6Render14CommandContext11SetViewportEjjjjff

## void Ice::Render::CommandContext::SetViewport(U32 left, U32 top, U32 width, U32 height, float dmin, float dmax, float scale[4], float bias[4])
## 30 cycles, 35 instructions, 140 bytes
FUNC(_ZN3Ice6Render14CommandContext11SetViewportEjjjjffPfS2_)
	rlwimi r4, r6, 16, 0, 15;			lwz r9, 8(r3);
	rlwimi r5, r7, 16, 0, 15;			lwz r0, 4(r3);
	addi r12, r9, 0x3C;					
	## stall
	cmplw cr7, r12, r0;
	bgt- cr7, _ZN3Ice6Render14CommandContext11SetViewportEjjjjffPfS2__OutOfSpace;	lwz r6, 116(r1);
	lis r0, 0x8;						
	stw r4, 4(r9);
	lis r8, 0x20;						lwz r7, 0(r6);
	ori r11, r0, 0x394;					
	stw r5, 8(r9);
	ori r0, r0, 0xA00;					
	stw r7, 28(r9);
	ori r8, r8, 0xA20;					
	stw r11, 12(r9);
	stw r0, 0(r9);
	stw r8, 24(r9);
	lwz r0, 4(r6);
	lwz r11, 8(r6);
	lwz r8, 12(r6);
	stw r0, 32(r9);
	stw r11, 36(r9);
	lwz r0, 0(r10);
	stw r8, 40(r9);
	lwz r11, 4(r10);
	stw r0, 44(r9);
	lwz r7, 8(r10);
	stw r11, 48(r9);
	lwz r8, 12(r10);
	stw r12, 8(r3);
	stw r7, 52(r9);
	stw r8, 56(r9);
	stfs f1, 16(r9);
	stfs f2, 20(r9);					blr;
SIMPLE_OUTOFSPACE_6R_2F(_ZN3Ice6Render14CommandContext11SetViewportEjjjjffPfS2_, 15, r3,r4,r5,r6,r7,r10, f1,f2)
        .size   ._ZN3Ice6Render14CommandContext11SetViewportEjjjjffPfS2_,.-._ZN3Ice6Render14CommandContext11SetViewportEjjjjffPfS2_

## void Ice::Render::CommandContext::SetScissor(U32 left, U32 top, U32 width, U32 height)
## 9 cycles, 11 instructions, 44 bytes
FUNC(_ZN3Ice6Render14CommandContext10SetScissorEjjjj)
	lis r0, 8;							lwz r9, 8(r3);
	rlwimi r4, r6, 16, 0, 15;			lwz r12, 4(r3);
	addi r11, r9, 12;						
	ori r0, r0, 0x8C0;
	cmplw cr7, r11, r12;
	rlwimi r5, r7, 16, 0, 15;				
	bgt- cr7, _ZN3Ice6Render14CommandContext10SetScissorEjjjj_OutOfSpace;	stw r4, 4(r9);
	stw r0, 0(r9);
	stw r5, 8(r9);
	stw r11, 8(r3);						blr;
SIMPLE_OUTOFSPACE_5(_ZN3Ice6Render14CommandContext10SetScissorEjjjj, 3, r3,r4,r5,r6,r7)
        .size   ._ZN3Ice6Render14CommandContext10SetScissorEjjjj,.-._ZN3Ice6Render14CommandContext10SetScissorEjjjj

## void Ice::Render::CommandContext::SetDepthFunc(ComparisonFunc func)
SIMPLE_STUFF_ARG_1W_FUNC(_ZN3Ice6Render14CommandContext12SetDepthFuncENS0_14ComparisonFuncE, 0xA6C)

## void Ice::Render::CommandContext::SetPolygonOffset(float factor, float units)
## 11 cycles, 10 instructions, 40 bytes
FUNC(_ZN3Ice6Render14CommandContext16SetPolygonOffsetEff)
	fadds f3, f2, f2;					lwz r12, 4(r3);
	lis r0, 8;							lwz r9, 8(r3);
	addi r11, r9, 12;
	ori r0, r0, 0xA78;
	cmplw cr7, r11, r12;
	bgt- cr7, _ZN3Ice6Render14CommandContext16SetPolygonOffsetEff_OutOfSpace;	stfs r1, 4(r9);						
	stw r11, 8(r3);
	stw r0, 0(r9);
	## stall 2 9
	## stall 2 10
	stfs f3, 8(r9);						blr;
SIMPLE_OUTOFSPACE_1R_2F(_ZN3Ice6Render14CommandContext16SetPolygonOffsetEff, 3, r3, f1,f2)
        .size   ._ZN3Ice6Render14CommandContext16SetPolygonOffsetEff,.-._ZN3Ice6Render14CommandContext16SetPolygonOffsetEff

## NOTE: This could be faster, but this function is rarely used in practice, as such it has been optimized for code size.
## void Ice::Render::CommandContext::SetPolygonStipplePattern(const void *pattern)
## 262 cycles, 13 instructions, 52 bytes
FUNC(_ZN3Ice6Render14CommandContext24SetPolygonStipplePatternEPKv)
    lis r0, 0x80;						lwz r9, 8(r3);
    li r5, 32;							lwz r10, 0(r4);
    addi r11, r9, 0x84;					lwz r12, 4(r3);
    cmplw cr7, r11, r12;
    bgt- cr7, _ZN3Ice6Render14CommandContext24SetPolygonStipplePatternEPKv_OutOfSpace;
    ori r0, r0, 0x1480;		        
    mtctr r5;						
    nop;								
    stw r0, 0(r9);
irsSpspLoop:
	stwu r10, 4(r9);
	lwzu r10, 4(r4);					bdnz irsSpspLoop;
	addi r9, r9, 4;
	stw r9, 8(r3);
	blr;
SIMPLE_OUTOFSPACE_2(_ZN3Ice6Render14CommandContext24SetPolygonStipplePatternEPKv, 33, r3,r4)
        .size   ._ZN3Ice6Render14CommandContext24SetPolygonStipplePatternEPKv,.-._ZN3Ice6Render14CommandContext24SetPolygonStipplePatternEPKv

## void Ice::Render::CommandContext::SetLineStipplePattern(U32 factor, U32 pattern)
## 9 cycles, 11 instructions, 44 bytes
FUNC(_ZN3Ice6Render14CommandContext21SetLineStipplePatternEjj)
    lis r11, 0x4;						lwz r9, 8(r3);
    cntlzw r6, r4;						lwz r12, 4(r3);
    addi r0, r9, 8;    					
    ori r11, r11, 0x1DB8;
    cmplw cr7, r0, r12;
    subfic r6, r6, 31;					
    bgt- cr7, _ZN3Ice6Render14CommandContext21SetLineStipplePatternEjj_OutOfSpace;	stw r11, 0(r9);
	rlwimi r6, r5, 16, 0, 15;      
	stw r0, 8(r3);
	stw r6, 4(r9);						blr;
SIMPLE_OUTOFSPACE_3(_ZN3Ice6Render14CommandContext21SetLineStipplePatternEjj, 2, r3,r4,r5)
        .size   ._ZN3Ice6Render14CommandContext21SetLineStipplePatternEjj,.-._ZN3Ice6Render14CommandContext21SetLineStipplePatternEjj

## void Ice::Render::CommandContext::SetLineWidth(U32 width)
SIMPLE_STUFF_ARG_1W_FUNC(_ZN3Ice6Render14CommandContext12SetLineWidthEj, 0x3B8)

## void Ice::Render::CommandContext::SetClipFunc(ClipFunc func0, ClipFunc func1, ClipFunc func2, ClipFunc func3, ClipFunc func4, ClipFunc func5)
## 14 cycles, 13 instructions, 52 bytes
FUNC(_ZN3Ice6Render14CommandContext11SetClipFuncENS0_8ClipFuncES2_S2_S2_S2_S2_)
	rlwimi r4, r5, 4, 0, 27;			lwz r10, 8(r3);
	lis r0, r4;							lwz r12, 4(r3);
	addi r11, r10, 8;
	rlwimi r4, r6, 8, 0, 23;
    cmplw cr7, r11, r12;
	rlwimi r4, r7, 12, 0, 19;
	bgt- cr7, _ZN3Ice6Render14CommandContext11SetClipFuncENS0_8ClipFuncES2_S2_S2_S2_S2__OutOfSpace;
	rlwimi r4, r8, 16, 0, 15;
	## stall
	rlwimi r4, r9, 20, 0, 11;
	ori r0, r0, 0x1478;
	stw r4, 4(r10);
	stw r11, 8(r3);
	stw r0, 0(r10);						blr;
SIMPLE_OUTOFSPACE_7(_ZN3Ice6Render14CommandContext11SetClipFuncENS0_8ClipFuncES2_S2_S2_S2_S2_, 2, r3,r4,r5,r6,r7,r8,r9)
        .size   ._ZN3Ice6Render14CommandContext11SetClipFuncENS0_8ClipFuncES2_S2_S2_S2_S2_,.-._ZN3Ice6Render14CommandContext11SetClipFuncENS0_8ClipFuncES2_S2_S2_S2_S2_

## void Ice::Render::CommandContext::SetPrimitiveRestartIndex(U32 index)
SIMPLE_STUFF_ARG_1W_FUNC(_ZN3Ice6Render14CommandContext24SetPrimitiveRestartIndexEj, 0x1DB0)

## void Ice::Render::CommandContext::SetPolygonMode(PolygonMode mode)
## 6 cycles, 9 instructions, 36 bytes
SIMPLE_STUFF_2ARG_2W_FUNC(_ZN3Ice6Render14CommandContext14SetPolygonModeENS0_11PolygonModeE, 0x1828, 4, 4)

## void Ice::Render::CommandContext::SetPolygonModeSeparate(PolygonMode modeFront, PolygonMode modeBack)
SIMPLE_STUFF_2ARG_2W_FUNC(_ZN3Ice6Render14CommandContext22SetPolygonModeSeparateENS0_11PolygonModeES2_, 0x1828, 4, 5)

## void Ice::Render::CommandContext::SetCullFace(CullFace cull)
SIMPLE_STUFF_ARG_1W_FUNC(_ZN3Ice6Render14CommandContext11SetCullFaceENS0_8CullFaceE, 0x1830)

## void Ice::Render::CommandContext::SetFrontFace(FrontFace front)
SIMPLE_STUFF_ARG_1W_FUNC(_ZN3Ice6Render14CommandContext12SetFrontFaceENS0_9FrontFaceE, 0x1834)

## void Ice::Render::CommandContext::SetClearColor(BgraColor color)
SIMPLE_STUFF_ARG_1W_FUNC(_ZN3Ice6Render14CommandContext13SetClearColorEj, 0x1D90)

## void Ice::Render::CommandContext::SetClearDepthStencil(U32 value)
SIMPLE_STUFF_ARG_1W_FUNC(_ZN3Ice6Render14CommandContext20SetClearDepthStencilEj, 0x1D8C)

## void Ice::Render::CommandContext::Clear(U32 flags)
SIMPLE_STUFF_ARG_1W_FUNC(_ZN3Ice6Render14CommandContext5ClearEj, 0x1D94)

## void Ice::Render::CommandContext::SetMultisampleParameters(bool enabled, bool alphaToCoverage, bool alphaToOne, U16 mask)
## 10 cycles, 11 instructions, 44 bytes
FUNC(_ZN3Ice6Render14CommandContext24SetMultisampleParametersEbbbt)
	rlwimi r4, r5, 4, 0, 27;			lwz r9, 8(r3);
	lis r0, 4;							lwz r12, 4(r3);
	addi r11, r9, 8;
	rlwimi r4, r6, 8, 0, 23;
	cmplw cr7, r11, r12;
	rlwimi r4, r7, 16, 0, 15;
	ori r0, r0, 0x1D7C;					bgt- cr7, _ZN3Ice6Render14CommandContext24SetMultisampleParametersEbbbt_OutOfSpace
	stw r4, 4(r9);
	stw r11, 8(r3);
	stw r0, 0(r9);						blr;
SIMPLE_OUTOFSPACE_5(_ZN3Ice6Render14CommandContext24SetMultisampleParametersEbbbt, 2, r3,r4,r5,r6,r7)
        .size   ._ZN3Ice6Render14CommandContext24SetMultisampleParametersEbbbt,.-._ZN3Ice6Render14CommandContext24SetMultisampleParametersEbbbt

## void Ice::Render::CommandContext::SetPointSize(float psize)
## 6 cycles, 9 instructions, 36 bytes
FUNC(_ZN3Ice6Render14CommandContext12SetPointSizeEf)
	lis r0, 4;							lwz r9, 8(r3);
	lwz r12, 4(r3);
	addi r11, r9, 8;
	ori r0, r0, 0x1EE0;
	cmplw cr7, r11, r12;
	bgt- cr7, _ZN3Ice6Render14CommandContext12SetPointSizeEf_OutOfSpace;	stfs f1, 4(r9);						
	stw r11, 8(r3);
	stw r0, 0(r9);						blr;
SIMPLE_OUTOFSPACE_1R_1F(_ZN3Ice6Render14CommandContext12SetPointSizeEf, 2, r3, f1)
        .size   ._ZN3Ice6Render14CommandContext12SetPointSizeEf,.-._ZN3Ice6Render14CommandContext12SetPointSizeEf

## void Ice::Render::CommandContext::SetPointSpriteParameters(bool enabled, U32 texcoordMask, SpriteMode mode)
## 8 cycles, 10 instructions, 40 bytes
FUNC(_ZN3Ice6Render14CommandContext24SetPointSpriteParametersEbjNS0_10SpriteModeE)
	rlwimi r4, r6, 1, 0, 30;			lwz r9, 8(r3);
	lis r0, 4;							lwz r12, 4(r3);
	addi r11, r9, 8;
	rlwimi r4, r5, 8, 0, 23;		
	cmplw cr7, r11, r12;
	ori r0, r0, 0x1EE8;					
	bgt- cr7, _ZN3Ice6Render14CommandContext24SetPointSpriteParametersEbjNS0_10SpriteModeE_OutOfSpace;	stw r4, 4(r9);
	stw r11, 8(r3);
	stw r0, 0(r9);						blr;
SIMPLE_OUTOFSPACE_4(_ZN3Ice6Render14CommandContext24SetPointSpriteParametersEbjNS0_10SpriteModeE, 2, r3,r4,r5,r6)
        .size   ._ZN3Ice6Render14CommandContext24SetPointSpriteParametersEbjNS0_10SpriteModeE,.-._ZN3Ice6Render14CommandContext24SetPointSpriteParametersEbjNS0_10SpriteModeE

## void Ice::Render::CommandContext::SetVertexAttribPointer(U32 index, U32 offset, AttribContext context)
## 8 cycles, 10 instructions, 40 bytes
FUNC(_ZN3Ice6Render14CommandContext22SetVertexAttribPointerEjjNS0_13AttribContextE)
    slwi r7, r4, 2;						lwz r9, 8(r3);
    or r6, r5, r6;						lwz r12, 4(r3);
    addi r0, r9, 8;						
    addi r7, r7, 0x1680;
	cmplw cr7, r0, r12;
    oris r7, r7, 0x4;
    bgt- cr7, _ZN3Ice6Render14CommandContext22SetVertexAttribPointerEjjNS0_13AttribContextE_OutOfSpace;	stw r6, 4(r9);
	stw r0, 8(r3);
	stw r7, 0(r9);						blr;
SIMPLE_OUTOFSPACE_4(_ZN3Ice6Render14CommandContext22SetVertexAttribPointerEjjNS0_13AttribContextE, 2, r3,r4,r5,r6)
        .size   ._ZN3Ice6Render14CommandContext22SetVertexAttribPointerEjjNS0_13AttribContextE,.-._ZN3Ice6Render14CommandContext22SetVertexAttribPointerEjjNS0_13AttribContextE

## void Ice::Render::CommandContext::SetVertexAttribBaseOffset(U32 baseOffset)
SIMPLE_STUFF_ARG_1W_FUNC(_ZN3Ice6Render14CommandContext25SetVertexAttribBaseOffsetEj, 0x1738)

## void Ice::Render::CommandContext::SetVertexAttribFormat(U32 index, AttribType type, AttribCount count, U32 stride, U32 divider)
## 10 cycles, 12 instructions, 48 bytes
FUNC(_ZN3Ice6Render14CommandContext21SetVertexAttribFormatEjNS0_10AttribTypeENS0_11AttribCountEjj)
	rlwimi r5, r6, 4, 0, 27;			lwz r9, 8(r3);
	slwi r10, r4, 2;					lwz r12, 4(r3);
	addi r0, r9, 8;						
	rlwimi r5, r7, 8, 0, 23;
	cmplw cr7, r0, r12;
	addi r10, r10, 0x1740;
	rlwimi r5, r8, 16, 0, 15;			bgt- cr7, _ZN3Ice6Render14CommandContext21SetVertexAttribFormatEjNS0_10AttribTypeENS0_11AttribCountEjj_OutOfSpace;
	oris r10, r10, 4;
	stw r5, 4(r9);
	stw r10, 0(r9);
	stw r0, 8(r3);						blr;
SIMPLE_OUTOFSPACE_6(_ZN3Ice6Render14CommandContext21SetVertexAttribFormatEjNS0_10AttribTypeENS0_11AttribCountEjj, 2, r3,r4,r5,r6,r7,r8)
        .size   ._ZN3Ice6Render14CommandContext21SetVertexAttribFormatEjNS0_10AttribTypeENS0_11AttribCountEjj,.-._ZN3Ice6Render14CommandContext21SetVertexAttribFormatEjNS0_10AttribTypeENS0_11AttribCountEjj

## void Ice::Render::CommandContext::SetVertexAttribFrequencyMode(U32 mode)
## 7 cycles, 8 instructions, 32 bytes
FUNC(_ZN3Ice6Render14CommandContext28SetVertexAttribFrequencyModeEj)
	lis r0, 4;							lwz r9, 8(r3);
	lwz r12, 4(r3);
	addi r11, r9, 8;
	ori r0, r0, 0x1FC0;
	cmplw cr7, r11, r12;
	bgt- cr7, _ZN3Ice6Render14CommandContext28SetVertexAttribFrequencyModeEj_OutOfSpace;	stw r4, 4(r9);
	stw r11, 8(r3);
	stw r0, 0(r9);						blr;
SIMPLE_OUTOFSPACE_2(_ZN3Ice6Render14CommandContext28SetVertexAttribFrequencyModeEj, 2, r3,r4)
        .size   ._ZN3Ice6Render14CommandContext28SetVertexAttribFrequencyModeEj,.-._ZN3Ice6Render14CommandContext28SetVertexAttribFrequencyModeEj

## void Ice::Render::CommandContext::DisableVertexAttribArray(U32 index)
## 8 cycles, 10 instructions, 40 bytes
FUNC(_ZN3Ice6Render14CommandContext24DisableVertexAttribArrayEj)
	slwi r6, r4, 2;						lwz r9, 8(r3);
	li r5, 2;							lwz r12, 4(r3);
	addi r11, r9, 8;						
	addi r6, r6, 0x1740;
	cmplw cr7, r11, r12;
	bgt- cr7, _ZN3Ice6Render14CommandContext24DisableVertexAttribArrayEj_OutOfSpace;	stw r5, 4(r9);
	oris r6, r6, 4;
	stw r11, 8(r3);
	stw r6, 0(r9);						blr;
SIMPLE_OUTOFSPACE_2(_ZN3Ice6Render14CommandContext24DisableVertexAttribArrayEj, 2, r3,r4)
        .size   ._ZN3Ice6Render14CommandContext24DisableVertexAttribArrayEj,.-._ZN3Ice6Render14CommandContext24DisableVertexAttribArrayEj

## void Ice::Render::CommandContext::SetVertexAttrib1f(U32 index, float x)
## 7 cycles, 9 instructions, 36 bytes
FUNC(_ZN3Ice6Render14CommandContext17SetVertexAttrib1fEjf)
	slwi r5, r4, 2;						lwz r9, 8(r3);
	lwz r12, 4(r3);
	addi r11, r9, 8;					
	addi r5, r5, 0x1E40;
	cmplw cr7, r11, r12;
	oris r5, r5, 4;						bgt- cr7, _ZN3Ice6Render14CommandContext17SetVertexAttrib1fEjf_OutOfSpace;
	stfs r1, 4(r9);						
	stw r11, 8(r3);
	stw r5, 0(r9);						blr;
SIMPLE_OUTOFSPACE_2R_1F(_ZN3Ice6Render14CommandContext17SetVertexAttrib1fEjf, 2, r3,r4, f1)
        .size   ._ZN3Ice6Render14CommandContext17SetVertexAttrib1fEjf,.-._ZN3Ice6Render14CommandContext17SetVertexAttrib1fEjf

## void Ice::Render::CommandContext::SetVertexAttrib1f(U32 index, const float *data)
## 8 cycles, 10 instructions, 40 bytes
FUNC(_ZN3Ice6Render14CommandContext17SetVertexAttrib1fEjPKf)
	slwi r7, r4, 2;						lwz r9, 8(r3);
	lwz r6, 0(r5);
	addi r7, r7, 0x1E40;				
	addi r11, r9, 8;					lwz r12, 4(r3);
	cmplw cr7, r11, r12;
	oris r7, r7, 4;
	bgt- cr7, _ZN3Ice6Render14CommandContext17SetVertexAttrib1fEjPKf_OutOfSpace;	stw r6, 4(r9);
	stw r11, 8(r3);
	stw r7, 0(r9);						blr;
SIMPLE_OUTOFSPACE_3(_ZN3Ice6Render14CommandContext17SetVertexAttrib1fEjPKf, 2, r3,r4,r5)
        .size   ._ZN3Ice6Render14CommandContext17SetVertexAttrib1fEjPKf,.-._ZN3Ice6Render14CommandContext17SetVertexAttrib1fEjPKf

## void Ice::Render::CommandContext::SetVertexAttrib2f(U32 index, float x, float y)
## 7 cycles, 10 instructions, 40 bytes
FUNC(_ZN3Ice6Render14CommandContext17SetVertexAttrib2fEjff)
	slwi r5, r4, 3;						lwz r9, 8(r3);
	lwz r12, 4(r3);
	addi r11, r9, 12;
	addi r5, r5, 0x1880;					
	cmplw cr7, r11, r12;
	oris r5, r5, 8;							bgt- cr7, _ZN3Ice6Render14CommandContext17SetVertexAttrib2fEjff_OutOfSpace;
	stfs f1, 4(r9);						
	stfs f2, 8(r9);						
	stw r11, 8(r3);
	stw r5, 0(r9);						blr;
SIMPLE_OUTOFSPACE_2R_2F(_ZN3Ice6Render14CommandContext17SetVertexAttrib2fEjff, 3, r3,r4, f1,f2)
        .size   ._ZN3Ice6Render14CommandContext17SetVertexAttrib2fEjff,.-._ZN3Ice6Render14CommandContext17SetVertexAttrib2fEjff

## void Ice::Render::CommandContext::SetVertexAttrib2f(U32 index, const float *data)
## 9 cycles, 12 instructions, 48 bytes
FUNC(_ZN3Ice6Render14CommandContext17SetVertexAttrib2fEjPKf)
	slwi r8, r4, 3;						lwz r9, 8(r3);
	lwz r6, 0(r5);
	addi r8, r8, 0x1880;					
	addi r11, r9, 12;					lwz r12, 4(r3);
	oris r8, r8, 8;						lwz r7, 4(r5);
	cmplw cr7, r11, r12;
	stw r8, 0(r9);
	bgt- cr7, _ZN3Ice6Render14CommandContext17SetVertexAttrib2fEjPKf_OutOfSpace;	stw r6, 4(r9);
	stw r7, 8(r9);
	stw r11, 8(r3);						blr;
SIMPLE_OUTOFSPACE_3(_ZN3Ice6Render14CommandContext17SetVertexAttrib2fEjPKf, 3, r3,r4,r5)
        .size   ._ZN3Ice6Render14CommandContext17SetVertexAttrib2fEjPKf,.-._ZN3Ice6Render14CommandContext17SetVertexAttrib2fEjPKf

## void Ice::Render::CommandContext::SetVertexAttrib3f(U32 index, float x, float y, float z)
## 7 cycles, 11 instructions, 44 bytes
FUNC(_ZN3Ice6Render14CommandContext17SetVertexAttrib3fEjfff)
	slwi r5, r4, 4;						lwz r9, 8(r3);
	lwz r12, 4(r3);
	addi r11, r9, 16;
	addi r5, r5, 0x1500;
	cmplw cr7, r11, r12;
	bgt- cr7, _ZN3Ice6Render14CommandContext17SetVertexAttrib3fEjfff_OutOfSpace;	stfs f1, 4(r9);						
	stfs f2, 8(r9);						oris r5, r5, 12;
	stfs f3, 12(r9);					
	stw r11, 8(r3);
	stw r5, 0(r9);						blr;
SIMPLE_OUTOFSPACE_2R_3F(_ZN3Ice6Render14CommandContext17SetVertexAttrib3fEjfff, 4, r3,r4, f1,f2,f3)
        .size   ._ZN3Ice6Render14CommandContext17SetVertexAttrib3fEjfff,.-._ZN3Ice6Render14CommandContext17SetVertexAttrib3fEjfff

## void Ice::Render::CommandContext::SetVertexAttrib3f(U32 index, const float *data)
## 12 cycles, 14 instructions, 56 bytes
FUNC(_ZN3Ice6Render14CommandContext17SetVertexAttrib3fEjPKf)
	slwi r10, r4, 4;					lwz r9, 8(r3);
	lwz r6, 0(r5);						
	lwz r12, 4(r3);						addi r11, r9, 16;
	lwz r7, 4(r5);						addi r10, r10, 0x1500;
	lwz r8, 8(r5);						cmplw cr7, r11, r12;
	oris r10, r10, 12;					bgt- cr7, _ZN3Ice6Render14CommandContext17SetVertexAttrib3fEjPKf_OutOfSpace;
	stw r6, 4(r9);
	stw r7, 8(r9);
	stw r8, 12(r9);
	stw r11, 8(r3);
	stw r10, 0(r9);						blr;
SIMPLE_OUTOFSPACE_3(_ZN3Ice6Render14CommandContext17SetVertexAttrib3fEjPKf, 4, r3,r4,r5)
        .size   ._ZN3Ice6Render14CommandContext17SetVertexAttrib3fEjPKf,.-._ZN3Ice6Render14CommandContext17SetVertexAttrib3fEjPKf

## void Ice::Render::CommandContext::SetVertexAttrib4f(U32 index, float x, float y, float z, float w)
## 8 cycles, 12 instructions, 48 bytes
FUNC(_ZN3Ice6Render14CommandContext17SetVertexAttrib4fEjffff)
	slwi r5, r4, 4;						lwz r9, 8(r3);
	lwz r12, 4(r3);
	addi r11, r9, 20;
	addi r5, r5, 0x1C00;
	cmplw cr7, r11, r12;
	oris r5, r5, 16;					bgt- cr7, _ZN3Ice6Render14CommandContext17SetVertexAttrib4fEjffff_OutOfSpace;
	stfs f1, 4(r9);						
	stfs f2, 8(r9);						
	stfs f3, 12(r9);					
	stfs f4, 16(r9);
	stw r11, 8(r3);
	stw r5, 0(r9);						blr;
SIMPLE_OUTOFSPACE_2R_4F(_ZN3Ice6Render14CommandContext17SetVertexAttrib4fEjffff, 5, r3,r4, f1,f2,f3,f4)
        .size   ._ZN3Ice6Render14CommandContext17SetVertexAttrib4fEjffff,.-._ZN3Ice6Render14CommandContext17SetVertexAttrib4fEjffff

## void Ice::Render::CommandContext::SetVertexAttrib4f(U32 index, const float *data)
## 14 cycles, 16 instructions, 64 bytes
FUNC(_ZN3Ice6Render14CommandContext17SetVertexAttrib4fEjPKf)
	slwi r12, r4, 4;					lwz r9, 8(r3);
	lwz r0, 4(r3);
	addi r11, r9, 20;
	lwz r6, 0(r5);						addi r12, r12, 0x1C00;
	lwz r7, 4(r5);						cmplw cr7, r11, r0;
	lwz r8, 8(r5);						oris r12, r12, 16;
	lwz r10, 12(r5);					bgt- cr7, _ZN3Ice6Render14CommandContext17SetVertexAttrib4fEjPKf_OutOfSpace;
	stw r6, 4(r9);
	stw r7, 8(r9);
	stw r8, 12(r9);
	stw r10, 16(r9);
	stw r11, 8(r3);
	stw r12, 0(r9);						blr;
SIMPLE_OUTOFSPACE_3(_ZN3Ice6Render14CommandContext17SetVertexAttrib4fEjPKf, 5, r3,r4,r5)
        .size   ._ZN3Ice6Render14CommandContext17SetVertexAttrib4fEjPKf,.-._ZN3Ice6Render14CommandContext17SetVertexAttrib4fEjPKf

## void Ice::Render::CommandContext::SetVertexAttrib2s(U32 index, I16 x, I16 y)
## 8 cycles, 10 instructions, 40 bytes
FUNC(_ZN3Ice6Render14CommandContext17SetVertexAttrib2sEjss)
	slwi r7, r4, 2;						lwz r9, 8(r3);
	rlwimi r5, r6, 16, 0, 15;			lwz r12, 4(r3);
	addi r11, r9, 8;
	addi r7, r7, 0x1900;
	cmplw cr7, r11, r12;
	oris r7, r7, 4;						bgt- cr7, _ZN3Ice6Render14CommandContext17SetVertexAttrib2sEjss_OutOfSpace;
	stw r5, 4(r9);
	stw r11, 8(r3);
	stw r7, 0(r9);						blr;
SIMPLE_OUTOFSPACE_4(_ZN3Ice6Render14CommandContext17SetVertexAttrib2sEjss, 2, r3,r4,r5,r6)
        .size   ._ZN3Ice6Render14CommandContext17SetVertexAttrib2sEjss,.-._ZN3Ice6Render14CommandContext17SetVertexAttrib2sEjss

## void Ice::Render::CommandContext::SetVertexAttrib4s(U32 index, I16 x, I16 y, I16 z, I16 w)
## 10 cycles, 12 instructions, 48 bytes
FUNC(_ZN3Ice6Render14CommandContext17SetVertexAttrib4sEjssss)
	slwi r10, r4, 3;					lwz r9, 8(r3);
	rlwimi r5, r6, 16, 0, 15;			lwz r12, 4(r3);
	addi r11, r9, 12;
	addi r10, r10, 0x1980;
	cmplw cr7, r11, r12;
	rlwimi r7, r8, 16, 0, 15;			
	oris r10, r10, 8;					bgt- cr7, _ZN3Ice6Render14CommandContext17SetVertexAttrib4sEjssss_OutOfSpace;
	stw r5, 4(r9);
	stw r7, 8(r9);
	stw r11, 8(r3);
	stw r10, 0(r9);						blr;
SIMPLE_OUTOFSPACE_6(_ZN3Ice6Render14CommandContext17SetVertexAttrib4sEjssss, 3, r3,r4,r5,r6,r7,r8)
        .size   ._ZN3Ice6Render14CommandContext17SetVertexAttrib4sEjssss,.-._ZN3Ice6Render14CommandContext17SetVertexAttrib4sEjssss

## void Ice::Render::CommandContext::SetVertexAttrib4Ns(U32 index, I16 x, I16 y, I16 z, I16 w)
## 10 cycles, 12 instructions, 48 bytes
FUNC(_ZN3Ice6Render14CommandContext18SetVertexAttrib4NsEjssss)
	slwi r10, r4, 3;					lwz r9, 8(r3);
	rlwimi r5, r6, 16, 0, 15;			lwz r12, 4(r3);
	addi r11, r9, 12;
	addi r10, r10, 0xa80;
	cmplw cr7, r11, r12;
	rlwimi r7, r8, 16, 0, 15;			
	oris r10, r10, 8;					bgt- cr7, _ZN3Ice6Render14CommandContext18SetVertexAttrib4NsEjssss_OutOfSpace;
	stw r5, 4(r9);
	stw r7, 8(r9);
	stw r11, 8(r3);
	stw r10, 0(r9);						blr;
SIMPLE_OUTOFSPACE_6(_ZN3Ice6Render14CommandContext18SetVertexAttrib4NsEjssss, 3, r3,r4,r5,r6,r7,r8)
        .size   ._ZN3Ice6Render14CommandContext18SetVertexAttrib4NsEjssss,.-._ZN3Ice6Render14CommandContext18SetVertexAttrib4NsEjssss

## void Ice::Render::CommandContext::SetVertexAttrib4Nub(U32 index, U8 x, U8 y, U8 z, U8 w)
## 10 cycles, 12 instructions, 48 bytes
FUNC(_ZN3Ice6Render14CommandContext19SetVertexAttrib4NubEjhhhh)
	slwi r10, r4, 2;					lwz r9, 8(r3);
	rlwimi r5, r6, 8, 16, 23;			lwz r12, 4(r3);
	addi r11, r9, 8;
	rlwimi r5, r7, 16, 8, 15;
	cmplw cr7, r11, r12;
	addi r10, r10, 0x1940;
	rlwimi r5, r8, 24, 0, 7;			bgt- cr7, _ZN3Ice6Render14CommandContext19SetVertexAttrib4NubEjhhhh_OutOfSpace;
	oris r10, r10, 4;
	stw r5, 4(r9);
	stw r10, 0(r9);
	stw r11, 8(r3);						blr;
SIMPLE_OUTOFSPACE_6(_ZN3Ice6Render14CommandContext19SetVertexAttrib4NubEjhhhh, 2, r3,r4,r5,r6,r7,r8)
        .size   ._ZN3Ice6Render14CommandContext19SetVertexAttrib4NubEjhhhh,.-._ZN3Ice6Render14CommandContext19SetVertexAttrib4NubEjhhhh

## void Ice::Render::CommandContext::SetVertexAttrib4Nub(U32 index, U32 xyzwData)
FUNC(_ZN3Ice6Render14CommandContext19SetVertexAttrib4NubEjj)
	slwi r6, r4, 2;						lwz r9, 8(r3);
	lwz r12, 4(r3);
	addi r6, r6, 0x1940;				
	addi r11, r9, 8;						
	oris r6, r6, 4;		
	cmplw cr7, r11, r12;					
	addi r0, r9, 4;						
	stw r6, 0(r9);						bgt- cr7, _ZN3Ice6Render14CommandContext19SetVertexAttrib4NubEjj_OutOfSpace
	stwbrx r5, 0, r0;
	stw r11, 8(r3);						blr;
SIMPLE_OUTOFSPACE_3(_ZN3Ice6Render14CommandContext19SetVertexAttrib4NubEjj, 2, r3,r4,r5)
        .size   ._ZN3Ice6Render14CommandContext19SetVertexAttrib4NubEjj,.-._ZN3Ice6Render14CommandContext19SetVertexAttrib4NubEjj

## void Ice::Render::CommandContext::DrawArrays(DrawMode mode, U32 start, U32 count)
## 0 Verts: 3 cycles
## 1-255 Verts: 37 cycles
## 256+ Verts: 31+ cycles
## 37 instructions, 148 bytes
FUNC(_ZN3Ice6Render14CommandContext10DrawArraysENS0_8DrawModeEjj)
		addi r11, r6, 0x5FF;			lwz r10, 8(r3);
        rldicl r12, r6, 0, 56;			lwz r9, 4(r3);
		rlwinm r11, r11, 32-6, 6, 29;
        cmpdi cr0, r6, 0;
        add r11, r11, r10;				beqlr cr0;
        neg r0, 12;						
		cmplw cr7, r11, r9;
		srdi r7, r0, 63;				bgt- cr7, _ZN3Ice6Render14CommandContext10DrawArraysENS0_8DrawModeEjj_OutOfSpace;
        srdi r6, r6, 8;
        addi r8, r10, 20;				
        add r0, r6, r7;					
        stw r4, 4(r10);
        li r4, 0;				        
        sldi r11, r0, 2;				
        lis r9, 0x4;					
        add r11, r8, 11;
        slwi r0, r0, 18;
        ori r9, r9, 0x1808;				
        oris r0, r0, 0x4000;				
        stw r11, 8(r3);
        cmpdi cr7, r6, 0;				
        stw r9, -8(r11);
        ori r0, r0, 0x1814;				
        stw r9, 0(r10);
        oris r5, r5, 0xff00;				
        stw r4, -4(r11);
        addi r12, r12, -1;				
        stw r0, 8(r10);					
        beq cr7, irsDaNoBlocks;
        mtctr r6;
irsDaLoop:
        stw r5, 12(r10);
        addi r10, r10, 4;					
        addi r5, r5, 0x100;				bdnz irsDaLoop;
irsDaNoBlocks:
        cmpdi cr7, r7, 0;
		rlwimi r5, r12, 24, 0, 7;		beqlr cr7;
        stw r5, 12(r10);				blr;
SIMPLE_OUTOFSPACE_CALC_4(_ZN3Ice6Render14CommandContext10DrawArraysENS0_8DrawModeEjj, r10,r11, r3,r4,r5,r6)
        .size   ._ZN3Ice6Render14CommandContext10DrawArraysENS0_8DrawModeEjj,.-._ZN3Ice6Render14CommandContext10DrawArraysENS0_8DrawModeEjj

## void Ice::Render::CommandContext::DrawElements(DrawMode mode, U32 start, U32 count, IndexType type, U32 indexOffset, IndexContext context)
## 0 Verts: 9 cycles
## 1-255 Verts: 41 cycles
## 256+ Verts: 35+ cycles
## 43 instructions, 172 bytes
FUNC(_ZN3Ice6Render14CommandContext12DrawElementsENS0_8DrawModeEjjNS0_9IndexTypeEjNS0_12IndexContextE)
		addi r0, r6, 0x8FF;				lwz r10, 8(r3);
        rldicl r12, r6, 0, 56;			lwz r11, 4(r3);
		rlwinm r0, r0, 32-6, 6, 29;
        cmpdi cr0, r6, 0;
        add r0, r0, r10;				beqlr cr0;
        or r7, r7, r9;	
        cmplw cr7, r0, r11;					
        bgt- cr7, _ZN3Ice6Render14CommandContext12DrawElementsENS0_8DrawModeEjjNS0_9IndexTypeEjNS0_12IndexContextE_OutOfSpace;	
        stw r8, 4(r10);	## inputs 8&9 consumed.
        stw r4, 16(r10);											## inputs 4 consumed.
        neg r0, r12;
		srdi r6, r6, 8;					
		stw r7, 8(r10);												## inputs 7 consumed.
        srdi r7, r0, 63;				
        addi r8, r10, 32;				
        add r0, r6, r7;					
        li r4, 0;				        
        sldi r11, r0, 2;				
        lis r9, 0x4;					
        add r11, r8, r11;
        slwi r0, r0, 18;
        ori r9, r9, 0x1808;				
        oris r0, r0, 0x4000;				
        stw r4, -4(r11);
        lis r4, 8;
        cmpdi cr7, r6, 0;				
        stw r11, 8(r3);
        ori r4, r4, 0x181c;
        ori r0, r0, 0x1824;				
        stw r9, -8(r11);
        oris r5, r5, 0xff00;				
        stw r9, 12(r10);
        addi r12, r12, -1;				
        stw r0, 20(r10);					
        stw r4, 0(r10);					beq cr7, irsDeNoBlocks;
        mtctr r6;
irsDeLoop:
        stw r5, 24(r10);					
        addi r10, r10, 4;					
        addi r5, r5, 256;				bdnz irsDeLoop;
irsDeNoBlocks:
        cmpdi cr7, r7, 0;
		rlwimi r5, r12, 24, 0, 7;		beqlr cr7;
        stw 5, 24(10);					blr;
SIMPLE_OUTOFSPACE_CALC_7(_ZN3Ice6Render14CommandContext12DrawElementsENS0_8DrawModeEjjNS0_9IndexTypeEjNS0_12IndexContextE, r10,r0, r3,r4,r5,r6,r7,r8,r9)
        .size   ._ZN3Ice6Render14CommandContext12DrawElementsENS0_8DrawModeEjjNS0_9IndexTypeEjNS0_12IndexContextE,.-._ZN3Ice6Render14CommandContext12DrawElementsENS0_8DrawModeEjjNS0_9IndexTypeEjNS0_12IndexContextE

## void Ice::Render::CommandContext::Begin(DrawMode mode)
SIMPLE_STUFF_ARG_1W_FUNC(_ZN3Ice6Render14CommandContext5BeginENS0_8DrawModeE, 0x1808)

## void Ice::Render::CommandContext::InsertIndexArrayOffsetAndFormat(IndexType type, U32 indexOffset, IndexContext context)
FUNC(_ZN3Ice6Render14CommandContext31InsertIndexArrayOffsetAndFormatENS0_9IndexTypeEjNS0_12IndexContextE)
	lis r0, 8;							lwz r9, 8(r3);
	or r6, r4, r6;						lwz r12, 4(r3);
	addi r11, r9, 12;
	ori r0, r0, 0x181C;
	cmplw cr7, r11, r12;
	stw r0, 0(r9);						bgt- cr7, _ZN3Ice6Render14CommandContext31InsertIndexArrayOffsetAndFormatENS0_9IndexTypeEjNS0_12IndexContextE_OutOfSpace;
	stw r5, 4(r9);
	stw r6, 8(r9);
	stw r11, 8(r3);						blr;
SIMPLE_OUTOFSPACE_4(_ZN3Ice6Render14CommandContext31InsertIndexArrayOffsetAndFormatENS0_9IndexTypeEjNS0_12IndexContextE, 3, r3,r4,r5,r6)
        .size   ._ZN3Ice6Render14CommandContext31InsertIndexArrayOffsetAndFormatENS0_9IndexTypeEjNS0_12IndexContextE,.-._ZN3Ice6Render14CommandContext31InsertIndexArrayOffsetAndFormatENS0_9IndexTypeEjNS0_12IndexContextE

## void Ice::Render::CommandContext::InsertDraw(DrawType drawType, U32 start, U32 count)
FUNC(_ZN3Ice6Render14CommandContext10InsertDrawENS0_8DrawTypeEjj)
	addi r11, r6, 0x1FF;			lwz r10, 8(r3);
	rldicl r12, r6, 0, 56;			lwz r9, 4(r3);
	rlwinm r11, r11, 32-6, 6, 29;
	cmpdi cr0, r6, 0;
	add r11, r11, r10;				beqlr cr0;
	neg r0, r12;					
	cmplw cr7, r11, r9;
	bgt- cr7, _ZN3Ice6Render14CommandContext10InsertDrawENS0_8DrawTypeEjj_OutOfSpace; srdi r6, r6, 8;
	srdi r7, r0, 63;				
	addi r8, r10, 4;				
	add r0, r6, r7;		
	oris r5, r5, 0xff00;
	sldi r11, r0, 2;				
	slwi r0, r0, 18;
	add r11, r8, r11;
	oris r0, r0, 0x4000;
	stw r11, 8(r3);
	or r0, r0, r4;				
	cmpdi cr7, r6, 0;				
	stw r0, 0(r10);					
	addi r12, r12, -1;				
	beq cr7, irsDNoBlocks;
	mtctr r6;
irsDLoop:
	stwu r5, 4(r10);					
	addi r5, r5, 256;				bdnz irsDLoop;
irsDNoBlocks:
	nop;
	cmpdi cr7, r7, 0;
	rlwimi r5, r12, 24, 0, 7;		beqlr cr7;
	stw r5, 4(r10);					blr;
SIMPLE_OUTOFSPACE_CALC_4(_ZN3Ice6Render14CommandContext10InsertDrawENS0_8DrawTypeEjj, r10,r11, r3,r4,r5,r6)
		.size   ._ZN3Ice6Render14CommandContext10InsertDrawENS0_8DrawTypeEjj,.-._ZN3Ice6Render14CommandContext10InsertDrawENS0_8DrawTypeEjj

## void Ice::Render::CommandContext::End()
SIMPLE_STUFF_CONST_1W_FUNC(_ZN3Ice6Render14CommandContext3EndEv, 0x1808, 0)

## void Ice::Render::CommandContext::InvalidatePreTransformCache()
SIMPLE_STUFF_CONST_1W_FUNC(_ZN3Ice6Render14CommandContext27InvalidatePreTransformCacheEv, 0x1710, 0)

## void Ice::Render::CommandContext::InvalidatePostTransformCache()
SIMPLE_STUFF_CONST_1W_FUNC(_ZN3Ice6Render14CommandContext28InvalidatePostTransformCacheEv, 0x1714, 0)

## void Ice::Render::CommandContext::SelectVertexProgram(const VertexProgram *__restrict program)
## 0 literals: 15 cycles
## 1 literals: 34 cycles
## 2+ literals: 34 + n * ~14 cycles
FUNC(_ZN3Ice6Render14CommandContext19SelectVertexProgramEPKNS0_13VertexProgramE)
## r0,r5,r6,r7,r8,r9,r10,r11,r12
	lis r10, 4;						lhz r5, 0x16(r4);	## r5 = m_constantCount
	lis r11, 8;						lwz r9, 8(r3);		## cur
	rlwinm r12, r5, 2, 15, 29;
	sldi r7, r5, 4;					
	sldi r8, r5, 3;
	cmpdi cr7, r5, 0;				
	add r7, r8, r7;
	mtctr r5;						
	addi r7, r7, 28;				lhz r5, 0x02(r4);	## r5 = m_instructionSlot
	ori r0, r10, 0x1ef8;			lwz r6, 0x04(r4);	## r6 = m_vertexAttributeMask
	ori r10, r10, 0x1ea0;			lwz r8, 4(r3);		## end
	ori r11, r11, 0x1ff0;			
	add r7, r7, r9;
	stw r10, 0(r9);					
	cmplw cr0, r7, r8;				
	bgt- cr0, _ZN3Ice6Render14CommandContext19SelectVertexProgramEPKNS0_13VertexProgramE_OutOfSpace;	lwz r8, 0x0C(r4);	## r8 = m_vertexLimits
	stw r5, 4(r9);
	stw r0, 20(r9);
	addi r5, r9, 28;				lwz r7, 0x08(r4);	## r7 = m_vertexResultMask
	stw r6, 12(r9);
	stw r8, 24(r9);
	stw r7, 16(r9);
	stw r11, 8(r9);					
	stw r5, 8(r3);					
	lis r6, 0x14;		
	addi r12, r12, 39;				beqlr- cr7;
	rlwinm r12, r12, 0, 0, 27;		## r12 = (sizeof(VertexProgram) + 11 + m_constantCount * 4) & ~15;
	ori r6, r6, 0x1efc;
	add r12, r12, r4;
irsSelectVertexProgramLoop:
	lwz r0, 0x18(r4);				
	lwz r7, 0(r12);					addi r4, r4, 4;
	lwz r8, 4(r12);
	lwz r10, 8(r12);
	stw r6, 0x00(r5);				
	stw r0, 0x04(r5);
	lwz r11, 12(r12);				addi r12, r12, 16;
	stw r7, 0x08(r5);					
	stw r8, 0x0C(r5);
	stw r10, 0x10(r5);
	stw r11, 0x14(r5);				
	addi r5, r5, 0x18;				bdnz irsSelectVertexProgramLoop;
	stw r5, 8(r3);					blr;
SIMPLE_OUTOFSPACE_CALC_2(_ZN3Ice6Render14CommandContext19SelectVertexProgramEPKNS0_13VertexProgramE, r9,r7, r3,r4)
        .size   ._ZN3Ice6Render14CommandContext19SelectVertexProgramEPKNS0_13VertexProgramE,.-._ZN3Ice6Render14CommandContext19SelectVertexProgramEPKNS0_13VertexProgramE

## void Ice::Render::CommandContext::LoadVertexProgram(const VertexProgram *__restrict program)
FUNC(_ZN3Ice6Render14CommandContext17LoadVertexProgramEPKNS0_13VertexProgramE)
	lis r0, 0x4;					lhz r5, 0(r4);	## r10 = m_instructionCount
	lhz r6, 2(r4); ## r11 = m_instructionSlot								
	srdi r12, r5, 3;
	rlwinm r10, r5, 4, 0, 28;		lwz r7, 8(r3);
	rlwinm r11, r5, 31, 17, 29;		lwz r9, 4(r3);
	ori r0, r0, 0x1E9C;				lhz r8, 0x12(r4); ## r8 = m_microcodeOffset
	add r10, r10, r11;
	mtctr r12;
	addi r10, r10, 12;
	add r10, r10, r7;
	cmpdi cr0, r12, 0;
	cmplw cr7, r10, r9;
	stw r0, 0(r7);					bgt- cr7, _ZN3Ice6Render14CommandContext17LoadVertexProgramEPKNS0_13VertexProgramE_OutOfSpace;
	stw r6, 4(r7);
	add r6, r8, r4;
	addi r7, r7, 8;
	nop;
	rlwinm r4, r5, 0, 29, 31;		beq cr0, irsLvpNoBlocks;
irsLvpBlockLoop:
	lwz r0, 0(r6);					lis r5, 0x80;
	lwz r9, 4(r6);					
	lwz r11, 8(r6);					ori r5, r5, 0xB80;
	stw r0, 4(r7);
	stw r5, 0(r7);
	lwz r10, 12(r6);
	stw r9, 8(r7);
	lwz r0, 16(r6);
	stw r11, 12(r7);
	lwz r9, 20(r6);
	stw r10, 16(r7);
	lwz r11, 24(r6);
	stw r0, 20(r7);
	lwz r10, 28(r6);
	stw r9, 24(r7);
	lwz r0, 32(r6);
	stw r11, 28(r7);
	lwz r9, 36(r6);
	stw r10, 32(r7);
	lwz r11, 40(r6);
	stw r0, 36(r7);
	lwz r10, 44(r6);
	stw r9, 40(r7);
	lwz r0, 48(r6);
	stw r11, 44(r7);
	lwz r9, 52(r6);
	stw r10, 48(r7);
	lwz r11, 56(r6);
	stw r0, 52(r7);
	lwz r10, 60(r6);
	stw r9, 56(r7);
	lwz r0, 64(r6);
	stw r11, 60(r7);
	lwz r9, 68(r6);
	stw r10, 64(r7);
	lwz r11, 72(r6);
	stw r0, 68(r7);
	lwz r10, 76(r6);
	stw r9, 72(r7);
	lwz r0, 80(r6);
	stw r11, 76(r7);
	lwz r9, 84(r6);
	stw r10, 80(r7);
	lwz r11, 88(r6);
	stw r0, 84(r7);
	lwz r10, 92(r6);
	stw r9, 88(r7);
	lwz r0, 96(r6);
	stw r11, 92(r7);
	lwz r9, 100(r6);
	stw r10, 96(r7);
	lwz r11, 104(r6);
	stw r0, 100(r7);
	lwz r10, 108(r6);
	stw r9, 104(r7);
	lwz r0, 112(r6);
	stw r11, 108(r7);
	lwz r8, 116(r6);
	stw r10, 112(r7);
	lwz r11, 120(r6);
	stw r0, 116(r7);
	lwz r0, 124(r6);				addi r6, r6, 128;
	stw r8, 120(r7);
	stw r11, 124(r7);
	stw r0, 128(r7);
	addi r7, r7, 132;				bdnz irsLvpBlockLoop;
irsLvpNoBlocks:
	cmpdi cr7, r4, 0;
	stw r7, 8(r3);					beqlr cr7;
	slwi r12, r4, 20;
	addi r7, r7, 4;
	ori r0, r12, 0xB80;
	mtctr r4;
	stw r0, -4(r7);
irsLvpRemainLoop:
	lwz r0, 0(r6);					
	lwz r9, 4(r6);
	stw r0, 0(r7);
	lwz r11, 8(r6);
	stw r9, 4(r7);
	lwz r0, 12(r6);
	stw r11, 8(r7);
	addi r6, r6, 16;
	stw r0, 12(r7);
	addi r7, r7, 16;				bdnz irsLvpRemainLoop;
	stw r7, 8(r3);					blr;
SIMPLE_OUTOFSPACE_CALC_2(_ZN3Ice6Render14CommandContext17LoadVertexProgramEPKNS0_13VertexProgramE, r7,r10, r3,r4)
		.size   ._ZN3Ice6Render14CommandContext17LoadVertexProgramEPKNS0_13VertexProgramE,.-._ZN3Ice6Render14CommandContext17LoadVertexProgramEPKNS0_13VertexProgramE

## void Ice::Render::CommandContext::SetVertexProgramConstant(U32 index, float x, float y, float z, float w)
FUNC(_ZN3Ice6Render14CommandContext24SetVertexProgramConstantEjffff)
	lis r0, 0x14;				lwz r9, 8(r3);
	addi r11, r9, 0x18;			lwz r12, 4(r3);
	ori r0, r0, 0x1EFC;
	cmplw cr7, r11, r12;
	stw r0, 0(r9);				bgt- cr7, _ZN3Ice6Render14CommandContext24SetVertexProgramConstantEjffff_OutOfSpace;
	stw r4, 4(r9);
	stfs f1, 8(r9);
	stfs f2, 12(r9);
	stfs f3, 16(r9);
	stfs f4, 20(r9);
	stw r11, 8(r3);				blr;
SIMPLE_OUTOFSPACE_2R_4F(_ZN3Ice6Render14CommandContext24SetVertexProgramConstantEjffff, 6, r3,r4, f1,f2,f3,f4)
		.size   ._ZN3Ice6Render14CommandContext24SetVertexProgramConstantEjffff,.-._ZN3Ice6Render14CommandContext24SetVertexProgramConstantEjffff

## void Ice::Render::CommandContext::SetVertexProgramConstant(U32 index, const float *data)
FUNC(_ZN3Ice6Render14CommandContext24SetVertexProgramConstantEjPKf)
	lwz r9, 8(r3);				lis r0, 0x14;
	lwz r12, 4(r3);
	lwz r6, 0x00(r5);
	lwz r7, 0x04(r5);			addi r11, r9, 0x18;
	lwz r8, 0x08(r5);			ori r0, r0, 0x1EFC;
	lwz r10, 0x0C(r5);			cmplw cr7, r11, r12;
	bgt- cr7, _ZN3Ice6Render14CommandContext24SetVertexProgramConstantEjPKf_OutOfSpace;	stw r6, 0x08(r9);				
	stw r7, 0x0C(r9);			
	stw r8, 0x10(r9);
	stw r10, 0x14(r9);
	stw r0, 0x00(r9);
	stw r4, 0x04(r9);
	stw r11, 8(r3);				blr;
SIMPLE_OUTOFSPACE_3(_ZN3Ice6Render14CommandContext24SetVertexProgramConstantEjPKf, 6, r3,r4,r5)
		.size   ._ZN3Ice6Render14CommandContext24SetVertexProgramConstantEjPKf,.-._ZN3Ice6Render14CommandContext24SetVertexProgramConstantEjPKf

## void Ice::Render::CommandContext::SetVertexProgram2Constants(U32 index, const float *data)
FUNC(_ZN3Ice6Render14CommandContext26SetVertexProgram2ConstantsEjPKf)
	lwz r9, 8(r3);				lis r0, 0x24;
	lwz r12, 4(r3);
	lwz r6, 0x00(r5);
	lwz r7, 0x04(r5);			addi r11, r9, 0x28;
	lwz r8, 0x08(r5);			ori r0, r0, 0x1EFC;
	lwz r10, 0x0C(r5);			cmplw cr7, r11, r12;
	bgt- cr7, _ZN3Ice6Render14CommandContext26SetVertexProgram2ConstantsEjPKf_OutOfSpace;	stw r6, 0x08(r9);				
	stw r7, 0x0C(r9);			
	stw r8, 0x10(r9);
	stw r10, 0x14(r9);
	lwz r6, 0x10(r5);
	lwz r7, 0x14(r5);
	lwz r8, 0x18(r5);
	lwz r10, 0x1C(r5);
	stw r6, 0x18(r9);				
	stw r7, 0x1C(r9);			
	stw r8, 0x20(r9);
	stw r10, 0x24(r9);
	stw r0, 0x00(r9);
	stw r4, 0x04(r9);
	stw r11, 8(r3);				blr;
SIMPLE_OUTOFSPACE_3(_ZN3Ice6Render14CommandContext26SetVertexProgram2ConstantsEjPKf, 10, r3,r4,r5)
		.size   ._ZN3Ice6Render14CommandContext26SetVertexProgram2ConstantsEjPKf,.-._ZN3Ice6Render14CommandContext26SetVertexProgram2ConstantsEjPKf

## void Ice::Render::CommandContext::SetVertexProgram3Constants(U32 index, const float *data)
FUNC(_ZN3Ice6Render14CommandContext26SetVertexProgram3ConstantsEjPKf)
	lwz r9, 8(r3);				lis r0, 0x34;
	lwz r12, 4(r3);
	lwz r6, 0x00(r5);
	lwz r7, 0x04(r5);			addi r11, r9, 0x38;
	lwz r8, 0x08(r5);			ori r0, r0, 0x1EFC;
	lwz r10, 0x0C(r5);			cmplw cr7, r11, r12;
	bgt- cr7, _ZN3Ice6Render14CommandContext26SetVertexProgram3ConstantsEjPKf_OutOfSpace;	stw r6, 0x08(r9);
	stw r7, 0x0C(r9);
	stw r8, 0x10(r9);
	stw r10, 0x14(r9);
	lwz r6, 0x10(r5);
	lwz r7, 0x14(r5);
	lwz r8, 0x18(r5);
	lwz r10, 0x1C(r5);
	stw r6, 0x18(r9);
	stw r7, 0x1C(r9);
	stw r8, 0x20(r9);
	stw r10, 0x24(r9);
	lwz r6, 0x20(r5);
	lwz r7, 0x24(r5);
	lwz r8, 0x28(r5);
	lwz r10, 0x2C(r5);
	stw r6, 0x28(r9);
	stw r7, 0x2C(r9);
	stw r8, 0x30(r9);
	stw r10, 0x34(r9);
	stw r0, 0x00(r9);
	stw r4, 0x04(r9);
	stw r11, 8(r3);				blr;
SIMPLE_OUTOFSPACE_3(_ZN3Ice6Render14CommandContext26SetVertexProgram3ConstantsEjPKf, 14, r3,r4,r5)
		.size   ._ZN3Ice6Render14CommandContext26SetVertexProgram3ConstantsEjPKf,.-._ZN3Ice6Render14CommandContext26SetVertexProgram3ConstantsEjPKf

## void Ice::Render::CommandContext::SetVertexProgram4Constants(U32 index, const float *data)
FUNC(_ZN3Ice6Render14CommandContext26SetVertexProgram4ConstantsEjPKf)
	lwz r9, 8(r3);				lis r0, 0x44;
	lwz r12, 4(r3);
	lwz r6, 0x00(r5);
	lwz r7, 0x04(r5);			addi r11, r9, 0x48;
	lwz r8, 0x08(r5);			ori r0, r0, 0x1EFC;
	lwz r10, 0x0C(r5);			cmplw cr7, r11, r12;
	bgt- cr7, _ZN3Ice6Render14CommandContext26SetVertexProgram4ConstantsEjPKf_OutOfSpace;	stw r6, 0x08(r9);
	stw r7, 0x0C(r9);
	stw r8, 0x10(r9);
	stw r10, 0x14(r9);
	lwz r6, 0x10(r5);
	lwz r7, 0x14(r5);
	lwz r8, 0x18(r5);
	lwz r10, 0x1C(r5);
	stw r6, 0x18(r9);
	stw r7, 0x1C(r9);
	stw r8, 0x20(r9);
	stw r10, 0x24(r9);
	lwz r6, 0x20(r5);
	lwz r7, 0x24(r5);
	lwz r8, 0x28(r5);
	lwz r10, 0x2C(r5);
	stw r6, 0x28(r9);
	stw r7, 0x2C(r9);
	stw r8, 0x30(r9);
	stw r10, 0x34(r9);
	lwz r6, 0x30(r5);
	lwz r7, 0x34(r5);
	lwz r8, 0x38(r5);
	lwz r10, 0x3C(r5);
	stw r6, 0x38(r9);
	stw r7, 0x3C(r9);
	stw r8, 0x40(r9);
	stw r10, 0x44(r9);
	stw r0, 0x00(r9);
	stw r4, 0x04(r9);
	stw r11, 8(r3);				blr;
SIMPLE_OUTOFSPACE_3(_ZN3Ice6Render14CommandContext26SetVertexProgram4ConstantsEjPKf, 18, r3,r4,r5)
		.size   ._ZN3Ice6Render14CommandContext26SetVertexProgram4ConstantsEjPKf,.-._ZN3Ice6Render14CommandContext26SetVertexProgram4ConstantsEjPKf

## void Ice::Render::CommandContext::SetVertexProgramConstants(U32 start, U32 count, const float *__restrict data)
FUNC(_ZN3Ice6Render14CommandContext25SetVertexProgramConstantsEjjPKf)
	rlwinm r10, r5, 4, 0, 27;		lwz r9, 4(r3);
	srdi r0, r5, 3;					lwz r7, 8(r3);
	rlwimi r11, r5, 0, 0, 28;
	addi r10, r10, 8;
	rlwinm r12, r5, 20, 9, 11;
	add r10, r10, r7;
	mtctr r0;
	add r10, r10, r11;
	cmpdi cr0, r0, 0;				
	cmplw cr7, r10, r9;
	bgt- cr7, _ZN3Ice6Render14CommandContext25SetVertexProgramConstantsEjjPKf_OutOfSpace;
	beq cr0, irsSvpcNoBlocks;
irsSvpcBlockLoop:
	lwz r0, 0x00(r6);				lis r8, 0x84;
	lwz r9, 0x04(r6);					
	lwz r11, 0x08(r6);				ori r8, r8, 0x1EFC;
	lwz r10, 0x0C(r6);
	stw r8, 0x00(r7);
	stw r4, 0x04(r7);
	stw r0, 0x08(r7);
	stw r9, 0x0C(r7);
	stw r11, 0x10(r7);
	stw r10, 0x14(r7);
	lwz r0, 0x10(r6);
	lwz r9, 0x14(r6);
	lwz r11, 0x18(r6);
	lwz r10, 0x1C(r6);
	stw r0, 0x18(r7);
	stw r9, 0x1C(r7);
	stw r11, 0x20(r7);
	stw r10, 0x24(r7);
	lwz r0, 0x20(r6);
	lwz r9, 0x24(r6);
	lwz r11, 0x28(r6);
	lwz r10, 0x2C(r6);
	stw r0, 0x28(r7);
	stw r9, 0x2C(r7);
	stw r11, 0x30(r7);
	stw r10, 0x34(r7);
	lwz r0, 0x30(r6);
	lwz r9, 0x34(r6);
	lwz r11, 0x38(r6);
	lwz r10, 0x3C(r6);
	stw r0, 0x38(r7);
	stw r9, 0x3C(r7);
	stw r11, 0x40(r7);
	stw r10, 0x44(r7);
	lwz r0, 0x40(r6);
	lwz r9, 0x44(r6);
	lwz r11, 0x48(r6);
	lwz r10, 0x4C(r6);
	stw r0, 0x48(r7);
	stw r9, 0x4C(r7);
	stw r11, 0x50(r7);
	stw r10, 0x54(r7);
	lwz r0, 0x50(r6);
	lwz r9, 0x54(r6);
	lwz r11, 0x58(r6);
	lwz r10, 0x5C(r6);
	stw r0, 0x58(r7);
	stw r9, 0x5C(r7);
	stw r11, 0x60(r7);
	stw r10, 0x64(r7);
	lwz r0, 0x60(r6);
	lwz r9, 0x64(r6);
	lwz r11, 0x68(r6);
	lwz r10, 0x6C(r6);
	stw r0, 0x68(r7);
	stw r9, 0x6C(r7);
	stw r11, 0x70(r7);
	stw r10, 0x74(r7);
	lwz r0, 0x70(r6);
	lwz r9, 0x74(r6);
	lwz r11, 0x78(r6);				addi r4, r4, 8;
	lwz r10, 0x7C(r6);				addi r6, r6, 0x80;
	stw r0, 0x78(r7);
	stw r9, 0x7C(r7);
	stw r11, 0x80(r7);
	stw r10, 0x84(r7);
	addi r7, r7, 0x88;				bdnz irsSvpcBlockLoop;
irsSvpcNoBlocks:
	cmpdi cr7, r12, 0;
	addis r0, r12, 4;
	stw r7, 8(r3);					beqlr cr7;
	rlwinm r12, r5, 0, 29, 31;
	ori r0, r0, 0x1EFC;
	mtctr r12;
	stw r0, 0(r7);
	stw r4, 4(r7);
	addi r7, r7, 8;
	nop;
irsSvpcRemainLoop:
	lwz r0, 0(r6);					
	lwz r9, 4(r6);
	lwz r11, 8(r6);
	lwz r10, 12(r6);				addi r6, r6, 16;
	stw r0, 0(r7);
	stw r9, 4(r7);
	stw r11, 8(r7);
	stw r10, 12(r7);
	addi r7, r7, 16;				bdnz irsSvpcRemainLoop;
	stw r7, 8(r3);					blr;
SIMPLE_OUTOFSPACE_CALC_4(_ZN3Ice6Render14CommandContext25SetVertexProgramConstantsEjjPKf, r7,r10, r3,r4,r5,r6)
		.size   ._ZN3Ice6Render14CommandContext25SetVertexProgramConstantsEjjPKf,.-._ZN3Ice6Render14CommandContext25SetVertexProgramConstantsEjjPKf

## void Ice::Render::CommandContext::SetFragmentProgram(const FragmentProgram *__restrict program)
FUNC(_ZN3Ice6Render14CommandContext18SetFragmentProgramEPKNS0_15FragmentProgramE)
	lwz r5, 8(r3);					lis r0, 0x4;
	lhz r6, 4(r4);					lis r11, 0x28;			## r6 = m_centroidMask
	lwz r12, 4(r3);					addi r10, r5, 0x3C;
	lwz r7, 8(r4);					ori r9, r0, 0x1D60;		## r7 = m_control
	lwz r8, 12(r4);					ori r0, r0, 0x8E4;		## r8 = m_offsetAndContext
	cmplw cr7, r10, r12;
	stw r0, 0(r5);
	lhz r10, 6(r4);					ori r11, r11, 0xB40;	## r10 = m_texcoordMask
	bgt- cr7, _ZN3Ice6Render14CommandContext18SetFragmentProgramEPKNS0_15FragmentProgramE_OutOfSpace;	stw r9, 8(r5);
	stw r7, 12(r5);
	stw r8, 4(r5);
	stwu r11, 16(r5);
	li r0, 10;
	sldi r6, r6, 4;							
	mtctr r0;
irsSfpLoop:
	rlwinm r11, r6, 0, 27, 27;
	srdi r6, r6, 1;
	rlwimi r11, r10, 0, 31, 31;
	srdi r10, r10, 1;
	stwu r11, 4(r5);				bdnz irsSfpLoop;
	addi r5, r5, 4;
	stw r5, 8(r3);					
	blr;
SIMPLE_OUTOFSPACE_2(_ZN3Ice6Render14CommandContext18SetFragmentProgramEPKNS0_15FragmentProgramE, 15, r3,r4)
		.size   ._ZN3Ice6Render14CommandContext18SetFragmentProgramEPKNS0_15FragmentProgramE,.-._ZN3Ice6Render14CommandContext18SetFragmentProgramEPKNS0_15FragmentProgramE

## void Ice::Render::CommandContext::SetNullFragmentProgram()
FUNC(_ZN3Ice6Render14CommandContext22SetNullFragmentProgramEv)
	lis r6, 0x4;					lwz r10, g_nullFragmentProgramOffset@toc(r2);
	lis r7, 0x200;					lwz r9, 8(r3);
	lis r8, 0x28;
	li r4, 1;
	addi r11, r9, 0x3C;				lwz r12, 4(r3);
	ori r0, r6, 0x8E4;
	cmplw cr7, r11, r12;
	ori r6, r6, 0x1D60;				bgt- cr7, _ZN3Ice6Render14CommandContext22SetNullFragmentProgramEv_OutOfSpace;
	ori r7, r7, 0x8400;				lwz r10, 0(r10);
	ori r8, r8, 0xB40;				
	ori r10, r10, 2;				
	stw r0, 0(r9);
	stw r10, 4(r9);
	stw r6, 8(r9);
	stw r7, 12(r9);
	stw r8, 16(r9);
	stw r4, 20(r9);
	stw r4, 24(r9);
	stw r4, 28(r9);
	stw r4, 32(r9);
	stw r4, 36(r9);
	stw r4, 40(r9);
	stw r4, 44(r9);
	stw r4, 48(r9);
	stw r4, 52(r9);
	stw r4, 56(r9);
	stw r11, 8(r3);					blr;
SIMPLE_OUTOFSPACE_1(_ZN3Ice6Render14CommandContext22SetNullFragmentProgramEv, 15, r3)
		.size   ._ZN3Ice6Render14CommandContext22SetNullFragmentProgramEv,.-._ZN3Ice6Render14CommandContext22SetNullFragmentProgramEv

## void Ice::Render::CommandContext::RefreshFragmentProgram(const FragmentProgram *__restrict program)
FUNC(_ZN3Ice6Render14CommandContext22RefreshFragmentProgramEPKNS0_15FragmentProgramE)
	lwz r9, 8(r3);				lis r0, 0x4;				
	lwz r12, 4(r3);
	lwz r10, 12(r4);			
	addi r11, r9, 8;
	ori r0, r0, 0x8E4;
	cmplw cr7, r11, r12;
	stw r0, 0(r9);
	bgt- cr7, _ZN3Ice6Render14CommandContext22RefreshFragmentProgramEPKNS0_15FragmentProgramE_OutOfSpace;	stw r10, 4(r9);
	stw r11, 8(r3);		        blr;
SIMPLE_OUTOFSPACE_2(_ZN3Ice6Render14CommandContext22RefreshFragmentProgramEPKNS0_15FragmentProgramE, 2, r3,r4)
		.size   ._ZN3Ice6Render14CommandContext22RefreshFragmentProgramEPKNS0_15FragmentProgramE,.-._ZN3Ice6Render14CommandContext22RefreshFragmentProgramEPKNS0_15FragmentProgramE

## void Ice::Render::CommandContext::SetFragmentProgramConstant(FragmentProgram *__restrict program, U32 index, const float *__restrict data, bool noProgramChange)
FUNC(_ZN3Ice6Render14CommandContext26SetFragmentProgramConstantEPNS0_15FragmentProgramEjPKfb)
## r0, r8, r9, r10, r11, r12
	lis r12, 0x8;					lwz r0, 20(r4);		## r0 = m_patchCount
	lis r10, 0x4;					lwz r9, 8(r3);
	cmplw cr7, r0, r5;				lwz r11, 4(r3);		## if(m_patchCount <= index) 
	slwi r5, r5, 2;					blelr cr7;						
	cmpdi cr0, r7, 0;									## r7 consumed
	add r5, r5, r4;					
	ori r0, r12, 0x6184;			
	lis r7, 0x1000;					lwz r5, 24(r5);		## r5 = m_patchData[index]
	add r5, r5, r4;
	ori r12, r12, 0x6300;			
	std r30, -16(r1);
	lhz r30, 0(r5);										## r30 = m_count
	slwi r8, r30, 2;					
	cmpdi cr7, r30, 0;
	addi r8, r8, 0x20;
	mtctr r30;											
	add r8, r8, r9;
	slwi r30, r30, 5;
	beq- cr7, _ZN3Ice6Render14CommandContext26SetFragmentProgramConstantEPNS0_15FragmentProgramEjPKfb_Exit;
	add r8, r8, r30;									## r30 consumed
	ori r10, r10, 0x630C;			
	cmplw cr7, r8, r11;
	ori r7, r7, 0x1000;				
	li r11, 0xB;					bgt cr7, _ZN3Ice6Render14CommandContext26SetFragmentProgramConstantEPNS0_15FragmentProgramEjPKfb_OutOfSpace;
_ZN3Ice6Render14CommandContext26SetFragmentProgramConstantEPNS0_15FragmentProgramEjPKfb_Back:
	bne cr0, irsSfpcNoChange;		stw r12, 20(r9);	## r12 consumed
	stw r0, 0(r9);										## r0 consumed
	stw r11, 24(r9);									## r11 consumed
	lwz r0, 12(r4);										## r0 = m_offsetAndContext
	stw r10, 12(r9);									## r10 consumed
	xori r11, r0, 1;
	stw r7, 28(r9);										## r7 consumed
	rlwinm r11, r11, 0, 31, 31;
	rlwinm r10, r0, 0, 0, 29;
	oris r11, r11, 0xfeed;								## r11 = context
	stw r10, 16(r9);
	stw r11, 4(r9);
	stw r11, 8(r9);					
	addi r9, r9, 0x20;									## r11 consumed
irsSfpcNoChange:
	lwz r0, 0(r6);
	stw r9, 8(r3);										
	lwz r4, 4(r6);
	lwz r11, 8(r6);
	lwz r10, 12(r6);									## r6 consumed
	rlwinm r0, r0, 16, 0xffffffff;
	rlwinm r4, r4, 16, 0xffffffff;
	rlwinm r11, r11, 16, 0xffffffff;
	rlwinm r10, r10, 16, 0xffffffff;
	lis r6, 0xc;
	lis r12, 0x1;
	lis r8, 0x10;
	ori r6, r6, 0xA304;
	ori r12, r12, 4;
	ori r8, r8, 0xA400;
irsSfpcLoop:
	lhzu r7, 2(r5);
	stw r8, 16(r9);
	rlwinm r30, r7, 8, 0, 15;
	stw r12, 8(r9);
	rlwimi r30, r7, 2, 22, 31;
	stw r12, 12(r9);
	stw r0, 20(r9);
	stw r30, 4(r9);
	stw r4, 24(r9);
	stw r11, 28(r9);
	stw r10, 32(r9);
	stw r6, 0(r9);
	addi r9, r9, 36;					bdnz irsSfpcLoop;
	stw r9, 8(r3);				
_ZN3Ice6Render14CommandContext26SetFragmentProgramConstantEPNS0_15FragmentProgramEjPKfb_Exit:			
	ld r30, -16(r1);
	blr;
_ZN3Ice6Render14CommandContext26SetFragmentProgramConstantEPNS0_15FragmentProgramEjPKfb_OutOfSpace:
	stdu r1, -256(r1);
	std r0, 0x78(r1);
	std r3, 0x70(r1);
	std r4, 0x68(r1);
	std r5, 0x60(r1);
	std r6, 0x58(r1);
	std r7, 0x50(r1);
	std r8, 0x48(r1);
	std r10, 0x38(r1);
	std r11, 0x30(r1);
	std r12, 0x28(r1);
	mfcr r0;
	std r0, 0x20(r1);
	mfctr r0;
	std r0, 0x18(r1);
	mflr r0;
	std r0, 144(r1);
	sub r4, r8, r9;
	srdi r4, r4, 2;
	bl ._ZN3Ice6Render14CommandContext13ReserveFailedEj;
	ld r0, 144(r1);
	mtlr r0;
	cmpdi cr7, r3, 0;
	beq- cr7, _ZN3Ice6Render14CommandContext26SetFragmentProgramConstantEPNS0_15FragmentProgramEjPKfb_OutOfSpace_Exit;
	ld r0, 0x18(r1);
	mtctr r0;
	ld r0, 0x20(r1);
	mtcr r0;
	ld r0, 0x78(r1);
	ld r3, 0x70(r1);
	ld r4, 0x68(r1);
	ld r5, 0x60(r1);
	ld r6, 0x58(r1);
	ld r7, 0x50(r1);
	ld r8, 0x48(r1);
	ld r10, 0x38(r1);
	ld r11, 0x30(r1);
	ld r12, 0x28(r1);
	lwz r9, 8(r3);
	addi r1, r1, 256;
	b _ZN3Ice6Render14CommandContext26SetFragmentProgramConstantEPNS0_15FragmentProgramEjPKfb_Back;
_ZN3Ice6Render14CommandContext26SetFragmentProgramConstantEPNS0_15FragmentProgramEjPKfb_OutOfSpace_Exit:
	addi r1, r1, 256;
	b _ZN3Ice6Render14CommandContext26SetFragmentProgramConstantEPNS0_15FragmentProgramEjPKfb_Exit;
		.size   ._ZN3Ice6Render14CommandContext26SetFragmentProgramConstantEPNS0_15FragmentProgramEjPKfb,.-._ZN3Ice6Render14CommandContext26SetFragmentProgramConstantEPNS0_15FragmentProgramEjPKfb

## void Ice::Render::CommandContext::SetFragmentProgramConstants(FragmentProgram *__restrict program, U32 start, U32 count, const float *__restrict data, bool noProgramChange)
FUNC(_ZN3Ice6Render14CommandContext27SetFragmentProgramConstantsEPNS0_15FragmentProgramEjjPKfb)
	## r0, r9, r10, r11, r12
	lwz r12, 8(r3);
	lwz r0, 4(r3);
	addi r9, r12, 0x20;
	std r30, -16(r1);
	cmplw cr7, r9, r0;
	std r29, -24(r1);
	bgt- cr7, _ZN3Ice6Render14CommandContext27SetFragmentProgramConstantsEPNS0_15FragmentProgramEjjPKfb_OutOfSpace;	std r24, -64(r1);
	std r25, -56(r1);
	std r26, -48(r1);
	std r28, -32(r1);
	cmpdi cr0, r6, 0;
	lis r10, 0x8;
	slwi r5, r5, 2;
	lis r11, 0x4;						beqlr cr0;
	add r5, r5, r4;
	cmpdi cr7, r8, 0;																## r8 consumed
	bne cr7, irsSfpcsLoop1;				lwz r8, 12(r4);								## r8 = m_offsetAndContext
	ori r0, r10, 0x6184;									
	xori r9, r8, 1;
	rlwinm r8, r8, 0, 0, 29;
	rlwinm r9, r9, 0, 31, 31;
	ori r10, r10, 0x6300;
	oris r9, r9, 0xfeed;
	ori r11, r11, 0x630C;
	stw r9, 4(r12);
	stw r9, 8(r12);
	lis r9, 0x1000;
	stw r0, 0(r12);
	ori r9, r9, 0x1000;
	stw r10, 20(r12);																## r10 consumed
	stw r9, 28(r12);																## r9 consumed
	stw r11, 12(r12);																## r11 consumed
	li r0, 0xB;
	stw r8, 16(r12);																## r8 consumed
	stw r0, 24(r12);																## r0 consumed
	addi r12, r12, 0x20;
	nop;
irsSfpcsLoop1:
	lwz r25, 24(r5);					addi r6, r6, -1;
	lwz r8, 12(r7);						addi r5, r5, 4;
	lwz r11, 4(r7);						cmpdi cr0, r6, 0;
	lwz r10, 8(r7);						rlwinm r8, r8, 16, 0xffffffff;
	lhzux r0, r25, r4;				
	lis r29, 0x1;
	slwi r24, r0, 2;
	slwi r26, r0, 5;
	ori r29, r29, 4;
	add r24, r24, r26;
	lwz r26, 4(r3);						rlwinm r11, r11, 16, 0xffffffff;
	add r24, r24, r12;
	lis r28, 0x10;
	cmplw cr7, r24, r26;
	bgt- cr7, _ZN3Ice6Render14CommandContext27SetFragmentProgramConstantsEPNS0_15FragmentProgramEjjPKfb_OutOfSpace2;
_ZN3Ice6Render14CommandContext27SetFragmentProgramConstantsEPNS0_15FragmentProgramEjjPKfb_Oos2Back:
	cmpdi cr7, r0, 0;					
	mtctr r0;							
	lwz r0, 0(r7);						addi r7, r7, 16;
	lis r24, 0xc;
	rlwinm r30, r0, 16, 0xffffffff;		beq cr7, irsSfpcsLoop1Continue;		
	rlwinm r26, r10, 16, 0xffffffff;
	ori r24, r24, 0xA304;
	ori r28, r28, 0xA400;
irsSfpcsLoop2:
	lhzu r0, 2(r25);
	stw r24, 0(r12);
	rlwinm r9, r0, 8, 0, 15;
	stw r29, 8(r12);
	rlwimi r9, r0, 2, 22, 31;
	stw r29, 12(r12);
	stw r28, 16(r12);
	stw r9, 4(r12);
	stw r30, 20(r12);
	stw r11, 24(r12);
	stw r26, 28(r12);
	stw r8, 32(r12);
	addi r12, r12, 36;					bdnz irsSfpcsLoop2;
irsSfpcsLoop1Continue:
	bne cr0, irsSfpcsLoop1;				
_ZN3Ice6Render14CommandContext27SetFragmentProgramConstantsEPNS0_15FragmentProgramEjjPKfb_Exit:
	ld r24, -64(r1);
	ld r25, -56(r1);
	ld r26, -48(r1);
	ld r28, -32(r1);
	ld r29, -24(r1);
	ld r30, -16(r1);					
	stw r12, 8(r3);						blr;
SIMPLE_OUTOFSPACE_6(_ZN3Ice6Render14CommandContext27SetFragmentProgramConstantsEPNS0_15FragmentProgramEjjPKfb, 8, r3,r4,r5,r6,r7,r8)
_ZN3Ice6Render14CommandContext27SetFragmentProgramConstantsEPNS0_15FragmentProgramEjjPKfb_OutOfSpace2:
	stdu r1, -256(r1);
	std r0, 0x78(r1);
	std r3, 0x70(r1);
	std r4, 0x68(r1);
	std r5, 0x60(r1);
	std r6, 0x58(r1);
	std r7, 0x50(r1);
	std r8, 0x48(r1);
	std r9, 0x40(r1);
	std r10, 0x38(r1);
	std r11, 0x30(r1);
	stw r12, 8(r3);
	mfcr r0;
	std r0, 0x20(r1);
	mfctr r0;
	std r0, 0x18(r1);
	mflr r0;
	std r0, 144(r1);
	sub r4, r24, r12;
	srdi r4, r4, 2;
	bl ._ZN3Ice6Render14CommandContext13ReserveFailedEj;
	ld r0, 144(r1);
	mtlr r0;
	cmpdi cr7, r3, 0;
	beq- cr7, _ZN3Ice6Render14CommandContext27SetFragmentProgramConstantsEPNS0_15FragmentProgramEjjPKfb_OutOfSpace2_Exit;
	ld r0, 0x18(r1);
	mtctr r0;
	ld r0, 0x20(r1);
	mtcr r0;
	ld r0, 0x78(r1);
	ld r3, 0x70(r1);
	ld r4, 0x68(r1);
	ld r5, 0x60(r1);
	ld r6, 0x58(r1);
	ld r7, 0x50(r1);
	ld r8, 0x48(r1);
	ld r9, 0x40(r1);
	ld r10, 0x38(r1);
	ld r11, 0x30(r1);
	lwz r12, 8(r3);
	addi r1, r1, 256;
	b _ZN3Ice6Render14CommandContext27SetFragmentProgramConstantsEPNS0_15FragmentProgramEjjPKfb_Oos2Back;
_ZN3Ice6Render14CommandContext27SetFragmentProgramConstantsEPNS0_15FragmentProgramEjjPKfb_OutOfSpace2_Exit:
	addi r1, r1, 256;
	b _ZN3Ice6Render14CommandContext27SetFragmentProgramConstantsEPNS0_15FragmentProgramEjjPKfb_Exit;
		.size   ._ZN3Ice6Render14CommandContext27SetFragmentProgramConstantsEPNS0_15FragmentProgramEjjPKfb,.-._ZN3Ice6Render14CommandContext27SetFragmentProgramConstantsEPNS0_15FragmentProgramEjjPKfb

## void Ice::Render::CommandContext::SetTexture(U32 unit, const Texture *__restrict texture)
FUNC(_ZN3Ice6Render14CommandContext10SetTextureEjPKNS0_7TextureE)
## r0,r6,r7,r8,r9,r10,r11,r12
	lwz r9, 8(r3);						rldic r7, r4, 2, 32;
	lwz r0, 0(r5);						
	lwz r12, 4(r3);						
	lwz r10, 4(r5);						addi r8, r9, 0x3C;
	lwz r11, 8(r5);						addi r6, r7, 0x1840;
	cmplw cr7, r8, r12;
	bgt- cr7, _ZN3Ice6Render14CommandContext10SetTextureEjPKNS0_7TextureE_OutOfSpace;	
	stw r0, 4(r9);
	stw r10, 8(r9);						
	lwz r0, 12(r5);						oris r6, r6, 0x4;
	stw r11, 12(r9);
	stw r6, 36(r9);
	lwz r10, 16(r5);					addi r6, r7, 0xB00;
	lwz r11, 20(r5);					addi r7, r7, 0x1D00;
	stw r0, 16(r9);
	stw r10, 20(r9);
	lwz r0, 24(r5);						oris r6, r6, 0x4;
	lwz r10, 28(r5);					oris r7, r7, 0x4;
	stw r11, 24(r9);
	stw r7, 52(r9);
	lwz r11, 36(r5);					slwi r7, r4, 5;
	stw r0, 28(r9);
	stw r10, 32(r9);
	lwz r0, 32(r5);						addi r7, r7, 0x1A00;
	stw r11, 48(r9);
	stw r6, 44(r9);
	lwz r10, 40(r5);					oris r7, r7, 0x20;
	stw r0, 40(r9);
	stw r10, 56(r9);
	stw r7, 0(r9);
	stw r8, 8(r3);						
	blr;
SIMPLE_OUTOFSPACE_3(_ZN3Ice6Render14CommandContext10SetTextureEjPKNS0_7TextureE, 15, r3,r4,r5)
		.size   ._ZN3Ice6Render14CommandContext10SetTextureEjPKNS0_7TextureE,.-._ZN3Ice6Render14CommandContext10SetTextureEjPKNS0_7TextureE

## void Ice::Render::CommandContext::SetTextureReduced(U32 unit, const TextureReduced *__restrict texture)
FUNC(_ZN3Ice6Render14CommandContext17SetTextureReducedEjPKNS0_14TextureReducedE)
## r0,r6,r7,r8,r9,r10,r11,r12
	lwz r11, 8(r3);						slwi r8, r4, 5;
	lwz r0, 0(r5);						slwi r12, r4, 2;
	lwz r7, 4(r3);						addi r6, r11, 0x28;
	lwz r10, 4(r5);						addi r8, r8, 0x1A00;
	lwz r9, 8(r5);						addi r12, r12, 0x1840;
	cmplw cr7, r6, r7;
	bgt- cr7, _ZN3Ice6Render14CommandContext17SetTextureReducedEjPKNS0_14TextureReducedE_OutOfSpace;
	stw r0, 4(r11);
	stw r10, 8(r11);
	lwz r0, 12(r5);						oris r8, r8, 0x1c;
	lwz r10, 16(r5);					oris r12, r12, 0x4;
	stw r9, 12(r11);
	lwz r9, 20(r5);
	stw r0, 16(r11);
	stw r10, 20(r11);
	stw r9, 24(r11);
	stw r8, 0(r11);
	stw r12, 32(r11);
	lwz r0, 24(r5);						
	lwz r10, 28(r5);					
	stw r0, 28(r11);
	stw r10, 36(r11);
	stw r6, 8(r3);						blr;
SIMPLE_OUTOFSPACE_3(_ZN3Ice6Render14CommandContext17SetTextureReducedEjPKNS0_14TextureReducedE, 10, r3,r4,r5)
		.size   ._ZN3Ice6Render14CommandContext17SetTextureReducedEjPKNS0_14TextureReducedE,.-._ZN3Ice6Render14CommandContext17SetTextureReducedEjPKNS0_14TextureReducedE

## void Ice::Render::CommandContext::SetTextureReducedBorderColor(U32 unit, U32 color)
FUNC(_ZN3Ice6Render14CommandContext28SetTextureReducedBorderColorEjj)
	lwz r9, 8(r3);						slwi r6, r4, 5;
	lwz r12, 4(r3);
	addi r0, r9, 8;
	addi r6, r6, 0x1A1C;
	cmplw cr7, r0, r12;
	oris r6, r6, 0x4;					bgt- cr7, _ZN3Ice6Render14CommandContext28SetTextureReducedBorderColorEjj_OutOfSpace;
	stw r5, 4(r9);
	stw r0, 8(r3);
	stw r6, 0(r9);						blr;
SIMPLE_OUTOFSPACE_3(_ZN3Ice6Render14CommandContext28SetTextureReducedBorderColorEjj, 2, r3,r4,r5)
		.size   ._ZN3Ice6Render14CommandContext28SetTextureReducedBorderColorEjj,.-._ZN3Ice6Render14CommandContext28SetTextureReducedBorderColorEjj

## void Ice::Render::CommandContext::SetTextureReducedBorderDepth(U32 unit, float depth)
FUNC(_ZN3Ice6Render14CommandContext28SetTextureReducedBorderDepthEjf)
	slwi r5, r4, 5;						lfs f0, fDepthMul@toc(r2);
	fmuls f2, f1, f0;					lwz r9, 8(r3);
	lwz r12, 4(r3);						
	addi r0, r9, 8;
	addi r5, r5, 0x1A1C;
	cmplw cr7, r0, r12;
	oris r5, r5, 0x4;					bgt- cr7, _ZN3Ice6Render14CommandContext28SetTextureReducedBorderDepthEjf_OutOfSpace;
	stw r0, 8(r3);
	fctidz f2, f2;						stw r5, 0(r9);						
	stfd f2, -16(r1);
	# 50 cycle stall
	ld r0, -16(r1);
	stw r0, 4(r9);						blr;
SIMPLE_OUTOFSPACE_2R_1F(_ZN3Ice6Render14CommandContext28SetTextureReducedBorderDepthEjf, 2, r3,r4, f1)
		.size   ._ZN3Ice6Render14CommandContext28SetTextureReducedBorderDepthEjf,.-._ZN3Ice6Render14CommandContext28SetTextureReducedBorderDepthEjf

## void Ice::Render::CommandContext::SetTextureReducedColorKey(U32 unit, U32 color)
FUNC(_ZN3Ice6Render14CommandContext25SetTextureReducedColorKeyEjj)
	slwi r6, r4, 2;						lwz r9, 8(r3);
	lwz r12, 4(r3);
	addi r0, r9, 8;
	cmplw cr7, r0, r12;
	addi r6, r6, 0x1D00;
	bgt- cr7, _ZN3Ice6Render14CommandContext25SetTextureReducedColorKeyEjj_OutOfSpace;	stw r5, 4(r9);
	oris r6, r6, 0x4;
	stw r0, 8(r3);
	stw r6, 0(r9);						blr;
SIMPLE_OUTOFSPACE_3(_ZN3Ice6Render14CommandContext25SetTextureReducedColorKeyEjj, 2, r3,r4,r5)
		.size   ._ZN3Ice6Render14CommandContext25SetTextureReducedColorKeyEjj,.-._ZN3Ice6Render14CommandContext25SetTextureReducedColorKeyEjj

## void Ice::Render::CommandContext::SetTextureReducedControl3(U32 unit, U32 control3Value)
FUNC(_ZN3Ice6Render14CommandContext25SetTextureReducedControl3Ejj)
	slwi r6, r4, 2;						lwz r9, 8(r3);
	lwz r12, 4(r3);
	addi r0, r9, 8;
	cmplw cr7, r0, r12;
	addi r6, r6, 0xB00;
	bgt- cr7, _ZN3Ice6Render14CommandContext25SetTextureReducedControl3Ejj_OutOfSpace;	stw r5, 4(r9);
	oris r6, r6, 0x4;
	stw r0, 8(r3);
	stw r6, 0(r9);						blr;
SIMPLE_OUTOFSPACE_3(_ZN3Ice6Render14CommandContext25SetTextureReducedControl3Ejj, 2, r3,r4,r5)
		.size   ._ZN3Ice6Render14CommandContext25SetTextureReducedControl3Ejj,.-._ZN3Ice6Render14CommandContext25SetTextureReducedControl3Ejj

## void Ice::Render::CommandContext::DisableTexture(U32 unit)
FUNC(_ZN3Ice6Render14CommandContext14DisableTextureEj)
	slwi r5, r4, 5;						lwz r9, 8(r3);
	lwz r12, 4(r3);						li r0, 0;
	addi r11, r9, 8;
	addi r5, r5, 0x1A0C;
	cmplw cr7, r11, r12;
	bgt- cr7, _ZN3Ice6Render14CommandContext14DisableTextureEj_OutOfSpace;	stw r0, 4(r9);
	oris r5, r5, 0x4;
	stw r11, 8(r3);
	stw r5, 0(r9);						blr;
SIMPLE_OUTOFSPACE_2(_ZN3Ice6Render14CommandContext14DisableTextureEj, 2, r3,r4)
		.size   ._ZN3Ice6Render14CommandContext14DisableTextureEj,.-._ZN3Ice6Render14CommandContext14DisableTextureEj

## void Ice::Render::CommandContext::SetVertexProgramTexture(U32 unit, const Texture *__restrict texture)
FUNC(_ZN3Ice6Render14CommandContext23SetVertexProgramTextureEjPKNS0_7TextureE)
## r0,r6,r7,r8,r9,r10,r11,r12
	li r0, -13;							lwz r11, 8(r3);
	slwi r7, r4, 5;						lwz r10, 4(r5);
	lwz r12, 4(r3);						addi r6, r11, 0x24;
	oris r0, r0, 0xffff;
	lwz r9, 0(r5);
	cmplw cr7, r6, r12;
	addi r7, r7, 0x900;
	and r10, r10, r0;					bgt- cr7, _ZN3Ice6Render14CommandContext23SetVertexProgramTextureEjPKNS0_7TextureE_OutOfSpace;
	oris r7, r7, 0x20;
	stw r10, 8(r11);
	li r10, 0x0F0F;						lwz r0, 8(r5);
	stw r7, 0(r11);
	stw r9, 4(r11);
	and r0, r0, r10;					lwz r10, 12(r5);
	lhz r9, 34(r5);
	stw r0, 12(r11);
	rlwinm r10, r10, 0, 0, 24;			lwz r0, 20(r5);
	stw r9, 20(r11);
	stw r10, 16(r11);
	rldicl r0, r0, 0, 51;				lwz r9, 24(r5);
	stw r0, 24(r11);
	stw r9, 28(r11);
	lwz r10, 28(r5);					
	stw r10, 32(r11);
	stw r6, 8(r3);						blr;
SIMPLE_OUTOFSPACE_3(_ZN3Ice6Render14CommandContext23SetVertexProgramTextureEjPKNS0_7TextureE, 9, r3,r4,r5)
		.size   ._ZN3Ice6Render14CommandContext23SetVertexProgramTextureEjPKNS0_7TextureE,.-._ZN3Ice6Render14CommandContext23SetVertexProgramTextureEjPKNS0_7TextureE

## void Ice::Render::CommandContext::DisableVertexProgramTexture(U32 unit)
FUNC(_ZN3Ice6Render14CommandContext27DisableVertexProgramTextureEj)
	slwi r5, r4, 5;						lwz r9, 8(r3);
	lwz r12, 4(r3);						li r0, 0;
	addi r11, r9, 8;
	addi r5, r5, 0x90C;
	cmplw cr7, r11, r12;
	bgt- cr7, _ZN3Ice6Render14CommandContext27DisableVertexProgramTextureEj_OutOfSpace;	stw r0, 4(r9);
	oris r5, r5, 0x4;
	stw r11, 8(r3);
	stw r5, 0(r9);						blr;
SIMPLE_OUTOFSPACE_2(_ZN3Ice6Render14CommandContext27DisableVertexProgramTextureEj, 2, r3,r4)
		.size   ._ZN3Ice6Render14CommandContext27DisableVertexProgramTextureEj,.-._ZN3Ice6Render14CommandContext27DisableVertexProgramTextureEj

## void Ice::Render::CommandContext::InvalidateTextureCache()
SIMPLE_STUFF_CONST_1W_FUNC(_ZN3Ice6Render14CommandContext22InvalidateTextureCacheEv, 0x1FD8, 1)

## void Ice::Render::CommandContext::InvalidateVertexTextureCache()
SIMPLE_STUFF_CONST_1W_FUNC(_ZN3Ice6Render14CommandContext28InvalidateVertexTextureCacheEv, 0x1FD8, 2)

## void Ice::Render::CommandContext::InvalidateDepthCull()
SIMPLE_STUFF_CONST_1W_FUNC(_ZN3Ice6Render14CommandContext19InvalidateDepthCullEv, 0x234, 0)

## void Ice::Render::CommandContext::SetDepthCullControl(DepthCullDirection dir, DepthCullFormat format)
FUNC(_ZN3Ice6Render14CommandContext19SetDepthCullControlENS0_18DepthCullDirectionENS0_15DepthCullFormatE)
	rlwimi r4, r5, 4, 0, 27;			lwz r9, 8(r3);
	lis r0, 4;							lwz r12, 4(r3);
	addi r11, r9, 8;
	ori r0, r0, 0x1EA4;
	cmplw cr7, r11, r12;
	bgt- cr7, _ZN3Ice6Render14CommandContext19SetDepthCullControlENS0_18DepthCullDirectionENS0_15DepthCullFormatE_OutOfSpace;	stw r4, 4(r9);
	stw r0, 0(r9);
	stw r11, 8(r3);						blr;
SIMPLE_OUTOFSPACE_3(_ZN3Ice6Render14CommandContext19SetDepthCullControlENS0_18DepthCullDirectionENS0_15DepthCullFormatE, 2, r3,r4,r5)
		.size   ._ZN3Ice6Render14CommandContext19SetDepthCullControlENS0_18DepthCullDirectionENS0_15DepthCullFormatE,.-._ZN3Ice6Render14CommandContext19SetDepthCullControlENS0_18DepthCullDirectionENS0_15DepthCullFormatE

## void Ice::Render::CommandContext::SetStencilCullHint(ComparisonFunc func, U32 ref, U32 mask)
FUNC(_ZN3Ice6Render14CommandContext18SetStencilCullHintENS0_14ComparisonFuncEjj)
	slwi r7, r6, 24;					lwz r9, 8(r3);
	lwz r12, 4(r3);						lis r0, 0x4;
	addi r11, r9, 8;
	rlwimi r7, r5, 16, 8, 15;
	cmplw cr7, r11, r12;
	ori r0, r0, 0x1EAC;
	rlwimi r7, r4, 0, 0xff;				bgt- cr7, _ZN3Ice6Render14CommandContext18SetStencilCullHintENS0_14ComparisonFuncEjj_OutOfSpace;
	stw r7, 4(r9);
	stw r0, 0(r9);
	stw r11, 8(r3);						blr;
SIMPLE_OUTOFSPACE_4(_ZN3Ice6Render14CommandContext18SetStencilCullHintENS0_14ComparisonFuncEjj, 2, r3,r4,r5,r6)
		.size   ._ZN3Ice6Render14CommandContext18SetStencilCullHintENS0_14ComparisonFuncEjj,.-._ZN3Ice6Render14CommandContext18SetStencilCullHintENS0_14ComparisonFuncEjj

## void Ice::Render::CommandContext::SetDepthCullFeedback(U32 feedbackValue)
SIMPLE_STUFF_ARG_1W_FUNC(_ZN3Ice6Render14CommandContext20SetDepthCullFeedbackEj, 0x1EA8)

## void Ice::Render::CommandContext::InsertReference(I32 value)
SIMPLE_STUFF_ARG_1W_FUNC(_ZN3Ice6Render14CommandContext15InsertReferenceEi, 0x0050)

## void Ice::Render::CommandContext::SetWaitSemaphoreIndex(U32 index)
FUNC(_ZN3Ice6Render14CommandContext21SetWaitSemaphoreIndexEj)
	lis r0, 0x4;						lwz r9, 8(r3);
	slwi r5, r4, 4;						lwz r12, 4(r3);
	addi r11, r9, 8;
	ori r0, r0, 0x0064;
	cmplw cr7, r11, r12;
	bgt- cr7, _ZN3Ice6Render14CommandContext21SetWaitSemaphoreIndexEj_OutOfSpace;
	stw r5, 4(r9);
	stw r0, 0(r9);
	stw r11, 8(r3);						blr;
SIMPLE_OUTOFSPACE_2(_ZN3Ice6Render14CommandContext21SetWaitSemaphoreIndexEj, 2, r3,r4)
		.size   ._ZN3Ice6Render14CommandContext21SetWaitSemaphoreIndexEj,.-._ZN3Ice6Render14CommandContext21SetWaitSemaphoreIndexEj

## void Ice::Render::CommandContext::WaitOnSemaphore(U32 value)
SIMPLE_STUFF_ARG_1W_FUNC(_ZN3Ice6Render14CommandContext15WaitOnSemaphoreEj, 0x0068)

## void Ice::Render::CommandContext::SetFlushPipeSemaphoreIndex(U32 index)
FUNC(_ZN3Ice6Render14CommandContext26SetFlushPipeSemaphoreIndexEj)
	lis r0, 0x4;						lwz r9, 8(r3);
	slwi r5, r4, 4;						lwz r12, 4(r3);
	addi r11, r9, 8;
	ori r0, r0, 0x1D6C;
	cmplw cr7, r11, r12;
	bgt- cr7, _ZN3Ice6Render14CommandContext26SetFlushPipeSemaphoreIndexEj_OutOfSpace;
	stw r5, 4(r9);
	stw r0, 0(r9);
	stw r11, 8(r3);						blr;
SIMPLE_OUTOFSPACE_2(_ZN3Ice6Render14CommandContext26SetFlushPipeSemaphoreIndexEj, 2, r3,r4)
		.size   ._ZN3Ice6Render14CommandContext26SetFlushPipeSemaphoreIndexEj,.-._ZN3Ice6Render14CommandContext26SetFlushPipeSemaphoreIndexEj

## void Ice::Render::CommandContext::FlushTexturePipeAndWriteSemaphore(U32 value)
SIMPLE_STUFF_ARG_1W_FUNC(_ZN3Ice6Render14CommandContext33FlushTexturePipeAndWriteSemaphoreEj, 0x1D74)

## void Ice::Render::CommandContext::FlushBackendPipeAndWriteSemaphore(U32 value)
FUNC(_ZN3Ice6Render14CommandContext33FlushBackendPipeAndWriteSemaphoreEj)
	mr r6, r4;							lwz r9, 8(r3);
	li r5, 0x1D70;						lwz r12, 4(r3);
	addi r11, r9, 8;
	rlwimi r6, r4, 16, 24, 31;
	cmplw cr7, r11, r12;
	oris r5, r5, 0x4;
	rlwimi r6, r4, 16, 8, 15;			bgt- cr7, _ZN3Ice6Render14CommandContext33FlushBackendPipeAndWriteSemaphoreEj_OutOfSpace;
	stw r5, 0(r9);
	stw r6, 4(r9);
	stw r11, 8(r3);						blr;
SIMPLE_OUTOFSPACE_2(_ZN3Ice6Render14CommandContext33FlushBackendPipeAndWriteSemaphoreEj, 2, r3,r4)
		.size   ._ZN3Ice6Render14CommandContext33FlushBackendPipeAndWriteSemaphoreEj,.-._ZN3Ice6Render14CommandContext33FlushBackendPipeAndWriteSemaphoreEj

## void Ice::Render::CommandContext::InsertJump(U32 offset)
FUNC(_ZN3Ice6Render14CommandContext10InsertJumpEj)
	oris r4, r4, 0x2000;				lwz r9, 8(r3);
	lwz r12, 4(r3);
	addi r0, r9, 4;
	cmplw cr7, r0, r12;
	bgt- cr7, _ZN3Ice6Render14CommandContext10InsertJumpEj_OutOfSpace;
	stw r4, 0(r9);
	stw r0, 8(r3);						blr;
SIMPLE_OUTOFSPACE_2(_ZN3Ice6Render14CommandContext10InsertJumpEj, 1, r3,r4)
		.size   ._ZN3Ice6Render14CommandContext10InsertJumpEj,.-._ZN3Ice6Render14CommandContext10InsertJumpEj

## void Ice::Render::CommandContext::InsertCall(U32 offset)
FUNC(_ZN3Ice6Render14CommandContext10InsertCallEj)
	ori r4, r4, 2;						lwz r9, 8(r3);
	lwz r12, 4(r3);
	addi r0, r9, 4;
	cmplw cr7, r0, r12;
	bgt- cr7, _ZN3Ice6Render14CommandContext10InsertCallEj_OutOfSpace;
	stw r4, 0(r9);
	stw r0, 8(r3);						blr;
SIMPLE_OUTOFSPACE_2(_ZN3Ice6Render14CommandContext10InsertCallEj, 1, r3,r4)
		.size   ._ZN3Ice6Render14CommandContext10InsertCallEj,.-._ZN3Ice6Render14CommandContext10InsertCallEj

## void Ice::Render::CommandContext::InsertReturn()
FUNC(_ZN3Ice6Render14CommandContext12InsertReturnEv)
	lis r0, 0x2;						lwz r9, 8(r3);
	lwz r12, 4(r3);
	addi r11, r9, 4;
	cmplw cr7, r11, r12;
	bgt- cr7, _ZN3Ice6Render14CommandContext12InsertReturnEv_OutOfSpace;
	stw r0, 0(r9);
	stw r11, 8(r3);						
	blr;
SIMPLE_OUTOFSPACE_1(_ZN3Ice6Render14CommandContext12InsertReturnEv, 1, r3)
		.size   ._ZN3Ice6Render14CommandContext12InsertReturnEv,.-._ZN3Ice6Render14CommandContext12InsertReturnEv

## void Ice::Render::CommandContext::InsertNop()
SIMPLE_STUFF_CONST_1W_FUNC(_ZN3Ice6Render14CommandContext9InsertNopEv, 0x100, 0)

## void Ice::Render::CommandContext::InsertHole(U32 size)
FUNC(_ZN3Ice6Render14CommandContext10InsertHoleEj)
	lwz r9, 8(r3);						li r0, 0;
	lwz r12, 4(r3);
	add r7, r9, r4;
	addi r5, r9, 15;
	addi r7, r7, 12;
	stw r0, 0(r9);
	cmplw cr7, r7, r12;
	rlwinm r5, r5, 0, 0, 27;			bgt- cr7, _ZN3Ice6Render14CommandContext10InsertHoleEj_OutOfSpace;
	stw r0, 4(r9);
	add r6, r5, r4;
	stw r0, 8(r9);
	stw r6, 8(r3);
	mr r3, r5;							blr;
_ZN3Ice6Render14CommandContext10InsertHoleEj_OutOfSpace:
	stdu r1, -128(r1);
	std r3, 0x78(r1);
	std r4, 0x70(r1);
	mflr r4;
	std r4, 144(r1);
	sub r4, r7, r9;
	srdi r4, r4, 2;
	bl ._ZN3Ice6Render14CommandContext13ReserveFailedEj;
	cmpdi cr7, r3, 0;
	ld r3, 144(r1);
	mtlr r3;
	ld r3, 0x78(r1);
	ld r4, 0x70(r1);
	addi r1, r1, 128;
	bne+ cr7, ._ZN3Ice6Render14CommandContext10InsertHoleEj;
	li r3, 0;
	blr;
		.size   ._ZN3Ice6Render14CommandContext10InsertHoleEj,.-._ZN3Ice6Render14CommandContext10InsertHoleEj

## void Ice::Render::CommandContext::SetRenderControl(RenderControlMode mode, U32 ioOffset)
FUNC(_ZN3Ice6Render14CommandContext16SetRenderControlENS0_17RenderControlModeEj)
	lis r10, 0x4;						lwz r9, 8(r3);
	or r7, r4, r5;						lwz r12, 4(r3);
	addi r0, r9, 0x10;
	li r6, 0;
	cmplw cr7, r0, r12;
	ori r11, r10, 0x0100;
	ori r10, r10, 0x1e98;				bgt- cr7, _ZN3Ice6Render14CommandContext16SetRenderControlENS0_17RenderControlModeEj_OutOfSpace;
	stw r0, 8(r3);
	stw r7, 12(r9);
	stw r11, 0(r9);
	stw r10, 8(r9);
	stw r6, 4(r9);						blr;
SIMPLE_OUTOFSPACE_3(_ZN3Ice6Render14CommandContext16SetRenderControlENS0_17RenderControlModeEj, 4, r3,r4,r5)
		.size   ._ZN3Ice6Render14CommandContext16SetRenderControlENS0_17RenderControlModeEj,.-._ZN3Ice6Render14CommandContext16SetRenderControlENS0_17RenderControlModeEj

## void Ice::Render::CommandContext::SetWindowClipMode(WindowClipMode mode)
SIMPLE_STUFF_ARG_1W_FUNC(_ZN3Ice6Render14CommandContext17SetWindowClipModeENS0_14WindowClipModeE, 0x2BC)

## void Ice::Render::CommandContext::SetWindow(U32 index, U32 left, U32 top, U32 right, U32 bottom)
FUNC(_ZN3Ice6Render14CommandContext9SetWindowEjjjjj)
	slwi r10, r4, 3;					lwz r9, 8(r3);
	rlwimi r5, r7, 16, 0, 15;			lwz r12, 4(r3);
	addi r0, r9, 0xC;
	addi r10, r10, 0x2c0;
	cmplw cr7, r0, r12;
	rlwimi r6, r8, 16, 0, 15;
	oris r10, r10, 0x8;					bgt- cr7, _ZN3Ice6Render14CommandContext9SetWindowEjjjjj_OutOfSpace;
	stw r5, 4(r9);
	stw r6, 8(r9);
	stw r10, 0(r9);
	stw r0, 8(r3);						blr;
SIMPLE_OUTOFSPACE_6(_ZN3Ice6Render14CommandContext9SetWindowEjjjjj, 3, r3,r4,r5,r6,r7,r8)
		.size   ._ZN3Ice6Render14CommandContext9SetWindowEjjjjj,.-._ZN3Ice6Render14CommandContext9SetWindowEjjjjj

## void Ice::Render::CommandContext::InsertPpuInterrupt(U32 parameterValue)
FUNC(_ZN3Ice6Render14CommandContext18InsertPpuInterruptEj)
	lis r10, 0xcafe;					lwz r9, 8(r3);
	lis r8, 0x4;						lwz r12, 4(r3);
	addi r11, r9, 0x10;
	ori r0, r8, 0xe000;
	cmplw cr7, r11, r12;
	ori r10, r10, 0xbabe;
	ori r8, r8, 0xeb00;					bgt- cr7, _ZN3Ice6Render14CommandContext18InsertPpuInterruptEj_OutOfSpace;
	stw r4, 12(r9);
	stw r0, 0(r9);
	stw r10, 4(r9);
	stw r8, 8(r9);
	stw r11, 8(r3);						blr;
SIMPLE_OUTOFSPACE_2(_ZN3Ice6Render14CommandContext18InsertPpuInterruptEj, 4, r3,r4)
		.size   ._ZN3Ice6Render14CommandContext18InsertPpuInterruptEj,.-._ZN3Ice6Render14CommandContext18InsertPpuInterruptEj

## void Ice::Render::CommandContext::WritePerformanceReport()
FUNC(_ZN3Ice6Render14CommandContext22WritePerformanceReportEv)
	lis r6, 0x4;						lwz r9, 8(r3);
	li r4, 0;							lwz r12, 4(r3);
	addi r0, r9, 0x10;
	li r5, 1;
	cmplw cr7, r0, r12;
	ori r7, r6, 0x110;
	ori r6, r6, 0x140;					bgt- cr7, _ZN3Ice6Render14CommandContext22WritePerformanceReportEv_OutOfSpace;
	stw r7, 0(r9);
	stw r4, 4(r9);
	stw r6, 8(r9);
	stw r5, 12(r9);
	stw r0, 8(r3);						blr;
SIMPLE_OUTOFSPACE_1(_ZN3Ice6Render14CommandContext22WritePerformanceReportEv, 4, r3)
		.size   ._ZN3Ice6Render14CommandContext22WritePerformanceReportEv,.-._ZN3Ice6Render14CommandContext22WritePerformanceReportEv

## void Ice::Render::CommandContext::SetRenderTarget(const RenderTarget *__restrict target)
FUNC(_ZN3Ice6Render14CommandContext15SetRenderTargetEPKNS0_12RenderTargetE)
## r0,r5,r6,r7,r8,r9,r10,r11,r12
	lwz r9, 8(r3);						lis r0, 0x20;
	lwz r12, 4(r4);						lis r5, 0x4;
	lwz r6, 4(r3);						addi r8, r9, 0x78;
	lwz r11, 8(r4);						
	cmplw cr7, r8, r6;
	bgt- cr7, _ZN3Ice6Render14CommandContext15SetRenderTargetEPKNS0_12RenderTargetE_OutOfSpace;	stw r12, 4(r9);
	stw r11, 8(r9);
	nop; ## fixme
	lwz r12, 16(r4);					ori r0, r0, 0x200;
	lwz r11, 64(r4);					ori r5, r5, 0x1D7C;
	stw r12, 16(r9);
	stw r11, 24(r9);
	stw r0, 0(r9);
	stw r5, 80(r9);
	lwz r12, 32(r4);					lis r0, 0x4;
	lwz r11, 20(r4);					lis r5, 0x10;
	stw r12, 20(r9);
	stw r11, 32(r9);
	lwz r12, 36(r4);					ori r5, r5, 0x280;
	lwz r11, 68(r4);					ori r0, r0, 0x22C;
	stw r12, 28(r9);
	stw r11, 40(r9);
	stw r0, 36(r9);
	stw r5, 44(r9);
	lwz r12, 24(r4);					lis r0, 0x4;
	lwz r11, 28(r4);					lis r5, 0x4;
	stw r12, 48(r9);
	stw r11, 52(r9);
	lwz r12, 44(r4);					ori r0, r0, 0x2B8;
	lwz r11, 40(r4);					ori r5, r5, 0x1D88;
	stw r12, 60(r9);
	stw r11, 56(r9);
	stw r0, 64(r9);
	stw r5, 72(r9);
	lwz r12, 76(r4);					lis r0, 0x8;
	lwz r11, 80(r4);					lis r5, 0x4;
	stw r12, 68(r9);
	stw r11, 76(r9);
	lwz r12, 48(r4);					ori r0, r0, 0x194;
	lwz r11, 72(r4);					ori r5, r5, 0x18C;
	stw r12, 92(r9);
	stw r11, 96(r9);
	stw r0, 88(r9);
	stw r5, 100(r9);
	lwz r10, 60(r4);					
	lwz r12, 52(r4);					lis r0, 0x8;
	stw r8, 8(r3);
	lwz r7, 12(r4);
	lwz r11, 56(r4);					ori r0, r0, 0x1B4;
	stw r7, 12(r9);
	rldicl r7, r7, 52, 60;
	stw r10, 112(r9);
	neg r7, r7;
	stw r0, 108(r9);
	srwi r7, r7, 31;
	stw r12, 104(r9);
	oris r7, r7, 0xffff;
	stw r11, 116(r9);					
	stw r7, 84(r9);						blr;
SIMPLE_OUTOFSPACE_2(_ZN3Ice6Render14CommandContext15SetRenderTargetEPKNS0_12RenderTargetE, 30, r3,r4)
		.size   ._ZN3Ice6Render14CommandContext15SetRenderTargetEPKNS0_12RenderTargetE,.-._ZN3Ice6Render14CommandContext15SetRenderTargetEPKNS0_12RenderTargetE

## void Ice::Render::CommandContext::ToggleRenderState(RenderState state, bool enable)
FUNC(_ZN3Ice6Render14CommandContext17ToggleRenderStateENS0_11RenderStateEb)
	oris r4, r4, 0x4;					lwz r9, 8(r3);
	lwz r12, 4(r3);
	addi r11, r9, 8;
	cmplw cr7, r11, r12;
	stw r4, 0(r9);
	bgt- cr7, _ZN3Ice6Render14CommandContext17ToggleRenderStateENS0_11RenderStateEb_OutOfSpace;	stw r5, 4(r9);
	stw r11, 8(r3);						blr;
SIMPLE_OUTOFSPACE_3(_ZN3Ice6Render14CommandContext17ToggleRenderStateENS0_11RenderStateEb, 2, r3,r4,r5)
		.size   ._ZN3Ice6Render14CommandContext17ToggleRenderStateENS0_11RenderStateEb,.-._ZN3Ice6Render14CommandContext17ToggleRenderStateENS0_11RenderStateEb

## void Ice::Render::CommandContext::SetDepthMinMaxControl(bool cullNearFarEnable, bool clampEnable, bool cullIgnoreW)
FUNC(_ZN3Ice6Render14CommandContext21SetDepthMinMaxControlEbbb)
	rlwimi r4, r5, 4, 0, 27;			lwz r9, 8(r3);
	lis r0, 4;							lwz r12, 4(r3);
	addi r11, r9, 8;
	rlwimi r4, r6, 8, 0, 23;
	cmplw cr7, r11, r12;
	ori r0, r0, 0x1D78;					bgt- cr7, _ZN3Ice6Render14CommandContext21SetDepthMinMaxControlEbbb_OutOfSpace;
	stw r4, 4(r9);
	stw r0, 0(r9);
	stw r11, 8(r3);						blr;
SIMPLE_OUTOFSPACE_4(_ZN3Ice6Render14CommandContext21SetDepthMinMaxControlEbbb, 2, r3,r4,r5,r6)
		.size   ._ZN3Ice6Render14CommandContext21SetDepthMinMaxControlEbbb,.-._ZN3Ice6Render14CommandContext21SetDepthMinMaxControlEbbb

## void Ice::Render::CommandContext::SetTextureCylindricalWrapEnable(U32 enable0to7, U32 enable8to9)
SIMPLE_STUFF_2ARG_2W_FUNC(_ZN3Ice6Render14CommandContext31SetTextureCylindricalWrapEnableEjj, 0x238, r4,r5)

## void Ice::Render::CommandContext::SetVertexProgramStaticBranchBits(U32 branchBits)
SIMPLE_STUFF_ARG_1W_FUNC(_ZN3Ice6Render14CommandContext32SetVertexProgramStaticBranchBitsEj, 0x1ff8)
