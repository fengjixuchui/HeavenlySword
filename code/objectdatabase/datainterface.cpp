
#include "objectdatabase/dataobject.h"
#include "objectdatabase/class_factory.h"
#include "objectdatabase/guidclient.h"

#include "objectdatabase/objectdatabase.h"
#include "objectdatabase/repeat.h"

//---------------------------------
// DataInterface
//---------------------------------

DataInterface::DataInterface( const ntstd::String& name, const GameGUID& id ) :
	m_Name( name ),
	m_InterfaceGUID( id )
{
}

void DataInterface::SetData( DataObject* pDO, const CHashedString& name, const ntstd::String& value )
{
	START_LOAD_TIME_PROFILER_ACC( 1 );

	DataInterface::iterator diIt = begin();
	while( diIt != end() )
	{
		if( name == (*diIt)->GetName() )
		{
			(*diIt)->SetData( pDO, value );
			ObjectDatabase::Get().SignalObjectChange( pDO );
			STOP_LOAD_TIME_PROFILER_ACC( 1 );
			return;
		}
		++diIt;
	}
	STOP_LOAD_TIME_PROFILER_ACC( 1 );
}

const ntstd::String DataInterface::GetData( DataObject* pDO, const CHashedString& name )
{
	DataInterface::iterator diIt = begin();
	while( diIt != end() )
	{
		if( name == (*diIt)->GetName() )
		{
			return (*diIt)->GetData( pDO );
		}
		++diIt;
	}
	return "";
}

void DataInterface::EditData( const DataObject* pDO , const CHashedString& name, const ntstd::String& data )
{
	DataInterface::iterator diIt = begin();
	while( diIt != end() )
	{
		if( name == (*diIt)->GetName() )
		{
			(*diIt)->EditData( pDO, data );
			ObjectDatabase::Get().SignalObjectChange( pDO );
			return;
		}
		++diIt;
	}
}

DataInterfaceField* DataInterface::GetFieldByName( const CHashedString& name )
{
		DataInterface::iterator diIt = begin();
		while( diIt != end() )
		{
			if( name == (*diIt)->GetName() )
			{
				return (*diIt);
			}
			++diIt;
		}

		return 0;
}

//---------------------------------
// StdDataInterface
//---------------------------------

StdDataInterface::StdDataInterface(	const ntstd::String& interface_name, 
						const ntstd::String& base_name, const GameGUID& id ) :
	DataInterface( interface_name, id ),
	m_ObjectName( base_name )
{
}

void StdDataInterface::SetData( DataObject* pDO, const CHashedString& name, const ntstd::String& value )
{
	if( m_NameFieldMap.find( name ) == m_NameFieldMap.end() )
	{
		one_time_assert_p( 1, m_NameFieldMap.find( name ) != m_NameFieldMap.end(), ("Attribute %s does not exist in interface %s, check the log for details\nThis warning will only appear once\n", ntStr::GetString(name), pDO->GetClassName() ) );
		return;
	}
	DataInterfaceField* pInterface = m_NameFieldMap[name];

	START_LOAD_TIME_PROFILER_ACC( 2 );
	pInterface->SetData( pDO, value );
	ObjectDatabase::Get().SignalObjectChange( pDO );
	STOP_LOAD_TIME_PROFILER_ACC( 2 );

}
StdDataInterface* StdDataInterface::CastTo( const ntstd::String& to, DataObject* object )
{
	typedef ntstd::Vector< ntstd::String, Mem::MC_ODB > checkIF;
	typedef ntstd::Vector< ntstd::String, Mem::MC_ODB > stackIF;

	stackIF possibleInterfaces;
	checkIF checkedInterfaces;

	// 100 should be plenty of interface space
	checkedInterfaces.reserve( 100 ); 
//	possibleInterfaces.reserve( 100 );

	// push the highest class we can cast to
	possibleInterfaces.push_back( object->GetClassName() );

	while( !possibleInterfaces.empty() )
	{
		stackIF::value_type className = possibleInterfaces.back();

		// check for the easy case (we want to cast to the actual class)
		if( strcmp( className.c_str(), to.c_str() ) == 0 )
		{
			return ObjectDatabase::Get().GetInterface( className.c_str() );
		}
		// pop off stack and push onto checked list
		possibleInterfaces.pop_back();
		checkedInterfaces.push_back( className );

		// now push children that we haven't already checked.
		CastToContainer::const_iterator castIt = m_CastToNames.begin();
		while( castIt != m_CastToNames.end() )
		{

			// check to see if we have already encountered this one
			checkIF::const_iterator found = ntstd::find( checkedInterfaces.begin(), checkedInterfaces.end(), (*castIt) );
			if( found == checkedInterfaces.end() )
			{
				// nope so push it on the stack
				possibleInterfaces.push_back( (*castIt) );
			}

			++castIt;
		}
	};

	return 0;
}

StdDataInterface::~StdDataInterface()
{
	iterator diIt = begin();
	while( diIt != end() )
	{
		NT_DELETE_CHUNK( Mem::MC_ODB, (*diIt) );
		++diIt;
	}

	CallBackMap::iterator cbIt = m_CallBackMap.begin();
	while( cbIt != m_CallBackMap.end() )
	{
		NT_DELETE_CHUNK( Mem::MC_ODB, cbIt->second );
		++cbIt;
	}
}

void StdDataInterface::CreateFastIndices()
{
	FieldContainer::const_iterator fcIt = begin();

	while( fcIt != end() )
	{
		m_NameFieldMap[ (*fcIt)->GetName() ] = (*fcIt);
		++fcIt;
	}
}

void StdDataInterface::EditData( DataObject* pDO, const CHashedString& name, const ntstd::String& value )
{
	if( m_NameFieldMap.find( name ) == m_NameFieldMap.end() )
	{
		one_time_assert_p( 1, m_NameFieldMap.find( name ) != m_NameFieldMap.end(), ("Attribute %s does not exist in interface %s, check the log for details\nThis warning will only appear once\n", ntStr::GetString(name), pDO->GetClassName() ) );
		return;
	}
	m_NameFieldMap[name]->EditData( pDO, value );
	ObjectDatabase::Get().SignalObjectChange( pDO );
}

const ntstd::String StdDataInterface::GetData( DataObject* pDO, const CHashedString& name )
{
	ntAssert_p( m_NameFieldMap.find( name ) != m_NameFieldMap.end(), ("Attribute does not exist in this interface") );
	return m_NameFieldMap[name]->GetData( pDO );
}

void StdDataInterface::AddField( DataInterfaceField* pField)
{
	m_Fields.push_back(pField);
	CreateFastIndices();
}


DataInterfaceField* StdDataInterface::GetFieldByName( const CHashedString& name )
{
	if( m_NameFieldMap.find( name ) != m_NameFieldMap.end() )
		return m_NameFieldMap[name];
	else
		return 0;
}

bool StdDataInterface::CallBack( const DataObject* pDO, const ntstd::String& func, CallBackParameter param0, CallBackParameter param1 )
{
	if( m_CallBackMap.find( func ) != m_CallBackMap.end() )
	{
		return m_CallBackMap[ func ]->CallBack( pDO, param0, param1 );
	} else
	{
		return true;
	}
}

//---------------------------------------------------------------
//!
//!	StdDataInterface::InheritFields(const StdDataInterface& obOther)
//!	Copy the fields of the other interface to this.
//! Rebuilds the Name to field map also.
//!
//---------------------------------------------------------------
void StdDataInterface::InheritFields(const StdDataInterface& obOther)
{
	ntError_p(obOther.m_Fields.size() > 0,("Other interface empty"));
	// Copy the fields vector
	for (FieldContainer::const_iterator fcIt = obOther.begin(); fcIt != obOther.end(); ++fcIt)
	{
		m_Fields.push_back((*fcIt)->Clone());
	}

	// Copy the cast to names
	for (CastToContainerConstIterator ccIt = obOther.m_CastToNames.begin(); ccIt != obOther.m_CastToNames.end(); ++ccIt)
	{
		m_CastToNames.push_back(*ccIt);
	}

	// Copy the Callback map
	for (CallBackConstIterator cbIt = obOther.m_CallBackMap.begin(); cbIt != obOther.m_CallBackMap.end(); ++cbIt)
	{
		m_CallBackMap[cbIt->first] = cbIt->second->Clone();
	}

	// Recreate the NameFieldMap
	CreateFastIndices();
}


