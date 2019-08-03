/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		Manage a joblist from the PPU side.
				SingleThreadJobList may only be used if you are sure only a single PPU thread
				will be adding at a time.
				MultiThreadSafeJobList is for use if a joblist may be added to by multiple PPU
				threads at once or by SPUs.
**/
//--------------------------------------------------------------------------------------------------

#ifndef WWS_JOB_JOB_LIST_H
#define WWS_JOB_JOB_LIST_H

//--------------------------------------------------------------------------------------------------

#include <cell/spurs/types.h>

#include <wwsbase/types.h>
#include <jobsystem/helpers.h>

#include <jobapi/jobdefinition.h>

//--------------------------------------------------------------------------------------------------

class JobListMarker;
struct JobListHeader;
union JobHeader;
struct CellSpurs;
struct DependencyCounter;
class AuditManager;

//--------------------------------------------------------------------------------------------------
//
//	You should never use the JobListPrivate class.  It is a private class used solely by
//	SingleThreadJobList and MultiThreadSafeJobList.
//	User code should only ever use SingleThreadJobList or MultiThreadSafeJobList.
//
//	Note that no virtual functions are used in this base class.
//
//--------------------------------------------------------------------------------------------------

class JobListPrivate
{
	friend class JobListMarker;

public:
	enum JobListCheckOptions
	{
		//These options can be ORed together for passing in as the options field to CheckJobList
		kNothing			= 0,
		kPrintCommands		= 1,
		kPrintWarnings		= 2,
		kPrintHints			= 4,

		kPrintCommandsAndWarnings		= kPrintCommands | kPrintWarnings,
		kPrintCommandsAndHints			= kPrintCommands | kPrintHints,
		kPrintWarningsAndHints			= kPrintWarnings | kPrintHints,
		kPrintCommandsWarningsAndHints	= kPrintCommands | kPrintWarnings | kPrintHints,
	};

	enum ReadyCountValues
	{
		kRequestOneSpu		= 1,
		kRequestAllSpus		= 8,
	};

					~JobListPrivate() { WWSJOB_ASSERT( m_pSpurs == NULL ); 	WWSJOB_ASSERT( m_workloadId == kInvalidWorkloadId );  }

	void			CheckJobList( U32 options = kNothing ) const;

	void			AttachToSpurs( CellSpurs* pSpurs, const U8* workPrios, U32 max_contention, AuditManager* pAuditManager = NULL );
	void			SetNewPriorities( const U8* workPrios );

	void			Shutdown( void );

	CellSpursWorkloadId		GetWorkloadId( void )		{	return m_workloadId;	}

	void*			GetWQAddr( void )					{	return m_pJobListHeader;	}
	const void*		GetWQAddr( void ) const				{	return m_pJobListHeader;	}

	U32				GetMaxNumJobs( void ) const			{	return m_maxNumElts;	}

	void			SetReadyCount( ReadyCountValues readyCount ) { SetReadyCount( (U32) readyCount ); }
	void			SetReadyCount( U32 readyCount );

	const char*		GetName( void ) const				{	return m_name;		}
	void			SetName( const char* name );

	CellSpurs*		GetSpursPtr( void )					{	return m_pSpurs;	}

protected:
					JobListPrivate( void );
					JobListPrivate( void* pBuffer, U32 bufferSize, const char* name );
	void			Init( void* pBuffer, U32 bufferSize, const char* name );

	void			ResetList( void );
	bool			IsJobMarkerPassed( U16 jobIndex ) const;
	void			StallForJobMarker( U16 jobIndex ) const;

	void			AttachToSpursPrivate( CellSpurs* pSpurs, const U8* workPrios, U32 maxContention, U32 baseAddr, AuditManager* pAuditManager );

	CellSpurs*		m_pSpurs;
	JobListHeader*	m_pJobListHeader;
	U32				m_maxNumElts;
	U32				m_maxContention;
	CellSpursWorkloadId	m_workloadId;

	static const U32	kMaxNameLength = 16;
	static const U32	kInvalidWorkloadId = 0xFFFFFFFF;

	char			m_name[kMaxNameLength];

private:
					JobListPrivate( const JobListPrivate& );
	JobListPrivate&	operator=( const JobListPrivate& );
};

inline void JobListPrivate::AttachToSpurs( CellSpurs* pSpurs, const U8* workPrios, U32 maxContention, AuditManager* pAuditManager )
{
	//This inline function is compiled in the context of the game
	//We use it to pass the "LsMemoryLimits::kJobAreaBasePageNum" into the library so that we can
	//double check that the game side is being compiled consistently with how the library was compiled
	AttachToSpursPrivate( pSpurs, workPrios, maxContention, LsMemoryLimits::kJobAreaBasePageNum, pAuditManager );
}


//--------------------------------------------------------------------------------------------------

class JobListMarker
{
public:
	friend bool operator == ( const JobListMarker& lhs, const JobListMarker& rhs );
	friend bool operator != ( const JobListMarker& lhs, const JobListMarker& rhs );

			JobListMarker() : m_jobIndex(0), m_pJobList(NULL) {}
			JobListMarker( U16 jobIndex, const JobListPrivate* pJobList ) : m_jobIndex( jobIndex ), m_pJobList( pJobList ) {}
	bool	IsJobMarkerPassed( void ) const { WWSJOB_ASSERT(m_pJobList); return m_pJobList->IsJobMarkerPassed( m_jobIndex ); }
	void	StallForJobMarker( void ) const { WWSJOB_ASSERT(m_pJobList); m_pJobList->StallForJobMarker( m_jobIndex ); }

	bool	IsJobMarkerClear( void ) const	{ return (NULL == m_pJobList); }
	void	ClearJobMarker( void )			{ m_jobIndex = 0; m_pJobList = NULL; }

private:
	U16						m_jobIndex;
	const JobListPrivate*	m_pJobList;
} WWSJOB_ALIGNED( 8 );

//--------------------------------------------------------------------------------------------------

inline bool operator == ( const JobListMarker& lhs, const JobListMarker& rhs )
{
	if ( lhs.m_jobIndex != rhs.m_jobIndex )
		return false;

	if ( lhs.m_pJobList != rhs.m_pJobList )
		return false;

	return true;
}

//--------------------------------------------------------------------------------------------------

inline bool operator != ( const JobListMarker& lhs, const JobListMarker& rhs )
{
	return ( ! ( lhs == rhs ) );
}


//--------------------------------------------------------------------------------------------------
//
//	This class may be used for building and accessing job lists in cases when you know that the
//	job list will only ever be accessed by a single thread at a time.
//
//--------------------------------------------------------------------------------------------------

class SingleThreadJobList		: public JobListPrivate
{
public:
					SingleThreadJobList( void );
					SingleThreadJobList( void* pBuffer, U32 bufferSize, const char* name = NULL );
	void			Init( void* pBuffer, U32 bufferSize, const char* name = NULL );

	JobListMarker	AddJob( const void* pJob, U16 jobSize );
	JobListMarker	AddJob( JobHeader job );
	JobListMarker	AddJobWithBreakpoint( const void* pJob, U16 jobSize );
	JobListMarker	AddJobWithBreakpoint( JobHeader job );

	JobListMarker	AddJobAndKick( const void* pJob, U16 jobSize, U32 readyCount );
	JobListMarker	AddJobAndKick( JobHeader job, U32 readyCount );
	JobListMarker	AddJobAndKickWithBreakpoint( const void* pJob, U16 jobSize, U32 readyCount );
	JobListMarker	AddJobAndKickWithBreakpoint( JobHeader job, U32 readyCount );

	JobListMarker	AddGeneralBarrier( const DependencyCounter* pBarrier );
	JobListMarker	AddGeneralBarrierAndKick( const DependencyCounter* pBarrier, U32 readyCount );

	JobListMarker	AddNopJob( void );

	U32				GetCurrentNumJobs( void ) const			{ return m_realNumJobsInList; }
	JobListMarker	CalculateJobListMarker( void ) const;

	bool			IsListFinished( void ) const;
	void			WaitForJobListEnd( void ) const;

	//void			SetReadyCount(  ReadyCountValues readyCount ) { WWSJOB_ASSERT(GetCurrentNumJobs() > 0); JobListPrivate::SetReadyCount(readyCount); }
	void			ResetList( void );

	void			ForcedFinish( void );	//Intended for debug purposes only


	JobHeader*		GetCurrentJobHeaderPtr( void ) const	{ return &m_pJobListHeader->m_jobHeader[GetCurrentNumJobs()]; }

	JobHeader*		InsertHole( U32 numJobs );	//Returns the pointer to numJobs worth of Nop Jobs

private:
	JobListMarker	AddJobInternal( JobHeader jobHeader );

	U32				m_realNumJobsInList;
};

//--------------------------------------------------------------------------------------------------
//
//	If multiple threads may access the job list at a single time, you must use a
//	MultiThreadSafeJobList.  This adds jobs and searchs for the end of the list in a multi-thread
//	safe and lock-less manner.
//
//--------------------------------------------------------------------------------------------------

class MultiThreadSafeJobList	: public JobListPrivate
{
public:
					MultiThreadSafeJobList( void ) {}
					MultiThreadSafeJobList( void* pBuffer, U32 bufferSize, const char* name = NULL ) : JobListPrivate( pBuffer, bufferSize, name ) {}
	void			Init( void* pBuffer, U32 bufferSize, const char* name = NULL ) { JobListPrivate::Init( pBuffer, bufferSize, name ); }

	JobListMarker	AddJob( const void* pJob, U16 jobSize );
	JobListMarker	AddJob( JobHeader job );
	JobListMarker	AddJobWithBreakpoint( const void* pJob, U16 jobSize );
	JobListMarker	AddJobWithBreakpoint( JobHeader job );

	JobListMarker	AddJobAndKick( const void* pJob, U16 jobSize, U32 readyCount );
	JobListMarker	AddJobAndKick( JobHeader job, U32 readyCount );
	JobListMarker	AddJobAndKickWithBreakpoint( const void* pJob, U16 jobSize, U32 readyCount );
	JobListMarker	AddJobAndKickWithBreakpoint( JobHeader job, U32 readyCount );

	JobListMarker	AddGeneralBarrier( const DependencyCounter* pBarrier );
	JobListMarker	AddGeneralBarrierAndKick( const DependencyCounter* pBarrier, U32 readyCount );

	JobListMarker	AddNopJob( void );

	U32				GetCurrentNumJobs( void ) const;
	JobListMarker	CalculateJobListMarker( void ) const;

	bool			IsListFinished( void ) const;
	void			WaitForJobListEnd( void ) const;

	//void			SetReadyCount(  ReadyCountValues readyCount ) { WWSJOB_ASSERT(GetCurrentNumJobs() > 0); JobListPrivate::SetReadyCount(readyCount); }
	void			ResetList( void );

	void			ForcedFinish( void );	//Intended for debug purposes only

	JobHeader*		GetCurrentJobHeaderPtr( void ) const	{ return &m_pJobListHeader->m_jobHeader[GetCurrentNumJobs()]; }

	//Since this is an intricate function to use correctly with MultiThreadSafeJobLists,
	//it is currently disabled until a user actually requests it.
	//JobHeader*	InsertHole( U32 numJobs );	//Fills in numJobs of Nop Jobs (discontiguous).  Returns a pointer to the first one.

private:
	JobListMarker	AddJobInternal( JobHeader jobHeader );
};

//--------------------------------------------------------------------------------------------------

#endif /* WWS_JOB_JOB_LIST_H */
