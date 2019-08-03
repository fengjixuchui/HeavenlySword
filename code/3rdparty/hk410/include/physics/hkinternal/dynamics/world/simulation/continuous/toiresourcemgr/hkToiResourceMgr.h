/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_DYNAMICS2_TOI_LISTENER_H
#define HK_DYNAMICS2_TOI_LISTENER_H

#include <hkinternal/dynamics/world/simulation/continuous/hkContinuousSimulation.h>


class hkWorld;
class hkStepInfo;

	// <todo> to be properly used in subSolve
	/// User callback's return value. Holds the requested way-of-action.
enum hkToiResourceMgrResponse
{
		// Continue processing the TOI.
	HK_TOI_RESOURCE_MGR_RESPONSE_CONTINUE,
		// Continue processing the TOI, do not attempt further expansion of the group of bodies used for localized solving.
	HK_TOI_RESOURCE_MGR_RESPONSE_DO_NOT_EXPAND_AND_CONTINUE,
		// Abort attempts of proper solution of the TOI. Backstep the entire hkSimulationIsland.
	HK_TOI_RESOURCE_MGR_RESPONSE_BACKSTEP,
};

	/// A class which is used to specify how much resources should be used the one single toi
	/// This class is controlled by the hkToiResourceMgr
struct hkToiResources 
{
			/// Specifies the minimum priority of constraints that will be processed in Localized Solving.
			/// This refers both to user's constrains and system-generated contact constraints.
			/// Default value: PRIORITY_TOI;
		hkConstraintInstance::ConstraintPriority m_minPriorityToProcess;

			/// The system calls resourcesDepleted() callback when this limit is reached.
		int m_maxNumEntities;
			/// The system calls resourcesDepleted() callback when this limit is reached.
		int m_maxNumActiveEntities;
			/// The system calls resourcesDepleted() callback when this limit is reached.
		int m_maxNumConstraints;

			/// Number of solver iterations to run in TOI events.
		int m_numToiSolverIterations;
			/// Number of additional solver iterations executed for FORCED constraints only.
		int m_numForcedToiFinalSolverIterations;

			/// Pointer to memory block to be used by the constraints solver. 
		char* m_scratchpad;
			/// Size of the memory block to be used by the constraints solver. 
		int m_scratchpadSize;


		hkToiResources() :	m_minPriorityToProcess(hkConstraintInstance::PRIORITY_TOI),
							m_maxNumEntities      (1000),
							m_maxNumActiveEntities(1000),
							m_maxNumConstraints   (1000),
							m_numToiSolverIterations(4),
							m_numForcedToiFinalSolverIterations(4),
							m_scratchpad          (HK_NULL),
							m_scratchpadSize      (0)
		{
		}
};


	/// An interface which is responsible for allocating resources for TOI handling
class hkToiResourceMgr: public hkReferencedObject
{
	public:
		struct ConstraintViolationInfo
		{
			inline hkEntity* getEntityA() { return m_constraint->getEntityA(); }
			inline hkEntity* getEntityB() { return m_constraint->getEntityB(); }

			hkConstraintInstance* m_constraint;

				// 'Most failing' contact point. Pointer set to null for non-contact constraints.
			const hkContactPoint* m_contactPoint;
			const class hkContactPointProperties* m_contactPointProperties; 
		};

	public:
		virtual ~hkToiResourceMgr() {}

			/// Sets localized solving parameters. Estimates should take into consideration:
			///  - total no of TOIs generated during this PSI frame
			/// Parameters: event -- event being handled,
			///             otherEvents -- other pending events.
		virtual hkResult beginToiAndSetupResources(const hkToiEvent& event, const hkArray<hkToiEvent>& otherEvents, hkToiResources& resourcesOut ) = 0;


			/// This function is called when a forced-priority constraint cannot be solved completely.
		virtual hkToiResourceMgrResponse cannotSolve(hkArray<ConstraintViolationInfo>& violatedConstraints)				 { return HK_TOI_RESOURCE_MGR_RESPONSE_CONTINUE; }

			/// Called when the Toi-handling routine runs out of memory resources or exceeds 
			/// specified limits (of the number of entities and constraints involved).
		virtual hkToiResourceMgrResponse resourcesDepleted()         { return HK_TOI_RESOURCE_MGR_RESPONSE_DO_NOT_EXPAND_AND_CONTINUE; }

			/// This is called at the end of the Toi-Event handling to free the resources.
		virtual void endToiAndFreeResources(const hkToiEvent& event, const hkArray<hkToiEvent>& otherEvents, const hkToiResources& resources ) = 0;


};


#endif // HK_DYNAMICS2_TOI_LISTENER_H

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
