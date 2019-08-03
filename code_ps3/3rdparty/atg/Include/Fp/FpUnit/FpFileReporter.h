//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		Derived stdout/stderr based reporter.

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef	FP_FILEREPORTER_H
#define	FP_FILEREPORTER_H

//--------------------------------------------------------------------------------------------------
//  INCLUDES
//--------------------------------------------------------------------------------------------------

#include <Fp/FpUnit/FpReporter.h>

//--------------------------------------------------------------------------------------------------
/**
	@class			FpStdReporter

	@brief			Derived reporter - provides stdout/stderr reporting via FwPrintf		
**/
//--------------------------------------------------------------------------------------------------

class FpFileReporter : public FpReporter
{ 
public:
	 FpFileReporter();
	 FpFileReporter(const char* filename);
	~FpFileReporter();

protected:
	virtual void Printf(const ReportType id, const char* const fmt);

protected:
	enum { cMaxPath = 260 };

protected:
	char		mFilename[cMaxPath];
	FILE*		mpFile;
};

//--------------------------------------------------------------------------------------------------
//  INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------------

#endif	// FP_UNIT_H
