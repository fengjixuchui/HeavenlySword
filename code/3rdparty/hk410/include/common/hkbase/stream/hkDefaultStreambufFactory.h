/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_DEAFULT_STREAMBUF_FACTORY
#define HK_DEAFULT_STREAMBUF_FACTORY 

#include <hkbase/hkBase.h>
#include <hkbase/stream/hkStreamReader.h>
#include <hkbase/stream/hkStreamWriter.h>
#include <hkbase/stream/hkStreambufFactory.h>
#include <hkbase/stream/impl/hkBufferedStreamReader.h>
#include <hkbase/stream/impl/hkBufferedStreamWriter.h>

#if defined(HK_PLATFORM_XBOX) || defined(HK_PLATFORM_XBOX360)
#	include <hkbase/stream/impl/hkStdioStreamReader.cxx>
#	include <hkbase/stream/impl/hkStdioStreamWriter.cxx>
	typedef hkStdioStreamWriter DefaultFileWriter;
	typedef hkStdioStreamReader DefaultFileReader;
#elif defined(HK_PLATFORM_GC)
#	include <hkbase/stream/impl/hkGameCubeDvdStreamReader.cxx>
#	include <hkbase/stream/impl/hkPrintfStreamWriter.cxx>
	class hkNullStreamWriter : public hkStreamWriter
	{
		public:
		
			hkNullStreamWriter(const char* name) { }
			virtual int write(const void*, int) { return 0; }
			virtual hkBool isOk() const { return false; }
	};
	typedef hkGameCubeDvdReader DefaultFileReader;
	typedef hkNullStreamWriter DefaultFileWriter;
#elif defined(HK_PLATFORM_PSP) || defined(HK_PLATFORM_PS2)
#	include <hkbase/stream/impl/hkPosixStreamReader.cxx>
#	include <hkbase/stream/impl/hkPosixStreamWriter.cxx>
	typedef hkPosixStreamWriter DefaultFileWriter;
	typedef hkPosixStreamReader DefaultFileReader;
#elif defined(HK_PLATFORM_UNIX)
#	include <hkbase/stream/impl/hkPosixStreamReader.cxx>
#	include <hkbase/stream/impl/hkPosixStreamWriter.cxx>
	typedef hkPosixStreamWriter DefaultFileWriter;
	typedef hkPosixStreamReader DefaultFileReader;
#elif defined(HK_PLATFORM_WIN32)
#	include <hkbase/stream/impl/hkStdioStreamReader.cxx>
#	include <hkbase/stream/impl/hkStdioStreamWriter.cxx>
	typedef hkStdioStreamWriter DefaultFileWriter;
	typedef hkStdioStreamReader DefaultFileReader;
#elif defined(HK_PLATFORM_PS3)
#	include <hkbase/stream/impl/hkStdioStreamReader.cxx>
#	include <hkbase/stream/impl/hkStdioStreamWriter.cxx>
	typedef hkStdioStreamWriter DefaultFileWriter;
	typedef hkStdioStreamReader DefaultFileReader;
#else
#	include <hkbase/stream/impl/hkStdioStreamReader.cxx>
#	include <hkbase/stream/impl/hkStdioStreamWriter.cxx>
	typedef hkStdioStreamWriter DefaultFileWriter;
	typedef hkStdioStreamReader DefaultFileReader;
#endif
	
class hkDefaultStreambufFactory : public hkStreambufFactory
{
	public:

		virtual hkStreamReader* openReader( const char* name )
		{
			hkStreamReader* s = new DefaultFileReader(name);
			if( s->markSupported() == false )
			{
				hkStreamReader* b = new hkBufferedStreamReader(s);
				s->removeReference();
				return b;
			}
			return s;
		}

		virtual hkStreamWriter* openWriter( const char* name )
		{
			hkStreamWriter* s = new DefaultFileWriter(name);
			hkStreamWriter* b = new hkBufferedStreamWriter(s);
			s->removeReference();
			return b;
		}

		static hkReferencedObject* create()
		{
			return new hkDefaultStreambufFactory();
		}
};

#endif // HK_DEAFULT_STREAMBUF_FACTORY 


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
