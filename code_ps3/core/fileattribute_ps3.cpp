/***************************************************************************************************
*	 
*	CLASS			CFileAttribute
*
*	DESCRIPTION		Useful class for file stat management
*
***************************************************************************************************/

#include <cell/fs/cell_fs_file_api.h>
#include <cell/fs/cell_fs_errno.h>
#include "core/fileattribute.h"


void CFileAttribute::GetStat(const char* pFileName)
{
	char pNetFSFileName[MAX_PATH];
	strcpy( pNetFSFileName, "/HS/" );
	strcat( pNetFSFileName, pFileName );

	struct CellFsStat stFileAttribute;
	m_iFileSize = m_iAccessTime = m_iCreateTime = m_iModifyTime = 0;

	CellFsErrno result = cellFsStat(pNetFSFileName, &stFileAttribute);

	if (result == CELL_FS_SUCCEEDED)
	{
		m_iFileSize = stFileAttribute.st_size;
		m_iAccessTime = stFileAttribute.st_atime;
		m_iModifyTime = stFileAttribute.st_mtime;
		m_iCreateTime = stFileAttribute.st_ctime;
	}
}
