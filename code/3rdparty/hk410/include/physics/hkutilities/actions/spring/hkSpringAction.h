/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_SPRING_ACTION_H
#define HK_SPRING_ACTION_H

#include <hkdynamics/action/hkBinaryAction.h>
#include <hkmath/hkMath.h>

extern const hkClass hkSpringActionClass;

/// A simple spring.
class hkSpringAction: public hkBinaryAction
{
	public:

		HK_DECLARE_REFLECTION();

			///Creates a spring with the specified construction info.
		hkSpringAction( hkRigidBody* entityA = HK_NULL, hkRigidBody* entityB = HK_NULL, hkUint32 userData = 0 );

			///Sets the spring's first rigid body.
		inline void setBodyA( hkRigidBody* ra, const hkVector4& positionAinA );

			///Sets the spring's second rigid body.
	    inline void setBodyB( hkRigidBody* rb, const hkVector4& positionBinB );

			/// Set the connection points to the bodies in world space. Make sure the
			/// body pointers are set before this call.
		void setPositionsInWorldSpace( const hkVector4& pivotA, const hkVector4& pivotB );

			/// Set the connection points to the bodies in body space. Make sure the
			/// body pointers are set before this call.
		void setPositionsInBodySpace( const hkVector4& pivotA, const hkVector4& pivotB );

			///Gets the spring connection position in rigid body A
		inline const hkVector4& getPositionAinA();

			///Gets the spring connection position in rigid body B
		inline const hkVector4& getPositionBinB();

			/// Sets the strength value for the spring.
        inline void setStrength( hkReal str );

			/// Gets the strength value for the spring.
		inline hkReal getStrength();

			/// Sets the damping value for the spring.
        inline void setDamping( hkReal damp );

			/// Gets the damping value for the spring.
        inline hkReal getDamping();

			/// Sets the rest length value for the spring.
        inline void setRestLength( hkReal restLength );

			/// Gets the rest length value for the spring.
        inline hkReal getRestLength();

			/// Sets the compression flag for the spring.
        inline void setOnCompression( hkBool onCompression );

			/// Gets the compression flag for the spring.
        inline hkBool getOnCompression();

			/// Sets the extension flag for the spring.
        inline void setOnExtension( hkBool onExtension );

			/// Gets the extension flag for the spring.
        inline hkBool getOnExtension();

			/// Gets the last force applied by the spring.
		inline const hkVector4& getLastForce();

		void applyAction( const hkStepInfo& stepInfo );

			/// hkAction clone interface
		virtual hkAction* clone( const hkArray<hkEntity*>& newEntities, const hkArray<hkPhantom*>& newPhantoms ) const;

	public:

		hkVector4 m_lastForce;

			/// The point in bodyA where the spring is attached. 	
		hkVector4 m_positionAinA; 

			/// The point in bodyB where the spring is attached. 	
		hkVector4 m_positionBinB;

			/// The desired rest length of the spring. 	
		hkReal	m_restLength;

			/// The desired strength of the spring. 	
		hkReal	m_strength;

			/// The damping to be applied to the spring. 	
		hkReal	m_damping;

			/// Flag to indicate if the spring acts during compression. 	
		hkBool	m_onCompression;

			/// Flag to indicate if the spring acts during extension. 	
		hkBool	m_onExtension;

	public:

		hkSpringAction( class hkFinishLoadedObjectFlag flag ) : hkBinaryAction(flag) {}
};

#include <hkutilities/actions/spring/hkSpringAction.inl>

#endif // HK_SPRING_H

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
