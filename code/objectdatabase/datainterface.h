#if !defined( DATAOBJECT_INTERNAL )
#ntError Internal dataobject file don't include manually (use dataobject.h)
#endif

class DataInterface
{
public:
	typedef ntstd::Vector<DataInterfaceField*, Mem::MC_ODB> container_type;
	typedef container_type::iterator iterator;
	typedef container_type::const_iterator const_iterator;
	typedef ntstd::Vector<ntstd::String, Mem::MC_ODB> CastToContainer;
	typedef CastToContainer::const_iterator	CastToContainerConstIterator;

	DataInterface( const ntstd::String& name, const GameGUID& id );

	virtual ~DataInterface(){};
	virtual const_iterator begin() const = 0;
	virtual const_iterator end() const = 0;

	virtual iterator begin() = 0;
	virtual iterator end() = 0;
	
	virtual CastToContainerConstIterator CastToNamesBegin() const = 0;
	virtual CastToContainerConstIterator CastToNamesEnd() const = 0;


	// object creation and destruction functions
	virtual void* CreateObject() = 0;
	virtual void PlacementCreateObject( void* mem ) = 0;
	virtual void DestroyObject( void* obj) = 0;
	virtual void PlacementDestroyObject( void* obj) = 0;

	virtual unsigned int GetSizeOfObject() = 0;

	virtual bool CallBack( const DataObject* pDO, const ntstd::String& func, CallBackParameter param0 = CHashedString::nullString, CallBackParameter = CHashedString::nullString ) = 0;

	virtual const ntstd::String& GetBaseObjectName() = 0;

	// helper function to may be very slow... (subclasses should optimise)
	virtual void SetData( DataObject* pDO, const CHashedString& name, const ntstd::String& value );

	// helper function to may be very slow... (subclasses should optimise)
	virtual const ntstd::String GetData( DataObject* pDO, const CHashedString& name );

	virtual void EditData( const DataObject* pDO , const CHashedString& name, const ntstd::String& data );

	// helper function to may be very slow... (subclasses should optimise)
	virtual DataInterfaceField* GetFieldByName( const CHashedString& name );

	// helper function to may be very slow... (subclasses should optimise)
	template<typename T>
		T Get( DataObject* pDO, const CHashedString& type )
		{
			DataInterface::iterator diIt = begin();
			while( diIt != end() )
			{
				if( type == (*diIt)->GetName() )
				{
					return (*diIt)->GetAs<T>(pDO);
				}
				++diIt;
			}
			return T();
		}


	const ntstd::String& GetName() const 
	{
		return m_Name;
	}

	const GameGUID& GetGUID() const 
	{
		return m_InterfaceGUID;
	}

	virtual bool IsHard() const
	{
		return true;
	}

	// slighty dogdy guid for now
	static GameGUID GetInterfaceGUIDFromName( const ntstd::String& name );

protected:
	ntstd::String m_Name;
	GameGUID	m_InterfaceGUID;
};


class StdDataInterface : public DataInterface
{
public:
	typedef DataInterface::container_type					container_type;
	typedef container_type::iterator						iterator;
	typedef container_type::const_iterator					const_iterator;
	typedef ntstd::Map<ntstd::String, InterfaceCallBack*, ntstd::less< ntstd::String >, Mem::MC_ODB>	CallBackMap;
	typedef CallBackMap::const_iterator						CallBackConstIterator;

	StdDataInterface(	const ntstd::String& interface_name, const ntstd::String& base_name, const GameGUID& id );
	~StdDataInterface();

	void InheritFields(const StdDataInterface& obOther);

	StdDataInterface* Clone() const;

	const ntstd::String& GetBaseObjectName()
	{
		return m_ObjectName;
	}

	virtual const_iterator begin() const
	{
		return m_Fields.begin();
	};
	virtual const_iterator end() const
	{
		return m_Fields.end();
	};
	virtual iterator begin()
	{
		return m_Fields.begin();
	};
	virtual iterator end()
	{
		return m_Fields.end();
	};

	virtual void AddField(DataInterfaceField* pField);

	void CreateFastIndices();

	virtual void EditData( DataObject* pDO, const CHashedString& name, const ntstd::String& value );

	virtual void SetData( DataObject* pDO, const CHashedString& name, const ntstd::String& value );

	virtual const ntstd::String GetData( DataObject* pDO, const CHashedString& name );

	virtual DataInterfaceField* GetFieldByName( const CHashedString& name );

	template<typename T>
		T Get( DataObject* pDO, const CHashedString& name )
	{
		ntAssert_p( m_NameFieldMap.find( name ) != m_NameFieldMap.end(), ("Attribute does not exist in this interface") );
		return m_NameFieldMap[name]->GetAs<T>( pDO );
	}


	virtual StdDataInterface* CastTo( const ntstd::String& to, DataObject* object );

	virtual bool CallBack( const DataObject* pDO, const ntstd::String& func, CallBackParameter param0 = CHashedString::nullString, CallBackParameter param1 = CHashedString::nullString );

	// these are fairly private access used for parent construction etc, not really for anythings else use
	virtual CallBackConstIterator CallBackBegin() const
	{
		return m_CallBackMap.begin();
	};
	virtual CallBackConstIterator CallBackEnd() const
	{
		return m_CallBackMap.end();
	};

	virtual CastToContainerConstIterator CastToNamesBegin() const
	{
		return m_CastToNames.begin();
	};
	virtual CastToContainerConstIterator CastToNamesEnd() const
	{
		return m_CastToNames.end();
	};

protected:
	typedef ntstd::Vector<DataInterfaceField*, Mem::MC_ODB> FieldContainer;
	typedef ntstd::Map<CHashedString, DataInterfaceField*, ntstd::less<CHashedString>, Mem::MC_ODB>	NameFieldMap;

	FieldContainer			m_Fields;
	CastToContainer			m_CastToNames;
	NameFieldMap			m_NameFieldMap;
	CallBackMap				m_CallBackMap;
	ntstd::String			m_ObjectName;
};
