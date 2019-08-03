

#include "objectdatabase/dataobject.h"
#include "objectdatabase/class_factory.h"
#include "objectdatabase/objectdatabase.h"

ClassFactory g_TheClassFactory;
ClassFactory::ClassTable ClassFactory::m_ClassConstructors[ClassFactory::MAX_CLASSES];
unsigned int ClassFactory::m_NumClassCons = 0;

void ClassFactory::RegisterClassConstructor(const char* pcString, ClassFactoryHelperBase* base )
{
	m_ClassConstructors[m_NumClassCons].m_Hash = GenerateHash(pcString);
	m_ClassConstructors[m_NumClassCons].m_CreationHelper = base;
	m_NumClassCons++;
}


void ClassFactory::DestroyHelpers()
{
	for( unsigned int i=0;i < m_NumClassCons;i++)
	{
		m_ClassConstructors[i].m_CreationHelper->Destroy();
	}
	m_NumClassCons = 0;
}
