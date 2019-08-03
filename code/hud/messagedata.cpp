//------------------------------------------------------------------------------------------
//!
//!	\file buttonhinter.cpp
//!
//------------------------------------------------------------------------------------------

// Necessary includes
#include "hud/messagedata.h"
#include "objectdatabase/dataobject.h"
#include "hud/hudmanager.h"
#include "gui/guitext.h"

// ---- Forward references ----

// ---- Interfaces ----
START_CHUNKED_INTERFACE( BaseMessageDataDef, Mem::MC_MISC  )
	PUBLISH_VAR_WITH_DEFAULT_AS ( m_obKey,		"",		Key )
END_STD_INTERFACE

START_CHUNKED_INTERFACE( MessageDataInt, Mem::MC_MISC  )
	DEFINE_INTERFACE_INHERITANCE(BaseMessageDataDef)
	COPY_INTERFACE_FROM(BaseMessageDataDef)

	PUBLISH_VAR_WITH_DEFAULT_AS( m_iValue, 0, Value ) 

	DECLARE_POSTCONSTRUCT_CALLBACK		(	PostConstruct )
END_STD_INTERFACE

START_CHUNKED_INTERFACE( MessageDataString, Mem::MC_MISC  )
	DEFINE_INTERFACE_INHERITANCE(BaseMessageDataDef)
	COPY_INTERFACE_FROM(BaseMessageDataDef)

	PUBLISH_VAR_WITH_DEFAULT_AS( m_obValue, "", Value ) 

	DECLARE_POSTCONSTRUCT_CALLBACK		(	PostConstruct )
END_STD_INTERFACE

void MessageDataInt::PostConstruct( void )
{
	if ( CHud::Exists() && CHud::Get().GetMessageDataManager() )
	{
		CHud::Get().GetMessageDataManager()->CreateValue( m_obKey, m_iValue);
	}
}

void MessageDataString::PostConstruct( void )
{
	if ( CHud::Exists() && CHud::Get().GetMessageDataManager() )
	{
		CHud::Get().GetMessageDataManager()->CreateValue( m_obKey, m_obValue);
	}
}

/////////////////////////////////////////
//
//	MessageDataManager
//	Construct/Destruct
//
/////////////////////////////////////////
MessageDataManager::~MessageDataManager()
{
	Reset();
}

/////////////////////////////////////////
//
//	MessageDataManager::Reset()
//
/////////////////////////////////////////
void MessageDataManager::Reset( void )
{
	// Don't clear these lists as they are populated globaly and not on level reseting

	//m_aobIntList.clear();
	//m_aobStringList.clear();
}

/////////////////////////////////////////
//
//	MessageDataManager
//	int operations
//
/////////////////////////////////////////
bool MessageDataManager::GetValue ( CHashedString obKey, int& iValue )
{
	iMapIter obIt = m_aobIntList.find ( obKey );

	if ( obIt != m_aobIntList.end() )
	{
		iValue = obIt->second;
		return true;
	}

	return false;
}

bool MessageDataManager::SetValue ( CHashedString obKey, int iValue )
{
	iMapIter obIt = m_aobIntList.find ( obKey );

	if ( obIt != m_aobIntList.end() )
	{
		obIt->second = iValue;
		return true;
	}

	return false;
}

void MessageDataManager::CreateValue ( CHashedString obKey, int iValue )
{
#ifdef _DEBUG
	iMapIter obIt = m_aobIntList.find ( obKey );
	ntAssert_p ( obIt == m_aobIntList.end(), ("Message data already in map\n") );
#endif // _DEBUG

	m_aobIntList[ obKey ] = iValue;
}

/////////////////////////////////////////
//
//	MessageDataManager
//	string operations
//
/////////////////////////////////////////
bool MessageDataManager::GetValue ( CHashedString obKey, ntstd::String& obValue )
{
	strMapIter obIt = m_aobStringList.find ( obKey );

	if ( obIt != m_aobStringList.end() )
	{
		obValue = obIt->second;
		return true;
	}

	return false;
}

bool MessageDataManager::SetValue ( CHashedString obKey, ntstd::String obValue )
{
	strMapIter obIt = m_aobStringList.find ( obKey );

	if ( obIt != m_aobStringList.end() )
	{
		obIt->second = obValue;
		return true;
	}

	return false;
}

void MessageDataManager::CreateValue ( CHashedString obKey, ntstd::String obValue )
{
#ifdef _DEBUG
	strMapIter obIt = m_aobStringList.find ( obKey );
	ntAssert_p ( obIt == m_aobStringList.end(), ("Message data already in map\n") );
#endif // _DEBUG

	m_aobStringList[ obKey ] = obValue;
}


/////////////////////////////////////////
//
//	MessageDataManager
//	To string interface
//
/////////////////////////////////////////
bool MessageDataManager::ToStringW ( CHashedString obKey, WCHAR_T* pwcBuffer, int iBufferLength )
{
	UNUSED ( iBufferLength );

	// Look in our int map
	iMapIter obIt = m_aobIntList.find ( obKey );

	if ( obIt != m_aobIntList.end() )
	{
		char pcTempString[MAX_PATH] = { 0 };

		ntstd::Ostringstream outStr;
		outStr << obIt->second;
		int iChars = strlen ( outStr.str().c_str() );
		ntError_p( iChars < iBufferLength, ("Buffer over run detected\n") );

		strncpy( pcTempString, outStr.str().c_str(), iChars );
		for ( int i = 0; i < iChars; i++ )
		{
			pwcBuffer[i] = (WCHAR_T)pcTempString[i];
		}
		
		return true;
	}

	// Look in our string map
	strMapIter obStrIt = m_aobStringList.find ( obKey );

	if ( obStrIt != m_aobStringList.end() )
	{
		// Get the string contents from the ID
		// I'm not using CStrings private BuildCharacters so no nested strings if the
		// string is included as game data, thank you.
		const WCHAR_T* pwcStringContents = CStringManager::Get().GetResourceString( obStrIt->second.c_str() );
		wcsncpy( pwcBuffer, pwcStringContents, wcslen ( pwcStringContents ) );

		return true;
	}

	return false;
}
