
#include "objectdatabase/dataobject.h"
#include "objectdatabase/class_factory.h"
#include "objectdatabase/guidclient.h"
#include "objectdatabase/objectdatabase.h"

GameGUID DataObject::GetObjectGUIDFromName(const ntstd::String& name )
{
	char buf[256];

	if( name.empty() )
	{
		return GameGUID();
	}

	// look the name up, if it doesn't exist add it
	if( GuidClient::Get().Lookup( name.c_str(), buf, 256 ) == false )
	{
		if( GuidClient::Get().Add( name.c_str() ) == false )
		{
			ntError_p( false, ("The GUID server does not appear to be responding\n") );
		}
		if( GuidClient::Get().Lookup( name.c_str(), buf, 256 ) == false )
		{
			ntError_p( false, ("Problems with the GUID system\n") );
		}
	}

	// for now use that guid
	GameGUID guid;
	guid.SetFromString( buf );

	return guid;
}

GameGUID DataInterface::GetInterfaceGUIDFromName( const ntstd::String& name )
{

	char buf[256];
	// look the name up, if it doesn't exist add it
	if( GuidClient::Get().Lookup( name.c_str(), buf, 256 ) == false )
	{
		if( GuidClient::Get().Add( name.c_str() ) == false )
		{
			ntError_p( false, ("The GUID server does not appear to be responding\n") );
		}
		if( GuidClient::Get().Lookup( name.c_str(), buf, 256 ) == false )
		{
			ntError_p( false, ("Problems with the GUID system\n") );
		}
	}

	// for now use that guid
	GameGUID guid;
	guid.SetFromString( buf );
	return guid;
}

ntstd::String DataObject::GetUniqueName()
{
	char pBuf[256];
	if( GuidClient::Get().AddUnique( pBuf ) == false )
	{
		ntAssert_p( false, ("Eek, AddUnique failed") );
	}

	return ntstd::String(pBuf);
}

void DataObject::SetClassName( const char* pClassName )
{
	m_ClassName = pClassName;
}



const ntstd::String DataObjectContainerDIF::GetData( const DataObject* pDO ) const
{
	ntstd::Ostringstream dataStr;
	const value_type& listRef = m_pDataObject->Get( pDO );
	dataStr << listRef.size();
	value_type::const_iterator tIt = listRef.begin();
	while( tIt != listRef.end() )
	{
		DataObject* pDO = *tIt;
		GameGUID guid = pDO->GetGUID();

		dataStr << " " << guid;
		++tIt;
	}
	return dataStr.str();
}
void DataObjectContainerDIF::SetData(  const DataObject* pDO, const ntstd::String& data )
{
	START_LOAD_TIME_PROFILER_ACC( 10 );

	STOP_LOAD_TIME_PROFILER_ACC( 2000 );

	ntstd::Istringstream dataStr(data);
	unsigned int listSize = 0;
	dataStr >> listSize;

	value_type& listRef = m_pDataObject->GetToSet( pDO );
	for( unsigned int i=0;i < listSize;i++)
	{
		GameGUID guid;
		dataStr >> guid;
		DataObject* pDONew = ObjectDatabase::Get().GetObjectOrForwardReference( guid );
		listRef.insert( listRef.end(), pDONew );
	}

	STOP_LOAD_TIME_PROFILER_ACC( 10 );
}
void DataObjectContainerDIF::EditData(  const DataObject* pDO, const ntstd::String& data )
{
	ntstd::Istringstream dataStr(data);
	value_type& listRef = m_pDataObject->GetToSet( pDO );

	char command = 0;
	dataStr >> command;

	switch( command )
	{
	case 'A':
		{
			// add the following guid/dataobject to the end of this container
			GameGUID guid;
			dataStr >> guid;
			listRef.insert( listRef.end(), ObjectDatabase::Get().GetDataObjectFromGUID( guid ) );
			break;
		}
	case 'D':
		{
			// free the specified index
			unsigned int index;
			dataStr >> index;
			value_type::iterator lIt = listRef.begin();
			ntstd::advance( lIt, index );
			listRef.erase( lIt );
			break;
		}
	case 'E':
		{
			// index newGUID
			unsigned int index;
			dataStr >> index;
			GameGUID newGuid;
			dataStr >> index >> newGuid;
			value_type::iterator lIt = listRef.begin();
			ntstd::advance( lIt, index );
			(*lIt) = ObjectDatabase::Get().GetDataObjectFromGUID( newGuid );
			break;
		}
	default:
		ntError_p( false, ("Invalid edit command") );
	}
}
