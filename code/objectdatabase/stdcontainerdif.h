#if !defined( DATAOBJECT_INTERNAL )
#ntError Internal dataobject file don't include manually (use dataobject.h)
#endif

template<typename T> class StdContainerDIF : public DataInterfaceField{
public:
	typedef T value_type;
	typedef typename T::iterator iterator;
	typedef typename T::const_iterator const_iterator;
	
	StdContainerDIF(  CHashedString name, DataObjectField< value_type >* obj ) :
		m_FieldName( name ),
		m_pDataObject(obj)
	{
		// compile type check to see if you used the wrong macro (like I did :-( )
		void*	dumA;
		T		dumB;
		static_assert( typeof_num(dumA) != typeof_num(dumB), UsePtrContainer_For_Pointers );
	}

	~StdContainerDIF()
	{
		NT_DELETE_CHUNK( Mem::MC_ODB, m_pDataObject );
	}

	virtual const ntstd::String GetData( const DataObject* pDO ) const
	{
		ntstd::Ostringstream dataStr;
		const value_type& listRef = m_pDataObject->Get( pDO );
		dataStr << listRef.size();
		const_iterator tIt = listRef.begin();
		while( tIt != listRef.end() )
		{
			dataStr << " " << *tIt;
			++tIt;
		}

		return dataStr.str();
	}

	virtual void SetData(  const DataObject* pDO, const ntstd::String& data )
	{
		START_LOAD_TIME_PROFILER_ACC( 8 );

		ntstd::Istringstream dataStr(data);
		unsigned int listSize = 0;
		dataStr >> listSize;
		if (listSize > 0)
		{
			// this check is only relevant if there is actually more to read, ie, the list size is > 0
		ntAssert_p(dataStr.good(), ("Something wrong hapened while reading size in: \"%s\"", data.c_str()));
		}

		value_type& listRef = m_pDataObject->GetToSet( pDO );
		listRef.resize( listSize );
		iterator tIt = listRef.begin();
		uint32_t uiCount = 0;
		while( tIt != listRef.end() )
		{
			typename value_type::value_type elem;
			dataStr >> elem;
			ntAssert_p(dataStr.good()||dataStr.eof(), ("Something wrong hapened while reading element %i in: \"%s\"", uiCount, data.c_str()));
			*tIt = elem;
			++tIt; ++uiCount;
		}

		// check there's no more !
		typename value_type::value_type elem;
		dataStr >> elem;
		ntAssert_p(!dataStr.good(), ("Declared size %i is less than actual size of: \"%s\"", listSize, data.c_str()));
		STOP_LOAD_TIME_PROFILER_ACC( 8 );

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
				// add the following item to the end of this container
				typename value_type::value_type data_value;
				dataStr >> data_value;		
				listRef.insert( listRef.end(), data_value );
				break;
			}
		case 'D':
			{
				// free the specified index
				unsigned int index;
				dataStr >> index;
				iterator lIt = listRef.begin();
				ntstd::advance( lIt, index );
				listRef.erase( lIt );
				break;
			}
		case 'E':
			{
				// index newGUID
				unsigned int index;
				typename value_type::value_type val;
				dataStr >> index >> val;
				
				iterator lIt = listRef.begin();
				ntstd::advance( lIt, index );
				(*lIt) = val;
				break;
			}
		default:
			ntError_p( false, ("Invalid edit command") );
		}
	}


	virtual void SetToDefault( const DataObject* pBase )
	{
		m_pDataObject->SetToDefault(pBase, T() );
	}
	virtual void SetToDefaultPost( DataObject*,InterfaceCallBack * )
	{
	}
	virtual void UnSetFromDefault( const DataObject* pBase )
	{
		m_pDataObject->UnSetFromDefault( pBase );
	}

	// ignored
	virtual void SetDefault( const ntstd::String& )
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
		return NT_NEW_CHUNK(Mem::MC_ODB) StdContainerDIF<T>( m_FieldName, m_pDataObject->Clone() );
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
};
