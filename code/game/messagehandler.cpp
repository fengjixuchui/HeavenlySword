//------------------------------------------------------------------------------------------
//!
//!	\file simpletransition.cpp
//!
//------------------------------------------------------------------------------------------

#include "game/messagehandler.h"
#include "game/attacks.h"

// Necessary includes
#include "game/entity.h"
#include "game/entity.inl"

#include "game/luaattrtable.h"
#include "game/luaexptypes.h"
#include "lua/ninjalua.h"

#include "game/fsm.h"

// These need to be included because we are not using the generic 
// meesaging system for cameras and triggers - TEMP - GH
#include "physics/triggervolume.h"
#include "camera/basiccamera.h"

// Debug includes
#include "core/visualdebugger.h"
#include "objectdatabase/dataobject.h"

#ifdef PLATFORM_PC
#include "input/inputhardware.h"
#endif


/***************************************************************************************************
* Interface for the Game Event/Message defintions
***************************************************************************************************/

START_STD_INTERFACE( GameMessage )
	PUBLISH_VAR_AS( m_obMessage, Message )
	PUBLISH_VAR_AS( m_obData, Data )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_iRequestCount, 1, RequestCount )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_iMaxSends, 1, MaxSends )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fMessageDelay, 0.0f, MessageDelay )
	PUBLISH_PTR_CONTAINER_AS( m_obReceivers, Receivers )

	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
	DECLARE_EDITORCHANGEVALUE_CALLBACK( EditorChangeValue )
END_STD_INTERFACE

START_STD_INTERFACE( GameEvent )
	PUBLISH_VAR_AS( m_obEventName, Event )
	PUBLISH_PTR_CONTAINER_AS( m_obGameMessages, Messages )

	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
END_STD_INTERFACE

START_STD_INTERFACE( GameEventList )
	PUBLISH_PTR_CONTAINER_AS( m_obGameEvents, Events )

	DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
END_STD_INTERFACE



/***************************************************************************************************
* Exposing the elements to Lua
***************************************************************************************************/

LUA_EXPOSED_START(CMessageHandler)

	LUA_EXPOSED_METHOD_RAW(Post,		Post,        "", "", "")
	LUA_EXPOSED_METHOD_RAW(PostDelayed,	PostDelayed, "", "", "")

	LUA_EXPOSED_METHOD(AddHandler,		AddHandler,      "", "", "") 
	LUA_EXPOSED_METHOD(SetStateMachine,	SetStateMachine, "", "", "") 
	LUA_EXPOSED_METHOD(ClearCallbacks,	ClearCallbacks,  "", "", "")
	LUA_EXPOSED_METHOD(ProcessEvent,	ProcessEvent,    "", "", "")

	LUA_EXPOSED_METHOD_GET(KeyboardEvents,	GetAcceptingKeyboardEvents, "")
	LUA_EXPOSED_METHOD_SET(KeyboardEvents,	SetAcceptingKeyboardEvents, "")

LUA_EXPOSED_END(CMessageHandler)

//#define _LUA_DEBUG_PRINT // Uncomment this line if you want debug output for every message


//------------------------------------------------------------------------------------------
//!
//!	GameMessage::GameMessage
//!	Construction
//!
//------------------------------------------------------------------------------------------
GameMessage::GameMessage( void )
:	m_obMessage(),
	m_obData(),
	m_iRequestCount( 0 ),
	m_iMaxSends( 0 ),
	m_fMessageDelay( 0.0f ),
	m_obReceivers(),
	m_iSendCount( 0 ),
	m_iRequestSendCount( 0 ),
	m_obDelayedSends()
{
}


//------------------------------------------------------------------------------------------
//!
//!	GameMessage::~GameMessage
//!	Destruction
//!
//------------------------------------------------------------------------------------------
GameMessage::~GameMessage( void )
{
}


//------------------------------------------------------------------------------------------
//!
//!	GameMessage::PostConstruct
//!	Check data after construction
//!
//------------------------------------------------------------------------------------------
void GameMessage::PostConstruct( void )
{
	// Make sure that we have a name
	user_warn_p( ( !m_obMessage.IsNull() ), ( "A GameMessage has been created with no name!" ) );

	// Make sure that the message delay is positive
	user_warn_p( ( m_fMessageDelay >= 0.0f ), ( "Message '%s' has a negative MessageDelay!\n", ntStr::GetString(m_obMessage) ) );
}


//------------------------------------------------------------------------------------------
//!
//!	GameMessage::EditorChangeValue
//!	If the data for this message is realtime edited then we reset all the dynamic stuff
//!
//------------------------------------------------------------------------------------------
bool GameMessage::EditorChangeValue( CallBackParameter, CallBackParameter  )
{
	// Reset
	Reset();

	// Don't know what this means
	return true;
}


//------------------------------------------------------------------------------------------
//!
//!	GameMessage::Reset
//!	Set the message back to the serialised state
//!
//------------------------------------------------------------------------------------------
void GameMessage::Reset( void )
{
	// Reset our counters
	m_iSendCount = 0;
	m_iRequestSendCount = 0;

	// Clear out our delayed send list
	m_obDelayedSends.clear();
}


//------------------------------------------------------------------------------------------
//!
//!	GameMessage::Update
//!	We have time delays on the messages so we need an update
//!
//------------------------------------------------------------------------------------------
void GameMessage::Update( float fTimeStep )
{
	// If we have no delayed items then there is nothing to do
	if ( m_obDelayedSends.size() == 0 )
		return;

	// Loop through our delayed times and see if we need to send any messages
	ntstd::List<float>::iterator obEnd = m_obDelayedSends.end();
	for ( ntstd::List<float>::iterator obIt = m_obDelayedSends.begin(); obIt != obEnd; )
	{
		// Reduce this value by the timestep
		*obIt -= fTimeStep;

		// If our time delay is up..
		if ( *obIt <= 0.0f )
		{
			// Send out the defined message
			SendMessage();

			// Clear this item and progress to the next
			obIt = m_obDelayedSends.erase( obIt );
		}

		else
		{
			// Progress to the next item
			++obIt;
		}
	}
}


//------------------------------------------------------------------------------------------
//!
//!	GameMessage::SendRequest
//!	Request that this message be sent, it may not be sent immediately or at all depending 
//!	on parameters
//!
//------------------------------------------------------------------------------------------
bool GameMessage::SendRequest( const CEntity* pobDefaultReceiver, const CEntity* pobSender )
{
	// If we have already been sent our maximum number of times we can't do anything,
	// but if Max Sends is zero we have no limit
	if ( ( m_iSendCount >= m_iMaxSends ) && ( m_iMaxSends != 0 ) )
		return false;

	// Increment our counter for send requests
	m_iRequestSendCount++;

	// Who wants to send the message?
	m_pobSender = pobSender;

	// If our send requests are enough to send a message
	if ( m_iRequestSendCount >= m_iRequestCount )
	{
		// Reset out send request counter
		m_iRequestSendCount = 0;

		// Check whether a message will be sent
		if ( WillSend() )
		{
			// We are definately going to send this message now - update the send count
			m_iSendCount++;

			// If we should use the default receiver - we'll use the direct message handler send
			if ( ( m_obReceivers.size() == 0 ) && ( pobDefaultReceiver ) )
			{
				// Make sure that the default receiver entity has a message handler
				if ( pobDefaultReceiver->GetMessageHandler() )
				{
					// Build and pass the message
					Message obMessage(eUndefinedMsgID);
					//obMessage.AssignNewTable( CLuaGlobal::Get().State() );
					obMessage.SetString(CHashedString(HASH_STRING_MSG), m_obMessage );
					obMessage.SetString(CHashedString(HASH_STRING_DATA), m_obData );
					obMessage.SetEnt(CHashedString(HASH_STRING_SENDER), m_pobSender );

					// Doesn't matter what our delay is - consider it delayed
					pobDefaultReceiver->GetMessageHandler()->QueueMessageDelayed( obMessage, m_fMessageDelay );
				}

				// Otherwise warn that we are trying to send to something without a message handler
				else
				{
					ntPrintf( "GameMessage: Failed to send message %s to %s, entity has no message handler\n", ntStr::GetString(m_obMessage), pobDefaultReceiver->GetName().c_str() );
				}
			}

			else
			{
				// If we have no time delay on this message - send it
				if ( m_fMessageDelay <= 0.0f )
					SendMessage();

				// ...otherwise we add our time delay to the time delayed list
				else
					m_obDelayedSends.push_back( m_fMessageDelay );
			}

			// We have generated a message
			return true;
		}
	}

	// If we are here then we haven't sent a message
	return false;
}


//------------------------------------------------------------------------------------------
//!
//!	GameMessage::WillSend
//!	Will this message be sent again
//!
//------------------------------------------------------------------------------------------
bool GameMessage::WillSend( void ) const 
{ 
	return ( ( ( m_iSendCount < m_iMaxSends ) || ( m_iMaxSends == 0 ) )
			 && 
			 ( !m_obMessage.IsNull() ) 
			 /*&& 
			 ( m_obMessage.GetLength() != 0 ) */); 
}

	
//------------------------------------------------------------------------------------------
//!
//!	GameMessage::SendMessage
//!	Actually send out the message
//!
//------------------------------------------------------------------------------------------
void GameMessage::SendMessage( void )
{
	// Loop through all our receivers ( if there are any )
	ntstd::List<void*>::iterator obEnd = m_obReceivers.end();
	for ( ntstd::List<void*>::iterator obIt = m_obReceivers.begin(); obIt != obEnd; ++obIt )
	{
		// Get the dataobject interface from our referenced receiver, so we can identify different types to send to
		DataObject* pobInterface = ObjectDatabase::Get().GetDataObjectFromPointer( *obIt );

		// Make sure we successfully got the interface
		if ( pobInterface )
		{
			// If the element is a trigger volume then we need to do something a bit different for the moment.  
			// Hopefully in the future we shall have a single mechanism here - triggers to become entities
			if ( strcmp( pobInterface->GetClassName(), "CTriggerVolume" ) == 0 )
			{
				// Cast our element to a trigger volume
				CTriggerVolume* pobTriggerVolume = static_cast<CTriggerVolume*>( *obIt );

				// We can't send a real message here - for the moment we are directly manipulating triggers - activate
				if ( m_obMessage.GetHash() == CHashedString( "Activate" ).GetValue() )
				{
					// ntPrintf( "Activating trigger volume %s\n", pobTriggerVolume->GetName() );
					pobTriggerVolume->SetActive( true );
				}

				// ....deactivate
				else if ( m_obMessage.GetHash() == CHashedString( "Deactivate" ).GetValue() )
				{
					// ntPrintf( "Deactivating trigger volume %s\n", pobTriggerVolume->GetName() );
					pobTriggerVolume->SetActive(false);
				}
			}

			// If this interface is a camera then we need to do something direct too.  Hopefully this method
			// will change too.  Can we make cameras entities?  Is there any point - do we get reuse?
			else if ( strcmp( pobInterface->GetClassName(), "BasicCameraTemplate" ) == 0 )
			{
				// Cast our element to a camera definition
				BasicCameraTemplate* pobCamera = static_cast<BasicCameraTemplate*>( *obIt );

				// We can't send a real message here - for the moment we are directly manipulating cameras - activate
				if ( m_obMessage.GetHash() == CHashedString( "Activate" ).GetValue() )
				{
					// ntPrintf( "Activating trigger volume %s\n", pobTriggerVolume->GetName() );
					pobCamera->Activate();
				}
			}

			// Otherwise we can assume that the other element is entity based
			else
			{
				// Cast our element pointer to a CEntity
				CEntity* pobEntity = static_cast<CEntity*>( *obIt );

				// Make sure that the receiver entity has a message handler
				if ( pobEntity->GetMessageHandler() )
				{
					// Tell people what we are upto
					// ntPrintf( "GameMessage: Sending message %s to entity %s\n", *m_obMessage, pobEntity->GetName().c_str() );

					// Build and pass the message
					Message obMessage(eUndefinedMsgID);
					obMessage.SetString(CHashedString(HASH_STRING_MSG), m_obMessage );
					obMessage.SetString(CHashedString(HASH_STRING_DATA), m_obData );
					obMessage.SetEnt(CHashedString(HASH_STRING_SENDER), m_pobSender );
					pobEntity->GetMessageHandler()->QueueMessage( obMessage );
				}

				// Otherwise warn that we are trying to send to something without a message handler
				else
				{
					ntPrintf( "GameMessage: Failed to send message %s to %s, entity has no message handler\n", ntStr::GetString(m_obMessage), pobEntity->GetName().c_str() );
				}
			}
		}
		else
		{
			// Warn users that we have an issue here
			ntPrintf( "GameMessage: Error, trying to send msg %s to a receiver that doesn't exist in the object database.\n", ntStr::GetString(m_obMessage) );
		}
	}
}


//------------------------------------------------------------------------------------------
//!
//!	GameEvent::GameEvent
//!	Construction
//!
//------------------------------------------------------------------------------------------
GameEvent::GameEvent( void )
:	m_obEventName(),
	m_obGameMessages()
{
}


//------------------------------------------------------------------------------------------
//!
//!	GameEvent::~GameEvent
//!	Destruction
//!
//------------------------------------------------------------------------------------------
GameEvent::~GameEvent( void )
{
}


//------------------------------------------------------------------------------------------
//!
//!	GameEvent::PostConstruct
//!	Check data after construction
//!
//------------------------------------------------------------------------------------------
void GameEvent::PostConstruct( void )
{
	// Make sure that we have a name
	user_warn_p( ( !m_obEventName.IsNull() ), ( "A GameEvent has been created with no name!" ) );

	// Make sure that we have a list of messages
	user_warn_p( ( m_obGameMessages.size() > 0 ), ( "GameEvent '%s' has no message list!\n", ntStr::GetString(m_obEventName) ) );
}


//------------------------------------------------------------------------------------------
//!
//!	GameEvent::Reset
//!	Reset our messages to their serialise state
//!
//------------------------------------------------------------------------------------------
void GameEvent::Reset( void )
{
	// If we have no messages drop out
	if ( m_obGameMessages.size() == 0 )
		return;

	// Simply call reset on all our messages
	ntstd::List<GameMessage*>::iterator obEnd = m_obGameMessages.end();
	for ( ntstd::List<GameMessage*>::iterator obIt = m_obGameMessages.begin(); obIt != obEnd; ++obIt )
		( *obIt )->Reset();
}


//------------------------------------------------------------------------------------------
//!
//!	GameEvent::Update
//!	We need an update here so we can deal with time delayed messages
//!
//------------------------------------------------------------------------------------------
void GameEvent::Update( float fTimeStep )
{
	// If we have no messages drop out
	if ( m_obGameMessages.size() == 0 )
		return;

	// Simply call update on all our messages
	ntstd::List<GameMessage*>::iterator obEnd = m_obGameMessages.end();
	for ( ntstd::List<GameMessage*>::iterator obIt = m_obGameMessages.begin(); obIt != obEnd; ++obIt )
		( *obIt )->Update( fTimeStep );
}


//------------------------------------------------------------------------------------------
//!
//!	GameEvent::IsEvent
//!	Is this event identified by the given name?
//!
//------------------------------------------------------------------------------------------
bool GameEvent::IsEvent( CHashedString pcEventName )
{
	// Check the given string against our name
	return ( m_obEventName == pcEventName );
}


//------------------------------------------------------------------------------------------
//!
//!	GameEvent::FireEvent
//!	Fire this event - split from IsEvent so we can force events - returns true if a message 
//!	was sent.
//!
//------------------------------------------------------------------------------------------
bool GameEvent::FireEvent( const CEntity* pobDefaultReceiver, const CEntity* pobSender )
{
	// We need to return true if a message was send
	bool bMessageSent = false;

	// If we have no messages drop out
	if ( m_obGameMessages.size() == 0 )
		return bMessageSent;

	// Simply call SendRequest on all our messages
	ntstd::List<GameMessage*>::iterator obEnd = m_obGameMessages.end();
	for ( ntstd::List<GameMessage*>::iterator obIt = m_obGameMessages.begin(); obIt != obEnd; ++obIt )
	{
		bMessageSent |= ( *obIt )->SendRequest( pobDefaultReceiver, pobSender );
	}

	// Indicate whether or not a message was successfully sent
	return bMessageSent;
}


//------------------------------------------------------------------------------------------
//!
//!	GameEvent::WillFireMessages
//!	Will this event result in any messages
//!
//------------------------------------------------------------------------------------------
bool GameEvent::WillFireMessages( void ) const
{
	// Set up a return value
	bool bWillFireMessage = false;

	// Loop through our associated messages
	ntstd::List<GameMessage*>::const_iterator obEnd = m_obGameMessages.end();
	for ( ntstd::List<GameMessage*>::const_iterator obIt = m_obGameMessages.begin(); obIt != obEnd; ++obIt )
	{
		bWillFireMessage |= ( *obIt )->WillSend();
	}

	// Tell them what we found out
	return bWillFireMessage;
}


//------------------------------------------------------------------------------------------
//!
//!	GameEventList::GameEventList
//!	Construction
//!
//------------------------------------------------------------------------------------------
GameEventList::GameEventList( void )
:	m_obGameEvents()
{
}


//------------------------------------------------------------------------------------------
//!
//!	GameEventList::~GameEventList
//!	Destruction
//!
//------------------------------------------------------------------------------------------
GameEventList::~GameEventList( void )
{
}


//------------------------------------------------------------------------------------------
//!
//!	GameEventList::PostConstruct
//!	Check data after construction
//!
//------------------------------------------------------------------------------------------
void GameEventList::PostConstruct( void )
{
#ifndef _RELEASE
	// Make sure that we don't have multiple entries of the same name in our list

	// If we have no events drop out
	if ( m_obGameEvents.size() == 0 )
		return;

	// Loop through all our events ( outer )
	ntstd::List<GameEvent*>::iterator obEnd = m_obGameEvents.end();
	for ( ntstd::List<GameEvent*>::iterator obOuterIt = m_obGameEvents.begin(); obOuterIt != obEnd; ++obOuterIt )
	{
		// Loop through them all again in an inner loop
		for ( ntstd::List<GameEvent*>::iterator obInnerIt = m_obGameEvents.begin(); obInnerIt != obEnd; ++obInnerIt )
		{
			// If our iterators aren't pointing to the same element
			if ( obInnerIt != obOuterIt )
			{
				// Make sure that we don't have the same name reference here
				user_warn_p( !( *obInnerIt )->IsEvent( ( *obOuterIt )->GetName() ), ( "Event '%s' is referenced twice in a single GameEventList!\n", ntStr::GetString(( *obOuterIt )->GetName()) ) );
			}
		}
	}

#endif
}


//------------------------------------------------------------------------------------------
//!
//!	GameEventList::Reset
//!	Reset our events to their serialise state
//!
//------------------------------------------------------------------------------------------
void GameEventList::Reset( void )
{
	// If we have no events drop out
	if ( m_obGameEvents.size() == 0 )
		return;

	// Simply call reset on all our events
	ntstd::List<GameEvent*>::iterator obEnd = m_obGameEvents.end();
	for ( ntstd::List<GameEvent*>::iterator obIt = m_obGameEvents.begin(); obIt != obEnd; ++obIt )
		( *obIt )->Reset();
}


//------------------------------------------------------------------------------------------
//!
//!	GameEventList::Update
//!	We need an update here so we can deal with time delayed messages
//!
//------------------------------------------------------------------------------------------
void GameEventList::Update( float fTimeStep )
{
	// If we have no events drop out
	if ( m_obGameEvents.size() == 0 )
		return;

	// Simply call update on all our events
	ntstd::List<GameEvent*>::iterator obEnd = m_obGameEvents.end();
	for ( ntstd::List<GameEvent*>::iterator obIt = m_obGameEvents.begin(); obIt != obEnd; ++obIt )
		( *obIt )->Update( fTimeStep );
}


//------------------------------------------------------------------------------------------
//!
//!	GameEventList::ProcessEvent
//!	Signal that an event has taken place - returns true if a message was sent
//!
//------------------------------------------------------------------------------------------
bool GameEventList::ProcessEvent( const char* pcEventName, const CEntity* pobDefaultReceiver, const CEntity* pobSender )
{
	// We'll need to check if an event is fired
	bool bEventFired = false;

	// If we have no events drop out
	if ( m_obGameEvents.size() == 0 )
		return bEventFired;

	// Otherwise loop through our events
	ntstd::List<GameEvent*>::iterator obEnd = m_obGameEvents.end();
	for ( ntstd::List<GameEvent*>::iterator obIt = m_obGameEvents.begin(); obIt != obEnd; ++obIt )
	{
		// Fire off the event if it is 'the one'
		if ( ( *obIt )->IsEvent( pcEventName ) )
		{
			// Fire the event and return that we have found some info
			bEventFired |= ( *obIt )->FireEvent( pobDefaultReceiver, pobSender );
		}
	}

	// Return true if a message was sent
	return bEventFired;
}

//------------------------------------------------------------------------------------------
//!
//!	GameEventList::WillHandleEvent
//!	Does this event list contain an event of a particular name?
//!
//------------------------------------------------------------------------------------------
bool GameEventList::WillHandleEvent( const char* pcEventName )
{
	// We'll need to check if an event is fired
	bool bWillHandle = false;

	// If we have no events drop out
	if ( m_obGameEvents.size() == 0 )
		return bWillHandle;

	// Otherwise loop through our events
	ntstd::List<GameEvent*>::iterator obEnd = m_obGameEvents.end();
	for ( ntstd::List<GameEvent*>::iterator obIt = m_obGameEvents.begin(); obIt != obEnd; ++obIt )
	{
		// Fire off the event if it is 'the one'
		if ( ( *obIt )->IsEvent( pcEventName ) )
		{
			// Fire the event and return that we have found some info
			bWillHandle |= ( *obIt )->WillFireMessages();
		}
	}

	// Return true if a message was sent
	return bWillHandle;
}


//------------------------------------------------------------------------------------------
//!
//!	GameEventList::WillFireMessages
//!	Will firing anything on this event list result in any messages
//!
//------------------------------------------------------------------------------------------
bool GameEventList::WillFireMessages( void ) const
{
	// Set up a return value
	bool bWillFireMessage = false;

	// Loop through our associated events
	ntstd::List<GameEvent*>::const_iterator obEnd = m_obGameEvents.end();
	for ( ntstd::List<GameEvent*>::const_iterator obIt = m_obGameEvents.begin(); obIt != obEnd; ++obIt )
	{
		bWillFireMessage |= ( *obIt )->WillFireMessages();
	}

	// Tell them what we found out
	return bWillFireMessage;
}


//------------------------------------------------------------------------------------------
//!
//!	CMessageSender::SendEmptyMessage
//!	Send a message with out any attached data
//!
//------------------------------------------------------------------------------------------
void CMessageSender::SendEmptyMessage( CHashedString pcMessageType, const CMessageHandler* pobDestination )
{
	static const CHashedString messageName(HASH_STRING_MSG);
	Message obMessage(eUndefinedMsgID);
	//obMessage.AssignNewTable(CLuaGlobal::Get().State());
	obMessage.SetString(messageName,pcMessageType);
	//ntAssert(!obMessage["Msg"].IsNil());
	pobDestination->QueueMessage(obMessage);
}

//------------------------------------------------------------------------------------------
//!
//!	CMessageSender::SendEmptyMessage
//!	Send a message with out any attached data
//!
//------------------------------------------------------------------------------------------
void CMessageSender::SendEmptyMessage( MessageID eMessageID, const CMessageHandler* pobDestination )
{
	Message obMessage(eMessageID);
	pobDestination->QueueMessage(obMessage);
}


//------------------------------------------------------------------------------------------
//!
//!	CMessageHandler::CMessageHandler
//!	Construction
//!
//------------------------------------------------------------------------------------------
CMessageHandler::CMessageHandler( CEntity* pobHostEnt, GameEventList* pobEventList ) 
:	m_pobHost( pobHostEnt ),
	m_obStateStack(),
	m_ActiveQueue(0),
	m_obMsgHandlers(),
	m_pobGameEventList( pobEventList ),
	m_KeyboardEvents(false),
	m_bProcessing(false)
{
	ATTACH_LUA_INTERFACE(CMessageHandler);

#ifndef _RELEASE
	// If we have a host grab the name for debug purposes
	if ( m_pobHost )
		m_obDebugInfo.SetEntityName( m_pobHost->GetName().c_str() );
#endif
}

//------------------------------------------------------------------------------------------
//!
//!	CMessageHandler::~CMessageHandler
//!	Destruction
//!
//------------------------------------------------------------------------------------------
CMessageHandler::~CMessageHandler()
{
}


//------------------------------------------------------------------------------------------
//!
//!	CMessageHandler::QueueMessage
//!	Add a message item to the queue stored on this object - which is run through on update
//!
//------------------------------------------------------------------------------------------
void CMessageHandler::QueueMessage(const Message& obMsg) const
{
	Receive(obMsg);
}


//------------------------------------------------------------------------------------------
//!
//!	CMessageHandler::QueueMessage
//!	Add a message item to the queue stored on this object - which is run through on update
//!
//------------------------------------------------------------------------------------------
void CMessageHandler::QueueMessageDelayed(const Message& obMsg, float fDelay) const
{
	Receive(obMsg, fDelay );
}


//------------------------------------------------------------------------------------------
//!
//!	CMessageHandler::Receive
//!	Add a message item to the queue stored on this object - which is run through on update
//!
//! PLEASE NOTE!!!!!!
//! IF YOU CHANGE THIS FUNCTION, YOU MAY NEED TO CHANGE THE 
//! template<> void CMessageHandler::ReceiveMsg<msg_combat_killed> () const
//! FUNCTIONS TOO!
//!
//------------------------------------------------------------------------------------------
void CMessageHandler::Receive(const Message& obMsg, float fDelay) const
{
	// Lets log a warning if we're paused (probably due to sectorisation)
	if(m_pobHost->IsPaused())
	{
		ntPrintf("Warning - Message '%s' sent to inactive Entity %s.\n", ntStr::GetString(obMsg.GetHashedString(CHashedString(HASH_STRING_MSG))), ntStr::GetString(m_pobHost->GetName()));
	}

	// Delays can now be packaged up in the message itself to make them full atoms - JML added 13-12-05
	if(fDelay == 0.f && obMsg.IsNumber("Delay"))
	{
		fDelay = obMsg.GetFloat("Delay");
	}

	// Make sure the object is on the correct stack
	if( fDelay <= 0.0f )
	{
		// Place the message in the list
		m_obMessageQueue[m_ActiveQueue].push_back( obMsg );
	}
	else
	{
		m_obDelayedQueue.push_back( Delayed( fDelay, obMsg ) );
	}
}

//------------------------------------------------------------------------------------------
//!
//!	CMessageHandler::ProcessEventWithDefaultReceiver
//!	Process an event if we can.  We might want to put some debug stuff in here
//!
//------------------------------------------------------------------------------------------
bool CMessageHandler::ProcessEventWithDefaultReceiver( const char* pcEventName, const CEntity* pobDefaultReceiver, const CEntity* pobSender ) 
{ 
	// If we have an game event list pass on the call
	if ( m_pobGameEventList ) 
		return m_pobGameEventList->ProcessEvent( pcEventName, pobDefaultReceiver, pobSender );

	// ...otherwise return false
	else
		return false;
}

//------------------------------------------------------------------------------------------
//!
//!	CMessageHandler::WillHandleEvent
//! Does this message handler handle an event of this name?
//!
//------------------------------------------------------------------------------------------
bool CMessageHandler::WillHandleEvent( const char* pcEventName ) 
{ 
	// If we have an game event list pass on the call
	if ( m_pobGameEventList ) 
		return m_pobGameEventList->WillHandleEvent( pcEventName );

	// ...otherwise return false
	else
		return false;
}


//------------------------------------------------------------------------------------------
//!
//!	CMessageHandler::Update
//!	Process the queued messages.  Deal with coroutines/callbacks etc etc
//!
//------------------------------------------------------------------------------------------
void CMessageHandler::Update( float fTimeDelta )
{
	// Update our game event list if we have one
	if ( m_pobGameEventList && m_pobGameEventList->Size() )
		m_pobGameEventList->Update( fTimeDelta );

	// If we don't have a host entity - drop out here - this allows us to
	// be used by other items that aren't entity based
	if ( !m_pobHost )
		return;

	m_bProcessing = true;

	// Process the delayed list.. 
	// Note, when removing an item from a ntstd::List whilst iterating through it. What happens to the 
	// iterator object if the item removed is the current item referenced from the iterator?
	// (I've assumed the iterator is forwarded to the next item)
	ntstd::List< Delayed >::iterator obDIt = m_obDelayedQueue.begin();

	// Run through all the delayed messages
	while( obDIt != m_obDelayedQueue.end() )
	{
		// Grab a reference to the delay from ##the iterator
		float& fDelay = obDIt->m_Delay;

		// Decrease the time in the iterator
		fDelay -= fTimeDelta;

		// If the message is ready to be processed, then place it on the queue
		if( fDelay <= 0.0f )
		{
			// Place a copy of the message on the normal queue.
			m_obMessageQueue[m_ActiveQueue].push_back( obDIt->m_Msg );

			// Remove the delayed message from the delayed message queue
			obDIt = m_obDelayedQueue.erase( obDIt );
		}
		else
		{
			// Delayed message still not ready for processing, ignore it for the moment. 
			++obDIt;
		}
	}

#ifdef PLATFORM_PC
	if( m_KeyboardEvents 
		&& CInputHardware::Get().GetContext() == INPUT_CONTEXT_SCRIPT )
	{
		CInputKeyboard& obKeyb = CInputHardware::Get().GetKeyboard();

		for( int iKey = 0; iKey < 256; ++iKey )
		{
			if( obKeyb.IsKeyPressed( (KEY_CODE) iKey ) || obKeyb.IsKeyPressed( (KEY_CODE) iKey, KEYM_SHIFT ) )
			{
				char acKey[2] = {obKeyb.KeyCodeToAscii( (KEY_CODE) iKey ) };
				m_obMessageQueue[m_ActiveQueue].push_back( CMessageHandler::Make( m_pobHost, "msg_key_pressed", (const char*)acKey) );
			}
			else if( obKeyb.IsKeyReleased( (KEY_CODE) iKey ) || obKeyb.IsKeyReleased( (KEY_CODE) iKey, KEYM_SHIFT ) )
			{
				char acKey[2] = {obKeyb.KeyCodeToAscii( (KEY_CODE) iKey ) };
				m_obMessageQueue[m_ActiveQueue].push_back( CMessageHandler::Make( m_pobHost, "msg_key_released", (const char*)acKey) );
			}
		}
	}
#endif

	// Make a copy of the message queue, then clear out the original, create a double buffer to prevent too many allocations. 
	ntstd::List<Message>& obMessageQueue = m_obMessageQueue[m_ActiveQueue];
	m_ActiveQueue = 1 - m_ActiveQueue;

	// Process each message until the list is empty
	while( !obMessageQueue.empty() )
	{
		CLuaGlobal::Get().SetTarg( m_pobHost );

		// Get the message
		Message& obMsg = obMessageQueue.front();

		// If we have a FSM then use it...
		if ( m_pobHost->HasFSM() )
		{
			m_pobHost->GetFSM()->ProcessMessage(obMsg);
		}

		// ...But also pass the message on to any other handlers too.
			for( ntstd::List< Handler >::iterator obIt = m_obMsgHandlers.begin();
					obIt != m_obMsgHandlers.end();
					++obIt )
			{
				Handler& rHandler = *obIt;
				rHandler.Call( obMsg );
			}

#ifndef _RELEASE
		//have we got the state table on the stack?
		if ( m_obStateStack.IsTable() ) 
		{
			const CHashedString& pcType = obMsg.GetHashedString(CHashedString(HASH_STRING_MSG));
			NinjaLua::LuaObject obFunc;
			m_obStateStack.Get("GetStateName",obFunc);
			NinjaLua::LuaFunctionRet<const char*> obGetStateName(obFunc);
			const char* pcStateName = obGetStateName(m_obStateStack);
			m_obDebugInfo.AddCallbackInformation( pcType, pcStateName);
		} 
#endif

		// remove it from the front of the queue
		obMessageQueue.pop_front();
	}

	CLuaGlobal::Get().SetTarg( 0 );

	m_bProcessing = false;
}


/***************************************************************************************************
*
*	FUNCTION		CMessageHandler::Post(NinjaLua::LuaState&)
*
*	DESCRIPTION		
*
***************************************************************************************************/
int	CMessageHandler::Post(NinjaLua::LuaState& State)
{
	return PostCommon(State, 0.0f);
}


/***************************************************************************************************
*
*	FUNCTION		CMessageHandler::PostCommon(NinjaLua::LuaState&)
*
*	DESCRIPTION		This function is really nasty.  We should either be using the new bind 
*					functionality or the old stuff.  This is a horrible hybrid.  Now we have
*					entity components explicitly unpacking lua states, which is a step backwards
*					as far as i am concerned.  GH
*
***************************************************************************************************/
int	CMessageHandler::PostCommon(NinjaLua::LuaState& State, float fDelay)
{
	// The first argument on the stack is the Destination entity
	CEntity* pDest = NinjaLua::LuaValue::Get<CEntity*>( State, 1 );

	// If the entity isn't valid, then return now. 
	if( !pDest || !pDest->GetMessageHandler() )
		return (0);

	// This call is only relevant if this message handler has a parent entity
	if ( !m_pobHost )
		return 0;

	// The second argument on the stack is the message
	//const char* pcString = NinjaLua::LuaValue::Get<const char*>( State, 2 );
	CHashedString pcString = NinjaLua::LuaValue::Get<CHashedString>( State, 2 );

	// Depending on the number of aruments.. 
	if( State.GetTop() == 2 )
	{
		pDest->GetMessageHandler()->Receive( CMessageHandler::Make( m_pobHost, pcString ), fDelay );
	}
	else if( State.GetTop() == 3 )
	{
		pDest->GetMessageHandler()->Receive( CMessageHandler::Make( m_pobHost, pcString,	NinjaLua::LuaValue::Get<NinjaLua::LuaObject>( State, 3 ) ), fDelay );
	}
	else if( State.GetTop() == 4 )
	{
		pDest->GetMessageHandler()->Receive(	CMessageHandler::Make( m_pobHost, pcString,	NinjaLua::LuaValue::Get<NinjaLua::LuaObject>( State, 3 ),
												NinjaLua::LuaValue::Get<NinjaLua::LuaObject>( State, 4 ) ), 
												fDelay );
	}
	else if( State.GetTop() == 5 )
	{
		pDest->GetMessageHandler()->Receive(	CMessageHandler::Make( m_pobHost, pcString,	NinjaLua::LuaValue::Get<NinjaLua::LuaObject>( State, 3 ),
												NinjaLua::LuaValue::Get<NinjaLua::LuaObject>( State, 4 ), NinjaLua::LuaValue::Get<NinjaLua::LuaObject>( State, 5 ) ), 
												fDelay );
	}
	else
	{
		ntAssert( false );
	}

	return (0);
}


/***************************************************************************************************
*
*	FUNCTION		CMessageHandler::PostDelayed(NinjaLua::LuaState&)
*
*	DESCRIPTION		
*
***************************************************************************************************/
int	CMessageHandler::PostDelayed(NinjaLua::LuaState& State)
{
	float fDelayed = NinjaLua::LuaValue::Get<float>( State, 1 );
	lua_remove( State, 1 );
	return PostCommon(State, fDelayed);
}


//------------------------------------------------------------------------------------------
//!
//!	CMessageHandler::GetCurrentState
//!	Return string for current state
//!
//------------------------------------------------------------------------------------------
void CMessageHandler::ClearMessageQueues()
{
	m_obMessageQueue[0].clear();
	m_obMessageQueue[1].clear();
	m_obDelayedQueue.clear();
}


//------------------------------------------------------------------------------------------
//!
//!	CMessageHandler::GetCurrentState
//!	Return string for current state
//!
//------------------------------------------------------------------------------------------
const char* CMessageHandler::GetCurrentState ()
{
	if ( m_obStateStack.IsTable() ) 
	{
		NinjaLua::LuaObject obFunc;
		m_obStateStack.Get("GetStateName",obFunc);
		NinjaLua::LuaFunctionRet<const char*> obGetStateName(obFunc);
		return obGetStateName(m_obStateStack);
	}

	// New C++ FSMs
	if(m_pobHost->HasFSM())
		return m_pobHost->GetFSM()->GetStateName();

	return "No State";
}


//------------------------------------------------------------------------------------------
//!
//!	CMessageHandler::SetCallback
//!	Assign a function to be called on reciept of the named message
//!
//------------------------------------------------------------------------------------------
void CMessageHandler::SetCallback( const char* pcMsgType, NinjaLua::LuaObject& obHandler )
{
	UNUSED( pcMsgType );
	UNUSED( obHandler );
	ntAssert_p( false, ("Depreciated") );
}


//------------------------------------------------------------------------------------------
//!
//!	CMessageHandler::ClearCallbacks
//!	Remove all the currently set callback message/function pairs
//!
//------------------------------------------------------------------------------------------
void CMessageHandler::ClearCallbacks()
{
	ntAssert_p( false, ("Depreciated") );
}


#ifndef _RELEASE
//------------------------------------------------------------------------------------------
//!
//!	CMessageHandler::MessageHandlerDebug::CopyNameData
//!	Creates a string and copies the information if there is any
//!
//------------------------------------------------------------------------------------------
void CMessageHandler::MessageHandlerDebug::CopyNameData( const char* pcName, const char*& pcDestination )
{
	// Make sure our destination pointer is currently NULL
	ntAssert( !pcDestination );

	if ( pcName )
	{
		char* pcTempName = NT_NEW char[strlen(pcName) + 1];
		strcpy( pcTempName, pcName );
		pcDestination = pcTempName;
	}
	else
	{
		char* pcTempName = NT_NEW char[strlen("NO INFO") + 1];
		strcpy( pcTempName, "NO INFO" );
		pcDestination = pcTempName;
	}
}


//------------------------------------------------------------------------------------------
//!
//!	CMessageHandler::MessageHandlerDebug::SetEntityName
//!	Simple debug handling stuff
//!
//------------------------------------------------------------------------------------------
void CMessageHandler::MessageHandlerDebug::SetEntityName( const char* pcEntityName )
{
	CopyNameData( pcEntityName, m_pcEntityName );
}


//------------------------------------------------------------------------------------------
//!
//!	CMessageHandler::MessageHandlerDebug::AddCallbackInformation
//!	Simple debug handling stuff
//!
//------------------------------------------------------------------------------------------
void CMessageHandler::MessageHandlerDebug::AddCallbackInformation(	CHashedString, 
																	const char*)
{
/*
	// Create a new callback item
	CallbackDebug* pTempCallbackData = NT_NEW CallbackDebug;

	// Copy the data we have been passed if valid
	CopyNameData( pcMessageName, pTempCallbackData->pcMessageName );
	CopyNameData( pcStateName, pTempCallbackData->pcStateName );

	// Shift down all the data in our history array
	NT_DELETE( CallbackItems[NUM_REMBEMERED_STATES - 1] );
	for( int i = NUM_REMBEMERED_STATES - 2; i >= 0; --i )
	{
		CallbackItems[i + 1] = CallbackItems[i];
	}

	// Add the new one to the top
	CallbackItems[0] = pTempCallbackData;

#ifdef _LUA_DEBUG_PRINT

	// Give some output if it is required

	// Make sure the entity name is set up - should be
	ntAssert( m_pcEntityName );

	// Print out the information that we have just been given
		ntPrintf( "Lua call on %s, message: %s, current state: %s. \n",
			m_pcEntityName,
			CallbackItems[0]->pcMessageName,
			CallbackItems[0]->pcStateName );

#endif // _LUA_DEBUG_PRINT
*/
}


//------------------------------------------------------------------------------------------
//!
//!	CMessageHandler::MessageHandlerDebug::DebugPrintStateAndMessageData
//!	Simple debug handling stuff
//!
//------------------------------------------------------------------------------------------
bool CMessageHandler::MessageHandlerDebug::DebugPrintStateAndMessageData( int iState, float fX, float fY, uint32_t dwColour ) const
{
	// Make sure that we we hold enough state for the requested information
	ntAssert_p( ( iState < NUM_REMBEMERED_STATES ), ( "The history for this many states is not being held" ) );

	if ( ( CallbackItems[iState] ) && ( CallbackItems[iState]->pcStateName ) && ( CallbackItems[iState]->pcMessageName ) )
	{
		// Print out the information like we have been told
		g_VisualDebug->Printf2D(	fX, fY, dwColour, 0, "State[%d]: %s, %s", 
										iState,
										CallbackItems[iState]->pcStateName,
										CallbackItems[iState]->pcMessageName );

		return true;
	}

	return false;
}

#endif // _RELEASE

//------------------------------------------------------------------------------------------
//!
//!	CMessageHandler::SetStateMachine
//!	Sets the message handler with the lua state machine based message handler
//!
//------------------------------------------------------------------------------------------
void CMessageHandler::SetStateMachine( NinjaLua::LuaObject obStateStack )
{
	ntError_p( obStateStack.IsTable(), ("SetStatemachine not supplied with a state machine") );
		
	// Make a copy of the state stack
	m_obStateStack = obStateStack;
}

//------------------------------------------------------------------------------------------
//!
//!	CMessageHandler::AddHandler
//!	This could be overkill having a list of Message handlers, but required for prototying 
//!
//------------------------------------------------------------------------------------------
void CMessageHandler::AddHandler( NinjaLua::LuaObject obC, NinjaLua::LuaFunction obH )
{
	m_obMsgHandlers.push_back( Handler( obC, obH ) );
}

//------------------------------------------------------------------------------------------
//!
//!	CMessageHandler::RemoveHandler
//!
//------------------------------------------------------------------------------------------
void CMessageHandler::RemoveHandler( NinjaLua::LuaObject obContext )
{
	// Find the handler and remove it. 
	for( ntstd::List< Handler >::iterator obIt = m_obMsgHandlers.begin(); obIt != m_obMsgHandlers.end(); ++obIt )
	{
		Handler& rHandler = *obIt;

		if( rHandler.m_obContext == obContext )
		{
			m_obMsgHandlers.erase( obIt );
			return;
		}
	}
}

//------------------------------------------------------------------------------------------
//!
//!	CMessageHandler::Handler::Call
//!	
//!
//------------------------------------------------------------------------------------------
void CMessageHandler::Handler::Call(Message& obMessage)
{
	// If the handler isn't valid. Then return now
	if( !m_obHandler.IsFunction() )
		return;

	// This is a temporary measure until we have all messages working on ID.
	obMessage.VerifyMsgString();

	if( m_obContext.IsNil() )
		m_obHandler( &obMessage );
	else
		m_obHandler( m_obContext, &obMessage );
}

//------------------------------------------------------------------------------------------
//!
//!	CMessageHandler::DumpSuperLog
//!	
//!
//------------------------------------------------------------------------------------------
#if (!defined _RELEASE) && (!defined _MASTER)

void CMessageHandler::DumpSuperLog( void ) const
{
	for( ntstd::Vector< MESSAGE_LOG >::const_iterator obIt = m_obMsgLog.begin();  obIt != m_obMsgLog.end(); ++obIt )
	{
		ntPrintf("%d:%d:%s:%s:%s\n", obIt->m_TimeStamp, obIt->m_CombatState, obIt->m_StatePreMsg.c_str(), obIt->m_StatePostMsg.c_str(), obIt->m_Msg.c_str() );
	}
}
#endif


//------------------------------------------------------------------------------------------
//!
//!	MessageHandler::TestPause
//!	
//!
//------------------------------------------------------------------------------------------
#if (!defined _RELEASE) && (!defined _MASTER)
void CMessageHandler::TestPaused(const Message& obMessage) const
{
	if(m_pobHost->IsPaused())
	{
		ntPrintf("Warning - Message '%s' sent to inactive Entity %s.\n", ntStr::GetString(obMessage.GetHashedString(CHashedString(HASH_STRING_MSG))), ntStr::GetString(m_pobHost->GetName()));
	}	
}
#endif

//------------------------------------------------------------------------------------------
//!
//!	MessageGlobals::MessageGlobals
//!	
//!
//------------------------------------------------------------------------------------------
MessageGlobals::MessageGlobals()
	:	m_iDummy(0)
// Construct all the global messages
	INITIALISE_GLOBAL_MESSAGE(msg_combat_killed)
	INITIALISE_GLOBAL_MESSAGE(msg_combat_struck)
	INITIALISE_GLOBAL_MESSAGE(msg_animdone)
	INITIALISE_GLOBAL_MESSAGE(msg_ai_attack)
	INITIALISE_GLOBAL_MESSAGE(msg_button_power)
	INITIALISE_GLOBAL_MESSAGE(msg_button_range)
	INITIALISE_GLOBAL_MESSAGE(msg_release_power)
	INITIALISE_GLOBAL_MESSAGE(msg_release_range)
	INITIALISE_GLOBAL_MESSAGE(msg_button_special) 
	INITIALISE_GLOBAL_MESSAGE(msg_buttonattack)
	INITIALISE_GLOBAL_MESSAGE(msg_release_attack)
	INITIALISE_GLOBAL_MESSAGE(msg_buttonaction)
	INITIALISE_GLOBAL_MESSAGE(msg_release_action)
	INITIALISE_GLOBAL_MESSAGE(msg_buttongrab)
	INITIALISE_GLOBAL_MESSAGE(msg_release_grab)
	INITIALISE_GLOBAL_MESSAGE(msg_buttondodge)

{
}

//------------------------------------------------------------------------------------------
//!
//!	MessageGlobals::~MessageGlobals
//!	
//!
//------------------------------------------------------------------------------------------
MessageGlobals::~MessageGlobals()
{

}



