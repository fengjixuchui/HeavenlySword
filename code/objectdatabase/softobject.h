#if !defined( DATAOBJECT_INTERNAL )
#ntError Internal dataobject file don't include manually (use dataobject.h)
#endif

#define SOFTOBJECTFACTORYINTERFACE_GUID GameGUID(0x3228bc29, 0xc73f, 0x479d, 0xaa, 0xe8, 0x14, 0x1, 0x50, 0x7, 0x91, 0x5a )
class SoftObjectFactoryInterface : public ClassFactoryHelperBase, public StdDataInterface
{
public:
	//! this ctor will be used for the main interface
	SoftObjectFactoryInterface();

	//! this ctor will be used a new soft object interface is constructed
	SoftObjectFactoryInterface( const ntstd::String& pSoftObjectName, const ntstd::String& pParentClass );

	~SoftObjectFactoryInterface();

	void Destroy();

	//! just return the current interface
	virtual DataInterface* GetInterface()
	{
		return this;
	}

	//! Create a new soft object with enough space to hold all the property currently defined
	virtual void* CreateObject();

	virtual void PlacementCreateObject( void* pMem );

	//! destroy a soft object of the 
	virtual void DestroyObject( void* obj );

	virtual void PlacementDestroyObject( void* pMem );

	SoftObjectFactoryInterface* CreateFactoryFor( const ntstd::String& pSoftObjectName, const ntstd::String& pParentClass );

	void AddProperty( const CHashedString& name, const ntstd::String& type, const ntstd::String& defaultVal );

	virtual unsigned int GetSizeOfObject()										
	{
		return m_iCurrentSize;
	}

	virtual bool IsHard() const
	{
		return false;
	}

	virtual const char* GetName()
	{
		return StdDataInterface::GetName().c_str();
	}
private:
	ntstd::String m_pParentClass;
	int m_iCurrentSize;
	void PushField( const CHashedString& name, int typeof_number, const ntstd::String& defaultVal );
};
