/*
 * Copyright (c) 2005 Naughty Dog, Inc.
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#include "icemeshinternal.h"

using namespace Ice;
using namespace Ice::MeshProc;

#include <stdlib.h>

namespace
{
	/**
	 * This array contains the number of halfword arguments (including
	 * the command halfword) in each command.  It's absolutely
	 * critical that this array stay in sync with the kCmd* enum in
	 * icemesh.h!!!  These values could be hard-coded in SPU code, but
	 * having them centrally located in the C version makes things
	 * much cleaner.
	 */
	U8 g_pCmdArgCounts[kCmdNumCommands] =
		{
			1, // kCmdCleanupAndExit
			0, // kCmdJump -- Is zero so as not to move the pointer forward after the jump.
			0, // kCmdCall -- Is zero so as not to move the pointer forward after the jump.
			0, // kCmdReturn -- Is zero so as not to move the pointer forward after the jump.
			2, // kCmdSetupNvControlInfo
			2, // kCmdSetupObjectInfo
			3, // kCmdSetupVertexInfo
			3, // kCmdSetupPmVertexInfo
			3, // kCmdSetupFormatInfo
			4, // kCmdSetupNvStream
			2, // kCmdSetupSwStream
			3, // kCmdSetupCustomCompress
			4, // kCmdSetupIndexes
			7, // kCmdSetupPixelShader
			2, // kCmdSetupViewportInfo
			2, // kCmdSetupRootTransformInfo
			2, // kCmdSetupShadowInfo
			3, // kCmdSetupEdges
			2, // kCmdSetupContinuousPmInfo
			2, // kCmdSetupDiscretePmInfo
			3, // kCmdSetupPmParent
			2, // kCmdSetupDmInfo
			3, // kCmdSetupDmDisplacements
			10,// kCmdSetupSkinning
			3, // kCmdSetupMatrices
			4, // kCmdStartInputListDma
			3, // kCmdSetupRunTable
			2, // kCmdSetupDeltaFormatInfo
			3, // kCmdBlendDeltaStream
			1, // kCmdStallOnInputDma
			1, // kCmdEndSetup
			1, // kCmdPerformDm
			1, // kCmdSkinObject
			1, // kCmdPerformPm
			1, // kCmdExtrudeShadows
			1, // kCmdTrimIndexes
			1, // kCmdTrimVertexes
			1, // kCmdOutputIndexes
			2, // kCmdInsertAttributeIntoNvStream
			1, // kCmdOutputCopiedNvStream
			1, // kCmdOutputConvertedUniformTable
		};

	/**
	 * Jump table of handler functions for every command.  Once again,
	 * you'd better keep this in sync with the commands enum in
	 * icemesh.h...
	 */
	typedef void (*CmdHandler)(U32*, IceQuadWord, IceQuadWord);
	CmdHandler g_cmdHandlers[kCmdNumCommands] =
		{
			CmdCleanupAndExit, // kCmdCleanupAndExit
			CmdJump,
			CmdCall,
			CmdReturn,
			CmdSetupNvControlInfo,
			CmdSetupObjectInfo,
			CmdSetupVertexInfo,
			CmdSetupPmVertexInfo,
			CmdSetupFormatInfo,
			CmdSetupNvStream,
			CmdSetupSwStream,
			CmdSetupCustomCompress,
			CmdSetupIndexes,
			CmdSetupPixelShader,
			CmdSetupViewportInfo,
			CmdSetupRootTransformInfo,
#if ICE_MESH_STENCIL_SHADOWS_ON
			CmdSetupShadowInfo,
			CmdSetupEdges,
#else
			NULL,
			NULL,
#endif // ICE_MESH_STENCIL_SHADOWS_ON
			CmdSetupContinuousPmInfo,
			CmdSetupDiscretePmInfo,
			CmdSetupPmParent,
			CmdSetupDmInfo,
			CmdSetupDmDisplacements,
			CmdSetupSkinning,
			CmdSetupMatrices,
			CmdStartInputListDma,
			CmdSetupRunTable,
			CmdSetupDeltaFormatInfo,
			CmdBlendDeltaStream,
			CmdStallOnInputDma,
			CmdEndSetup,
			CmdPerformDm,
			CmdSkinObject,
			CmdPerformPm,
#if ICE_MESH_STENCIL_SHADOWS_ON
			CmdExtrudeShadows,
#else
			NULL,
#endif // ICE_MESH_STENCIL_SHADOWS_ON
			CmdTrimIndexes,
			CmdTrimVertexes,
			CmdOutputIndexes,
			CmdInsertAttributeIntoNvStream,
			CmdOutputCopiedNvStream,
			CmdOutputConvertedUniformTable,
		};
}

/**
 * Interpret an SPU command list.
 *
 * @param pCommandList Word-aligned pointer to command list. The first
 *                     entry is the length of the list in bytes; the
 *                     rest of the commands and their arguments follow.
 */
void Ice::MeshProc::InterpretCommandList(U32 *pStaticMem)
{
	// Skip command list size (2 bytes)
	pStaticMem[kStaticCmdParsePtr] = pStaticMem[kStaticCmdPtr] + 2;

	// The command ID and flags of the current command are stored in
	// the first halfword of cmdQuads[0].  The rest of cmdQuads[0]
	// contains the first seven arguments of the command. cmdQuads[1]
	// contains arguments 8-15.  This prevents each command from
	// having to shuffle its own arguments into place.
	IceQuadWord cmdQuad1, cmdQuad2;
	INSERT_AUDIT(kMeshAuditCmdStart, 0, 0);

	do
	{
		U16 *pCmdData = (U16 *)pStaticMem[kStaticCmdParsePtr];
		cmdQuad1 = IceQuadWord(pCmdData[0], pCmdData[1], pCmdData[2], pCmdData[3], pCmdData[4], pCmdData[5], pCmdData[6], pCmdData[7]);
		cmdQuad2 = IceQuadWord(pCmdData[8], pCmdData[9], pCmdData[10], pCmdData[11], pCmdData[12], pCmdData[13], pCmdData[14], pCmdData[15]);
//		memcpy(&cmdQuad1, (void *)pStaticMem[kStaticCmdParsePtr], sizeof(IceQuadWord));
//		memcpy(&cmdQuad2, (void *)(pStaticMem[kStaticCmdParsePtr] + sizeof(IceQuadWord)), sizeof(IceQuadWord));
		U8 cmdId    = cmdQuad1.m_data.u8.A;
		ICE_ASSERT(cmdId < kCmdNumCommands);

#ifdef __SPU__
#if COLLECT_MESH_STATS
		U32 value = 0;
		switch(cmdId) {
			case kCmdCleanupAndExit:
				/*???*/value = 0;
				break;
			case kCmdJump:
			case kCmdCall:
			case kCmdReturn:
			case kCmdSetupNvControlInfo:
			case kCmdSetupObjectInfo:
			case kCmdSetupVertexInfo:
			case kCmdSetupPmVertexInfo:
			case kCmdSetupFormatInfo:
			case kCmdSetupCustomCompress:
			case kCmdSetupPixelShader:
			case kCmdSetupViewportInfo:
			case kCmdSetupRootTransformInfo:
			case kCmdSetupShadowInfo:
			case kCmdSetupContinuousPmInfo:
			case kCmdSetupDiscretePmInfo:
			case kCmdSetupDmInfo:
			case kCmdStartInputListDma:
			case kCmdSetupDeltaFormatInfo:
			case kCmdStallOnInputDma:
			case kCmdEndSetup:
				value = 0;
				break;
			case kCmdSetupNvStream:
			case kCmdSetupSwStream:
			case kCmdPerformDm:
			case kCmdSkinObject:
				value = pStaticMem[kStaticVertexCount];
				break;
			case kCmdSetupIndexes:
			case kCmdSetupEdges:
			case kCmdSetupPmParent:
			case kCmdSetupDmDisplacements:
			case kCmdSetupMatrices:
				value = cmdQuad1.m_data.u16.C;
				break;
			case kCmdSetupSkinning:
				value = cmdQuad2.m_data.u16.B;
				break;
			case kCmdSetupRunTable:
				value = cmdQuad1.m_data.u16.C >> 1;
				break;
			case kCmdBlendDeltaStream:
				/*???*/value = 0;
				break;
			case kCmdPerformPm:
				/*???*/value = 0;
				break;
			case kCmdExtrudeShadows:
				value = pStaticMem[kStaticEdgeCount];
				break;
			case kCmdTrimIndexes:
			case kCmdTrimVertexes:
				value = (pStaticMem[kStaticIndexCount] - pStaticMem[kStaticHaloIndexCount]) / 3;
				break;
			case kCmdOutputIndexes:
				value = pStaticMem[kStaticIndexCount] - pStaticMem[kStaticHaloIndexCount];
				break;
			case kCmdInsertAttributeIntoNvStream:
			case kCmdOutputCopiedNvStream:
			case kCmdOutputConvertedUniformTable:
				if (pStaticMem[kStaticReindexPtr] > 0)
					value = pStaticMem[kStaticReindexCount];
				else
					value = pStaticMem[kStaticOutputVertexCount];
				break;
			default:
				value = 0;
		}
#endif
#endif

		// Handle the command
		g_cmdHandlers[cmdId](pStaticMem, cmdQuad1, cmdQuad2);

#if COLLECT_MESH_STATS
		INSERT_AUDIT(kMeshAuditCmd, cmdId, value);
#endif

		// Increment the command parse pointer appropriately
		pStaticMem[kStaticCmdParsePtr] += g_pCmdArgCounts[cmdId] * 2; // 2 bytes per arg
	} while(cmdQuad1.m_data.u8.A != kCmdCleanupAndExit);
}

void Ice::MeshProc::CmdJump(U32* pStaticMem, IceQuadWord cmdQuad1, IceQuadWord cmdQuad2)
{
	CMD_LOG(cmdQuad1, cmdQuad2);

	// Sets the command parser pointer to the offset in the input buffer specified in the jump command.
	pStaticMem[kStaticCmdParsePtr] = pStaticMem[kStaticInputBufferPtr] + cmdQuad1.m_data.u16.B;
}

void Ice::MeshProc::CmdCall(U32* pStaticMem, IceQuadWord cmdQuad1, IceQuadWord cmdQuad2)
{
	CMD_LOG(cmdQuad1, cmdQuad2);

	// The call stack has a maximum depth of two.
	U32 callDepth = pStaticMem[kStaticCmdParseCallDepth];
	ICE_ASSERT(callDepth < 2);

	// Push the next command parse address onto the call stack.
	pStaticMem[kStaticCmdParseCallDepth] = callDepth + 1;
	pStaticMem[kStaticCmdParseCallStack + callDepth] = pStaticMem[kStaticCmdParsePtr] + 4;

	// Sets the command parser pointer to the offset in the input buffer specified in the jump command.
	pStaticMem[kStaticCmdParsePtr] = pStaticMem[kStaticInputBufferPtr] + cmdQuad1.m_data.u16.B;
}

void Ice::MeshProc::CmdReturn(U32* pStaticMem, IceQuadWord cmdQuad1, IceQuadWord cmdQuad2)
{
	CMD_LOG(cmdQuad1, cmdQuad2);

	// The call stack has a maximum depth of two.
	U32 callDepth = pStaticMem[kStaticCmdParseCallDepth];
	ICE_ASSERT(callDepth > 0);

	// Pop the command parser address off of the call stack.
	pStaticMem[kStaticCmdParseCallDepth] = callDepth - 1;
	pStaticMem[kStaticCmdParsePtr] = pStaticMem[kStaticCmdParseCallStack + callDepth - 1];
}

#if DEBUG_LOG_COMMANDS
void Ice::MeshProc::LogCommand(const char *funcName, IceQuadWord cmdQuad1, IceQuadWord cmdQuad2)
{
	PRINTF("Calling %s with args:\n", funcName);
	U32 numArgs = g_pCmdArgCounts[cmdQuad1.m_data.u8.A];
	U16 *args = (U16*)&cmdQuad1;
	U32 argIndex = 0;
	for(U8 i=0; i<numArgs; ++i)
	{
		if (i == 8)
		{
			args = (U16*)&cmdQuad2;
			PRINTF("\n");
		}
		PRINTF("\t0x%04X", args[i & 0x7]);
	}
	PRINTF("\n");
}
#endif

#ifndef __SPU__
/**
 * Debug routine that can identify a variety of common problems
 * with command lists, including (but not limited to):
 * - Attempts to execute invalid commands.
 * - Attempts to execute commands in an invalid order.
 * - Attempts to execute mesh processing commands without
 *   previously calling the proper xfer commands to set up the
 *   data.
 * - Unterminated or oversized command lists
 *
 * @param pCommandList The command list to validate.
 *
 * @return false if a problem is detected with the command list;
 *         true otherwise. It goes without saying that we can't
 *         detect all possible problems with command lists, so a
 *         return value of true is not a guarentee of safety!
 *         Perhaps this function should be called
 *         "IsProbablyValidCommandList"...
 */
bool IsValidCommandList(const U16 *pCommandList)
{
	const U16 reportedSize = pCommandList[0];
	U16       actualSize   = 2;
	const U16 kMaximumSize = 256;

	const U16 *pCommandToParse = pCommandList+1;
	U8 lastCommandId           = 0;

	bool hasNvControl          = false;
	bool hasObjectInfo         = false;
	U32  hasVertexInfo         = 0;     // these two need to be U32s
	U32  hasPmVertexInfo       = 0;     // so we can xor them (no ^^ operator!)
	bool hasNvStream           = false;
	bool hasIndex              = false;
	bool hasViewportInfo       = false;
	bool hasShadowInfo         = false;
	bool hasDmInfo             = false;
	bool hasDmDisplacements    = false;
	bool hasPmParentTable      = false;
	bool hasDiscretePmInfo     = false;
	bool hasContinuousPmInfo   = false;
	bool hasEdgeList           = false;
	bool hasSkinningTables     = false;
	bool hasMatrices           = false;
	bool hasStreamInWorkBuffer = false;

	bool isValid = true;

	U8 commandId = pCommandToParse[0] >> 8;
	while (commandId != kCmdCleanupAndExit)
	{
		// Invalid command IDs are a fatal error; we don't know how
		// far to advance to the next command.
		if (commandId >= kCmdNumCommands)
		{
			PRINTF("Error at cmds[%d]: Invalid command ID %d\n", actualSize/2, commandId);
			PRINTF("FATAL error detected; aborting validation.\n");
			isValid = false;
			return false;
		}

		// Each command ID must be >= the last one to ensure correct
		// processing.  There are a couple exceptions where a series
		// of 2-3 commands may be repeated an arbitrary number of
		// times.  For the purpose of numerical comparison, those
		// four routines are all treated as one.
		U8 effectiveCommandId = commandId;
		if (commandId >= kCmdStartInputListDma && commandId <= kCmdStallOnInputDma)
			effectiveCommandId = kCmdStartInputListDma;
		if (effectiveCommandId < lastCommandId)
		{
			PRINTF("Error at cmds[%d]: Command IDs must be in ascending order!\n", actualSize/2);
			isValid = false;
		}

		// Make sure the prequisites of the mesh processing commands
		// are met.

		// The following commands provide data needed by ALL mesh
		// processing commands:
		if (commandId == kCmdSetupNvControlInfo)
			hasNvControl = true;
		else if (commandId == kCmdSetupObjectInfo)
			hasObjectInfo = true;
		else if (commandId == kCmdSetupVertexInfo)
			hasVertexInfo = 1;
		else if (commandId == kCmdSetupPmVertexInfo)
			hasPmVertexInfo = 1;
		else if (commandId == kCmdSetupNvStream)
		{
			hasNvStream = true;
			if (pCommandToParse[0] & 0x01) // check copy bit
				hasStreamInWorkBuffer = true;
		}

		// See whether the basic prereqs for mesh processing have been met.
		bool hasBasicData = hasNvControl && (hasVertexInfo || hasPmVertexInfo) && hasNvStream;

		// The following commands provide data needed by specific
		// mesh processing commands:
		if (commandId == kCmdSetupViewportInfo)
			hasViewportInfo = true;
		else if (commandId == kCmdSetupIndexes)
			hasIndex = true;
		else if (commandId == kCmdSetupShadowInfo)
			hasShadowInfo = true;
		else if (commandId == kCmdSetupDmInfo)
			hasDmInfo = true;
		else if (commandId == kCmdSetupDmDisplacements)
			hasDmDisplacements = true;
		else if (commandId == kCmdSetupPmParent)
			hasPmParentTable = true;
		else if (commandId == kCmdSetupDiscretePmInfo)
			hasDiscretePmInfo = true;
		else if (commandId == kCmdSetupContinuousPmInfo)
			hasContinuousPmInfo = true;
		else if (commandId == kCmdSetupEdges)
			hasEdgeList = true;
		else if (commandId == kCmdSetupSkinning)
			hasSkinningTables = true;
		else if (commandId == kCmdSetupMatrices)
			hasMatrices = true;

		// Certain commands are invalid if their specific
		// prerequisites aren't met:
		if (commandId == kCmdSetupNvStream &&
			!(hasVertexInfo ^ hasPmVertexInfo))
		{
			PRINTF("Error at cmds[%d]: SetupNvStream requires ONE of CmdVertexInfo or CmdPmVertexInfo\n", actualSize/2);
			isValid = false;
		}
		else if (commandId == kCmdTrimVertexes &&
			!(hasBasicData && hasIndex && hasViewportInfo))
		{
			PRINTF("Error at cmds[%d]: TrimVertexes called without prerequisites\n", actualSize/2);
			isValid = false;
		}
		else if (commandId == kCmdTrimIndexes &&
			!(hasBasicData && hasIndex && hasViewportInfo))
		{
			PRINTF("Error at cmds[%d]: TrimIndexes called without prerequisites\n", actualSize/2);
			isValid = false;
		}

		else if (commandId == kCmdExtrudeShadows &&
			!(hasBasicData && hasIndex && hasShadowInfo && hasEdgeList))
		{
			PRINTF("Error at cmds[%d]: ExtrudeShadows called without prerequisites\n", actualSize/2);
			isValid = false;
		}
		else if (commandId == kCmdPerformDm &&
			!(hasBasicData && hasDmDisplacements && hasDmInfo))
		{
			PRINTF("Error at cmds[%d]: PerformDm called without prerequisites\n", actualSize/2);
			isValid = false;
		}
		else if (commandId == kCmdPerformPm &&
			!(hasBasicData && hasPmParentTable && (hasDiscretePmInfo || hasContinuousPmInfo)))
		{
			PRINTF("Error at cmds[%d]: PerformPm called without prerequisites\n", actualSize/2);
			isValid = false;
		}
		else if (commandId == kCmdPerformPm && hasContinuousPmInfo && !hasViewportInfo)
		{
			PRINTF("Error at cmds[%d]: PerformPm called without prerequisites\n", actualSize/2);
			isValid = false;
		}
		else if (commandId == kCmdSkinObject &&
			!(hasBasicData && hasSkinningTables && hasMatrices))
		{
			PRINTF("Error at cmds[%d]: SkinObject called without prerequisites\n", actualSize/2);
			isValid = false;
		}
		else if (commandId == kCmdInsertAttributeIntoNvStream &&
			!(hasBasicData && hasStreamInWorkBuffer))
		{
			PRINTF("Error at cmds[%d]: InsertAttributeIntoNvStream called without prerequisites\n", actualSize/2);
			isValid = false;
		}


		// Advance to next command
		lastCommandId = effectiveCommandId;
		actualSize += g_pCmdArgCounts[commandId]*2;
		pCommandToParse += g_pCmdArgCounts[commandId];
		commandId = pCommandToParse[0] >> 8;

		// Make sure the command list isn't too big
		if (actualSize > reportedSize)
		{
			PRINTF("Error at cmds[%d]: Size of command list %d is greater than reported size %d\n",
				actualSize/2, actualSize, reportedSize);
			PRINTF("FATAL error detected; aborting validation.\n");
			isValid = false;
			return false;
		}
		if (actualSize > kMaximumSize)
		{
			PRINTF("Error at cmds[%d]: Size of command list %d is greater than allowed maximum %d\n",
				actualSize/2, actualSize, kMaximumSize);
			PRINTF("                   (Possible unterminated command list?)\n");
			PRINTF("FATAL error detected; aborting validation.\n");
			isValid = false;
			return false;
		}
	}

	// It's normal for the reported size of the command to be padded
	// out to fill the last quadword.  However, if we find the command
	// to be terminated earlier than that, it's probably the case that
	// the command is being prematurely terminated.
	if (actualSize < reportedSize-16)
	{
		PRINTF("Warning at cmds[%d]: CleanupAndExit found earlier than expected (found at %d, expected at %d)\n", actualSize/2, actualSize/2, reportedSize/2);
	}

	return isValid;
}

#endif

