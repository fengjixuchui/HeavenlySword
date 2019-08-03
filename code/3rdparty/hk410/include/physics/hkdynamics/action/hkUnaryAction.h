/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_DYNAMICS2_UNARY_ACTION_H
#define HK_DYNAMICS2_UNARY_ACTION_H

#include <hkdynamics/action/hkAction.h>

extern const hkClass hkUnaryActionClass;

	/// You can use this as a base class for hkActions that operates on a single hkEntity.
	/// In addition to the hkAction interface, this class provides
	/// some useful basic functionality for actions, such as a callback that
	/// removes the action from the simulation if its hkEntity is removed.
class hkUnaryAction : public hkAction
{
	public:

		HK_DECLARE_REFLECTION();
		
			/// Constructor creates a hkUnaryAction that operates on the specified hkEntity.
		hkUnaryAction(hkEntity* body = HK_NULL, hkUint32 userData = 0);

			/// Constructor.
		~hkUnaryAction();

			/// hkAction interface implementation.
		virtual void getEntities( hkArray<hkEntity*>& entitiesOut );

			/// Remove self from the hkWorld when the hkEntity is removed.
		virtual void entityRemovedCallback(hkEntity* entity);

			/// Sets m_body, adds a reference to it and adds the action as a listener.
			/// NB: Only intended to be called pre-simulation i.e. before hkUnaryAction
			/// is added to an hkWorld.
		void setEntity(hkEntity* rigidBody);

			/// Gets m_body.
		inline hkEntity* getEntity();

			/// The applyAction() method does the actual work of the action, and is called at every simulation step.
		virtual void applyAction( const hkStepInfo& stepInfo ) = 0;

	protected:

			/// The hkEntity.
		hkEntity* m_entity;

	public:

		hkUnaryAction( class hkFinishLoadedObjectFlag flag ) : hkAction(flag) {}
};

#include <hkdynamics/action/hkUnaryAction.inl>

#endif // HK_DYNAMICS2_UNARY_ACTION_H

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
