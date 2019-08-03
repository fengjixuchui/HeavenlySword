//------------------------------------------------------------------------------------------
//!
//!	\file message.h
//!
//------------------------------------------------------------------------------------------

#ifndef _MESSAGE_H
#define _MESSAGE_H

#define MSG_DBGNAME(id) "Unknown"

#ifndef _RELEASE
#define CHECKINDEX(i) \
	ntAssert_p(i >=0 && i < MAXPARAMS, ("Message %s - Parameter Index out of range",MSG_DBGNAME(m_id)));

#define CHECKMSGPARAM(MSGTYPE) \
	ntAssert_p(i >=0 && i < MAXPARAMS, ("Message %s - Parameter Index out of range",MSG_DBGNAME(m_id))); \
	ntAssert_p(m_params[i].GetType() == MSGTYPE, ("Message %s - bad param %d\n",MSG_DBGNAME(m_id),i));
//#define _USE_MESSAGE_DEBUG_INFO
#else
#define CHECKINDEX(i)
#define CHECKMSGPARAM(MSGTYPE)
#endif

//namespace NinjaLua {class LuaObject;}
#include "game/luaglobal.h"
#include "messages.h"

// Alexey debug
#ifdef PLATFORM_PS3
#include <libsn.h>
#endif

class CEntity;
namespace FSM {class StateMachine;}


class MessageInner
{
public:

	enum Type {T_NIL, T_INT, T_FLOAT, T_BOOL, T_STRING, T_PTR, T_LUA, T_HASH};

	struct Param
	{
		Param() : pv(0), type(T_NIL) {}
		Param(const Param& rhs) : sName(rhs.sName), type(rhs.type)
		{
			switch(type)
			{
			case T_LUA:
				plua = NT_NEW_CHUNK(Mem::MC_ENTITY) NinjaLua::LuaObject;
				*plua = *rhs.plua;
				break;
			case T_STRING:
				s = rhs.s;
				break;
			default:
				pv = rhs.pv;
			}
		}

		Param& operator=(const Param& rhs)
		{
			sName = rhs.sName;
			SetType(rhs.type);
			switch(type)
			{
			case T_LUA:
				*plua = *rhs.plua;
				break;
			case T_STRING:
				s = rhs.s;
				break;
			default:
				pv = rhs.pv;
			}
			return *this;
		}


		~Param() {SetType(T_NIL);}

		void SetType(Type t);
		Type GetType() const {return type;}

		// Temporary for lua support
		void SetValue(const NinjaLua::LuaObject& v);

		// Unnamed param support
		void SetValue(int v)				{ SetType(T_INT); i = v; }
		void SetValue(float v)				{ SetType(T_FLOAT); f = v; }
		void SetValue(bool v)				{ SetType(T_BOOL); b = v; }
		void SetValue(const char* v)		{ SetType(T_STRING); s = v; }
		void SetValue(CEntity* v)			{ SetType(T_PTR); pv = v; }
		void SetValue(CHashedString v)		{ SetType(T_HASH); hs = ntStr::GetHashKey(v); }

		CHashedString					sName;	// Parameters won't be named for long, this is to support the old lua way of working.

		ntstd::String					s;		// Not a POD so can't be part of union below
		union
		{
			NinjaLua::LuaObject*		plua;
			CEntity*					pv;
			float						f;
			int							i;
			bool						b;
			CHashedString::HashKeyType	hs;
		};

	private:
		Type type;
	};

	MessageInner();
	MessageInner(const MessageInner&);
	~MessageInner() {};

	int       GetParamCount() const { return m_iParamCount;}

	Type GetType(int i) const	{CHECKINDEX(i); return m_params[i].GetType();}
	bool IsInt(int i) const		{CHECKINDEX(i); return m_params[i].GetType() == T_INT;}
	bool IsFloat(int i) const	{CHECKINDEX(i); return m_params[i].GetType() == T_FLOAT;}
	bool IsNumber(int i) const	{CHECKINDEX(i); return m_params[i].GetType() == T_INT || m_params[i].GetType() == T_FLOAT;}
	bool IsBool(int i) const	{CHECKINDEX(i); return m_params[i].GetType() == T_BOOL;}
	bool IsString(int i) const	{CHECKINDEX(i); return m_params[i].GetType() == T_STRING;}
	bool IsEntity(int i) const	{CHECKINDEX(i); return m_params[i].GetType() == T_PTR;}
	bool IsLua(int i) const		{CHECKINDEX(i); return m_params[i].GetType() == T_LUA;}
	bool IsHash(int i) const	{CHECKINDEX(i); return m_params[i].GetType() == T_HASH;}

	MessageInner::Type GetType(CHashedString k) const	{int i = FindParam(k); if(i<0) return T_NIL; return m_params[i].GetType();}
	bool IsInt(const char* k) const		{int i = FindParam(k); if(i<0) return false; return m_params[i].GetType() == T_INT;}
	bool IsFloat(const char* k) const	{int i = FindParam(k); if(i<0) return false; return m_params[i].GetType() == T_FLOAT;}
	bool IsNumber(const char* k) const	{int i = FindParam(k); if(i<0) return false; return m_params[i].GetType() == T_INT || m_params[i].GetType() == T_FLOAT;}
	bool IsBool(const char* k) const	{int i = FindParam(k); if(i<0) return false; return m_params[i].GetType() == T_BOOL;}
	bool IsString(const char* k) const	{int i = FindParam(k); if(i<0) return false; return m_params[i].GetType() == T_STRING;}
	bool IsPtr(const char* k) const		{int i = FindParam(k); if(i<0) return false; return m_params[i].GetType() == T_PTR;}
	bool IsLua(const char* k) const		{int i = FindParam(k); if(i<0) return false; return m_params[i].GetType() == T_LUA;}
	bool IsHash(const char* k) const	{int i = FindParam(k); if(i<0) return false; return m_params[i].GetType() == T_HASH;}

	int GetInt(int i) const							{CHECKMSGPARAM(T_INT);    return m_params[i].i;}
	float GetFloat(int i) const						{CHECKMSGPARAM(T_FLOAT);  return m_params[i].f;}
	bool GetBool(int i) const						{CHECKMSGPARAM(T_BOOL);   return m_params[i].b;}
	const char* GetString(int i) const				{CHECKMSGPARAM(T_STRING); return ntStr::GetString(m_params[i].s);}
	CHashedString GetHashedString(int i) const;
	const CEntity* GetEntity(int i) const			{CHECKMSGPARAM(T_PTR);    return m_params[i].pv;}
	const NinjaLua::LuaObject& GetLua(int i) const	{CHECKMSGPARAM(T_LUA);    return *m_params[i].plua;}

	int				GetInt(const char* k) const;
	float			GetFloat(const char* k) const;
	bool			GetBool(const char* k) const;
	const char*		GetString(const char* k) const;
	CHashedString	GetHashedString(const CHashedString k) const;
	CEntity*		GetEnt(const char* k) const;
	CEntity*		GetEnt(const CHashedString& k) const;

	template<class T> T*	GetPtr(const char* k) const;			
	void			SetInt(CHashedString k, int v);				
	void			SetFloat(CHashedString k, float v);			
	void			SetBool(CHashedString k, bool v);			
	void			SetString(CHashedString k, const char* v);	
	void			SetString(CHashedString k, char* v);			
	void			SetString(CHashedString k, CHashedString v);	
	void			SetEnt(CHashedString k, CEntity* v);
	void			SetEnt(CHashedString k, const CEntity* v);
	void			SetLua(CHashedString k, const NinjaLua::LuaObject& v);

	template< typename TYPE >
	void AddParam(const TYPE& v) 
	{
		CHECKINDEX(m_iParamCount);
		m_params[m_iParamCount++].SetValue( v );
#if (!defined _RELEASE) && (!defined _MASTER)
		if (m_iParamCount > m_gMaxParamsAllocated)
			m_gMaxParamsAllocated = m_iParamCount;
#endif
	}

	// -----------------------------------------------------------------------------------------------------------------------------------------
	// For Lua support... named parameter support will soon disappear
	// -----------------------------------------------------------------------------------------------------------------------------------------
	int                    FindParam(const CHashedString& k) const;
	int                    FindOrCreateParam(const CHashedString& k);

	int                    GetValue(const CHashedString& sNamed);
	int                    SetValue(const CHashedString& sNamed, const NinjaLua::LuaObject& v);
	int                    GetValue(int i);
	int                    AddValue(const NinjaLua::LuaObject& v);

	const NinjaLua::LuaObject& GetLua(const char* k) const;
	const NinjaLua::LuaObject& GetLua(const CHashedString& k) const;

	void                   SetUnnamedParams() {m_bUnnamedParams = true;}
	bool                   HasUnnamedParams() {return m_bUnnamedParams;}

	// Ideally private
	static const int		MAXPARAMS = 12;
	Param					m_params[MAXPARAMS];
	int                  m_iParamCount;
	// temporary - makeint messages - remove soon
	bool                 m_bUnnamedParams;

#if (!defined _RELEASE) && (!defined _MASTER)
	static int				m_gMaxParamsAllocated;
#endif

};

class Message
{
public:
	Message(MessageID id, bool bLuaInterface = true );
	Message(const Message& rhs);
	~Message() {}

	MessageID GetID()    const {return m_id;}
	float     GetDelay() const {return m_fDelay;}
	int       GetParamCount() const { return m_obInner->GetParamCount();}

	// Parameter Types
	
	MessageInner::Type GetType(int i)  const {return m_obInner->GetType(i);}
	bool IsInt(int i)    const {return m_obInner->IsInt(i);}
	bool IsFloat(int i)  const {return m_obInner->IsFloat(i);}
	bool IsNumber(int i) const {return m_obInner->IsNumber(i);}
	bool IsBool(int i)   const {return m_obInner->IsBool(i);}
	bool IsString(int i) const {return m_obInner->IsString(i);}
	bool IsEntity(int i) const {return m_obInner->IsEntity(i);}
	bool IsLua(int i)    const {return m_obInner->IsLua(i);}
	bool IsHash(int i)   const {return m_obInner->IsHash(i);}

	MessageInner::Type GetType(CHashedString k) const {return m_obInner->GetType(k);}
	bool IsInt(const char* k) const		{return m_obInner->IsInt(k);}
	bool IsFloat(const char* k) const	{return m_obInner->IsFloat(k);}
	bool IsNumber(const char* k) const	{return m_obInner->IsNumber(k);}
	bool IsBool(const char* k) const	{return m_obInner->IsBool(k);}
	bool IsString(const char* k) const	{return m_obInner->IsString(k);}
	bool IsPtr(const char* k) const		{return m_obInner->IsPtr(k);}
	bool IsLua(const char* k) const		{return m_obInner->IsLua(k);}
	bool IsHash(const char* k) const	{return m_obInner->IsHash(k);}
	
	// Parameter Gets
	int				GetInt(int i) const				{return m_obInner->GetInt(i);}
	float			GetFloat(int i) const			{return m_obInner->GetFloat(i);}
	bool			GetBool(int i) const			{return m_obInner->GetBool(i);}
	const char*		GetString(int i) const			{return m_obInner->GetString(i);}
	CHashedString	GetHashedString(int i) const	{return m_obInner->GetHashedString(i);}
	const CEntity*	GetEntity(int i) const			{return m_obInner->GetEntity(i);}
	const NinjaLua::LuaObject& GetLua(int i) const	{return m_obInner->GetLua(i);}

	int				GetInt(const char* k) const			{return m_obInner->GetInt(k);}
	float			GetFloat(const char* k)  const		{return m_obInner->GetFloat(k);}
	bool			GetBool(const char* k)   const		{return m_obInner->GetBool(k);}
	const char*		GetString(const char* k) const		{return m_obInner->GetString(k);}
	CHashedString	GetHashedString(const CHashedString k) const	{return m_obInner->GetHashedString(k);}
	CEntity*		GetEnt(const char* k) const			{return m_obInner->GetEnt(k);}
	CEntity*		GetEnt(const CHashedString& k) const{return m_obInner->GetEnt(k);}


	// Parameter Sets
	template< typename TYPE >
	void AddParam(const TYPE& v) {m_obInner->AddParam(v);}

	HAS_LUA_INTERFACE()


	
	template<class T> T*	GetPtr(const char* k) const					{return m_obInner->GetPtr(k);}
	void					SetInt(CHashedString k, int v)				{return m_obInner->SetInt(k,v);}
	void					SetFloat(CHashedString k, float v)			{return m_obInner->SetFloat(k,v);}
	void					SetBool(CHashedString k, bool v)			{return m_obInner->SetBool(k,v);}
	void					SetString(CHashedString k, const char* v)	{return m_obInner->SetString(k,v);}
	void					SetString(CHashedString k, char* v)			{return m_obInner->SetString(k,v);}
	void					SetString(CHashedString k, CHashedString v)	{return m_obInner->SetString(k,v);}

	void                   SetEnt(CHashedString k, CEntity* v)					{return m_obInner->SetEnt(k,v);}
	void                   SetEnt(CHashedString k, const CEntity* v)			{return m_obInner->SetEnt(k,v);}
	void                   SetLua(CHashedString k, const NinjaLua::LuaObject& v){return m_obInner->SetLua(k,v);}

	const NinjaLua::LuaObject& GetLua(const char* k) const				{return m_obInner->GetLua(k);}
	const NinjaLua::LuaObject& GetLua(const CHashedString& k) const		{return m_obInner->GetLua(k);}

	//void                   SetID(MessageID id)					{ m_id = id; }
	void                   FillFromLuaTable(const NinjaLua::LuaObject& tbl);

	// For Lua support...
	int                    GetValue(const CHashedString& sNamed) {return m_obInner->GetValue(sNamed);}
	int                    SetValue(const CHashedString& sNamed, const NinjaLua::LuaObject& v) {return m_obInner->SetValue(sNamed, v);}
	int                    GetValue(int i) {return m_obInner->GetValue(i);}
	int                    AddValue(const NinjaLua::LuaObject& v) {return m_obInner->AddValue(v);}

	void                   SetUnnamedParams() {m_obInner->SetUnnamedParams();}
	bool                   HasUnnamedParams() {return m_obInner->HasUnnamedParams();}
	// -----------------------------------------------------------------------------------------------------------------------------------------
	// End Lua backward compatibility Funcs
	// -----------------------------------------------------------------------------------------------------------------------------------------

private:
	CSharedPtr<MessageInner> m_obInner;
	float                m_fDelay;
	MessageID            m_id;

	void VerifyMessageID();
	friend class FSM::StateMachine;

public:
	void VerifyMsgString(); // This will go...


#ifdef _DEBUG
	const char* GetIDName() const;
#endif

};

LV_DECLARE_USERDATA(Message);

#endif //_MESSAGE_H
