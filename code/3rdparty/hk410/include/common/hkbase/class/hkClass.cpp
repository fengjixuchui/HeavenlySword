/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkbase/hkBase.h>
#include <hkbase/class/hkClass.h>
#include <hkbase/class/hkClassMember.h>
#include <hkbase/class/hkClassEnum.h>
#include <hkbase/class/hkInternalClassMember.h>
#include <hkbase/memory/hkLocalArray.h>
#include <hkbase/stream/impl/hkCrc32StreamWriter.h>
#include <hkbase/stream/hkOArchive.h>

HK_COMPILE_TIME_ASSERT( sizeof(hkInternalClassMember) == sizeof(hkClassMember) );
HK_COMPILE_TIME_ASSERT( sizeof(hkInternalClassEnum) == sizeof(hkClassEnum) );
HK_COMPILE_TIME_ASSERT( sizeof(hkInternalClassEnumItem) == sizeof(hkClassEnum::Item) );

hkClass::hkClass(const char* className,
				const hkClass* parentClass,
				int objectSizeInBytes,
				const hkClass** implementedInterfaces,
				int numImplementedInterfaces,
				const hkClassEnum* declaredEnums,
				int numDeclaredEnums,
				const hkClassMember* members,
				int numMembers, 
				const void* defaults )
	:	m_name(className),
		m_parent(parentClass),
		m_objectSize(objectSizeInBytes),
		//m_implementedInterfaces(implementedInterfaces),
		m_numImplementedInterfaces(numImplementedInterfaces),
		m_declaredEnums( declaredEnums ),
		m_numDeclaredEnums( numDeclaredEnums ),
		m_declaredMembers(members),
		m_numDeclaredMembers(numMembers),
		m_defaults(defaults)
{
}

const char* hkClass::getName() const
{
	return m_name;
}

const hkClass* hkClass::getParent() const
{
	return m_parent;
}

hkClass* hkClass::getParent()
{
	return const_cast<hkClass*>(m_parent);
}

int hkClass::getInheritanceDepth() const
{
	int depth = 0;
	const hkClass* c = this;
	while( c != HK_NULL )
	{
		++depth;
		c = c->m_parent;
	}
	return depth;
}

hkBool hkClass::isSuperClass(const hkClass& k) const
{
	const hkClass* c = &k;
	while( c )
	{
		if( c == this )
		{
			return true;
		}
		c = c->getParent();
	}
	return false;
}

#define RETURN_SUM_MEMBERS(MEMBER) \
	const hkClass* c = this->m_parent; \
	int RETURN = MEMBER; \
	while( c ) \
	{ \
		RETURN += c->MEMBER; \
		c = c->m_parent; \
	} \
	return RETURN

int hkClass::getNumInterfaces() const
{
	RETURN_SUM_MEMBERS(m_numImplementedInterfaces);
}

const hkClass* hkClass::getInterface( int i ) const
{
	return HK_NULL;
}

const hkClass* hkClass::getDeclaredInterface( int i ) const
{
	return HK_NULL;
}

int hkClass::getNumDeclaredInterfaces() const
{
	return m_numImplementedInterfaces;
}

int hkClass::getNumEnums() const
{
	RETURN_SUM_MEMBERS(m_numDeclaredEnums);
}

const hkClassEnum& hkClass::getEnum(int enumIndex) const
{
	int numEnums = getNumEnums();
	HK_ASSERT(0x275d8b19, enumIndex >= 0 && enumIndex < numEnums );
	const hkClass* c = this;
	int localIndex = enumIndex - numEnums;
	while( c )
	{
		localIndex += c->m_numDeclaredEnums;
		if( localIndex >= 0 )
		{
			return c->m_declaredEnums[localIndex];
		}
		c = c->m_parent;
	}
	HK_ASSERT2(0x1036239f, 0, "notreached");
	return m_declaredEnums[0];
}

const hkClassEnum* hkClass::getEnumByName(const char* name) const
{
	for( int i = 0; i < getNumEnums(); ++i)
	{
		const hkClassEnum& e = getEnum(i);
		if( hkString::strCmp(e.getName(), name) == 0)
		{
			return &e;
		}
	}
	return HK_NULL;
}


int hkClass::getNumMembers() const
{
	RETURN_SUM_MEMBERS(m_numDeclaredMembers);
}

const hkClassMember& hkClass::getMember(int memberIndex) const
{
	int numMembers = getNumMembers();
	HK_ASSERT(0x275d8b19, memberIndex >= 0 && memberIndex < numMembers );
	const hkClass* c = this;
	int localIndex = memberIndex - numMembers;
	while( c )
	{
		localIndex += c->m_numDeclaredMembers;
		if( localIndex >= 0 )
		{
			return c->m_declaredMembers[localIndex];
		}
		c = c->m_parent;
	}
	HK_ASSERT2(0x1036239f, 0, "notreached");
	return m_declaredMembers[0];
}

hkClassMember& hkClass::getMember(int memberIndex)
{
	const hkClass* constThis = this;
	return const_cast<hkClassMember&>( constThis->getMember(memberIndex) );
}

int hkClass::getNumDeclaredMembers() const
{
	return m_numDeclaredMembers;
}

const hkClassMember& hkClass::getDeclaredMember(int i) const
{
	HK_ASSERT(0x39d720db, i>=0 && i < m_numDeclaredMembers);
	return m_declaredMembers[i];
}

const hkClassMember* hkClass::getMemberByName(const char* name) const
{
	for( int i = 0; i < getNumMembers(); ++i)
	{
		const hkClassMember& m = getMember(i);
		if( hkString::strCmp(m.getName(), name) == 0)
		{
			return &m;
		}
	}
	return HK_NULL;
}


int hkClass::getObjectSize() const
{
	return m_objectSize;
}

void hkClass::setObjectSize(int size)
{
	m_objectSize = size;
}

hkBool hkClass::hasVtable() const
{
	const hkClass* c = this;
	while(c->getParent())
	{
		c = c->getParent();
	}
	HK_ON_DEBUG(int v = getNumInterfaces());
	int i = c->m_numImplementedInterfaces;
	HK_ASSERT2(0x279061ac, (i==0 && v==0) || (i!=0 && v!=0), "Vtable is not in base class.");
	return i != 0;
}

hkUint32 hkClass::getSignature(int signatureFlags) const
{
	hkCrc32StreamWriter crc;
	bool recurse = (signatureFlags & SIGNATURE_LOCAL)==0;
	const hkClass* c = this;
	while( c )
	{
		c->writeSignature(&crc);
		c = recurse ? c->getParent() : HK_NULL;
	}
	
	return crc.getCrc32();
}

hkResult hkClass::getDefault(int memberIndex, hkStreamWriter* writer) const
{
	int numMember = getNumMembers();
	HK_ASSERT(0x275d8b19, memberIndex >= 0 && memberIndex < numMember );
	const hkClass* c = this;
	int localIndex = memberIndex - numMember;
	while( c )
	{
		localIndex += c->m_numDeclaredMembers;
		if( localIndex >= 0 )
		{
			if( c->m_defaults )
			{
				int defIndex = reinterpret_cast<const int*>(c->m_defaults)[localIndex];
				if( defIndex >= 0 )
				{
					writer->write( static_cast<const char*>(c->m_defaults)+defIndex, c->m_declaredMembers[localIndex].getSizeInBytes() );
					return HK_SUCCESS;
				}
			}
			break;
		}
		c = c->m_parent;
	}
	return HK_FAILURE;
}



void hkClass::writeSignature( hkStreamWriter* w ) const
{
	hkOArchive oa(w);
	//oa.writeRaw( m_name, hkString::strLen(m_name) );	// don't include name
	//oa.write32(m_objectSize ); // size not needed for cross platform signature.

	int i;
	
	for( i = 0; i < m_numImplementedInterfaces; ++i )
	{
		// crc.write( m_implementedInterfaces[i] );
	}
	oa.write32(m_numImplementedInterfaces);
	
	for( i = 0; i < m_numDeclaredEnums; ++i )
	{
		m_declaredEnums[i].writeSignature( w );
	}
	oa.write32( m_numDeclaredEnums );
	
	for( i = 0; i < m_numDeclaredMembers; ++i )
	{
		const hkClassMember& member = m_declaredMembers[i];
		if( member.hasClass()
			&& member.getType() != hkClassMember::TYPE_POINTER
			&& member.getSubType() != hkClassMember::TYPE_POINTER )
		{
			member.getStructClass().writeSignature( w );
		}
		if( member.getType() == hkClassMember::TYPE_ENUM )
		{
			member.getEnumClass().writeSignature(w);
		}
		const hkInternalClassMember& m = reinterpret_cast<const hkInternalClassMember&>(member);
		oa.writeRaw( m.m_name, hkString::strLen(m.m_name) );
		oa.write16( m.m_type );
		oa.write16( m.m_subtype );
		oa.write16( m.m_cArraySize );
		oa.write16( m.m_flags );
		//oa.write16( m.m_offset ); // offset not needed for cross platform signature.
	}
	oa.write32( m_numDeclaredMembers );
	// don't include defaults in signature.
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
