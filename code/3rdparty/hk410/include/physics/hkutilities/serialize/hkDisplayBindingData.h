/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HKUTILITIES_BINDINGDATA_H
#define HKUTILITIES_BINDINGDATA_H

#include <hkmath/hkMath.h>
#include <hkbase/htl/hkArray.h>

extern const class hkClass hkRigidBodyDisplayBindingClass;
extern const class hkClass hkPhysicsSystemDisplayBindingClass;
extern const class hkClass hkDisplayBindingDataClass;

// A single 'rigid body <-> display object' binding
struct hkRigidBodyDisplayBinding 
{
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_UTILITIES, hkRigidBodyDisplayBinding );
	HK_DECLARE_REFLECTION();
		
	class hkRigidBody* m_rigidBody;
	
	class hkxMesh* m_displayObject;
	
	hkMatrix4 m_rigidBodyFromDisplayObjectTransform;
};

// A physics system and its collection of bindings
struct hkPhysicsSystemDisplayBinding
{
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_UTILITIES, hkPhysicsSystemDisplayBinding );
	HK_DECLARE_REFLECTION();
	
	hkArray<hkRigidBodyDisplayBinding*> m_bindings;
	
	class hkPhysicsSystem* m_system;
	
		/// For serialization we need a ctor that does not call the ctor of the array on load.
	hkPhysicsSystemDisplayBinding() { }
	hkPhysicsSystemDisplayBinding(hkFinishLoadedObjectFlag f) : m_bindings(f) { }  
};

// A collection of display bindings
struct hkDisplayBindingData
{
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_UTILITIES, hkDisplayBindingData );
	HK_DECLARE_REFLECTION();
	
	hkArray<hkRigidBodyDisplayBinding*> m_rigidBodyBindings;
	hkArray<hkPhysicsSystemDisplayBinding*> m_physicsSystemBindings;
	
		/// For serialization we need a ctor that does not call the ctor of the arrays on load.
	hkDisplayBindingData() { }
	hkDisplayBindingData(hkFinishLoadedObjectFlag f) : m_rigidBodyBindings(f), m_physicsSystemBindings(f) { }  
};

#endif // HKUTILITIES_BINDINGDATA_H

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
