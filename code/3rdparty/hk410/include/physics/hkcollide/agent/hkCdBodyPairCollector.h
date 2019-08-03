/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_CD_BODY_PAIR_COLLECTOR
#define HK_COLLIDE2_CD_BODY_PAIR_COLLECTOR


	/// This class is used as an interface to the collision detector ( e.g. getPenetrations() )
	/// to collect pairs of hkCdBody
class hkCdBodyPairCollector
{
	public:

		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_AGENT, hkCdBodyPairCollector);

			/// Default constructor calls reset(), initializing the early out variable to false.
		inline hkCdBodyPairCollector();

		inline virtual ~hkCdBodyPairCollector();

			/// This is the function called for every hit of the collision detector.
			/// Note: for optimization purposes this should set the m_earlyOut:
			/// - true if you want to get no more hits
			/// - false if you want to get more hits (which is the default)
		virtual void addCdBodyPair( const hkCdBody& bodyA, const hkCdBody& bodyB ) = 0;

			/// resets m_earlyOut to false. You must call this function if you want to reuse an object of this class.
		inline void reset();	

			/// Gets the early out, if true, no more CdBodyPairs are reported
		inline hkBool getEarlyOut( ) const;

	protected:

		hkBool m_earlyOut;

};

#include <hkcollide/agent/hkCdBodyPairCollector.inl>


#endif // HK_COLLIDE2_CD_BODY_PAIR_COLLECTOR

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
