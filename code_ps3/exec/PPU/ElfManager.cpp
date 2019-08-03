//----------------------------------------------------------------------------------------
//! 
//! \filename exec\ppu\elfmanager.cpp
//! 
//----------------------------------------------------------------------------------------

// enable auto load on debu
#ifndef _RELEASE
#	define _ELF_ENABLE_RELOAD
#endif // _DEBUG

#include "exec/ppu/ElfManager.h"
#include "exec/ppu/spuprogram_ps3.h"

#ifdef _ELF_ENABLE_RELOAD
#	include "exec/Ppu/exec_ps3.h"
#endif // _ELF_ENABLE_RELOAD

//**************************************************************************************
//	
//**************************************************************************************
static const char *ElfsToAlwaysLoad[] =
{
	"army_spu_ps3.mod",
	"army2_spu_ps3.mod",
	""
};

//**************************************************************************************
//	Reload all SPU modules.
//**************************************************************************************
void ElfManager::ReloadAll()
{
#	ifdef _ELF_ENABLE_RELOAD
	{
		// flush all current jobs
		Exec::FrameEnd();

		// reload all spu code
		for (	ProgramMap::iterator it = m_Programs.begin();
				it != m_Programs.end();
				++it )
		{
			it->second->Reload();
		}

		// Only restart AFTER we've reloaded!
		Exec::FrameReset();
	}
#	else // _ELF_ENABLE_RELOAD
	ntPrintf("ElfManager::ReloadAll() is DISABLED: elf has NOT been reloaded !!!!!\n");
#	endif // _ELF_ENABLE_RELOAD
}

//**************************************************************************************
//	
//**************************************************************************************
bool ElfManager::Load( const CKeyString &filename )
{
	// Check that we don't load the same thing twice.
	ProgramMap::const_iterator it = m_Programs.find( filename.GetHash() );
	if ( it != m_Programs.end() )
	{
		// Duplicate found.
		ntError_p( false, ("SPU elf is already loaded. You tried to load %s, but this clashed with the elf %s.", ntStr::GetString(filename), ntStr::GetString( ( *it ).second->GetName() ) ) );

		return false;
	}

	// Load the program and add it to the map.
	SPUProgram *prog = NT_NEW SPUProgram( filename );
	ntError( prog != NULL );

	m_Programs.insert( ProgramMap::value_type( filename.GetHash(), prog ) );

	return true;
}

//**************************************************************************************
//	
//**************************************************************************************
bool ElfManager::Unload( const CKeyString &filename )
{
	// Check we have this elf already loaded.
	ProgramMap::iterator it = m_Programs.find( filename.GetHash() );
	if ( it == m_Programs.end() )
	{
		// We haven't loaded this program.
		ntError_p( false, ("Can't unload %s as we didn't load it.", ntStr::GetString(filename)) );

		return false;
	}

	NT_DELETE( ( *it ).second );
	( *it ).second = NULL;

	m_Programs.erase( it );

	return true;
}

//**************************************************************************************
//	
//**************************************************************************************
const SPUProgram *ElfManager::GetProgram( const CKeyString &filename ) const
{
	ProgramMap::const_iterator it = m_Programs.find( filename.GetHash() );
	if ( it == m_Programs.end() )
	{
		ntError_p( false, ("Cannot find elf %s, returning NULL", ntStr::GetString(filename)) );
		return NULL;
	}

	return ( *it ).second;
}


//**************************************************************************************
//	
//**************************************************************************************
ElfManager::ElfManager()
{
	uint32_t i = 0;
	while ( ElfsToAlwaysLoad[ i ][ 0 ] != '\0' )
	{
		Load( ElfsToAlwaysLoad[ i++ ] );
	}
}

//**************************************************************************************
//	
//**************************************************************************************
ElfManager::~ElfManager()
{
	for (	ProgramMap::iterator it = m_Programs.begin();
			it != m_Programs.end(); )
	{
		// We delete the object pointed-to, but not the map node itself (see below).
		NT_DELETE( ( *it ).second );
		( *it ).second = NULL;                  
		// [scee_st] crash fix: it used to call erase() here, but erasing map items when 
		// iterating through can leave the iterator invalid. Instead we just clear() the map 
		// at the end.
		
		++it;
	}

	m_Programs.clear();
}

