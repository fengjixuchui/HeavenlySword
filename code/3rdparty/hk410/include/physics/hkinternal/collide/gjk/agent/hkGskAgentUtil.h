/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_GSK_AGENT_UTIL_H
#define HK_COLLIDE2_GSK_AGENT_UTIL_H


inline void hkGskAgentUtil_tryToAddSecondPoint( const hkAgent3Input&input, hkGsk::GetClosesetPointInput& gskInput, hkGskCache& gskCache, hkGskManifold& gskManifold, const hkVector4& separatingNormal, const hkVector4& pointOnB, hkProcessCollisionOutput& output )
{
	// do a simple collision restitution of first body
	hkTransform aTb2;
	gskInput.m_aTb = &aTb2;
	{
		hkVector4 normalInA; normalInA.setRotatedInverseDir( input.m_bodyA->getTransform().getRotation(), separatingNormal);
		hkVector4 pointInA;  pointInA.setTransformedInversePos(input.m_bodyA->getTransform(), pointOnB );
		
		hkVector4 mcr; mcr.setSub4( pointInA, input.m_bodyA->getMotionState()->getSweptTransform().m_centerOfMassLocal );
		hkVector4 arm; arm.setCross( mcr, normalInA );
		if ( arm.lengthSquared3() > 0.0f )
		{
			arm.normalize3();
		}
		else
		{
			arm = hkTransform::getIdentity().getColumn(1);
		}

		hkQuaternion q; q.setAxisAngle( arm, 0.01f );
		hkTransform n;
		n.getRotation().set( q );
		n.getTranslation().setMul4( hkMath::max2( 0.0f, separatingNormal(3)) + input.m_bodyA->getMotionState()->m_objectRadius * 0.01f, normalInA );

		aTb2.setMul( n, input.m_aTb );
	}
			
	hkVector4 separatingNormal2;
	hkVector4 pointOnB2;
	if( hkGsk::getClosestPoint( gskInput, gskCache, separatingNormal2, pointOnB2 ) != HK_FAILURE )
	{
		hkBool closestPointInManifold = hkGskManifold_doesPointExistAndResort( gskManifold, gskCache );
		if ( !closestPointInManifold )
		{
			hkProcessCdPoint* ccp = output.reserveContactPoints(1);
			ccp->m_contact.getPosition() = pointOnB2;
			ccp->m_contact.setSeparatingNormal( separatingNormal );
			hkProcessCdPoint* resultPointArray = output.getEnd() - gskManifold.m_numContactPoints;
			hkGskManifoldAddStatus addStatus = hkGskManifold_addPoint( gskManifold, *input.m_bodyA, *input.m_bodyB, *input.m_input, output, gskCache, ccp, resultPointArray, input.m_contactMgr, HK_GSK_MANIFOLD_CREATE_ID_ALWAYS );
			if ( addStatus == HK_GSK_MANIFOLD_POINT_ADDED )
			{
				output.commitContactPoints(1);
			}
			else
			{
				output.abortContactPoints(1);
			}
		}
	}
}

// hkVector4& separatingNormal is output only.
HK_FORCE_INLINE void HK_CALL hkGskAgentUtil_processCollisionNoTim(const hkAgent3Input& input, hkAgentEntry* entry, hkAgentData* agentData, hkGskCache& gskCache, hkGskManifold& gskManifold, hkVector4& separatingNormal,	hkProcessCollisionOutput& output)
{
	hkVector4 pointOnB;
	hkGsk::GetClosesetPointInput gskInput;
	{
		gskInput.m_shapeA = static_cast<const hkConvexShape*>(input.m_bodyA->getShape());
		gskInput.m_shapeB = static_cast<const hkConvexShape*>(input.m_bodyB->getShape());
		gskInput.m_aTb = &input.m_aTb;
		gskInput.m_transformA = &input.m_bodyA->getTransform();
		gskInput.m_collisionTolerance = input.m_input->getTolerance();
	}

	if( hkGsk::getClosestPoint( gskInput, gskCache, separatingNormal, pointOnB ) == HK_FAILURE )
	{
		if ( gskManifold.m_numContactPoints)
		{
			hkGskManifold_cleanup( gskManifold, input.m_contactMgr, *output.m_constraintOwner );
		}
	}
	else
	{
		int closestPointInManifold = (int)hkGskManifold_doesPointExistAndResort( gskManifold, gskCache );

		//HK_INTERNAL_TIMER_SPLIT_LIST("getPoints");
		

		// Get a pointer to the current output pointer
		hkProcessCdPoint* resultPointArray = output.getEnd();

		//
		// if there are other points in the manifold, get all those points first
		//
		if ( gskManifold.m_numContactPoints > closestPointInManifold)
		{
			hkGskManifoldWork work;
			hkGskManifold_init( gskManifold, separatingNormal, *input.m_bodyA, *input.m_bodyB, input.m_input->getTolerance(), work );
			hkGskManifold_verifyAndGetPoints( gskManifold, work, closestPointInManifold, output, input.m_contactMgr ); 
		}

		//
		//	Handle the first (== closest point) specially
		//
		//HK_INTERNAL_TIMER_SPLIT_LIST("convert1st");
		{
			hkProcessCdPoint* ccp = output.reserveContactPoints(1);
			ccp->m_contact.getPosition() = pointOnB;
			ccp->m_contact.setSeparatingNormal( separatingNormal );

			if ( closestPointInManifold )
			{
				//
				//	use existing contact point id, as this point was already in the manifold
				//
				ccp->m_contactPointId = gskManifold.m_contactPoints[0].m_id;
				output.commitContactPoints(1);
			}
			else
			{ 
				//
				// try to add the contact point to the manifold
				//
				const hkCollisionQualityInfo& sq = *input.m_input->m_collisionQualityInfo;
				int dim = gskCache.m_dimA + gskCache.m_dimB;
				hkReal createContactRangeMax = (dim==4)? sq.m_create4dContact: sq.m_createContact;

				if ( separatingNormal(3) < createContactRangeMax )
				{
					//	add point to manifold, the new point will aways be point 0
					hkGskManifoldAddStatus addStatus = hkGskManifold_addPoint( gskManifold, *input.m_bodyA, *input.m_bodyB, *input.m_input, output, gskCache, ccp, resultPointArray, input.m_contactMgr, HK_GSK_MANIFOLD_NO_ID_FOR_POTENTIALS );

					// really added
					if ( addStatus == HK_GSK_MANIFOLD_POINT_ADDED )
					{
						// take new point and check whether the new point just is a potential point
						if ( ccp->m_contactPointId != HK_INVALID_CONTACT_POINT)
						{
							output.commitContactPoints(1);
							//	try to create a second contact point
							//if ( gskManifold.m_numContactPoints == 1 )
							//{
							//	hkGskAgentUtil_tryToAddSecondPoint( input, gskInput, gskCache, gskManifold, separatingNormal, pointOnB, output );
							//}
						}
						else
						{
							if ( output.m_potentialContacts && entry )
							{
								if ( input.m_contactMgr->reserveContactPoints(1) == HK_SUCCESS )
								{
									hkProcessCollisionOutput::ContactRef& contactRef = *(output.m_potentialContacts->m_firstFreePotentialContact++);
									contactRef.m_contactPoint = ccp;
									contactRef.m_agentEntry = entry;
									contactRef.m_agentData   = agentData;
								}
								else
								{
									goto removeAndRejectNewPoint;
								}
							}
							else
							{
								ccp->m_contactPointId = input.m_contactMgr->addContactPoint( *input.m_bodyA, *input.m_bodyB, *input.m_input, output, &gskCache, ccp->m_contact );
								if ( ccp->m_contactPointId == HK_INVALID_CONTACT_POINT )
								{
removeAndRejectNewPoint:
									HK_ASSERT( 0xf043daed, gskManifold.m_contactPoints[0].m_id == HK_INVALID_CONTACT_POINT );
									hkGskManifold_removePoint( gskManifold, 0 );
									goto rejectNewPoint;
								}
								gskManifold.m_contactPoints[0].m_id = ccp->m_contactPointId;

							}
							output.commitContactPoints(1);
						}
					}
					else if (addStatus == HK_GSK_MANIFOLD_POINT_REJECTED )
					{
						// take first point
rejectNewPoint:
						ccp = resultPointArray; 
						output.abortContactPoints(1);
					}
					else if ( addStatus == HK_GSK_MANIFOLD_TWO_POINT2_REJECTED )
					{
							// remove last point in the output array
						output.commitContactPoints(-1);
						output.abortContactPoints(1);
						ccp = resultPointArray; 
					}
					else // replaced
					{
						// take new point
						ccp = resultPointArray + ( addStatus - HK_GSK_MANIFOLD_POINT_REPLACED0 );
						output.abortContactPoints(1);
					}
				}
			}
			// fixes HVK-2168: added '&& ccp < output.m_firstFreeContactPoint'
			// because we can only report a representativeContact if we have a contact
			if ( output.m_potentialContacts && ccp < output.m_firstFreeContactPoint )
			{
				*output.m_potentialContacts->m_firstFreeRepresentativeContact = ccp;	
				output.m_potentialContacts->m_firstFreeRepresentativeContact++;
			}
		}
	}
}

#endif // HK_COLLIDE2_GSK_AGENT_UTIL_H

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
