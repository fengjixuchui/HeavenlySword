/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_CONSTRAINTSOLVER2_CONTACT_POINT_PROPERTIES_H
#define HK_CONSTRAINTSOLVER2_CONTACT_POINT_PROPERTIES_H

#include <hkconstraintsolver/solve/hkSolverResults.h>
#include <hkmath/basetypes/hkContactPointMaterial.h>


	/// This class is used to get and set the friction for a contact point. You can also use it to attach your own user data
	/// to a contact point.  This can be used for example to set a friction map value in when a contact point is added
	/// so that the same data can be used when the contact point is being updated (from a processContactCallback() for example)
class hkContactPointProperties : public hkSolverResults, public hkContactPointMaterial
{
	public:
			/// returns true if this contact point was really used by the solver
			/// and an impulse was applied
		inline hkBool wasUsed()
		{
			return !isPotential() && ( m_impulseApplied > 0.f );
		}

	public:
			// internal data, not for client use
		hkReal m_internalDataA;
};

#endif // HK_CONSTRAINTSOLVER2_CONTACT_POINT_PROPERTIES_H

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
