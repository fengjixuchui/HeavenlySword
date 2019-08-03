/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_DYNAMICS2_BINARY_ACTION_H
#define HK_DYNAMICS2_BINARY_ACTION_H

#include <hkdynamics/action/hkAction.h>
class hkRigidBody;

extern const hkClass hkBinaryActionClass;

	/// You can use this as a base class for hkActions that operate on a pair of
	/// hkRigidBodies. In addition to the hkAction interface, this class provides
	/// some useful basic functionality for actions, such as a callback that removes
	/// the action from the simulation if either of its hkRigidBodies is removed.
class hkBinaryAction : public hkAction
{
	public:
		
		HK_DECLARE_REFLECTION();

			/// Constructor creates a new hkBinaryAction that operates on the specified entities.
		hkBinaryAction( hkEntity* entityA, hkEntity* entityB, hkUint32 userData = 0 );

			/// hkAction interface implementation.
		virtual void getEntities( hkArray<hkEntity*>& entitiesOut );

			/// Removes the action from the hkWorld if one of its hkRigidBodies is removed.
		virtual void entityRemovedCallback(hkEntity* entity);

			/// Sets m_bodyA, adds a reference to it and adds the action as a listener.
			/// NB: Only intended to be called pre-simulation i.e. before hkBinaryAction
			/// is added to an hkWorld.
		void setEntityA( hkEntity* entityA);

			/// Gets m_bodyA.
		inline hkEntity* getEntityA();

			/// Sets m_bodyB, adds a reference to it and adds the action as a listener.
			/// NB: Only intended to be called pre-simulation i.e. before hkBinaryAction
			/// is added to an hkWorld.
		void setEntityB( hkEntity* entityB);

			/// Gets m_bodyB.
		inline hkEntity* getEntityB();

			/// The applyAction() method does the actual work of the action, and is called at every simulation step.
		virtual void applyAction( const hkStepInfo& stepInfo ) = 0;

	protected:
			/// Destructor.
		virtual ~hkBinaryAction();

			/// The first entity.
		hkEntity* m_entityA;

			/// The second entity.
		hkEntity* m_entityB;

		void _referenceBodies(); 

	public:

		hkBinaryAction( class hkFinishLoadedObjectFlag flag ) : hkAction(flag) {}
};

#include <hkdynamics/action/hkBinaryAction.inl>


#endif // HK_DYNAMICS2_BINARY_ACTION_H

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
