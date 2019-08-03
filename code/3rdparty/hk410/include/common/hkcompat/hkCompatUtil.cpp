/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkcompat/hkCompat.h>

#include <hkcompat/hkCompatUtil.h>

typedef hkPointerMap<const hkClass*, hkInt32> UpdatedClassesMap;

static void convertPointerCharToCString( hkClassMember& klassMem )
{
	if (klassMem.getType() == hkClassMember::TYPE_POINTER
		&& klassMem.getSubType() == hkClassMember::TYPE_CHAR)
	{
		klassMem.setType(hkClassMember::TYPE_CSTRING);
		klassMem.setSubType(hkClassMember::TYPE_VOID);
	}
	/*
	//
	// this code is just for demonstration purposes only
	// as an example of how to identify old way of
	// c-strings array declaration
	// SIMPLEARRAY / POINTER -> special "STRUCT"
	// with one 'char*' member POINTER / CHAR
	//
	// The new way to declare c-strings array is
	// SIMPLEARRAY / CSTRING
	// no need to specify a dummy structure anymore,
	// just use 'char**' member inplace
	//
	else if (klassMem.getType() == hkClassMember::TYPE_SIMPLEARRAY
			&& klassMem.getSubType() == hkClassMember::TYPE_POINTER
			&& klassMem.hasClass() == true)
	{
		const hkClass& stringStruct = klassMem.getStructClass();
		if (stringStruct.getNumDeclaredMembers() == 1
			&& stringStruct.getParent() == HK_NULL
			&& stringStruct.hasVtable() == false
			&& stringStruct.getDeclaredMember(0).hasClass() == false
			&& ( stringStruct.getDeclaredMember(0).getType() == hkClassMember::TYPE_POINTER
				 && stringStruct.getDeclaredMember(0).getSubType() == hkClassMember::TYPE_CHAR
				|| stringStruct.getDeclaredMember(0).getType() == hkClassMember::TYPE_CSTRING
				   && stringStruct.getDeclaredMember(0).getSubType() == hkClassMember::TYPE_VOID))
		{
			const hkClassMember& structMem = stringStruct.getDeclaredMember(0);

			HK_ASSERT(0, (structMem.getType() == hkClassMember::TYPE_POINTER
							&& structMem.getSubType() == hkClassMember::TYPE_CHAR) );

			HK_ASSERT(0, (structMem.getType() == hkClassMember::TYPE_CSTRING
							&& structMem.getSubType() == hkClassMember::TYPE_VOID) );

			klassMem.setSubType(hkClassMember::TYPE_CSTRING);
		}
	}
	*/
}

static void convertPointerCharToCString( hkClass& klass, UpdatedClassesMap& updatedClasses )
{
	if( updatedClasses.hasKey( &klass ) == false )
	{
		updatedClasses.insert( &klass, 1 );

		for (int i = 0; i < klass.getNumDeclaredMembers(); i++)
		{
			const hkClassMember& klassMem = klass.getDeclaredMember(i);
			if (klassMem.hasClass())
			{
				convertPointerCharToCString( *const_cast<hkClass*>( &klassMem.getStructClass() ), updatedClasses );
			}
			convertPointerCharToCString( *const_cast<hkClassMember*>( &klassMem ) );
		}
		
		if (klass.getParent() != HK_NULL)
		{
			convertPointerCharToCString( *klass.getParent(), updatedClasses );
		}
	}
}

void hkCompatUtil::convertPointerCharToCString( hkArray<hkVariant>& objectsInOut )
{
	UpdatedClassesMap updatedClasses;
	for( int i = 0; i < objectsInOut.getSize(); ++i )
	{
		const hkClass* klass = objectsInOut[i].m_class;
		convertPointerCharToCString( *const_cast<hkClass*>( klass ), updatedClasses );
	}
}

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
