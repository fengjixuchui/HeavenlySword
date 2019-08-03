/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_AGENT_MACHINE_UTIL_H
#define HK_COLLIDE2_AGENT_MACHINE_UTIL_H

struct hkAgentNnTrack;
struct hkAgent1nTrack;
class hkStatisticsCollector;

	/// Some helper functions for agent streams
class hkAgentMachineUtil
{
	public:
			/// Runtime memory monitoring
		static void HK_CALL calcNnStatistics( const hkAgentNnTrack& track, hkStatisticsCollector* collector );

			/// Some but only check the contact mgrs
		static void HK_CALL calcNnStatisticsContactMgrsOnly( const hkAgentNnTrack& track, hkStatisticsCollector* collector );

			/// Runtime memory monitoring
		static void HK_CALL calc1nStatistics( const hkAgent1nTrack& agentTrack, hkStatisticsCollector* collector );
};

#endif // HK_COLLIDE2_AGENT_MACHINE_UTIL_H

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
