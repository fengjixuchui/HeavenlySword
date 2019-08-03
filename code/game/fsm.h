
//------------------------------------------------------------------------------------------
//!
//!	\file fsm.h
//!
//------------------------------------------------------------------------------------------

#ifndef FSM_INC
#define FSM_INC

#include "message.h"

//----------------------------------------------------------------------------------------------
//!	FSM
//----------------------------------------------------------------------------------------------
namespace FSM
{
	// Forward Decls.
	class StateMachine;

	//------------------------------------------------------------------------------------------
	//!
	//!	State
	//!	A state in our state machine.
	//!
	//------------------------------------------------------------------------------------------
	class StateBase
	{
	public:
		virtual ~StateBase() {;}

		virtual bool Process(FSM::StateMachine&, const Message&) {return false;}
		virtual StateBase* Parent() = 0;
		
		virtual const char* GetName() = 0;
	};

	template<class PARENT>
	class State : public StateBase
	{
	public:
		virtual StateBase* Parent() {return PARENT::GetInstance();}
	};

	//------------------------------------------------------------------------------------------
	//!
	//!	StateMachine
	//!	
	//!
	//------------------------------------------------------------------------------------------
	class StateMachine
	{
	public:
		StateMachine() : m_pState(0), m_pBase(0) {;}
		virtual ~StateMachine() {;}

		void ProcessMessage(Message& msg)
		{
			msg.VerifyMessageID();
			FSM::StateBase* pState = m_pState;

			while(pState)
			{
				if(pState->Process(*this, msg))
					return;
				pState = pState->Parent();
			}

			Process(*this, msg);
		}

		void SetState(FSM::StateBase* pState) 
		{
			//ntPrintf("FSM::SetState - Prev %s - Next %s\n", m_pState->GetName(), pState->GetName());
			if(m_pState == pState)
				return;

			FSM::StateBase* pExiting = m_pState;
			while(pExiting)
			{
				if(pExiting == pState || (pState && pExiting == pState->Parent()))
					break;

				pExiting->Process(*this, Message(State_Exit)); 
				pExiting = pExiting->Parent();
			}
			m_pState = pState; 
			if(pState != pExiting && pState )
				pState->Process(*this, Message(State_Enter));
		}

		void Update() 
		{
			if(m_pState && m_pState->Process(*this, Message(State_Update,false)))
				return;
			
			Process(*this, Message(State_Update,false));
		}

		static FSM::StateBase* GetInstance() {return 0;}

		virtual bool Process(StateMachine&, const Message&) {return false;}
		virtual void DebugRender(float&, float&) {;}

		void SetBase(void* pBase) {m_pBase = pBase;}
		void* GetBase() {return m_pBase;}

		// Return the debug state name of the current state in the FSM
		const char* GetStateName() const { return m_pState ? m_pState->GetName() : "No State"; }

	protected:
		FSM::StateBase* m_pState;
		void*           m_pBase;
	};
};

#define STATEMACHINE(name, base)										\
class name : public FSM::StateMachine									\
{																		\
public:																	\
	typedef name _thisclass;											\
	typedef base _baseclass;											\
	static const char*  GetFSMName()  {return #name;}					\
	static const char*  GetBaseName() {return #base;}

#define STATE(name)														\
class name : public FSM::State<_thisclass>								\
{																		\
public:																	\
	static name* GetInstance(){static name ret; return &ret;}			\
	typedef name _thisclass;											\
	virtual const char*  GetName() {return #name;}

#define ME ((_baseclass*)FSM.GetBase())

#define BEGIN_EVENTS													\
	virtual bool Process(FSM::StateMachine& FSM, const Message& msg)	\
	{																	\
		/*POTENTIALLY*/UNUSED(FSM);										\
		switch(msg.GetID())												\
		{

#define EVENT(name)														\
		case name:

#define END_EVENT(x)													\
			return x;

#define EVENT_MAPPING(name, func)										\
		case name:														\
			return ME->func(msg);


#define ON_ENTER EVENT(State_Enter)
#define END_ON_ENTER return true;

#define ON_EXIT EVENT(State_Exit)
#define END_ON_EXIT return true;

#define ON_UPDATE EVENT(State_Update)
#define END_ON_UPDATE return true;

#define IGNORE_EVENT(name) case name: return true;

#define END_EVENTS														\
		default:														\
			return false;												\
		}																\
	}

#define END_EVENTS_DEBUG																													\
		default:																															\
		ntPrintf("%s(%d): Unhandled message: '%s' in state %s\n", __FILE__, __LINE__, msg.GetIDName(), FSM.GetStateName());	\
			return false;																													\
		}																																	\
	}



#define END_STATE														\
};

#define END_STATEMACHINE												\
};

#define SET_STATE(x) FSM.SetState(x::GetInstance())
#define POP_STATE()  FSM.SetState(this->Parent())
#define SET_INITIAL_STATE(x) m_pState = x::GetInstance()
#define EXTERNALLY_SET_STATE(fsmname, state) m_pFSM->SetState(fsmname::state::GetInstance());

#define DETACH_FSM()													\
{																		\
	m_pFSM->SetState( 0 );												\
	delete m_pFSM;														\
	m_pFSM = 0;															\
}

#define ATTACH_FSM(x)													\
{																		\
	m_pFSM = x;															\
	m_pFSM->SetBase(this);												\
	Message msg(State_Enter);											\
	m_pFSM->ProcessMessage(msg);										\
}

#endif // FSM_INC
