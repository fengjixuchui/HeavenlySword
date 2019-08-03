/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */


#ifndef HK_COLLIDE2_MOPP_UTILITY_H
#define HK_COLLIDE2_MOPP_UTILITY_H


#include <hkcollide/shape/mopp/hkMoppFitToleranceRequirements.h>
#include <hkinternal/collide/mopp/code/hkMoppCode.h>


class hkShapeContainer;
class hkMoppMediator;


/// This class provides useful functionality for managing, maintaining, and creating MOPPs.
class hkMoppUtility
{
	public:
		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_CDINFO,hkMoppUtility);

			/// Builds the MOPP for a given set of shapes with the specified fit tolerance requirements. 
			/// The returned hkMoppCode is a referenced object, so you just need to call removeReference() when you're finished with it -
			/// the system will look after deleting the object.
			/// Note: This method can take significant time to complete. Mopp code is a platform independent byte code and should
			/// be precomputed offline and loaded at runtime.
		static hkMoppCode* HK_CALL buildCode(const hkShapeContainer* shapeContainer, const hkMoppFitToleranceRequirements& req);


	protected:

			// called by buildCode()
		static hkMoppCode* HK_CALL buildCodeInternal(hkMoppMediator& mediator, const hkMoppFitToleranceRequirements& moppFtr);

};



#endif // HK_COLLIDE2_MOPP_UTILITY_H

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
