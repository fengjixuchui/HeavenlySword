//------------------------------------------------------------------------------------------
//!
//!	\file messagehandler.h
//!
//------------------------------------------------------------------------------------------

#ifndef _MESSAGEHANDLER_H
#define _MESSAGEHANDLER_H

// Necessary includes
#include "game/luaglobal.h"
#include "game/message.h"

// Forward declarations
class CEntity;
class CMessageHandler;


//------------------------------------------------------------------------------------------
//!
//!	GameMessage
//!	A class describing a message to be sent on how to send it.  I realise that this is not
//!	the most efficient implementation of this system - but at this stage i want to get the
//!	functionality clear and the implementation robust.
//!
//------------------------------------------------------------------------------------------
class GameMessage
{
public:

	HAS_INTERFACE( GameMessage );

	// Construction destruction
	GameMessage( void );
	~GameMessage( void );

	// Check data after construction
	void PostConstruct( void );

	// If the data for this message is realtime edited then we reset all the dynamic stuff
	bool EditorChangeValue( CallBackParameter pcItem, CallBackParameter pcValue );

	// We have time delays on the messages so we need an update
	void Update( float fTimeStep );

	// Request that this message be sent, it may not be sent immediately or at all depending on parameters
	bool SendRequest( const CEntity* pobDefaultReceiver, const CEntity* pobSender = 0 );

	// Will this message be sent again
	bool WillSend( void ) const;

	// Set the message back to the serialised state
	void Reset( void );
	
protected:
	
	// What is the message?
	CHashedString m_obMessage;

	// Do we have any data associated with it?
	CHashedString m_obData;

        const CEntity* m_pobSender;

	// How many send requests should we get before we actually send?
	int m_iRequestCount;

	// How many times can this message be sent?
	int m_iMaxSends;

	// How long should we wait after a send request to actually send?
	float m_fMessageDelay;

	// Who do we get sent to?
	ntstd::List<void*> m_obReceivers;

private:

	// Helper functions
	void SendMessage( void );

	// How many times have i been sent
	int m_iSendCount;

	// How many times has it been requested that i be sent
	int m_iRequestSendCount;

	// Do we have any time delayed send requests?
	ntstd::List<float> m_obDelayedSends;

};


//------------------------------------------------------------------------------------------
//!
//!	GameEvent
//!	A named game event associated with a set of messages to send when it occurs
//!
//------------------------------------------------------------------------------------------
class GameEvent
{
public:

	HAS_INTERFACE( GameEvent );

	// Construction destruction
	GameEvent( void );
	~GameEvent( void );

	// Check data after construction
	void PostConstruct( void );

	// We need an update here so we can deal with time delayed messages
	void Update( float fTimeStep );

	// Is this event identified by the given name?
	bool IsEvent( CHashedString pcEventName );

	// Fire this event - split from IsEvent so we can force events - returns true if a message was sent
	bool FireEvent( const CEntity* pobDefaultReceiver, const CEntity* pobSender = 0 );

	// Will this event result in any messages
	bool WillFireMessages( void ) const;

	// Reset our messages to their serialise state
	void Reset( void );

#ifndef _RELEASE
	// Debug only access to our name
	const CHashedString& GetName( void ) { return m_obEventName; }
#endif

protected:

	// The name of this event
	CHashedString m_obEventName;

	// The messages that need to be sent on this event
	ntstd::List<GameMessage*> m_obGameMessages;

};


//------------------------------------------------------------------------------------------
//!
//!	GameEventList
//!	A List of Game Events to be processed
//!
//------------------------------------------------------------------------------------------
class GameEventList
{
public:

	HAS_INTERFACE( GameEventList );

	// Construction destruction
	GameEventList( void );
	~GameEventList( void );

	// Check data after construction
	void PostConstruct( void );

	// We need an update here so we can deal with time delayed messages
	void Update( float fTimeStep );

	// Signal that an event has taken place - returns true if a message was sent
	bool ProcessEvent( const char* pcEventName, const CEntity* pobDefaultReceiver, const CEntity* pobSender = 0 );
	bool WillHandleEvent( const char* pcEventName );

	// Will firing anything on this event list result in any messages
	bool WillFireMessages( void ) const;

	// Reset our events to their serialise state
	void Reset( void );
	
	// Return the number of game events waiting. 
	int Size() const { return m_obGameEvents.size(); }
	
protected:

	// Our list of events
	ntstd::List<GameEvent*> m_obGameEvents;
};


//------------------------------------------------------------------------------------------
//!
//!	CMessageSender
//!	A helper class to construct and send out messages from the game engine to the scripting 
//! environment.  This also provides a little bit of insulation between engine and scripting
//! ie a user of this class doesn't have to know anything about the lua environment.
//!
//------------------------------------------------------------------------------------------
class CMessageSender
{
public:
	static void SendEmptyMessage( CHashedString pcMessageType,	const CMessageHandler* pobDestination );
	static void SendEmptyMessage( MessageID eMessageID,			const CMessageHandler* pobDestination );
};


//------------------------------------------------------------------------------------------
//!
//!	CMessageHandler
//!	This class should eventually completely replace CMessageStore and CCallbackComponent.
//! Placing a message with the message handler is a const operation - since this should be 
//! the only real point of object interaction.  An Entity holding a const pointer to another
//! should definately be able to send it a message.
//!
//!	We currently have conflicting scripted state - the one contained here and the new one
//!	contained in the AI system.  In the future i will be taking the more advanced 
//!	functionality of the AI system and merging into this one - GH.
//!
//------------------------------------------------------------------------------------------
class CMessageHandler
{
	// Structure defining a delayed message
	struct Delayed
	{	
		// The ammount of time to delay the message.
		float m_Delay;
		// The message to send
		Message	m_Msg;

		// Constructor for the object
		Delayed( float fDelay, const Message& Msg ) :  m_Delay( fDelay ),  m_Msg( Msg ) { }
	};

	// A class to hold a message handler
	class Handler
	{
	public:
		Handler() {}
		Handler( const NinjaLua::LuaObject& obC, const NinjaLua::LuaFunction& obH ) :
			m_obContext( obC ),
			m_obHandler( obH )
			{}

		// Call the message handler
		void Call(Message&);

		NinjaLua::LuaObject		m_obContext;
		NinjaLua::LuaFunction	m_obHandler;
	};

public:

	// Construction
	CMessageHandler( CEntity* pobHostEnt, GameEventList* pobEventList = 0 );
	~CMessageHandler( void );

	HAS_LUA_INTERFACE()

	// add a message to the pending message queue
	void QueueMessage(const Message& obMsg ) const;
	void QueueMessageDelayed(const Message& obMsg, float fDelay ) const;
	void Receive(const Message& obMsg, float fDelay = 0.0f ) const;

	template<MessageID I>
	void ReceiveMsg() const;

	// For the lua interface - should probably merge queue and queue delayed in here
	int	 Post(NinjaLua::LuaState&);
	int	 PostDelayed(NinjaLua::LuaState&);

	// Clear the message queues
	void ClearMessageQueues();

	// Helper methods for sending messages
	static inline Message Make( const CEntity* pSender, CHashedString pcMessage );
	template<typename P1> static inline Message Make( const CEntity* pSender, CHashedString pcMessage, const P1& );
	template<typename P1, typename P2> static inline Message Make( const CEntity* pSender, CHashedString pcMessage, const P1&, const P2& );
	template<typename P1, typename P2, typename P3> static inline Message Make( const CEntity* pSender, CHashedString pcMessage, const P1&, const P2&, const P3& );

	// Process an event if we can - the default receiver will be used if the message has no receivers set up
	bool ProcessEventWithDefaultReceiver( const char* pcEventName, const CEntity* pobDefaultReceiver, const CEntity* pobSender = 0);
	bool ProcessEvent( const char* pcEventName ) { return ProcessEventWithDefaultReceiver( pcEventName, 0, m_pobHost ); }
	bool WillHandleEvent( const char* pcEventName );

	// Check whether this handler will fire a message from any of its events
	bool ProcessingEventWillFireMessages( void ) const { return ( ( m_pobGameEventList ) && ( m_pobGameEventList->WillFireMessages() ) ); }

	// Set the event processor back to its serialised state
	void ResetEventProcessor( void ) { if ( m_pobGameEventList ) m_pobGameEventList->Reset(); }

	// process all pending messages.
	void Update(float fTimeDelta);

	void SetCallback( const char* obMsg, NinjaLua::LuaObject& obHandler );
	void ClearCallbacks();

	bool GetAcceptingKeyboardEvents()       {return m_KeyboardEvents;}
	void SetAcceptingKeyboardEvents(bool b) {m_KeyboardEvents = b;}

	const char* GetCurrentState ();

	// Set the lua message handler
	void SetStateMachine( NinjaLua::LuaObject );

	// Add a context and handler 
	void AddHandler( NinjaLua::LuaObject obContext, NinjaLua::LuaFunction obFunction );

	// Remove a handler by a given context
	void RemoveHandler( NinjaLua::LuaObject obContext );

	// Is the message handler currently processing its message queue
	bool Processing(void) const { return m_bProcessing; }

private:

	// Helper for posting functions
	int	PostCommon(NinjaLua::LuaState&, float fDelay);

	// Our hoting parent entity
	CEntity* m_pobHost;

	// Object pointing to the Stack stack
	NinjaLua::LuaObject		m_obStateStack;

#if (!defined _RELEASE) && (!defined _MASTER)
	// Debug func to test if a message is being sent to a paused entity
	void TestPaused(const Message& obMessage) const;
#endif

	// Received messages are plonked here, ready for processing by Update() - this is mutable because
	// entities need to have const access to each other.  This list should be the only ability that
	// an external entity has to effect the potential state of the owner of the message handler
	int								m_ActiveQueue;
	mutable ntstd::List< Message >	m_obMessageQueue[2];
	mutable ntstd::List< Delayed > m_obDelayedQueue;

	// Handlers list
	mutable ntstd::List< Handler > m_obMsgHandlers;

	// A pointer to any events that we may need to process messages for
	GameEventList* m_pobGameEventList;

	// Process keyboard events bool. Debug code. 
	bool m_KeyboardEvents;

	bool m_bProcessing;


	// Debug Code.

#ifndef _RELEASE

	// All this is written with simple arrays rather than anything fancy
	// which should make debugging easier - one hopes
	class MessageHandlerDebug
	{
	public:

		// Construction
		MessageHandlerDebug( void )
		:	m_pcEntityName( 0 )
		{
			// Clear our array of pointers to debug information
			for ( int i = 0; i < NUM_REMBEMERED_STATES; ++i )
				CallbackItems[i] = 0;
		}

		// Clear up
		~MessageHandlerDebug( void )
		{
			for ( int i = 0; i < NUM_REMBEMERED_STATES; ++i )
				NT_DELETE( CallbackItems[i] );

			NT_DELETE( m_pcEntityName );
		}

		// Some helper stuff
		void CopyNameData( const char* pcName, const char*& pcDestination );
		void SetEntityName( const char* pcEntityName );
		void AddCallbackInformation( CHashedString pcMessageName, const char* pcStateName );
		bool DebugPrintStateAndMessageData( int iState, float fX, float fY, uint32_t dwColour ) const;

		// Another sub class to hold the debug strings
		class CallbackDebug
		{
		public:
			// Construction
			CallbackDebug( void ) 
			:	pcMessageName( 0 ), 
				pcStateName( 0 ) 
			{}

			// Clearup
			~CallbackDebug( void )
			{
				NT_DELETE( pcMessageName );
				NT_DELETE( pcStateName );
			}

			const char* pcMessageName;
			const char* pcStateName;
		};

		const char* m_pcEntityName;

		static const int NUM_REMBEMERED_STATES = 5;

		const CallbackDebug* CallbackItems[NUM_REMBEMERED_STATES];
	};


	// We want one of these too for debug purposes
	MessageHandlerDebug m_obDebugInfo;

	struct MESSAGE_LOG
	{ 
		u_int m_TimeStamp;
		u_int m_CombatState;
		ntstd::String m_StatePreMsg;
		ntstd::String m_StatePostMsg;
		ntstd::String m_Msg;
	};
	ntstd::Vector< MESSAGE_LOG >	m_obMsgLog;
public:

	// Access to debug information - we hold a fixed number of past states - the pointers
	// to the strings will not be valid when they fall off the history stack - and the 
	int		GetSizeOfDebugStateHistory( void ) const { return m_obDebugInfo.NUM_REMBEMERED_STATES; }
	bool	DebugPrintStateAndMessageData( int iState, float fX, float fY, uint32_t dwColour ) const { return m_obDebugInfo.DebugPrintStateAndMessageData( iState, fX, fY, dwColour ); }

	void DumpSuperLog(void) const;
#endif // _RELEASE
};

LV_DECLARE_USERDATA(CMessageHandler);




//------------------------------------------------------------------------------------------
//!	
//!
//------------------------------------------------------------------------------------------
Message CMessageHandler::Make( const CEntity* pSender, CHashedString pcMessage ) 
{ 
	Message obMsg(eUndefinedMsgID);
	obMsg.SetString( CHashedString(HASH_STRING_MSG), pcMessage );	// Assign the subject to the message
	obMsg.SetEnt( CHashedString(HASH_STRING_SENDER), pSender );		// Assign the sender
	return obMsg;  
}

//------------------------------------------------------------------------------------------
//!	
//!
//------------------------------------------------------------------------------------------
template<typename P1> inline 
Message CMessageHandler::Make( const CEntity* pSender, CHashedString pcMessage, const P1& p1 )
{
	Message obMsg(eUndefinedMsgID);
	obMsg.SetString( CHashedString(HASH_STRING_MSG), pcMessage );		// Assign the subject to the message
	obMsg.SetEnt( CHashedString(HASH_STRING_SENDER), pSender );		// Assign the sender
	obMsg.SetUnnamedParams();			// As there are unnamed params, mark the message
	obMsg.AddParam( p1 );
	return obMsg;
}

//------------------------------------------------------------------------------------------
//!	
//!
//------------------------------------------------------------------------------------------
template<typename P1, typename P2> inline 
Message CMessageHandler::Make( const CEntity* pSender, CHashedString pcMessage, const P1& p1, const P2& p2 )
{
	Message obMsg(eUndefinedMsgID);						// Create a new message
	obMsg.SetString( CHashedString(HASH_STRING_MSG), pcMessage );		// Assign the subject to the message
	obMsg.SetEnt( CHashedString(HASH_STRING_SENDER), pSender );		// Assign the sender
	obMsg.SetUnnamedParams();			// As there are unnamed params, mark the message
	obMsg.AddParam( p1 );
	obMsg.AddParam( p2 );
	return obMsg;
}

//------------------------------------------------------------------------------------------
//!	
//!
//------------------------------------------------------------------------------------------
template<typename P1, typename P2, typename P3> inline 
Message CMessageHandler::Make( const CEntity* pSender, CHashedString pcMessage, const P1& p1, const P2& p2, const P3& p3 )
{
	Message obMsg(eUndefinedMsgID);
	obMsg.SetString( CHashedString(HASH_STRING_MSG), pcMessage );		// Assign the subject to the message
	obMsg.SetEnt( CHashedString(HASH_STRING_SENDER), pSender );		// Assign the sender
	obMsg.SetUnnamedParams();			// As there are unnamed params, mark the message
	obMsg.AddParam( p1 );
	obMsg.AddParam( p2 );
	obMsg.AddParam( p3 );
	return obMsg;
}


// Setup a macro to define the behavior for a known message type
# if (defined _RELEASE) || (defined _MASTER)
#define TEST_PAUSED(p)
#else
#define TEST_PAUSED(p) TestPaused(p);
#endif

#define DECLARE_GLOBAL_MESSAGE(m)		\
	Message	m_##m;

#define INITIALISE_GLOBAL_MESSAGE(m)	\
	, m_##m(m)

#define DEFINE_GLOBAL_MESSAGE(m)		\
template<>	\
inline void CMessageHandler::ReceiveMsg<m> () const				\
{															\
	ntError_p(MessageGlobals::Exists(), ("No MessageGlobals object is instantiated\n"));		\
	Message& obMessage = MessageGlobals::Get().m_##m;								\
	TEST_PAUSED(obMessage);									\
	m_obMessageQueue[m_ActiveQueue].push_back(obMessage);	\
}


//------------------------------------------------------------------------------------------
//!
//!	MessageGlobals
//!	A singleton to wrap a number of globally accessible Messages
//!
//------------------------------------------------------------------------------------------
class MessageGlobals : public Singleton<MessageGlobals>
{
public:
friend class CMessageHandler;	// To allow access to each of the global messages
	MessageGlobals();
	~MessageGlobals();

private:
	int	m_iDummy;	// This is only here to place first in the constructor list... don't remove!

	// Declare a message for each globally accessible message
	DECLARE_GLOBAL_MESSAGE(msg_combat_killed)
	DECLARE_GLOBAL_MESSAGE(msg_combat_struck)
	DECLARE_GLOBAL_MESSAGE(msg_animdone)
	DECLARE_GLOBAL_MESSAGE(msg_ai_attack)
	DECLARE_GLOBAL_MESSAGE(msg_button_power)
	DECLARE_GLOBAL_MESSAGE(msg_button_range)
	DECLARE_GLOBAL_MESSAGE(msg_release_power)
	DECLARE_GLOBAL_MESSAGE(msg_release_range)
	DECLARE_GLOBAL_MESSAGE(msg_button_special) 
	DECLARE_GLOBAL_MESSAGE(msg_buttonattack)
	DECLARE_GLOBAL_MESSAGE(msg_release_attack)
	DECLARE_GLOBAL_MESSAGE(msg_buttonaction)
	DECLARE_GLOBAL_MESSAGE(msg_release_action)
	DECLARE_GLOBAL_MESSAGE(msg_buttongrab)
	DECLARE_GLOBAL_MESSAGE(msg_release_grab)
	DECLARE_GLOBAL_MESSAGE(msg_buttondodge)
};

DEFINE_GLOBAL_MESSAGE(msg_combat_killed)
DEFINE_GLOBAL_MESSAGE(msg_combat_struck)
DEFINE_GLOBAL_MESSAGE(msg_animdone)
DEFINE_GLOBAL_MESSAGE(msg_ai_attack)
DEFINE_GLOBAL_MESSAGE(msg_button_power)
DEFINE_GLOBAL_MESSAGE(msg_button_range)
DEFINE_GLOBAL_MESSAGE(msg_release_power)
DEFINE_GLOBAL_MESSAGE(msg_release_range)
DEFINE_GLOBAL_MESSAGE(msg_button_special) 
DEFINE_GLOBAL_MESSAGE(msg_buttonattack)
DEFINE_GLOBAL_MESSAGE(msg_release_attack)
DEFINE_GLOBAL_MESSAGE(msg_buttonaction)
DEFINE_GLOBAL_MESSAGE(msg_release_action)
DEFINE_GLOBAL_MESSAGE(msg_buttongrab)
DEFINE_GLOBAL_MESSAGE(msg_release_grab)
DEFINE_GLOBAL_MESSAGE(msg_buttondodge)


// This is the default implementation 
template<MessageID I>
inline void CMessageHandler::ReceiveMsg() const
{
	Message obMessage(I);
	Receive(obMessage);
}

#endif
 // _MESSAGEHANDLER_H









