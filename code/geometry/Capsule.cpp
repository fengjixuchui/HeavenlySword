/***************************************************************************************************
*
*	DESCRIPTION		A class to represent a capsule.
*
*	NOTES
*
***************************************************************************************************/

//**************************************************************************************
//	Includes files.
//**************************************************************************************
#include "geometry/Capsule.h"

//**************************************************************************************
//	
//**************************************************************************************
CPoint Capsule::GetClosestPoint( const CPoint &P ) const
{
	float t = m_Segment.GetVector().Dot( P ^ m_Segment.GetPoint( 0 ) ) / m_Segment.GetLengthSquared();
	t = ntstd::Max( ntstd::Min( t, 1.0f ), 0.0f );	// Clamp to [0,1].

	CPoint C = m_Segment.GetPoint( 0 ) + t * m_Segment.GetVector();

	if ( t == 0.0f || t == 1.0f )
	{
		CDirection push_out( P - C );
		push_out.Normalise();

		return C + push_out * m_Radius;
	}
	else
	{
		CDirection R = ( m_Segment.GetPoint( 1 ) ^ m_Segment.GetPoint( 0 ) ).Cross( CDirection( 0.0f, 1.0f, 0.0f ) );
		R.Normalise();

		float delta = R.Dot( C ^ P ) >= 0.0f ? 1.0f : -1.0f;

		return C + delta * m_Radius * R;
	}
}

//**************************************************************************************
//	
//**************************************************************************************
bool Capsule::IsInside( const CPoint &P ) const
{
	return ( P ^ GetClosestPoint( P ) ).LengthSquared() < m_Radius*m_Radius;
}




