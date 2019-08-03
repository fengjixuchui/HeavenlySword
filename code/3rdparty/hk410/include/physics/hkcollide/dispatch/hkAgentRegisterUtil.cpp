/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkcollide/hkCollide.h>
#include <hkcollide/dispatch/hkAgentRegisterUtil.h>

#include <hkcollide/agent/gjk/hkGskConvexConvexAgent.h>
#include <hkcollide/agent/gjk/hkGskfAgent.h>
#include <hkcollide/agent/gjk/hkPredGskfAgent.h>

#include <hkcollide/agent/boxbox/hkBoxBoxAgent.h>
#include <hkcollide/agent/bv/hkBvAgent.h>
#include <hkcollide/agent/bvtree/hkBvTreeAgent.h>
#include <hkcollide/agent/bvtreestream/hkMoppBvTreeStreamAgent.h>

#include <hkcollide/agent/bvtree/hkMoppAgent.h>
#include <hkcollide/agent/phantom/hkPhantomAgent.h>
#include <hkcollide/agent/heightfield/hkHeightFieldAgent.h>
//#include <hkcollide/agent/tripatch/hkTriPatchAgent.h>

#include <hkcollide/agent/spheresphere/hkSphereSphereAgent.h>
#include <hkcollide/agent/spherecapsule/hkSphereCapsuleAgent.h>
#include <hkcollide/agent/spheretriangle/hkSphereTriangleAgent.h>

#include <hkcollide/agent/capsulecapsule/hkCapsuleCapsuleAgent.h>
#include <hkcollide/agent/capsuletriangle/hkCapsuleTriangleAgent.h>
#include <hkcollide/agent/spherebox/hkSphereBoxAgent.h>
#include <hkcollide/agent/multispheretriangle/hkMultiSphereTriangleAgent.h>
#include <hkcollide/agent/multirayconvex/hkMultiRayConvexAgent.h>

#include <hkcollide/agent/transform/hkTransformAgent.h>
#include <hkcollide/agent/list/hkListAgent.h>
#include <hkcollide/agent/convexlist/hkConvexListAgent.h>
#include <hkcollide/agent/shapecollection/hkShapeCollectionAgent.h>
#include <hkcollide/agent/multisphere/hkMultiSphereAgent.h>

#include <hkcollide/agent/bvtreestream/hkBvTreeStreamAgent.h>
#include <hkcollide/dispatch/agent3bridge/hkAgent3Bridge.h>

#include <hkinternal/collide/agent3/predgskagent3/hkPredGskAgent3.h>
#include <hkinternal/collide/agent3/predgskcylinderagent3/hkPredGskCylinderAgent3.h>
#include <hkinternal/collide/agent3/capsuletriangle/hkCapsuleTriangleAgent3.h>
#include <hkinternal/collide/agent3/boxbox/hkBoxBoxAgent3.h>



// Register agents
void HK_CALL hkAgentRegisterUtil::registerAllAgents(hkCollisionDispatcher* dis)
{

	hkRegisterAlternateShapeTypes(dis);

	//
	//	Warning: order of registring agents is important, later entries override earlier entries
	//


	//
	//	Unary agents handling secondary type
	//
	{
		hkBvAgent::registerAgent(dis);
		hkMultiSphereAgent::registerAgent( dis );

		// hkBvTreeAgent gets special treatment, as it overrides several
		// hkBvAgent entries, which will cause an assert 
		//
		{
			dis->setEnableChecks( false );

			// Register bvTree against everything, and bvTree vs bvTree special case to create a bvTree agent for the larger tree
			hkBvTreeAgent::registerAgent(dis);

			// Register mopp against everything (already done), and mopp vs mopp special case (using size of mopp code) to create a bvTree agent for the larger tree
			// Also replaces the linear cast static function for mopp
			hkMoppAgent::registerAgent(dis);

			// Register stream bvtree for bvTree against convex objects			
			hkBvTreeStreamAgent::registerAgent(dis);

			// Replaces the linear cast static function for mopp
			hkMoppBvTreeStreamAgent::registerAgent(dis);
			dis->setEnableChecks( true );
		}

		//
		// hkShapeCollectionAgent gets special treatment, as it overrides several
		// hkBvAgent entries, which will cause an assert
		//
		{
			dis->setEnableChecks( false );
			// This will override the shape collection vs bvTree (used to be set to bvTree), to be hkShapeCollectionAgent
			hkShapeCollectionAgent::registerAgent(dis);
			dis->setEnableChecks( true );
		}
		
		hkTransformAgent::registerAgent(dis);	
		hkPhantomAgent::registerAgent(dis);
	}

	{
		// Register list agent against everything else (overrides shape collection agent)
		hkListAgent::registerAgent( dis );

		// Register the convex list for hkConvexList shapes against convex shapes
		// This dispatches to a special dispatch function in hkConvexListAgent for hkConvexShape vs hkConvexListShape
		// The convex list shape can be treated as a list, a convex list, or a convex object selected on a per
		// collision basis - see the dispatch function for details.
		hkConvexListAgent::registerAgent( dis );

		// This dispatches to a special dispatch function in bvTreeStream for hkBvTreeShape vs hkConvexListShape
		// The convex list shape can be treated as a list, a convex list, or a convex object selected on a per
		// collision basis - see the dispatch function for details.
		hkBvTreeStreamAgent::registerConvexListAgent(dis);

		hkHeightFieldAgent::registerAgent( dis );
	}



	//
	//	Default Convex - convex agents
	//
	{
		hkPredGskfAgent::registerAgent(dis);

		hkPredGskAgent3::registerAgent3( dis );
	}

	//
	//	Special agents
	//
	{
	    hkPredGskCylinderAgent3::registerAgent3( dis );

		// Warning: The box-box agent fail for small object (say 3cm in size).
		hkBoxBoxAgent::registerAgent(dis);
		hkBoxBoxAgent3::registerAgent3(dis);
		hkSphereSphereAgent::registerAgent(dis);
		hkSphereCapsuleAgent::registerAgent(dis);

			// register sphere triangle
		hkSphereTriangleAgent::registerAgent(dis);

			// As the hkSphereTriangleAgent does not weld, we have to use the hkPredGskAgent3 agent
		hkPredGskAgent3::registerAgent3( dis, HK_SHAPE_TRIANGLE, HK_SHAPE_SPHERE );
		hkPredGskAgent3::registerAgent3( dis, HK_SHAPE_SPHERE, HK_SHAPE_TRIANGLE );

		hkSphereBoxAgent::registerAgent(dis);

		hkCapsuleCapsuleAgent::registerAgent(dis);

		hkCapsuleTriangleAgent::registerAgent(dis);
		hkCapsuleTriangleAgent3::registerAgent3( dis );

		hkMultiSphereTriangleAgent::registerAgent(dis);
		hkMultiRayConvexAgent::registerAgent(dis);
		hkBvTreeStreamAgent::registerMultiRayAgent(dis);
	}


	//dis->debugPrintTable();
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
