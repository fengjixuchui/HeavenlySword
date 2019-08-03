/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */


hkContactMgrFactory* hkCollisionDispatcher::getContactMgrFactory(int responseA, int responseB) const
{
	HK_ASSERT2(0x34f4a8bd,  unsigned(responseA) < HK_MAX_RESPONSE_TYPE, "Response Type A is outside [ 0 .. " << HK_MAX_RESPONSE_TYPE-1 << "]" );
	HK_ASSERT2(0x28956f94,  unsigned(responseB) < HK_MAX_RESPONSE_TYPE, "Response Type B is outside [ 0 .. " << HK_MAX_RESPONSE_TYPE-1 << "]" );
	return m_contactMgrFactory[ responseA ] [ responseB ];
}


hkCollisionDispatcher::CreateFunc hkCollisionDispatcher::getCollisionAgentCreationFunction( hkShapeType typeA, hkShapeType typeB, hkCollisionDispatcher::IsAgentPredictive predictive ) const
{
	HK_ASSERT2(0x632d17d0,  unsigned(typeA) < HK_MAX_SHAPE_TYPE, "You can only access types between [0.." << HK_MAX_SHAPE_TYPE-1 << "]");
	HK_ASSERT2(0x40304d2d,  unsigned(typeB) < HK_MAX_SHAPE_TYPE, "You can only access types between [0.." << HK_MAX_SHAPE_TYPE-1 << "]");
	int idx = (predictive ? m_agent2TypesPred : m_agent2Types)[typeA][typeB];
	return m_agent2Func[idx].m_createFunc;
}


hkCollisionDispatcher::GetPenetrationsFunc hkCollisionDispatcher::getGetPenetrationsFunc( hkShapeType typeA, hkShapeType typeB ) const
{
	HK_ASSERT2(0x3bb42402,  unsigned(typeA) < HK_MAX_SHAPE_TYPE, "You can only access types between [0.." << HK_MAX_SHAPE_TYPE-1 << "]");
	HK_ASSERT2(0x5f31c19f,  unsigned(typeB) < HK_MAX_SHAPE_TYPE, "You can only access types between [0.." << HK_MAX_SHAPE_TYPE-1 << "]");
	int idx = m_agent2Types[typeA][typeB];
	return m_agent2Func[idx].m_getPenetrationsFunc;
}

hkCollisionDispatcher::GetClosestPointsFunc hkCollisionDispatcher::getGetClosestPointsFunc( hkShapeType typeA, hkShapeType typeB ) const
{
	HK_ASSERT2(0x18e676ad,  unsigned(typeA) < HK_MAX_SHAPE_TYPE, "You can only access types between [0.." << HK_MAX_SHAPE_TYPE-1 << "]");
	HK_ASSERT2(0x5e9be6aa,  unsigned(typeB) < HK_MAX_SHAPE_TYPE, "You can only access types between [0.." << HK_MAX_SHAPE_TYPE-1 << "]");
	int idx = m_agent2Types[typeA][typeB];
	return m_agent2Func[idx].m_getClosestPointFunc;
}

hkCollisionDispatcher::LinearCastFunc hkCollisionDispatcher::getLinearCastFunc( hkShapeType typeA, hkShapeType typeB ) const
{
	HK_ASSERT2(0x76474747,  unsigned(typeA) < HK_MAX_SHAPE_TYPE, "You can only access types between [0.." << HK_MAX_SHAPE_TYPE-1 << "]");
	HK_ASSERT2(0x1921f43f,  unsigned(typeB) < HK_MAX_SHAPE_TYPE, "You can only access types between [0.." << HK_MAX_SHAPE_TYPE-1 << "]");
	int idx = m_agent2Types[typeA][typeB];
	return m_agent2Func[idx].m_linearCastFunc;
}

hkBool hkCollisionDispatcher::getIsFlipped( hkShapeType typeA, hkShapeType typeB ) const
{
	HK_ASSERT2(0x76474747,  unsigned(typeA) < HK_MAX_SHAPE_TYPE, "You can only access types between [0.." << HK_MAX_SHAPE_TYPE-1 << "]");
	HK_ASSERT2(0x1921f43f,  unsigned(typeB) < HK_MAX_SHAPE_TYPE, "You can only access types between [0.." << HK_MAX_SHAPE_TYPE-1 << "]");
	int idx = m_agent2Types[typeA][typeB];
	return m_agent2Func[idx].m_isFlipped;
}

hkAgent3::CreateFunc   hkCollisionDispatcher::getAgent3CreateFunc  ( hkAgent3::AgentType type )
{
	return m_agent3Func[type ].m_createFunc;
}

hkAgent3::DestroyFunc  hkCollisionDispatcher::getAgent3DestroyFunc ( hkAgent3::AgentType type )
{
	return m_agent3Func[type ].m_destroyFunc;
}

hkAgent3::CleanupFunc  hkCollisionDispatcher::getAgent3CleanupFunc ( hkAgent3::AgentType type )
{
	return m_agent3Func[type ].m_cleanupFunc;
}

hkAgent3::RemovePointFunc  hkCollisionDispatcher::getAgent3RemovePointFunc ( hkAgent3::AgentType type )
{
	return m_agent3Func[type ].m_removePointFunc;
}

hkAgent3::CommitPotentialFunc  hkCollisionDispatcher::getAgent3CommitPotentialFunc ( hkAgent3::AgentType type )
{
	return m_agent3Func[type ].m_commitPotentialFunc;
}

hkAgent3::CreateZombieFunc  hkCollisionDispatcher::getAgent3CreateZombieFunc ( hkAgent3::AgentType type )
{
	return m_agent3Func[type ].m_createZombieFunc;
}

hkAgent3::UpdateFilterFunc  hkCollisionDispatcher::getAgent3UpdateFilterFunc ( hkAgent3::AgentType type )
{
	return m_agent3Func[type ].m_updateFilterFunc;
}

hkAgent3::InvalidateTimFunc  hkCollisionDispatcher::getAgent3InvalidateTimFunc ( hkAgent3::AgentType type )
{
	return m_agent3Func[type ].m_invalidateTimFunc;
}

hkAgent3::WarpTimeFunc  hkCollisionDispatcher::getAgent3WarpTimeFunc ( hkAgent3::AgentType type )
{
	return m_agent3Func[type ].m_warpTimeFunc;
}

hkAgent3::ProcessFunc  hkCollisionDispatcher::getAgent3ProcessFunc ( hkAgent3::AgentType type )
{
	return m_agent3Func[type ].m_processFunc;
}

hkAgent3::SepNormalFunc hkCollisionDispatcher::getAgent3SepNormalFunc( hkAgent3::AgentType type )
{
	return m_agent3Func[type ].m_sepNormalFunc;
}

hkAgent3::Symmetric     hkCollisionDispatcher::getAgent3Symmetric( hkAgent3::AgentType type )
{
	return m_agent3Func[type ].m_symmetric;
}

hkAgent3::AgentType hkCollisionDispatcher::getAgent3Type( hkShapeType typeA, hkShapeType typeB, hkBool predictive ) const
{
	if ( predictive )
	{
		return m_agent3TypesPred[typeA] [ typeB ];
	}
	return m_agent3Types[typeA] [ typeB ];
}

hkCollisionAgent* hkCollisionDispatcher::getNewCollisionAgent(const hkCdBody& collA,  const hkCdBody& collB,
															  const hkCollisionInput& environment, hkContactMgr* mgr) const
{
	const hkShapeType typeA = collA.getShape()->getType();
	const hkShapeType typeB = collB.getShape()->getType();

	HK_ASSERT2(0x7f1a216c,  unsigned(typeA) < HK_MAX_SHAPE_TYPE, "You can only access types between [0.." << HK_MAX_SHAPE_TYPE-1 << "]");
	HK_ASSERT2(0x6ecac429,  unsigned(typeB) < HK_MAX_SHAPE_TYPE, "You can only access types between [0.." << HK_MAX_SHAPE_TYPE-1 << "]");

	//CreateFunc f = m_agentCreationTable[ typeA ] [ typeB ];
	hkCollisionDispatcher::IsAgentPredictive predictive = static_cast<hkCollisionDispatcher::IsAgentPredictive>((int)environment.m_createPredictiveAgents);
	CreateFunc f = getCollisionAgentCreationFunction(typeA, typeB, predictive);

	return f(collA, collB, environment, mgr);
}


hkBool hkCollisionDispatcher::hasAlternateType( hkShapeType type, hkShapeType alternateType )
{
	HK_ASSERT2(0x4fa8c66f,  unsigned(type) < HK_MAX_SHAPE_TYPE, "You can only access types between [0.." << HK_MAX_SHAPE_TYPE-1 << "]");
	HK_ASSERT2(0x135739e6,  unsigned(alternateType) < HK_MAX_SHAPE_TYPE, "You can only access types between [0.." << HK_MAX_SHAPE_TYPE-1 << "]");

	return 0 != ( m_hasAlternateType[type] & (1<<alternateType) );

}

hkCollisionDispatcher::CollisionQualityIndex hkCollisionDispatcher::getCollisionQualityIndex( hkCollidableQualityType a, hkCollidableQualityType b)
{
	HK_ASSERT2(0xf056aef3,  unsigned(a) < HK_COLLIDABLE_QUALITY_MAX, "You can only use types between [0.." << HK_COLLIDABLE_QUALITY_MAX-1 << "]");
	HK_ASSERT2(0xf056aef4,  unsigned(b) < HK_COLLIDABLE_QUALITY_MAX, "You can only use types between [0.." << HK_COLLIDABLE_QUALITY_MAX-1 << "]");
	return m_collisionQualityTable[a][b];
}

hkCollisionQualityInfo* hkCollisionDispatcher::getCollisionQualityInfo( CollisionQualityIndex index)
{
	HK_ASSERT2(0xf056aef4,  unsigned(index) < HK_MAX_COLLISION_QUALITIES, "You can only use types between [0.." << HK_MAX_COLLISION_QUALITIES-1 << "]");
	return &m_collisionQualityInfo[index];
}


/*
* Havok SDK - PUBLIC RELEASE, BUILD(#20060902)
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
