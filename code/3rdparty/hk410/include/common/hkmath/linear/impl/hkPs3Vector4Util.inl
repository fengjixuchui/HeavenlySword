/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#if defined(HK_PLATFORM_PS3SPU)
#   define HK_VECTOR4UTIL_convertToUint16
    inline void HK_CALL hkVector4Util::convertToUint16( const hkVector4& in, const hkVector4& offset, const hkVector4& scale, hkIntUnion64& out)
    {
        hkQuadReal inoff = spu_add( in.getQuad(), offset.getQuad() );
        inoff = spu_mul( inoff, scale.getQuad() );
        vector signed int v32 = spu_convts( inoff, 0 );
		out.quad = (hkQuadReal)spu_shuffle( v32, v32, (vector unsigned char){2,3,6,7,10,11,14,15, 0,0,0,0,0,0,0,0 } );
    }

#   define HK_VECTOR4UTIL_convertToUint16WithClip
    inline void HK_CALL hkVector4Util::convertToUint16WithClip( const hkVector4& in, const hkVector4& offset, const hkVector4& scale, const hkVector4& min, const hkVector4& max, hkIntUnion64& out)
    {
        hkVector4 t;
        t.setMax4( in, min );
        t.setMin4( t,  max );
        convertToUint16( t, offset, scale, out);
    }

#   define HK_VECTOR4UTIL_convertToUint32
    inline void HK_CALL hkVector4Util::convertToUint32( const hkVector4& in, const hkVector4& offset, const hkVector4& scale, hkUint32* out)
    {
        hkQuadReal inoff = spu_add( in.getQuad(), offset.getQuad() );
        inoff = spu_mul( inoff, scale.getQuad() );
        vector signed int v32 = spu_convts( inoff, 0 );
        *(vector signed int*)(out) = v32;
    }

#   define HK_VECTOR4UTIL_convertToUint32WithClip
    inline void HK_CALL hkVector4Util::convertToUint32WithClip( const hkVector4& in, const hkVector4& offset, const hkVector4& scale, const hkVector4& min, const hkVector4& max, hkUint32* out)
    {
        hkVector4 t;
        t.setMax4( in, min );
        t.setMin4( t,  max );
        convertToUint32( t, offset, scale, out);
    }
#endif

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
