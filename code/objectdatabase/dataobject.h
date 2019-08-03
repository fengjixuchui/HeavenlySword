//------------------------------------------------------
//!
//!	\file core\dataobject.h
//!
//------------------------------------------------------
#if !defined(CORE_DATAOBJECT_H)
#define CORE_DATAOBJECT_H

#include <sstream>

#include "objectdatabase/typeof.h"
#include "objectdatabase/gameguid.h"
#include "core/profiling.h"

#define DATAOBJECT_INTERNAL

#define string_of(x) #x

class UnknownClass;
class ObjectContainer;

class DataObject
{
public:
	//typedef ntstd::String	NameType;
	typedef CKeyString	NameType;

	DataObject( const NameType& name, const GameGUID& id, const char* className ) :
		m_Name( name ),
		m_ObjectGUID( id ),
		m_ClassName( className ),
		m_pParentContainer(0),
		m_pFixupList(0)
	{
	}

	// return a pointer to the object this actually reprensent (the physical C class)
	virtual void* GetBasePtr() const = 0;

	// helper function that returns the object nicely casted (no checks are done to see if valid tho
	template<class T> T* GetBaseObject()
	{
		return (T*) GetBasePtr();
	}


	virtual bool IsValid() const = 0;

	//! name of this object (object name not class name i.e. ClassA bob; this returns bob)
	const NameType& GetName() const 
	{
		return m_Name;
	}

	//! Returns the actual class this object is regardless of casting, this is
	//! actually whats it is. "Unknown Class" if its basically void*
	//! i.e. ClassA bob; this returns ClassA
	const char* GetClassName() const 
	{
		return m_ClassName;
	}

	// the object container this DataObject is a child of
	ObjectContainer* GetParent() const
	{
		return m_pParentContainer;
	}

	//! GUID of this object, stays the same even if the name changes 
	const GameGUID& GetGUID() const 
	{
		return m_ObjectGUID;
	}

	//! slighty dogdy guid for now
	static GameGUID GetObjectGUIDFromName( const ntstd::String& name );

	// returns a unique name, unfortanately at the moment its not very unique at all
	static ntstd::String GetUniqueName();

	//! DANGER DANGER this changes the actual class of the embedded object! 
	//! Don't USE unless you know what your doing
	void SetClassName( const char* pClassName );

	//! Set the objects name to something else.
	void SetName( const NameType& name )
	{
		m_Name = name;
	}

	// these member function are specific to the CStruct type of data objects
	// other types just ntAssert as they shouldn't be called in these cases
	//! used by physical C++ storage thing-me-bobs (CStructs)
	virtual char* GetPhysicalAddress( unsigned int ) const
	{
		ntAssert_p( false, ("This should only be called on CStruct DataObjects") );
		return 0;
	}
	//! used by physical C++ storage thing-mem-bobs (CStructs)
	virtual void Reset( void* )
	{
		ntAssert_p( false, ("This should only be called on CStruct DataObjects") );
	}


	typedef ntstd::List<void**,Mem::MC_ODB> FixupList;
	void CreateFixupList()
	{
		m_pFixupList = NT_NEW_CHUNK(Mem::MC_ODB) FixupList();
	}
	void DeleteFixupList()
	{
		NT_DELETE_CHUNK( Mem::MC_ODB, m_pFixupList ); m_pFixupList = 0;
	}
	void AddFixup( void** ptr )
	{
		m_pFixupList->push_back( ptr );
	}
	FixupList* GetFixupList()
	{
		return m_pFixupList;
	}
protected:
	virtual ~DataObject(){};

	// set the object container that contains this object; does no extra processing like updating the parent
	void SetParent(ObjectContainer* parent) {
		m_pParentContainer = parent;
	}

	friend class ObjectDatabase;

	NameType	m_Name;
	GameGUID	m_ObjectGUID;
	const char* m_ClassName; //!< Most derived class top (object is actually of type m_ClassName)

	ObjectContainer* m_pParentContainer;

	//! when a forward reference is created this is new'ed and added to whenever something wants
	//! the actual pointer to the object. When the objects is created this is traverse filing in
	//! and then deleting it
	FixupList* m_pFixupList; 
};

// forward decl
class DataInterface;
//------------------------------------------------------
//!
//!	ClassFactoryHelperBase. Interface for ClassFactoryHelper
//!
//------------------------------------------------------
class  ClassFactoryHelperBase
{
public:
	virtual ~ClassFactoryHelperBase() {};
	virtual void Destroy() = 0;
	virtual DataInterface* GetInterface() = 0;
	virtual const char* GetName() = 0; // useful for debugging
};


#include "cstructfield.h"
#include "interfacecallback.h"
#include "cstruct.h"
#include "datainterface.h"
#include "singleitemdif.h"
#include "stdenumdif.h"
#include "stdglobalenumdif.h"
#include "accessorfield.h"

// Pointer interface need the objectbase to convert GUID to objects/forward references
#include "objectdatabase/objectdatabase.h"

#include "stdcontainerdif.h"
#include "singlepointerdif.h"
#include "dataobjectcontainerdif.h"
#include "pointercontainerdif.h"
#include "deeplistdif.h"

// now the macros
#include "do_macros.h"

#include "class_factory.h"
#include "dostreamhelpers.h"

#include "softobject.h"
#include "xmlcompat.h"

#undef DATAOBJECT_INTERNAL

#endif // end CORE_DATAOBJECT_H
