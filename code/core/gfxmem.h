#if !defined(CORE_GFXMEM_H)
#define CORE_GFXMEM_H

#include "core/mem.h"

//! whether we can use the rest of debug memory to overflow
#if defined( _HAVE_DEBUG_MEMORY ) && !defined( _MASTER )
#define _HAVE_DEBUG_OVERFLOW	// comment out to limit to 192 of main memory
#endif

//! whether we collapse all small chunks into one large one
#ifdef _MASTER
#define _COLLAPSE_SMALL_CHUNKS
#endif

namespace Mem
{
	// this ickyness is to work around Gc current memory model which reckons it owns all the RSX Main RAM
	// note this should be GcInitParams::kDriverHostMemSize but that brings in all the ice header
	// so I validate it in the CPP file in case it changes...

	namespace GcMem
	{
		static const uint32_t ICE_RAM_SIZE = ( 80*1024 + 128 ) + 4*16;

		static const uint32_t GC_PUSH_BUFFER_SIZE = (Mb * 11);		
		static const uint32_t GC_SCRATCH_RAM_SIZE = Mb * 10;
		static const uint32_t GC_GPUREPORT_RAM_SIZE = Kb * 16;

		// amount of RSX addressable reserved for GC
		static const uint32_t GC_XDDR_RAM	= ICE_RAM_SIZE + (GC_PUSH_BUFFER_SIZE*2) + GC_SCRATCH_RAM_SIZE + GC_GPUREPORT_RAM_SIZE;
	};

	namespace GpMem
	{
		// amount of RSX addressable reserved for debug prims
		// note, is doubled because of double buffered render context
		static const uint32_t GP_PRIMS_PB_SIZE	= Mb;

		#ifdef _GOLD_MASTER 
		static const uint32_t GP_XDDR_RAM = 0; // no debug primitves on gold master buids
		#else
		static const uint32_t GP_XDDR_RAM = GP_PRIMS_PB_SIZE * 2;
		#endif
	};

	namespace Heresy
	{
		static const uint32_t HERESY_PUSH_BUFFER_SPACE = Mb * 5;
		static const uint32_t HERESY_SHADERDICT_SPACE = Mb;

		// amount of RSX addressable reserved for Heresy
		static const uint32_t HERESY_XDDR_RAM = HERESY_PUSH_BUFFER_SPACE + HERESY_SHADERDICT_SPACE;
	};

	// amount visible for 'host mem' rsx usage.
	static const uint32_t RSX_MAIN_USER_SIZE	=	Mb * 6;

	// All proceeding chunks divvy up the 192 megs of main ram
	//-------------------------------------------------------------

	// total RSX addressable XDDR memory
	// 33 (31) + 2 + 6 + 6 = Mb * 47 (45)
	static const uint32_t RSX_ADDRESSABLE_SIZE	=	ROUND_POW2( GcMem::GC_XDDR_RAM, Mb ) +
													GpMem::GP_XDDR_RAM +
													Heresy::HERESY_XDDR_RAM +
													RSX_MAIN_USER_SIZE;

	// these are here and not in the overflow chunk as they may requre
	// garunteeed unfragmented memory, large allocations or the like
	static const uint32_t GFX_CHUNK_SIZE	= 10	* Mb;
	static const uint32_t LOADER_CHUNK_SIZE = 6		* Mb;
	static const uint32_t ANIM_CHUNK_SIZE	= 20	* Mb;
	static const uint32_t HAVOK_CHUNK_SIZE	= 8		* Mb;
	static const uint32_t AUDIO_CHUNK_SIZE	= 16	* Mb;

	// All all of these chunks disappear on a _MASTER build into one large chunk
	//-------------------------------------------------------------
	static const uint32_t OBD_CHUNK_SIZE	= 4 * Mb;
		
	static const uint32_t ARMY_CHUNK_SIZE	= 2 * Mb;
	static const uint32_t LUA_CHUNK_SIZE	= 2 * Mb;
	static const uint32_t ENTITY_CHUNK_SIZE	= 2 * Mb;
	static const uint32_t MISC_CHUNK_SIZE	= 6 * Mb;

	static const uint32_t AI_CHUNK_SIZE		= 512 * Kb;
	static const uint32_t CAMERA_CHUNK_SIZE	= 256 * Kb;
	static const uint32_t EFFECT_CHUNK_SIZE	= 128 * Kb;
	static const uint32_t PROC_CHUNK_SIZE	= 1 * Mb;
};

#endif

