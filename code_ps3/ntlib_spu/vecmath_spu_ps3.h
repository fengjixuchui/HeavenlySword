//----------------------------------------------------------------------------------------
//! 
//! \filename ntlib_spu\vecmath_spu_ps3.h
//!
//!	SPU versions of the PPU/PC CPoint, CDirection, CVector and CQuat classes.
//!	NOTE: Some functions are not present.
//!
//! SUPER C:
//! This is a 'super' C implementation, it should be 
//! treated as C code using C++ style syntax and not 
//! true C++ code. In particular, ctors must not be 
//! required, nor dtors (they can exist for convience but
//! must not be required). No vtables and types must be PODs
//! \see http://kong:8080/confluence/display/CH/SuperC
//!
//----------------------------------------------------------------------------------------

#ifndef	VECMATH_SPU_PS3_H_
#define	VECMATH_SPU_PS3_H_

#ifndef __SPU__
#	error This file can only be included in an SPU project.
#endif // !__SPU__

#include <math.h>
#include <bits/sce_math.h>
#include "ntlib_spu/basetypes_spu.h"



#include "ntlib_spu/debug_spu.h"




// altivec/vmx register type definition..
typedef	vector float v128;
typedef const v128& v128_arg;

enum	CLEAR_CONSTRUCT_MODE
{
	CONSTRUCT_CLEAR,
};

enum	IDENTITY_CONSTRUCT_MODE
{
	CONSTRUCT_IDENTITY,
};

class CPoint;
class CDirection;
class CVector;
class CQuat;
class CMatrix;




///////////////////////////////////////////////////
// VMX -> SPU
inline vec_float4 vec_mergel(vec_float4 a, vec_float4 b)
{
	return (spu_shuffle(a, b, (vec_uchar16){ 8,  9, 10, 11, 24, 25, 26, 27, 12, 13, 14, 15, 28, 29, 30, 31}));
}
inline vec_float4 vec_mergeh(vec_float4 a, vec_float4 b)
{
	return (spu_shuffle(a, b, (vec_uchar16){0, 1, 2, 3, 16, 17, 18, 19, 4, 5, 6, 7, 20, 21, 22, 23}));
}
#define vec_splat(_a, _b)	spu_splats(spu_extract(_a, _b))
#define vec_madd(_a, _b, _c)	spu_madd(_a, _b, _c)
// End VMX -> SPU
///////////////////////////////////////////////////


const v128 g_float4_zeroInW = (v128){ 1.0f, 1.0f, 1.0f, 0.0f };
const v128 g_float4_oneInW = (v128){ 0.0f, 0.0f, 0.0f, 1.0f };
const v128 g_float4_allzero = (v128){ 0.0f, 0.0f, 0.0f, 0.0f };


/***************************************************************************************************
*
*	Some useful generic SPU-SIMD functions.
*
***************************************************************************************************/
namespace Intrinsics
{
	v128	SPU_DP3		( v128 a, v128 b );
	v128	SPU_DP4		( v128 a, v128 b );
	v128	SPU_Negate	( v128 a );
	v128	SPU_Sin		( v128 a );
	v128	SPU_Cos		( v128 a );
	v128	SPU_ACos	( v128 a );
	v128	SPU_Div		( v128 a, v128 b );
	v128	SPU_Sqrt	( v128 a );
	v128	SPU_RSqrt	( v128 a );
	v128	SPU_Abs		( v128 a );
	v128	SPU_Min		( v128 a, v128 b );
	v128	SPU_Max		( v128 a, v128 b );
	void	SPU_SinCos	( v128 a, v128 *s, v128 *c );
}

//#define ENABLE_SPU_VECTORMATHS_UNITTESTS
#ifdef ENABLE_SPU_VECTORMATHS_UNITTESTS
namespace VecMaths_Private
{
	void	UnitTest	();
}
#endif // ENABLE_SPU_VECTORMATHS_UNITTESTS

/***************************************************************************************************
*
*	Constants - mostly for shuffle intrinsics.
*	[x y z w] [a b c d] are the vectors.
*
***************************************************************************************************/
#define VECTORMATH_SHUF_X 0x00010203
#define VECTORMATH_SHUF_Y 0x04050607
#define VECTORMATH_SHUF_Z 0x08090a0b
#define VECTORMATH_SHUF_W 0x0c0d0e0f
#define VECTORMATH_SHUF_A 0x10111213
#define VECTORMATH_SHUF_B 0x14151617
#define VECTORMATH_SHUF_C 0x18191a1b
#define VECTORMATH_SHUF_D 0x1c1d1e1f
#define VECTORMATH_SHUF_0 0x80808080
#define VECTORMATH_MAKE_SHUFFLE( a, b, c, d ) ((vec_uchar16)(vec_uint4){ (a), (b), (c), (b) })

#define VECTORMATH_SHUF_XYZA (vec_uchar16)(vec_uint4){ VECTORMATH_SHUF_X, VECTORMATH_SHUF_Y, VECTORMATH_SHUF_Z, VECTORMATH_SHUF_A }
#define VECTORMATH_SHUF_ZXYW (vec_uchar16)(vec_uint4){ VECTORMATH_SHUF_Z, VECTORMATH_SHUF_X, VECTORMATH_SHUF_Y, VECTORMATH_SHUF_W }
#define VECTORMATH_SHUF_YZXW (vec_uchar16)(vec_uint4){ VECTORMATH_SHUF_Y, VECTORMATH_SHUF_Z, VECTORMATH_SHUF_X, VECTORMATH_SHUF_W }
#define VECTORMATH_SHUF_WABC (vec_uchar16)(vec_uint4){ VECTORMATH_SHUF_W, VECTORMATH_SHUF_A, VECTORMATH_SHUF_B, VECTORMATH_SHUF_C }
#define VECTORMATH_SHUF_ZWAB (vec_uchar16)(vec_uint4){ VECTORMATH_SHUF_Z, VECTORMATH_SHUF_W, VECTORMATH_SHUF_A, VECTORMATH_SHUF_B }
#define VECTORMATH_SHUF_XYZA (vec_uchar16)(vec_uint4){ VECTORMATH_SHUF_X, VECTORMATH_SHUF_Y, VECTORMATH_SHUF_Z, VECTORMATH_SHUF_A }
#define VECTORMATH_SHUF_YZAB (vec_uchar16)(vec_uint4){ VECTORMATH_SHUF_Y, VECTORMATH_SHUF_Z, VECTORMATH_SHUF_A, VECTORMATH_SHUF_B }
#define VECTORMATH_SHUF_ZABC (vec_uchar16)(vec_uint4){ VECTORMATH_SHUF_Z, VECTORMATH_SHUF_A, VECTORMATH_SHUF_B, VECTORMATH_SHUF_C }
#define VECTORMATH_SHUF_WZYX (vec_uchar16)(vec_uint4){ VECTORMATH_SHUF_W, VECTORMATH_SHUF_Z, VECTORMATH_SHUF_Y, VECTORMATH_SHUF_X }
#define VECTORMATH_SHUF_ZWXY (vec_uchar16)(vec_uint4){ VECTORMATH_SHUF_Z, VECTORMATH_SHUF_W, VECTORMATH_SHUF_X, VECTORMATH_SHUF_Y }
#define VECTORMATH_SHUF_YXWZ (vec_uchar16)(vec_uint4){ VECTORMATH_SHUF_Y, VECTORMATH_SHUF_X, VECTORMATH_SHUF_W, VECTORMATH_SHUF_Z }

#define VECTORMATH_SHUF_XAYB ((vec_uchar16)(vec_uint4){ VECTORMATH_SHUF_X, VECTORMATH_SHUF_A, VECTORMATH_SHUF_Y, VECTORMATH_SHUF_B })
#define VECTORMATH_SHUF_ZCWD ((vec_uchar16)(vec_uint4){ VECTORMATH_SHUF_Z, VECTORMATH_SHUF_C, VECTORMATH_SHUF_W, VECTORMATH_SHUF_D })
#define VECTORMATH_SHUF_ZBW0 ((vec_uchar16)(vec_uint4){ VECTORMATH_SHUF_Z, VECTORMATH_SHUF_B, VECTORMATH_SHUF_W, VECTORMATH_SHUF_0 })
#define VECTORMATH_SHUF_XCY0 ((vec_uchar16)(vec_uint4){ VECTORMATH_SHUF_X, VECTORMATH_SHUF_C, VECTORMATH_SHUF_Y, VECTORMATH_SHUF_0 })
#define VECTORMATH_SHUF_ZDW0 ((vec_uchar16)(vec_uint4){ VECTORMATH_SHUF_Z, VECTORMATH_SHUF_D, VECTORMATH_SHUF_W, VECTORMATH_SHUF_0 })
#define VECTORMATH_SHUF_XAZC ((vec_uchar16)(vec_uint4){ VECTORMATH_SHUF_X, VECTORMATH_SHUF_A, VECTORMATH_SHUF_Z, VECTORMATH_SHUF_C })
#define VECTORMATH_SHUF_ZDXB ((vec_uchar16)(vec_uint4){ VECTORMATH_SHUF_Z, VECTORMATH_SHUF_D, VECTORMATH_SHUF_X, VECTORMATH_SHUF_B })
#define VECTORMATH_SHUF_YBWD ((vec_uchar16)(vec_uint4){ VECTORMATH_SHUF_Y, VECTORMATH_SHUF_B, VECTORMATH_SHUF_W, VECTORMATH_SHUF_D })
#define VECTORMATH_SHUF_XDZB ((vec_uchar16)(vec_uint4){ VECTORMATH_SHUF_X, VECTORMATH_SHUF_D, VECTORMATH_SHUF_Z, VECTORMATH_SHUF_B })
#define VECTORMATH_SHUF_YAWC ((vec_uchar16)(vec_uint4){ VECTORMATH_SHUF_Y, VECTORMATH_SHUF_A, VECTORMATH_SHUF_W, VECTORMATH_SHUF_C })
#define VECTORMATH_SHUF_ZBXD ((vec_uchar16)(vec_uint4){ VECTORMATH_SHUF_Z, VECTORMATH_SHUF_B, VECTORMATH_SHUF_X, VECTORMATH_SHUF_D })
#define VECTORMATH_SHUF_XYCD ((vec_uchar16)(vec_uint4){ VECTORMATH_SHUF_X, VECTORMATH_SHUF_Y, VECTORMATH_SHUF_C, VECTORMATH_SHUF_D })
#define VECTORMATH_SHUF_ZYXW ((vec_uchar16)(vec_uint4){ VECTORMATH_SHUF_Z, VECTORMATH_SHUF_Y, VECTORMATH_SHUF_X, VECTORMATH_SHUF_W })



#define VECTORMATH_UNIT_1000 (vec_float4){ 1.0f, 0.0f, 0.0f, 0.0f }
#define VECTORMATH_UNIT_0100 (vec_float4){ 0.0f, 1.0f, 0.0f, 0.0f }
#define VECTORMATH_UNIT_0010 (vec_float4){ 0.0f, 0.0f, 1.0f, 0.0f }
#define VECTORMATH_UNIT_0001 (vec_float4){ 0.0f, 0.0f, 0.0f, 1.0f }
#define VECTORMATH_SLERP_TOL 0.999f

#define SCALAR_PI 3.14159
#define SCALAR_TWO_PI SCALAR_PI * 2.0f
#define SCALAR_HALF_PI SCALAR_PI * 0.5f

#define VECTORMATH_PI (vec_float4){ SCALAR_PI, SCALAR_PI, SCALAR_PI, SCALAR_PI }
#define VECTORMATH_TWO_PI (vec_float4){ SCALAR_TWO_PI, SCALAR_TWO_PI, SCALAR_TWO_PI, SCALAR_TWO_PI }
#define VECTORMATH_HALF_PI (vec_float4){ SCALAR_HALF_PI, SCALAR_HALF_PI, SCALAR_HALF_PI, SCALAR_HALF_PI }


/***************************************************************************************************
*
*	CLASS			CPoint
*
*	DESCRIPTION		Defines a point in R^3.
*
*	NOTES			This object occupies 16 Bytes of aligned storage.
*
***************************************************************************************************/
class CPoint
{
	public:
		//
		//	Accessors.
		//
		v128 &		Quadword		()					{ return m_Vector; }
		v128		QuadwordValue	()			const	{ return m_Vector; }

		float		X				()			const	{ return spu_extract( m_Vector, 0 ); }
		float		Y				()			const	{ return spu_extract( m_Vector, 1 ); }
		float		Z				()			const	{ return spu_extract( m_Vector, 2 ); }

		float		operator []		( int i )	const	{ return spu_extract( m_Vector, i ); }

	public:
		//
		//	Basic functionality.
		//
		float		Length			() 								const;
		float		LengthSquared	() 								const;
		float		Dot				( const CPoint &point )			const;
		float		Dot				( const CDirection &direction )	const;
		CPoint		Abs				()								const;
		CPoint		Min				( const CPoint &point ) 		const;
		CPoint		Max				( const CPoint &point ) 		const;

		static CPoint 	Lerp		( const CPoint &a, const CPoint &b, float t ) { return a + ( b - a ) * t; }

	public:
		//
		//	Clear the point to (0,0,0).
		//
		void		Clear	()	{ m_Vector = spu_splats( 0.0f ); }

	public:
		//
		//	Operator overloads.
		//
		inline CPoint 		operator + 		( const CDirection &rhs ) 	const;
		inline CPoint 		operator + 		( const CPoint &rhs ) 		const;
		inline CPoint 		operator - 		( const CDirection &rhs ) 	const;
		inline CPoint 		operator - 		( const CPoint &rhs ) 		const;
		inline CDirection	operator ^		( const CPoint &rhs )		const;
		inline CPoint 		operator * 		( const CDirection &rhs ) 	const;
		inline CPoint 		operator * 		( const CPoint &rhs ) 		const;
		inline CPoint 		operator * 		( float rhs ) 				const	{ return CPoint( spu_mul( m_Vector, spu_splats( rhs ) ) ); }
		inline CPoint 		operator / 		( float rhs ) 				const	{ return CPoint( Intrinsics::SPU_Div( m_Vector, spu_splats( rhs ) ) ); }

		inline CPoint &		operator += 	( const CDirection &rhs );
		inline CPoint &		operator += 	( const CPoint &rhs );
		inline CPoint &		operator -= 	( const CDirection &rhs );
		inline CPoint &		operator -= 	( const CPoint &rhs );
		inline CPoint &		operator *= 	( const CDirection &rhs );
		inline CPoint &		operator *= 	( const CPoint &rhs )				{ m_Vector = spu_mul( m_Vector, rhs.m_Vector ); return *this; }
		inline CPoint &		operator *=		( float rhs )						{ m_Vector = spu_mul( m_Vector, spu_splats( rhs ) ); return *this; }
		inline CPoint &		operator /=		( float rhs )						{ m_Vector = Intrinsics::SPU_Div( m_Vector, spu_splats( rhs ) ); return *this; }

		friend CPoint		operator *		( float lhs, const CPoint &rhs )	{ return rhs * lhs; }

		inline CPoint		operator -		()							const	{ return CPoint( (v128)spu_xor( m_Vector, ( (vector float)(vector signed int){ 0x80000000, 0x80000000, 0x80000000, 0x80000000 } ) ) ); }

	public:
		//
		//	Ctors.
		//
		CPoint() {}
		CPoint( float x, float y, float z )
		:	m_Vector( (vector float){ x, y, z, 1.0f } )
		{}

		explicit CPoint( v128 v ) : m_Vector( v ) {}

		explicit CPoint( const CVector &vec );
		explicit CPoint( const CDirection &dir );
		explicit CPoint( CLEAR_CONSTRUCT_MODE clear_mode )
		{
			if ( clear_mode == CONSTRUCT_CLEAR )
			{
				Clear();
			}
		}

	private:
		//
		//	Aggregated members.
		//
		friend class CDirection;
		friend class CVector;
		friend class CQuat;
		friend class CMatrix;

		v128	m_Vector;
}
__attribute__ ( (aligned( 16 )) );

/***************************************************************************************************
*
*	CLASS			CDirection
*
*	DESCRIPTION		Defines a direction in R^3.
*
*	NOTES			This object occupies 16 Bytes of aligned storage.
*
***************************************************************************************************/
class CDirection
{
	public:
		//
		//	Accessors.
		//
		v128 &		Quadword		()					{ return m_Vector; }
		v128		QuadwordValue	()			const	{ return m_Vector; }

		float		X				()			const	{ return spu_extract( m_Vector, 0 ); }
		float		Y				()			const	{ return spu_extract( m_Vector, 1 ); }
		float		Z				()			const	{ return spu_extract( m_Vector, 2 ); }

		float		operator []		( int i )	const	{ return spu_extract( m_Vector, i ); }

	public:
		//
		//	Basic functionality.
		//
		float		Length			() 								const;
		float		LengthSquared	() 								const;
		float		Dot				( const CDirection &direction )	const;
		float		Dot				( const CPoint &point )			const;
		CDirection	Cross			( const CDirection &direction ) const;
		CDirection	Abs				()								const;
		CDirection	Min				( const CDirection &dir ) 		const;
		CDirection	Max				( const CDirection &dir )	 	const;
		void		Normalise		();

		static CDirection 	Lerp	( const CDirection &a, const CDirection &b, float t ) { return a + ( b - a ) * t; }

	public:
		//
		//	Clear the direction to (0,0,0).
		//
		void		Clear	()	{ m_Vector = spu_splats( 0.0f ); }

	public:
		//
		//	Operator overloads.
		//
		inline CDirection 	operator + 		( const CDirection &rhs ) 	const;
		inline CPoint 		operator + 		( const CPoint &rhs ) 		const;
		inline CDirection 	operator - 		( const CDirection &rhs ) 	const;
		inline CPoint	 	operator - 		( const CPoint &rhs ) 		const;
		inline CDirection 	operator * 		( const CDirection &rhs ) 	const;
		inline CDirection 	operator * 		( float rhs ) 				const	{ return CDirection( spu_mul( m_Vector, spu_splats( rhs ) ) ); }
		inline CDirection 	operator / 		( float rhs ) 				const	{ return CDirection( Intrinsics::SPU_Div( m_Vector, spu_splats( rhs ) ) ); }

		inline CDirection &	operator += 	( const CDirection &rhs );
		inline CDirection &	operator -= 	( const CDirection &rhs );
		inline CDirection &	operator *= 	( const CDirection &rhs )			{ m_Vector = spu_mul( m_Vector, rhs.m_Vector ); return *this; }
		inline CDirection &	operator *=		( float rhs )						{ m_Vector = spu_mul( m_Vector, spu_splats( rhs ) ); return *this; }
		inline CDirection &	operator /=		( float rhs )						{ m_Vector = Intrinsics::SPU_Div( m_Vector, spu_splats( rhs ) ); return *this; }

		friend CDirection	operator *		( float lhs, const CDirection &rhs ){ return rhs * lhs; }

		inline CDirection	operator -		()							const	{ return CDirection( (v128)spu_xor( m_Vector, ( (vector float)(vector signed int){ 0x80000000, 0x80000000, 0x80000000, 0x80000000 } ) ) ); }

	public:
		//
		//	Ctors.
		//
		CDirection() {}
		CDirection( float x, float y, float z )
		:	m_Vector( (vector float){ x, y, z, 0.0f } )
		{}

		explicit CDirection( v128 v ) : m_Vector( v ) {}

		explicit CDirection( const CPoint &p ) : m_Vector( p.m_Vector ) {}
		explicit CDirection( const CVector &vec );
		explicit CDirection( CLEAR_CONSTRUCT_MODE clear_mode )
		{
			if ( clear_mode == CONSTRUCT_CLEAR )
			{
				Clear();
			}
		}

	private:
		//
		//	Aggregated members.
		//
		friend class CPoint;
		friend class CVector;
		friend class CQuat;
		friend class CMatrix;

		v128	m_Vector;
}
__attribute__ ( (aligned( 16 )) );

/***************************************************************************************************
*
*	CLASS			CVector.
*
*	DESCRIPTION		Defines a generic vector quantity in R^4.
*
*	NOTES			This object occupies 16 Bytes of aligned storage.
*
***************************************************************************************************/
class CVector
{
	public:
		//
		//	Accessors.
		//
		v128 &		Quadword		()					{ return m_Vector; }
		v128		QuadwordValue	()			const	{ return m_Vector; }

		float		X				()			const	{ return spu_extract( m_Vector, 0 ); }
		float		Y				()			const	{ return spu_extract( m_Vector, 1 ); }
		float		Z				()			const	{ return spu_extract( m_Vector, 2 ); }
		float		W				()			const	{ return spu_extract( m_Vector, 3 ); }

		float		operator []		( int i )	const	{ return spu_extract( m_Vector, i ); }

	public:
		//
		//	Basic functionality.
		//
		float		Length			() 							const;
		float		LengthSquared	() 							const;
		float		Dot				( const CVector &vec )		const;
		CVector		Abs				()							const;
		CVector		Min				( const CVector &vec ) 		const;
		CVector		Max				( const CVector &vec ) 		const;

		static CVector Lerp			( const CVector &a, const CVector &b, float t ) { return a + ( b - a ) * t; }

	public:
		//
		//	Clear the vector to (0,0,0).
		//
		void		Clear	()	{ m_Vector = spu_splats( 0.0f ); }

	public:
		//
		//	Operator overloads.
		//
		inline CVector 		operator + 		( const CVector &rhs ) 		const;
		inline CVector 		operator - 		( const CVector &rhs ) 		const;
		inline CVector 		operator * 		( const CVector &rhs ) 		const;
		inline CVector 		operator * 		( float rhs ) 				const	{ return CVector( spu_mul( m_Vector, spu_splats( rhs ) ) ); }
		inline CVector 		operator / 		( const CVector &rhs ) 		const;
		inline CVector 		operator / 		( float rhs ) 				const	{ return CVector( Intrinsics::SPU_Div( m_Vector, spu_splats( rhs ) ) ); }

		inline CVector &	operator += 	( const CVector &rhs );
		inline CVector &	operator -= 	( const CVector &rhs );
		inline CVector &	operator *= 	( const CVector &rhs )				{ m_Vector = spu_mul( m_Vector, rhs.m_Vector ); return *this; }
		inline CVector &	operator *=		( float rhs )						{ m_Vector = spu_mul( m_Vector, spu_splats( rhs ) ); return *this; }
		inline CVector &	operator /= 	( const CVector &rhs );
		inline CVector &	operator /=		( float rhs )						{ m_Vector = Intrinsics::SPU_Div( m_Vector, spu_splats( rhs ) ); return *this; }

		friend CVector		operator *		( float lhs, const CVector &rhs )	{ return rhs * lhs; }

		inline CVector		operator -		()							const	{ return CVector( (v128)spu_xor( m_Vector, ( (vector float)(vector signed int){ 0x80000000, 0x80000000, 0x80000000, 0x80000000 } ) ) ); };

	public:
		//
		//	Ctors.
		//
		CVector() {}
		CVector( float x, float y, float z, float w )
		:	m_Vector( (vector float){ x, y, z, w } )
		{}

		explicit CVector( v128 v ) : m_Vector( v ) {}

		explicit CVector( const CPoint &p ) : m_Vector( p.m_Vector ) {}
		explicit CVector( const CDirection &d ) : m_Vector( d.m_Vector ) {}
		explicit CVector( CLEAR_CONSTRUCT_MODE clear_mode )
		{
			if ( clear_mode == CONSTRUCT_CLEAR )
			{
				Clear();
			}
		}

	private:
		//
		//	Aggregated members.
		//
		friend class CDirection;
		friend class CPoint;
		friend class CQuat;

		v128	m_Vector;
}
__attribute__ ( (aligned( 16 )) );

/***************************************************************************************************
*
*	CLASS			CQuat.
*
*	DESCRIPTION		Defines a quaternion.
*
*	NOTES			This object occupies 16 Bytes of aligned storage.
*
***************************************************************************************************/
class CQuat
{
	public:
		//
		//	Accessors.
		//
		v128 &		Quadword		()					{ return m_Vector; }
		v128		QuadwordValue	()			const	{ return m_Vector; }

		float		X				()			const	{ return spu_extract( m_Vector, 0 ); }
		float		Y				()			const	{ return spu_extract( m_Vector, 1 ); }
		float		Z				()			const	{ return spu_extract( m_Vector, 2 ); }
		float		W				()			const	{ return spu_extract( m_Vector, 3 ); }

		float		operator []		( int i )	const	{ return spu_extract( m_Vector, i ); }

	public:
		//
		//	Basic functionality.
		//
		void			Normalise		();

		float			Dot				( const CQuat &q ) const;
		float			Length			() const;

						// Use this quaternion to rotate the given quantity.
		v128			Rotate			( v128 v )					const;
		CPoint			Rotate			( const CPoint &p )			const { return CPoint( Rotate( p.m_Vector ) ); }
		CDirection		Rotate			( const CDirection &d ) 	const { return CDirection( Rotate( d.m_Vector ) ); }

		static CQuat	RotationBetween	( const CDirection &from, const CDirection &to );
		static CQuat	Slerp			( const CQuat &q0, const CQuat &q1, float t );

	public:
		//
		//	Clear the quaternion/set the quaternion to represent an identity rotation.
		//
		void		Clear			()					{ m_Vector = spu_splats( 0.0f ); }
		void		SetIdentity		()					{ m_Vector = (vector float){ 0.0f, 0.0f, 0.0f, 1.0f }; }

	public:
		//
		//	Operator overloads.
		//
		CQuat &		operator *= 	( const CQuat &rhs );
		CQuat &		operator *= 	( float rhs );
		CQuat &		operator /= 	( float rhs );

		CQuat		operator - 		() 							const;
		CQuat		operator ~ 		() 							const;
		CQuat 		operator * 		( const CQuat &rhs )		const;

		CQuat 		operator * 		( float rhs )				const;
		CQuat 		operator / 		( float rhs )				const;

		friend CQuat	operator *	( float lhs, const CQuat &rhs )	{ return rhs * lhs; }

	public:
		//
		//	Ctors.
		//
		CQuat() {}
		CQuat( float x, float y, float z, float w )
		:	m_Vector( (vector float){ x, y, z, w } )
		{}

		explicit CQuat( v128 v ) : m_Vector( v ) {}
		explicit CQuat( const CVector &v ) : m_Vector( v.m_Vector ) {}

		explicit CQuat( const CDirection &from, const CDirection &to ) : m_Vector( RotationBetween( from, to ).m_Vector ) {}
		explicit CQuat( const CDirection &axis, float angle )
		{
			vec_float4 half_angle = spu_mul( spu_splats( angle ), spu_splats( 0.5f ) );
			
			vec_float4 s, c;
			Intrinsics::SPU_SinCos( half_angle, &s, &c );

			m_Vector = spu_sel( spu_mul( axis.m_Vector, s ), c, (vector unsigned int)spu_maskb( 0x000f ) );
		}

		explicit CQuat( const CMatrix &matrix );

		explicit CQuat( CLEAR_CONSTRUCT_MODE clear_mode )
		{
			if ( clear_mode == CONSTRUCT_CLEAR )
			{
				Clear();
			}
		}

		explicit CQuat( IDENTITY_CONSTRUCT_MODE identity_mode )
		{
			if ( identity_mode == CONSTRUCT_IDENTITY )
			{
				SetIdentity();
			}
		}

	private:
		//
		//	Aggregated members.
		//
		friend class CDirection;
		friend class CVector;
		friend class CPoint;
		friend class CMatrix;

		v128	m_Vector;
}
__attribute__ ( (aligned( 16 )) );

//*****************************************************************************
//
//	CMatrix. Practically no support on SPUs here - just a conversion from
//	CQuat and some accessors really.
//
//*****************************************************************************
class CMatrix
{
	public:
		//
		//	Row accessors.
		//
		v128 &		GetRow0()			{ return m_Rows[ 0 ]; }
		v128 &		GetRow1()			{ return m_Rows[ 1 ]; }
		v128 &		GetRow2()			{ return m_Rows[ 2 ]; }
		v128 &		GetRow3()			{ return m_Rows[ 3 ]; }

		v128 		GetRow0()	const	{ return m_Rows[ 0 ]; }
		v128 		GetRow1()	const	{ return m_Rows[ 1 ]; }
		v128 		GetRow2()	const	{ return m_Rows[ 2 ]; }
		v128 		GetRow3()	const	{ return m_Rows[ 3 ]; }
		
		v128 &		operator[](int i)			{ return m_Rows[ i ]; }
		v128		operator[](int i) const		{ return m_Rows[ i ]; }

	public:
		//
		//	Ctors.
		//
		CMatrix() {}
		CMatrix( const CQuat &q, const CPoint &translation = CPoint( CONSTRUCT_CLEAR ) )
		{
			// This badly needs a vectorised version!
			float x2 = q.X() * q.X();
			float y2 = q.Y() * q.Y();
			float z2 = q.Z() * q.Z();

			float xy = q.X() * q.Y(); 
			float xz = q.X() * q.Z();
			float xw = q.X() * q.W();
			float yz = q.Y() * q.Z();
			float yw = q.Y() * q.W();
			float zw = q.Z() * q.W();

			m_Rows[ 0 ] = CDirection( 1.0f - 2.0f*( y2 + z2 ), 2.0f*( xy + zw ), 2.0f*( xz - yw ) ).m_Vector;
			m_Rows[ 1 ] = CDirection( 2.0f*( xy - zw ), 1.0f - 2.0f*( x2 + z2 ), 2.0f*( yz + xw ) ).m_Vector;
			m_Rows[ 2 ] = CDirection( 2.0f*( xz + yw ), 2.0f*( yz - xw ), 1.0f - 2.0f*( x2 + y2 ) ).m_Vector;
			m_Rows[ 3 ] = translation.m_Vector;
		}

		inline	CMatrix::CMatrix( CLEAR_CONSTRUCT_MODE ) 
		{
			v128 zero = spu_splats(0.0f);
			m_Rows[0] = zero;
			m_Rows[1] = zero;
			m_Rows[2] = zero;
			m_Rows[3] = zero;
		}



		CMatrix GetTranspose() const
		{
			CMatrix obOutput;

			v128 vrow1 = m_Rows[0];
			v128 vrow2 = m_Rows[1];
			v128 vrow3 = m_Rows[2];
			v128 vrow4 = m_Rows[3];

			v128 vtemp1 = vec_mergel( vrow1, vrow3 );
			v128 vtemp2 = vec_mergeh( vrow1, vrow3 );
			v128 vtemp3 = vec_mergel( vrow2, vrow4 );
			v128 vtemp4 = vec_mergeh( vrow2, vrow4 );

			obOutput[0] = vec_mergeh( vtemp2, vtemp4 );
			obOutput[1] = vec_mergel( vtemp2, vtemp4 );
			obOutput[2] = vec_mergeh( vtemp1, vtemp3 );
			obOutput[3] = vec_mergel( vtemp1, vtemp3 );

			return obOutput;
		}

		
		CMatrix	CMatrix::GetAffineInverse( void ) const
		{
			CMatrix obOutput;

			v128_arg vrow1 = m_Rows[0];
			v128_arg vrow2 = m_Rows[1];
			v128_arg vrow3 = m_Rows[2];
			v128_arg vrow4 = m_Rows[3];

			v128 vtemp1 = vec_mergel( vrow1, vrow3 );
			v128 vtemp2 = vec_mergeh( vrow1, vrow3 );
			v128 vtemp3 = vec_mergel( vrow2, vrow4 );
			v128 vtemp4 = vec_mergeh( vrow2, vrow4 );

			obOutput.m_Rows[0] = vec_mergeh( vtemp2, vtemp4 );
			obOutput.m_Rows[1] = vec_mergel( vtemp2, vtemp4 );
			obOutput.m_Rows[2] = vec_mergeh( vtemp1, vtemp3 );

			obOutput.m_Rows[0] = spu_mul (obOutput.m_Rows[0], g_float4_zeroInW);
			obOutput.m_Rows[1] = spu_mul (obOutput.m_Rows[1], g_float4_zeroInW);
			obOutput.m_Rows[2] = spu_mul (obOutput.m_Rows[2], g_float4_zeroInW);


			CPoint	obTranslation = -GetTranslation();
			v128 result;

			result = vec_madd( vec_splat(obTranslation.QuadwordValue(), 0), obOutput.m_Rows[0], g_float4_allzero );
			result = vec_madd( vec_splat(obTranslation.QuadwordValue(), 1), obOutput.m_Rows[1], result );
			result = vec_madd( vec_splat(obTranslation.QuadwordValue(), 2), obOutput.m_Rows[2], result );

			obOutput.m_Rows[3] = spu_add( spu_mul( result, g_float4_zeroInW ), g_float4_oneInW );

			return obOutput;
		}


//const v128 g_float4_zeroInW = (v128){ 1.0f, 1.0f, 1.0f, 0.0f };
//const v128 g_float4_oneInW = (v128){ 0.0f, 0.0f, 0.0f, 1.0f };
//const v128 g_float4_allzero = (v128){ 0.0f, 0.0f, 0.0f, 0.0f };

		inline	const	CDirection&	CMatrix::GetXAxis() const
		{
			return *( CDirection* )&m_Rows[0];
		}

		inline	void	CMatrix::SetXAxis( const CDirection& obXAxis )
		{
			m_Rows[0] = spu_mul( obXAxis.QuadwordValue(), g_float4_zeroInW );
		}

		inline	const	CDirection&	CMatrix::GetYAxis() const
		{
			return *( CDirection* )&m_Rows[1];
		}

		inline	void	CMatrix::SetYAxis( const CDirection& obYAxis )
		{
			m_Rows[1] = spu_mul( obYAxis.QuadwordValue(),g_float4_zeroInW );
		}

		inline	const	CDirection&	CMatrix::GetZAxis() const
		{
			return *( CDirection* )&m_Rows[2];
		}

		inline	void	CMatrix::SetZAxis( const CDirection& obZAxis )
		{
			m_Rows[2] = spu_mul( obZAxis.QuadwordValue(), g_float4_zeroInW );
		}

		inline	const	CPoint&	CMatrix::GetTranslation() const
		{
			return *( CPoint* )&m_Rows[3];
		}

		inline	void	CMatrix::SetTranslation( const CPoint& obTranslation )
		{
			m_Rows[3] = spu_add( spu_mul( obTranslation.QuadwordValue(), g_float4_zeroInW ), g_float4_oneInW );
		}


        void	Perspective( float fFieldOfViewY, float fAspectRatio, float fNear, float fFar )
        {
	        float fRcpTanFOV = 1.0f / tanf( fFieldOfViewY * 0.5f );
	        float fQ = fFar / ( fFar - fNear );
            const float fOne = 1.f;

	        // diag(-1, 1, 1, 1) * normal projection matrix
            m_Rows[ 0 ] = (v128){ -fRcpTanFOV / fAspectRatio, 0.0f, 0.0f, 0.0f };
            m_Rows[ 1 ] = (v128){ 0.0f, fRcpTanFOV, 0.0f, 0.0f };
            m_Rows[ 2 ] = (v128){ 0.0f, 0.0f, fQ, fOne }; 
            m_Rows[ 3 ] = (v128){ 0.0f, 0.0f, -fNear * fQ, 0.0f };
        }


	private:
		//
		//	Aggregated members.
		//
		v128	m_Rows[ 4 ];
}
__attribute__ ( (aligned( 16 )) );

inline CVector		operator *	( const CVector& obVector, const CMatrix& obMatrix )
{
	CVector	obResult;
	v128 result;
	v128_arg vrow0 = obMatrix.GetRow0();
	v128_arg vrow1 = obMatrix.GetRow1();
	v128_arg vrow2 = obMatrix.GetRow2();
	v128_arg vrow3 = obMatrix.GetRow3();

	result = spu_madd( spu_splats( spu_extract( obVector.QuadwordValue(), 0 ) ), vrow0, (v128){0, 0, 0, 0} );
	result = spu_madd( spu_splats( spu_extract( obVector.QuadwordValue(), 1 ) ), vrow1, result );
	result = spu_madd( spu_splats( spu_extract( obVector.QuadwordValue(), 2 ) ), vrow2, result );
	result = spu_madd( spu_splats( spu_extract( obVector.QuadwordValue(), 3 ) ), vrow3, result );

	obResult.Quadword() = result;

	return obResult;
}

inline v128		operator *	( const v128 obVector, const CMatrix& obMatrix )
{
	v128 result;
	v128_arg vrow0 = obMatrix.GetRow0();
	v128_arg vrow1 = obMatrix.GetRow1();
	v128_arg vrow2 = obMatrix.GetRow2();
	v128_arg vrow3 = obMatrix.GetRow3();

	result = spu_madd( spu_splats( spu_extract( obVector, 0 ) ), vrow0, (v128){0, 0, 0, 0} );
	result = spu_madd( spu_splats( spu_extract( obVector, 1 ) ), vrow1, result );
	result = spu_madd( spu_splats( spu_extract( obVector, 2 ) ), vrow2, result );
	result = spu_madd( spu_splats( spu_extract( obVector, 3 ) ), vrow3, result );

	return result;
}


inline CMatrix* MatrixMultiply(CMatrix* dest, CMatrix const* lhs, CMatrix const* rhs)
{

    dest -> GetRow0() = lhs -> GetRow0() * *rhs;
    dest -> GetRow1() = lhs -> GetRow1() * *rhs;
    dest -> GetRow2() = lhs -> GetRow2() * *rhs;
    dest -> GetRow3() = lhs -> GetRow3() * *rhs;

    return dest;
}
inline CMatrix& MatrixMultiply(CMatrix& dest, CMatrix const& lhs, CMatrix const& rhs)
{

    dest.GetRow0() = lhs.GetRow0() * rhs;
    dest.GetRow1() = lhs.GetRow1() * rhs;
    dest.GetRow2() = lhs.GetRow2() * rhs;
    dest.GetRow3() = lhs.GetRow3() * rhs;

    return dest;
}

// apprently it's bad, but I need it for the hair
// MONSTERS\Frank 23/03/2006 18:39:41
inline CMatrix  operator* (CMatrix const& lhs, CMatrix const& rhs)
{
    CMatrix temp;
	
    temp.GetRow0() = lhs.GetRow0() * rhs;
    temp.GetRow1() = lhs.GetRow1() * rhs;
    temp.GetRow2() = lhs.GetRow2() * rhs;
    temp.GetRow3() = lhs.GetRow3() * rhs;

    return temp;

}


//*****************************************************************************
//*****************************************************************************
//	
//	CQuat implementation.	
//
//*****************************************************************************
//*****************************************************************************

//*****************************************************************************
//	Overloaded operators.
//*****************************************************************************
inline CQuat &CQuat::operator *= ( const CQuat &rhs )
{
//	result.X() = ( W() * rhs.X() ) + ( X() * rhs.W() ) + ( Y() * rhs.Z() ) - ( Z() * rhs.Y() );
//	result.Y() = ( W() * rhs.Y() ) - ( X() * rhs.Z() ) + ( Y() * rhs.W() ) + ( Z() * rhs.X() );
//	result.Z() = ( W() * rhs.Z() ) + ( X() * rhs.Y() ) - ( Y() * rhs.X() ) + ( Z() * rhs.W() );
//	result.W() = ( W() * rhs.W() ) - ( X() * rhs.X() ) - ( Y() * rhs.Y() ) - ( Z() * rhs.Z() );

	vector float rhs_shuffle_x = spu_shuffle( rhs.m_Vector, rhs.m_Vector, VECTORMATH_SHUF_WZYX );
	vector float rhs_shuffle_y = spu_shuffle( rhs.m_Vector, rhs.m_Vector, VECTORMATH_SHUF_ZWXY );
	vector float rhs_shuffle_z = spu_shuffle( rhs.m_Vector, rhs.m_Vector, VECTORMATH_SHUF_YXWZ );

	vector float dp4_x = Intrinsics::SPU_DP4( m_Vector, spu_mul( rhs_shuffle_x, (vector float){ 1.0f, 1.0f, -1.0f, 1.0f } ) );
	vector float dp4_y = Intrinsics::SPU_DP4( m_Vector, spu_mul( rhs_shuffle_y, (vector float){ -1.0f, 1.0f, 1.0f, 1.0f } ) );
	vector float dp4_z = Intrinsics::SPU_DP4( m_Vector, spu_mul( rhs_shuffle_z, (vector float){ 1.0f, -1.0f, 1.0f, 1.0f } ) );
	vector float dp4_w = Intrinsics::SPU_DP4( m_Vector, spu_mul( rhs.m_Vector, (vector float){ -1.0f, -1.0f, -1.0f, 1.0f } ) );

	dp4_x = (vector float)spu_and( (vector unsigned int)dp4_x, (vector unsigned int){ 0xffffffff, 0, 0, 0 } );
	dp4_y = (vector float)spu_and( (vector unsigned int)dp4_y, (vector unsigned int){ 0, 0xffffffff, 0, 0 } );
	dp4_z = (vector float)spu_and( (vector unsigned int)dp4_z, (vector unsigned int){ 0, 0, 0xffffffff, 0 } );
	dp4_w = (vector float)spu_and( (vector unsigned int)dp4_w, (vector unsigned int){ 0, 0, 0, 0xffffffff } );

	// Not great this - too many pipeline stalls.
	m_Vector = spu_add( dp4_x, spu_add( dp4_y, spu_add( dp4_z, dp4_w ) ) );
	return *this;
}

inline CQuat &CQuat::operator *= ( float rhs )
{
	m_Vector = spu_mul( m_Vector, spu_splats( rhs ) );
	return *this;
}

inline CQuat &CQuat::operator /= ( float rhs )
{
	m_Vector = Intrinsics::SPU_Div( m_Vector, spu_splats( rhs ) );
	return *this;
}

inline CQuat CQuat::operator - () const
{
	return CQuat( (v128)spu_xor( m_Vector, ( (vector float)(vector signed int){ 0x80000000, 0x80000000, 0x80000000, 0x80000000 } ) ) );
}

inline CQuat CQuat::operator ~ () const
{
	return CQuat( (v128)spu_xor( m_Vector, ( (vector float)(vector signed int){ 0x80000000, 0x80000000, 0x80000000, 0 } ) ) );
}

inline CQuat CQuat::operator * ( const CQuat &rhs ) const
{
//	result.X() = ( W() * rhs.X() ) + ( X() * rhs.W() ) + ( Y() * rhs.Z() ) - ( Z() * rhs.Y() );
//	result.Y() = ( W() * rhs.Y() ) - ( X() * rhs.Z() ) + ( Y() * rhs.W() ) + ( Z() * rhs.X() );
//	result.Z() = ( W() * rhs.Z() ) + ( X() * rhs.Y() ) - ( Y() * rhs.X() ) + ( Z() * rhs.W() );
//	result.W() = ( W() * rhs.W() ) - ( X() * rhs.X() ) - ( Y() * rhs.Y() ) - ( Z() * rhs.Z() );

	vector float rhs_shuffle_x = spu_shuffle( rhs.m_Vector, rhs.m_Vector, VECTORMATH_SHUF_WZYX );
	vector float rhs_shuffle_y = spu_shuffle( rhs.m_Vector, rhs.m_Vector, VECTORMATH_SHUF_ZWXY );
	vector float rhs_shuffle_z = spu_shuffle( rhs.m_Vector, rhs.m_Vector, VECTORMATH_SHUF_YXWZ );

	vector float dp4_x = Intrinsics::SPU_DP4( m_Vector, spu_mul( rhs_shuffle_x, (vector float){ 1.0f, 1.0f, -1.0f, 1.0f } ) );
	vector float dp4_y = Intrinsics::SPU_DP4( m_Vector, spu_mul( rhs_shuffle_y, (vector float){ -1.0f, 1.0f, 1.0f, 1.0f } ) );
	vector float dp4_z = Intrinsics::SPU_DP4( m_Vector, spu_mul( rhs_shuffle_z, (vector float){ 1.0f, -1.0f, 1.0f, 1.0f } ) );
	vector float dp4_w = Intrinsics::SPU_DP4( m_Vector, spu_mul( rhs.m_Vector, (vector float){ -1.0f, -1.0f, -1.0f, 1.0f } ) );

	dp4_x = (vector float)spu_and( (vector unsigned int)dp4_x, (vector unsigned int){ 0xffffffff, 0, 0, 0 } );
	dp4_y = (vector float)spu_and( (vector unsigned int)dp4_y, (vector unsigned int){ 0, 0xffffffff, 0, 0 } );
	dp4_z = (vector float)spu_and( (vector unsigned int)dp4_z, (vector unsigned int){ 0, 0, 0xffffffff, 0 } );
	dp4_w = (vector float)spu_and( (vector unsigned int)dp4_w, (vector unsigned int){ 0, 0, 0, 0xffffffff } );

	// Not great this - too many pipeline stalls.
	return CQuat( spu_add( dp4_x, spu_add( dp4_y, spu_add( dp4_z, dp4_w ) ) ) );
}

inline CQuat CQuat::operator * ( float rhs ) const
{
	return CQuat( spu_mul( m_Vector, spu_splats( rhs ) ) );
}

inline CQuat CQuat::operator / ( float rhs ) const
{
	return CQuat( Intrinsics::SPU_Div( m_Vector, spu_splats( rhs ) ) );
}

inline CQuat::CQuat( const CMatrix &obMatrix )
{
#	define MGet( m, r, c ) spu_extract( m[ r ], c )

	float	fTrace = MGet( obMatrix, 0, 0 ) + MGet( obMatrix, 1, 1 ) + MGet( obMatrix, 2, 2 );
	float	fS;

	// Check the diagonal

	if ( fTrace > 0.0f )
	{
		fS	= spu_extract( Intrinsics::SPU_Sqrt( spu_splats( fTrace + 1.0f ) ), 0 );
		float w_temp = fS / 2.0f;
		fS	= 0.5f / fS;
		CVector temp(	( MGet( obMatrix, 1, 2 ) - MGet( obMatrix, 2, 1 ) ) * fS,
						( MGet( obMatrix, 2, 0 ) - MGet( obMatrix, 0, 2 ) ) * fS,
						( MGet( obMatrix, 0, 1 ) - MGet( obMatrix, 1, 0 ) ) * fS,
						w_temp );
		m_Vector = temp.QuadwordValue();
	}
	else
	{
		static const int aiNext[3] = { 1, 2, 0 };
		int	i, j, k;

		// Diagonal is negative
		i = 0;

		if ( MGet( obMatrix, 1, 1 ) > MGet( obMatrix, 0, 0 ) )
			i = 1;

		if ( MGet( obMatrix, 2, 2 ) > MGet( obMatrix, i, i ) )
			i = 2;

		j = aiNext[i];
		k = aiNext[j];

		fS = spu_extract( Intrinsics::SPU_Sqrt( spu_splats( ( MGet( obMatrix, i, i ) - ( MGet( obMatrix, j, j ) + MGet( obMatrix, k, k ) ) ) + 1.0f ) ), 0 );

		//Index( i )	= fS * 0.5f;
		CVector index_i;
		switch ( i )
		{
			case 0:
				index_i = CVector( fS * 0.5f, 0.0f, 0.0f, 0.0f );
				break;
			case 1:
				index_i = CVector( 0.0f, fS * 0.5f, 0.0f, 0.0f );
				break;
			case 2:
				index_i = CVector( 0.0f, 0.0f, fS * 0.5f, 0.0f );
				break;
			default:
				index_i = CVector( fS * 0.5f, 0.0f, 0.0f, 0.0f );
				ntError( false );	// i should always be 0, 1 or 2.
				break;
		}

		if ( fS != 0.0f )
			fS = 0.5f / fS;

		//Index( j )	= ( MGet( obMatrix, i, j ) + MGet( obMatrix, j, i ) ) * fS;
		CVector index_j;
		switch ( j )
		{
			case 0:
				index_j = CVector( ( MGet( obMatrix, i, j ) + MGet( obMatrix, j, i ) ) * fS, 0.0f, 0.0f, 0.0f );
				break;
			case 1:
				index_j = CVector( 0.0f, ( MGet( obMatrix, i, j ) + MGet( obMatrix, j, i ) ) * fS, 0.0f, 0.0f );
				break;
			case 2:
				index_j = CVector( 0.0f, 0.0f, ( MGet( obMatrix, i, j ) + MGet( obMatrix, j, i ) ) * fS, 0.0f );
				break;
			default:
				index_j = CVector( 0.0f, ( MGet( obMatrix, i, j ) + MGet( obMatrix, j, i ) ) * fS, 0.0f, 0.0f );
				ntError( false );	// i should always be 0, 1 or 2.
				break;
		}

//		Index( k )	= ( MGet( obMatrix, i, k ) + MGet( obMatrix, k, i ) ) * fS;
		CVector index_k;
		switch ( k )
		{
			case 0:
				index_k = CVector( ( MGet( obMatrix, i, k ) + MGet( obMatrix, k, i ) ) * fS, 0.0f, 0.0f, 0.0f );
				break;
			case 1:
				index_k = CVector( 0.0f, ( MGet( obMatrix, i, k ) + MGet( obMatrix, k, i ) ) * fS, 0.0f, 0.0f );
				break;
			case 2:
				index_k = CVector( 0.0f, 0.0f, ( MGet( obMatrix, i, k ) + MGet( obMatrix, k, i ) ) * fS, 0.0f );
				break;
			default:
				index_k = CVector( 0.0f, 0.0f, ( MGet( obMatrix, i, k ) + MGet( obMatrix, k, i ) ) * fS, 0.0f );
				ntError( false );	// i should always be 0, 1 or 2.
				break;
		}

//		Index( 3 )	= ( MGet( obMatrix, j, k ) - MGet( obMatrix, k, j ) ) * fS;
		CVector res = index_i + index_j + index_k + CVector( 0.0f, 0.0f, 0.0f, ( MGet( obMatrix, j, k ) - MGet( obMatrix, k, j ) ) * fS );
		m_Vector = res.QuadwordValue();
	}

	Normalise();

#	undef MGet
}

//*****************************************************************************
//	Functional operations.
//*****************************************************************************
inline void CQuat::Normalise()
{
	vector float dot, shifted_4;
	
	shifted_4 = spu_rlqwbyte( m_Vector, 4 );

	dot = spu_mul( m_Vector, m_Vector );
	dot = spu_madd( shifted_4, shifted_4, dot );
	dot = spu_add( spu_rlqwbyte( dot, 8 ), dot );

	dot = spu_splats( spu_extract( dot, 0 ) );

	m_Vector = spu_mul( m_Vector, Intrinsics::SPU_RSqrt( dot ) );
}

inline float CQuat::Length() const
{
	vector float shifted_4 = spu_rlqwbyte( m_Vector, 4 );
	vector float result = spu_mul( m_Vector, m_Vector );
	result = spu_madd( shifted_4, shifted_4, result );
	return spu_extract( Intrinsics::SPU_Sqrt( spu_add( spu_rlqwbyte( result, 8 ), result ) ), 0 );
}

inline float CQuat::Dot( const CQuat &q ) const
{
	return spu_extract( Intrinsics::SPU_DP4( m_Vector, q.m_Vector ), 0 );
}

inline v128 CQuat::Rotate( v128 v ) const
{
// Inline version.
//	return ( *this * ( CQuat( v ) * this->operator ~ () ) ).m_Vector;

// Explained version.
	CQuat p( v );

	CQuat conj = this->operator ~ ();

	CQuat half_res = p * conj;
	CQuat quat_res = *this * half_res;

	return quat_res.m_Vector;
}

inline CQuat CQuat::RotationBetween( const CDirection &from, const CDirection &to )
{
	CQuat result;

	vector float cosAngle, cosAngleX2Plus2, recipCosHalfAngleX2, cosHalfAngleX2;

	cosAngle = Intrinsics::SPU_DP3( from.m_Vector, to.m_Vector );

	cosAngle = spu_shuffle( cosAngle, cosAngle, (vector unsigned char)spu_splats( 0x00010203 ) );
	cosAngleX2Plus2 = spu_madd( cosAngle, spu_splats( 2.0f ), spu_splats( 2.0f ) );
	recipCosHalfAngleX2 = Intrinsics::SPU_RSqrt( cosAngleX2Plus2 );
	cosHalfAngleX2 = spu_mul( recipCosHalfAngleX2, cosAngleX2Plus2 );

	CDirection crossVec( from.Cross( to ) );

	result.m_Vector = spu_mul( crossVec.m_Vector, recipCosHalfAngleX2 );
	result.m_Vector = spu_sel( result.m_Vector, spu_mul( cosHalfAngleX2, spu_splats( 0.5f ) ), (vector unsigned int)spu_maskb( 0x000f ) );

	return result;
}

inline CQuat CQuat::Slerp( const CQuat &q0, const CQuat &q1, float t )
{
	ntError_p( fabsf( q0.Length() - 1.0f ) < 0.001f, ("SLERP: q0 is not normalised, length is %f", q0.Length()) );
	ntError_p( fabsf( q1.Length() - 1.0f ) < 0.001f, ("SLERP: q1 is not normalised, length is %f", q1.Length()) );

	// Ripped from the SCE vectormath libs.
	vec_float4 scales, scale0, scale1, cosAngle, angle, tttt, oneMinusT, angles, sines;
	vec_uint4 selectMask;
	vec_uchar16 shuffle_xxxx = (vec_uchar16)spu_splats((int)0x00010203);
	vec_uchar16 shuffle_yyyy = (vec_uchar16)spu_splats((int)0x04050607);
	vec_uchar16 shuffle_zzzz = (vec_uchar16)spu_splats((int)0x08090a0b);
	cosAngle = Intrinsics::SPU_DP4( q0.m_Vector, q1.m_Vector );
	cosAngle = spu_shuffle( cosAngle, cosAngle, shuffle_xxxx );
	selectMask = (vec_uint4)spu_cmpgt( spu_splats(0.0f), cosAngle );
	cosAngle = spu_sel( cosAngle, Intrinsics::SPU_Negate( cosAngle ), selectMask );
	CQuat start( spu_sel( q0.m_Vector, Intrinsics::SPU_Negate( q0.m_Vector ), selectMask ) );
	selectMask = (vec_uint4)spu_cmpgt( spu_splats(VECTORMATH_SLERP_TOL), cosAngle );
	angle = Intrinsics::SPU_ACos( cosAngle );
	tttt = spu_splats(t);
	oneMinusT = spu_sub( spu_splats(1.0f), tttt );
	angles = spu_sel( spu_splats(1.0f), oneMinusT, (vec_uint4)spu_maskb(0x0f00) );
	angles = spu_sel( angles, tttt, (vec_uint4)spu_maskb(0x00f0) );
	angles = spu_mul( angles, angle );
	sines = Intrinsics::SPU_Sin( angles );
	scales = Intrinsics::SPU_Div( sines, spu_shuffle( sines, sines, shuffle_xxxx ) );
	scale0 = spu_sel( oneMinusT, spu_shuffle( scales, scales, shuffle_yyyy ), selectMask );
	scale1 = spu_sel( tttt, spu_shuffle( scales, scales, shuffle_zzzz ), selectMask );

	CQuat res( spu_madd( start.m_Vector, scale0, spu_mul( q1.m_Vector, scale1 ) ) );
	ntError_p( fabsf( res.Length() - 1.0f ) < 0.001f, ("SLERP is returning non-normalised quats. Length is %f\n", res.Length()) );
	return res;

//	return CQuat( spu_madd( start.m_Vector, scale0, spu_mul( q1.m_Vector, scale1 ) ) );
}

//*****************************************************************************
//*****************************************************************************
//	
//	CVector implementation.	
//
//*****************************************************************************
//*****************************************************************************

//*****************************************************************************
//	Overloaded operators.
//*****************************************************************************
inline CVector CVector::operator / ( const CVector &rhs ) const
{
	return CVector( Intrinsics::SPU_Div( m_Vector, rhs.m_Vector ) );
}

inline CVector CVector::operator * ( const CVector &rhs ) const
{
	return CVector( spu_mul( m_Vector, rhs.m_Vector ) );
}

inline CVector CVector::operator + ( const CVector &rhs ) const
{
	return CVector( spu_add( m_Vector, rhs.m_Vector ) );
}

inline CVector CVector::operator - ( const CVector &rhs ) const
{
	return CVector( spu_sub( m_Vector, rhs.m_Vector ) );
}

inline CVector &CVector::operator += ( const CVector &rhs )
{
	m_Vector = spu_add( m_Vector, rhs.m_Vector );
	return *this;
}

inline CVector &CVector::operator -= ( const CVector &rhs )
{
	m_Vector = spu_sub( m_Vector, rhs.m_Vector );
	return *this;
}

inline CVector &CVector::operator /= ( const CVector &rhs )
{
	m_Vector = Intrinsics::SPU_Div( m_Vector, rhs.m_Vector );
	return *this;
}

//*****************************************************************************
//	Functional operations.
//*****************************************************************************
inline float CVector::Length() const
{
	vector float shifted_4 = spu_rlqwbyte( m_Vector, 4 );
	vector float result = spu_mul( m_Vector, m_Vector );
	result = spu_madd( shifted_4, shifted_4, result );
	return spu_extract( Intrinsics::SPU_Sqrt( spu_add( spu_rlqwbyte( result, 8 ), result ) ), 0 );
}

inline float CVector::LengthSquared() const
{
	vector float shifted_4 = spu_rlqwbyte( m_Vector, 4 );
	vector float result = spu_mul( m_Vector, m_Vector );
	result = spu_madd( shifted_4, shifted_4, result );
	return spu_extract( spu_add( spu_rlqwbyte( result, 8 ), result ), 0 );
}

inline float CVector::Dot( const CVector &d ) const
{
	return spu_extract( Intrinsics::SPU_DP4( m_Vector, d.m_Vector ), 0 );
}

inline CVector CVector::Abs() const
{
	return CVector( Intrinsics::SPU_Abs( m_Vector ) );
}

inline CVector CVector::Min( const CVector &p ) const
{
	return CVector( Intrinsics::SPU_Min( m_Vector, p.m_Vector ) );
}

inline CVector CVector::Max( const CVector &p ) const
{
	return CVector( Intrinsics::SPU_Max( m_Vector, p.m_Vector ) );
}

//*****************************************************************************
//*****************************************************************************
//	
//	CDirection implementation.	
//
//*****************************************************************************
//*****************************************************************************

//*****************************************************************************
//	Ctors.
//*****************************************************************************
inline CDirection::CDirection( const CVector &vec )
:	m_Vector( vec.m_Vector )
{}

//*****************************************************************************
//	Overloaded operators.
//*****************************************************************************
inline CDirection CDirection::operator * ( const CDirection &rhs ) const
{
	return CDirection( spu_mul( m_Vector, rhs.m_Vector ) );
}

inline CDirection CDirection::operator + ( const CDirection &rhs ) const
{
	return CDirection( spu_add( m_Vector, rhs.m_Vector ) );
}

inline CPoint CDirection::operator + ( const CPoint &rhs ) const
{
	return CPoint( spu_add( m_Vector, rhs.m_Vector ) );
}

inline CDirection CDirection::operator - ( const CDirection &rhs ) const
{
	return CDirection( spu_sub( m_Vector, rhs.m_Vector ) );
}

inline CPoint CDirection::operator - ( const CPoint &rhs ) const
{
	return CPoint( spu_sub( m_Vector, rhs.m_Vector ) );
}

inline CDirection &CDirection::operator += ( const CDirection &rhs )
{
	m_Vector = spu_add( m_Vector, rhs.m_Vector );
	return *this;
}

inline CDirection &CDirection::operator -= ( const CDirection &rhs )
{
	m_Vector = spu_sub( m_Vector, rhs.m_Vector );
	return *this;
}

//*****************************************************************************
//	Functional operations.
//*****************************************************************************
inline float CDirection::Length() const
{
	return spu_extract( Intrinsics::SPU_Sqrt( Intrinsics::SPU_DP3( m_Vector, m_Vector ) ), 0 );
}

inline float CDirection::LengthSquared() const
{
	return spu_extract( Intrinsics::SPU_DP3( m_Vector, m_Vector ), 0 );
}

inline float CDirection::Dot( const CPoint &p ) const
{
	return spu_extract( Intrinsics::SPU_DP3( m_Vector, p.m_Vector ), 0 );
}

inline float CDirection::Dot( const CDirection &d ) const
{
	return spu_extract( Intrinsics::SPU_DP3( m_Vector, d.m_Vector ), 0 );
}

inline CDirection CDirection::Abs() const
{
	return CDirection( Intrinsics::SPU_Abs( m_Vector ) );
}

inline CDirection CDirection::Min( const CDirection &d ) const
{
	return CDirection( Intrinsics::SPU_Min( m_Vector, d.m_Vector ) );
}

inline CDirection CDirection::Max( const CDirection &d ) const
{
	return CDirection( Intrinsics::SPU_Max( m_Vector, d.m_Vector ) );
}

inline CDirection CDirection::Cross( const CDirection &d ) const
{
	vec_float4 tmp0, tmp1, tmp2, tmp3, result;
	tmp0 = spu_shuffle( m_Vector,	m_Vector,	VECTORMATH_SHUF_YZXW );
	tmp1 = spu_shuffle( d.m_Vector,	d.m_Vector,	VECTORMATH_SHUF_ZXYW );
	tmp2 = spu_shuffle( m_Vector,	m_Vector,	VECTORMATH_SHUF_ZXYW );
	tmp3 = spu_shuffle( d.m_Vector,	d.m_Vector,	VECTORMATH_SHUF_YZXW );
	result = spu_mul( tmp0, tmp1 );
	result = spu_nmsub( tmp2, tmp3, result );
	return CDirection( result );
}

inline void CDirection::Normalise()
{
	v128 shifted_4 = spu_rlqwbyte( m_Vector, 4 );
	v128 len_sq = spu_mul( m_Vector, m_Vector );
	v128 shifted_8 = spu_rlqwbyte( m_Vector, 8 );
	len_sq = spu_madd( shifted_4, shifted_4, len_sq );
	len_sq = spu_madd( shifted_8, shifted_8, len_sq );
	len_sq = spu_splats( spu_extract( len_sq, 0 ) );
	m_Vector = spu_mul( m_Vector, Intrinsics::SPU_RSqrt( len_sq ) );
}

//*****************************************************************************
//*****************************************************************************
//	
//	CPoint implementation.	
//
//*****************************************************************************
//*****************************************************************************

//*****************************************************************************
//	Ctors.
//*****************************************************************************
inline CPoint::CPoint( const CDirection &dir )
:	m_Vector( dir.m_Vector )
{}

inline CPoint::CPoint( const CVector &vec )
:	m_Vector( vec.m_Vector )
{}

//*****************************************************************************
//	Overloaded operators.
//*****************************************************************************
inline CPoint CPoint::operator * ( const CDirection &rhs ) const
{
	return CPoint( spu_mul( m_Vector, rhs.m_Vector ) );
}

inline CPoint CPoint::operator * ( const CPoint &rhs ) const
{
	return CPoint( spu_mul( m_Vector, rhs.m_Vector ) );
}

inline CPoint CPoint::operator + ( const CDirection &rhs ) const
{
	return CPoint( spu_add( m_Vector, rhs.m_Vector ) );
}

inline CPoint CPoint::operator + ( const CPoint &rhs ) const
{
	return CPoint( spu_add( m_Vector, rhs.m_Vector ) );
}

inline CPoint CPoint::operator - ( const CDirection &rhs ) const
{
	return CPoint( spu_sub( m_Vector, rhs.m_Vector ) );
}

inline CPoint CPoint::operator - ( const CPoint &rhs ) const
{
	return CPoint( spu_sub( m_Vector, rhs.m_Vector ) );
}

inline CDirection CPoint::operator ^ ( const CPoint &rhs ) const
{
	return CDirection( spu_sub( m_Vector, rhs.m_Vector ) );
}

inline CPoint &CPoint::operator *= ( const CDirection &rhs )
{
	m_Vector = spu_mul( m_Vector, rhs.m_Vector );
	return *this;
}

inline CPoint &CPoint::operator += ( const CDirection &rhs )
{
	m_Vector = spu_add( m_Vector, rhs.m_Vector );
	return *this;
}

inline CPoint &CPoint::operator += ( const CPoint &rhs )
{
	m_Vector = spu_add( m_Vector, rhs.m_Vector );
	return *this;
}

inline CPoint &CPoint::operator -= ( const CDirection &rhs )
{
	m_Vector = spu_sub( m_Vector, rhs.m_Vector );
	return *this;
}

inline CPoint &CPoint::operator -= ( const CPoint &rhs )
{
	m_Vector = spu_sub( m_Vector, rhs.m_Vector );
	return *this;
}

//*****************************************************************************
//	Functional operations.
//*****************************************************************************
inline float CPoint::Length() const
{
	return spu_extract( Intrinsics::SPU_Sqrt( Intrinsics::SPU_DP3( m_Vector, m_Vector ) ), 0 );
}

inline float CPoint::LengthSquared() const
{
	return spu_extract( Intrinsics::SPU_DP3( m_Vector, m_Vector ), 0 );
}

inline float CPoint::Dot( const CPoint &p ) const
{
	return spu_extract( Intrinsics::SPU_DP3( m_Vector, p.m_Vector ), 0 );
}

inline float CPoint::Dot( const CDirection &d ) const
{
	return spu_extract( Intrinsics::SPU_DP3( m_Vector, d.m_Vector ), 0 );
}

inline CPoint CPoint::Abs() const
{
	return CPoint( Intrinsics::SPU_Abs( m_Vector ) );
}

inline CPoint CPoint::Min( const CPoint &p ) const
{
	return CPoint( Intrinsics::SPU_Min( m_Vector, p.m_Vector ) );
}

inline CPoint CPoint::Max( const CPoint &p ) const
{
	return CPoint( Intrinsics::SPU_Max( m_Vector, p.m_Vector ) );
}

namespace Intrinsics
{
	inline v128	SPU_DP3( v128 a, v128 b )
	{
		v128 result;
		result = spu_mul( a, b );
		result = spu_madd( spu_rlqwbyte( a, 4 ), spu_rlqwbyte( b, 4 ), result );
		result = spu_madd( spu_rlqwbyte( a, 8 ), spu_rlqwbyte( b, 8 ), result );

		// Dp3 is now valud in element 0, need to copy to others.
		return spu_splats( spu_extract( result, 0 ) );
	}

	inline v128	SPU_DP4( v128 a, v128 b )
	{
		v128 result;
		result = spu_mul( a, b );
		result = spu_madd( spu_rlqwbyte( a, 4 ), spu_rlqwbyte( b, 4 ), result );
		return spu_add( spu_rlqwbyte( result, 8 ), result );
	}

	inline v128	SPU_Negate( v128 a )
	{
		return (v128)spu_xor( a, ( (vector float)(vector signed int){ 0x80000000, 0x80000000, 0x80000000, 0x80000000 } ) );
	}

	inline vector unsigned int	SPU_Sign( v128 a )
	{
		return spu_and( (vector unsigned int)a, ( (vector unsigned int){ 0x80000000, 0x80000000, 0x80000000, 0x80000000 } ) );
	}

	inline v128	SPU_Sin( v128 a )
	{
		// XXX: Should experiment with using sinf4fast instead.
		return sinf4( a );
	}

	inline v128	SPU_Cos( v128 a )
	{
		// XXX: Should experiment with using cosf4fast instead.
		return cosf4( a );
	}

	inline v128	SPU_ACos( v128 a )
	{
		// XXX: Should experiment with using acosf4fast instead.
		return acosf4( a );
	}

	inline v128	SPU_Div( v128 a, v128 b )
	{
		// XXX: Should experiment with using recipf4fast instead - although the fast
		//		version just uses spu_re to calculate an approximation to 12-bits of
		//		accuracy in the mantissa; may not be good enough.
		return spu_mul( a, recipf4( b ) );
	}

	inline v128	SPU_Sqrt( v128 a )
	{
		// XXX: Should experiment with using sqrtf4fast instead.
		return sqrtf4( a );
	}

	inline v128	SPU_RSqrt( v128 a )
	{
		// XXX: Should experiment with using rsqrtf4fast instead.
		return rsqrtf4( a );
	}

	inline v128	SPU_Abs( v128 a )
	{
		return (v128)spu_and( a, ( (vector float)(vector signed int){ 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff } ) );
	}

	inline v128	SPU_Min( v128 a, v128 b )
	{
		// mask_a = a <= b,
		// mask_b = ~( a <= b ).
		vector unsigned int mask_a = spu_cmpgt( b, a );
		vector unsigned int mask_b = spu_xor( mask_a, (vector unsigned int){ 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff } );

		return spu_add( (v128)spu_and( (vector unsigned int)a, mask_a ),
						(v128)spu_and( (vector unsigned int)b, mask_b ) );
	}

	inline v128	SPU_Max( v128 a, v128 b )
	{
		// mask_a = a > b,
		// mask_b = ~( a > b ).
		vector unsigned int mask_a = spu_cmpgt( a, b );
		vector unsigned int mask_b = spu_xor( mask_a, (vector unsigned int){ 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff } );

		return spu_add( (v128)spu_and( (vector unsigned int)a, mask_a ),
						(v128)spu_and( (vector unsigned int)b, mask_b ) );
	}

	inline void	SPU_SinCos( v128 a, v128 *s, v128 *c )
	{
		// XXX: Should experiment with using sincosf4fast instead.
		sincosf4( a, s, c );
	}
}

#endif //!VECMATH_SPU_PS3_H_

