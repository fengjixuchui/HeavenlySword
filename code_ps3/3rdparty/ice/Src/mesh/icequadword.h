/*
 * Copyright (c) 2005 Naughty Dog, Inc. 
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#ifndef ICEQUADWORD_H
#define ICEQUADWORD_H

namespace Ice
{
	namespace MeshProc
	{
		class IceQuadWord
		{
		public:
			union {
				struct
				{
					F32 A,B,C,D;
				} f32;
				struct
				{
					F64 A,B;
				} f64;
				struct
				{
					U8 A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P;
				} u8;
				struct
				{
					U16 A,B,C,D,E,F,G,H;
				} u16;
				struct
				{
					U32 A,B,C,D;
				} u32;
				struct
				{
					U64 A,B;
				} u64;
				struct
				{
					I8 A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P;
				} i8;
				struct
				{
					I16 A,B,C,D,E,F,G,H;
				} i16;
				struct
				{
					I32 A,B,C,D;
				} i32;
				struct
				{
					I64 A,B;
				} i64;

			} m_data;

			bool operator==(const IceQuadWord &rhs)
			{
				return (m_data.f32.A == rhs.m_data.f32.A &&
					m_data.f32.B == rhs.m_data.f32.B &&
					m_data.f32.C == rhs.m_data.f32.C &&
					m_data.f32.D == rhs.m_data.f32.D);
			}
			bool operator!=(const IceQuadWord &rhs)
			{
				return !(*this == rhs);
			}

			IceQuadWord(void)
			{

			}

			IceQuadWord(F32 A, F32 B, F32 C, F32 D)
			{
				m_data.f32.A = A;
				m_data.f32.B = B;
				m_data.f32.C = C;
				m_data.f32.D = D;
			}

			IceQuadWord(F64 A, F64 B)
			{
				m_data.f64.A = A;
				m_data.f64.B = B;
			}

			IceQuadWord(U8 A, U8 B, U8 C, U8 D, U8 E, U8 F, U8 G, U8 H, U8 I, U8 J,
				U8 K, U8 L, U8 M, U8 N, U8 O, U8 P)
			{
				m_data.u8.A = A;
				m_data.u8.B = B;
				m_data.u8.C = C;
				m_data.u8.D = D;
				m_data.u8.E = E;
				m_data.u8.F = F;
				m_data.u8.G = G;
				m_data.u8.H = H;
				m_data.u8.I = I;
				m_data.u8.J = J;
				m_data.u8.K = K;
				m_data.u8.L = L;
				m_data.u8.M = M;
				m_data.u8.N = N;
				m_data.u8.O = O;
				m_data.u8.P = P;
			}

			IceQuadWord(U16 A, U16 B, U16 C, U16 D, U16 E, U16 F, U16 G, U16 H)
			{
				m_data.u16.A = A;
				m_data.u16.B = B;
				m_data.u16.C = C;
				m_data.u16.D = D;
				m_data.u16.E = E;
				m_data.u16.F = F;
				m_data.u16.G = G;
				m_data.u16.H = H;
			}

			IceQuadWord(U32 A, U32 B, U32 C, U32 D)
			{
				m_data.u32.A = A;
				m_data.u32.B = B;
				m_data.u32.C = C;
				m_data.u32.D = D;
			}

			IceQuadWord(U64 A, U64 B)
			{
				m_data.u64.A = A;
				m_data.u64.B = B;
			}

			IceQuadWord(I8 A, I8 B, I8 C, I8 D, I8 E, I8 F, I8 G, I8 H, I8 I, I8 J,
				I8 K, I8 L, I8 M, I8 N, I8 O, I8 P)
			{
				m_data.i8.A = A;
				m_data.i8.B = B;
				m_data.i8.C = C;
				m_data.i8.D = D;
				m_data.i8.E = E;
				m_data.i8.F = F;
				m_data.i8.G = G;
				m_data.i8.H = H;
				m_data.i8.I = I;
				m_data.i8.J = J;
				m_data.i8.K = K;
				m_data.i8.L = L;
				m_data.i8.M = M;
				m_data.i8.N = N;
				m_data.i8.O = O;
				m_data.i8.P = P;
			}

			IceQuadWord(I16 A, I16 B, I16 C, I16 D, I16 E, I16 F, I16 G, I16 H)
			{
				m_data.i16.A = A;
				m_data.i16.B = B;
				m_data.i16.C = C;
				m_data.i16.D = D;
				m_data.i16.E = E;
				m_data.i16.F = F;
				m_data.i16.G = G;
				m_data.i16.H = H;
			}

			IceQuadWord(I32 A, I32 B, I32 C, I32 D)
			{
				m_data.i32.A = A;
				m_data.i32.B = B;
				m_data.i32.C = C;
				m_data.i32.D = D;
			}

			IceQuadWord(I64 A, I64 B)
			{
				m_data.i64.A = A;
				m_data.i64.B = B;
			}

			IceQuadWord(F32 A)
			{
				m_data.f32.A = A;
				m_data.f32.B = A;
				m_data.f32.C = A;
				m_data.f32.D = A;
			}

			IceQuadWord(F64 A)
			{
				m_data.f64.A = A;
				m_data.f64.B = A;
			}

			IceQuadWord(U8 A)
			{
				m_data.u8.A = A;
				m_data.u8.B = A;
				m_data.u8.C = A;
				m_data.u8.D = A;
				m_data.u8.E = A;
				m_data.u8.F = A;
				m_data.u8.G = A;
				m_data.u8.H = A;
				m_data.u8.I = A;
				m_data.u8.J = A;
				m_data.u8.K = A;
				m_data.u8.L = A;
				m_data.u8.M = A;
				m_data.u8.N = A;
				m_data.u8.O = A;
				m_data.u8.P = A;
			}

			IceQuadWord(U16 A)
			{
				m_data.u16.A = A;
				m_data.u16.B = A;
				m_data.u16.C = A;
				m_data.u16.D = A;
				m_data.u16.E = A;
				m_data.u16.F = A;
				m_data.u16.G = A;
				m_data.u16.H = A;
			}

			IceQuadWord(U32 A)
			{
				m_data.u32.A = A;
				m_data.u32.B = A;
				m_data.u32.C = A;
				m_data.u32.D = A;
			}

			IceQuadWord(U64 A)
			{
				m_data.u64.A = A;
				m_data.u64.B = A;
			}

			IceQuadWord(I8 A)
			{
				m_data.i8.A = A;
				m_data.i8.B = A;
				m_data.i8.C = A;
				m_data.i8.D = A;
				m_data.i8.E = A;
				m_data.i8.F = A;
				m_data.i8.G = A;
				m_data.i8.H = A;
				m_data.i8.I = A;
				m_data.i8.J = A;
				m_data.i8.K = A;
				m_data.i8.L = A;
				m_data.i8.M = A;
				m_data.i8.N = A;
				m_data.i8.O = A;
				m_data.i8.P = A;
			}

			IceQuadWord(I16 A)
			{
				m_data.i16.A = A;
				m_data.i16.B = A;
				m_data.i16.C = A;
				m_data.i16.D = A;
				m_data.i16.E = A;
				m_data.i16.F = A;
				m_data.i16.G = A;
				m_data.i16.H = A;
			}

			IceQuadWord(I32 A)
			{
				m_data.i32.A = A;
				m_data.i32.B = A;
				m_data.i32.C = A;
				m_data.i32.D = A;
			}

			IceQuadWord(I64 A)
			{
				m_data.i64.A = A;
				m_data.i64.B = A;
			}

		private:
		};
	}
}

#endif // ICEQUADWORD_H
