//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		Derived application based reporter.

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef	FP_APP_REPORTER_H
#define	FP_APP_REPORTER_H

//--------------------------------------------------------------------------------------------------
//  INCLUDES
//--------------------------------------------------------------------------------------------------

#include <Fp/FpUnit/FpReporter.h>

//--------------------------------------------------------------------------------------------------
/**
	@class			FpAppReporter

	@brief			Derived reporter - provides error reporting via OutputDebugString / MessageBox
					on Win32 platforms and simple printf on others.
**/
//--------------------------------------------------------------------------------------------------

class FpAppReporter : public FpReporter
{ 
public:
	FpAppReporter();

protected:
	virtual void Printf(const ReportType id, const char* const fmt);
};

//--------------------------------------------------------------------------------------------------
//  INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------------

#endif	// FP_APP_REPORTER_H
