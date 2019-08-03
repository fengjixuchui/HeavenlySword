//----------------------------------------------------------------------------------------
//! 
//! \filename exec\ppu\elfmanager.h
//! 
//----------------------------------------------------------------------------------------
#ifndef ELFMANAGER_H_
#define ELFMANAGER_H_

class	SPUProgram;
struct	ElfManagerKeyBind;

class ElfManager : public Singleton< ElfManager >
{
	public:
		//
		//	Load/Unload an SPU ELF/BIN file.
		//	Both functions return 'true' for success and 'false' otherwise.
		//
		bool		Load	( const CKeyString &filename );
		bool		Unload	( const CKeyString &filename );

	public:
		//
		//	Find an SPU program by name.
		//
		const SPUProgram *	GetProgram	( const CKeyString &filename )	const;

	public:
		//
		//	Ctor, dtor.
		//
		ElfManager();
		~ElfManager();

		//! Debug aid, reloads the elfs
		void ReloadAll();
	private:
		// Reload all elves.
		friend struct ElfManagerKeyBind;


		//
		//	Hash-table of elf/bin files, hash is filename hash.
		//
		typedef ntstd::Map< uint32_t, SPUProgram * > ProgramMap;
		ProgramMap	m_Programs;
};


#endif // !ELFMANAGER_H_
