//----------------------------------------------------------------------------------------
//! 
//! \filename exec\ppu\sputask_ps3.cpp
//! 
//----------------------------------------------------------------------------------------

#include "exec_ps3.h"
#include "spuprogram_ps3.h"

#include "core/util_ps3.h"
#include <cell/fs/cell_fs_file_api.h>

SpuModuleHandle LoadSpuModule( const char* pFilename ) {
	int	fd;
	int ret;

	ret = cellFsOpen( pFilename, CELL_FS_O_RDONLY, &fd, NULL, 0 );
	WWSJOB_ASSERT_MSG( CELL_FS_SUCCEEDED == ret, ("Failed to open file \"%s\"\n", pFilename) );

	uint64_t	fileSize;
	ret = cellFsLseek( fd, 0, CELL_FS_SEEK_END, &fileSize );
	WWSJOB_ASSERT( CELL_FS_SUCCEEDED == ret );

	void* data			= (void*) NT_MEMALIGN_CHUNK( Mem::MC_MISC, fileSize, 128 ); //memalign( 128, fileSize );
	WWSJOB_ASSERT( data );
	WWSJOB_ASSERT( WwsJob_IsAligned( data, 128 ) );

	uint64_t	fileSize2;
	ret = cellFsLseek( fd, 0, CELL_FS_SEEK_SET, &fileSize2 );
	WWSJOB_ASSERT( CELL_FS_SUCCEEDED == ret );

	uint64_t	nRead;
	ret = cellFsRead( fd, data, fileSize, &nRead );
	WWSJOB_ASSERT( CELL_FS_SUCCEEDED == ret );
					
	// Close the file
	ret = cellFsClose( fd );
	WWSJOB_ASSERT( CELL_FS_SUCCEEDED == ret );

	return SpuModuleHandle( data, WwsJob_AlignU32( fileSize, 128 ), pFilename ); }

void FreeSpuModule( SpuModuleHandle handle )
{
	WWSJOB_ASSERT( handle.GetAddress() );
	//free( const_cast< void* >( handle.GetAddress() ) );
	NT_FREE_CHUNK( Mem::MC_MISC, (uintptr_t)handle.GetAddress() );
}

SPUProgram::SPUProgram	( const CKeyString &spu_mod_filename )
:	m_Name				( spu_mod_filename )
{
	Init();
}

void SPUProgram::Init()
{
	static const int32_t MaxSpuPath = 128;
	char extended_filename[ MaxSpuPath ];
	Util::GetSpuProgramPath( ntStr::GetString(m_Name), extended_filename, MaxSpuPath );
	ntPrintf( "SPUProgram::Init loading minimal module %s\n", extended_filename );

	m_SpuModuleHandle = LoadSpuModule( extended_filename );
}

void SPUProgram::Reload()
{
	FreeSpuModule( m_SpuModuleHandle );
	Init();
}

SPUProgram::~SPUProgram( )
{
	FreeSpuModule( m_SpuModuleHandle );
}



