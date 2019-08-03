/***************************************************************************************************
*
*	DESCRIPTION		A unit consists of number of a a single type of grunts under command of a 
*					Battalion
*
*	NOTES
*
***************************************************************************************************/

#ifndef ARMY_UNIT_H_
#define ARMY_UNIT_H_

//**************************************************************************************
//	Includes files.
//**************************************************************************************
#if !defined( ARMY_PPU_SPU_H_ )
#include "army/army_ppu_spu.h"
#endif

//**************************************************************************************
//	Forward declarations.
//**************************************************************************************

//**************************************************************************************
//	Typedefs.
//**************************************************************************************

//------------------------------------------------------
//!
//! Unit Logic - What 'program' this particular unit
//! is doing
//!
//------------------------------------------------------
enum UNIT_LOGIC
{
	UL_WAITING_TO_ATTACK,	//!< the army is held back waiting to attack
};

//------------------------------------------------------
//!
//! Unit represent the higher level logic for a group of 
//! grunts of the same type (bar gfx variations).
//!
//------------------------------------------------------
struct Unit
{
	uint32_t		m_NumGrunts		: MAX_GRUNTS_BITS;	//!< how many grunts in this unit (id's must be continous)			12b
	uint32_t		m_FirstGruntID	: MAX_GRUNTS_BITS;	//!< in the grunt index table units are continous so who's first?	24b
	uint32_t		m_BattalionID	: MAX_BATTALIONS_BITS;				//!< which battalion we belong to					30b
	uint32_t		m_padd32bits0	: 2;								//!< spare

	uint32_t		m_UnitType		: MAX_UNIT_TYPES_BITS;				//!< our type										4b	
	uint32_t		m_UnitID		: MAX_UNITS_PER_BATTALION_BITS;		//!< which unit this grunt belongs to				7b
	uint32_t		m_RunSpeed		: 6;								//!< 0-64 units per frame							13b	
	uint32_t		m_WalkSpeed		: 6;								//!< 0-64											19b
	uint32_t		m_DiveSpeed		: 6;								//!< 0-64											25
	uint32_t		m_PersonalSpace	: 5;								//!< 4.1 fixed point how fat this type is (radii)	30b
	uint32_t		m_padd32bits	: 2;								//!< spare

	float			m_fInnerPlayerTrackRadius;							//!< inner player track radius neither need to be float
	float			m_fOuterPlayerTrackRadius;							//!< outer player track radius

	float			m_CirclePlayerRadius;								//!< radius for player-circling behaviour.

}; // 16 bytes each

#endif	// !UNIT_H_

