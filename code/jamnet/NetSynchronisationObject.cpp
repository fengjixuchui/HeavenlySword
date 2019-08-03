/***************************************************************************************************
*
*	DESCRIPTION		NetMan.cpp
*
*	NOTES			Definition of the network manager class
*
***************************************************************************************************/

#include "jamnet/netsynchronisationobject.h"
#include "jamnet/netsynchronisationman.h"

NetSynchronisationObject::NetSynchronisationObject() 
{
	m_iID = -1;
	m_iDataCount=0; 
	m_pInfluenceCondition=0;
	m_bMaster = false;
}

NetSynchronisationObject::~NetSynchronisationObject()
{
	for(ntstd::List<NetSynchronisationVarBase*>::iterator it = m_syncData.begin(); it != m_syncData.end(); it++)
	{
		NT_DELETE( (*it) );
	}
}

void NetSynchronisationObject::SetSynchronisationData(int iID, char* pData)
{
	ntAssert(iID >= 0 && iID < m_iDataCount);

	for(ntstd::List<NetSynchronisationVarBase*>::iterator it = m_syncData.begin(); it != m_syncData.end(); it++)
	{
		if((*it)->GetID() == iID)
		{
			(*it)->Set(pData);
			return;
		}
	}

	// No data with that id...
	ntAssert("Could not match a variable for synchronisation.\n");
}


void NetSynchronisationObject::AddSynchronisationData(int iID, NetSynchronisationVarBase* pData)
{
	ntAssert(iID == m_iDataCount); // Just for now, will make better asap
	m_iDataCount++;

	pData->SetID(*this, iID);
	m_syncData.push_back(pData);

	
}

void NetSynchronisationObject::Update()
{
	// Any Changes?
	for(ntstd::List<NetSynchronisationVarBase*>::iterator it = m_syncData.begin(); it != m_syncData.end(); it++)
	{
		(*it)->CheckForChanges();
	}

	// Update Clients
	for(int iClient = 0; iClient < NetMan::MAX_CLIENTS; iClient++)
	{
		if(m_influenceSet.HasClient(iClient))
		{
			NetSynchronisationClient& client = NetSynchronisationMan::Get().GetClient(iClient);
			ntAssert(client.IsValid());

			for(ntstd::List<NetSynchronisationVarBase*>::iterator it = m_syncData.begin(); it != m_syncData.end(); it++)
			{
				if(m_influenceSet.IsNew(iClient) || (*it)->HasChanged())
					(*it)->Update(client);
			}
		}
	}
}

NetMsgSyncVar::NetMsgSyncVar(const NetSynchronisationVarBase &var)
{
	m_iTypeID   = var.GetTypeID();
	m_iObjectID = var.GetParentID();
	m_iItemID   = (char)var.GetID();
	m_iDataSize = var.GetSize();
	NT_MEMCPY(m_pData, var.GetData(), m_iDataSize);
}

void NetMsgSyncVar::Despatch(CSafeRef<NetClient> pobClient, const class NetAddr& obAddr) const
{
	ntPrintf("SYNC VAR %d %d\n", m_iObjectID, m_iItemID);
	NetSynchronisationObject* pObj = NetSynchronisationMan::Get().GetObject(m_iObjectID);

	if(!pObj)
		pObj = NetSynchronisationMan::Get().CreateObject(m_iTypeID, m_iObjectID);

	NetSynchronisationMan::Get().Syncronise(m_iObjectID, m_iItemID, (char*)&m_pData);
	UNUSED(obAddr);
}

int NetSynchronisationVarBase::GetTypeID() const {return m_pParent->GetTypeID();}
int NetSynchronisationVarBase::GetParentID() const {return m_pParent->GetID();}

void NetMsgSyncKillObj::Despatch(CSafeRef<NetClient> pobClient, const class NetAddr& obAddr) const
{
	UNUSED(obAddr);
	NetSynchronisationObject* pObj = NetSynchronisationMan::Get().RemoveObject(m_iObjectID);
	NT_DELETE( pObj );
}
