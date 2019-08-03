#ifndef _FILEDATE_H_
#define _FILEDATE_H_

#include "core/fileattribute.h"

// when do we want to allow in game editing
#ifdef _RELEASE
	#undef _ONTHEFLY_EDITING
#else
	#define _ONTHEFLY_EDITING
#endif // _RELEASE


//--------------------------------------------------
//!
//!	Class used to check file modification
//!	This is used for the "edit and continue" in-game editing (reload on the fly)
//!	Exciting class with lots of stuff to describe
//!
//--------------------------------------------------

class FileDate
{
private:
	/// file name
	ntstd::String m_pathName;
	
	/// this is a hack
	/// this filename should always be the same as m_pathName
	/// unfortunately, some part of the game assume a different relative
	/// path than the content directory (texture for instance)
	ntstd::String m_gameName;
	
	/// last modification date
	CFileAttribute m_fileAttribute;

public:	
	/// get path name
	const ntstd::String& GetPathName();
	
	// get game name
	const ntstd::String& GetGameName();

	/// constructor
	FileDate(const char * pcPathName);
	
	/// constructor
	FileDate(const char * pcPathName, const char * pcGameName);
	
	/// update date, return true if any change
	inline bool IsNewerVersionOnDisk()
	{
#ifdef _ONTHEFLY_EDITING
		return Update();
#else
		return false;
#endif // _RELEASE
	}

private:
	
	/// set pOldLastModification to the modification date of file pcFilename
	/// return true if any difference with input value pOldLastModification
	bool Update();

};



#endif // end of _FILEDATE_H_
