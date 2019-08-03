/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_CONSTRAINTSOLVER2_JACOBIAN_HEADER_SCHEMA_H
#define HK_CONSTRAINTSOLVER2_JACOBIAN_HEADER_SCHEMA_H

#include <hkconstraintsolver/jacobian/hkJacobianSchema.h>

#include <hkconstraintsolver/jacobian/hkJacobianElement.h>
#include <hkconstraintsolver/accumulator/hkVelocityAccumulator.h>
#include <hkconstraintsolver/solve/hkSolverResults.h>
#include <hkbase/thread/hkSpuUtils.h>


class hkJacobianElement;
class hkSolverResults;

class hkJacobianHeaderSchema : public hkJacobianSchema
{
	public:

		HK_FORCE_INLINE void initHeader(	const hkUint32 bodyAOffset, const hkUint32 bodyBOffset, 
											const hkUint32 jacobianOffset, HK_CPU_PTR(hkSolverResults*) sr, 
											int solverResultStriding );

		HK_FORCE_INLINE hkJacobianElement*     getJacobian( hkJacobianElement*     jacobianBuffer    ) const { return hkAddByteOffset(jacobianBuffer,    m_jacobianOffset); }
		HK_FORCE_INLINE hkVelocityAccumulator* getBodyA   ( hkVelocityAccumulator* accumulatorBuffer ) const { return hkAddByteOffset(accumulatorBuffer, m_bodyAOffset   ); }
		HK_FORCE_INLINE hkVelocityAccumulator* getBodyB   ( hkVelocityAccumulator* accumulatorBuffer ) const { return hkAddByteOffset(accumulatorBuffer, m_bodyBOffset   ); }

	private:
		
		hkUint32 m_jacobianOffset;
		hkUint32 m_bodyAOffset;
		hkUint32 m_bodyBOffset;

	public:
		int m_solverResultStriding;
		HK_CPU_PTR(hkSolverResults*) m_solverResultInMainMemory;
};

class hkJacobianGotoSchema: public hkJacobianSchema
{
	public:
		hkUint32 m_offset;

		HK_FORCE_INLINE void initGoto( hkJacobianSchema* destination )
		{
			m_offset = int(hkGetByteOffset( this, destination ));
			setType( SCHEMA_TYPE_GOTO, sizeof( *this ), JACOBIANS_TYPE_NONE_GOTO32 );
		}

		HK_FORCE_INLINE void initOffset( int offset )
		{
			m_offset = offset;
			setType( SCHEMA_TYPE_GOTO, sizeof( *this ), JACOBIANS_TYPE_NONE_GOTO32 );
		}
};

// relative jump
class hkJacobianGoto8Schema: public hkJacobianSchema
{
	public:
		HK_FORCE_INLINE void initGoto( hkJacobianSchema* destination )
		{
			int offset = int(hkGetByteOffset( this, destination ));
			setType( SCHEMA_TYPE_GOTO8, offset, JACOBIANS_TYPE_NONE );
		}

		HK_FORCE_INLINE void initOffset( int offset )
		{
			setType( SCHEMA_TYPE_GOTO8, offset, JACOBIANS_TYPE_NONE );
		}
};

void hkJacobianHeaderSchema::initHeader( const hkUint32 bodyAOffset, const hkUint32 bodyBOffset, 
										 const hkUint32 jacobianOffset, HK_CPU_PTR(hkSolverResults*) sr, 
										 int solverResultStriding )
{
	setType( SCHEMA_TYPE_HEADER, sizeof( *this ), JACOBIANS_TYPE_HEADER );

	m_jacobianOffset = jacobianOffset;
	m_bodyAOffset    = bodyAOffset;
	m_bodyBOffset    = bodyBOffset;

	m_solverResultInMainMemory = sr;
	m_solverResultStriding = solverResultStriding;
}

#endif // HK_CONSTRAINTSOLVER2_JACOBIAN_HEADER_SCHEMA_H

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
