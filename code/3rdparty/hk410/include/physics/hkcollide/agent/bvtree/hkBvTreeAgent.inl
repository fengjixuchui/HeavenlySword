/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

//
//	This inl file is implementing local function for the hkBvTreeAgent
//

#ifdef HK_DEBUG
//#	define HK_BV_TREE_DISPLAY_AABB
#endif

#if defined(HK_BV_TREE_DISPLAY_AABB)
#	include <hkvisualize/hkDebugDisplay.h>
#endif

hkResult hkBvTreeAgent::calcAabbAndQueryTree( const hkCdBody& bodyA,	const hkCdBody& bodyB, const hkTransform& bTa,
											 const hkVector4& linearTimInfo, const hkProcessCollisionInput& input,
											 hkAabb* cachedAabb, hkArray<hkShapeKey>& hitListOut )
{
	//
	// Calc the aabb extruded by the relative movement
	//
	hkAabb aabb;
#ifdef HK_BV_TREE_DISPLAY_AABB
	hkAabb baseAabb;
#endif
	{
		//added an early out so if the aabb is the same, don't query the mopp and don't  sort, nor call the dispatch/agent
		{
			const hkMotionState* msA = bodyA.getMotionState();
			const hkMotionState* msB = bodyB.getMotionState();

				// if using continuous physics, expand the aabb backwards
				// rotate tim into the bvTree space
			hkVector4 timInfo;	timInfo._setRotatedInverseDir( bodyB.getTransform().getRotation(), linearTimInfo );


			hkVector4 aabbExtents; 
			if ( input.m_collisionQualityInfo->m_useContinuousPhysics )
			{
					// object A rotates within object B with the diff of both angular velocities
				const hkReal secOrdErrA = (msA->m_deltaAngle(3) + msB->m_deltaAngle(3)) * msA->m_objectRadius;

					// The angular velocity gets correctly calculated into the trajectory of object A
					// we only need to calculate the maximum error. So we use the square of the error B
				const hkReal secOrdErrB = msB->m_deltaAngle(3) * msB->m_deltaAngle(3) * msB->m_objectRadius;

				const hkReal checkEpsilon = input.m_tolerance * 0.5f + (secOrdErrA + secOrdErrB );
				bodyA.getShape()->getAabb( bTa, checkEpsilon, aabb );


				// restrict the size of the aabb to the worst case radius size
				hkVector4 massCenterAinB;
				{
					hkVector4 radius4; radius4.setAll3( msA->m_objectRadius + input.m_tolerance * 0.5f + secOrdErrB );
					massCenterAinB._setTransformedInversePos(bodyB.getTransform(), msA->getSweptTransform().m_centerOfMass1 );
					hkVector4 maxR; maxR.setAdd4( massCenterAinB, radius4 );
					hkVector4 minR; minR.setSub4( massCenterAinB, radius4 );
					aabb.m_min.setMax4( aabb.m_min, minR );
					aabb.m_max.setMin4( aabb.m_max, maxR );
				}

				// export the size of the base aabb
				aabbExtents.setSub4( aabb.m_max, aabb.m_min );

				// expand the aabb backwards
				{
					// correct the timInfo if we have a rotating tree
					if (msB->m_deltaAngle(3) > 0.00f )
					{
						hkVector4 relPos; relPos.setSub4( massCenterAinB, msB->getSweptTransform().m_centerOfMassLocal );
						hkVector4 offsetOut; offsetOut.setCross( relPos, msB->m_deltaAngle );
						hkReal f = input.m_stepInfo.m_deltaTime * msB->getSweptTransform().getInvDeltaTime();
						timInfo.addMul4( f, offsetOut );
					}

					hkVector4 zero;		zero.setZero4();
					hkVector4 minPath; 	minPath.setMin4( zero, timInfo );
					hkVector4 maxPath;	maxPath.setMax4( zero, timInfo );

#ifdef HK_BV_TREE_DISPLAY_AABB
					baseAabb = aabb;
					//baseAabb.m_min.add4( timInfo );
					//baseAabb.m_max.add4( timInfo );
#endif
					
					aabb.m_min.add4( minPath );
					aabb.m_max.add4( maxPath );
				}
			}
			else
			{
				const hkReal checkEpsilon = input.m_tolerance * 0.5f;
				bodyA.getShape()->getAabb( bTa, checkEpsilon, aabb );
				aabbExtents.setSub4( aabb.m_max, aabb.m_min );
#ifdef HK_BV_TREE_DISPLAY_AABB
				baseAabb = aabb;
#endif
			}

			//
			//	Try to do some AABB caching to reduce the number of calls to the bounding volume structure
			//
			if (cachedAabb)
			{
				if ( cachedAabb->contains( aabb ))
				{
					return HK_FAILURE;
				}

				hkVector4 zero; zero.setZero4();
				hkVector4 minPath;minPath.setMin4( zero, timInfo );
				hkVector4 maxPath;maxPath.setMax4( zero, timInfo );


				// expand aabb so we have a higher chance of a hit next frame
				// we expand it by half of our tolerance
				hkVector4 expand4; expand4.setAll3( input.m_tolerance * 0.5f );
				aabb.m_min.sub4( expand4 );
				aabb.m_max.add4( expand4 );

				// expand along our path linearly at least 2 frames ahead
				// but a maximum of 40% of the original aabb
				const hkReal maxExpand = 0.4f;
				const hkReal framesLookAhead = -2.0f;

				hkVector4 minExtentPath; minExtentPath.setMul4( framesLookAhead, maxPath );
				hkVector4 maxExtentPath; maxExtentPath.setMul4( framesLookAhead, minPath );

				hkVector4 maxExpand4; maxExpand4.setMul4( maxExpand, aabbExtents );
				maxExtentPath.setMin4( maxExtentPath, maxExpand4 );
				hkVector4 minExpand4; minExpand4.setNeg4(maxExpand4);
				minExtentPath.setMax4( minExtentPath, minExpand4 );

				aabb.m_min.add4( minExtentPath );
				aabb.m_max.add4( maxExtentPath );
				*cachedAabb = aabb;
			}
		}
	}

	//
	// display the aabb and the cached aabb
	//
#ifdef HK_BV_TREE_DISPLAY_AABB
	{
		hkAabb* bb = &baseAabb; 
		int color = hkColor::YELLOW;
		for ( int a = 0; a < 2; a ++)
		{
			for ( int x = 0; x < 2; x ++ )
			{	for ( int y = 0; y < 2; y ++ )
				{	for ( int z = 0; z < 2; z ++ )
					{
						hkVector4 a; a.set( (&bb->m_min)[x](0), (&bb->m_min)[y](1), (&bb->m_min)[z](2) );
						a.setTransformedPos( bodyB.getTransform(), a );
						hkVector4 b;

						b.set( (&bb->m_min)[1-x](0), (&bb->m_min)[y](1), (&bb->m_min)[z](2) );
						b.setTransformedPos( bodyB.getTransform(), b );
						HK_DISPLAY_LINE( a, b, color );
						b.set( (&bb->m_min)[x](0), (&bb->m_min)[1-y](1), (&bb->m_min)[z](2) );
						b.setTransformedPos( bodyB.getTransform(), b );
						HK_DISPLAY_LINE( a, b, color );
						b.set( (&bb->m_min)[x](0), (&bb->m_min)[y](1), (&bb->m_min)[1-z](2) );
						b.setTransformedPos( bodyB.getTransform(), b );
						HK_DISPLAY_LINE( a, b, color );
			}	}	}
			color = hkColor::BLUE;
			bb = cachedAabb;
			if (!bb) 
			{
				break;
			}
		}
	}
#endif
	//
	// query the BvTreeShape
	//
	const hkBvTreeShape* bvB = static_cast<const hkBvTreeShape*>( bodyB.getShape() );

	bvB->queryAabb( aabb, hitListOut );
	return HK_SUCCESS;
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
