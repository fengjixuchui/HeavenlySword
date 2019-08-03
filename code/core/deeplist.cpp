#include "objectdatabase/dataobject.h"
#include "deeplist.h"

DeepList::DeepList()
{
}

DeepList::~DeepList()
{
}

void DeepList::LinkObject(void* pObject)
{
	DataObject *pDO = ObjectDatabase::Get().GetDataObjectFromPointer( pObject );
	if (pDO != 0)
	{
		LinkDataObject(pDO);
	}
}

void DeepList::UnlinkObject(void* pObject)
{
	DataObject *pDO = ObjectDatabase::Get().GetDataObjectFromPointer( pObject );
	if (pDO != 0)
	{
		UnlinkDataObject(pDO);
	}
}

DeepListIterator DeepList::DeepIterator() const
{
	if (!m_obFixupList.empty())
	{
		ValidateFixup();
	}
	return DeepListIterator(this);
}

void DeepList::LinkDataObject(DataObject *pObject)
{
	if (!m_obFixupList.empty())
	{
		ValidateFixup();
	}
	ntAssert_p( !pObject->GetGUID().IsNull(), ("Object to link up into deeplist is an invalid database object.") );
	if ( ntstd::find( m_obObjects.begin(), m_obObjects.end(), pObject ) == m_obObjects.end() )
	{
		m_obObjects.push_back( pObject );
	}
}

void DeepList::UnlinkDataObject(DataObject *pObject)
{
	if (!m_obFixupList.empty())
	{
		ValidateFixup();
	}
	m_obObjects.erase( ntstd::find( m_obObjects.begin(), m_obObjects.end(), pObject ) );
}

void DeepList::ValidateFixup() const
{
	FixupList::iterator it = m_obFixupList.begin();
	while (it != m_obFixupList.end())
	{
		DataObject *pDO = ObjectDatabase::Get().GetDataObjectFromPointer( *it );
		if (pDO != 0 && pDO->IsValid())
		{
			ntAssert_p( !pDO->GetGUID().IsNull(), ("There is an invalid object in the fixup list.") );
			if ( ntstd::find( m_obObjects.begin(), m_obObjects.end(), pDO ) == m_obObjects.end() )
			{
				m_obObjects.push_back( pDO );
			}
			it = m_obFixupList.erase(it);
		}
		else
		{
            it++;
		}
	}
}

DeepListIterator::DeepListIterator(const DeepList* pDeepList)
{
	Reset(pDeepList);
}

DeepListIterator::~DeepListIterator()
{
	m_obActualIterators.clear();
	m_obEndIterators.clear();
}

void DeepListIterator::Reset(const DeepList* pDeepList)
{
	// empty "stacks"
	m_obActualIterators.clear();
	m_obEndIterators.clear();
	m_obActualIterators.push_front( pDeepList->m_obObjects.begin() );
	m_obEndIterators.push_front( pDeepList->m_obObjects.end() );
	TunnelDownContainers();
}

DeepListIterator::operator bool() const
{
	return !m_obActualIterators.empty();
}

bool DeepListIterator::operator++()
{
	InternalIterator &it = *m_obActualIterators.begin();

	++it;
	while ( !m_obActualIterators.empty() )
	{
		TunnelDownContainers();
		if ( !m_obActualIterators.empty() && *m_obActualIterators.begin() != *m_obEndIterators.begin() )
		{
			return true;
		}
	}
	return false;
}

const void* DeepListIterator::GetValue()
{
	InternalIterator it = *(m_obActualIterators.begin());
	return (*it)->GetBasePtr();
}

void DeepListIterator::TunnelDownContainers()
{
	while ( *m_obActualIterators.begin() != *m_obEndIterators.begin() )
	{
		DataObject* pDO = *(*m_obActualIterators.begin());
		ntAssert_p( !pDO->GetGUID().IsNull(), ("There is an invalid object in the deeplist.") );
		if ( strcmp( pDO->GetClassName(), "ObjectContainer" ) != 0 )
		{
			break;
		}
		// recurse into the container!
		ObjectContainer *container = (ObjectContainer*)pDO->GetBasePtr();
		m_obActualIterators.push_front( container->m_ContainedObjects.begin() );
		m_obEndIterators.push_front( container->m_ContainedObjects.end() );
	}

	// "Pop the stack" when we've reached the end of an iterator
	while ( !m_obActualIterators.empty() && *m_obActualIterators.begin() == *m_obEndIterators.begin() )
	{
		m_obActualIterators.pop_front();
		m_obEndIterators.pop_front();
		if ( !m_obActualIterators.empty() )
		{
			InternalIterator &it = *m_obActualIterators.begin();
			InternalIterator &endIt = *m_obEndIterators.begin();
			if ( it != endIt )
			{
				++it;
				TunnelDownContainers(); // recurse, we might have found another container to tunnel into again!
			}
		}
	}
}
