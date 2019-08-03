/***************************************************************************************************
*
*	DESCRIPTION		Simple class to encapsulate a circle in two dimensions.
*
*	NOTES
*
***************************************************************************************************/

#ifndef CIRCLE2D_H_
#define CIRCLE2D_H_

//**************************************************************************************
//	Includes files.
//**************************************************************************************
#include "core/boundingvolumes.h"

//**************************************************************************************
//	Typedefs.
//**************************************************************************************

//**************************************************************************************
//	
//**************************************************************************************
class Circle2D
{
	public:
		//
		//	Useful testing functions.
		//
						// Returns true if the point (x,y) is inside the circle.
		inline bool		IsInside	( float x, float y )		const;

						// Returns true if the passed circle intersects this circle.
		inline bool		Intersects	( const Circle2D &circle )	const;

	public:
		//
		//	Return AABB of this circle.
		//
		inline void		CreateAABB	( CAABB &aabb )				const;

	public:
		//
		//	Helper to create a 3D version of the centre point on the XZ plane.
		//
		inline CPoint	Get3DCentre	()							const;

	public:
		//
		//	Ctors.
		//
		Circle2D() {}
		Circle2D( float cx, float cy, float r )
		:	m_Cx			( cx )
		,	m_Cy			( cy )
		,	m_Radius		( r )
		,	m_RadiusSquared	( r * r )
		{}

	public:
		//
		//	Public data members.
		//
		float m_Cx, m_Cy;		// R^2 centre position.
		float m_Radius;
		float m_RadiusSquared;
};

//**************************************************************************************
//	
//**************************************************************************************
bool Circle2D::IsInside( float x, float y ) const
{
	float dx( x - m_Cx );
	float dy( y - m_Cy );

	return ( dx*dx + dy*dy ) <= m_RadiusSquared;
}

//**************************************************************************************
//	
//**************************************************************************************
bool Circle2D::Intersects( const Circle2D &circle ) const
{
	float dx( circle.m_Cx - m_Cx );
	float dy( circle.m_Cy - m_Cy );

	float radii_sum( m_Radius + circle.m_Radius );

	return ( dx*dx + dy*dy ) <= radii_sum * radii_sum;
}

//**************************************************************************************
//	
//**************************************************************************************
void Circle2D::CreateAABB( CAABB &aabb ) const
{
	aabb.Min() = CPoint( m_Cx - m_Radius, 0.0f, m_Cy - m_Radius );
	aabb.Max() = CPoint( m_Cx + m_Radius, 0.0f, m_Cy + m_Radius );
}

//**************************************************************************************
//	
//**************************************************************************************
CPoint Circle2D::Get3DCentre() const
{
	return CPoint( m_Cx, 0.0f, m_Cy );
}

#endif	// !CIRCLE2D_H_

