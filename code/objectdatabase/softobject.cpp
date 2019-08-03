
#include "objectdatabase/dataobject.h"
#include "objectdatabase/class_factory.h"
#include "objectdatabase/guidclient.h"
#include "objectdatabase/objectdatabase.h"
#include "objectdatabase/repeat.h"

//------------------------------------
// SoftObjectFactory
//------------------------------------

SoftObjectFactoryInterface::SoftObjectFactoryInterface( const ntstd::String& pSoftObjectName, const ntstd::String& pParentClass ) :
		StdDataInterface( pSoftObjectName + "Interface", pSoftObjectName, DataInterface::GetInterfaceGUIDFromName(pSoftObjectName+ "Interface") ),
		m_pParentClass( pParentClass ),
		m_iCurrentSize(0)
{
	ObjectDatabase::Get().RegisterSoftInterface( pSoftObjectName.c_str(), this );

	if( !m_pParentClass.empty() )
	{
		StdDataInterface* pInterface = ObjectDatabase::Get().GetInterface( m_pParentClass.c_str() );
		m_iCurrentSize += pInterface->GetSizeOfObject();

		StdDataInterface::const_iterator inIt = pInterface->begin();
		while( inIt != pInterface->end() )
		{
			const CHashedString& name = (*inIt)->GetName();
			const ntstd::String& defaultT = (*inIt)->GetDefault();
			// does this field already exist
			if( m_NameFieldMap.find( name ) == m_NameFieldMap.end() )
			{
				// no add it
				if( (*inIt)->IsHard() )
				{
					m_Fields.push_back( (*inIt)->Clone() );
				} else
				{
					PushField( name, (*inIt)->GetType(), defaultT );
				}
				m_NameFieldMap[ name ] = m_Fields.back(); // this will be overwritten when CreateFastIndices() is called but its useful here
			} else	
			{
				// yes just update the default
				m_NameFieldMap[ name ]->SetDefault( defaultT );
			}

			++inIt;
		}

		StdDataInterface::CallBackConstIterator coIt = pInterface->CallBackBegin();
		while( coIt != pInterface->CallBackEnd() )
		{
			m_CallBackMap[ (*coIt).first ] = (*coIt).second->Clone();
			++coIt;
		}
		// add parent name 
		m_CastToNames.push_back( m_pParentClass );
		// add parents cast to list (Not sure this is the right idea..)
		StdDataInterface::CastToContainerConstIterator cnIt = pInterface->CastToNamesBegin();
		while( cnIt != pInterface->CastToNamesEnd() )
		{
			m_CastToNames.push_back( *cnIt );
			++cnIt;
		}
	}	
}

SoftObjectFactoryInterface::SoftObjectFactoryInterface() : 
	StdDataInterface( "SoftObjectFactoryInterface", "SoftObjectFactory", SOFTOBJECTFACTORYINTERFACE_GUID ),
	m_iCurrentSize(0)
{
}

SoftObjectFactoryInterface::~SoftObjectFactoryInterface()
{
	// todo unregister interface
}

void* SoftObjectFactoryInterface::CreateObject()
{
	// hmmm what memory pool should soft objects come out of... (parents?)
	void* pMem =  NT_NEW_CHUNK(Mem::MC_ODB) char[ m_iCurrentSize ];
	PlacementCreateObject( pMem ); // call our own placement create object
	return pMem;
}

void SoftObjectFactoryInterface::PlacementCreateObject( void* pMem )
{
	if( !m_pParentClass.empty() )
	{
		StdDataInterface* pInterface = ObjectDatabase::Get().GetInterface( m_pParentClass.c_str() );
		pInterface->PlacementCreateObject( pMem );
	}	
}

void SoftObjectFactoryInterface::DestroyObject( void* obj)
{
	// call our placement destory object
	PlacementDestroyObject( obj );

	// actually free the backing memory
	NT_DELETE_ARRAY_CHUNK( Mem::MC_ODB, ( (char*)obj ) );
}

void SoftObjectFactoryInterface::PlacementDestroyObject( void* pMem )
{
	if( !m_pParentClass.empty() )
	{
		StdDataInterface* pInterface = ObjectDatabase::Get().GetInterface( m_pParentClass.c_str() );
		pInterface->PlacementDestroyObject( pMem );
	}
}

SoftObjectFactoryInterface* SoftObjectFactoryInterface::CreateFactoryFor( const ntstd::String& pSoftObjectName, const ntstd::String& pParentClass )
{
	return NT_NEW_CHUNK(Mem::MC_ODB) SoftObjectFactoryInterface( pSoftObjectName, pParentClass );
}

void SoftObjectFactoryInterface::AddProperty( const CHashedString& name, const ntstd::String& type, const ntstd::String& defaultVal )
{
	int typeof_number = ConvertTypeOfStringToNum( type.c_str() );
	ntError_p( typeof_number != 0, ("Invalid type for a soft property") );
	PushField( name, typeof_number, defaultVal );
}

void SoftObjectFactoryInterface::PushField( const CHashedString& name, int typeof_number, const ntstd::String& defaultVal )
{
	// check for existing field
	if( m_NameFieldMap.find( name ) != m_NameFieldMap.end() )
	{
		m_NameFieldMap[ name ]->SetDefault( defaultVal );
		return;
	}

	ntstd::Istringstream is( defaultVal );

	unsigned int sizeof_type = ReturnTypeOfSize( typeof_number );
	unsigned int pow2size = Util::NextPow2(sizeof_type);
	unsigned int alignedBase = ROUND_POW2( m_iCurrentSize, pow2size );

	// special case detect pointers, enums etc.
	if( typeof_number == typeof_num_of_type<void*>() )
	{
		if( defaultVal == "NULL" || defaultVal == "Null" || defaultVal == "null" )
		{
			m_Fields.push_back( NT_NEW_CHUNK(Mem::MC_ODB) SinglePointerDIF<void*>( name, NT_NEW_CHUNK(Mem::MC_ODB) CStructField<void*,true>( alignedBase ), ntstd::String() ) );
		} else
		{
			m_Fields.push_back( NT_NEW_CHUNK(Mem::MC_ODB) SinglePointerDIF<void*>( name, NT_NEW_CHUNK(Mem::MC_ODB) CStructField<void*,true>( alignedBase ), defaultVal ) );
		}
		m_iCurrentSize = (alignedBase + sizeof_type);

		return;
	}

#define localVoidStartListHelp(chunk)														\
	if( typeof_number == typeof_num_of_type<ntstd::List<void*, chunk> >() )					\
	{																						\
		m_Fields.push_back( NT_NEW_CHUNK(Mem::MC_ODB) PointerContainerDIF<ntstd::List<void*, chunk> >( name, NT_NEW_CHUNK(Mem::MC_ODB) CStructField< ntstd::List<void*,chunk>,true >( alignedBase ) ) ); \
		m_iCurrentSize = (alignedBase + sizeof_type);										\
		return;																				\
	}

	// this makes sure softobject are created with the correct chunk
	localVoidStartListHelp( Mem::MC_GFX );
	localVoidStartListHelp( Mem::MC_MISC );
	localVoidStartListHelp( Mem::MC_ODB );
	localVoidStartListHelp( Mem::MC_HAVOK );
	localVoidStartListHelp( Mem::MC_AI );
	localVoidStartListHelp( Mem::MC_LOADER );
	localVoidStartListHelp( Mem::MC_ARMY );
	localVoidStartListHelp( Mem::MC_LUA );
	localVoidStartListHelp( Mem::MC_ENTITY );
	localVoidStartListHelp( Mem::MC_CAMERA );
	localVoidStartListHelp( Mem::MC_ANIMATION );
	localVoidStartListHelp( Mem::MC_EFFECTS );
	localVoidStartListHelp( Mem::MC_PROCEDURAL );

#undef localVoidStartListHelp

	if( typeof_number == typeof_num_of_type<bool>() )
	{
		m_Fields.push_back( NT_NEW_CHUNK(Mem::MC_ODB) SingleItemDIF<bool>( name, NT_NEW_CHUNK(Mem::MC_ODB) CStructField<bool,true>( alignedBase ), false) );
		m_Fields.back()->SetDefault( defaultVal );
		m_iCurrentSize = (alignedBase + sizeof_type);
		return;
	}

#define localPUSH(N)	\
	case N:			\
	{				\
		typeof_num_declare(N) defaultT; \
		ConvertIStreamToType( is, typeof_number, &defaultT ); \
		m_Fields.push_back( NT_NEW_CHUNK(Mem::MC_ODB) SingleItemDIF<typeof(defaultT)>( name, NT_NEW_CHUNK(Mem::MC_ODB) CStructField<typeof(defaultT),true>( alignedBase ), defaultT ) ); \
		m_iCurrentSize = (alignedBase + sizeof_type);	\
		return; \
	}

	switch( typeof_number )
	{
		REPEAT( NUM_TYPEOF_MACROS, localPUSH );
	default:
		ntError_p( false, ("Invalid soft attribute Push field type") );
	};
#undef localPUSH
}

void SoftObjectFactoryInterface::Destroy()
{
	NT_DELETE_CHUNK( Mem::MC_ODB, this );
}

