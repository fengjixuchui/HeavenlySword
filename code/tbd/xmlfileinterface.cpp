
#include "objectdatabase/dataobject.h"

#include "tbd/filedate.h"
#include "tbd/franktmp.h"
#include "tbd/xmlfileinterface.h"





KeepMeContainer::KeepMe::KeepMe()
{
	// nothing
}
KeepMeContainer::KeepMe::~KeepMe()
{
	// nothing
}







KeepMeContainer::~KeepMeContainer()
{
	ReleaseKeepMe();
}

void KeepMeContainer::ReleaseKeepMe()
{
	SAFEDELETE(m_pKeepMe);
}

//! constructor
KeepMeContainer::KeepMeContainer()
	:m_pKeepMe(0)
{
	// nothing
};



XmlFileInterface::XmlFileInterface():
	m_pFile(0),
	m_pMainObject(0)
{
	// nothing
}


XmlFileInterface::~XmlFileInterface()
{
	SAFEDELETE(m_pFile);
	ReleaseKeepMe();
}




bool XmlFileInterface::Reload(XmlFileInterface*& pObject)
{
	ntAssert(pObject->m_pFile);
	if(pObject->m_pFile->IsNewerVersionOnDisk())
	{
		KeepMe* pTmp = pObject->GetKeepMe();
		pObject->SetKeepMeToZero();
		ntstd::String objectName = ntStr::GetString(pObject->m_pMainObject->GetName());
		ntstd::String pathName = pObject->m_filePath;
		bool bIsDeleted = Delete(pObject);
		UNUSED(bIsDeleted);
		ntAssert(bIsDeleted);
		pObject = Load(objectName.c_str(), pathName.c_str());
		pObject->SetKeepMe(pTmp);
		
		pObject->Finalise();
		pObject->PropagateChange();
		
		return true;
	}
	else
	{
		return false;
	}
}

// reload pointer, rewrite pointer
XmlFileInterface* XmlFileInterface::Create(const char* pathName)
{
	ntstd::String pathNameStr = ntstd::String(pathName);
	ntAssert(pathNameStr.substr(pathNameStr.size()-4,pathNameStr.size()) == ntstd::String(".xml"));
	
	int iFindSlash;
	for(iFindSlash = strlen(pathName)-1 ; iFindSlash >= 0 ; iFindSlash-- )
	{
		if( pathNameStr[iFindSlash] == '\\')
		{
			break;
		}
	}
	
	iFindSlash++;
	ntAssert(iFindSlash>1);
	
	ntstd::String objectNameStr = pathNameStr.substr(iFindSlash,pathNameStr.size()-4-iFindSlash);
	pathNameStr = pathNameStr.substr(0,iFindSlash);
	// no object shoulkd be on
	
	XmlFileInterface* pRes = Load(objectNameStr.c_str(),pathNameStr.c_str());
	pRes->Finalise();
	return pRes;
}

// reload pointer, rewrite pointer
XmlFileInterface* XmlFileInterface::Create(const char* objectName, const char* pathName)
{
	XmlFileInterface* pRes = Load(objectName,pathName);
	pRes->Finalise();
	return pRes;
}

XmlFileInterface* XmlFileInterface::Load(const char* objectName, const char* pathName)
{
	ntstd::String fileName = GetFileNameFromObjectName(objectName,pathName);
	char acFileName[512];
	Util::GetFiosFilePath( fileName.c_str(), acFileName );
	FileBuffer file(acFileName);
	ObjectDatabase::Get().LoadDataObject( &file, fileName.c_str() );
	DataObject* pDataObject = ObjectDatabase::Get().GetDataObjectFromName(objectName);
	
	ntError_p( pDataObject, ( "Failed to open file '%s'\n", fileName.c_str() ) );

	XmlFileInterface* pRes = static_cast<XmlFileInterface*>(pDataObject->GetBasePtr());
	pRes->m_pFile = NT_NEW FileDate(fileName.c_str());
	pRes->m_pMainObject = pDataObject;
	pRes->m_filePath = ntstd::String(pathName);
	
	return pRes;
}

bool XmlFileInterface::Delete(XmlFileInterface* pObject)
{
	if(pObject->m_pMainObject)
	{
		ntAssert(ObjectDatabase::Get().GetDataObjectFromPointer( pObject )!=0);
		ObjectContainer* pContainer = pObject->m_pMainObject->GetParent();
		ntAssert(pContainer);
		ObjectDatabase::Get().Destroy(pContainer);
		ntAssert(ObjectDatabase::Get().GetDataObjectFromPointer( pObject )==0);
		return true;
	}
	else
	{
		return false;
	}
}



/*
XmlFileInterface* XmlFileInterface::New(const char* objectName, const char* objectType)
{
	ntstd::String containerName = GetContainerNameFromObjectName(objectName);
	
	ObjectContainer* pParentContainer = ObjectDatabase::Get().GetCurrentContainer();	
	ObjectContainer* pContainer = ObjectDatabase::Get().AddContainer( containerName.c_str() );
	ObjectDatabase::Get().SetCurrentContainer(  pContainer );
	DataObject* pMainObject =  ObjectDatabase::Get().ConstructObject( objectType, objectName);
	ObjectDatabase::Get().SetCurrentContainer(  pParentContainer );
	XmlFileInterface* pXml = static_cast<XmlFileInterface*>(pMainObject->GetBasePtr());
	pXml->m_pMainObject = pMainObject;
	pXml->Finalise();
	return pXml;
}


bool XmlFileInterface::Save(const char* filePath)
{
	if(filePath == 0)
	{
		if(!HasFile())
		{
			return false;
		}
	}
	else
	{
		m_filePath = ntstd::String(filePath);
	}
	
	ntstd::String fileName = GetFileNameFromObjectName(m_pMainObject->GetName().c_str(),m_filePath.c_str());
	IOOfstream ofs = IOOfstream(fileName.c_str());
	DataObject* pDataObject = ObjectDatabase::Get().GetDataObjectFromPointer( m_pContainer );
	ObjectDatabase::Get().SaveDataObject(pDataObject,&ofs,fileName.c_str(), true,false);
	return true;
}

*/


// return true if a file is associated to this object
bool XmlFileInterface::HasFile()
{
	return !m_filePath.empty();
}
