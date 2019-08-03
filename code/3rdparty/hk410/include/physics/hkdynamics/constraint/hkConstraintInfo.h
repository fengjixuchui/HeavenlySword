/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_DYNAMICS2_CONSTRAINT_INFO_H
#define HK_DYNAMICS2_CONSTRAINT_INFO_H

//! Keep the members of this synced with the generic constraint cinfo
//! as it has to save all these members.
struct hkConstraintInfo
{
	// We need this reflected as it is serialized in the generic constraint.
	HK_DECLARE_REFLECTION();

		// This variable only has a meaning in an island (it is equal to sizeOfJacobians for constraints).
		// It realates to splitting buffers on playstation
	int m_maxSizeOfJacobians;
	int m_sizeOfJacobians;
	int m_sizeOfSchemas;
	int m_numSolverResults;

	void clear( ) { m_maxSizeOfJacobians = 0; m_sizeOfJacobians = 0; m_sizeOfSchemas = 0; m_numSolverResults = 0; }
	void addHeader( ) { m_sizeOfSchemas += HK_SIZE_OF_JACOBIAN_HEADER_SCHEMA; }
	void add( int schemaSize, int jacSize, int numSolverResults = 1 ) { m_sizeOfSchemas += schemaSize; m_sizeOfJacobians += jacSize; m_numSolverResults += numSolverResults; }

	inline void add( const hkConstraintInfo& other);
	inline void sub( const hkConstraintInfo& other);
};

inline void hkConstraintInfo::add( const hkConstraintInfo& delta)
{
	m_maxSizeOfJacobians = hkMath::max2( m_maxSizeOfJacobians, delta.m_maxSizeOfJacobians);
	m_maxSizeOfJacobians = hkMath::max2( m_maxSizeOfJacobians, delta.m_sizeOfJacobians);
	m_sizeOfJacobians  += delta.m_sizeOfJacobians;
	m_sizeOfSchemas    += delta.m_sizeOfSchemas;
	m_numSolverResults += delta.m_numSolverResults;
}

inline void hkConstraintInfo::sub( const hkConstraintInfo& delta)
{
	m_sizeOfJacobians  -= delta.m_sizeOfJacobians;
	m_sizeOfSchemas    -= delta.m_sizeOfSchemas;
	m_numSolverResults -= delta.m_numSolverResults;
}

#endif // HK_DYNAMICS2_CONSTRAINT_DATA_H

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
