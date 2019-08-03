#if !defined( DATAOBJECT_INTERNAL )
#ntError Internal dataobject file don't include manually (use dataobject.h)
#endif

class StdEnumDIF : public DataInterfaceField
{
public:
	typedef unsigned int value_type;
	typedef ntstd::Map<value_type, ntstd::String, ntstd::less<value_type>, Mem::MC_ODB>	enum_container;

	StdEnumDIF(  const CHashedString& name, DataObjectField< value_type >* obj, const value_type& defaultT );

	~StdEnumDIF();

	virtual const ntstd::String GetData( const DataObject* pDO ) const;

	virtual void SetData(  const DataObject* pDO, const ntstd::String& data );

	virtual void EditData( const DataObject* pDO, const ntstd::String& data);

	virtual void SetToDefault( const DataObject* pBase );

	virtual void SetToDefaultPost( DataObject*,InterfaceCallBack * ){}

	virtual void UnSetFromDefault( const DataObject* ){}

	virtual void SetDefault(const ntstd::String& defaultT);

	virtual const ntstd::String GetDefault() const;

	virtual DataInterfaceField* Clone() const;

	virtual const ntstd::String GetMetaData() const;

	virtual CHashedString GetName() const
	{
		return m_FieldName;
	}

	virtual unsigned int GetType() const
	{
		value_type dummy;
		return typeof_num(dummy);
	}

	virtual const ntstd::String GetFieldClass() const
	{
		return "enum";
	}

	virtual bool IsHard()
	{
		return m_pDataObject->IsHard();
	}

	void PushEnumValue( unsigned int value, const ntstd::String& name );

	void SetGlobalMode( const ntstd::String& globalName );

protected:
	virtual const void* GetVoidStarTo( const DataObject* pBase ) const
	{
		return &m_pDataObject->Get( pBase );
	}

	DataObjectField<value_type>*		m_pDataObject;
	CHashedString						m_FieldName;		
	value_type							m_Default;
	enum_container						m_EnumValues;
	ntstd::String						m_GlobalName;
};
