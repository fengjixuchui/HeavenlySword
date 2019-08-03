/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_UTILITIES2_RIGIDBODY_CENTREOFMASS_VIEWER_H
#define HK_UTILITIES2_RIGIDBODY_CENTREOFMASS_VIEWER_H

#include <hkutilities/visualdebugger/viewer/hkWorldViewerBase.h>
#include <hkdynamics/world/listener/hkWorldPostSimulationListener.h>
#include <hkdynamics/entity/hkEntityListener.h>

class hkDebugDisplayHandler;
class hkWorld;
class hkRigidBody;

	/// Shows the centre of mass of all rigid bodies added to the world.
class hkRigidBodyCentreOfMassViewer :	public hkWorldViewerBase,
								protected hkEntityListener, protected hkWorldPostSimulationListener
{
	public:

			/// Creates a hkRigidBodyCentreOfMassViewer.
		static hkProcess* HK_CALL create(const hkArray<hkProcessContext*>& contexts);

			/// Registers the hkRigidBodyCentreOfMassViewer with the hkViewerFactory.
		static void HK_CALL registerViewer();

			/// Gets the tag associated with this viewer type
		virtual int getProcessTag() { return m_tag; }

		virtual void init();

		static inline const char* HK_CALL getName() { return "CentreOfMass"; }

	protected:

		hkRigidBodyCentreOfMassViewer(const hkArray<hkProcessContext*>& contexts);
		virtual ~hkRigidBodyCentreOfMassViewer();

		void addWorld(hkWorld* world);
		void removeWorld(hkWorld* world);

		virtual void entityAddedCallback( hkEntity* entity );
		virtual void entityRemovedCallback( hkEntity* entity );

		virtual void postSimulationCallback( hkWorld* world );

		virtual void worldAddedCallback( hkWorld* world );
		virtual void worldRemovedCallback( hkWorld* world );

	protected:
		static int m_tag;

		hkArray<hkRigidBody*> m_entitiesCreated;
};

#endif	// HK_UTILITIES2_RIGIDBODY_CENTREOFMASS_VIEWER_H


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
