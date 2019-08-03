#if !defined( DATAOBJECT_INTERNAL )
#ntError Internal dataobject file don't include manually (use dataobject.h)
#endif

class DataObjectContainerDIF : public DataInterfaceField
{
public:
	// [scee_st] chunked this list into ODB -- the nodes' overhead looks quite significant now
	typedef ntstd::List<DataObject*, Mem::MC_ODB> value_type;
	
	
	DataObjectContainerDIF(  CHashedString name, DataObjectField< value_type >* obj ) :
		m_pDataObject(obj),
		m_FieldName( name )
	{
	}

	~DataObjectContainerDIF()
	{
		NT_DELETE_CHUNK( Mem::MC_ODB, m_pDataObject );
	}

	virtual const ntstd::String GetData( const DataObject* pDO ) const;

	virtual void SetData(  const DataObject* pDO, const ntstd::String& data );

	virtual void EditData(  const DataObject* pDO, const ntstd::String& data );

	virtual void SetToDefault( const DataObject* pBase )
	{
		m_pDataObject->SetToDefault( pBase, value_type() );
	}
	virtual void UnSetFromDefault( const DataObject* pBase )
	{
		m_pDataObject->UnSetFromDefault( pBase );
	}

	virtual void SetToDefaultPost( DataObject*, InterfaceCallBack* )
	{
	}
	// ignored
	virtual void SetDefault(const ntstd::String&)
	{
	};

	virtual const ntstd::String GetDefault() const
	{
		return "0";
	};

	CHashedString GetName() const													
	{																					
		return m_FieldName;																
	}																					
	const ntstd::String GetMetaData() const												
	{					
		value_type dummy;
		return ntstd::String( typeof_name(dummy) );											
	}																					

	virtual unsigned int GetType() const
	{
		value_type dummy;
		return typeof_num(dummy);
	}

	const ntstd::String GetFieldClass() const
	{
		return "container";
	}
	virtual DataInterfaceField* Clone() const
	{
		return NT_NEW_CHUNK(Mem::MC_ODB) DataObjectContainerDIF( m_FieldName, m_pDataObject->Clone() );
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

	DataObjectField<value_type>*	m_pDataObject;
	CHashedString m_FieldName;	
};

