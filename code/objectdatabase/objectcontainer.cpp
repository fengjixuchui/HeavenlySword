//------------------------------------------------------
//!
//!	\file core\objectcontainer.cpp
//!
//------------------------------------------------------

#include "objectdatabase/dataobject.h"

START_CHUNKED_INTERFACE( ObjectContainer, Mem::MC_ODB )
	PUBLISH_DATAOBJECT_CONTAINER_AS( m_ContainedObjects, ContainedObjects )
	PUBLISH_VAR_AS( m_FileName, FileName )
END_STD_INTERFACE

ObjectContainer::ObjectContainer()
{
}

ObjectContainer::~ObjectContainer()
{
	// reverse order destroy
	while( m_ContainedObjects.size() > 0  )
	{
		ObjectDatabase::Get().DestroyObject( m_ContainedObjects.back() );
	}

}

void ObjectContainer::AddObject( DataObject* pObject )
{
	m_ContainedObjects.push_back( pObject );
}

void ObjectContainer::RemoveObject( DataObject* pObject )
{
	ObjectList::iterator ocIt = ntstd::find( m_ContainedObjects.begin(), m_ContainedObjects.end(), pObject );
	ntError_p( ocIt != m_ContainedObjects.end(), ("Object is not contained in this container") );
	m_ContainedObjects.erase( ocIt );
}

void ObjectContainer::InternalRemoveObjectLinkSafe( DataObject* pObject )
{
	ObjectList::iterator ocIt = ntstd::find( m_ContainedObjects.begin(), m_ContainedObjects.end(), pObject );
	if( ocIt == m_ContainedObjects.end() )
		return;
	m_ContainedObjects.erase( ocIt );
}


