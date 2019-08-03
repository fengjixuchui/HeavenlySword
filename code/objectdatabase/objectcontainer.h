//------------------------------------------------------
//!
//!	\file core\objectcontainer.h
//!
//------------------------------------------------------
#if !defined(CORE_OBJECT_CONTAINER_H)
#define CORE_OBJECT_CONTAINER_H

class ObjectContainer
{	
public:
	friend class DataVisitor;
	friend class ObjectDatabase;

	// [scee_st] originally this list wasn't chunked but it looks like the overhead is quite large
	typedef ntstd::List<DataObject*, Mem::MC_ODB> ObjectList;

	ObjectContainer();

	~ObjectContainer();

	void AddObject( DataObject* pObject );

	void RemoveObject( DataObject* pObject );

	ObjectList m_ContainedObjects;
	ntstd::String m_FileName; //! some object container are also file containers
private:
	//! if the object is contained Delete the link without deleting the object, if the object isn't in here do nothing SLOW SLOW
	void InternalRemoveObjectLinkSafe( DataObject* pObject );
};


#endif
