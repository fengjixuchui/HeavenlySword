/***************************************************************************************************
*
*	DESCRIPTION		Simple class to encapsulate a sector of a circle in two dimensions.
*
*	NOTES
*
***************************************************************************************************/

#ifndef SECTOR2D_H_
#define SECTOR2D_H_

//**************************************************************************************
//	Includes files.
//**************************************************************************************
#include "core/Maths.h"
#include "core/boundingvolumes.h"
#include "Circle2D.h"

//**************************************************************************************
//	Typedefs.
//**************************************************************************************

//**************************************************************************************
//       m_Facing
//          ^
//        --|--
//      |   |   |  <---- supposed to be circular arc :)
//   --     |     --
// |        |        |
//  \       |       / E
//   \      |      /
//    \     |     /
//     \    |    /
//      \   |   /
//       \--|--/ <-|
//        \ | /    |
//         \|/    m_BaseAngle
//          v
//        m_Apex
//
//
//	m_Radius is the distance from m_Apex to point E on the diagram.
//
//**************************************************************************************
class Sector2D
{
	public:
		//
		//	Useful functions.
		//
						// Is (x,y) inside the sector.
		inline bool		IsInside	( float x, float y )	const;

	public:
		//
		//	Return AABB of this sector - N.B. this doesn't give a great bound as it
		//	only returns the AABB of the enclosing circle.
		//
		inline void		CreateAABB	( CAABB &aabb )				const;

	public:
		//
		//	Ctor.
		//
		Sector2D( float apexX, float apexY, float radius,
			      float facingX, float facingY, float angle_in_rads )
		:	m_ApexX				( apexX )
		,	m_ApexY				( apexY )
		,	m_Radius			( radius )
		,	m_FacingX			( facingX )
		,	m_FacingY			( facingY )
		,	m_BaseAngle			( angle_in_rads )
		,	m_CosHalfBaseAngle	( cosf( angle_in_rads / 2.0f ) )
		{
			ntAssert_p( angle_in_rads <= PI, ("Sectors with base angles of more than 180degs are not supported.") );

			float facing_len( sqrtf( m_FacingX*m_FacingX + m_FacingY*m_FacingY ) );
			ntError_p( facing_len > 0.0f, ("You must supply a non-zero facing vector.") );

			// Normalise our facing direction.
			m_FacingX /= facing_len;
			m_FacingY /= facing_len;
		}

	public:
		//
		//	Public data members - explained in comment at top of class.
		//
		float	m_ApexX, m_ApexY;
		float	m_Radius;
		float	m_FacingX, m_FacingY;
		float	m_BaseAngle;				// In Radians.
		float	m_CosHalfBaseAngle;
};

//**************************************************************************************
//	
//**************************************************************************************
bool Sector2D::IsInside( float x, float y ) const
{
	if ( !Circle2D( m_ApexX, m_ApexY, m_Radius ).IsInside( x, y ) )
	{
		return false;
	}

	// The scalar product between our facing direction and a 2d vector
	// from our apex to (x,y) will give us the cosine of the angle
	// between them, if this angle is greater than our base angle then
	// the point is outside the sector.
	float dx( x - m_ApexX );
	float dy( y - m_ApexY );
	float Dlength( sqrtf( dx*dx + dy*dy ) );

	static const float DIST_FUDGE = 0.0001f;
	if ( Dlength > DIST_FUDGE )
	{
		dx /= Dlength;
		dy /= Dlength;
	}

	float cos_angle( dx * m_FacingX + dy * m_FacingY );
	if ( cos_angle <= m_CosHalfBaseAngle )
	{
		// If cos_angle <= cos_half_base_angle then angle > half_base_angle.
		return false;
	}

	return true;
}

//**************************************************************************************
//	
//**************************************************************************************
void Sector2D::CreateAABB( CAABB &aabb ) const
{
	aabb.Min() = CPoint( m_ApexX - m_Radius, 0.0f, m_ApexY - m_Radius );
	aabb.Max() = CPoint( m_ApexX + m_Radius, 0.0f, m_ApexY + m_Radius );
}


#endif	// !SECTOR2D_H_

