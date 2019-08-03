/*
* Copyright (c) 2005 Naughty Dog, Inc. 
* A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
* Use and distribution without consent strictly prohibited
* 
* Revision History:
*  - Created 7/26/05
*/
#ifndef ICESPUCODEGENERATION_H
#define ICESPUCODEGENERATION_H

namespace Ice
{
	namespace MeshProc
	{
		enum SpuOpCode
		{
			kOpBr		= 0x190,
			kOpBrsl		= 0x198,
			kOpBra		= 0x180,
			kOpBrasl	= 0x188,
			kOpFsmbi	= 0x194,
			kOpLqa		= 0x184,
			kOpLqr		= 0x19C,
			kOpStop		= 0x000,
			kOpStopd	= 0x140,
			kOpLnop		= 0x001,
			kOpSync		= 0x002,
			kOpDsync	= 0x003,
			kOpMfspr	= 0x00c,
			kOpRdch		= 0x00d,
			kOpRchcnt	= 0x00f,
			kOpHbra		= 0x080,
			kOpHbrr		= 0x090,
			kOpBrz		= 0x100,
			kOpBrnz		= 0x108,
			kOpBrhz		= 0x110,
			kOpBrhnz	= 0x118,
			kOpStqa		= 0x104,
			kOpStqr		= 0x11C,
			kOpMtspr	= 0x10c,
			kOpWrch		= 0x10d,
			kOpLqd		= 0x1a0,
			kOpBi		= 0x1a8,
			kOpBisl		= 0x1a9,
			kOpIret		= 0x1aa,
			kOpBisled	= 0x1ab,
			kOpHbr		= 0x1ac,
			kOpFrest	= 0x1b8,
			kOpFrsqest	= 0x1b9,
			kOpFsm		= 0x1b4,
			kOpFsmh		= 0x1b5,
			kOpFsmb		= 0x1b6,
			kOpGb		= 0x1b0,
			kOpGbh		= 0x1b1,
			kOpGbb		= 0x1b2,
			kOpCbd		= 0x1f4,
			kOpChd		= 0x1f5,
			kOpCwd		= 0x1f6,
			kOpCdd		= 0x1f7,
			kOpRotqbii	= 0x1f8,
			kOpRotqbyi	= 0x1fc,
			kOpRotqmbii	= 0x1f9,
			kOpRotqmbyi	= 0x1fd,
			kOpShlqbii	= 0x1fb,
			kOpShlqbyi	= 0x1ff,
			kOpStqd		= 0x120,
			kOpBihnz	= 0x12b,
			kOpBihz		= 0x12a,
			kOpBinz		= 0x129,
			kOpBiz		= 0x128,
			kOpCbx		= 0x1d4,
			kOpChx		= 0x1d5,
			kOpCwx		= 0x1d6,
			kOpCdx		= 0x1d7,
			kOpLqx		= 0x1c4,
			kOpRotqbi	= 0x1d8,
			kOpRotqmbi	= 0x1d9,
			kOpShlqbi	= 0x1db,
			kOpRotqby	= 0x1dc,
			kOpRotqmby	= 0x1dd,
			kOpShlqby	= 0x1df,
			kOpRotqbybi	= 0x1cc,
			kOpRotqmbybi	= 0x1cd,
			kOpShlqbybi	= 0x1cf,
			kOpStqx		= 0x144,
			kOpShufb	= 0x580,
			kOpIl		= 0x204,
			kOpIlh		= 0x20c,
			kOpIlhu		= 0x208,
			kOpIla		= 0x210,
			kOpNop		= 0x201,
			kOpIohl		= 0x304,
			kOpAndbi	= 0x0b0,
			kOpAndhi	= 0x0a8,
			kOpAndi		= 0x0a0,
			kOpOrbi		= 0x030,
			kOpOrhi		= 0x028,
			kOpOri		= 0x020,
			kOpXorbi	= 0x230,
			kOpXorhi	= 0x228,
			kOpXori		= 0x220,
			kOpAhi		= 0x0e8,
			kOpAi		= 0x0e0,
			kOpSfhi		= 0x068,
			kOpSfi		= 0x060,
			kOpCgtbi	= 0x270,
			kOpCgthi	= 0x268,
			kOpCgti		= 0x260,
			kOpClgtbi	= 0x2f0,
			kOpClgthi	= 0x2e8,
			kOpClgti	= 0x2e0,
			kOpCeqbi	= 0x3f0,
			kOpCeqhi	= 0x3e8,
			kOpCeqi		= 0x3e0,
			kOpHgti		= 0x278,
			kOpHlgti	= 0x2f8,
			kOpHeqi		= 0x3f8,
			kOpMpyi		= 0x3a0,
			kOpMpyui	= 0x3a8,
			kOpCflts	= 0x3b0,
			kOpCfltu	= 0x3b2,
			kOpCsflt	= 0x3b4,
			kOpCuflt	= 0x3b6,
			kOpFesd		= 0x3b8,
			kOpFrds		= 0x3b9,
			kOpFscrrd	= 0x398,
			kOpFscrwr	= 0x3ba,
			kOpClz		= 0x2a5,
			kOpCntb		= 0x2b4,
			kOpXsbh		= 0x2b6,
			kOpXshw		= 0x2ae,
			kOpXswd		= 0x2a6,
			kOpRoti		= 0x078,
			kOpRotmi	= 0x079,
			kOpRotmai	= 0x07a,
			kOpShli		= 0x07b,
			kOpRothi	= 0x07c,
			kOpRothmi	= 0x07d,
			kOpRotmahi	= 0x07e,
			kOpShlhi	= 0x07f,
			kOpA		= 0x0c0,
			kOpAh		= 0x0c8,
			kOpSf		= 0x040,
			kOpSfh		= 0x048,
			kOpCgt		= 0x240,
			kOpCgtb		= 0x250,
			kOpCgth		= 0x248,
			kOpClgt		= 0x2c0,
			kOpClgtb	= 0x2d0,
			kOpClgth	= 0x2c8,
			kOpCeq		= 0x3c0,
			kOpCeqb		= 0x3d0,
			kOpCeqh		= 0x3c8,
			kOpHgt		= 0x258,
			kOpHlgt		= 0x2d8,
			kOpHeq		= 0x3d8,
			kOpFceq		= 0x3c2,
			kOpFcmeq	= 0x3ca,
			kOpFcgt		= 0x2c2,
			kOpFcmgt	= 0x2ca,
			kOpAnd		= 0x0c1,
			kOpNand		= 0x0c9,
			kOpOr		= 0x041,
			kOpNor		= 0x049,
			kOpXor		= 0x241,
			kOpEqv		= 0x249,
			kOpAndc		= 0x2c1,
			kOpOrc		= 0x2c9,
			kOpAbsdb	= 0x053,
			kOpAvgb		= 0x0d3,
			kOpSumb		= 0x253,
			kOpDfa		= 0x2cc,
			kOpDfm		= 0x2ce,
			kOpDfs		= 0x2cd,
			kOpFa		= 0x2c4,
			kOpFm		= 0x2c6,
			kOpFs		= 0x2c5,
			kOpMpy		= 0x3c4,
			kOpMpyh		= 0x3c5,
			kOpMpyhh	= 0x3c6,
			kOpMpyhhu	= 0x3ce,
			kOpMpys		= 0x3c7,
			kOpMpyu		= 0x3cc,
			kOpFi		= 0x3d4,
			kOpRot		= 0x058,
			kOpRotm		= 0x059,
			kOpRotma	= 0x05a,
			kOpShl		= 0x05b,
			kOpRoth		= 0x05c,
			kOpRothm	= 0x05d,
			kOpRotmah	= 0x05e,
			kOpShlh		= 0x05f,
			kOpMpyhha	= 0x346,
			kOpMpyhhau	= 0x34e,
			kOpDfma		= 0x35c,
			kOpDfms		= 0x35d,
			kOpDfnms	= 0x35e,
			kOpDfnma	= 0x35f,
			kOpFma		= 0x700,
			kOpFms		= 0x780,
			kOpFnms		= 0x680,
			kOpMpya		= 0x600,
			kOpSelb		= 0x400,
			kOpAddx		= 0x340,
			kOpCg		= 0x0c2,
			kOpCgx		= 0x342,
			kOpSfx		= 0x341,
			kOpBg		= 0x042,
			kOpBgx		= 0x343,
			kOpR00		= 0x1c0,
			kOpR01		= 0x1c1,
			kOpR02		= 0x1c2,
			kOpR03		= 0x1c3,
			kOpR04		= 0x1d0,
			kOpR05		= 0x1d1,
			kOpR06		= 0x1d2,
			kOpR07		= 0x1d3,
			kOpR11		= 0x145,
			kOpR12		= 0x146,
			kOpR13		= 0x147,
			kOpR14		= 0x154,
			kOpR15		= 0x155,
			kOpR16		= 0x156,
			kOpR17		= 0x157,
			kOpR20		= 0x1e8,
			kOpR31		= 0x1ad,
			kOpR32		= 0x1ae,
			kOpR33		= 0x1af,
			kOpHbii		= 0x12c,
			kOpR41		= 0x12d,
			kOpR42		= 0x12e,
			kOpR43		= 0x12f,
			kOpHbchnz	= 0x117,
			kOpHbchz	= 0x116,
			kOpHbcnz	= 0x115,
			kOpHbcz		= 0x114,
			kOpR50		= 0x010,
			kOpR60		= 0x008,
			kOpR70		= 0x200,
			kOpR72		= 0x202,
			kOpR73		= 0x203,
			kOpR80		= 0x378,
		};

		inline U32 MakeLqd(U32 resultReg, I32 imm, U32 addressReg)
		{
			return	  (kOpLqd << 21) 
				| ((((U32)imm >> 4) & 0x3FF) << 14) 
				| ((addressReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeLqx(U32 resultReg, U32 addressRegA, U32 addressRegB)
		{
			return	  (kOpLqx << 21)
				| ((addressRegB & 0x7f) << 14)
				| ((addressRegA & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeLqa(U32 resultReg, I32 imm)
		{
			return	  (kOpLqa << 21)
				| ((((U32)imm >> 2) & 0xffff) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeLqr(U32 resultReg, I32 imm)
		{
			return	  (kOpLqr << 21)
				| ((((U32)imm >> 2) & 0xffff) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeStqd(U32 resultReg, I32 imm, U32 addressReg)
		{
			return	  (kOpStqd << 21) 
				| ((((U32)imm >> 4) & 0x3FF) << 14) 
				| ((addressReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeStqx(U32 resultReg, U32 addressRegA, U32 addressRegB)
		{
			return	  (kOpStqx << 21)
				| ((addressRegB & 0x7f) << 14)
				| ((addressRegA & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeStqa(U32 resultReg, I32 imm)
		{
			return	  (kOpStqa << 21)
				| ((((U32)imm >> 2) & 0xffff) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeStqr(U32 resultReg, I32 imm)
		{
			return	  (kOpStqr << 21)
				| ((((U32)imm >> 2) & 0xffff) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeCbd(U32 resultReg, I32 imm, U32 aReg)
		{
			return	  (kOpCbd << 21)
				| (((U32)imm >> 14) & 0x7F)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeCbx(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpCbx << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeChd(U32 resultReg, I32 imm, U32 aReg)
		{
			return	  (kOpChd << 21)
				| (((U32)imm >> 14) & 0x7F)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeChx(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpChx << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeCwd(U32 resultReg, I32 imm, U32 aReg)
		{
			return	  (kOpCwd << 21)
				| (((U32)imm >> 14) & 0x7F)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeCwx(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpCwx << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeCdd(U32 resultReg, I32 imm, U32 aReg)
		{
			return	  (kOpCdd << 21)
				| (((U32)imm >> 14) & 0x7F)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeCdx(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpCdx << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeIlh(U32 resultReg, I32 imm)
		{
			return	  (kOpIlh << 21)
				| (((U32)imm & 0xffff) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeIlhu(U32 resultReg, I32 imm)
		{
			return	  (kOpIlhu << 21)
				| (((U32)imm & 0xffff) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeIl(U32 resultReg, I32 imm)
		{
			return	  (kOpIl << 21)
				| (((U32)imm & 0xffff) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeIla(U32 resultReg, U32 imm)
		{
			return	  (kOpIl << 21)
				| (((U32)imm & 0x3ffff) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeIohl(U32 resultReg, I32 imm)
		{
			return	  (kOpIohl << 21)
				| (((U32)imm & 0xffff) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeAh(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpAh << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeAhi(U32 resultReg, I32 imm, U32 aReg)
		{
			return	  (kOpAhi << 21)
				| (((U32)imm >> 14) & 0x3FF)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeA(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpA << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeAi(U32 resultReg, I32 imm, U32 aReg)
		{
			return	  (kOpAi << 21)
				| (((U32)imm >> 14) & 0x3FF)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		/*
		inline U32 MakeAsc(U32 resultReg, U32 aReg, U32 bReg)
		{
		return	  (kOpAsc << 21)
		| ((bReg & 0x7f) << 14)
		| ((aReg & 0x7f) << 7)
		| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeAci(U32 resultReg, U32 aReg, U32 bReg)
		{
		return	  (kOpAci << 21)
		| ((bReg & 0x7f) << 14)
		| ((aReg & 0x7f) << 7)
		| ((resultReg & 0x7f) << 0);
		}
		*/

		inline U32 MakeSfh(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpSfh << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeSfhi(U32 resultReg, I32 imm, U32 aReg)
		{
			return	  (kOpSfhi << 21)
				| (((U32)imm >> 14) & 0x3FF)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeSf(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpSf << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeSfi(U32 resultReg, I32 imm, U32 aReg)
		{
			return	  (kOpSfi << 21)
				| (((U32)imm >> 14) & 0x3FF)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		/*
		inline U32 MakeSsb(U32 resultReg, U32 aReg, U32 bReg)
		{
		return	  (kOpSsb << 21)
		| ((bReg & 0x7f) << 14)
		| ((aReg & 0x7f) << 7)
		| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeSbi(U32 resultReg, U32 aReg, U32 bReg)
		{
		return	  (kOpSbi << 21)
		| ((bReg & 0x7f) << 14)
		| ((aReg & 0x7f) << 7)
		| ((resultReg & 0x7f) << 0);
		}
		*/

		inline U32 MakeMpy(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpMpy << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeMpyu(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpMpyu << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeMpyi(U32 resultReg, I32 imm, U32 aReg)
		{
			return	  (kOpMpyi << 21)
				| (((U32)imm >> 14) & 0x3FF)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeMpyui(U32 resultReg, I32 imm, U32 aReg)
		{
			return	  (kOpMpyui << 21)
				| (((U32)imm >> 14) & 0x3FF)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeMpya(U32 resultReg, U32 aReg, U32 bReg, U32 cReg)
		{
			return	  (kOpMpya << 21)
				| ((cReg & 0x7f) << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeMpyh(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpMpyh << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeMpys(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpMpys << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeMpyhh(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpMpyhh << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeMpyhhu(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpMpyhhu << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeMpyhha(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpMpyhha << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeMpyhhau(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpMpyhhau << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeClz(U32 resultReg, U32 aReg)
		{
			return	  (kOpClz << 21)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeCntb(U32 resultReg, U32 aReg)
		{
			return	  (kOpCntb << 21)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeFsmbi(U32 resultReg, U32 imm)
		{
			return	  (kOpFsmbi << 21)
				| (((U32)imm & 0xffff) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeFsmb(U32 resultReg, U32 aReg)
		{
			return	  (kOpFsmb << 21)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeFsmh(U32 resultReg, U32 aReg)
		{
			return	  (kOpFsmh << 21)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeFsm(U32 resultReg, U32 aReg)
		{
			return	  (kOpFsm << 21)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeGbb(U32 resultReg, U32 aReg)
		{
			return	  (kOpGbb << 21)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeGbh(U32 resultReg, U32 aReg)
		{
			return	  (kOpGbh << 21)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeGb(U32 resultReg, U32 aReg)
		{
			return	  (kOpGb << 21)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeAvgb(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpAvgb << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeAbsdb(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpAbsdb << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeSumb(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpSumb << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeXsbh(U32 resultReg, U32 aReg)
		{
			return	  (kOpXsbh << 21)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeXshw(U32 resultReg, U32 aReg)
		{
			return	  (kOpXshw << 21)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeXswd(U32 resultReg, U32 aReg)
		{
			return	  (kOpXswd << 21)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeAnd(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpAnd << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeAndc(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpAndc << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeAndbi(U32 resultReg, U32 aReg, I32 imm)
		{
			return	  (kOpAndbi << 21)
				| (((U32)imm & 0x3ff) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeAndi(U32 resultReg, U32 aReg, I32 imm)
		{
			return	  (kOpAndi << 21)
				| (((U32)imm & 0x3ff) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeAndhi(U32 resultReg, U32 aReg, I32 imm)
		{
			return	  (kOpAndhi << 21)
				| (((U32)imm & 0x3ff) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeOr(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpOr << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeOrc(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpOrc << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeOrbi(U32 resultReg, U32 aReg, I32 imm)
		{
			return	  (kOpOrbi << 21)
				| (((U32)imm & 0x3ff) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeOri(U32 resultReg, U32 aReg, I32 imm)
		{
			return	  (kOpOri << 21)
				| (((U32)imm & 0x3ff) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeOrhi(U32 resultReg, U32 aReg, I32 imm)
		{
			return	  (kOpOrhi << 21)
				| (((U32)imm & 0x3ff) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeXor(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpXor << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeXorbi(U32 resultReg, U32 aReg, I32 imm)
		{
			return	  (kOpXorbi << 21)
				| (((U32)imm & 0x3ff) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeXori(U32 resultReg, U32 aReg, I32 imm)
		{
			return	  (kOpXori << 21)
				| (((U32)imm & 0x3ff) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeXorhi(U32 resultReg, U32 aReg, I32 imm)
		{
			return	  (kOpXorhi << 21)
				| (((U32)imm & 0x3ff) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeNand(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpNand << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeNor(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpNor << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeEqv(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpEqv << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeSelb(U32 resultReg, U32 aReg, U32 bReg, U32 cReg)
		{
			return	  (kOpSelb << 21)
				| ((cReg & 0x7f) << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeShufb(U32 resultReg, U32 aReg, U32 bReg, U32 cReg)
		{
			return	  (kOpShufb << 21)
				| ((cReg & 0x7f) << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeShlh(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpShlh << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeShlhi(U32 resultReg, U32 aReg, I32 imm)
		{
			return	  (kOpShlhi << 21)
				| (((U32)imm & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeShl(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpShl << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeShli(U32 resultReg, U32 aReg, I32 imm)
		{
			return	  (kOpShli << 21)
				| (((U32)imm & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeShlqbi(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpShlqbi << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeShlqbii(U32 resultReg, U32 aReg, I32 imm)
		{
			return	  (kOpShlqbii << 21)
				| (((U32)imm & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeShlqby(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpShlqby << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeShlqbyi(U32 resultReg, U32 aReg, I32 imm)
		{
			return	  (kOpShlqbyi << 21)
				| (((U32)imm & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeShlqbybi(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpShlqby << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeRoth(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpRoth << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeRothi(U32 resultReg, U32 aReg, I32 imm)
		{
			return	  (kOpRothi << 21)
				| (((U32)imm & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeRot(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpRot << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeRoti(U32 resultReg, U32 aReg, I32 imm)
		{
			return	  (kOpRoti << 21)
				| (((U32)imm & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeRotqby(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpRotqby << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeRotqbyi(U32 resultReg, U32 aReg, I32 imm)
		{
			return	  (kOpRotqbyi << 21)
				| (((U32)imm & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeRotqbybi(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpRotqby << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeRotqbi(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpRotqbi << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeRotqbii(U32 resultReg, U32 aReg, I32 imm)
		{
			return	  (kOpRotqbii << 21)
				| (((U32)imm & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeRothm(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpRothm << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeRothmi(U32 resultReg, U32 aReg, I32 imm)
		{
			return	  (kOpRothmi << 21)
				| (((U32)imm & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeRotm(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpRothm << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeRotmi(U32 resultReg, U32 aReg, I32 imm)
		{
			return	  (kOpRothmi << 21)
				| (((U32)imm & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeRotqmby(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpRotqmby << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeRotqmbyi(U32 resultReg, U32 aReg, I32 imm)
		{
			return	  (kOpRotqmbyi << 21)
				| (((U32)imm & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeRotqmbi(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpRotqmbi << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeRotqmbii(U32 resultReg, U32 aReg, I32 imm)
		{
			return	  (kOpRotqmbii << 21)
				| (((U32)imm & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeRotqmbybi(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpRotqmbybi << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeRotmah(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpRotmah << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeRotmahi(U32 resultReg, U32 aReg, I32 imm)
		{
			return	  (kOpRotmahi << 21)
				| (((U32)imm & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeRotma(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpRotmah << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeRotmai(U32 resultReg, U32 aReg, I32 imm)
		{
			return	  (kOpRotmahi << 21)
				| (((U32)imm & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeHeq(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpHeq << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeHeqi(U32 resultReg, U32 aReg, I32 imm)
		{
			return	  (kOpHeqi << 21)
				| (((U32)imm & 0x3ff) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeHgt(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpHgt << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeHgti(U32 resultReg, U32 aReg, I32 imm)
		{
			return	  (kOpHgti << 21)
				| (((U32)imm & 0x3ff) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeHlgt(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpHlgt << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeHlgti(U32 resultReg, U32 aReg, I32 imm)
		{
			return	  (kOpHlgti << 21)
				| (((U32)imm & 0x3ff) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeCeqb(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpCeqb << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeCeqbi(U32 resultReg, U32 aReg, I32 imm)
		{
			return	  (kOpCeqbi << 21)
				| (((U32)imm & 0x3ff) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeCeqh(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpCeqh << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeCeqhi(U32 resultReg, U32 aReg, I32 imm)
		{
			return	  (kOpCeqhi << 21)
				| (((U32)imm & 0x3ff) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeCeq(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpCeq << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeCeqi(U32 resultReg, U32 aReg, I32 imm)
		{
			return	  (kOpCeqi << 21)
				| (((U32)imm & 0x3ff) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeCgtb(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpCgtb << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeCgtbi(U32 resultReg, U32 aReg, I32 imm)
		{
			return	  (kOpCgtbi << 21)
				| (((U32)imm & 0x3ff) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeCgth(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpCgth << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeCgthi(U32 resultReg, U32 aReg, I32 imm)
		{
			return	  (kOpCgthi << 21)
				| (((U32)imm & 0x3ff) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeCgt(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpCgt << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeCgti(U32 resultReg, U32 aReg, I32 imm)
		{
			return	  (kOpCgti << 21)
				| (((U32)imm & 0x3ff) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeClgtb(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpClgtb << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeClgtbi(U32 resultReg, U32 aReg, I32 imm)
		{
			return	  (kOpClgtbi << 21)
				| (((U32)imm & 0x3ff) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeClgth(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpClgth << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeClgthi(U32 resultReg, U32 aReg, I32 imm)
		{
			return	  (kOpClgthi << 21)
				| (((U32)imm & 0x3ff) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeClgt(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpClgt << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeClgti(U32 resultReg, U32 aReg, I32 imm)
		{
			return	  (kOpClgti << 21)
				| (((U32)imm & 0x3ff) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeBr(I32 imm)
		{
			return	  (kOpBr << 21)
				| ((((U32)imm >> 2) & 0xffff) << 7);
		}

		inline U32 MakeBra(I32 imm)
		{
			return	  (kOpBra << 21)
				| ((((U32)imm >> 2) & 0xffff) << 7);
		}

		inline U32 MakeBrsl(U32 resultReg, I32 imm)
		{
			return	  (kOpBrsl << 21)
				| ((((U32)imm >> 2) & 0xffff) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeBrasl(U32 resultReg, I32 imm)
		{
			return	  (kOpBrasl << 21)
				| ((((U32)imm >> 2) & 0xffff) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeBi(U32 aReg)
		{
			return	  (kOpBi << 21)
				| ((aReg & 0x7f) << 7);
		}

		inline U32 MakeBisled(U32 resultReg, U32 aReg)
		{
			return	  (kOpBisled << 21)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeBisl(U32 resultReg, U32 aReg)
		{
			return	  (kOpBisl << 21)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeBrnz(U32 resultReg, I32 imm)
		{
			return	  (kOpBrnz << 21)
				| ((((U32)imm >> 2) & 0xffff) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeBrz(U32 resultReg, I32 imm)
		{
			return	  (kOpBrz << 21)
				| ((((U32)imm >> 2) & 0xffff) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeBrhnz(U32 resultReg, I32 imm)
		{
			return	  (kOpBrhnz << 21)
				| ((((U32)imm >> 2) & 0xffff) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeBrhz(U32 resultReg, I32 imm)
		{
			return	  (kOpBrhz << 21)
				| ((((U32)imm >> 2) & 0xffff) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeBiz(U32 resultReg, U32 aReg)
		{
			return	  (kOpBiz << 21)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeBinz(U32 resultReg, U32 aReg)
		{
			return	  (kOpBinz << 21)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeBihz(U32 resultReg, U32 aReg)
		{
			return	  (kOpBihz << 21)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeBihnz(U32 resultReg, U32 aReg)
		{
			return	  (kOpBihnz << 21)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeHbr(U32 imm, U32 aReg)
		{
			return	  (kOpHbr << 21)
				| (((U32)imm >> 2) & 0x7f) | ((((U32)imm >> 2) & 0x180) << 7)
				| ((aReg & 0x7f) << 7);
		}

		inline U32 MakeHbra(U32 imm1, U32 imm2)
		{
			return	  (kOpHbra << 21)
				| (((U32)imm1 >> 2) & 0x7f) | ((((U32)imm1 >> 2) & 0x180) << 16)
				| ((((U32)imm2 >> 2) & 0xffff) << 7);
		}

		inline U32 MakeHbrr(U32 imm1, U32 imm2)
		{
			return	  (kOpHbrr << 21)
				| (((U32)imm1 >> 2) & 0x7f) | ((((U32)imm1 >> 2) & 0x180) << 16)
				| ((((U32)imm2 >> 2) & 0xffff) << 7);
		}

		inline U32 MakeFa(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpFa << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeDfa(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpDfa << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeFs(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpFs << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeDfs(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpDfs << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeFm(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpFm << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeDfm(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpDfm << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeFma(U32 resultReg, U32 aReg, U32 bReg, U32 cReg)
		{
			return	  (kOpFma << 21)
				| ((cReg & 0x7f) << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeDfma(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpDfma << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeFnms(U32 resultReg, U32 aReg, U32 bReg, U32 cReg)
		{
			return	  (kOpFnms << 21)
				| ((cReg & 0x7f) << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeDfnms(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpDfnms << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeDfnma(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpDfnma << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeFms(U32 resultReg, U32 aReg, U32 bReg, U32 cReg)
		{
			return	  (kOpFms << 21)
				| ((cReg & 0x7f) << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeDfms(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpDfms << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeFrest(U32 resultReg, U32 aReg)
		{
			return	  (kOpFrest << 21)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeFrsqest(U32 resultReg, U32 aReg)
		{
			return	  (kOpFrsqest << 21)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeFi(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpFi << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeCsflt(U32 resultReg, U32 aReg, U32 imm)
		{
			return	  (kOpCsflt << 21)
				| ((imm & 0xff) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeCflts(U32 resultReg, U32 aReg, U32 imm)
		{
			return	  (kOpCflts << 21)
				| ((imm & 0xff) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeCuflt(U32 resultReg, U32 aReg, U32 imm)
		{
			return	  (kOpCuflt << 21)
				| ((imm & 0xff) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeCfltu(U32 resultReg, U32 aReg, U32 imm)
		{
			return	  (kOpCfltu << 21)
				| ((imm & 0xff) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeFrds(U32 resultReg, U32 aReg)
		{
			return	  (kOpFrds << 21)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeFesd(U32 resultReg, U32 aReg)
		{
			return	  (kOpFesd << 21)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeFceq(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpFceq << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeFcmeq(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpFcmeq << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeFcgt(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpFcgt << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeFcmgt(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpFcmgt << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeStop()
		{
			return (kOpStop << 21);
		}

		inline U32 MakeStopd()
		{
			return (kOpStopd << 21);
		}

		inline U32 MakeNop()
		{
			return (kOpNop << 21);
		}

		inline U32 MakeLnop()
		{
			return (kOpLnop << 21);
		}

		inline U32 MakeSync()
		{
			return (kOpSync << 21);
		}

		inline U32 MakeDsync()
		{
			return (kOpDsync << 21);
		}

		inline U32 MakeMfspr(U32 resultReg, U32 aReg)
		{
			return	  (kOpFcmgt << 21)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeFscrrd(U32 resultReg)
		{
			return	  (kOpFscrrd << 21)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeFscrwr(U32 aReg)
		{
			return	  (kOpFscrwr << 21)
				| ((aReg & 0x7f) << 7);
		}

		inline U32 MakeMtspr(U32 resultReg, U32 aReg)
		{
			return	  (kOpMtspr << 21)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeRdch(U32 resultReg, U32 aReg)
		{
			return	  (kOpRdch << 21)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeRchcnt(U32 resultReg, U32 aReg)
		{
			return	  (kOpRchcnt << 21)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeWrch(U32 aReg, U32 resultReg)
		{
			return	  (kOpWrch << 21)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeIret()
		{
			return (kOpIret << 21);
		}

		inline U32 MakeAddx(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpAddx << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeCg(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpCg << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeCgx(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpCgx << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeSfx(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpSfx << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeBg(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpBg << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}

		inline U32 MakeBgx(U32 resultReg, U32 aReg, U32 bReg)
		{
			return	  (kOpBgx << 21)
				| ((bReg & 0x7f) << 14)
				| ((aReg & 0x7f) << 7)
				| ((resultReg & 0x7f) << 0);
		}
	}
}

#endif // ICESPUCODEGENERATION_H
