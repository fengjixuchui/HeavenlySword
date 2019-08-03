//--------------------------------------------------------------------------------------------------
/**
	@file
	
	@brief		FpXml parser : string to number conversion utilities (AsciiTo*)

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.
**/
//--------------------------------------------------------------------------------------------------

#ifndef FP_XML_STRING_ASCII_TO_H
#define FP_XML_STRING_ASCII_TO_H

//--------------------------------------------------------------------------------------------------
//	NAMESPACE DEFINITION
//--------------------------------------------------------------------------------------------------

namespace FpXmlString
{
	///@name Ascii individual value conversion functions
	//@{
	u64 	AsciiToU64(const char* const pBuffer, const char* const pBufferEnd=NULL, bool* pSuccess=NULL,
					   const char** ppParseEnd=NULL);
	s64 	AsciiToS64(const char* const pBuffer, const char* const pBufferEnd=NULL, bool* pSuccess=NULL,
					   const char** ppParseEnd=NULL);
	u32 	AsciiToU32(const char* const pBuffer, const char* const pBufferEnd=NULL, bool* pSuccess=NULL,
					   const char** ppParseEnd=NULL);
	s32 	AsciiToS32(const char* const pBuffer, const char* const pBufferEnd=NULL, bool* pSuccess=NULL,
					   const char** ppParseEnd=NULL);
	u16 	AsciiToU16(const char* const pBuffer, const char* const pBufferEnd=NULL, bool* pSuccess=NULL,
					   const char** ppParseEnd=NULL);
	s16		AsciiToS16(const char* const pBuffer, const char* const pBufferEnd=NULL, bool* pSuccess=NULL,
					   const char** ppParseEnd=NULL);
	u8 		AsciiToU8(const char* const pBuffer, const char* const pBufferEnd=NULL, bool* pSuccess=NULL,
					  const char** ppParseEnd=NULL);
	s8 		AsciiToS8(const char* const pBuffer, const char* const pBufferEnd=NULL, bool* pSuccess=NULL,
					  const char** ppParseEnd=NULL);
	float	AsciiToFloat(const char* const pBuffer, const char* const pBufferEnd=NULL, bool* pSuccess=NULL,
						 const char** ppParseEnd=NULL);
	bool	AsciiToBool(const char* const pBuffer, const char* const pBufferEnd=NULL, bool* pSuccess=NULL,
			  			const char** ppParseEnd=NULL);
	//@}

	///@name Array helpers
	//@{
	int 	AsciiToArray(u64* pDest, const char* pSrc, int numToRead, const char separator=0);
	int 	AsciiToArray(s64* pDest, const char* pSrc, int numToRead, const char separator=0); 
	int 	AsciiToArray(u32* pDest, const char* pSrc, int numToRead, const char separator=0); 
	int 	AsciiToArray(s32* pDest, const char* pSrc, int numToRead, const char separator=0);
	int 	AsciiToArray(u16* pDest, const char* pSrc, int numToRead, const char separator=0); 
	int 	AsciiToArray(s16* pDest, const char* pSrc, int numToRead, const char separator=0); 
	int 	AsciiToArray(u8* pDest, const char* pSrc, int numToRead, const char separator=0);  
	int 	AsciiToArray(s8* pDest, const char* pSrc, int numToRead, const char separator=0);  
	int 	AsciiToArray(float* pDest, const char* pSrc, int numToRead, const char separator=0); 
	int 	AsciiToArray(bool* pDest, const char* pSrc, int numToRead, const char separator=0);
	//@}
};

//--------------------------------------------------------------------------------------------------

#endif	//FP_XML_STRING_ASCII_TO_H
