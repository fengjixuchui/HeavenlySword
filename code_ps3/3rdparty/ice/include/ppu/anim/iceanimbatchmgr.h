/*
 * Copyright (c) 2003-2005 Naughty Dog, Inc.
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#ifndef ICE_JOINT_ALLOC_H
#define ICE_JOINT_ALLOC_H

#include "icebase.h"
#include "icejointanim.h"
#include "icecommandstream.h"
#include "iceworkbuffer.h"

namespace Ice {
	namespace Anim {
		struct AnimClipGroupConstData;

		/*!
		 * \brief The batch manager is the entity that receives data-streams and commands 
		 * from animation code. Its job is to make sure that the commands are sent to 
		 * tasks on the SPU's, and that they are executed correctly.
		 *
		 * In doing so, it controls and sets up the memory maps while the SPU processes 
		 * its data. The output is a chain of DMA list commands. The SPU's will DMA these
		 * chains of data and commands into local memory, then it will execute the 
		 * operations given by the commands, and while it is doing this, it will DMA up
		 * the next DMA list block, in order for the SPU to embark onto the next block
		 * of data.
		 *
		 * The fundamental output of the batch manager is thus the DMA lists. The DMA 
		 * lists start with a header comprising four U32's:
		 *   - The location of the command list (after this DMA list has been transferred
		 *     to local memory).
		 *   - The location of the next DMA list header in the chain (again, after this 
		 *      DMA).
		 *   - The size in bytes for the following list-DMA.
		 * After the header, is the size+address list-DMA.
		 *
		 * The data being uploaded is initially a DMA list. This initial DMA list, when 
		 * executed, will upload the first memory area up to the input buffer, and thus 
		 * the chain is started.
		 *
		 * The command list is currently a stream of U16's, starting with the command 
		 * enumeration.  Based on the command, a variable amount of arguments can be 
		 * attached. 
		 *
		 * The class keeps track of all memory being used throughout the execution. Since 
		 * this is done ahead-of-time, the input locations and output locations are known, 
		 * and built into the command stream. 
		 *
		 * The memory is split into two input-buffers, one work buffer and two output-
		 * buffers. Input- and output-buffers are double-buffered. The work buffer is 
		 * dynamically allocated for temporary buffer space, JointParam buffers (SQT-data) 
		 * and JointTransform-buffers (SM). In addition we also allocate valid-bits (or the 
		 * state) of the blending operations.
		 */
		class BatchManager
		{
		public:
			typedef BatchJob::Location Location;
			static const BatchJob::Location kLocationNull = BatchJob::WorkBuffer::kLocationNull;

			/*!
			 * \brief Constructor.
			 * \param pBuffer		Pointer to a memory buffer in which to build the batch data
			 */
			BatchManager(BatchJob::MemoryBuffer *pBuffer);

			//! Restore this BatchManager to a pristine state.
			void Reset();

			//! Returns the command stream associated with this batch
			BatchJob::CommandStream& GetCommandStream() { return m_cmdStream; }
			BatchJob::CommandStream const& GetCommandStream() const { return m_cmdStream; }

			//! Returns the work buffer map associated with this batch
			BatchJob::WorkBuffer& GetWorkBuffer() { return m_workBuffer; }
			BatchJob::WorkBuffer const& GetWorkBuffer() const { return m_workBuffer; }

			//! Returns the work buffer location of the joint params table, or WorkBuffer::kLocationNull if it is not allocated
			Location GetJointParamsTableLocation() const { return m_jointParamsLoc; }
			//! Returns the work buffer location of the float channel table, or WorkBuffer::kLocationNull if it is not allocated
			Location GetFloatChannelTableLocation() const { return m_floatChannelLoc; }
			//! Returns the work buffer location of the scalar table, or WorkBuffer::kLocationNull if it is not allocated
			Location GetSdkScalarTableLocation() const { return m_sdkScalarTableLoc; }
			//! Returns the work buffer location of the sdk vector table, or WorkBuffer::kLocationNull if it is not allocated
			Location GetSdkVectorTableLocation() const { return m_sdkVectorTableLoc; }
			//! Returns the work buffer location of the joint transform table, or WorkBuffer::kLocationNull if it is not allocated
			//! Note that, we have internally allocated one extra JointTransform at the start of the table, and here return
			//! the location of the JointTransform for joint 0, not the extra 'root' JointTransform.
			Location GetJointTransformTableLocation() const;

			//! Create commands for an animation node tree.
			//! Allocates animated JointParams and float channel tables.
			//!	Returns false if work buffer space is exhausted.
			bool BatchTaskCommands(JointHierarchy const* pHierarchy, AnimationNode const* pRootNode);

			//! Create commands for parenting joint set iSet.
			//! Requires animated JointParams table.
			//! Allocates animated and procedural JointTransform (for iSet >= 1) tables if not already allocated.
			//!	Returns false if work buffer space is exhausted
			bool BatchParentingSet(JointHierarchy const* pHierarchy, unsigned iSet);

			//! Create commands for sending the partial default pose for procedural joints not set by any procedural commands.
			//! Requires procedural JointParams table.
			//! Allocates procedural JointTransform table if not already allocated.
			//!	Returns false if work buffer space is exhausted.
			bool BatchProceduralDefaultPose(JointHierarchy const* pHierarchy);

			//! Create driver commands from the driven keys in the hierarchy.
			//! Requires animated joint params and float channel tables.
			//! Creates sdk scalar table if not already allocated.
			//!	Returns false if work buffer space is exhausted.
			bool BatchSdkDriverCommands(JointHierarchy const* pHierarchy, float const* pInputControls);

			//! Create function commands from the driven keys in the hierarchy.
			//! Requires sdk scalar table.
			//!	Returns false if work buffer space is exhausted
			bool BatchSdkFunctionCommands(JointHierarchy const* pHierarchy);

			//! Create commands to do any direct float channel to output commands required by this hierarchy.
			//! Requires float channel table.
			//!	Returns false if work buffer space is exhausted
			bool BatchSdkBypassCommands(JointHierarchy const *pHierarchy, float *pOutputControls, U32& sdkBypassCount);

			//! Create driven commands from the driven keys in the hierarchy.
			//! Requires sdk scalar table.
			//! Creates procedural joint params and sdk vector tables if not already created.
			//!	Returns false if work buffer space is exhausted
			bool BatchSdkDrivenCommands(JointHierarchy const* pHierarchy, float* pOutputControls);

			//! Create commands from the constraints in the hierarchy.
			//! Requires animated joint transform table.  Requires sdk scalar, and sdk vector tables if any driven constraint attributes.
			//! Creates procedural joint params table if not already created.
			//!	Returns false if work buffer space is exhausted
			bool BatchConstraintCommands(JointHierarchy const* pHierarchy);

			//! Create commands to output world-space matrices.
			//! Requires animated and procedural joint transform tables.
			//!	Returns false if work buffer space is exhausted
			bool BatchOutputWorldMatricesCommands(JointHierarchy const* pHierarchy, SMath::Mat34 const* pObjectTransform, SMath::Mat44* pResultTransforms);

			//! Create commands to output matrices for skinning.
			//! Requires animated and procedural joint transform tables.
			//!	Returns false if work buffer space is exhausted
			bool BatchOutputSkinningMatricesCommands(JointHierarchy const* pHierarchy, SMath::Mat34 const* pObjectTransform, SMath::Mat44* pResultTransforms);

			//! Create procedural joint params table if it does not already exist.
			//!	Returns false if work buffer space is exhausted.
			bool AllocateProceduralJointParamsTable(JointHierarchy const* pHierarchy);
			//! Create animated joint transform table if it does not already exist.
			//!	Returns false if work buffer space is exhausted.
			bool AllocateAnimatedJointTransformTable(JointHierarchy const* pHierarchy);
			//! Create procedural joint transform table if it does not already exist.
			//!	Returns false if work buffer space is exhausted.
			bool AllocateProceduralJointTransformTable(JointHierarchy const* pHierarchy);
			//! Create sdk scalar table if it does not already exist.
			//!	Returns false if work buffer space is exhausted.
			bool AllocateSdkScalarTable(JointHierarchy const* pHierarchy);
			//! Create sdk vector table if it does not already exist.
			//!	Returns false if work buffer space is exhausted.
			bool AllocateSdkVectorTable(JointHierarchy const* pHierarchy);

			//! Frees the animated and procedural joint params tables.
			void FreeJointParamsTable();
			//! Frees the animated and procedural joint transform tables.
			void FreeJointTransformTable();
			//! Frees the float channel table.
			void FreeFloatChannelTable();
			//! Frees the sdk scalar table.
			void FreeSdkScalarTable();
			//! Frees the sdk vector table.
			void FreeSdkVectorTable();

			//! Finalizes this batched task, returning the batched task initial dma list.
			//! Frees all unfreed tables.
			void Finalize(U32 const*& pInitialDmaList, U32& initialDmaListSize);

			/*!	Uploads a binary buffer and stores it at locWorkBuffer in the work buffer.
			 *	locWorkBuffer will generally be a location returned by WorkBuffer::AllocHigh()
			 *	or WorkBuffer::AllocLow().
			 */
			void CreateUploadCommand(void const* pBuffer, U32F bufferSize, Location locWorkBuffer);

			/*!	Creates a command to execute the code at locCode with the arguments 
			 *	(locInputData, arg1, arg2, ...).  locCode will generally be a work buffer
			 *	location previously filled with a call to CreateUploadCommand() plus
			 *	the offset of an entry point to that code.
			 *	Note that this is a helper function, and that you are welcome to treat it
			 *	as example code to be duplicated and modified in order to exercise more 
			 *	control over input data and arguments.
			 *
			 *	Typical calling code would look something like:
			 *	Location locCode = batchMgr.GetWorkBuffer().AllocHigh(plugin_spu_bin_size, kWorkTypeSpuCode);
			 *	if (locCode == WorkBuffer::kLocationNull) { ICE_ASSERT(!"Out of work buffer space"); }
			 *	if (IsExecutingOnPpu()) {
			 *		void *pPpuCodeBuffer = BatchManager::CreateCodeBufferForPpu(plugin_spu_bin_size, 1, &plugin_spu_bin_entry, MyPpuFunction, pMyBuffer);
			 *		batchMgr.CreateUploadCommand(pPpuCodeBuffer, plugin_spu_bin_size, locCode);
			 *	} else
			 *		batchMgr.CreateUploadCommand(plugin_spu_bin, plugin_spu_bin_size, locCode);
			 *	Location locJointParams = batchMgr.GetWorkBuffer().FindLocationByType(kWorkTypeJointParams);
			 *	batchMgr.CreatePlugInCommand(locCode, plugin_spu_bin_entry, pInputData, inputDataSize, numData, locJointParams);
			 */
			void CreatePlugInCommand(Location locCode, U32F codeEntry, void const* pInputData, U32F inputDataSize, U16 arg1 = 0, U16 arg2 = 0, U16 arg3 = 0, U16 arg4 = 0, U16 arg5 = 0, U16 arg6 = 0, U16 arg7 = 0);

			/*!	Creates a command to execute the code in the binary buffer pCode starting
			 *	at entry point offset codeEntry, with the arguments (locInputData, arg1, arg2, ...).
			 *	The code will be executed directly in the input buffer and will not persist,
			 *	so codeSize + inputDataSize must be less than approximately 0x5F80.
			 *	Note that this is a helper function, and that you are welcome to treat it
			 *	as example code to be duplicated and modified in order to exercise more 
			 *	control over input data and arguments.
			 *	
			 *	Typical calling code would look something like:
			 *	Location locJointParams = batchMgr.GetWorkBuffer().FindLocationByType(kWorkTypeJointParams);
			 *	if (IsExecutingOnPpu()) {
			 *		void *pPpuCodeBuffer = BatchManager::CreateCodeBufferForPpu(plugin_spu_bin_size, 1, &plugin_spu_bin_entry, MyPpuFunction, pMyBuffer);
			 *		batchMgr.CreateInlinePlugInCommand(pPpuCodeBuffer, plugin_spu_bin_size, plugin_spu_bin_entry, pInputData, inputDataSize, numData, locJointParams);
			 *	} else
			 *		batchMgr.CreateInlinePlugInCommand(plugin_spu_bin, plugin_spu_bin_size, plugin_spu_bin_entry, pInputData, inputDataSize, numData, locJointParams);
			 */
			void CreateInlinePlugInCommand(void const* pCode, U32F codeSize, U32F codeEntry, void const* pInputData, U32F inputDataSize, U16 arg1 = 0, U16 arg2 = 0, U16 arg3 = 0, U16 arg4 = 0, U16 arg5 = 0, U16 arg6 = 0, U16 arg7 = 0);

#if ICEANIM_PERFORMANCE_STATS
			void DEBUG_ResetStats() {
				memset(&m_perfStats, 0, sizeof(m_perfStats));
#if BATCHJOB_DMA_STATS
				m_cmdStream.DEBUG_ResetStats();
#endif
			}
			PerfStats const& DEBUG_GetStats() const { return m_perfStats; }	//!< PPU timing stats collected by batch manager
			PerfStats& DEBUG_GetStats() { return m_perfStats; }				//!< PPU timing stats collected by batch manager
#endif

		protected:
			//! Create commands for an arbitrary animation node.
			//!     All nodes operate on data in place at blendLoc in the format:
			//!		if (m_numJoints>0) { JointParam joints[m_numJoints], JointParam safeJoint, ValidBits validBits }
			//!		if (m_numFloatChanels>0) { float floatChannels[(m_numFloatChanels+3)&~3], float safeFloatChannels[4], ValidBits validBits }
			//!	Returns false if work buffer space is exhausted
			bool CreateNodeCommands(AnimationNode const *pNode, BlendGroupDesc const& groupDesc, U32F instance, Location const *pChannelGroupLoc, ValidBits *pResultBits);

			//! Create commands from an animation clip node. 
			//!	Returns false if work buffer space is exhausted
			bool CreateClipCommands(ClipNode const *pNode, BlendGroupDesc const& groupDesc, U32F instance, Location const *pChannelGroupLoc, ValidBits *pResultBits);

			//! Create commands for a blend node.
			//!	Returns false if work buffer space is exhausted
			bool CreateBlendCommands(BlendNode const *pNode, BlendGroupDesc const& groupDesc, U32F instance, Location const *pChannelGroupLoc, ValidBits *pResultBits);

			//! Create commands for a flip node.
			//!	Returns false if work buffer space is exhausted
			bool CreateFlipCommands(FlipNode const *pNode, BlendGroupDesc const& groupDesc, U32F instance, Location const *pChannelGroupLoc, ValidBits *pResultBits);

			//! Create commands for a snap-shot node.
			//!	Returns false if work buffer space is exhausted
			bool CreateSnapshotCommands(SnapshotNode const *pNode, BlendGroupDesc const& groupDesc, U32F instance, Location const *pChannelGroupLoc, ValidBits *pResultBits);

			//! Create commands for a pose node.
			//!	Returns false if work buffer space is exhausted
			bool CreatePoseCommands(PoseNode const *pNode, BlendGroupDesc const& groupDesc, U32F instance, Location const *pChannelGroupLoc, ValidBits *pResultBits);

			//! Create commands for a unary plug-in node.
			//!	Returns false if work buffer space is exhausted
			bool CreateUnaryPlugInCommands(UnaryPlugInNode const *pNode, BlendGroupDesc const& groupDesc, U32F instance, Location const *pChannelGroupLoc, ValidBits *pResultBits);

			//! Create commands for a binary plug-in node.
			//!	Returns false if work buffer space is exhausted
			bool CreateBinaryPlugInCommands(BinaryPlugInNode const *pNode, BlendGroupDesc const& groupDesc, U32F instance, Location const *pChannelGroupLoc, ValidBits *pResultBits);

			//! Create commands for a blend group whose parameters (m_group, m_numJoints, m_numFloatChannels, m_firstChannelGroup, m_numChannelGroups, m_firstFloatChannel) 
			//! have already been set up to output and fill in undefined channels at blendLoc.
			//!	Returns false if work buffer space is exhausted
			bool CreateTaskCommandsForGroup(AnimationNode const *pRootNode, BlendGroupDesc const& groupDesc, JointParams const *pJointDefaults, float const *pFloatChannelDefaults);

			static const BatchJob::BatchJobBufferDesc kBufferDesc;
			BatchJob::CommandStream m_cmdStream;	//!< Command stream.
			BatchJob::WorkBuffer m_workBuffer;		//!< memory manager for SPU local store work buffer

			Location m_jointParamsLoc;		//!< Location of joint pose
			Location m_floatChannelLoc;		//!< Location of float channel pose
			Location m_sdkScalarTableLoc;	//!< Location of scalar table
			Location m_sdkVectorTableLoc;	//!< Location of SDK vector table
			Location m_jointTransformLoc;	//!< Location of joint transform table

#if ICEANIM_PERFORMANCE_STATS
			PerfStats m_perfStats;			//!< PPU timing stats collected by batch manager
#endif
		};
	}
}

#endif // ICE_JOINT_ALLOC_H
