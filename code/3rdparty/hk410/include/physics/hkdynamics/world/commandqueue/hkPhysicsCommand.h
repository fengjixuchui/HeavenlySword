/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_DYNAMICS2_PHYSICS_COMMAND_H
#define HK_DYNAMICS2_PHYSICS_COMMAND_H

#include <hkbase/hkBase.h>
#include <hkdynamics/constraint/hkConstraintInfo.h>

	//
	//	The base class of all physics commands
	//
struct hkPhysicsCommand
{
	enum TYPE
	{
		TYPE_ADD_CONSTRAINT_TO_LOCKED_ISLAND,
		TYPE_REMOVE_CONSTRAINT_FROM_LOCKED_ISLAND,
		TYPE_END
	};
	hkPhysicsCommand(TYPE type): m_type(type){}

	hkEnum<TYPE,hkUchar> m_type;
};


	//
	//	A small helper template class used for simple single object commands
	//
template<hkPhysicsCommand::TYPE COMMAND_TYPE, typename OBJECT_TYPE>
struct hkSingleObjectPhysicsCommand: public hkPhysicsCommand
{
	HK_FORCE_INLINE hkSingleObjectPhysicsCommand( OBJECT_TYPE object ): hkPhysicsCommand( COMMAND_TYPE ), m_object(object){}
	OBJECT_TYPE m_object;
};

//
//	A small helper template class used for simple dual object commands
//
template<hkPhysicsCommand::TYPE COMMAND_TYPE, typename OBJECT_TYPE0, typename OBJECT_TYPE1>
struct hkDualObjectPhysicsCommand: public hkPhysicsCommand
{
	HK_FORCE_INLINE hkDualObjectPhysicsCommand( OBJECT_TYPE0 object0, OBJECT_TYPE1 object1 ): hkPhysicsCommand( COMMAND_TYPE ), m_object0(object0), m_object1(object1){}
	OBJECT_TYPE0 m_object0;
	OBJECT_TYPE1 m_object1;
};

struct hkConstraintInfoExtended: public hkConstraintInfo
{
	hkConstraintInstance* m_constraint;
};

	//
	//	Simple single objects commands
	//
typedef hkDualObjectPhysicsCommand  <hkPhysicsCommand::TYPE_ADD_CONSTRAINT_TO_LOCKED_ISLAND,      hkConstraintInstance*,int> hkAddConstraintToCriticalLockedIslandPhysicsCommand;
typedef hkSingleObjectPhysicsCommand<hkPhysicsCommand::TYPE_REMOVE_CONSTRAINT_FROM_LOCKED_ISLAND, hkConstraintInstance*> hkRemoveConstraintFromCriticalLockedIslandPhysicsCommand;


extern void hkPhysicsCommandMachineProcess( hkWorld* world, hkPhysicsCommand* begin, hkPhysicsCommand* end );

#endif // HK_DYNAMICS2_PHYSICS_COMMAND_H

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
