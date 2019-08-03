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

HK_COMPILE_TIME_ASSERT(hkStreamWriter::STREAM_SET==FIO_SEEK_SET
					&& hkStreamWriter::STREAM_CUR==FIO_SEEK_CUR
					&& hkStreamWriter::STREAM_END==FIO_SEEK_END );

extern hkResult hkGameCubeUsbInit();


/// An EXI2USB based File IO Streambuf for Gamecube
class hkGameCubeUsbWriter : public hkStreamWriter
{
	public:

		hkGameCubeUsbWriter(const char* fname)
			:	m_fileHandle(HK_NULL),
				m_fileOffset(0)
		{
			// make sure MCC, FIO, etc is initialized
			if( hkGameCubeUsbInit() == HK_SUCCESS )
			{
				// open file
				m_fileHandle = FIOFopen(fname, FIO_OPEN_WRONLY | FIO_OPEN_CREAT);
				if (m_fileHandle == FIO_INVALID_HANDLE)
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
				flush();
				FIOFclose(m_fileHandle);
				m_fileHandle = HK_NULL;
			}
		}

		virtual ~hkGameCubeUsbWriter()
		{
			close();			
		}

		virtual hkBool isOk() const
		{
			return m_fileHandle != HK_NULL;
		}


		virtual int write( const void* buf, int nbytes )
		{
			HK_ASSERT(0x3373366a, m_fileHandle != HK_NULL);
			
			// 32 byte align
			int nbytesToWrite = OSRoundUp32B(nbytes);
			
			// flush CPU cache
			DCFlushRange( const_cast<void*>(buf), nbytes);
			
			int numWritten = FIOFwrite(m_fileHandle, const_cast<void*>(buf), nbytesToWrite);

			if( numWritten > 0 ) // success
			{
				// only move forward by the original number of bytes
				m_fileOffset += nbytes;
				
				// move the write pointer back to the original number of buyes 
				FIOFseek(m_fileHandle, m_fileOffset, FIO_SEEK_TOP);
				
				return numWritten;
			}
			else
			{
				int fioError = FIOGetLastError();
				hkcerr << "HK GCN FIO: Write failed with error #" << fioError << hkendl;
				close();
				return 0;
			}
		}

		virtual void flush()
		{
			HK_ASSERT(0x7618add8, m_fileHandle != HK_NULL);
			FIOFflush(m_fileHandle);
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
};


//
// Usb file operations init
//

static int  g_exiChannel = -1;
static hkBool g_exiEnumerated  = false;
static hkBool g_mccInitialized = false;
static hkBool g_fioInitialized = false;
static OSThreadQueue fioThreadQueue;

//
//	MCC callbacks
//
static BOOL hioEnumCallback(s32 channel)
{	
	g_exiChannel = channel;
	g_exiEnumerated = true;
	OSWakeupThread(&fioThreadQueue);
	return false;
}

static void mccCallback(MCCSysEvent event)
{
	if (event == MCC_SYSEVENT_INITIALIZED)
	{
		g_mccInitialized = true;
		OSWakeupThread(&fioThreadQueue);
	}
}

hkResult hkGameCubeUsbInit()
{
	if( !g_mccInitialized )
	{
		// enum EXI interface
		if( MCCEnumDevices(hioEnumCallback) )
		{
			// init thread queue
			OSInitThreadQueue(&fioThreadQueue);
			
			// spin until EXI enumerated
			hkprintf("HK GCN FIO: Waiting for EXI enumeration...\n");
			while (!g_exiEnumerated)
			{
				// spin until enumerated
				OSSleepThread(&fioThreadQueue);
			}
			hkprintf("HK GCN FIO: EXI enumeration complete, EXI channel# = %d! \n", g_exiChannel);
		}
		else
		{
			hkcerr <<"HK GCN FIO: Can't detect EXI Adapter!" << hkendl;
			return HK_FAILURE;
		}
		
		// init MCC communication layer
		if ( MCCInit((MCCExiChannel) g_exiChannel, 0, mccCallback) )
		{
			// spin until MCC initialised
			hkprintf("HK GCN FIO: Waiting for MCC init by host...\n");
			while (!g_mccInitialized)
			{
				// spin until initialized
				OSSleepThread(&fioThreadQueue);
			}
			hkprintf("HK GCN FIO: MCC initialized!\n");
		}
		else
		{
			hkcerr << "HK GCN FIO: MCC communication not available" << hkendl;
			return HK_FAILURE;
		}
	}
	
	// make sure FIO is initialized
	if( !g_fioInitialized )
	{
		// init FIO communication layer
		if ( FIOInit((MCCExiChannel) g_exiChannel, MCC_CHANNEL_1, 10) )
		{
			// spin util host available
			hkprintf("HK GCN FIO: Waiting for FIO connection by host...\n");
			while (!g_fioInitialized)
			{
				// wait until FIO connected
				g_fioInitialized = FIOQuery();
			}
			hkprintf("HK GCN FIO: FIO Connection established!\n");
		}
		else
		{
			hkcerr << "HK GCN FIO: FIO communication not available." << hkendl;
			return HK_FAILURE;
		}
	}
	return HK_SUCCESS;
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
