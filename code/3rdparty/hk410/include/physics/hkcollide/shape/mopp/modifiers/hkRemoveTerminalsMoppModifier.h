/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
//
//

#ifndef HK_COLLIDE2_MOPP_REMOVE_TERMINALS_MODIFIER_H
#define HK_COLLIDE2_MOPP_REMOVE_TERMINALS_MODIFIER_H

#include <hkinternal/collide/mopp/machine/hkMoppModifier.h>
#include <hkcollide/agent/hkCdBody.h>

class hkMoppCode;
class hkShapeCollection;


/// This class allows you to efficiently remove terminals from a mopp
/// You can many instances of this class as long each class removed different terminals from the mopp.
/// This class builds a single aabb for all terminals to remove to speed up the building of the internal tables.
/// To get good performance, the triangle to remove should be all in the same area
class hkRemoveTerminalsMoppModifier: public hkMoppModifier
{
	public:
			/// Build this modifier but does not alter the moppCode yet, so you can pre construct this object
		hkRemoveTerminalsMoppModifier( const hkMoppCode* moppCode, const hkShapeCollection* shapeCollection, hkArray<hkShapeKey>& shapesToRemove );
		virtual ~hkRemoveTerminalsMoppModifier();

			/// Actually remove the terminals from the moppCode. This function is very fast!!!
		void applyRemoveTerminals( hkMoppCode* moppCode );

			/// Undo the removal of the terminals. This function is very fast!!!
		void undoRemoveTerminals( hkMoppCode* moppCode );

	public:
			// hkMoppModifier interface implementation
		virtual hkBool shouldTerminalBeRemoved( unsigned int id, const unsigned int *properties );

			// hkMoppModifier interface implementation
		virtual void addTerminalRemoveInfo( int relativeMoppAddress );

	public:
		struct hkMoppRemoveInfo
		{
			hkUchar m_originalMoppCode;
			unsigned int m_moppOffset:24;
		};

		hkArray<hkMoppRemoveInfo> m_removeInfo;

	private:
			/// for construction only
		hkArray<hkShapeKey> *m_tempShapesToRemove;
};		

#endif // HK_COLLIDE2_MOPP_REMOVE_TERMINALS_MODIFIER_H


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
