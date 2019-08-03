/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		Manage processing of audits from the PPU side
**/
//--------------------------------------------------------------------------------------------------

#ifndef WWS_JOB_AUDIT_MANAGER_H
#define WWS_JOB_AUDIT_MANAGER_H

//--------------------------------------------------------------------------------------------------

#include <wwsbase/types.h>
#include <jobsystem/helpers.h>
#include <jobapi/audittypes.h>

//--------------------------------------------------------------------------------------------------

struct AuditBufferHeader;
struct CellSpurs;

//--------------------------------------------------------------------------------------------------

class AuditManager
{
public:
	typedef bool (*ProcessAuditFunction)( const AuditManager* pAuditManager, U32 spuNum, U16 id, U32 time, bool hwordIsValid, U16 hData, U16 numDwords, const void* pAuditData, void* pUserData );

	enum WwsJob_NumBuffers
	{
		kSingleBuffered		= 1,
		kDoubleBuffered		= 2,
	};

	enum WwsJob_EnableHeaderPrinting
	{
		kDontPrintHeaders,
		kPrintHeaders,
	};

								AuditManager();
								AuditManager( void* pAuditBufferBase, U32 auditBufferSize, U32 numSpus, WwsJob_NumBuffers numBuffers );

	void						InitAuditMemory( void* pAuditBufferBase, U32 auditBufferSize, U32 numSpus, WwsJob_NumBuffers numBuffers );

	volatile void*				GetAuditMemory( void )						{ return m_pAuditHeader; }
	const void*					GetSpuAuditOutputBuffer( U32 bufferIndexNum, U32 spuNum ) const;
	U32							GetEachSpuAuditBufferSize( void	) const		{ return m_eachSpuAuditBufferSize; }
	U32							GetNumDwordsInAuditOutputBuffer( U32 bufferIndexNum, U32 spuNum ) const;

	static bool					PrintAudit( const AuditManager* pAuditManager, U32 spuNum, U16 id, U32 time, bool hwordIsValid, U16 hData, U16 numDwords, const void* pAuditData, void* pUserData );
	void						ProcessAuditBuffer( U32 bufferIndexNum, U32 spuNum, U32 maxAuditsToProcess, ProcessAuditFunction func, WwsJob_EnableHeaderPrinting enableHeaderPrinting, void* pUserData ) const;
	void						ProcessAuditBuffersForAllSpus( U32 bufferIndexNum, ProcessAuditFunction func, WwsJob_EnableHeaderPrinting enableHeaderPrinting, void* pUserData ) const;

	U16							AllocIdRange( U32 numIds );
	void						RegisterAuditData( U16 baseValidID, U16 endValidIDs /*exclusive*/, const char* systemName, const char* const* pAuditFormatStringArray );
	U16							AllocIdRangeAndRegisterAuditData( U32 numIds, const char* systemName, const char* const* pAuditFormatStringArray );

	void						GetAuditFormatString( U16 id, const char** ppSystem, const char** ppText ) const;

	void						EmptyAuditBuffer( U32 bufferIndexNum, U32 spuNum );
	void						EmptyAuditBuffersForAllSpus( U32 bufferIndexNum );

	void						SetJobManagerAuditsEnabled( bool enableAudits );
	void						SetJobAuditsEnabled( bool enableAudits );
	void						SetAuditOutputBufferNum( U32 bufferNum );

	void						ImmediateModeDataU32( U32 data, U32 spuNum, ProcessAuditFunction func, void* pUserData );

	void						WaitForAudits( U32 bufferIndexNum, U32 spuNum ) const;
	void						WaitForAuditsForAllSpus( U32 bufferIndexNum ) const;

	void						WritePaSuiteFile( CellSpurs* pSpurs, const char* name );

private:
	void						ImmediateModeDataU64( U64 data64, U32 spuNum, ProcessAuditFunction func, void* pUserData );
	void						Init( void );

	volatile AuditBufferHeader*	m_pAuditHeader;
	U32							m_eachSpuAuditBufferSize;

	struct AuditSystemData
	{
		U16					m_baseValidId;
		U16					m_finalValidId;
		const char*			m_pSystemName;
		const char* const*	m_pAuditFormatStringArray;
	};

	static const U32		kMaxAuditSystems = 32;
	AuditSystemData			m_auditSystemData[kMaxAuditSystems];
	U32						m_numAuditSystems;

	U32						m_numSpusSharingBigBuffer;
	U32						m_numBuffers;	//eg. are audits single-bufferd, double-buffered etc...

	U32						m_idAllocMarker;

	enum
	{
		kMaximumAuditId = 0x1FFF,
	};


	//The following are used for processing immediate mode audits
	union AuditData
	{
		U64		m_u64;
		U32		m_u32[2];
		Audit	m_audit;
	};

	static const U32		kMaxSpus = 6;
	bool					m_cachedU32Exists[kMaxSpus];
	U32						m_cachedU32Value[kMaxSpus];
	static const U32		kMaxDwordsInSingleAudit = 64;
	U32						m_currentNumU64Elements[kMaxSpus];
	AuditData				m_currentAuditDataCache[kMaxSpus][kMaxDwordsInSingleAudit];


};

//--------------------------------------------------------------------------------------------------

#endif /* WWS_JOB_AUDIT_MANAGER_H */
