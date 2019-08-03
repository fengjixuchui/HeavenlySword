//--------------------------------------------------
//!
//!	\file vecmath_spu_soa_ps3.h
//!	Crappy SOA math lib. Needs re-factoring and moving
//! some functionality from CPoint_SOA to CVector_SOA
//! 
//--------------------------------------------------


#ifndef VECMATH_SPU_SOA_PS3_H
#define VECMATH_SPU_SOA_PS3_H

#ifndef VECMATH_SPU_PS3_H_
#include "vecmath_spu_ps3.h"
#endif

class CPoint_SOA
{
public:
    CPoint_SOA(v128 vec0, v128 vec1, v128 vec2, v128 vec3)
    {
        v128 tmp0, tmp1, tmp2, tmp3;
        tmp0 = spu_shuffle( vec0, vec2, VECTORMATH_SHUF_XAYB );
        tmp1 = spu_shuffle( vec1, vec3, VECTORMATH_SHUF_XAYB );
        tmp2 = spu_shuffle( vec0, vec2, VECTORMATH_SHUF_ZCWD );
        tmp3 = spu_shuffle( vec1, vec3, VECTORMATH_SHUF_ZCWD );
        m_X = spu_shuffle( tmp0, tmp1, VECTORMATH_SHUF_XAYB );
        m_Y = spu_shuffle( tmp0, tmp1, VECTORMATH_SHUF_ZCWD );
        m_Z = spu_shuffle( tmp2, tmp3, VECTORMATH_SHUF_XAYB );
    }

    CPoint_SOA(v128 vec)
    {
        vec_uchar16 shuffle_xxxx = (vec_uchar16)spu_splats((int)0x00010203);
        vec_uchar16 shuffle_yyyy = (vec_uchar16)spu_splats((int)0x04050607);
        vec_uchar16 shuffle_zzzz = (vec_uchar16)spu_splats((int)0x08090a0b);

        m_X = spu_shuffle( vec, vec, shuffle_xxxx );
        m_Y = spu_shuffle( vec, vec, shuffle_yyyy );
        m_Z = spu_shuffle( vec, vec, shuffle_zzzz );
    }


	CPoint_SOA( v128 xxxx, v128 yyyy, v128 zzzz )
	{
		m_X = xxxx;
		m_Y = yyyy;
		m_Z = zzzz;
	}

	CPoint_SOA()
	{
		m_X = g_float4_allzero;
		m_Y = g_float4_allzero;
		m_Z = g_float4_allzero;
	}

	CPoint_SOA( const CPoint_SOA& other )
	{
		m_X = other.m_X;
		m_Y = other.m_Y;
		m_Z = other.m_Z;
	}

    v128 X() const
    {
        return m_X;
    }
    v128 Y() const
    {
        return m_Y;
    }
    v128 Z() const
    {
        return m_Z;
    }

	// ref access
	v128& X()
    {
        return m_X;
    }
    v128& Y()
    {
        return m_Y;
    }
    v128& Z()
    {
        return m_Z;
    }


	void operator=( const CPoint_SOA& other )
	{
		m_X = other.m_X;
		m_Y = other.m_Y;
		m_Z = other.m_Z;
	}


    inline static v128 Dot(CPoint_SOA const& lhs, CPoint_SOA const& rhs)
    {
        v128 result;
        result = spu_mul( lhs.X(), rhs.X() );
		result = spu_add( result, spu_mul( lhs.Y(), rhs.Y() ) );
		result = spu_add( result, spu_mul( lhs.Z(), rhs.Z() ) );
        return result;
    }

	inline static CPoint_SOA Cross( const CPoint_SOA& lhs, const CPoint_SOA& rhs )
	{
		CPoint_SOA result;

		result.X() = spu_sub( spu_mul( lhs.Y(), rhs.Z() ), spu_mul( lhs.Z(), rhs.Y() ) );
		result.Y() = spu_sub( spu_mul( lhs.Z(), rhs.X() ), spu_mul( lhs.X(), rhs.Z() ) );
		result.Z() = spu_sub( spu_mul( lhs.X(), rhs.Y() ), spu_mul( lhs.Y(), rhs.X() ) );

		return result;
	}



	inline void operator-=( const CPoint_SOA& other )
	{
		m_X = spu_sub( m_X, other.m_X );
		m_Y = spu_sub( m_Y, other.m_Y );
		m_Z = spu_sub( m_Z, other.m_Z );
	}

	inline void operator+=( const CPoint_SOA& other )
	{
		m_X = spu_add( m_X, other.m_X );
		m_Y = spu_add( m_Y, other.m_Y );
		m_Z = spu_add( m_Z, other.m_Z );
	}


	inline void ComponentMul( v128 v )
	{
		v128 v_xxxx = spu_splats( spu_extract( v, 0 ) );
		v128 v_yyyy = spu_splats( spu_extract( v, 1 ) );
		v128 v_zzzz = spu_splats( spu_extract( v, 2 ) );

		m_X = spu_mul( m_X, v_xxxx );
		m_Y = spu_mul( m_Y, v_yyyy );
		m_Z = spu_mul( m_Z, v_zzzz );
	}


	inline void ComponentAdd( v128 v )
	{
		v128 v_xxxx = spu_splats( spu_extract( v, 0 ) );
		v128 v_yyyy = spu_splats( spu_extract( v, 1 ) );
		v128 v_zzzz = spu_splats( spu_extract( v, 2 ) );

		m_X = spu_add( m_X, v_xxxx );
		m_Y = spu_add( m_Y, v_yyyy );
		m_Z = spu_add( m_Z, v_zzzz );
	}


	inline void ComponentSub( v128 v )
	{
		v128 v_xxxx = spu_splats( spu_extract( v, 0 ) );
		v128 v_yyyy = spu_splats( spu_extract( v, 1 ) );
		v128 v_zzzz = spu_splats( spu_extract( v, 2 ) );

		m_X = spu_sub( m_X, v_xxxx );
		m_Y = spu_sub( m_Y, v_yyyy );
		m_Z = spu_sub( m_Z, v_zzzz );
	}


	inline v128 LengthSqr( void ) const
	{
		v128 result;
		result = spu_mul( m_X, m_X );
		result = spu_add( result, spu_mul( m_Y, m_Y ) );
		result = spu_add( result, spu_mul( m_Z, m_Z ) );
        return result;
	}


	inline v128 Length( void ) const
	{
		return Intrinsics::SPU_Sqrt( LengthSqr() );
	}


	inline void Normalise( void )
	{
		v128 lengthInv = Intrinsics::SPU_RSqrt( LengthSqr() );

		m_X = spu_mul( m_X, lengthInv );
		m_Y = spu_mul( m_Y, lengthInv );
		m_Z = spu_mul( m_Z, lengthInv );
	}

	inline void DeSwizzle( CPoint& result0, CPoint& result1, CPoint& result2, CPoint& result3 ) const
	{
		v128 tmp0, tmp1;
		tmp0 = spu_shuffle( m_X, m_Z, VECTORMATH_SHUF_XAYB );
		tmp1 = spu_shuffle( m_X, m_Z, VECTORMATH_SHUF_ZCWD );
		result0 = CPoint( spu_shuffle( tmp0, m_Y, VECTORMATH_SHUF_XAYB ) );
		result1 = CPoint( spu_shuffle( tmp0, m_Y, VECTORMATH_SHUF_ZBW0 ) );
		result2 = CPoint( spu_shuffle( tmp1, m_Y, VECTORMATH_SHUF_XCY0 ) );
		result3 = CPoint( spu_shuffle( tmp1, m_Y, VECTORMATH_SHUF_ZDW0 ) );
	}

	inline void DeSwizzle( v128& result0, v128& result1, v128& result2, v128& result3 ) const
	{
		v128 tmp0, tmp1;
		tmp0 = spu_shuffle( m_X, m_Z, VECTORMATH_SHUF_XAYB );
		tmp1 = spu_shuffle( m_X, m_Z, VECTORMATH_SHUF_ZCWD );
		result0 = spu_shuffle( tmp0, m_Y, VECTORMATH_SHUF_XAYB );
		result1 = spu_shuffle( tmp0, m_Y, VECTORMATH_SHUF_ZBW0 );
		result2 = spu_shuffle( tmp1, m_Y, VECTORMATH_SHUF_XCY0 );
		result3 = spu_shuffle( tmp1, m_Y, VECTORMATH_SHUF_ZDW0 );
	}

private:



private:
    v128 m_X;
    v128 m_Y;
    v128 m_Z;
};


inline static CPoint_SOA operator-( const CPoint_SOA& lhs, const CPoint_SOA& rhs )
{
	return CPoint_SOA
		(
		spu_sub( lhs.X(), rhs.X() ),
		spu_sub( lhs.Y(), rhs.Y() ),
		spu_sub( lhs.Z(), rhs.Z() )
		);
}

inline static CPoint_SOA operator+( const CPoint_SOA& lhs, const CPoint_SOA& rhs )
{
	return CPoint_SOA
		( 
		spu_add( lhs.X(), rhs.X() ),
		spu_add( lhs.Y(), rhs.Y() ),
		spu_add( lhs.Z(), rhs.Z() )
		);
}



class CQuat_SOA
{
public:
	CQuat_SOA( const CQuat& quat )
	{
		vec_uchar16 shuffle_xxxx = (vec_uchar16)spu_splats((int)0x00010203);
		vec_uchar16 shuffle_yyyy = (vec_uchar16)spu_splats((int)0x04050607);
		vec_uchar16 shuffle_zzzz = (vec_uchar16)spu_splats((int)0x08090a0b);
		vec_uchar16 shuffle_wwww = (vec_uchar16)spu_splats((int)0x0c0d0e0f);
		

		m_X = spu_shuffle( quat.QuadwordValue(), quat.QuadwordValue(), shuffle_xxxx );
		m_Y = spu_shuffle( quat.QuadwordValue(), quat.QuadwordValue(), shuffle_yyyy );
		m_Z = spu_shuffle( quat.QuadwordValue(), quat.QuadwordValue(), shuffle_zzzz );
		m_W = spu_shuffle( quat.QuadwordValue(), quat.QuadwordValue(), shuffle_wwww );
	}

	CQuat_SOA( const CQuat_SOA& other )
	{
		m_X = other.X();
		m_Y = other.Y();
		m_Z = other.Z();
		m_W = other.W();
	}

	CQuat_SOA( const CQuat& quat0, const CQuat& quat1, const CQuat& quat2, const CQuat& quat3 )
    {
        v128 tmp0, tmp1, tmp2, tmp3;
        tmp0 = spu_shuffle( quat0.QuadwordValue(), quat2.QuadwordValue(), VECTORMATH_SHUF_XAYB );
        tmp1 = spu_shuffle( quat1.QuadwordValue(), quat3.QuadwordValue(), VECTORMATH_SHUF_XAYB );
        tmp2 = spu_shuffle( quat0.QuadwordValue(), quat2.QuadwordValue(), VECTORMATH_SHUF_ZCWD );
        tmp3 = spu_shuffle( quat1.QuadwordValue(), quat3.QuadwordValue(), VECTORMATH_SHUF_ZCWD );
        m_X = spu_shuffle( tmp0, tmp1, VECTORMATH_SHUF_XAYB );
        m_Y = spu_shuffle( tmp0, tmp1, VECTORMATH_SHUF_ZCWD );
        m_Z = spu_shuffle( tmp2, tmp3, VECTORMATH_SHUF_XAYB );
		m_W = spu_shuffle( tmp2, tmp3, VECTORMATH_SHUF_ZCWD );
    }

	CQuat_SOA( v128 xxxx, v128 yyyy, v128 zzzz, v128 wwww )
	{
		m_X = xxxx;
		m_Y = yyyy;
		m_Z = zzzz;
		m_W = wwww;
	}

	CQuat_SOA()
	{
		m_X = g_float4_allzero;
		m_Y = g_float4_allzero;
		m_Z = g_float4_allzero;
		m_W = g_float4_allzero;
	}

	v128 X() const
    {
        return m_X;
    }
    v128 Y() const
    {
        return m_Y;
    }
    v128 Z() const
    {
        return m_Z;
    }

	 v128 W() const
    {
        return m_W;
    }

	// ref access
	v128& X()
    {
        return m_X;
    }
    v128& Y()
    {
        return m_Y;
    }
    v128& Z()
    {
        return m_Z;
    }
	 v128& W()
    {
        return m_W;
    }
	

	inline CQuat_SOA operator ~ ( void ) const
	{
		using namespace Intrinsics;
		return CQuat_SOA( SPU_Negate( X() ), SPU_Negate( Y() ), SPU_Negate( Z() ), W() );
	}

private:
    v128 m_X;
    v128 m_Y;
    v128 m_Z;
	v128 m_W;
};


inline const CPoint_SOA Rotate( const CPoint_SOA & vec, const CQuat_SOA & quat )
{
	v128 tmpX, tmpY, tmpZ, tmpW;
	tmpX = spu_sub( spu_add( spu_mul( quat.W(), vec.X() ), spu_mul( quat.Y(), vec.Z() ) ), spu_mul( quat.Z(), vec.Y() ) );
	tmpY = spu_sub( spu_add( spu_mul( quat.W(), vec.Y() ), spu_mul( quat.Z(), vec.X() ) ), spu_mul( quat.X(), vec.Z() ) );
	tmpZ = spu_sub( spu_add( spu_mul( quat.W(), vec.Z() ), spu_mul( quat.X(), vec.Y() ) ), spu_mul( quat.Y(), vec.X() ) );
	tmpW = spu_add( spu_add( spu_mul( quat.X(), vec.X() ), spu_mul( quat.Y(), vec.Y() ) ), spu_mul( quat.Z(), vec.Z() ) );
	
	return CPoint_SOA(
		spu_add( spu_sub( spu_add( spu_mul( tmpW, quat.X() ), spu_mul( tmpX, quat.W() ) ), spu_mul( tmpY, quat.Z() ) ), spu_mul( tmpZ, quat.Y() ) ),
		spu_add( spu_sub( spu_add( spu_mul( tmpW, quat.Y() ), spu_mul( tmpY, quat.W() ) ), spu_mul( tmpZ, quat.X() ) ), spu_mul( tmpX, quat.Z() ) ),
		spu_add( spu_sub( spu_add( spu_mul( tmpW, quat.Z() ), spu_mul( tmpZ, quat.W() ) ), spu_mul( tmpX, quat.Y() ) ), spu_mul( tmpY, quat.X() ) )
	);
}

#endif

