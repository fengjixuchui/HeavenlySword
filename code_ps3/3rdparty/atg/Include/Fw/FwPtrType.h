//--------------------------------------------------------------------------------------------------
/**
	@file		FwPtrType.h

	@brief		defines FW_64_BIT_POINTERS macro as 1 or 0 according to target platform
				defines p32 and p64

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef FW_PTR_TYPE_H
#define FW_PTR_TYPE_H

#if defined(_WIN32) || defined (__i386__)
#define FW_64_BIT_POINTERS		0
#elif defined(_WIN64)
#define FW_64_BIT_POINTERS		1
#elif defined(__PPU__)
#define	FW_64_BIT_POINTERS		0
#elif defined(__SPU__)
#define FW_64_BIT_POINTERS		0
#else
#error Target is not supported by FwPtrType.h
#endif

#endif // FW_PTR_TYPE_H
