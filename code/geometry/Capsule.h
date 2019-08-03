/***************************************************************************************************
*
*	DESCRIPTION		A class to represent a capsule.
*
*	NOTES
*
***************************************************************************************************/

#ifndef CAPSULE_H_
#define CAPSULE_H_

//**************************************************************************************
//	Includes files.
//**************************************************************************************
#include "geometry/Segment.h"

//**************************************************************************************
//	Forward declarations.
//**************************************************************************************

//**************************************************************************************
//	Typedefs.
//**************************************************************************************

//**************************************************************************************
//	
//**************************************************************************************
class Capsule
{
	public:
		//
		//	Get the closest point on the boundary of the capsule to the given point.
		//
		CPoint			GetClosestPoint		( const CPoint &P )		const;

	public:
		//
		//	Work out whether the given point is inside the capsule or not.
		//
		bool			IsInside			( const CPoint &P )		const;

	public:
		//
		//	Accessors.
		//
		const CPoint &	GetPoint			( int i )	const	{ return m_Segment.GetPoint( i ); }

						// Returns length between points.
		float			GetLength			()			const	{ return m_Segment.GetLength(); }
		float			GetLengthSquared	()			const	{ return m_Segment.GetLengthSquared(); }

		const Segment &	GetSegment			()			const	{ return m_Segment; }

		float			GetRadius			()			const	{ return m_Radius; }

	public:
		//
		//	Ctor.
		//
		Capsule() {}

		Capsule( const CPoint &p0, const CPoint &p1, float radius )
		:	m_Segment( p0, p1 )
		,	m_Radius( radius )
		{
		}

	public:
		//
		//	Aggregated members.
		//
		Segment		m_Segment;
		float		m_Radius;
};

#endif	// !CAPSULE_H_

