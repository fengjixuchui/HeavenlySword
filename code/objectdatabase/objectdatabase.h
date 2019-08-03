//------------------------------------------------------
//!
//!	\file core\object_database.h
//!
//------------------------------------------------------
#if !defined(CORE_OBJECT_DATABASE_H)
#define CORE_OBJECT_DATABASE_H

class ObjectContainer;

#include "core/io.h"

// forward decl
class ClassFactoryHelperBase;
class ObjectChangeTracker;
//------------------------------------------------------
//!
//! A singleton factory for creating objects and interfaces
//! and mantains the structure and search capabilities of the
//! object database
//!
//------------------------------------------------------

class ObjectDatabase : public Singleton<ObjectDatabase>
{
public:
	friend class ObjectContainer;
	ObjectDatabase();

	~ObjectDatabase();

	//! this constructs and object and returns a pointer to the actual object
	//! Note no type checking is done, don't mix pClassName and T dangerously
	template<class T>
		T* Construct( const char* pClassName, const ntstd::String& name = DataObject::GetUniqueName(), ObjectContainer* pContainer = 0 )
	{
		DataObject* pDO = ConstructObject( pClassName, name, GameGUID(), pContainer );
		return (T*) pDO->GetBasePtr();
	}

	//! opposite of Construst
	template<class T>
		void Destroy( T* pClass )
	{
		DataObject* pDO = GetDataObjectFromPointer( pClass );
		DestroyObject( pDO );
	}

	//! construct an object of class pClassName with instance name, name
	DataObject* ConstructObject(	const char* pClassName, 
									const ntstd::String& name = DataObject::GetUniqueName(), 
									const GameGUID& guid = GameGUID(), 
									ObjectContainer* pContainer = 0, 
									bool bDoDefaults = true, 
									bool bDoPostLoadDefaults = true );


	//! construct an object into an existing DataObject
	void ConstructObject( const char* pClassName, DataObject* pDO, bool bDoDefaults = true, bool bDoPostLoadDefaults = true );

	//! destroy a data object constructed with ConstructObject
	void DestroyObject( DataObject* pUnknown );

	//! clone a data object, returns a completely new data object (and assocaited object) exactly the same as the one passed in
	DataObject* CloneObject( const DataObject* pDO, const CKeyString& sName );

	void ReparentObject( DataObject* pDO, ObjectContainer *pNewParent );

	//! pass in Test gets StdDataInterface to a Test object
	StdDataInterface* GetInterface( const char* pClassName ) const;

	//! GetInterface without an ntAssert if the interface doesn't exist
	StdDataInterface* GetInterfaceUnsafe( const char* pClassName ) const ;


	//! pass in an data object get its interface
	StdDataInterface* GetInterface( const DataObject* pDO ) const
	{
		return GetInterface( pDO->GetClassName() );
	}

	DataObject* GetObjectOrForwardReference( const GameGUID& guid );

	//! Returns the actual c object associated with a GUID
	void* GetObjectPtrFromGUID( const GameGUID& guid ) const;

	//! returns the data object used by interfaceing from a GUID
	DataObject* GetDataObjectFromGUID( const GameGUID& guid ) const;

	DataObject* GetDataObjectFromPointer( const void* ptr ) const;

	DataObject* GetDataObjectFromName( const CHashedString& name ) const;

	bool IsValidContainerName( const ntstd::String& name ) const;
	bool IsValidObjectName( const ntstd::String& name ) const;

	// To help convert functionality from legacy to dataobjects will return "" if no interface or name
	DataObject::NameType GetNameFromPointer( const void* ptr ) const;
	const char* GetInterfaceNameFromPointer( const void* ptr ) const;

	// helper version that return the actual pointer to the object rather than the data object
	template< class T >
		T GetPointerFromGUID( const GameGUID& guid ) const
	{
		DataObject* pDO = GetDataObjectFromGUID( guid );
		if( pDO == 0 )
		{
			return 0;
		}
		else
		{
			return (T) pDO->GetBasePtr();
		}
	}

	template< class T >
		T GetPointerFromName(const CHashedString& name) const
	{
		DataObject* pDO = GetDataObjectFromName( name );
		if( pDO == 0 )
		{
			return 0;
		}
		else
		{
			return (T) pDO->GetBasePtr();
		}
	}



	ObjectContainer* AddContainer( const char* pName, const GameGUID& guid = GameGUID() );
	void DeleteContainer( ObjectContainer* pContainer );

	void SetCurrentContainer( ObjectContainer* pContainer );
	ObjectContainer* GetCurrentContainer();
	ObjectContainer* GetGlobalContainer()
	{
		return m_GlobalObjectContainer;
	}

	void SaveCurrentContainer( CSimpleStream* pStream, const ntstd::String& containerName  = ntstd::String() );

	//! true if the file loaded o.k. false otherwise
	bool LoadDataObject( FileBuffer* pFile, const ntstd::String& fileName );

	void SaveDataObject( DataObject* pDO, CSimpleStream* pStream, const ntstd::String& containerName = ntstd::String(), bool bRecurse = true, bool bWelderClip = false  );

	// for welder dump all known interfaces (whether used or not)
	void SaveAllInterfaces( CSimpleStream* pStream );

	void RegisterSoftInterface( const char* pcString, ClassFactoryHelperBase* base );

	// adds a new tracker, note currently only one tracker is supported at any time
	void AddRemoteTracker( ObjectChangeTracker* pRemote );
	// removes the tracker this doesn't free the object, the owner is responsible for memory issues
	void RemoveRemoteTracker( ObjectChangeTracker* pRemote );

	// to signal to any (potentially) attached trackers that the data of a certain object has changed
	void SignalObjectChange( const DataObject* pDO );

	// usually not needed unless you ConstructObject and change the default
	void DoPostLoadDefaults( DataObject* pDO );

	// add a global enum object of provided name
	GlobalEnum& AddGlobalEnum( const ntstd::String& name );

	const GlobalEnum& GetGlobalEnum( const ntstd::String& name ) const;

	//! rename data object returns false if the name already exists
	bool RenameDataObject( DataObject* pDO, const ntstd::String& name );

	//! rename object helper version of above but passed an actual object point
	//! returns false if the name already exists
	template< class T >
		bool RenameObject( const T* pObject, const ntstd::String& name )
		{
			return RenameDataObject( GetDataObjectFromPointer( pObject ), name );
		}

	//! cleans up any forward references that we never used
	void CleanForwardReferences();

protected:

	typedef ntstd::Map< GameGUID, DataObject*, ntstd::less<GameGUID>, Mem::MC_ODB > GUIDIndex;
	typedef ntstd::Map< void*, DataObject*, ntstd::less<void*>, Mem::MC_ODB > BasePtrIndex;
	//typedef ntstd::Map< ntstd::String, DataObject*, Mem::MC_ODB > NameIndex;
	typedef ntstd::Map< DataObject::NameType, DataObject*, ntstd::less<DataObject::NameType>, Mem::MC_ODB > NameIndex;
	typedef ntstd::Map< ntstd::String, GlobalEnum, ntstd::less<ntstd::String>, Mem::MC_ODB> GlobalEnumIndex;
	typedef ntstd::List< ObjectContainer*, Mem::MC_ODB > ContainerList;
	typedef ntstd::Map<unsigned int, unsigned int, ntstd::less<unsigned int>, Mem::MC_ODB> HashToIndexMap;
	typedef ntstd::List<DataObject*, Mem::MC_ODB> DataObjectList;

	// do any post load defaults
	void DoPostLoadDefaults( DataObjectList& objectsToPost );

	DataObject* GetValidDataObject( const GameGUID& guid ) const;

	void DoPostLoadFixups( DataObject* pDO );

	// remove the indexing to the object
	void UnRegisterObject( DataObject* object );

	GUIDIndex				m_GUIDIndex;
	BasePtrIndex			m_BasePtrIndex;
	NameIndex				m_NameIndex;

	GlobalEnumIndex			m_GlobalEnumIndex;

	// The master object of the database, the global namespace
	ObjectContainer*		m_GlobalObjectContainer;

	ObjectContainer*		m_CurrentContainer;

	ContainerList			m_ContainerList; //!< List of all the containers we know about

	// map a class name hash to an funciton index
	HashToIndexMap m_HashToIndex;

	// an object tracker, current we only support a single remote interface, in future we may need to extend this
	ObjectChangeTracker* m_pObjectTracker;

};

#include "objectcontainer.h"

#endif
