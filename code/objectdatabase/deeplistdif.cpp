#include "dataobject.h"
// Alexey : used in InsertGUID as a temporary thing...
#ifdef PLATFORM_PS3
#include <calloca>
#else
#include <malloc.h>
#endif

const ntstd::String DeepListDIF::GetData( const DataObject* pDO ) const
{
	ntstd::Ostringstream dataStr;
	int dataValues = 0;
	DeepList& pDeepList = m_pDataObject->GetToSet( pDO );

	pDeepList.ValidateFixup();
	ntstd::List<DataObject*, Mem::MC_ODB>::iterator tIt = pDeepList.m_obObjects.begin();
	while( tIt != pDeepList.m_obObjects.end() )
	{
		DataObject *pPointedTo = *tIt;
		if( pPointedTo != 0 && !pPointedTo->GetGUID().IsNull() )
		{
			dataValues++;
			dataStr << " " << pPointedTo->GetGUID();
		} else {
			// we've got an invalid object in the list, but we need to be careful about removing it;
			// for the moment, let's just log it
			ntPrintf("'%s' in '%s' has an invalid object in its deep list, skipping\n", ntStr::GetString(m_FieldName), ntStr::GetString(pDO->GetName()));
		}
		++tIt;
	}

	ntstd::Ostringstream resultStr;
	resultStr << dataValues;
	resultStr << dataStr.str();
	return resultStr.str();
}

void DeepListDIF::SetData(  const DataObject* pDO, const ntstd::String& data )
{
	START_LOAD_TIME_PROFILER_ACC( 3 );

	ntstd::Istringstream dataStr(data);
	unsigned int listSize = 0;
	dataStr >> listSize;

	DeepList& pDeepList = m_pDataObject->GetToSet( pDO );
	for( unsigned int i=0;i < listSize;i++)
	{
		GameGUID guid;
		dataStr >> guid;
		InsertGUID( pDO, pDeepList, guid );
	}
	STOP_LOAD_TIME_PROFILER_ACC( 3);
}

void DeepListDIF::EditData(  const DataObject* pDO, const ntstd::String& data )
{
	ntstd::Istringstream dataStr(data);
	DeepList& pDeepList = m_pDataObject->GetToSet( pDO );

	char command = 0;
	dataStr >> command;

	switch( command )
	{
	case 'A':
		{
			// add the following guid/dataobject to the end of this container
			GameGUID guid;
			dataStr >> guid;
			pDeepList.ValidateFixup();
			InsertGUID( pDO, pDeepList, guid );
			break;
		}

	case 'D':
		{
			// free the specified index
			GameGUID guid;
			dataStr >> guid;
			DataObject* pDO = ObjectDatabase::Get().GetObjectOrForwardReference( guid );
			pDeepList.UnlinkDataObject( pDO );
			break;
		}
	case 'E':
		{
			// index newGUID
			unsigned int index;
			GameGUID newGuid;
			dataStr >> index >> newGuid;
			// free the specified index
			pDeepList.ValidateFixup();
			ntstd::List<DataObject*, Mem::MC_ODB>::iterator lIt = pDeepList.m_obObjects.begin();
			ntstd::advance( lIt , index );
			DataObject *pNewPointedTo = ObjectDatabase::Get().GetDataObjectFromGUID( newGuid );
			if( lIt != pDeepList.m_obObjects.end() &&  pNewPointedTo != 0 )
			{
				(*lIt) = pNewPointedTo;
			}
			break;
		}
	default:
		ntError_p( false, ("Invalid edit command") );
	}
}

void DeepListDIF::SetToDefault( const DataObject* pBase )
{
	m_pDataObject->SetToDefault(pBase, defaultList );
}

void DeepListDIF::SetToDefaultPost( DataObject*, InterfaceCallBack* )
{
}

void DeepListDIF::UnSetFromDefault( const DataObject* pBase )
{
	m_pDataObject->UnSetFromDefault( pBase );
}

void DeepListDIF::SetDefault(const ntstd::String& )
{
}

const ntstd::String DeepListDIF::GetDefault() const
{
	return "0";
}

CHashedString DeepListDIF::GetName() const
{
	return m_FieldName;
}

const ntstd::String DeepListDIF::GetMetaData() const
{
	return "DeepList";
}

unsigned int DeepListDIF::GetType() const
{
	DeepList dummy;
	return typeof_num(dummy);
}

const ntstd::String DeepListDIF::GetFieldClass() const
{
	return "DeepList";
}

DataInterfaceField* DeepListDIF::Clone() const
{
	return NT_NEW_CHUNK(Mem::MC_ODB) DeepListDIF( m_FieldName, m_pDataObject->Clone() );
}

bool DeepListDIF::IsHard()
{
	return m_pDataObject->IsHard();
}

const void* DeepListDIF::GetVoidStarTo( const DataObject* pBase ) const
{
	return &m_pDataObject->Get( pBase );
}

void DeepListDIF::InsertGUID( const DataObject* pDO, DeepList& pDeepList, const GameGUID& guid )
{
	if ( !guid.IsNull() )
	{
		// we have a valid GUID lets ask the object database for a data object to it
		DataObject* pNewDO = ObjectDatabase::Get().GetObjectOrForwardReference( guid );
		ntAssert_p( pNewDO != 0, ("DeepList is referring to non-existant database object!") );
		ntAssert_p( !pNewDO->GetGUID().IsNull(), ("DeepList is referring to a null GUID database object!") );
		if( !pNewDO->IsValid() )
		{
			// object is nothing but a forward reference; the object itself doesn't 
			// exist so we store the DO object and register to the database we want
			// a fixup when the object really appears
			pDeepList.m_obFixupList.push_back( 0 );
			ntAssert_p( pNewDO->GetFixupList() != 0, ("Eek no fixup list") );
			pNewDO->AddFixup( (void**)&pDeepList.m_obFixupList.back() );
			// ALEXEY_TODO : god I hate what I have to do here
			const char* name = ntStr::GetString(pDO->GetName());
			static const char* refString = "Referer = ";
			unsigned int totalLen =  strlen(name) + strlen(refString) + 1;
			char* newName = (char*)alloca(totalLen);
			strcpy(newName, refString);
			strcat(newName, name);
			pNewDO->SetName( DataObject::NameType( DataObject::NameType(newName)));
		}
		else
		{
			// object exists and is ready to go
			pDeepList.LinkDataObject( pNewDO );
		}
	}
}

