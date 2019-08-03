#include "filedate.h"

/// constructor
FileDate::FileDate(const char * pcPathName)
	:m_pathName(pcPathName)
	,m_gameName()
	,m_fileAttribute(m_pathName.c_str())
{
	//InitDate();
}

/// constructor
FileDate::FileDate(const char * pcPathName, const char * pcGameName)
	:m_pathName(pcPathName)
	,m_gameName(pcGameName)
	,m_fileAttribute(pcPathName)
{
	//InitDate();
}

//
bool FileDate::Update()
{
	time_t lastModification = m_fileAttribute.GetModifyTime();
	m_fileAttribute.Reset(m_pathName.c_str());
	return m_fileAttribute.GetModifyTime()>lastModification;
}


/// get path name
const ntstd::String& FileDate::GetPathName()
{
	return m_pathName;
}

// get game name
const ntstd::String& FileDate::GetGameName()
{
	if(m_gameName.empty())
	{
		return m_pathName;
	}
	else
	{
		return m_gameName;
	}
}
