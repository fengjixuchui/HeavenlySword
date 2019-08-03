/***************************************************************************************************
*
*	DESCRIPTION		A Battlefield holds a heightfield and collision objects, that represent the
*					playfield an army works in
*
*	NOTES
*
***************************************************************************************************/

#ifndef BATTLEFIELD_CHUNK_INL_
#define BATTLEFIELD_CHUNK_INL_

//------------------------------------------------------
//!
//! Convert a global heighfield coord into a local heightfield
//! coordinate without range checks
//!
//------------------------------------------------------
inline void BFC_GetLocalHeightfieldCoordUnchecked( const BattlefieldChunk* restrict pChunk, float hfX, float hfY, float* restrict x, float* restrict y )
{
	*x = hfX - pChunk->m_LeftCoord;
	*y = hfY - pChunk->m_TopCoord;
}

//------------------------------------------------------
//!
//! Convert a global heighfield coord into a local heightfield
//! coordinate with range checks
//!
//------------------------------------------------------
inline bool BFC_GetLocalHeightfieldCoord(  const BattlefieldChunk* restrict pChunk, float hfX, float hfY, float* restrict x, float* restrict y )
{
	BFC_GetLocalHeightfieldCoordUnchecked( pChunk, hfX, hfY, x, y );
	if( *x < 0 || *x >= BF_CHUNK_WIDTH )
		return false;
	if( *y < 0 || *y >= BF_CHUNK_HEIGHT )
		return false;
	return true;
}

#define ARMY_HEIGHTFIELD_FRAC_BITS 8
#define ARMY_HEIGHTFIELD_FLOAT_MULT	(255.f)



//------------------------------------------------------
//!
//! given local heightfield coordinates return a point 
//! sampled height
//!
//------------------------------------------------------
inline int16_t BFC_GetLocalRawHeightPoint( const BattlefieldChunk* restrict pChunk, float x, float y )
{
	int x_idx = (int) floor(x);
	int y_idx = (int) floor(y);
	return (int16_t)(pChunk->m_HeightField[ ( ((y_idx+1) * (BF_CHUNK_WIDTH+2)) + (x_idx+1)) ] * ARMY_HEIGHTFIELD_FLOAT_MULT);
}

//------------------------------------------------------
//!
//! given local heightfield coordinates return a bilinearly
//! filtered height
//!
//------------------------------------------------------
inline int16_t BFC_GetLocalRawHeightBilinear( const BattlefieldChunk* restrict pChunk, float x, float y )
{
	int x_idx = (int) floor(x);
	int y_idx = (int) floor(y);
	// Track the position's coordinate local to this grid cell so we can
	// accurately interpolate later on.
	float remainder_x( x - float( x_idx ) );
	float remainder_y( y - float( y_idx ) );

	// Work out the bi-linear interpolants.
	float interpolants[2][2];
	interpolants[ 0 ][ 0 ] = pChunk->m_HeightField[ ( ((y_idx+1) * (BF_CHUNK_WIDTH+2)) + (x_idx+1)) ];
	interpolants[ 1 ][ 0 ] = pChunk->m_HeightField[ ( ((y_idx+1) * (BF_CHUNK_WIDTH+2)) + (x_idx+2)) ];
	interpolants[ 0 ][ 1 ] = pChunk->m_HeightField[ ( ((y_idx+2) * (BF_CHUNK_WIDTH+2)) + (x_idx+1)) ];
	interpolants[ 1 ][ 1 ] = pChunk->m_HeightField[ ( ((y_idx+2) * (BF_CHUNK_WIDTH+2)) + (x_idx+2)) ];

	// Perform the bi-linear interpolation on the height values.
	float x0_interpolation = ( interpolants[ 0 ][ 0 ] * ( 1.0f - remainder_x ) ) + ( interpolants[ 1 ][ 0 ] * remainder_x );
	float x1_interpolation = ( interpolants[ 0 ][ 1 ] * ( 1.0f - remainder_x ) ) + ( interpolants[ 1 ][ 1 ] * remainder_x );
	float interpolation = ( x0_interpolation * ( 1.0f - remainder_y ) ) + ( x1_interpolation * remainder_y );

	float f = (interpolation  * ARMY_HEIGHTFIELD_FLOAT_MULT);

	return (int16_t)f;
}

#define BFC_USE_BILINEAR_HEIGHT_SAMPLE

//------------------------------------------------------
//!
//! Return the local height given a battlefield position
//! only legal if the position is within the chunk chunk
//! that is passed in
//!
//------------------------------------------------------
inline int16_t BFC_GetLocalHeight( const BattlefieldChunk* restrict pChunk, const BattlefieldHeader* restrict pHeader, const BF_Position* restrict pPosition )
{
	float hfX, hfY;
	float x, y;
	BF_GetHeightFieldCoord( pPosition, pHeader, &hfX, &hfY );
#if !defined( _RELEASE )
	bool inChunk = BFC_GetLocalHeightfieldCoord( pChunk, hfX, hfY, &x, &y );
	if( inChunk )
	{
#if defined( BFC_USE_BILINEAR_HEIGHT_SAMPLE )
		return BFC_GetLocalRawHeightBilinear( pChunk, x, y );
#else
		return BFC_GetLocalRawHeightPoint( pChunk, x, y );
#endif // BILINEAR?
	} else
	{
//		ntPrintf( "************** Position not in this chunk <%i,%i> **************\n", pPosition->m_X, pPosition->m_Y );
		return 0;
	}
#else
	BFC_GetLocalHeightfieldCoordUnchecked( pChunk, hfX, hfY, &x, &y );
#if defined( BFC_USE_BILINEAR_HEIGHT_SAMPLE )
	return BFC_GetLocalRawHeightBilinear( pChunk, x, y );
#else
	return BFC_GetLocalRawHeightPoint( pChunk, x, y );
#endif // BILINEAR?

#endif // !_RELEASE
}

#endif // BATTLEFIELD_CHUNK_INL_
