/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

HK_FORCE_INLINE hkBitField::hkBitField( int numBits )
:	m_numWords( ( numBits + 31 ) >> 5 )
{
	m_words = new hkUint32[m_numWords];
}

HK_FORCE_INLINE hkBitField::hkBitField( int numBits, int initialValue )
:	m_numWords( ( numBits + 31 ) >> 5 )
{
	HK_ASSERT( 0xa63ab345, initialValue >= 0 && initialValue <= 1 );

	m_words = new hkUint32[m_numWords];

	assignAll( initialValue );
}

HK_FORCE_INLINE hkBitField::~hkBitField()
{
	delete [] m_words;
}

HK_FORCE_INLINE int hkBitField::get( int index )
{
	HK_ASSERT( 0x48d17bd3, index >= 0 && (index >> 5) < m_numWords );

	int arrayIndex = index >> 5;
	return ( ( m_words[arrayIndex] >> ( index & 0x1f ) ) & 1 );
}

HK_FORCE_INLINE void hkBitField::set( int index )
{
	HK_ASSERT( 0x48a97bc3, index >= 0 && (index >> 5) < m_numWords );

	int arrayIndex = index >> 5;
	m_words[arrayIndex] |= ( 1 << ( index & 0x1f ) );
}

HK_FORCE_INLINE void hkBitField::clear( int index )
{
	HK_ASSERT( 0x38a87bb3, index >= 0 && (index >> 5) < m_numWords );

	int arrayIndex = index >> 5;
	m_words[arrayIndex] &= ~( 1 << ( index & 0x1f ) );
}

HK_FORCE_INLINE void hkBitField::assign( int index, int value )
{
	HK_ASSERT( 0x48a27b13, index >= 0 && (index >> 5) < m_numWords );
	HK_ASSERT( 0xe68bb345, value >= 0 && value <= 1 );

	// this is kind of complicated but avoids a branch

	int arrayIndex = index >> 5;
	hkUint32 word = m_words[arrayIndex];
	int upperShift = index & 0x1f;
	int lowerShift = 32 - upperShift;

	// get the upper bits aligned to the right
	hkUint32 upperBits = ( word >> upperShift );

	// insert the new bit
	upperBits &= 0xfffe;
	upperBits |= value;

	// isolate the lower bits
	hkUint32 lowerBits = ( word << lowerShift ) >> lowerShift;

	// merge the lower with the upper (including the new bit)
	m_words[arrayIndex] = ( upperBits << upperShift ) | lowerBits;
}

HK_FORCE_INLINE void hkBitField::assignAll( int value )
{
	HK_ASSERT( 0xa59289bb, value >= 0 && value <= 1 );

	hkUint32 fill = value ? 0xffff : 0;

	for( int i = 0; i < m_numWords; i++ )
	{
		m_words[i] = fill;
	}
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
