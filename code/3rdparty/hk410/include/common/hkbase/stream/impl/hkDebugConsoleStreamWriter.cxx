/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#include <hkbase/hkBase.h>

#if defined(HK_PLATFORM_WIN32)
#	define WIN32_LEAN_AND_MEAN
#	include <windows.h>
#elif defined(HK_PLATFORM_XBOX) || defined(HK_PLATFORM_XBOX360)
#	include <xtl.h>
#else
#	error debug console is only for win32 or xbox
#endif

/// StreamWriter which prints output using OutputDebugString.
/// Unfortunately there is no easy way to check whether OutputDebugString
/// has succeeded (win9x issues) so console output will simply
/// disappear when not debugging.
class hkDebugConsoleStreamWriter : public hkStreamWriter
{
	public:

		hkDebugConsoleStreamWriter(hkStreambufFactory::StdStream s)
		{
		}
		

		virtual int write(const void* buf, int nbytes)
		{
			// careful about accessing buf[-1]
			if(nbytes!=0)
			{
				const char* cbuf = static_cast<const char*>(buf);
				if(cbuf[nbytes-1] == HK_NULL)
				{
					OutputDebugString( cbuf );
				#if !defined(HK_PLATFORM_XBOX) && !defined(HK_PLATFORM_XBOX360) // as printf is routed through debug out anyway
					printf("%s", cbuf);
				#endif
				}
				else
				{
					hkArray<char> wbuf(nbytes+1);
					hkArray<char>::copy( &wbuf[0], cbuf, nbytes);
					wbuf[nbytes] = '\0';
					OutputDebugString( &wbuf[0] );
				#if !defined(HK_PLATFORM_XBOX) && !defined(HK_PLATFORM_XBOX360) // as printf is routed through debug out anyway
					printf("%s", &wbuf[0]);
				#endif
				}
			}

			return nbytes;
		}

		hkBool isOk() const
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
