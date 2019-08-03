//--------------------------------------------------------------------------------------------------
/**
	@file		SkTaskList.h

	@brief		The Task List is an array of tasks.  The Shared Area is set to point to a
				Task List so that SPUs can work through the tasks.

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef MASTER_TASK_LIST_H
#define MASTER_TASK_LIST_H

//--------------------------------------------------------------------------------------------------
//  INCLUDES
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
//  MACRO DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
//  EXTERNAL DECLARATIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
//  CLASS DEFINITIONS
//--------------------------------------------------------------------------------------------------
union KernelCommand;
struct SkTaskSetupObject;
class SkCommandList;
class SkDependency;

//--------------------------------------------------------------------------------------------------
/**
	@class			SkTaskList

	@brief			The Task List is an array of tasks.  The Shared Area is set to point to
					a Task List so that SPUs can work through the tasks.
**/
//--------------------------------------------------------------------------------------------------

class SkTaskList
{
public:
	enum TaskListCheckerPrintOption
	{
		kDontPrintCommands	= 0,
		kPrintCommands		= 1,
	};

						SkTaskList( u32 maxTasks, u32 numSpusRequired = 1 );
						~SkTaskList();

	void				AddTask( const SkCommandList& rCommandList );

	void				AddMultipleSpuTask( const SkCommandList& rCommandList0, const SkCommandList& rCommandList1, const SkDependency& dependency );
	void				AddMultipleSpuTask( const SkCommandList& rCommandList0, const SkCommandList& rCommandList1, const SkCommandList& rCommandList2, const SkDependency& dependency );
	void				AddMultipleSpuTask( const SkCommandList& rCommandList0, const SkCommandList& rCommandList1, const SkCommandList& rCommandList2, const SkCommandList& rCommandList3, const SkDependency& dependency );
	void				AddMultipleSpuTask( const SkCommandList& rCommandList0, const SkCommandList& rCommandList1, const SkCommandList& rCommandList2, const SkCommandList& rCommandList3, const SkCommandList& rCommandList4, const SkDependency& dependency );
	void				AddMultipleSpuTask( const SkCommandList& rCommandList0, const SkCommandList& rCommandList1, const SkCommandList& rCommandList2, const SkCommandList& rCommandList3, const SkCommandList& rCommandList4, const SkCommandList& rCommandList5, const SkDependency& dependency );
	void				AddMultipleSpuTask( const SkCommandList& rCommandList0, const SkCommandList& rCommandList1, const SkCommandList& rCommandList2, const SkCommandList& rCommandList3, const SkCommandList& rCommandList4, const SkCommandList& rCommandList5, const SkCommandList& rCommandList6, const SkDependency& dependency );
	void				AddMultipleSpuTask( const SkCommandList& rCommandList0, const SkCommandList& rCommandList1, const SkCommandList& rCommandList2, const SkCommandList& rCommandList3, const SkCommandList& rCommandList4, const SkCommandList& rCommandList5, const SkCommandList& rCommandList6, const SkCommandList& rCommandList7, const SkDependency& dependency );

	SkTaskSetupObject*	CloseTaskList( void );

	void				ResetTaskList( void );

	void				ErrorCheckTaskList( TaskListCheckerPrintOption printCommands ) const;

	u32					GetNumTasks( void ) const			{ return m_currentEntry; }

	u32					GetNumSpusRequired( void ) const	{ return m_minSpusRequired; }

private:
	void				AddTaskWithDependencyStall( const SkCommandList& rCommandList, const SkDependency& dependency );
	void				AddTaskWithDependencyDecrement( const SkCommandList& rCommandList, const SkDependency& dependency );

	u32					m_minSpusRequired;	//In order to process this task list this is the minimum number of SPUs required.  More that this will help speed up processing.
	u32					m_maxTasks;
	u32					m_currentEntry;
	SkTaskSetupObject*	m_pTaskSetupObjectStack;
};

//--------------------------------------------------------------------------------------------------
//  INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------------

#endif // MASTER_TASK_LIST_H
