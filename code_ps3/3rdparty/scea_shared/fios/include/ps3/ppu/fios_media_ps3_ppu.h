/**
	\file fios_media_ps3_ppu.h

	Class definitions for the fios::ps3media class.

    Copyright (C) Sony Computer Entertainment America, Inc., 2005-2006.  All rights reserved.
*/
#ifndef _H_fios_ps3_ppu_media
#define _H_fios_ps3_ppu_media
#include "sceacommon/include/sceatargetmacros.h"
#if SCEA_TARGET_OS_PS3_PPU || SCEA_TARGET_OS_PS3_PU

#include "fios/include/fios_types.h"
#include "fios/include/fios_platform.h"
#include <cell/cell_fs.h>
#include <sys/paths.h>

namespace fios {


/* \internal
	Number of filehandles that PS3 supports. */
#define FIOS_PS3MAXFILES    256


/**
* \addtogroup FIOSMedia	Media
* @{
*/

/**
	\brief Media class for generic PS3 drives.
	This class is the lowest common denominator and works for all types of
	media -- hard disks, CD/DVD, and network drives. Future subclasses
	will be created to optimize at least CD/DVD/BluRay access.
*/
class FIOS_EXPORT ps3media : public media
{
public:
	FIOS_OBJECT_NEW_AND_DELETE(SCEA::Memory::kMemPurposeFIOSMediaObject|SCEA::Memory::kPoolIDLongTerm)

	/** Constructor from a PS3 filesystem path.
		The path should be to a directory in the PS3 filesystem tree. The default value of
		SYS_APP_HOME corresponds to the fileserver root. If you wish to use SYS_HOST_ROOT,
		or another type of mount (eg CNFS) you will need to mount it yourself!
	*/
	ps3media(
		const char *pPath = SYS_APP_HOME);
	
	/** Destructor. */
	virtual ~ps3media();
	
	/**
		\brief Returns the debugging name of a variable by its index.
		\param[in] index    Index of the variable to query.
		\return Name of the variable as a C-string, or NULL if the index is unknown.
	*/
	virtual const char *getVariableName(size_t index) const;

	/**
		\brief Returns a unique identifier for the currently loaded media.
		\see media::readIdentifier
		\return A unique identifier for the media, or 0 if no media is present.
	*/
	virtual U64 readIdentifier();
	
protected:
	/** \internal
	    @{ */
	platform::mutex m_dirLock;       //!< Lock protecting kMEDIAACTION_GETDIRENTRIES.
	int m_dirFD;                     //!< Directory filehandle for #kMEDIAACTION_GETDIRENTRIES, or -1 if not open.
	off_t m_dirOffset;               //!< How many entries have been retrieved from the directory so far.
	platform::rwlock m_fdLock;       //!< Lock to protect internal fd state when we are doing an lseek/read. Will be obsolete once we get a pread API.
	CellFsDirent m_dirent;           //!< Most recent directory entry retrieved.
	char m_dirPath[FIOS_PATH_MAX];   //!< Path of the open directory filehandle.
	char m_mountPath[256];           //!< Mount path for the media.
	
	/* Stuff for handling fd indirection */
	static platform::nativefd_t allocateIndirectFD();
	static void deallocateIndirectFD(platform::nativefd_t pFD);
	
	/** Called by worker threads to execute I/O.
	    \param[in,out]  pIOP   Request to dispatch. */
	void executeIOP(mediaioparams *pIOP);

	/** Converts an incoming path to a filename.
		\param[in]  pPath      User path.
		\param[out] pFilename  PS3 path. Buffer should be at least CELL_FS_MAX_MP_LENGTH+CELL_FS_MAX_FS_PATH_LENGTH+1 chars.
		\return Returns pFilename.
	*/
	char * mediaPathToPS3Path(
		const char *pPath,
		char * pFilename) const;
	
	/** Converts a PS3 filesystem error code to a FIOS error code.
	    \param[in]  err        Error to convert. Defaults to errno.
		\return A FIOS error code.
	*/
	err_t ps3ErrorToFIOSError(
		CellFsErrno err = errno);

	/** Implementation of getdirentries.
		\param[in,out]  pIOP        Request for kMEDIAACTION_GETDIRENTRIES.
		\param[in]      pFilename   Native PS3 filename. (not a media path!)
		\return True if a retry is needed, false if the operation is complete.
	*/
	bool getDirEntriesImp(mediaioparams *pIOP, char *pFilename);
	
	/** Waits for media to reappear, and then closes and reopens all open filehandles.
		\param[in]      filename    Buffer to hold native filenames.
	*/
	void waitForMedia(char filename[]);
	
	/*@}*/
};




/*@}*/

}; /* namespace fios */

#endif /* SCEA_TARGET_OS_PS3_PPU || SCEA_TARGET_OS_PS3_PU */
#endif /* _H_fios_media_ps3_ppu */
