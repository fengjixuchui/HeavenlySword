/***************************************************************************************************
*	 
*	CLASS			CFileAttribute
*
*	DESCRIPTION		Useful class for file stat management
*
***************************************************************************************************/

#include <sys/stat.h>
#include "core/fileattribute.h"


void CFileAttribute::GetStat(const char* pFileName)
{
	struct stat stFileAttribute;
	m_iFileSize = m_iAccessTime = m_iCreateTime = m_iModifyTime = 0;

	if (stat(pFileName, &stFileAttribute) == 0)
	{
		m_iFileSize = stFileAttribute.st_size;
		m_iAccessTime = stFileAttribute.st_atime;
		m_iModifyTime = stFileAttribute.st_mtime;
		m_iCreateTime = stFileAttribute.st_ctime;
	}
}
