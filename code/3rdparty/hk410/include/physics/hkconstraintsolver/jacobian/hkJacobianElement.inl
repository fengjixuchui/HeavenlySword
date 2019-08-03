/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

hkReal hk1Lin2AngJacobian::getDiag( const hkVelocityAccumulator& mA, const hkVelocityAccumulator& mB ) const
{
	const hk1Lin2AngJacobian& jac = *this;
	HK_ON_DEBUG( hkReal l = hkMath::fabs( hkReal(this->m_linear0.length3()) - 1.0f); )
	HK_ASSERT2(0x75d662fb, l < 0.01f , "To call getDiag, the length of the linear part must be 1.0f" );

	hkVector4 ang0; ang0.setMul4( jac.m_angular[0], jac.m_angular[0] );
	hkVector4 ang1; ang1.setMul4( jac.m_angular[1], jac.m_angular[1] );

	ang0.mul4( mA.m_invMasses );
	ang1.mul4( mB.m_invMasses );
	ang0.add4( ang1 );

	hkReal x = mA.m_invMasses(3) + mB.m_invMasses(3) + HK_REAL_EPSILON;
	hkReal dot = ang0.horizontalAdd3();
	return dot + x;
}

/*
hkReal hk1Lin2AngJacobian::getDiagAny( const hkVelocityAccumulator& mA, const hkVelocityAccumulator& mB ) const
{
	const hk1Lin2AngJacobian& jac = *this;

	hkVector4 lin; lin.setMul4( jac.m_linear0, jac.m_linear0 );
	hkVector4 ang0; ang0.setMul4( jac.m_angular[0], jac.m_angular[0] );
	hkVector4 ang1; ang1.setMul4( jac.m_angular[1], jac.m_angular[1] );

	hkVector4 lin0; lin0.setMul4( mA.m_invMasses(3), lin );
	hkVector4 lin1; lin1.setMul4( mB.m_invMasses(3), lin );

	ang0.mul4( mA.m_invMasses );
	ang1.mul4( mB.m_invMasses );

	lin0.add4( lin1 );
	ang0.add4( ang1 );

	ang0.add4( lin0 );
	hkReal x = HK_REAL_EPSILON;
	return ang0.horizontalAdd3() + x;
}
*/

hkReal hk2AngJacobian::getAngularDiag( const hkVelocityAccumulator& mA, const hkVelocityAccumulator& mB ) const
{
	const hk2AngJacobian& jac = *this;
	hkVector4 ang0; ang0.setMul4( jac.m_angular[0], jac.m_angular[0] );
	hkVector4 ang1; ang1.setMul4( jac.m_angular[1], jac.m_angular[1] );

	ang0.mul4( mA.m_invMasses );
	ang1.mul4( mB.m_invMasses );
	ang0.add4( ang1 );

	hkReal x = HK_REAL_EPSILON;
	hkReal d = ang0.horizontalAdd3();
	return d + x;
}


/// get the non diag element of the 2*2 inv mass matrix in the case that jacA and jacB share exactly the same rigid bodies
/// which is get J dot ((M-1)*JacB) 
/// <todo: think of a better name and do all plattforms
hkReal hk2AngJacobian::getNonDiagOptimized( const hkVelocityAccumulator& mA, const hkVelocityAccumulator& mB, const hk2AngJacobian& jacB ) const 
{
	const hk2AngJacobian& jacA = *this;

	hkVector4 ang0; ang0.setMul4( jacA.m_angular[0], jacB.m_angular[0] );
	hkVector4 ang1; ang1.setMul4( jacA.m_angular[1], jacB.m_angular[1] );

	ang0.mul4( mA.m_invMasses );
	ang1.mul4( mB.m_invMasses );
	ang0.add4( ang1 );
	hkReal d = ang0.horizontalAdd3();
	return d;
}

/// get the non diag element in the case that jacA and jacB share exactly the same rigid bodies
hkReal hk2AngJacobian::getNonDiagSameObjects( const hkVelocityAccumulator& mA, const hkVelocityAccumulator& mB, const hk2AngJacobian& jacB ) const 
{
	return getNonDiagOptimized( mA, mB,jacB);
}


/// get the non diag element in the case that jacA and jacB share exactly the same rigid bodies
hkReal hk2AngJacobian::getNonDiagDifferentObjects_With2ndBodyFromFirstObject( const hkVelocityAccumulator& mA, const hk2AngJacobian& jacB ) const 
{
	const hk2AngJacobian& jacA = *this;

	hkVector4 ang0; ang0.setMul4( jacA.m_angular[1], jacB.m_angular[0] );
	ang0.mul4( mA.m_invMasses );
	hkReal d = ang0.horizontalAdd3();
	return d;
}


/// get the non diag element of the 2*2 inv mass matrix in the case that jacA and jacB share exactly the same rigid bodies
/// which is get J dot ((M-1)*JacB) 
hkReal hk1Lin2AngJacobian::getNonDiagOptimized( const hkVelocityAccumulator& mA, const hkVelocityAccumulator& mB, const hk1Lin2AngJacobian& jacB ) const 
{
	const hk1Lin2AngJacobian& jacA = *this;

	hkVector4 lin; lin.setMul4( jacA.m_linear0, jacB.m_linear0 );

	hkVector4 ang0; ang0.setMul4( jacA.m_angular[0], jacB.m_angular[0] );
	hkVector4 ang1; ang1.setMul4( jacA.m_angular[1], jacB.m_angular[1] );

	hkVector4 mA3; mA3.setBroadcast3clobberW(mA.m_invMasses, 3);
	hkVector4 mB3; mB3.setBroadcast3clobberW(mB.m_invMasses, 3);
	hkVector4 lin0; lin0.setMul4( mA3, lin );
	hkVector4 lin1; lin1.setMul4( mB3, lin );

	ang0.mul4( mA.m_invMasses );
	ang1.mul4( mB.m_invMasses );

	lin0.add4( lin1 );
	ang0.add4( ang1 );

	ang0.add4( lin0 );

	hkReal d = ang0.horizontalAdd3();
	return d;
}

hkReal hk1Lin2AngJacobian::getNonDiagSameObjects( const hkVelocityAccumulator& mA, const hkVelocityAccumulator& mB, const hk1Lin2AngJacobian& jacB ) const 
{
	return getNonDiagOptimized( mA, mB,jacB);
}

hkReal hk1Lin2AngJacobian::getNonDiagDifferentObjects( const hkVelocityAccumulator& mA, const hk1Lin2AngJacobian& jacB ) const 
{
	const hk1Lin2AngJacobian& jacA = *this;

	hkVector4 lin; lin.setMul4( jacA.m_linear0, jacB.m_linear0 );

	hkVector4 ang0; ang0.setMul4( jacA.m_angular[0], jacB.m_angular[0] );

	hkVector4 mA3; mA3.setBroadcast( mA.m_invMasses,3 );
	hkVector4 lin0; lin0.setMul4( mA3, lin );

	ang0.mul4( mA.m_invMasses );

	ang0.add4( lin0 );

	hkReal d = ang0.horizontalAdd3();
	return d;
}

/// get the non diag element in the case that jacA and jacB share exactly the same rigid bodies
hkReal hk1Lin2AngJacobian::getNonDiagDifferentObjects_With2ndBodyFromFirstObject( const hkVelocityAccumulator& mA, const hk1Lin2AngJacobian& jacB ) const 
{
	const hk1Lin2AngJacobian& jacA = *this;

	hkVector4 lin; lin.setMul4( jacA.m_linear0, jacB.m_linear0 );

	hkVector4 ang0; ang0.setMul4( jacA.m_angular[1], jacB.m_angular[0] );

	hkVector4 mA3; mA3.setBroadcast( mA.m_invMasses,3 );
	hkVector4 lin0; lin0.setMul4( mA3, lin );

	ang0.mul4( mA.m_invMasses );
	ang0.sub4( lin0 );

	hkReal d = ang0.horizontalAdd3();
	return d;
}



hkReal hk2Lin2AngJacobian::getDiag( const hkVelocityAccumulator& mA, const hkVelocityAccumulator& mB, hkReal leverageRatio ) const
{
	const hk2Lin2AngJacobian& jac = *this;

	hkVector4 ang0; ang0.setMul4( jac.m_angular[0], jac.m_angular[0] );
	hkVector4 ang1; ang1.setMul4( jac.m_angular[1], jac.m_angular[1] );

	ang0.mul4( mA.m_invMasses );
	ang1.mul4( mB.m_invMasses );
	leverageRatio *= leverageRatio;
	ang0.add4(ang1);

	hkReal x = mA.m_invMasses(3) + mB.m_invMasses(3) * leverageRatio + HK_REAL_EPSILON;
	hkReal dot = ang0.horizontalAdd3(); // ?
	return dot + x;
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
