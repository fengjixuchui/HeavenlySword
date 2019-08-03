/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_CONSTRAINTSOLVER2_JACOBIAN_SCHEMA_H
#define HK_CONSTRAINTSOLVER2_JACOBIAN_SCHEMA_H

	/// the base class of all constraint commands passed to the solver
class hkJacobianSchema
{
	public:

		enum SchemaType
		{
			// Note: we assume 	SCHEMA_TYPE_END will be 0 in hkdynamics code (where we do not want to include this file). 
			// Do not change this!
			SCHEMA_TYPE_END = 0,
			SCHEMA_TYPE_HEADER,
			SCHEMA_TYPE_GOTO8,
			SCHEMA_TYPE_SINGLE_CONTACT,
			SCHEMA_TYPE_PAIR_CONTACT,
			SCHEMA_TYPE_1D_ANGULAR_MOTOR,
			SCHEMA_TYPE_1D_LINEAR_MOTOR,
			SCHEMA_TYPE_1D_FRICTION,
			SCHEMA_TYPE_2D_FRICTION,
			SCHEMA_TYPE_3D_FRICTION,
			SCHEMA_TYPE_1D_ANGULAR_FRICTION,
			SCHEMA_TYPE_1D_ANGULAR_LIMITS,
			SCHEMA_TYPE_1D_LINEAR_LIMITS,
			SCHEMA_TYPE_1D_ANGULAR,
			SCHEMA_TYPE_1D_BILATERAL,
			SCHEMA_TYPE_1D_BILATERAL_USER_TAU,	

			SCHEMA_TYPE_SET_MASS,				
			SCHEMA_TYPE_ADD_VELOCITY,			

			SCHEMA_TYPE_1D_PULLEY,

			SCHEMA_TYPE_SHIFT_SOLVER_RESULTS_POINTER,

			SCHEMA_TYPE_SIMPLE_END,			// the end of the simple types synchronize with hkSolverExport::exportImpulsesAndRhs

			SCHEMA_TYPE_GOTO,				// warning this goto must be after SCHEMA_TYPE_SIMPLE_END


			SCHEMA_TYPE_STIFF_SPRING_CHAIN,	
			SCHEMA_TYPE_BALL_SOCKET_CHAIN,
			SCHEMA_TYPE_POWERED_CHAIN
		};

			// this is used for debug output and for skipping unused jacobians in the beginning of exportImpulsesAndRhs
			// the order is important
		enum JacobianType
		{
			// Note: we assume JACOBIANS_TYPE_END will be 0 in hkdynamics code (where we do not want to include this file). 
			// Do not change this!
			// the order of this enum must be synchronized with hkSolverExport::exportImpulsesAndRhs()
			JACOBIANS_TYPE_END = 0,
			JACOBIANS_TYPE_HEADER,		// thats a header

				// no solver results
			JACOBIANS_TYPE_NONE,		// no jacobians
			JACOBIANS_TYPE_NONE_GOTO32, // no jacobians, but a special goto schema
			JACOBIANS_TYPE_VECTOR4, 
			JACOBIANS_TYPE_2VECTOR4, 

				// variable number of solver results
			JACOBIANS_TYPE_STIFF_SPRING_CHAIN,
			JACOBIANS_TYPE_BALL_SOCKET_CHAIN,
			JACOBIANS_TYPE_POWERED_CHAIN,

				// 1 solver results
			JACOBIANS_TYPE_1D_BIL,		// thats 1 jacobian (BIL) with one linear and two angular vectors
			JACOBIANS_TYPE_1D_ANG,		// thats 1 jacobian (ANG) with two angular vectors
			JACOBIANS_TYPE_1D_LLAA,		// thats 1 jacobian (LLAA) with two linear and two angular vectors

				// 2 solver results
			JACOBIANS_TYPE_2D_BIL,			// thats 2 jacobian BIL

				// 3 solver results 
			JACOBIANS_TYPE_2D_BIL_1D_ANG,	// thats 2 BIL and 1 ANG


		};

		SchemaType   getSchemaType()	const	{ return static_cast<SchemaType>(m_type.m_s.m_type); }
		int			 getSchemaSize()	const	{ return m_type.m_s.m_schemaSize; }
		JacobianType getJacobianType()	const	{ return static_cast<JacobianType>(m_type.m_s.m_jacobianType); }


	protected:
		union
		{
			struct 
			{
				SchemaType          m_type:8;
				JacobianType		m_jacobianType:8;
				unsigned			m_schemaSize:16; // large schemas are needed when using constraint chains
			} m_s;
			hkInt32 m_combined;
		} m_type;

	public:
		inline void setType(SchemaType schemaType, int schemaSize, JacobianType jacType)
		{
			HK_ASSERT2(0xad6755bb, schemaSize <= 0xffff, "Schema size to large.");

#		if HK_ENDIAN_LITTLE
			m_type.m_combined = schemaType | ( jacType << 8 ) | ( schemaSize << 16 );
#		elif HK_ENDIAN_BIG
			m_type.m_combined = (schemaType<<24) | ( jacType << 16 ) | schemaSize;
#		else
#			error unknown endianness
#		endif
		}


		enum 
		{
			// Jacobian end flag combined are two zero types + 4-byte size.
#		if HK_ENDIAN_LITTLE
				JACOBIAN_TYPE_END_FLAG_COMBINED = 0x00040000
#		elif HK_ENDIAN_BIG
				JACOBIAN_TYPE_END_FLAG_COMBINED = 0x00000004
#		else
#			error unknown endianness
#		endif

		};

};

#endif // HK_CONSTRAINTSOLVER2_JACOBIAN_SCHEMA_H

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
