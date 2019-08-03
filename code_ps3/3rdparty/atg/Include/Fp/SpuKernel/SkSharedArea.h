//--------------------------------------------------------------------------------------------------
/**
	@file		SkSharedArea.h

	@brief		This class holds the Shared Area memory that will be accessed atomically and shared
				by multiple SPUs

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef SK_SHARED_AREA_H
#define SK_SHARED_AREA_H

//--------------------------------------------------------------------------------------------------
//  INCLUDES
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
//  MACRO DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
//  DECLARATIONS
//--------------------------------------------------------------------------------------------------

class SkTaskList;
class SkDependency;

//--------------------------------------------------------------------------------------------------
//  CLASS DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@class			SkSharedArea

	@brief			Holds the Shared Area that will be accessed atomically and shared by multiple
					SPUs

	@note			The Shared Area is setup to point to a Task List and tracks how far
					through the Task List the SPUs have reached

	@note			The Shared Area also contains the Dependency array.  The PU can interact with
					Dependencies too via this class
**/
//--------------------------------------------------------------------------------------------------

class SkSharedArea
{
public:
	enum SpuKernelExitMode
	{
		kPollForMoreTasks	= 0,
		kExitSpus			= 1,
	};
						SkSharedArea( void* pMem, u32 memSize, u32 numSpusRequired = 1 );
						~SkSharedArea();

	void				SetTaskLists( SkTaskList& standardTaskList, SkTaskList& priorityTaskList );
	void				UpdateTaskListSizes( SkTaskList& standardTaskList, SkTaskList& priorityTaskList );

	void				SetSpuCompletionMode( enum SpuKernelExitMode );
	void*				GetSharedAreaMemPtr( void )								{ return m_pSharedAreaMem; }

	u32					GetDependencyValue( const SkDependency& dependency );
	void				DecrementDependencyValue( const SkDependency& dependency );

	u32					GetNumSpusRequired( void ) const						{ return m_minSpusRequired; }

	SkDependency		AllocDependencyCounter( u32 initialValue );

	u16					AllocUniqueId( void );

	void				ClearDependencyAllocations( void );
	void				SnapshotDependencyState( void* pMem, u32 memSize ) const;
	void				ResetToDependencyStateSnapshot( const void* pMem, u32 memSize );

private:
	u32					m_minSpusRequired;	//This SharedArea is pointing at task lists that need at least this many SPUs in order to safely be processed

	void*				m_pSharedAreaMem;
	u32					m_numAtomicLines;

	struct
	{
		u32	m_currCacheLine;
		u32	m_currDepSlot;
		u32	m_currLowerBitOffset;
	}					m_depAlloc;

	u16					m_currUniqueId;

	void				AssignDependencyInitialValue( const SkDependency& dependency, u32 initialValue );
};

//--------------------------------------------------------------------------------------------------
//  INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------------

#endif // SK_SHARED_AREA_H
