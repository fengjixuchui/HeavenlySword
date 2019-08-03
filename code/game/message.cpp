//------------------------------------------------------------------------------------------
//!
//!	\file message.cpp
//!
//------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------------------
#include "message.h"
#include "game/luaglobal.h"

#include "entity.h"

//------------------------------------------------------------------------------------------
// Register the message container type with Ninja Lua
//------------------------------------------------------------------------------------------
LUA_EXPOSED_CONTAINER(Message, GetValue, SetValue)



MessageInner::MessageInner() 
	: m_iParamCount(0)
	, m_bUnnamedParams(false) 
{
}

MessageInner::MessageInner(const MessageInner& other)
	: m_iParamCount(other.m_iParamCount)
	, m_bUnnamedParams(other.m_bUnnamedParams) 
{
	for(int i = 0; i < other.m_iParamCount; i++)
		m_params[i] = other.m_params[i];
}

// Static
#if (!defined _RELEASE) && (!defined _MASTER)
int	MessageInner::m_gMaxParamsAllocated = 0;
#endif


//------------------------------------------------------------------------------------------
//!  public overloaded constructor  Message
//!
//!  @param [in]       id MessageID     
//!  @param [in]       bLuaInterface bool  [=1]    
//!
//!  @remarks <TODO: insert remarks here>
//!
//!  @author GavB @date 16/08/2006
//------------------------------------------------------------------------------------------
Message::Message(MessageID id, bool bLuaInterface ) : 
	m_obInner(new MessageInner),
	m_fDelay(0.f), 
	m_id(id)
{ 
	if(bLuaInterface) 
	{ 
		ATTACH_LUA_INTERFACE(Message);
	} 
}

//------------------------------------------------------------------------------------------
//!  public overloaded constructor  Message
//!
//!  @param [in]       rhs const Message &    
//!
//!  @remarks <TODO: insert remarks here>
//!
//!  @author GavB @date 16/08/2006
//------------------------------------------------------------------------------------------
Message::Message(const Message& rhs) : 
	m_obInner(rhs.m_obInner),
	m_fDelay(rhs.m_fDelay), 
	m_id(rhs.m_id)
{
	COPY_LUA_INTERFACE(rhs);
}


//------------------------------------------------------------------------------------------
//!
//!	Message::FillFromLuaTable
//!	Construct a message from a lua table
//!
//------------------------------------------------------------------------------------------
void Message::FillFromLuaTable(const NinjaLua::LuaObject& tbl)
{
	for(NinjaLua::LuaIterator itTbl(tbl);itTbl;++itTbl)
		SetValue(itTbl.GetKey().GetString(), itTbl.GetValue());
}


//------------------------------------------------------------------------------------------
//!
//!	MessageInner::FindParam
//!	Find a named parameter
//!
//------------------------------------------------------------------------------------------
int MessageInner::FindParam(const CHashedString& k) const
{
	for(int i = 0; i < m_iParamCount; i++)
	{
		if(m_params[i].sName == k)
			return i;
	}

	return -1;
}


//------------------------------------------------------------------------------------------
//!
//!	MessageInner::FindOrCreateParam
//!	Find a named parameter, if one does not exist then create one
//!
//------------------------------------------------------------------------------------------
int MessageInner::FindOrCreateParam(const CHashedString& k)
{
	for(int i = 0; i < m_iParamCount; i++)
	{
		if(m_params[i].sName == k)
			return i;
	}

	if(m_iParamCount < MAXPARAMS)
	{
		m_params[m_iParamCount].sName = k;
		return m_iParamCount++;
	}
	
	ntPrintf("%s(%d): Could not find or create parameter %s\n", __FILE__, __LINE__, ntStr::GetString(k));
		return -1;
}


//------------------------------------------------------------------------------------------
//!
//!	MessageInner::GetValue
//!	Get the value of a parameter
//!
//------------------------------------------------------------------------------------------
int MessageInner::GetValue(int i)
{
	CHECKINDEX(i);

	switch(m_params[i].GetType())
	{
	case T_INT:
		return NinjaLua::LuaValue::Push(CLuaGlobal::Get().State(), m_params[i].i);
	case T_FLOAT:
		return NinjaLua::LuaValue::Push(CLuaGlobal::Get().State(), m_params[i].f);
	case T_BOOL:
		return NinjaLua::LuaValue::Push(CLuaGlobal::Get().State(), m_params[i].b);
	case T_STRING:
		return NinjaLua::LuaValue::Push(CLuaGlobal::Get().State(), ntStr::GetString(m_params[i].s));
	case T_PTR:
		return NinjaLua::LuaValue::Push<const CEntity*>(CLuaGlobal::Get().State(), m_params[i].pv);
	case T_LUA:
		m_params[i].plua->Push(&CLuaGlobal::Get().State());
		return 1;
	case T_HASH :
		return NinjaLua::LuaValue::Push(CLuaGlobal::Get().State(), CHashedString(m_params[i].hs));
	case T_NIL:
		return 0;
	}

	return 0;
}


//------------------------------------------------------------------------------------------
//!
//!	MessageInner::GetValue
//!	Get the value of a named parameter
//!
//------------------------------------------------------------------------------------------
int MessageInner::GetValue(const CHashedString& sNamed)
{
	int i = FindParam(sNamed);
	if(i < 0)
	{
#ifdef _WIN32
		ntPrintf("Could not find paramater named %s on message.\n", ntStr::GetString(sNamed));
#endif
		return 0;
	}

	return GetValue(i);
}


//------------------------------------------------------------------------------------------
//!
//!	MessageInner::SetValue
//!	Set the value of a parameter
//!
//------------------------------------------------------------------------------------------
int MessageInner::AddValue(const NinjaLua::LuaObject& v)
{
	if(m_iParamCount >= MAXPARAMS)
	{
		ntAssert(false);
		return 0;
	}

	m_params[m_iParamCount++].SetValue(v);

	return 0;
}


//------------------------------------------------------------------------------------------
//!
//!	MessageInner::SetValue
//!	Set the value of a named parameter
//!
//------------------------------------------------------------------------------------------
int MessageInner::SetValue(const CHashedString& sNamed, const NinjaLua::LuaObject& v)
{
	int iParam = FindOrCreateParam(sNamed);

	if(iParam < 0)
	{
		ntAssert(false);
		return 0;
	}

	m_params[iParam].SetValue(v);
	return 0;
}


void MessageInner::Param::SetType(Type t)
{
	if(type == t)
		return;

	if(type == T_LUA)
	{
		NT_DELETE_CHUNK(Mem::MC_ENTITY, plua);
		plua = 0;
	}
	else if(t == T_LUA)
		plua = NT_NEW_CHUNK(Mem::MC_ENTITY) NinjaLua::LuaObject(CLuaGlobal::Get().State());

	type = t;
}


void MessageInner::Param::SetValue(const NinjaLua::LuaObject& v)
{
	switch(v.GetType())
	{
	case LUA_TNUMBER:
		if(abs(v.GetFloat() - v.GetInteger()) > EPSILON)
		{
			SetType(T_FLOAT);
			f = v.GetFloat();
		}
		else
		{
			SetType(T_INT);
			i = v.GetInteger();
		}
		break;
	case LUA_TBOOLEAN:
		SetType(T_BOOL);
		b = v.GetBoolean();
		break;
	case LUA_TSTRING:
		//SetType(T_STRING);
		//s = v.GetString();
		SetType(T_HASH);
		hs = ntStr::GetHashKey(v.GetHashedString());
		break;
	case LUA_TUSERDATA:
		SetType(T_PTR);
		if(v.Is<CEntity*>())
		{
			pv = v.GetUserData<CEntity*>();
			break;
		}
	default:
		SetType(T_LUA);
		*plua = v;
		break;
	}
}


int	MessageInner::GetInt(const char* k)    const
{
	int i = FindParam(k); 
	if(i<0)
	{
		ntPrintf("No named parameter %s\n", k);
		return 0;
	}                                          
	if(m_params[i].GetType()==T_INT) 
		return m_params[i].i; 
	if(m_params[i].GetType()==T_FLOAT) 
		return (int)m_params[i].f; 
	ntAssert(false); 
	return 0;
}

float MessageInner::GetFloat(const char* k)  const
{
	int i = FindParam(k); 
	if(i<0) 
	{
		ntPrintf("No named parameter %s\n", k); 
		return 0.f;
	}                                        
	if(m_params[i].GetType()==T_INT) 
		return (float)m_params[i].i; 
	if(m_params[i].GetType()==T_FLOAT) 
		return m_params[i].f; 
	ntAssert(false); 
	return 0.f;
}

bool MessageInner::GetBool(const char* k)   const 
{
	int i = FindParam(k); 
	if(i<0) 
	{
		ntPrintf("No named parameter %s\n", k); 
		return false;
	}                                      
	ntAssert(m_params[i].GetType()==T_BOOL); 
	return m_params[i].b;
}

const char* MessageInner::GetString(const char* k) const 
{
	int i = FindParam(k); 
	if(i<0) 
	{
		ntPrintf("No named parameter %s\n", k); 
		static char nullStr[] = {"NULL"}; 
		return nullStr;
	} 
	ntAssert(m_params[i].GetType()==T_STRING); 
	return ntStr::GetString(m_params[i].s);
}

CHashedString MessageInner::GetHashedString(const CHashedString k) const
{
	int i = FindParam(k); 
	if(i<0)
	{
		ntPrintf("No named parameter %s\n", ntStr::GetString(k));
		return CHashedString();
	}
	if(m_params[i].GetType()==T_HASH)
		return CHashedString(m_params[i].hs);
	else if(m_params[i].GetType()==T_STRING)
		return CHashedString(m_params[i].s);

	ntAssert(false);
	return CHashedString();
}

CEntity* MessageInner::GetEnt(const char* k)          const 
{
	int i = FindParam(k); 
	if(i<0) 
	{
		ntPrintf("No named parameter %s\n", k); return 0;
	}                  
	ntAssert(m_params[i].GetType()==T_PTR); 
	return m_params[i].pv;
}

CEntity* MessageInner::GetEnt(const CHashedString& k) const 
{
	int i = FindParam(k); 
	if(i<0) 
	{
		ntPrintf("No named parameter %s\n", k.GetDebugString()); 
		return 0;
	} 
	ntAssert(m_params[i].GetType()==T_PTR); 
	return m_params[i].pv;
}

const NinjaLua::LuaObject& MessageInner::GetLua(const char* k)          const 
{
	int i = FindParam(k); 
	ntError(i>=0);                                                             
	ntAssert(m_params[i].GetType()==T_LUA); 
	return *m_params[i].plua;
}

const NinjaLua::LuaObject& MessageInner::GetLua(const CHashedString& k) const 
{
	int i = FindParam(k); 
	ntError(i>=0);                                                             
	ntAssert(m_params[i].GetType()==T_LUA); 
	return *m_params[i].plua;
}


template<class T> T* MessageInner::GetPtr(const char* k)     const 
{
	int i = FindParam(k); 
	ntError(i>0); 
	ntAssert(m_params[i].type==T_PTR); 
	return (T*)m_params[i].pv;
}


void MessageInner::SetEnt(CHashedString k, CEntity* v)
{
	int i = FindOrCreateParam(k);
	if(i<0) return;
	m_params[i].SetType(T_PTR);
	m_params[i].sName = k;
	m_params[i].pv = v;
}
void MessageInner::SetEnt(CHashedString k, const CEntity* v)
{
	int i = FindOrCreateParam(k);
	if(i<0) return;
	m_params[i].SetType(T_PTR);
	m_params[i].sName = k;
	m_params[i].pv = (CEntity*)v;
}
void MessageInner::SetLua(CHashedString k, const NinjaLua::LuaObject& v)
{
	int i = FindOrCreateParam(k);
	if(i<0) return;
	m_params[i].SetType(T_LUA);
	m_params[i].sName = k;
	*m_params[i].plua = v;
}


void MessageInner::SetInt(CHashedString k, int v)            
{
	int i = FindOrCreateParam(k); 
	if(i<0) return; 
	m_params[i].SetType(T_INT);    
	m_params[i].sName = k; 
	m_params[i].i = v;
}

void MessageInner::SetFloat(CHashedString k, float v)          
{
	int i = FindOrCreateParam(k); 
	if(i<0) return; 
	m_params[i].SetType(T_FLOAT);  
	m_params[i].sName = k; 
	m_params[i].f = v;
}

void MessageInner::SetBool(CHashedString k, bool v)
{
	//snPause();
	int i = FindOrCreateParam(k);
	if(i<0) return;
	m_params[i].SetType(T_BOOL);
	m_params[i].sName = k;
	m_params[i].b = v;
}
void MessageInner::SetString(CHashedString k, const char* v)
{
	//snPause();
	int i = FindOrCreateParam(k);
	if(i<0) return;
	m_params[i].SetType(T_STRING);
	m_params[i].sName = k;
	m_params[i].s = v;
}
void MessageInner::SetString(CHashedString k, char* v)
{
	//snPause();
	int i = FindOrCreateParam(k);
	if(i<0) return;
	m_params[i].SetType(T_STRING);
	m_params[i].sName = k;
	m_params[i].s = v;
}
void MessageInner::SetString(CHashedString k, CHashedString v)
{
	//snPause();
	int i = FindOrCreateParam(k);
	if(i<0) return; 
	m_params[i].SetType(T_HASH);
	m_params[i].sName = k;
	m_params[i].hs = ntStr::GetHashKey(v);
}

CHashedString MessageInner::GetHashedString(int i) const
{
	if(i<0 || i >= MAXPARAMS)
		return CHashedString();

	if(m_params[i].GetType()==T_HASH)
		return CHashedString(m_params[i].hs);
	else if(m_params[i].GetType()==T_STRING)
		return CHashedString(m_params[i].s);

	return CHashedString();
}

