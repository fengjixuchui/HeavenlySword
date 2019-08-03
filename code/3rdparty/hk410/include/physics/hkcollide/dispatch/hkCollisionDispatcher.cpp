/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkcollide/hkCollide.h>
#include <hkbase/debugutil/hkStatisticsCollector.h>

#include <hkcollide/dispatch/contactmgr/hkContactMgrFactory.h>
#include <hkinternal/collide/broadphase/3axissweep/hk3AxisSweep.h>
#include <hkcollide/dispatch/hkAgentRegisterUtil.h>
#include <hkcollide/dispatch/agent3bridge/hkAgent3Bridge.h>
#include <hkcollide/agent/null/hkNullAgent.h>

#if ( HK_POINTER_SIZE == 4 )
#	define HK_AGENT3_AGENT_SIZE 128
#	define HK_AGENT3_SECTOR_SIZE 512
#else //XXX
#	define HK_AGENT3_AGENT_SIZE 160
#	define HK_AGENT3_SECTOR_SIZE 960
#endif

static void HK_CALL hkNullGetPenetrationsFunc   (const hkCdBody& A, const hkCdBody& B, const hkCollisionInput& input, hkCdBodyPairCollector& collector )
{
	HK_ON_DEBUG( const char* typeA = hkGetShapeTypeName( A.getShape()->getType() ) );
	HK_ON_DEBUG( const char* typeB = hkGetShapeTypeName( B.getShape()->getType() ) );
	HK_WARN_ONCE(0x38206782,  "Have you called hkAgentRegisterUtil::registerAllAgents? Do not know how to dispatch types " << typeA << " vs " << typeB);
}

static void HK_CALL hkNullGetClosestPointsFunc (const hkCdBody& A, const hkCdBody& B, const hkCollisionInput& input, hkCdPointCollector& output )
{
	HK_ON_DEBUG( const char* typeA = hkGetShapeTypeName( A.getShape()->getType() ) );
	HK_ON_DEBUG( const char* typeB = hkGetShapeTypeName( B.getShape()->getType() ) );
	HK_WARN_ONCE(0x19fb8b53,  "Have you called hkAgentRegisterUtil::registerAllAgents? Do not know how to dispatch types " << typeA << " vs " << typeB);
}

static void HK_CALL hkNullLinearCastFunc       (const hkCdBody& A, const hkCdBody& B, const hkLinearCastCollisionInput& input, hkCdPointCollector& castCollector, hkCdPointCollector* startCollector )
{
	HK_ON_DEBUG( const char* typeA = hkGetShapeTypeName( A.getShape()->getType() ) );
	HK_ON_DEBUG( const char* typeB = hkGetShapeTypeName( B.getShape()->getType() ) );
	HK_WARN_ONCE(0x523361cf,  "Have you called hkAgentRegisterUtil::registerAllAgents? Do not know how to dispatch types " << typeA << " vs " << typeB);
}

namespace hkNullAgent3
{
	hkAgentData* HK_CALL create( const hkAgent3Input& input, hkAgentEntry* entry, hkAgentData* freeMemory )
	{
		HK_ON_DEBUG( const char* typeA = hkGetShapeTypeName( input.m_bodyA->getShape()->getType() ) );
		HK_ON_DEBUG( const char* typeB = hkGetShapeTypeName( input.m_bodyB->getShape()->getType() ) );
		HK_WARN_ONCE(0x523361df,  "Have you called hkAgentRegisterUtil::registerAllAgents? Do not know how to dispatch types " << typeA << " vs " << typeB);
		entry->m_streamCommand = hkAgent3::STREAM_NULL;
		return freeMemory;
	}

	void HK_CALL destroy( hkAgentEntry* entry, hkAgentData* agentData, hkContactMgr* mgr, hkCollisionConstraintOwner& constraintOwner )
	{

	}

	hkAgentData*  HK_CALL process( const hkAgent3ProcessInput& input, hkAgentEntry* entry, hkAgentData* agentData, hkVector4* separatingNormalOut, hkProcessCollisionOutput& result)
	{
		return agentData;
	}
}



void hkCollisionDispatcher::resetCreationFunctions()
{
	//
	// reset all response types
	//
	m_numAgent2Types = 1;
	m_numAgent3Types = 1;
	{
		for (int i = 0; i < HK_MAX_SHAPE_TYPE; ++i)
		{
			for (int j = 0; j < HK_MAX_SHAPE_TYPE; ++j)
			{
				if ( m_debugAgent2Table )
				{
					m_debugAgent2Table    [0][i][j].m_priority = 100;
					m_debugAgent2TablePred[0][i][j].m_priority = 100;
					m_debugAgent3Table    [0][i][j].m_priority = 100;
					m_debugAgent3TablePred[0][i][j].m_priority = 100;
				}
				m_agent2Types[i][j] = HK_AGENT_TYPE_NULL;
				m_agent3Types[i][j] = HK_AGENT_TYPE_NULL;
				m_agent2TypesPred[i][j] = HK_AGENT_TYPE_NULL;
				m_agent3TypesPred[i][j] = HK_AGENT_TYPE_NULL;
			}
		}
	}

	AgentFuncs& f2 = m_agent2Func[HK_AGENT_TYPE_NULL];
	f2.m_createFunc = hkNullAgent::createNullAgent;
	f2.m_getPenetrationsFunc = hkNullAgent::staticGetPenetrations;
	f2.m_getClosestPointFunc = hkNullAgent::staticGetClosestPoints;
	f2.m_linearCastFunc = hkNullAgent::staticLinearCast;
	f2.m_isFlipped = false;
	f2.m_isPredictive = true;


	Agent3FuncsIntern& f3 = m_agent3Func[HK_AGENT_TYPE_NULL];
	f3.m_createFunc   = hkNullAgent3::create;
	f3.m_destroyFunc  = hkNullAgent3::destroy;
	f3.m_cleanupFunc  = HK_NULL; //hkNullAgent3::cleanup;
	f3.m_removePointFunc  = HK_NULL; //hkNullAgent3::cleanup;
	f3.m_commitPotentialFunc  = HK_NULL; //hkNullAgent3::cleanup;
	f3.m_createZombieFunc  = HK_NULL; //hkNullAgent3::cleanup;
	f3.m_updateFilterFunc  = HK_NULL; //hkNullAgent3::cleanup;
	f3.m_sepNormalFunc = HK_NULL; //hkNullAgent3::sepNormal;
	f3.m_invalidateTimFunc = HK_NULL;
	f3.m_warpTimeFunc = HK_NULL;
	f3.m_processFunc  = hkNullAgent3::process;
	f3.m_symmetric    = hkAgent3::IS_SYMMETRIC;

	// register default agent silently
	HK_ON_DEBUG( int bridgeId = ) hkAgent3Bridge::registerAgent3(this);
	HK_ASSERT( 0xf05dae13, bridgeId == HK_AGENT_TYPE_BRIDGE );
	m_collisionAgentRegistered = false;

	// reset the priorities of the bridge
	if ( m_debugAgent3Table )
	{
		for (int i = 0; i < HK_MAX_SHAPE_TYPE; ++i)
		{
			for (int j = 0; j < HK_MAX_SHAPE_TYPE; ++j)
			{
				m_debugAgent3Table    [0][i][j].m_priority = 100;
				m_debugAgent3TablePred[0][i][j].m_priority = 100;
			}
		}
	}
}


hkCollisionDispatcher::hkCollisionDispatcher(CreateFunc defaultCollisionAgent, hkContactMgrFactory* defaultContactMgrFactory )
:	m_defaultCollisionAgent(defaultCollisionAgent)
{
#ifdef HK_DEBUG
	m_debugAgent2Table = hkAllocate<hkCollisionDispatcher::DebugTable>(1, HK_MEMORY_CLASS_COLLIDE );
	m_debugAgent2TablePred = hkAllocate<hkCollisionDispatcher::DebugTable>(1, HK_MEMORY_CLASS_COLLIDE);
	m_debugAgent3Table = hkAllocate<hkCollisionDispatcher::DebugTable>(1, HK_MEMORY_CLASS_COLLIDE);
	m_debugAgent3TablePred = hkAllocate<hkCollisionDispatcher::DebugTable>(1, HK_MEMORY_CLASS_COLLIDE);
#else 
	m_debugAgent2Table		= HK_NULL;
	m_debugAgent2TablePred  = HK_NULL;
	m_debugAgent3Table		= HK_NULL;
	m_debugAgent3TablePred	= HK_NULL;
#endif

	m_collisionAgentRegistered = false;
	m_checkEnabled = true;
	m_numAgent3Types = 0;
	
	//
	// reset all agents
	//
	{
		for ( int i = 0; i < HK_MAX_RESPONSE_TYPE; i++ )
		{
			for ( int j = 0; j < HK_MAX_RESPONSE_TYPE; j++ )
			{
				m_contactMgrFactory[i][j] = defaultContactMgrFactory;
				if ( defaultContactMgrFactory )
				{
					defaultContactMgrFactory->addReference();
				}
			}
		}
	}


	//
	//	reset all shape rules
	//
	{
		for (int i = 0; i < HK_MAX_SHAPE_TYPE; i++ )
		{
			m_hasAlternateType[i] = 1<<i;
		}
	}

	resetCreationFunctions();


	// we know that our bridge agent is very small, so need to 
	// reserve lots of memory for 32 bytes
	m_agent3AgentSize  = HK_AGENT3_AGENT_SIZE;
	m_agent3SectorSize = HK_AGENT3_SECTOR_SIZE;
	m_agent3Registered = false;
}

void hkCollisionDispatcher::setEnableChecks( hkBool checkEnabled)
{
	m_checkEnabled = checkEnabled;
}

void hkCollisionDispatcher::disableDebugging()
{
	if ( m_debugAgent2Table )
	{
		hkDeallocate(m_debugAgent2Table);
		hkDeallocate(m_debugAgent2TablePred);
		hkDeallocate(m_debugAgent3Table);
		hkDeallocate(m_debugAgent3TablePred);
		m_debugAgent2Table		= HK_NULL;
		m_debugAgent2TablePred  = HK_NULL;
		m_debugAgent3Table		= HK_NULL;
		m_debugAgent3TablePred	= HK_NULL;
	}
}

hkCollisionDispatcher::~hkCollisionDispatcher()
{
	disableDebugging();

	{
		for ( int i = 0; i < HK_MAX_RESPONSE_TYPE; i++ )
		{
			for ( int j = 0; j < HK_MAX_RESPONSE_TYPE; j++ )
			{
				if (m_contactMgrFactory[i][j])
				{
					m_contactMgrFactory[i][j]->removeReference();
				}
			}
		}
	}
}

void hkCollisionDispatcher::registerCollisionAgent(AgentFuncs& f, hkShapeType typeA, hkShapeType typeB)
{
	HK_ASSERT2( 0xad000301, m_numAgent2Types < HK_MAX_AGENT2_TYPES, "You are running out of agent2 entries");


	//
	//	Register tables
	//
	m_agent2Func[ m_numAgent2Types ] = f;

	internalRegisterCollisionAgent( m_agent3Types, HK_AGENT_TYPE_BRIDGE, typeA, typeB, typeA, typeB, m_debugAgent3Table, 0 );
	internalRegisterCollisionAgent( m_agent2Types, m_numAgent2Types,     typeA, typeB, typeA, typeB, m_debugAgent2Table, 0 );

	if ( f.m_isPredictive )
	{
		internalRegisterCollisionAgent( m_agent3TypesPred, HK_AGENT_TYPE_BRIDGE, typeA, typeB, typeA, typeB, m_debugAgent3TablePred, 0 );
		internalRegisterCollisionAgent( m_agent2TypesPred, m_numAgent2Types,     typeA, typeB, typeA, typeB, m_debugAgent2TablePred, 0 );
	}
	m_numAgent2Types++;
}

int hkCollisionDispatcher::registerAgent3( Agent3Funcs& f, hkShapeType typeA, hkShapeType typeB )
{
	m_agent3AgentSize  = HK_AGENT3_AGENT_SIZE;
	m_agent3SectorSize = HK_AGENT3_SECTOR_SIZE;
	m_agent3Registered = true;

	HK_ASSERT2( 0xf0180404, m_numAgent3Types < HK_MAX_AGENT3_TYPES, "You are running out of agent3 entries");

	//
	//	check for symmetric
	//
	Agent3FuncsIntern f3;
	(Agent3Funcs&)f3 = f;
	f3.m_symmetric = hkAgent3::IS_SYMMETRIC;

	if ( (typeA != typeB) && (f.m_ignoreSymmetricVersion == false) )
	{
		f3.m_symmetric = hkAgent3::IS_NOT_SYMMETRIC_AND_FLIPPED;
		m_agent3Func[ m_numAgent3Types ] = f3;
		internalRegisterCollisionAgent( m_agent3Types, m_numAgent3Types, typeB, typeA, typeB, typeA, m_debugAgent3Table, 0 );

		if ( f3.m_isPredictive )
		{
			internalRegisterCollisionAgent( m_agent3TypesPred, m_numAgent3Types, typeA, typeB, typeA, typeB, m_debugAgent3TablePred, 0 );
		}
		m_numAgent3Types++;

		f3.m_symmetric = hkAgent3::IS_NOT_SYMMETRIC;
	}
		
	m_agent3Func[ m_numAgent3Types ] = f3;
	internalRegisterCollisionAgent( m_agent3Types, m_numAgent3Types, typeA, typeB, typeA, typeB, m_debugAgent3Table, 0 );
	
	if ( f3.m_isPredictive )
	{
		internalRegisterCollisionAgent( m_agent3TypesPred, m_numAgent3Types, typeA, typeB, typeA, typeB, m_debugAgent3TablePred, 0 );
	}
	return m_numAgent3Types++;
}


void hkCollisionDispatcher::internalRegisterCollisionAgent(hkUchar agentTypesTable[HK_MAX_SHAPE_TYPE][HK_MAX_SHAPE_TYPE], int agentType, hkShapeType typeA, hkShapeType typeB, hkShapeType origA, hkShapeType origB, DebugTable *debugTable, int depth)
{
	HK_ASSERT2(0x53270a5c,  HK_SHAPE_ALL <= typeA && typeA < HK_MAX_SHAPE_TYPE, "You can only access types between [HK_SHAPE_ALL .." << HK_MAX_SHAPE_TYPE << "]");
	HK_ASSERT2(0x328325a1,  HK_SHAPE_ALL <= typeB && typeB < HK_MAX_SHAPE_TYPE, "You can only access types between [HK_SHAPE_ALL .." << HK_MAX_SHAPE_TYPE << "]");
	HK_ASSERT2(0x76e7dd75,  depth < 10, "Infinite loop: your alternate shape types have a circular dependency");
	m_collisionAgentRegistered = true;


	//
	//	Traverse the hierarchy to more specialized agents
	//  If there is a rule:   shapeA inheritsFrom shapeB    and shapeB is either origA or origB, call this function with shapeA
	//
	{
		for ( int i = 0; i < m_shapeInheritance.getSize(); i++ )
		{
			ShapeInheritance& si = m_shapeInheritance[i];

			if ( si.m_alternateType == typeA  )
			{
				internalRegisterCollisionAgent( agentTypesTable, agentType, si.m_primaryType, typeB, origA, origB, debugTable, depth+1 );
			}
			if ( si.m_alternateType == typeB )
			{
				internalRegisterCollisionAgent( agentTypesTable, agentType, typeA, si.m_primaryType, origA, origB, debugTable, depth+1 );
			}
		}
	}

	{
		//
		//	Some helper code to replace  HK_SHAPE_ALL by an iteration
		//
		int beginA = typeA;
		int beginB = typeB;
		int endA = typeA+1;
		int endB = typeB+1;
		int priority = depth;
		if ( typeA == HK_SHAPE_ALL )
		{
			beginA = HK_FIRST_SHAPE_TYPE;
			endA = HK_MAX_SHAPE_TYPE;
			priority++;
		}
		if ( typeB == HK_SHAPE_ALL )
		{
			beginB = HK_FIRST_SHAPE_TYPE;
			endB = HK_MAX_SHAPE_TYPE;
			priority++;
		}

		//
		//	Iterate over all combinations (if you are not using HK_SHAPE_ALL, than this results in a single iteration)
		//
		for (int a = beginA; a < endA; a++)
		{
			for (int b = beginB; b < endB; b++)
			{
				//
				//	Fill in the agent table
				//
				agentTypesTable[a][b] = (hkUchar)agentType;

				//
				//	Do some debugging
				//
				if ( debugTable )
				{
					DebugEntry& de = debugTable[0][a][b]; 

					if ( m_checkEnabled && priority > de.m_priority )
					{
						//
						//	error
						//
						const char* oldA = hkGetShapeTypeName( hkShapeType(de.m_typeA) );
						const char* oldB = hkGetShapeTypeName( hkShapeType(de.m_typeB) );
						const char* newA = hkGetShapeTypeName( hkShapeType(typeA) );
						const char* newB = hkGetShapeTypeName( hkShapeType(typeB) );
						char buffer[1024];
						hkString::snprintf( buffer, 1000,
							"Agent handling types <%s-%s> would override more specialized agent <%s-%s>\n"
							"Maybe the order of registering your collision agent is wrong, make sure you register your alternate type agents first",
							newA,newB, oldA,oldB );
						HK_ASSERT2(0x62b50e8a,  0, buffer );

					}
					HK_ASSERT2(0x4271c5c2,  priority < 256 && origA < 256 && origB <256, "Currently there is a limitation of 256 shape types" );

					de.m_priority = char(priority);
					de.m_typeA = char(origA);
					de.m_typeB = char(origB);
				}
			}
		}
	}
}



// subfunction which keeps the m_hasAlternateType up to date
void hkCollisionDispatcher::updateHasAlternateType( hkShapeType primaryType, hkShapeType alternateType, int depth )
{
	HK_ASSERT2(0x33af9996,  depth < 100, "Your shape dependency graph contains a cycle");

		// add all the children
	m_hasAlternateType[ primaryType ] = m_hasAlternateType[ primaryType ] | m_hasAlternateType[ alternateType ];


	//	Traverse up the hierarchy
	{
		for (int i = 0; i < m_shapeInheritance.getSize(); i++)
		{
			ShapeInheritance& si = m_shapeInheritance[i];
			if ( si.m_alternateType == primaryType  )
			{
				// recurse up
				updateHasAlternateType( si.m_primaryType, si.m_alternateType, depth+1 );
			}
		}
	}
}

void hkCollisionDispatcher::registerAlternateShapeType( hkShapeType primaryType, hkShapeType alternateType )
{
	HK_ASSERT2(0x3b2d0f10,  HK_FIRST_SHAPE_TYPE <= primaryType && primaryType < HK_MAX_SHAPE_TYPE, "You can only access types between [HK_FIRST_SHAPE_TYPE ..." << HK_MAX_SHAPE_TYPE << "]");
	HK_ASSERT2(0x549a2997,  HK_FIRST_SHAPE_TYPE <= alternateType && alternateType < HK_MAX_SHAPE_TYPE, "You can only access types between [HK_FIRST_SHAPE_TYPE ..." << HK_MAX_SHAPE_TYPE << "]");
	HK_ASSERT2(0x350560c3,  primaryType != alternateType, "Your primary type must be different from the alternateType" );

	//
	//	If we already have registered agents, we have to unregister them all,
	//  register our shape type and reregister them all again
	//
	if ( m_collisionAgentRegistered != false )
	{
		HK_ASSERT2(0x70111344, 0,  "You have to register all shapeTypes before call registerCollisionAgent() ");
	}

	//
	//	Search for duplicated entries
	//
	{
		for (int i = 0; i < m_shapeInheritance.getSize(); i++)
		{
			ShapeInheritance& si = m_shapeInheritance[i];
			if ( si .m_primaryType == primaryType && si.m_alternateType == alternateType )
			{
				HK_WARN(0x3e3a6c67, "Agent registered twice, deleting the original entry");
				m_shapeInheritance.removeAtAndCopy(i);
				i--;
			}
		}
	}

	//
	//	updateHasAlternateType
	//
	{
		updateHasAlternateType( primaryType, alternateType, 0 );
	}


	//
	//	Add to our list
	//
	{
		ShapeInheritance& si = m_shapeInheritance.expandOne();
		si.m_primaryType = primaryType;
		si.m_alternateType = alternateType;
	}
}



void hkCollisionDispatcher::registerContactMgrFactory( hkContactMgrFactory* fac, int responseA, int responseB )
{
	HK_ASSERT2(0x14b1ad5b,  0 <= responseA && responseA < HK_MAX_RESPONSE_TYPE, "Response Type A is outside [ 0 .. " << HK_MAX_RESPONSE_TYPE << "]" );
	HK_ASSERT2(0x154b139d,  0 <= responseB && responseB < HK_MAX_RESPONSE_TYPE, "Response Type B is outside [ 0 .. " << HK_MAX_RESPONSE_TYPE << "]" );

	fac->addReference();
	m_contactMgrFactory[ responseB] [responseA ]->removeReference();
	m_contactMgrFactory[ responseB] [responseA ] = fac;

	fac->addReference();
	m_contactMgrFactory[ responseA ][ responseB ]->removeReference();
	m_contactMgrFactory[ responseA ][ responseB ] = fac;
}

void hkCollisionDispatcher::registerContactMgrFactoryWithAll( hkContactMgrFactory* fac, int responseA )
{
	HK_ASSERT2(0x478d2d55,  responseA < HK_MAX_RESPONSE_TYPE, "You can only register types between [0.." << HK_MAX_RESPONSE_TYPE << "]");
	
	for (int i = 0; i < HK_MAX_RESPONSE_TYPE; i++ )
	{
		fac->addReference();

		m_contactMgrFactory[ i ][ responseA ]->removeReference();
		m_contactMgrFactory[ i ][ responseA ] = fac;

		fac->addReference();

		m_contactMgrFactory[ responseA ][ i ]->removeReference();
		m_contactMgrFactory[ responseA ][ i ] = fac;
	}
}

void hkCollisionDispatcher::debugPrintTable()
{
	HK_REPORT_SECTION_BEGIN(0x5e4345e4, "hkCollisionDispatcher::debugPrintTable" );

	if ( !m_debugAgent2Table || !m_debugAgent2TablePred)
	{
		HK_WARN( 0xf0324455, "Debugging disabled, cannot print debug table" );
		return;
	}
	char buf[255];
	{
		for (int a = 0; a < HK_MAX_SHAPE_TYPE; a++)
		{
			hkString str = hkString("\nEntries for (continuous)") + hkGetShapeTypeName( hkShapeType(a) );
			HK_REPORT(str);

			for (int b = HK_FIRST_SHAPE_TYPE; b < HK_MAX_SHAPE_TYPE; b++)
			{
				DebugEntry& de = m_debugAgent2TablePred[0][a][b];
				if ( de.m_priority >= 100 )
				{
					continue;
				}

				const char* sB = hkGetShapeTypeName( hkShapeType(b) );
				const char* oldA = hkGetShapeTypeName( hkShapeType(de.m_typeA) );
				const char* oldB = hkGetShapeTypeName( hkShapeType(de.m_typeB) );
		
				hkString::snprintf(buf, 255, "vs %30s <%i:%s-%s>", sB, de.m_priority, oldA, oldB );
				HK_REPORT(buf);
			}
		}
	}
	{
		for (int a = 0; a < HK_MAX_SHAPE_TYPE; a++)
		{
			hkString str = hkString("\nEntries for (discrete)") + hkGetShapeTypeName( hkShapeType(a) );
			HK_REPORT(str);

			for (int b = HK_FIRST_SHAPE_TYPE; b < HK_MAX_SHAPE_TYPE; b++)
			{
				DebugEntry& de = m_debugAgent2Table[0][a][b];
				if ( de.m_priority >= 100 )
				{
					continue;
				}

				const char* sB = hkGetShapeTypeName( hkShapeType(b) );
				const char* oldA = hkGetShapeTypeName( hkShapeType(de.m_typeA) );
				const char* oldB = hkGetShapeTypeName( hkShapeType(de.m_typeB) );
		
				hkString::snprintf(buf, 255, "vs %30s <%i:%s-%s>", sB, de.m_priority, oldA, oldB );
				HK_REPORT(buf);
			}
		}
	}
	HK_REPORT_SECTION_END();
}

void hkCollisionDispatcher::calcStatistics( hkStatisticsCollector* collector ) const
{
	collector->beginObject(HK_NULL, collector->MEMORY_INSTANCE, this);

	if ( m_debugAgent2Table )
	{
		collector->addChunk( "DebugTable", collector->MEMORY_ENGINE, m_debugAgent2Table, sizeof(*m_debugAgent2Table) );
		collector->addChunk( "DebugTable", collector->MEMORY_ENGINE, m_debugAgent2TablePred, sizeof(*m_debugAgent2TablePred) );
		collector->addChunk( "DebugTable", collector->MEMORY_ENGINE, m_debugAgent3Table, sizeof(*m_debugAgent3Table) );
		collector->addChunk( "DebugTable", collector->MEMORY_ENGINE, m_debugAgent3TablePred, sizeof(*m_debugAgent3TablePred) );
	}
	collector->endObject();
}



void hkCollisionDispatcher::initCollisionQualityInfo( InitCollisionQualityInfo& input )
{
	m_expectedMinPsiDeltaTime = input.m_minDeltaTime;
	m_expectedMaxLinearVelocity = input.m_maxLinearVelocity;

	// This is the distance an objects travels under gravity within one timestep
	float distPerTimeStep = 0.5f * input.m_gravityLength * input.m_minDeltaTime * input.m_minDeltaTime;

	{
		hkCollisionQualityInfo& sq = m_collisionQualityInfo[ COLLISION_QUALITY_PSI ];

		sq.m_keepContact         = input.m_collisionTolerance;
		sq.m_create4dContact     = input.m_collisionTolerance;

		sq.m_manifoldTimDistance = input.m_collisionTolerance;
		if ( input.m_enableNegativeManifoldTims )
		{
			sq.m_manifoldTimDistance = -2.0f * distPerTimeStep;
		}
		sq.m_createContact       = input.m_collisionTolerance;
		if ( input.m_enableNegativeToleranceToCreateNon4dContacts )
		{
			sq.m_createContact       = -1.0f * distPerTimeStep;
		}

		sq.m_maxContraintViolation    = HK_REAL_MAX;
		sq.m_useContinuousPhysics	  = false;
		sq.m_constraintPriority       = input.m_defaultConstraintPriority;

		hkReal d = -1e20f;
		sq.m_minSeparation		= d * 0.5f;
		sq.m_minExtraSeparation	= d * 0.5f;
		sq.m_toiSeparation		= d * 0.1f;
		sq.m_toiExtraSeparation	= d * 0.1f;
		sq.m_toiAccuracy		= hkMath::fabs(d * 0.05f);
		sq.m_minSafeDeltaTime	= 1.0f;
		sq.m_minAbsoluteSafeDeltaTime = 1.0f;
		sq.m_minToiDeltaTime	= 1.0f;
	}

	// copy the PSI-quality-info values as defaults for further quality infos
	{
		m_collisionQualityInfo[ COLLISION_QUALITY_TMP_EXPAND_MANIFOLD ] = m_collisionQualityInfo[ COLLISION_QUALITY_PSI ];
		m_collisionQualityInfo[ COLLISION_QUALITY_TOI ]                 = m_collisionQualityInfo[ COLLISION_QUALITY_PSI ];
		m_collisionQualityInfo[ COLLISION_QUALITY_TOI_HIGHER ]          = m_collisionQualityInfo[ COLLISION_QUALITY_PSI ];
		m_collisionQualityInfo[ COLLISION_QUALITY_TOI_FORCED ]          = m_collisionQualityInfo[ COLLISION_QUALITY_PSI ];
	}  

	{
		hkCollisionQualityInfo& sq = m_collisionQualityInfo[ COLLISION_QUALITY_TMP_EXPAND_MANIFOLD ];
		sq.m_manifoldTimDistance = input.m_collisionTolerance;
		sq.m_create4dContact     = input.m_collisionTolerance;
		sq.m_createContact       = input.m_collisionTolerance;
	}

	// This is the smallest thickness of the object diveded by the allowedPenetration
	const hkReal radiusToAllowedPenetrationRatio = 5.0f; // This increases collisionAgent's tolerance.m_safeDeltaTimeStep

	{

		hkCollisionQualityInfo& sq = m_collisionQualityInfo[ COLLISION_QUALITY_TOI ];

		if (input.m_wantContinuousCollisionDetection)
		{
			const int    numToisTillAllowedPenetration     = 3;		// The number of Tois till we reach the full penetration
			const hkReal maxInitialRelPenetration          = 0.6f;	// The max relative initial penetration distance for the first Toi


			sq.m_minSeparation		= -1.0f * maxInitialRelPenetration;
			sq.m_minExtraSeparation	= -1.0f * (1.0f - maxInitialRelPenetration) / (numToisTillAllowedPenetration-1);
			sq.m_toiSeparation		= -0.5f * maxInitialRelPenetration;
			sq.m_toiExtraSeparation	= -0.7f * (1.0f - maxInitialRelPenetration) / (numToisTillAllowedPenetration-1);
			sq.m_toiAccuracy		= -0.3f * sq.m_toiExtraSeparation;
			sq.m_maxContraintViolation  = -1.0f * sq.m_minExtraSeparation;

			sq.m_minSafeDeltaTime   = radiusToAllowedPenetrationRatio / input.m_maxLinearVelocity;
			sq.m_minAbsoluteSafeDeltaTime   = 0.005f / input.m_maxLinearVelocity; // Should work for bodies 1cm in size.
			sq.m_minToiDeltaTime	= -2.0f * sq.m_minExtraSeparation / input.m_maxLinearVelocity;

			sq.m_constraintPriority = input.m_toiConstraintPriority;
			sq.m_useContinuousPhysics   = true;

			HK_ASSERT2(0xad2342da, sq.m_minToiDeltaTime > 0.0f && sq.m_minSafeDeltaTime > 0.0f, "Internal error: incorrect initialization of surface qualities.");
}
	}

	{
		hkCollisionQualityInfo& sq = m_collisionQualityInfo[ COLLISION_QUALITY_TOI_HIGHER ];

		if (input.m_wantContinuousCollisionDetection)
		{
			const int    numToisTillAllowedPenetration     = 4;		// The number of Tois till we reach the full penetration
			const hkReal maxInitialRelPenetration          = 0.5f;	// The max relative initial penetration distance for the first Toi


			sq.m_minSeparation		= -1.0f * maxInitialRelPenetration;
			sq.m_minExtraSeparation	= -1.0f * (1.0f - maxInitialRelPenetration) / (numToisTillAllowedPenetration-1);
			sq.m_toiSeparation		= -0.5f * maxInitialRelPenetration;
			sq.m_toiExtraSeparation	= -0.7f * (1.0f - maxInitialRelPenetration) / (numToisTillAllowedPenetration-1);
			sq.m_toiAccuracy		= -0.3f * sq.m_toiExtraSeparation;
			sq.m_maxContraintViolation  = -1.0f * sq.m_minExtraSeparation;

			sq.m_minSafeDeltaTime   = radiusToAllowedPenetrationRatio / input.m_maxLinearVelocity;
			sq.m_minAbsoluteSafeDeltaTime   = 0.005f / input.m_maxLinearVelocity; // Should work for bodies 1cm in size.
			sq.m_minToiDeltaTime	= -1.0f * sq.m_minExtraSeparation / input.m_maxLinearVelocity;

			sq.m_constraintPriority = input.m_toiHigherConstraintPriority;
			sq.m_useContinuousPhysics   = true;

			HK_ASSERT2(0xad2342da, sq.m_minToiDeltaTime > 0.0f && sq.m_minSafeDeltaTime > 0.0f, "Internal error: incorrect initialization of surface qualities.");
		}
	}

	{

		hkCollisionQualityInfo& sq = m_collisionQualityInfo[ COLLISION_QUALITY_TOI_FORCED ];

		sq.m_keepContact         = input.m_collisionTolerance;

		if (input.m_wantContinuousCollisionDetection)
		{
			const int    numToisTillAllowedPenetration      = 20;	// The number of Tois till we reach the full penetration
			const hkReal maxInitialRelPenetration           = 0.4f;	// The max relative initial penetration distance for the first Toi


			sq.m_minSeparation		= -1.0f * maxInitialRelPenetration;
			sq.m_minExtraSeparation	= -1.0f * (1.0f - maxInitialRelPenetration) / (numToisTillAllowedPenetration-1);
			sq.m_toiSeparation		= -0.5f * maxInitialRelPenetration;
			sq.m_toiExtraSeparation	= -0.7f * (1.0f - maxInitialRelPenetration) / (numToisTillAllowedPenetration-1);
			sq.m_toiAccuracy		= -0.3f * sq.m_toiExtraSeparation;
			sq.m_maxContraintViolation  = -1.0f * sq.m_minExtraSeparation;

			sq.m_minSafeDeltaTime   = radiusToAllowedPenetrationRatio / input.m_maxLinearVelocity;
			sq.m_minAbsoluteSafeDeltaTime   = 0.005f / input.m_maxLinearVelocity; // Should work for bodies 1cm in size.
			sq.m_minToiDeltaTime	= -1.0f * sq.m_minExtraSeparation / input.m_maxLinearVelocity;

			sq.m_constraintPriority = input.m_toiForcedConstraintPriority;
			sq.m_useContinuousPhysics   = true;

			HK_ASSERT2(0xad2342da, sq.m_minToiDeltaTime > 0.0f && sq.m_minSafeDeltaTime > 0.0f, "Internal error: incorrect initialization of surface qualities.");
		}
	}

	{
		for ( int i = 0; i < HK_COLLIDABLE_QUALITY_MAX; i++)
		{
			for ( int j = 0; j < HK_COLLIDABLE_QUALITY_MAX; j++)
			{
				m_collisionQualityTable[i][j] = COLLISION_QUALITY_PSI;
			}
		}
	}

#define T m_collisionQualityTable

#define L HK_COLLIDABLE_QUALITY_FIXED
#define K HK_COLLIDABLE_QUALITY_KEYFRAMED
#define R HK_COLLIDABLE_QUALITY_KEYFRAMED_REPORTING
#define D HK_COLLIDABLE_QUALITY_DEBRIS
#define M HK_COLLIDABLE_QUALITY_MOVING
#define C HK_COLLIDABLE_QUALITY_CRITICAL
#define B HK_COLLIDABLE_QUALITY_BULLET
#define U HK_COLLIDABLE_QUALITY_USER


#define QI hkUchar(COLLISION_QUALITY_INVALID)
#define QP hkUchar(COLLISION_QUALITY_PSI)
#define QT hkUchar(COLLISION_QUALITY_TOI)
#define QH hkUchar(COLLISION_QUALITY_TOI_HIGHER)
#define QF hkUchar(COLLISION_QUALITY_TOI_FORCED)


	T[L][L] = QI;	T[L][K] = QI;	T[L][R] = QT;	T[L][D] = QP;	T[L][M] = QH;	T[L][C] = QF;	T[L][B] = QH;
	T[K][L] = QI;	T[K][K] = QI;	T[K][R] = QT;	T[K][D] = QP;	T[K][M] = QT;	T[K][C] = QT;	T[K][B] = QT;
	T[R][L] = QT;	T[R][K] = QT;	T[R][R] = QT;	T[R][D] = QP;	T[R][M] = QT;	T[R][C] = QT;	T[R][B] = QT;
	T[D][L] = QP;	T[D][K] = QP;	T[D][R] = QP;	T[D][D] = QP;	T[D][M] = QP;	T[D][C] = QP;	T[D][B] = QT;
	T[M][L] = QH;	T[M][K] = QT;	T[M][R] = QT;	T[M][D] = QP;	T[M][M] = QP;	T[M][C] = QT;	T[M][B] = QT;
	T[C][L] = QF;	T[C][K] = QT;	T[C][R] = QT;	T[C][D] = QP;	T[C][M] = QT;	T[C][C] = QT;	T[C][B] = QT;
	T[B][L] = QH;	T[B][K] = QT;	T[B][R] = QT;	T[B][D] = QT;	T[B][M] = QT;	T[B][C] = QT;	T[B][B] = QT;

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
