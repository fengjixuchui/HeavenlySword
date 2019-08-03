/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

void hkMoppVirtualMachine::addHit(unsigned int id, const unsigned int properties[hkMoppCode::MAX_PRIMITIVE_PROPERTIES])
{
	hkMoppPrimitiveInfo info;
	info.ID = id;
/*
	switch ( hkMoppCode::MAX_PRIMITIVE_PROPERTIES )
	{
	//case 3: info.properties[2] = properties[2];
	//case 2:	info.properties[1] = properties[1];
	//case 1:	info.properties[0] = properties[0];
	case 0:	break;
	//default: HK_ASSERT(0x2387d53b, 0);
	}
*/
	m_primitives_out->pushBack( info ); 

	HK_ASSERT2(0x6dcad53c, properties[0] == 0, "This mopp code format has been deprecated. You need to rebuild your mopp code.");

#ifdef HK_MOPP_DEBUGGER_ENABLED
	if ( hkMoppDebugger::getInstance().find() )
	{
		hkprintf("Adding correct triangle as %i %i\n", id, properties[0]);
	}
#endif
}

void hkMoppVirtualMachine::initQuery( hkArray<hkMoppPrimitiveInfo>* primitives_out )
{
	hkMoppVirtualMachine::m_primitives_out = primitives_out;
}

hkMoppVirtualMachine::hkMoppVirtualMachine()
{

}
hkMoppVirtualMachine::~hkMoppVirtualMachine()
{

}

int HK_CALL hkMoppVirtualMachine::toIntMin(hkReal x)
{
	return hkMath::hkToIntFast(x)-1;
}

int HK_CALL hkMoppVirtualMachine::toIntMax(hkReal x)
{
	return hkMath::hkToIntFast(x)+1;
}


inline int HK_CALL hkMoppVirtualMachine::read24( const unsigned char* PC )
{
	return (PC[0]<<16) + (PC[1]<<8) + PC[2];
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
