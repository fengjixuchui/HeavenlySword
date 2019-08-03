/***************************************************************************************************
*
*	DESCRIPTION		Header file defining basic task functionality.
*
*	NOTES
*
***************************************************************************************************/

#ifndef _AI_TASK_H
#define _AI_TASK_H

// Forward declarations
class CAIComponent;

/***************************************************************************************************
*
*	CLASS			CAITask
*
*	DESCRIPTION		
*
***************************************************************************************************/

class CAITask
{
	// I don't like friends much, but sometimes they do help :)
	friend class CAITaskMan;
	friend class CAITaskContainer;

public:

	enum eAI_TASK_STATE
	{
		AI_TASK_INVALID,
		AI_TASK_ACTIVATE,
		AI_TASK_ACTIVE,
		AI_TASK_SLEEPING,
		AI_TASK_COMPLETED,

		AI_TASK_STATE_MAX
	};

	// Default constructor
	CAITask(void) :
		m_eState(AI_TASK_INVALID)
	{
	}

	// Default destructor
	virtual ~CAITask(void) {}

// methods...

	// When the Task is added to the list, the following mehtod will be called
	virtual void OnListAdd(void) {}

	// When the Task is removed from the list, the following method will be called
	virtual void OnListRemove(void) {}

	// The update called only once before the furst update. 
	virtual void OnFirstUpdate(CAIComponent*) {}

	// The main update for the Task
	virtual void Update(float fTimeChange, CAIComponent* ) = 0;

	// Return the current state for the Task
	eAI_TASK_STATE GetState(void) const { return m_eState; } 

	// Set the tasks name
	void SetName(const char* pcName) { m_TaskName = pcName; }

	// Obtain the set name.
	const char* GetName(void) const { return m_TaskName.c_str(); }

protected:

	// Set the current state for the Task
	void SetState(eAI_TASK_STATE eNewState) { m_eState = eNewState; } 

private:

	// The name of the current task.
	ntstd::String m_TaskName;

	// The current state of the task
	eAI_TASK_STATE	m_eState;
};


#endif // _AI_TASK_H
