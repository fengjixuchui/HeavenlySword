//------------------------------------------------------------------------------------------
//!
//!	\file game/capturesystem.h
//!	Header for the CaptureSystem
//!
//! Capture system is purely responsible for writing out a sequence of images or frames to
//! an AVI or even a single image. It takes care of automatic naming etc...
//!
//------------------------------------------------------------------------------------------

#ifndef _CAPTURE_SYSTEM
#define _CAPTURE_SYSTEM

// ---- Includes ----
#include "game/commandresult.h"

// ---- Forward declares ----
class CAVIGenerator;
class CHashedString;

//------------------------------------------------------------------------------------------
//!
//! CaptureSystem
//! A simpler system to automate writing of a stream of image files or avi to disk
//!
//------------------------------------------------------------------------------------------
class CaptureSystem : public Singleton<CaptureSystem>
{
public:
	// enumerate capture mode types
	enum CAPTURE_MODE
	{
		IMAGE,	// Image files
		AVI		// Movie
	};

	// Const / destr
	CaptureSystem();
	~CaptureSystem();

	// Is the capture active
	bool IsCapturing() {return m_bCapturing;}

	// Set the capture mode
	void SetCaptureMode(CAPTURE_MODE eMode);

	// Toggle the capture system
	COMMAND_RESULT ToggleCaptureMode();

	// Start capturing
	void StartCapture();

	// Stop capturing
	void StopCapture();

	// Update the CaptureSystem
	void Update(float fTime);

	// Set the framerate
	void SetFrameRate(float fRate) {m_fFrameRate = fRate;}

	// Set the capture name
	void SetCaptureName(const char* obName) {m_obFileName = obName;}

	// Set the capture dir
	void SetCaptureDir(const char* obName) {m_obDirName = obName;}

	// Create a screenshot
	COMMAND_RESULT DoScreenShot();

private:

	// Initialise AVI writing
	void InitialiseAVIWrite();

	// Initialise Image writing
	void InitialiseImageWrite();

	// Update Image write
	void UpdateAVIWrite();

	// Update Image write
	void UpdateImageWrite();

	// Generate the next name for the dir
	ntstd::String GenerateNextDir();

	// Generate the next name for the file
	ntstd::String GenerateNextFile(const char* pcExtension);

	// Get the current filename
	ntstd::String CurrentFileName(const char* pcExtension);

	// Get the current filename
	ntstd::String CurrentDirName();

	// Write text to logfile
	void LogWrite(const char* pcText);

	// Has the capture been initialised
	bool m_bInitialised;

	// The current capture mode
	CAPTURE_MODE m_eMode;

	// Is a capture in progress
	bool m_bCapturing;

	// Number of frames captured
	int m_iFramesCaptured;

	// Filename for capture
	ntstd::String	m_obFileName;

	// Directory name for capture
	ntstd::String	m_obDirName;

	// The framerate of the movie / image sequence
	float m_fFrameRate;

	// An AVI generator
	CAVIGenerator* m_pobAVIGenerator;

	// Handle for logfile
	FILE* m_pLogFile;

	// The current index for output
	int m_iFileIndex;

	// The current index for output
	int m_iDirIndex;

	// The current frame index;
	int m_iFrameIndex;

// ---- Control options ----

	// The number increment for directories (0 for no number) 
	int m_iDirInc;

	// The number increment for filenames (0 for no number) 
	int m_iFileInc;

	// The number of frames to write
	int m_iFramesToWrite;
};

#endif //_CAPTURE_SYSTEM
