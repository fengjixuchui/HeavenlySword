/***************************************************************************************************
*
*	DESCRIPTION		Sampler that gives poisson disc distributed samples (kinda).
*
*	NOTES
*
***************************************************************************************************/

//**************************************************************************************
//	Include files.
//**************************************************************************************
#include "tbd/PoissonSampler.h"
#include "game/randmanager.h"

//**************************************************************************************
//	
//**************************************************************************************
PoissonSampler::PoissonSampler( int num_points, const char * const filename )
{
#ifdef PLATFORM_PC
	int num_points_over_10 = num_points / 10;
	if ( num_points_over_10 * 10 < num_points )
	{
		num_points_over_10++;
	}
	num_points = num_points_over_10 * 10;

	FILE *fp( fopen( filename, "rb" ) );
	{
		int num_sample_sets;
		fread( &num_sample_sets, sizeof( int ), 1, fp );

		if ( num_sample_sets * 10 < num_points )
		{
			ntError_p( false, ("Too many sample points requested.") );
			return;
		}

		ntstd::Vector< Private::BasicPoint > curr_point_set;

		int curr_num_points = 0;
		do
		{
			if ( feof( fp ) )
			{
				ntError_p( false, ("Too many sample points requested.") );
				break;
			}

			int num_samples;
			fread( &num_samples, sizeof( int ), 1, fp );

			curr_point_set.clear();

			for ( int i=0;i<num_samples;i++ )
			{
				Private::BasicPoint bp;
				fread( &bp, sizeof( Private::BasicPoint ), 1, fp );

				curr_point_set.push_back( bp );
			}

			ntError( int( curr_point_set.size() ) == num_samples );
			curr_num_points = num_samples;
		}
		while ( curr_num_points != num_points );

		m_PointArray = curr_point_set;
	}
	fclose( fp );
#elif PLATFORM_PS3
	for ( int32_t i=0;i<num_points;i++ )
	{
		Private::BasicPoint point;
		point.x = erandf( 1.0f ) - 0.5f;
		point.y = erandf( 1.0f ) - 0.5f;
		point.z = erandf( 1.0f ) - 0.5f;

		m_PointArray.push_back( point );
	}
#else
#	error unknown platform definition.
#endif
}

