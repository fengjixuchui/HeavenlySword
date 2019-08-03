#if !defined( HERESY_CAPI_H )
#define HERESY_CAPI_H

#if defined( __cplusplus )
extern "C"
{
#endif

#if !defined( HERESY_HARDWARE_H )
#include "heresy_hardware.h"
#endif

#if !defined( CORE_DOUBLEENDERFRAMEALLOCATOR_H )
#include "core/doubleenderframeallocator.h"
#endif

#if !defined( __SPU__ )
#define USE_ICE_AS_BACKEND_TO_HERESY
#endif

#define HERETIC_NUM_PB_MARKERS			4

#define HPBP_PIXEL_CONSTANT_FIXUP			 0x0
#define HPBP_VERTEX_CONSTANT_FIXUP			 0x1
#define HPBP_TEXTURE_FIXUP					 0x2
#define HPBP_PIXEL_SHADER_FIXUP				 0x3
#define HPBP_INSTANCE_JUMP_PRE_DRAW_FIXUP	 0x4
#define HPBP_INSTANCE_JUMP_POST_DRAW_FIXUP	 0x5
#define HPBP_UNUSED_FIXUP0					 0x6
#define HPBP_UNUSED_FIXUP1					 0x7

#define HPBP_DATA_MASK				~0x7
#define HPBP_FIXUP_TYPE_MASK		0x7
#define HPBP_DATA_MASK_LENGTH		0x3

#define HPBP_PIXEL_SHADER_SIZE_MASK ~0xC000

typedef struct Heresy_PushBufferPatchT
{
	uint16_t m_Semantic;		//!< what we want to stick the offset provided
	uint16_t m_offset;			//!< offset from start of push buffer (in words so max addressable range is 64Kb*4 = 256Kb
	uint32_t m_iData;			//!< Start of some user data the NOTE the bottom two bits have the fixup type, so you only have 30 bits (enough for a pointer)
} Heresy_PushBufferPatch;

typedef struct Heresy_PushBufferT
{
	uint32_t* m_pStart;
	uint32_t* m_pEnd;
	uint32_t* m_pCurrent;

	void*	m_pDummy; // used for ice command context fast to'ing and fro'ing

	uint16_t m_iNumPatches; //!< how many patches have we already used for this push buffer
	uint16_t m_iMaxPatches; //!< how many Heresy_PushBufferPatch follow this structure for patches (potentially could be a constant...) 
							//!< m_iScratch follows MaxPatches and is copied per frame to the double buffer XDDR space

	uint16_t m_iCurScratch; //!< in word so max scratch 64Kb*4= 256Kb
	uint16_t m_iMaxScratch;	//!< How much scratch follows MaxPatches (in 4 bytes) for a max of 256Kb

	uint16_t m_iMarker[HERETIC_NUM_PB_MARKERS];	//!< HERETIC_NUM_MARKERS (4) offset markers
							
	//! our structure currently 256 bits with 4 markers (for 2 SPU registers)
} Heresy_PushBuffer;

//! for now this should exactly match Ice::Render::VertexProgram and ideally a nice static_assert somewhere in you code would enforce this...
//! however if it makes sense to change for SPU PB creation, then we will have to munge Vertex shaders binary format...
typedef struct Heresy_VertexShaderT
{
	uint16_t m_instructionCount;
	uint16_t m_instructionSlot;
	// 4 bytes

	uint32_t m_vertexAttributeMask;
	uint32_t m_vertexResultMask;
	uint32_t m_vertexLimits;
	// 12 + 4 = 16 bytes

	uint32_t m_microCodeOffset;
	uint32_t m_constantCount;
	// 8 + 16 = 24 bytes

	// followed by constants here
} Heresy_VertexShader;

typedef struct Heresy_PixelShaderT
{
	//! The microcode size, in bytes.
	uint32_t m_microcodeSize;
	//! The mask of texcoords that have centroid sampling enabled. Coordinate 0 is the LSB.
	uint16_t m_centroidMask;
	//! The mask of texcoords that are 2D only. Coordinate 0 is the LSB.
	uint16_t m_texcoordMask;
	//! Packed information about various state.
	uint32_t m_control;
	//! The offset of the microcode in the context. LSB is the context Ice::Render::FragmentProgramContext.
	uint32_t m_offsetAndContext;
	//! Offset to microcode, in bytes, relative to beginning of this struct.
	uint32_t m_microcodeOffset;
	//! Number of immediate patches ('fragment program constants').
	uint32_t m_patchCount;	
	//! Each Ice patch (of which ther are m_patchCount) consist of a uint16_t offset count and then offsets. 32 bit padded. so 
	//! 'constant number' is actuall just an index to the patch table you get the patch by ((uint32_t*)(pixelshader+1)) + index
} Heresy_PixelShader;

typedef struct Heresy_IcePatchDataT
{
	// how many places to we have patch
	uint16_t m_count;
	// followed by m_count unsigned 16 bit offsets
} Heresy_IcePatchData;

//! This is based on the Ice/libgcm model which defines a texture as having an implicit filter and wrap/clamp state
//! however we don't... so for us it may well be better to seperate the 'texture' set (offset, format) from the sampler
//! state ala D3D which might save a few redudent register set... i.e. we currently almost always set address twice
typedef struct Heresy_TextureT
{
	uint32_t m_baseOffset;			// Register TEXTURE0_OFFSET
	uint32_t m_format;				// Register TEXTURE0_FORMAT
	uint32_t m_control1;			// Register TEXTURE0_ADDRESS
	uint32_t m_control2;			// Register TEXTURE0_CONTROL
	uint32_t m_swizzle;				// Register TEXTURE0_REMAP
	uint32_t m_filter;				// Register TEXTURE0_FILTER
	uint32_t m_size1;				// Register TEXTURE0_SIZE_WH
	uint32_t m_borderColor;			// Register TEXTURE0_BORDER_COLOUR
	uint32_t m_size2;				// Register TEXTURE0_SIZE_DP
	// 1st batch 32 bytes = 10 registers

	uint32_t m_control3;			// Register TEXTURE0_BRILINEAR
	// 2nd batch 4 bytes = 1 register

	uint32_t m_colorKeyColor;		// Register TEXTURE0_COLOURKEYCONTROL
	// 3rd batch 4 bytes = 1 register
} Heresy_Texture;

typedef struct Heresy_GlobalDataT
{
	// these are used to compute the memory
	uint32_t m_RSXMainBaseAdjust;	// this is used to move from a main memory address to a RSX offset for memory allocated from Mem::MC_RSX_MAIN_INTERNAL
	uint32_t m_IceVramOffset;

	// note this is single threaded safe only... so each thread requires its own global data with its own pushbuffer allocator
	// not very global at the mo... this is a fairly rare op though, so might benefit from a thread safe alloctor hmmm...
	DoubleEnderFrameAllocatorC	m_PushBufferAllocator;

} Heresy_GlobalData;

// the problem with C and SPU code is that we don't want to link to ICE libraries for the simple functions...
// so I take a 'pragmetic' view, for longer complex function, as I simple thunk through to Ice (which won't work on SPU)
// for smaller more commonly use function I have macros that don't use ICE, so are independent of the libs etc.

// the 'safe' option is to call the ice function from a cpp file, but of course extra function overhead (as we can't inline the C++ code)
// however there is current no SPU ice, so no safe stuff for us when needed there
#if defined(USE_ICE_AS_BACKEND_TO_HERESY)

// sets up the provided vertex shader into the input pushbuffer
void Heresy_SetVertexShader( Heresy_PushBuffer* pPB, const Heresy_VertexShader* pProgram );

// sets up the provided pixel shader into the input pushbuffer
void Heresy_SetPixelShader( Heresy_PushBuffer* pPB, const Heresy_PixelShader* pProgram );

//---
// I've choose for now to just use ICE pixel shader constant functions cos of the complexity but this is an obvious candidate to do on SPU
void Heresy_SetPixelShaderConstant4F( restrict Heresy_PushBuffer* pb, Heresy_PixelShader* pProgram, uint32_t constantnum, const float * restrict constant );


//---
// currently now VRAM allocator, we use ICE
void* Heresy_AllocatePixelShaderSpaceInGDDR( uint16_t space );

// just a dummy to help ICE work
uint32_t Heresy_DummyCommandFillCallback( void*, uint32_t );

// setup function for initilising from Ice and libgcm
void Heresy_InitGlobalData( Heresy_GlobalData* pGlobal );

#endif

// c macros for heresy... potentially allow the very very fastest including array filling but lightly used for now
#include "heresy_cmacros.h"

// c inlines where most of the good stuff lives
#include "heresy_cinline.h"


#if defined( __cplusplus )
}
#endif

#endif
