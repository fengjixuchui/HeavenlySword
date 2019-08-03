#if !defined( DATAOBJECT_INTERNAL )
#ntError Internal dataobject file don't include manually (use dataobject.h)
#endif

//! Helper object to pack a single (non-compound) object from a external interface to a DataObjectField
template< typename T> class SingleItemDIFHelper : public DataInterfaceField
{
public:
	typedef T value_type;
	SingleItemDIFHelper( CHashedString name, DataObjectField<T>* obj, const T& defaultT) :
		m_FieldName( name ),
		m_pDataObject( obj ),
		m_Default( defaultT ),
		m_bUseDefault( true )
	{
	}

	// special marker for macro's
	SingleItemDIFHelper( CHashedString name, DataObjectField<T>* obj, DataInterfaceField::MDM ) :
		m_FieldName( name ),
		m_pDataObject( obj ),
		m_bUseDefault( false )
	{
	}
	~SingleItemDIFHelper()
	{
		NT_DELETE_CHUNK( Mem::MC_ODB, m_pDataObject );
	}

	virtual const ntstd::String GetData( const DataObject* pDO ) const
	{
		ntstd::Ostringstream dataStr;
		dataStr << m_pDataObject->Get( pDO );
		return dataStr.str();
	}

	virtual void SetData(  const DataObject* pDO, const ntstd::String& data )
	{
		START_LOAD_TIME_PROFILER_ACC( 4 );
		ntstd::Istringstream dataStr(data);
		T val;
		dataStr >> val;
		m_pDataObject->Set( pDO, val );
		STOP_LOAD_TIME_PROFILER_ACC( 4 );
	}
	virtual void EditData(  const DataObject* pDO, const ntstd::String& data )
	{
		// set and edit are the same for single items..
		SetData( pDO, data );
	}


	virtual void SetToDefault( const DataObject* pBase )
	{
		if( m_bUseDefault == true )
		{
			m_pDataObject->SetToDefault( pBase, m_Default );
		}
	}
	virtual void SetToDefaultPost( DataObject*,InterfaceCallBack * )
	{
	}
	virtual void UnSetFromDefault( const DataObject* pBase )
	{
		if( m_bUseDefault == true )
		{
			m_pDataObject->UnSetFromDefault( pBase );
		}
	}

	virtual void SetDefault(const ntstd::String& defaultT)
	{
		ntstd::Istringstream dataStr(defaultT);
		dataStr >> m_Default;
	};

	virtual const ntstd::String GetDefault() const
	{
		ntstd::Ostringstream dataStr;
		dataStr << m_Default;
		return dataStr.str();
	};

	DataObjectField<T>* GetDataObjectField()
	{
		return m_pDataObject;
	}

	CHashedString GetName() const													
	{																					
		return m_FieldName;																
	}																					
	const ntstd::String GetMetaData() const												
	{					
		return ntstd::String( typeof_name(m_Default) );											
	}				

	virtual unsigned int GetType() const
	{
		value_type dummy;
		return typeof_num(dummy);
	}

	const ntstd::String GetFieldClass() const
	{
		return "variable";
	}

	virtual bool IsHard()
	{
		return m_pDataObject->IsHard();
	}

protected:
	virtual const void* GetVoidStarTo( const DataObject* pBase ) const
	{
		return &m_pDataObject->Get( pBase );
	}

	CHashedString m_FieldName;		
	DataObjectField<T>*	m_pDataObject;
	T			m_Default;
	bool		m_bUseDefault;
};

// use the help makes it easy to specialize SetData for backwards compatibility
template< typename T> class SingleItemDIF : public SingleItemDIFHelper<T>
{
public:
	typedef T value_type;
	SingleItemDIF( CHashedString name, DataObjectField<T>* obj, const T& defaultT) :
		SingleItemDIFHelper<T>( name, obj, defaultT ){}

	// special marker for macro's
	SingleItemDIF( CHashedString name, DataObjectField<T>* obj, DataInterfaceField::MDM ) :
		SingleItemDIFHelper<T>( name, obj, DataInterfaceField::MACRO_DEFAULT_MARKER ){}

	~SingleItemDIF(){}

	virtual DataInterfaceField* Clone() const
	{
		return NT_NEW_CHUNK(Mem::MC_ODB) SingleItemDIF<T>( this->m_FieldName, this->m_pDataObject->Clone(), this->m_Default );
	}
};

// bool specialization
template<> class SingleItemDIF<bool> : public SingleItemDIFHelper<bool>
{
public:
	typedef bool value_type;
	SingleItemDIF( CHashedString name, DataObjectField<bool>* obj, const bool& defaultT) :
		SingleItemDIFHelper<bool>( name, obj, defaultT ){}

	// special marker for macro's
	SingleItemDIF( CHashedString name, DataObjectField<bool>* obj, DataInterfaceField::MDM ) :
		SingleItemDIFHelper<bool>( name, obj, DataInterfaceField::MACRO_DEFAULT_MARKER ){}

	~SingleItemDIF<bool>(){}

	virtual const ntstd::String GetData( const DataObject* pDO ) const
	{
		// make 100% sure that we convert to ONLY 1 or 0
		return m_pDataObject->Get( pDO ) ? "1" : "0";
	}

	virtual void SetData(  const DataObject* pDO, const ntstd::String& data )
	{
		char cFirstChar = data[0];
		if ( cFirstChar == 'f' || cFirstChar == 'F'  || (cFirstChar == '0' && data[1] == '\0') )
		{
			START_LOAD_TIME_PROFILER_ACC( 5 );
			m_pDataObject->Set( pDO, false );
			STOP_LOAD_TIME_PROFILER_ACC( 5 );
		}
		else
		{
			START_LOAD_TIME_PROFILER_ACC( 5 );
			m_pDataObject->Set( pDO, true );
			STOP_LOAD_TIME_PROFILER_ACC( 5 );
		}
	}
	virtual void SetDefault( const ntstd::String& data )
	{
		char cFirstChar = data[0];
		if ( cFirstChar == 'f' || cFirstChar == 'F'  || (cFirstChar == '0' && data[1] == '\0') )
		{
			m_Default = false;
		}
		else 
		{
			m_Default = true;
		}
	}
	virtual DataInterfaceField* Clone() const
	{
		return NT_NEW_CHUNK(Mem::MC_ODB) SingleItemDIF<bool>( m_FieldName, m_pDataObject->Clone(), m_Default );
	}
};

#include "editable/flipflop.h"

// CFlipFlop specialization
template<> class SingleItemDIF<CFlipFlop> : public SingleItemDIFHelper<CFlipFlop>
{
public:
	typedef bool value_type;
	SingleItemDIF( CHashedString name, DataObjectField<CFlipFlop>* obj, const CFlipFlop& defaultT) :
		SingleItemDIFHelper<CFlipFlop>( name, obj, defaultT ){}

	// special marker for macro's
	SingleItemDIF( CHashedString name, DataObjectField<CFlipFlop>* obj, MDM ) :
		SingleItemDIFHelper<CFlipFlop>( name, obj, DataInterfaceField::MACRO_DEFAULT_MARKER ){}

	~SingleItemDIF<CFlipFlop>(){}

	virtual const ntstd::String GetData( const DataObject* pDO ) const
	{
		return ntstd::String( m_pDataObject->Get( pDO ).ToString() );
	}

	virtual void SetData(  const DataObject* pDO, const ntstd::String& data )
	{
		START_LOAD_TIME_PROFILER_ACC( 6 );
		m_pDataObject->GetToSet( pDO ).Build( data.c_str() );
		STOP_LOAD_TIME_PROFILER_ACC( 6 );
	}

	virtual DataInterfaceField* Clone() const
	{
		return NT_NEW_CHUNK(Mem::MC_ODB) SingleItemDIF<CFlipFlop>( m_FieldName, m_pDataObject->Clone(), m_Default );
	}
};
