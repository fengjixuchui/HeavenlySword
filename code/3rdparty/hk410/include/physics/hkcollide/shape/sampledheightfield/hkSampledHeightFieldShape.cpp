/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkcollide/hkCollide.h>

#include <hkcollide/shape/sampledheightfield/hkSampledHeightFieldShape.h>
#include <hkcollide/util/hkAabbUtil.h>
#include <hkmath/linear/hkVector4Util.h>
#include <hkcollide/shape/hkShapeRayCastInput.h>
#include <hkinternal/collide/util/hkCollideTriangleUtil.h>
#include <hkcollide/shape/hkRayHitCollector.h>

hkSampledHeightFieldShape::hkSampledHeightFieldShape( const hkSampledHeightFieldBaseCinfo& ci )
{
	m_xRes = ci.m_xRes;
	m_zRes = ci.m_zRes;

		// setting the extents based on the current heightfield dimensions
	m_extents.set( ci.m_xRes-1.0f, ci.m_maxHeight - ci.m_minHeight, ci.m_zRes-1.0f );
	m_extents.mul4( ci.m_scale );

	if ( ci.m_minHeight > ci.m_maxHeight )
	{
		m_heightCenter = -1.0f;
		m_extents(1) = -1.0f;
	}
	else
	{
		m_heightCenter = 0.5f * ( ci.m_minHeight + ci.m_maxHeight );
	}


	const hkVector4& v = ci.m_scale;
	m_intToFloatScale = ci.m_scale;
	m_floatToIntScale.set( 1.f/v(0), 1.0f/v(1), 1.f/v(2) );

	//
	//	Correct for incorrect float to int conversion
	//
	{
		hkReal c = hkVector4Util::getFloatToInt16FloorCorrection();
		hkVector4 c4; c4.setMul4( c, m_intToFloatScale );
		c4(1) = 0;
		m_floatToIntOffsetFloorCorrected = c4;
	}
	HK_ASSERT2(0x746bd538,  m_xRes < 0x8000 && m_zRes < 0x8000, "The current heightfield dimension is limited to 16k * 16k cells" );
}

void hkSampledHeightFieldShape::getCinfo( hkSampledHeightFieldBaseCinfo& cinfo ) const
{
	HK_ASSERT2(0x4b50036a, false, "getCinfo for Sampled Height Field is not implemented yet.");
}

hkShapeType hkSampledHeightFieldShape::getType() const { return HK_SHAPE_SAMPLED_HEIGHT_FIELD; }

void hkSampledHeightFieldShape::getAabb( const hkTransform& localToWorld, hkReal tolerance, hkAabb& out ) const
{
	hkReal minY = getHeightAt(0,0);
	hkReal maxY = minY;
	if ( m_extents(1) < 0 )
	{
		for ( int i = 0; i < m_xRes; i++ )
		{
			for (int j = 0; j < m_zRes; j++ )
			{
				hkReal z = getHeightAt(i,j);
				minY = hkMath::min2( minY, z );
				maxY = hkMath::max2( maxY, z );
			}
		}
		minY *= this->m_intToFloatScale(1);
		maxY *= this->m_intToFloatScale(1);

		// maybe the z signRayDir is reverse, so correct this
		hkReal h = minY;
		minY = hkMath::min2( minY, maxY );	
		maxY = hkMath::max2( h,    maxY );

		hkSampledHeightFieldShape* base = const_cast<hkSampledHeightFieldShape*>(this);
		base->m_heightCenter = 0.5f * ( minY + maxY );
		base->m_extents(1) = maxY - minY;
	}


	hkVector4 center; 	center.setMul4( 0.5f, m_extents );
	hkVector4 halfExtents = center;

	center(1) = m_heightCenter;

	hkAabbUtil::calcAabb( localToWorld, halfExtents, center, tolerance, out );
}



void hkSampledHeightFieldShape::_getHeightAndNormalAt( int x, int z, hkReal subX, hkReal subZ, hkVector4& normalOut, hkReal& heightOut, int& triangleIndexOut ) const
{
	normalOut.setAll3( 1.0f );

	if ( getTriangleFlip() )
	{
		hkReal h00 = this->getHeightAt( x+0, z   );
		hkReal h11 = this->getHeightAt( x+1, z+1 );
		if ( subX > subZ )
		{
			hkReal h10 = this->getHeightAt( x+1, z   );

			hkReal dx = h10 - h00;
			hkReal dz = h11 - h10;

			heightOut = h00  + subZ * dz + subX * dx;

			normalOut(0) = -dx;
			normalOut(2) = -dz;
			triangleIndexOut = 1;
		}
		else
		{
			hkReal h01 = this->getHeightAt( x, z+1 );

			hkReal dx = h11 - h01;
			hkReal dz = h01 - h00;

			heightOut = h00  + subZ * dz + subX * dx;

			normalOut(0) = -dx;
			normalOut(2) = -dz;
			triangleIndexOut = 0;
		}
	}
	else
	{
		hkReal h10 = this->getHeightAt( x+1, z  );
		hkReal h01 = this->getHeightAt( x,   z+1 );
		if ( subX + subZ > 1.0f )
		{
			hkReal h11 = this->getHeightAt( x+1, z+1 );

			hkReal dx = h11 - h01;
			hkReal dz = h11 - h10;

			heightOut = h10  + subZ * dz + (subX - 1.0f) * dx;

			normalOut(0) = -dx;
			normalOut(2) = -dz;
			triangleIndexOut = 1;
		}
		else
		{
			hkReal h00 = this->getHeightAt( x+0, z   );

			hkReal dx = h10 - h00;
			hkReal dz = h01 - h00;

			heightOut = h00  + subZ * dz + subX * dx;

			normalOut(0) = -dx;
			normalOut(2) = -dz;
			triangleIndexOut = 0;
		}
	}
}

void hkSampledHeightFieldShape::getHeightAndNormalAt( int xPos, int zPos, hkReal subX, hkReal subZ, hkVector4& normalOut, hkReal& heightOut, int& triangleIndexOut ) const
{
	_getHeightAndNormalAt( xPos, zPos, subX, subZ, normalOut,heightOut, triangleIndexOut );
}


/// Basic Ideas:
///     - We use an 3dda-algorithm. 
///     - We try to identify when the ray crosses the z-plane. Therefore we keep track whether the
///       ray is currently above or under the heightfield.
/// Spaces:
///		First we calculate a projection from shape space into raycast space:
///		In raycast space the ray goes from (0,0,0) to (1,1,1). If the ray is flat in one signRayDir
///	    then the start and end value is set to 2 (effectively disabling further checks).
///		We keep three pointers (one for each axis) pointing to the next cell boundary to cross.
///		We find the closest border and cross it.
///		Whenever we cross a border, we also enter a new cell. So we need to calculate the 'y' value
///     and check whether we crossed the height fields.
/// Notes on the implementation:
///  -  unlike other collector casts, this one does not transform the normal wrt cdBody.
///  - 	shapeKey = hitX << 1 + (hitZ<<16). Lowest bit is used for triangle index.
///  -  The ray can never tunnel through two neighboring triangles.
///  -  The ray is not epsilon-proof when it just touches the landscape.
///  -  The ray algorithm uses a fast walking algorithm over the landscape. 
///     That means very long rays over very big heightfields can be CPU-expensive.
///  -  The code is quite fast, however it is also quite big. Try to combine many ray queries together to minimize instruction cache misses.
///  -  Rays that are nearly vertical to the heightfield get a different optimized treatment.
///  -  There are no restrictions on the scale. It can be anything, even negative.
///  -  If the ray starts or ends outside the heightfield, it gets clipped correctly.
///  -  The size of the heightfield is limited to 16k * 16k.
void hkSampledHeightFieldShape::castRayInternal( const hkShapeRayCastInput& input, const hkCdBody& cdBody, hkBool reportPenetratingStartPosition, hkReal maxExtraPenetration, hkRayHitCollector& collector) const
{
	HK_TIMER_BEGIN("rcHeightFild", HK_NULL);
	hkIntUnion64 intFrom;
	hkVector4Util::convertToUint16( input.m_from, m_floatToIntOffsetFloorCorrected, m_floatToIntScale, intFrom );

	// EP is the next grid line, that the ray will cross
	int ep[3];

	ep[0] = intFrom.i16[0];
	ep[2] = intFrom.i16[2];

	//
	//	Convert the ray into local int space
	//
	hkVector4 from; from.setMul4( input.m_from, m_floatToIntScale );
	hkVector4 to;     to.setMul4( input.m_to,   m_floatToIntScale );

	hkBool triangleFlip = getTriangleFlip();


	//
	//	Remap the coordinates to
	//  x = x Dir
	//  y = Diag Dir
	//  z = z Dir
	//  w = height
	//
	from(3) = from(1);
	to(3) = to(1);


	if ( !triangleFlip )
	{
		from(1) = from(0) + from(2);
		to(1)   =   to(0) +   to(2);
	}
	else
	{
		from(1) = from(0) - from(2);
		to(1)   =   to(0) -   to(2);
	}



	//
	//	Initialize the scale and offset in a way, so that the ray 
	//  goes from 0,0,0 to 1,1,1. If the ray is flat in one signRayDir
	//  than the start and end value is set to 2
	//
	hkVector4 gridLineDistance;	// the distance between two grid line boundaries projected onto the ray
	hkVector4 signedGridLineDistance;
	hkVector4 absRayDir;		
	int signRayDir[3];	       // +-1 based on the sign of the to-from
	hkReal distToNextCell[3];  // the abs distance from the ray start point in [0..1]-space to the next cell boundary
	{
		for (int i=0; i<3; i++)
		{
			hkReal delta     = to(i) - from(i);
			hkReal fabsDelta = hkMath::fabs( delta );
			absRayDir(i)     = fabsDelta;

			//
			//	Check for allowed division
			//
			if ( fabsDelta < HK_REAL_EPSILON * 256.f ) 
			{
				gridLineDistance(i) = 0.0f;
				signRayDir[i] = -1;
				distToNextCell[i] = HK_REAL_MAX;
				continue;
			}

			hkReal invDelta = 1.0f / delta;
			signedGridLineDistance(i) = invDelta;

			if ( delta < 0 )
			{
				signRayDir[i] = -1;
				gridLineDistance(i) = -invDelta;
			}
			else
			{
				signRayDir[i] = 1;
				ep[i]++;	// if we walk left, the next boundary is right
				gridLineDistance(i) = invDelta;
			}

			distToNextCell[i] = (ep[i] - from(i) ) * invDelta;
		}
	}

		//
		// check for nearly vertical raycast ( or any two combinations being zero
		//
	{
		if ( ( gridLineDistance(0) + gridLineDistance(2) == 0.0f )
			|| ( gridLineDistance(0) + gridLineDistance(1) == 0.0f )
			|| ( gridLineDistance(1) + gridLineDistance(2) == 0.0f )	)
		{
			
			int ix = ep[0];
			int iz = ep[2];
			if ( hkUint32( ix ) >= hkUint32( m_xRes - 1 ) || hkUint32( iz ) >= hkUint32( m_zRes  - 1) )
			{
				HK_TIMER_END();
				return;
			}

			hkShapeRayCastOutput output;

			hkReal fieldHeight;
			int triangleIndexOut;
			_getHeightAndNormalAt( ix, iz, from(0) - ix, from(2) - iz, output.m_normal, fieldHeight, triangleIndexOut );
			const hkReal h0 = from(3) - fieldHeight;
			const hkReal h1 = to(3) - fieldHeight;

			if ( h1 > h0 )
			{
				HK_TIMER_END();
				return;
			}

			hkReal hitFraction;
			if ( reportPenetratingStartPosition && h0 < 0.0f  )
			{	
				if ( h1 > h0 - maxExtraPenetration )
				{
					HK_TIMER_END();
					return;
				}
				hitFraction = 0.0f;
			}
			else
			{
				if ( h0 < 0.0f  || h1 >= 0.0f)
				{
					// return if startpoint is under or endpoint is above
					HK_TIMER_END();
					return;
				}
				hitFraction = h0 / (h0 - h1);
			}
			//
			//	Export hit
			//
			if ( hitFraction < collector.m_earlyOutHitFraction )
			{
				output.m_normal.mul4( m_floatToIntScale ); 
				output.m_normal.normalize3();
				output.m_hitFraction = hitFraction;
				output.m_extraInfo = (ix << 1) + (iz << 16) + triangleIndexOut;

				collector.addRayHit( cdBody, output );
			}
			HK_TIMER_END();
			return;
		}
	}

	//static int count;
	//if ( ++ count == 758 )
	//{
	//	count = count;
	//}


		//
		//	Clip (x,z only, y will be recalculated later and does not need clipping)
		//
	hkVector4 clippedFrom = from;
	{
		int origEp0 = ep[0];
		int origEp2 = ep[2];
		//
		//	Clip x first
		//
		while(1)
		{
			hkReal ratio;
			if ( signRayDir[0] > 0 )
			{
				if ( ep[0] > 0 )
				{
					break;
				}
				if ( to(0) < 0.0f )
				{
					HK_TIMER_END();
					return;
				}
				ratio = -from(0) * gridLineDistance(0);
				ep[0] = 1;
			}
			else
			{
				if ( ep[0] <= m_xRes-2 )
				{
					break;
				}
				hkReal maxVal = hkReal(m_xRes-1);
				if ( to(0) > maxVal )
				{
					HK_TIMER_END();
					return;
				}
				ratio = (from(0) - maxVal) * gridLineDistance(0);
				ep[0] = m_xRes-2;
			}
			distToNextCell[0] = ratio + gridLineDistance(0);
			clippedFrom.setInterpolate4( from, to, ratio );
			ep[2] = origEp2 + hkMath::hkToIntFast( ratio * absRayDir(2) ) * signRayDir[2] - 2;
			while ( clippedFrom(2) >= ep[2] ) { ep[2] ++; }
			ep[2] += signRayDir[2] >> 1;
			distToNextCell[2] = (ep[2] - from(2) ) * signedGridLineDistance(2);
			break;
		}
		//
		//	Clip z
		//
		while(1)
		{
			hkReal ratio;
			if ( signRayDir[2] > 0 )
			{
				if ( ep[2] > 0 )
				{
					break;
				}
				if ( to(2) < 0.0f )
				{
					HK_TIMER_END();
					return;
				}
				ratio = -from(2) * gridLineDistance(2);
				ep[2] = 1;
			}
			else
			{
				if ( ep[2] <= m_zRes-2 )
				{
					break;
				}
				hkReal maxVal = hkReal(m_zRes-1);
				if ( to(2) > maxVal )
				{
					HK_TIMER_END();
					return;
				}
				ratio = (from(2) - maxVal) * gridLineDistance(2);
				ep[2] = m_zRes-2;
			}
			distToNextCell[2] = ratio + gridLineDistance(2);
			clippedFrom.setInterpolate4( from, to, ratio );
			ep[0] = origEp0 + hkMath::hkToIntFast( ratio * absRayDir(0) ) * signRayDir[0] - 2;
			while ( clippedFrom(0) >= ep[0] ) { ep[0] ++; }
			ep[0] += signRayDir[0] >> 1;
			distToNextCell[0] = (ep[0] - from(0) ) * signedGridLineDistance(0);
			break;
		}
		
		//
		//	Recheck x 
		//
		{
			if ( hkUint32(ep[0]) >= hkUint32(m_xRes) )
			{
				HK_TIMER_END();
				return;
			}
			int x = ep[0] - signRayDir[0];
			if ( hkUint32(x) >= hkUint32(m_xRes) )
			{
				HK_TIMER_END();
				return;
			}
		}
	}

	//
	//	Check for diagonals.
	//  if diagDir = true ->   diagonal is 'orthogonal' to the ray
	//  if diagDir = false ->  diagonal is 'parallel' to the ray
	//
	hkBool diagDir = ( ( (signRayDir[0] ^ signRayDir[2]) >= 0 ) ^ triangleFlip );
	if ( gridLineDistance(1) == 0.0f)
	{
		diagDir = 0;
	}


	//
	//	Flag which indicates in which triangle the ray currently is.
	//  Each cell is diagonally split into 2 triangles. 
	//  One triange which is closer, and one which is further away (seen by the ray)
	//  If the ray is currently in the further away triangle   inMoreDistantTriangle is set to 2
	//													else   inMoreDistantTriangle is set to 0
	int	inMoreDistantTriangle  = 2;

	
		//
		//	Recalcutate the diagonal info
		//
	{
		ep[1]   =   (!triangleFlip) ? (ep[0] + ep[2]) : (ep[0] - ep[2]);

		hkReal subPos = clippedFrom(1) - hkReal( ep[1] );
		if ( diagDir )
		{
			if ( hkMath::fabs(subPos) > 1.0f )
			{
				ep[1] -= signRayDir[1];
			}
			else
			{
				inMoreDistantTriangle ^= 2;
			}
		}
		else
		{
			subPos *= signRayDir[1]; // correct the sign
			if ( subPos > 0.0f )
			{
				inMoreDistantTriangle ^= 2;
				ep[1] += signRayDir[1];
			}
		}
		if ( gridLineDistance(1) != 0.0f)
		{
			distToNextCell[1] = (ep[1] - from(1) )  * gridLineDistance(1) * signRayDir[1];
		}
		else
		{
			distToNextCell[1] = HK_REAL_MAX;
		}
	}


	//
	//	Now we use a trick to get our start info:
	//  We extent our ray backwards to the next cell boundary
	//
	int backWalkingDir;
	{
		//
		//	Calculate the distance backwards
		//
		hkReal distToPrevCell[3];
		for (int i = 0; i < 3; i++ )
		{
			if ( gridLineDistance(i) == 0.0f )
			{ 
				distToPrevCell[i] = -HK_REAL_MAX;
				continue;
			}
			hkReal cd = distToNextCell[i] - gridLineDistance(i);
			distToPrevCell[i] = cd;

			//HK_ASSERT(0x298c1b30,  cd + (from(i) - clippedFrom(i)) * signedGridLineDistance(i) <= 0.00001f );
			//HK_ASSERT(0x7f0ac726,  distToNextCell[i] + (from(i) - clippedFrom(i)) * signedGridLineDistance(i) >= -0.0001f );
		}

		//
		//	Search for the greatest value == shortest distance
		//
		{
			if ( diagDir )
			{
				backWalkingDir = ( !inMoreDistantTriangle ) ? 1 : ( (distToPrevCell[0]>distToPrevCell[2]) ? 0 : 2 );
			}
			else
			{
				if ( absRayDir(0) < absRayDir(2) )
				{
					backWalkingDir = ( inMoreDistantTriangle ) ? 2 : ( (distToPrevCell[0]>distToPrevCell[1]) ? 0 : 1 );
				}
				else
				{
					backWalkingDir = ( inMoreDistantTriangle ) ? 0 : ( (distToPrevCell[1]>distToPrevCell[2]) ? 1 : 2 );
				}
			}
			//HK_ASSERT(0x31805407,  distToPrevCell[0] <= distToPrevCell[backWalkingDir] + 0.00001f );
			//HK_ASSERT(0x6e6b23bc,  distToPrevCell[1] <= distToPrevCell[backWalkingDir] + 0.00001f );
			//HK_ASSERT(0x49c6afb8,  distToPrevCell[2] <= distToPrevCell[backWalkingDir] + 0.00001f );
		}

		//
		// walk backwards
		//
		ep[backWalkingDir] -= signRayDir[backWalkingDir];
		distToNextCell[backWalkingDir] -= gridLineDistance(backWalkingDir);
	}


	//
	//	variables indicating the status of our ray when it last crossed a cell boundary
	//  we initialize the values in a way so that our first loop does not produce a hit
	//
	hkReal lastFieldHeight		= HK_REAL_MAX;
	hkReal lastRayHeight		= 0.0f;
	hkReal distToLastCrossedCell = -1.0f;

	int sInd = backWalkingDir;	// start walking forward as walked backward
	goto examineCrossedGridBorder;
	while(1)
	{

		//
		// find closest cell boundary
		//
		{
			if ( diagDir )
			{
				sInd = ( !inMoreDistantTriangle ) ? 1 : ( (distToNextCell[0]<distToNextCell[2]) ? 0 : 2 );
			}
			else
			{
				if ( absRayDir(0) < absRayDir(2) )
				{
					sInd = ( inMoreDistantTriangle ) ? 2 : ( (distToNextCell[0]<distToNextCell[1]) ? 0 : 1 );
				}
				else
				{
					sInd = ( inMoreDistantTriangle ) ? 0 : ( (distToNextCell[1]<distToNextCell[2]) ? 1 : 2 );
				}
			}
			//HK_ASSERT(0x79385cf1,  distToNextCell[0] >= distToNextCell[sInd] - 0.00001f );
			//HK_ASSERT(0x68799b0d,  distToNextCell[1] >= distToNextCell[sInd] - 0.00001f );
			//HK_ASSERT(0x191c9b27,  distToNextCell[2] >= distToNextCell[sInd] - 0.00001f );
		}
examineCrossedGridBorder:
		//
		// check for ray leaving our heightfield area
		// Note: we use an unsigned compare to compare for <0 and >m_xRes
		//
		if ( (hkUint32(ep[0]) >= hkUint32(m_xRes)) || (hkUint32(ep[2]) >= hkUint32(m_zRes)) )
		{
			HK_TIMER_END();
			return;
		}

		//	Stop if distance is too big
		if ( distToLastCrossedCell > collector.m_earlyOutHitFraction )
		{
			break; 
		}

		//
		//	Get current height
		//
		hkReal fieldHeight;
		hkReal rayHeight;
		hkReal fraction = distToNextCell[sInd];
		{
			hkVector4 curPos; curPos.setInterpolate4( from, to, fraction );

			int x0 = ep[0];
			int x1 = ep[0] - signRayDir[0];
			int z0 = ep[2];

			if ( sInd == 0 )
			{
				int z1 = ep[2] - signRayDir[2];
				hkReal h0 = getHeightAt( x0, z1 );
				hkReal h1 = getHeightAt( x0, z0 );
				hkReal frac = hkMath::fabs(curPos(2) - hkReal( ep[2] ));
				fieldHeight = h1 + (h0 - h1) * frac;
			}
			else if (sInd == 2)
			{
				hkReal h0 = getHeightAt( x1, z0 );
				hkReal h1 = getHeightAt( x0, z0 );
				hkReal frac = hkMath::fabs(curPos(0) - hkReal( ep[0] ));
				fieldHeight = h1 + (h0 - h1) * frac;
			}
			else 
			{
				int z1 = z0;
				hkReal frac = hkMath::fabs( curPos(0) - hkReal( ep[0] ));

				if ( diagDir )
				{
					z1 -= signRayDir[2];
					//HK_ON_DEBUG( hkReal frac2 = hkMath::fabs(curPos(2) - hkReal( ep[2] )) );
					//HK_ASSERT2(0x600aa376, hkMath::fabs( frac + frac2 - 1.0f )  < 0.5f, "Diagonals crossed inconsistently" );
				}
				else
				{
					z0 -= signRayDir[2];
					//HK_ON_DEBUG( hkReal frac2 = hkMath::fabs(curPos(2) - hkReal( ep[2] )) );
					//HK_ASSERT2(0x2f4fe9b8, hkMath::fabs( frac - frac2 )  < 0.5f, "Diagonals crossed inconsistently" );
				}
				hkReal h0 = getHeightAt( x1, z0 );
				hkReal h1 = getHeightAt( x0, z1 );
				fieldHeight = h1 + (h0 - h1) * frac;
			}
			rayHeight = curPos(3);
		}
		
		//
		//	Check for hit and calculate the hit fraction
		//
		hkReal hitFraction;
		{
			if ( reportPenetratingStartPosition )
			{
				// check whether we are completely outside
				if ( rayHeight >= fieldHeight )
				{
					goto setAndWalk;
				}

				// check the slope of the ray, only report a hit if we increase our
				// penetration depth
				const hkReal height = rayHeight - fieldHeight; // < 0 
				const hkReal lastHeight = lastRayHeight - lastFieldHeight;
				const hkReal cellFraction = fraction - distToLastCrossedCell;
				
				if ( height >= lastHeight - cellFraction * maxExtraPenetration)
				{
					goto setAndWalk;
				}

				if ( lastHeight <= 0.0f)
				{
					//
					//	In case the start is penetrating 
					//  we can report a hit at the last crossed cell or at the ray start
					//
					hitFraction = hkMath::max2( 0.0f, distToLastCrossedCell );
					goto reportHit;				
				}
			}
			else if ( rayHeight >= fieldHeight || lastRayHeight < lastFieldHeight )
			{
				// we are above the surface
				// or we are already under, so just recalc
setAndWalk:		lastFieldHeight = fieldHeight;
				lastRayHeight = rayHeight;
				distToLastCrossedCell = fraction;
				goto walk;
			}

			hkReal height = rayHeight - fieldHeight; // < 0 

			// two ways to get the hit fraction (to optimize floating point accuracy)
			if ( distToLastCrossedCell >= 0 )
			{
				hkReal lastHeight = lastRayHeight - lastFieldHeight; // >=0
				hkReal subFraction = lastHeight / ( lastHeight - height  ); // [0..1]
				hitFraction = distToLastCrossedCell + ( fraction - distToLastCrossedCell ) * subFraction; // >= 0
			}
			else
			{
				// calculate the height for start point of the ray
				hkReal f = fraction / (fraction - distToLastCrossedCell);
				hkReal startFieldHeight = lastFieldHeight * f + fieldHeight * (1.0f - f);

				hkReal lastHeight = from(3) - startFieldHeight;
				if ( lastHeight < 0.0f )
				{
					if (!reportPenetratingStartPosition)
					{
						goto setAndWalk;
					}
					lastHeight = 0.0f;
				}
				// lastHeight>=0, height<0 -> subFraction = [0..1[
				hkReal subFraction = lastHeight / ( lastHeight - height  );
				hitFraction = fraction * subFraction;
			}
		}

reportHit:
		lastFieldHeight = fieldHeight;
		lastRayHeight = rayHeight;
		distToLastCrossedCell = fraction;

		//
		//	Check validity of hit
		//
		{
			//if ( hitFraction < 0.0f )
			//{
			//	goto walk;
			//}
			if ( hitFraction > collector.m_earlyOutHitFraction )
			{
				HK_TIMER_END();
				return;
			}
		}

		//
		//	Get the hit information and call the collector
		//

		// Also determine whether we are in triangle 1 or 0 in the quad (this is used to set the correct shape key).
		int triangleIndex;
		{
			//
			//	Get the normal 
			//
			hkShapeRayCastOutput output;
			output.m_normal(3) = 0.0f;
			output.m_normal(1) = 1.0f;
			{
				int x0 = ep[0];
				int x1 = ep[0] - signRayDir[0];
				int z0 = ep[2];
				int z1 = ep[2] - signRayDir[2];

				if ( diagDir )
				{
					hkReal h0 = getHeightAt( x0, z1 );
					hkReal h1 = getHeightAt( x1, z0 );

					if ( sInd == 1)	// take the backward triangle
					{
						hkReal h2 = getHeightAt( x1, z1 );
						output.m_normal(0) = ( h2 - h0 );
						output.m_normal(2) = ( h2 - h1 );
						triangleIndex = (z1 > z0) ^ triangleFlip;
					}
					else
					{
						hkReal h2 = getHeightAt( x0, z0 );
						output.m_normal(0) = ( h1 - h2 );
						output.m_normal(2) = ( h0 - h2 );
						triangleIndex = (z0 > z1) ^ triangleFlip;
					}
				}
				else
				{
					hkReal h0 = getHeightAt( x1, z1 );
					hkReal h1 = getHeightAt( x0, z0 );

					if ( sInd == 0 || (sInd == 1 && ( absRayDir(0) < absRayDir(2) )) )
					{
						hkReal h2 = getHeightAt( x0, z1 );
						output.m_normal(0) = ( h0 - h2 );
						output.m_normal(2) = ( h2 - h1 );
						triangleIndex = (z1 > z0) ^ triangleFlip;
					}
					else 
					{
						hkReal h2 = getHeightAt( x1, z0 );
						output.m_normal(0) = ( h2 - h1 );
						output.m_normal(2) = ( h0 - h2 );
						triangleIndex = (z0 > z1) ^ triangleFlip;
					}
				}
			}

			//
			//	export hit
			//
			{
				output.m_normal(0) *=  signRayDir[0];
				output.m_normal(2) *=  signRayDir[2];

					// we calculated the normal using an optimized cross product
					// so we need to multiply the x-component with m_intToFloatScale(1)*m_intToFloatScale(2), the y with ..(0)*...(2)
					// and the z with ..(1)*..(2)
					// instead we multiply with ..(0)*..(1)*..(2) and devide by ..(0)/..(1)/..(2). 
					// As we normalize afterwards, we can simply remove the multiply and just use the divide
				output.m_normal.mul4( m_floatToIntScale ); 
				output.m_normal.normalize3();
				output.m_hitFraction = hitFraction;
				int ix = ep[0] - (signRayDir[0]>>1) - 1;	// == the min( x, x-signRayDir );
				int iz = ep[2] - (signRayDir[2]>>1) - 1;	// == the min( z, z-signRayDir );
				output.m_extraInfo = (ix << 1) + (iz << 16) + triangleIndex;
				collector.addRayHit( cdBody, output );

				// if we report a hit, disable any more penetration checks
				if ( reportPenetratingStartPosition )	
				{
					reportPenetratingStartPosition = false;
				}
			}
		}
		
		//
		//	walk 
		//
walk:
		inMoreDistantTriangle ^= 2;
		ep[sInd] += signRayDir[sInd];
		distToNextCell[sInd] += gridLineDistance(sInd);
	} // while(1)

	HK_TIMER_END();
}

namespace
{
	struct NearestHitCollector : public hkRayHitCollector
	{
		NearestHitCollector(hkShapeRayCastOutput& output) : m_hasHit(false), m_output(output)
		{
			m_earlyOutHitFraction = output.m_hitFraction;
		}

		virtual void addRayHit( const hkCdBody& cdBody, const hkShapeRayCastCollectorOutput& hitInfoIn )
		{
			if ( hitInfoIn.m_hitFraction < m_earlyOutHitFraction )
			{
				const hkShapeRayCastOutput& hitInfo = static_cast<const hkShapeRayCastOutput&>(hitInfoIn);
				m_earlyOutHitFraction = hitInfo.m_hitFraction;
				m_output.m_normal = hitInfo.m_normal;
				m_output.m_hitFraction = hitInfo.m_hitFraction;
				m_output.m_extraInfo = hitInfoIn.m_extraInfo;
				m_output.setKey( HK_INVALID_SHAPE_KEY );
				m_hasHit = true;
			}
		}

		hkBool m_hasHit;
		hkShapeRayCastOutput& m_output;
	};
}

hkBool hkSampledHeightFieldShape::castRay( const hkShapeRayCastInput& input, hkShapeRayCastOutput& results) const
{
	NearestHitCollector myCollector(results);

	hkCdBody* cdBody = HK_NULL;
	castRayInternal( input, *cdBody, false, 0.0f, myCollector );

	return myCollector.m_hasHit;
}

namespace
{
	struct RotateNormalHitCollector : public hkRayHitCollector
	{
		RotateNormalHitCollector(hkRayHitCollector& collector) : m_collector(collector)
		{
			m_earlyOutHitFraction = collector.m_earlyOutHitFraction;
		}

		virtual void addRayHit( const hkCdBody& cdBody, const hkShapeRayCastCollectorOutput& hitIn )
		{
			hkShapeRayCastCollectorOutput hitOut = hitIn;
			hitOut.m_normal._setMul3( cdBody.getTransform().getRotation(), hitIn.m_normal );
			m_collector.addRayHit( cdBody, hitOut );
		}
		hkRayHitCollector& m_collector;
	};
}

void hkSampledHeightFieldShape::castRayWithCollector( const hkShapeRayCastInput& input, const hkCdBody& cdBody, hkRayHitCollector& collector) const
{
	RotateNormalHitCollector myCollector(collector);
	castRayInternal( input, cdBody, false, 0.0f, myCollector );
}

void hkSampledHeightFieldShape::castSphere( const hkSphereCastInput& input, const hkCdBody& cdBody, hkRayHitCollector& collector ) const
{
	hkShapeRayCastInput in2 = input;
	hkReal delta = input.m_radius;
	if ( this->m_intToFloatScale(1) > 0.0f )
	{
		delta = -delta;
	}
	in2.m_from(1) += delta;
	in2.m_to(1) += delta;

	RotateNormalHitCollector myCollector(collector);
	castRayInternal( in2, cdBody, true, input.m_maxExtraPenetration, collector );
}

/*
* Havok SDK - CLIENT RELEASE, BUILD(#20060902)
*
* Confidential Information of Havok.  (C) Copyright 1999-2006 
* Telekinesys Research Limited t/a Havok. All Rights Reserved. The Havok
* Logo, and the Havok buzzsaw logo are trademarks of Havok.  Title, ownership
* rights, and intellectual property rights in the Havok software remain in
* Havok and/or its suppliers.
*
* Use of this software for evaluation purposes is subject to and indicates 
* acceptance of the End User licence Agreement for this product. A copy of 
* the license is included with this software and is also available from salesteam@havok.com.
*
*/
