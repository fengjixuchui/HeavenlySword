/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkserialize/hkSerialize.h>
#include <hkserialize/serialize/xml/hkXmlObjectReader.h>
#include <hkserialize/serialize/hkRelocationInfo.h>
#include <hkserialize/util/xml/hkXmlParser.h>
#include <hkbase/stream/hkIstream.h>
#include <hkbase/stream/hkStreamReader.h>
#include <hkbase/memory/hkLocalArray.h>
#include <hkbase/class/hkClassEnum.h>
#include <hkbase/htl/hkTree.h>

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

	template <typename T>
	T& lookupMember(void* start)
	{
		return *reinterpret_cast<T*>( start );
	}

	struct Buffer
	{
		public:

			enum Pad
			{
				PAD_NONE = 1,
				PAD_4 = 4,
				PAD_8 = 8,
				PAD_16 = 16
			};

			Buffer(hkArray<char>& c)
				:	m_buf(c)
			{
			}

				// Reserve space for nbytes - fill with zeros
				// Return offset of writable space
			int reserve(int nbytes, Pad pad = PAD_NONE)
			{
				int orig = m_buf.getSize();
				int size = HK_NEXT_MULTIPLE_OF(pad, orig + nbytes);
				m_buf.setSize( size, 0 );
				m_buf.setSizeUnchecked( orig );
				return orig;
			}

				// Advance by nbytes - should never cause reallocation
				// because we should have previously reserve()d space.
			void advance( int nbytes, Pad pad = PAD_NONE )
			{
				int size = HK_NEXT_MULTIPLE_OF(pad, m_buf.getSize() + nbytes);
				HK_ASSERT2(0x12402f4c, size <= m_buf.getCapacity(), "Overflowing XML write buffer capacity, will cause resize." );
				m_buf.setSizeUnchecked( size );
			}
			
			void* pointerAt( int offset )
			{
				HK_ASSERT2(0x6c591289, offset <= m_buf.getSize(), "Offset of pointer not within XML buffer range." );
				return m_buf.begin() + offset;
			}

		private:

			hkArray<char>& m_buf;
	};

	void* addByteOffset(void* p, int n)
	{
		return static_cast<char*>(p) + n;
	}
}


hkXmlObjectReader::hkXmlObjectReader(hkXmlParser* parser)
	: m_parser(parser)
{
	if(parser)
	{
		m_parser->addReference();
	}
	else
	{
		m_parser = new hkXmlParser();
	}
}

hkXmlObjectReader::~hkXmlObjectReader()
{
	m_parser->removeReference();
}

static hkResult extractCstring(int memberStartOffset, const hkString& text, Buffer& buffer, hkRelocationInfo& reloc);

static inline hkBool isSpace(int c)
{
	return (c==' ') || (c =='\t') || (c=='\n') || (c=='\r');
}

static int getNumElementsInMember( const hkClassMember& member )
{
	int asize = member.getCstyleArraySize();
	return asize ? asize : 1;
}

// read a member into the location curp.
// The input data is available as an istream.
// Arrays are temporarily stored in array* for later processing.
static hkResult readSinglePodMember(hkClassMember::Type mtype, void* curp, hkIstream& is )
{
	switch( mtype ) // match order with order of hktypes
	{
		case hkClassMember::TYPE_BOOL:
		{
			hkBool* f = static_cast<hkBool*>(curp);
			is >> *f;
			break;
		}
		case hkClassMember::TYPE_CHAR:
		{
			char* f = static_cast<char*>(curp);
			is.read(f, 1);
			break;
		}
		case hkClassMember::TYPE_INT8:
		{
			hkInt8* f = static_cast<hkInt8*>(curp);
			int foo;
			is >> foo;
			*f = static_cast<hkInt8>(foo);
			break;
		}
		case hkClassMember::TYPE_UINT8:
		{
			hkUint8* f = static_cast<hkUint8*>(curp);
			int foo;
			is >> foo;
			*f = static_cast<hkUint8>(foo);
			break;
		}
		case hkClassMember::TYPE_INT16:
		{
			hkInt16* f = static_cast<hkInt16*>(curp);
			is >> *f;
			break;
		}
		case hkClassMember::TYPE_UINT16:
		{
			hkUint16* f = static_cast<hkUint16*>(curp);
			is >> *f;
			break;
		}
		case hkClassMember::TYPE_INT32:
		{
			hkInt32* f = static_cast<hkInt32*>(curp);
			is >> *f;
			break;
		}
		case hkClassMember::TYPE_UINT32:
		{
			hkUint32* f = static_cast<hkUint32*>(curp);
			is >> *f;
			break;
		}
		case hkClassMember::TYPE_INT64:
		{
			hkInt64* f = static_cast<hkInt64*>(curp);
			is >> *f;
			break;
		}
		case hkClassMember::TYPE_UINT64:
		{
			hkUint64* f = static_cast<hkUint64*>(curp);
			is >> *f;
			break;
		}
		case hkClassMember::TYPE_ULONG:
		{
			hkUint64 tmpStorage;
            is >> tmpStorage;
			*static_cast<hkUlong*>(curp) = hkUlong(tmpStorage);
			break;
		}
		case hkClassMember::TYPE_REAL:
		{
			hkReal* f = static_cast<hkReal*>(curp);
			is >> *f;
			break;
		}
		case hkClassMember::TYPE_VECTOR4:
		case hkClassMember::TYPE_QUATERNION:
		{
			hkReal* f = static_cast<hkReal*>(curp);
			is >> f[0] >> f[1] >> f[2] >> f[3];
			break;
		}
		case hkClassMember::TYPE_MATRIX3:
		case hkClassMember::TYPE_ROTATION:
		{
			hkReal* f = static_cast<hkReal*>(curp);
			hkString::memSet(f, 0, 12*sizeof(hkReal));
			is >> f[0] >> f[1] >> f[2];
			is >> f[4] >> f[5] >> f[6];
			is >> f[8] >> f[9] >> f[10];
			break;
		}
		case hkClassMember::TYPE_QSTRANSFORM:
		{
			hkReal* f = static_cast<hkReal*>(curp);
			is >> f[0] >> f[1] >> f[2];
			is >> f[4] >> f[5] >> f[6] >> f[7];
			is >> f[8] >> f[9] >> f[10];
			f[3] = f[11] = 0;
			break;
		}
		case hkClassMember::TYPE_MATRIX4:
		{
			hkReal* f = static_cast<hkReal*>(curp);
			is >> f[0] >> f[1] >> f[2] >> f[3];
			is >> f[4] >> f[5] >> f[6] >> f[7];
			is >> f[8] >> f[9] >> f[10] >> f[11];
			is >> f[12] >> f[13] >> f[14] >> f[15];
			break;
		}
		case hkClassMember::TYPE_TRANSFORM:
		{
			hkReal* f = static_cast<hkReal*>(curp);
			is >> f[0] >> f[1] >> f[2];
			is >> f[4] >> f[5] >> f[6] ;
			is >> f[8] >> f[9] >> f[10];
			is >> f[12] >> f[13] >> f[14];
			f[3] = f[7] = f[11] = 0;
			f[15] = 1;
			break;
		}
		default:
		{
			HK_ERROR(0x19fca9ad, "Class member unknown / unhandled: " << mtype );
		}
	}
	return is.isOk() ? HK_SUCCESS : HK_FAILURE;
}

static hkString extractText(hkXmlParser& parser, hkStreamReader* reader, hkXmlParser::StartElement* start, bool canonicalize)
{
	hkString ret;

	hkXmlParser::Node* node = HK_NULL;
	parser.nextNode(&node, reader);
	if( hkXmlParser::Characters* chars = node->asCharacters() )
	{
		if( canonicalize )
		{
			chars->canonicalize("(),");
		}
		ret = chars->text;
		delete node;
	}
	else if( node->asEnd() )
	{
		// empty characters ok
		parser.putBack(node);
	}
	else
	{
		HK_ASSERT2(0x6cc04e1a, 0, "Parse error, expected characters after " << start->name );
	}
		
	return ret;
}

static hkResult consumeEndElement(hkXmlParser& parser, hkStreamReader* reader, hkXmlParser::StartElement* start)
{
	hkResult retValue = HK_SUCCESS;
	hkXmlParser::Node* node = HK_NULL;
	parser.nextNode(&node, reader);
	hkXmlParser::EndElement* end = node->asEnd();
	if( end == HK_NULL || end->name != start->name )
	{
		HK_ASSERT2(0x4cadab61, end != HK_NULL, "Parse error, expected end element for " << start->name );
		HK_ASSERT2(0x2c0e93cf, end->name == start->name, "Mismatched end element for " << start->name );
		retValue = HK_FAILURE;
	}
	delete node;
	return retValue;
}

static int loadSimpleArray(const hkClassMember& member, const hkString& text, Buffer& buffer, hkRelocationInfo& reloc)
{
	int numElements = 0;
	hkClassMember::Type mtype = member.getArrayType();
	int msize = member.getArrayMemberSize();
	switch( mtype )
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
			hkIstream istream( text.cString(), text.getLength() );
			int off = buffer.reserve( msize );
			while( readSinglePodMember(mtype, buffer.pointerAt(off), istream) == HK_SUCCESS )
			{
				numElements += 1;
				buffer.advance( msize );
				off = buffer.reserve( msize );
			}
			break;
		}
		case hkClassMember::TYPE_ZERO:
		{
			break;
		}
		case hkClassMember::TYPE_POINTER:
		case hkClassMember::TYPE_FUNCTIONPOINTER:
		{
			int s = 0;
			for( int i = 0; i < text.getLength(); ++i )
			{
				if( isSpace(text[i]) )
				{
					if( s != i )
					{
						hkLocalArray<char> id( i - s + 1 );
						hkString::memCpy( id.begin(), text.cString()+s, i-s );
						id.setSize(i-s+1);
						id[i-s] = 0;
						int off = buffer.reserve( sizeof(void*) );
						reloc.addImport( off, id.begin() );
						buffer.advance( sizeof(void*) );
						numElements += 1;
					}
					s = i+1;
					continue;
				}
			}
			if( s != text.getLength() )
			{
				int off = buffer.reserve( sizeof(void*) );
				reloc.addImport( off, text.cString()+s );
				buffer.advance( sizeof(void*) );
				numElements += 1;
			}
			break;
		}
		case hkClassMember::TYPE_STRUCT:
		case hkClassMember::TYPE_VARIANT:
		case hkClassMember::TYPE_ARRAY:
		case hkClassMember::TYPE_INPLACEARRAY:
		case hkClassMember::TYPE_ENUM:
		case hkClassMember::TYPE_SIMPLEARRAY:
		case hkClassMember::TYPE_HOMOGENEOUSARRAY:
		default:
		{
			// these aren't simple types
			HK_ASSERT2(0x6cc0400a, 0, "Load simple array called on a member that is not a simple array." );
		}
	}
	buffer.reserve(0, Buffer::PAD_16);
	buffer.advance(0, Buffer::PAD_16);
	
	return numElements;
}

static void destroyXmlNode(void* p)
{
	hkXmlParser::Node* n = *static_cast<hkXmlParser::Node**>(p);
	delete n;
}

static hkResult extractCstring(int memberStartOffset, const hkString& text, Buffer& buffer, hkRelocationInfo& reloc)
{
	if( text.getLength() != 1 || text[0] != 0 ) // null pointer is represented as single null
	{
		int len = text.getLength() + 1; // include null
		int textOffset = buffer.reserve( len, Buffer::PAD_16 );
		hkString::memCpy( buffer.pointerAt(textOffset), text.cString(), len );
		reloc.addLocal( memberStartOffset, textOffset );
		buffer.advance( len, Buffer::PAD_16 );
		return HK_SUCCESS;
	}
	return HK_FAILURE;
}

static hkResult readClassBody(
	const hkClass& klass,
	int classStartOffset,
	Buffer& buffer,
	hkXmlParser::StartElement* topElement,
	hkXmlParser& parser,
	hkStreamReader* reader,
	hkRelocationInfo& reloc )
{
	if( klass.hasVtable() )
	{
		reloc.addFinish( classStartOffset, klass.getName() );
	}
	hkXmlParser::Node* node;

	while( parser.nextNode(&node, reader) == HK_SUCCESS )
	{
		if( hkXmlParser::StartElement* startElement = node->asStart() )
		{
			HK_ASSERT2(0x3acc8f13, startElement->name == "hkparam", "XML element starts with " << startElement->name << ", not with the expected 'hkparam'." );
			const char* paramName  = startElement->getAttribute("name",HK_NULL);
			HK_ASSERT2(0x3bbb5581, paramName != HK_NULL, "XML element missing 'name' attribute." );

			const hkClassMember* member = klass.getMemberByName(paramName);
 			if( member == HK_NULL )
			{
				HK_WARN(0x28cd8bfc, "Unknown member '"<<klass.getName()<<"::"<<paramName<<"'. Ignoring it.");
				hkTree<hkXmlParser::Node*> tree(destroyXmlNode);
				parser.expandNode(startElement, tree, reader);
				// tree destructor deletes xml nodes.
				continue;
			}
			
			switch( member->getType() )
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
					hkString text = extractText(parser, reader, startElement, true);
					hkIstream iss( text.cString(), text.getLength() );
					void* memberAddress = buffer.pointerAt( classStartOffset + member->getOffset() );
					int numElem = getNumElementsInMember(*member);
					int elemSize = member->getSizeInBytes() / numElem;
					for( int i = 0; i < numElem; ++i )
					{
						readSinglePodMember( member->getType(), memberAddress, iss );
						memberAddress = addByteOffset(memberAddress, elemSize);
					}
					break;
				}
				case hkClassMember::TYPE_ZERO:
				{
					break; // auto zeroed anyway
				}
				case hkClassMember::TYPE_CSTRING:
				{
					hkString text = extractText(parser, reader, startElement, false);
					extractCstring(classStartOffset + member->getOffset(), text, buffer, reloc);
					break;
				}
				case hkClassMember::TYPE_POINTER:
				case hkClassMember::TYPE_FUNCTIONPOINTER:
				{
					if( member->getType() == hkClassMember::TYPE_POINTER && member->getSubType() == hkClassMember::TYPE_CHAR )
					{
						hkString text = extractText(parser, reader, startElement, false);
						extractCstring(classStartOffset + member->getOffset(), text, buffer, reloc);
					}
					else
					{
						hkString text = extractText(parser, reader, startElement, true);
						int curElement = 0;
						int maxElements = getNumElementsInMember(*member);
						int elemSize = member->getSizeInBytes() / maxElements;
						int memberOffset = classStartOffset + member->getOffset();
						int s = 0;
						for( int i = 0; curElement < maxElements && i < text.getLength(); ++i )
						{
							if( isSpace(text[i]) )
							{
								if( s != i )
								{
									hkLocalArray<char> id( i - s + 1 );
									hkString::memCpy( id.begin(), text.cString()+s, i-s );
									id.setSize(i-s+1);
									id[i-s] = 0;
									reloc.addImport( memberOffset + curElement * elemSize, id.begin() );
									++curElement;
								}
								s = i+1;
								continue;
							}
						}
						if( s != text.getLength() )
						{
							if( curElement < maxElements )
							{
								reloc.addImport( memberOffset + curElement * elemSize, text.cString()+s );
								++curElement;
							}
							else
							{
								HK_WARN(0x7635e7cf, "Too many initializers for " << klass.getName() << "::" << member->getName() );
							}
						}
					}
					break;
				}
				case hkClassMember::TYPE_ARRAY:
				case hkClassMember::TYPE_INPLACEARRAY:
				case hkClassMember::TYPE_SIMPLEARRAY:
				{
					int numElements = -1;
					int arrayBeginOffset = buffer.reserve(0);

					if( member->getSubType() == hkClassMember::TYPE_STRUCT )
					{
						const char* numElementsString = startElement->getAttribute("numelements",HK_NULL);
						HK_ASSERT2(0x3cbc5582, numElementsString != 0, "Could not find 'numelements' attribute in an array of structs.");
						numElements = hkString::atoi( numElementsString );
						const hkClass& sclass = member->getStructClass();
						int ssize = sclass.getObjectSize();
						buffer.reserve( ssize * numElements, Buffer::PAD_16 );
						buffer.advance( ssize * numElements, Buffer::PAD_16 );
						for( int i = 0; i < numElements; ++i )
						{
							hkXmlParser::Node* snode = HK_NULL;
							parser.nextNode(&snode, reader);
							readClassBody( sclass, arrayBeginOffset + i*ssize, buffer,
								snode->asStart(), parser, reader, reloc );
							delete snode;
						}
						// maybe peek and assert next is </hkparam>
					}
					else if( member->getSubType() == hkClassMember::TYPE_ENUM )
					{
						HK_ASSERT2(0x3cbc5583,0, "Arrays of enums not supported in XML reader yet.");
						//XXX fixme
					}
					else if (member->getArrayType() == hkClassMember::TYPE_CSTRING)
					{
						const char* numElementsString = startElement->getAttribute("numelements",HK_NULL);
						HK_ASSERT2(0x3cbc5582, numElementsString != 0, "Could not find 'numelements' attribute in an array of c-strings.");
						numElements = hkString::atoi( numElementsString );
						buffer.reserve( sizeof(char*) * numElements, Buffer::PAD_16 );
						buffer.advance( sizeof(char*) * numElements, Buffer::PAD_16 );
						for( int i = 0; i < numElements; ++i )
						{
							hkXmlParser::Node* snode = HK_NULL;
							parser.nextNode(&snode, reader);
							HK_ASSERT2(0x3acc8f13, snode->asStart() && snode->asStart()->name == "hkcstring", "Expected <hkcstring>" );

							hkString text = extractText(parser, reader, startElement, false);
							extractCstring(arrayBeginOffset + i*sizeof(char*), text, buffer, reloc);
							// skip to next hkcstring
							delete snode;
							parser.nextNode(&snode, reader);
							HK_ASSERT2(0x3acc8f13, snode->asEnd() && snode->asEnd()->name == "hkcstring", "Expected </hkcstring>" );
							delete snode;
						}
					}
					else
					{
						bool canonicalize = member->getSubType() != hkClassMember::TYPE_STRUCT
							&& member->getSubType() != hkClassMember::TYPE_VARIANT;
						hkString text = extractText(parser, reader, startElement, canonicalize);
						numElements = loadSimpleArray(*member, text, buffer, reloc );
					}
					
					DummyArray& dummy = lookupMember<DummyArray>( buffer.pointerAt(classStartOffset + member->getOffset()) );
					dummy.ptr = HK_NULL;
					dummy.size = numElements;
					
					if( numElements > 0 )
					{
						reloc.addLocal( classStartOffset + member->getOffset(), arrayBeginOffset );
					}

					if( member->getType() != hkClassMember::TYPE_SIMPLEARRAY )
					{
						dummy.cap = numElements | hkArray<char>::DONT_DEALLOCATE_FLAG | hkArray<char>::LOCKED_FLAG;
					}
					break;
				}
				case hkClassMember::TYPE_ENUM:
				{
					hkString text = extractText(parser, reader, startElement, false);
					HK_ASSERT2(0x59318e1b, text.getLength(), "Parse error, expected characters after " << startElement->name );
					const hkClassEnum& cenum = member->getEnumType();
					
					int val = 0;
					if( cenum.getValueOfName( text.cString(), &val) != HK_SUCCESS )
					{
						HK_WARN(0x555b54ab, "Invalid enum string '" << text.cString() << "' found for '"
							<< cenum.getName() << "' in member '" << member->getName() );	
					}
					member->setEnumValue( buffer.pointerAt(classStartOffset + member->getOffset()), val);
					break;
				}
				case hkClassMember::TYPE_STRUCT:
				{
					for( int i = 0; i < getNumElementsInMember(*member); ++i )
					{
						hkXmlParser::Node* snode = HK_NULL;
						parser.nextNode(&snode, reader);
						hkXmlParser::StartElement* structStart = snode->asStart();
						HK_ASSERT2(0x48b01b1e, structStart != HK_NULL && (structStart->name == "struct" || structStart->name == "hkobject"),
							"Parse error, expected <struct> or <hkobject> after " << startElement->name );
						int structOffset = classStartOffset + member->getOffset() + i * member->getStructClass().getObjectSize();
						readClassBody( member->getStructClass(), structOffset,
							buffer, structStart, parser, reader, reloc );
						delete snode;
					}
					break;
				}
				case hkClassMember::TYPE_HOMOGENEOUSARRAY:
				{
					int numElements = -1;
					const char* numElementsString = startElement->getAttribute("numelements",HK_NULL);
					HK_ASSERT2(0x3cbc5582, numElementsString != 0, "Could not find 'numelements' attribute in homogenous array.");
					numElements = hkString::atoi( numElementsString );

					// Load the embedded class
					hkLocalArray<char> localStorage(1024);
					Buffer localBuffer( localStorage );
					int classOffset = -1;
					{
						// Grab next node
						hkXmlParser::Node* snode = HK_NULL;
						parser.nextNode(&snode, reader);
						hkXmlParser::StartElement* classStart = snode->asStart();

						// Load in data as hkClass
						hkRelocationInfo localReloc;
						localBuffer.reserve(hkClassClass.getObjectSize(), Buffer::PAD_16);
						localBuffer.advance(hkClassClass.getObjectSize(), Buffer::PAD_16);
						HK_ON_DEBUG(hkResult result = )readClassBody( hkClassClass, 0, localBuffer, classStart, parser, reader, localReloc );
						HK_ASSERT2(0x43e32345, result == HK_SUCCESS, "Unable to read embedded class for homogenous array");

						// Copy class to main buffer 
						classOffset = buffer.reserve( localStorage.getSize(), Buffer::PAD_16 );
						buffer.advance( localStorage.getSize(), Buffer::PAD_16 );
						hkString::memCpy( buffer.pointerAt(classOffset), localBuffer.pointerAt(0), localStorage.getSize() );

						// Copy relocs
						{
							int i;
							for (i=0; i < localReloc.m_local.getSize() ;i++)
							{
								hkRelocationInfo::Local& local = localReloc.m_local[i];
								reloc.addLocal( local.m_fromOffset + classOffset, local.m_toOffset + classOffset );
							}

							for (i=0; i < localReloc.m_global.getSize() ;i++)
							{
								hkRelocationInfo::Global& global = localReloc.m_global[i];
								void * addr = global.m_toAddress;
								reloc.addGlobal( global.m_fromOffset + classOffset, addr, global.m_toClass );
							}

							for (i=0; i < localReloc.m_finish.getSize() ;i++)
							{
								hkRelocationInfo::Finish& finish = localReloc.m_finish[i];
								reloc.addFinish( finish.m_fromOffset + classOffset, finish.m_className );
							}
							for (i=0; i < localReloc.m_imports.getSize() ;i++)
							{
								hkRelocationInfo::Import& external = localReloc.m_imports[i];
								reloc.addImport( external.m_fromOffset + classOffset, external.m_identifier);
							}
						}

						// Apply fixups to local buffer
						localReloc.applyLocalAndGlobal( localBuffer.pointerAt(0) ); 

						delete snode;
					}

					const hkClass& sclass = *reinterpret_cast<hkClass*>( localBuffer.pointerAt(0) );

					int ssize = sclass.getObjectSize();
					int arrayBeginOffset = buffer.reserve(0);
					buffer.reserve( ssize * numElements, Buffer::PAD_16 );
					buffer.advance( ssize * numElements, Buffer::PAD_16 );


					for( int i = 0; i < numElements; ++i )
					{
						hkXmlParser::Node* snode = HK_NULL;
						parser.nextNode(&snode, reader);
						readClassBody( sclass, arrayBeginOffset + i*ssize, buffer,
							snode->asStart(), parser, reader, reloc );
						delete snode;
					}

					DummyHomogeneousArray& dummy = lookupMember<DummyHomogeneousArray>( buffer.pointerAt(classStartOffset + member->getOffset()) );
					dummy.klass = HK_NULL;
					dummy.data = HK_NULL;
					dummy.size = numElements;

					// Add local for class
					reloc.addLocal( classStartOffset + member->getOffset(), classOffset );

					if( numElements > 0 )
					{
						reloc.addLocal( classStartOffset + member->getOffset() +sizeof(hkClass*), arrayBeginOffset );
					}

					break;
				}
				case hkClassMember::TYPE_VARIANT:
				{
					hkString text = extractText(parser, reader, startElement, true);
					int space = text.indexOf(' ');
					if( space != -1 )
					{
						int off = classStartOffset + member->getOffset();
						hkString s = text.substr(0, space);
						reloc.addImport( off, s.cString() );
						s = text.substr(space+1);
						reloc.addImport( off+sizeof(void*), s.cString() );
					}
					break;
				}
				default:
				{
					HK_ASSERT2(0x58b01b1f,0,"Found unknown (or unhandled) class member type in XML read.");
					break;
				}
			}
			if( consumeEndElement(parser, reader, startElement) == HK_FAILURE )
			{
				delete node;
				break;
			}
		}
		else if ( hkXmlParser::EndElement* ee = node->asEnd() )
		{
			if( topElement && (ee->name == topElement->name ))
			{
				delete node;
				return HK_SUCCESS;
			}
		}
		delete node;
	}
	return HK_FAILURE;
}

int hkXmlObjectReader::readObject(
	hkStreamReader* reader,
	void* buf, int bufSize,
	const hkClass& klass,
	hkRelocationInfo& reloc )
{
	hkArray<char> array;
	array.reserve(bufSize);
	if( readObject(reader, array, klass, reloc ) == HK_SUCCESS )
	{
		if( array.getSize() <= bufSize )
		{
			hkString::memCpy( buf, array.begin(), array.getSize() );
			return array.getSize();
		}
	}
	return -1;
}

hkResult hkXmlObjectReader::readObject(
	hkStreamReader* reader,
	hkArray<char>& array,
	const hkClass& klass,
	hkRelocationInfo& reloc )
{
	HK_ASSERT2( 0x5412ce0d, reader->markSupported(), "Stream needs to support marking");
	hkXmlParser::Node* node;
	Buffer buffer(array);
	hkResult result = HK_FAILURE;

	while( m_parser->nextNode(&node, reader) == HK_SUCCESS )
	{
		if( hkXmlParser::StartElement* startElement = node->asStart() )
		{
			if( startElement->name == "hkobject")
			{
				int objectStart = buffer.reserve(klass.getObjectSize(), Buffer::PAD_16);
				buffer.advance(klass.getObjectSize(), Buffer::PAD_16);
				result = readClassBody( klass, objectStart, buffer, startElement, *m_parser, reader, reloc );
			}
			else
			{
				HK_ASSERT2(0x5ae0b569, 0, "Unknown tag " << startElement->name );
			}
		}
		else if( hkXmlParser::Characters* characters = node->asCharacters() )
		{
			characters->canonicalize();
			HK_ASSERT2(0x742a0073, characters->text.getLength()==0, "unexpected characters" << startElement->name );
			
		}
		else if( hkXmlParser::EndElement* endElement = node->asEnd() )
		{
			HK_ASSERT2(0x46a5a10e, 0, "unexpected end node" << endElement->name );
		}
		else
		{
			HK_ERROR(0x6a858ec3, "Unknown element type returned from XML parser.");
		}
		delete node;
		break;
	}
	return result;
}

static void eatMarker( hkStreamReader* reader, const char* marker )
{
	const char* cur = marker;

	while(1)
	{
		int c = reader->readChar();
		HK_ASSERT( 0x5e1b58b1, c != -1 );
		if( !isSpace(c) )
		{
			HK_ASSERT2( 0x3d92ea88, c == *cur, "Eat marker broken in XML reader." );
			cur += 1;
			break;
		}
	}
	while( *cur )
	{
		HK_ON_DEBUG(int c = )reader->readChar();
		HK_ASSERT2( 0x2f41c29e, c != -1, "Eat marker broken in XML reader."  );
		HK_ASSERT2( 0x3ceebe22, c == *cur, "Eat marker broken in XML reader."  );
		cur += 1;
	}
}

hkResult hkXmlObjectReader::readRaw( hkStreamReader* reader, void* buf, int bufLen )
{
	eatMarker(reader, "<![CDATA[");
	hkResult result = base64read( reader, buf, bufLen );
	if( result == HK_SUCCESS )
	{
		eatMarker(reader, "]]>");
	}
	return result;
}

hkResult hkXmlObjectReader::base64read( hkStreamReader* sr, void* buf, int len )
{
	HK_ASSERT2( 0x5412ce0d, sr->markSupported(), "Stream needs to support marking");

	//XXX batch this instead of single byte reads.
	static const signed char ascii2bin[128] = 
	{
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,
		52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,
		-1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
		15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,
		-1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
		41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1
	};
	
	hkUint8* cur = static_cast<hkUint8*>(buf);
	const int inBytesPerOutByte[3] = { 0, 2, 3 };
	const int outBytesPerInBytes[4] = { 0, 0, 1, 2 };
	int bytesNeeded = (len/3)*4 + inBytesPerOutByte[len%3];

	int ibuflen = 0;
	unsigned char ibuf[4] = { 0 }; // ibuf contains only 6 bit chars
	while( bytesNeeded > 0 )
	{
		// read char by char discarding unknown chars
		unsigned char inchar;
		if( sr->read(&inchar, 1) != 1 )
		{
			return HK_FAILURE; // exit
		}
		if( ascii2bin[inchar & 0x7f] != -1 ) // to 6 bit
		{
			--bytesNeeded;
			ibuf[ ibuflen++ ] = hkUint8(ascii2bin[inchar]);
			
			if( ibuflen == 4 ) // got a chunk
			{
				ibuflen = 0;				
				cur[0] = hkUint8((ibuf[0] << 2) | (ibuf[1] >> 4 ));
				cur[1] = hkUint8((ibuf[1] << 4) | (ibuf[2] >> 2 ));
				cur[2] = hkUint8((ibuf[2] << 6) | ibuf[3] );
				cur += 3;
				ibuf[0] = ibuf[1] = ibuf[2] = ibuf[3] = 0;
			}
		}
	}

	// eat padding '='
	while( sr->isOk() )
	{
		sr->setMark(1);
		unsigned char c = 0;
		sr->read( &c, 1 );
		if( c != '=' )
		{
			sr->rewindToMark();
			break;
		}
	}
	
	if( ibuflen ) // handle leftovers
	{
		unsigned char obuf[3];
		obuf[0] = hkUint8((ibuf[0] << 2) | (ibuf[1] >> 4 ));
		obuf[1] = hkUint8((ibuf[1] << 4) | (ibuf[2] >> 2 ));
		obuf[2] = hkUint8((ibuf[2] << 6) | ibuf[3] );

		for( int i = 0; i < outBytesPerInBytes[ibuflen]; ++i )
		{
			cur[i] = obuf[i];
		}
	}
	return HK_SUCCESS;
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
