//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		Reporter functionality for unit tests.

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef	FP_REPORTER_H
#define	FP_REPORTER_H

//--------------------------------------------------------------------------------------------------
//  INCLUDES
//--------------------------------------------------------------------------------------------------



#define ATG_IDS_ERROR				"Error"
#define ATG_IDS_WARNING				"Warning"
#define ATG_IDS_UNIT_PASS			"Unit Pass"
#define ATG_IDS_UNIT_FAIL			"Unit Fail"

//--------------------------------------------------------------------------------------------------
/**
	@class			FpReporter

	@brief			A class that provides a simple deriveable reporting interface	
**/
//--------------------------------------------------------------------------------------------------

class FpReporter : public FwNonCopyable
{ 
public:
	FpReporter();
	virtual ~FpReporter();

	// Operations

	void		Log(const char* const pMsg);
	void		LogMessage(const char* const pMsg);
	void		LogWarning(const char* const pMsg);
	bool		LogError(const char* const pMsg);
	void		LogPass(const char* const pMsg);
	void		LogFail(const char* const pMsg);
	void		LogPass();
	void		LogFail();

	void		Logf(const char* const pMsg, ...);
	void		LogMessagef(const char* const pMsg, ...);
	void		LogWarningf(const char* const pMsg, ...);
	bool		LogErrorf(const char* const pMsg, ...);
	void		LogPassf(const char* const pMsg, ...);
	void		LogFailf(const char* const pMsg, ...);

	uint		GetErrorCount() const;
	uint		GetWarningCount() const;

	uint		GetPassCount() const;
	uint		GetFailCount() const;

	void		ResetCounters();
	void		ReportCounters();

protected:
	// Enumerations
	// log identifiers passed to derived Printf implementation
	typedef enum
	{
		cMessage,
		cWarning,
		cError,
		cUnitPass,
		cUnitFail
	} ReportType;

	virtual void Printf(const ReportType id, const char* const fmt);

protected:
	// Attributes

	uint		m_errorCount;		///< Cumulative number of errors
	uint		m_warningCount;		///< Cumulative number of warnings
	uint		m_passCount;		///< Cumulative number of passes
	uint		m_failCount;		///< Cumulative number of failures
};

//--------------------------------------------------------------------------------------------------
//  INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

inline uint FpReporter::GetErrorCount() const
{
	return m_errorCount;
}

inline uint FpReporter::GetWarningCount() const
{
	return m_warningCount;
}

inline uint FpReporter::GetPassCount() const
{
	return m_passCount;
}

inline uint FpReporter::GetFailCount() const
{
	return m_failCount;
}

inline void FpReporter::Log(const char* const pMsg)
{
	this->Printf(FpReporter::cMessage, pMsg);
}

inline void FpReporter::LogMessage(const char* const pMsg)
{
	this->Printf(FpReporter::cMessage, pMsg);
}

inline void FpReporter::LogWarning(const char* const pMsg)
{
	m_warningCount++;
	this->Printf(FpReporter::cWarning, ATG_IDS_WARNING ": ");
	this->Printf(FpReporter::cWarning, pMsg);
}

inline bool FpReporter::LogError(const char* const pMsg)
{
	m_errorCount++;
	this->Printf(FpReporter::cError, ATG_IDS_ERROR ": ");
	this->Printf(FpReporter::cError, pMsg);
	return false;
}

inline void FpReporter::LogPass(const char* const pMsg)
{
	m_passCount++;
	this->Printf(FpReporter::cUnitPass, ATG_IDS_UNIT_PASS ": ");
	this->Printf(FpReporter::cUnitPass, pMsg);
}

inline void FpReporter::LogFail(const char* const pMsg)
{
	m_failCount++;
	this->Printf(FpReporter::cUnitFail, ATG_IDS_UNIT_FAIL ": ");
	this->Printf(FpReporter::cUnitFail, pMsg);
}

inline void FpReporter::LogPass()
{
	m_passCount++;
	this->Printf(FpReporter::cUnitPass, ATG_IDS_UNIT_PASS "\n");
}

inline void FpReporter::LogFail()
{
	m_failCount++;
	this->Printf(FpReporter::cUnitFail, ATG_IDS_UNIT_FAIL "\n");
}

//--------------------------------------------------------------------------------------------------

#endif	// FP_REPORTER_H
