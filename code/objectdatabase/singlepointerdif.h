#if !defined( DATAOBJECT_INTERNAL )
#ntError Internal dataobject file don't include manually (use dataobject.h)
#endif

template<typename T>
class SinglePointerDIF : public DataInterfaceField
{
public:
	typedef T value_type;

	SinglePointerDIF( const CHashedString& name, DataObjectField<value_type>* obj, ntstd::String autoConstruct ) :
									m_FieldName( name ),
									m_pDataObject( obj ),
									m_bAutoConstruct( autoConstruct )
	{
	}

	~SinglePointerDIF()
	{	
		NT_DELETE_CHUNK( Mem::MC_ODB, m_pDataObject );
	}

	virtual const ntstd::String GetData( const DataObject* pDO ) const
	{	
		ntstd::Ostringstream dataStr;
		DataObject* pPointedTo = ObjectDatabase::Get().GetDataObjectFromPointer( m_pDataObject->Get(pDO) );
		GameGUID guid;
		if( pPointedTo != 0 )
		{
			guid = pPointedTo->GetGUID();
		}
		dataStr << guid;
		return dataStr.str();
	}

	virtual void SetData(  const DataObject* pDO, const ntstd::String& data )
	{
		START_LOAD_TIME_PROFILER_ACC( 7 );

		START_LOAD_TIME_PROFILER_ACC( 70 );
		ntstd::Istringstream dataStr(data);
		GameGUID guid;
		dataStr >> guid;
		STOP_LOAD_TIME_PROFILER_ACC( 70 );

		START_LOAD_TIME_PROFILER_ACC( 700 );
		// if the guid was null do nothing rely on the SetToDefault doing its job
		if( guid.IsNull() )
		{

			// if we have a string it may be a object name we can only back reference
			// off named objects though so no fixups aloud
			DataObject* pAlreadyDO = ObjectDatabase::Get().GetDataObjectFromName( CHashedString(data) );
			if( pAlreadyDO != 0 )
			{
				// object exists and is ready to go
				m_pDataObject->Set( pDO, (value_type)pAlreadyDO->GetBasePtr() );

				STOP_LOAD_TIME_PROFILER_ACC( 7 );
				STOP_LOAD_TIME_PROFILER_ACC( 700 );

				return;

			} else
			{
				// this was never meant to happen but for backward compatiblity...
				// we have name forward reference
				guid = DataObject::GetObjectGUIDFromName(data);
				if( guid.IsNull() )
				{
					m_pDataObject->Set( pDO, 0 );
					STOP_LOAD_TIME_PROFILER_ACC( 7 );
					STOP_LOAD_TIME_PROFILER_ACC( 700 );
					return;
				}
			}
		}
		STOP_LOAD_TIME_PROFILER_ACC( 700 );

		// we have a valid GUID lets ask the object database for a data object to it
		START_LOAD_TIME_PROFILER_ACC( 71 );
		DataObject* pNewDO = ObjectDatabase::Get().GetObjectOrForwardReference( guid );
		STOP_LOAD_TIME_PROFILER_ACC( 71 );

		START_LOAD_TIME_PROFILER_ACC( 74 );
		const bool isValid = pNewDO->IsValid();
		STOP_LOAD_TIME_PROFILER_ACC( 74 );
		if( !isValid )
		{
			START_LOAD_TIME_PROFILER_ACC( 72 );
			// object is nothing but a forward reference the object itself doesn't 
			// exist so we store the DO object and register to the database we want
			// a fixup when the object really appears
			m_pDataObject->Set( pDO, 0 );
			ntAssert_p( pNewDO->GetFixupList() != 0, ("Eek no fixup list") );
			pNewDO->AddFixup( (void**) &m_pDataObject->Get( pDO ) );
			pNewDO->SetName( DataObject::NameType("Referencer = " + ntstd::String(ntStr::GetString(pDO->GetName()))) );
			STOP_LOAD_TIME_PROFILER_ACC( 72 );
		} else
		{
			START_LOAD_TIME_PROFILER_ACC( 73 );
			// object exists and is ready to go
			m_pDataObject->Set( pDO, (value_type)pNewDO->GetBasePtr() );
			STOP_LOAD_TIME_PROFILER_ACC( 73 );
		}

		STOP_LOAD_TIME_PROFILER_ACC( 7 );
	}

	virtual void EditData(  const DataObject* pDO, const ntstd::String& data )
	{
		// set and edit are the same for single pointers..
		SetData( pDO, data );
	}

	virtual void SetToDefault( const DataObject* pBase )
	{
		m_pDataObject->SetToDefault( pBase, 0 );
	}

	virtual void UnSetFromDefault( const DataObject* pBase )
	{
		m_pDataObject->UnSetFromDefault( pBase );
	}

	// special case of default setting after loading data
	virtual void SetToDefaultPost( DataObject* pBase, InterfaceCallBack* pCallback )
	{
		if( !m_bAutoConstruct.empty() && m_pDataObject->Get(pBase) == 0 )
		{
			// change of behavior to support existing XML files, before autoconstructing check that and object of that name exists
			// if it does point this element to it.
			// ALEXEY_TODO
			// But first, check to see if the default is actually a GUID, otherwise the set-to-default has different behaviour than
			// setting the data normally!
			// - VINCENT
			ntstd::Istringstream autoStr(m_bAutoConstruct);
			GameGUID guid;
			autoStr >> guid;
			DataObject* pAlreadyDO = 0;
			if (!guid.IsNull())
			{
				pAlreadyDO = ObjectDatabase::Get().GetDataObjectFromGUID(guid);
			}
			if (pAlreadyDO == 0)
			{
				pAlreadyDO = ObjectDatabase::Get().GetDataObjectFromName( CHashedString(m_bAutoConstruct) );
			}

			if( pAlreadyDO == 0 || !pAlreadyDO->IsValid() )
			{
				ObjectContainer* pParentContainer = pBase->GetParent();
				if(pCallback)
				{
					DataObject* pNewDO = ObjectDatabase::Get().ConstructObject( m_bAutoConstruct.c_str(), DataObject::GetUniqueName(), GameGUID(), pParentContainer, true, false  );
					m_pDataObject->Set( pBase, (value_type)pNewDO->GetBasePtr() );
					pCallback->CallBack( pBase, (char*) pNewDO, m_FieldName );
				} else
				{
					m_pDataObject->Set( pBase, (value_type)ObjectDatabase::Get().ConstructObject( m_bAutoConstruct.c_str(), DataObject::GetUniqueName(),GameGUID(), pParentContainer  )->GetBasePtr() );
				}
			} else
			{
				m_pDataObject->Set( pBase, (value_type)pAlreadyDO->GetBasePtr() );
			}
		}
	}

	virtual void SetDefault(const ntstd::String& defaultT)
	{
		m_bAutoConstruct = defaultT;
	}

	virtual const ntstd::String GetDefault() const
	{
		return m_bAutoConstruct;
	}

	DataObjectField<void*>* GetDataObjectField()
	{
		return m_pDataObject;
	}

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
		return "ptr";
	}

	virtual DataInterfaceField* Clone() const
	{
		return NT_NEW_CHUNK(Mem::MC_ODB) SinglePointerDIF<value_type>( m_FieldName, m_pDataObject->Clone(),m_bAutoConstruct );
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
	DataObjectField<value_type>*	m_pDataObject;
	ntstd::String			m_bAutoConstruct;
};
