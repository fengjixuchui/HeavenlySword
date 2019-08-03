/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_UTILITIES2_SIMULATION_ISLAND_VIEWER_H
#define HK_UTILITIES2_SIMULATION_ISLAND_VIEWER_H

#include <hkbase/hkBase.h>
#include <hkbase/htl/hkObjectArray.h>
#include <hkutilities/visualdebugger/viewer/hkWorldViewerBase.h>
#include <hkdynamics/world/listener/hkWorldPostSimulationListener.h>
#include <hkvisualize/shape/hkDisplayAABB.h>


class hkDebugDisplayHandler;
class hkWorld;

	/// Displays the bounding boxes of all simulation islands in the world.  The the boxes will
	/// be blue for active simulation islands and green for inactive simulation islands.
class hkSimulationIslandViewer : public hkWorldViewerBase, protected hkWorldPostSimulationListener
{
	public:

			/// Creates a hkSimulationIslandViewer.
		static hkProcess* HK_CALL create( const hkArray<hkProcessContext*>& contexts );

			/// Registers the hkSimulationIslandViewer with the hkViewerFactory.
		static void HK_CALL registerViewer();

			/// Gets the tag associated with this viewer type
		virtual int getProcessTag() { return m_tag; }

		static inline const char* HK_CALL getName() { return "Islands"; }

			// Control the state of the simulation island viewer
		hkBool m_showActiveIslands;
		hkBool m_showInactiveIslands;

	protected:

		hkSimulationIslandViewer( const hkArray<hkProcessContext*>& contexts );
		virtual ~hkSimulationIslandViewer();

		virtual void postSimulationCallback(hkWorld* world );
		virtual void worldAddedCallback( hkWorld* world );
		virtual void worldRemovedCallback( hkWorld* world );

		hkObjectArray<hkDisplayAABB> m_inactiveIslandDisplayGeometries;
		hkObjectArray<hkDisplayAABB> m_activeIslandDisplayGeometries;

		static int m_tag;
};

#endif	// HK_UTILITIES2_SIMULATION_ISLAND_VIEWER_H

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
