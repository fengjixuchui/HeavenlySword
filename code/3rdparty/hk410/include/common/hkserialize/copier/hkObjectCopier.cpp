/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok).  Not for disclosure or distribution without Havok's
 * prior written consent.This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2006 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <hkserialize/hkSerialize.h>
#include <hkbase/class/hkClass.h>
#include <hkbase/class/hkClassEnum.h>
#include <hkbase/stream/hkOArchive.h>
#include <hkbase/stream/hkStreambufFactory.h>
#include <hkbase/stream/hkStreamWriter.h>
#include <hkbase/memory/hkLocalArray.h>
#include <hkserialize/copier/hkObjectCopier.h>
#include <hkserialize/serialize/hkRelocationInfo.h>

#if 0
	extern "C" int printf(const char*,...);
#	define PRINT(A) printf A
#else
#	define PRINT(A) /* nothing */
#endif

namespace
{
	bool inRange(int num, int lo, int hi)
	{
		return num >= lo && num < hi;
	}
	template <typename T>
	T min2(T a, T b)
	{
		return a<b ? a : b;
	}
}
extern const hkClass hkClassClass;

hkObjectCopier::hkObjectCopier(const hkStructureLayout& layoutIn, const hkStructureLayout& layoutOut)
	:	m_layoutIn( layoutIn ), m_layoutOut( layoutOut )
{
	m_byteSwap = ( m_layoutIn.getRules().m_littleEndian != m_layoutOut.getRules().m_littleEndian );
	HK_ASSERT(0x3bcc68e5, layoutIn.getRules().m_bytesInPointer == sizeof(void*) );
}

hkObjectCopier::~hkObjectCopier()
{
}

namespace 
{
	typedef hkArray<char> hkAnyArray;

	template <typename T>
	struct DummyArray
	{
		T* data;
		int size;
		int capAndFlags;
	};

	struct DummyHomogeneousArray
	{
		hkClass* klass;
		void* data;
		int size;
		//		int capAndFlags;
	};
}

static void padUp( hkStreamWriter* w, int pad=16 )
{
	int o = w->tell();
	hkLocalArray<char> buf(pad);
	const unsigned char padChar = HK_ON_DEBUG( (unsigned char)(0x7f) ) + 0; 
	buf.setSize(pad, (const char)padChar);
	if( o & (pad-1) )
	{
		w->write( buf.begin(), pad - (o&(pad-1)) );
	}
}

static int calcCArraySize( const hkClassMember& member )
{
	return (member.getCstyleArraySize()) ? member.getCstyleArraySize() : 1;
}

void hkObjectCopier::writeZero(
	hkOArchive& oa,
	const hkClassMember& memberOut )
{
	//
	// Write zero for every different subtype
	//
	hkClassMember::Type subtype = memberOut.getSubType();
	int size = 0;
	switch( subtype )
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
		case hkClassMember::TYPE_REAL:
		case hkClassMember::TYPE_VECTOR4:
		case hkClassMember::TYPE_QUATERNION:
		case hkClassMember::TYPE_MATRIX3:
		case hkClassMember::TYPE_ROTATION:
		case hkClassMember::TYPE_QSTRANSFORM:
		case hkClassMember::TYPE_MATRIX4:
		case hkClassMember::TYPE_TRANSFORM:
		case hkClassMember::TYPE_ENUM:
		{	
			size = memberOut.getSizeInBytes();
			break;
		}
		case hkClassMember::TYPE_ULONG:
		case hkClassMember::TYPE_POINTER:
		case hkClassMember::TYPE_FUNCTIONPOINTER:
		{
			size = m_layoutOut.getRules().m_bytesInPointer * calcCArraySize( memberOut );
			break;
		}
		case hkClassMember::TYPE_ARRAY:
		case hkClassMember::TYPE_INPLACEARRAY:
		case hkClassMember::TYPE_SIMPLEARRAY:
		case hkClassMember::TYPE_HOMOGENEOUSARRAY:
		{
			// Twice the size of pointers.
			HK_COMPILE_TIME_ASSERT( sizeof(void*) <= 8 );

			if( subtype == hkClassMember::TYPE_HOMOGENEOUSARRAY )
			{
				size += m_layoutOut.getRules().m_bytesInPointer;
			}

			size += m_layoutOut.getRules().m_bytesInPointer;
			size += sizeof(hkUint32); // size
			if( subtype == hkClassMember::TYPE_ARRAY )
			{
				size += sizeof(hkUint32); // capacity
			}
			break;
		}
		case hkClassMember::TYPE_STRUCT: // single struct
		{
			const hkClass& sclass = memberOut.getStructClass();
			size = sclass.getObjectSize() * calcCArraySize( memberOut );
			break;
		}
		case hkClassMember::TYPE_VARIANT:
		{
			size = m_layoutOut.getRules().m_bytesInPointer * 2 * calcCArraySize( memberOut );
			break;
		}
		default:
		{
			HK_ERROR(0x5ef4e5a4, "Unknown class member type found!" );
			break;
		}
	}
	HK_ASSERT2(0x5ef4e5a4, size != 0, "Incorrect size for class member calculated.");

	hkLocalArray<char> zero(size);
	zero.setSize(size,0);
	oa.writeRaw( zero.begin(), size );
}

static void writePodArray(hkOArchive& oa, const hkClassMember::Type mtype,
						  int elemsize, int nelem, const void* startAddress )
{
	switch( mtype )
	{
		case hkClassMember::TYPE_VECTOR4:
		case hkClassMember::TYPE_QUATERNION:
		case hkClassMember::TYPE_ROTATION:
		case hkClassMember::TYPE_MATRIX3:
		case hkClassMember::TYPE_MATRIX4:
		case hkClassMember::TYPE_QSTRANSFORM:
		case hkClassMember::TYPE_TRANSFORM:
		{
			nelem *= (elemsize/ sizeof(hkReal) );
			elemsize = sizeof(hkReal);
			/* fall through */
		}
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
		{
			oa.writeArrayGeneric( startAddress, elemsize, nelem );
			break;
		}
		case hkClassMember::TYPE_POINTER:
		case hkClassMember::TYPE_FUNCTIONPOINTER:
		case hkClassMember::TYPE_ARRAY:
		case hkClassMember::TYPE_INPLACEARRAY:
		case hkClassMember::TYPE_STRUCT:
		case hkClassMember::TYPE_SIMPLEARRAY:
		case hkClassMember::TYPE_HOMOGENEOUSARRAY:
		case hkClassMember::TYPE_VARIANT:
		{
			HK_ASSERT2(0x747e1e04, 0, "Write POD array called with non-pod type." );
		}
		default:
		{
			HK_ERROR(0x747e1e03, "Unknown class member found during write of plain data array.");
		}		
	}
}


int hkObjectCopier::saveBody(
	const void* dataIn, // data source
	const hkClass& klassIn, // class source
	hkOArchive& oa, // dest data
	const hkClass& klassOut // class dest
)
{
	hkStreamWriter* writer = oa.getStreamWriter();
	int classStart = writer->tell();

	for( int memberIdx = 0; memberIdx < klassOut.getNumMembers(); ++memberIdx )
	{
		const hkClassMember& memberOut = klassOut.getMember( memberIdx );
		int memberOutStart = classStart + memberOut.getOffset();
		writer->seek( memberOutStart, hkStreamWriter::STREAM_SET );

		if( const hkClassMember* memberInPtr = klassIn.getMemberByName( memberOut.getName() ) )
		{
			const hkClassMember& memberIn = *memberInPtr;

			if( memberIn.getType() == memberOut.getType()
				&& memberIn.getSubType() == memberOut.getSubType() )
			{
				const void* addressIn = static_cast<const char*>(dataIn) + memberIn.getOffset();

				switch( memberOut.getType() )
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
						int nelemIn = calcCArraySize( memberIn );
						int nelemOut = calcCArraySize( memberOut );
						int nelem = min2(nelemIn, nelemOut);
						int realSize = memberOut.getType() == hkClassMember::TYPE_ULONG
										? m_layoutOut.getRules().m_bytesInPointer
										: memberOut.getSizeInBytes();
						writePodArray(oa, memberOut.getType(),
							realSize / nelemOut, nelem, addressIn);
						break;
					}
					case hkClassMember::TYPE_ENUM:
					{
						int nelemIn = calcCArraySize( memberIn );
						int nelemOut = calcCArraySize( memberOut );
						int nelem = min2(nelemIn, nelemOut);
						const hkClassEnum& enumIn = memberIn.getEnumClass();
						const hkClassEnum& enumOut = memberOut.getEnumClass();
						int enumBytes = memberIn.getSizeInBytes() / nelemIn;
						const void* enumInPtr = addressIn;
						for( int i = 0; i < nelem; ++i )
						{
							int valueIn = memberIn.getEnumValue(enumInPtr);
							int valueOut = 0;
							const char* nameIn;
							if ( enumIn.getNameOfValue(valueIn, &nameIn) == HK_SUCCESS )
							{
								if ( enumOut.getValueOfName( nameIn, &valueOut ) == HK_SUCCESS )
								{
								}
								else
								{
									HK_WARN(0x337d3f12, "couldn't convert "<< klassOut.getName() << "::" << enumOut.getName() << " " << nameIn << " to a value");
								}
							}
							else
							{
								HK_WARN(0x337d3f13, "couldn't convert "<< klassIn.getName() << "::" << enumIn.getName() << " "<<valueIn<<" to a name");
							}
							switch( enumBytes )
							{
								case 1: oa.write8( hkInt8(valueOut) ); break;
								case 2: oa.write16( hkInt16(valueOut) ); break;
								case 4: oa.write32( hkInt32(valueOut) ); break;
							}
							enumInPtr = hkAddByteOffsetConst(enumInPtr, enumBytes);
						}
						break;
					}
					case hkClassMember::TYPE_ZERO:
					{
						writeZero( oa, memberOut );
						break;
					}
					case hkClassMember::TYPE_CSTRING:
					case hkClassMember::TYPE_POINTER:
					case hkClassMember::TYPE_FUNCTIONPOINTER:
					{
						int nelem = calcCArraySize( memberOut );
						HK_COMPILE_TIME_ASSERT( sizeof(void*) <= 8 );
						hkUint64 zero = 0;
						for( int j = 0; j < nelem; ++j )
						{
							oa.writeRaw( &zero, m_layoutOut.getRules().m_bytesInPointer );
						}
						break;
					}
					case hkClassMember::TYPE_ARRAY:
					case hkClassMember::TYPE_SIMPLEARRAY:
					case hkClassMember::TYPE_HOMOGENEOUSARRAY:
					case hkClassMember::TYPE_INPLACEARRAY:
					{
						HK_COMPILE_TIME_ASSERT( sizeof(void*) <= 8 );

						int arraySize;
						if( memberOut.getType() == hkClassMember::TYPE_HOMOGENEOUSARRAY )
						{
							hkUint64 aid = 0; // class pointer
							oa.writeArrayGeneric( &aid, m_layoutOut.getRules().m_bytesInPointer, 1 );
							arraySize = static_cast<const DummyHomogeneousArray*>( addressIn )->size;
						}
						else
						{
							arraySize = static_cast<const DummyArray<char>*>( addressIn )->size;
						}

						hkUint64 aid = 0; // data pointer
						oa.writeArrayGeneric( &aid, m_layoutOut.getRules().m_bytesInPointer, 1 );
						oa.write32( arraySize ); // size
						if( memberOut.getType() == hkClassMember::TYPE_ARRAY )
						{
							// Make sure we store it as 'locked' and not to be deallocated
							// so that when it is read in it will be correctly setup.
							int capAndFlags = arraySize | hkAnyArray::DONT_DEALLOCATE_FLAG | hkAnyArray::LOCKED_FLAG;
							oa.write32( capAndFlags ); 
						}
						break;
					}
					case hkClassMember::TYPE_STRUCT: // single struct
					{
						const hkClass& structIn = memberIn.getStructClass();
						const hkClass& structOut = memberOut.getStructClass();
						int nelem = min2( calcCArraySize( memberIn ), calcCArraySize( memberOut ) );

						for( int i = 0 ; i < nelem; ++i)
						{
							const void* din = static_cast<const char*>(addressIn)+i*structIn.getObjectSize();
							saveBody( din, structIn, oa, structOut );
						}
						
						break;
					}
					case hkClassMember::TYPE_VARIANT:
					{
						hkUint64 aid[2] = {0,0}; // data, class pointer
						int nelem = min2( calcCArraySize( memberIn ), calcCArraySize( memberOut ) );
						for( int i = 0; i < nelem; ++i )
						{
							oa.writeArrayGeneric( aid, m_layoutOut.getRules().m_bytesInPointer, 2 );
						}
						break;
					}
					default:
					{
						HK_ERROR(0x641e3e03, "Unknown class member found during write of data.");
					}
				}
				continue; // successful copy
			}
		}
		klassOut.getDefault( memberIdx, writer ); // try to get default
	}
	// skip possible end padding
	oa.getStreamWriter()->seek( classStart + klassOut.getObjectSize(), hkStreamWriter::STREAM_SET );
	return classStart;
}

static hkResult saveCstring(const char* dataIn, int klassMemOffset, hkOArchive& dataOut, hkRelocationInfo& fixups)
{
	fixups.addLocal( klassMemOffset, dataOut.getStreamWriter()->tell() );
	dataOut.writeRaw( dataIn, hkString::strLen(dataIn)+1 );
	return HK_SUCCESS;
}

void hkObjectCopier::saveExtras(
	const void* dataIn,
	const hkClass& klassIn,
	hkOArchive& dataOut,
	const hkClass& klassOut,
	int classStart, // offset of where dataIn is stored
	hkRelocationInfo& fixups, // fixups to apply on load
	int level
)
{
	if ( level == 0 )
	{
		// assumes vtable is at start
		fixups.addFinish( classStart, klassOut.getName() );
	}
	++level;
	
	for( int memberIdx = 0; memberIdx < klassOut.getNumMembers(); ++memberIdx )
	{
		const hkClassMember& memberOut = klassOut.getMember(memberIdx);
		if( const hkClassMember* memberInPtr = klassIn.getMemberByName(memberOut.getName() ) )
		{
			const hkClassMember& memberIn = *memberInPtr;
			const void* addressIn = static_cast<const char*>(dataIn) + memberIn.getOffset();

			if( memberIn.getType() != memberOut.getType() ||
				memberIn.getSubType() != memberOut.getSubType() )
			{
				//HK_WARN(0x337d3f11, "fixme: member '" << klassIn.getName() << "::" << memberIn.getName() << "' type has changed");
				continue;
			}

			switch( memberOut.getType() )
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
				case hkClassMember::TYPE_ENUM:
				case hkClassMember::TYPE_ZERO:
				{
					break; // nothing extra for these
				}
				case hkClassMember::TYPE_CSTRING:
				{
					const void* ptrFrom = static_cast<const char*>(addressIn);
					const void* ptrTo = *static_cast<const void*const*>(ptrFrom);
					if( ptrTo != HK_NULL )
					{
						saveCstring(static_cast<const char*>(ptrTo), classStart + memberOut.getOffset(), dataOut, fixups);
					}
					break;
				}
				case hkClassMember::TYPE_POINTER:
				{
					HK_ASSERT2(0x4ca4f06d, (memberOut.getSubType() != hkClassMember::TYPE_CSTRING && memberOut.getSubType() != hkClassMember::TYPE_POINTER), "The pointer to c-string and general pointers are unsupported.");
					int nelem = min2( calcCArraySize(memberIn), calcCArraySize(memberOut) );
					for( int i = 0; i < nelem; ++i )
					{
						const void* ptrFrom = static_cast<const char*>(addressIn) + i * m_layoutIn.getRules().m_bytesInPointer;
						const void* ptrTo = *static_cast<const void*const*>(ptrFrom); 
						if( ptrTo != HK_NULL )
						{
							int memOffsetOut = memberOut.getOffset() + i * m_layoutOut.getRules().m_bytesInPointer;
							if( memberOut.getSubType() == hkClassMember::TYPE_CHAR )
							{
								saveCstring(static_cast<const char*>(ptrTo), classStart + memOffsetOut, dataOut, fixups);
							}
							else
							{
								const hkClass* k = ( memberIn.getSubType() == hkClassMember::TYPE_STRUCT )
									? &memberOut.getStructClass() 
									: HK_NULL;
								fixups.addGlobal( classStart + memOffsetOut, const_cast<void*>(ptrTo), const_cast<hkClass*>(k) );
							}
						}
					}
					break;
				}
				case hkClassMember::TYPE_FUNCTIONPOINTER:
				{
					//XXX add external
					break;
				}
				case hkClassMember::TYPE_ARRAY:
				case hkClassMember::TYPE_INPLACEARRAY:
				case hkClassMember::TYPE_SIMPLEARRAY:
				{
					const DummyArray<char>* carray = (const DummyArray<char>*)addressIn;

					if( carray->size )
					{
						if( memberOut.getSubType() == hkClassMember::TYPE_POINTER ) // array of pointer
						{
							fixups.addLocal( classStart + memberOut.getOffset(), dataOut.getStreamWriter()->tell() );
							hkUint64 zero = 0;
							HK_COMPILE_TIME_ASSERT( sizeof(void*) <= sizeof(zero) );
							const DummyArray<void*>* parray = (const DummyArray<void*>*)addressIn;
							for( int i = 0; i < parray->size; ++i )
							{
								const hkClass* k = &memberIn.getStructClass();
								fixups.addGlobal(
									dataOut.getStreamWriter()->tell(),
									parray->data[i],
									const_cast<hkClass*>(k) );
								dataOut.writeRaw( &zero, m_layoutOut.getRules().m_bytesInPointer );
							}
						}
						else if( memberOut.getSubType() == hkClassMember::TYPE_CSTRING ) // array of c-strings
						{
							PRINT(("saveExtras\tsimple array (char*), classStart = %p:\n", classStart));

							fixups.addLocal( classStart + memberOut.getOffset(), dataOut.getStreamWriter()->tell() );

							PRINT(("\taddLocal at %p\n", classStart + memberOut.getOffset()));

							hkUint64 zero = 0;
							HK_COMPILE_TIME_ASSERT( sizeof(char**) <= sizeof(zero) );
							const DummyArray<char*>* cstringArray = (const DummyArray<char*>*)addressIn;
							
							PRINT(("\tarray size = %d\n", cstringArray->size));

							int arrayStart = dataOut.getStreamWriter()->tell();
							for( int i = 0; i < cstringArray->size; ++i )
							{
								dataOut.writeRaw( &zero, m_layoutOut.getRules().m_bytesInPointer );
							}

							for( int i = 0; i < cstringArray->size; ++i)
							{
								if ( cstringArray->data[i] != HK_NULL )
								{
									PRINT(("\taddLocal at %p, cstring='%s'\n", arrayStart + i * m_layoutOut.getRules().m_bytesInPointer, cstringArray->data[i]));
									saveCstring(cstringArray->data[i], arrayStart + i * m_layoutOut.getRules().m_bytesInPointer, dataOut, fixups);
								}
							}

						}
						else if( memberOut.getSubType() == hkClassMember::TYPE_STRUCT ) // array of struct
						{
							int structStart = dataOut.getStreamWriter()->tell();
							fixups.addLocal( classStart + memberOut.getOffset(), structStart );
							const hkClass& sin = memberIn.getStructClass();
							const hkClass& sout = memberOut.getStructClass();

							const char* cur = carray->data;
							for( int i = 0; i < carray->size; ++i )
							{
								saveBody(cur + i*sin.getObjectSize(), sin, dataOut, sout);
							}
							for( int j = 0; j < carray->size; ++j )
							{
								saveExtras(cur + j*sin.getObjectSize(), sin,
									dataOut, sout,
									structStart+j*sout.getObjectSize(), fixups, level);
							}
						}
						else if( memberOut.getSubType() == hkClassMember::TYPE_VARIANT )
						{
							int arrayStart = dataOut.getStreamWriter()->tell();
							fixups.addLocal( classStart + memberOut.getOffset(), arrayStart );

							const DummyArray<hkVariant>* varray = (const DummyArray<hkVariant>*)addressIn;

							for( int i = 0; i < varray->size; ++i )
							{
								hkUint64 aid[2] = {0,0}; // data, class pointer
								dataOut.writeArrayGeneric( aid, m_layoutOut.getRules().m_bytesInPointer, 2 );
							}
							for( int j = 0; j < varray->size; ++j )
							{
								int vstart = (2*j)*m_layoutOut.getRules().m_bytesInPointer;
								fixups.addGlobal(
									arrayStart + vstart,
									varray->data[j].m_object,
									varray->data[j].m_class );
								fixups.addGlobal(
									arrayStart + vstart + m_layoutOut.getRules().m_bytesInPointer,
									const_cast<hkClass*>(varray->data[j].m_class),
									&hkClassClass );
							}
						}
						else // array of POD type
						{
							fixups.addLocal( classStart + memberOut.getOffset(), dataOut.getStreamWriter()->tell() );
							writePodArray(dataOut, memberOut.getSubType(), memberOut.getArrayMemberSize(),
								carray->size, carray->data );
						}
					}
					break;
				}
				case hkClassMember::TYPE_HOMOGENEOUSARRAY:
				{
					// class ptr, data ptr, size
					const DummyHomogeneousArray* darray = (const DummyHomogeneousArray*)addressIn;
					const hkClass& sin = *(darray->klass);
					if( const hkClass* soutPtr = &sin )
					{
						// Save the class pointer
						HK_ASSERT(0x065087ba, darray->klass != HK_NULL);
						fixups.addGlobal(
							classStart + memberOut.getOffset(),
							darray->klass,
							const_cast<hkClass*>(&hkClassClass) );

						int arrayStart = dataOut.getStreamWriter()->tell();

						// data pointer
						fixups.addLocal(
							classStart + memberOut.getOffset() + m_layoutOut.getRules().m_bytesInPointer,
							arrayStart );

						const char* cur = reinterpret_cast<const char*>( darray->data );
						const hkClass& sout = *soutPtr;
						for( int i = 0; i < darray->size; ++i )
						{
							saveBody(cur + i*sin.getObjectSize(), sin, dataOut, sout );
						}
						for( int j = 0; j < darray->size; ++j )
						{
							saveExtras(cur + j*sin.getObjectSize(), sin,
								dataOut, sout,
								arrayStart+j*sout.getObjectSize(), fixups, level);
						}
					}
					else
					{
						HK_WARN(0x34f95a55, "No hkClass for " << sin.getName() );
					}

					break;
				}
				case hkClassMember::TYPE_STRUCT:
				{
					const hkClass& sin = memberIn.getStructClass();
					const hkClass& sout = memberOut.getStructClass();
					int structStart = classStart + memberOut.getOffset();

					int nelemIn = calcCArraySize( memberIn );
					int nelemOut = calcCArraySize( memberOut );
					int nelem = min2(nelemIn, nelemOut);

					for( int i = 0 ; i < nelem; ++i )
					{
						const void* din = static_cast<const char*>(addressIn) + i * sin.getObjectSize();
						saveExtras( din, sin, dataOut, sout,
							structStart + i * sout.getObjectSize(), fixups, level);
					}
					break;
				}
				case hkClassMember::TYPE_VARIANT:
				{
					int nelemIn = calcCArraySize( memberIn );
					int nelemOut = calcCArraySize( memberOut );
					int nelem = min2(nelemIn, nelemOut);

					for( int i = 0; i < nelem; ++i )
					{
						const void* vobject = static_cast<const char*>(addressIn) + (i*2  )*m_layoutIn.getRules().m_bytesInPointer;
						const void* vclass  = static_cast<const char*>(addressIn) + (i*2+1)*m_layoutIn.getRules().m_bytesInPointer;
						vobject = *static_cast<const void*const*>(vobject);
						vclass  = *static_cast<const void*const*>(vclass);
						if( vobject )
						{
							fixups.addGlobal(
								classStart + memberOut.getOffset() + (2*i  )*m_layoutOut.getRules().m_bytesInPointer,
								const_cast<void*>(vobject),
								static_cast<const hkClass*>(vclass) );
						}
						if( vclass )
						{
							fixups.addGlobal(
								classStart + memberOut.getOffset() + (2*i+1)*m_layoutOut.getRules().m_bytesInPointer,
								const_cast<void*>(vclass),
								&hkClassClass );
						}
					}

					break;
				}
				default:
				{
					HK_ERROR(0x641e3e05, "Unknown class member found during write of data.");
				}
			}
			padUp(dataOut.getStreamWriter());
		}
	}
}

hkResult hkObjectCopier::copyObject( const void* dataIn, const hkClass& klassIn,
		   hkStreamWriter* dataOut, const hkClass& klassOut, hkRelocationInfo& reloc )
{
	PRINT(("Starting %s %p at %i\n", klassIn.getName(), dataIn, dataOut->tell()));
	
#ifdef HK_DEBUG
	int objectStart = dataOut->tell();
	int origLocal = reloc.m_local.getSize();
	int origGlobal = reloc.m_global.getSize();
	int origVirtual = reloc.m_finish.getSize();
	int origImports = reloc.m_imports.getSize();
#endif

	hkOArchive oa( dataOut, m_byteSwap );

	// pass 1 - save the class data itself
	int classStart = saveBody( dataIn, klassIn, oa, klassOut );
	padUp(dataOut);

	// pass 2 - save arrays etc
	saveExtras( dataIn, klassIn, oa, klassOut, classStart, reloc );
	padUp(dataOut);
#ifdef HK_DEBUG
	const char ERROR_STRING[] = "Fixup out of range in platform write.";
	{
		int i;
		int objectEnd = dataOut->tell();
		for( i = origLocal; i < reloc.m_local.getSize(); ++i )
		{
			hkRelocationInfo::Local& fix = reloc.m_local[i];
			HK_ASSERT2(0x4ea064f8, inRange(fix.m_fromOffset, objectStart, objectEnd), ERROR_STRING  );
			HK_ASSERT2(0x68ca252c, inRange(fix.m_toOffset, objectStart, objectEnd), ERROR_STRING );
		}
		for( i = origGlobal; i < reloc.m_global.getSize(); ++i )
		{
			hkRelocationInfo::Global& fix = reloc.m_global[i];
			HK_ASSERT2(0x747a20cd, inRange(fix.m_fromOffset, objectStart, objectEnd), ERROR_STRING );
		}
		for( i = origVirtual; i < reloc.m_finish.getSize(); ++i )
		{
			hkRelocationInfo::Finish& fix = reloc.m_finish[i];
			HK_ASSERT2(0x24c38635, inRange(fix.m_fromOffset, objectStart, objectEnd), ERROR_STRING );
		}
		for( i = origImports; i < reloc.m_imports.getSize(); ++i )
		{
			hkRelocationInfo::Import& fix = reloc.m_imports[i];
			HK_ASSERT2(0x3fe40756, inRange(fix.m_fromOffset, objectStart, objectEnd), ERROR_STRING );
		}
	}
#endif

	return oa.isOk() ? HK_SUCCESS : HK_FAILURE;
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
