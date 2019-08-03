/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_MATH_CONTACT_POINT_MATERIAL_H
#define HK_MATH_CONTACT_POINT_MATERIAL_H

#include <hkmath/hkMath.h>
	/// This class is used to get and set the friction for a contact point. You can also use it to attach your own user data
	/// to a contact point.  This can be used for example to set a friction map value in when a contact point is added
	/// so that the same data can be used when the contact point is being updated (from a processContactCallback() for example)
class hkContactPointMaterial
{
	public:
		HK_DECLARE_REFLECTION();

			/// Get the friction for the contact point, see setFriction().
		inline hkReal getFriction( ) const;

			/// Get the friction as a 8.8 fixed point value
		inline int getFriction8_8( ) const;

			/// Set the friction for the contact point.
			/// Usually the value will be between 0 and 1, but may be set be greater than 1.
			/// Internally this is stored as an 8.8 fixed point value, so
			/// values must lie in the range 0 (no friction) to 255 (max friction).
		inline void setFriction( hkReal r );

			/// Set the restitution for the contact point, see setRestitution().
		inline hkReal getRestitution( ) const;

			/// Set the restitution for the contact point.
			/// Usually the value will be between 0 (all energy lost in collision) and 1 (max restitution), but may be set greater than 1.
			/// Internally this is stored as an 1.7 fixed point value, so values must lie in the range 0 to 1.98.
			/// Note: for a contact point that has been around for several frames this has virtually no effect. You need to set the
			/// restitution for a contact point on creation of the contact point (when the approaching velocities are non-zero).
		inline void setRestitution( hkReal r );

#if !defined (HK_PLATFORM_PS3SPU)
			/// Get the user data	
		inline void* getUserData() const;

			/// Set the user data. This allows you to store any info, or a pointer with a contact point.
		inline void setUserData( void* data );
#endif

			/// returns true if the contact point might still be a potential contact point
		inline hkBool isPotential();

			// 
			//	Internal section
			//

	protected:
		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_COLLIDE, hkContactPointMaterial );

			/// Not used by Havok, feel free to use it
		void* m_userData; //+nosave
			
			/// The friction stored as 8.8 fixed point -> range [0...255]
		hkUint16 m_friction;	

			/// the restitution stored as 1.7 fixed point -> range [0...1.98]
		hkUint8 m_restitution;

	public:
		void reset()
		{
			m_friction = 0;
			m_restitution = 0;
			m_flags = CONTACT_IS_NEW_AND_POTENTIAL;
		}


		enum  FlagEnum
		{
			// set for a contact point which is not verified
			CONTACT_IS_NEW_AND_POTENTIAL  = 1,

			// internal solver optimization, do not change (engine can crash if you do)
			CONTACT_USES_SOLVER_PATH2 = 2,
		};

		/// see FlagEnum for how it is used.
		/// Warning: do not modify yourself
		hkUint8 m_flags;
};



#include <hkmath/basetypes/hkContactPointMaterial.inl>

#endif // HK_MATH_CONTACT_POINT_MATERIAL_H

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
