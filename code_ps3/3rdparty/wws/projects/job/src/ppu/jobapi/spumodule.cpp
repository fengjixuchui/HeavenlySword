/*
 * Copyright (c) 2003-2006 Sony Computer Entertainment.
 * Use and distribution without consent strictly prohibited.
 */

//--------------------------------------------------------------------------------------------------
/**
	@file		

	@brief		Wrapper class for managing an SPU module.
				Note that the handle is not reference counted.
**/
//--------------------------------------------------------------------------------------------------

#include <stdlib.h>

#include <cell/fs/cell_fs_file_api.h>

#include <jobapi/spumodule.h>

//--------------------------------------------------------------------------------------------------

SpuModuleHandle::SpuModuleHandle()
:	m_data( NULL ),
	m_fileSize( 0 )
{
}

//--------------------------------------------------------------------------------------------------

SpuModuleHandle::SpuModuleHandle( const void* pData, U32 fileSize, const char* name )
:	m_data( pData ),
	m_fileSize( fileSize )
{
	if ( !IsValidModule() )
	{
		if ( name )
			JobPrintf( "ERROR: \"%s\" is not a valid module\n", name );
		else
			JobPrintf( "ERROR: module at 0x%08X is not a valid module\n", (U32)pData );

		PrintModuleHeader();
		if ( name )
			WWSJOB_ASSERT_MSG( false, ( "ERROR: \"%s\" is not a valid module\n", name ) );
		else
			WWSJOB_ASSERT_MSG( false, ( "ERROR: module at 0x%08X is not a valid module\n", (U32)pData ) );
	}

	/*if ( name )
	{
		//Anything in the .data section is suspicious because it won't be re-initialized if the job is re-run.
		//If it wasn't going to be modified it should have been in ".rodata".
		//If it was zero-initialized it should be in the ".bss".
		//Hence there shouldn't really be anything in ".data".
		if ( GetDataSize() )
			JobPrintf( "WARNING: \"%s\" has 0x%X bytes in the .data section\n", name, GetDataSize() );

		//Global ctors and dtors are not recommended for SPU programs.  At present we do call them, but this is more because of
		//compiler issues.  Certainly we don't guarantee calling dtors in the future and calling ctors may change too.
		//User's should use global ctors.
		if ( GetNumCtorDtorElements() )
			JobPrintf( "WARNING: \"%s\" has %d elements in total on its ctor and dtor lists\n", name, GetNumCtorDtorElements() );

		//if ( GetUploadAddr() ) 
			//JobPrintf( "NOTE: \"%s\" is a fixed addr module for uploading to 0x%X\n", name, GetUploadAddr() );
		//else
			//JobPrintf( "NOTE:\"%s\" is a position independent module\n", name );
	}
	else
	{
		if ( GetDataSize() )
			JobPrintf( "WARNING: module at 0x%08X has 0x%X bytes in the .data section\n", (U32)pData, GetDataSize() );

		if ( GetNumCtorDtorElements() )
			JobPrintf( "WARNING: module at 0x%08X has %d elements in total on its ctor and dtor lists\n", (U32)pData, GetNumCtorDtorElements() );

		//if ( GetUploadAddr() ) 
			//JobPrintf( "NOTE: module at 0x%08X is a fixed addr module for uploading to 0x%X\n", (U32)pData, GetUploadAddr() );
		//else
			//JobPrintf( "NOTE: module at 0x%08X is a position independent module\n", (U32)pData );
	}*/
}

//--------------------------------------------------------------------------------------------------

U32 SpuModuleHandle::GetRequiredBufferSizeInBytes( void ) const
{
	WWSJOB_ASSERT( m_data );
	const SpuModuleHeader* spuModHdr = (const SpuModuleHeader*) m_data;
	return spuModHdr->m_bssOffset + spuModHdr->m_bssSize;
}

//--------------------------------------------------------------------------------------------------

U32 SpuModuleHandle::GetRequiredBufferSizeInPages( void ) const
{
	//round up to next page size
	return ( WwsJob_AlignU32( GetRequiredBufferSizeInBytes(), 1024 ) / 1024 );
}

//--------------------------------------------------------------------------------------------------

bool SpuModuleHandle::IsValidModule( void ) const
{
	//Check the file isn't suspiciously small
	if ( GetFileSize() <= 0x30 )
		 return false;

	if ( GetCodeMarker() != kSpuModuleMarker )
		return false;

	//Check that the ila markers are correct
	if ( (GetIla0() & 0xFE0001FF) != 0x42000002 )	//ila $2, (val16 << 2) | 0b00
		return false;
	if ( (GetIla1() & 0xFE0001FF) != 0x42000082 )	//ila $2, (val16 << 2) | 0b01
		return false;
	if ( (GetIla2() & 0xFE0001FF) != 0x42000102 )	//ila $2, (val16 << 2) | 0b10
		return false;
	if ( (GetIla3() & 0xFE0001FF) != 0x42000182 )	//ila $2, (val16 << 2) | 0b11
		return false;

	//And that the ila marker values are not all zero
	if ( ((GetIla0() & 0x01FFFE00) == 0)
		&& ((GetIla1() & 0x01FFFE00) == 0)
		&& ((GetIla2() & 0x01FFFE00) == 0)
		&& ((GetIla3() & 0x01FFFE00) == 0) )
	{
		return false;
	}

	//Make sure the correct linker script has been used
	if ( GetLinkerScriptVersion() != kLinkerScriptVersionNumber )
		return false;
		
	//Make sure the bss is valid
	if ( !WwsJob_IsAligned( GetBssStart(), 16 ) )
		return false;
	if ( !WwsJob_IsAligned( GetBssSize(), 16 ) )
		return false;

	return true;
}

//--------------------------------------------------------------------------------------------------

void SpuModuleHandle::PrintModuleHeader( void ) const
{
	const U32* pData = (const U32*) GetAddress();
	WWSJOB_UNUSED( pData );

	JobPrintf( "0x%08X_%08X_%08X_%08X\n", pData[0], pData[1], pData[2], pData[3] );
	JobPrintf( "0x%08X_%08X_%08X_%08X\n", pData[4], pData[5], pData[6], pData[7] );
	JobPrintf( "0x%08X_%08X_%08X_%08X\n", pData[8], pData[9], pData[10], pData[11] );
	JobPrintf( "(Note: Expected linker script version number is %d)\n", kLinkerScriptVersionNumber );
}

//--------------------------------------------------------------------------------------------------
