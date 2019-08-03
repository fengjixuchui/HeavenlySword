/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HKVEHICLE_ENGINE_DEFAULT_hkVehicleDefaultEngine_XML_H
#define HKVEHICLE_ENGINE_DEFAULT_hkVehicleDefaultEngine_XML_H

#include <hkvehicle/engine/hkVehicleEngine.h>
	

/// The hkVehicleDefaultEngine class implements an hkVehicleEngine with torque curves,
/// clutch slip and engine resistance. The torque is calculated as a function of the
/// user input (acceleration pedal), the current RPM and the engine resistance. The
/// RPM is calculated from the transmission, the clutch slip parameters and the user
/// input.
class hkVehicleDefaultEngine : public hkVehicleEngine
{
	public:

		HK_DECLARE_REFLECTION();
	
			/// Default constructor
		hkVehicleDefaultEngine();
		
		//
		// Methods
		//
		
			/// Sets the current values of the torque and rpm.
		virtual void calcEngineInfo(const hkReal deltaTime, const hkVehicleInstance* vehicle, const hkVehicleDriverInput::FilteredDriverInputOutput& FilteredDriverInputOutput, const hkVehicleTransmission::TransmissionOutput& TransmissionOutput, EngineOutput& engineOutput);
		
		//
		// Members
		//
	public:
		
			/// The min RPM of the engine.
		hkReal m_minRPM;
		
			/// The optimum RPM, where the gross torque of the engine is maximal. 	
		hkReal m_optRPM;
		
			/// The max RPM the engine can reach. 	
		hkReal m_maxRPM;
		
			/// The maximum gross torque the engine can supply at the optimum RPM. 	
		hkReal m_maxTorque;
		
			/// Defines the gross torque at the min rpm as a factor to the torque at optimal
			/// 	RPM. 	
		hkReal m_torqueFactorAtMinRPM;
		
			/// Defines the gross torque at the max rpm as a factor to the torque at optimal
			/// 	RPM. 	
		hkReal m_torqueFactorAtMaxRPM;
		
			/// Defines the engine resistance torque at the min rpm as a factor to the torque at
			/// 	optimal RPM. 	
		hkReal m_resistanceFactorAtMinRPM;
		
			/// Defines the engine resistance torque at the opt rpm as a factor to the torque at
			/// 	optimal RPM. 	
		hkReal m_resistanceFactorAtOptRPM;
		
			/// Defines the engine resistance torque at the max rpm as a factor to the torque at
			/// 	optimal RPM. 	
		hkReal m_resistanceFactorAtMaxRPM;
		
			/// An extra RPM for the motor in case the clutch is slipping at very low speeds. 	
		hkReal m_clutchSlipRPM;

	public:
		hkVehicleDefaultEngine(hkFinishLoadedObjectFlag f) { }

};

#endif // HKVEHICLE_ENGINE_DEFAULT_hkVehicleDefaultEngine_XML_H

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
