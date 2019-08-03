//------------------------------------------------------------------------------------------
//!
//!	\file game/capturesystem.cpp
//!	Implementation of the CaptureSystem
//!
//! Capture system is purely responsible for writing out a sequence of images or frames to
//! an AVI or even a single image. It takes care of automatic naming etc...
//!
//------------------------------------------------------------------------------------------

#include "core/timer.h"
#include "game/capturesystem.h"
#include "gfx/graphicsdevice.h"
#include "gfx/renderer.h"

#include <sys/types.h>
#include <sys/stat.h>


#ifndef PLATFORM_PS3
#include <direct.h>
#include "game/avigenerator.h"
#else
#include <cell/fs/cell_fs_file_api.h>
#include <sys/fs.h>
#endif


//------------------------------------------------------------------------------------------
//!
//!	CaptureSystem::CaptureSystem
//!	Construction - Default
//!
//------------------------------------------------------------------------------------------
CaptureSystem::CaptureSystem() :
	m_bInitialised(false),
	m_eMode(IMAGE),
	m_bCapturing(false),
	m_iFramesCaptured(0),
	m_obFileName("Capture"),
	m_obDirName("Capture"),
	m_fFrameRate(30),
#ifndef PLATFORM_PS3
	m_pobAVIGenerator(0),
#endif
	m_pLogFile(0),
	m_iFileIndex(-1),
	m_iDirIndex(-1),
	m_iFrameIndex(0),
	m_iDirInc(0),
	m_iFileInc(0),
	m_iFramesToWrite(-1)
{
}

//------------------------------------------------------------------------------------------
//!
//!	CaptureSystem::~CaptureSystem
//!	Destructor - Default
//!
//------------------------------------------------------------------------------------------
CaptureSystem::~CaptureSystem()
{
	// End it all...
	StopCapture();
}


//------------------------------------------------------------------------------------------
//!
//!	CaptureSystem::ToggleCaptureMode
//!	Toggle the capture system
//! \return COMMAND_RESULT indicating success
//!
//------------------------------------------------------------------------------------------
COMMAND_RESULT CaptureSystem::ToggleCaptureMode()
{
	// Toggle Capture
	m_bCapturing = !m_bCapturing;
	// Call the appropriate start/end function
	m_bCapturing ? StartCapture() : StopCapture();
	return CR_SUCCESS;
}

//------------------------------------------------------------------------------------------
//!
//!	CaptureSystem::StartCapture
//!	Start the capturing
//!
//------------------------------------------------------------------------------------------
void CaptureSystem::StartCapture()
{
	// Initialise the mode
	switch (m_eMode)
	{
	case AVI:
		InitialiseAVIWrite();
		break;
	case IMAGE:
		InitialiseImageWrite();
		break;
	default:
		break;
	};

	// Bail if it was not correctly initialised
	if (!m_bInitialised)
		return;

    // Set the timer to fixed framerate
	CTimer::m_bUsedFixedTimeChange = true;
	CTimer::m_fFixedTimeChange = 1.0f / m_fFrameRate;

	// Indicate that we are capturing
	m_bCapturing = true;
}

//------------------------------------------------------------------------------------------
//!
//!	CaptureSystem::StopCapture
//!	Stop the capturing
//!
//------------------------------------------------------------------------------------------
void CaptureSystem::StopCapture()
{
#ifndef PLATFORM_PS3
	// If we were movie writing, release the movie
	if (m_pobAVIGenerator)
	{
		m_pobAVIGenerator->ReleaseEngine();
		NT_DELETE( m_pobAVIGenerator );
		m_pobAVIGenerator = 0;
	}
#endif

	// Indicate non initialised
	m_bInitialised = false;

	// Indicate that we are no longer capturing
	m_bCapturing = false;

	// Close log
	LogWrite("Stopping Capture\n");
	LogWrite("----------------\n");

	// Close our logfile
	if (m_pLogFile) 
	{
		fclose(m_pLogFile);
		m_pLogFile = 0;
	}

	// Flag no frames to write (cleans up screenshot mode)
	m_iFramesToWrite = 0;
}

//------------------------------------------------------------------------------------------
//!
//!	CaptureSystem::Update
//!	Update the capture system
//! \param fTime - current time
//!
//------------------------------------------------------------------------------------------
void CaptureSystem::Update(float fTime)
{
	UNUSED(fTime);
	// See if it is valid to do anything
	if (m_bCapturing && m_bInitialised)
	{
		switch (m_eMode)
		{
		case AVI:
			UpdateAVIWrite();
			break;
		case IMAGE:
			UpdateImageWrite();
			break;
		default:
			break;
		};

		// Move to next frame
		m_iFramesCaptured++;

		// Process number of frames to write
		if (m_iFramesToWrite > 0)
		{
			m_iFramesToWrite--;
			if (m_iFramesToWrite == 0)
			{
				StopCapture();
			}
		}

	}
}

//------------------------------------------------------------------------------------------
//!
//!	CaptureSystem::SetCaptureMode
//! Set the capture mode
//! \param eMode - the capture mode to set
//!
//------------------------------------------------------------------------------------------
void CaptureSystem::SetCaptureMode(CAPTURE_MODE eMode)
{
	// Check that we aren't already capturing
	if (m_bCapturing)
	{
		ntError_p(0, ("Cannot change capture mode whilst capture is in progress"));
		return;
	}

	// Set the new mode
	m_eMode = eMode;
}

//------------------------------------------------------------------------------------------
//!
//!	CaptureSystem::InitialiseAviWrite
//!	Initialise AVI writing
//!
//------------------------------------------------------------------------------------------
void CaptureSystem::InitialiseAVIWrite()
{
	// Set filename increments
	m_iFileInc = 1;
	m_iDirInc = 0;

	m_iFrameIndex = 0;

#ifndef PLATFORM_PS3
	// Generate the dir name
	const char* obDirName = ntStr::GetString(GenerateNextDir());

	// Create the directory for the capture output
	if ( mkdir( obDirName) == ENOENT)
	{
		ntPrintf("Could not create capture directory");
		return;
	}

	// Create a bitmap info header to describe the input
	BITMAPINFOHEADER bmih;
	bmih.biSize = sizeof(BITMAPINFOHEADER);
	bmih.biWidth = (LONG)DisplayManager::Get().GetInternalWidth();
	bmih.biHeight = (LONG)DisplayManager::Get().GetInternalWidth();
	bmih.biPlanes = 1;
	bmih.biBitCount = 32;
	bmih.biCompression = BI_RGB;
	bmih.biSizeImage = 0;
	bmih.biXPelsPerMeter = 4096;
	bmih.biYPelsPerMeter = 4096;
	bmih.biClrUsed = 0;
	bmih.biClrImportant = 0;

	const char* obFileName = ntStr::GetString(GenerateNextFile(".avi"));

	// Create new generator
	m_pobAVIGenerator = NT_NEW CAVIGenerator(obFileName, &bmih, (int)m_fFrameRate);

	// Start the movie generation
	m_pobAVIGenerator->InitEngine();

	// Mark initialisation complete
	m_bInitialised = true;

	// Open log file
	m_pLogFile = fopen("capturelog.txt", "a+");
	LogWrite("Starting Capture\n");
	LogWrite("----------------\n");
	char cBuffer[256];
	sprintf(cBuffer, "Creating %s\n", obFileName);
	LogWrite(cBuffer);
#endif
}

//------------------------------------------------------------------------------------------
//!
//!	CaptureSystem::InitialiseImageWrite
//!	Initialise Image writing
//!
//------------------------------------------------------------------------------------------
void CaptureSystem::InitialiseImageWrite()
{
	// Set filename increments
	m_iFileInc = 1;
	// Increment directories ... unless in screenshot mode ( m_iFramesToWrite is 1)
	m_iDirInc = (m_iFramesToWrite == 1) ?	0 : 1;

	// Generate the dir name
	ntstd::String obDirName = GenerateNextDir();
#ifndef PLATFORM_PS3
	// Create the directory for the capture output
	if ( _mkdir( ntStr::GetString(obDirName)) == ENOENT)
	{
		ntPrintf("Could not create capture directory");
		return;
	}
#else
 	// Create the directory for the capture output
	char dirName[MAX_PATH];
	Util::GetFullGameDataFilePath( ntStr::GetString(obDirName), dirName );
 	CellFsErrno res = cellFsMkdir( dirName, CELL_FS_DEFAULT_CREATE_MODE_1 );
 	if ( res != CELL_FS_OK)
 	{
 		ntPrintf("Could not create capture directory - err no 0x%X, path = %s\n", res, dirName);
 		return;
 	}
#endif

	// Mark initialisation complete
	m_bInitialised = true;

	// Open log file
	m_pLogFile = fopen("capturelog.txt", "a+");
	LogWrite("Starting Capture\n");
	LogWrite("----------------\n");
	if (m_iFramesToWrite == 1)
		LogWrite("Creating screenshot...\n");
	else
		LogWrite("Creating stream...\n");
}

//------------------------------------------------------------------------------------------
//!
//!	CaptureSystem::UpdateAVIWrite
//!	Update AVI writing
//!
//------------------------------------------------------------------------------------------
void CaptureSystem::UpdateAVIWrite()
{
#ifndef PLATFORM_PS3

	// ---- Write a movie frame ---- 

	IDirect3DSurface9* pSurf;
	LPD3DXBUFFER pobBack = 0;
	bool bSuccess = false;
	char cBuffer[256];

	// Get surface
	HRESULT hr = GetD3DDevice()->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pSurf );
	if (!Util::DXSafeCheckError(hr, "AutoMovie GetBackBuffer() failed:- "))
	{
		// Save the BMP file to memory
		hr = D3DXSaveSurfaceToFileInMemory (&pobBack, D3DXIFF_BMP, pSurf, 0, 0);
		if (!Util::DXSafeCheckError(hr, "AutoMovie D3DXSaveSurfaceToFileInMemory() failed:- "))
		{
			// Get a uint8_t pointer to the actual address of the data
			uint8_t* pcTemp = (uint8_t*)pobBack->GetBufferPointer();
			// Add it to the movie
			m_pobAVIGenerator->AddFrame(pcTemp + ((LPBITMAPFILEHEADER)pcTemp)->bfOffBits);

			sprintf(cBuffer, "Writing frame #%05d\n", m_iFrameIndex);
			m_iFrameIndex++;

			LogWrite(cBuffer);

			// indicate success
			bSuccess = true;
		}
		// release resources
		pobBack->Release();
	}
#endif
}

//------------------------------------------------------------------------------------------
//!
//!	CaptureSystem::UpdateImageWrite
//!	Update Image writing
//!
//------------------------------------------------------------------------------------------
void CaptureSystem::UpdateImageWrite()
{
	// Find the next valid filename - including extension
	ntstd::String obFilename = GenerateNextFile(".dds");
	// Find the filename - excluding extension
	obFilename = CurrentFileName("");
	Renderer::Get().m_targetCache.GetPrimaryColourTarget()->SaveToDisk(ntStr::GetString(obFilename));
	LogWrite("Writing - ");
	LogWrite(ntStr::GetString(obFilename));
	LogWrite("\n");
}

//------------------------------------------------------------------------------------------
//!
//! CaptureSystem::LogWrite
//! Write text to logfile
//!
//------------------------------------------------------------------------------------------
void CaptureSystem::LogWrite(const char* pcText)
{
	// Check that we have a valid log file
	if (m_pLogFile)
	{
		// Write the text out
		int iLength = strlen(pcText);
		fwrite(pcText, sizeof(char), iLength, m_pLogFile);
	}
}


//------------------------------------------------------------------------------------------
//!
//! CaptureSystem::GenerateDirName
//! Generate the name to use for the directory
//! On PC, this function will iterate until it finds a directory with number that does not exist already
//! On PS3, it will write the directory number to file
//!
//------------------------------------------------------------------------------------------
ntstd::String CaptureSystem::GenerateNextDir()
{
	ntstd::String obDir;

#if 0

	File numberIn;
	numberIn.Open("capture.txt", File::FT_READ);
	if (numberIn.IsValid())
	{
		numberIn.Read(cBuffer, 256);
		numberIn.Close();
		m_iDirIndex = atoi(cBuffer);
	}
#endif

	// Do not iterate if no increment is set
	if (0 == m_iDirInc)
	{
		return CurrentDirName();
	}

	// Iterate for next available directory
	for(bool bLoop = true; bLoop;)
	{
		// Increment the directory index
		if (m_iDirInc > 0)
		{
			m_iDirIndex+= m_iDirInc;
			m_iFileIndex = 0;
		}

		// Generate dirname
		obDir = CurrentDirName();

#ifndef PLATFORM_PS3
		struct stat buf;
		bLoop = (0 == stat( ntStr::GetString(obDir), &buf ));
#else
		char aName[256];
		Util::GetFiosFilePath( ntStr::GetString(obDir), aName );
		int fd = 0;
		if (CELL_FS_OK == cellFsOpendir(aName, &fd))
		{
			cellFsClosedir(fd);
		}
		else
		{
			bLoop = false;
		}
#endif
	}

#ifdef PLATFORM_PS3
	// Write the current index to a file
	File numberOut;
	char cBuffer[256];
	sprintf(cBuffer, "%d\nThis file is autogenerated. Do not edit\n", m_iDirIndex);
	numberOut.Open("capture.txt", File::FT_WRITE);
	numberOut.Write(cBuffer, strlen(cBuffer));
#endif

	// Return the result
	return obDir;
}

//------------------------------------------------------------------------------------------
//!
//! CaptureSystem::GenerateFileName
//! Generate the name to use for the file
//!
//------------------------------------------------------------------------------------------
ntstd::String CaptureSystem::GenerateNextFile(const char* pcExtension)
{
	ntstd::String obFileName;
	FILE* pFile;
	do
	{
		// Increment the file index
		m_iFileIndex+=m_iFileInc;
		
		// Generate dirname
		obFileName = CurrentFileName(pcExtension);
		// Try opening the file to see if it exists
		pFile = fopen( ntStr::GetString(obFileName), "r" );
		if (pFile)
		{
			// Close it again
			fclose(pFile);
		}
	}
	while (pFile && (m_iFileInc > 0));

	// Return the result
	return obFileName;
}


//------------------------------------------------------------------------------------------
//!
//! CaptureSystem::CurrentDirName
//! Generate the name to use for the directory
//!
//------------------------------------------------------------------------------------------
ntstd::String CaptureSystem::CurrentDirName()
{
	char cDir[256];
	// Generate dirname
	(m_iDirInc > 0) ? sprintf(cDir, "%s%05d", ntStr::GetString(m_obDirName), m_iDirIndex) : sprintf(cDir, "%s", ntStr::GetString(m_obDirName));
	// Return the result
	return cDir;
}

//------------------------------------------------------------------------------------------
//!
//! CaptureSystem::CurrentFileName
//! Generate the name to use for the directory
//!
//------------------------------------------------------------------------------------------
ntstd::String CaptureSystem::CurrentFileName(const char* pcExtension)
{
	char cFileName[512];
	char cFile[256];

	// Generate dirname
	ntstd::String obDirName = CurrentDirName();

	// Generate file
	(m_iFileInc > 0) ? sprintf(cFile, "%s%05d", ntStr::GetString(m_obFileName), m_iFileIndex) : sprintf(cFile, "%s", ntStr::GetString(m_obFileName));

	// Generate filename
	sprintf(cFileName, "%s\\%s%s", ntStr::GetString(obDirName), cFile, pcExtension);
	return cFileName;
}

//------------------------------------------------------------------------------------------
//!
//! CaptureSystem::DoScreenShot
//! Generate a screenshot
//!
//------------------------------------------------------------------------------------------
COMMAND_RESULT CaptureSystem::DoScreenShot()
{
	m_iFramesToWrite = 1;
	SetCaptureMode(IMAGE);
	StartCapture();
	return CR_SUCCESS;
}

