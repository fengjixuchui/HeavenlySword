/*
 * Copyright (c) 2003-2005 Naughty Dog, Inc.
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#ifndef ICE_ICEANIMTASKMAN_H
#define ICE_ICEANIMTASKMAN_H

#include "icebatchjoblist.h"

#if ICE_TARGET_PS3_PPU
struct CellSpurs;
class AuditManager;
#endif

namespace Ice
{
    namespace Anim
    {
		struct AnimationTask {
			U32 const* 						pInitialDmaList;
			U32 							initialDmaListSize;
			mutable Ice::BatchJob::JobId	jobId;
		};

		/*!
		 * Initialize task manager data for SPU animation.
		 */
#if ICE_TARGET_PS3_PPU
		void InitSpuAnimation(CellSpurs *pSpurs, SpuModuleHandle const& iceanimModule, U32 numSpus, U32 firstSpu = 0, AuditManager* pAuditManager = NULL);
#else
		void InitSpuAnimation();
#endif
		/*!
		 * Clean up task manager data for SPU animation.
		 */
		void TerminateSpuAnimation();

		/*!
		 * Begin a block of animation tasks for the SPU.
		 *
		 * Only one block of animation tasks need be defined per frame, but
		 * multiple blocks may be defined for convenience.
		 *
		 * Blocks may not overlap; that is, you may not begin a new block
		 * until you have ended the previous block with EndSpuAnimationTasks
		 * and verified that it has completed with
		 * IsSpuAnimationTasksComplete().
		 *
		 * Blocks are not limitted in size, but there is a maximum number
		 * that may be pending for execution at once and there is a maximum
		 * total size of initialization data that can be queued - should you
		 * manage to submit tasks fast enough that there is no room for another
		 * to be added, AddSpuAnimationTask() may block until room becomes
		 * available.
		 *
		 * To avoid blocking, you should check the return value of
		 * IsRoomForSpuAnimationTask() before calling AddSpuAnimationTask().
		 */
		void BeginSpuAnimationTasks();
		/*!
		 * End a block of animation tasks for the SPU.
		 */
		void EndSpuAnimationTasks();

		/*!
		 * Updates the double buffered task lists, by kicking the next list
		 * whenever the current running list completes.
		 * Note that all functions below call PollSpuAnimationTasks(), and
		 * so calling it is generally unnecessary.  It may however be useful
		 * to call this directly if you plan on running unrelated code for
		 * a significant period of time in the middle of defining a task
		 * block.
		 */
		void PollSpuAnimationTasks();

		/*!
		 * Wait for all tasks in the current _ended_ block of SPU animation tasks to complete.
		 * WaitForSpuAnimationTasks() can not be called before EndSpuAnimationTasks()
		 * has been called, as there is no defined end to the current block until this time.
		 */
		void WaitForSpuAnimationTasks();
		/*!
		 * Check if all tasks in the current _ended_ block of SPU animation tasks have completed.
		 * Even if all tasks that have so far been submitted have completed,
		 * IsSpuAnimationTasksComplete() always returns false until EndSpuAnimationTasks()
		 * has been called, as there is no defined end to the current block until this time.
		 */
		bool IsSpuAnimationTasksComplete();

		/*!
		 * Wait for a particular animation task in the current block to complete.
		 * This may be called before or after EndSpuAnimationTasks(), as it does
		 * not require the current block to be ended.
		 */
		void WaitForSpuAnimationTask(AnimationTask const *pTask);
		/*!
		 * Check if a particular animation task in the current block has completed.
		 * This may be called before or after EndSpuAnimationTasks(), as it does
		 * not require the current block to be ended.
		 */
		bool IsSpuAnimationTaskComplete(AnimationTask const *pTask);

		/*!
		 * Returns the current state of the animation task system.
		 */
		enum AnimTaskState {
			kAnimTasksUninitialized = BatchJob::kJobBlockUninitialized,	//!< InitSpuAnimation has not been called, or TerminateSpuAnimation has been called
			kAnimTasksComplete =BatchJob::kJobBlockCompleted,	//!< No tasks are currently running and no block has been started
			kAnimTasksQueuing = BatchJob::kJobBlockQueuing,		//!< between BeginSpuAnimationTasks and EndSpuAnimationTasks; tasks may be running, but no end has been defined
			kAnimTasksRunning = BatchJob::kJobBlockRunning,		//!< EndSpuAnimationTasks has been called but tasks are still running
			kAnimTasksFull = 	BatchJob::kJobBlockFull,		//!< No room for any animation tasks to queue right now
		};
		AnimTaskState GetSpuAnimationTaskState();

		/*!
		 * Check if the given task can now be added with AddSpuAnimationTask()
		 * Returns false if there is no task slot available to add the task or
		 * if there isn't enough buffer space to hold the task.
		 */
		bool IsRoomForSpuAnimationTask(AnimationTask const *pTask);

		/*!
		 * These functions are generally for use by AnimateJointsBatched(), which generates
		 * data in the internal format required by the animation task:
		 *
		 * Add an animation task to the current block of SPU animation tasks, starting
		 * with the given task data as startup data.
		 *
		 * Note that this operation may block until IsRoomForSpuAnimationTask()
		 * returns true, if there is not currently room to add the task.
		 */
		void AddSpuAnimationTask(AnimationTask const *pTask);
		/*!
		 * Calling RunAnimationTaskOnPpu simulates an SPU animation task on the PPU
		 * using the same startup data as AddAnimationTaskOnSpu().
		 */
		void RunAnimationTaskOnPpu(AnimationTask const *pTask);
	}
};

#endif //ICE_ICEANIMTASKMAN_H
