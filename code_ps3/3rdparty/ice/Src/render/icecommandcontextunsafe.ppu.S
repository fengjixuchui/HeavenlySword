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

## 7 cycles, 8 instructions, 32 bytes
#define SIMPLE_STUFF_ARG_1W_FUNC(name, register) \
	FUNC(name) \
	lis 0, 4;							lwz 9, 8(3); \
	\
	addi 11, 9, 8; \
	ori 0, 0, register; \
	stw 4, 4(9); \
	stw 11, 8(3); \
	stw 0, 0(9);						blr; \
	.size   .name,.-.name

## 8 cycles, 9 instructions, 36 bytes	
#define SIMPLE_STUFF_2ARG_2W_FUNC(name, register, arg0, arg1) \
	FUNC(name) \
	lis 0, 8;							lwz 9, 8(3); \
	\
	addi 11, 9, 12; \
	ori 0, 0, register; \
	stw arg0, 4(9); \
	stw arg1, 8(9); \
	stw 11, 8(3); \
	stw 0, 0(9);						blr; \
	.size   .name,.-.name

## 7 cycles, 9 instructions, 36 bytes
#define SIMPLE_STUFF_CONST_1W_FUNC(name, register, constant) \
	FUNC(name) \
	lis 0, 4;							lwz 9, 8(3); \
	li 4, constant; \
	addi 11, 9, 8; \
	ori 0, 0, register; \
	stw 4, 4(9); \
	stw 11, 8(3); \
	stw 0, 0(9);						blr; \
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


## void Ice::Render::Unsafe::CommandContext::WriteReport(ReportType type, U32 ioOffset)
## 7 cycles, 9 instructions, 40 bytes
FUNC(_ZN3Ice6Render6Unsafe14CommandContext11WriteReportENS0_10ReportTypeEj)
	rlwimi r5, r4, 24, 0, 7;			lwz	r9, 8(r3);
	lis r0, 0x4;
	addi r11, r9, 8;						
	ori r0, r0, 0x1800;
	stw r5, 4(r9);
	stw r0, 0(r9);
	stw r11, 8(r3);						blr;
        .size   ._ZN3Ice6Render6Unsafe14CommandContext11WriteReportENS0_10ReportTypeEj,.-._ZN3Ice6Render6Unsafe14CommandContext11WriteReportENS0_10ReportTypeEj

## void Ice::Render::Unsafe::CommandContext::ResetReport(ReportType type)
SIMPLE_STUFF_ARG_1W_FUNC(_ZN3Ice6Render6Unsafe14CommandContext11ResetReportENS0_10ReportTypeE, 0x17C8)

## void Ice::Render::Unsafe::CommandContext::SetDrawBufferMask(U32 mask)
## ~8 cycles, 11 instructions, 44 bytes
FUNC(_ZN3Ice6Render6Unsafe14CommandContext17SetDrawBufferMaskEj)
	cmplwi cr7, 4, 1;					lwz 9, 8(3);
	lis 11, 4;							ble+ cr7, iruSdbmNoMrt;
	ori 4, 4, 0x10;
iruSdbmNoMrt:
	ori 11, 11, 0x220;														
	addi 0, 9, 8;						
	stw 4, 4(9);
	stw 11, 0(9);
	stw 0, 8(3);						blr;
        .size   ._ZN3Ice6Render6Unsafe14CommandContext17SetDrawBufferMaskEj,.-._ZN3Ice6Render6Unsafe14CommandContext17SetDrawBufferMaskEj

## void Ice::Render::Unsafe::CommandContext::SetAlphaFunc(ComparisonFunc func, U32 ref)
SIMPLE_STUFF_2ARG_2W_FUNC(_ZN3Ice6Render6Unsafe14CommandContext12SetAlphaFuncENS0_14ComparisonFuncEj, 0x308, 4, 5)

## void Ice::Render::Unsafe::CommandContext::SetMrtBlendEnable(U32 mask)
## 12 cycles, 14 instructions, 44 bytes
FUNC(_ZN3Ice6Render6Unsafe14CommandContext17SetMrtBlendEnableEj)
	lis 0, 4;							lwz 9, 8(3);
	lis 10, 4;
	ori 0, 0, 0x310;
	ori 10, 10, 0x36C;
	rlwinm 8, 4, 0, 28, 30;				
	stw 0, 0(9);
	clrlwi 4, 4, 31;					
	stw 10, 8(9);
	addi 11, 9, 16;						
	stw 8, 12(9);
	stw 4, 4(9);
	stw 11, 8(3);						blr;
        .size   ._ZN3Ice6Render6Unsafe14CommandContext17SetMrtBlendEnableEj,.-._ZN3Ice6Render6Unsafe14CommandContext17SetMrtBlendEnableEj

## void Ice::Render::Unsafe::CommandContext::SetBlendFunc(BlendFactor src, BlendFactor dst)
## 9 cycles, 11 instructions, 44 bytes
FUNC(_ZN3Ice6Render6Unsafe14CommandContext12SetBlendFuncENS0_11BlendFactorES3_)
	lis 0, 8;							lwz 9, 8(3);
	rlwimi 4, 4, 16, 0, 15;
	ori 0, 0, 0x314;
	rlwimi 5, 5, 16, 0, 15;				
	stw 4, 4(9);
	addi 11, 9, 12;						
	stw 0, 0(9);
	stw 5, 8(9);
	stw 11, 8(3);						blr;
        .size   ._ZN3Ice6Render6Unsafe14CommandContext12SetBlendFuncENS0_11BlendFactorES3_,.-._ZN3Ice6Render6Unsafe14CommandContext12SetBlendFuncENS0_11BlendFactorES3_
	
## @@@ This could potentially be combined with SetBlendFunc for an additional 1 cycle cost.
## void Ice::Render::Unsafe::CommandContext::SetBlendFuncSeparate(BlendFactor srcRGB, BlendFactor dstRGB, BlendFactor srcAlpha, BlendFactor dstAlpha)
## 9 cycles, 11 instructions, 44 bytes
FUNC(_ZN3Ice6Render6Unsafe14CommandContext20SetBlendFuncSeparateENS0_11BlendFactorES3_S3_S3_)
	lis 0, 8;							lwz 9, 8(3);
	rlwimi 4, 6, 16, 0, 15;
	ori 0, 0, 0x314;
	rlwimi 5, 7, 16, 0, 15;				
	stw 4, 4(9);
	addi 11, 9, 12;						
	stw 0, 0(9);
	stw 5, 8(9);
	stw 11, 8(3);						blr;
        .size   ._ZN3Ice6Render6Unsafe14CommandContext20SetBlendFuncSeparateENS0_11BlendFactorES3_S3_S3_,.-._ZN3Ice6Render6Unsafe14CommandContext20SetBlendFuncSeparateENS0_11BlendFactorES3_S3_S3_
										
## void Ice::Render::Unsafe::CommandContext::SetBlendEquation(BlendEquation equation)
## 7 cycles, 9 instructions, 36 bytes
FUNC(_ZN3Ice6Render6Unsafe14CommandContext16SetBlendEquationENS0_13BlendEquationE)
	lis 0, 4;							lwz 9, 8(3);
	rlwimi 4, 4, 16, 0, 15;
	ori 0, 0, 0x320;
	addi 10, 9, 8;						
	stw 4, 4(9);
	stw 0, 0(9);
	stw 10, 8(3);						blr;
        .size   ._ZN3Ice6Render6Unsafe14CommandContext16SetBlendEquationENS0_13BlendEquationE,.-._ZN3Ice6Render6Unsafe14CommandContext16SetBlendEquationENS0_13BlendEquationE
	
## @@@ This could potentially be combined with SetBlendEquation for an additional 1 cycle cost.
## void Ice::Render::Unsafe::CommandContext::SetBlendEquationSeparate(BlendEquation equationRGB, BlendEquation equationAlpha)
## 7 cycles, 9 instructions, 36 bytes
FUNC(_ZN3Ice6Render6Unsafe14CommandContext24SetBlendEquationSeparateENS0_13BlendEquationES3_)
	lis 0, 4;							lwz 9, 8(3);
	rlwimi 4, 5, 16, 0, 15;
	ori 0, 0, 0x320;
	addi 11, 9, 8;						
	stw 4, 4(9);
	stw 0, 0(9);
	stw 11, 8(3);						blr;
        .size   ._ZN3Ice6Render6Unsafe14CommandContext24SetBlendEquationSeparateENS0_13BlendEquationES3_,.-._ZN3Ice6Render6Unsafe14CommandContext24SetBlendEquationSeparateENS0_13BlendEquationES3_

## void Ice::Render::Unsafe::CommandContext::SetBlendColor(ArgbColor color)
SIMPLE_STUFF_ARG_1W_FUNC(_ZN3Ice6Render6Unsafe14CommandContext13SetBlendColorEj, 0x31C)

## void Ice::Render::Unsafe::CommandContext::SetFloatBlendColor(half red, half green, half blue, half alpha)
## 11 cycles, 13 instructions, 52 bytes
FUNC(_ZN3Ice6Render6Unsafe14CommandContext18SetFloatBlendColorEtttt)
	lis 0, 4;							lwz 9, 8(3);
	rlwimi 4, 5, 16, 0, 15;
	ori 10, 0, 0x37C;
	ori 0, 0, 0x31C;					
	stw 4, 4(9);
	rlwimi 6, 7, 16, 0, 15;				
	stw 10, 8(9);
	addi 11, 9, 16;						
	stw 0, 0(9);
	stw 6, 12(9);
	stw 11, 8(3);						blr;
        .size   ._ZN3Ice6Render6Unsafe14CommandContext18SetFloatBlendColorEtttt,.-._ZN3Ice6Render6Unsafe14CommandContext18SetFloatBlendColorEtttt

## void Ice::Render::Unsafe::CommandContext::SetColorMask(bool red, bool green, bool blue, bool alpha)
## 18 cycles, 20 instructions, 80 bytes
FUNC(_ZN3Ice6Render6Unsafe14CommandContext12SetColorMaskEbbbb)
	rlwinm r0, r7, 4, 27, 27;				lwz r11, 8(r3);
	rlwimi r0, r4, 5, 26, 26;
	rlwimi r0, r5, 6, 25, 25;
	rlwimi r0, r6, 7, 24, 24;
	lis r9, 4;
	rlwimi r0, r0, 4, 20, 23;
	rlwimi r6, r5, 8, 0, 23;
	rlwimi r0, r0, 4, 16, 19;
	rlwimi r6, r4, 16, 0, 15;
	ori r10, r9, 0x370;
	rlwimi r6, r7, 24, 0, 7;				
	stw r0, 12(r11);
	ori r9, r9, 0x324;					
	stw r10, 8(r11);
	addi r0, r11, 16;						
	stw r6, 4(r11);
	stw r9, 0(r11);
	stw r0, 8(r3);						blr;
        .size   ._ZN3Ice6Render6Unsafe14CommandContext12SetColorMaskEbbbb,.-._ZN3Ice6Render6Unsafe14CommandContext12SetColorMaskEbbbb

## void Ice::Render::Unsafe::CommandContext::SetMrtColorMask(U16 mask)
## 14 cycles, 16 instructions, 64 bytes
FUNC(_ZN3Ice6Render6Unsafe14CommandContext15SetMrtColorMaskEt)
	lis 9, 4;							lwz 8, 8(3);
	rlwinm 0, 4, 29, 31, 31;			
	rlwimi 0, 4, 6, 23, 23;
	rlwimi 0, 4, 15, 15, 15;
	rlwimi 0, 4, 24, 7, 7;
	ori 10, 9, 0x370;
	ori 9, 9, 0x324;					
	stw 0, 4(8);
	rlwinm 4, 4, 0, 16, 27;				
	stw 10, 8(8);
	addi 11, 8, 16;						
	stw 9, 0(8);
	stw 4, 12(8);
	stw 11, 8(3);						blr;
        .size   ._ZN3Ice6Render6Unsafe14CommandContext15SetMrtColorMaskEt,.-._ZN3Ice6Render6Unsafe14CommandContext15SetMrtColorMaskEt

## void Ice::Render::Unsafe::CommandContext::SetDepthMask(bool mask)
SIMPLE_STUFF_ARG_1W_FUNC(_ZN3Ice6Render6Unsafe14CommandContext12SetDepthMaskEb, 0xA70)

## void Ice::Render::Unsafe::CommandContext::SetStencilMask(U32 mask)
## 10 cycles, 11 instructions, 44 bytes
FUNC(_ZN3Ice6Render6Unsafe14CommandContext14SetStencilMaskEj)
	lis 0, 4;							lwz 9, 8(3);
	## stall
	ori 11, 0, 0x34C;					
	stw 4, 12(9);
	ori 0, 0, 0x32C;					
	stw 4, 4(9);
	addi 10, 9, 16;						
	stw 11, 8(9);
	stw 0, 0(9);
	stw 10, 8(3);						blr;
        .size   ._ZN3Ice6Render6Unsafe14CommandContext14SetStencilMaskEj,.-._ZN3Ice6Render6Unsafe14CommandContext14SetStencilMaskEj

## @@@ This could potentially be combined with SetStencilMask for an additional 1 cycle cost.
## void Ice::Render::Unsafe::CommandContext::SetStencilMaskSeparate(U32 maskFront, U32 maskBack)
## 10 cycles, 11 instructions, 44 bytes
FUNC(_ZN3Ice6Render6Unsafe14CommandContext22SetStencilMaskSeparateEjj)
	lis 0, 4;							lwz 9, 8(3);
## stall
	ori 11, 0, 0x34C;					
	stw 5, 12(9);
	ori 0, 0, 0x32C;					
	stw 4, 4(9);
	addi 10, 9, 16;						
	stw 11, 8(9);
	stw 0, 0(9);
	stw 10, 8(3);						blr;
        .size   ._ZN3Ice6Render6Unsafe14CommandContext22SetStencilMaskSeparateEjj,.-._ZN3Ice6Render6Unsafe14CommandContext22SetStencilMaskSeparateEjj

## void Ice::Render::Unsafe::CommandContext::SetStencilFunc(ComparisonFunc func, U32 ref, U32 mask)
## 14 cycles, 15 instructions, 60 bytes
FUNC(_ZN3Ice6Render6Unsafe14CommandContext14SetStencilFuncENS0_14ComparisonFuncEjj)
	lis 10, 0xC;						lwz 9, 8(3);
## stall
	ori 11, 10, 0x330;					
	stw 4, 4(9);
	ori 10, 10, 0x350;					
	stw 5, 8(9);
	addi 0, 9, 32;						
	stw 6, 12(9);
	stw 4, 20(9);
	stw 5, 24(9);
	stw 6, 28(9);
	stw 11, 0(9);
	stw 10, 16(9);
	stw 0, 8(3);						blr;
        .size   ._ZN3Ice6Render6Unsafe14CommandContext14SetStencilFuncENS0_14ComparisonFuncEjj,.-._ZN3Ice6Render6Unsafe14CommandContext14SetStencilFuncENS0_14ComparisonFuncEjj

## @@@ This could potentially be combined with SetStencilFunc for an additional 3 cycle cost.
## void Ice::Render::Unsafe::CommandContext::SetStencilFuncSeparate(ComparisonFunc funcFront, U32 refFront, U32 maskFront, ComparisonFunc funcBack, U32 refBack, U32 maskBack)
## 14 cycles, 15 instructions, 60 bytes
FUNC(_ZN3Ice6Render6Unsafe14CommandContext22SetStencilFuncSeparateENS0_14ComparisonFuncEjjS3_jj)
	lis r10, 0xC;						lwz r12, 8(r3);
	## stall
	stw r4, 4(r12);
	stw r5, 8(r12);
	stw r6, 12(r12);
	stw r7, 20(r12);
	ori r11, r10, 0x330;					
	stw r8, 24(r12);
	ori r10, r10, 0x350;					
	stw r9, 28(r12);
	addi r4, r12, 32;						
	stw r11, 0(r12);
	stw r10, 16(r12);
	stw r4, 8(r3);						blr;
        .size   ._ZN3Ice6Render6Unsafe14CommandContext22SetStencilFuncSeparateENS0_14ComparisonFuncEjjS3_jj,.-._ZN3Ice6Render6Unsafe14CommandContext22SetStencilFuncSeparateENS0_14ComparisonFuncEjjS3_jj

## void Ice::Render::Unsafe::CommandContext::SetStencilOp(StencilOp sfail, StencilOp dfail, StencilOp dpass)
## 14 cycles, 15 instructions, 60 bytes
FUNC(_ZN3Ice6Render6Unsafe14CommandContext12SetStencilOpENS0_9StencilOpES3_S3_)
	lis r10, 0xC;						lwz r12, 8(r3);
	## stall
	stw r4, 4(r12);
	stw r5, 8(r12);
	stw r6, 12(r12);
	stw r4, 20(r12);
	ori r11, r10, 0x33C;					
	stw r5, 24(r12);
	ori r10, r10, 0x35C;					
	stw r6, 28(r12);
	addi r4, r12, 32;						
	stw r11, 0(r12);
	stw r10, 16(r12);
	stw r4, 8(r3);						blr;
        .size   ._ZN3Ice6Render6Unsafe14CommandContext12SetStencilOpENS0_9StencilOpES3_S3_,.-._ZN3Ice6Render6Unsafe14CommandContext12SetStencilOpENS0_9StencilOpES3_S3_


## void Ice::Render::Unsafe::CommandContext::SetStencilOpSeparate(StencilOp sfailFront, StencilOp dfailFront, StencilOp dpassFront, StencilOp sfailBack, StencilOp dfailBack, StencilOp dpassBack)
## 14 cycles, 15 instructions, 60 bytes
FUNC(_ZN3Ice6Render6Unsafe14CommandContext20SetStencilOpSeparateENS0_9StencilOpES3_S3_S3_S3_S3_)
	lis r10, 0xC;						lwz r12, 8(r3);
	## stall
	stw r4, 4(r12);
	stw r5, 8(r12);
	stw r6, 12(r12);
	stw r7, 20(r12);
	ori r11, r10, 0x33C;					
	stw r8, 24(r12);
	ori r10, r10, 0x35C;					
	stw r9, 28(r12);
	addi r4, r12, 32;						
	stw r11, 0(r12);
	stw r10, 16(r12);
	stw r4, 8(r3);						blr;
        .size   ._ZN3Ice6Render6Unsafe14CommandContext20SetStencilOpSeparateENS0_9StencilOpES3_S3_S3_S3_S3_,.-._ZN3Ice6Render6Unsafe14CommandContext20SetStencilOpSeparateENS0_9StencilOpES3_S3_S3_S3_S3_

## void Ice::Render::Unsafe::CommandContext::SetShadeModel(ShadeModel model)
SIMPLE_STUFF_ARG_1W_FUNC(_ZN3Ice6Render6Unsafe14CommandContext13SetShadeModelENS0_10ShadeModelE, 0x368)
										
## void Ice::Render::Unsafe::CommandContext::SetLogicOp(LogicOp op)
SIMPLE_STUFF_ARG_1W_FUNC(_ZN3Ice6Render6Unsafe14CommandContext10SetLogicOpENS0_7LogicOpE, 0x378)

## void Ice::Render::Unsafe::CommandContext::SetFogMode(FogMode mode)
SIMPLE_STUFF_ARG_1W_FUNC(_ZN3Ice6Render6Unsafe14CommandContext10SetFogModeENS0_7FogModeE, 0x8CC)

## void Ice::Render::Unsafe::CommandContext::SetFogRange(float fmin, float fmax)
## 31 cycles, 14 instructions, 56 bytes
FUNC(_ZN3Ice6Render6Unsafe14CommandContext11SetFogRangeEff)
	fsubs f1, f2, f1;					lfs f0, fOne@toc(r2);
	lis r0, 8;							lwz r9, 8(r3);
	addi r11, r9, 12;
	ori r0, r0, 0x8D0;					
	stw r11, 8(r3);
	stw r0, 0(r9);
	## stall f1 8
	## stall f1 9
	## stall f1 10
	fdivs f1, f0, f1;
	## stall f1 2
	## stall f1 3
	## stall f1 4
	## stall f1 5
	## stall f1 6
	## stall f1 7
	## stall f1 8
	## stall f1 9
	## stall f1 10
	fmadds f2, f2, f1, f0;
	fneg f1, f1;
	## stall f2 3
	## stall f2 4
	## stall f2 5
	## stall f2 6
	## stall f2 7
	## stall f2 8
	## stall f2 9
	## stall f2 10
	stfs f2, 4(r9);
	stfs f1, 8(r9);						blr;
        .size   ._ZN3Ice6Render6Unsafe14CommandContext11SetFogRangeEff,.-._ZN3Ice6Render6Unsafe14CommandContext11SetFogRangeEff

## void Ice::Render::Unsafe::CommandContext::SetFogDensity(float density)
## cycles, instructions, bytes
FUNC(_ZN3Ice6Render6Unsafe14CommandContext13SetFogDensityEf)
	lis 0, 0x8;							lfs 0, fDensity@toc(2);
	fmuls 1, 1, 0;						lwz 9, 8(3);
	ori 0, 0, 0x8D0;					
	addi 11, 9, 12;				
	stw 0, 0(9);
	lis 0, 0x3fc0;						
	stw 11, 8(3);
	stw 0, 4(9);
	stfs 1, 8(9);						blr;
        .size   ._ZN3Ice6Render6Unsafe14CommandContext13SetFogDensityEf,.-._ZN3Ice6Render6Unsafe14CommandContext13SetFogDensityEf
 
## void Ice::Render::Unsafe::CommandContext::SetDepthBounds(float dmin, float dmax)
## 6 cycles, 9 instructions, 36 bytes
FUNC(_ZN3Ice6Render6Unsafe14CommandContext14SetDepthBoundsEff)
	lis 0, 0x8;							lwz 9, 8(3);
	## stall
	stfs 1, 4(9);						addi 11, 9, 12;						
	stfs 2, 8(9);						ori 0, 0, 0x384;					
	stw 11, 8(3);
	stw 0, 0(9);						blr;
        .size   ._ZN3Ice6Render6Unsafe14CommandContext14SetDepthBoundsEff,.-._ZN3Ice6Render6Unsafe14CommandContext14SetDepthBoundsEff

## void Ice::Render::Unsafe::CommandContext::SetViewport(U32 left, U32 top, U32 width, U32 height, float dmin, float dmax)
## 55 cycles, 54 instructions, 216 bytes
FUNC(_ZN3Ice6Render6Unsafe14CommandContext11SetViewportEjjjjff)
	lis r8, 8;							lwz r9, 8(r3);
	lis r11, 8;							
	std r6, -16(r1);
	ori r8, r8, 0xa00;					
	std r7, -24(r1);
	ori r11, r11, 0x394;					
	std r4, -32(r1);
	rlwimi r4, r6, 16, 0, 15;				
	std r5, -40(r1);
	rlwimi r5, r7, 16, 0, 15;				
	stw r8, 0(r9);
	stw r5, 8(r9);
	stw r11, 12(r9);
	stw r4, 4(r9);
	lis r10, 0x20;						
	li r5, 0;							
	ori r10, r10, 0xa20;					
	addi r0, r9, 0x3C;					
	stw r5, 56(r9);
	fsubs f8, f2, f1;					lfd f0, -16(r1);
	fcfid f0, f0;						lfd f7, -24(r1);
	fcfid f7, f7;						lfd f11, -32(r1);
	fadds f10, f1, f2;					stfs f1, 16(r9);
	fcfid f11, f11;					
	## stall 0 5
	## stall 0 6
	## stall 0 7
	## stall 0 8
	## stall 0 9
	## stall 0 10
	frsp f9, f0;						lfs 0, fHalf@toc(2);
	frsp f13, f7;						lfd 7, -40(1);
	fcfid f7, f7;						stfs 2, 20(9);
	fmuls f8, f8, f0;					
	frsp f12, f11;
	fmuls f10, f10, f0;
	## stall 9 7
	## stall 9 8
	## stall 9 9
	## stall 9 10
	fmuls f9, f9, f0;
	fmuls f13, f13, f0;				
	frsp f11, f7;					
	fadds f12, f9, f12;				
	fadds f11, f13, f11;			
	fneg f13, f13;					
	stfs f10, 36(r9);
	stfs f9, 44(r9);
	stfs f8, 52(r9);
	stw r5, 40(r9);
	stw r10, 24(r9);
	stw r0, 8(r3);
	stfs f12, 28(r9);
	stfs f11, 32(r9);
	stfs f13, 48(r9);					blr;
        .size   ._ZN3Ice6Render6Unsafe14CommandContext11SetViewportEjjjjff,.-._ZN3Ice6Render6Unsafe14CommandContext11SetViewportEjjjjff

## void Ice::Render::Unsafe::CommandContext::SetViewport(U32 left, U32 top, U32 width, U32 height, float dmin, float dmax, float scale[4], float bias[4])
## 30 cycles, 35 instructions, 140 bytes
FUNC(_ZN3Ice6Render6Unsafe14CommandContext11SetViewportEjjjjffPfS3_)
	rlwimi 4, 6, 16, 0, 15;				lwz 9, 8(3);
	rlwimi 5, 7, 16, 0, 15;				lwz 6, 116(1);
	lis 0, 0x8;							
	stw 4, 4(9);
	lis 8, 0x20;						lwz 7, 0(6);
	ori 11, 0, 0x394;					
	stw 5, 8(9);
	ori 0, 0, 0xA00;					
	stw 7, 28(9);
	ori 8, 8, 0xA20;					
	stw 11, 12(9);
	stw 0, 0(9);
	stw 8, 24(9);
	lwz 0, 4(6);
	lwz 11, 8(6);
	lwz 8, 12(6);
	stw 0, 32(9);
	stw 11, 36(9);
	lwz 0, 0(10);
	stw 8, 40(9);
	lwz 11, 4(10);
	stw 0, 44(9);
	addi 0, 9, 60;						lwz 7, 8(10);
	stw 11, 48(9);
	lwz 8, 12(10);
	stw 0, 8(3);
	stw 7, 52(9);
	stw 8, 56(9);
	stfs 1, 16(9);
	stfs 2, 20(9);						blr;
        .size   ._ZN3Ice6Render6Unsafe14CommandContext11SetViewportEjjjjffPfS3_,.-._ZN3Ice6Render6Unsafe14CommandContext11SetViewportEjjjjffPfS3_

## void Ice::Render::Unsafe::CommandContext::SetScissor(U32 left, U32 top, U32 width, U32 height)
## 9 cycles, 11 instructions, 44 bytes
FUNC(_ZN3Ice6Render6Unsafe14CommandContext10SetScissorEjjjj)
	lis 0, 8;							lwz 9, 8(3);
	rlwimi 4, 6, 16, 0, 15;
	ori 0, 0, 0x8C0;
	rlwimi 5, 7, 16, 0, 15;				
	stw 4, 4(9);
	addi 11, 9, 12;						
	stw 0, 0(9);
	stw 5, 8(9);
	stw 11, 8(3);						blr;
        .size   ._ZN3Ice6Render6Unsafe14CommandContext10SetScissorEjjjj,.-._ZN3Ice6Render6Unsafe14CommandContext10SetScissorEjjjj

## void Ice::Render::Unsafe::CommandContext::SetDepthFunc(ComparisonFunc func)
SIMPLE_STUFF_ARG_1W_FUNC(_ZN3Ice6Render6Unsafe14CommandContext12SetDepthFuncENS0_14ComparisonFuncE, 0xA6C)

## void Ice::Render::Unsafe::CommandContext::SetPolygonOffset(float factor, float units)
## 11 cycles, 10 instructions, 40 bytes
FUNC(_ZN3Ice6Render6Unsafe14CommandContext16SetPolygonOffsetEff)
	fadds 2, 2, 2;
	lis 0, 8;							lwz 9, 8(3);
	addi 11, 9, 12;
	stfs 1, 4(9);						ori 0, 0, 0xA78;					
	stw 11, 8(3);
	stw 0, 0(9);
	## stall 2 7
	## stall 2 8
	## stall 2 9
	## stall 2 10
	stfs 2, 8(9);						blr;
        .size   ._ZN3Ice6Render6Unsafe14CommandContext16SetPolygonOffsetEff,.-._ZN3Ice6Render6Unsafe14CommandContext16SetPolygonOffsetEff

## NOTE: This could be faster, but this rarely used in practice, as such it has been optimized for code size.
## void Ice::Render::Unsafe::CommandContext::SetPolygonStipplePattern(const void *pattern)
## 262 cycles, 13 instructions, 52 bytes
FUNC(_ZN3Ice6Render6Unsafe14CommandContext24SetPolygonStipplePatternEPKv)
    lis r0, 0x80;						lwz r9, 8(r3);
    li r5, 32;							lwz r10, 0(r4);
    ori r0, r0, 0x1480;		        
    mtctr r5;						
    nop;								
    stw r0, 0(r9);
iruSpspLoop:
	stwu r10, 4(r9);
	lwzu r10, 4(r4);					bdnz iruSpspLoop;
	addi r9, r9, 4;
	stw 9, 8(3);
	blr;
        .size   ._ZN3Ice6Render6Unsafe14CommandContext24SetPolygonStipplePatternEPKv,.-._ZN3Ice6Render6Unsafe14CommandContext24SetPolygonStipplePatternEPKv

## void Ice::Render::Unsafe::CommandContext::SetLineStipplePattern(U32 factor, U32 pattern)
## 9 cycles, 11 instructions, 44 bytes
FUNC(_ZN3Ice6Render6Unsafe14CommandContext21SetLineStipplePatternEjj)
    lis 11, 0x4;						lwz 9, 8(3);
    cntlzw 4, 4;
    ori 11, 11, 0x1DB8;
    subfic 4, 4, 31;
    addi 0, 9, 8;    					
    stw 11, 0(9);
	rlwimi 4, 5, 16, 0, 15;      
	stw 0, 8(3);
	stw 4, 4(9);						blr;
        .size   ._ZN3Ice6Render6Unsafe14CommandContext21SetLineStipplePatternEjj,.-._ZN3Ice6Render6Unsafe14CommandContext21SetLineStipplePatternEjj

## void Ice::Render::Unsafe::CommandContext::SetLineWidth(U32 width)
SIMPLE_STUFF_ARG_1W_FUNC(_ZN3Ice6Render6Unsafe14CommandContext12SetLineWidthEj, 0x3B8)

## void Ice::Render::Unsafe::CommandContext::SetClipFunc(ClipFunc func0, ClipFunc func1, ClipFunc func2, ClipFunc func3, ClipFunc func4, ClipFunc func5)
## 14 cycles, 13 instructions, 52 bytes
FUNC(_ZN3Ice6Render6Unsafe14CommandContext11SetClipFuncENS0_8ClipFuncES3_S3_S3_S3_S3_)
	rlwimi 4, 5, 4, 0, 27;				lwz 10, 8(3);
	lis 0, 4;
	rlwimi 4, 6, 8, 0, 23;
	## stall
	rlwimi 4, 7, 12, 0, 19;
	## stall
	rlwimi 4, 8, 16, 0, 15;
	## stall
	rlwimi 4, 9, 20, 0, 11;
	addi 11, 10, 8;
	ori 0, 0, 0x1478;					
	stw 4, 4(10);
	stw 11, 8(3);
	stw 0, 0(10);						blr;
        .size   ._ZN3Ice6Render6Unsafe14CommandContext11SetClipFuncENS0_8ClipFuncES3_S3_S3_S3_S3_,.-._ZN3Ice6Render6Unsafe14CommandContext11SetClipFuncENS0_8ClipFuncES3_S3_S3_S3_S3_

## void Ice::Render::Unsafe::CommandContext::SetPrimitiveRestartIndex(U32 index)
SIMPLE_STUFF_ARG_1W_FUNC(_ZN3Ice6Render6Unsafe14CommandContext24SetPrimitiveRestartIndexEj, 0x1DB0)

## void Ice::Render::Unsafe::CommandContext::SetPolygonMode(PolygonMode mode)
## 6 cycles, 9 instructions, 36 bytes
SIMPLE_STUFF_2ARG_2W_FUNC(_ZN3Ice6Render6Unsafe14CommandContext14SetPolygonModeENS0_11PolygonModeE, 0x1828, 4, 4)

## void Ice::Render::Unsafe::CommandContext::SetPolygonModeSeparate(PolygonMode modeFront, PolygonMode modeBack)
SIMPLE_STUFF_2ARG_2W_FUNC(_ZN3Ice6Render6Unsafe14CommandContext22SetPolygonModeSeparateENS0_11PolygonModeES3_, 0x1828, 4, 5)

## void Ice::Render::Unsafe::CommandContext::SetCullFace(CullFace cull)
SIMPLE_STUFF_ARG_1W_FUNC(_ZN3Ice6Render6Unsafe14CommandContext11SetCullFaceENS0_8CullFaceE, 0x1830)

## void Ice::Render::Unsafe::CommandContext::SetFrontFace(FrontFace front)
SIMPLE_STUFF_ARG_1W_FUNC(_ZN3Ice6Render6Unsafe14CommandContext12SetFrontFaceENS0_9FrontFaceE, 0x1834)

## void Ice::Render::Unsafe::CommandContext::SetClearColor(BgraColor color)
SIMPLE_STUFF_ARG_1W_FUNC(_ZN3Ice6Render6Unsafe14CommandContext13SetClearColorEj, 0x1D90)

## void Ice::Render::Unsafe::CommandContext::SetClearDepthStencil(U32 value)
SIMPLE_STUFF_ARG_1W_FUNC(_ZN3Ice6Render6Unsafe14CommandContext20SetClearDepthStencilEj, 0x1D8C)

## void Ice::Render::Unsafe::CommandContext::Clear(U32 flags)
SIMPLE_STUFF_ARG_1W_FUNC(_ZN3Ice6Render6Unsafe14CommandContext5ClearEj, 0x1D94)

## void Ice::Render::Unsafe::CommandContext::SetMultisampleParameters(bool enabled, bool alphaToCoverage, bool alphaToOne, U16 mask)
## 10 cycles, 11 instructions, 44 bytes
FUNC(_ZN3Ice6Render6Unsafe14CommandContext24SetMultisampleParametersEbbbt)
	rlwimi 4, 5, 4, 0, 27;				lwz 9, 8(3);
	lis 0, 4;
	rlwimi 4, 6, 8, 0, 23;
	## stall
	rlwimi 4, 7, 16, 0, 15;
	addi 11, 9, 8;
	ori 0, 0, 0x1D7C;					
	stw 4, 4(9);
	stw 11, 8(3);
	stw 0, 0(9);						blr;
        .size   ._ZN3Ice6Render6Unsafe14CommandContext24SetMultisampleParametersEbbbt,.-._ZN3Ice6Render6Unsafe14CommandContext24SetMultisampleParametersEbbbt

## void Ice::Render::Unsafe::CommandContext::SetPointSize(float psize)
## 6 cycles, 9 instructions, 36 bytes
FUNC(_ZN3Ice6Render6Unsafe14CommandContext12SetPointSizeEf)
	lis 0, 4;							lwz 9, 8(3);
	nop;
	addi 11, 9, 8;
	stfs 1, 4(9);						ori 0, 0, 0x1EE0;					
	stw 11, 8(3);
	stw 0, 0(9);						blr;
        .size   ._ZN3Ice6Render6Unsafe14CommandContext12SetPointSizeEf,.-._ZN3Ice6Render6Unsafe14CommandContext12SetPointSizeEf

## void Ice::Render::Unsafe::CommandContext::SetPointSpriteParameters(bool enabled, U32 texcoordMask, SpriteMode mode)
## 8 cycles, 10 instructions, 40 bytes
FUNC(_ZN3Ice6Render6Unsafe14CommandContext24SetPointSpriteParametersEbjNS0_10SpriteModeE)
	rlwimi 4, 6, 1, 0, 30;				lwz 9, 8(3);
	lis 0, 4;
	rlwimi 4, 5, 8, 0, 23;
	addi 11, 9, 8;
	ori 0, 0, 0x1EE8;					
	stw 4, 4(9);
	stw 11, 8(3);
	stw 0, 0(9);						blr;
        .size   ._ZN3Ice6Render6Unsafe14CommandContext24SetPointSpriteParametersEbjNS0_10SpriteModeE,.-._ZN3Ice6Render6Unsafe14CommandContext24SetPointSpriteParametersEbjNS0_10SpriteModeE

## void Ice::Render::Unsafe::CommandContext::SetVertexAttribPointer(U32 index, U32 offset, AttribContext context)
## 8 cycles, 10 instructions, 40 bytes
FUNC(_ZN3Ice6Render6Unsafe14CommandContext22SetVertexAttribPointerEjjNS0_13AttribContextE)
    slwi 4, 4, 2;						lwz 9, 8(3);
    or 6, 5, 6;
    addi 4, 4, 0x1680;
    addi 0, 9, 8;						
    stw 6, 4(9);
    oris 4, 4, 0x4;
	stw 0, 8(3);
	stw 4, 0(9);						blr;
        .size   ._ZN3Ice6Render6Unsafe14CommandContext22SetVertexAttribPointerEjjNS0_13AttribContextE,.-._ZN3Ice6Render6Unsafe14CommandContext22SetVertexAttribPointerEjjNS0_13AttribContextE

## void Ice::Render::Unsafe::CommandContext::SetVertexAttribBaseOffset(U32 baseOffset)
SIMPLE_STUFF_ARG_1W_FUNC(_ZN3Ice6Render6Unsafe14CommandContext25SetVertexAttribBaseOffsetEj, 0x1738)

## void Ice::Render::Unsafe::CommandContext::SetVertexAttribFormat(U32 index, AttribType type, AttribCount count, U32 stride, U32 divider)
## 10 cycles, 12 instructions, 48 bytes
FUNC(_ZN3Ice6Render6Unsafe14CommandContext21SetVertexAttribFormatEjNS0_10AttribTypeENS0_11AttribCountEjj)
	rlwimi 5, 6, 4, 0, 27;				lwz 9, 8(3);
	slwi 4, 4, 2;
	rlwimi 5, 7, 8, 0, 23;
	addi 4, 4, 0x1740;
	rlwimi 5, 8, 16, 0, 15;
	oris 4, 4, 4;
	addi 0, 9, 8;						
	stw 5, 4(9);
	stw 4, 0(9);
	stw 0, 8(3);						blr;
        .size   ._ZN3Ice6Render6Unsafe14CommandContext21SetVertexAttribFormatEjNS0_10AttribTypeENS0_11AttribCountEjj,.-._ZN3Ice6Render6Unsafe14CommandContext21SetVertexAttribFormatEjNS0_10AttribTypeENS0_11AttribCountEjj

## void Ice::Render::Unsafe::CommandContext::SetVertexAttribFrequencyMode(U32 mode)
## 7 cycles, 8 instructions, 32 bytes
FUNC(_ZN3Ice6Render6Unsafe14CommandContext28SetVertexAttribFrequencyModeEj)
	lis 0, 4;							lwz 9, 8(3);
	## stall
	addi 11, 9, 8;
	ori 0, 0, 0x1FC0;					
	stw 4, 4(9);
	stw 11, 8(3);
	stw 0, 0(9);						blr;
        .size   ._ZN3Ice6Render6Unsafe14CommandContext28SetVertexAttribFrequencyModeEj,.-._ZN3Ice6Render6Unsafe14CommandContext28SetVertexAttribFrequencyModeEj

## void Ice::Render::Unsafe::CommandContext::DisableVertexAttribArray(U32 index)
## 8 cycles, 10 instructions, 40 bytes
FUNC(_ZN3Ice6Render6Unsafe14CommandContext24DisableVertexAttribArrayEj)
	slwi r4, r4, 2;						lwz r9, 8(r3);
	li r5, 2;
	addi r4, r4, 0x1740;
	addi r11, r9, 8;						
	stw r5, 4(r9);
	addis r4, r4, 4;
	stw r11, 8(r3);
	stw r4, 0(r9);						blr;
        .size   ._ZN3Ice6Render6Unsafe14CommandContext24DisableVertexAttribArrayEj,.-._ZN3Ice6Render6Unsafe14CommandContext24DisableVertexAttribArrayEj

## void Ice::Render::Unsafe::CommandContext::SetVertexAttrib1f(U32 index, float x)
## 7 cycles, 9 instructions, 36 bytes
FUNC(_ZN3Ice6Render6Unsafe14CommandContext17SetVertexAttrib1fEjf)
	slwi r4, r4, 2;						lwz r9, 8(r3);
	## stall
	stfs r1, 4(r9);						addi r4, r4, 0x1E40;					
	addi r11, r9, 8;					
	addis r4, r4, 4;
	stw r11, 8(r3);
	stw r4, 0(r9);						blr;
        .size   ._ZN3Ice6Render6Unsafe14CommandContext17SetVertexAttrib1fEjf,.-._ZN3Ice6Render6Unsafe14CommandContext17SetVertexAttrib1fEjf

## void Ice::Render::Unsafe::CommandContext::SetVertexAttrib1f(U32 index, const float *data)
## 8 cycles, 10 instructions, 40 bytes
FUNC(_ZN3Ice6Render6Unsafe14CommandContext17SetVertexAttrib1fEjPKf)
	slwi r4, r4, 2;						lwz r9, 8(r3);
										lwz r6, 0(r5);
	addi r4, r4, 0x1E40;				
	addi r11, r9, 8;						
	stw r6, 4(r9);
	addis r4, r4, 4;
	stw r11, 8(r3);
	stw r4, 0(r9);						blr;
        .size   ._ZN3Ice6Render6Unsafe14CommandContext17SetVertexAttrib1fEjPKf,.-._ZN3Ice6Render6Unsafe14CommandContext17SetVertexAttrib1fEjPKf

## void Ice::Render::Unsafe::CommandContext::SetVertexAttrib2f(U32 index, float x, float y)
## 7 cycles, 10 instructions, 40 bytes
FUNC(_ZN3Ice6Render6Unsafe14CommandContext17SetVertexAttrib2fEjff)
	slwi r4, r4, 3;						lwz r9, 8(r3);
	## stall
	stfs f1, 4(r9);						addi r4, r4, 0x1880;					
	stfs f2, 8(r9);						addi r11, r9, 12;						
	oris r4, r4, 8;
	stw r11, 8(r3);
	stw r4, 0(r9);						blr;
        .size   ._ZN3Ice6Render6Unsafe14CommandContext17SetVertexAttrib2fEjff,.-._ZN3Ice6Render6Unsafe14CommandContext17SetVertexAttrib2fEjff

## void Ice::Render::Unsafe::CommandContext::SetVertexAttrib2f(U32 index, const float *data)
## 9 cycles, 12 instructions, 48 bytes
FUNC(_ZN3Ice6Render6Unsafe14CommandContext17SetVertexAttrib2fEjPKf)
	slwi r4, r4, 3;						lwz r9, 8(r3);
	lwz r6, 0(r5)
	addi r4, r4, 0x1880;				lwz r7, 4(r5)
	addi r11, r9, 12;						
	stw r6, 4(r9);
	oris r4, r4, 8;							
	stw r7, 8(r9);
	stw r11, 8(r3);
	stw r4, 0(r9);						blr;
        .size   ._ZN3Ice6Render6Unsafe14CommandContext17SetVertexAttrib2fEjPKf,.-._ZN3Ice6Render6Unsafe14CommandContext17SetVertexAttrib2fEjPKf

## void Ice::Render::Unsafe::CommandContext::SetVertexAttrib3f(U32 index, float x, float y, float z)
## 7 cycles, 11 instructions, 44 bytes
FUNC(_ZN3Ice6Render6Unsafe14CommandContext17SetVertexAttrib3fEjfff)
	slwi r4, r4, 4;						lwz r9, 8(r3);
	## stall
	stfs f1, 4(r9);						addi r4, r4, 0x1500;					
	stfs f2, 8(r9);						addi r11, r9, 16;						
	stfs f3, 12(r9);					oris r4, r4, 12;							
	stw r11, 8(r3);
	stw r4, 0(r9);						blr;
        .size   ._ZN3Ice6Render6Unsafe14CommandContext17SetVertexAttrib3fEjfff,.-._ZN3Ice6Render6Unsafe14CommandContext17SetVertexAttrib3fEjfff

## void Ice::Render::Unsafe::CommandContext::SetVertexAttrib3f(U32 index, const float *data)
## 12 cycles, 14 instructions, 56 bytes
FUNC(_ZN3Ice6Render6Unsafe14CommandContext17SetVertexAttrib3fEjPKf)
	slwi r4, r4, 4;						lwz r9, 8(r3);
	lwz r6, 0(r5);
	lwz r7, 4(r5);
	lwz r8, 8(r5);
	addi r4, r4, 0x1500;					
	stw r6, 4(r9);
	addi r11, r9, 16;						
	stw r7, 8(r9);
	oris r4, r4, 12;							
	stw r8, 12(r9);
	stw r11, 8(r3);
	stw r4, 0(r9);						blr;
        .size   ._ZN3Ice6Render6Unsafe14CommandContext17SetVertexAttrib3fEjPKf,.-._ZN3Ice6Render6Unsafe14CommandContext17SetVertexAttrib3fEjPKf

## void Ice::Render::Unsafe::CommandContext::SetVertexAttrib4f(U32 index, float x, float y, float z, float w)
## 8 cycles, 12 instructions, 48 bytes
FUNC(_ZN3Ice6Render6Unsafe14CommandContext17SetVertexAttrib4fEjffff)
	slwi r4, r4, 4;						lwz r9, 8(r3);
	## stall
	stfs f1, 4(r9);						addi r4, r4, 0x1C00;					
	stfs f2, 8(r9);						addi r11, r9, 20;						
	stfs f3, 12(r9);					oris r4, r4, 16;							
	stfs f4, 16(r9);
	stw r11, 8(r3);
	stw r4, 0(r9);						blr;
        .size   ._ZN3Ice6Render6Unsafe14CommandContext17SetVertexAttrib4fEjffff,.-._ZN3Ice6Render6Unsafe14CommandContext17SetVertexAttrib4fEjffff

## void Ice::Render::Unsafe::CommandContext::SetVertexAttrib4f(U32 index, const float *data)
## 14 cycles, 16 instructions, 64 bytes
FUNC(_ZN3Ice6Render6Unsafe14CommandContext17SetVertexAttrib4fEjPKf)
	slwi r4, r4, 4;						lwz r9, 8(r3);
	lwz r6, 0(r5);
	addi r11, r9, 20;
	lwz r7, 4(r5);						addi r4, r4, 0x1C00;
	lwz r8, 8(r5);						
	lwz r10, 12(r5);					oris r4, r4, 16;
	stw r6, 4(r9);
	stw r7, 8(r9);
	stw r8, 12(r9);
	stw r10, 16(r9);
	stw r11, 8(r3);
	stw r4, 0(r9);						blr;
        .size   ._ZN3Ice6Render6Unsafe14CommandContext17SetVertexAttrib4fEjPKf,.-._ZN3Ice6Render6Unsafe14CommandContext17SetVertexAttrib4fEjPKf

## void Ice::Render::Unsafe::CommandContext::SetVertexAttrib2s(U32 index, I16 x, I16 y)
## 8 cycles, 10 instructions, 40 bytes
FUNC(_ZN3Ice6Render6Unsafe14CommandContext17SetVertexAttrib2sEjss)
	slwi r4, r4, 2;						lwz r9, 8(r3);
	rlwimi r5, r6, 16, 0, 15;
	addi r4, r4, 0x1900;				
	addi r11, r9, 8;						
	stw r5, 4(r9);
	oris r4, r4, 4;						
	stw r11, 8(r3);
	stw r4, 0(r9);						blr;
        .size   ._ZN3Ice6Render6Unsafe14CommandContext17SetVertexAttrib2sEjss,.-._ZN3Ice6Render6Unsafe14CommandContext17SetVertexAttrib2sEjss

## void Ice::Render::Unsafe::CommandContext::SetVertexAttrib4s(U32 index, I16 x, I16 y, I16 z, I16 w)
## 10 cycles, 12 instructions, 48 bytes
FUNC(_ZN3Ice6Render6Unsafe14CommandContext17SetVertexAttrib4sEjssss)
	slwi r4, r4, 3;						lwz r9, 8(r3);
	rlwimi r5, r6, 16, 0, 15;
	rlwimi r7, r8, 16, 0, 15;
	addi r4, r4, 0x1980;					
	stw r5, 4(r9);
	addi r11, r9, 12;						
	stw r7, 8(r9);
	oris r4, r4, 8;						
	stw r11, 8(r3);
	stw r4, 0(r9);						blr;
        .size   ._ZN3Ice6Render6Unsafe14CommandContext17SetVertexAttrib4sEjssss,.-._ZN3Ice6Render6Unsafe14CommandContext17SetVertexAttrib4sEjssss

## void Ice::Render::Unsafe::CommandContext::SetVertexAttrib4Ns(U32 index, I16 x, I16 y, I16 z, I16 w)
## 10 cycles, 12 instructions, 48 bytes
FUNC(_ZN3Ice6Render6Unsafe14CommandContext18SetVertexAttrib4NsEjssss)
	slwi r4, r4, 3;						lwz r9, 8(r3);
	rlwimi r5, r6, 16, 0, 15;
	rlwimi r7, r8, 16, 0, 15;
	addi r4, r4, 0xa80;					
	stw r5, 4(r9);
	addi r11, r9, 12;						
	stw r7, 8(r9);
	oris r4, r4, 8;						
	stw r11, 8(r3);
	stw r4, 0(r9);						blr;
        .size   ._ZN3Ice6Render6Unsafe14CommandContext18SetVertexAttrib4NsEjssss,.-._ZN3Ice6Render6Unsafe14CommandContext18SetVertexAttrib4NsEjssss

## void Ice::Render::Unsafe::CommandContext::SetVertexAttrib4Nub(U32 index, U8 x, U8 y, U8 z, U8 w)
## 10 cycles, 12 instructions, 48 bytes
FUNC(_ZN3Ice6Render6Unsafe14CommandContext19SetVertexAttrib4NubEjhhhh)
	slwi r4, r4, 2;						lwz r9, 8(r3);
	rlwimi r5, r6, 8, 16, 23;
	addi r4, r4, 0x1940;				
	rlwimi r5, r7, 16, 8, 15;
	oris r4, r4, 4;							
	rlwimi r5, r8, 24, 0, 7;
	addi r11, r9, 8;						
	stw r4, 0(r9);
	stw r5, 4(r9);
	stw r11, 8(r3);						blr;
        .size   ._ZN3Ice6Render6Unsafe14CommandContext19SetVertexAttrib4NubEjhhhh,.-._ZN3Ice6Render6Unsafe14CommandContext19SetVertexAttrib4NubEjhhhh

## void Ice::Render::Unsafe::CommandContext::SetVertexAttrib4Nub(U32 index, U32 xyzwData)
FUNC(_ZN3Ice6Render6Unsafe14CommandContext19SetVertexAttrib4NubEjj)
	slwi r4, r4, 2;						lwz r9, 8(r3);
	addi r4, r4, 0x1940;				
	addi r11, r9, 8;						
	oris r4, r4, 4;							
	addi r0, r9, 4;						
	stw r4, 0(r9);
	stwbrx r5, 0, r0;
	stw r11, 8(r3);						blr;
        .size   ._ZN3Ice6Render6Unsafe14CommandContext19SetVertexAttrib4NubEjj,.-._ZN3Ice6Render6Unsafe14CommandContext19SetVertexAttrib4NubEjj

## void Ice::Render::Unsafe::CommandContext::DrawArrays(DrawMode mode, U32 start, U32 count)
## 0 Verts: 3 cycles
## 1-255 Verts: 37 cycles
## 256+ Verts: 31+ cycles
## 37 instructions, 148 bytes
FUNC(_ZN3Ice6Render6Unsafe14CommandContext10DrawArraysENS0_8DrawModeEjj)
        rldicl r12, r6, 0, 56;			
        cmpdi cr0, r6, 0;
        neg r0, r12;					beqlr cr0;
		srdi r6, r6, 8;					lwz r10, 8(r3);
        srdi r7, r0, 63;		        
        addi r8, r10, 20;				
        add r0, r6, r7;					
        stw r4, 4(r10);
        li r4, 0;				        
        sldi r11, r0, 2;				
        lis r9, 0x4;					
        add r11, r8, r11;
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
        beq cr7, iruDaNoBlocks;
        mtctr r6;
iruDaLoop:
        stw r5, 12(r10);					
        addi r10, r10, 4;					
        addi r5, r5, 256;				bdnz iruDaLoop;
iruDaNoBlocks:
        cmpdi cr7, r7, 0;
		rlwimi r5, r12, 24, 0, 7;		beqlr cr7;
        stw r5, 12(r10);				blr;
        .size   ._ZN3Ice6Render6Unsafe14CommandContext10DrawArraysENS0_8DrawModeEjj,.-._ZN3Ice6Render6Unsafe14CommandContext10DrawArraysENS0_8DrawModeEjj

## void Ice::Render::Unsafe::CommandContext::DrawElements(DrawMode mode, U32 start, U32 count, IndexType type, U32 indexOffset, IndexContext context)
## 0 Verts: 9 cycles
## 1-255 Verts: 41 cycles
## 256+ Verts: 35+ cycles
## 43 instructions, 172 bytes
FUNC(_ZN3Ice6Render6Unsafe14CommandContext12DrawElementsENS0_8DrawModeEjjNS0_9IndexTypeEjNS0_12IndexContextE)
        rldicl 12, 6, 0, 56;			lwz 10, 8(3);
        cmpdi cr0, 6, 0;
        or 7, 7, 9;						
        stw 8, 4(10);			## inputs 8&9 consumed.
        neg 0, 12;						
        stw 4, 16(10);			## inputs 4 consumed.
		srdi 6, 6, 8;					
		stw 7, 8(10);			## inputs 7 consumed.
        srdi 7, 0, 63;					beqlr cr0;
        addi 8, 10, 32;				
        add 0, 6, 7;					
        li 4, 0;				        
        sldi 11, 0, 2;				
        lis 9, 0x4;					
        add 11, 8, 11;
        slwi 0, 0, 18;
        ori 9, 9, 0x1808;				
        oris 0, 0, 0x4000;				
        stw 4, -4(11);
        lis 4, 8;
        cmpdi cr7, 6, 0;				
        stw 11, 8(3);
        ori 4, 4, 0x181c;
        ori 0, 0, 0x1824;				
        stw 9, -8(11);
        oris 5, 5, 0xff00;				
        stw 9, 12(10);
        addi 12, 12, -1;				
        stw 0, 20(10);					
        stw 4, 0(10);					beq cr7, iruDeNoBlocks;
        mtctr 6;
iruDeLoop:
        stw 5, 24(10);					
        addi 10, 10, 4;					
        addi 5, 5, 256;					bdnz iruDeLoop;
iruDeNoBlocks:
        cmpdi cr7, 7, 0;
		rlwimi 5, 12, 24, 0, 7;			beqlr cr7;
        stw 5, 24(10);					blr;
        .size   ._ZN3Ice6Render6Unsafe14CommandContext12DrawElementsENS0_8DrawModeEjjNS0_9IndexTypeEjNS0_12IndexContextE,.-._ZN3Ice6Render6Unsafe14CommandContext12DrawElementsENS0_8DrawModeEjjNS0_9IndexTypeEjNS0_12IndexContextE

## void Ice::Render::Unsafe::CommandContext::Begin(DrawMode mode)
SIMPLE_STUFF_ARG_1W_FUNC(_ZN3Ice6Render6Unsafe14CommandContext5BeginENS0_8DrawModeE, 0x1808)

## void Ice::Render::Unsafe::CommandContext::InsertIndexArrayOffsetAndFormat(IndexType type, U32 indexOffset, IndexContext context)
FUNC(_ZN3Ice6Render6Unsafe14CommandContext31InsertIndexArrayOffsetAndFormatENS0_9IndexTypeEjNS0_12IndexContextE)
	lis r0, 8;							lwz r9, 8(r3);
	or r6, r4, r6;
	ori r0, r0, 0x181C;
	stw r5, 4(r9);
	addi r11, r9, 12;
	stw r6, 8(r9);
	stw r0, 0(r9);
	stw r11, 8(r3);						blr;
        .size   ._ZN3Ice6Render6Unsafe14CommandContext31InsertIndexArrayOffsetAndFormatENS0_9IndexTypeEjNS0_12IndexContextE,.-._ZN3Ice6Render6Unsafe14CommandContext31InsertIndexArrayOffsetAndFormatENS0_9IndexTypeEjNS0_12IndexContextE

## void Ice::Render::Unsafe::CommandContext::InsertDraw(DrawType drawType, U32 start, U32 count)
FUNC(_ZN3Ice6Render6Unsafe14CommandContext10InsertDrawENS0_8DrawTypeEjj)
	rldicl r12, r6, 0, 56;			
	cmpdi cr0, r6, 0;
	neg r0, r12;					beqlr cr0;
	srdi r6, r6, 8;					lwz r10, 8(r3);
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
	beq cr7, iruDNoBlocks;
	mtctr r6;
iruDLoop:
	stwu r5, 4(r10);					
	addi r5, r5, 256;				bdnz iruDLoop;
iruDNoBlocks:
	nop;
	cmpdi cr7, r7, 0;
	rlwimi r5, r12, 24, 0, 7;		beqlr cr7;
	stw r5, 4(r10);					blr;
		.size   ._ZN3Ice6Render6Unsafe14CommandContext10InsertDrawENS0_8DrawTypeEjj,.-._ZN3Ice6Render6Unsafe14CommandContext10InsertDrawENS0_8DrawTypeEjj

## void Ice::Render::Unsafe::CommandContext::End()
SIMPLE_STUFF_CONST_1W_FUNC(_ZN3Ice6Render6Unsafe14CommandContext3EndEv, 0x1808, 0)

## void Ice::Render::Unsafe::CommandContext::InvalidatePreTransformCache()
SIMPLE_STUFF_CONST_1W_FUNC(_ZN3Ice6Render6Unsafe14CommandContext27InvalidatePreTransformCacheEv, 0x1710, 0)

## void Ice::Render::Unsafe::CommandContext::InvalidatePostTransformCache()
SIMPLE_STUFF_CONST_1W_FUNC(_ZN3Ice6Render6Unsafe14CommandContext28InvalidatePostTransformCacheEv, 0x1714, 0)

## void Ice::Render::Unsafe::CommandContext::SelectVertexProgram(const VertexProgram *__restrict program)
## 0 literals: 15 cycles
## 1 literals: 34 cycles
## 2+ literals: 34 + n * ~14 cycles
FUNC(_ZN3Ice6Render6Unsafe14CommandContext19SelectVertexProgramEPKNS0_13VertexProgramE)
	lis r10, 4;						lwz r9, 8(r3);
	lis r11, 8;						lhz r5, 0x16(r4); ## r5 = m_constantCount
	ori r12, r10, 0x1ef8;			lwz r6, 0x04(r4); ## r6 = m_vertexAttributeMask
	ori r10, r10, 0x1ea0;			lwz r7, 0x08(r4); ## r7 = m_vertexResultMask
	ori r11, r11, 0x1ff0;			lwz r8, 0x0C(r4); ## r8 = m_vertexLimits
	cmpdi cr7, r5, 0;				
	stw r12, 20(r9);
	rlwinm r12, r5, 2, 15, 29;
	stw r7, 16(r9);
	mtctr r5;						lhz r5, 0x02(r4); ## r5 = m_instructionSlot
	stw r6, 12(r9);
	stw r5, 4(r9);
	addi r5, r9, 28;
	stw r8, 24(r9);
	stw r10, 0(r9);					
	stw r11, 8(r9);					
	stw r5, 8(r3);					beqlr- cr7;
	addi r12, r12, 39;		
	lis r6, 0x14;		
	rlwinm r12, r12, 0, 0, 27;		## r12 = (sizeof(VertexProgram) + 11 + m_constantCount * 4) & ~15;
	ori r6, r6, 0x1efc;
	add r12, r12, r4;
iruSelectVertexProgramLoop:
	lwz r0, 0x18(r4);				
	lwz r7, 0(r12);					addi r4, r4, 4;
	lwz r8, 4(r12);
	lwz r10, 8(r12);
	stw r6, 0x00(r5);
	lwz r11, 12(r12);				
	stw r0, 0x04(r5);				addi r12, r12, 16;
	stw r7, 0x08(r5);					
	stw r8, 0x0C(r5);
	stw r10, 0x10(r5);
	stw r11, 0x14(r5);				
	addi r5, r5, 0x18;				bdnz iruSelectVertexProgramLoop;
	stw r5, 8(r3);					blr;
        .size   ._ZN3Ice6Render6Unsafe14CommandContext19SelectVertexProgramEPKNS0_13VertexProgramE,.-._ZN3Ice6Render6Unsafe14CommandContext19SelectVertexProgramEPKNS0_13VertexProgramE

## void Ice::Render::Unsafe::CommandContext::LoadVertexProgram(const VertexProgram *__restrict program)
FUNC(_ZN3Ice6Render6Unsafe14CommandContext17LoadVertexProgramEPKNS0_13VertexProgramE)
	lis r0, 0x4;					lwz r7, 8(r3);
	ori r0, r0, 0x1E9C;				lhz r5, 0(r4);	## r10 = m_instructionCount
	lhz r6, 2(r4); ## r11 = m_instructionSlot
	lhz r8, 0x12(r4); ## r0
	stw r6, 4(r7);
	stw r0, 0(r7);
	srdi r6, r5, 3;				
	addi r7, r7, 8;
	mtctr r6;
	cmpdi cr0, r6, 0;				add r6, r8, r4;
	rlwinm r4, r5, 0, 29, 31;		beq cr0, iruLvpNoBlocks;
iruLvpBlockLoop:
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
	addi r7, r7, 132;				bdnz iruLvpBlockLoop;
iruLvpNoBlocks:
	cmpdi cr7, r4, 0;
	stw r7, 8(r3);					beqlr cr7;
	slwi r12, r4, 20;
	addi r7, r7, 4;
	ori r0, r12, 0xB80;
	mtctr r4;
	stw r0, -4(r7);
iruLvpRemainLoop:
	lwz r0, 0(r6);					
	lwz r9, 4(r6);
	stw r0, 0(r7);
	lwz r11, 8(r6);
	stw r9, 4(r7);
	lwz r0, 12(r6);
	stw r11, 8(r7);
	addi r6, r6, 16;
	stw r0, 12(r7);
	addi r7, r7, 16;				bdnz iruLvpRemainLoop;
	stw r7, 8(r3);					blr;
		.size   ._ZN3Ice6Render6Unsafe14CommandContext17LoadVertexProgramEPKNS0_13VertexProgramE,.-._ZN3Ice6Render6Unsafe14CommandContext17LoadVertexProgramEPKNS0_13VertexProgramE

## void Ice::Render::Unsafe::CommandContext::SetVertexProgramConstant(U32 index, float x, float y, float z, float w)
FUNC(_ZN3Ice6Render6Unsafe14CommandContext24SetVertexProgramConstantEjffff)
	lis r0, 0x14;				lwz r9, 8(r3);
	## stall
	stfs f1, 8(r9);				addi r11, r9, 24;			
	stfs f2, 12(r9);			ori r0, r0, 0x1EFC;			
	stw r4, 4(r9);
	stw r0, 0(r9);
	stw r11, 8(r3);
	stfs f3, 16(r9);
	stfs f4, 20(r9);	        blr;
		.size   ._ZN3Ice6Render6Unsafe14CommandContext24SetVertexProgramConstantEjffff,.-._ZN3Ice6Render6Unsafe14CommandContext24SetVertexProgramConstantEjffff

## void Ice::Render::Unsafe::CommandContext::SetVertexProgramConstant(U32 index, const float *data)
FUNC(_ZN3Ice6Render6Unsafe14CommandContext24SetVertexProgramConstantEjPKf)
	lwz r9, 8(r3);				lis r0, 0x14;				
	lwz r6, 0x00(r5);
	lwz r7, 0x04(r5);			
	lwz r8, 0x08(r5);			addi r11, r9, 0x18;
	lwz r10, 0x0C(r5);			ori r0, r0, 0x1EFC;
	stw r6, 0x08(r9);				
	stw r7, 0x0C(r9);			
	stw r8, 0x10(r9);
	stw r10, 0x14(r9);
	stw r0, 0x00(r9);
	stw r4, 0x04(r9);
	stw r11, 8(r3);				blr;
		.size   ._ZN3Ice6Render6Unsafe14CommandContext24SetVertexProgramConstantEjPKf,.-._ZN3Ice6Render6Unsafe14CommandContext24SetVertexProgramConstantEjPKf

## void Ice::Render::Unsafe::CommandContext::SetVertexProgram2Constants(U32 index, const float *data)
FUNC(_ZN3Ice6Render6Unsafe14CommandContext26SetVertexProgram2ConstantsEjPKf)
	lwz r9, 8(r3);				lis r0, 0x24;
	lwz r6, 0x00(r5);
	lwz r7, 0x04(r5);			
	lwz r8, 0x08(r5);			ori r0, r0, 0x1EFC;
	lwz r10, 0x0C(r5);			addi r11, r9, 0x28;
	stw r6, 0x08(r9);
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
		.size   ._ZN3Ice6Render6Unsafe14CommandContext26SetVertexProgram2ConstantsEjPKf,.-._ZN3Ice6Render6Unsafe14CommandContext26SetVertexProgram2ConstantsEjPKf

## void Ice::Render::Unsafe::CommandContext::SetVertexProgram3Constants(U32 index, const float *data)
FUNC(_ZN3Ice6Render6Unsafe14CommandContext26SetVertexProgram3ConstantsEjPKf)
	lwz r9, 8(r3);				lis r0, 0x34;
	lwz r6, 0x00(r5);
	lwz r7, 0x04(r5);			
	lwz r8, 0x08(r5);			ori r0, r0, 0x1EFC;
	lwz r10, 0x0C(r5);			addi r11, r9, 0x38;
	stw r6, 0x08(r9);
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
		.size   ._ZN3Ice6Render6Unsafe14CommandContext26SetVertexProgram3ConstantsEjPKf,.-._ZN3Ice6Render6Unsafe14CommandContext26SetVertexProgram3ConstantsEjPKf

## void Ice::Render::Unsafe::CommandContext::SetVertexProgram4Constants(U32 index, const float *data)
FUNC(_ZN3Ice6Render6Unsafe14CommandContext26SetVertexProgram4ConstantsEjPKf)
	lwz r9, 8(r3);				lis r0, 0x44;
	lwz r6, 0x00(r5);
	lwz r7, 0x04(r5);			
	lwz r8, 0x08(r5);			ori r0, r0, 0x1EFC;
	lwz r10, 0x0C(r5);			addi r11, r9, 0x48;
	stw r6, 0x08(r9);
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
		.size   ._ZN3Ice6Render6Unsafe14CommandContext26SetVertexProgram4ConstantsEjPKf,.-._ZN3Ice6Render6Unsafe14CommandContext26SetVertexProgram4ConstantsEjPKf

## void Ice::Render::Unsafe::CommandContext::SetVertexProgramConstants(U32 start, U32 count, const float *__restrict data)
FUNC(_ZN3Ice6Render6Unsafe14CommandContext25SetVertexProgramConstantsEjjPKf)
	srdi r0, r5, 3;					lwz r7, 8(r3);
	rlwinm r12, r5, 20, 9, 11;
	mtctr r0;
	cmpdi cr0, r0, 0;				
	beq cr0, iruSvpcNoBlocks;
iruSvpcBlockLoop:
	lwz r0, 0x00(r6);				lis r8, 0x84;
	lwz r9, 0x04(r6);					
	lwz r11, 0x08(r6);				
	lwz r10, 0x0C(r6);				ori r8, r8, 0x1EFC;
	stw r4, 0x04(r7);
	stw r0, 0x08(r7);
	stw r9, 0x0C(r7);
	stw r11, 0x10(r7);
	stw r10, 0x14(r7);
	stw r8, 0x00(r7);
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
	addi r7, r7, 0x88;				bdnz iruSvpcBlockLoop;
iruSvpcNoBlocks:
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
iruSvpcRemainLoop:
	lwz r0, 0(r6);					
	lwz r9, 4(r6);
	lwz r11, 8(r6);
	lwz r10, 12(r6);				addi r6, r6, 16;
	stw r0, 0(r7);
	stw r9, 4(r7);
	stw r11, 8(r7);
	stw r10, 12(r7);
	addi r7, r7, 16;				bdnz iruSvpcRemainLoop;
	stw r7, 8(r3);					blr;
		.size   ._ZN3Ice6Render6Unsafe14CommandContext25SetVertexProgramConstantsEjjPKf,.-._ZN3Ice6Render6Unsafe14CommandContext25SetVertexProgramConstantsEjjPKf

## void Ice::Render::Unsafe::CommandContext::SetFragmentProgram(const FragmentProgram *__restrict program)
FUNC(_ZN3Ice6Render6Unsafe14CommandContext18SetFragmentProgramEPKNS0_15FragmentProgramE)
	lis r0, 0x4;					lwz r5, 8(r3);
	lis r11, 0x28;					lhz r6, 4(r4);		## r6 = m_centroidMask
	ori r9, r0, 0x1D60;				lwz r7, 8(r4);		## r7 = m_control
	ori r0, r0, 0x8E4;				lwz r8, 12(r4);		## r8 = m_offsetAndContext
	ori r11, r11, 0xB40;			lhz r10, 6(r4);		## r10 = m_texcoordMask
	stw r9, 8(r5);
	stw r0, 0(r5);
	stw r7, 12(r5);
	stw r8, 4(r5);
	stwu r11, 16(r5);
	li r0, 10;
	sldi r6, r6, 4;					
	mtctr r0;
iruSfpLoop:
	rlwinm r11, r6, 0, 27, 27;
	srdi r6, r6, 1;
	rlwimi r11, r10, 0, 31, 31;
	srdi r10, r10, 1;
	stwu r11, 4(r5);				bdnz iruSfpLoop;
	addi r5, r5, 4;
	stw r5, 8(r3);					blr;
		.size   ._ZN3Ice6Render6Unsafe14CommandContext18SetFragmentProgramEPKNS0_15FragmentProgramE,.-._ZN3Ice6Render6Unsafe14CommandContext18SetFragmentProgramEPKNS0_15FragmentProgramE

## void Ice::Render::Unsafe::CommandContext::SetNullFragmentProgram()
FUNC(_ZN3Ice6Render6Unsafe14CommandContext22SetNullFragmentProgramEv)
	lis r6, 0x4;					lwz r11, g_nullFragmentProgramOffset@toc(r2);
	lis r7, 0x200;					lwz r9, 8(r3);
	lis r8, 0x28;
	li r4, 1;
	ori r0, r6, 0x8E4;
	ori r6, r6, 0x1D60;
	ori r7, r7, 0x8400;				lwz r10, 0(r11);
	ori r8, r8, 0xB40;
	ori r10, r10, 2;				
	addi r11, r9, 0x3C;
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
		.size   ._ZN3Ice6Render6Unsafe14CommandContext22SetNullFragmentProgramEv,.-._ZN3Ice6Render6Unsafe14CommandContext22SetNullFragmentProgramEv

## void Ice::Render::Unsafe::CommandContext::RefreshFragmentProgram(const FragmentProgram *__restrict program)
FUNC(_ZN3Ice6Render6Unsafe14CommandContext22RefreshFragmentProgramEPKNS0_15FragmentProgramE)
        lis r0, 0x4;				lwz r9, 8(r3);
        lwz r10, 12(r4);
        ori r0, r0, 0x8E4;
        addi r11, r9, 8;
        stw r0, 0(r9);
        stw r10, 4(r9);
        stw r11, 8(r3);		        blr;
		.size   ._ZN3Ice6Render6Unsafe14CommandContext22RefreshFragmentProgramEPKNS0_15FragmentProgramE,.-._ZN3Ice6Render6Unsafe14CommandContext22RefreshFragmentProgramEPKNS0_15FragmentProgramE

## void Ice::Render::Unsafe::CommandContext::SetFragmentProgramConstant(FragmentProgram *__restrict program, U32 index, const float *__restrict data, bool noProgramChange)
FUNC(_ZN3Ice6Render6Unsafe14CommandContext26SetFragmentProgramConstantEPNS0_15FragmentProgramEjPKfb)
## r0, r8, r9, r10, r11, r12
	std r30, -16(r1);
	nop;
	lis r12, 0x8;					lwz r0, 20(r4);		## r0 = m_patchCount
	lis r10, 0x4;					lwz r9, 8(r3);
	cmplw cr7, r0, r5;									## if(m_patchCount <= index) 
	blelr cr7;						
	slwi r5, r5, 2;					
	cmpdi cr0, r7, 0;									## r7 consumed
	add r5, r5, r4;					
	lis r7, 0x1000;
	ori r0, r12, 0x6184;			lwz r5, 24(r5);		## r5 = m_patchData[index]
	ori r7, r7, 0x1000;				
	add r5, r5, r4;
	ori r12, r12, 0x6300;			
	lhz r8, 0(r5);										## r8 = m_count
	ori r10, r10, 0x630C;			
	cmpdi cr7, r8, 0;
	li r11, 0xB;					beqlr cr7;
	bne cr0, iruSfpcNoChange;		stw r12, 20(r9);	## r12 consumed
	stw r11, 24(r9);									## r11 consumed
	stw r0, 0(r9);										## r0 consumed
	lwz r0, 12(r4);										## r0 = m_offsetAndContext
	stw r10, 12(r9);									## r10 consumed
	xori r11, r0, 1;
	stw r7, 28(r9);										## r7 consumed
	rlwinm r11, r11, 0, 31, 31;
	rlwinm r10, r0, 0, 0, 29;
	oris r11, r11, 0xfeed;								## r11 = context
	stw r10, 16(r9);
	stw r11, 4(r9);
	stw r11, 8(r9);										## r11 consumed
	addi r9, r9, 0x20;
iruSfpcNoChange:
	lwz r0, 0(r6);
	stw r9, 8(r3);										## r4 consumed
	lwz r4, 4(r6);
	lwz r11, 8(r6);
	lwz r10, 12(r6);									## r6 consumed
	rlwinm r0, r0, 16, 0xffffffff;
	rlwinm r4, r4, 16, 0xffffffff;
	rlwinm r11, r11, 16, 0xffffffff;
	rlwinm r10, r10, 16, 0xffffffff;
	mtctr r8;											## r8 consumed
	lis r6, 0xc;
	lis r12, 0x1;
	lis r8, 0x10;
	ori r6, r6, 0xA304;
	ori r12, r12, 4;
	ori r8, r8, 0xA400;
	nop;
iruSfpcLoop:
	lhz r7, 2(r5);						addi r5, r5, 2;
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
	addi r9, r9, 36;					bdnz iruSfpcLoop;
	stw r9, 8(r3);						
	ld r30, -16(r1);
	blr;
		.size   ._ZN3Ice6Render6Unsafe14CommandContext26SetFragmentProgramConstantEPNS0_15FragmentProgramEjPKfb,.-._ZN3Ice6Render6Unsafe14CommandContext26SetFragmentProgramConstantEPNS0_15FragmentProgramEjPKfb

## void Ice::Render::Unsafe::CommandContext::SetFragmentProgramConstants(FragmentProgram *__restrict program, U32 start, U32 count, const float *__restrict data, bool noProgramChange)
FUNC(_ZN3Ice6Render6Unsafe14CommandContext27SetFragmentProgramConstantsEPNS0_15FragmentProgramEjjPKfb)
	## r0, r9, r10, r11, r12
	std r24, -64(r1);
	std r25, -56(r1);
	std r26, -48(r1);
	std r28, -32(r1);
	std r29, -24(r1);
	std r30, -16(r1);
	cmpdi cr0, r6, 0;
	lis r10, 0x8;
	slwi r5, r5, 2;						beqlr cr0;
	lis r11, 0x4;
	add r5, r5, r4;
	cmpdi cr7, r8, 0;					lwz r12, 8(r3);								## r8 consumed
	bne cr7, iruSfpcsLoop1;				lwz r8, 12(r4);								## r8 = m_offsetAndContext
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
iruSfpcsLoop1:
	lwz r25, 24(r5);					addi r6, r6, -1;
	lwz r8, 12(r7);						addi r5, r5, 4;
	lwz r11, 4(r7);						cmpdi cr0, r6, 0;
	lwz r10, 8(r7);						rlwinm r8, r8, 16, 0xffffffff;
	rlwinm r11, r11, 16, 0xffffffff;
	lis r24, 0xc;
	lhzux r0, r25, r4;					
	rlwinm r26, r10, 16, 0xffffffff;
	cmpdi cr7, r0, 0;					
	mtctr r0;
	lwz r0, 0(r7);						addi r7, r7, 16;
	lis r29, 0x1;
	lis r28, 0x10;
	rlwinm r30, r0, 16, 0xffffffff;		beq cr7, iruSfpcsLoop1Continue;		
	ori r24, r24, 0xA304;
	ori r29, r29, 4;
	ori r28, r28, 0xA400;
iruSfpcsLoop2:
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
	addi r12, r12, 36;					bdnz iruSfpcsLoop2;
iruSfpcsLoop1Continue:
	bne cr0, iruSfpcsLoop1;				ld r24, -64(r1);
	ld r25, -56(r1);
	ld r26, -48(r1);
	ld r28, -32(r1);
	ld r29, -24(r1);
	ld r30, -16(r1);					
	stw r12, 8(r3);						blr;
		.size   ._ZN3Ice6Render6Unsafe14CommandContext27SetFragmentProgramConstantsEPNS0_15FragmentProgramEjjPKfb,.-._ZN3Ice6Render6Unsafe14CommandContext27SetFragmentProgramConstantsEPNS0_15FragmentProgramEjjPKfb

## void Ice::Render::Unsafe::CommandContext::SetTexture(U32 unit, const Texture *__restrict texture)
FUNC(_ZN3Ice6Render6Unsafe14CommandContext10SetTextureEjPKNS0_7TextureE)
	lwz r9, 8(r3);						slwi r12, r4, 5;
	lwz r0, 0(r5);						rldic r4, r4, 2, 32;
	lwz r10, 4(r5);						addi r12, r12, 0x1A00;
	lwz r11, 8(r5);						addi r6, r4, 0x1840;
	stw r0, 4(r9);
	stw r10, 8(r9);
	lwz r0, 12(r5);						oris r12, r12, 0x20;
	lwz r10, 16(r5);					oris r6, r6, 0x4;
	stw r11, 12(r9);
	stw r12, 0(r9);
	lwz r11, 20(r5);					addi r7, r4, 0xB00;
	stw r6, 36(r9);
	stw r0, 16(r9);
	stw r10, 20(r9);
	stw r11, 24(r9);
	lwz r0, 24(r5);						addi r4, r4, 0x1D00;
	lwz r10, 28(r5);					oris r7, r7, 0x4;
	lwz r11, 36(r5);					oris r4, r4, 0x4;
	stw r0, 28(r9);
	stw r10, 32(r9);
	stw r11, 48(r9);
	stw r7, 44(r9);
	stw r4, 52(r9);
	lwz r0, 32(r5);
	lwz r10, 40(r5);					addi r11, r9, 60;
	stw r0, 40(r9);
	stw r10, 56(r9);
	stw r11, 8(r3);						blr;
		.size   ._ZN3Ice6Render6Unsafe14CommandContext10SetTextureEjPKNS0_7TextureE,.-._ZN3Ice6Render6Unsafe14CommandContext10SetTextureEjPKNS0_7TextureE

## void Ice::Render::Unsafe::CommandContext::SetTextureReduced(U32 unit, const TextureReduced *__restrict texture)
FUNC(_ZN3Ice6Render6Unsafe14CommandContext17SetTextureReducedEjPKNS0_14TextureReducedE)
	lwz r11, 8(r3);						slwi r8, r4, 5;
	lwz r0, 0(r5);						slwi r4, r4, 2;
	lwz r10, 4(r5);						addi r8, r8, 0x1A00;
	lwz r9, 8(r5);						addi r4, r4, 0x1840;
	stw r0, 4(r11);
	stw r10, 8(r11);
	lwz r0, 12(r5);						oris r8, r8, 0x1c;
	lwz r10, 16(r5);					oris r4, r4, 0x4;
	stw r9, 12(r11);
	lwz r9, 20(r5);
	stw r0, 16(r11);
	stw r10, 20(r11);
	stw r9, 24(r11);
	stw r8, 0(r11);
	stw r4, 32(r11);
	lwz r0, 24(r5);						addi r9, r11, 40;
	lwz r10, 28(r5);					
	stw r0, 28(r11);
	stw r10, 36(r11);
	stw r9, 8(r3);						blr;
		.size   ._ZN3Ice6Render6Unsafe14CommandContext17SetTextureReducedEjPKNS0_14TextureReducedE,.-._ZN3Ice6Render6Unsafe14CommandContext17SetTextureReducedEjPKNS0_14TextureReducedE

## void Ice::Render::Unsafe::CommandContext::SetTextureReducedBorderColor(U32 unit, U32 color)
FUNC(_ZN3Ice6Render6Unsafe14CommandContext28SetTextureReducedBorderColorEjj)
	lwz r9, 8(r3);						slwi r4, r4, 5;
	# stall
	addi r4, r4, 0x1A1C;
	stw r5, 4(r9);
	addi r0, r9, 8;
	oris r4, r4, 0x4;
	stw r0, 8(r3);
	stw r4, 0(r9);
	blr;
		.size   ._ZN3Ice6Render6Unsafe14CommandContext28SetTextureReducedBorderColorEjj,.-._ZN3Ice6Render6Unsafe14CommandContext28SetTextureReducedBorderColorEjj

## void Ice::Render::Unsafe::CommandContext::SetTextureReducedBorderDepth(U32 unit, float depth)
FUNC(_ZN3Ice6Render6Unsafe14CommandContext28SetTextureReducedBorderDepthEjf)
	slwi r4, r4, 5;						lfs f0, fDepthMul@toc(r2);
	fmuls f1, f1, f0;					lwz r9, 8(r3);
	addi r4, r4, 0x1A1C;
	addi r0, r9, 8;
	oris r4, r4, 0x4;
	stw r0, 8(r3);
	stw r4, 0(r9);						fctidz f1, f1;
	stfd f1, -16(r1);
	# 50 cycle stall
	ld r0, -16(r1);
	stw r0, 4(r9);						blr;
		.size   ._ZN3Ice6Render6Unsafe14CommandContext28SetTextureReducedBorderDepthEjf,.-._ZN3Ice6Render6Unsafe14CommandContext28SetTextureReducedBorderDepthEjf

## void Ice::Render::Unsafe::CommandContext::SetTextureReducedColorKey(U32 unit, U32 color)
FUNC(_ZN3Ice6Render6Unsafe14CommandContext25SetTextureReducedColorKeyEjj)
	slwi r4, r4, 2;						lwz r9, 8(r3);
	## stall
	addi r4, r4, 0x1D00;
	stw r5, 4(r9);
	addi r0, r9, 8;
	oris r4, r4, 0x4;
	stw r0, 8(r3);
	stw r4, 0(r9);						blr;
		.size   ._ZN3Ice6Render6Unsafe14CommandContext25SetTextureReducedColorKeyEjj,.-._ZN3Ice6Render6Unsafe14CommandContext25SetTextureReducedColorKeyEjj

## void Ice::Render::Unsafe::CommandContext::SetTextureReducedControl3(U32 unit, U32 control3Value)
FUNC(_ZN3Ice6Render6Unsafe14CommandContext25SetTextureReducedControl3Ejj)
	slwi r4, r4, 2;						lwz r9, 8(r3);
	## stall
	addi r4, r4, 0xB00;
	stw r5, 4(r9);
	addi r0, r9, 8;
	oris r4, r4, 0x4;
	stw r0, 8(r3);
	stw r4, 0(r9);						blr;
		.size   ._ZN3Ice6Render6Unsafe14CommandContext25SetTextureReducedControl3Ejj,.-._ZN3Ice6Render6Unsafe14CommandContext25SetTextureReducedControl3Ejj

## void Ice::Render::Unsafe::CommandContext::DisableTexture(U32 unit)
FUNC(_ZN3Ice6Render6Unsafe14CommandContext14DisableTextureEj)
	slwi r4, r4, 5;						lwz r9, 8(r3);
	li r0, 0;
	addi r4, r4, 0x1A0C;
	addi r11, r9, 8;
	stw r0, 4(r9);
	oris r4, r4, 0x4;
	stw r11, 8(r3);
	stw r4, 0(r9);						blr;
		.size   ._ZN3Ice6Render6Unsafe14CommandContext14DisableTextureEj,.-._ZN3Ice6Render6Unsafe14CommandContext14DisableTextureEj

## void Ice::Render::Unsafe::CommandContext::SetVertexProgramTexture(U32 unit, const Texture *__restrict texture)
FUNC(_ZN3Ice6Render6Unsafe14CommandContext23SetVertexProgramTextureEjPKNS0_7TextureE)
	li r0, -13;							lwz r11, 8(r3);
	slwi r4, r4, 5;						lwz r10, 4(r5);
	oris r0, r0, 0xffff;
	lwz r9, 0(r5);
	addi r4, r4, 0x900;
	and r10, r10, r0;
	oris r4, r4, 0x20;
	stw r10, 8(r11);
	li r10, 0x0F0F;						lwz r0, 8(r5);
	stw r4, 0(r11);
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
	lwz r10, 28(r5);					addi r0, r11, 36;
	stw r10, 32(r11);
	stw r0, 8(r3);						blr;
		.size   ._ZN3Ice6Render6Unsafe14CommandContext23SetVertexProgramTextureEjPKNS0_7TextureE,.-._ZN3Ice6Render6Unsafe14CommandContext23SetVertexProgramTextureEjPKNS0_7TextureE

## void Ice::Render::Unsafe::CommandContext::DisableVertexProgramTexture(U32 unit)
FUNC(_ZN3Ice6Render6Unsafe14CommandContext27DisableVertexProgramTextureEj)
	slwi r4, r4, 5;						lwz r9, 8(r3);
	li r0, 0;
	addi r4, r4, 0x90C;
	addi r11, r9, 8;
	stw r0, 4(r9);
	oris r4, r4, 0x4;
	stw r11, 8(r3);
	stw r4, 0(r9);						blr;
		.size   ._ZN3Ice6Render6Unsafe14CommandContext27DisableVertexProgramTextureEj,.-._ZN3Ice6Render6Unsafe14CommandContext27DisableVertexProgramTextureEj

## void Ice::Render::Unsafe::CommandContext::InvalidateTextureCache()
SIMPLE_STUFF_CONST_1W_FUNC(_ZN3Ice6Render6Unsafe14CommandContext22InvalidateTextureCacheEv, 0x1FD8, 1)

## void Ice::Render::Unsafe::CommandContext::InvalidateVertexTextureCache()
SIMPLE_STUFF_CONST_1W_FUNC(_ZN3Ice6Render6Unsafe14CommandContext28InvalidateVertexTextureCacheEv, 0x1FD8, 2)

## void Ice::Render::Unsafe::CommandContext::InvalidateDepthCull()
SIMPLE_STUFF_CONST_1W_FUNC(_ZN3Ice6Render6Unsafe14CommandContext19InvalidateDepthCullEv, 0x234, 0)

## void Ice::Render::Unsafe::CommandContext::SetDepthCullControl(DepthCullDirection dir, DepthCullFormat format)
FUNC(_ZN3Ice6Render6Unsafe14CommandContext19SetDepthCullControlENS0_18DepthCullDirectionENS0_15DepthCullFormatE)
	rlwimi r4, r5, 4, 0, 27;			lwz r9, 8(r3);
	lis r0, 4;
	addi r11, r9, 8;
	ori r0, r0, 0x1EA4;
	stw r4, 4(r9);
	stw r0, 0(r9);
	stw r11, 8(r3);						blr;
		.size   ._ZN3Ice6Render6Unsafe14CommandContext19SetDepthCullControlENS0_18DepthCullDirectionENS0_15DepthCullFormatE,.-._ZN3Ice6Render6Unsafe14CommandContext19SetDepthCullControlENS0_18DepthCullDirectionENS0_15DepthCullFormatE
		
SIMPLE_STUFF_ARG_1W_FUNC(_ZN3Ice6Render6Unsafe14CommandContext21SetDepthCullDirectionENS0_18DepthCullDirectionE, 0x1EA4)

## void Ice::Render::Unsafe::CommandContext::SetStencilCullHint(ComparisonFunc func, U32 ref, U32 mask)
FUNC(_ZN3Ice6Render6Unsafe14CommandContext18SetStencilCullHintENS0_14ComparisonFuncEjj)
	slwi r6, r6, 24;					lwz r9, 8(r3);
	lis r0, 0x4;
	rlwimi r6, r5, 16, 8, 15;
	ori r0, r0, 0x1EAC;
	rlwimi r6, r4, 0, 0xff;
	addi r11, r9, 8;
	stw r6, 4(r9);
	stw r0, 0(r9);
	stw r11, 8(r3);						blr;
		.size   ._ZN3Ice6Render6Unsafe14CommandContext18SetStencilCullHintENS0_14ComparisonFuncEjj,.-._ZN3Ice6Render6Unsafe14CommandContext18SetStencilCullHintENS0_14ComparisonFuncEjj

## void Ice::Render::Unsafe::CommandContext::SetDepthCullFeedback(U32 feedbackValue)
SIMPLE_STUFF_ARG_1W_FUNC(_ZN3Ice6Render6Unsafe14CommandContext20SetDepthCullFeedbackEj, 0x1EA8)

## void Ice::Render::Unsafe::CommandContext::InsertReference(I32 value)
SIMPLE_STUFF_ARG_1W_FUNC(_ZN3Ice6Render6Unsafe14CommandContext15InsertReferenceEi, 0x0050)

## void Ice::Render::Unsafe::CommandContext::SetWaitSemaphoreIndex(U32 index)
FUNC(_ZN3Ice6Render6Unsafe14CommandContext21SetWaitSemaphoreIndexEj)
	lis r0, 0x4;						lwz r9, 8(r3);
	slwi r4, r4, 4;
	ori r0, r0, 0x0064;
	addi r11, r9, 8;
	stw r4, 4(r9);
	stw r0, 0(r9);
	stw r11, 8(r3);						blr;
		.size   ._ZN3Ice6Render6Unsafe14CommandContext21SetWaitSemaphoreIndexEj,.-._ZN3Ice6Render6Unsafe14CommandContext21SetWaitSemaphoreIndexEj

## void Ice::Render::Unsafe::CommandContext::WaitOnSemaphore(U32 value)
SIMPLE_STUFF_ARG_1W_FUNC(_ZN3Ice6Render6Unsafe14CommandContext15WaitOnSemaphoreEj, 0x0068)

## void Ice::Render::Unsafe::CommandContext::SetFlushPipeSemaphoreIndex(U32 index)
FUNC(_ZN3Ice6Render6Unsafe14CommandContext26SetFlushPipeSemaphoreIndexEj)
	lis r0, 0x4;						lwz r9, 8(r3);
	slwi r4, r4, 4;
	ori r0, r0, 0x1D6C;
	addi r11, r9, 8;
	stw r4, 4(r9);
	stw r0, 0(r9);
	stw r11, 8(r3);						blr;
		.size   ._ZN3Ice6Render6Unsafe14CommandContext26SetFlushPipeSemaphoreIndexEj,.-._ZN3Ice6Render6Unsafe14CommandContext26SetFlushPipeSemaphoreIndexEj

## void Ice::Render::Unsafe::CommandContext::FlushTexturePipeAndWriteSemaphore(U32 value)
SIMPLE_STUFF_ARG_1W_FUNC(_ZN3Ice6Render6Unsafe14CommandContext33FlushTexturePipeAndWriteSemaphoreEj, 0x1D74)

## void Ice::Render::Unsafe::CommandContext::FlushBackendPipeAndWriteSemaphore(U32 value)
FUNC(_ZN3Ice6Render6Unsafe14CommandContext33FlushBackendPipeAndWriteSemaphoreEj)
	mr r6, r4;							lwz r9, 8(r3);
	li r5, 0x1D70;
	addi r0, r9, 8;
	rlwimi r6, r4, 16, 24, 31;
	oris r5, r5, 0x4;
	rlwimi r6, r4, 16, 8, 15;
	stw r5, 0(r9);
	stw r6, 4(r9);
	stw r0, 8(r3);						
	blr;
		.size   ._ZN3Ice6Render6Unsafe14CommandContext33FlushBackendPipeAndWriteSemaphoreEj,.-._ZN3Ice6Render6Unsafe14CommandContext33FlushBackendPipeAndWriteSemaphoreEj

## void Ice::Render::Unsafe::CommandContext::InsertJump(U32 offset)
FUNC(_ZN3Ice6Render6Unsafe14CommandContext10InsertJumpEj)
	oris r4, r4, 0x2000;				lwz r9, 8(r3);
	## stall	
	addi r0, r9, 4;
	stw r4, 0(r9);
	stw r0, 8(r3);						blr;
		.size   ._ZN3Ice6Render6Unsafe14CommandContext10InsertJumpEj,.-._ZN3Ice6Render6Unsafe14CommandContext10InsertJumpEj

## void Ice::Render::Unsafe::CommandContext::InsertCall(U32 offset)
FUNC(_ZN3Ice6Render6Unsafe14CommandContext10InsertCallEj)
	ori r4, r4, 2;						lwz r9, 8(r3);
	## stall
	addi r0, r9, 4;
	stw r4, 0(r9);
	stw r0, 8(r3);						blr;
		.size   ._ZN3Ice6Render6Unsafe14CommandContext10InsertCallEj,.-._ZN3Ice6Render6Unsafe14CommandContext10InsertCallEj

## void Ice::Render::Unsafe::CommandContext::InsertReturn()
FUNC(_ZN3Ice6Render6Unsafe14CommandContext12InsertReturnEv)
	lis r0, 0x2;						lwz r9, 8(r3);
	## stall
	addi r11, r9, 4;
	stw r0, 0(r9);
	stw r11, 8(r3);						blr;
		.size   ._ZN3Ice6Render6Unsafe14CommandContext12InsertReturnEv,.-._ZN3Ice6Render6Unsafe14CommandContext12InsertReturnEv

## void Ice::Render::Unsafe::CommandContext::InsertNop()
SIMPLE_STUFF_CONST_1W_FUNC(_ZN3Ice6Render6Unsafe14CommandContext9InsertNopEv, 0x100, 0)

## void Ice::Render::Unsafe::CommandContext::InsertHole(U32 size)
FUNC(_ZN3Ice6Render6Unsafe14CommandContext10InsertHoleEj)
	cmpdi cr0, r4, 0;
	beqlr cr0;
	lwz r9, 8(r3);						li r0, 0;
	## stall
	addi r5, r9, 15;
	stw r0, 0(r9);
	rlwinm r5, r5, 0, 0, 27;
	stw r0, 4(r9);
	add r4, r5, r4;
	stw r0, 8(r9);
	stw r4, 8(r3);
	mr r3, r5;							blr;
		.size   ._ZN3Ice6Render6Unsafe14CommandContext10InsertHoleEj,.-._ZN3Ice6Render6Unsafe14CommandContext10InsertHoleEj

## void Ice::Render::Unsafe::CommandContext::SetRenderControl(RenderControlMode mode, U32 ioOffset)
FUNC(_ZN3Ice6Render6Unsafe14CommandContext16SetRenderControlENS0_17RenderControlModeEj)
	lis r10, 0x4;						lwz r9, 8(r3);
	or r5, r4, r5;
	addi r0, r9, 16;
	ori r11, r10, 0x0100;
	stw r0, 8(r3);
	ori r10, r10, 0x1e98;
	stw r5, 12(r9);
	li r0, 0;
	stw r11, 0(r9);
	stw r10, 8(r9);
	stw r0, 4(r9);
	blr;
		.size   ._ZN3Ice6Render6Unsafe14CommandContext16SetRenderControlENS0_17RenderControlModeEj,.-._ZN3Ice6Render6Unsafe14CommandContext16SetRenderControlENS0_17RenderControlModeEj

## void Ice::Render::Unsafe::CommandContext::SetWindowClipMode(WindowClipMode mode)
SIMPLE_STUFF_ARG_1W_FUNC(_ZN3Ice6Render6Unsafe14CommandContext17SetWindowClipModeENS0_14WindowClipModeE, 0x2BC)

## void Ice::Render::Unsafe::CommandContext::SetWindow(U32 index, U32 left, U32 top, U32 right, U32 bottom)
FUNC(_ZN3Ice6Render6Unsafe14CommandContext9SetWindowEjjjjj)
	slwi r4, r4, 3;						lwz r9, 8(r3);
	rlwimi r5, r7, 16, 0, 15;
	addi r4, r4, 0x2c0;
	rlwimi r6, r8, 16, 0, 15;
	oris r4, r4, 0x8;
	stw r5, 4(r9);
	addi r0, r9, 12;
	stw r6, 8(r9);
	stw r4, 0(r9);
	stw r0, 8(r3);						blr;
		.size   ._ZN3Ice6Render6Unsafe14CommandContext9SetWindowEjjjjj,.-._ZN3Ice6Render6Unsafe14CommandContext9SetWindowEjjjjj

## void Ice::Render::Unsafe::CommandContext::InsertPpuInterrupt(U32 parameterValue)
FUNC(_ZN3Ice6Render6Unsafe14CommandContext18InsertPpuInterruptEj)
	lis r10, 0xcafe;					lwz r9, 8(r3);
	lis r8, 0x4;
	stw r4, 12(r9);
	ori r0, r8, 0xe000;
	ori r10, r10, 0xbabe;
	ori r8, r8, 0xeb00;
	stw r0, 0(r9);
	addi r11, r9, 16;
	stw r10, 4(r9);
	stw r8, 8(r9);
	stw r11, 8(r3);
	blr;
		.size   ._ZN3Ice6Render6Unsafe14CommandContext18InsertPpuInterruptEj,.-._ZN3Ice6Render6Unsafe14CommandContext18InsertPpuInterruptEj

## void Ice::Render::Unsafe::CommandContext::WritePerformanceReport()
FUNC(_ZN3Ice6Render6Unsafe14CommandContext22WritePerformanceReportEv)
	lis r6, 0x4;						lwz r9, 8(r3);
	li r4, 0;
	li r5, 1;
	ori r7, r6, 0x110;
	ori r6, r6, 0x140;
	addi r0, r9, 16;
	stw r7, 0(r9);
	stw r4, 4(r9);
	stw r6, 8(r9);
	stw r5, 12(r9);
	stw r0, 8(r3);
	blr;
		.size   ._ZN3Ice6Render6Unsafe14CommandContext22WritePerformanceReportEv,.-._ZN3Ice6Render6Unsafe14CommandContext22WritePerformanceReportEv

## void Ice::Render::Unsafe::CommandContext::SetRenderTarget(const RenderTarget *__restrict target)
FUNC(_ZN3Ice6Render6Unsafe14CommandContext15SetRenderTargetEPKNS0_12RenderTargetE)
	lwz r9, 8(r3);
	nop; ## fixme
	lwz r12, 4(r4);						lis r0, 0x20;
	lwz r11, 8(r4);						lis r5, 0x4;
	stw r12, 4(r9);
	stw r11, 8(r9);
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
	lwz r10, 60(r4);					addi r8, r9, 0x78;
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
		.size   ._ZN3Ice6Render6Unsafe14CommandContext15SetRenderTargetEPKNS0_12RenderTargetE,.-._ZN3Ice6Render6Unsafe14CommandContext15SetRenderTargetEPKNS0_12RenderTargetE

## void Ice::Render::Unsafe::CommandContext::ToggleRenderState(RenderState state, bool enable)
FUNC(_ZN3Ice6Render6Unsafe14CommandContext17ToggleRenderStateENS0_11RenderStateEb)
	oris r4, r4, 0x4;					lwz r9, 8(r3);
	nop;
	addi r11, r9, 8;
	stw r4, 0(r9);
	stw r5, 4(r9);
	stw r11, 8(r3);						blr;
		.size   ._ZN3Ice6Render6Unsafe14CommandContext17ToggleRenderStateENS0_11RenderStateEb,.-._ZN3Ice6Render6Unsafe14CommandContext17ToggleRenderStateENS0_11RenderStateEb

## void Ice::Render::Unsafe::CommandContext::SetDepthMinMaxControl(bool cullNearFarEnable, bool clampEnable, bool cullIgnoreW)
FUNC(_ZN3Ice6Render6Unsafe14CommandContext21SetDepthMinMaxControlEbbb)
	rlwimi  r4, r5, 4, 0, 27;			lwz     r9, 8(r3);
	lis     r0, 4;
	addi    r11, r9, 8;
	rlwimi  r4, r6, 8, 0, 23;
	ori     r0, r0, 0x1D78;
	stw     r4, 4(r9);
	stw     r0, 0(r9);
	stw     r11, 8(r3);					blr;
		.size   ._ZN3Ice6Render6Unsafe14CommandContext21SetDepthMinMaxControlEbbb,.-._ZN3Ice6Render6Unsafe14CommandContext21SetDepthMinMaxControlEbbb

## void Ice::Render::Unsafe::CommandContext::SetTextureCylindricalWrapEnable(U32 enable0to7, U32 enable8to9)
SIMPLE_STUFF_2ARG_2W_FUNC(_ZN3Ice6Render6Unsafe14CommandContext31SetTextureCylindricalWrapEnableEjj, 0x238, r4,r5)

## void Ice::Render::Unsafe::CommandContext::SetVertexProgramStaticBranchBits(U32 branchBits)
SIMPLE_STUFF_ARG_1W_FUNC(_ZN3Ice6Render6Unsafe14CommandContext32SetVertexProgramStaticBranchBitsEj, 0x1ff8)
