/***************************************************************************************************
*
*	DESCRIPTION		NetMan.cpp
*
*	NOTES			Definition of the network manager class
*
***************************************************************************************************/

#include "jamnet/netsynchronisationman.h"

//////////////////////////////////////////////////////
// Register our Network Messages
//////////////////////////////////////////////////////
REGISTER_NETMSG(NetMsgSyncVar);
REGISTER_NETMSG(NetMsgSyncKillObj);

/*// Testing testing testing
class CSyncTest* gObj = 0;

class CSyncTest : public NetSynchronisationObject
{
public:
	CSyncTest() {m_iID = m_iNextID++;
				 AddSynchronisationData(0, NT_NEW NetSynchronisationVar<int>(&m_i));
				 AddSynchronisationData(1, NT_NEW NetSynchronisationVar<bool>(&m_b));
				 SetInfluenceCondition(NT_NEW NetInfluenceConditionAlways());}

	~CSyncTest()
	{
		if(IsMaster())
		{
			NetMsgSyncKillObj obMsg(GetID());
			NetMan::SendAll(obMsg);
			NetSynchronisationMan::Get().RemoveObject(GetID());
		}

		gObj = 0;
	}

	int Get() {return m_i;}
	void Set(int i) {m_i = i;}

	bool Get2() {return m_b;}
	void Set2(bool b) {m_b = b;}

	virtual unsigned int GetTypeID() {return m_iTypeID;}
	virtual unsigned int GetID() {return m_iID;}

	static void Register()
	{
		NetSynchronisationMan::Get().RegisterObject(m_iTypeID, CreateLocal);
	}

	static NetSynchronisationObject* CreateLocal()
	{
		gObj = NT_NEW CSyncTest();
		return gObj;
	}

private:
	static const unsigned int m_iTypeID = 0;
	static unsigned int m_iNextID;
	unsigned int m_iID;

	int m_i;
	bool m_b;
	
	
};

unsigned int CSyncTest::m_iNextID = 0;
*/
NetSynchronisationMan::NetSynchronisationMan()
{
	memset(m_pCreators, 0, sizeof(m_pCreators));
}

NetSynchronisationMan::~NetSynchronisationMan()
{
	m_objects.empty();
}

ntstd::List<NetSynchronisationObject*>::iterator NetSynchronisationMan::AddObject(NetSynchronisationObject* ob)
{
	m_objects.push_back(ob);
	return m_objects.end();
}

void NetSynchronisationMan::RemoveObject(ntstd::List<NetSynchronisationObject*>::iterator it)
{
	m_objects.erase(it);
}

NetSynchronisationObject* NetSynchronisationMan::RemoveObject(int iID)
{
	for(ntstd::List<NetSynchronisationObject*>::iterator it = m_objects.begin(); it != m_objects.end(); it++)
	{
		if((*it)->GetID() == iID)
		{
			NetSynchronisationObject *pRet = *it;
			m_objects.erase(it);
			return pRet;
		}
	}

	return 0;
}


NetSynchronisationObject* NetSynchronisationMan::GetObject(int iID)
{
	for(ntstd::List<NetSynchronisationObject*>::iterator it = m_objects.begin(); it != m_objects.end(); it++)
	{
		if((*it)->GetID() == iID)
			return *it;
	}

	return 0;
}

void NetSynchronisationMan::AddClient(CSafeRef<NetClient> client)
{
	m_aClients[client->GetID()].Set(client);
}

void NetSynchronisationMan::RemoveClient(CSafeRef<NetClient> client)
{
}

void NetSynchronisationMan::Update()
{
	for(ntstd::List<NetSynchronisationObject*>::iterator it = m_objects.begin(); it != m_objects.end(); it++)
	{
		// Build Influence Sets
		NetInfluenceCondition* pCond = (*it)->GetInfluenceCondition();
		NetInfluenceSet &set = (*it)->GetInfluenceSet();
		ntAssert(pCond);

		for(int i = 0; i < NetMan::MAX_CLIENTS; i++)
		{
			if(m_aClients[i].IsValid() && (*pCond)(m_aClients[i]))
				set.Set(i);
			else
				set.Clear(i);
		}

		// Update
		(*it)->Update();

	}
}

void NetSynchronisationMan::Syncronise(int iObjectID, int iItemID, char* pData)
{
	NetSynchronisationObject *pObj = GetObject(iObjectID);

	if(!pObj)
	{
		NETLOG_WARNING("No object matching ID");
		return;
	}

	pObj->SetSynchronisationData(iItemID, pData);

}

void NetSynchronisationMan::RegisterObject(int iType, NetSynchronisationObjectCreationFunc pFunc)
{
	m_pCreators[iType] = pFunc;
}

NetSynchronisationObject* NetSynchronisationMan::CreateObject(int iTypeID, int iID)
{
	ntPrintf(" ###SYNC### - Creating object type(%d) id(%d)\n", iTypeID, iID);
	NetSynchronisationObject* pObj = m_pCreators[iTypeID]();
	AddObject(pObj);
	return pObj;
}
