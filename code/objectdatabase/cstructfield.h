#if !defined( DATAOBJECT_INTERNAL )
#ntError Internal dataobject file don't include manually (use dataobject.h)
#endif

// forward decl 
class InterfaceCallBack;

//! meta-object encapsulating the editing interface to a single field of data object
class DataInterfaceField
{
public:
	// the types are just the typeof_num(x) of the a varible type
	// only one actual enum value for unknown is explicitly stated
	enum TYPE { OTHER_TYPE = 0xFFFFFFFF };

	enum MDM { MACRO_DEFAULT_MARKER };
	virtual ~DataInterfaceField(){};

	virtual const ntstd::String GetFieldClass() const = 0;

	virtual CHashedString GetName() const = 0;
	virtual const ntstd::String GetMetaData() const = 0;

	virtual unsigned int GetType() const = 0;

	virtual const ntstd::String GetData(  const DataObject* pBase ) const = 0;
	virtual void SetData(  const DataObject* pBase , const ntstd::String& ) = 0;
	virtual void EditData( const DataObject* pBase , const ntstd::String& ) = 0;

	// returns the item as a particular type
	template<typename T>
		const T& GetAs( const DataObject* pBase ) const
	{
		return *(reinterpret_cast<const T*>( GetVoidStarTo(pBase) ) );
	}

	virtual void SetToDefault( const DataObject* pBase ) = 0;
 
	// most things don't won't to do anything here, but true soft field construction should call a dtor to clean up properly
	virtual void UnSetFromDefault( const DataObject* ) = 0;

	virtual bool IsHard() = 0;

	// for pointers, call after loading object attributes this is allowed to create object if it does 
	// it should call the passed in callback with the constructed object in param0. The object should be constructed with DoPost = false. 
	virtual void SetToDefaultPost( DataObject* pBase, InterfaceCallBack* pCallback ) = 0;

	// these get and set the actual default value
	virtual void SetDefault( const ntstd::String& ) = 0;
	virtual const ntstd::String GetDefault() const = 0;

	virtual DataInterfaceField* Clone() const = 0;

protected:
	//! secret source, this is used with some nasty casts for GetAs to work..
	virtual const void* GetVoidStarTo( const DataObject* pBase ) const = 0;

};

//! C interface to an data object field, a simple virtual get/set 
template<typename T> class DataObjectField
{
public:
	virtual ~DataObjectField(){};
	virtual const T& Get( const DataObject* ) const = 0;
	virtual void Set( const DataObject*, const T& ) = 0;
	virtual T& GetToSet( const DataObject* ) = 0;

	virtual void SetToDefault( const DataObject*, const T& ) = 0;
	virtual void UnSetFromDefault( const DataObject* ) = 0;
	virtual DataObjectField<T>* Clone() const = 0;
	virtual bool IsHard() = 0;

};

//! data object field holding a pointer to an int. Used to look inside a structure to a integer type
template<typename T, bool bPlacementNew = false> class CStructField : public DataObjectField<T>											
{																								
public:			
	CStructField( unsigned int iDataOffset) : 
		m_iDataOffset( iDataOffset )
	{
	}

	virtual const T& Get( const DataObject* pBase ) const																
	{																							
		return *(reinterpret_cast<const T*>( pBase->GetPhysicalAddress(m_iDataOffset) ) );																		
	}																							
	virtual void Set( const DataObject* pBase, const T& data )														
	{																							
		*(reinterpret_cast<T*>( pBase->GetPhysicalAddress(m_iDataOffset) ) ) = data;																		
	};	
	virtual T& GetToSet( const DataObject* pBase )
	{
		return *(reinterpret_cast<T*>( pBase->GetPhysicalAddress(m_iDataOffset) ) );
	}
	virtual void SetToDefault( const DataObject* pBase, const T& defaultT )
	{
		if( bPlacementNew )
		{
			NT_PLACEMENT_NEW ( pBase->GetPhysicalAddress(m_iDataOffset) ) T(defaultT);
		} else
		{
			GetToSet(pBase) = defaultT;
		}
	}
	virtual void UnSetFromDefault( const DataObject* pBase )
	{
		if( bPlacementNew )
		{
			((T*)pBase->GetPhysicalAddress(m_iDataOffset))->~T();	
		}
	}

	virtual DataObjectField<T>* Clone() const
	{
		return NT_NEW_CHUNK(Mem::MC_ODB) CStructField<T,bPlacementNew>( m_iDataOffset );
	};

	virtual bool IsHard()
	{
		return !bPlacementNew;
	}

private:
	unsigned int					m_iDataOffset;																				
};

