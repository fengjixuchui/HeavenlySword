/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

//
// String Internal Representation
//


char* hkString::Rep::getData()
{
	return reinterpret_cast<char*>(this+1);
}

const char* hkString::Rep::getData() const
{
	return reinterpret_cast<const char*>(this+1);
}

char* hkString::Rep::refCopy()
{
	++m_references;
	return getData();
}

int hkString::Rep::getSize() const
{
	return hkSizeOf(Rep)+m_capacity+1;
}

void hkString::Rep::unRef()
{
	if(--m_references < 0)
	{
		freeMemory();
	}
}

//
// private methods
//
HK_FORCE_INLINE hkString::hkString(Rep* r)
	: m_string(r->getData())
{
}

// inline methods used by other inlines come first

HK_FORCE_INLINE hkString::Rep& hkString::getRep() const
{
	return *(reinterpret_cast<Rep*>(m_string)-1);
}

HK_FORCE_INLINE hkString::Rep& hkString::getEmptyStringRep()
{
	return *reinterpret_cast<Rep*>(s_emptyStringRep);
}

HK_FORCE_INLINE int hkString::getLength() const
{
	return getRep().m_length;
}

//
// constructors
//

HK_FORCE_INLINE hkString::hkString()
{
	m_string = getEmptyStringRep().refCopy();
}

HK_FORCE_INLINE hkString::hkString(const hkString& other)
{
	m_string = other.getRep().refCopy();
}

HK_FORCE_INLINE hkString::hkString(const char* s)
{
	if( s != HK_NULL )
	{
		int slen = strLen(s);
		m_string = Rep::create(slen)->getData();
		hkString::memCpy(m_string, s, slen+1); // copy null too
	}
	else
	{
		m_string = getEmptyStringRep().refCopy();
	}	
}

HK_FORCE_INLINE hkString::hkString(const char* buf, int len)
{
	HK_ASSERT(0x1b0d49f3, len >= 0);
	m_string = Rep::create(len)->getData();
	hkString::memCpy(m_string, buf, len);
	m_string[len] = 0; // null terminate
}

HK_FORCE_INLINE hkString& hkString::operator= (const hkString& other)
{
	char* newStr = other.getRep().refCopy();
	getRep().unRef();
	m_string = newStr;
	return *this;
}
		
HK_FORCE_INLINE hkString& hkString::operator= (const char* s)
{
	if(s != HK_NULL && *s != 0)
	{
		int slen = strLen(s);
		// create new rep if current one is too small or shared.
		if(getRep().m_capacity < slen || getRep().m_references > 0)
		{
			getRep().unRef();
			m_string = Rep::create(slen)->getData();
		}
		hkString::memCpy(m_string, s, slen+1); // copy null too
		getRep().m_length = slen;
	}
	else
	{
		getRep().unRef();
		m_string = getEmptyStringRep().refCopy();
	}
	return *this;
}

HK_FORCE_INLINE hkString::~hkString()
{
	getRep().unRef();
}

HK_FORCE_INLINE char hkString::operator[] (int index) const
{
	return m_string[index];
}

HK_FORCE_INLINE const char* hkString::cString() const
{
	return m_string;
}

HK_FORCE_INLINE int hkString::compareTo(const hkString& other) const
{
	return strCmp( cString(), other.cString() );
}
HK_FORCE_INLINE int hkString::compareTo(const char* other) const
{
	return strCmp( cString(), other );
}

HK_FORCE_INLINE int hkString::compareToIgnoreCase(const hkString& other) const
{
	return strCasecmp( cString(), other.cString() );
}

HK_FORCE_INLINE int hkString::compareToIgnoreCase(const char* other) const
{
	return strCasecmp( cString(), other );
}

HK_FORCE_INLINE hkBool hkString::operator< (const hkString& other) const
{
	return compareTo(other) < 0;
}

HK_FORCE_INLINE hkBool hkString::operator== (const hkString& other) const
{
	return compareTo(other) == 0;
}
HK_FORCE_INLINE hkBool hkString::operator!= (const hkString& other) const
{
	return compareTo(other) != 0;
}
HK_FORCE_INLINE hkBool hkString::operator== (const char* other) const
{
	return compareTo(other) == 0;
}
HK_FORCE_INLINE hkBool hkString::operator!= (const char* other) const
{
	return compareTo(other) != 0;
}

void HK_CALL hkString::memCpy4( void* dst, const void* src, int numWords)
{
	const hkUint32* src32 = reinterpret_cast<const hkUint32*>(src);
	hkUint32* dst32       = reinterpret_cast<      hkUint32*>(dst);
	{
		for (int i = 0; i < numWords; i++)
		{
			*(dst32++) = *(src32++);
		}
	}
}

void HK_CALL hkString::memCpy16( void* dst, const void* src, int numQuads)
{
#if defined (HK_PLATFORM_PS3) || defined (HK_PLATFORM_PS3SPU)
	const vector signed int* srcQuad = reinterpret_cast<const vector signed int*>(src);
	vector signed int* dstQuad = reinterpret_cast<vector signed int*>(dst);
	{
		for (int i = 0; i < numQuads; i++)
		{
			*(dstQuad++) = *(srcQuad++);
		}
	}
#else
	HK_ASSERT2( 0xf021d445, (hkUlong(dst) & 0xf) == 0, "Unaligned address" );
	HK_ASSERT2( 0xf021d446, (hkUlong(src) & 0xf) == 0, "Unaligned address" );
	const hkUint32* src32 = reinterpret_cast<const hkUint32*>(src);
	hkUint32* dst32 = reinterpret_cast<      hkUint32*>(dst);
	{
		for (int i = 0; i < numQuads; i++)
		{
			dst32[0] = src32[0];
			dst32[1] = src32[1];
			dst32[2] = src32[2];
			dst32[3] = src32[3];
			dst32+= 4;
			src32+= 4;
		}
	}
#endif
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
