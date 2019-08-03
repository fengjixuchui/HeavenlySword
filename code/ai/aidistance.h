#ifndef _AIDISTANCE_H_
#define _AIDISTANCE_H_

namespace AIDistance
{
	CPoint ClosestPointOnLine( const CPoint& a, const CPoint& b, const CPoint& p )
	{
		CPoint c = p - a;
		CPoint V = b - a;

		float d = V.Length();

		V /= d;
		float t = V.Dot( c );

		// if t is beyond the extents of the segment, return the nearest endpoint
		if (t < 0.0f)	{ return a; }
		if (t > d)		{ return b; }

		// Return the point between a and b
		//set length of V to t. V is normalized so this is easy
		V.X() = V.X() * t;
		V.Y() = V.Y() * t;
		V.Z() = V.Z() * t;

		return CPoint( a + V );
	}

	CPoint ClosestPointOnTriangle( const CPoint& a, const CPoint& b, const CPoint& c, const CPoint& p, float* dist = NULL )
	{
		CPoint closestAB = ClosestPointOnLine( a, b, p );
		CPoint closestBC = ClosestPointOnLine( b, c, p );
		CPoint closestCA = ClosestPointOnLine( c, a, p );

		float distAB = (p - closestAB).Length();
		float distBC = (p - closestBC).Length();
		float distCA = (p - closestCA).Length();

		float min = distAB;
		CPoint& result = closestAB;

		if (distBC < min)
		{
			min = distBC;
			result = closestBC;
		}

		if (distCA < min)
		{
			min = distCA;
			result = closestCA;
		}

		if (dist)	{ *dist = min; }
		return result;
	}
}

#endif // end of _AIDISTANCE_H_
