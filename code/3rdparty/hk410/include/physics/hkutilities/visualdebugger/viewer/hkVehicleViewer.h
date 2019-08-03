/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_UTILITIES2_VEHICLE_VIEWER_H
#define HK_UTILITIES2_VEHICLE_VIEWER_H

#include <hkutilities/visualdebugger/viewer/hkWorldViewerBase.h>
#include <hkdynamics/world/listener/hkWorldPostSimulationListener.h>

/// Draws the internal components of any vehicles in the simulation.
/// This viewer allows a user of the Havok Visual Debugger (VDB) tool to see
/// the internal components of any objects of type hkVehicleInstance (or any
/// objects of a type derived from hkVehicleInstance). The viewer draws the rays
/// cast by the vehicle for each wheel. The suspension hard point is draw, as are
/// the contact points where the wheel rays hit the ground. The center of each
/// wheel is drawn, as are the axles that connect the wheels together. Each wheel
/// is also drawn, to allow the user to see the size and orientation of the wheels
/// of the vehicle.
class hkVehicleViewer :
	public hkWorldViewerBase,
	public hkActionListener
{
	public:

		HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_VDB);

		/// Creates a hkContactPointViewer.
		static hkProcess* HK_CALL create(const hkArray<hkProcessContext*>& contexts);

		/// Registers the hkContactPointViewer with the hkViewerFactory.
		static void HK_CALL registerViewer();

		/// Gets the tag associated with this viewer type
		virtual int getProcessTag() { return m_tag; }

		virtual void init();

		/// A function to return the name of the viewer that will be displayed
		/// in the visual debugger.
		static inline const char* HK_CALL getName() { return "VehicleViewer"; }
		
	protected:

		hkVehicleViewer(const hkArray<hkProcessContext*>& contexts);
		virtual ~hkVehicleViewer();

		/* Override these methods from hkWorldViewerBase */
		virtual void worldAddedCallback( hkWorld* world );
		virtual void worldRemovedCallback( hkWorld* world );
		virtual void step(hkReal frameTimeInMs);

		/* Override these methods from hkActionListener */
		virtual void actionAddedCallback( hkAction* action );
		virtual void actionRemovedCallback( hkAction* action );

	protected:
		
		/// This is a unique identifier to allow the VDB identify which viewer
		/// has called the drawing functions.
		static int m_tag;

		/// The number of individual line segments that will be used to draw the
		/// wheels.
		static const int s_numWheelSegments;

		/// This array stores a pointer to each vehicle in the scene.
		hkArray<class hkVehicleInstance*> m_vehicles;
};

#endif	// HK_UTILITIES2_VEHICLE_VIEWER_H

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
