/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

.section .data16,"aw"

.align 4	//Note that the following only require 16 byte alignment each

.global g_lsaAudits
g_lsaAudits: .word 0,0,0,0

.global g_mmaAudits
g_mmaAudits: .word 0,0,0,0

.global g_mmaAuditsBase
g_mmaAuditsBase: .word 0,0,0,0

.global g_mmaAuditsEndLessAuditBlockSizePlus1
g_mmaAuditsEndLessAuditBlockSizePlus1: .word 0,0,0,0

.global g_bJobAuditsEnabled
g_bJobAuditsEnabled: .word 0,0,0,0

.global g_bJobManagerAuditsEnabled
g_bJobManagerAuditsEnabled: .word 0,0,0,0

.global g_mmaAuditBufferOutputHeader
g_mmaAuditBufferOutputHeader: .word 0,0,0,0

.global g_WwsJob_bufferSetArray
g_WwsJob_bufferSetArray:	.word 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0  
				.word 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0  
				.word 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0

.global g_WwsJob_bufferArray
g_WwsJob_bufferArray:	.word 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0  
			.word 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0  
			.word 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0  
			.word 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0  
			.word 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0  
			.word 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0  
			.word 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0  
			.word 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0  
			.word 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0  
			.word 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0  
			.word 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0  
			.word 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0  

.global g_WwsJob_logicalToBufferNumArray
g_WwsJob_logicalToBufferNumArray:	.word 0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF
					.word 0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF
					.word 0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF
					.word 0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF
					.word 0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF
					.word 0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF

//NOTE: the following masks must match the values of LsMemoryLimits::kJobAreaBasePageNum and kJobAreaEndPageNum (currently 16 and 256-8)!
.global g_WwsJob_initialPageMask
g_WwsJob_initialPageMask: .word 0xFFFF0000,0x00000000,0x00000000,0x00000000, 0x00000000,0x00000000,0x00000000,0x000000FF

.global g_WwsJob_usedPageMask
g_WwsJob_usedPageMask: .word 0xFFFF0000,0x00000000,0x00000000,0x00000000, 0x00000000,0x00000000,0x00000000,0x000000FF

.global g_WwsJob_shareablePageMask
g_WwsJob_shareablePageMask: .word 0,0,0,0, 0,0,0,0

.global g_WwsJob_reservedPageMask
g_WwsJob_reservedPageMask: .word 0xFFFF0000,0x00000000,0x00000000,0x00000000, 0x00000000,0x00000000,0x00000000,0x000000FF

.global g_WwsJob_usedDmaTagMask
g_WwsJob_usedDmaTagMask: .word 0x000000FF,0x000000FF,0x000000FF,0x000000FF

.global g_WwsJob_loadJobState	//NOTE: g_WwsJob_loadJobState is a halfword value in the prefered hword[1] - hword[0] may be non-zero!
g_WwsJob_loadJobState: .word 0,0,0,0

.global g_WwsJob_runJobState
g_WwsJob_runJobState: .word 0,0,0,0

.global g_WwsJob_nextLoadJobNum
g_WwsJob_nextLoadJobNum: .word 0,0,0,0

.global g_WwsJob_loadJobNum
g_WwsJob_loadJobNum: .word 0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF

.global g_WwsJob_runJobNum
g_WwsJob_runJobNum: .word 0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF

.global g_WwsJob_storeJobNum
g_WwsJob_storeJobNum: .word 0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF

.global g_WwsJob_lastStoreJobNum
g_WwsJob_lastStoreJobNum: .word 2,2,2,2

.global g_WwsJob_pJobCodeBufferSet
g_WwsJob_pJobCodeBufferSet: .word 0,0,0,0

.global g_WwsJob_pJobCodeBuffer
g_WwsJob_pJobCodeBuffer: .word 0,0,0,0

.global g_WwsJob_lsaJobCodeBuffer
g_WwsJob_lsaJobCodeBuffer: .word 0,0,0,0

.extern WwsJob_JobApi_kExecuteCommands
.extern GetBufferTag
.extern WwsJob_JobApi_kGetBufferTags
.extern WwsJob_JobApi_kUseDmaTagId
.extern WwsJob_JobApi_kFreeDmaTagId
.extern WwsJob_JobApi_kLoadNextJob
.extern WwsJob_JobApi_kFreeLogicalBuffer
.extern WwsJob_JobApi_kFreeLogicalBuffers
.extern WwsJob_JobApi_kStoreAudit
.global g_WwsJob_dataForJob
g_WwsJob_dataForJob:	.word 0,0,0,0 // m_spuNum 0x00 0
			.word 0,0,0,0 // m_jobId 0x10 16
			.word 0,0,0,0 // m_pParameters 0x20 32
			.word WwsJob_JobApi_kExecuteCommands, GetBufferTag, WwsJob_JobApi_kGetBufferTags, WwsJob_JobApi_kUseDmaTagId // m_pJobApi[0], m_pJobApi[1], m_pJobApi[2], m_pJobApi[3]
			.word WwsJob_JobApi_kFreeDmaTagId, WwsJob_JobApi_kLoadNextJob, WwsJob_JobApi_kFreeLogicalBuffer, WwsJob_JobApi_kFreeLogicalBuffers // m_pJobApi[4], m_pJobApi[5], m_pJobApi[6], m_pJobApi[7]
			.word WwsJob_JobApi_kStoreAudit, 0, 0, 0 // m_pJobApi[8]
			.word 0,0,0,0 // m_bJobAuditsEnabled 0x60 96

.global g_WwsJob_firstBufferNum
g_WwsJob_firstBufferNum: .word 0,0,0,0

.global g_WwsJob_numFreeBuffers
g_WwsJob_numFreeBuffers: .word 0,0,0,0

.global g_WwsJob_bBreakpointRequested
g_WwsJob_bBreakpointRequested: .word 0,0,0,0

.global g_WwsJob_timeStamp
g_WwsJob_timeStamp: .word 0,0,0,0

.global g_WwsJob_eaWorkLoad
g_WwsJob_eaWorkLoad: .word 0,0,0,0

.global g_WwsJob_spursWorkloadId
g_WwsJob_spursWorkloadId: .word 0,0,0,0

.global g_WwsJob_jobIndex
g_WwsJob_jobIndex: .word 0,0,0,0

//Surely this only needs 1 qword, not 2?
.global g_WwsJob_jobHeader
g_WwsJob_jobHeader: .word 0,0,0,0, 0,0,0,0

.global g_WwsJob_jobDataArray
g_WwsJob_jobDataArray:	.word 0,0,0xFFFF,0, 0,0,0,0
			.word 0,0,0xFFFF,0, 0,0,0,0
			.word 0,0,0xFFFF,0, 0,0,0,0

.global g_currentJobHeaderCacheEa
g_currentJobHeaderCacheEa: .word 0,0,0,0

.global g_dependencyCache
g_dependencyCache: .word 0,0,0,0

.global g_currentDependencyCacheEa
g_currentDependencyCacheEa: .word 0,0,0,0

.global g_sideStack
g_sideStack: .word 0,0,0,0, 0,0,0,0

.global g_cltrjLrStore
g_cltrjLrStore:	.word 0,0,0,0
	
.global g_nullDmaInterrupt
g_nullDmaInterrupt: .word 0x80000000, 0x00000000, 0x00000000, 0x00000000

.global g_BisledRegisterStore
g_BisledRegisterStore: .word 0,0,0,0

.global g_countToNextPoll
g_countToNextPoll: .word 0,0,0,0

.global g_stackSetupValue
g_stackSetupValue: .word 0x3FFD0, 0x1FC0, 0, 0

.global g_ilaMarkerShuffMask
g_ilaMarkerShuffMask: .word 0x00010405, 0x08090C0D, 0x00010405, 0x08090C0D
