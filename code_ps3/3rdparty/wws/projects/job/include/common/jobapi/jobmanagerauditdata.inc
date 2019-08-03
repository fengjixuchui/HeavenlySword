/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		Define the enums and text strings for all audits of the job manager.
				Note that this is not a standard header file for normal inclusion by a user.
				"jobmanagerauditids.h" should be used instead
**/
//--------------------------------------------------------------------------------------------------

#ifndef AUDIT_DATA
#error jobmanagerauditdata.inc is not a standard header file for user inclusion
#endif

//--------------------------------------------------------------------------------------------------


// keep this audit 0
AUDIT_DATA( kNopAudit,											"(NopAudit):" )



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// the following audits are always in the wwsjob manager to know who to assign time to (but the user can disable them dynamically)
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Warning: Please don't change kWwsJob_begin, kWwsJob_end, kWwsJob_ChangeLoadToRunJob_jobBegin or kWwsJob_ChangeLoadToRunJob_jobEnd
//	in any significnat manner as this would break our compatibitliy with PA Suite.  These enum values must remain as 1,2,7,8
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// cycles assigned to wwsjob (overhead)
AUDIT_DATA( kWwsJob_begin,		 		  						"\n\n\nbegin:, pad, eaWorkload=U32, pad32" )
// cycles assigned to spursKernel
AUDIT_DATA( kWwsJob_end,					   					"end:" )

// push state, cycles assigned to wwsjob (interrupts)
AUDIT_DATA( kWwsJob_InterruptHandler_begin,						"InterruptHandler_begin:" )
// cycles assigned to (pop state)
AUDIT_DATA( kWwsJob_InterruptHandler_end,						"InterruptHandler_end:" )

//These two are only actually used as bookmarks for the PA
//This is the genuine start of the interrupt handler
//The two above audits above aren't actually written until after all the registers have been stored off
AUDIT_DATA( kWwsJob_InterruptHandler_real_begin,					"InterruptHandler_real_begin:" )
AUDIT_DATA( kWwsJob_InterruptHandler_real_end,						"InterruptHandler_real_end:" )

// cycles assigned to job (data is also provided to know which job)
AUDIT_DATA( kWwsJob_ChangeLoadToRunJob_jobBegin,	  	 		"\nChangeLoadToRunJob_jobBegin: jobIndex=U16 uniqueID=U64" )
// cycles assigned to wwsjob (overhead)
AUDIT_DATA( kWwsJob_ChangeLoadToRunJob_jobEnd,					"ChangeLoadToRunJob_jobEnd:\n" )

// cycles assigned to wwsjob (Api)
AUDIT_DATA( kWwsJob_JobApi_begin,					  	 		"JobApi_begin:" )
// cycles assigned to job
AUDIT_DATA( kWwsJob_JobApi_end,						 	 		"JobApi_end:" )

// cycles assigned to spursKernel
AUDIT_DATA( kWwsJob_PollSpursKernel_begin,						"PollSpursKernel_begin:" )
// cycles assigned to wwsjob
AUDIT_DATA( kWwsJob_PollSpursKernel_end,						"PollSpursKernel_end:" )

// cycles assigned to spursKernel
AUDIT_DATA( kWwsJob_setSpursKernelReadyCount_begin_zero,		"setSpursKernelReadyCount_begin (set to 0):" )
// cycles assigned to wwsjob
AUDIT_DATA( kWwsJob_setSpursKernelReadyCount_end_zero,				"setSpursKernelReadyCount_end (set to 0):" )

// cycles assigned to spursKernel
AUDIT_DATA( kWwsJob_setSpursKernelReadyCount_begin_old,			"setSpursKernelReadyCount_begin (restore to old value):" )
// cycles assigned to wwsjob
AUDIT_DATA( kWwsJob_setSpursKernelReadyCount_end_old,				"setSpursKernelReadyCount_end (restore to old value):" )

// cycles assigned to job (DMA/pipeline)
AUDIT_DATA( kWwsJob_WwsMain_waitForLoadDma_begin,			"WwsMain_waitForLoadDma_begin:" )
// cycles assigned to wwsjob
AUDIT_DATA( kWwsJob_WwsMain_waitForLoadDma_end,			        "WwsMain_waitForLoadDma_end:" )

// cycles assigned to job (DMA/pipeline)
AUDIT_DATA( kWwsJob_WwsMain_waitForStoreDma_begin,			"WwsMain_waitForStoreDma_begin:" )
// cycles assigned to wwsjob
AUDIT_DATA( kWwsJob_WwsMain_waitForStoreDma_end,			"WwsMain_waitForStoreDma_end:" )

// cycles assigned to wwsjob
AUDIT_DATA( kWwsJob_WwsMain_beginLoop,   						"WwsMain_beginLoop: loadJobState=U16" )

// cycles assigned to wwsjob, retries assigned to job (DMA/pipeline)
AUDIT_DATA( kWwsJob_JobApi_TryExecuteCmds_begin,   					"JobApi_TryExecuteCmds_begin:" )
// cycles assigned to job (DMA)
AUDIT_DATA( kWwsJob_JobApi_TryExecuteCmds_end,   					"JobApi_TryExecuteCmds_end:" )
// cycles assigned to wwsjob api calls
AUDIT_DATA( kWwsJob_JobApi_TryExecuteCmdsDma_end,  					"JobApi_TryExecuteCmdsDma_end:" )

//////////////////////////////////////////////////////////////////////////
// the following "Important" audits may (or may not) be compiled into the code delivered to the studios
// (but the user can disable them dynamically)
//////////////////////////////////////////////////////////////////////////

AUDIT_DATA( kWwsJob_WwsInterrupts_begin,						"WwsInterrupts_begin: pad pad32 interruptDmaTagMask=U32" )

AUDIT_DATA( kWwsJob_JobApi_executeCommands,						"JobApi_executeCommands:" )
AUDIT_DATA( kWwsJob_JobApi_loadNextJob,							"JobApi_loadNextJob:" )
AUDIT_DATA( kWwsJob_JobApi_bufferSets,							"JobApi_bufferSets: words =dump" )
AUDIT_DATA( kWwsJob_JobApi_logicalToBufNums,					"JobApi_logicalToBufNums: bytes =dump" )
AUDIT_DATA( kWwsJob_JobApi_buffers,								"JobApi_buffers: dwords=dump" )
AUDIT_DATA( kWwsJob_JobApi_stopLoadingJobs,						"JobApi_stopLoadingJobs:" )

AUDIT_DATA( kWwsJob_CopyStoreShareBufsToRun_copyBufSetToRun,	"CopyStoreShareBufsToRun_copyBufSetToRun: bufSet#=U16, pad32 bufSetData=U32" )
AUDIT_DATA( kWwsJob_CopyStoreShareBufsToRun_afterCopy,			"CopyStoreShareBufsToRun_afterCopy: bufSet#=U08, buf#=U08, runBuf=U64, storeBuf=U64" )

AUDIT_DATA( kWwsJob_TryDumpAllStoreShareBufs_activeShareBuffer,	"TryDumpAllStoreShareBufs_activeShareBuffer: bufSet#=U08, buf#=U08, buf=U64" )

//NOTE: this following important audit could not be included in the downcoded version due to 
// size and register coloring issues:
AUDIT_DATA( kWwsJob_TryDumpShareBuf_writeShareBuf,				"TryDumpShareBuf_writeShareBuf: pageNum=U08, numPages=U08, mmLength=U24, dmaTagId=U08, mmAdrs=U32" )

AUDIT_DATA( kWwsJob_TryChangeFreeToLoadJob_readLoadCmds,		"TryChangeFreeToLoadJob_readLoadCmds: pad mmaWorkLoad=U32, jobIndex=U32, mmaLoadCmds=U32, loadCmdSize=U32" )

AUDIT_DATA( kWwsJob_TryChangeStoreToFreeJob_clearUsed,			"TryChangeStoreToFreeJob_clearUsed: bufSet#=U08, buf#=U08, buf=U64" )
AUDIT_DATA( kWwsJob_TryChangeStoreToFreeJob_inactivateNonShareBuf, "TryChangeStoreToFreeJob_inactivateNonShareBuf: bufSet#=U08, buf#=U08, buf=U64" )
AUDIT_DATA( kWwsJob_TryChangeStoreToFreeJob_jobDone,			"TryChangeStoreToFreeJob_jobDone: storeJob#=U16" )

AUDIT_DATA( kWwsJob_TryExecuteLoadCmds,							"TryExecuteLoadCmds:" )

AUDIT_DATA( kWwsJob_TryExecuteCmds_reserveBufSet,				"TryExecuteCmds_reserveBufSet: bufSet#=U16, pad32 page#=U08, #pages/buf=U08, reserved=U01, #bufs=U07, firstBuf#=U08" )
AUDIT_DATA( kWwsJob_TryExecuteCmds_unreserveBufSet,				"TryExecuteCmds_unreserveBufSet: bufSet#=U16, pad32 page#=U08, #pages/buf=U08, reserved=U01, #bufs=U07, firstBuf#=U08" )
AUDIT_DATA( kWwsJob_TryExecuteCmds_useBuffer,					"TryExecuteCmds_useBuffer: inputRead=U01, gather=U01, outputShare=U01, shareWriteIfDiscard=U01, pad03 bufSet#=U04, logicalBuf#=U05" )
AUDIT_DATA( kWwsJob_TryExecuteCmds_foundMatch,					"TryExecuteCmds_foundMatch: buf#=U16, prevBuf=U64, buf(tmp)=U64" )
AUDIT_DATA( kWwsJob_TryExecuteCmds_takeBuf,						"TryExecuteCmds_takeBuf: buf#=U16, page#=U16, #pages=U16, lsaBufDword=U32" )
AUDIT_DATA( kWwsJob_TryExecuteCmds_prevJobBufShareCollide,		"TryExecuteCmds_prevJobBufShareCollide: prevJobBufSet#=U08, prevJobBuf#=U08" )
AUDIT_DATA( kWwsJob_TryExecuteCmds_gatherRead,					"TryExecuteCmds_gatherRead: pad lsListElements=U32, mmAdrs=U32, mmLength=U32, dmaTagId=U32" )
AUDIT_DATA( kWwsJob_TryExecuteCmds_read,						"TryExecuteCmds_read: pad lsAdrs=U32, mmAdrs=U32, mmLength=U32, dmaTagId=U32" )
AUDIT_DATA( kWwsJob_TryExecuteCmds_readyToRunJob,				"TryExecuteCmds_readyToRunJob: page#=U16, lsaBufDword=U32, lsaParams=U32, paramDword=U64" )
AUDIT_DATA( kWwsJob_TryExecuteCmds_cmdCantExecute,				"TryExecuteCmds_cmdCantExecute:" )

AUDIT_DATA( kWwsJob_FreeLogicalBuffer,							"FreeLogicalBuffer: bufSet#=U08, buf#=U08" )

AUDIT_DATA( kWwsJob_GetBufferTag,								"GetBufferTag: bufSet#=U16, buf#=U32, useDmaTagId=U32" )
AUDIT_DATA( kWwsJob_GetBufferTag_returnBufTag,					"GetBufferTag_returnBufTag: pad16 pad16 dmaTagId=U16, lsaWords=U16, lsLenWords=U16, mmAdrs=U32, mmLen=U32" )

AUDIT_DATA( kWwsJob_FreeDmaTadId,								"FreeDmaTagId: dmaTagId=U16, pad32 dmaTags=U32" )

AUDIT_DATA( kWwsJob_UseDmaTagId,								"UseDmaTagId:, dmaTagId=U16, pad32 dmaTags=U32" )




//////////////////////////////////////////////////////////////////////////
// the following "Verbose" audits may (or may not) be compiled into the code for C++ debugging at ICE
// (but the user can disable them dynamically)
//	These are *not* available to studios
//	If you enable these, you should also enable the Important audits
//////////////////////////////////////////////////////////////////////////

AUDIT_DATA( kWwsJob_Initialize,	 		  						"Initialize: pad mmaWorkLoad=U32, lsaLoadCmds=U32, lsaBufSets=U32 \nlsaBufs=U32, pad32 lsaLogicalToBufNums=U32" )

AUDIT_DATA( kWwsJob_WwsInterrupts_readLoadCommandsDone,			"WwsInterrupts_readLoadCommandsDone:" )
AUDIT_DATA( kWwsJob_WwsInterrupts_writeRunJobShareBufsDone,		"WwsInterrupts_writeRunJobShareBufsDone:" )
AUDIT_DATA( kWwsJob_WwsInterrupts_writeAllStoreJobShareBufsDone,"WwsInterrupts_writeAllStoreJobShareBufsDone:" )
AUDIT_DATA( kWwsJob_WwsInterrupts_writeStoreJobBuffersDone,		"WwsInterrupts_writeStoreJobBuffersDone:" )
AUDIT_DATA( kWwsJob_WwsInterrupts_tryToExecuteLoadCommands,		"WwsInterrupts_tryToExecuteLoadCommands:" )
AUDIT_DATA( kWwsJob_WwsInterrupts_end,							"WwsInterrupts_end:" )

//AUDIT_DATA( kWwsJob_JobApi_tryToExecuteJobCommands,				"JobApi_tryToExecuteJobCommands: lsaCmdsInWords=U16" )
//AUDIT_DATA( kWwsJob_JobApi_waitForRunJobReadsToFinish,			"JobApi_waitForRunJobReadsToFinish:" )
AUDIT_DATA( kWwsJob_JobApi_getBufferTag,						"JobApi_getBufferTag:" )
AUDIT_DATA( kWwsJob_JobApi_getBufferTags,						"JobApi_getBufferTags: pad lsaBuf=U32, #entries=U32" )
AUDIT_DATA( kWwsJob_JobApi_useDmaTagId,							"JobApi_useDmaTagId:" )
AUDIT_DATA( kWwsJob_JobApi_freeDmaTagId,						"JobApi_freeDmaTagId: dmaTagId=U16" )
AUDIT_DATA( kWwsJob_JobApi_freeLogicalBuffer,					"JobApi_freeLogicalBuffer:" )
AUDIT_DATA( kWwsJob_JobApi_freeLogicalBuffers,					"JobApi_freeLogicalBuffers: pad lsaBuf=U32, #entries=U32" )
AUDIT_DATA( kWwsJob_JobApi_tryToExecuteLoadCommands,			"JobApi_tryToExecuteLoadCommands:" )

AUDIT_DATA( kWwsJob_CopyStoreShareBufsToRun, 					"CopyStoreShareBufsToRun: runJob#=U08, lastStoreJob#=U08" )
AUDIT_DATA( kWwsJob_CopyStoreShareBufsToRun_bufSetActive,		"CopyStoreShareBufsToRun_bufSetActive: bufSet#=U16, pad32 page#=U08, #pages/buf=U08, reserved=U01, #bufs=U07, firstBuf#=U08" )
AUDIT_DATA( kWwsJob_CopyStoreShareBufsToRun_runPagesEverReserved,"CopyStoreShareBufsToRun_runPagesEverReserved: pad pages=dump" )
AUDIT_DATA( kWwsJob_CopyStoreShareBufsToRun_bufSet,				"CopyStoreShareBufsToRun_bufSet: bufSet#=U16, runBufSet=U32, storeBufSet=U32" )
AUDIT_DATA( kWwsJob_CopyStoreShareBufsToRun_storeBufSetCollides,"CopyStoreShareBufsToRun_storeBufSetCollides:" )
AUDIT_DATA( kWwsJob_CopyStoreShareBufsToRun_bufIndex,			"CopyStoreShareBufsToRun_bufIndex: bufIndex(w/job#)=U16, buf=U64" )
AUDIT_DATA( kWwsJob_CopyStoreShareBufsToRun_reservedBuf,		"CopyStoreShareBufsToRun_reservedBuf: bufSet#=U16, bufSet=U32, buf#=U32" )
AUDIT_DATA( kWwsJob_CopyStoreShareBufsToRun_prevJobBufSet,		"CopyStoreShareBufsToRun_prevJobBufSet: prevJobBufsetCompatible=U16, pad32 prevJobBufSet=U32, prevJobBuffer=U64" )
//AUDIT_DATA( kWwsJob_CopyStoreShareBufsToRun_tooFewBufsToPromote,"CopyStoreShareBufsToRun_tooFewBufsToPromote:" )
//AUDIT_DATA( kWwsJob_CopyStoreShareBufsToRun_runBufReserved,		"CopyStoreShareBufsToRun_runBufReserved: buf#=U16, storeBuf=dump" )
AUDIT_DATA( kWwsJob_CopyStoreShareBufsToRun_copyShareStoreBufToRun,"CopyStoreShareBufsToRun_copyShareStoreBufToRun:" )
AUDIT_DATA( kWwsJob_CopyStoreShareBufsToRun_prevJobBufUsed,		"CopyStoreShareBufsToRun_prevJobBufUsed:" )

AUDIT_DATA( kWwsJob_TryDumpAllStoreShareBufs,					"TryDumpAllStoreShareBufs:" )
AUDIT_DATA( kWwsJob_TryDumpAllStoreShareBufs_buf,				"TryDumpAllStoreShareBufs_buf: buf#(with job#)=U16, buf=U64" )
AUDIT_DATA( kWwsJob_TryDumpAllStoreShareBufs_activeBuf,			"TryDumpAllStoreShareBufs_activeBuf: bufSet#=U16, bufSet=U32, buf#=U32" )
AUDIT_DATA( kWwsJob_TryDumpAllStoreShareBufs_outputWriteAllShareBufsInt, "TryDumpAllStoreShareBufs_outputWriteAllShareBufsInt:" )

AUDIT_DATA( kWwsJob_TryDumpShareBuf,							"TryDumpShareBuf: pad, buf=U64" )
AUDIT_DATA( kWwsJob_TryDumpShareBuf_clearWriteIfDiscard,		"TryDumpShareBuf_clearWriteIfDiscard: pad buf=U64" )
AUDIT_DATA( kWwsJob_TryDumpShareBuf_bufDmaTagIdExists,			"TryDumpShareBuf_bufDmaTagIdExists: bufDmaTagId=U16" )
AUDIT_DATA( kWwsJob_TryDumpShareBuf_ongoingDma,					"TryDumpShareBuf_ongoingDma:" )
AUDIT_DATA( kWwsJob_TryDumpShareBuf_inactivateBuf,				"TryDumpShareBuf_inactivateBuf: pad buf=U64" )

AUDIT_DATA( kWwsJob_TryChangeFreeToLoadJob,						"TryChangeFreeToLoadJob:" )
AUDIT_DATA( kWwsJob_TryChangeFreeToLoadJob_jobExists,			"TryChangeFreeToLoadJob_jobExists: loadJob#=U16, loadCmdSize=U16, jobHdrCmd=U16, mmaLoadCmds=U32" )
AUDIT_DATA( kWwsJob_TryChangeFreeToLoadJob_clearBufSetsAndBufs,	"TryChangeFreeToLoadJob_clearBufSetsAndBufs: loadJob#=U16, lsaBufSets=U24, lsaBufs=U20, lsaLog2Buf#s=U20" )
AUDIT_DATA( kWwsJob_TryChangeFreeToLoadJob_incTimeStamp,		"TryChangeFreeToLoadJob_incTimeStamp: timeStamp=U16" )

AUDIT_DATA( kWwsJob_ChangeRunToStoreJob,						"ChangeRunToStoreJob: storeJob#=U16" )
AUDIT_DATA( kWwsJob_ChangeRunToStoreJob_changeDone,				"ChangeRunToStoreJob_changeDone: loadJobState=U16" )
AUDIT_DATA( kWwsJob_ChangeRunToStoreJob_ongoingDma,				"ChangeRunToStoreJob_ongoingDma:" )

AUDIT_DATA( kWwsJob_TryChangeStoreToFreeJob,		   		 	"TryChangeStoreToFreeJob: storeJob#=U16" )
AUDIT_DATA( kWwsJob_TryChangeStoreToFreeJob_buf,				"TryChangeStoreToFreeJob_buf: buf#(with job#)=U16, buf=U64" )
AUDIT_DATA( kWwsJob_TryChangeStoreToFreeJob_reservedBufNowInactive,	"TryChangeStoreToFreeJob_reservedBufNowInactive:" )
AUDIT_DATA( kWwsJob_TryChangeStoreToFreeJob_activeBuf,			"TryChangeStoreToFreeJob_activeBuf: bufSet#=U16, bufSet=U32, buf#=U32" )
AUDIT_DATA( kWwsJob_TryChangeStoreToFreeJob_bufUsed,			"TryChangeStoreToFreeJob_bufUsed:" )
AUDIT_DATA( kWwsJob_TryChangeStoreToFreeJob_ongoingDma,			"TryChangeStoreToFreeJob_ongoingDma:" )
AUDIT_DATA( kWwsJob_TryChangeStoreToFreeJob_ongoingDmas,		"TryChangeStoreToFreeJob_ongoingDmas:" )

AUDIT_DATA( kWwsJob_TryFreeTagAndUsedPages,						"TryFreeTagAndUsedPages: buf#=U16, buf=U64" )
AUDIT_DATA( kWwsJob_TryFreeTagAndUsedPages_tagExists,			"TryFreeTagAndUsedPages_tagExists: dmaTagId=U16" )
AUDIT_DATA( kWwsJob_TryFreeTagAndUsedPages_ongoingDma,			"TryFreeTagAndUsedPages_ongoingDma:" )
AUDIT_DATA( kWwsJob_TryFreeTagAndUsedPages_nonShareBufFreeTag,	"TryFreeTagAndUsedPages_nonShareBufFreeTag:" )

AUDIT_DATA( kWwsJob_TryChangeStoreToFreeJob_nowFree, 			"TryChangeStoreToFreeJob_nowFree: minActive=U16, pad32 mmaWorkload=U32" )

AUDIT_DATA( kWwsJob_TryExecuteLoadCmds_runDmaOngoing,			"TryExecuteLoadCmds_runDmaOngoing:" )
AUDIT_DATA( kWwsJob_TryExecuteLoadCmds_allCmdsFinished,			"TryExecuteLoadCmds_allCmdsFinished:" )
AUDIT_DATA( kWwsJob_TryExecuteLoadCmds_end,  					"TryExecuteLoadCmds_end:" )

AUDIT_DATA( kWwsJob_TryExecuteCmds,			  					"TryExecuteCmds: prevJob#=U08, job#=U08, pad32 lsaCmds=U32" )
AUDIT_DATA( kWwsJob_TryExecuteCmds_tryReserveBufSet,			"TryExecuteCmds_tryReserveBufSet: bufSet#=U16" )
AUDIT_DATA( kWwsJob_TryExecuteCmds_tryUnreserveBufSet,			"TryExecuteCmds_tryUnreserveBufSet: bufSetMask=U16" )
AUDIT_DATA( kWwsJob_TryExecuteCmds_continueAfterDumpShare,		"TryExecuteCmds_continueAfterDumpShare:" )
AUDIT_DATA( kWwsJob_TryExecuteCmds_findInputMatch,				"TryExecuteCmds_findInputMatch:" )
AUDIT_DATA( kWwsJob_TryExecuteCmds_getBufToTake,				"TryExecuteCmds_getBufToTake:" )
AUDIT_DATA( kWwsJob_TryExecuteCmds_bufIsReserved,				"TryExecuteCmds_bufIsReserved: buf#=U16" )
AUDIT_DATA( kWwsJob_TryExecuteCmds_noPagesUsed,					"TryExecuteCmds_noPagesUsed:" )
AUDIT_DATA( kWwsJob_TryExecuteCmds_sharedPagesAndPrevDiscards,	"TryExecuteCmds_sharedPagesAndPrevDiscards:" )
AUDIT_DATA( kWwsJob_TryExecuteCmds_prevJobBufAllocated,			"TryExecuteCmds_prevJobBufAllocated:" )
AUDIT_DATA( kWwsJob_TryExecuteCmds_takePreference,				"TryExecuteCmds_takePreference: pad pad32 takePreference=U32" )
AUDIT_DATA( kWwsJob_TryExecuteCmds_bestSoFar,					"TryExecuteCmds_bestSoFar: buf#=U16" )
AUDIT_DATA( kWwsJob_TryExecuteCmds_allBufsInUse,				"TryExecuteCmds_allBufsInUse:" )
AUDIT_DATA( kWwsJob_TryExecuteCmds_someSharePageCollisions,		"TryExecuteCmds_someSharePageCollisions:" )


AUDIT_DATA( kWwsJob_TryExecuteCmds_prevJobBuf,					"TryExecuteCmds_prevJobBuf: prevJobBufIndex=U16, prevJobBuf=U64" )
AUDIT_DATA( kWwsJob_TryExecuteCmds_prevJobBufShare,				"TryExecuteCmds_prevJobBufShare: prevJobBufSet#=U08, prevJobBuf#=U08, pad32 bufSet=U32" )
//AUDIT_DATA( kWwsJob_TryExecuteCmds_prevBufSetActive,			"TryExecuteCmds_prevBufSetActive: prevJob: bufSet#=U16, pad32 page#=U08, #pages/buf=U08, reserved=U01, #bufs=U07, firstBuf#=U08" )
//AUDIT_DATA( kWwsJob_TryExecuteCmds_prevBufShare,				"TryExecuteCmds_prevBufShare: prevJob: buf#=U16, buf=U64" )
AUDIT_DATA( kWwsJob_TryExecuteCmds_prevJobBufBeingDumped,		"TryExecuteCmds_prevJobBufBeingDumped:" )
AUDIT_DATA( kWwsJob_TryExecuteCmds_mustContinueLater,			"TryExecuteCmds_mustContinueLater: pad lsaBuf=U32, lsaLog2Buf#=U32, buf=U64" )
AUDIT_DATA( kWwsJob_TryExecuteCmds_inputReadOn,					"TryExecuteCmds_inputReadOn:" )
AUDIT_DATA( kWwsJob_TryExecuteCmds_storeShareDmaOngoing,		"TryExecuteCmds_storeShareDmaOngoing:" )
AUDIT_DATA( kWwsJob_TryExecuteCmds_inputGatherOn,				"TryExecuteCmds_inputGatherOn:" )
AUDIT_DATA( kWwsJob_TryExecuteCmds_finallySaveBuf,				"TryExecuteCmds_finallySaveBuf: logicalBuf#=U08, buf#=U08, buf=U64" )
AUDIT_DATA( kWwsJob_TryExecuteCmds_setLogicalToBufNum,			"TryExecuteCmds_setLogicalToBufNum: logicalBuf#=U08, buf#=U08, pad32 lsaLog2Buf#=U32" )
AUDIT_DATA( kWwsJob_TryExecuteCmds_clearBufSetReservedPages,	"TryExecuteCmds_clearBufSetReservedPages: bufSet#=U16" )
AUDIT_DATA( kWwsJob_TryExecuteCmds_requestDepDec,				"TryExecuteCmds_requestDepDec: pad mmaDependency=U32" )
AUDIT_DATA( kWwsJob_TryExecuteCmds_runJob,						"TryExecuteCmds_runJob:" )
AUDIT_DATA( kWwsJob_TryExecuteCmds_startWriteShareBufInt,		"TryExecuteCmds_startWriteShareBufInt:" )
AUDIT_DATA( kWwsJob_TryExecuteCmds_abortRunSomeCmdsNotDone,		"TryExecuteCmds_abortRunSomeCmdsNotDone:" )
AUDIT_DATA( kWwsJob_TryExecuteCmds_endCmd,						"TryExecuteCmds_endCmd:" )

AUDIT_DATA( kWwsJob_ChangeLoadToRunJob,							"ChangeLoadToRunJob: loadJob#=U16" )
AUDIT_DATA( kWwsJob_ChangeLoadToRunJob_aboutToRunJob,			"ChangeLoadToRunJob_aboutToRunJob:, pad lsaJobCodeBuf=U32, spursWorkloadId=U16, jobIndex=U16" )

AUDIT_DATA( kWwsJob_FreeLogicalBuffer_setInactive,				"FreeLogicalBuffer_setInactive: pad buf=U64" )

AUDIT_DATA( kWwsJob_GetBufferTag_sharedToLaterJob,				"GetBufferTag_sharedToLaterJob: next: job#=U08, buf#(w/job#)=U08, buf=U64" )
AUDIT_DATA( kWwsJob_GetBufferTag_gotDmaTagFromLoadJob,			"GetBufferTag_gotDmaTagFromLoadJob: dmaTagId=U16, loadJobBuf=U64" )
AUDIT_DATA( kWwsJob_GetBufferTag_gotDmaTag,						"GetBufferTag_gotDmaTag: dmaTagId=U16, buf=U64" )

AUDIT_DATA( kWwsJob_GetLogicalBuffer,							"GetLogicalBuffer: job#=U04, logicalBufSet#=U06, logicalBuf#=U06" )
AUDIT_DATA( kWwsJob_GetLogicalBuffer_bufferSet,					"GetLogicalBuffer_bufferSet: pad lsaBufSet=U32, page#=U08, #pages/buf=U08, reserved=U01, #bufs=U07, firstBuf#=U08" )
AUDIT_DATA( kWwsJob_GetLogicalBuffer_buffer,					"GetLogicalBuffer_buffer: buf#=U08, page#=U08, p8Bufs=U24, pBuf=U20, pLog2Buf#s=U20, buf=U64" )

AUDIT_DATA( kWwsJob_GetJobFromJobList,							"GetJobFromJobList:" )
AUDIT_DATA( kWwsJob_GetJobFromJobList_noWork,					"GetJobFromJobList_noWork:" )
AUDIT_DATA( kWwsJob_GetJobFromJobList_2ndTryHasWork,			"GetJobFromJobList_2ndTryHasWork:" )

AUDIT_DATA( kWwsJob_ReturnToSpursKernel,						"ReturnToSpursKernel:" )

AUDIT_DATA( kWwsJob_DisableInterrupts,							"DisableInterrupts:" )

AUDIT_DATA( kWwsJob_EnableInterrupts,							"EnableInterrupts:" )

AUDIT_DATA( kWwsJob_GetPageMasks,								"GetPageMasks: page#=U08, #pages=U08" )

AUDIT_DATA( kWwsJob_IsDmaTagMaskDone,							"IsDmaTagMaskDone: pad dmaTagMask=U32, done=U32" )

AUDIT_DATA( kWwsJob_SetDwordMem,								"SetDwordMem: pad lsaMem=U32, size=U32, data=U64" )

AUDIT_DATA( kWwsJob_SetQwordMem,								"SetQwordMem: pad lsaMem=U32, size=U32, data=U64" )

//AUDIT_DATA( kWwsJob_WwsMain_beginLoop,   						"WwsMain_beginLoop: loadJobState=U16" )
AUDIT_DATA( kWwsJob_WwsMain_noLoadJob,   						"WwsMain_noLoadJob: nextLoadJobNum=I16" )
AUDIT_DATA( kWwsJob_WwsMain_stillNoLoadJob, 					"WwsMain_stillNoLoadJob:" )
AUDIT_DATA( kWwsJob_WwsMain_noLoadOrStoreJob, 					"WwsMain_noLoadOrStoreJob:" )
AUDIT_DATA( kWwsJob_WwsMain_waitForStoreJobShareBufsDma,		"WwsMain_waitForStoreJobShareBufsDma:" )
//AUDIT_DATA( kWwsJob_WwsMain_waitTillLoadJobReadyToRun, 			"WwsMain_waitTillLoadJobReadyToRun:" )
//AUDIT_DATA( kWwsJob_WwsMain_waitTillNoStoreJob,		 			"WwsMain_waitTillNoStoreJob:" )

// these are used in many routines
AUDIT_DATA( kWwsJob_dmaTags,	   								"dmaTags: pad pad32 dmaTags=U32" )
AUDIT_DATA( kWwsJob_reservedPages,								"reservedPages:  pad reserved pages=dump" )
AUDIT_DATA( kWwsJob_usedPages,									"usedPages:      pad used     pages=dump" )
AUDIT_DATA( kWwsJob_shareablePages,								"shareablePages: pad shareablePages=dump" )

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
