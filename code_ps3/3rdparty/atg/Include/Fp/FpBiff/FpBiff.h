//--------------------------------------------------------------------------------------------------
/**
	@file		FpBiff.h
	
	@brief		support for BIFF binary format

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef FP_BIFF_H
#define FP_BIFF_H

//--------------------------------------------------------------------------------------------------
//	INCLUDES
//--------------------------------------------------------------------------------------------------

class FwResourceHandle;

//--------------------------------------------------------------------------------------------------
//	STRUCT DEFINITIONS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@struct			FpBiffTag

	@brief			the 12 byte metadata tag used by the BIFF format
**/
//--------------------------------------------------------------------------------------------------

struct FpBiffTag
{
	u32 	m_tag;										///< 4 character tag
	u32		m_offset;									///< offset from start of buffer
	u32		m_size;										///< size of data

	enum
	{
		kRootTag			= FW_MAKE_TAG( 'A','T','G',' ' ),
		kContainerOffset	= 0xffffffff,				///< special tag denoting container node
	};

	bool	IsContainer() const		{ return ( m_offset == kContainerOffset ); }
};

//--------------------------------------------------------------------------------------------------
/**
	@class			FpBiffWalker

	@brief			helper class for traversing BIFF node hierarchies
**/
//--------------------------------------------------------------------------------------------------

class FpBiffWalker
{
public:
	typedef bool	Callback( const FpBiffTag* pTag, const u8* pBuffer, void* pUserData );

	enum Mode
	{
		kCallbackOnRoot,
		kNoCallbackOnRoot,
	};

	enum OffsetType
	{
		kOffsetFromStart,								///< offsets are from the start of the data
		kOffsetFromOffset,								///< offsets are from the address of the offset
	};

	static const FpBiffTag*		GetRootTag( const void* pData, uint dataSize, OffsetType offsetType=kOffsetFromStart );
	static const FpBiffTag*		GetRootTag( const FwResourceHandle& resource, OffsetType offsetType=kOffsetFromStart );
	static const FpBiffTag*		Recurse( const FpBiffTag* pRootTag, Callback* pCallback, const u8* pBuffer, void* pUserData, Mode mode=kCallbackOnRoot );
	static const FpBiffTag*		Recurse( const FwResourceHandle& resource, Callback* pCallback, void* pUserData, Mode mode=kCallbackOnRoot, OffsetType offsetType=kOffsetFromStart );
	static void					CountImmediateChildren( const FpBiffTag* pRootTag, u32 tag, u32& count );

protected:
	static bool					CountCallback( const FpBiffTag* pTag, const u8* pBuffer, void* pUserData );
};

#endif // FP_BIFF_H
