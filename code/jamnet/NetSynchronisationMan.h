/***************************************************************************************************
*
*	DESCRIPTION		NetMan.cpp
*
*	NOTES			Definition of the network manager class
*
***************************************************************************************************/

#ifndef _NET_SYNCH_H
#define _NET_SYNCH_H

#include "jamnet/netsynchronisationobject.h"
#include "jamnet/netman.h"

typedef NetSynchronisationObject* NetSynchronisationObjectCreationFunc() ;

class NetSynchronisationMan : public Singleton<NetSynchronisationMan>
{
public:
	NetSynchronisationMan();
	~NetSynchronisationMan();

	ntstd::List<NetSynchronisationObject*>::iterator AddObject(NetSynchronisationObject* pObject);
	void RemoveObject(ntstd::List<NetSynchronisationObject*>::iterator it);
	NetSynchronisationObject* RemoveObject(int iID);
	NetSynchronisationObject* GetObject(int iID);

	void RegisterObject(int iType, NetSynchronisationObjectCreationFunc pFunc);
	NetSynchronisationObject* CreateObject(int iType, int iID);

	void AddClient(CSafeRef<NetClient> client);
	void RemoveClient(CSafeRef<NetClient> client);
	NetSynchronisationClient& GetClient(int iID) {ntAssert(iID >= 0 && iID < NetMan::MAX_CLIENTS); return m_aClients[iID];}

	void Syncronise(int iObjectID, int iItemID, char* pData);

	void Update();

private:
	ntstd::List<NetSynchronisationObject*> m_objects;
	NetSynchronisationClient			  m_aClients[NetMan::MAX_CLIENTS];

	//Temp test impl
	NetSynchronisationObjectCreationFunc* m_pCreators[128];
};

#endif // _NET_SYNCH_H
