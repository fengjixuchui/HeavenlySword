/**
	\file fios_catalogcache.h

	Class definitions for the fios::catalogcache class. The catalogcache derives from fios::mediafilter,
	which itself derives from fios::media.
	
    Copyright (C) Sony Computer Entertainment America, Inc., 2005-2006.  All rights reserved.
*/
#ifndef _H_fios_catalogcache
#define _H_fios_catalogcache

#include "fios_types.h"
#include "fios_base.h"
#include "fios_platform.h"

namespace fios {

/** \brief A media filter layer that can map paths to blocks.
	The catalog cache layer is most useful when the media supports block I/O.
	However, it provides some minimal functionality when the media only
	provides path I/O APIs as well -- for example, it will cache file sizes and existence.
*/
class FIOS_EXPORT catalogcache : public mediafilter
{
public:
	FIOS_OBJECT_NEW_AND_DELETE(SCEA::Memory::kMemPurposeFIOSMediaObject|SCEA::Memory::kPoolIDLongTerm)

	/** \brief Structure describing a single file extent. */
	typedef struct extent {
		off_t      byteAddress;  //!< Byte address on disk of this extent
		off_t      byteLength;   //!< Length in bytes of this extent
	} extent;
	/** \brief Structure describing an entire file, as a list of file extents. */
	typedef struct extentlist {
		U16        numExtents;   //!< Number of extents
		ptrdiff_t  offsetToExtents;  //!< Offset to an array of numExtents extent structures.
		inline extent * getExtents() { return reinterpret_cast<extent*>(reinterpret_cast<U8*>(this)+offsetToExtents); }
		inline const extent * getExtents() const { return reinterpret_cast<const extent*>(reinterpret_cast<const U8*>(this)+offsetToExtents); }
	} extentlist;
	/** \brief Structure describing an entry, as a list of equivalent files with identical attributes. */
	typedef struct entry {
		off_t      fileSize;     //!< Size in bytes of the file
		U32        statFlags;    //!< Stat flags
		U16        entrySize;    //!< Size of this entire chunk of data, including any embedded extentlist and extent structures
		U16        numCopies;    //!< Number of copies of the file on disk
		ptrdiff_t  offsetToExtentLists; //!< Offset to an array of numCopies extentlist structures.
		inline extentlist * getExtentLists() { return reinterpret_cast<extentlist*>(reinterpret_cast<U8*>(this)+offsetToExtentLists); }
		inline const extentlist * getExtentLists() const { return reinterpret_cast<const extentlist*>(reinterpret_cast<const U8*>(this)+offsetToExtentLists); }
	} entry;
	
	/**
		\brief Iterator interface that can be used to provide a list of paths to cache.
	*/
	class FIOS_EXPORT path_iterator {
	public:
		virtual ~path_iterator() {}
		virtual U32 estimateNumberOfFiles() = 0;
		virtual err_t getNextPathToCache(const char *&pOut, off_t &byteAddr, off_t &byteLength, U32 &statFlags) = 0;
	};
	
public:
	catalogcache(media *pNextMedia, const char *pMediaRelativeTreeToCache = "/", U32 numFiles = 256);
	catalogcache(media *pNextMedia, path_iterator &pathIterator);
	virtual ~catalogcache();
	
	/** \brief Rebuilds the cache. I/O will be blocked until the rebuild is complete.
		\param[in] pMediaRelativeTreeToCache    Tree to cache on the media.
	*/
	virtual void rebuild(const char *pMediaRelativeTreeToCache = "/");
	
	/** \brief Rebuilds the cache. I/O will be blocked until the rebuild is complete.
		\param[in] pathIterator    Path iterator that provides a list of media-relative file paths to cache.
	*/
	virtual void rebuild(path_iterator &pathIterator);
	
	/** \brief Looks up a file.
		\param[in]  pPath       Path to look up.
		\return The entry that was found, or NULL to indicate that it was not found. This pointer is internal to the catalogcache and should not be deleted. */
	virtual const entry * lookup(const char *pPath);
	
	/** \brief Resets the cache. All memory is released. */
	virtual void reset();
	
	/**
		\brief Starts an I/O operation.
		\param[in,out] pIOP           I/O parameters describing the operation.
		\see media::executeIOP
	*/
	virtual void executeIOP(
		mediaioparams *pIOP);
	
	/*
		\brief Immediately returns the predicted PCE coefficients for an I/O request.
		\param[in,out] pIOP           I/O parameters describing the operation.
		\see media::speculateIOP
	*/
	virtual void speculateIOP(
		mediaioparams *pIOP);
	
protected:
	platform::rwlock m_rwLock;
	collections::hashtable	m_hashtable;
	bool m_disabled;
	const entry *m_gdeEntry;
	off_t m_gdeOffset;
	collections::hashtable::entry_iterator m_gdeIterator;

	inline void disable() { reset(); m_disabled = true; }
	
}; /* class catalogcache */

}; /* namespace fios */
#endif /* _H_fios_catalogcache */
