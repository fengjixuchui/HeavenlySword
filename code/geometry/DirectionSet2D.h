/***************************************************************************************************
*
*	DESCRIPTION		A set of directions and associated sectors in 2d.
*
*	NOTES
*
***************************************************************************************************/

#ifndef DIRECTIONSET2D_H_
#define DIRECTIONSET2D_H_

//**************************************************************************************
//	Includes files.
//**************************************************************************************
#include "Sector2D.h"

//**************************************************************************************
//	Typedefs.
//**************************************************************************************

//**************************************************************************************
//	
//**************************************************************************************
enum Directions
{
	NN = 0,
	NE,
	EE,
	SE,
	SS,
	SW,
	WW,
	NW,

	NUM_DIRECTIONS
};

//**************************************************************************************
//	
//**************************************************************************************
class DirectionSet2D
{
	public:
		//
		//	Accessors.
		//
		void	AddDirection	( Directions d )			{ m_Directions |= 1<<d; }
		void	RemoveDirection	( Directions d )			{ m_Directions |= ~(1<<d); }

		bool	HasDirection	( Directions d )	const	{ return ( m_Directions & (1<<d) ) == (1<<d); }

		void	ClearDirections	()							{ m_Directions = 0; }

	public:
		//
		//	Is point P inside any of the sectors associated with this
		//	direction set?
		//
		bool	IsInside		( const CPoint &P )	const;

	public:
		//
		//	Ctor.
		//
		DirectionSet2D()
		:	m_Directions( 0 )
		{}

		// Construct from a comma separated list of direction names
		// (named as the Directions enum members).
		explicit DirectionSet2D( const char * const direction_string );

	private:
		//
		//	A bit-field for the directions.
		//
		int		m_Directions;
};

#endif	// !DIRECTIONSET2D_H_

