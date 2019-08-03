// AVIGenerator.h: interface for the CAVIGenerator class.
//
// A class to easily create AVI
//
// Original code : Example code in WriteAvi.c of MSDN
// 
// Author : Jonathan de Halleux. dehalleux@auto.ucl.ac.be
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_AVIGENERATOR_H__6BAF2E9D_3866_4779_A43B_D1B21E7E4F39__INCLUDED_)
#define AFX_AVIGENERATOR_H__6BAF2E9D_3866_4779_A43B_D1B21E7E4F39__INCLUDED_

#if defined(PLATFORM_PC)

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// c:\Program Files\Microsoft Visual Studio .NET 2003\Vc7\PlatformSDK\Include\MMSystem.h(1837)
// warning C4201: nonstandard extension used : nameless struct/union
#pragma warning( disable : 4201 )

// needed headers
#include <comdef.h>
#include <memory.h>
#include <tchar.h>
#include <string.h>
#include <vfw.h>

#pragma comment ( lib, "vfw32.lib")

// undefine this if you don't use MFC
//#define _AVIGENERATOR_USE_MFC

/*! \brief A simple class to create AVI video stream.


\par Usage

  Step 1 : Declare an CAVIGenerator object
  Step 2 : Set Bitmap by calling SetBitmapHeader functions + other parameters
  Step 3 : Initialize engine by calling InitEngine
  Step 4 : Send each frames to engine with function AddFrame
  Step 5 : Close engine by calling ReleaseEngine

\par Demo Code:

\code
	CAVIGenerator AviGen;
	uint8_t* bmBits;

	// set characteristics
	AviGen.SetRate(20);							// set 20fps
	AviGen.SetBitmapHeader(GetActiveView());	// give info about bitmap

	AviGen.InitEngine();

	..... // Draw code, bmBits is the buffer containing the frame
	AviGen.AddFrame(bmBits);
	.....

	AviGen.ReleaseEngine();
\endcode

\par Update history:

	- {\bf 22-10-2002} Minor changes in constructors.

\author : Jonathan de Halleux, dehalleux@auto.ucl.ac.be (2001)
*/
class CAVIGenerator  
{
public:
	//! \name Constructors and destructors
	//@{
	//! Default constructor 
	CAVIGenerator();
#ifdef _AVIGENERATOR_USE_MFC
	//! Inplace constructor with CView
	CAVIGenerator(LPCTSTR sFileName, CView* pView, uint32_t dwRate);
#endif
	//! Inplace constructor with BITMAPINFOHEADER
	CAVIGenerator(LPCTSTR sFileName, LPBITMAPINFOHEADER lpbih, uint32_t dwRate);
	~CAVIGenerator();
	//@}

	//! \name  AVI engine function
	//@{
	/*! \brief  Initialize engine and choose codex

	 Some asserts are made to check that bitmap has been properly initialized
	*/
	HRESULT InitEngine();

	/*! \brief Adds a frame to the movie. 
	
	The data pointed by bmBits has to be compatible with the bitmap description of the movie.
	*/
	HRESULT AddFrame(uint8_t* bmBits);
	//! Release ressources allocated for movie and close file.
	void ReleaseEngine();
	//@}

	//! \name Setters and getters
	//@{
#ifdef _AVIGENERATOR_USE_MFC
	//! Sets bitmap info to match pView dimension.
	void SetBitmapHeader(CView* pView);
#endif
	//! Sets bitmap info as in lpbih
	void SetBitmapHeader(LPBITMAPINFOHEADER lpbih);
	//! returns a pointer to bitmap info
	LPBITMAPINFOHEADER GetBitmapHeader()							{	return &m_bih;};
	//! sets the name of the ouput file (should be .avi)
	void SetFileName(LPCTSTR _sFileName)							{	m_sFile=_sFileName; MakeExtAvi();};
	//! Sets FrameRate (should equal or greater than one)
	void SetRate(uint32_t dwRate)										{	m_dwRate=dwRate;};
	//@}
	
	//! \name Error handling
	//@{
	//! returns last  ntError message
	LPCTSTR GetLastErrorMessage() const								{	return m_sError;};
	//@}

protected:	
	//! name of output file
	_bstr_t m_sFile;			
	//! Frame rate 
	uint32_t m_dwRate;	
	//! structure contains information for a single stream
	BITMAPINFOHEADER m_bih;	
	//! last ntError string
	_bstr_t m_sError;

private:
	void MakeExtAvi();
	//! frame counter
	long m_lFrame;
	//! file interface pointer
	PAVIFILE m_pAVIFile;
	//! Address of the stream interface
	PAVISTREAM m_pStream;		
	//! Address of the compressed video stream
	PAVISTREAM m_pStreamCompressed; 
};

#endif

#endif // !defined(AFX_AVIGENERATOR_H__6BAF2E9D_3866_4779_A43B_D1B21E7E4F39__INCLUDED_)
