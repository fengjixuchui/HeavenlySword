/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkcollide/hkCollide.h>

#include <hkcollide/dispatch/agent3bridge/hkAgent3Bridge.h>
#include <hkcollide/dispatch/hkCollisionDispatcher.h>


int hkAgent3Bridge::registerAgent3( hkCollisionDispatcher* dispatcher )
{
	hkCollisionDispatcher::Agent3Funcs f;
	f.m_commitPotentialFunc = commitPotential;
	f.m_createZombieFunc    = createZombie;
	f.m_removePointFunc     = removePoint;

	f.m_createFunc   = create;
	f.m_processFunc  = process;
	f.m_sepNormalFunc = HK_NULL; //sepNormalFunc;
	f.m_cleanupFunc  = HK_NULL;
	f.m_destroyFunc  = destroy;
	f.m_updateFilterFunc = updateFilter;
	f.m_invalidateTimFunc = invalidateTim;
	f.m_warpTimeFunc = warpTime;
	f.m_isPredictive = true;
	int id = dispatcher->registerAgent3( f, HK_SHAPE_ALL, HK_SHAPE_ALL );
	return id;
}

hkAgentData* hkAgent3Bridge::create  ( const hkAgent3Input& input, hkAgentEntry* entry, hkAgentData* freeMemory )
{
	hkCollisionAgent* subAgent = input.m_input->m_dispatcher->getNewCollisionAgent( input.m_bodyA[0], input.m_bodyB[0], input.m_input[0], input.m_contactMgr );
	entry->m_userData = (hkUlong)subAgent;
	entry->m_streamCommand = hkAgent3::STREAM_CALL_AGENT;
	entry->m_numContactPoints = hkUchar(-1);
	return freeMemory;
}


hkAgentData* hkAgent3Bridge::process ( const hkAgent3ProcessInput& input, hkAgentEntry* entry, hkAgentData* agentData, hkVector4* separatingNormalOut, hkProcessCollisionOutput& result)
{
	HK_WARN_ONCE(0xf0ff00b0, "hkAgent3Bridge::process should never be called" ); // should never be called, as the stuff is inlined anyway
	hkCollisionAgent* agent = reinterpret_cast<hkCollisionAgent*>(entry->m_userData);
	agent->processCollision( input.m_bodyA[0], input.m_bodyB[0], input.m_input[0], result );
	return agentData;
}



void hkAgent3Bridge::destroy ( hkAgentEntry* entry, hkAgentData* agentData, hkContactMgr* mgr, hkCollisionConstraintOwner& constraintOwner )
{
	hkCollisionAgent* agent = reinterpret_cast<hkCollisionAgent*>(entry->m_userData);
	agent->cleanup(constraintOwner);
}

void hkAgent3Bridge::updateFilter(hkAgentEntry* entry, hkAgentData* agentData, hkCdBody& bodyA, hkCdBody& bodyB, const hkCollisionInput& input, hkCollisionConstraintOwner& constraintOwner)
{
	hkCollisionAgent* agent = reinterpret_cast<hkCollisionAgent*>(entry->m_userData);
	agent->updateShapeCollectionFilter(bodyA, bodyB, input, constraintOwner);
}

void hkAgent3Bridge::invalidateTim(hkAgentEntry* entry, hkAgentData* agentData, hkCollisionInput& input)
{
	hkCollisionAgent* agent = reinterpret_cast<hkCollisionAgent*>(entry->m_userData);
	agent->invalidateTim(input);
}

void hkAgent3Bridge::warpTime(hkAgentEntry* entry, hkAgentData* agentData, hkTime oldTime, hkTime newTime, hkCollisionInput& input)
{
	hkCollisionAgent* agent = reinterpret_cast<hkCollisionAgent*>(entry->m_userData);
	agent->warpTime(oldTime, newTime, input);
}


void HK_CALL hkAgent3Bridge::removePoint( hkAgentEntry* entry, hkAgentData* agentData, hkContactPointId idToRemove )
{
	hkCollisionAgent* agent = reinterpret_cast<hkCollisionAgent*>(entry->m_userData);
	agent->removePoint( idToRemove );
}

void HK_CALL hkAgent3Bridge::commitPotential( hkAgentEntry* entry, hkAgentData* agentData, hkContactPointId newId )
{
	hkCollisionAgent* agent = reinterpret_cast<hkCollisionAgent*>(entry->m_userData);
	agent->commitPotential( newId );
}

void HK_CALL hkAgent3Bridge::createZombie( hkAgentEntry* entry, hkAgentData* agentData, hkContactPointId idTobecomeZombie )
{
	hkCollisionAgent* agent = reinterpret_cast<hkCollisionAgent*>(entry->m_userData);
	agent->createZombie( idTobecomeZombie );
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
