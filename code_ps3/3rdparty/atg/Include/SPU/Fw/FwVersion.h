//--------------------------------------------------------------------------------------------------
/**
	@file		FwVersion.h
	
	@brief		Version Information Macros

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef	FW_VERSION_H
#define	FW_VERSION_H

#define	__ATG_SDK_MAJOR			0
#define	__ATG_SDK_MINOR			0
#define	__ATG_SDK_POINT			0

#define	ATG_SDK_VERSION_NUMBER	( __ATG_SDK_MAJOR * 10000 + __ATG_SDK_MINOR * 100 + __ATG_SDK_POINT )

#if ( ( __ATG_SDK_MAJOR == 0 ) && ( __ATG_SDK_MINOR == 0 ) && ( __ATG_SDK_POINT == 0 ) )
#define	ATG_SDK_VERSION_STRING	"Internal (" __DATE__ ", " __TIME__ ")"
#else
#define __ATG_SDK_VERSION_STRING_STR( X ) #X
#define __ATG_SDK_VERSION_STRING_IMPL( X, Y, Z ) __ATG_SDK_VERSION_STRING_STR( X ) "." __ATG_SDK_VERSION_STRING_STR( Y ) "." __ATG_SDK_VERSION_STRING_STR( Z ) 
#define	ATG_SDK_VERSION_STRING	__ATG_SDK_VERSION_STRING_IMPL( __ATG_SDK_MAJOR, __ATG_SDK_MINOR, __ATG_SDK_POINT )
#endif

#endif	// FW_VERSION_H
