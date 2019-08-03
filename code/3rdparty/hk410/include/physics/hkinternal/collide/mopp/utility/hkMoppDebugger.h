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

#ifndef HK_COLLIDE2_MOPP_DEBUGGER_H
#define HK_COLLIDE2_MOPP_DEBUGGER_H

#include <hkmath/hkMath.h>


// the idea of the debugger is that we are searching our triangle in the entire original tree and
// remembering all paths to this node.

// when it comes to the final virtual machine, we easily can check, which paths are not taken,
// and which extra paths are processed which shouldn't be processed
class hkMoppDebugger : public hkSingleton<hkMoppDebugger> 
{
	protected:
		friend class hkSingleton<hkMoppDebugger>;

		class hkMoppPath 
		{
		public:
			enum { HK_PATH_MAX_DEPTH = 256 };
			char m_path[HK_PATH_MAX_DEPTH];
		};

		hkArray<hkMoppPath> m_paths;

		// local variables used by queryTriOnTree 
		int		m_searchTriIndex;
		int		m_searchDepth;
		char*	m_searchPath;

		struct hkDbgQuery
		{
			int m_primitiveOffset;
			unsigned int m_properties[16];
		};


		void addHit(unsigned int id, const unsigned int properties[hkMoppCode::MAX_PRIMITIVE_PROPERTIES]);
		
		void queryTriOnTree( hkDbgQuery *query, const unsigned char *PC, int triIndex, char* path, int depth);

		/// Called when something went wrong, Good place to set a breakpoint;
		void errorOccured();

	public: 
		hkMoppDebugger();

			/// The main initialization function, call this before you call a mopp query
		void initUsingCodeAndTri( const hkMoppCode* moppCode, const int triangleIndex);

			/// call this if you want to disable the debugger. Note: the debugger still costs CPU
		void initDisabled();

	protected:
		char m_currentPath[hkMoppPath::HK_PATH_MAX_DEPTH];
		int  m_currentDepth;

		//
		//	internal public functions, called by the machines
		//
	public:
		hkBool find();

		void startRecurse();

		// returns the current recursion depth
		int recurseLeft();

		// returns the current recursion depth
		int recurseRight();
		void pop(int recursionDepth);
		void rejectLeft();
		void rejectRight();

};

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
