#if !defined( DATAOBJECT_INTERNAL )
#ntError Internal dataobject file don't include manually (use dataobject.h)
#endif

template<class T>
class PointerContainerDIF : public DataInterfaceField
{
public:
	typedef T value_type;
	PointerContainerDIF(  const CHashedString& name, DataObjectField< value_type >* obj ) :
		m_pDataObject( obj ),
		m_FieldName( name )
	{
	}

	~PointerContainerDIF()
	{
		NT_DELETE_CHUNK( Mem::MC_ODB, m_pDataObject );
	}

	virtual const ntstd::String GetData( const DataObject* pDO ) const
	{
		ntstd::Ostringstream dataStr;
		int dataValues = 0;
		value_type& listRef = m_pDataObject->GetToSet( pDO );
		typename value_type::iterator tIt = listRef.begin();
		while( tIt != listRef.end() )
		{
			DataObject* pPointedTo = ObjectDatabase::Get().GetDataObjectFromPointer( *tIt );
			GameGUID guid;
			if( pPointedTo != 0 && !pPointedTo->GetGUID().IsNull() )
			{
				dataValues++;
				dataStr << " " << pPointedTo->GetGUID();
			}
			else
			{
				// we've got an invalid object in the list, but we need to be careful about removing it;
				// for the moment, let's just log it
				ntPrintf("'%s' in '%s' has an invalid object in its list, skipping\n", ntStr::GetString(m_FieldName), ntStr::GetString(pDO->GetName()));
			}
			++tIt;
		}

		ntstd::Ostringstream resultStr;
		resultStr << dataValues;
		resultStr << dataStr.str();
		return resultStr.str();
	}

	virtual void SetData(  const DataObject* pDO, const ntstd::String& data )
	{
		START_LOAD_TIME_PROFILER_ACC( 3 );
	
		ntstd::Istringstream dataStr(data);
		unsigned int listSize = 0;
		dataStr >> listSize;

		value_type& listRef = m_pDataObject->GetToSet( pDO );
		for( unsigned int i=0;i < listSize;i++)
		{
			GameGUID guid;
			dataStr >> guid;
			InsertGUID( pDO, listRef, guid );
		}
		STOP_LOAD_TIME_PROFILER_ACC( 3);
	}
	void EditData(  const DataObject* pDO, const ntstd::String& data )
	{
		ntstd::Istringstream dataStr(data);
		value_type& listRef = m_pDataObject->GetToSet( pDO );

		char command = 0;
		dataStr >> command;

		switch( command )
		{
		case 'A':
			{
				// add the following guid/dataobject to the end of this container
				GameGUID guid;
				dataStr >> guid;
				InsertGUID( pDO, listRef, guid );
				break;
			}

		case 'D':
			{
				// free the specified index
				GameGUID guid;
				dataStr >> guid;
				DataObject* pDO = ObjectDatabase::Get().GetObjectOrForwardReference( guid );
				listRef.remove( pDO->GetBasePtr() );
				break;
			}
		case 'E':
			{
				// index newGUID
				unsigned int index;
				GameGUID newGuid;
				dataStr >> index >> newGuid;
				// free the specified index
				typename value_type::iterator lIt = listRef.begin();
				ntstd::advance( lIt , index );
				typename value_type::value_type pNewPointedTo = ObjectDatabase::Get().GetPointerFromGUID<typename value_type::value_type>( newGuid );
				if( lIt != listRef.end() &&  pNewPointedTo != 0 )
				{
					(*lIt) = pNewPointedTo;
				}
				break;
			}
		default:
			ntError_p( false, ("Invalid edit command") );
		}
	}

	virtual void SetToDefault( const DataObject* pBase )
	{
		value_type dummy;
		m_pDataObject->SetToDefault(pBase, dummy );
	}
	virtual void SetToDefaultPost( DataObject*, InterfaceCallBack* )
	{
	}
	virtual void UnSetFromDefault( const DataObject* pBase )
	{
		m_pDataObject->UnSetFromDefault( pBase );
	}


	// ignored 
	virtual void SetDefault(const ntstd::String& )
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
		return NT_NEW_CHUNK(Mem::MC_ODB) PointerContainerDIF<T>( m_FieldName, m_pDataObject->Clone() );
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

	void InsertGUID(  const DataObject* pDO, value_type& listRef, const GameGUID& guid )
	{
		// if the guid was null do nothing rely on the SetToDefault doing its job
		if( guid.IsNull() )
		{
			listRef.insert( listRef.end(), 0 );
		} else
		{
			// we have a valid GUID lets ask the object database for a data object to it
			DataObject* pNewDO = ObjectDatabase::Get().GetObjectOrForwardReference( guid );
			if( !pNewDO->IsValid() )
			{
				// object is nothing but a forward reference the object itself doesn't 
				// exist so we store the DO object and register to the database we want
				// a fixup when the object really appears
				listRef.insert( listRef.end(), 0 );
				ntAssert_p( pNewDO->GetFixupList() != 0, ("Eek no fixup list") );
				pNewDO->AddFixup( (void**)&listRef.back() );
				// ALEXEY_TODO: horroble horror, make sure it happens only at load time and then remove as soon as we get rid of the xml format
				pNewDO->SetName( ntStr::GetString(ntstd::String("Referer = " + ntstd::String( ntStr::GetString(pDO->GetName())))) );
			} else
			{
				// object exists and is ready to go
				listRef.insert( listRef.end(), (typename value_type::value_type)pNewDO->GetBasePtr() );
			}
		}
	}

	DataObjectField<value_type>*	m_pDataObject;
	CHashedString m_FieldName;	
};
