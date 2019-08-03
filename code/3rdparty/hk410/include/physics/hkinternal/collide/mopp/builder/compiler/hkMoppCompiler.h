/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

//
// Havok Memory Optimised Partial Polytope Compiler
// This class generates the tree to be passed to the assembler.
// All primitives need to be added to the system and a filename specified BEFORE compilation.
//

#ifndef HK_COLLIDE2_MOPP_COMPILER_H
#define HK_COLLIDE2_MOPP_COMPILER_H


	// maximum number of nodes used if interleaved building is enabled
#define HK_MOPP_ENABLED_INTERLEAVED_BUILDING_SPLITTER_MAX_NODES 4096


/// the mopp compiler is the wrapper around different algorithms needed
/// to compile a set of convex primitives (defined by the mediator)
/// into a mopp byte code
class hkMoppCompiler 
{
	public:

		/// standard constructor
		hkMoppCompiler( hkMoppMeshType meshType = HK_MOPP_MT_LANDSCAPE );  

		/// standard destructor
		~hkMoppCompiler();  

		/// optionally set splitter params
		/// for defining how the volume is split into a partial poyltope hierarchy
		void setSplitParams( const hkMoppSplitter::hkMoppSplitParams& );

		/// optionally set some params
		void setCostParams( const hkMoppCostFunction::hkMoppSplitCostParams& );

		/// optionally set assembler params
		void setAssemblerParams  ( const hkMoppAssembler::hkMoppAssemblerParams& );

		/// optionally enable/disable interleaved building
		///
		/// The mopp compiler has two operating modes:
		/// - interleaved building enabled:<br>
		///   the hkMoppCompiler grabs a fixed buffer size (roughly 2 megabytes) independent of the number of triangles 
		///   (which is good, if you have lots and lots of triangles).
		/// - interleaved building disabled<br>
		///   it calculates the correct buffer size. However this buffer size can be extremely
		///   huge as each triangle takes more than 430 bytes.
		///
		/// By default interleaved building is enabled.
		void enableInterleavedBuilding(bool);

		/// Get the size of the temporary storage which is internally required by the compiler.
		/// Note: the compiler is doing very little allocations and deallocations except for generating the code.
		int calculateRequiredBufferSize( hkMoppMediator* );

		/// compile the primitives defined by the mediator into the mopp byte code
		hkMoppCode* compile(hkMoppMediator* m_mediator, char* buffer = HK_NULL, int bufferSize = 0 );

	public:
		/// the root node of the compilation process
		hkMoppTreeNode* m_debugRootNode;
	protected:

		hkBool m_interleavedBuildingEnabled;
		hkMoppSplitter::hkMoppSplitParams				m_splitParams;
		hkMoppCostFunction::hkMoppSplitCostParams		m_splitCostParams;
		hkMoppAssembler::hkMoppAssemblerParams		m_assemblerParams;
};

#endif // HK_COLLIDE2_MOPP_COMPILER_H

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
