/***************************************************************************************************
*
*	DESCRIPTION		Simple class to encapsulate a ray.
*
*	NOTES
*
***************************************************************************************************/

#ifndef RAY_H_
#define RAY_H_

//**************************************************************************************
//	Includes files.
//**************************************************************************************

//**************************************************************************************
//	Typedefs.
//**************************************************************************************

//**************************************************************************************
//	
//**************************************************************************************
class Ray
{
	public:
		//
		//	Evaluate a position along the ray by specifying a parameter.
		//	Uses the formula result = S + t*D.
		//
		CPoint				Evaluate		( float t )	const	{ return m_S + m_D * t; }

	public:
		//
		//	Accessors.
		//
		const CPoint &		GetOrigin		()			const	{ return m_S; }
		const CDirection &	GetDirection	()			const	{ return m_D; }

	public:
		//
		//	Distance from ray to point in space.
		//
		float				Distance		( const CPoint &p )	const	{ return fsqrtf( DistanceSq( p ) ); }
		float				DistanceSq		( const CPoint &p )	const
		{
			ntError_p( fabsf( m_D.Length() - 1.0f ) < 0.005f, ("The ray's direction must be normalised for this function to work!") );

			float t = m_D.Dot( p ^ m_S );

			if ( t >= 0.0f )
			{
				CPoint closest_point = Evaluate( t );

				return ( p - closest_point ).LengthSquared();
			}

			return ( p - m_S ).LengthSquared();
		}

	public:
		//
		//	Ctors.
		//
		Ray() {}
		Ray( const CPoint &S, const CDirection &D )
		:	m_S( S )
		,	m_D( D )
		{}
	
	private:
		//
		//	Aggregated members.
		//
		CPoint		m_S;
		CDirection	m_D;
};

#endif	// !RAY_H_

