
namespace ElfTypes
{

//--------------------------------------------------------------------------------------------------
//	ELF DEFINITIONS (stolen from sti, who stole them somewhere else)
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@brief	e_ident[] Identification Indexes
	@internal
**/
//--------------------------------------------------------------------------------------------------

enum
{
	EI_MAG0			= 0,		// File identification 
	EI_MAG1			= 1,		// File identification 
	EI_MAG2			= 2,		// File identification 
	EI_MAG3			= 3,		// File identification 
	EI_CLASS		= 4,		// File class 
	EI_DATA			= 5,		// Data encoding 
	EI_VERSION		= 6,		// File version 
	EI_OSABI		= 7,		// Operating system/ABI identification 
	EI_ABIVERSION	= 8,		// ABI version 
	EI_PAD			= 9,		// Start of padding bytes 
	EI_NIDENT		= 16		// Size of e_ident[] 
};

//-----------------------------------------------------------------------------------------------------
/**
	@brief	e_ident[EI_MAG0] to e_ident[EI_MAG3]
	@internal
**/
//--------------------------------------------------------------------------------------------------

enum
{
	ELFMAG0			= 0x7f,		// e_ident[EI_MAG0] 
	ELFMAG1			= 'E',		// e_ident[EI_MAG1] 
	ELFMAG2			= 'L',		// e_ident[EI_MAG2] 
	ELFMAG3			= 'F'		// e_ident[EI_MAG3] 
};

//-----------------------------------------------------------------------------------------------------
/**
	@brief	e_ident[EI_CLASS] values
	@internal
**/
//-----------------------------------------------------------------------------------------------------

enum
{
	ELFCLASSNONE	= 0,		// Invalid class 
	ELFCLASS32		= 1,		// 32-bit objects
	ELFCLASS64		= 2		// 64-bit objects
};

//-----------------------------------------------------------------------------------------------------
/**
	@brief	e_ident[EI_DATA] values
	@internal
**/
//-----------------------------------------------------------------------------------------------------

enum
{
	ELFDATANONE		= 0,		// Invalid data encoding 
	ELFDATA2LSB		= 1,		// little endian 
	ELFDATA2MSB		= 2			// big endian 
};

//-----------------------------------------------------------------------------------------------------
/**
	@brief	e_ident[EI_VERSION] values
	@internal
**/
//-----------------------------------------------------------------------------------------------------

enum
{
	EV_NONE			= 0,		// Invalid version 
	EV_CURRENT		= 1			// Current version
};

//-----------------------------------------------------------------------------------------------------
/**
	@brief	e_type values
	@internal
**/
//-----------------------------------------------------------------------------------------------------

enum
{
	ET_NONE			= 0,		// No file type
	ET_REL			= 1,		// Relocatable file
	ET_EXEC			= 2,		// Executable file
	ET_DYN			= 3,		// Shared object file
	ET_CORE			= 4			// Core file
};

//-----------------------------------------------------------------------------------------------------
/**
	@brief	e_machine values
	@internal
**/
//-----------------------------------------------------------------------------------------------------

enum
{
	EM_PPC			= 20,		// PowerPC 32-bit
	EM_PPC64		= 21,		// PowerPC 64-bit 
	EM_SPU			= 23		// SPU
};

//-----------------------------------------------------------------------------------------------------
/**
	@brief	p_type values
	@internal
**/
//-----------------------------------------------------------------------------------------------------

enum
{
	PT_LOAD			= 1,		// Loadable 
};

//-----------------------------------------------------------------------------------------------------
/**
	@brief	p_flags values
	@internal
**/
//-----------------------------------------------------------------------------------------------------

enum
{
	PF_R			= 4,
	PF_W			= 2,
	PF_X			= 1
};

//-----------------------------------------------------------------------------------------------------
/**
	@brief	s_type
	@internal
**/
//-----------------------------------------------------------------------------------------------------

enum
{
	SHT_NULL		= 0,
	SHT_PROGBITS	= 1,
	SHT_STRTAB		= 3
};

//-----------------------------------------------------------------------------------------------------
/**
	@brief	s_flags values
	@internal
**/
//-----------------------------------------------------------------------------------------------------

enum
{
	SHF_WRITE		= 1,
	SHF_ALLOC		= 2,
	SHF_EXECINSTR	= 4
};

//--------------------------------------------------------------------------------------------------
//	ELF TYPE DEFINITIONS (stolen from sti, who stole them somewhere else)
//--------------------------------------------------------------------------------------------------

typedef u32	Elf32_Addr;
typedef u32	Elf32_Off;
typedef u16	Elf32_Half;
typedef u32	Elf32_Word;
typedef s32	Elf32_Sword;

//-----------------------------------------------------------------------------------------------------
/**
	@brief		elf32 main header

	@internal
**/
//-----------------------------------------------------------------------------------------------------

typedef struct 
{
  unsigned char	e_ident[EI_NIDENT];
  Elf32_Half	e_type;
  Elf32_Half	e_machine;
  Elf32_Word	e_version;
  Elf32_Addr	e_entry;
  Elf32_Off		e_phoff;
  Elf32_Off		e_shoff;
  Elf32_Word	e_flags;
  Elf32_Half	e_ehsize;
  Elf32_Half	e_phentsize;
  Elf32_Half	e_phnum;
  Elf32_Half	e_shentsize;
  Elf32_Half	e_shnum;
  Elf32_Half	e_shstrndx;
} Elf32_Ehdr;

//-----------------------------------------------------------------------------------------------------
/**
	@brief		elf32 progbits header

	@internal
**/
//-----------------------------------------------------------------------------------------------------

typedef struct elf32_phdr
{
  Elf32_Word	p_type;
  Elf32_Off		p_offset;
  Elf32_Addr	p_vaddr;
  Elf32_Addr	p_paddr;
  Elf32_Word	p_filesz;
  Elf32_Word	p_memsz;
  Elf32_Word	p_flags;
  Elf32_Word	p_align;
} Elf32_Phdr;

//-----------------------------------------------------------------------------------------------------
/**
	@brief		elf32 section header

	@internal
**/
//-----------------------------------------------------------------------------------------------------

typedef struct 
{
  Elf32_Word	sh_name;
  Elf32_Word	sh_type;
  Elf32_Word	sh_flags;
  Elf32_Addr	sh_addr;
  Elf32_Off		sh_offset;
  Elf32_Word	sh_size;
  Elf32_Word	sh_link;
  Elf32_Word	sh_info;
  Elf32_Word	sh_addralign;
  Elf32_Word	sh_entsize;
} Elf32_Shdr;

}
