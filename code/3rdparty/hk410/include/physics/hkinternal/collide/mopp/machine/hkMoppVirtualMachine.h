/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_MOPP_VIRTUAL_MACHINE_H
#define HK_COLLIDE2_MOPP_VIRTUAL_MACHINE_H

// Virtual Machine command definitions

//#define HK_MOPP_DEBUGGER_ENABLED

#ifdef HK_MOPP_DEBUGGER_ENABLED
#	include <hkinternal/hkInternal.h>
#	include <hkinternal/collide/mopp/builder/hkbuilder.h>
#	include <hkinternal/collide/mopp/utility/hkMoppDebugger.h>
#	define HK_QVM_DBG(x) x
#	define HK_QVM_DBG2(var,x) int var = x
#else 
#	define HK_QVM_DBG(x)
#	define HK_QVM_DBG2(var,x) 
#endif




//this is the structure that all virtual machines will return on being queried
//it represents a primitive ID and an array of primitive properties
class hkMoppPrimitiveInfo
{
	public:
		hkUint32 ID;
		//unsigned int properties[hkMoppCode::MAX_PRIMITIVE_PROPERTIES];
};

//class HK_ALIGNED_VARIABLE(hkMoppVirtualMachine,16) 
class hkMoppVirtualMachine
{
	public:
		typedef int hkMoppFixedPoint;  

	public:
		// standard constructor
		inline hkMoppVirtualMachine();										
		// standard destructor
		inline ~hkMoppVirtualMachine();										

	protected:
		inline void addHit(unsigned int id, const unsigned int properties[hkMoppCode::MAX_PRIMITIVE_PROPERTIES]);

		inline void initQuery( hkArray<hkMoppPrimitiveInfo>* m_primitives_out );

			/// returns an integer which is smaller than x
		static inline int HK_CALL toIntMin(hkReal x);

			/// returns an integer which is larger than x
		static inline int HK_CALL toIntMax(hkReal x);

		static inline int HK_CALL read24( const unsigned char* PC );


	public:
		hkArray<hkMoppPrimitiveInfo>* m_primitives_out;
		int		padding[3];

};

#include <hkinternal/collide/mopp/machine/hkMoppVirtualMachine.inl>

#endif // HK_COLLIDE2_MOPP_VIRTUAL_MACHINE_H

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
