/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkmath/hkMath.h>
#include <hkmath/linear/hkVector4Util.h>
#include <hkbase/htl/hkAlgorithm.h>

#if !defined(HK_PLATFORM_PS3SPU)
hkBool hkMatrix4::isOk() const
{
	const hkReal* f = reinterpret_cast<const hkReal*>(this);
	for(int i=0; i<16; ++i)
	{
		if( hkMath::isFinite(f[i]) == false )
		{
			return false;
		}
	}
	return true;
}
#endif

void hkMatrix4::transpose()
{
	hkMatrix4& m = *this;
	hkAlgorithm::swap( m(0,1), m(1,0) );
	hkAlgorithm::swap( m(0,2), m(2,0) );
	hkAlgorithm::swap( m(0,3), m(3,0) );

	hkAlgorithm::swap( m(1,2), m(2,1) );
	hkAlgorithm::swap( m(1,3), m(3,1) );
	hkAlgorithm::swap( m(2,3), m(3,2) );
}

void hkMatrix4::setTranspose( const hkMatrix4& s )
{
	hkMatrix4& d = *this;

	d(0,0) = s(0,0);
	d(1,1) = s(1,1);
	d(2,2) = s(2,2);
	d(3,3) = s(3,3);

	d(3,0) = 0.0f;
	d(3,1) = 0.0f;
	d(3,2) = 0.0f;
	d(3,3) = 1.0f;

	d(1,0) = s(0,1);	
	d(0,1) = s(1,0);	
	
	d(2,0) = s(0,2);	
	d(0,2) = s(2,0);	
	
	d(3,0) = s(0,3);	
	d(0,3) = s(3,0);	

	d(2,1) = s(1,2);	
	d(1,2) = s(2,1);	
	
	d(3,1) = s(1,3);
	d(1,3) = s(3,1);

	d(3,2) = s(2,3);
	d(2,3) = s(3,2);

}

// aTc = aTb * bTc
void hkMatrix4::setMul( const hkMatrix4& a, const hkMatrix4& b )
{
	HK_ASSERT(0x6d9d1d43,  this != &a );
	HK_ASSERT(0x64a8df81,  this != &b );

	hkMatrix4 aTrans = a;
	aTrans.transpose();

	hkMatrix4& result = *this;

	for (int c=0; c < 4; c++)
	{
		for (int r=0; r < 4; r++)
		{
			result(r,c) = aTrans.getColumn(r).dot4( b.getColumn(c) );
		}
	}
}

void hkMatrix4::setMul( hkSimdRealParameter scale, const hkMatrix4& a)
{
	HK_ASSERT(0x53e345d4,  this != &a);

	getColumn(0).setMul4(scale, a.getColumn(0));
	getColumn(1).setMul4(scale, a.getColumn(1));
	getColumn(2).setMul4(scale, a.getColumn(2));
	getColumn(3).setMul4(scale, a.getColumn(3));
}

void hkMatrix4::setMulInverse( const hkMatrix4& a, const hkMatrix4& b )
{
	hkMatrix4 binverse = b;
	binverse.invert( HK_REAL_EPSILON );
	setMul( a, binverse );
}

#if !defined(HK_PLATFORM_PS3SPU)
hkBool hkMatrix4::isApproximatelyEqual( const hkMatrix4& m, hkReal zero) const
{
	return	   m_col0.equals3( m.getColumn(0), zero )
			&& m_col1.equals3( m.getColumn(1), zero )
			&& m_col2.equals3( m.getColumn(2), zero )
			&& m_col3.equals3( m.getColumn(3), zero );
}


hkResult hkMatrix4::invert(hkReal epsilon)
{
	hkMatrix4& m = *this;

    hkReal fA0 = m(0,0)*m(1,1) - m(0,1)*m(1,0);
    hkReal fA1 = m(0,0)*m(1,2) - m(0,2)*m(1,0);
    hkReal fA2 = m(0,0)*m(1,3) - m(0,3)*m(1,0);
    hkReal fA3 = m(0,1)*m(1,2) - m(0,2)*m(1,1);
    hkReal fA4 = m(0,1)*m(1,3) - m(0,3)*m(1,1);
    hkReal fA5 = m(0,2)*m(1,3) - m(0,3)*m(1,2);
    hkReal fB0 = m(2,0)*m(3,1) - m(2,1)*m(3,0);
    hkReal fB1 = m(2,0)*m(3,2) - m(2,2)*m(3,0);
    hkReal fB2 = m(2,0)*m(3,3) - m(2,3)*m(3,0);
    hkReal fB3 = m(2,1)*m(3,2) - m(2,2)*m(3,1);
    hkReal fB4 = m(2,1)*m(3,3) - m(2,3)*m(3,1);
    hkReal fB5 = m(2,2)*m(3,3) - m(2,3)*m(3,2);

    hkReal det = fA0*fB5 - fA1*fB4 + fA2*fB3 + fA3*fB2 - fA4*fB1 + fA5*fB0;
    hkSimdReal invDet = 1.0f/det;

	if ( hkMath::fabs(det) <= epsilon )
	{
        return HK_FAILURE;
	}

    hkMatrix4 temp;
    temp(0,0) = + m(1,1)*fB5 - m(1,2)*fB4 + m(1,3)*fB3;
    temp(1,0) = - m(1,0)*fB5 + m(1,2)*fB2 - m(1,3)*fB1;
    temp(2,0) = + m(1,0)*fB4 - m(1,1)*fB2 + m(1,3)*fB0;
    temp(3,0) = - m(1,0)*fB3 + m(1,1)*fB1 - m(1,2)*fB0;
    temp(0,1) = - m(0,1)*fB5 + m(0,2)*fB4 - m(0,3)*fB3;
    temp(1,1) = + m(0,0)*fB5 - m(0,2)*fB2 + m(0,3)*fB1;
    temp(2,1) = - m(0,0)*fB4 + m(0,1)*fB2 - m(0,3)*fB0;
    temp(3,1) = + m(0,0)*fB3 - m(0,1)*fB1 + m(0,2)*fB0;
    temp(0,2) = + m(3,1)*fA5 - m(3,2)*fA4 + m(3,3)*fA3;
    temp(1,2) = - m(3,0)*fA5 + m(3,2)*fA2 - m(3,3)*fA1;
    temp(2,2) = + m(3,0)*fA4 - m(3,1)*fA2 + m(3,3)*fA0;
    temp(3,2) = - m(3,0)*fA3 + m(3,1)*fA1 - m(3,2)*fA0;
    temp(0,3) = - m(2,1)*fA5 + m(2,2)*fA4 - m(2,3)*fA3;
    temp(1,3) = + m(2,0)*fA5 - m(2,2)*fA2 + m(2,3)*fA1;
    temp(2,3) = - m(2,0)*fA4 + m(2,1)*fA2 - m(2,3)*fA0;
    temp(3,3) = + m(2,0)*fA3 - m(2,1)*fA1 + m(2,2)*fA0;

	setMul(invDet, temp);
 
    return HK_SUCCESS;
}
#endif


void hkMatrix4::add( const hkMatrix4& a )
{
	getColumn(0).add4( a.getColumn(0) );
	getColumn(1).add4( a.getColumn(1) );
	getColumn(2).add4( a.getColumn(2) );
	getColumn(3).add4( a.getColumn(3) );
}

void hkMatrix4::sub( const hkMatrix4& a )
{
	getColumn(0).sub4( a.getColumn(0) );
	getColumn(1).sub4( a.getColumn(1) );
	getColumn(2).sub4( a.getColumn(2) );
	getColumn(3).sub4( a.getColumn(3) );
}

void hkMatrix4::mul( const hkMatrix4& a )
{

	hkMatrix4 temp;
	temp.setMul( *this, a );
	*this = temp;
}

void hkMatrix4::mul( hkSimdRealParameter scale)
{
	getColumn(0).mul4(scale);
	getColumn(1).mul4(scale);
	getColumn(2).mul4(scale);
	getColumn(3).mul4(scale);
}

#if !defined(HK_PLATFORM_PS3SPU)
void hkMatrix4::get4x4RowMajor(hkReal* d) const
{
	const hkReal* p = &m_col0(0);
	for (int i = 0; i<4; i++)
	{
		d[0] = p[0];
		d[4] = p[1];
		d[8] = p[2];
		d[12] = p[3];
		d+= 1;
		p+= 4;
	}
}

void hkMatrix4::set4x4RowMajor(const hkReal* p)
{
	hkReal* d = &m_col0(0);
	for (int i = 0; i<4; i++)
	{
		d[0] = p[0];
		d[1] = p[4];
		d[2] = p[8];
		d[3] = p[12];
		p+= 1;
		d+= 4;
	}
}

void hkMatrix4::get4x4ColumnMajor(hkReal* d) const
{
	const hkReal* p = &m_col0(0);
	for (int i = 0; i<4; i++)
	{
		d[0] = p[0];
		d[1] = p[1];
		d[2] = p[2];
		d[3] = p[3];
		d+= 4;
		p+= 4;
	}
}

void hkMatrix4::set4x4ColumnMajor(const hkReal* p)
{
	hkReal* d = &m_col0(0);
	for (int i = 0; i<4; i++)
	{
		d[0] = p[0];
		d[1] = p[1];
		d[2] = p[2];
		d[3] = p[3];
		d+= 4;
		p+= 4;
	}
}
void hkMatrix4::set(const hkTransform& t)
{
	const hkRotation& r = t.getRotation();
	m_col0 = r.getColumn(0);
	m_col1 = r.getColumn(1);
	m_col2 = r.getColumn(2);
	m_col3 = t.getTranslation();
	m_col0(3) = 0;
	m_col1(3) = 0;
	m_col2(3) = 0;
	m_col3(3) = 1;
}
#endif

HK_ALIGN16( hkReal hkMatrix4Identity[] ) =
{
	1,0,0,0,
	0,1,0,0,
	0,0,1,0,
	0,0,0,1
};

/*
* Havok SDK - PUBLIC RELEASE, BUILD(#20060902)
*
* Confidential Information of Havok.  (C) Copyright 1999-2006 
* Telekinesys Research Limited t/a Havok. All Rights Reserved. The Havok
* Logo, and the Havok buzzsaw logo are trademarks of Havok.  Title, ownership
* rights, and intellectual property rights in the Havok software remain in
* Havok and/or its suppliers.
*
* Use of this software for evaluation purposes is subject to and indicates 
* acceptance of the End User licence Agreement for this product. A copy of 
* the license is included with this software and is also available from salesteam@havok.com.
*
*/
