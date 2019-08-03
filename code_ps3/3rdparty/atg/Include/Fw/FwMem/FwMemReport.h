//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		Memory Reporting

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef	FW_MEM_REPORT_H
#define	FW_MEM_REPORT_H

#ifdef	ATG_MEMORY_DEBUG_ENABLED

//--------------------------------------------------------------------------------------------------
/**
	@class			FwMemReport
	
	@brief			Container for memory reporting functionality

	@note			This functionality is present only in DEBUG and DEVELOPMENT builds. 
	
**/
//--------------------------------------------------------------------------------------------------

class	FwMemReport
{
public:

	enum	SortType
	{
		kLargestTotal,
		kLargestAllocation,
		kLargestNumAllocations,
		kAlphabetical,

		kNumberOfSortTypes
	};

	enum	SnapshotBuffer
	{
		kSnapshotBufferPrevious		= -2,
		kSnapshotBufferCurrent		= -1,

		kSnapshotBuffer0			= 0,	
		kSnapshotBuffer1			= 1,

		kSnapshotBufferUser1		= 2,
		kSnapshotBufferUser2		= 3,

		kNumberOfSnapshotBuffers,
	};

	enum	DiscrepancyStatus
	{
		kNoDiscrepancies,
		kFoundDiscrepancies,
		kSnapshotsNotPopulated,
		kSnapshotsForDifferentContexts,
	};

	enum	DiscrepancyMode
	{
		kAccidentalFrees,
		kMemoryLeaks,
	};

	typedef void	( DiscrepancyCallback )( const char* pTag, int lineNumber, int allocCount, void* pUserData );

	static	void				GenerateSnapshot( SnapshotBuffer bufferType, SortType sortType = kLargestTotal, const char* pContextName = NULL );
	static	const char*			GetSortTypeDescription( SortType sortType );

	static	DiscrepancyStatus	ProcessDiscrepancies( SnapshotBuffer buffer0, SnapshotBuffer buffer1, DiscrepancyMode mode, DiscrepancyCallback* pCallback = NULL, void* pUserData = NULL );
	static	void				ShowDiscrepancies( SnapshotBuffer buffer0 = kSnapshotBufferCurrent, SnapshotBuffer buffer1 = kSnapshotBufferPrevious );

	static	void				ShowCurrent( SortType sortType = kLargestTotal, const char* pContextName = NULL );

private:

	static const int	kMaxQueryRecords	= 512;	///< Maximum number of unique query records we support

	struct	QueryRecord
	{
		const char*	pTag;							///< Text tag associated with the allocation
		s16			lineNumber;						///< Line number associated with the allocation
		s16			contextIndex;					///< Context index associated with this allocation
		int			allocCount;						///< Number of allocations were made with this tag/line/context
		int			memTotal;						///< Total memory allocated in all contributing allocations
		int			avgSize;						///< Average allocation size 
		int			minSize;						///< Minimum allocation size
		int			maxSize;						///< Maximum allocation size
	};

	struct	QuerySummary
	{
		int		numberOfRecords;					///< Number of records found during a query
		int		numberOfAllocations;				///< Number of allocations found during a query 
		int		totalAllocationSize;				///< Total size of all valid allocations found during a query
		s16		contextIndex;						///< The context index used to generate this summary
		s16		summaryValid;						///< Used to ensure that we don't show discrepancies between unused snapshots
	};

	struct	ReportQueryParams
	{
		QueryRecord*	pQueryRecordArray;			///< Pointer to an array of QueryRecord objects
		QuerySummary*	pQuerySummary;				///< Pointer to an object holding summary snapshot information
	};

	static	int				ms_snapshotIndex;
	static	QueryRecord		ms_queryRecords[ kNumberOfSnapshotBuffers ][ kMaxQueryRecords ];
	static	QuerySummary	ms_querySummary[ kNumberOfSnapshotBuffers ];

	static	void	SortSnapshot( ReportQueryParams* pQueryParams, SortType sortType );
	static	void	QueryAllocationCallback( const FwMemDebug::AllocationInfo& allocInfo, void* pUserData );
	static	void	DiscrepancyFreeCallback( const char* pTag, int lineNumber, int allocCount, void* pUserData ); 
	static	void	DiscrepancyLeakCallback( const char* pTag, int lineNumber, int allocCount, void* pUserData ); 
};

#endif	// ATG_MEMORY_DEBUG_ENABLED

#endif	// FW_MEM_REPORT
