//----------------------------------------------------------------------------------------
//! 
//! \filename exec\ppu\spuprogram_ps3.h
//! 
//----------------------------------------------------------------------------------------
#if !defined( EXEC_PPU_SPUPROGRAM_PS3_H )
#define EXEC_PPU_SPUPROGRAM_PS3_H

#include "jobapi/spumodule.h"

class SPUTask;
class Exec;

class SPUProgram
{
	public:
		explicit SPUProgram( const CKeyString &spu_mod_filename );
		~SPUProgram();

		const SpuModuleHandle &	GetModule	()	const	{ return m_SpuModuleHandle; }

		const CKeyString &		GetName		()	const	{ return m_Name; }
		
		// reload the elf (debug purpose)
		void					Reload		();

	private:
		// init everyting, after construct and reload
		void Init();

	private:
		//! A handle to an SPU minimal module program.
		SpuModuleHandle		m_SpuModuleHandle;

		//! Name+Hash - used for looking up the SPU program.
		CKeyString			m_Name;
};

#endif // EXEC_PPU_SPUPROGRAM_PS3_H
