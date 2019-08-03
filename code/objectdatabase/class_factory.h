//------------------------------------------------------
//!
//!	\file core\class_factory.h
//!
//------------------------------------------------------
#if !defined(CORE_CLASS_FACTORY_H)
#define CORE_CLASS_FACTORY_H


//------------------------------------------------------
//!
//! Mad template thing...
//!
//------------------------------------------------------
typedef const char* (*ClassHelperCrazyFunc)();


//------------------------------------------------------
//!
//!	ClassFactoryHelper Used by the interface system
//! so that interfaces can be defined anywhere and its 
//! just works
//!
//------------------------------------------------------
template<class T, ClassHelperCrazyFunc T0>
class ClassFactoryHelper : public ClassFactoryHelperBase
{
public:
	friend class ClassFactory;
	ClassFactoryHelper();

	void Destroy();

	virtual DataInterface* GetInterface()
	{
		if( m_pInterface == 0 )
		{
			m_pInterface = NT_NEW_CHUNK(Mem::MC_ODB) T;
		}
		return m_pInterface;
	};

	virtual const char* GetName()
	{
		return T0();
	}
private:
	DataInterface* m_pInterface;
};

//------------------------------------------------------
//!
//! A completely static system for creating classes (used
//! by interfaces only). No memory allocations occur
//! There is a fixed number of classes possible
//!
//------------------------------------------------------
class ClassFactory
{
public:
	friend class ObjectDatabase;

	static void RegisterClassConstructor(const char* pcString, ClassFactoryHelperBase* base );

	// say bye bye to everything...
	static void DestroyHelpers();
private:
	static unsigned int GenerateHash( const char* pcString )
	{
		unsigned int uiTag = 0;

		while( *pcString )
		{
			uiTag = ( uiTag << 7 ) + uiTag;
			uiTag = uiTag + *pcString++;
		}
		return( uiTag );
	}

	static const int MAX_CLASSES = 4096;

	static struct ClassTable
	{
		unsigned int	m_Hash;
		ClassFactoryHelperBase*	m_CreationHelper;
	} m_ClassConstructors[MAX_CLASSES];

	static unsigned int m_NumClassCons;
};

template<class T, ClassHelperCrazyFunc T0>
inline 	ClassFactoryHelper<T,T0>::ClassFactoryHelper() :
	m_pInterface(0)
{
	ClassFactory::RegisterClassConstructor(T0(), this );
}

template<class T, ClassHelperCrazyFunc T0>
inline void ClassFactoryHelper<T,T0>::Destroy()
{
	if( m_pInterface != 0 )
	{
		NT_DELETE_CHUNK( Mem::MC_ODB, m_pInterface );
		m_pInterface = 0;
	}
}


// we need to use compiler specific selectany/weak linking. 
// If a specific compiler doesn't have this kind of extension we will need a cpp
// that does all the actual registering

#if defined(_MSC_VER)

#define REGISTER_INTERFACE( name )												\
inline const char* name##_Name_Of_InterfaceFunc(){ return #name; }						\
__declspec(selectany) ClassFactoryHelper<name, &name##_Name_Of_InterfaceFunc> name##_Interface_Factory_Helper;

#elif defined( __GCC__ )

#define REGISTER_INTERFACE( name )												\
inline const char* name##_Name_Of_InterfaceFunc(){ return #name; }						\
__attribute((weak)) ClassFactoryHelper<name, &name##_Name_Of_InterfaceFunc> name##_Interface_Factory_Helper;

#endif


#endif // end CORE_CLASS_FACTORY_H
