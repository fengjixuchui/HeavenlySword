/***************************************************************************************************
*
*	DESCRIPTION		Sampler that gives poisson disc distributed samples (kinda).
*
*	NOTES
*
***************************************************************************************************/

#ifndef POISSONSAMPLER_H_
#define POISSONSAMPLER_H_

//**************************************************************************************
//	Include files.
//**************************************************************************************

//**************************************************************************************
//	
//**************************************************************************************
namespace Private
{
	struct BasicPoint
	{
		float x, y, z;
	};
}

//**************************************************************************************
//	
//**************************************************************************************
class PoissonSampler
{
	public:
		//
		//	Return the ith point.
		//
		CPoint	GetPoint	( int i )	const	{ ntError( i < int( m_PointArray.size() )); return CPoint( m_PointArray[ i ].x, m_PointArray[ i ].y, m_PointArray[ i ].z ); }

	public:
		//
		//	Ctor.
		//
		PoissonSampler( int num_points, const char * const filename = "data\\poisson.pos" );

	private:
		//
		//	Aggregated members.
		//
		ntstd::Vector< Private::BasicPoint >	m_PointArray;
};

#endif // POISSONSAMPLER_H_
