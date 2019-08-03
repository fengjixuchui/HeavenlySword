#include "anonymouscomponent.h"


CAnonymousEntComponent::CAnonymousEntComponent()
	:m_name("")
{
	// nothing
}
CAnonymousEntComponent::CAnonymousEntComponent(const ntstd::String& name)
	:m_name(name)
{
	// nothing
}

CAnonymousEntComponent::~CAnonymousEntComponent()
{
	// nothing
}


CAnonymousEntComponentMap::CAnonymousEntComponentMap()
{
	// nothing
}

CAnonymousEntComponentMap::~CAnonymousEntComponentMap()
{
	// nothing
}


bool CAnonymousEntComponentMap::Add(CAnonymousEntComponent* aec)
{
	ntAssert(aec->HasName());
	if(Find(aec->GetName()))
	{
		user_warn_msg(( "CAnonymousEntComponent %s allready exists." , aec->GetName().c_str() ));
		return false;
	}
	else
	{
		m_container.insert(Container::value_type(aec->GetName().c_str(),aec));
		return true;
	}
}

bool CAnonymousEntComponentMap::Remove( CAnonymousEntComponent* pobComp )
{
	ntAssert( pobComp );
	return Remove( pobComp->GetName().c_str() );
}

bool CAnonymousEntComponentMap::Remove(const char* pcCompName )
{
	ntAssert( pcCompName );

	Container::iterator it = m_container.find( pcCompName );
	if( it != m_container.end() )
	{
		m_container.erase( it );
		return true;
	}
	
	return false;
}

// return 0 is not found
CAnonymousEntComponent* CAnonymousEntComponentMap::Find(const ntstd::String& name) const
{
	//m_finder.m_pAec->m_name = name; 
	if (!m_container.empty())
	{
		Container::const_iterator it = m_container.find(name.c_str());
		if(it != m_container.end())
		{
			return it->second;
		}
	}

	return 0;
}
