/**
	\file fios_base.h
	
	Base class for all FIOS classes.
	
    Copyright (C) Sony Computer Entertainment America, Inc., 2005-2006.  All rights reserved.
*/

#ifndef _H_fios_base
#define _H_fios_base

#include "fios_types.h"

namespace fios {

/**
	\brief Base class for most FIOS objects.
	This class provides shared functionality for most of the major classes in FIOS. Currently
	it provides new and delete operators, and some basic validity checking in debug builds.
*/
class FIOS_EXPORT object
{
public:
	/** Standard new. Overridden to use FIOS's memory allocator.
		\param numBytes Number of bytes to allocate.
		\param memPurpose Purpose from fios_sceamem.h, passed to allocator
		\return Allocated pointer.
	*/
	static void * operator new(size_t numBytes, U32 memPurpose);
	
	/** Standard delete. Overridden to use FIOS's memory allocator.
		\param pPtr Pointer to deallocate.
		\param memPurpose Purpose from fios_sceamem.h, passed to allocator
	*/
	static void operator delete(void *pPtr, U32 memPurpose);
	
	/** Array new. Overridden to use FIOS's memory allocator.
		\param numBytes Number of bytes to allocate.
		\param memPurpose Purpose from fios_sceamem.h, passed to allocator
		\return Allocated pointer.
	*/
	static void * operator new[](size_t numBytes, U32 memPurpose);
	
	/** Array delete. Overridden to use FIOS's memory allocator.
		\param pPtr Pointer to deallocate.
		\param memPurpose Purpose from fios_sceamem.h, passed to allocator
	*/
	static void operator delete[](void *pPtr, U32 memPurpose);

	/** Placement new.
		\param numBytes Number of bytes to allocate.
		\param pPlacement Placement.
		\return Allocated pointer, which will be the same as the pPlacement parameter.
	*/
	inline static void * operator new(size_t numBytes, void *pPlacement) { FIOS_UNUSED(numBytes); return pPlacement; }

	/** Placement delete, required by some compilers to match placement new.
		\param pPtr Pointer to deallocate.
		\param pPlacement Placement.
	*/
	inline static void operator delete(void *pPtr, void *pPlacement) { FIOS_UNUSED(pPtr); FIOS_UNUSED(pPlacement); }
	
public:
	/** \internal
		\brief Constructor. Sets a check value in debug builds. */
	object();
	/** \internal
	    Destructor. Clears the check value in debug builds. */
	~object();
	/** \internal
		\brief Check value */
	U32 m_checkValue;
	/** \internal
		\brief Class ID, visible in debugger */
	U32 m_classID;

	/** \internal
	 @{ */
	enum e_CLASSIDS
	{
		kGeneric = 0x6F626A20,      // 'obj '
		kOp = 0x6F702020,           // 'op  ',
		kCatalogCache = 0x63616368, // 'cach',
		kFilehandle = 0x66682020,   // 'fh  ',
		kScheduler = 0x73636864,    // 'schd',
		kMedia = 0x6D646961,        // 'mdia',
		kStream = 0x7374726D,       // 'strm',
		kThread = 0x74687264,       // 'thrd',
		kMutex = 0x6D757478,        // 'mutx',
		kCond = 0x636F6E64,         // 'cond',
		kRWLock = 0x72776C6B        // 'rwlk' 
	};
	/* @} */

}; /* class object */


#define FIOS_OBJECT_NEW_AND_DELETE(purpose) \
	inline static void * operator new(size_t numBytes) { return fios::object::operator new(numBytes,purpose); } \
	inline static void operator delete(void *pPtr) { fios::object::operator delete(pPtr,purpose); } \
	inline static void * operator new[](size_t numBytes) { return fios::object::operator new[](numBytes,purpose); } \
	inline static void operator delete[](void *pPtr) { fios::object::operator delete[](pPtr,purpose); } \
	inline static void * operator new(size_t numBytes, U32 memPurpose) { return fios::object::operator new(numBytes,memPurpose); } \
	inline static void operator delete(void *pPtr, U32 memPurpose) { fios::object::operator delete(pPtr,memPurpose); } \
	inline static void * operator new[](size_t numBytes, U32 memPurpose) { return fios::object::operator new[](numBytes,memPurpose); } \
	inline static void operator delete[](void *pPtr, U32 memPurpose) { fios::object::operator delete[](pPtr,memPurpose); } \
	inline static void * operator new(size_t numBytes, void *pPlacement) { FIOS_UNUSED(numBytes); return pPlacement; } \
	inline static void operator delete(void *pPtr, void *pPlacement) { FIOS_UNUSED(pPtr); FIOS_UNUSED(pPlacement); }



}; /* namespace fios */

#endif /* _H_fios_base */

