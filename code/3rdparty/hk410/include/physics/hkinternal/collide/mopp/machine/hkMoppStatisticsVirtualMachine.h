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
class hkMoppStatisticsVirtualMachine : public hkMoppVirtualMachine
{
	public:
			// standard constructor
		inline hkMoppStatisticsVirtualMachine(){}
		// standard destructor
		inline ~hkMoppStatisticsVirtualMachine(){}

		HK_FORCE_INLINE void queryAll(const hkMoppCode* code);

		void printStatistics(const hkMoppCode* code);

	public: 
		struct Entry
		{
			int m_numSplitA;
			int m_numSplitDiag;
			int m_numSingleSplit;
			int m_numSingleSplitJump;
			int m_numDoubleCut;
			int m_numDoubleCut24;
			int m_numJump8;
			int m_numJump16;
			int m_numJump24;
			int m_numScale;
			int m_numReoffset8;
			int m_numReoffset16;
			int m_numReoffset32;
			int m_numTerm4;
			int m_numTerm8;
			int m_numTerm16;
			int m_numTerm24;
			int m_numTerm32;
			
			
		};
		struct hkMoppStatisticsVirtualMachineQuery
		{
			unsigned int m_primitiveOffset;  
			unsigned int m_properties[hkMoppCode::MAX_PRIMITIVE_PROPERTIES];

		};
		enum { MAX_DEEP = 32 };

		Entry m_topDown[MAX_DEEP];
		Entry m_bottomUp[MAX_DEEP];

		int queryOnTree	( const hkMoppStatisticsVirtualMachineQuery* query, int deep, const unsigned char* commands);
};

void hkMoppStatisticsVirtualMachine::queryAll(const hkMoppCode* code)
{
	hkMoppStatisticsVirtualMachineQuery query;
	query.m_primitiveOffset = 0;
	query.m_properties[0] = 0;
	hkString::memSet( &m_topDown[0], 0, sizeof(m_topDown));
	hkString::memSet( &m_bottomUp[0], 0, sizeof(m_bottomUp));
	queryOnTree( &query, 0, &code->m_data[0]);
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
