/**
	\file fios_configuration.h

	Configuration switches for the File I/O Scheduler. You may edit or override this
	header file to control the behavior of FIOS.

    Copyright (C) Sony Computer Entertainment America, Inc., 2005-2006.  All rights reserved.
*/

#ifndef _H_fios_configuration
#define _H_fios_configuration

#ifndef FIOSDEBUG
/** Set to 1 to enable debug behaviors of FIOS, including debug and informational output. */
# define FIOSDEBUG   ((DEBUG || _DEBUG) && !NDEBUG)
#endif

#ifndef FIOS_MEMORYAPI_LIBMEM
/** If set, FIOS will include libmem.h and use MemoryPools as its primary interface. */
# define FIOS_MEMORYAPI_LIBMEM		0
#endif

#ifndef FIOS_MEMORYAPI_SCEAMEM
/** If set, FIOS will include sceacommon/include/sceamem.h and use SCEA::Memory::Allocator as its primary interface. */
# define FIOS_MEMORYAPI_SCEAMEM     1
#endif

#if FIOS_MEMORYAPI_SCEAMEM && FIOSDEBUG
# define MEMORY_TRANSPARENT_ANNOTATION    1
#endif

#ifndef FIOS_CONSTANT_LOG2
/** Preprocessor log2 function for numeric constants. */
# define FIOS_CONSTANT_LOG2(val)    ((val <= 1) ? 0:\
                                     (val <= 2) ? 1:\
									 (val <= 4) ? 2:\
									 (val <= 8) ? 3:\
									 (val <= 16) ? 4:\
									 (val <= 32) ? 5:\
									 (val <= 64) ? 6:\
									 (val <= 128) ? 7:\
									 (val <= 256) ? 8:\
									 (val <= 512) ? 9:\
									 (val <= 1024) ? 10:\
									 (val <= 2048) ? 11:\
									 12)
#endif

#ifndef FIOS_ALIGNOF
/** Returns the log2 of the alignment of a class or other object. */
# define FIOS_ALIGNOF(t)	FIOS_CONSTANT_LOG2(__alignof(t))
#endif

/* -------------------------------------------------------------------------- */
/* LibMem support */
/* -------------------------------------------------------------------------- */
#if FIOS_MEMORYAPI_LIBMEM
# define FIOS_ALLOCATOR		Memory::MemoryPool
# define FIOS_ALLOCATE(allocator,type,id,dst,size)                  ((dst) = static_cast<type>((allocator)->MemoryAllocate((size))))
# define FIOS_ALLOCATE_ALIGNED(allocator,type,id,dst,size,align)	((dst) = static_cast<type>((allocator)->MemoryAllocateAligned((size),(Memory::MemoryAlignment)(1<<(align)))))
# define FIOS_DEALLOCATE(allocator,id,ptr)                          (allocator)->MemoryDeallocate((void*)((ptr)))
#endif

/* -------------------------------------------------------------------------- */
/* sceamem support */
/* -------------------------------------------------------------------------- */
#if FIOS_MEMORYAPI_SCEAMEM
# define FIOS_ALLOCATOR     SCEA::Memory::Allocator
# define FIOS_ALLOCATE_ALIGNED(allocator,type,id,dst,size,align)           ((dst) = static_cast<type>((allocator)->Allocate((SCEA::Memory::MemSize)size,id|(align<<SCEA::Memory::kMemAlignShift),SCEAMEM_ANNOTATION)))
# define FIOS_ALLOCATE_SHORTTERM_ALIGNED(allocator,type,id,dst,size,align) FIOS_ALLOCATE_ALIGNED(allocator,type,id|SCEA::Memory::kPoolIDShortTerm,dst,size,align)
# define FIOS_ALLOCATE_LONGTERM_ALIGNED(allocator,type,id,dst,size,align)  FIOS_ALLOCATE_ALIGNED(allocator,type,id|SCEA::Memory::kPoolIDLongTerm,dst,size,align)
# define FIOS_ALLOCATE(allocator,type,id,dst,size)                         FIOS_ALLOCATE_ALIGNED(allocator,type,id,dst,size,0)
# define FIOS_ALLOCATE_SHORTTERM(allocator,type,id,dst,size)               FIOS_ALLOCATE_SHORTTERM_ALIGNED(allocator,type,id,dst,size,0)
# define FIOS_ALLOCATE_LONGTERM(allocator,type,id,dst,size)                FIOS_ALLOCATE_LONGTERM_ALIGNED(allocator,type,id,dst,size,0)
# define FIOS_DEALLOCATE_ALIGNED(allocator,id,ptr,align)                   (allocator)->Deallocate((void*)(ptr),id|(align<<SCEA::Memory::kMemAlignShift),SCEAMEM_ANNOTATION)
# define FIOS_DEALLOCATE_SHORTTERM_ALIGNED(allocator,id,ptr,align)         FIOS_DEALLOCATE_ALIGNED(allocator,id|SCEA::Memory::kPoolIDShortTerm,ptr,align)
# define FIOS_DEALLOCATE_LONGTERM_ALIGNED(allocator,id,ptr,align)          FIOS_DEALLOCATE_ALIGNED(allocator,id|SCEA::Memory::kPoolIDLongTerm,ptr,align)
# define FIOS_DEALLOCATE(allocator,id,ptr)                                 FIOS_DEALLOCATE_ALIGNED(allocator,id,ptr,0)
# define FIOS_DEALLOCATE_SHORTTERM(allocator,id,ptr)                       FIOS_DEALLOCATE_SHORTTERM_ALIGNED(allocator,id,ptr,0)
# define FIOS_DEALLOCATE_LONGTERM(allocator,id,ptr)                        FIOS_DEALLOCATE_LONGTERM_ALIGNED(allocator,id,ptr,0)
#endif

/* --------------------------------------------------------------------------- */
/* Extensions to support short-term, long-term, callocate, strdup, new, delete */
/* --------------------------------------------------------------------------- */

#ifndef FIOS_ALLOCATE_SHORTTERM
/** Overridable macro used by FIOS to allocate from the allocator, with a hint that it's a temporary allocation with a short life. */
# define FIOS_ALLOCATE_SHORTTERM(allocator,type,id,dst,size)                FIOS_ALLOCATE((allocator),type,(id),(dst),(size))
/** Overridable macro used by FIOS to allocate aligned memory from the allocator, with a hint that it's a temporary allocation with a short life. */
# define FIOS_ALLOCATE_SHORTTERM_ALIGNED(allocator,type,id,dst,size,align)	FIOS_ALLOCATE_ALIGNED((allocator),type,(id),(dst),(size),(align))
#endif

#ifndef FIOS_ALLOCATE_LONGTERM
/** Overridable macro used by FIOS to allocate from an allocator, with a hint that it's an allocation with a long life. */
# define FIOS_ALLOCATE_LONGTERM(allocator,type,id,dst,size)                  FIOS_ALLOCATE((allocator),type,(id),(dst),(size))
/** Overridable macro used by FIOS to allocate aligned memory from the allocator, with a hint that it's an allocation with a long life. */
# define FIOS_ALLOCATE_LONGTERM_ALIGNED(allocator,type,id, dst,size,align) 	 FIOS_ALLOCATE_ALIGNED((allocator),type,(id),(dst),(size),(align))
#endif

#ifndef FIOS_CALLOCATE
/** Overridable macro used by FIOS to allocate zeroed memory from the allocator. */
# define FIOS_CALLOCATE(allocator,type,id,dst,nelem,size)                         static_cast<type>((((FIOS_ALLOCATE((allocator),type,(id),(dst),(nelem)*(size)) != NULL) ? memset((dst),0,(nelem)*(size)):NULL)))
/** Overridable macro used by FIOS to allocate zeroed, aligned memory from the allocator. */
# define FIOS_CALLOCATE_ALIGNED(allocator,type,id,dst,nelem,size,align)           static_cast<type>((((FIOS_ALLOCATE_ALIGNED((allocator),type,(id),(dst),(nelem)*(size),align) != NULL) ? memset((dst),0,(nelem)*(size)):NULL)))
/** Overridable macro used by FIOS to allocate zeroed memory from the allocator, with a hint that it's a temporary allocation with a short life. */
# define FIOS_CALLOCATE_SHORTTERM(allocator,type,id,dst,nelem,size)               static_cast<type>((((FIOS_ALLOCATE_SHORTTERM((allocator),type,(id),(dst),(nelem)*(size)) != NULL) ? memset((dst),0,(nelem)*(size)):NULL)))
/** Overridable macro used by FIOS to allocate aligned, zeroed memory from the allocator, with a hint that it's a temporary allocation with a short life. */
# define FIOS_CALLOCATE_SHORTTERM_ALIGNED(allocator,type,id,dst,nelem,size,align) static_cast<type>((((FIOS_ALLOCATE_SHORTTERM_ALIGNED((allocator),type,(id),(dst),(nelem)*(size),align) != NULL) ? memset((dst),0,(nelem)*(size)):NULL)))
/** Overridable macro used by FIOS to allocate zeroed memory from the allocator, with a hint that it's an allocation with a long life. */
# define FIOS_CALLOCATE_LONGTERM(allocator,type,id,dst,nelem,size)                static_cast<type>((((FIOS_ALLOCATE_LONGTERM((allocator),type,(id),(dst),(nelem)*(size)) != NULL) ? memset((dst),0,(nelem)*(size)):NULL)))
/** Overridable macro used by FIOS to allocate aligned, zeroed memory from the allocator, with a hint that it's an allocation with a long life. */
# define FIOS_CALLOCATE_LONGTERM_ALIGNED(allocator,type,id,dst,nelem,size,align)  static_cast<type>((((FIOS_ALLOCATE_LONGTERM_ALIGNED((allocator),type,(id),(dst),(nelem)*(size),align) != NULL) ? memset((dst),0,(nelem)*(size)):NULL)))
#endif

#ifndef FIOS_STRDUP
/** Overridable macro used by FIOS to duplicate a string from an allocator. */
# define FIOS_STRDUP(allocator,id,dst,src)            ((src) ? FIOS_ALLOCATE((allocator),char *,(id),(dst),strlen(src)+1):NULL) ? strcpy(const_cast<char*>((dst)),(src)):NULL
/** Overridable macro used by FIOS to duplicate a string from an allocator, with a hint that it's a temporary allocation with a short life. */
# define FIOS_STRDUP_SHORTTERM(allocator,id,dst,src)  ((src) ? FIOS_ALLOCATE_SHORTTERM((allocator),char *,(id),(dst),strlen(src)+1):NULL) ? strcpy(const_cast<char*>((dst)),(src)):NULL
/** Overridable macro used by FIOS to duplicate a string from an allocator, with a hint that it's an allocation with a long life. */
# define FIOS_STRDUP_LONGTERM(allocator,id,dst,src)   ((src) ? FIOS_ALLOCATE_LONGTERM((allocator),char *,(id),(dst),strlen(src)+1):NULL) ? strcpy(const_cast<char*>((dst)),(src)):NULL
#endif

#ifndef FIOS_NEW
/** Overridable macro used by FIOS to allocate a class. Class must support placement new/delete, and arguments to
     the constructor are given following this macro. */
# define FIOS_NEW(allocator,type,id,dst)       (dst) = ((FIOS_ALLOCATE_ALIGNED((allocator),type *,(id),(dst),sizeof(type),FIOS_ALIGNOF(type))) == NULL) ? NULL:new ((void*)dst) type
/** Overridable macro used by FIOS to deallocate a class. */
# define FIOS_DELETE(allocator,type,id,ptr)    (ptr ? ((ptr)->~type(), FIOS_DEALLOCATE_ALIGNED((allocator),(id),(ptr),FIOS_ALIGNOF(type))):(void)0)
#endif


#endif /* _H_fios_configuration */
