/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HKVEHICLE_ENGINE_hkVehicleENGINE_XML_H
#define HKVEHICLE_ENGINE_hkVehicleENGINE_XML_H


#include <hkmath/hkMath.h>
#include <hkvehicle/driverinput/hkVehicleDriverInput.h>
#include <hkvehicle/transmission/hkVehicleTransmission.h>

class hkVehicleInstance;


/// The hkVehicleEngine is the component responsible for calculating values
/// related to the engine of the vehicle, in particular the engine torque and RPM. A
/// typical hkVehicleEngine implementation would collaborate with the vehicle's
/// hkVehicleDriverInput (for the accelerator pedal input) and its
/// hkVehicleTransmission (for RPM and torque transmission).
class hkVehicleEngine : public hkReferencedObject
{
	public:

		HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_VEHICLE);
		HK_DECLARE_REFLECTION();

		/// Container for data output by the engine calculations.
		/// Note that each of these members can be accessed through 
		/// the hkVehicleInstance.
		struct EngineOutput
		{
			/// The torque currently supplied by the engine.
			hkReal m_torque;

			/// The RPM the engine is currently running at.
			hkReal m_rpm;
		};

		//
		// Methods
		//
		
			/// Sets the current values of the torque and rpm.
		virtual void calcEngineInfo(const hkReal deltaTime, const hkVehicleInstance* vehicle, const hkVehicleDriverInput::FilteredDriverInputOutput& FilteredDriverInputOutput, const hkVehicleTransmission::TransmissionOutput& TransmissionOutput, EngineOutput& engineOutput ) = 0;
};

#endif // HKVEHICLE_ENGINE_hkVehicleENGINE_XML_H

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
