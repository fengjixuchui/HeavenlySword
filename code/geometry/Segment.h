/***************************************************************************************************
*
*	DESCRIPTION		A class to represent a finite line segment.
*
*	NOTES
*
***************************************************************************************************/

#ifndef SEGMENT_H_
#define SEGMENT_H_

//**************************************************************************************
//	Includes files.
//**************************************************************************************

//**************************************************************************************
//	Forward declarations.
//**************************************************************************************

//**************************************************************************************
//	Typedefs.
//**************************************************************************************

//**************************************************************************************
//	
//**************************************************************************************
class Segment
{
	public:
		//
		//	Given a point, P, find the closest point on the segment to P.
		//
		CPoint			GetClosestPoint		( const CPoint &P )	const;

	public:
		//
		//	Accessors.
		//
		const CPoint &	GetPoint			( int i )	const	{ ntError( i >= 0 && i < 2 ); return m_P[ i ]; }

		float			GetLength			()			const	{ return fsqrtf( m_LengthSquared ); }
		float			GetLengthSquared	()			const	{ return m_LengthSquared; }

		const CDirection &	GetVector		()			const	{ return m_P1_sub_P0; }

	public:
		//
		//	Ctor.
		//
		Segment() {}

		Segment( const CPoint &p0, const CPoint &p1 )
		:	m_P1_sub_P0( p1 ^ p0 )
		,	m_LengthSquared( m_P1_sub_P0.LengthSquared() )
		{
			m_P[ 0 ] = p0;
			m_P[ 1 ] = p1;
		}

	public:
		//
		//	Aggregated members.
		//
		CPoint		m_P[ 2 ];
		CDirection	m_P1_sub_P0;
		float		m_LengthSquared;
};

#endif	// !SEGMENT_H_

