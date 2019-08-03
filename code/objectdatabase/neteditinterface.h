
#ifndef _NET_EDIT_INTERFACE
#define _NET_EDIT_INTERFACE

#include "core/net.h"
#include "objectdatabase/ObjectChangeTracer.h"


// forward decl
class DataObject;

class CNetEditInterface : public Singleton<CNetEditInterface>, public ObjectChangeTracker
{
public:
	CNetEditInterface();
	~CNetEditInterface();
	bool Startup();	

	void Update();

	void Dispatch(const char* pcCommand);

	DataObject* GetSelected()
	{
		return m_pSelected;
	}

	DataObject* GetSelectedParent()
	{
		return m_pSelectedParent;
	}

	void Reset()
	{
		m_pSelected = 0;
		m_pSelectedParent = 0;
	}

protected:
	typedef ntstd::String StringType;
	typedef ntstd::List<StringType> StringList;

	void Listen();

	bool StartListen();

	static inline const StringType& ExtractKeyString( const StringList& parseList, int iIndex )
	{
		StringList::const_iterator it = parseList.begin();
		ntstd::advance( it, iIndex );
		return *it;
	}

	static inline const char* ExtractString( const StringList& parseList, int iIndex )
	{
		return ntStr::GetString(ExtractKeyString( parseList, iIndex ));
	}

	void NotifyTracker(ntstd::String command);
	void CloseTrackerStream();

	// Editer connection port
	static const int m_iPort = 4321;

	// Flag to indicate connection status
	bool m_bConnected;

	// Connection objects
	netserver server;
	netstream remote;

	// Client connection objects
	netclient client;
	netstream remoteCallback;

	bool m_bEcho;

	DataObject* m_pSelected;
	DataObject* m_pSelectedParent;

	static const int CNetEditInterface::NET_COMPONENT_VERSION;
};



#endif // _NET_EDIT_INTERFACE
