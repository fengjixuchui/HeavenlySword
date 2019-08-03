/***************************************************************************************************
*
*	DESCRIPTION		A set of directions and associated sectors in 2d.
*
*	NOTES
*
***************************************************************************************************/

//**************************************************************************************
//	Includes files.
//**************************************************************************************
#include "DirectionSet2D.h"

//**************************************************************************************
//
//**************************************************************************************
namespace DirectionSet2D_Private
{
	static const Sector2D Sectors[ NUM_DIRECTIONS ] =
	{
		Sector2D( 0.0f, 0.0f, 10000.0f,  0.0f,  1.0f, 45.0f * DEG_TO_RAD_VALUE ),	// NN
		Sector2D( 0.0f, 0.0f, 10000.0f, -1.0f,  1.0f, 45.0f * DEG_TO_RAD_VALUE ),	// NE
		Sector2D( 0.0f, 0.0f, 10000.0f, -1.0f,  0.0f, 45.0f * DEG_TO_RAD_VALUE ),	// EE
		Sector2D( 0.0f, 0.0f, 10000.0f, -1.0f, -1.0f, 45.0f * DEG_TO_RAD_VALUE ),	// SE
		Sector2D( 0.0f, 0.0f, 10000.0f,  0.0f, -1.0f, 45.0f * DEG_TO_RAD_VALUE ),	// SS
		Sector2D( 0.0f, 0.0f, 10000.0f,	 1.0f, -1.0f, 45.0f * DEG_TO_RAD_VALUE ),	// SW
		Sector2D( 0.0f, 0.0f, 10000.0f,  1.0f,  0.0f, 45.0f * DEG_TO_RAD_VALUE ),	// WW
		Sector2D( 0.0f, 0.0f, 10000.0f,  1.0f,  1.0f, 45.0f * DEG_TO_RAD_VALUE )	// NW
	};

	static const char * const DirectionNames[ NUM_DIRECTIONS ] =
	{
		"NN",
		"NE",
		"EE",
		"SE",
		"SS",
		"SW",
		"WW",
		"NW"
	};
}

//**************************************************************************************
//
//**************************************************************************************
bool DirectionSet2D::IsInside( const CPoint &P ) const
{
	for ( int d=0;d<NUM_DIRECTIONS;d++ )
	{
		if ( HasDirection( Directions( d ) ) )
		{
			if ( DirectionSet2D_Private::Sectors[ d ].IsInside( P.X(), P.Z() ) )
			{
				return true;
			}
		}
	}

	return false;
}

//**************************************************************************************
//
//**************************************************************************************
DirectionSet2D::DirectionSet2D( const char * const direction_string )
:	m_Directions( 0 )
{
	for ( int i=0;i<NUM_DIRECTIONS;i++ )
	{
		if ( strstr( direction_string, DirectionSet2D_Private::DirectionNames[ i ] ) != NULL )
		{
			AddDirection( Directions( i ) );
		}
	}
}







