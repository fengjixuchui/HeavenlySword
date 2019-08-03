/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */


//
// hkMoppObbVirtualMachine Constructor
//
hkMoppObbVirtualMachine::hkMoppObbVirtualMachine()
{
}

//
// hkMoppObbVirtualMachine Destructor
//
hkMoppObbVirtualMachine::~hkMoppObbVirtualMachine()
{
	// nothing to do
}


void hkMoppObbVirtualMachine::generateQueryFromAabb(const hkVector4& aabbMin, const hkVector4& aabbMax, hkMoppObbVirtualMachineQuery& query)
{
#	if defined(HK_PS2) && (HK_CONFIG_SIMD == HK_CONFIG_SIMD_ENABLED)
	__asm__  (
"	.set noreorder \n" \
"		lqc2		vf9, 0x0(%5)		# vf9 = code->m_info.m_offset (includes scale at w) \n" \
"		addi		$3,  $0, 1		    # move $6.xyzw = 1,0,0,0 \n" \
"		vsub.xyz	vf6, %3, vf9		# vf6 += translation - offset = MIN \n" \
"		vsub.xyz	vf7, %4, vf9		# vf7 += translation - offset = MAX \n" \
"		pextlw		$3, $3, $3			# $3xyzw = 1,1,0,0 \n" \
"		vmulw		vf6, vf6, vf9		# vf6 = MIN * (scale,scale,scale,scale) \n" \
"		vmulw		vf7, vf7, vf9		# vf7 = MAX * (scale,scale,scale,scale) \n" \
"		sq			$0,  0x0(%2)		# m_offset.all = 0 \n" \
"		pextlw		$3, $3, $3			# $3xyzw = 1,1,1,1 \n" \
"		vftoi0		vf6, vf6			# vf6 = floor_to_int(vf6) \n" \
"		vftoi0		vf7, vf7			# vf7 = floor_to_int(vf7) \n" \
"		qmfc2		$4,	vf6				# $4 = &Min result \n" \
"		qmfc2		$5,	vf7				# $5 = &Max result \n" \
"		sqc2		vf6, 0x10(%1)		# m_xLo  = entire result for Min(overwrites all to radius) \n" \
"		sqc2		vf7, 0x0(%1)		# m_xHi  = entire result for Max(overwrites all to padding) \n" \
"		psraw		$4, $4, 16			# min result >> 16 \n" \
"		psraw		$5, $5, 16			# max result >> 16 \n" \
"		paddw 		$5, $5, $3 			# max values and radius += 1 \n" \
"		sq			$4, 0x10(%0)		# query.m_xLo = shifted start result \n" \
"		sq			$5, 0x0(%0)			# query.m_xHi = shifted end result \n" \
"	.set reorder \n" \
	 :
	  :	"r"(&query.m_xHi), "r"(&m_xHi), "r"(&query.m_offset_x),								// 012
	    "j"(aabbMin.getQuad()), "j"(aabbMax.getQuad()), "r"(&m_code->m_info.m_offset)				// 345
	  :	 "vf5", "vf6", "vf7", "vf8", "vf9", "vf10", "vf11", "vf12", "$2", "$3", "$4", "$5", "memory");
#	else // HK_PS2


	const hkVector4& maxV = aabbMax;
	const hkVector4& minV = aabbMin;


	//Scales the query into 16.16 fixed precision integer format
	m_xLo = toIntMin((minV(0) - m_code->m_info.m_offset(0)) * m_code->m_info.getScale());
	m_xHi = toIntMax((maxV(0) - m_code->m_info.m_offset(0)) * m_code->m_info.getScale());

	m_yLo = toIntMin((minV(1) - m_code->m_info.m_offset(1)) * m_code->m_info.getScale());
	m_yHi = toIntMax((maxV(1) - m_code->m_info.m_offset(1)) * m_code->m_info.getScale());

	m_zLo = toIntMin((minV(2) - m_code->m_info.m_offset(2)) * m_code->m_info.getScale());
	m_zHi = toIntMax((maxV(2) - m_code->m_info.m_offset(2)) * m_code->m_info.getScale());

	query.m_xLo = (m_xLo >> 16);
	query.m_xHi = (m_xHi >> 16) + 1;

	query.m_yLo = (m_yLo >> 16);
	query.m_yHi = (m_yHi >> 16) + 1;

	query.m_zLo = (m_zLo >> 16);
	query.m_zHi = (m_zHi >> 16) + 1;


	query.m_offset_x = 0;
	query.m_offset_y = 0;
	query.m_offset_z = 0;
#endif

	//any re-offsetting will occur in the tree
	query.m_primitiveOffset = 0;							
	query.m_shift = 0;

	query.m_properties[0] = 0;

	//hkprintf("Query %x,%x : %x,%x  %x,%x\n", query.m_xLo, query.m_xHi, query.m_yLo, query.m_yHi, query.m_zLo, query.m_zHi );
	/*
	for(int p = 0; p < hkMoppCode::MAX_PRIMITIVE_PROPERTIES; p++)
	{
		query.m_properties[p] = 0;
	}
	*/

	//now that the tempState is the currentState, we can override the currentState
}


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
