/***************************************************************************************************
*
* Defines and constants that control the flow of PPU and SPU army code
* Also stuck some really base types that everything in the army need...
*
***************************************************************************************************/

#ifndef ARMY_PPU_SPU_H_
#define ARMY_PPU_SPU_H_

//! maximum number of battlefield chunks we can possible have (this is enough for 2048x2048 with each chunk being 64x64
#define MAX_BATTLEFIELD_CHUNKS_BITS			10
#define MAX_GRUNTS_BITS						12
#define MAX_BATTALIONS_BITS					6
#define MAX_UNITS_PER_BATTALION_BITS		2
#define MAX_UNIT_TYPES_BITS					4
#define MAX_BATTALION_COMMANDS				16
#define MAX_BATTALION_COMMAND_INSTR_BITS	6
#define MAX_BATTALION_COMMAND_PARAM0_BITS	16
#define MAX_BATTALION_COMMAND_PARAM1_BITS	10
#define MAX_GRUNT_MAJOR_STATE_BITS			4
#define MAX_GRUNT_ANIM_STATE_BITS			4
#define MAX_GRUNT_RENDER_TYPE_BITS			2
#define MAX_RENDERSTATECHANGECOUNTER_BITS	4
#define MAX_STATIC_AI_POLYGONS				4

// enough for 16 types of events
#define NUM_BFE_BITS						4
// event radius can be the entire half the battlefield (so almost cover the whole thing...)
#define MAX_BFE_RADIUS_BITS					15

#define MAX_BATTLEFIELD_CHUNKS			(0x1 << MAX_BATTLEFIELD_CHUNKS_BITS)
#define MAX_GRUNTS						(0x1 << MAX_GRUNTS_BITS)
// we only actually allow 15 so that that whole battalion array is < 4KB
#define MAX_BATTALIONS					((0x1 << MAX_BATTALIONS_BITS)-1)
#define MAX_UNITS_PER_BATTALION			(0x1 << MAX_UNITS_PER_BATTALION_BITS)
#define MAX_UNIT_TYPES					(0x1 << MAX_UNIT_TYPES_BITS)
#define MAX_BATTALION_COMMAND_INSTR		(1 << MAX_BATTALION_COMMAND_INSTR_BITS)
#define MAX_BATTALION_COMMAND_PARAM0	(1 << MAX_BATTALION_COMMAND_PARAM0_BITS)
#define MAX_BATTALION_COMMAND_PARAM1	(1 << MAX_BATTALION_COMMAND_PARAM1_BITS)
#define MAX_GRUNT_ANIM_STATES			(1 << MAX_GRUNT_ANIM_STATE_BITS)

#define MAX_RENDERSTATECHANGECOUNTER_COUNTER ((1 << MAX_RENDERSTATECHANGECOUNTER_BITS)-1)

#define MAX_BATTLEFIELD_EVENTS			16
#define MAX_BATTLEFIELD_OBSTACLES		48
#define NIBBLE_EXTRACT_BOTTOM(x)	((x) & 0xF)
#define NIBBLE_EXTRACT_TOP(x)		(((x) >> 4) & 0xF)

// x=bottom | y=top
#define NIBBLE_PACK(x,y)			(((x) & 0xF) | (((y) & 0xF) << 4))

//------------------------------------------------------
//!
//! this is a compressed position used for things on 
//! the battlefield. Its 2 16 bit ints which are normalised
//! indices into the battlefield.
//!
//------------------------------------------------------
struct BF_Position
{
	uint16_t m_X;
	uint16_t m_Y;
};

//------------------------------------------------------
//!
//! a 16 bit normalised angle 
//! 0		= + the battle field Y
//! 0.25	= + the battle field X
//! 0.5		= - the battle field Y
//! 0.75	= - the battle field X
//! 1.0		= + the battle field Y
//!
//------------------------------------------------------
struct BF_Orientation
{
	uint16_t m_Rotation;
};

//------------------------------------------------------
//!
//! Currently I have one SPU elf with multiple sub
//! tasks in it... this determines which one, if code
//! space gets to be a problem we will move to multiple
//! elves
//!
//------------------------------------------------------
enum SPU_RUN_TYPE
{
	BATTALION_UPDATE,
	GRUNT_CHUNKIFY_AND_SORT,
	GRUNT_LOGIC_EXECUTE,
	UNIT_RENDER,
};

// some parameter are aliased, so be careful kids!!

// these 3 are used by all army_spu task
#define SRTA_RUN_TYPE					0
#define SRTA_BATTALION_ARRAY			1
#define SRTA_BATTLEFIELD_HEADER			2

// GRUNT_CHUNKIFY_AND_SORT needs grunts
// GRUNT_LOGIC_EXECUTE needs grunts
// UNIT_RENDER needs grunts 
// BATTALION_UPDATE need grunts tho last frame is okay
#define SRTA_GRUNTS						3

// GRUNT_CHUNKIFY_AND_SORT needs all the chunk headers
#define SRTA_BATTLEFIELD_CHUNKS_HEADER	4
// GRUNT_LOGIC_EXECUTE needs the chunk itself
#define SRTA_BATTLEFIELD_CHUNK			4
// BATTALION_UPDATE need battalion ID
#define SRTA_BATTALION_ID				4
// UNIT_RENDER needs the army render allocator
#define SRTA_GRUNT_RENDER_ALLOCATOR		4

// GRUNT_LOGIC_EXECUTE needs the first grunt index for its chunk
#define SRTA_FIRST_GRUNT_INDEX			5
// GRUNT_CHUNKIFY_AND_SORT needs the army job adder
#define SRTA_ARMY_JOB_ADDER				5
// UNIT_RENDER needs to output closegrunts somewhere
#define SRTA_RENDER_GRUNTS				5


// GRUNT_CHUNKIFY_AND_SORT needs a barrier job installed from the SPU
// so that stuff doesn't start to get rendered before we've finished
// updating it! :)
#define SRTA_ARMY_BARRIER_EA			6

//---------
// We need to send up some graphics EA to be passed on
// I'm packing them in the space m_data of RenderJobAdder
// this are indice
#define RJAP_DEST_GRUNTS_EA				0


enum BATTLEFIELD_EVENT_TYPE
{
	BET_PLAYER_TRACKER	= 0,
	BET_EXPLOSION,
	BET_BAZOOKA_SHOT,
	
	BET_SYNC_ATTACK_WAKE,
	BET_SPEED_ATTACK_WAKE,
	BET_POWER_ATTACK_WAKE,
	BET_RANGE_ATTACK_WAKE
};

#endif // ARMY_PPU_SPU_H_
