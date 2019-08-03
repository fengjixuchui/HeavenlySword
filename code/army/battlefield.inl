/***************************************************************************************************
*
*	DESCRIPTION		A Battlefield holds a heightfield and collision objects, that represent the
*					playfield an army works in
*
*	NOTES
*
***************************************************************************************************/

#ifndef BATTLEFIELD_INL_
#define BATTLEFIELD_INL_

#define BF_POS_MUL		(65536)
#define BF_POS_RECIP	(1.f / BF_POS_MUL)

//! convert a battlefield position into world space
inline CPoint BF_PositionToWorldSpace(  const BF_Position* restrict pPosition, const BattlefieldHeader* restrict pHeader )
{
	return pHeader->m_WorldTopLeft + 
					(CDirection(pHeader->m_WorldHoriz,0.f,0.f) * (pPosition->m_X * BF_POS_RECIP)) +
					(CDirection(0.f,0.f, pHeader->m_WorldVert) * (pPosition->m_Y * BF_POS_RECIP));
}

//! convert a world space position into battlefield space
inline bool BF_WorldSpaceToBF_Position( const CPoint& worldspace, const BattlefieldHeader* restrict pHeader, BF_Position* restrict pPosition )
{
	CPoint localspace = (worldspace - pHeader->m_WorldTopLeft);
	float x = (localspace.X() / pHeader->m_WorldHoriz);
	float y = (localspace.Z() / pHeader->m_WorldVert);
	if( (x < 0) || (x > 1) )
		return false;
	if( (y < 0) || (y > 1) )
		return false;

	pPosition->m_X = (uint16_t)(x * BF_POS_MUL);
	pPosition->m_Y = (uint16_t)(y * BF_POS_MUL);

	return true;
}

//! gives a rough idea of a distance (useful for radii where accuracy isn't too important)
inline float BF_ApproxDistanceToWorldSpace( uint16_t dist, const BattlefieldHeader* restrict pHeader  )
{
	BF_Position a, b;
	a.m_X = 0;
	a.m_Y = 0;
	b.m_X = 0;
	b.m_Y = dist;
	CPoint pa = BF_PositionToWorldSpace( &a, pHeader );
	CPoint pb = BF_PositionToWorldSpace( &b, pHeader );
	CPoint dif = pb - pa;
	return dif.Length();
}

// convert a battlefield position into a heightfield coordinate
inline void BF_GetHeightFieldCoord( const BF_Position* restrict pPosition, const BattlefieldHeader* restrict pHeader, float* restrict x, float* restrict y )
{
	*x = (((float)pPosition->m_X) * BF_POS_RECIP) * pHeader->m_TotalWidth;
	*y = (((float)pPosition->m_Y) * BF_POS_RECIP) * pHeader->m_TotalHeight;
}

//! retutns the chunk id for a given position
//! get rid of these divides using multiplication inverse constant stuff!!!
inline uint16_t BF_GetChunkIndex( const BF_Position* restrict pPosition, const BattlefieldHeader* restrict pHeader )
{
	const uint16_t x = pPosition->m_X / (BF_POS_MUL / pHeader->m_WidthInChunks);
	const uint16_t y = pPosition->m_Y / (BF_POS_MUL / pHeader->m_HeightInChunks);
	return (y * pHeader->m_WidthInChunks) + x;
}

#if defined( __SPU__ )

//! convert a battlefield position into world space
inline v128 BF_PositionToWorldSpace(  const v128 v, const BattlefieldHeader* restrict pHeader )
{
	return (pHeader->m_WorldTopLeft + 
					(CDirection(pHeader->m_WorldHoriz,0.f,0.f) * spu_extract(v,0) * BF_POS_RECIP) +
					(CDirection(0.f,0.f, pHeader->m_WorldVert)  * spu_extract(v,1) * BF_POS_RECIP)).Quadword();
}


#if !defined( _RELEASE )
#include "syncprims_spu.h"

inline void BF_DrawDebugLine( const BattlefieldHeader* restrict pHeader, const CPoint& a, const CPoint& b, uint32_t col )
{
	ntError( pHeader != NULL );
	uint32_t iIndex = AtomicIncrementU( (uint32_t) pHeader->m_iNumDebugLines );
	if ( iIndex >= MAX_ARMY_SPU_DEBUG_LINES )
	{
		return;
	}

	static_assert_in_class( ( sizeof( SPU_DebugLine ) & 0xf ) == 0, Must_be_16_byte_aligned );
	uint32_t eaAddr = ((uint32_t)pHeader->m_pDebugLineBuffer) + (iIndex * sizeof(SPU_DebugLine));
	ntError_p( ( eaAddr & 0xf ) == 0, ("BF_DrawDebugLine: ea = 0x%08X which is not 16-byte aligned. line buffer = 0x%08X, iIndex = %i", eaAddr, (uintptr_t)( pHeader->m_pDebugLineBuffer ), iIndex) );
	SPU_DebugLine tmp;
	tmp.a = a;
	tmp.b = b;
	tmp.col = col;

	ntDMA::Params lineDmaParams;
	ntDMA_ID lineId = ntDMA::GetFreshID();
	lineDmaParams.Init32( &tmp, eaAddr, sizeof(SPU_DebugLine) , lineId );
	ntDMA::DmaToPPU( lineDmaParams );
 	ntDMA::StallForCompletion( lineId );
	ntDMA::FreeID( lineId );

}

inline void BF_DrawDebugLine( const BattlefieldHeader* restrict pHeader, const BF_Position* restrict a, const BF_Position* restrict b, uint32_t col )
{
	ntError( pHeader != NULL );
	CPoint aPnt = BF_PositionToWorldSpace( a, pHeader );
	CPoint bPnt = BF_PositionToWorldSpace( b, pHeader );
	BF_DrawDebugLine( pHeader, aPnt, bPnt, col );
}

inline void BF_DrawDebugLine( const BattlefieldHeader* restrict pHeader, const v128 a, const v128 b, uint32_t col )
{
	ntError( pHeader != NULL );
	CPoint aPnt( BF_PositionToWorldSpace( a, pHeader ) );
	CPoint bPnt( BF_PositionToWorldSpace( b, pHeader ) );
	BF_DrawDebugLine( pHeader, aPnt, bPnt, col );
}

#else // end !_RELEASE

#define BF_DrawDebugLine(header, a, b, col)

#endif // end _RELEASE

#endif // end !__SPU__

#endif // BATTLEFIELD_INL_
