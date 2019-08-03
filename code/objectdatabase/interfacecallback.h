#if !defined( DATAOBJECT_INTERNAL )
#ntError Internal dataobject file don't include manually (use dataobject.h)
#endif

//! a helper for callback
class InterfaceCallBack
{																								
public:		
	// this signature covers a fair few callbacks (I know I could do some tricky templates but bugger off). Just ignore any you don't care about
	virtual bool CallBack( const DataObject* pBase, CallBackParameter param0 = NULL, CallBackParameter param1 = NULL) = 0;
	virtual InterfaceCallBack* Clone() = 0;
	virtual ~InterfaceCallBack() {};
};

//!--------------------------
//! A helper class for a PostConstruct callback
//---------------------------
template<class Base>
class PostConstructCallBack : public InterfaceCallBack
{
public:
	//! no parameters just get called on the object when its constructed
	typedef void (Base::*PostConstruct)();
	
	PostConstructCallBack( PostConstruct callback ) :
		m_CallBack(callback)
	{
	}

	bool CallBack( const DataObject* pBase, CallBackParameter param0 = NULL, CallBackParameter param1 = NULL)
	{		
		UNUSED(param0); UNUSED(param1); // NOT USED
		Base* obj = (Base*) pBase->GetBasePtr();
		(obj->*m_CallBack)();
		return true;
	}

	virtual InterfaceCallBack* Clone()
	{
		return NT_NEW_CHUNK(Mem::MC_ODB) PostConstructCallBack<Base>( m_CallBack );
	}
private:
	PostConstruct m_CallBack;

};

//!--------------------------
//! A helper class for a PostPostConstruct callback
//---------------------------
template<class Base>
class PostPostConstructCallBack : public InterfaceCallBack
{
public:
	//! no parameters just get called on the object when its constructed after all the other entities have had PostConstruct called
	typedef void (Base::*PostPostConstruct)();
	
	PostPostConstructCallBack( PostPostConstruct callback ) :
		m_CallBack(callback)
	{
	}

	bool CallBack( const DataObject* pBase, CallBackParameter param0 = NULL, CallBackParameter param1 = NULL )
	{		
		UNUSED(param0); UNUSED(param1); // NOT USED
		Base* obj = (Base*) pBase->GetBasePtr();
		(obj->*m_CallBack)();
		return true;
	}

	virtual InterfaceCallBack* Clone()
	{
		return NT_NEW_CHUNK(Mem::MC_ODB) PostPostConstructCallBack<Base>( m_CallBack );
	}
private:
	PostPostConstruct m_CallBack;

};

//!--------------------------
//! A helper class for a AutoConstruct callback
//---------------------------
template<class Base>
class AutoConstructCallBack : public InterfaceCallBack
{
public:
	//! called on the parent object when an auto construct (object default construction occurs), 
	//! 1st param the field thats being created into, 2nd the object thats going in there
	typedef void (Base::*AutoConstruct)( const DataInterfaceField* pField );
	
	AutoConstructCallBack( AutoConstruct callback ) :
		m_CallBack(callback)
	{
	}

	bool CallBack( const DataObject* pBase, CallBackParameter param0 = NULL, CallBackParameter param1 = NULL )
	{		
		Base* parent = (Base*) pBase->GetBasePtr();
		DataInterfaceField* pField = (DataInterfaceField*) param0.GetPtr();
		UNUSED(param1); // NOT USED
		(parent->*m_CallBack)( pField );
		return true;
	}
	virtual InterfaceCallBack* Clone()
	{
		return NT_NEW_CHUNK(Mem::MC_ODB) AutoConstructCallBack<Base>( m_CallBack );
	}

private:
	AutoConstruct m_CallBack;

};

//!--------------------------
//! A helper class for an EditorSelect callback
//---------------------------

template<class Base>
class EditorSelectCallBack : public InterfaceCallBack
{
public:
	//! called when an editor selected or deselects an object
	typedef void (Base::*EditorSelect)( bool bSelect );
	
	EditorSelectCallBack( EditorSelect callback ) :
		m_CallBack(callback)
	{
	}

	bool CallBack( const DataObject* pBase, CallBackParameter param0 = NULL, CallBackParameter param1 = NULL )
	{
		UNUSED(param1); // NOT USED
		Base* obj = (Base*) pBase->GetBasePtr();
		bool bSelect = param0.GetInt();
		(obj->*m_CallBack)( bSelect );
		return true;
	}
	virtual InterfaceCallBack* Clone()
	{
		return NT_NEW_CHUNK(Mem::MC_ODB) EditorSelectCallBack<Base>( m_CallBack );
	}

private:
	EditorSelect m_CallBack;

};

//!--------------------------
//! A helper class for an EditorSelectParent callback
//---------------------------

template<class Base>
class EditorSelectParentCallBack : public InterfaceCallBack
{
public:
	//! called on the parent object when an editor selected or deselects a child object
	typedef void (Base::*EditorSelectParent)( bool bSelect );
	
	EditorSelectParentCallBack( EditorSelectParent callback ) :
		m_CallBack(callback)
	{
	}

	bool CallBack( const DataObject* pBase, CallBackParameter param0 = NULL, CallBackParameter param1 = NULL )
	{
		UNUSED(param1); // NOT USED
		Base* obj = (Base*) pBase->GetBasePtr();
		bool bSelect = param0.GetInt(); // !!((intptr_t)param0);
		(obj->*m_CallBack)( bSelect );
		return true;
	}
	virtual InterfaceCallBack* Clone()
	{
		return NT_NEW_CHUNK(Mem::MC_ODB) EditorSelectParentCallBack<Base>( m_CallBack );
	}

private:
	EditorSelectParent m_CallBack;

};

//!--------------------------
//! A helper class for an EditorChangeValue callback
//---------------------------

template<class Base>
class EditorChangeValueCallBack : public InterfaceCallBack
{
public:
	//! called on an object just before a field is changed, if false is return the default change behavior will be stopped 
	typedef bool (Base::*EditorChangeValue)( CallBackParameter pFieldName, CallBackParameter pValue );
	
	EditorChangeValueCallBack( EditorChangeValue callback ) :
		m_CallBack(callback)
	{
	}

	bool CallBack( const DataObject* pBase, CallBackParameter param0 = NULL, CallBackParameter param1 = NULL )
	{
		Base* obj = (Base*) pBase->GetBasePtr();
		return (obj->*m_CallBack)( param0, param1 );
	}
	virtual InterfaceCallBack* Clone()
	{
		return NT_NEW_CHUNK(Mem::MC_ODB) EditorChangeValueCallBack<Base>( m_CallBack );
	}

private:
	EditorChangeValue m_CallBack;

};

//!--------------------------
//! A helper class for an EditorChangeParent callback
//---------------------------

template<class Base>
class EditorChangeParentCallBack : public InterfaceCallBack
{
public:
	//! called on the parent object when an field has been change
	typedef void (Base::*EditorChangeParent)();
	
	EditorChangeParentCallBack( EditorChangeParent callback ) :
		m_CallBack(callback)
	{
	}

	bool CallBack( const DataObject* pBase, CallBackParameter param0 = NULL, CallBackParameter param1 = NULL )
	{
		UNUSED(param0); UNUSED(param1); // NOT USED
		Base* obj = (Base*) pBase->GetBasePtr();
		(obj->*m_CallBack)();
		return true;
	}
	virtual InterfaceCallBack* Clone()
	{
		return NT_NEW EditorChangeParentCallBack<Base>( m_CallBack );
	}

private:
	EditorChangeParent m_CallBack;

};

//!--------------------------
//! A helper class for an EditorDeleteParent callback
//---------------------------

template<class Base>
class EditorDeleteParentCallBack : public InterfaceCallBack
{
public:
	//! called on the parent object when a list item has been deleted...
	typedef void (Base::*EditorDeleteParent)();
	
	EditorDeleteParentCallBack( EditorDeleteParent callback ) :
		m_CallBack(callback)
	{
	}

	bool CallBack( const DataObject* pBase, CallBackParameter param0 = NULL, CallBackParameter param1 = NULL )
	{
		UNUSED(param0); UNUSED(param1); // NOT USED
		Base* obj = (Base*) pBase->GetBasePtr();
		(obj->*m_CallBack)();
		return true;
	}
	virtual InterfaceCallBack* Clone()
	{
		return NT_NEW_CHUNK(Mem::MC_ODB) EditorDeleteParentCallBack<Base>( m_CallBack );
	}

private:
	EditorDeleteParent m_CallBack;

};

//!--------------------------
//! A helper class for an delete callback
//!--------------------------
template < class Base >
class DeleteCallback : public InterfaceCallBack
{
	public:
		typedef void ( Base::*DeleteObject )();

		DeleteCallback( DeleteObject callback )
		:	m_Callback( callback )
		{}

		bool CallBack( const DataObject *base, CallBackParameter param0 = NULL, CallBackParameter param1 = NULL )
		{
			UNUSED( param0 );
			UNUSED( param1 );

			Base *obj = (Base *)base->GetBasePtr();
			(obj->*m_Callback )();
			return true;
		}

		virtual InterfaceCallBack *Clone()
		{
			return NT_NEW_CHUNK(Mem::MC_ODB) DeleteCallback< Base >( m_Callback );
		}

	private:
		DeleteObject m_Callback;
};

//!--------------------------
//! A helper class for an delete callback
//!--------------------------
template < class Base >
class EditorRenameCallback : public InterfaceCallBack
{
	public:
		typedef bool ( Base::*RenameObject )( CallBackParameter );

		EditorRenameCallback( RenameObject callback )
		:	m_Callback( callback )
		{}

		bool CallBack( const DataObject *base, CallBackParameter param0 = NULL, CallBackParameter param1 = NULL )
		{
			UNUSED( param1 );

			Base *obj = (Base *)base->GetBasePtr();
			return (obj->*m_Callback )(param0);
		}

		virtual InterfaceCallBack *Clone()
		{
			return NT_NEW_CHUNK(Mem::MC_ODB) EditorRenameCallback< Base >( m_Callback );
		}

	private:
		RenameObject m_Callback;
};

//!--------------------------
//! A helper class for an DebugRenderNet callback
//---------------------------

template<class Base>
class DebugRenderNetCallBack : public InterfaceCallBack
{
public:
	//! called on the parent object when a list item has been deleted...
	typedef void (Base::*DebugRenderNet)();
	
	DebugRenderNetCallBack( DebugRenderNet callback ) :
		m_CallBack(callback)
	{
	}

	bool CallBack( const DataObject* pBase, CallBackParameter param0 = NULL, CallBackParameter param1 = NULL )
	{
		UNUSED(param0); UNUSED(param1); // NOT USED
		Base* obj = (Base*) pBase->GetBasePtr();
		(obj->*m_CallBack)();
		return true;
	}
	virtual InterfaceCallBack* Clone()
	{
		return NT_NEW_CHUNK(Mem::MC_ODB) DebugRenderNetCallBack<Base>( m_CallBack );
	}

private:
	DebugRenderNet m_CallBack;

};
