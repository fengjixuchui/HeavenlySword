/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_CONSTRAINTSOLVER2_JACOBIAN_ELEMENT_H
#define HK_CONSTRAINTSOLVER2_JACOBIAN_ELEMENT_H


#include <hkconstraintsolver/accumulator/hkVelocityAccumulator.h>
#include <hkconstraintsolver/jacobian/hkJacobianHeaderSchema.h>

class hkSolverResults;
class hk2AngJacobian;
class hk1Lin2AngJacobian;
class hk2Lin2AngJacobian;

class hkJacobianElement
{
	public:
		inline hk1Lin2AngJacobian* as1Lin2Ang(){ return reinterpret_cast<hk1Lin2AngJacobian*>(this); }
		inline const hk1Lin2AngJacobian* as1Lin2Ang() const { return reinterpret_cast<const hk1Lin2AngJacobian*>(this); }

		inline hk2AngJacobian* as2Ang(){ return reinterpret_cast<hk2AngJacobian*>(this); }
		inline const hk2AngJacobian* as2Ang() const { return reinterpret_cast<const hk2AngJacobian*>(this); }

		inline hk2Lin2AngJacobian* as2Lin2Ang(){ return reinterpret_cast<hk2Lin2AngJacobian*>(this); }
		inline const hk2Lin2AngJacobian* as2Lin2Ang() const { return reinterpret_cast<const hk2Lin2AngJacobian*>(this); }
};


class hk2AngJacobian
{
	public:

		/// the angular part of the constraint
		//  angular[0].w component is the invJacDiag
		hkVector4	m_angular[2];


		inline float getInvJacDiag() const		{			return m_angular[0](3);		}
		inline hkSimdReal getInvJacDiagSr() const		{	return m_angular[0].getSimdAt(3);		}
		inline void  setInvJacDiag( float v )	{			m_angular[0](3) = v;		}



		inline void  setAngularRhs( float v )	{			m_angular[1](3) = v;		}
		inline float getAngularRhs( ) const		{			return m_angular[1](3);		}
		inline hkSimdReal getAngularRhsSr( ) const		{	return m_angular[1].getSimdAt(3);		}


		inline hkJacobianElement* next(int i=1)	{ return  reinterpret_cast<hkJacobianElement*>(this+i); }
//	private:
	
			// get the diag where |linear| = 0.0f and |angular| = 1.0f
		HK_FORCE_INLINE float getAngularDiag( const hkVelocityAccumulator& mA, const hkVelocityAccumulator& mB ) const;

		HK_FORCE_INLINE hkReal getNonDiagOptimized( const hkVelocityAccumulator& mA, const hkVelocityAccumulator& mB, const hk2AngJacobian& jacB ) const;
		HK_FORCE_INLINE hkReal getNonDiagSameObjects( const hkVelocityAccumulator& mA, const hkVelocityAccumulator& mB, const hk2AngJacobian& jacB ) const; 
		HK_FORCE_INLINE hkReal getNonDiagDifferentObjects_With2ndBodyFromFirstObject( const hkVelocityAccumulator& mA, const hk2AngJacobian& jacB ) const;

};

class hk1Lin2AngJacobian
{
	public:

			/// the linear part
		hkVector4	m_linear0;		// .w component is the rhs

			/// the angular part of the constraint
		// angular[0].w component is the invJacDiag
		// angular[1].w usage is dependent on the hkJacobianSchema
		hkVector4	m_angular[2];


			// This casts the jacobian to a hk2Ang jacobian. Note that you cannot access its Rhs or AngularRhs values.
			// This requires that body hk2AngJac's and hk1Lin2AngJac's getInvJacDiag() use the same variable in m_angular vectors .
		inline hk2AngJacobian& as2AngJacobian_internal() { return *reinterpret_cast<hk2AngJacobian*>(&m_angular[0]); }

		// warning: call the next function only after setting the linear and angular part
		inline void setRHS( float v )			{			m_linear0(3) = v;		}
		inline float& getRHS()					{			return m_linear0(3);	}
		inline const float& getRHS() const		{			return m_linear0(3);	}
		inline const hkSimdReal getRhsSr() const		{			return m_linear0.getSimdAt(3);	}
												
		inline float getInvJacDiag() const		{			return m_angular[0](3);	}
		inline hkSimdReal getInvJacDiagSr() const		{			return m_angular[0].getSimdAt(3);	}
		inline void setInvJacDiag( float v )	{			m_angular[0](3) = v;	}
												
		inline float getUserValue() const		{			return m_angular[1](3);	}
		inline void setUserValue( float v )		{			m_angular[1](3) = v;	}


		inline hkJacobianElement* next(int i = 1){ return  reinterpret_cast<hkJacobianElement*>(this+i); }


//	private:
			// get J dot ((M-1)*J)  restrictions: |linear| = 1.0f
		HK_FORCE_INLINE float getDiag( const hkVelocityAccumulator& mA, const hkVelocityAccumulator& mB ) const;

			/// Get the non diagonal element of the 2*2 inverse mass matrix in the case that jacA and jacB share exactly the same rigid bodies.
			/// Gets J dot ((M-1)*JacB)
			/// This is a special implementation which makes use of the fact that some PS2 implementations left the last
			/// jacobian in registers. Be extra careful when using this function.
		HK_FORCE_INLINE float getNonDiagOptimized( const hkVelocityAccumulator& mA, const hkVelocityAccumulator& mB, const hk1Lin2AngJacobian& jacB ) const;

			/// Get the non diagonal element in the case that jacA and jacB share exactly the same rigid bodies.
			/// Gets J dot ((M-1)*JacB).
			/// If this and jacB are connecting the same object pair in the same direction.
		HK_FORCE_INLINE float getNonDiagSameObjects( const hkVelocityAccumulator& mA, const hkVelocityAccumulator& mB, const hk1Lin2AngJacobian& jacB ) const;

			/// Get the non diagonal element in the case of different rigid bodies.
			/// Gets J dot ((M-1)*JacB).
			/// Given that mA is the common object of both jacobians and the mB differ.
		HK_FORCE_INLINE float getNonDiagDifferentObjects( const hkVelocityAccumulator& mA, const hk1Lin2AngJacobian& jacB ) const;

		// get J dot ((M-1)*JacB)  
		// given that mB of this jacobian is mA of jacB
		HK_FORCE_INLINE float getNonDiagDifferentObjects_With2ndBodyFromFirstObject( const hkVelocityAccumulator& mA, const hk1Lin2AngJacobian& jacB ) const;

};

class hkJacTriple2Bil1ang
{
public:
	hk1Lin2AngJacobian  m_jac0;
	hk1Lin2AngJacobian  m_jac1;
	hk2AngJacobian		m_jac2;
};


class hk2Lin2AngJacobian
{
	public:

			/// the linear part. m_linear[0].w component is the rhs
		hkVector4   m_linear[2];	

			/// the angular part of the constraint
			// angular[0].w component is the invJacDiag
			// angular[1].w usage is dependent on the hkJacobianSchema
		hkVector4	m_angular[2];


		// warning: call the next function only after setting the linear and angular part
		inline void setRHS( float v )				{			m_linear[0](3) = v;		}
		inline float getRHS() const					{			return m_linear[0](3);	}
		inline hkSimdReal getRhsSr() const					{			return m_linear[0].getSimdAt(3);	}
		
		inline float getInvJacDiag() const			{			return m_angular[0](3);	}
		inline hkSimdReal getInvJacDiagSr() const	{			return m_angular[0].getSimdAt(3);	}
		inline void setInvJacDiag( float v )		{			m_angular[0](3) = v;	}
												

		inline hkJacobianElement* next(int i = 1)	{ return  reinterpret_cast<hkJacobianElement*>(this+i); }


//	private:

			// get J dot ((M-1)*J)  restrictions: |linear| = 1.0f
		HK_FORCE_INLINE float getDiag( const hkVelocityAccumulator& mA, const hkVelocityAccumulator& mB, hkReal leverageRatio ) const;
};


#include <hkconstraintsolver/jacobian/hkJacobianElement.inl>

#endif // HK_CONSTRAINTSOLVER2_JACOBIAN_ELEMENT_H

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
