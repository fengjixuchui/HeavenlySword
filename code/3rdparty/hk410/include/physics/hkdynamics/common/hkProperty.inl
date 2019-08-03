/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

inline void hkPropertyValue::setReal( const hkReal r )
{
	// Use a union to ensure that the hkReal is
	// always stored in the LSB end of m_data
	// regardless of platform endianness
	union {
		hkReal r;
		hkUint32 u;
	} u;
	u.r = r;
	m_data = u.u;
}

inline void hkPropertyValue::setInt( const int i )
{
	m_data = i;
}

inline void hkPropertyValue::setPtr( void* p )
{
	m_data = reinterpret_cast<hkUlong>(p);
}

// constructors here for inlining

inline hkPropertyValue::hkPropertyValue( const int i )
{ 
	setInt(i);
}

inline hkPropertyValue::hkPropertyValue( const hkReal r )
{ 
	setReal(r);
}

inline hkPropertyValue::hkPropertyValue( void* p )
{ 
	setPtr(p);
}


inline hkReal hkPropertyValue::getReal() const
{
	// see comment in setReal
	union {
		hkReal r;
		hkUint32 u;
	} u;
	u.u = hkUint32(m_data);
	return u.r;
}

inline int hkPropertyValue::getInt() const
{
	return static_cast<int>(m_data);
}

inline void* hkPropertyValue::getPtr() const
{
	return reinterpret_cast<void*>(static_cast<hkUlong>(m_data));
}

inline hkProperty::hkProperty() 
{
}

inline hkProperty::hkProperty( hkUint32 key, hkInt32 value ) 
:	m_key(key), m_value(value)
{ 
}

inline hkProperty::hkProperty( hkUint32 key, hkPropertyValue value ) 
:	m_key(key), m_value(value)
{ 
}

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
