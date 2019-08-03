//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		Simple unit test interface - provides common functionality for unit tests

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef FP_UNIT_H
#define FP_UNIT_H

//--------------------------------------------------------------------------------------------------
//  INCLUDES
//--------------------------------------------------------------------------------------------------

#include <Fp/FpUnit/FpReporter.h>		// included here so that all unittests simply need to include FpUnit.h


//--------------------------------------------------------------------------------------------------
/**
	@class			FpUnit

	@brief			A singleton unit test class
**/
//--------------------------------------------------------------------------------------------------

class FpUnit : public FwNonCopyable
{
public:
	/// Separator types for dividing unit test output.
	enum SeparatorType
	{
		kNormalSeparator,						///< Normal	----------- ...
		kSoftSeparator							///< Soft	- - - - - - ... 
	};

public:
	// Initialise & shutdown
	// Not a great deal of functionality as yet...
	static void				Initialise(FpReporter* pReporter);
	static FpReporter*		GetReporter();

	static void				PrintHeader(const char* pUnitTestName);
	static void				PrintFooter(const char* pFile, const char* pTimeStamp);
	static void				PrintSeparator(SeparatorType separator = kNormalSeparator);

	static void				StartTest(const char* pFmt, ...);
	static void				Print( const char* const pMsg );
	static void				Printf( const char* const pFmt, ... );
	static void				Errorf( const char* const pFmt, ... );
	static void				Warningf( const char* const pFmt, ... );
	static void				Passf( const char* const pFmt, ... );
	static void				Failf( const char* const pFmt, ... );
	static void				Testf( const bool bCond, const char* const pFmt, ... );
	static void				Pass();
	static void				Fail();
	static void				Test( const bool bCond );
	static int				EndTest(void);

	static int				GetNumTestErrors(void);
	static int				GetNumTotalErrors(void);
	static void				ResetCounters();

	static void				SetBreakOnFail(bool bState);
	static void				SetBreakOnError(bool bState);

private:
	FpUnit();

private:

	static const char*		ms_pkNormalSeparator;
	static const char*		ms_pkSoftSeparator;
	static const char*		ms_pkSofterSeparator;
	static const char*		ms_pkHardSeparator;

	static FpReporter*		ms_pReporter;					///< Ptr to output stream.
	
	static int				ms_testNum;						///< Current test number.
	static int				ms_testNumLastEnd;				///< Number of last test ended
	
	static int				ms_numPassesTotal;				///< Cumulative number of calls to Passf
	static int				ms_numFailuresTotal;			///< Cumulative number of calls to Failf
	static int				ms_numErrorsTotal;				///< Cumulative number of calls to Errorf
	static int				ms_numWarningsTotal;			///< Cumulative number of calls to Warningf
	
	static bool				ms_isInitialised;				///
	static bool				ms_breakOnError;				///< Break on errors. defaults to true in debug builds
	static bool				ms_breakOnFail;					///< Break on failure. defaults to true in debug builds
};


//--------------------------------------------------------------------------------------------------
//  Simple helper macro for tests
//--------------------------------------------------------------------------------------------------

#ifdef ATG_DEBUG_MODE
	#define FP_UNIT_TEST(condition)																	\
	do																								\
	{																								\
		FpUnit::Testf(cond," in file %s, at line %d\n",__FILE__,__LINE__);							\
	} while (false)
#else
	#define FP_UNIT_TEST(condition)																	\
	do																								\
	{																								\
		FpUnit::Test(cond);																			\
	} while (false)
#endif

//--------------------------------------------------------------------------------------------------

#endif	// FP_UNIT_H
