/***************************************************************************************************
*	 
*	CLASS			CFileAttribute
*
*	DESCRIPTION		Useful class for file stat management
*
***************************************************************************************************/

#include <sys/types.h>

class CFileAttribute
{
public:
	CFileAttribute(const char* pFileName) { GetStat(pFileName); }
	void Reset(const char* pFileName) { GetStat(pFileName); };

	size_t GetFileSize(void)	{	return m_iFileSize;		}
	time_t GetAccessTime(void)	{	return m_iAccessTime;	}
	time_t GetModifyTime(void)	{	return m_iModifyTime;	}
	time_t GetCreateTime(void)	{	return m_iCreateTime;	}
	void SetFileSize(size_t iFileSize){	m_iFileSize = iFileSize; }
	void SetAccessTime(time_t iTime){	m_iAccessTime = iTime; }
	void SetModifyTime(time_t iTime){	m_iModifyTime = iTime; }
	void SetCreateTime(time_t iTime){	m_iCreateTime = iTime; }

	bool IsBiggerThan(CFileAttribute& oFileAtt){	return (m_iFileSize > oFileAtt.GetFileSize() ? true:false);	}
	bool IsSmallerThan(CFileAttribute& oFileAtt){	return (m_iFileSize < oFileAtt.GetFileSize() ? true:false);	}
	bool isNewerThan(CFileAttribute& oFileAtt){	return (m_iModifyTime > oFileAtt.GetModifyTime() ? true:false);	}
	bool isOlderThan(CFileAttribute& oFileAtt){	return (m_iModifyTime < oFileAtt.GetModifyTime() ? true:false);	}

private:
	void GetStat(const char* pFileName);

	size_t m_iFileSize;
	time_t m_iAccessTime;
	time_t m_iModifyTime;
	time_t m_iCreateTime;
};
