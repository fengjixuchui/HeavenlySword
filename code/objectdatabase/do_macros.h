#if !defined( DATAOBJECT_INTERNAL )
#ntError Internal dataobject file don't include manually (use dataobject.h)
#endif

//! Place at the top of an interface defination, to reflect this class to the objectdatabase
//! namee should be the name of the class to reflect
#define START_STD_INTERFACE( name )												\
	START_CHUNKED_INTERFACE( name, Mem::MC_MISC )

#define START_CHUNKED_INTERFACE( name, chunk )									\
class name##Interface;															\
REGISTER_INTERFACE( name##Interface )											\
class name##Interface : public StdDataInterface									\
{																				\
public:																			\
	typedef name name_type;														\
	virtual void PlacementCreateObject( void* mem )								\
	{																			\
		NT_PLACEMENT_NEW (mem) name;											\
	}																			\
	virtual void PlacementDestroyObject( void* dobj )							\
	{																			\
		((name*)dobj)->~name();													\
		UNUSED(dobj); /* bizarre vs2003 warning fix*/							\
	}																			\
	virtual void* CreateObject()												\
	{																			\
		void* pTemp = NT_NEW_CHUNK( chunk ) name;								\
		return pTemp;															\
	}																			\
	virtual void DestroyObject( void* obj)										\
	{																			\
		PlacementDestroyObject( obj );											\
		NT_FREE_CHUNK( chunk, (uintptr_t) obj );								\
	}																			\
	virtual unsigned int GetSizeOfObject()										\
	{																			\
		return sizeof( name );													\
	}																			\
	virtual void PostConstruct()												\
	{																			\
	}																			\
	name##Interface() :															\
		StdDataInterface(	string_of(name##Interface),	string_of(name), DataInterface::GetInterfaceGUIDFromName( string_of(name##Interface) ) )	\
		{																		\
			const Mem::MEMORY_CHUNK mem_chunk = chunk;							\
			DataInterfaceField* pobField;										\
			UNUSED(pobField);													\
			UNUSED( mem_chunk );

#define END_STD_INTERFACE														\
			CreateFastIndices();												\
		}																		\
};																				

#define EVIL_FIXED_OFFSET 4	//!< SCE compiler workaround to prevent spurious 'incorrect use of offsetof macro' warnings

//! Helper class to get access to member variables
class DataVisitor
{
public:
	template<typename T0, typename T1>
	static int GetMemberByteOffset( const T1* member )
	{
		const T0* pClass = reinterpret_cast<const T0*>(EVIL_FIXED_OFFSET);
		const char* pObj = reinterpret_cast<const char*>( pClass );
		const char* pMember = reinterpret_cast<const char*>( member );
		return (pMember - pObj);
	}
};
//--------------------------------------------------------
//--------------------------------------------------------
//--------------------------------------------------------

//! returns the type of member variable at compile time (for template spec)
#define GET_MEMBER_VAR_TYPE( name, var )															\
	typeof(reinterpret_cast<const name*>(EVIL_FIXED_OFFSET)->var)

//! gets memory chunk of a list
#define GET_CONTAINER_MEMORY_CHUNK( name, var )														\
	(sizeof(char [reinterpret_cast<const name *>(EVIL_FIXED_OFFSET)-> var.memChunk_]))

//! gets the offset from the base of a class in a mannor that copes with multiple inhertance etc
#define GET_MEMBER_BYTE_OFFSET( name, var )															\
	DataVisitor::GetMemberByteOffset<name>( &reinterpret_cast<const name*>(EVIL_FIXED_OFFSET)->var )

//! expose a variable thats in a CStruct (standard C++ type class or struct)
#define DECLARE_CSTRUCTURE_VARIABLE( var, name, defaultT )															\
	m_Fields.push_back( NT_NEW_CHUNK(Mem::MC_ODB) SingleItemDIF< GET_MEMBER_VAR_TYPE(name_type,var) >( #name, NT_NEW_CHUNK(Mem::MC_ODB) CStructField< GET_MEMBER_VAR_TYPE(name_type,var) >( GET_MEMBER_BYTE_OFFSET( name_type, var ) ), defaultT ) );

//! expose a std container thats in a CStruct (standard C++ type class or struct)
#define DECLARE_CSTRUCTURE_CONTAINER( var, name )															\
	m_Fields.push_back( NT_NEW_CHUNK(Mem::MC_ODB) StdContainerDIF< GET_MEMBER_VAR_TYPE(name_type,var) >( #name, NT_NEW_CHUNK(Mem::MC_ODB) CStructField< GET_MEMBER_VAR_TYPE(name_type,var) >( GET_MEMBER_BYTE_OFFSET( name_type, var ) ) ) );

//! expose a std container of dataobjects thats in a CStruct (standard C++ type class or struct)
#define DECLARE_CSTRUCTURE_DATAOBJECT_CONTAINER( var, name )															\
	m_Fields.push_back( NT_NEW_CHUNK(Mem::MC_ODB) DataObjectContainerDIF( #name, NT_NEW_CHUNK(Mem::MC_ODB) CStructField< GET_MEMBER_VAR_TYPE(name_type,var) >( GET_MEMBER_BYTE_OFFSET( name_type, var ) ) ) );

//! expose a std container of pointers thats in a CStruct (standard C++ type class or struct)
#define DECLARE_CSTRUCTURE_PTR_CONTAINER( var, name)			\
	m_Fields.push_back( NT_NEW_CHUNK(Mem::MC_ODB) PointerContainerDIF<ntstd::List<void*, GET_CONTAINER_MEMORY_CHUNK(name_type, var) > >( #name, NT_NEW_CHUNK(Mem::MC_ODB) CStructField< ntstd::List<void*, GET_CONTAINER_MEMORY_CHUNK(name_type, var) > >( GET_MEMBER_BYTE_OFFSET( name_type, var ) ) ) ); \

//! expose a pointer thats in a CStruct (standard C++ type class or struct)
#define DECLARE_CSTRUCTURE_PTR( var, name, autoconstruct )			\
	m_Fields.push_back( NT_NEW_CHUNK(Mem::MC_ODB) SinglePointerDIF<void*>( #name, NT_NEW_CHUNK(Mem::MC_ODB) CStructField<void*>( GET_MEMBER_BYTE_OFFSET( name_type, var ) ), autoconstruct ) );

#define DECLARE_CSTRUCTURE_DEEP_LIST( var, name )			\
	m_Fields.push_back( NT_NEW_CHUNK(Mem::MC_ODB) DeepListDIF( #name, NT_NEW_CHUNK(Mem::MC_ODB) CStructField< DeepList >( GET_MEMBER_BYTE_OFFSET( name_type, var ) ) ) );

//! tell the interface that we are inherited from another class (used for the safe cast system)
#define DEFINE_INTERFACE_INHERITANCE( var )	m_CastToNames.push_back( #var );

//! copy the fields from another interface - perhaps this should be combined with DEFINE_INTERFACE_INHERITANCE
#define COPY_INTERFACE_FROM( iface ) \
	StdDataInterface*	pInterface	= ObjectDatabase::Get().GetInterface( #iface ); \
	ntError_p(pInterface, ("Cannot copy from interface that does not exist (yet?)")); \
	InheritFields(*pInterface);

#define OVERRIDE_DEFAULT( name, value )	\
	pobField = GetFieldByName(#name);	\
	ntError_p(pobField, ("Could not find field - " #name));	\
	pobField->SetDefault(value);

//! expose a variable in a cstruct that is accessed via accesors
#define DECLARE_CSTRUCTURE_VARIABLE_WITH_ACCESSOR( type, name, getter, setter, defaultT)															\
	{ \
		type dummy; \
		m_Fields.push_back( NT_NEW_CHUNK(Mem::MC_ODB) SingleItemDIF<typeof(dummy)>( #name, NT_NEW_CHUNK(Mem::MC_ODB) AccessorField< typeof(dummy), name_type>( &name_type::getter, &name_type::setter ), defaultT ) ); \
	}

//! expose a pointer variable that is accessed via accessors
#define DECLARE_PTR_VARIABLE_WITH_ACCESSOR( type, name, getter, setter, defaultT )															\
	{ \
		type dummy; \
		m_Fields.push_back( NT_NEW_CHUNK(Mem::MC_ODB) SinglePointerDIF<typeof(dummy)>( #name, NT_NEW_CHUNK(Mem::MC_ODB) AccessorField< typeof(dummy), name_type>( &name_type::getter, &name_type::setter ), defaultT ) ); \
	}


//--------------------------------------------------------
//--------------------------------------------------------
//--------------------------------------------------------

//! start a enum in a cstruct
#define START_CSTRUCTURE_ENUM( var, name, defaultT ) \
	{																																			\
		m_Fields.push_back( NT_NEW_CHUNK(Mem::MC_ODB) StdEnumDIF( #name, NT_NEW_CHUNK(Mem::MC_ODB) CStructField<unsigned int>( GET_MEMBER_BYTE_OFFSET( name_type, var ) ), defaultT ) );	\
		StdEnumDIF* enumInterface = (StdEnumDIF*) m_Fields.back();																			

//! inform the interface that the current enum is a global (externally defined enum)
#define DECLARE_ENUM_GLOBAL( name ) enumInterface->SetGlobalMode( #name );

//! declar an enum name and value to the current enum
#define DECLARE_ENUM_VALUE( value, name )	enumInterface->PushEnumValue( value, #name );

//! finish a enum in a cstruct
#define END_CSTRUCTURE_ENUM }

//--------------------------------------------------------
//--------------------------------------------------------
//--------------------------------------------------------

#define DECLARE_CALLBACK(name, callbacktype, callback)										\
{																							\
	CallBackConstIterator it = m_CallBackMap.find(name);									\
	if(it != m_CallBackMap.end())															\
	{																						\
		NT_DELETE_CHUNK( Mem::MC_ODB, m_CallBackMap[name]);									\
	}																						\
	m_CallBackMap[name] = NT_NEW_CHUNK(Mem::MC_ODB) callbacktype<name_type>( &name_type::callback );			\
}

//! insert a post construct callback into this interface
#define DECLARE_POSTCONSTRUCT_CALLBACK( callback ) DECLARE_CALLBACK("PostConstruct", PostConstructCallBack, callback)
//! insert a post post construct callback into this interface
#define DECLARE_POSTPOSTCONSTRUCT_CALLBACK( callback ) DECLARE_CALLBACK("PostPostConstruct", PostPostConstructCallBack, callback)
//! insert a auto construct callback into this interface
#define DECLARE_AUTOCONSTRUCT_CALLBACK( callback ) DECLARE_CALLBACK("AutoConstruct", AutoConstructCallBack, callback)
//! insert a callback for editor select messages
#define DECLARE_EDITORSELECT_CALLBACK( callback ) DECLARE_CALLBACK("EditorSelect", EditorSelectCallBack, callback)
//! insert a callback for editor select parent messages
#define DECLARE_EDITORSELECTPARENT_CALLBACK( callback )	DECLARE_CALLBACK("EditorSelectParent", EditorSelectParentCallBack, callback)
//! insert a callback for editor change value messages
#define DECLARE_EDITORCHANGEVALUE_CALLBACK( callback ) DECLARE_CALLBACK(ntstd::String("EditorChangeValue"), EditorChangeValueCallBack, callback)
//! insert a callback for editor parent messages
#define DECLARE_EDITORCHANGEPARENT_CALLBACK( callback )	DECLARE_CALLBACK("EditorChangeParent", EditorChangeParentCallBack, callback)
//! insert a callback for editor parent messages
#define DECLARE_EDITORDELETEPARENT_CALLBACK( callback )	DECLARE_CALLBACK("EditorDeleteParent", EditorDeleteParentCallBack, callback)
//! insert a callback for debug rendering from the net connected editor messages
#define DECLARE_DEBUGRENDER_FROMNET_CALLBACK( callback ) DECLARE_CALLBACK("DebugRenderNet", DebugRenderNetCallBack, callback)
//! insert a callback for deletion of this object in the objectdatabase.
#define DECLARE_DELETE_CALLBACK( callback ) DECLARE_CALLBACK( "DeleteObject", DeleteCallback, callback );
//! insert a callback for the renaming of this object in the objectdatabase.
#define DECLARE_EDITORRENAME_CALLBACK( callback ) DECLARE_CALLBACK( "EditorRenameObject", EditorRenameCallback, callback );

//--------------------------------------------------------
//--------------------------------------------------------
// User friendly names of the actual macros above
// Use these
//--------------------------------------------------------
//--------------------------------------------------------

// synonyms

//! Publish a variable to the reflection system with the exposed name the same as the variable
#define PUBLISH_VAR( var ) DECLARE_CSTRUCTURE_VARIABLE( var, var, DataInterfaceField::MACRO_DEFAULT_MARKER  )
//! Publish a variable to the reflection system, user defined exposed name
#define PUBLISH_VAR_AS( var, name ) DECLARE_CSTRUCTURE_VARIABLE( var, name, DataInterfaceField::MACRO_DEFAULT_MARKER )
//! Publish a variable to the reflection system, exposed name same as variable, user defined default
#define PUBLISH_VAR_WITH_DEFAULT( var, defaultT ) DECLARE_CSTRUCTURE_VARIABLE( var, var, defaultT  )
//! Publish a variable to the reflection system, user defined exposed name, user defined default
#define PUBLISH_VAR_WITH_DEFAULT_AS( var, defaultT, name ) DECLARE_CSTRUCTURE_VARIABLE( var, name, defaultT )

//! Publish a variable to the reflection system with an accesor
#define PUBLISH_ACCESSOR( type, name, getter, setter ) DECLARE_CSTRUCTURE_VARIABLE_WITH_ACCESSOR( type, name, getter,setter, DataInterfaceField::MACRO_DEFAULT_MARKER )
//! Publish a variable to the reflection system with an accesor with a default
#define PUBLISH_ACCESSOR_WITH_DEFAULT( type, name, getter, setter, defaultT ) DECLARE_CSTRUCTURE_VARIABLE_WITH_ACCESSOR( type, name, getter,setter, defaultT )

//! Publish a pointer variable to the reflection system with an accesor
#define PUBLISH_PTR_ACCESSOR( type, name, getter, setter ) DECLARE_PTR_VARIABLE_WITH_ACCESSOR( type, name, getter, setter, "" )
//! Publish a pointer variable to the reflection system with an accesor with a default
#define PUBLISH_PTR_ACCESSOR_WITH_DEFAULT( type, name, getter, setter, defaultT ) DECLARE_PTR_VARIABLE_WITH_ACCESSOR( type, name, getter, setter, #defaultT )

//! Publish a std container to the reflection system, expose name the same
#define PUBLISH_CONTAINER( var ) DECLARE_CSTRUCTURE_CONTAINER( var, var )
//! Publish a std container to the reflection system, user defined exposed name
#define PUBLISH_CONTAINER_AS( var, name ) DECLARE_CSTRUCTURE_CONTAINER( var, name )

//! Publish a container of datao object to the reflection system
#define PUBLISH_DATAOBJECT_CONTAINER( var ) DECLARE_CSTRUCTURE_DATAOBJECT_CONTAINER( var, var )
//! Publish a container of datao object to the reflection system, user defined exposed name
#define PUBLISH_DATAOBJECT_CONTAINER_AS( var, name ) DECLARE_CSTRUCTURE_DATAOBJECT_CONTAINER( var, name )

//! publish the varaiable is a C++ ptr
#define PUBLISH_PTR( var ) DECLARE_CSTRUCTURE_PTR(var, var, "" )
//! publish the varaiable is a C++ ptr, exposed name is user defined
#define PUBLISH_PTR_AS( var, name ) DECLARE_CSTRUCTURE_PTR(var, name, "" )
//! publish the varaiable is a C++ ptr, user defined default
#define PUBLISH_PTR_WITH_DEFAULT( var, defaultT ) DECLARE_CSTRUCTURE_PTR(var, var, #defaultT )
//! publish the varaiable is a C++ ptr, user defined default,exposed name is user defined
#define PUBLISH_PTR_WITH_DEFAULT_AS( var, name, defaultT ) DECLARE_CSTRUCTURE_PTR(var, name, #defaultT )

//! publish this varible as a container of pointer
#define PUBLISH_PTR_CONTAINER( var ) DECLARE_CSTRUCTURE_PTR_CONTAINER( var, var )
//! publish this varible as a container of pointer, with a custom exposed name
#define PUBLISH_PTR_CONTAINER_AS( var, name ) DECLARE_CSTRUCTURE_PTR_CONTAINER( var, name )

//! publish this variable as a container of pointers to object containers (and normal objects)
#define PUBLISH_DEEP_LIST( var ) DECLARE_CSTRUCTURE_DEEP_LIST( var, var )
//! publish this variable as a container of object containers (and normal objects), with a custom exposed name
#define PUBLISH_DEEP_LIST_AS( var, name ) DECLARE_CSTRUCTURE_DEEP_LIST( var, name )

//--------------------------------------------------------
//! start a local enum (not supported via welder)
#define PUBLISH_ENUM( var ) START_CSTRUCTURE_ENUM( var, var, DataInterfaceField::MACRO_DEFAULT_MARKER  )
//! start a local enum with a custom name
#define PUBLISH_ENUM_AS( var, name ) START_CSTRUCTURE_ENUM( var, name, DataInterfaceField::MACRO_DEFAULT_MARKER )
//! start a local enum with a default
#define PUBLISH_ENUM_WITH_DEFAULT( var, defaultT ) START_CSTRUCTURE_ENUM( var, var, defaultT )
//! start a local enum with a custom name and a default
#define PUBLISH_ENUM_WITH_DEFAULT_AS( var, name, defaultT ) START_CSTRUCTURE_ENUM( var, name, defaultT )
//! put a value into the enum. exposed name is the same as the value
#define PUSH_ENUM_VALUE( value ) DECLARE_ENUM_VALUE( value, value )
//! put a value into the enum. name is user defined
#define PUSH_ENUM_VALUE_AS( value, name ) DECLARE_ENUM_VALUE( value, name )
//! end the local enum
#define END_ENUM	END_CSTRUCTURE_ENUM

//! publish this variable as a global enum, 
#define PUBLISH_GLOBAL_ENUM_AS( var, name, enumname )		\
		PUBLISH_ENUM_AS( var, name )						\
		DECLARE_ENUM_GLOBAL( enumname )						\
		END_ENUM								

//! publish this variable as a global enum, 
#define PUBLISH_GLOBAL_ENUM_WITH_DEFAULT_AS( var, name, enumname, deafultT )		\
		PUBLISH_ENUM_WITH_DEFAULT_AS( var, name, deafultT )						\
		DECLARE_ENUM_GLOBAL( enumname )						\
		END_ENUM								

