/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkserialize/hkSerialize.h>
#include <hkserialize/serialize/xml/hkXmlObjectWriter.h>
#include <hkserialize/serialize/hkRelocationInfo.h>

#include <hkbase/class/hkClass.h>
#include <hkbase/class/hkClassEnum.h>
#include <hkbase/stream/hkOArchive.h>
#include <hkbase/stream/hkStreambufFactory.h>
#include <hkbase/stream/hkStreamWriter.h>
#include <hkbase/memory/hkLocalArray.h>

namespace
{
	struct DummyArray
	{
		void* ptr;
		int size;
		int cap;
	};

	struct DummyHomogeneousArray 
	{ 
			const hkClass* klass; 
			void* data; 
			int size; 
			//              int capAndFlags; 
	};

	typedef char XmlName[256];
}

static hkResult writeCstring(const void*const* array, int nelem, hkOstream& os);

int hkXmlObjectWriter::SequentialNameFromAddress::nameFromAddress( const void* addr, char* buf, int bufSize )
{
	int n = m_map.getWithDefault(addr,0);
	if( n == 0 )
	{
		n = m_map.getSize();
		m_map.insert( addr, m_map.getSize()+1 );
	}
	return hkString::snprintf(buf, bufSize, "#%04i", n);
}

hkXmlObjectWriter::hkXmlObjectWriter(NameFromAddress& n)
	: m_nameFromAddress(n)
{
	m_indent.pushBack(0);
	m_indent.popBack();
}

typedef hkArray<char> Indent;

static void adjustIndent(Indent& in, int delta)
{
	HK_ASSERT2(0x766ac26b, in.getSize() + delta >= 0, "XML writer indentation underflowed." );
	in.setSize( in.getSize() + delta + 1, '\t');
	// Keep a terminating null 'past the end'
	in.back() = 0;
	in.popBack();
}
static inline void increaseIndent(Indent& in) { adjustIndent(in, 1); }
static inline void decreaseIndent(Indent& in) { adjustIndent(in,-1); }

template <typename T>
const T& lookupMember(const void* start)
{
	return *reinterpret_cast<const T*>( start );
}

static int getNumElementsInMember( const hkClassMember& member )
{
	int asize = member.getCstyleArraySize();
	return asize ? asize : 1;
}

void hkXmlObjectWriter::beginElement(hkStreamWriter* writer, const char* name, const char*const* attributes, hkBool prefixNewline)
{
	if( prefixNewline )
	{
		writer->write("\n", 1);
		writer->write(m_indent.begin(), m_indent.getSize());
	}
	hkOstream os(writer);
	os.printf("<%s", name );
	if( attributes )
	{
		for( int i = 0; attributes[i] != HK_NULL; i += 2 )
		{
			os.printf(" %s=\"%s\"", attributes[i], attributes[i+1] );
		}		
	}

	os.printf(">");
	adjustIndent(1);
}

void hkXmlObjectWriter::endElement(hkStreamWriter* writer, const char* name, hkBool prefixNewline)
{
	adjustIndent(-1);
	if( prefixNewline )
	{
		writer->write("\n", 1);
		writer->write(m_indent.begin(), m_indent.getSize());
	}
	writer->write( "</", 2 );
	writer->write( name, hkString::strLen(name) );
	writer->write( ">", 1 );
}

// save raw data with base64 encoding.
// see rfc2045 Section 6.8 for a description of the format.
hkResult hkXmlObjectWriter::base64write( hkStreamWriter* sw, const void* buf, int len )
{
	const char* cur = static_cast<const char*>(buf);

	const unsigned char bin2ascii[64] =
	{
		'A','B','C','D','E','F','G','H','I','J','K','L','M',
		'N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
		'a','b','c','d','e','f','g','h','i','j','k','l','m',
		'n','o','p','q','r','s','t','u','v','w','x','y','z',
		'0','1','2','3','4','5','6','7','8','9','+','/'
	};
	hkOArchive oa(sw);

	int blocksTillNewline = 19; // 19*4 = 76 chars per line
	while( len > 0 )
	{
		unsigned char ibuf[3] = { 0 };
		int ibuflen = len >= 3 ? 3 : len;
		hkString::memCpy( ibuf, cur, ibuflen );
		cur += ibuflen;
		len -= ibuflen;
		unsigned char obuf[4];
		obuf[0] = bin2ascii[(ibuf[0] >> 2)];
		obuf[1] = bin2ascii[((ibuf[0] << 4) & 0x30) | (ibuf[1] >> 4 )];
		obuf[2] = bin2ascii[((ibuf[1] << 2) & 0x3c) | (ibuf[2] >> 6 )];
		obuf[3] = bin2ascii[(ibuf[2] & 0x3f)];

		if( ibuflen >= 3 )
		{
			oa.writeRaw(obuf, 4);
		}
		else // need to pad up to 4
		{
			switch( ibuflen )
			{
				case 1:
					obuf[2] = obuf[3] = '=';
					oa.writeRaw(obuf, 4);
					break;
				case 2:
					obuf[3] = '=';
					oa.writeRaw(obuf, 4);
			}
			break;
		}

		if( --blocksTillNewline == 0)
		{
			oa.writeRaw("\n", 1);
			blocksTillNewline = 19;
		}

		if( sw->isOk() == false )
		{
			return HK_FAILURE;
		}
	}
	return HK_SUCCESS;
}



//
// Save packet
//
// When writing, do not append a newline - the object being written decides whether or
// not to begin a newline. This makes formatting much easier.
static void writePodMember(hkClassMember::Type mtype, const void* memberAddress, hkOstream& os, hkXmlObjectWriter& xml);
static void writeArray( Indent& indent, const hkClassMember& member, const void* memberAddress, hkOstream& os, hkXmlObjectWriter& xml );
static void writeMember( Indent& indent, const hkClassMember& member, const void* objectData, hkOstream& os, hkXmlObjectWriter& xml );
static hkResult writeStruct( Indent &indent, const hkClass& klass, const void* objectData, hkOstream& os, hkXmlObjectWriter& xml );

static void writePodMember(hkClassMember::Type mtype, const void* memberAddress, hkOstream& os, hkXmlObjectWriter& xml)
{
	switch(mtype)
	{
		case hkClassMember::TYPE_BOOL:
		{
			hkBool b = lookupMember<hkBool>(memberAddress);
			os << (b ? "true" : "false");
			break;
		}
		case hkClassMember::TYPE_CHAR:
		{
			char c = lookupMember<char>(memberAddress);
			os.printf("%c", c);
			break;
		}
		case hkClassMember::TYPE_INT8:
		{
			int i = lookupMember<hkInt8>(memberAddress);
			os.printf("%i", i);
			break;
		}
		case hkClassMember::TYPE_UINT8:
		{
			unsigned u = lookupMember<hkUint8>(memberAddress);
			os.printf("%u", u);
			break;
		}
		case hkClassMember::TYPE_INT16:
		{
			os.printf("%i", lookupMember<hkInt16>(memberAddress) );
			break;
		}
		case hkClassMember::TYPE_UINT16:
		{
			os.printf("%u", lookupMember<hkUint16>(memberAddress) );
			break;
		}
		case hkClassMember::TYPE_INT32:
		{
			os.printf("%i", lookupMember<hkInt32>(memberAddress) );
			break;
		}
		case hkClassMember::TYPE_UINT32:
		{
			os.printf("%u", lookupMember<hkUint32>(memberAddress) );
			break;
		}
		case hkClassMember::TYPE_INT64:
		{
			os.printf(HK_PRINTF_FORMAT_INT64, lookupMember<hkInt64>(memberAddress) );
			break;
		}
		case hkClassMember::TYPE_UINT64:
		{
			os.printf(HK_PRINTF_FORMAT_UINT64, lookupMember<hkUint64>(memberAddress) );
			break;
		}
		case hkClassMember::TYPE_ULONG:
		{
			os.printf(HK_PRINTF_FORMAT_ULONG, lookupMember<hkUlong>(memberAddress) );
			break;
		}
		case hkClassMember::TYPE_REAL:
		{
			os.printf("%f", lookupMember<hkReal>(memberAddress) );
			break;
		}
		case hkClassMember::TYPE_VECTOR4:
		case hkClassMember::TYPE_QUATERNION:
		{
			const hkReal* r = reinterpret_cast<const hkReal*>( memberAddress );
			os.printf("(%f %f %f %f)", r[0], r[1], r[2], r[3] );
			break;
		}
		case hkClassMember::TYPE_MATRIX3:
		case hkClassMember::TYPE_ROTATION:
		{
			const hkReal* r = reinterpret_cast<const hkReal*>( memberAddress );
			os.printf("(%f %f %f)", r[0], r[1], r[2] );
			os.printf("(%f %f %f)", r[4], r[5], r[6] );
			os.printf("(%f %f %f)", r[8], r[9], r[10] );
			break;
		}
		case hkClassMember::TYPE_QSTRANSFORM:
		{
			const hkReal* r = reinterpret_cast<const hkReal*>( memberAddress );
			os.printf("(%f %f %f)", r[0], r[1], r[2] );
			os.printf("(%f %f %f %f)", r[4], r[5], r[6], r[7] );
			os.printf("(%f %f %f)", r[8], r[9], r[10] );
			break;
		}
		case hkClassMember::TYPE_MATRIX4:
		{
			const hkReal* r = reinterpret_cast<const hkReal*>( memberAddress );
			os.printf("(%f %f %f %f)", r[0], r[1], r[2], r[3] );
			os.printf("(%f %f %f %f)", r[4], r[5], r[6], r[7] );
			os.printf("(%f %f %f %f)", r[8], r[9], r[10], r[11] );
			os.printf("(%f %f %f %f)", r[12], r[13], r[14], r[15] );
			break;
		}
		case hkClassMember::TYPE_TRANSFORM:
		{
			const hkReal* r = reinterpret_cast<const hkReal*>( memberAddress );
			os.printf("(%f %f %f)", r[0], r[1], r[2] );
			os.printf("(%f %f %f)", r[4], r[5], r[6] );
			os.printf("(%f %f %f)", r[8], r[9], r[10] );
			os.printf("(%f %f %f)", r[12], r[13], r[14] );
			break;
		}
		case hkClassMember::TYPE_POINTER:
		{
			XmlName name;
			int len = xml.nameFromAddress( lookupMember<const void*>(memberAddress), name, sizeof(name) );
			os.write(name, len);
			break;
		}
		case hkClassMember::TYPE_CSTRING:
		{
			os.printf("<hkcstring>");
			writeCstring(static_cast<const void*const*>( memberAddress ), 1, os);
			os.printf("</hkcstring>");
			break;
		}
		//TYPE_FUNCTIONPOINTER
		//TYPE_ARRAY
		//TYPE_INPLACEARRAY
		//TYPE_ENUM
		//TYPE_STRUCT
		//TYPE_SIMPLEARRAY
		//TYPE_HOMOGENEOUSARRAY
		//TYPE_VARIANT
		default:
		{
			HK_ASSERT2(0x10cb67f0, 0, "Unhandled type '" << mtype << "'");
		}
	}
}

static void writeArray(Indent& indent, const hkClassMember& member, const void* memberAddress, hkOstream& os, hkXmlObjectWriter& xml )
{
	typedef hkArray<char> array; // As long as we don't reference capacity member this is the same as hkSimpleArray

	int elemsize = member.getArrayMemberSize();
	const array& a = *reinterpret_cast<const array*>( memberAddress );
	const char* p = a.begin();
	hkClassMember::Type elemType = member.getArrayType();

	if( a.getSize() )
	{
		increaseIndent(indent);

		switch(elemType)
		{
			case hkClassMember::TYPE_BOOL:
			case hkClassMember::TYPE_CHAR:
			case hkClassMember::TYPE_INT8:
			case hkClassMember::TYPE_UINT8:
			case hkClassMember::TYPE_INT16:
			case hkClassMember::TYPE_UINT16:
			case hkClassMember::TYPE_INT32:
			case hkClassMember::TYPE_UINT32:
			case hkClassMember::TYPE_INT64:
			case hkClassMember::TYPE_UINT64:
			case hkClassMember::TYPE_ULONG:
			case hkClassMember::TYPE_REAL:
			case hkClassMember::TYPE_VECTOR4:
			case hkClassMember::TYPE_QUATERNION:
			case hkClassMember::TYPE_MATRIX3:
			case hkClassMember::TYPE_ROTATION:
			case hkClassMember::TYPE_QSTRANSFORM:
			case hkClassMember::TYPE_MATRIX4:
			case hkClassMember::TYPE_TRANSFORM:
			case hkClassMember::TYPE_POINTER:
			case hkClassMember::TYPE_CSTRING:
			{
				int elemsPerLine;
				if( elemType >= hkClassMember::TYPE_VECTOR4 && elemType <= hkClassMember::TYPE_TRANSFORM )
				{
					elemsPerLine = 1;
				}
				else
				{
					elemsPerLine = 16;
				}
				for( int i = 0; i < a.getSize(); ++i)
				{
					if( i%elemsPerLine == 0 || elemType == hkClassMember::TYPE_CSTRING)
					{
						os.printf("\n%s", indent.begin());
					}
					else
					{
						os << ' ';
					}
					writePodMember( elemType, p, os, xml );
					p += elemsize;
				}
				break;
			}
			case hkClassMember::TYPE_ZERO:
			{
				os.printf("<!-- zero array %s -->", member.getName());
				break;
			}
			case hkClassMember::TYPE_ARRAY:
			case hkClassMember::TYPE_INPLACEARRAY:
			case hkClassMember::TYPE_ENUM:
			case hkClassMember::TYPE_SIMPLEARRAY:
			case hkClassMember::TYPE_HOMOGENEOUSARRAY:
			{
				HK_ASSERT2(0x29772c5a, 0, "Array of this type not allowed");
				break;
			}

			case hkClassMember::TYPE_STRUCT:
			{
				for( int i = 0; i < a.getSize(); ++i)
				{
					writeStruct( indent, member.getStructClass(), p, os, xml );
					p += elemsize;
				}
				break;
			}
			
			case hkClassMember::TYPE_VARIANT:
			{
				for( int i = 0; i < a.getSize(); ++i)
				{
					const hkVariant& var = lookupMember<hkVariant>(p);
					XmlName oname, cname;
					xml.nameFromAddress(var.m_object, oname, sizeof(oname));
					xml.nameFromAddress(var.m_class, cname, sizeof(cname));
					os.printf("(%s %s%s)", oname, cname, (i+1 < a.getSize() ? " " : "") );
					p += elemsize;
				}
				break;
			}

			default:
			{
				HK_ASSERT2(0x1099d8a5, 0, "Unhandled type '" << elemType << "'");
			}
		}
		decreaseIndent(indent);
		os.printf("\n%s", indent.begin());
	}
}

static hkResult writeStruct(Indent &indent, const hkClass& klass, const void* objectData, hkOstream& os, hkXmlObjectWriter& xml )
{
	os.printf("\n%s<hkobject>", indent.begin());
	increaseIndent(indent);
	for( int i = 0; i < klass.getNumMembers(); ++i )
	{
		writeMember( indent, klass.getMember(i), objectData, os, xml );
	}
	decreaseIndent(indent);
	os.printf("\n%s</hkobject>", indent.begin());
	return HK_SUCCESS;
}

static hkResult writeCstring(const void*const* array, int nelem, hkOstream& os)
{
	for( int elemIndex = 0; elemIndex < nelem; ++elemIndex )
	{
		if( array[elemIndex] != HK_NULL )
		{
			const char* s = static_cast<const char*>(array[elemIndex]); // start of run
			const char* p = s; // current char
			while( *p != 0 ) // scan for entities
			{
				if( *p <= 20 || *p >= 127 )
				{
					os.write(s, int(p-s));
					os.printf("&#%u;", static_cast<unsigned char>(*p) );
					s = p+1;
				}
				else
				{
					switch(*p)
					{
						case '<':
						case '>':
						case '&':
						case '"':
						case '\'':
						{
							os.write(s, int(p-s));
							s = p+1;
							const char* entities[] = { "<&lt;", ">&gt;", "&&amp;", "\"&quot;", "'&apos;", HK_NULL };
							for( int i = 0; entities[i] != HK_NULL; ++i )
							{
								if( entities[i][0] == *p )
								{
									const char* ent = entities[i]+1;
									os.write( ent, hkString::strLen(ent) );
									break;
								}
							}
						}
					}
				}
				++p;
			}
			os.write(s,int(p-s)); // finish
		}
		else
		{
			os << "&#0;";
		}
	}
	return HK_SUCCESS;
}

static void writeMember( Indent& indent, const hkClassMember& member, const void* objectData, hkOstream& os, hkXmlObjectWriter& xml )
{
	const void* memberAddress = ((const char*)objectData) + member.getOffset();

	if( member.getType() == hkClassMember::TYPE_POINTER && member.getSubType() == hkClassMember::TYPE_CHAR && *(const void*const*)memberAddress == HK_NULL )
	{
		os.printf("\n%s<!-- <hkparam name=\"%s\">(null)</hkparam> -->", indent.begin(), member.getName() );
		return;
	}

	os.printf("\n%s<hkparam name=\"%s\"", indent.begin(), member.getName() );
	switch( member.getType() )
	{
		case hkClassMember::TYPE_ARRAY:
		case hkClassMember::TYPE_INPLACEARRAY:
		case hkClassMember::TYPE_SIMPLEARRAY:
		{
			os.printf(" numelements=\"%i\"", static_cast<const DummyArray*>(memberAddress)->size );
			break;
		}
		case hkClassMember::TYPE_HOMOGENEOUSARRAY: 
		{ 
			os.printf(" numelements=\"%i\"", static_cast<const DummyHomogeneousArray*>(memberAddress)->size ); 
			break;
		} 
		default:
		{
			// not an array
		}
	}
	os.printf(">");

	switch( member.getType() )
	{
		case hkClassMember::TYPE_BOOL:
		case hkClassMember::TYPE_CHAR:
		case hkClassMember::TYPE_INT8:
		case hkClassMember::TYPE_UINT8:
		case hkClassMember::TYPE_INT16:
		case hkClassMember::TYPE_UINT16:
		case hkClassMember::TYPE_INT32:
		case hkClassMember::TYPE_UINT32:
		case hkClassMember::TYPE_INT64:
		case hkClassMember::TYPE_UINT64:
		case hkClassMember::TYPE_ULONG:
		case hkClassMember::TYPE_REAL:
		case hkClassMember::TYPE_VECTOR4:
		case hkClassMember::TYPE_QUATERNION:
		case hkClassMember::TYPE_MATRIX3:
		case hkClassMember::TYPE_ROTATION:
		case hkClassMember::TYPE_QSTRANSFORM:
		case hkClassMember::TYPE_MATRIX4:
		case hkClassMember::TYPE_TRANSFORM:
		{
			int numElement = getNumElementsInMember( member );
			int elemsize = member.getSizeInBytes() / numElement;
			const char* p = static_cast<const char*>( memberAddress );
			for( int i = 0; i < numElement; ++i )
			{
				if( member.getType() == hkClassMember::TYPE_CHAR )
				{
					os.printf("%c", *p);
				}

				else
				{
					if( i )
					{
						os.printf( (i % 50 == 0) ? "\n" : " ");
					}

					writePodMember(member.getType(), p, os, xml);
				}

				p += elemsize;
			}
			break;
		}

		case hkClassMember::TYPE_ZERO:
		{
			os.printf("<!-- zero %s -->", member.getName());
			break;
		}

		case hkClassMember::TYPE_CSTRING:
		{
			int nelem = getNumElementsInMember( member );
			const void*const* array = static_cast<const void*const*>( memberAddress );
			writeCstring(array, nelem, os);
			break;
		}
		case hkClassMember::TYPE_POINTER:
		{
			HK_ASSERT2(0x4ca4f06d, (member.getSubType() != hkClassMember::TYPE_CSTRING && member.getSubType() != hkClassMember::TYPE_POINTER), "The pointer to c-string and general pointers are unsupported.");
			int nelem = getNumElementsInMember( member );
			const void*const* array = static_cast<const void*const*>( memberAddress );
			if( member.getSubType() == hkClassMember::TYPE_CHAR )
			{
				writeCstring(array, nelem, os);
			}
			else
			{
				for( int elemIndex = 0; elemIndex < nelem; ++elemIndex )
				{
					if( array[elemIndex] != HK_NULL )
					{
						XmlName id;
						xml.nameFromAddress(array[elemIndex], id, sizeof(id));
						os.printf("%s", id);
					}
					else
					{
						os << "null";
					}

					// If there are more elements left in the list, print
					// a space before printing the next one.
					if (elemIndex < nelem-1)
					{
						os << " ";
					}

				}
			}
			break;
		}

		case hkClassMember::TYPE_FUNCTIONPOINTER:
		{
			int nelem = getNumElementsInMember( member );
			for( int i = 0; i < nelem; ++i )
			{
				os << "&null;";
			}
			break;
		}

		case hkClassMember::TYPE_ARRAY:
		case hkClassMember::TYPE_INPLACEARRAY:
		case hkClassMember::TYPE_SIMPLEARRAY:
		{
			writeArray( indent, member, memberAddress, os, xml );
			break;
		}

		case hkClassMember::TYPE_ENUM:
		{
			const hkClassEnum& e = member.getEnumType();
			int value = member.getEnumValue(memberAddress);
			const char* valueName = HK_NULL;
			if( e.getNameOfValue( value, &valueName) == HK_SUCCESS )
			{
				os.printf(valueName);
			}
			else
			{
				os.printf("INVALID_VALUE_%i", value);
			}
			break;
		}

		case hkClassMember::TYPE_STRUCT:
		{
			increaseIndent(indent);
			const hkClass& sclass = member.getStructClass();
			int numStruct = getNumElementsInMember( member );
			const char* p = static_cast<const char*>( memberAddress );
			int elemsize = sclass.getObjectSize();
			for( int i = 0; i < numStruct; ++i )
			{
				writeStruct( indent, sclass, p, os, xml );
				p += elemsize;
			}
			decreaseIndent(indent);
			os << '\n' << indent.begin();
			break;
		}

		case hkClassMember::TYPE_HOMOGENEOUSARRAY:
		{
			const hkClass* arrayMemberClass = *reinterpret_cast<const hkClass* const*>(memberAddress);

			increaseIndent(indent);

			os.printf("\n%s<!-- Homogeneous Class -->", indent.begin());

			const char* attributes[5] = {0};
			attributes[0] = "name";
			attributes[1] = arrayMemberClass->getName();
			attributes[2] = "class";
			attributes[3] = hkClassClass.getName();
			// Write out the class
			xml.beginElement( os.getStreamWriter(), "hkobject", attributes );
			for( int memberIndex = 0; memberIndex < hkClassClass.getNumMembers(); ++memberIndex )
			{
				writeMember( indent, hkClassClass.getMember(memberIndex), arrayMemberClass, os, xml);
			}
			xml.endElement( os.getStreamWriter(), "hkobject" );

			os.printf("\n%s<!-- Homogeneous Data -->", indent.begin());

			typedef hkArray<char> array;
			const array& a = *reinterpret_cast<const array*>( (const char*)memberAddress + sizeof(hkClass*)); // TODO - better cast?

			int elemsize = (arrayMemberClass)->getObjectSize();
			const char* p = a.begin();

			for( int arrayIndex = 0; arrayIndex < a.getSize(); ++arrayIndex)
			{
				writeStruct( indent, *arrayMemberClass, p, os, xml );
				p += elemsize;
			}
			decreaseIndent(indent);
			break;
		}

		case hkClassMember::TYPE_VARIANT:
		{
			int nelem = getNumElementsInMember( member );
			const hkVariant* vars = reinterpret_cast<const hkVariant*>(memberAddress);
			for( int i = 0; i < nelem; ++i )
			{
				const hkVariant& var = vars[i];
				if (var.m_object && var.m_class)
				{
					XmlName oname, cname;
					xml.nameFromAddress(var.m_object, oname, sizeof(oname));
					xml.nameFromAddress(var.m_class, cname, sizeof(cname));
					os.printf("(%s %s%s)", oname, cname, (i+1 < nelem ? " " : "") );
				}
			}
			break;
		}

		default:
		{
			HK_ERROR(0x40a18b57, "Unhandled member type found!");
		}
	}
	os.printf("</hkparam>");
}

hkResult hkXmlObjectWriter::writeObjectWithElement( hkStreamWriter* writer, const void* objectData, const hkClass& klass, const char* objectName, const char*const* attributes )
{
	int numAttributes = 0;
	if( attributes != HK_NULL )
	{
		for( ; attributes[numAttributes] != HK_NULL; ++numAttributes )	{ }
	}
	hkArray<const char*> attr;
	attr.reserve(4 + numAttributes + 1);
	if( objectName != HK_NULL )
	{
		attr.pushBackUnchecked("name");
		attr.pushBackUnchecked(objectName);
	}
	attr.pushBackUnchecked("class");
	attr.pushBackUnchecked(klass.getName());
	if( attributes != HK_NULL )
	{
		hkString::memCpy( attr.expandByUnchecked(numAttributes), attributes, numAttributes*sizeof(char*) );
	}
	attr.pushBackUnchecked(HK_NULL);

	beginElement( writer, "hkobject", attr.begin() );
	hkRelocationInfo reloc;
	writeObject( writer, objectData, klass, reloc );
	endElement(writer, "hkobject");

	return writer->isOk() ? HK_SUCCESS : HK_FAILURE;
}

hkResult hkXmlObjectWriter::writeObject( hkStreamWriter* writer, const void* objectData, const hkClass& klass, hkRelocationInfo& reloc )
{
	hkOstream os( writer);

	for( int i = 0; i < klass.getNumMembers(); ++i )
	{
		writeMember( m_indent, klass.getMember(i), objectData, os, *this);
	}

	return os.isOk() ? HK_SUCCESS : HK_FAILURE;
}


hkResult hkXmlObjectWriter::writeRaw(hkStreamWriter* writer, const void* buf, int len )
{
	hkOstream os(writer);

	os.printf("\n%s<hkrawdata size=\"%i\"><![CDATA[\n",	m_indent.begin(), len );
	hkResult result =  base64write( writer, buf, len );
	os.printf("\n]]></hkrawdata>");

	return result;
}

void hkXmlObjectWriter::adjustIndent( int delta )
{
	::adjustIndent( m_indent, delta );
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
