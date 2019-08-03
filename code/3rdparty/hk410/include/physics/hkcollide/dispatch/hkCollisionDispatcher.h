/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */


#ifndef HK_COLLIDE2_COLLISION_DISPATCHER_H
#define HK_COLLIDE2_COLLISION_DISPATCHER_H

#include <hkcollide/agent/hkCdBody.h>
#include <hkcollide/shape/hkShape.h>
#include <hkinternal/collide/agent3/hkAgent3.h>
#include <hkcollide/agent/hkCollidableQualityType.h>
#include <hkcollide/agent/hkCollisionQualityInfo.h>
#include <hkcollide/agent/hkCollisionInput.h>

class hkCollidable;
class hkCollisionAgent;
class hkContactMgr;
struct hkCollisionInput;
class hkContactMgrFactory;
class hkCdPointCollector;
enum hkShapeType;

#define HK_MAX_RESPONSE_TYPE 8
#define HK_MAX_SHAPE_TYPE 32
#define HK_MAX_AGENT2_TYPES 64
#define HK_MAX_AGENT3_TYPES 16
#define HK_MAX_COLLISION_QUALITIES 8


class hkCdPointCollector;
class hkCdBodyPairCollector;
struct hkLinearCastCollisionInput;

	/// This class is used to find the correct function which performs collision detection between pairs of shapes.
	/// It's based on the following rules:
	///  - Each shapes has a type
	///  - For each pair of types, there must be a function registered to handle this pair of types
	///
	/// However implementing all combinations would be quite complex. Therefore we allow for:
	///  - shapes to actually subclass other more basic shapes (e.g. a box is a convex shape)
	///    and then reuse a common base function
	///  - functions, which work on a pair of shapes, where only one shape type is used (e.g. hkTransformShape, hkBvTreeShape)
	///    and the other ignored (HK_SHAPE_ALL).
	///    in this case we have to register a function with a pair of types ( HK_SHAPE_MY_TYPE, HK_SHAPE_ALL )
	/// 
	/// To implement this behavior, the user has to
	///  - first declare the shape inheritance hierarchy by calling registerAlternateShapeType for each inheritance link
	///  - register all functions which handle collisions between two shape types.
	/// 
	/// Note: You still cannot call registerAlternateShapeType after you registered your first agent
	/// <br>
	/// Also this class can dispatch hkContactMgrFactory s. This dispatch is quite simple as there is no inheritance.
	/// Also this class combines two hkCollidableQualityTypes into a hkCdInteractionQualtityType
	/// <br>
	/// In addition to that, this class also dispatches a pair of hkCollidableQualityType into a hkCollisionQualityInfo structure
class hkCollisionDispatcher : public hkReferencedObject
{
	public:

		HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_CDINFO);

			/// A function to create a collision agent
		typedef hkCollisionAgent* (HK_CALL *CreateFunc)(const hkCdBody& collA, const hkCdBody& collB, const hkCollisionInput& env, hkContactMgr* mgr);

			/// A function to check whether two objects are penetrating
		typedef void (HK_CALL *GetPenetrationsFunc)     (const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdBodyPairCollector& collector );

			/// A function to find the closest points
		typedef void (HK_CALL *GetClosestPointsFunc)   (const hkCdBody& bodyA, const hkCdBody& bodyB, const hkCollisionInput& input, hkCdPointCollector& output );

			/// A function to do linear casts
		typedef void (HK_CALL *LinearCastFunc)         (const hkCdBody& bodyA, const hkCdBody& bodyB, const hkLinearCastCollisionInput& input, hkCdPointCollector& castCollector, hkCdPointCollector* startCollector );

			/// For backward compatability
		typedef CreateFunc CollisionAgentCreationFunction;

		typedef hkChar CollisionQualityIndex;

			/// A helper struct, which is used as an input to registerCollisionAgent
		struct AgentFuncs
		{
				/// The agent creation functions
			CreateFunc           m_createFunc;

				/// A static penetration function
			GetPenetrationsFunc  m_getPenetrationsFunc;

				/// A static get closest distance functions
			GetClosestPointsFunc m_getClosestPointFunc;

				/// A static linear cast function
			LinearCastFunc       m_linearCastFunc;

				/// A flag telling the system, whether a symmetric agent is involved.
				/// If the collision detector sees the flag to be set to true, it tries
				/// to swap objectA with objectB
			hkBool				 m_isFlipped;

				/// A flag indicating whether the object can handle continuous collision
			hkBool				 m_isPredictive;

			AgentFuncs() : m_isFlipped(false), m_isPredictive(false) {}
		};

			/// A helper struct, which is used as an input to registerCollisionAgent
			/// for streamed agents. Read hkAgent3 for details
		struct Agent3Funcs
		{
			Agent3Funcs() : m_updateFilterFunc(HK_NULL), m_invalidateTimFunc(HK_NULL), 
				            m_warpTimeFunc(HK_NULL), m_sepNormalFunc(HK_NULL),
							m_isPredictive(false), m_ignoreSymmetricVersion(false)
			{
			}

			hkAgent3::CreateFunc      m_createFunc;
			hkAgent3::DestroyFunc     m_destroyFunc;
			hkAgent3::CleanupFunc     m_cleanupFunc;
			hkAgent3::RemovePointFunc m_removePointFunc;
			hkAgent3::CommitPotentialFunc m_commitPotentialFunc;
			hkAgent3::CreateZombieFunc  m_createZombieFunc;
			hkAgent3::UpdateFilterFunc  m_updateFilterFunc;
			hkAgent3::InvalidateTimFunc m_invalidateTimFunc;
			hkAgent3::WarpTimeFunc      m_warpTimeFunc;
			hkAgent3::SepNormalFunc     m_sepNormalFunc;
			hkAgent3::ProcessFunc       m_processFunc;
			hkBool                    m_isPredictive;
			hkBool						m_ignoreSymmetricVersion;
		};


		struct InitCollisionQualityInfo
		{
			hkReal m_gravityLength;
			hkReal m_collisionTolerance;
			hkReal m_minDeltaTime;
			hkReal m_maxLinearVelocity;
			hkUint16 m_defaultConstraintPriority;
			hkUint16 m_toiConstraintPriority;
			hkUint16 m_toiHigherConstraintPriority;
			hkUint16 m_toiForcedConstraintPriority;
			hkBool m_wantContinuousCollisionDetection;
			hkBool m_enableNegativeManifoldTims;
			hkBool m_enableNegativeToleranceToCreateNon4dContacts;
		};

		enum AgentType
		{
			HK_AGENT_TYPE_NULL = 0,
			HK_AGENT_TYPE_BRIDGE = 1
		};

		enum TableType
		{
			TABLE_TYPE_AGENT2,
			TABLE_TYPE_AGENT2_PRED,
			TABLE_TYPE_AGENT3,
			TABLE_TYPE_AGENT3_PRED
		};

		enum IsAgentPredictive
		{
			IS_NOT_PREDICTIVE,
			IS_PREDICTIVE
		};



	public:

			/// create the collision dispatcher
		hkCollisionDispatcher( CreateFunc defaultCreationFunction, hkContactMgrFactory* defaultContactMgrFactory );

			/// delete it
		~hkCollisionDispatcher();



			//
			//	Shape Types
			//


			/// register a shape type and a possible alternative shape type, which
			/// is used if the first type does not give a valid answer
			/// Note: you should register all alternate shapes BEFORE you can register the collision
			/// agent creation functions (otherwise it gets very CPU intensive).<br>
			/// <br>
			/// Background:<br>
			/// Since havok2.2, the inheritance of shapes is defined in the collision dispatcher,
			/// not in the shapes. That means before registering agents, all shape types and their
			/// alternate(=superClass) type needs to be registered.
			/// - Example: A triangle is a convex shape, so in order for the convex agent to 
			///   pick up a triangle the following lines must precede the registration of the convex
			///   agent: dispatcher->registerAlternateShapeTypes( HK_SHAPE_TRIANGLE, HK_SHAPE_CONVEX );
		void registerAlternateShapeType( hkShapeType primaryType, hkShapeType alternateType );


			/// returns true, if type has a path to alternateType or type == alternateType
			/// Note: This function is really fast
		inline hkBool hasAlternateType( hkShapeType type, hkShapeType alternateType );
			



			//
			//	Agents
			//


			/// register an agent for a given pair of shape types, use HK_SHAPE_ALL for one type if you want to register
			/// the other type with all agents.
			/// Later calls to this functions override existing values, that means later calls automatically have a higher priority.
			///  - Note: You should register all shape alternate Types before you call registerCollisionAgent()
			///  - Note: A call to this function is slow
			///  - Note: Agent using the streaming technology must be registered just after the non
			///          streaming version is registered
			/// 
			/// Build in checks:<br>
			/// Example: Say you register a fast sphere-sphere agent first and than you register a slow convex-convex agent.
			///			 As later registrations overrides earlier, the convex-convex agent will actually override the
			///			 sphere-sphere one (given that there is a rule that a sphere is a convex object). Bad :-( <br>
			///          Unfortunately this  wrong registration is usually difficult to find.<br>
			///
			///	Solution:
			///  - Currently the program checks, if a more general agent overrides an more specialized agent and asserts.
			///    However in some cases the check does not work, so you can disable it by calling setEnableChecks(false). 
			///  - Call debugPrintTable() to get a text file which shows you check the contents of the table by hand
		void registerCollisionAgent( AgentFuncs& f, hkShapeType typeA, hkShapeType typeB );


			/// disable checks if you want to force override existing entries when calling registerCollisionAgent.
			/// Don't forget to re-enabling the checks, once you have done your unchecked operations.
		void setEnableChecks( hkBool checkEnabled);

			/// Debug print of the current table entries
		void debugPrintTable();

			/// Disables debugging. This should be done at the end of registering all agents to save some memory.
			/// Note: Checking is only enabled in debug
		void disableDebugging();


			/// returns a new collision agent for a collidable pair
		HK_FORCE_INLINE hkCollisionAgent* getNewCollisionAgent(	const hkCdBody& a, const hkCdBody& b, const hkCollisionInput& env, hkContactMgr* mgr) const;

			/// get the agent creation function registered for a given pair. 
		HK_FORCE_INLINE CreateFunc getCollisionAgentCreationFunction( hkShapeType typeA, hkShapeType typeB, hkCollisionDispatcher::IsAgentPredictive predictive ) const;

			/// get the functions, which allows you to query two collision collidables for penetration cases
		HK_FORCE_INLINE GetPenetrationsFunc getGetPenetrationsFunc( hkShapeType typeA, hkShapeType typeB ) const;

			/// get the functions, which allows you to query two collision collidables for penetration cases
		HK_FORCE_INLINE GetClosestPointsFunc getGetClosestPointsFunc( hkShapeType typeA, hkShapeType typeB ) const;

			/// get the functions, which allows you to query two collision collidables for penetration cases
		HK_FORCE_INLINE LinearCastFunc getLinearCastFunc( hkShapeType typeA, hkShapeType typeB ) const;

		HK_FORCE_INLINE hkBool getIsFlipped( hkShapeType typeA, hkShapeType typeB ) const;


			/// Same as registerCollisionAgent, but for the hkAgent3 technology
		int registerAgent3( Agent3Funcs& f, hkShapeType typeA, hkShapeType typeB );

		HK_FORCE_INLINE hkAgent3::AgentType getAgent3Type( hkShapeType typeA, hkShapeType typeB, hkBool predicitve ) const;

		HK_FORCE_INLINE hkAgent3::CreateFunc           getAgent3CreateFunc  ( hkAgent3::AgentType type );
		HK_FORCE_INLINE hkAgent3::DestroyFunc          getAgent3DestroyFunc ( hkAgent3::AgentType type );
		HK_FORCE_INLINE hkAgent3::CleanupFunc          getAgent3CleanupFunc ( hkAgent3::AgentType type );
		HK_FORCE_INLINE hkAgent3::RemovePointFunc      getAgent3RemovePointFunc ( hkAgent3::AgentType type );
		HK_FORCE_INLINE hkAgent3::CommitPotentialFunc  getAgent3CommitPotentialFunc ( hkAgent3::AgentType type );
		HK_FORCE_INLINE hkAgent3::CreateZombieFunc     getAgent3CreateZombieFunc ( hkAgent3::AgentType type );
		HK_FORCE_INLINE hkAgent3::UpdateFilterFunc     getAgent3UpdateFilterFunc ( hkAgent3::AgentType type );
		HK_FORCE_INLINE hkAgent3::InvalidateTimFunc    getAgent3InvalidateTimFunc ( hkAgent3::AgentType type );
		HK_FORCE_INLINE hkAgent3::WarpTimeFunc	       getAgent3WarpTimeFunc ( hkAgent3::AgentType type );
		HK_FORCE_INLINE hkAgent3::ProcessFunc          getAgent3ProcessFunc ( hkAgent3::AgentType type );
		HK_FORCE_INLINE hkAgent3::SepNormalFunc        getAgent3SepNormalFunc( hkAgent3::AgentType type );
		HK_FORCE_INLINE hkAgent3::Symmetric            getAgent3Symmetric( hkAgent3::AgentType type );
			//
			//	Contact Mgrs
			//

			/// register a contact factory
		void registerContactMgrFactory( hkContactMgrFactory*, int responseA, int responseB );


			/// Get a contact manager factory - used to create a contact manager.
		HK_FORCE_INLINE hkContactMgrFactory* getContactMgrFactory(int responseA, int responseB) const;

			/// register a contact factory with all other types: Note overrides existing entries
		void registerContactMgrFactoryWithAll( hkContactMgrFactory*, int responseA );



			//
			//	hkCollisionQualityInfo
			//
		HK_FORCE_INLINE CollisionQualityIndex getCollisionQualityIndex( hkCollidableQualityType a, hkCollidableQualityType b);

		HK_FORCE_INLINE hkCollisionQualityInfo* getCollisionQualityInfo( CollisionQualityIndex index );
		
		void initCollisionQualityInfo( InitCollisionQualityInfo& input );
		
			// hkBaseObject interface implementation
		void calcStatistics( hkStatisticsCollector* collector ) const;

	public:

		struct ShapeInheritance
		{
			hkShapeType m_primaryType;
			hkShapeType m_alternateType;
		};

		struct AgentEntry
		{
			hkShapeType m_typeA;
			hkShapeType m_typeB;
			hkBool m_checkEnabled;
			AgentFuncs  m_createFunc;
		};

		struct Agent3Entry
		{
			hkShapeType m_typeA;
			hkShapeType m_typeB;
			hkBool m_checkEnabled;
			Agent3Funcs  m_funcs;
		};

			// A helper struct, which is used as an input to registerFullAgent
		struct Agent3FuncsIntern: public Agent3Funcs
		{
			hkAgent3::Symmetric       m_symmetric;
		};

		struct DebugEntry
		{
				/// Shape Type A
			char m_typeA;
				/// Shape Type B
			char m_typeB;
				
				/// The sum of all shape rules which have to be applied to get to this entry.
				/// E.g. we have two spheres (m_typeAB = HK_SHAPE_SPHERE)
				///  - if we register a hkSphereSphere Agent, no indirections are needed and the priority is 0
				///  - if we register a hkSphereConvex Agent, 1 indirections is needed and the priority is 1
				///  - if we register a hkConvexConvex Agent, 2 indirections are needed and the priority is 2
				///  - if we register a hkSphere-All Agent, 1 indirections is needed and the priority is 1
			char m_priority;
		};

		typedef DebugEntry	DebugTable[HK_MAX_SHAPE_TYPE][HK_MAX_SHAPE_TYPE];


	protected:
		void internalRegisterCollisionAgent( hkUchar agentTypesTable[HK_MAX_SHAPE_TYPE][HK_MAX_SHAPE_TYPE], int agentType, hkShapeType typeA, hkShapeType typeB, hkShapeType origA, hkShapeType origB, DebugTable *debugTable, int recursionDepth );

		void updateHasAlternateType( hkShapeType primaryType, hkShapeType alternateType, int depth );

		void resetCreationFunctions();


	public:

				/// Collision quality setting for a collision between a pair of objects
			// values must be corresponding to hkConstraintInstance::ConstraintPriority
		enum CollisionQualityLevel
		{
				/// invalid
			COLLISION_QUALITY_INVALID,

				/// use this to get discrete stepping behaviour or for fixed objects
			COLLISION_QUALITY_PSI,

				/// Application: Ensure non-penetration between pairs of non-fixed objects
				/// Use this if you want to prevent bullet through paper
			COLLISION_QUALITY_TOI,

				/// Same as above.
				/// Application: Ensure non-penetration between moving objects & fixed objects
				/// Tech. Info: This ensures constraints to be treated as 'higher-priority' in PSI solver
			COLLISION_QUALITY_TOI_HIGHER,

				/// Application: Ensure and monitor non-penetration between moving objects & fixed objects.
				///              You receive callbacks when penetration occurs.
				/// Tech. Info: This ensures constraints to be treated as 'higher-priority' in PSI & TOI solvers
				///             plus 'forced' during TOI handling
			COLLISION_QUALITY_TOI_FORCED,

				/// Only used internally for expanding contact manifold in hkContinuousSimulation::updateManifold().
				///	Will try to collect as many contact points as possible.
			COLLISION_QUALITY_TMP_EXPAND_MANIFOLD

		};


		CreateFunc	m_defaultCollisionAgent;

		hkContactMgrFactory*	m_contactMgrFactory[HK_MAX_RESPONSE_TYPE][HK_MAX_RESPONSE_TYPE];

			/// for each shapeType T,  m_hasAlternateType[T] has a bit set for each superclass
		hkUint32				m_hasAlternateType[ HK_MAX_SHAPE_TYPE ];	

			/// CollidableType x collidableType x {0, Pred} -> agentType dispatching for agent2 & agent3 technology.
			///  - m_agent2Types hold the proper agent's index for a given collision pair for non predictive collision detection
		    ///  - m_agent2TypesPred .................................................... for predictive collision detection
		int						m_numAgent2Types;
		hkUchar				    m_agent2Types    [HK_MAX_SHAPE_TYPE][HK_MAX_SHAPE_TYPE];
		hkUchar				    m_agent2TypesPred[HK_MAX_SHAPE_TYPE][HK_MAX_SHAPE_TYPE];
		AgentFuncs				m_agent2Func     [ HK_MAX_AGENT2_TYPES ];

		int						m_numAgent3Types;
		hkUchar				    m_agent3Types    [HK_MAX_SHAPE_TYPE][HK_MAX_SHAPE_TYPE];
		hkUchar				    m_agent3TypesPred[HK_MAX_SHAPE_TYPE][HK_MAX_SHAPE_TYPE];
		Agent3FuncsIntern       m_agent3Func     [ HK_MAX_AGENT3_TYPES ];

			/// CollidableQuality x collidableQuality -> collisionQuality dispatching
		hkChar					m_collisionQualityTable[ HK_COLLIDABLE_QUALITY_MAX][HK_COLLIDABLE_QUALITY_MAX];
		hkCollisionQualityInfo  m_collisionQualityInfo[ HK_MAX_COLLISION_QUALITIES ];


			/// This is set to true if there is at least one call to registerCollisionAgent() made
		hkBool m_collisionAgentRegistered;
		hkBool m_agent3Registered;

			/// Specify agent & sector size for hkAgentNnTracks (in the NN machine).
			/// Those should be identical for all hkAgentNnTracks in a given hkWorld
		int   m_agent3SectorSize;
		int   m_agent3AgentSize;

			/// If set, produces an Assert whenever a more-general agent overrides a more-specific agent in dispatch tables.
		hkBool m_checkEnabled;

			/// All alternate shape rules (a simple collection of all registerAlternateShapeType() calls )
		hkArray<ShapeInheritance> m_shapeInheritance;

			/// Additional information to the main tables. This allows for debugging 
			/// a wrong order of registering agents, see DebugEntry
		DebugTable	*m_debugAgent2Table;
		DebugTable	*m_debugAgent2TablePred;
		DebugTable	*m_debugAgent3Table;
		DebugTable	*m_debugAgent3TablePred;

	public:
		//
		// The following two memebers are only used when you call hkWorld::getCinfo(...)
		// They're set upon dispatcher initialization.
		//
		hkReal m_expectedMaxLinearVelocity;
		hkReal m_expectedMinPsiDeltaTime;
};


#include <hkcollide/dispatch/hkCollisionDispatcher.inl>

#endif // HK_COLLIDE2_COLLISION_DISPATCHER_H

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
