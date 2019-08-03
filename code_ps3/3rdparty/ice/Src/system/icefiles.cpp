/*
 * Copyright (c) 2003-2005 Naughty Dog, Inc. 
 * A Wholly Owned Subsidiary of Sony Computer Entertainment, Inc.
 * Use and distribution without consent strictly prohibited
 */

#include "icefiles.h"
#include "icebase.h"

#include <cell/cell_fs.h>

using namespace Ice;

static bool s_initialized = false;

static char s_workingFolder[256] = SYS_APP_HOME "/";

bool
InitializeFileSystem()
{
	return true;
}

void 
ReleaseFileSystem()
{
}

void
Ice::GetErrorDescription ( int error, char * paErrorBuffer, int iBufferSize )
{

#define DEFAULT_ERROR_ANNOTATION(error) \
	case error: strncpy(paErrorBuffer, #error, iBufferSize); break;

	switch ( error )
	{
		DEFAULT_ERROR_ANNOTATION(CELL_FS_SUCCEEDED)
		DEFAULT_ERROR_ANNOTATION(CELL_FS_EDOM)
		DEFAULT_ERROR_ANNOTATION(CELL_FS_EFAULT)
		DEFAULT_ERROR_ANNOTATION(CELL_FS_EFBIG)
		DEFAULT_ERROR_ANNOTATION(CELL_FS_EFPOS)
		DEFAULT_ERROR_ANNOTATION(CELL_FS_EMLINK)
		DEFAULT_ERROR_ANNOTATION(CELL_FS_ENFILE)
		DEFAULT_ERROR_ANNOTATION(CELL_FS_ENOENT)        
		DEFAULT_ERROR_ANNOTATION(CELL_FS_ENOSPC)        
		DEFAULT_ERROR_ANNOTATION(CELL_FS_ENOTTY)        
		DEFAULT_ERROR_ANNOTATION(CELL_FS_EPIPE)         
		DEFAULT_ERROR_ANNOTATION(CELL_FS_ERANGE)        
		DEFAULT_ERROR_ANNOTATION(CELL_FS_EROFS)         
		DEFAULT_ERROR_ANNOTATION(CELL_FS_ESPIPE)        
		DEFAULT_ERROR_ANNOTATION(CELL_FS_E2BIG)         
		DEFAULT_ERROR_ANNOTATION(CELL_FS_EACCES)        
		DEFAULT_ERROR_ANNOTATION(CELL_FS_EAGAIN)        
		DEFAULT_ERROR_ANNOTATION(CELL_FS_EBADF)           
		DEFAULT_ERROR_ANNOTATION(CELL_FS_EBUSY)           
//		DEFAULT_ERROR_ANNOTATION(CELL_FS_ECHILD)          
		DEFAULT_ERROR_ANNOTATION(CELL_FS_EEXIST)          
		DEFAULT_ERROR_ANNOTATION(CELL_FS_EINTR)           
		DEFAULT_ERROR_ANNOTATION(CELL_FS_EINVAL)          
		DEFAULT_ERROR_ANNOTATION(CELL_FS_EIO)             
		DEFAULT_ERROR_ANNOTATION(CELL_FS_EISDIR)          
		DEFAULT_ERROR_ANNOTATION(CELL_FS_EMFILE)          
		DEFAULT_ERROR_ANNOTATION(CELL_FS_ENODEV)          
		DEFAULT_ERROR_ANNOTATION(CELL_FS_ENOEXEC)         
		DEFAULT_ERROR_ANNOTATION(CELL_FS_ENOMEM)          
		DEFAULT_ERROR_ANNOTATION(CELL_FS_ENOTDIR)         
		DEFAULT_ERROR_ANNOTATION(CELL_FS_ENXIO)           
		DEFAULT_ERROR_ANNOTATION(CELL_FS_EPERM)           
		DEFAULT_ERROR_ANNOTATION(CELL_FS_ESRCH)          
		DEFAULT_ERROR_ANNOTATION(CELL_FS_EXDEV)           
		DEFAULT_ERROR_ANNOTATION(CELL_FS_EBADMSG)         
		DEFAULT_ERROR_ANNOTATION(CELL_FS_ECANCELED)       
		DEFAULT_ERROR_ANNOTATION(CELL_FS_EDEADLK)         
		DEFAULT_ERROR_ANNOTATION(CELL_FS_EILSEQ)        
		DEFAULT_ERROR_ANNOTATION(CELL_FS_EINPROGRESS)     
		DEFAULT_ERROR_ANNOTATION(CELL_FS_EMSGSIZE)        
		DEFAULT_ERROR_ANNOTATION(CELL_FS_ENAMETOOLONG)    
		DEFAULT_ERROR_ANNOTATION(CELL_FS_ENOLCK)          
		DEFAULT_ERROR_ANNOTATION(CELL_FS_ENOSYS)          
		DEFAULT_ERROR_ANNOTATION(CELL_FS_ENOTEMPTY)       
		DEFAULT_ERROR_ANNOTATION(CELL_FS_ENOTSUP)         
		DEFAULT_ERROR_ANNOTATION(CELL_FS_ETIMEDOUT)       
		DEFAULT_ERROR_ANNOTATION(CELL_FS_EFSSPECIFIC)   
		DEFAULT_ERROR_ANNOTATION(CELL_FS_EOVERFLOW)     
		default: strncpy(paErrorBuffer,"Undefined error",iBufferSize); break;
	}

#undef DEFAULT_ERROR_ANNOTATION
} 

void
Ice::SetCurrentWorkingFolder(const char *path)
{
	::printf("SetCurrentWorkingFolder(%s)\n", path);
	strcpy(s_workingFolder, path);
	
	// Make sure it ends with an "/".
	int len = strlen(s_workingFolder);
	if (s_workingFolder[len - 1] != '/') {
		strcat(s_workingFolder, "/");
	}
}

const char *
Ice::GetCurrentWorkingFolder()
{
	return s_workingFolder;
}

void
Ice::OutputExistingFiles(const char *path)
{
    int ret;
    int fd;
    CellFsDirent  dent;
    U64 readSize;

	if (!s_initialized) {
		s_initialized = InitializeFileSystem();
	}
	
    ret = cellFsOpendir (path, &fd);
    if (ret) {
        ::printf("cellFsOpendir(%s) returned %d\n", path, ret);
        return;
    }

    for (int i = 0; ; i++) {
        ret = cellFsReaddir (fd, &dent, &readSize);
        if (ret) {
			printf("cellFsReaddir(%s) returned %d\n", path, ret);
            cellFsClosedir (fd);
            return;
        }
        if (readSize == 0) { break; }

		//char extname[CELL_FS_MAX_

        printf("%s\n", dent.d_name);

		//CellFsStat stat;

		//cellFsStat(dent.d_name, &stat, 
    }

    ret = cellFsClosedir (fd);
    if (ret) {
        printf("cellFsClosedir(%s) returned %d\n", path, ret);
        return;
    }
}

File::File(const char *name)
{
	if (!s_initialized) {
		s_initialized = InitializeFileSystem();

		if (s_initialized) {
			::printf("---------------------------------------\n");
			::printf("$ ls /app_home\n");
			Ice::OutputExistingFiles("/app_home");
			::printf("---------------------------------------\n");
		}
	}

	CellFsErrno error; 

	if (name[0] == '/') {
		strcpy(m_filename, SYS_APP_HOME);
		strcat(m_filename, name);
	} else {
		strcpy(m_filename, SYS_APP_HOME "/");
		strcat(m_filename, s_workingFolder);
		strcat(m_filename, name);
	}

	if (s_initialized) {
		error = cellFsOpen(m_filename, CELL_FS_O_RDONLY, &m_fd, nullptr, 0);

		if (error == CELL_FS_SUCCEEDED) 
		{
			::printf("Opened '%s' for reading.\n", m_filename);
			m_ok = true;
		}
		else {
			// get error descirption
			char buffer[1024];
			GetErrorDescription(error,buffer,1024);
			// spit out error message
			::printf("ERROR: Failed to open file '%s', %s (%i).\n", m_filename, buffer, error);		
			m_ok = false;
		}
	}
	else {
		::printf("ERROR: Couldn't mount file system.\n");
		m_ok = false;
	}
}

File::~File()
{
	if (m_ok) {
		cellFsClose(m_fd);
	}
}

U64 File::GetSize(void) const
{
	if (m_ok) {
		CellFsStat stat;

		cellFsFstat(m_fd, &stat);

		return stat.st_size;
	}
	else 
	{
		return 0;
	}
}

void File::Read(U64 start, U64 size, void *data) const
{
	if (!m_ok) 
		return;

	CellFsErrno error;
	U64 readBytes;
	U64 pos;
	
	error = cellFsLseek(m_fd, start, CELL_FS_SEEK_SET, &pos);
	(void)pos;
	
	if (error != CELL_FS_SUCCEEDED) {
		::printf("ERROR: Couldn't seek to %llx in %s (fd=%d).\n", 
				 start, m_filename, m_fd);
		return;
	}
	
	error = cellFsRead(m_fd, data, size, &readBytes);
	
	if (error != CELL_FS_SUCCEEDED) {
		::printf("ERROR: Couldn't read %llx bytes from %s (fd=%d).\n", 
				 size, m_filename, m_fd);
		return;
	}
	
	if (readBytes != size) {
		::printf("WARNING: cellFsRead(%d, %lld, %p) in %s returned %lld\n",
				 m_fd, size, data, m_filename, readBytes);
	}
}
