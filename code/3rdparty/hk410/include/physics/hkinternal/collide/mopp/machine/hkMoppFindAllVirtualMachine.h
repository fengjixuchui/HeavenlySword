/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
//
// Havok Memory Optimised Partial Polytope Debugger
// This class helps debugging the mopp assembler and virtual machine
//

#ifndef HK_COLLIDE2_MOPP_FIND_ALL_H
#define HK_COLLIDE2_MOPP_FIND_ALL_H

#include <hkinternal/collide/mopp/machine/hkMoppVirtualMachine.h>


// the idea of the debugger is that we are searching our triangle in the entire original tree and
// remembering all paths to this node.

// when it comes to the final virtual machine, we easily can check, which paths are not taken,
// and which extra paths are processed which shouldn't be processed
class hkMoppFindAllVirtualMachine : public hkMoppVirtualMachine
{
	public:
			// standard constructor
		inline hkMoppFindAllVirtualMachine(){}
		// standard destructor
		inline ~hkMoppFindAllVirtualMachine(){}

		HK_FORCE_INLINE void queryAll(const hkMoppCode* code, hkArray<hkMoppPrimitiveInfo>* primitives_out);

	public: 
		struct hkMoppFindAllVirtualMachineQuery
		{
			unsigned int m_primitiveOffset;  
			unsigned int m_properties[hkMoppCode::MAX_PRIMITIVE_PROPERTIES];
		};

		void queryOnTree	( const hkMoppFindAllVirtualMachineQuery* query, const unsigned char* commands);
		void queryOnTreeLeft	( const hkMoppFindAllVirtualMachineQuery* query, const unsigned char* commands);
		void queryOnTreeRight	( const hkMoppFindAllVirtualMachineQuery* query, const unsigned char* commands);
};

void hkMoppFindAllVirtualMachine::queryAll(const hkMoppCode* code, hkArray<hkMoppPrimitiveInfo>* primitives_out)
{
	m_primitives_out = primitives_out;
	hkMoppFindAllVirtualMachineQuery query;
	query.m_primitiveOffset = 0;
	query.m_properties[0] = 0;
	queryOnTree( &query, &code->m_data[0]);
}

#endif // HK_COLLIDE2_MOPP_DEBUGGER_H

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
