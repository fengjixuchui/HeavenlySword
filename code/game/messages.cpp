//------------------------------------------------------------------------------------------
//!
//!	\file messages.cpp
//!
//------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
// Redefinition of MSG declaration macros
//------------------------------------------------------------------------------------------
#include "messages.h"
#include "message.h"

#undef _MESSAGES_H
#undef BEGIN_MSG_LIST
#undef END_MSG_LIST
#undef REGISTER_MSG

struct MessageIDPair
{
	//char*		_pcMessageName;
	CHashedString	_pcMessageName;
	MessageID		_id;
};

static int iNextMsgID = 1;

#define BEGIN_MSG_LIST static MessageIDPair* GetMessageMap() { static MessageIDPair MessageMap[] = {	\
	{ CHashedString("FSMOnEnter"), (MessageID)-2 },	\
	{ CHashedString("FSMOnExit"), (MessageID)-1 },	\
	{ CHashedString("eUndefinedMsgID"), (MessageID)0 },

//#define END_MSG_LIST { "NULL", -999 }, };
#define END_MSG_LIST { CHashedString(HASH_STRING_NULL), (MessageID)-999 }, }; return MessageMap; }

#define REGISTER_MSG(msg) { CHashedString( #msg ), (MessageID)iNextMsgID++ },


//------------------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------------------
#include "messages.h"


//------------------------------------------------------------------------------------------
//!
//!	Message::VerifyMessageID
//!	Attempts to give a message without an ID, an ID based on the message string
//!
//------------------------------------------------------------------------------------------
void Message::VerifyMessageID()
{
	MessageID iMsgID = GetID();

	// Check if the message doesnt have an ID yet
	if (iMsgID == eUndefinedMsgID)
	{
		CHashedString msgName = GetHashedString("Msg");

		int iMapIter = 0;

		//while ( stricmp(MessageMap[iMapIter]._pcMessageName, "NULL") != 0 )
		while ( GetMessageMap()[iMapIter]._pcMessageName != CHashedString(HASH_STRING_NULL) )
		{
			if(msgName == GetMessageMap()[iMapIter]._pcMessageName)
			{
				m_id = GetMessageMap()[iMapIter]._id;
				break;
			}

			iMapIter++;
		}
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Message::VerifyMsgString
//!	Attempts to give a message with an ID, the correct "Msg" Param required by other code
//!
//------------------------------------------------------------------------------------------
void Message::VerifyMsgString()
{
	if(m_obInner->FindParam("Msg") < 0)
	{
		MessageID iMsgID = GetID();

		int iMapIter = 0;

		//while ( stricmp(MessageMap[iMapIter]._pcMessageName, "NULL") != 0 )
		while ( GetMessageMap()[iMapIter]._pcMessageName != CHashedString(HASH_STRING_NULL) )
		{
			if(iMsgID == GetMessageMap()[iMapIter]._id)
			{
				//msg.Set("Msg", MessageMap[iMapIter]._pcMessageName );
				SetString(CHashedString(HASH_STRING_MSG), GetMessageMap()[iMapIter]._pcMessageName );
				break;
			}

			iMapIter++;
		}
	}
}


//------------------------------------------------------------------------------------------
//!
//!	Message::GetIDString
//!	
//!
//------------------------------------------------------------------------------------------
#ifdef _DEBUG
const char* Message::GetIDName() const
{
	return ntStr::GetString(GetMessageMap()[m_id]._pcMessageName);
}
#endif
