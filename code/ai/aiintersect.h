#ifndef _AIINTERSECT_H_
#define _AIINTERSECT_H_

namespace AIIntersect
{
	inline bool PointInAABB( const CPoint& pos, const CPoint& centre, const CPoint& extents )
	{
		const CPoint diff = (pos - centre).Abs();
		return 
			diff.X() <= fabs(extents.X())
			&&
			diff.Y() <= fabs(extents.Y())
			&&
			diff.Z() <= fabs(extents.Z());
	}

	static bool _SameSide( const CPoint& p1, const CPoint& p2, const CPoint& l1, const CPoint& l2 )
	{
		// returns true if p1 and p2 are on the same side of the line through l1, l2
		bool bSame =	(	((p1.X() - l1.X()) * (l2.Z() - l1.Z()) - (l2.X() - l1.X()) * (p1.Z() - l1.Z()))
					*		((p2.X() - l1.X()) * (l2.Z() - l1.Z()) - (l2.X() - l1.X()) * (p2.Z() - l1.Z()))	) > 0;

		return bSame;
	}

	inline bool PointInTriangle( const CPoint& p, const CPoint& a, const CPoint& b, const CPoint& c )
	{
		if (	(p.X() < min( a.X(), min( b.X(), c.X() ) )) || (p.Z() < min( a.Z(), min( b.Z(), c.Z() ) ))
			||	(p.X() > max( a.X(), max( b.X(), c.X() ) )) || (p.Z() > max( a.Z(), max( b.Z(), c.Z() ) )) )
		{
			// outside bounding box
			return false;
		}

		// test point against triangle planes
		return ( _SameSide( p, a, b, c ) && _SameSide( p, b, a, c ) && _SameSide(p, c, a, b) );
	}
}


#endif // end of _AIINTERSECT_H_
