/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#include <hkbase/hkBase.h>
#include <hkbase/fwd/hkcstdio.h>
using namespace std;

class hkPrintfStreamWriter : public hkStreamWriter
{
	public:

		hkPrintfStreamWriter(hkStreambufFactory::StdStream s)
		{
		}

		virtual int write( const void* buf, int nbytes)
		{
			// careful about accessing buf[-1]
			if( nbytes > 0 )
			{
				const char* cbuf = static_cast<const char*>(buf);
				if ( cbuf[nbytes-1] == HK_NULL)
				{
					printf(static_cast<const char*>(buf));
				}
				else // need to null terminate
				{
					hkArray<char> wbuf(int(nbytes+1));
					hkArray<char>::copy( &wbuf[0], static_cast<const char*>(buf), int(nbytes));
					wbuf[int(nbytes)] = '\0';
					printf( &wbuf[0] ); 
				}
			}
			return nbytes;
		}

		virtual hkBool isOk() const
		{
			return true;
		}
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
