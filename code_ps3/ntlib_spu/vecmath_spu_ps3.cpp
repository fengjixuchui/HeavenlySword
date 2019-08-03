/***************************************************************************************************
*
*	Vector Maths 
*
*	This cpp file contains a unit test for the SPU vector maths classes.
*
***************************************************************************************************/

#ifndef __SPU__
#	error This file can only be included in an SPU project.
#endif // !__SPU__

//*****************************************************************************
//	Includes.
//*****************************************************************************
#include "vecmath_spu_ps3.h"
#include "debug_spu.h"

#ifdef ENABLE_SPU_VECTORMATHS_UNITTESTS

//*****************************************************************************
//	Forward declarations.
//*****************************************************************************
namespace
{
	void UnitTestIntrinsics	();
	void UnitTestVector		();
	void UnitTestDirection	();
	void UnitTestPoint		();
	void UnitTestQuat		();
	void UnitTestMatrix		();
}

//*****************************************************************************
//	Test macros.
//*****************************************************************************
#ifdef _DEBUG
#	define UNIT_TEST( expr )			do { if ( !(expr) ) { ntPrintf( "%s(%d): Unit-test failed: %s\n", __FILE__, __LINE__, #expr ); ntBreakpoint(); } } while ( 0 )
#	define UNIT_TEST_MSG( expr, msg )	do { if ( !(expr) ) { ntPrintf( "%s(%d): Unit-test failed: %s\n", __FILE__, __LINE__, #expr ); ntPrintf msg; ntBreakpoint(); } } while ( 0 )
#else
#	define UNIT_TEST( expr )			do {} while ( 0 )
#	define UNIT_TEST_MSG( expr, msg )	do {} while ( 0 )
#endif

//*****************************************************************************
//	The exposed unit-test function.
//*****************************************************************************
void VecMaths_Private::UnitTest()
{
	UnitTestIntrinsics();
	UnitTestVector();
	UnitTestDirection();
	UnitTestPoint();
	UnitTestQuat();
	UnitTestMatrix();
}

namespace	// anonymous namespace.
{

//*****************************************************************************
//	Static variables.
//*****************************************************************************
static const vector float test_vec1 = (vector float){ 1.0f, 2.0f, 3.0f, 4.0f };
static const vector float test_vec2 = (vector float){ 5.0f, 6.0f, 7.0f, 8.0f };

//*****************************************************************************
//	The intrinsics unit-test function.
//*****************************************************************************
void UnitTestIntrinsics()
{
	union Conv
	{
		vector float vec;
		float components[ 4 ];
	} res;

	res.vec = test_vec1;
//	ntPrintf( "test_vec1 = <%f, %f, %f, %f>\n", res.components[ 0 ], res.components[ 1 ], res.components[ 2 ], res.components[ 3 ] );

	res.vec = test_vec2;
//	ntPrintf( "test_vec2 = <%f, %f, %f, %f>\n", res.components[ 0 ], res.components[ 1 ], res.components[ 2 ], res.components[ 3 ] );

	res.vec = Intrinsics::SPU_DP4( test_vec1, test_vec2 );
//	ntPrintf( "dp4 = <%f, %f, %f, %f>\n", res.components[ 0 ], res.components[ 1 ], res.components[ 2 ], res.components[ 3 ] );

	UNIT_TEST( fabsf( res.components[ 0 ] - 70.0f ) < 0.00000001f );
	UNIT_TEST( fabsf( res.components[ 1 ] - 70.0f ) < 0.00000001f );
	UNIT_TEST( fabsf( res.components[ 2 ] - 70.0f ) < 0.00000001f );
	UNIT_TEST( fabsf( res.components[ 3 ] - 70.0f ) < 0.00000001f );

	res.vec = Intrinsics::SPU_DP3( test_vec1, test_vec2 );
//	ntPrintf( "dp3 = <%f, %f, %f, %f>\n", res.components[ 0 ], res.components[ 1 ], res.components[ 2 ], res.components[ 3 ] );

	UNIT_TEST( fabsf( res.components[ 0 ] - 38.0f ) < 0.00000001f );
	UNIT_TEST( fabsf( res.components[ 1 ] - 38.0f ) < 0.00000001f );
	UNIT_TEST( fabsf( res.components[ 2 ] - 38.0f ) < 0.00000001f );
	UNIT_TEST( fabsf( res.components[ 3 ] - 38.0f ) < 0.00000001f );
}

//*****************************************************************************
//	The CVector unit-test function.
//*****************************************************************************
void UnitTestVector()
{
	CVector v1( test_vec1 );
	CVector v2( test_vec2 );

	UNIT_TEST( fabsf( v1.Length() - 5.477225575f ) < 0.00001f );
	UNIT_TEST( fabsf( v1.LengthSquared() - 30.0f ) < 0.0000001f );
	UNIT_TEST( fabsf( v2.Length() - 13.19090596f ) < 0.00001f );
	UNIT_TEST( fabsf( v2.LengthSquared() - 174.0f ) < 0.0000001f );

	UNIT_TEST( v1.X() == 1.0f );
	UNIT_TEST( v1.Y() == 2.0f );
	UNIT_TEST( v1.Z() == 3.0f );
	UNIT_TEST( v1.W() == 4.0f );

	CVector v3 = v1 + v2;
	UNIT_TEST( fabsf( v3.LengthSquared() - 344.0f ) < 0.00000001f );

	UNIT_TEST( fabsf( v1.Dot( v2 ) - 70.0f ) < 0.000001f );

	v3 = ( v1 - 2.0f * v2 ).Abs();
	UNIT_TEST( v3.X() == 9.0f );
	UNIT_TEST( v3.Y() == 10.0f );
	UNIT_TEST( v3.Z() == 11.0f );
	UNIT_TEST( v3.W() == 12.0f );

	v3 = v2;
	v3 -= v1;
	UNIT_TEST( v3.X() == 4.0f );
	UNIT_TEST( v3.Y() == 4.0f );
	UNIT_TEST( v3.Z() == 4.0f );
	UNIT_TEST( v3.W() == 4.0f );

	v3 *= 1.5f;
	UNIT_TEST( v3.X() == 6.0f );
	UNIT_TEST( v3.Y() == 6.0f );
	UNIT_TEST( v3.Z() == 6.0f );
	UNIT_TEST( v3.W() == 6.0f );

	v3 /= 3.0f;
	UNIT_TEST( fabsf( v3.X() - 2.0f ) < 0.000000001f );
	UNIT_TEST( fabsf( v3.Y() - 2.0f ) < 0.000000001f );
	UNIT_TEST( fabsf( v3.Z() - 2.0f ) < 0.000000001f );
	UNIT_TEST( fabsf( v3.W() - 2.0f ) < 0.000000001f );

	v3 += v1;
	UNIT_TEST( fabsf( v3.LengthSquared() - 86.0f ) < 0.00000001f );

	v3 = -v3;
	UNIT_TEST( v3.X() == -3.0f );
	UNIT_TEST( v3.Y() == -4.0f );
	UNIT_TEST( v3.Z() == -5.0f );
	UNIT_TEST( v3.W() == -6.0f );

	v3 /= v1;
	UNIT_TEST( v3.X() == -3.0f );
	UNIT_TEST( v3.Y() == -2.0f );
	UNIT_TEST( v3.Z() == -5.0f / 3.0f );
	UNIT_TEST( v3.W() == -1.5f );

	v3 = v1.Min( v2 );
	UNIT_TEST( v3.X() == 1.0f );
	UNIT_TEST( v3.Y() == 2.0f );
	UNIT_TEST( v3.Z() == 3.0f );
	UNIT_TEST( v3.W() == 4.0f );

	v3 = v1.Max( v2 );
	UNIT_TEST( v3[ 0 ] == 5.0f );
	UNIT_TEST( v3[ 1 ] == 6.0f );
	UNIT_TEST( v3[ 2 ] == 7.0f );
	UNIT_TEST( v3[ 3 ] == 8.0f );

	v3 = CVector( CONSTRUCT_CLEAR );
	UNIT_TEST( v3.X() == 0.0f );
	UNIT_TEST( v3.Y() == 0.0f );
	UNIT_TEST( v3.Z() == 0.0f );
	UNIT_TEST( v3.W() == 0.0f );

	v2.Clear();
	UNIT_TEST( v2.X() == 0.0f );
	UNIT_TEST( v2.Y() == 0.0f );
	UNIT_TEST( v2.Z() == 0.0f );
	UNIT_TEST( v2.W() == 0.0f );
}

//*****************************************************************************
//	The CDirection unit-test function.
//*****************************************************************************
void UnitTestDirection()
{
	CDirection v1( test_vec1 );
	CDirection v2( test_vec2 );

	UNIT_TEST( fabsf( v1.Length() - 3.741657387f ) < 0.00001f );
	UNIT_TEST( fabsf( v1.LengthSquared() - 14 ) < 0.0000001f );
	UNIT_TEST( fabsf( v2.Length() - 10.48808848f ) < 0.00001f );
	UNIT_TEST( fabsf( v2.LengthSquared() - 110.0f ) < 0.0000001f );

	UNIT_TEST( v1.X() == 1.0f );
	UNIT_TEST( v1.Y() == 2.0f );
	UNIT_TEST( v1.Z() == 3.0f );

	CDirection v3 = v1 + v2;
	UNIT_TEST( fabsf( v3.LengthSquared() - 200.0f ) < 0.00000001f );

	UNIT_TEST( fabsf( v1.Dot( v2 ) - 38.0f ) < 0.000001f );

	v3 = ( v1 - 2.0f * v2 ).Abs();
	UNIT_TEST( v3.X() == 9.0f );
	UNIT_TEST( v3.Y() == 10.0f );
	UNIT_TEST( v3.Z() == 11.0f );

	v3 = v2;
	v3 -= v1;
	UNIT_TEST( v3.X() == 4.0f );
	UNIT_TEST( v3.Y() == 4.0f );
	UNIT_TEST( v3.Z() == 4.0f );

	v3 *= 1.5f;
	UNIT_TEST( v3.X() == 6.0f );
	UNIT_TEST( v3.Y() == 6.0f );
	UNIT_TEST( v3.Z() == 6.0f );

	v3 /= 3.0f;
	UNIT_TEST( fabsf( v3.X() - 2.0f ) < 0.000000001f );
	UNIT_TEST( fabsf( v3.Y() - 2.0f ) < 0.000000001f );
	UNIT_TEST( fabsf( v3.Z() - 2.0f ) < 0.000000001f );

	v3 += v1;
	UNIT_TEST( fabsf( v3.LengthSquared() - 50.0f ) < 0.00000001f );

	v3 = -v3;
	UNIT_TEST( v3.X() == -3.0f );
	UNIT_TEST( v3.Y() == -4.0f );
	UNIT_TEST( v3.Z() == -5.0f );

	v3 = v1.Min( v2 );
	UNIT_TEST( v3.X() == 1.0f );
	UNIT_TEST( v3.Y() == 2.0f );
	UNIT_TEST( v3.Z() == 3.0f );

	v3 = v1.Max( v2 );
	UNIT_TEST( v3[ 0 ] == 5.0f );
	UNIT_TEST( v3[ 1 ] == 6.0f );
	UNIT_TEST( v3[ 2 ] == 7.0f );

	v3 = CDirection( CONSTRUCT_CLEAR );
	UNIT_TEST( v3.X() == 0.0f );
	UNIT_TEST( v3.Y() == 0.0f );
	UNIT_TEST( v3.Z() == 0.0f );

	CPoint p1( v1 );
	UNIT_TEST( fabsf( v1.Dot( p1 ) - v1.LengthSquared() ) < 0.000000001f );
	UNUSED( p1 );

	v3 = v2;
//	ntPrintf( "<%f, %f, %f>\n", v3.X(), v3.Y(), v3.Z() );
	v3.Normalise();
//	ntPrintf( "<%f, %f, %f>\n", v3.X(), v3.Y(), v3.Z() );
	UNIT_TEST( fabsf( v3.Length() - 1.0f ) < 0.0000001f );

	v3 = v1.Cross( v2 );
	UNIT_TEST( fabsf( v3.X() - -4.0f ) < 0.00000001f );
	UNIT_TEST( fabsf( v3.Y() -  8.0f ) < 0.00000001f );
	UNIT_TEST( fabsf( v3.Z() - -4.0f ) < 0.00000001f );

	v2.Clear();
	UNIT_TEST( v2.X() == 0.0f );
	UNIT_TEST( v2.Y() == 0.0f );
	UNIT_TEST( v2.Z() == 0.0f );
}

//*****************************************************************************
//	The CPoint unit-test function.
//*****************************************************************************
void UnitTestPoint()
{
	CPoint v1( test_vec1 );
	CPoint v2( test_vec2 );

	UNIT_TEST( fabsf( v1.Length() - 3.741657387f ) < 0.00001f );
	UNIT_TEST( fabsf( v1.LengthSquared() - 14 ) < 0.0000001f );
	UNIT_TEST( fabsf( v2.Length() - 10.48808848f ) < 0.00001f );
	UNIT_TEST( fabsf( v2.LengthSquared() - 110.0f ) < 0.0000001f );

	UNIT_TEST( v1.X() == 1.0f );
	UNIT_TEST( v1.Y() == 2.0f );
	UNIT_TEST( v1.Z() == 3.0f );

	CPoint v3 = v1 + v2;
	UNIT_TEST( fabsf( v3.LengthSquared() - 200.0f ) < 0.00000001f );

	UNIT_TEST( fabsf( v1.Dot( v2 ) - 38.0f ) < 0.000001f );

	v3 = ( v1 - 2.0f * v2 ).Abs();
	UNIT_TEST( v3.X() == 9.0f );
	UNIT_TEST( v3.Y() == 10.0f );
	UNIT_TEST( v3.Z() == 11.0f );

	v3 = v2;
	v3 -= v1;
	UNIT_TEST( v3.X() == 4.0f );
	UNIT_TEST( v3.Y() == 4.0f );
	UNIT_TEST( v3.Z() == 4.0f );

	v3 *= 1.5f;
	UNIT_TEST( v3.X() == 6.0f );
	UNIT_TEST( v3.Y() == 6.0f );
	UNIT_TEST( v3.Z() == 6.0f );

	v3 /= 3.0f;
	UNIT_TEST( fabsf( v3.X() - 2.0f ) < 0.000000001f );
	UNIT_TEST( fabsf( v3.Y() - 2.0f ) < 0.000000001f );
	UNIT_TEST( fabsf( v3.Z() - 2.0f ) < 0.000000001f );

	v3 += v1;
	UNIT_TEST( fabsf( v3.LengthSquared() - 50.0f ) < 0.00000001f );

	v3 = -v3;
	UNIT_TEST( v3.X() == -3.0f );
	UNIT_TEST( v3.Y() == -4.0f );
	UNIT_TEST( v3.Z() == -5.0f );

	v3 = v1.Min( v2 );
	UNIT_TEST( v3.X() == 1.0f );
	UNIT_TEST( v3.Y() == 2.0f );
	UNIT_TEST( v3.Z() == 3.0f );

	v3 = v1.Max( v2 );
	UNIT_TEST( v3[ 0 ] == 5.0f );
	UNIT_TEST( v3[ 1 ] == 6.0f );
	UNIT_TEST( v3[ 2 ] == 7.0f );

	v3 = CPoint( CONSTRUCT_CLEAR );
	UNIT_TEST( v3.X() == 0.0f );
	UNIT_TEST( v3.Y() == 0.0f );
	UNIT_TEST( v3.Z() == 0.0f );

	CDirection d1( v1 );
	UNIT_TEST( fabsf( v1.Dot( d1 ) - v1.LengthSquared() ) < 0.000000001f );
	UNUSED( d1 );

	v2.Clear();
	UNIT_TEST( v2.X() == 0.0f );
	UNIT_TEST( v2.Y() == 0.0f );
	UNIT_TEST( v2.Z() == 0.0f );
}

//*****************************************************************************
//	The CQuat unit-test function.
//*****************************************************************************
void UnitTestQuat()
{
	CDirection d1( 1.0f, 0.0f, 0.0f );
	CDirection d2( 1.0f, 0.0f, 1.0f );

	d1.Normalise();
	d2.Normalise();

	CQuat q1( CONSTRUCT_IDENTITY );
	CQuat q2( d1, d2 );

	UNIT_TEST( fabsf( q2.W() - 0.923879532f ) < 0.0000001f );
	UNIT_TEST( fabsf( q2.X() ) < 0.0000001f );
	UNIT_TEST( fabsf( q2.Y() - -0.382683432f ) < 0.0000001f );
	UNIT_TEST( fabsf( q2.Z() ) < 0.0000001f );

//	ntPrintf( "q1 = <%f, %f, %f, %f>\n", q1.X(), q1.Y(), q1.Z(), q1.W() );
//	ntPrintf( "q2 = <%f, %f, %f, %f>\n", q2.X(), q2.Y(), q2.Z(), q2.W() );

	CQuat q3 = q1 * q2 * ~q1;
//	ntPrintf( "* q1*q2*~q1 = <%f, %f, %f, %f>\n", q3.X(), q3.Y(), q3.Z(), q3.W() );
	UNIT_TEST( fabsf( q3.W() - 0.923879532f ) < 0.0001f );
	UNIT_TEST( fabsf( q3.X() ) < 0.0000001f );
	UNIT_TEST( fabsf( q3.Y() - -0.382683432f ) < 0.0001f );
	UNIT_TEST( fabsf( q3.Z() ) < 0.0000001f );

	q3 = q1;
	q3 *= q2;
	q3 *= ~q1;
//	ntPrintf( "*= q1*q2*~q1 = <%f, %f, %f, %f>\n", q3.X(), q3.Y(), q3.Z(), q3.W() );
	UNIT_TEST( fabsf( q3.W() - 0.923879532f ) < 0.0001f );
	UNIT_TEST( fabsf( q3.X() ) < 0.0000001f );
	UNIT_TEST( fabsf( q3.Y() - -0.382683432f ) < 0.0001f );
	UNIT_TEST( fabsf( q3.Z() ) < 0.0000001f );

	q1 = CQuat( CDirection( 0.0f, 1.0f, 0.0f ), 45.0f * 3.1415926535f / 180.0f );
//	ntPrintf( "q1 = <%f, %f, %f, %f>\n", q1.X(), q1.Y(), q1.Z(), q1.W() );
//	ntPrintf( "q2 = <%f, %f, %f, %f>\n", q2.X(), q2.Y(), q2.Z(), q2.W() );
	q3 = q1 * q2;
//	ntPrintf( "q1*q2 = <%f, %f, %f, %f>\n", q3.X(), q3.Y(), q3.Z(), q3.W() );
	UNIT_TEST( fabsf( q3.W() - 1.0f ) < 0.0001f );
	UNIT_TEST( fabsf( q3[ 0 ] ) < 0.0001f );
	UNIT_TEST( fabsf( q3[ 1 ] ) < 0.0001f );
	UNIT_TEST( fabsf( q3[ 2 ] ) < 0.0001f );

	float dot = q1.Dot( q2 );
//	ntPrintf( "dot = %f\n", dot );
	UNIT_TEST( fabsf( dot - 0.707106781f ) < 0.00001f );
	UNUSED( dot );

	q1 = CQuat( CDirection( 0.0f, 1.0f, 0.0f ), 45.0f * 3.1415926535f / 180.0f );
	CDirection dir( 1.0f, 0.0f, 0.0f );
	CDirection res_dir = q1.Rotate( dir );
//	ntPrintf( "res_dir = <%f, %f, %f>\n", res_dir.X(), res_dir.Y(), res_dir.Z() );
	UNIT_TEST( fabsf( res_dir.X() - ( 1.0f / sqrtf( 2.0f ) ) ) < 0.001f );
	UNIT_TEST( fabsf( res_dir.Y() ) < 0.001f );
	UNIT_TEST( fabsf( res_dir.Z() + ( 1.0f / sqrtf( 2.0f ) ) ) < 0.001f );

	q1.Clear();
	UNIT_TEST( q1.X() == 0.0f );
	UNIT_TEST( q1.Y() == 0.0f );
	UNIT_TEST( q1.Z() == 0.0f );
	UNIT_TEST( q1.W() == 0.0f );

	q3.SetIdentity();
	UNIT_TEST( q3.X() == 0.0f );
	UNIT_TEST( q3.Y() == 0.0f );
	UNIT_TEST( q3.Z() == 0.0f );
	UNIT_TEST( q3.W() == 1.0f );
}

//*****************************************************************************
//	The CMatrix unit-test function.
//*****************************************************************************
void UnitTestMatrix()
{
	CQuat q( CDirection( 0.0f, 1.0f, 0.0f ), 45.0f * 3.1415926535f / 180.0f );
	
	CPoint translation( 0.0f, 10.0f, 0.0f );

	CMatrix mat( q, translation );

	CVector rows[ 4 ] =
	{
		CVector( mat.GetRow0() ),
		CVector( mat.GetRow1() ),
		CVector( mat.GetRow2() ),
		CVector( mat.GetRow3() )
	};
	UNUSED( rows );

	float recip_sqrt_2 = 1.0f / sqrtf( 2.0f );
	UNUSED( recip_sqrt_2 );

	UNIT_TEST( fabsf( rows[ 0 ].X() - recip_sqrt_2 ) < 0.0001f );
	UNIT_TEST( fabsf( rows[ 0 ].Y() ) < 0.0001f );
	UNIT_TEST( fabsf( rows[ 0 ].Z() + recip_sqrt_2 ) < 0.0001f );
	UNIT_TEST( fabsf( rows[ 0 ].W() ) < 0.0001f );

	UNIT_TEST( fabsf( rows[ 1 ].X() ) < 0.0001f );
	UNIT_TEST( fabsf( rows[ 1 ].Y() - 1.0f ) < 0.0001f );
	UNIT_TEST( fabsf( rows[ 1 ].Z() ) < 0.0001f );
	UNIT_TEST( fabsf( rows[ 1 ].W() ) < 0.0001f );

	UNIT_TEST( fabsf( rows[ 2 ].X() - recip_sqrt_2 ) < 0.0001f );
	UNIT_TEST( fabsf( rows[ 2 ].Y() ) < 0.0001f );
	UNIT_TEST( fabsf( rows[ 2 ].Z() - recip_sqrt_2 ) < 0.0001f );
	UNIT_TEST( fabsf( rows[ 2 ].W() ) < 0.0001f );

	UNIT_TEST( rows[ 3 ].X() == 0.0f );
	UNIT_TEST( rows[ 3 ].Y() == 10.0f );
	UNIT_TEST( rows[ 3 ].Z() == 0.0f );
	UNIT_TEST( rows[ 3 ].W() == 1.0f );
}

}	// anonymous namespace.

#undef UNIT_TEST

#endif // ENABLE_SPU_VECTORMATHS_UNITTESTS
