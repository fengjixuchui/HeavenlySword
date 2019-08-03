/*
 * Copyright (c) 2003-2006 Naughty Dog, Inc. 
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#ifndef ICE_PERFEVENTS_H
#define ICE_PERFEVENTS_H

namespace Ice
{
	namespace Render
	{

		enum DomainType
		{
			kDomainHost      = 0,
			kDomainFrontend  = 1,
			kDomainRop       = 2,
			kDomainBackend   = 3,
			kDomainMemory    = 4,
			kDomainIoif      = 5
		};
		
		enum HostEventType
		{
			//! Number of 64-bit transfers received by the pusher from system 
			//! memory.
			/*! This event identifies the amount of traffic that enters the 
			    Host Interface unit through the AGP interface. 
			*/
			kHostEventAgpToPusher = 1,
			//! Number of 64-bit transfers received by the pusher from 
			//! graphics memory.
			/*! This experiment identifies the amount of traffic that enters 
			    the Host Interface unit through the frame buffer interface.
			*/
			kHostEventFbToPusher  = 2,
			kNumHostEvents
		};
		
		enum FrontendEventType
		{
			//! Cycles during which Vertex Processing Engine (VPE) 0 is 
			//! stalling because of data dependencies, waiting for instruction 
			//! to complete.
			kFrontendEventInstructionStallVpe0Short          = 1,
			//! Number of primitives (triangles, lines, points) sent from 
			//! Setup to Coarse raster.
			kFrontendEventSetupToRasterActive                = 2,
			kFrontendEventSetupToRasterIdle                  = 3,
			//! Cycles during which Setup is stalled by Coarse raster.
			kFrontendEventSetupToRasterStalling              = 4,
			//! Cycles during which Setup starves Coarse raster.
			kFrontendEventSetupToRasterStarving              = 5,
			kFrontendEventVpe0ThreadDone0                    = 6,
			kFrontendEventVpe0ThreadDone1                    = 7,
			kFrontendEventVpe0ThreadDone2                    = 8,
			//! Number of instructions issued on Vertex Processing 
			//! Engine (VPE) 0.
			kFrontendEventInstructionIssuedVpe0              = 9,
			kFrontendEventVpcToStpActive                     = 10,
			//! Cycles during which Viewport Cull Block (VPC) is stalled by 
			//! Setup.
			kFrontendEventVpcToStpStalling                   = 11,
			kFrontendEventVpcToStpIdle                       = 12,
			kFrontendEventVpcToStpStarving                   = 13,
			kFrontendEventCoardGridLineCull                  = 14,
			//! Number of Vertex Texture Fetch (VTF) cache misses.
			kFrontendEventVtfCacheMiss                       = 15,
			//! Number of Vertex Texture Fetch (VTF) requests.
			kFrontendEventVtfRequest                         = 16,
			kFrontendEventDepthCullToFRasterActive           = 17,
			kFrontendEventDepthCullToFRasterStalling         = 18,
			kFrontendEventDepthCullToFRasterTransactionCount = 19,
			//! Number of primitives processed in the geometry subsystem.  
			/*! This experiment counts points, lines, and triangles.
			*/
			kFrontendEventPrimitiveCount                     = 20,
			//! Cycles during which Attribute RAM (ATR) is transferring data 
			//! to Color Assembly (CAS).
			kFrontendEventAtrToCasActive                     = 21,
			//! Cycles during which Attribute RAM (ATR) is stalled waiting 
			//! for Color Assembly (CAS).
			kFrontendEventAtrToCasStalling                   = 22,
			//! Count the number of 8x8 pixel tiles sent from Coarse to Fine 
			//! Raster.
			kFrontendEventCRasterOutputTileCount             = 23,
			kFrontendEventCRasterSearchMode                  = 24,
			kFrontendEventCRasterToCasStalling               = 25,
			kFrontendEventCRasterToDepthCullActive           = 26,
			kFrontendEventCRasterToDepthCullStalling         = 27,
			kFrontendEventFrontendToIdxActive                = 28,
			kFrontendEventFrontendToIdxStalling              = 29,
			kFrontendEventFrontendToIdxIdle                  = 30,
			kFrontendEventFrontendToIdxStarving              = 31,
			kFrontendEventIdxToVabCmdActive                  = 32,
			kFrontendEventIdxToVabCmdStalling                = 33,
			kFrontendEventIdxToVabCmdIdle                    = 34,
			kFrontendEventIdxToVabCmdStarving                = 35,
			kFrontendEventIdxToVabDinActive                  = 36,
			kFrontendEventIdxToVabDinStalling                = 37,
			kFrontendEventIdxBypassFifoFull                  = 38,
			//! Number of vertex attributes that are fetched from Index Vertex 
			//! Processor (IDX) and passed down to the geometry subsystem.
			kFrontendEventIdxGeomAttributeCount              = 39,
			//! Number of hits in the Index Vertex Processor (IDX) 
			//! post-transform cache.
			kFrontendEventIdxVertexCacheHit                  = 40,
			//! Number of misses in the Index Vertex Processor (IDX) 
			//! post-transform cache.
			kFrontendEventIdxVertexCacheMiss                 = 41,
			kFrontendEventIdxDmaStalled                      = 42,
			//! Cycles during which Index Vertex Processor (IDX) is waiting for
			//! memory data on Frame Buffer Interface (FBI) or PMI.
			kFrontendEventIdxWaitForMemData                  = 43,
			//! Number of points (not lines or triangles) sent from Setup 
			//! to Coarse raster.
			kFrontendEventSetupPoints                        = 44,
			//! Number of primitives (triangles, lines, points) culled in 
			//! Setup.
			kFrontendEventSetupCulls                         = 45,
			//! Number of lines (not triangles or points) sent from Setup 
			//! to Coarse raster.
			kFrontendEventSetupLines                         = 46,
			//! Number of triangles (not lines or points) sent from Setup 
			//! to Coarse raster.
			kFrontendEventSetupTriangles                     = 47,
			//! Number of primitives culled by by Viewport Cull Block (VPC).
			kFrontendEventVpcCullPrimitive                   = 48,
			//! Number of triangles culled by Viewport Cull Block (VPC) due to 
			//! having all vertices out of the frustum.
			kFrontendEventVpcCullFrustum                     = 49,
			//! Number of small triangles culled by Viewport Cull Block (VPC) 
			//! due to having zero area.
			kFrontendEventVpcCullZeroArea                    = 50,
			//! Number of small triangles culled for backface by Viewport Cull 
			//! Block (VPC).
			kFrontendEventVpcCullBackface                    = 51,
			//! Number of primitives too large for Viewport Cull Block (VPC) 
			//! to cull.
			kFrontendEventVpcCullTooLarge                    = 52,
			//! Number of primitives in Viewport Cull Block (VPC) with some, 
			//! but not all, vertices out of the frustum.
			kFrontendEventVpcCullWouldClip                   = 53,
			kFrontendEventIdxFetchNoDma                      = 54,
			kFrontendEventIdxFetchFbi                        = 55,
			kFrontendEventIdxFetchSysram                     = 56,
			kFrontendEventIdxDinStalled                      = 57,
			kFrontendEventVpe0DataStall0                     = 58,
			kFrontendEventVpe0DataStall1                     = 59,
			kFrontendEventVpe0DataStall2                     = 60,
			kFrontendEventVpe0OtherStall0                    = 61,
			kFrontendEventVpe0OtherStall1                    = 62,
			kFrontendEventVpe0OtherStall2                    = 63,
			//! Cycles during which Viewport Cull Block (VPC) is stalled 
			//! waiting for an index buffer.
			kFrontendEventAllIBufsUsed                       = 64,
			kFrontendEventWaitLoadingIBuf                    = 65,
			kFrontendEventWairForOBuf                        = 66,
			kFrontendEventVabToVpcBackPressure               = 67,
			kFrontendEventVabToVpeBackPressure               = 68,
			kFrontendEventSharedBusInUse                     = 69,
			kFrontendEventWaitForVpeFlush                    = 70,
			kFrontendEventVabCommandLine                     = 71,
			//! Number of post-transform cache misses.
			kFrontendEventVabCommandLoad                     = 72,
			kFrontendEventVabCommandPoint                    = 73,
			kFrontendEventVabCommandTriangle                 = 74,
			//! Cycles during which Viewport Cull Block (VPC) is stalled 
			//! waiting for the 4-entry fifo.
			kFrontendEventFifoIsFullStall                    = 75,
			kFrontendEventNoBackRefCountStall                = 76,
			kFrontendEventOBufPerVpe                         = 77,
			kFrontendEventIdxToVabDinStarving                = 78,
			kFrontendEventColorRasterToCasActive             = 79,
			kFrontendEventRasterizedPixelCount1              = 80,
			kFrontendEventRasterizedPixelCount2              = 81,
			kFrontendEventPostTransformCacheHit1             = 82,
			kFrontendEventPostTransformCacheHit2             = 83,
			kFrontendEventPostTransformCacheMiss1            = 84,
			kFrontendEventPostTransformCacheMiss2            = 85,
			kFrontendEventIdxVertexCount1                    = 86,
			kFrontendEventIdxVertexCount2                    = 87,
			kNumFrontendEvents
		};
		
		enum RopEventType
		{
			kRopEventColorRopWaitsForInterlock                  = 1,
			//! Number of Color Raster Operations (CROP) compress writes that 
			//! succeed.
			kRopEventColorRopWriteCompress                      = 2,
			kRopEventDepthReadInterlockStall                    = 3,
			kRopEventDepthWriteCompressOkCompressedCompressed   = 4,
			kRopEventDepthWriteCompressOkNoneCompressed         = 5,
			kRopEventDepthWriteCompressOkUncompressedCompressed = 6,
			kRopEventDepthWriteTotal                            = 7,
			kNumRopEvents
		};

		enum BackendEventType
		{
			kBackendEventShaderNumPasses                        = 1,
			kBackendEventCasToStriActive                        = 2,
			kBackendEventCasToStriStalling                      = 3,
			kBackendEventClipIdToSqdStalling                    = 4,
			kBackendEventSqdEndSegAny                           = 5,
			kBackendEventSqdEndSegCmbIpuTimeout                 = 6,
			kBackendEventSqdEndSegRastTimeout                   = 7,
			kBackendEventSqdEndSegRegisterLimit                 = 8,
			kBackendEventSqdEndSegShaderBundle                  = 9,
			kBackendEventSqdEndSegSlotLimitForSqc               = 10,
			kBackendEventSqdEndSegTriangleLimit                 = 11,
			kBackendEventSqdPacketCovered8x4                    = 12,
			kBackendEventSqdPacketImTwoTexFourQuad              = 13,
			kBackendEventSqdPacketNoCoverage8x4                 = 14,
			kBackendEventSqdPacketNonImTwoTexFourQuad           = 15,
			kBackendEventStriToSqdActive                        = 16,
			kBackendEventStriToSqdIdle                          = 17,
			kBackendEventStriToSqdStalling                      = 18,
			kBackendEventStriToSqdStarving                      = 19,
			kBackendEventTexToSrbStalling                       = 20,
			//! Cycles spent resolving data cache conflicts resulting from 
			//! non-power of 2 wrapped textures.
			kBackendEventTextureCacheBankConflict               = 21,
			//! Cycles during which a quad is accessing more than four unique 
			//! texture cache lines using GT4 mode.
			kBackendEventTextureCacheGt4Count                   = 22,
			//! Cycles during which Texture stalls because multiple texture 
			//! cache lines map to the same set.
			kBackendEventTextureCacheSetIndexCollision          = 23,
			kBackendEventTextureCoardLodFootprintCount          = 24,
			kBackendEventTextureDeadlockAvoidanceStallsPipe     = 25,
			kBackendEventTextureFineLodFootprintCount           = 26,
			kBackendEventTextureFootprintCount                  = 27,
			kBackendEventTextureAnisoCount1                     = 28,
			kBackendEventTextureAnisoCount2                     = 29,
			kBackendEventTextureAnisoCount4                     = 30,
			kBackendEventTextureAnisoCount6                     = 31,
			kBackendEventTextureAnisoCount8                     = 32,
			kBackendEventTextureAnisoCount10                    = 33,
			kBackendEventTextureAnisoCount12                    = 34,
			kBackendEventTextureAnisoCount14                    = 35,
			kBackendEventTextureAnisoCount16                    = 36,
			kBackendEventTextureQuadCount                       = 37,
			kBackendEventTextureWaitsForMCache                  = 38,
			kBackendEventTextureRBufferFull                     = 39,
			kBackendEventTextureXBufferFull                     = 40,
			kBackendEventMCacheSet0MissCount                    = 41,
			kBackendEventMCacheSet0RequestCount                 = 42,
			kBackendEventMCacheSet0InuseFbStall                 = 43,
			kBackendEventMCacheSet0InUseCachelineStall          = 44,
			kBackendEventMCacheSet1MissCount                    = 45,
			kBackendEventMCacheSet1RequestCount                 = 46,
			kBackendEventMCacheSet1InUseCachelineStall          = 47,
			kBackendEventMCacheSet1InUseFbStall                 = 48,
			kBackendEventMCacheSet2MissCount                    = 49,
			kBackendEventMCacheSet2RequestCount                 = 50,
			kBackendEventMCacheSet2InUseCachelineStall          = 51,
			kBackendEventMCacheSet2InuseFbStall                 = 52,
			kBackendEventMCacheTex0ClientFifoFull               = 53,
			kBackendEventMCacheTex0DataArbStall                 = 54,
			kBackendEventMCacheTex0DataTokenStall               = 55,
			kBackendEventMCacheTex0TagStall                     = 56,
			kBackendEventMCacheVertexClientFifoFull             = 57,
			kBackendEventMCacheVertexTagStall                   = 58,
			kBackendEventRaster2DWaitsForShader                 = 59,
			kBackendEventSctToTexActive                         = 60,
			kBackendEventSctToTexStalling                       = 61,
			kBackendEventSctToTexIdle                           = 62,
			kBackendEventSctToTexStarving                       = 63,
			kBackendEventShaderAllKillQuads                     = 64,
			kBackendEventShaderAllQuads                         = 65,
			kBackendEventShaderFirstPassQuads                   = 66,
			kBackendEventShaderFirstPassKillQuads               = 67,
			kBackendEventShaderComputationBottomWaitsForRegisterFile0 = 68,
			kBackendEventShaderGateKeeperWaitsForInstructionProcessor0 = 69,
			kBackendEventShaderInstructionFetchFromTexture      = 70,
			kBackendEventShaderInstructionFetchNoTexture        = 71,
			kBackendEventShaderRemapperAndBackendWaitsForRegisterFile0 = 72,
			kBackendEventShaderTriangleAttributeCount           = 73,
			//! Number of 8x4 packets covered (even partially) by Fine Raster.
			kBackendEventFineRasterTilesCovered                 = 74,
			//! Number of 8x4 packets with no coverage by Fine Raster.
			kBackendEventFineRasterTilesUncovered               = 75,
			//! Number of 8x4 packets received by the Shader Quad 
			//! Distributor (SQD) from Raster for fastz (quad8).
			kBackendEventFineRasterTilesFastDepth               = 76,
			//! The number of state bundles seen by the fine raster.
			kBackendEventFineRasterBundles                      = 77,
			kBackendEventSqdToSqcActive                         = 78,
			kBackendEventSqdToSqcIdle                           = 79,
			kBackendEventSqdToSqcStalling                       = 80,
			kBackendEventSqdToSqcStarving                       = 81,
			kBackendEventSqcToPropActive                        = 82,
			kBackendEventSqcToPropIdle                          = 83,
			kBackendEventSqcToPropStalling                      = 84,
			kBackendEventSqcToPropStarving                      = 85,
			kBackendEventPropToCBufStalling                     = 86,
			kBackendEventPropToCBufStarving                     = 87,
			kBackendEventCoalesceIdle                           = 88,
			kBackendEventCoalesceNewTile                        = 89,
			kBackendEventCoalesceBundle                         = 90,
			kBackendEventCoalesceReplaceTile                    = 91,
			kBackendEventCoalesceJoin                           = 92,
			kBackendEventCoalesceFillTile                       = 93,
			kBackendEventCoalesceHwGlobalTile                   = 94,
			kBackendEventCoalesceHwPartitionTile                = 95,
			kBackendEventCoalesceColorStall                     = 96,
			kBackendEventCoalesceDepthStall                     = 97,
			kBackendEventCoalesceClipQuad                       = 98,
			kBackendEventCoalesceReplaceBecauseOfBackStencil    = 99,
			kBackendEventTexToSrbStarving                       = 100,
			kBackendEventSrfToSqc0Starving                      = 101,
			kBackendEventTexToSrbActive                         = 102,
			kBackendEventSrfToSqc0Active                        = 103,
			kNumBackendEvents
		};

		enum MemoryEventType
		{
			kMemoryEventFbRwCount1                              = 1,
			kMemoryEventFbRwCount2                              = 2,
			kMemoryEventFbDramcAcpd1st                          = 3,
			kMemoryEventFbDramcAcpd2nd                          = 4,
			kMemoryEventFbDramcAct1st                           = 5,
			kMemoryEventFbDramcAct2nd                           = 6,
			kMemoryEventFbDramcIdle1st                          = 7,
			kMemoryEventFbDramcIdle2nd                          = 8,
			kMemoryEventFbDramcMisc1st                          = 9,
			kMemoryEventFbDramcMisc2nd                          = 10,
			kMemoryEventFbDramcNoWait1st                        = 11,
			kMemoryEventFbDramcNoWait2nd                        = 12,
			kMemoryEventFbDramcPre1st                           = 13,
			kMemoryEventFbDramcPre2nd                           = 14,
			kMemoryEventFbDramcRef1st                           = 15,
			kMemoryEventFbDramcRef2nd                           = 16,
			kMemoryEventFbDramcRead1st                          = 17,
			kMemoryEventFbDramcRead2nd                          = 18,
			kMemoryEventFbDramcRwCount1st                       = 19,
			kMemoryEventFbDramcRwCount2nd                       = 20,
			kMemoryEventFbDramcWrite1st                         = 21,
			kMemoryEventFbDramcWrite2nd                         = 22,
			kMemoryEventFbDramcDcmp1st                          = 23,
			kMemoryEventFbDramcDcmp2nd                          = 24,
			kMemoryEventFbDramcWait1st                          = 25,
			kMemoryEventFbDramcWait2nd                          = 26,
			kMemoryEventFbRmwStateIdle                          = 27,
			kMemoryEventFbRmwStateTagWait                       = 28,
			kMemoryEventFbRmwStateRmwWait                       = 29,
			kMemoryEventFbRmwStateStalled                       = 30,
			kMemoryEventFbRmwStateRmwFastClearCompressed        = 31,
			kMemoryEventFbRmwStateRmwFastClearExpanded          = 32,
			kMemoryEventFbRmwStateCompressedWrite               = 33,
			kMemoryEventFbRmwStateRmwNormalWrite                = 34,
			kMemoryEventFbRmwStateRmwStart                      = 35,
			kMemoryEventFbRmwStateWriteOther                    = 36,
			kMemoryEventFbRmwStateConditionalRead               = 37,
			kMemoryEventFbRmwStateRmwCompressedRead             = 38,
			kMemoryEventFbRmwStateRmwExpandedRead               = 39,
			kMemoryEventFbRmwStateRmwNormalRead                 = 40,
			kMemoryEventFbFaClientColorRopRead                  = 41,
			kMemoryEventFbFaClientColorRopWrite                 = 42,
			kMemoryEventFbFaClientGrfe                          = 43,
			kMemoryEventFbFaClientHost                          = 44,
			kMemoryEventFbFaClientTcdma                         = 45,
			kMemoryEventFbFaClientTex                           = 46,
			kMemoryEventFbFaClientDepthRopRead                  = 47,
			kMemoryEventFbFaClientDepthRopWrite                 = 48,
			kMemoryEventFbDramcState1stIdle                     = 49,
			kMemoryEventFbDramcState1stAct                      = 50,
			kMemoryEventFbDramcState1stPre                      = 51,
			kMemoryEventFbDramcState1stRef                      = 52,
			kMemoryEventFbDramcState1stMrs                      = 53,
			kMemoryEventFbDramcState1stRead1Subp                = 54,
			kMemoryEventFbDramcState1stRead2Subp                = 55,
			kMemoryEventFbDramcState1stWrite1Subp               = 56,
			kMemoryEventFbDramcState1stWrite2Subp               = 57,
			kMemoryEventFbDramcState1stDecmp                    = 58,
			kMemoryEventFbDramcState1stAcpd                     = 59,
			kMemoryEventFbDramcState1stMisc                     = 60,
			kMemoryEventFbDramcState1stWait                     = 61,
			kMemoryEventFbDramcState2ndIdle                     = 62,
			kMemoryEventFbDramcState2ndAct                      = 63,
			kMemoryEventFbDramcState2ndPre                      = 64,
			kMemoryEventFbDramcState2ndRef                      = 65,
			kMemoryEventFbDramcState2ndMrs                      = 66,
			kMemoryEventFbDramcState2ndRead1Subp                = 67,
			kMemoryEventFbDramcState2ndRead2Subp                = 68,
			kMemoryEventFbDramcState2ndWrite1Subp               = 69,
			kMemoryEventFbDramcState2ndWrite2Subp               = 70,
			kMemoryEventFbDramcState2ndDecmp                    = 71,
			kMemoryEventFbDramcState2ndAcpd                     = 72,
			kMemoryEventFbDramcState2ndMisc                     = 73,
			kMemoryEventFbDramcState2ndWait                     = 74,
			kMemoryEventFbDramcWaitNotWait                      = 75,
			kMemoryEventFbDramcWaitPdex                         = 76,
			kMemoryEventFbDramcWaitR2WTurn                      = 77,
			kMemoryEventFbDramcWaitW2RTurn                      = 78,
			kMemoryEventFbDramcWaitRmwi                         = 79,
			kMemoryEventFbDramcWaitR2PTurn                      = 80,
			kMemoryEventFbDramcWaitW2PTurn                      = 81,
			kMemoryEventFbDramcWaitRcd                          = 82,
			kMemoryEventFbDramcWaitRext                         = 83,
			kMemoryEventFbDramcWaitRp                           = 84,
			kMemoryEventFbDramcWaitRc                           = 85,
			kMemoryEventFbDramcWaitRas                          = 86,
			kMemoryEventFbDramcWaitRfc                          = 87,
			kMemoryEventFbDramcWaitAp                           = 88,
			kMemoryEventFbDramcWaitData                         = 89,
			kMemoryEventFbDramcWaitOther                        = 90,
			kMemoryEventFbRmwClientColorRop                     = 91,
			kMemoryEventFbRmwClientDepthRop                     = 92,
			kMemoryEventFbRmwClientTex                          = 93,
			kMemoryEventFbRmwClientClipId                       = 94,
			kMemoryEventFbRmwClientTcdma                        = 95,
			kMemoryEventFbRmwClientGrfe                         = 96,
			kMemoryEventFbRmwClientHost                         = 97,
			kMemoryEventFbRmwClientOther                        = 98,
			kMemoryEventFbRmwStateHpTagWait                     = 99,
			kMemoryEventFbRmwStateCompFc                        = 100,
			kMemoryEventFbRmwStateExpFc                         = 101,
			kMemoryEventFbRmwStateCompWrite                     = 102,
			kMemoryEventFbRmwStateNormWrite                     = 103,
			kMemoryEventFbRmwStateCondRead                      = 104,
			kMemoryEventFbRmwStateCompRead                      = 105,
			kMemoryEventFbRmwStateExpRead                       = 106,
			kMemoryEventFbRmwStateNormRead                      = 107,
			kMemoryEventFbRmwStateDiscard                       = 108,
			kMemoryEventFbRmwStateUnknown                       = 109,
			kMemoryEventFbFaHpClientOtherRead                   = 110,
			kMemoryEventFbFaHpClientOtherWrite                  = 111,
			kMemoryEventFbFaHpClientCrtc1Read                   = 112,
			kMemoryEventFbFaHpClientCurs1Read                   = 113,
			kMemoryEventFbFaHpClientCrtc2Read                   = 114,
			kMemoryEventFbFaHpClientCurs2Read                   = 115,
			kMemoryEventFbFaHpClientScalRead                    = 116,
			kMemoryEventFbFaHpClientVipRead                     = 117,
			kMemoryEventFbFaHpClientVipWrite                    = 118,
			kMemoryEventFbFaHpClientSpare0                      = 119,
			kMemoryEventFbFaHpClientSpare1                      = 120,
			kMemoryEventFbFaHpClientTexRead                     = 121,
			kMemoryEventFbFaHpClientDepthRopRead                = 122,
			kMemoryEventFbFaHpClientDepthRopWrite               = 123,
			kMemoryEventFbFaHpClientColorRopRead                = 124,
			kMemoryEventFbFaHpClientColorRopWrite               = 125,
			kMemoryEventFbFaLpClientOtherRead                   = 126,
			kMemoryEventFbFaLpClientOtherWrite                  = 127,
			kMemoryEventFbFaLpClientHostRead                    = 128,
			kMemoryEventFbFaLpClientHostWrite                   = 129,
			kMemoryEventFbFaLpClientClipIdRead                  = 130,
			kMemoryEventFbFaLpClientClipIdWrite                 = 131,
			kMemoryEventFbFaLpClientGrfeRead                    = 132,
			kMemoryEventFbFaLpClientGrfeWrite                   = 133,
			kMemoryEventFbFaLpClientMpegRead                    = 134,
			kMemoryEventFbFaLpClientMpegWrite                   = 135,
			kMemoryEventFbFaLpClientTcdmaRead                   = 136,
			kMemoryEventFbFaLpClientGartRead                    = 137,
			kMemoryEventFbFaLpClientSpare0                      = 138,
			kMemoryEventFbFaLpClientSpare1                      = 139,
			kMemoryEventFbFaLpClientPrivRead                    = 140,
			kMemoryEventFbFaLpClientPrivWrite                   = 141,
			kMemoryEventFbFaStateIdle                           = 142,
			kMemoryEventFbFaStateGntSameClient                  = 143,
			kMemoryEventFbFaStateGntNewClient                   = 144,
			kMemoryEventFbFaStateStallSameClient                = 145,
			kMemoryEventFbFaStateStallNewClient                 = 146,
			kNumMemoryEvents
		};
		
		enum IoifEventType
		{
			kIoifEventLatencyBucket0               = 1,
			kIoifEventLatencyBucket1               = 2,
			kIoifEventLatencyBucket2               = 3,
			kIoifEventLatencyBucket3               = 4,
			kIoifEventVirtualChannel0WriteData32B  = 5,
			kIoifEventVirtualChannel1WriteData32B  = 6,
			kIoifEventVirtualChannel0ReadData32B   = 7,
			kIoifEventVirtualChannel1ReadData32B   = 8,
			kIoifEventVirtualChannel0              = 9,
			kIoifEventVirtualChannel1              = 10,
			kIoifEventIoifToCcuRequestPacketValid  = 11,
			kIoifEventIoifToCcuRequestPacketReady  = 12,
			kIoifEventUcpVc0Request                = 13,
			kIoifEventUcpVc1Request                = 14,
			kIoifEventUcpVc0NoTag                  = 15,
			kIoifEventUcpVc1NoCredit               = 16,
			kIoifEventUcpVc0FenceWait              = 17,
			kIoifEventUcpVc1FenceWait              = 18,
			kIoifEventIoifToCcuDataPacketValid     = 19,
			kIoifEventIoidToCcuDataPacketReady     = 20,
			kNumIoifEvents
		};
	}
}

#endif // ICE_PERFEVENTS_H
