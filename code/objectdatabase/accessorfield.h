#if !defined( DATAOBJECT_INTERNAL )
#ntError Internal dataobject file don't include manually (use dataobject.h)
#endif

#include "game/commandresult.h"
#include "tbd/TypeSelect.h"

//! accessor field, calls get/set user functions when requested
template<typename T, class Base> 
class AccessorField : public DataObjectField<T>											
{																								
public:			
	typedef T (Base::*Getter)() const;
	typedef void (Base::*Setter)(const T&);
	
	AccessorField( Getter get, Setter set ) :
		m_GetterFunc(get),
		m_SetterFunc(set)
	{
	}

	virtual const T& Get( const DataObject* pBase ) const																
	{		
		Base* obj = (Base*) pBase->GetBasePtr();
		m_Temp = (obj->*m_GetterFunc)();
		return m_Temp;
	}
	virtual void Set( const DataObject* pBase, const T& data )
	{																							
		Base* obj = (Base*) pBase->GetBasePtr();
		(obj->*m_SetterFunc)(data);
		return;
	}

	virtual T& GetToSet( const DataObject* pBase )
	{
		ntError_p(false, ("non const get not supported from AccessorFields") );
		Base* obj = (Base*) pBase->GetBasePtr();
		m_Temp = (obj->*m_GetterFunc)();
		return m_Temp;
	}

	virtual void SetToDefault( const DataObject* pBase, const T& defaultT )
	{
		Set( pBase, defaultT );
	}
	virtual void UnSetFromDefault( const DataObject* )
	{
	}
	virtual DataObjectField<T>* Clone() const
	{
		return NT_NEW_CHUNK(Mem::MC_ODB) AccessorField<T,Base>( m_GetterFunc, m_SetterFunc );
	};
	virtual bool IsHard()
	{
		return true;
	}

private:
	Getter	m_GetterFunc;
	Setter	m_SetterFunc;
	mutable T		m_Temp; //!< used to allow Get to pass back a reference but Getter to return an object (to work with CEntity::GetPosition etc)
};

//! We want to make sure that a HashedString is initialized to default properly
namespace CommandAccessorFieldHelper_BigHelper
{
	template <typename T>
	inline T SetToZero(T)
	{
		return T((uint32_t)0);
	}

	template<>
	inline CHashedString SetToZero(CHashedString)
	{
		return CHashedString();
	}
}

//! special type of accessor field to support calling commands, only has a setter! (the getter is a dummy)
//! this is just a helper to make it easier to do template specialization on the CommandAccessor
template<typename T>
class CommandAccessorFieldHelper : public DataObjectField<T>											
{
public:	
	CommandAccessorFieldHelper() : m_Temp(CommandAccessorFieldHelper_BigHelper::SetToZero(m_Temp))
	{
	}

	virtual const T& Get( const DataObject* pBase ) const
	{
		UNUSED(pBase);
		return m_Temp;
	}

	virtual T& GetToSet( const DataObject* pBase )
	{
		UNUSED(pBase);
		return m_Temp;
	}

	virtual void SetToDefault( const DataObject* pBase, const T& defaultT )
	{
		Set( pBase, defaultT );
	}
	virtual void UnSetFromDefault( const DataObject* )
	{
	}
	virtual bool IsHard()
	{
		return true;
	}

private:
	T				m_Temp; //!< used to pass back 0 data for the dummy getter
};


//! Command accessor that accepts a param. There is a template specialization below which
//! specializes with HasParam is false. Since that class is more specific, THIS class will
//! ONLY be used if HasParam is actually true!
template<typename T, typename T2, bool IsConst, bool HasParam>
class CommandAccessorField : public CommandAccessorFieldHelper<T>											
{																								
public:
	// either initialize with a Setter or a SetterNoInput, arguments to the setter
	// are automatically discarded for the SetterNoInput
	typedef COMMAND_RESULT (T2::*Setter)(const T&);
	typedef COMMAND_RESULT (T2::*SetterConst)(const T&) const;

	// select the type of setter depending on template argument
	typedef typename TypeSelect< IsConst, SetterConst, Setter>::ResultType FuncType;
	
	// select the type of the instance depending on IsConst template argument
	typedef typename TypeSelect< IsConst, const T2*, T2*>::ResultType InstanceType;
	
	CommandAccessorField( FuncType set, InstanceType pInstance ) :
		m_SetterFunc(set), m_pInstance(pInstance)
	{
	}

	virtual void Set( const DataObject* pBase, const T& data )
	{
		UNUSED(pBase);
		(m_pInstance->*m_SetterFunc)(data);
	}
	
	virtual CommandAccessorField<T,T2,IsConst,HasParam>* Clone() const
	{
		return NT_NEW_CHUNK(Mem::MC_ODB) CommandAccessorField<T,T2,IsConst,HasParam>( m_SetterFunc, m_pInstance );
	};
private:
	FuncType		m_SetterFunc;
	InstanceType	m_pInstance;
};

//! Command accessor that accepts a param. This partial specialization is more specific
//! than the above class and will be used when HasParam is false.
template<typename T, typename T2, bool IsConst>
class CommandAccessorField<T,T2,IsConst,false> : public CommandAccessorFieldHelper<T>											
{																								
public:
	// either initialize with a Setter or a SetterNoInput, arguments to the setter
	// are automatically discarded for the SetterNoInput
	typedef COMMAND_RESULT (T2::*SetterNoInput)();
	typedef COMMAND_RESULT (T2::*SetterNoInputConst)() const;

	// select the type of setter depending on IsConst template argument
	typedef typename TypeSelect< IsConst, SetterNoInputConst, SetterNoInput>::ResultType FuncType;
	
	// select the type of the instance depending on IsConst template argument
	typedef typename TypeSelect< IsConst, const T2*, T2*>::ResultType InstanceType;

	CommandAccessorField( FuncType set, InstanceType pInstance ) :
		m_SetterFunc(set), m_pInstance(pInstance)
	{
	}

	virtual void Set( const DataObject* pBase, const T& data )
	{
		UNUSED(pBase);
		UNUSED(data); // discard parameter
		(m_pInstance->*m_SetterFunc)();
	}

	virtual CommandAccessorField<T,T2,IsConst,false>* Clone() const
	{
		return NT_NEW_CHUNK(Mem::MC_ODB) CommandAccessorField<T,T2,IsConst,false>( m_SetterFunc, m_pInstance );
	};
private:
	FuncType		m_SetterFunc;
	InstanceType    m_pInstance ;
};
