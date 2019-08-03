
#include "objectdatabase/dataobject.h"
#include "objectdatabase/objectdatabase.h"

StdEnumDIF::StdEnumDIF(  const CHashedString& name, DataObjectField< value_type >* obj, const value_type& defaultT ) :
	m_pDataObject( obj ),
	m_FieldName( name ),
	m_Default( defaultT ),
	m_GlobalName() // an empty global name indicates local mode
{
}

StdEnumDIF::~StdEnumDIF()
{
	NT_DELETE_CHUNK( Mem::MC_ODB, m_pDataObject );
}

const ntstd::String StdEnumDIF::GetData( const DataObject* pDO ) const
{
	if( m_GlobalName.empty() )
	{
		// local mode enum
		// local modes just store an integer, the metadata holds the readible name
		ntstd::Ostringstream dataStr;
		dataStr << m_pDataObject->Get(pDO);
		return dataStr.str();
	} else
	{
		// global mode enum
		// global modes stores the data as GLOBALNAME:VALUE_NAME (the original enum way)
		value_type val = m_pDataObject->Get(pDO);

		const GlobalEnum& genum = ObjectDatabase::Get().GetGlobalEnum( m_GlobalName );
		const ntstd::String& name = genum.GetName( val );

		return m_GlobalName + ":" + name;
	}
}

void StdEnumDIF::SetData(  const DataObject* pDO, const ntstd::String& data )
{
	START_LOAD_TIME_PROFILER_ACC( 9 );
	if( m_GlobalName.empty() )
	{
		// local mode enum
		// local modes just store an integer, the metadata holds the readible name
		ntstd::Istringstream dataStr(data);
		unsigned int enumValue;
		dataStr >> enumValue;
		enum_container::const_iterator enIt = m_EnumValues.find( enumValue );
		ntAssert_p( enIt != m_EnumValues.end(), ("Enum value doesn't exist") );
		m_pDataObject->Set( pDO, enumValue );
	} else
	{
		// global mode enum
		// global modes stores the data as GLOBALNAME:VALUE_NAME (the original enum way)
		char pWholeString[1024];
		char* pFirstPart = pWholeString;
		UNUSED(pFirstPart);
		char* pSecondPart = pWholeString;
		strncpy( pWholeString, data.c_str(), 1024 );
		for( int i=0;i < 1024;i++)
		{
			ntAssert_p( i != 1023, ("This is not a valid enum String (1024 characters before a colon!") ); // this would be an invalid string
			ntAssert_p( *pSecondPart != 0, ("This is not a valid enum string (No Colon before the end)") ); // so would this
			if( *pSecondPart == ':' )
			{
				*pSecondPart = 0; // crack the string into two
				pSecondPart++;
				break;
			}
			pSecondPart++;
		}
		ntAssert( strcmp( pFirstPart, m_GlobalName.c_str() ) == 0 );
		
		const GlobalEnum& genum = ObjectDatabase::Get().GetGlobalEnum( m_GlobalName );
		value_type val = genum.GetValue( pSecondPart );
		m_pDataObject->Set( pDO, val );
	}
	STOP_LOAD_TIME_PROFILER_ACC( 9 );

}

void StdEnumDIF::EditData( const DataObject* pDO, const ntstd::String& data)
{
	// for enum's edit is the same as set
	SetData( pDO, data );
}

void StdEnumDIF::SetToDefault( const DataObject* pBase )
{
	m_pDataObject->GetToSet(pBase) = m_Default;
}

void StdEnumDIF::SetDefault(const ntstd::String& defaultT)
{
	ntstd::Istringstream dataStr(defaultT);
	dataStr >> m_Default;
};

const ntstd::String StdEnumDIF::GetDefault() const
{
	ntstd::Ostringstream dataStr;
	dataStr << m_Default;
	return dataStr.str();
};


void StdEnumDIF::PushEnumValue( unsigned int value, const ntstd::String& name )
{
	m_EnumValues[ value ] = name;
}

const ntstd::String StdEnumDIF::GetMetaData() const												
{
	ntstd::Ostringstream dataStr;
	value_type dummy;

	if( m_GlobalName.empty() )
	{
		// local mode
		dataStr << "enum" << " " ;					// type of thing (enum)
		dataStr << typeof_name(dummy) << " ";		// type holding enum (unsigned int)
		dataStr << m_EnumValues.size() << " " ;		// number of enums values
		enum_container::const_iterator enIt = m_EnumValues.begin();
		while( enIt != m_EnumValues.end() )
		{
			// pair of value and string value
			dataStr << (*enIt).first << " " <<  (*enIt).second << " ";
			++enIt;
		}
	} else
	{
		dataStr << "global_enum " << m_GlobalName;
	}

	return dataStr.str();											
}				
																													
DataInterfaceField* StdEnumDIF::Clone() const
{
	StdEnumDIF* pNew = NT_NEW_CHUNK(Mem::MC_ODB) StdEnumDIF( m_FieldName, m_pDataObject->Clone(), m_Default );
	pNew->m_EnumValues = m_EnumValues;
	pNew->m_GlobalName = m_GlobalName;
	return pNew;
}

void StdEnumDIF::SetGlobalMode( const ntstd::String& globalName )
{
	ntAssert_p( m_EnumValues.empty(), ("Local and global enums don't mix, its one of t'other") );
	// set us into global enum mode (where the enums come from an enum in enumlist
	m_GlobalName = globalName;
}

