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

#ifndef WWS_JOB_SPU_MODULE_H
#define WWS_JOB_SPU_MODULE_H

//--------------------------------------------------------------------------------------------------

#include <jobapi/spumoduleheader.h>

//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@class			SpuModuleHandle

	@brief			Helper class for holding a pointer to an SPU Module and accessing it's header

	@note			The file might be loaded at run-time or embedded in the PPU elf

	@note			Since this class is basically just a wrapper for a pointer it may be
					passed by value
**/
//--------------------------------------------------------------------------------------------------

class SpuModuleHandle
{
public:
				SpuModuleHandle();
				SpuModuleHandle( const void* data, U32 fileSize, const char* name = NULL );

	const void*	GetAddress( void ) const	{ WWSJOB_ASSERT( m_data ); return m_data; }
	U32			GetFileSize( void ) const	{ WWSJOB_ASSERT( m_data ); return m_fileSize; }

	U32			GetEntry( void ) const		{ WWSJOB_ASSERT( m_data ); return ((const SpuModuleHeader*) m_data)->m_entryOffset;  }
	U32			GetBssStart( void ) const 	{ WWSJOB_ASSERT( m_data ); return ((const SpuModuleHeader*) m_data)->m_bssOffset;  }
	U32			GetBssSize( void )const 	{ WWSJOB_ASSERT( m_data ); return ((const SpuModuleHeader*) m_data)->m_bssSize;  }

	U32			GetCodeMarker( void ) const	{ WWSJOB_ASSERT( m_data ); return ((const SpuModuleHeader*) m_data)->m_codeMarker;  }

	U32			GetIla0( void ) const		{ WWSJOB_ASSERT( m_data ); return ((const SpuModuleHeader*) m_data)->m_ila0;  }
	U32			GetIla1( void ) const		{ WWSJOB_ASSERT( m_data ); return ((const SpuModuleHeader*) m_data)->m_ila1;  }
	U32			GetIla2( void ) const		{ WWSJOB_ASSERT( m_data ); return ((const SpuModuleHeader*) m_data)->m_ila2;  }
	U32			GetIla3( void ) const		{ WWSJOB_ASSERT( m_data ); return ((const SpuModuleHeader*) m_data)->m_ila3;  }

	U32			GetDataSize( void )const			{ WWSJOB_ASSERT( m_data ); return ((const SpuModuleHeader*) m_data)->m_dataSize;  }
	U32			GetNumCtorDtorElements( void )const { WWSJOB_ASSERT( m_data ); return (((const SpuModuleHeader*) m_data)->m_ctorDtroListSize) / 4;  }
	U32			GetUploadAddr( void )const		 	{ WWSJOB_ASSERT( m_data ); return ((const SpuModuleHeader*) m_data)->m_uploadAddr;  }
	U32			GetLinkerScriptVersion( void )const	{ WWSJOB_ASSERT( m_data ); return (((const SpuModuleHeader*) m_data)->m_linkerScriptVersion) & 0x7FFFFFFF;  }
	U32			IsPlugin( void ) const				{ WWSJOB_ASSERT( m_data ); return (((const SpuModuleHeader*) m_data)->m_linkerScriptVersion) & 0x80000000;	}

	U32			GetRequiredBufferSizeInBytes( void ) const;
	U32			GetRequiredBufferSizeInPages( void ) const;

	bool		IsValidModule( void ) const;
	void		PrintModuleHeader( void ) const;

private:
	const void*	m_data;
	U32			m_fileSize;

	static const U32 kLinkerScriptVersionNumber = 4;
} WWSJOB_ALIGNED( 8 );

//--------------------------------------------------------------------------------------------------

typedef SpuModuleHandle SpuPluginHandle;

//--------------------------------------------------------------------------------------------------

#endif /* WWS_JOB_SPU_MODULE_H */
