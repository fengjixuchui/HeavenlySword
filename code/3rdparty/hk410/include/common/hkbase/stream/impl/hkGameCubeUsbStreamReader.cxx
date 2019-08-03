/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#include <hkbase/hkBase.h>
#include <hkbase/fwd/hkcstdio.h>

#include <dolphin.h>
#include <dolphin/fio.h>

HK_COMPILE_TIME_ASSERT(hkStreamReader::STREAM_SET==FIO_SEEK_SET
					&& hkStreamReader::STREAM_CUR==FIO_SEEK_CUR
					&& hkStreamReader::STREAM_END==FIO_SEEK_END );


// defined in hkGameCubeUsbWriter.cxx
extern hkResult hkGameCubeUsbInit();


/// An EXI2USB based File IO reader for Gamecube
class hkGameCubeUsbReader : public hkStreamReader
{
	public:

		hkGameCubeUsbReader(const char* fname, OpenMode mode)
			:	m_fileHandle(HK_NULL),
				m_fileOffset(0),
				m_fileLength(0)
		{
			// make sure MCC, FIO, etc is initialized
			if( hkGameCubeUsbInit() == HK_SUCCESS )
			{
				// open file
				m_fileHandle = FIOFopen(fname, FIO_OPEN_RDONLY );
				if (m_fileHandle != FIO_INVALID_HANDLE)
				{
					m_fileLength = FIOFseek(m_fileHandle, 0, FIO_SEEK_LAST);
					// seek to start of file
					FIOFseek(m_fileHandle, 0, FIO_SEEK_TOP);
				}
				else
				{
					hkcerr << "HK GCN FIO: Can't open file " << fname << hkendl;
					hkcerr << "HK GCN FIO: Note: Files can only be opened once!" << hkendl;
				}
			}
		}
    
		// close file
		void close()
		{
			if(m_fileHandle != HK_NULL)
			{
				FIOFclose(m_fileHandle);
				m_fileHandle = HK_NULL;
			}
		}

		virtual ~hkGameCubeUsbReader()
		{
			close();			
		}

		virtual hkBool isOk() const
		{
			return m_fileHandle != HK_NULL;
		}

		virtual int read( void* buf, int nbytes )
		{
			HK_ASSERT(0x47d42bd5, m_fileHandle != HK_NULL);

			// check for EOF
			if (m_fileOffset >= m_fileLength)
			{
				return 0;
			}

			// check for overshoot
			if ((m_fileOffset + nbytes) > m_fileLength)
			{
				nbytes = m_fileLength - m_fileOffset;
				
				// 32 byte allign
				nbytes = OSRoundUp32B(nbytes);
			}
			
			int numRead = FIOFread(m_fileHandle, buf, nbytes);
			if (numRead > 0)
			{
				m_fileOffset += numRead;
				return numRead;
			}
			else
			{
				int fioError = FIOGetLastError();
				hkcerr << "HK GCN FIO: Read failed with error #" << fioError << hkendl;
				close();
				return 0;
			}
		}
		
		virtual hkBool seekTellSupported() const
		{
			return true;
		}

		virtual hkResult seek( int offset, SeekWhence whence)
		{
			HK_ASSERT(0x4db3353c, m_fileHandle != HK_NULL);
			m_fileOffset = FIOFseek(m_fileHandle, offset, whence);
			return m_fileOffset != unsigned(-1)
				? HK_SUCCESS
				: HK_FAILURE;
		}

		virtual int tell() const
		{
			return static_cast<int>( FIOFseek(m_fileHandle, 0, SEEK_CUR) );
		}

	protected:

		FIOHandle m_fileHandle;
		unsigned m_fileOffset;
		unsigned m_fileLength;
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
