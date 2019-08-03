//------------------------------------------------------
//!
//! \file army/battelion.h
//!	A Battalion is a group of units that work towards a common task/goal directed
//! by this armies General.
//!
//------------------------------------------------------

#ifndef BATTALION_H_
#define BATTALION_H_

//**************************************************************************************
//	Includes files.
//**************************************************************************************
#if !defined( ARMY_PPU_SPU_H_ )
#include "army/army_ppu_spu.h"
#endif

#if !defined( ARMY_UNIT_H_ )
#include "army/unit.h"
#endif

//**************************************************************************************
//	Forward declarations.
//**************************************************************************************

//**************************************************************************************
//	Typedefs.
//**************************************************************************************


//------------------------------------------------------
//!
//! Battalion Logic - What 'program' this particular battalion
//! is doing
//!
//------------------------------------------------------
enum BATTALION_LOGIC
{
	BL_NOOP					= 0, 
	BL_JUMP					= 1,		//!< param0 = command sequence num to jump to
	BL_HOLD_POSITION		= 2,		//!< param0 = time to wait -1 infinite
	BL_MARCH_TO				= 3,		//!< march to the specified position param0 = speed (0 = 1), param1 = radius (0 = 750)
	BL_SET_SPEED_MULT		= 4,		//!< param0 = percentage multiplier (100 = 1.f) for battalion speed
	BL_MARCH_AND_ROTATE		= 5,
	BL_RESPAWN				= 6,		//!< param0 = command sequence num to jump to (-1 not too)
	BL_SET_COHERSION		= 7,		//!< param0 = % cohersion factor (30 = normal)
	BL_TAUNT				= 8,		//!< param0 = time to wait jeering -1 infinite
	BL_WAIT_ON_FLAG			= 9,		//!< param0 = flag number (currently 0-15)
	BL_STAGING_POST			= 10,		//!< param0 = 'line' number param1 = flag number
	BL_SET_PLAYER_TRACKING	= 11,		//!< param0 = player tracking on/off
	BL_INTERUPT_RET			= 12,		//!< return from an interupt command patch
	BL_EXPLODE				= 13,		//!< the battalion 'explode' for bombermen
	BL_SET_PLAYER_CIRCLE	= 14,		//!< param0 = player circling on/off.
};


//------------------------------------------------------
//!
//! Formation Order is a measure of how tightly controlled this 
//! battalion is. 
//!
//------------------------------------------------------
enum BATTALION_FORMATION_ORDER
{
	//! No order and no control
	BFO_DISORDER,	
	//! Little structure but some basic order 
	BFO_BARBARIAN,	
	//! Semi-regular, grunts keep basic structure but
	//! not tight exact structures
	BFO_SEMI_REGULAR,
	//! Regular, each grunt keeps his place very precisely
	//! and does what his commander says fairly accurately
	BFO_REGULAR,
};

//------------------------------------------------------
//!
//! Orders, are the overall control we are doing
//! grunts read this as there commanders orders
//!
//------------------------------------------------------
enum BATTALION_ORDERS
{
	BO_NO_CONTROL,
	BO_FORWARD_MARCH,
	BO_HOLD_POSITION,
	BO_RETREAT,
	BO_RUN_FOR_YOUR_LIVES,
	BO_ATTACK,
	BO_CHARGE,
	BO_ROTATE,
	BO_RESPAWN,
	BO_TAUNT,
	BO_EXPLODE,
	BO_RUN_TO_CIRCLE
};

//------------------------------------------------------
//!
//! a few 'standard' shapes and probably lots of custom
//! section specific ones...
//!
//------------------------------------------------------
enum BATTALION_SHAPE
{
	//! a N x M line N = horizontal 
	//! each units in different lines
	BS_ORDERED_LINES,
	BS_UNORDERED_SQUARE
};

//------------------------------------------------------
//!
//! a enum for each unit type... 16 MAX cos we use a 
//! nibble
//!
//------------------------------------------------------
enum BATTALION_UNIT_TYPE
{
	BUT_FODDER			= 0x0,
	BUT_SWORDSMAN		= 0x1,
	BUT_SHIELDSMAN		= 0x2,
	BUT_CHAINMAN		= 0x3,
	BUT_COMMANDER		= 0x4,
	BUT_AXEMAN			= 0x5,
	BUT_ORANGUMAN		= 0x6,
	BUT_NINJA			= 0x7,
	BUT_CROSSBOWMAN		= 0x8,
	BUT_BAZOOKAMAN		= 0x9,
	BUT_COMRADES		= 0xA,
	BUT_BOMBERMAN		= 0xB,

	BUT_MAX = MAX_UNIT_TYPES, //!<really serious! don't go over this!!!
};

// 8 bit for various usage
enum BATTALION_FLAGS
{
	BF_AT_STAGING_POST		= (1 << 0),				// are we at our staging post
	BF_PLAYER_TRACKING		= (1 << 1),				// do we want to player track
	BF_IN_INTERUPT			= (1 << 2),				// currently interupting
	BF_RELATIVE_COMMANDS	= (1 << 3),				// are our battalion commands relative 
	BF_PLAYER_ATTACK		= (1 << 4),				// do we ever want to attack the player
	BF_PLAYER_CIRCLE		= (1 << 5),							// do we want to circle the player
};

//------------------------------------------------------
//!
//! unit arranged in ordered lines, each line in grunts is
//! m_Width wide and m_NumLines*m_DepthPerLine deep
//! You have upto MAX_UNITS_PER_BATTALIONS in the shape
//! and the m_UnitInLine array controls which unit makes 
//! up each line
//! through various combination (some not obvious I admit)
//! you can have a solid block of 1 unit of 256 x 192 grunts
//! alternating lines of 2 unit of upto 256 wide x 12 deep
//! or even more complex patterins (2 of 1 unit followed by 
//! 2 of another etc)...
//! NOTE : In practise other limiations will hit first 
//! (grunts per unit and total number of grunts)
//!
//------------------------------------------------------
struct ShapeOrderedLineData
{
	uint16_t m_Width : 8;			//!< how many grunts per line MAX 256
	uint16_t m_NumLines : 4;		//!< how many lines
	uint16_t m_DepthPerLine : 4;	//!< how many grunts deep per line

	//! unit type is a nibble so we get 2 per bytes (MAX 4 units per battalion)
	uint8_t m_UnitInLine[ MAX_UNITS_PER_BATTALION / 2];

	uint16_t m_OffsetAlternateLines : 1;
	uint16_t m_dummy : 15; //  makes 8 bytes
}; // 8 bytes total

//------------------------------------------------------
//!
//! grunts are distrubuted in a square with a simple
//! percentage unit type distrubution
//!
//------------------------------------------------------
struct ShapeUnorderedSquare
{
	uint16_t m_NumUnitTypes : MAX_UNITS_PER_BATTALION;		// 2 bits
	uint16_t m_NumGrunts	: 10; // upto 1024 grunts (way too much for a single battalion!)
	uint16_t m_dummyX		: (16 - 10 - MAX_UNITS_PER_BATTALION); // spare


	// m_NumUnitTypes of 
	//		a nibble of unit type 
	uint8_t m_UnitType[ MAX_UNITS_PER_BATTALION / 2 ];	// 2 bytes
	uint8_t m_Percentage[MAX_UNITS_PER_BATTALION]; // 4 bytes

}; // 8 bytes total

//------------------------------------------------------
//!
//! for the battalion virtual machine, this is a single
//! machine instruction. 
//!
//------------------------------------------------------
struct BattalionCommand
{
	BF_Position m_Pos;			//!< where is this commands position total 4 bytes
	uint16_t m_iParam0;			//!< a 16 int param unique to each command 0xFFFF = -1/INVALID but rest all positive...
	uint16_t m_iCommand : MAX_BATTALION_COMMAND_INSTR_BITS;	//!< 6 bits for the command = 64 possible commands 
	uint16_t m_iParam1 : MAX_BATTALION_COMMAND_PARAM1_BITS;	//!< an extra 10 bit param (val must be <1024) always positive
}; // total 8 bytes

//------------------------------------------------------
//!
//! Battalion have upto N units embedded
//! essentially this and battlefield header are the
//! constant data accessable to all SPU army code.. so
//! make sure its small.
//! Each battalion is 256 bytes in size, we can have 4K worth
//! of them at any one time
//!
//------------------------------------------------------
struct Battalion
{
	uint16_t			m_CommandNum : MAX_BATTALION_COMMAND_INSTR_BITS;		//!< what command we are currently running
	uint16_t			m_Orders : 6;			//<! our orders (how many bits? 6 lets us fit shape here
	uint16_t			m_Shape				: 4;	//!< what ckind of shape

	uint16_t			m_FormationOrder	: 4;								//!< how tightly we hold our shape					4 total 4 bits
	uint16_t			m_BattalionID		: MAX_BATTALIONS_BITS;				//!< our own battalion ID mainly for debugging		6 total 10 bits
	uint16_t			m_iNumUnits			: MAX_UNITS_PER_BATTALION_BITS;		//!< how many units									3 total 13 bits
	uint16_t			m_dummybit			: 3;								//													3 total 16 bits

	union
	{
		uint16_t			m_timeDecr;			// some orders use a count down timer, this is it
		uint16_t			m_curParam0;		// we copy param0 
	};

	uint16_t	m_iNumGrunts;					// how many grunts in this battalion

	union ShapeUnion
	{
		ShapeOrderedLineData			m_OrderedLine;
		ShapeUnorderedSquare			m_UnorderedSquare;
	} m_ShapeData;

	BF_Position		m_TopLeft;		//!< the rough extents of the battalion (only when the battalion is roughly ordered)
	BF_Position		m_BotRight;		//!< the rough extents of the battalion (only when the battalion is roughly ordered)

	uint8_t			m_BattalionSpeed;	//!< when are regimented, we can only move at the speed of the slowest unit this is that


	uint8_t			m_BattalionFlags;	//!< some interesting flags
	uint16_t		m_BattalionRotation;

	BF_Position		m_POI;				//!< a point of interest, to grunts (dependant on state)

	BF_Position		m_RelativePnt;		//!< for relative commands this is our parents position we are relative to

	float			m_BattalionSpeedPercentageMultiplier;
	float			m_BattalionCohersionFactor;

	float			m_fMinInnerPlayerTrackRadius;							//!< inner player track radius neither need to be float
	float			m_fMaxOuterPlayerTrackRadius;							//!< outer player track radius 

	float			m_MinCirclePlayerRadius;								//!< Radius of the circle used in the "circle-player" state.

	BattalionCommand m_InteruptStack[3];									//!< enough space to store the pre-interupt commands
	uint16_t		m_InteruptReg;											//!< saved register

	Unit			m_Units[MAX_UNITS_PER_BATTALION];		// total 64 bytes
	BattalionCommand	m_BattalionCommands[ MAX_BATTALION_COMMANDS +2 ] ; // total 16+2 * 8 bytes (extra 2 for interupt space)
} __attribute__ ( (aligned( 256 )) ); 

//------------------------------------------------------
//!
//! BattalionArray must be DMA'able
//!
//------------------------------------------------------
struct BattalionArray
{
	uint8_t			m_iNumBattalions;
	uint8_t			m_padd8[3];

	Battalion		m_BattalionArray[ MAX_BATTALIONS ];
} __attribute__ ( (aligned( 128 )) );

#endif	// !BATTALION_H_
