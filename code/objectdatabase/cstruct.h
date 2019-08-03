#if !defined( DATAOBJECT_INTERNAL )
#ntError Internal dataobject file don't include manually (use dataobject.h)
#endif

template<class T> class CStruct : public DataObject
{
public:
	// create a CStruct data object from an existing struct in memory
	CStruct( T& ptr, const char* className = "UnknownClass", const ntstd::String& name = DataObject::GetUniqueName() ) :
		DataObject( name, GetObjectGUIDFromName(name), className ),
		m_pBasePtr(&ptr)
	{
	}

	T* operator->()
	{
		return m_pBasePtr;
	}

	const T* operator->() const
	{
		return m_pBasePtr;
	}

	// is this a valid object (has it backing store)
	bool IsValid() const
	{
		return (m_pBasePtr!= 0);
	}

	virtual char* GetPhysicalAddress( unsigned int iOffset ) const
	{
		return( reinterpret_cast<char*>(m_pBasePtr) + iOffset );
	};

	virtual void* GetBasePtr() const
	{
		return m_pBasePtr;
	}

protected:
	T* m_pBasePtr;
};


template<> class CStruct<UnknownClass> : public DataObject
{
public:
	CStruct() :
		DataObject( NameType(), GameGUID(), "UnknownClass" ),
		m_pBasePtr( 0 )
	{
	}

	//! create a CStruct data object from an existing struct in memory
	CStruct( void* ptr, const ntstd::String& name = DataObject::GetUniqueName() , const char* pClassName = "UnknownClass" ) :
	DataObject( ntStr::GetString(name), GetObjectGUIDFromName(name), pClassName ),
		m_pBasePtr( reinterpret_cast<char*>(ptr) )
	{
	}

	//! create a CStruct data object from an existing struct in memory
	CStruct( void* ptr, const ntstd::String& name = DataObject::GetUniqueName() , const GameGUID& guid = GameGUID(), const char* pClassName = "UnknownClass" ) :
		DataObject( ntStr::GetString(name), guid, pClassName ),
		m_pBasePtr( reinterpret_cast<char*>(ptr) )
	{
	}

	//! create from a pointer and GUID, name should come later...
	CStruct( void* ptr, const GameGUID& guid, const char* pClassName = "UnknownClass" ) :
		DataObject( "HorseWithNoName", guid, pClassName ),
		m_pBasePtr( reinterpret_cast<char*>(ptr) )
	{
	}
	// create a forward reference all we know is the guid at this stage, fill in the details when we get them
	CStruct( const GameGUID& guid ) :
		DataObject( "ForwardReference", guid, "UnknownClass" ),
		m_pBasePtr( 0 )	
	{
	}

	// is this a valid object (has it backing store)
	bool IsValid() const
	{
		return (m_pBasePtr!= 0);
	}




	//! convert any CStruct into an unknown class
	template<class U> 
		struct rebind 
		{ 
			typedef CStruct<U> other; 
		};
	//! ctor from related
	template<class U>
		CStruct( const CStruct<U>& other ) :
		DataObject( other.m_Name, other.m_ObjectGUID, other.m_ClassName ),
		m_pBasePtr( reinterpret_cast<char*>(other.m_pBasePtr) )
	{
	}

	virtual char* GetPhysicalAddress( unsigned int iOffset ) const
	{
		return( m_pBasePtr + iOffset );
	};

	void Reset( void* ptr )
	{
		m_pBasePtr = reinterpret_cast<char*>(ptr);
	}

	virtual void* GetBasePtr() const
	{
		return m_pBasePtr;
	}

	friend ntstd::Istream& operator >>(ntstd::Istream &is, CStruct<UnknownClass> &obj)
	{
		is >> obj.m_ObjectGUID;
		return is;
	}

	friend ntstd::Ostream& operator <<(ntstd::Ostream &os, const CStruct<UnknownClass> &obj)
	{
		os << obj.m_ObjectGUID;
		return os;
	}
protected:
	char* m_pBasePtr;
};
