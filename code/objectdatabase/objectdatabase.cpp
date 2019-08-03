//------------------------------------------------------
//!
//!	\file core\object_database.cpp
//!
//------------------------------------------------------

#include "objectdatabase/dataobject.h"
#include "objectdatabase/objectchangetracer.h"

// Include the Expat XML Parser
#include "expat/xmlparse.h"
#include "editable/enums.h"
#include "objectdatabase/guidclient.h"

#include "core/profiling.h"

REGISTER_INTERFACE( SoftObjectFactoryInterface );

// debuging aid
//#define BREAK_ON_CONTAINERNAME "Z:/HS/content_neutral/entities/Resources/ninja_sequences/Fort/Seq_ForestClearingC_TreeCrossCommander/Seq_ForestClearingC_TreeCrossCommander.xml"
//#define BREAK_ON_BASENAME "Main_ArmyMessageHub_1"
//#define BREAK_ON_INTERFACENAME "ArmyMessageHub"

//------------------------------------------------------
//!
//! Constructs the object database
//!
//------------------------------------------------------
ObjectDatabase::ObjectDatabase() :
	m_GlobalObjectContainer( 0 ),
	m_CurrentContainer( 0 ),
	m_pObjectTracker( 0 )
{
	for(unsigned int i=0;i < ClassFactory::m_NumClassCons;i++)
	{
		const ClassFactory::ClassTable& clas = ClassFactory::m_ClassConstructors[i];
		m_HashToIndex[ clas.m_Hash ] = i;
	}

	// manually create the global container (its a special case)
	DataInterface* val = GetInterface( "ObjectContainer" );
	CStruct<UnknownClass>* obj = NT_NEW_CHUNK(Mem::MC_ODB) CStruct<UnknownClass>( val->CreateObject(), "GlobalContainer", "ObjectContainer" );
	m_GlobalObjectContainer = (ObjectContainer*)obj->GetBasePtr();
	m_GUIDIndex[ obj->GetGUID() ] = obj;
	m_BasePtrIndex[ obj->GetBasePtr() ] = obj;
	m_NameIndex[ obj->GetName() ] = obj;

	SetCurrentContainer( m_GlobalObjectContainer );

	BuildEnums();
}

//------------------------------------------------------
//!
//! Destroys the object database
//!
//------------------------------------------------------
ObjectDatabase::~ObjectDatabase()
{
	DeleteContainer( m_GlobalObjectContainer );

	// only guid index left are forward references that were never actually used
	// clean them up and list them to fix the XML
	GUIDIndex::iterator guIt = 	m_GUIDIndex.begin();
	while( guIt != m_GUIDIndex.end() )
	{
		GameGUID guid = guIt->first;
		DataObject* pDO = guIt->second;

		char buf[128] = {0};
		GuidClient::Get().LookupGuid( guid.GetAsString().c_str(), buf, 128 );
		ntPrintf( "Not in any container (unused forward reference?): %s %s %s \n", ntStr::GetString(pDO->GetName()), buf, ntStr::GetString(guid.GetAsString()) );
		pDO->DeleteFixupList();
		
		CStruct<UnknownClass>* unknown = (CStruct<UnknownClass>*)pDO;
		NT_DELETE_CHUNK( Mem::MC_ODB, unknown );

		++guIt;
	}
}
//------------------------------------------------------
//!
//! Clean forward references
//!
//------------------------------------------------------
void ObjectDatabase::CleanForwardReferences()
{
	// clean up any forward referecnces
	GUIDIndex::iterator guIt = 	m_GUIDIndex.begin();
	while( guIt != m_GUIDIndex.end() )
	{
		DataObject* pDO = guIt->second;

		if( strcmp( pDO->GetClassName(), "UnknownClass") == 0 )
		{
			ntError_p( pDO->IsValid() == false, ("All UnknownClass should be invalid\n") );
			pDO->DeleteFixupList();	
			CStruct<UnknownClass>* unknown = (CStruct<UnknownClass>*)pDO;
			NT_DELETE_CHUNK( Mem::MC_ODB, unknown );
			guIt = m_GUIDIndex.erase( guIt );
		} else
		{
			++guIt;
		}
	}
}

//------------------------------------------------------
//!
//! Insert a soft interface into the class factory system
//!
//------------------------------------------------------
void ObjectDatabase::RegisterSoftInterface( const char* pcString, ClassFactoryHelperBase* base )
{
	char buffer[256];
	strcpy( buffer, pcString );
	strcat( buffer, "Interface" );

	ClassFactory::RegisterClassConstructor( buffer, base );
	const ClassFactory::ClassTable& clas = ClassFactory::m_ClassConstructors[ClassFactory::m_NumClassCons-1];
	m_HashToIndex[ clas.m_Hash ] = ClassFactory::m_NumClassCons-1;
}


//------------------------------------------------------
//!
//! Set the current container, any creation that doesn't
//! pass a specific container will be created here
//!
//------------------------------------------------------
void ObjectDatabase::SetCurrentContainer( ObjectContainer* pContainer )
{
	m_CurrentContainer = pContainer;
}
//------------------------------------------------------
//!
//! returns the current container
//!
//------------------------------------------------------
ObjectContainer* ObjectDatabase::GetCurrentContainer()
{
	return m_CurrentContainer;
}


//------------------------------------------------------
//!
//! Creates and add a new container of the specfied name
//! guid can be a null GUID in which case one is created
//!
//------------------------------------------------------
ObjectContainer* ObjectDatabase::AddContainer( const char* pName, const GameGUID& guid )
{
	DataObject* pDO;
	if (guid.IsNull()) {
		// If the GUID is null, it can't have been created as a forward reference.
		pDO = ConstructObject( "ObjectContainer", pName, guid, 0, true, false );
	} else {
		// If the GUID is not null, it MIGHT have been created as a forward reference already.
		// If it wasn't, this will implicitly create the forward reference first.
		pDO = ObjectDatabase::Get().GetObjectOrForwardReference( guid );
		pDO->SetName( pName );
		// Now fill in the rest of the details of the forward reference, making it a valid reference.
		ObjectDatabase::Get().ConstructObject( "ObjectContainer", pDO, true, false );
	}
	
	ObjectContainer* objCont = (ObjectContainer*) pDO->GetBasePtr();
	m_ContainerList.push_back( objCont );

	return objCont;
}

//------------------------------------------------------
//!
//! Delete the container object, this will destroy all 
//! objects owned by this container
//!
//------------------------------------------------------
void ObjectDatabase::DeleteContainer( ObjectContainer* pContainer )
{
	Destroy<ObjectContainer>( pContainer );
}

//------------------------------------------------------
//!
//! Given a guid return a void* to the actual object
//! this can be cast directly to the actual class.
//! Returns NULL if the object isn't valid
//!
//------------------------------------------------------
void* ObjectDatabase::GetObjectPtrFromGUID( const GameGUID& guid ) const
{
	DataObject* pDO = GetValidDataObject( guid );
	if( pDO )
		return pDO->GetBasePtr();
	else
		return 0;
}
//------------------------------------------------------
//!
//! Given a pointer to an object returns its dataobject
//! This allows you to easily swap between the real object
//! and its associated DataObject
//!
//------------------------------------------------------
DataObject* ObjectDatabase::GetDataObjectFromPointer( const void* ptr ) const
{
	BasePtrIndex::const_iterator odmIt = m_BasePtrIndex.find( (void*) ptr );
	if( odmIt != m_BasePtrIndex.end() )
	{
		return (*odmIt).second;
	} else
	{
		return 0;
	}
}


//------------------------------------------------------
//!
//! Given a pointer to an object returns its name - if
//! there is an interface available. 
//!
//------------------------------------------------------
DataObject::NameType ObjectDatabase::GetNameFromPointer( const void* ptr ) const
{
	// Get our data interface
	DataObject* pDO = GetDataObjectFromPointer( ptr );

	// If we have one then pass back the name
	if ( pDO )
		return pDO->GetName();

	// Otherwise hand back a pointer to emptiness
	else
		return DataObject::NameType();
}

//------------------------------------------------------
//!
//! Given a pointer to an object returns the name of its
//! interface.  
//!
//------------------------------------------------------
const char* ObjectDatabase::GetInterfaceNameFromPointer( const void* ptr ) const
{
	// Get our data interface
	DataObject* pDO = GetDataObjectFromPointer( ptr );

	// If we have one then pass back the interface name
	if ( pDO )
		return pDO->GetClassName();

	// Otherwise hand back a pointer to emptiness
	else
		return "";
}

//------------------------------------------------------
//!
//! Given a guid return the associated DataObject or NULL
//! if the object database doesn't hold an object of that
//! GUID
//!
//------------------------------------------------------
DataObject* ObjectDatabase::GetDataObjectFromGUID( const GameGUID& guid ) const
{
	GUIDIndex::const_iterator odmIt = m_GUIDIndex.find( guid );
	if( odmIt != m_GUIDIndex.end() )
	{
		return (*odmIt).second;
	} else
	{
		return 0;
	}
}

//------------------------------------------------------
//!
//! Given a name string return the associated DataObject or NULL
//! if the object database doesn't hold an object of that
//! GUID
//!
//------------------------------------------------------
DataObject* ObjectDatabase::GetDataObjectFromName( const CHashedString& name ) const
{
	NameIndex::const_iterator odmIt = m_NameIndex.find( DataObject::NameType(name) );
	if( odmIt != m_NameIndex.end() )
	{
		return (*odmIt).second;
	} else
	{
		return 0;
	}
}


//------------------------------------------------------
//!
//! Given a name string return whether it is a valid container
//! name for the object database, i.e., it is not empty and
//! consists only of alpha-numeric characters (or the
//! following exceptions: '_', '-', '#', '!', ' ', ':', '\', '/', '.'
//!
//------------------------------------------------------
bool ObjectDatabase::IsValidContainerName( const ntstd::String& name ) const
{
	ntstd::String validObjectChars = "abcdefghijklmnopqrstuvwxyzABCDEFGHJIJKLMNOPQRSTUVWXYZ0123456789_-#!\\.:/ ";
	return !name.empty() && name.find_first_not_of( validObjectChars ) == ntstd::String::npos;
}

//------------------------------------------------------
//!
//! Given a name string return whether it is a valid object
//! name for the object database, i.e., it is not empty and
//! consists only of alpha-numeric characters (or the
//! following exceptions: '_', '-', '#', '!'
//!
//------------------------------------------------------
bool ObjectDatabase::IsValidObjectName( const ntstd::String& name ) const
{
	ntstd::String validObjectChars = "abcdefghijklmnopqrstuvwxyzABCDEFGHJIJKLMNOPQRSTUVWXYZ0123456789_-#!";
	return !name.empty() && name.find_first_not_of( validObjectChars ) == ntstd::String::npos;
}

//------------------------------------------------------
//!
//! Looks up a data object by GUID but return NULL if
//! its just a forward reference
//!
//------------------------------------------------------
DataObject* ObjectDatabase::GetValidDataObject( const GameGUID& guid ) const
{
	GUIDIndex::const_iterator odmIt = m_GUIDIndex.find( guid );
	if( odmIt != m_GUIDIndex.end() )
	{
		return 0;
	} else
	{
		if( (*odmIt).second->IsValid() )
		{
			return (*odmIt).second;
		} else
		{
			return 0;
		}
	}
}

//------------------------------------------------------
//!
//! Construct an object, this is the big one basically
//! replaced new for objects the live the object database
//! system.
//! \param pClassName class name you want to construct
//! \param name unique name for this new object
//! \param guid unique guid, a null guid will get an auto- 
//!		defined one
//! \param pContainer container to construct object into, 
//!		0 to use the current one
//! \param bDoDefault you probably do so leave true
//! \param bDoPostLoadDefaults if false, 
//!      call DoPostLoadDefaults manually when your ready
//! \return The DataObject use GetBasePtr() to get the actual
//!		object you asked for
//!
//------------------------------------------------------
DataObject* ObjectDatabase::ConstructObject(	const char* pClassName, 
												const ntstd::String& name, 
												const GameGUID& guid, 
												ObjectContainer* pContainer, 
												bool bDoDefaults, 
												bool bDoPostLoadDefaults )
{
	//ntPrintf("Constructing %s of type %s, container is %x\n", name.c_str(), pClassName, pContainer);
	//snPause();
	DataInterface* val = GetInterface( pClassName );
	ntError_p( val, ("Class '%s' does not exist\n",pClassName ) );

	CStruct<UnknownClass>* obj;
	if( guid.IsNull() )
	{
		ntError_p( !name.empty(), ("No GUID and no name bad - Container Name %s", pContainer->m_FileName.c_str() ) );
		obj = NT_NEW_CHUNK(Mem::MC_ODB) CStruct<UnknownClass>( val->CreateObject(), name, DataObject::GetObjectGUIDFromName(name), val->GetBaseObjectName().c_str() );
	} else
	{
		obj = NT_NEW_CHUNK(Mem::MC_ODB) CStruct<UnknownClass>( val->CreateObject(), name, guid, val->GetBaseObjectName().c_str() );
	}

	ntError_p( !obj->GetGUID().IsNull(), ("Eeek, GUID is NULL") );

	// unique GUID check
	GUIDIndex::const_iterator odmIt = m_GUIDIndex.find( obj->GetGUID() );
	UNUSED(odmIt);
	user_error_p( odmIt == m_GUIDIndex.end(), ("GUID isn't unique %s Name:", ntStr::GetString(obj->GetGUID().GetAsString()), ntStr::GetString(obj->GetName())) );

	m_GUIDIndex[ obj->GetGUID() ] = obj;
	m_BasePtrIndex[ obj->GetBasePtr() ] = obj;
	m_NameIndex[ obj->GetName() ] = obj;

	if( pContainer == 0 )
		pContainer = GetCurrentContainer();

	pContainer->AddObject( obj );
	obj->SetParent( pContainer );

	if( bDoDefaults )
	{
		// set defaults
		StdDataInterface* pInterface = (StdDataInterface*)val;
		StdDataInterface::iterator diIt = pInterface->begin();
		while( diIt != pInterface->end() )
		{
			(*diIt)->SetToDefault( obj );
			++diIt;
		}
	}
	if( bDoPostLoadDefaults )
	{

		DataObjectList objectToPost;
		objectToPost.push_back( obj );

		DoPostLoadDefaults( objectToPost );
	}

	// notify tracker
	if( m_pObjectTracker && obj->GetParent() )
		m_pObjectTracker->ObjectCreated( obj->GetGUID(), GetDataObjectFromPointer(obj->GetParent())->GetGUID() );

	return obj;
}

//------------------------------------------------------
//!
//! run through an object fixup list, fixing pointers
//! to point to this object (part of forward referencing)
//!
//------------------------------------------------------
void ObjectDatabase::DoPostLoadFixups( DataObject* pDO )
{
	// This is getting called a lot, but doesn't seem to take much time.
	// the profiler registered over 24000 calls in the army demo, which took less than 0.07 seconds in total.
	// LOAD_PROFILE( ObjectDatabase_DoPostLoadFixups )

	// do fixups
	if( pDO->GetFixupList() )
	{
		// fixup magic
		DataObject::FixupList::const_iterator fuIt = pDO->GetFixupList()->begin();
		while( fuIt != pDO->GetFixupList()->end() )
		{
			*(*fuIt) = pDO->GetBasePtr();
			++fuIt;
		}
		pDO->DeleteFixupList();
	}
}
//------------------------------------------------------
//!
//! Helper version for a single object DoPostLoadDefault
//!
//------------------------------------------------------
void ObjectDatabase::DoPostLoadDefaults( DataObject* pDO )
{
	DataObjectList listDO;
	listDO.push_back( pDO );
	DoPostLoadDefaults( listDO );
}

//------------------------------------------------------
//!
//! Walk the list of dataobjects doing the correct
//! post load sequence, includes fixups, postconstruct, 
//! autoconstructs.
//!
//------------------------------------------------------
void ObjectDatabase::DoPostLoadDefaults( DataObjectList& objectsToPost )
{
	LOAD_PROFILE( ObjectDatabase_DoPostLoadDefaults )

	// fix up
	DataObjectList::const_iterator fixUpIt = objectsToPost.begin();
	while( fixUpIt != objectsToPost.end() )
	{
		ObjectDatabase::Get().DoPostLoadFixups( *fixUpIt );
		++fixUpIt;
	}

	class FixupCallBack : public InterfaceCallBack
	{
	public:
		FixupCallBack( DataObjectList& DOList ) :
			m_DOList( DOList ){}
		bool CallBack( const DataObject* pParentDO, CallBackParameter param0 = NULL, CallBackParameter param1 = NULL )
		{
			DataObject* pNewDO = (DataObject*) param0.GetPtr();
			ObjectDatabase::Get().DoPostLoadFixups( pNewDO );
			m_DOList.push_back( pNewDO ); // to end of list so  defaultPost and post construct happen

			m_AutoConstruct.push_back( ntstd::pair<const DataObject*, CallBackParameter>(pParentDO, param1) ); // so autoconstruct are called post construct etc.
			return true;
		}
		virtual InterfaceCallBack* Clone()
		{
			return NT_NEW_CHUNK(Mem::MC_ODB) FixupCallBack( m_DOList );
		}
		DataObjectList& m_DOList;
		ntstd::List< ntstd::pair<const DataObject*, CallBackParameter>, Mem::MC_ODB > m_AutoConstruct;
	} FixupCallBackObj( objectsToPost );

	// post defaults
	{
		LOAD_PROFILE( PostDefaults )
		DataObjectList::const_iterator postIt = objectsToPost.begin();
	
		while( postIt != objectsToPost.end() )
		{
			PROFILE_TIMED_OPERATION_WARNING( PostDefaults, ( *postIt )->GetName().GetString(), 0.2f )

			// set defaults Post, objects can be auto constructed here
			StdDataInterface* pInterface = (StdDataInterface*)GetInterface( (*postIt) );
			StdDataInterface::iterator diIt = pInterface->begin();
			while( diIt != pInterface->end() )
			{
				(*diIt)->SetToDefaultPost( (*postIt), &FixupCallBackObj );
				++diIt;
			}
			++postIt;
		}
	}

	// post construct callback
	{
		LOAD_PROFILE( PostConstruct )

		DataObjectList::const_iterator postCallIt = objectsToPost.begin();
		while( postCallIt != objectsToPost.end() )
		{
			PROFILE_TIMED_OPERATION_WARNING( PostConstruct, ( *postCallIt )->GetName().GetString(), 0.2f )

			StdDataInterface* pInterface = ObjectDatabase::Get().GetInterface( *postCallIt );
			pInterface->CallBack( *postCallIt, "PostConstruct" );			
			++postCallIt;
		}
	}

	// auto construct callback
	{
		LOAD_PROFILE( AutoConstruct )

		ntstd::List< ntstd::pair<const DataObject*, CallBackParameter>, Mem::MC_ODB >::const_iterator autoCallIt = FixupCallBackObj.m_AutoConstruct.begin();
		while( autoCallIt != FixupCallBackObj.m_AutoConstruct.end() )
		{
			const DataObject* pParentDO = (*autoCallIt).first;
			PROFILE_TIMED_OPERATION_WARNING( AutoConstruct, pParentDO->GetName().GetString(), 0.2f )

			StdDataInterface* pInterface = ObjectDatabase::Get().GetInterface( pParentDO );
			DataInterfaceField* pDIF = pInterface->GetFieldByName( CHashedString((*autoCallIt).second) );
			pInterface->CallBack( pParentDO, "AutoConstruct", (char*) pDIF );
			++autoCallIt;
		}
	}

	// post post construct callback. When this is called all objects PostConstruct and auto construct have been called
	// which means for example that entities can look inside its parents attribute table
	{
		LOAD_PROFILE( PostPostConstruct )

		DataObjectList::const_iterator postpostCallIt = objectsToPost.begin();
		while( postpostCallIt != objectsToPost.end() )
		{
			PROFILE_TIMED_OPERATION_WARNING( PostPostConstruct, ( *postpostCallIt )->GetName().GetString(), 0.2f )

			StdDataInterface* pInterface = ObjectDatabase::Get().GetInterface( *postpostCallIt );
			pInterface->CallBack( *postpostCallIt, "PostPostConstruct" );
			++postpostCallIt;
		}
	}
}

//------------------------------------------------------
//!
//! Construct Object into a forward referenced dataobject
//! Used to make a non valid forward reference into a 
//! valid real Dataobject
//!
//------------------------------------------------------
void ObjectDatabase::ConstructObject( const char* pClassName, DataObject* pDO, bool bDoDefaults, bool bDoPostLoadDefaults )
{
	user_warn_p( !pDO->IsValid(), ("Trying to create an object with name %s, this name is already in use", ntStr::GetString(pDO->GetName()) ) );
	ntError_p( !pDO->IsValid(), ("Constructing in an already fully formed data object (Name %s) is bad", ntStr::GetString(pDO->GetName()) ) );
	// this way of setting class names is important as the object only store a pointer
	// the interface base naem ptr will last at least as long as the object
	DataInterface* val = GetInterface( pClassName );
	pDO->SetClassName( val->GetBaseObjectName().c_str() ); 
	ntError_p( val, ("Class '%s' does not exist\n",pClassName ) );
	pDO->Reset( val->CreateObject() );

	if( bDoDefaults )
	{
		// set defaults
		StdDataInterface* pInterface = (StdDataInterface*)val;
		StdDataInterface::iterator diIt = pInterface->begin();
		while( diIt != pInterface->end() )
		{
			(*diIt)->SetToDefault( pDO );
			++diIt;
		}
	}
	if( bDoPostLoadDefaults )
	{
		DataObjectList objectToPost;
		objectToPost.push_back( pDO );

		DoPostLoadDefaults( objectToPost );
	}

	m_BasePtrIndex[ pDO->GetBasePtr() ] = pDO;
	m_NameIndex[ pDO->GetName() ] = pDO;
	GetCurrentContainer()->AddObject( pDO );
	pDO->SetParent( GetCurrentContainer() );


}

//------------------------------------------------------
//!
//! Destroys the specific data object, if the data object
//! is a object container handles it correctly.
//! Afterward the objects and any sub-objects no longer
//! exist!
//!
//------------------------------------------------------
void ObjectDatabase::DestroyObject( DataObject* pUnknown )
{
	ObjectContainer* pParent = pUnknown->GetParent();

	// notify tracker
	if( m_pObjectTracker && pParent )
		m_pObjectTracker->ObjectDeleted( pUnknown->GetGUID(), GetDataObjectFromPointer(pParent)->GetGUID() );

	if( strcmp( pUnknown->GetClassName(), "ObjectContainer") == 0 )
	{
		ObjectContainer* pContainer = (ObjectContainer*) pUnknown->GetBasePtr();
		if ( !pParent ) {
			// must be the global container
			// no parent must be the global container
			ObjectDatabase::Get().UnRegisterObject( pUnknown );
			m_ContainerList.erase( ntstd::find( m_ContainerList.begin(), m_ContainerList.end(), pContainer ) );
		} else
		{
			// move the current container up if we are the current one
			if( pContainer == GetCurrentContainer() )
			{
				SetCurrentContainer( pParent );
			}

			pParent->RemoveObject( pUnknown );
			ObjectDatabase::Get().UnRegisterObject( pUnknown );
			m_ContainerList.erase( ntstd::find( m_ContainerList.begin(), m_ContainerList.end(), pContainer ) );
		}
	} else
	{
		ntAssert_p( pParent != 0, ("Object does not exist in the object database") );
		pParent->RemoveObject( pUnknown );
		ObjectDatabase::Get().UnRegisterObject( pUnknown );
	}
}

void ObjectDatabase::ReparentObject( DataObject* pDO, ObjectContainer *pNewParent )
{
	ObjectContainer *pOriginalParent = pDO->GetParent();
	if (pOriginalParent != 0)
	{
		pOriginalParent->RemoveObject( pDO );
	}
	pDO->SetParent( pNewParent );
	pNewParent->AddObject( pDO );
}

//------------------------------------------------------
//!
//! Internal bit of the destruction system
//!
//------------------------------------------------------
void ObjectDatabase::UnRegisterObject( DataObject* object )
{
	// ntAssert the object exists in the database
	GUIDIndex::iterator odmIt = m_GUIDIndex.find( object->GetGUID() );
	ntError_p( odmIt != m_GUIDIndex.end(), ("Attempt to unregister an object that isn't in the database") );
	m_GUIDIndex.erase( odmIt );

	// safety check
	if( strcmp( object->GetClassName(), "UnknownClass") == 0 )
	{
		char buf[128];
		GuidClient::Get().LookupGuid( object->GetGUID().GetAsString().c_str(), buf, 128 );
		ntError_p( 0, ("ATTN! Missing Object Referenced As %s. FIX IT CHUMP!", buf ) );
	}
	// in case this object never got completely fixedup
	if( object->GetFixupList() != 0 )
	{
		// is this really a benign thing? surely this is caught in the forward refernce ntError?
		char buf[128];
		GuidClient::Get().LookupGuid( object->GetGUID().GetAsString().c_str(), buf, 128 );
		ntPrintf( "Still has fixup list (unused forward reference?): %s %s\n", ntStr::GetString(object->GetName()), buf );
		object->DeleteFixupList();
	}
	
	// now also remove the entry from the GUID Client (not earlier, as it's still being used!)
	GuidClient::Get().Remove(object->GetGUID().GetAsString().c_str());

	StdDataInterface* pInterface = GetInterface( object->GetClassName() );

	// unset defaults this is used by soft interfaces to undo placement news that have occured
	StdDataInterface::iterator diIt = pInterface->begin();
	while( diIt != pInterface->end() )
	{
		(*diIt)->UnSetFromDefault( object );
		++diIt;
	}

	pInterface->DestroyObject( object->GetBasePtr() );

	// now remove the pointer this is not fatal (some objects don't have pointers)
	BasePtrIndex::iterator bpmIt = m_BasePtrIndex.find( object->GetBasePtr() );
	if( bpmIt != m_BasePtrIndex.end() )
	{
		m_BasePtrIndex.erase( bpmIt );
	}
	// now remove the name this is not fatal (some objects don't have names)
	NameIndex::iterator nameIt = m_NameIndex.find( object->GetName() );
	if( nameIt != m_NameIndex.end() )
	{
		m_NameIndex.erase( nameIt );
	}
	

	NT_DELETE_CHUNK( Mem::MC_ODB, object );
}

//------------------------------------------------------
//!
//! Given a class name return a standard data interface
//! asserts on unknown interface name
//!
//------------------------------------------------------
StdDataInterface* ObjectDatabase::GetInterface(  const char* pClassName ) const
{

	StdDataInterface* val = GetInterfaceUnsafe( pClassName );
	ntError_p( val, ("Interface '%s' does not exist\n",pClassName ) );
	return val;
}

//------------------------------------------------------
//!
//! Given a class name return a standard data interface
//! 0 if the interface doesn't exist
//!
//------------------------------------------------------
StdDataInterface* ObjectDatabase::GetInterfaceUnsafe(  const char* pClassName ) const
{
	char buffer[256];
	strcpy( buffer, pClassName );
	strcat( buffer, "Interface" );
	unsigned int iClassHash = ClassFactory::GenerateHash(buffer);
	HashToIndexMap::const_iterator it = m_HashToIndex.find( iClassHash );

	DataInterface* val = 0;
	if( it != m_HashToIndex.end() )
	{
		const ClassFactory::ClassTable& clas = ClassFactory::m_ClassConstructors[it->second];
		val = clas.m_CreationHelper->GetInterface();
	}

	return (StdDataInterface*)( val );
}

//------------------------------------------------------
//!
//! Gets an object by GUID, if the object doesn't exist
//! creates a forward reference and returns that.
//! forward reference dataobject are special, they are
//! in a zombie state, half alive, half dead.
//!
//------------------------------------------------------
DataObject* ObjectDatabase::GetObjectOrForwardReference( const GameGUID& guid )
{
	DataObject* pDO = GetDataObjectFromGUID( guid );
	if( pDO == 0 )
	{
		CStruct<UnknownClass>* obj = NT_NEW_CHUNK(Mem::MC_ODB) CStruct<UnknownClass>( guid );
		obj->CreateFixupList();
		m_GUIDIndex[ obj->GetGUID() ] = obj;
		return obj;
	} else
	{
		return pDO;
	}
}

namespace
{
	//------------------------------------------------------
	//!
	//! Private little functor for ntstd::transform to fix
	//! XML crappiness
	//!
	//------------------------------------------------------
	struct LocalStringTransform
	{
		char operator()(char c)
		{
			switch( c )
			{
			case '<': // transfrom < to !
				return '!';
			case '>': // transfrom > to !
				return '!';
			case '\"': // transform " to ~
				return '~';

			default: // leave alone
				return c;
			}
		}
	} ;
	//------------------------------------------------------
	//!
	//! Private little function to invert to above
	//! XML crappiness fix
	//!
	//------------------------------------------------------

	void UnTransformString( ntstd::String& str )
	{
		bool bFirst = true;
		for(unsigned int i=0;i < str.size();i++)
		{
			char c = str[i];
			switch( c )
			{
			case '!':
				if( bFirst )
				{
					str[i] = '<';
					bFirst = false;
				} else
				{
					str[i] = '>';
					bFirst = true;
				}
				break;
			case '~':
				str[i] = '\"';
				break;
			default: // do nothing
				{
				}
			}
		}
	}

}

//------------------------------------------------------
//!
//! Saves the current container to the data stream.
//!
//------------------------------------------------------
void ObjectDatabase::SaveCurrentContainer( CSimpleStream* pStream, const ntstd::String& containerName )
{
	SaveDataObject( GetDataObjectFromPointer( GetCurrentContainer() ), pStream, containerName );
}

//------------------------------------------------------
//!
//! Save a data object to a stream, 
//!
//------------------------------------------------------
void ObjectDatabase::SaveDataObject( DataObject* pDOrig, CSimpleStream* pStream, const ntstd::String& containerName, bool bRecurse,bool bWelderClip)
{
	bool bOrigIsContainer = false;
	if( strcmp(pDOrig->GetClassName(), "ObjectContainer") == 0 && bWelderClip == false )
	{
		ObjectContainer* pContainer = (ObjectContainer*) pDOrig->GetBasePtr();
		pContainer->m_FileName = containerName;
		bOrigIsContainer = true;
	}

	typedef ntstd::List<GameGUID> GuidList;
	GuidList listContainersToSave;
	listContainersToSave.push_back( pDOrig->GetGUID() );

	if( bRecurse )
	{
		// first build a list of object to save
		GuidList::const_iterator guIt = listContainersToSave.begin();
		while( guIt != listContainersToSave.end() )
		{
			GameGUID guid = (*guIt); 

			DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromGUID( guid );
			if( strcmp( pDO->GetClassName(), "UnknownClass") == 0 )
			{
				char buf[128];
				GuidClient::Get().LookupGuid( pDO->GetGUID().GetAsString().c_str(), buf, 128 );
				ntError_p( 0, ("ATTN! Missing Object Referenced As %s. FIX IT CHUMP!", buf ) );
				++guIt;
				continue;
			}
			StdDataInterface* pInterface = ObjectDatabase::Get().GetInterface( pDO->GetClassName() );
			StdDataInterface::iterator diIt = pInterface->begin();
			while( diIt != pInterface->end() )
			{
				ntstd::String metadata = (*diIt)->GetMetaData();
				ntstd::String var = (*diIt)->GetData( pDO );

				if( metadata == typeof_name( ntstd::List<DataObject*>() ) )
				{
					// add a list of containers we need to recurse into
					ntstd::Istringstream in_stream(var);
					unsigned int listSize;
					in_stream >> listSize;

					for( unsigned int i=0;i < listSize;i++)
					{
						GameGUID guid;
						in_stream >> guid; // convert back to guid
						// check to see if valid to be saved
						if( !guid.IsNull() && ntstd::find( listContainersToSave.begin(), listContainersToSave.end(), guid ) == listContainersToSave.end() )
						{
							// is this a container?
							DataObject* pNewDO = GetDataObjectFromGUID( guid );
							if( strcmp(pNewDO->GetClassName(), "ObjectContainer" ) == 0 )
							{
								listContainersToSave.push_back( guid );
							}
						}
					}
				}

				++diIt;
			}
			++guIt;
		}
	}

	// next go through list saving any object in the containers
	GuidList::const_iterator guIt = listContainersToSave.begin();
	while( guIt != listContainersToSave.end() )
	{
		GameGUID guid = (*guIt);
		DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromGUID( guid );

		ntAssert_p( (strcmp( pDO->GetClassName(), "UnknownClass") != 0), ("Internal ntError trying to save a container with UnknownClass?!?!") );

		// special case if we want to save a non container, but usually only extract stuff from containers
		if( strcmp( pDO->GetClassName(), "ObjectContainer") == 0 || bOrigIsContainer == false )
		{
			ntstd::Ostringstream beginStr;
			beginStr << "<Container name=\"";
			if( bOrigIsContainer )
			{
				ObjectContainer* pOC = (ObjectContainer*) pDO->GetBasePtr();
				if( pOC->m_FileName.empty() )
				{
					beginStr << ntStr::GetString(pDO->GetName());
				} else
				{
					 beginStr << ntStr::GetString(pOC->m_FileName);
				}
			}

			beginStr << "\" guid=\"" << pDO->GetGUID() << "\" version=\"1.0\">\n";
			(*pStream) << beginStr.str().c_str();

			GuidList listObjectsToSave;

			// if we are saving a non container object make sure it gets saved
			if( bOrigIsContainer == false )
			{
				listObjectsToSave.push_back( pDOrig->GetGUID() );
			}


			// extract list of contained objects
			StdDataInterface* pContainerInterface = ObjectDatabase::Get().GetInterface( pDO->GetClassName() );
			StdDataInterface::iterator diIt = pContainerInterface->begin();
			while( diIt != pContainerInterface->end() )
			{
				ntstd::String metadata = (*diIt)->GetMetaData();
				ntstd::String var = (*diIt)->GetData( pDO );

				if( metadata == typeof_name( ntstd::List<DataObject*>() ) )
				{
					// add a list of containers we need to recurse into
					ntstd::Istringstream in_stream(var);
					unsigned int listSize;
					in_stream >> listSize;

					for( unsigned int i=0;i < listSize;i++)
					{
						GameGUID coguid;
						in_stream >> coguid; // convert back to guid
						// check to see if valid to be saved
						if( !coguid.IsNull() && ntstd::find( listObjectsToSave.begin(), listObjectsToSave.end(), coguid ) == listObjectsToSave.end() )
						{
							listObjectsToSave.push_back( coguid );
						}
					}
				}
				++diIt;
			}

			// save all objects that belong in this container
			GuidList::const_iterator obIt = listObjectsToSave.begin();
			while( obIt != listObjectsToSave.end() )
			{
				// save each object
				GameGUID coguid = (*obIt);
				DataObject* pNewDO = GetDataObjectFromGUID( coguid );
				StdDataInterface* pNewInterface = ObjectDatabase::Get().GetInterface( pNewDO->GetClassName() );

				if( strcmp( pNewDO->GetClassName(), "UnknownClass") == 0 )
				{
					char buf[128];
					GuidClient::Get().LookupGuid( pNewDO->GetGUID().GetAsString().c_str(), buf, 128 );
					ntError_p( 0, ("ATTN! Missing Object Referenced As %s. FIX IT CHUMP!", buf ) );
					++obIt;
					continue;
				}

#if defined( BREAK_ON_BASENAME)
				if( pNewDO->GetName() == BREAK_ON_BASENAME )
				{
					DebugBreakNow();

				}
#endif
				ntstd::Ostringstream dataStr;
				dataStr << "\t<Base " 
						<< "name=\"" << pNewDO->GetName() << "\" "
						<< "guid=\"" << pNewDO->GetGUID() << "\" " 
						<< "type=\"" << pNewDO->GetClassName() << "\"" << ">\n";

				StdDataInterface::iterator diIt = pNewInterface->begin();
				while( diIt != pNewInterface->end() )
				{
					LocalStringTransform m_Transform;
					ntstd::String metadata = (*diIt)->GetMetaData();
					ntstd::String name = ntStr::GetString((*diIt)->GetName());
					ntstd::String var = (*diIt)->GetData( pNewDO );

					ntstd::transform( metadata.begin(), metadata.end(), metadata.begin(), m_Transform );
					ntstd::transform( name.begin(), name.end(), name.begin(), m_Transform );
					ntstd::transform( var.begin(), var.end(), var.begin(), m_Transform );

					dataStr << "\t\t<Item "
							<< "name=\"" << name << "\" " 
							<< "type=\"" << metadata <<"\" "
							<< "value=\"" << var << "\""
							<< "/>\n";

					++diIt;
				}
				dataStr << "\t</Base>\n";
				(*pStream) << dataStr.str().c_str();
				++obIt;
			}

			// is this the first container? is so put the container tag after all containters
			if( guIt != listContainersToSave.begin() )
			{
				ntstd::Ostringstream endStr;
				endStr << "</Container>\n";
				(*pStream) << endStr.str().c_str();
			}
		}

		// next container
		++guIt;
	}

	// close the first (file) container
	ntstd::Ostringstream endStr;
	endStr << "</Container>\n";
	(*pStream) << endStr.str().c_str();

}

//------------------------------------------------------
//!
//! Save all the registered interfaces 
//!
//------------------------------------------------------
void ObjectDatabase::SaveAllInterfaces( CSimpleStream* pStream )
{
	// first lets do global enums
	GlobalEnumIndex::const_iterator enumIt = m_GlobalEnumIndex.begin();
	while( enumIt != m_GlobalEnumIndex.end() )
	{
		const ntstd::String& enumName = enumIt->first;
		// save this interface
		ntstd::Ostringstream dataStrA;
		dataStrA << "\t<Interface " 
				<< "name=\"" << enumName << "\" "
				<< "guid=\"" << DataObject::GetObjectGUIDFromName( enumName ) << "\" " 
				<< ">\n";
		(*pStream) << dataStrA.str().c_str();

		GlobalEnum::const_iterator itemIt = enumIt->second.begin();
		while( itemIt != enumIt->second.end() )
		{
			ntstd::Ostringstream dataStrB;
			dataStrB << "\t\t<Item "
					<< "name=\"" << itemIt->second << "\" " 
					<< "type=\"enum\" "
					<< "value=\"" << itemIt->first << "\""
					<< "/>\n";
			(*pStream) << dataStrB.str().c_str();

			++itemIt;
		}


		ntstd::Ostringstream dataStrC;
		dataStrC << "\t</Interface>\n";
		(*pStream) << dataStrC.str().c_str();

		++enumIt;
	}

	HashToIndexMap::const_iterator inIt = m_HashToIndex.begin();
	while( inIt != m_HashToIndex.end() )
	{
		const ClassFactory::ClassTable& clas = ClassFactory::m_ClassConstructors[inIt->second];
		DataInterface* pInterface = clas.m_CreationHelper->GetInterface();

		// save this interface
		ntstd::Ostringstream dataStrA;
		dataStrA << "\t<Interface " 
				<< "name=\"" << pInterface->GetBaseObjectName() << "\" "
				<< "guid=\"" << pInterface->GetGUID() << "\" " 
				<< ">\n";
		(*pStream) << dataStrA.str().c_str();

		DataInterface::iterator diIt = pInterface->begin();
		while( diIt != pInterface->end() )
		{
			LocalStringTransform m_Transform;
			ntstd::String metadata = (*diIt)->GetMetaData();
			ntstd::String name = ntStr::GetString((*diIt)->GetName());
			ntstd::String var = (*diIt)->GetDefault();

			ntstd::transform( metadata.begin(), metadata.end(), metadata.begin(), m_Transform );
			ntstd::transform( name.begin(), name.end(), name.begin(), m_Transform );
			ntstd::transform( var.begin(), var.end(), var.begin(), m_Transform );

			ntstd::Ostringstream dataStrB;
			dataStrB << "\t\t<Item "
					<< "name=\"" << name << "\" " 
					<< "type=\"" << metadata <<"\" "
					<< "value=\"" << var << "\""
					<< "/>\n";
			(*pStream) << dataStrB.str().c_str();
			++diIt;
		}

		// Add the cast-to types as items at the end of the interface, if there is at least one cast-to name.
		DataInterface::CastToContainerConstIterator castIt = pInterface->CastToNamesBegin();
		if (castIt != pInterface->CastToNamesEnd())
		{
			ntstd::Ostringstream dataStrC;
			LocalStringTransform transformB;
			ntstd::String type = typeof_name( TypeOfOdbStringList() );

			ntstd::transform( type.begin(), type.end(), type.begin(), transformB );
			dataStrC << "\t\t<Item name=\"CastToNames\" type=\"" << type.c_str() << "\" value=\"";
			dataStrC << (*castIt).c_str();
			++castIt;
			while ( castIt != pInterface->CastToNamesEnd() )
			{
				dataStrC << " ";
				dataStrC << (*castIt).c_str();
				++castIt;
			}
			dataStrC << "\"/>\n";
			(*pStream) << dataStrC.str().c_str();
		}

		ntstd::Ostringstream dataStrD;
		dataStrD << "\t</Interface>\n";
		(*pStream) << dataStrD.str().c_str();
		++inIt;
	}

}


namespace 
{

//------------------------------------------------------
//!
//! XPat callback stuff
//!
//------------------------------------------------------
struct XPatContainer
{
	XPatContainer() :
		m_pDO(0),
		m_pInterface(0),
		m_bInInterface(false),
		m_pSoftInterface(0),
		m_fVersion(0),
		m_bIgnoreBase(false),
		m_bIsABase(false)
	{
	}

	ObjectContainer*	m_pContainer;

	DataObject*			m_pDO;
	ntstd::String			m_sLastItemName;

	StdDataInterface*	m_pInterface;
	bool				m_bInInterface;
	SoftObjectFactoryInterface*	m_pSoftInterface;

	// compat list handling
	struct BaseNode
	{
		BaseNode() : m_pDO(0) {}
		BaseNode( DataObject* pDO, const ntstd::String& str ) :
			m_pDO(pDO), m_sItemName(str) {};

		DataObject*			m_pDO;
		ntstd::String			m_sItemName;
	};
	typedef ntstd::Vector<BaseNode, Mem::MC_ODB> BaseNodeVector;
	BaseNodeVector		m_BaseStack;
	float				m_fVersion;

	struct XPatUserData* m_Parent;

	bool				m_bIgnoreBase;
	bool				m_bIsABase;
};

	
// Our parser user data
struct XPatUserData
{
	XPatUserData() : m_iBaseCount(0) {};

	typedef ntstd::Vector<XPatContainer, Mem::MC_ODB> XPatContainerList;
	XPatContainerList m_ContainerStack;

	ntstd::String		m_FileName;
	XML_Parser			m_Parser;
	XPatContainer*		m_CurrentContainer;

	typedef ntstd::List<DataObject*, Mem::MC_ODB> DataObjectList;
	DataObjectList		m_ObjectsToPost;
	u_int				m_iBaseCount;
};


void ContainerElementHandler( XPatUserData* pData, const char **attr )
{
	ntstd::String objectName;
	GameGUID gameGUID;
	float fVersion = 0.0f;
	while (*attr)
	{
		ntstd::String obName(*attr);
		attr++;

		if( obName == "name" )
		{
			objectName = *attr;
		} else 
		if( obName == "guid" )
		{
			gameGUID.SetFromString( *attr );
		} else
		if( obName == "version" )
		{
			ntstd::Istringstream data( *attr );
			data >> fVersion;
		}
		attr++;
	}
#if defined(BREAK_ON_CONTAINERNAME)
	if( objectName == BREAK_ON_CONTAINERNAME )
	{
		DebugBreakNow();
	}
#endif

	if( pData->m_ContainerStack.empty() )
	{
		objectName = pData->m_FileName;
	}

	ntAssert_p( ObjectDatabase::Get().IsValidContainerName(objectName), ("Container %s has invalid characters", objectName.c_str()) );

	// add it
	pData->m_ContainerStack.push_back( XPatContainer() );
	pData->m_CurrentContainer = &pData->m_ContainerStack.back();
	pData->m_CurrentContainer->m_Parent = pData;

	// see if this container already exists
	DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromName( ntStr::GetString(objectName) );
	if( pDO == 0 )
	{
		pData->m_CurrentContainer->m_pContainer = ObjectDatabase::Get().AddContainer( objectName.c_str(), gameGUID );
		if ( !gameGUID.IsNull() ) {
			// find the wrapping DataObject again to put it in the "to post" list
			DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromGUID( gameGUID );
			pData->m_ObjectsToPost.push_back( pDO );
		}
	} else
	{
		pData->m_CurrentContainer->m_pContainer = (ObjectContainer*) pDO->GetBasePtr();
/*
		if( fVersion > 0.0f )
		{
			pData->m_CurrentContainer->m_pContainer = (ObjectContainer*) pDO->GetBasePtr();
		}
		else
		{
			ntError_p( false, ("Duplicate container names are now longer allowed %s", objectName.c_str()) );
		}
*/
	}

	ObjectDatabase::Get().SetCurrentContainer( pData->m_CurrentContainer->m_pContainer );
	pData->m_CurrentContainer->m_fVersion = fVersion;
	if( objectName.size() > 4 )
	{
		ntstd::String ext = &objectName[ objectName.size() -4 ];
		if( ext == ".xml" )
		{
			pData->m_CurrentContainer->m_pContainer->m_FileName = objectName;
		}
	}
}

void BaseElementHandler( XPatUserData* pUserData, const char **origattr )
{
	XPatContainer* pData = pUserData->m_CurrentContainer;

	ntstd::String objectName;
	GameGUID gameGUID;
	ntstd::String className;

	const char **attr = origattr;
	while (*attr)
	{
		ntstd::String obName(*attr);
		attr++;

		if( obName == "name" )
		{
			objectName = *attr;
		} else 
		if( obName == "guid" )
		{
			gameGUID.SetFromString( *attr );
		} else
		if( obName == "type" )
		{
			className = *attr;
		}
		attr++;
	}
#if defined(BREAK_ON_BASENAME)
	if( objectName == ntstd::String(BREAK_ON_BASENAME) )
	{
//		DebugBreakNow();

		int a = 0;
		UNUSED(a);
	}
#endif

	ntAssert_p( objectName.empty() || ObjectDatabase::Get().IsValidObjectName(objectName), ("Base %s has invalid characters", objectName.c_str()) );
	ntAssert_p( className != "", ("Base %s has no type attribute", objectName.c_str() ) );

	if( className == ntstd::String("ObjectContainer") )
	{
		ContainerElementHandler( pUserData, origattr );
		pUserData->m_CurrentContainer->m_bIgnoreBase = true;
		pUserData->m_CurrentContainer->m_bIsABase = true;
		return;
	}
	
	pUserData->m_iBaseCount++;

	// create a new object
	if( pData->m_fVersion < 1.0f && pData->m_pDO != 0) // old XML version
	{
		const ntstd::String& containerName = pData->m_sLastItemName;
		CHashedString containerHash(ntStr::GetString(containerName));

		// check this is a list type (they are implemented in the old system as bases within bases)
		DataInterfaceField* pDataField = pData->m_pInterface->GetFieldByName( containerHash );
		if( pDataField && pDataField->GetFieldClass() == "container" )
		{
			pData->m_BaseStack.push_back( XPatContainer::BaseNode( pData->m_pDO, containerName ) );

			pData->m_pInterface = 0;
			pData->m_pDO = 0;
		} else
		{
			ntAssert_p( false, ("Compatibilty ntError ntError") );
		}
	} else
	{
		ntAssert_p( pData->m_pDO == 0, ("Base inside a base object") );
	}

	if( gameGUID.IsNull() )
	{
		if( objectName.empty() )
		{
			objectName = DataObject::GetUniqueName();
		}

		gameGUID = DataObject::GetObjectGUIDFromName( objectName );
		ntError_p( !gameGUID.IsNull(), ("Cannot obtain GUID for this base") );
	}

	// safely allow interfaces to be deleted, the XML structure will be ignored
	if( ObjectDatabase::Get().GetInterfaceUnsafe( className.c_str() ) == 0)
	{
		ntPrintf( "The class %s does not exist, the object will be ignored\n", className.c_str() );
		pData->m_bIgnoreBase = true;
		return;
	}


	// objects are constructed to the current container
	pData->m_pDO = ObjectDatabase::Get().GetObjectOrForwardReference( gameGUID );
	pData->m_pDO->SetName( ntStr::GetString(objectName) );
	ObjectDatabase::Get().ConstructObject( className.c_str(), pData->m_pDO , true, false );
	pData->m_pInterface = ObjectDatabase::Get().GetInterface( pData->m_pDO->GetClassName() );

}




void ItemElementHandler( XPatUserData* pUserData, const char **attr )
{
	XPatContainer* pData = pUserData->m_CurrentContainer;

	if(pData->m_bIgnoreBase )
	{
		return;
	}

	if( !pData->m_bInInterface )
	{
		// create a new object
		ntAssert_p( pData->m_pDO != 0, ("Item without a base object") );
	}

	ntstd::String name("");
	ntstd::String type("");
	ntstd::String value("");

	while (*attr)
	{
		ntstd::String obName(*attr);
		attr++;

		if( obName == "name" )
		{
			name = *attr;
		} else
		if( obName == "type" )
		{
			type = *attr;
			UnTransformString( type );
		} else 
		if( obName == "value" )
		{
			value = *attr;
		}
		attr++;
	}

	if( pData->m_fVersion < 1.0f )
	{
		// record the last item name for backwards compat
		pData->m_sLastItemName = name;

		if( type == "vector" )
		{
			type = "point";
		} else if( type == "reference" )
		{
			type = "void*";
		} else if( type == "list" )
		{
			type = "std::list<void*>";
		}
	}

	CHashedString	hashName(name);

	// in an interface a item is a new soft property (value is the default
	if( pData->m_bInInterface )
	{
		pData->m_pSoftInterface->AddProperty( hashName, type, value );
	} else
	{
		// in a base its a new data value
		pData->m_pInterface->SetData( pData->m_pDO, hashName, value );
	}

}
void InterfaceElementHandler( XPatUserData* pUserData, const char **attr )
{
	XPatContainer* pData = pUserData->m_CurrentContainer;

	// create a new object
	ntAssert_p( !pData->m_bInInterface && pData->m_pSoftInterface == 0, ("Interface inside an interface") );

	ntstd::String name("");
	GameGUID gameGUID;
	ntstd::String type("");
	ntstd::String value("");
	ntstd::String mredonly("");

	while (*attr)
	{
		ntstd::String obName(*attr);
		attr++;

		if( obName == "name" )
		{
			name = *attr;
		} else
		if( obName == "guid" )
		{
			gameGUID.SetFromString( *attr );
		} else
		if( obName == "type" )
		{
			type = *attr;
			UnTransformString( type );
		} else 
		if( obName == "value" )
		{
			value = *attr;
		}
		if( obName == "mredonly" )
		{
			mredonly = *attr;
		}
		attr++;
	}
#if defined(BREAK_ON_INTERFACENAME)
	if( name == BREAK_ON_INTERFACENAME)
	{
		DebugBreakNow();
	}
#endif

	pData->m_bInInterface = true;
	if( !mredonly.empty() )
	{
		// currently any mred tag
		pData->m_bIgnoreBase = true;
		return;
	}

	// see if this interface is already defined 
	StdDataInterface *pTestInterface = ObjectDatabase::Get().GetInterfaceUnsafe( name.c_str() );
	UNUSED(pTestInterface); // All used for assertations
	ntError_p( pTestInterface == 0, ("Interface %s definition already exists", name.c_str()) );

	// get the soft object factory factorys interface
	SoftObjectFactoryInterface* pFactoryFactoryInterface = 
		(SoftObjectFactoryInterface*) ObjectDatabase::Get().GetInterface( "SoftObjectFactory" );
	ntAssert_p( pFactoryFactoryInterface != 0, ("No Soft Object Interface") );

	// create a factory for this type of soft interface
	pData->m_pSoftInterface = pFactoryFactoryInterface->CreateFactoryFor( name, type );
}


void StartElementHandler(void *pUserData, const char *el, const char **attr)
{
	XPatUserData* pData = (XPatUserData*) pUserData;
	if( ntstd::String(el) == "Container" )
	{
		START_LOAD_TIME_PROFILER_ACC( 20 );
		ContainerElementHandler( pData, attr );
		STOP_LOAD_TIME_PROFILER_ACC( 20 );
		return;
	} else
	if( ntstd::String(el) == "Base" )
	{
		START_LOAD_TIME_PROFILER_ACC( 21 );
		BaseElementHandler( pData, attr );
		STOP_LOAD_TIME_PROFILER_ACC( 21 );
		return;
	} else
	if( ntstd::String(el) == "Item" )
	{
		START_LOAD_TIME_PROFILER_ACC( 22 );
		ItemElementHandler( pData, attr );
		STOP_LOAD_TIME_PROFILER_ACC( 22 );
		return;
	} else
	if( ntstd::String(el) == "Interface" )
	{
		START_LOAD_TIME_PROFILER_ACC( 23 );
		InterfaceElementHandler( pData, attr );
		STOP_LOAD_TIME_PROFILER_ACC( 23 );
		return;
	}
}

void EndContainerElementHandler( XPatUserData* pUserData )
{
//	XPatContainer* pData = pUserData->m_CurrentContainer;
	pUserData->m_ContainerStack.pop_back();
	if( pUserData->m_ContainerStack.empty() )
	{
		pUserData->m_CurrentContainer = 0;
	} else
	{
		pUserData->m_CurrentContainer = &pUserData->m_ContainerStack.back();
		ObjectDatabase::Get().SetCurrentContainer( pUserData->m_CurrentContainer->m_pContainer );
	}
	return;
}
void EndBaseElementHandler( XPatUserData* pUserData )
{
	XPatContainer* pData = pUserData->m_CurrentContainer;

	if( pData->m_bIgnoreBase == true )
	{
		pData->m_bIgnoreBase = false;
	} else
	{
		ntAssert_p( pData->m_pDO != 0, ("End Base without base object") );
		ntAssert_p( pData->m_bInInterface == false, ("Interface in a base object") );
		pData->m_Parent->m_ObjectsToPost.push_back( pData->m_pDO );
	}
	if( pData->m_fVersion < 1.0f && !pData->m_BaseStack.empty() ) // old XML version
	{
		XPatContainer::BaseNode& node = pData->m_BaseStack.back();
		
		StdDataInterface* pInterface = ObjectDatabase::Get().GetInterface( node.m_pDO );
		ntstd::Ostringstream str;
		str << "A " << pData->m_pDO->GetGUID();
		pInterface->EditData( node.m_pDO, ntStr::GetString(node.m_sItemName), str.str() );

		pData->m_pInterface = pInterface;
		pData->m_pDO = node.m_pDO;
		pData->m_sLastItemName = node.m_sItemName;
		pData->m_BaseStack.pop_back();
	} else
	{
		pData->m_pDO = 0;
		pData->m_pInterface = 0;
	}
	return;
}
void EndElementHandler(void *pvUserData, const char *el)
{
	START_LOAD_TIME_PROFILER_ACC( 11 );

	XPatUserData* pUserData = (XPatUserData*) pvUserData;
	XPatContainer* pData = pUserData->m_CurrentContainer;

	if( ntstd::String(el) == "Container" )
	{
		EndContainerElementHandler( pUserData );
	}
	if( ntstd::String(el) == "Base" )
	{
		if( pData->m_bIsABase == true )
		{
			EndContainerElementHandler( pUserData );
		} else
		{
			EndBaseElementHandler( pUserData );
		}
	}
	if( ntstd::String(el) == "Interface" )
	{
		ntAssert_p( pData->m_bInInterface == true, ("End Interface without interface object") );
		pData->m_bInInterface = false;
		if( pData->m_bIgnoreBase == true )
		{
			pData->m_bIgnoreBase = false;
		} else
		{
			pData->m_pSoftInterface->CreateFastIndices();
			pData->m_pSoftInterface = 0;
		}
	}

	STOP_LOAD_TIME_PROFILER_ACC( 11 );

}

void CommentHandler( void* pUserData, const char* pcComment )
{
	UNUSED(pUserData);
	UNUSED(pcComment);
}

}
bool ObjectDatabase::LoadDataObject( FileBuffer* pFile, const ntstd::String& fileName )
{
	LOAD_PROFILE( ObjectDatabase_LoadDataObject )

	RESET_LOAD_TIME_PROFILER_ACC( 1 );
	RESET_LOAD_TIME_PROFILER_ACC( 2 );
	RESET_LOAD_TIME_PROFILER_ACC( 3 );
	RESET_LOAD_TIME_PROFILER_ACC( 4 );
	RESET_LOAD_TIME_PROFILER_ACC( 5 );
	RESET_LOAD_TIME_PROFILER_ACC( 6 );
	RESET_LOAD_TIME_PROFILER_ACC( 7 );
	RESET_LOAD_TIME_PROFILER_ACC( 8 );
	RESET_LOAD_TIME_PROFILER_ACC( 9 );
	RESET_LOAD_TIME_PROFILER_ACC( 10);
	RESET_LOAD_TIME_PROFILER_ACC( 11 );

	RESET_LOAD_TIME_PROFILER_ACC( 20 );
	RESET_LOAD_TIME_PROFILER_ACC( 21 );
	RESET_LOAD_TIME_PROFILER_ACC( 22 );
	RESET_LOAD_TIME_PROFILER_ACC( 23 );

	RESET_LOAD_TIME_PROFILER_ACC( 70 );
	RESET_LOAD_TIME_PROFILER_ACC( 71 );
	RESET_LOAD_TIME_PROFILER_ACC( 72 );
	RESET_LOAD_TIME_PROFILER_ACC( 73 );
	RESET_LOAD_TIME_PROFILER_ACC( 74);

	RESET_LOAD_TIME_PROFILER_ACC( 700);



	ObjectContainer* pOldCurrent = 0;

	pOldCurrent = GetCurrentContainer();

	// Create an expat parser
	XML_Parser obParser = XML_ParserCreate( 0 );
	// Make sure that that worked
	ntAssert_p( obParser, ( "I couldn't create an expat parser" ) );

	// Create a struct to manage this parse job
	struct XPatUserData strParseInfo;
	// Set the parser in the structure
	strParseInfo.m_Parser = obParser;
	strParseInfo.m_FileName = fileName;

	// Set a pointer so the handlers can see this information
	XML_SetUserData( obParser, &strParseInfo );

	// Set the handlers for starting and ending tags
	// These will deal with the element names and attribute pairs
	XML_SetElementHandler( obParser, StartElementHandler, EndElementHandler );

	// A call to handle the file's comments 
	XML_SetCommentHandler( obParser, CommentHandler );

	// Now get on and do some parsing action
	{
		LOAD_PROFILE( ParseXML )

		if ( !XML_Parse( obParser, **pFile, pFile->GetSize(), 1 ) )
		{
			// Show us the ntError
			user_warn_msg( ("Parse ntError at line %d:\n%s (see log for filename)\n", XML_GetCurrentLineNumber( obParser ), XML_ErrorString( XML_GetErrorCode( obParser ) ) ) );

			// try and fix up stuff up to where we got...
			DoPostLoadDefaults( strParseInfo.m_ObjectsToPost );

			// Clean up our mess
			XML_ParserFree( obParser );

			if( pOldCurrent != 0 )
			{
				SetCurrentContainer( pOldCurrent );
			}

			PRINT_LOAD_TIME_PROFILER_ACC( DataInterface_SetData_Helper, 1 );
			PRINT_LOAD_TIME_PROFILER_ACC( StdDataInterface_SetData_Helper, 2 );
			PRINT_LOAD_TIME_PROFILER_ACC( PointerContainerDIF_SetData, 3 );
			PRINT_LOAD_TIME_PROFILER_ACC( SingleItemDIFHelper_SetData, 4 );
			PRINT_LOAD_TIME_PROFILER_ACC( SingleItemDIF_bool_SetData, 5 );
			PRINT_LOAD_TIME_PROFILER_ACC( SingleItemDIF_CFlipFlop_SetData, 6 );
			PRINT_LOAD_TIME_PROFILER_ACC( SinglePointerDIF_SetData, 7 );
			PRINT_LOAD_TIME_PROFILER_ACC( StdContainerDIF_SetData, 8 );
			PRINT_LOAD_TIME_PROFILER_ACC( StdEnumDIF_SetData, 9 );
			PRINT_LOAD_TIME_PROFILER_ACC( DataObjectContainerDIF_SetData, 10);
			PRINT_LOAD_TIME_PROFILER_ACC( StartElementHandler_Container, 20 );
			PRINT_LOAD_TIME_PROFILER_ACC( StartElementHandler_Base, 21 );
			PRINT_LOAD_TIME_PROFILER_ACC( StartElementHandler_Item, 22 );
			PRINT_LOAD_TIME_PROFILER_ACC( StartElementHandler_Interface, 23 );

			PRINT_LOAD_TIME_PROFILER_ACC( SinglePointerDIF_SetData_GUID, 70);
			PRINT_LOAD_TIME_PROFILER_ACC( SinglePointerDIF_SetData_GetObjectOrForwardReference, 71);
			PRINT_LOAD_TIME_PROFILER_ACC( SinglePointerDIF_SetData_ForwardReference, 72);
			PRINT_LOAD_TIME_PROFILER_ACC( SinglePointerDIF_SetData_Set, 73);
			PRINT_LOAD_TIME_PROFILER_ACC( SinglePointerDIF_SetData_IsValid, 74);

			PRINT_LOAD_TIME_PROFILER_ACC( SinglePointerDIF_SetData_NullGUID, 700);

			// tell the caller that the file loading was aborted
			return false;
		}
	}


	PRINT_LOAD_TIME_PROFILER_ACC( DataInterface_SetData_Helper, 1 );
	PRINT_LOAD_TIME_PROFILER_ACC( StdDataInterface_SetData_Helper, 2 );
	PRINT_LOAD_TIME_PROFILER_ACC( PointerContainerDIF_SetData, 3 );
	PRINT_LOAD_TIME_PROFILER_ACC( SingleItemDIFHelper_SetData, 4 );
	PRINT_LOAD_TIME_PROFILER_ACC( SingleItemDIF_bool_SetData, 5 );
	PRINT_LOAD_TIME_PROFILER_ACC( SingleItemDIF_CFlipFlop_SetData, 6 );
	PRINT_LOAD_TIME_PROFILER_ACC( SinglePointerDIF_SetData, 7 );
	PRINT_LOAD_TIME_PROFILER_ACC( StdContainerDIF_SetData, 8 );
	PRINT_LOAD_TIME_PROFILER_ACC( StdEnumDIF_SetData, 9 );
	PRINT_LOAD_TIME_PROFILER_ACC( DataObjectContainerDIF_SetData, 10);
	PRINT_LOAD_TIME_PROFILER_ACC( EndElementHandler, 11 );
	PRINT_LOAD_TIME_PROFILER_ACC( StartElementHandler_Container, 20 );
	PRINT_LOAD_TIME_PROFILER_ACC( StartElementHandler_Base, 21 );
	PRINT_LOAD_TIME_PROFILER_ACC( StartElementHandler_Item, 22 );
	PRINT_LOAD_TIME_PROFILER_ACC( StartElementHandler_Interface, 23 );

	PRINT_LOAD_TIME_PROFILER_ACC( SinglePointerDIF_SetData_GUID, 70);
	PRINT_LOAD_TIME_PROFILER_ACC( SinglePointerDIF_SetData_GetObjectOrForwardReference, 71);
	PRINT_LOAD_TIME_PROFILER_ACC( SinglePointerDIF_SetData_ForwardReference, 72);
	PRINT_LOAD_TIME_PROFILER_ACC( SinglePointerDIF_SetData_Set, 73);
	PRINT_LOAD_TIME_PROFILER_ACC( SinglePointerDIF_SetData_IsValid, 74);
	PRINT_LOAD_TIME_PROFILER_ACC( SinglePointerDIF_SetData_NullGUID, 700);

	DoPostLoadDefaults( strParseInfo.m_ObjectsToPost );

	// Get rid of the expat parser
	XML_ParserFree( obParser );

	if( pOldCurrent != 0 )
	{
		SetCurrentContainer( pOldCurrent );
	}



	return true; // loaded o.k.
}

void ObjectDatabase::AddRemoteTracker( ObjectChangeTracker* pRemote )
{
	m_pObjectTracker = pRemote;
}

void ObjectDatabase::RemoveRemoteTracker( ObjectChangeTracker* pRemote )
{
	UNUSED(pRemote);
	// currently only support one tracker so just ntAssert...
	ntAssert( pRemote == m_pObjectTracker );
	m_pObjectTracker = 0;
}

void ObjectDatabase::SignalObjectChange( const DataObject* pDO )
{
	if (m_pObjectTracker && pDO->GetParent())
	{
		m_pObjectTracker->ObjectChanged(pDO->GetGUID(), GetDataObjectFromPointer(pDO->GetParent())->GetGUID());
	}
}

GlobalEnum& ObjectDatabase::AddGlobalEnum( const ntstd::String& name )
{
	ntAssert_p( m_GlobalEnumIndex.find( name ) == m_GlobalEnumIndex.end(), ("Global enum already exists") );
	return m_GlobalEnumIndex[name];
}

const GlobalEnum& ObjectDatabase::GetGlobalEnum( const ntstd::String& name ) const
{
	GlobalEnumIndex::const_iterator geIt = m_GlobalEnumIndex.find( name );
	ntAssert_p( geIt != m_GlobalEnumIndex.end(), ("global enum doesn't exist") );
	return (*geIt).second;
}

//------------------------------------------------------
//!
//! Alters the name and database indices of a data object
//!
//------------------------------------------------------
bool ObjectDatabase::RenameDataObject( DataObject* pDO, const ntstd::String& name )
{
	// get the old name iterator and do checks to see database is valid
	NameIndex::iterator odmIt = m_NameIndex.find( pDO->GetName() );
	ntAssert_p( odmIt != m_NameIndex.end(), ("DataObject does not exist in database") );
	ntAssert_p( odmIt->second == pDO, ("Name and data object don't match") );

	// validate the new name, either check if it's a valid container or object name, depending on its type
	if ( strcmp(pDO->GetClassName(), "ObjectContainer") == 0 )
	{
		if ( !IsValidContainerName( name ) )
		{
			return false;
		}
	}
	else
	{
		if ( !IsValidObjectName(name) )
		{
			return false;
		}
	}

	DataObject::NameType hashName(name);

	NameIndex::iterator newIt = m_NameIndex.find( hashName );
	if( newIt != m_NameIndex.end() )
	{
		// name already exists in the data base so renaming would be bad
		return false;
	}

	// o.k. we have checked everything is o.k. lets do the swap
	m_NameIndex.erase( odmIt );
	pDO->SetName( hashName );
	m_NameIndex[ hashName ] = pDO;

	GuidClient::Get().Rename(pDO->GetGUID().GetAsString().c_str(), name.c_str());

	return true;
}

//------------------------------------------------------
//!
//! clone a data object, returns a completely new data object (and assocaited object) exactly the same as the one passed in
//!
//------------------------------------------------------
DataObject* ObjectDatabase::CloneObject( const DataObject* pDefOb, const CKeyString& sName )
{
	StdDataInterface* pDefInterface  = ObjectDatabase::Get().GetInterface( pDefOb );

	// An evil twin is born...
	DataObject* pDO = ObjectDatabase::Get().ConstructObject(pDefOb->GetClassName(), sName.GetString(), GameGUID(), 0, true, false);

	// Copy over all the exposed parameters
	for(ntstd::Vector<DataInterfaceField*, Mem::MC_ODB>::const_iterator fcIt = pDefInterface->begin(); fcIt != pDefInterface->end(); ++fcIt)
	{
		(*fcIt)->SetData(pDO, (*fcIt)->GetData(pDefOb));
	}

	DoPostLoadDefaults( pDO );

	return pDO;
}
