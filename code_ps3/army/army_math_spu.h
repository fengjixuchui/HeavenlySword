/***************************************************************************************************
*
*	DESCRIPTION		A unit consists of number of a a single type of grunts under command of a 
*					Battalion
*
*	NOTES
*
***************************************************************************************************/

#ifndef ARMY_MATH_SPU_H_
#define ARMY_MATH_SPU_H_

#define ARMY_PIE 3.14159f


inline v128 Cross2D( v128 p, v128 q )
{
	v128 qs = spu_shuffle( q, q, VECTORMATH_MAKE_SHUFFLE( VECTORMATH_SHUF_Y, VECTORMATH_SHUF_X, VECTORMATH_SHUF_W, VECTORMATH_SHUF_Z ) );
	v128 m = spu_mul( p, qs );
	v128 ms = spu_shuffle( m,m, VECTORMATH_MAKE_SHUFFLE( VECTORMATH_SHUF_Y, VECTORMATH_SHUF_X, VECTORMATH_SHUF_W, VECTORMATH_SHUF_Z ) );
	return spu_sub( m, ms );
}

// this does uses the 4tuple vector to do 2 simulatiosly (and unconnected Dot2D)
// the dot of x,y is in x,y and the dot of z,y is in z,w
inline v128 Dot2D( v128 a, v128 b )
{
	v128 ab = spu_mul( a, b );
	return spu_add( ab, spu_shuffle( ab, ab, VECTORMATH_MAKE_SHUFFLE( VECTORMATH_SHUF_Y, VECTORMATH_SHUF_X, VECTORMATH_SHUF_W, VECTORMATH_SHUF_Z) ) ); 
}

inline v128 Magnitude2DFast( v128 a )
{
	return sqrtf4fast( Dot2D(a,a) );
}

inline v128 Normalise2DFast( v128 a )
{
	return spu_mul(a, rsqrtf4fast( Dot2D(a,a) ) );
}

inline v128 Magnitude2D( v128 a )
{
	return sqrtf4( Dot2D(a,a) );
}

inline v128 Normalise2D( v128 a )
{
	return spu_mul(a, rsqrtf4( Dot2D(a,a) ) );
}
inline v128 PerpCW2D( v128 a )
{
	// nx = y;
	// ny = -x

	v128 as = spu_shuffle( a, a, VECTORMATH_MAKE_SHUFFLE( VECTORMATH_SHUF_Y, VECTORMATH_SHUF_X, VECTORMATH_SHUF_W, VECTORMATH_SHUF_Z ) );
	// this is a negate on the two original X channels only, leaving the Y alone
	return (v128)spu_xor( as, ( (vector float)(vector signed int){ 0x00000000, 0x80000000, 0x00000000, 0x80000000 } ) );
}

inline v128 Vec2ToOrientation( v128 a )
{
	static const v128 quarter = (vector float){ 0.25f, 0.25f, 0.25f, 0.25f }; 
	static const v128 three_quarter = (vector float){ 0.75f, 0.75f, 0.75f, 0.75f }; 
	static const v128 neg_quarter = (vector float){ -0.25f, -0.25f, -0.25f, -0.25f };
	static const v128 upVec = (vector float){ 0.f, 1.0f, 0.f, 1.0f };

	v128 d = Dot2D( a, upVec );

	if( spu_extract(a,0) < 0 )
	{
		// return ((a dot upvec)-1)* (-0.25)
		// aka (a dot upvec)*-0.25 +0.25
		return spu_madd( d, neg_quarter, quarter );
	} else
	{
		// return 0.5 + ((a dot upvec)+1)*(0.25)
		// aka 0.5 + (a dot upvec)*0.25 -0.25
		// aka (a dot upvec)*0.25 + 0.75
		return spu_madd( d, quarter, three_quarter );
	}

}

inline v128 Orientation2Vec( v128 a )
{

	float fOtheta = spu_extract( a, 0 ) * 2 * ARMY_PIE;
	float fdX = -1 * sin( fOtheta );
	float fdY = 1 * cos( fOtheta);

	return (vector float){ fdX, fdY, fdX, fdY };
}
#endif
