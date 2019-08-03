/***************************************************************************************************
*
*	DESCRIPTION		NetMan.cpp
*
*	NOTES			Definition of the network manager class
*
***************************************************************************************************/

#ifndef _NET_SYNC_OBJ_H
#define _NET_SYNC_OBJ_H

#include "jamnet/netclient.h"
#include "jamnet/netman.h"

class NetSynchronisationVarBase;

class NetSynchronisationClient
{
public:
	NetSynchronisationClient(CSafeRef<NetClient> client = CSafeRef<NetClient>(NULL_REF)) 
		: m_client(client) {;}

		bool operator==(CSafeRef<NetClient> client) {return client == m_client;}
		bool IsValid() {return m_client;}

		int GetID() {return m_client->GetID();}

		void Send(NetMsg& obMsg) {m_client->AddToSendQueue(obMsg);}

private:
	friend class NetSynchronisationMan;
	void Set(CSafeRef<NetClient> client) {m_client = client;}

private:
	CSafeRef<NetClient> m_client;
};

class NetMsgSyncVar : public NetMsg
{
public:
	DECLARE_NETMSG_TYPE(NETMSG_SYNC_VAR);

	NetMsgSyncVar() {m_iObjectID = 0; m_iDataSize = 0;}
	NetMsgSyncVar(const NetSynchronisationVarBase &var);

	// Interface
	virtual	int	 GetLength() const {return m_iDataSize+sizeof(int)*3+sizeof(char);}
	virtual	int  SerialiseFrom(const uint8_t* pData) 
	{
		m_iTypeID = *((int*)pData);
		pData += sizeof(int);
		m_iObjectID = *((int*)pData);
		pData += sizeof(int);
		m_iItemID = *((char*)pData);
		pData += sizeof(char);
		m_iDataSize = *((int*)pData);
		ntAssert(m_iDataSize <= 8);
		pData += sizeof(int);
		NT_MEMCPY(m_pData, pData, m_iDataSize);

		return GetLength();
	}
	virtual int  SerialiseTo(uint8_t* pData) const
	{
		*((int*)pData) = m_iTypeID;
		pData += sizeof(int);
		*((int*)pData) = m_iObjectID;
		pData += sizeof(int);
		*((char*)pData) = m_iItemID;
		pData += sizeof(char);
		*((int*)pData) = m_iDataSize;
		pData += sizeof(int);
		NT_MEMCPY(pData, m_pData, m_iDataSize);
		return GetLength();
	}

	virtual void Despatch(CSafeRef<NetClient> pobClient, const class NetAddr& obAddr) const;

	virtual bool IsUrgent() const {return false;}
	virtual bool Reliable() const {return false;}

private:
	int   m_iTypeID;
	int   m_iObjectID;
	char  m_iItemID;
	int   m_iDataSize;
	uint8_t  m_pData[8];
	
};

//------------------------------------------------------------------------------------------
//!
//!	NetSynchronisationCondition
//!	Abstract Base Class for defining synchronisation conditions
//!
//------------------------------------------------------------------------------------------
class NetInfluenceCondition
{
public:
	virtual ~NetInfluenceCondition() {};
	virtual bool operator()(NetSynchronisationClient& client) = 0;

	void SetParent(class NetSynchronisationObject* pobj) {m_pParent = pobj;}

private:
	NetSynchronisationObject *m_pParent;
};

class NetInfluenceConditionAlways : public NetInfluenceCondition
{
public:
	virtual bool operator()(NetSynchronisationClient&) {return true;}
};

class NetInfluenceConditionNever : public NetInfluenceCondition
{
public:
	virtual bool operator()(NetSynchronisationClient&) {return false;}
};

class NetInfluenceConditionAnd : public NetInfluenceCondition
{
public:
	NetInfluenceConditionAnd(NetInfluenceCondition &lhs, NetInfluenceCondition &rhs) 
		: m_lhs(lhs), m_rhs(rhs) {;}

	virtual bool operator()(NetSynchronisationClient& client) {return m_lhs(client) && m_rhs(client);}

private:
	NetInfluenceCondition& m_lhs;
	NetInfluenceCondition& m_rhs;
};

class NetInfluenceConditionOr : public NetInfluenceCondition
{
public:
	NetInfluenceConditionOr(NetInfluenceCondition& lhs, NetInfluenceCondition& rhs)
		: m_lhs(lhs), m_rhs(rhs) {;}

	virtual bool operator()(NetSynchronisationClient& client) {return m_lhs(client) || m_rhs(client);}

private:
	NetInfluenceCondition& m_lhs;
	NetInfluenceCondition& m_rhs;
};

class NetInfluenceConditionNot : public NetInfluenceCondition
{
public:
	NetInfluenceConditionNot(NetInfluenceCondition& lhs)
		: m_lhs(lhs) {;}

	virtual bool operator()(NetSynchronisationClient& client) {return !m_lhs(client);}

private:
	NetInfluenceCondition& m_lhs;
};

class NetInfluenceConditionInRange : public NetInfluenceCondition
{
public:
	NetInfluenceConditionInRange(float f) {m_fRangeSqrd = f*f;}

	virtual bool operator()(NetSynchronisationClient& client);

private:
	float m_fRangeSqrd;
};

class NetInfluenceConditionIsVisible : public NetInfluenceCondition
{
public:
	virtual bool operator()(NetSynchronisationClient& client);
};




class NetInfluenceSet
{
public:
	void Clear(int iID) {m_clients[iID] = false;}
	void Set(int iID) {if(m_clients[iID]) m_new[iID] = false; else {m_clients[iID] = true; m_new[iID] = true;}}

	bool HasClient(int iID) {return m_clients[iID];}
	bool IsNew(int iID) {return m_new[iID];}

private:
	bool m_clients[NetMan::MAX_CLIENTS];
	bool m_new[NetMan::MAX_CLIENTS];
};


//------------------------------------------------------------------------------------------
//!
//!	NetSynchronisationVar
//!	
//!
//------------------------------------------------------------------------------------------
class NetSynchronisationVarBase
{
public:
	virtual ~NetSynchronisationVarBase() {};

	int	GetID() const {return m_iID;}
	virtual bool HasChanged() const {return m_bChanged;}

	virtual void Set(char *pData) = 0;
	virtual void CheckForChanges() = 0;
	virtual void Update(NetSynchronisationClient& client) = 0;

	class NetSynchronisationObject* GetParentP() {return m_pParent;}
	int GetTypeID() const;
	int GetParentID() const;
	const NetSynchronisationObject* GetParent() {return m_pParent;}
	virtual int			GetSize() const = 0;
	virtual const uint8_t *GetData() const = 0;

protected:
	bool m_bChanged;

private:
	void SetID(NetSynchronisationObject& obj, int iItem) {m_pParent = &obj; m_iID = iItem;}
	friend class NetSynchronisationObject;

private:
	NetSynchronisationObject* m_pParent;
	int m_iID;
};

template<class T>
class NetSynchronisationVar : public NetSynchronisationVarBase
{
public:
	NetSynchronisationVar(T *pAddr) {m_bChanged = true; m_pAddr = pAddr;}

	virtual void Set(char *pData) {m_lastTick = *m_pAddr = *((T*)pData); }

	virtual void CheckForChanges() {m_bChanged = (m_lastTick != *m_pAddr); if(m_bChanged) m_lastTick = *m_pAddr;}
	virtual void Update(NetSynchronisationClient& client)
	{
		const NetSynchronisationVar<T> base = *this;
		NetMsgSyncVar obMsg(base);
		client.Send(obMsg);
	}

	virtual int GetSize() const			{return sizeof(T);}
	virtual const uint8_t *GetData() const {return (uint8_t*)m_pAddr;}

private:
	T*  m_pAddr;
	T   m_lastTick;	
};

//------------------------------------------------------------------------------------------
//!
//!	NetSynchronisationObject
//!	An object that can be synchronised across the network
//!
//------------------------------------------------------------------------------------------
class NetSynchronisationObject
{
public:
	NetSynchronisationObject();
	virtual ~NetSynchronisationObject();

	virtual int GetID() = 0;
	virtual int GetTypeID() = 0;

	void SetMaster() {m_bMaster = true;}
	void SetSlave()  {m_bMaster = false;}
	bool IsMaster()  {return m_bMaster;}
	bool IsSlave()   {return !m_bMaster;}

	void SetSynchronisationData(int iID, char* pData);
	void Migrate()     {;}

	void AddSynchronisationData(int iID, NetSynchronisationVarBase* pData);

	void SetInfluenceCondition(NetInfluenceCondition* pCond) {m_pInfluenceCondition = pCond;}
	NetInfluenceCondition* GetInfluenceCondition() {return m_pInfluenceCondition;}

	NetInfluenceSet& GetInfluenceSet() {return m_influenceSet;}

	void Update();

private:
	bool						  m_bMaster;
	int							  m_iID;

	int									   m_iDataCount;
	ntstd::List<NetSynchronisationVarBase*> m_syncData;

	NetInfluenceCondition*		  m_pInfluenceCondition;
	NetInfluenceSet			  m_influenceSet;
};

class NetMsgSyncKillObj : public NetMsg
{
public:
	DECLARE_NETMSG_TYPE(NETMSG_SYNC_KILLOBJ);

	NetMsgSyncKillObj() {m_iObjectID = 0;}
	NetMsgSyncKillObj(int iID) {m_iObjectID = iID;}

	// Interface
	virtual	int	 GetLength() const {return sizeof(int);}
	virtual	int  SerialiseFrom(const uint8_t* pData) 
	{
		m_iObjectID = *((int*)pData);
		return GetLength();
	}
	virtual int  SerialiseTo(uint8_t* pData) const
	{
		*((int*)pData) = m_iObjectID;
		return GetLength();
	}

	virtual void Despatch(CSafeRef<NetClient> pobClient, const class NetAddr& obAddr) const;

	virtual bool IsUrgent() const {return false;}
	virtual bool Reliable() const {return false;}

private:
	int   m_iObjectID;
};

#endif // _NET_SYNC_OBJ_H
