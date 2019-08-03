#if !defined( DATAOBJECT_INTERNAL )
#ntError Internal dataobject file don't include manually (use dataobject.h)
#endif

#include "core/deeplist.h"

class DeepListDIF : public DataInterfaceField
{
public:
	DeepListDIF( CHashedString name, DataObjectField< DeepList >* obj )
		: m_pDataObject( obj ), m_FieldName( name )
	{
	}

	~DeepListDIF()
	{
		NT_DELETE_CHUNK( Mem::MC_ODB, m_pDataObject );
	}

	virtual const ntstd::String GetData( const DataObject* pDO ) const;
	
	virtual void SetData(  const DataObject* pDO, const ntstd::String& data );

	void EditData(  const DataObject* pDO, const ntstd::String& data );

	virtual void SetToDefault( const DataObject* pBase );

	virtual void SetToDefaultPost( DataObject*, InterfaceCallBack* );

	virtual void UnSetFromDefault( const DataObject* pBase );

	virtual void SetDefault(const ntstd::String& );

	virtual const ntstd::String GetDefault() const;

	CHashedString GetName() const;

	const ntstd::String GetMetaData() const;

	virtual unsigned int GetType() const;

	const ntstd::String GetFieldClass() const;

	virtual DataInterfaceField* Clone() const;

	virtual bool IsHard();

protected:
	virtual const void* GetVoidStarTo( const DataObject* pBase ) const;

	void InsertGUID( const DataObject* pDO, DeepList& pDeepList, const GameGUID& guid );

	DeepList defaultList;
	DataObjectField< DeepList >* m_pDataObject;
	CHashedString m_FieldName;
};
