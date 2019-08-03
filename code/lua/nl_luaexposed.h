#ifndef _LUAEXPOSED_H
#define _LUAEXPOSED_H

class Dynamics_Lua;
namespace Physics {class System;}

namespace NinjaLua
{

//------------------------------------------------------------------------------------------
//!
//!	LuaRawMethodBind                                                                                 
//!	This is an method that retrieves it's arguments from the lua_state itself
//!
//------------------------------------------------------------------------------------------
class LuaRawMethodBind
{
public:
	int operator() (void* pvObj, LuaState& pLua) const
	{
		return m_Fn(pvObj, m_Func, pLua);
	}

private:
	typedef int (*LuaCallBindFn)(void* pvObj, void* pvFunc, LuaState& pLua);

	LuaCallBindFn m_Fn;
	void*		  m_Func;

// Static Maker
public:
	template<class CLASS>
	struct LuaRawBind
	{
		static int Bind(void* pvObj, void* pvFunc, LuaState& State)
		{
			typedef int (CLASS::*FuncPtr)( LuaState& );
			FuncPtr pFunc = *(FuncPtr*)pvFunc;

			return (((CLASS*)pvObj)->*pFunc)(State);
		}
	};

	template<class CLASS>
	static LuaRawMethodBind Maker(void* FuncPtr)
	{
		LuaRawMethodBind obCaller;
		obCaller.m_Func = FuncPtr;
		obCaller.m_Fn   = &LuaRawBind<CLASS>::Bind;
		return obCaller;
	}

};



//------------------------------------------------------------------------------------------
//!
//!	LuaMethodBind                                                                                 
//!	This is a method for which we retrieve it's arguments for it.
//!
//------------------------------------------------------------------------------------------
class LuaMethodBind
{
public:
	int operator() (void* pvObj, LuaState& State) const
	{
		int iTop = State.GetTop();
		int iRets = m_Fn(pvObj, m_Func, State);
		int iNewTop = State.GetTop();

		if(iTop+iRets != iNewTop)
		{
			bool b;
			b = true;
		}

		if(iTop != 0)
		{
			bool b;
			b = true;
		}

		return iRets;
	}

	static int luaCaller( lua_State* pLua )
	{
		LuaMethodBind*	pBindMethod = (LuaMethodBind*) lua_touserdata(pLua, lua_upvalueindex(1));
		void*			pObj = (void*) lua_touserdata(pLua, lua_upvalueindex(2));
		return (*pBindMethod)(pObj, NinjaLua::GetState(pLua) );
	}

	typedef int (*LuaCallBindFn)(void* pvObj, void* pvFunc, LuaState& State);

private:
	LuaCallBindFn m_Fn;
	void*		  m_Func;


// Static Maker
public:

	static LuaMethodBind Make(LuaCallBindFn pFn, void* pFunc)
	{
		LuaMethodBind obCaller;
		obCaller.m_Fn	= pFn;
		obCaller.m_Func	= pFunc;
		return obCaller;
	}
/*
	template< typename CLASS >
	static LuaMethodBind MakeIt(LuaCallBindFn pFn, void (CLASS::*pFunc)(void) )
	{
		LuaMethodBind obCaller;
		obCaller.m_Fn	= pFn;
		obCaller.m_Func	= pFunc;
		return obCaller;
	}
*/
	template< typename CLASS >
	struct Storage { typedef void (CLASS::*FuncPtr)(void); FuncPtr m_pFunc; };
};

//------------------------------------------------------------------------------------------
//!
//!	LuaStaticMethodBind                                                                                 
//!	This is a method for which we retrieve it's arguments for it.
//!
//------------------------------------------------------------------------------------------
class LuaStaticMethodBind
{
public:
	int operator() (LuaState& State) const
	{
		return m_Fn(m_Func, State);
	}

	static int luaCaller( lua_State* pLua )
	{
		LuaStaticMethodBind* pBindMethod = (LuaStaticMethodBind*) lua_touserdata(pLua, lua_upvalueindex(1));
		return (*pBindMethod)( NinjaLua::GetState(pLua) );
	}

	typedef int (*LuaCallBindFn)(void* pvFunc, LuaState& State);

private:
	LuaCallBindFn m_Fn;
	void*		  m_Func;


// Static Maker
public:
	static LuaStaticMethodBind Make(LuaCallBindFn pFn, void* pFunc)
	{
		LuaStaticMethodBind obCaller;
		obCaller.m_Fn	= pFn;
		obCaller.m_Func	= pFunc;
		return obCaller;
	}
};


//------------------------------------------------------------------------------------------
// Generate the call templates
//------------------------------------------------------------------------------------------
#define LUA_CALL_CHECK(VALUE)				{if VALUE											{ntPrintf("%s: Bad parameter.\n", rState.FileAndLine()); return 0;}}
#define LUA_CALL_CHECK_ARG(ARGN, NUMARGS)	{if(!LuaValue::Is<P##ARGN>(rState,ARGN-NUMARGS-1))	{ntPrintf("%s: Bad parameter (%d) for bind function, got %s expected %s.\n",  rState.FileAndLine(), ARGN, NinjaLua::LuaValue::TypeName(rState,ARGN-NUMARGS-1), NinjaLua::LuaValue::ArgTypeName<P##ARGN>()); return 0;}}
#define LUA_CALL_CHECK_ARG_COUNT(NUMARGS)	{if(lua_gettop(&(*rState)) < NUMARGS)				{ntPrintf("%s: Incorrect number of arguments for bind function.\n", rState.FileAndLine()); return 0;}}

//------------------------------------------------------------------------------------------
// With return values...
//------------------------------------------------------------------------------------------
#define LUA_CALL_FUNC(FUNC)		return LuaValue::Push<RET>(rState, FUNC)
#define LUA_CALL_TEMPLATE		template <typename CLASS, typename RET> 
#define LUA_CALL_RETURN			RET
#define LUA_CALL_SPECIALISED	
#include "nl_Luacall.h"


//------------------------------------------------------------------------------------------
// ...and void specialised templates.
//------------------------------------------------------------------------------------------
#define LUA_CALL_FUNC(FUNC)		UNUSED(FUNC); UNUSED(rState); return 0
#define LUA_CALL_TEMPLATE		template <typename CLASS> 
#define LUA_CALL_RETURN			void
#define LUA_CALL_SPECIALISED	<CLASS, void>
#include "nl_Luacall.h"


//------------------------------------------------------------------------------------------
//!
//!	LuaMethodBind_Helper
//! Read/Write Read-Only Member	
//!
//------------------------------------------------------------------------------------------
struct LuaMethodBind_Helper
{
	// No Params
	template<typename CLASS, typename RET> // non-const 
	static LuaMethodBind Maker(RET (CLASS::*pFunc)(), void* pvFunc)        
	{return LuaCall<CLASS, RET>::Get(pFunc,pvFunc);}

	template<typename CLASS, typename RET> // const version
	static LuaMethodBind Maker(RET (CLASS::*pFunc)() const, void* pvFunc) 
	{return Maker((RET (CLASS::*)()) pFunc, pvFunc);}

	template<typename RET> 
	static LuaStaticMethodBind Maker(RET (pFunc)())        
// AHOY! doesnt compile on GCC!	{return LuaStaticCall<RET, RET>::Get(pFunc,pvFunc);}
	{return LuaStaticCall<RET, RET>::Get(pFunc);}

	// 1 Param
	template<typename CLASS, typename RET, typename P1> 
	static LuaMethodBind Maker(RET (CLASS::*pFunc)(P1), void* pvFunc)  
	{ return LuaCall<CLASS, RET>::Get(pFunc,pvFunc);	}

	template<typename CLASS, typename RET, typename P1> 
	static LuaMethodBind Maker(RET (CLASS::*pFunc)(P1) const, void* pvFunc)  
	{ typedef RET (CLASS::*FuncPtr)(P1); return Maker( (FuncPtr) pFunc, pvFunc ); }


	// 2 Params
	template<typename CLASS, typename RET, typename P1, typename P2> 
	static LuaMethodBind Maker(RET (CLASS::*pFunc)(P1,P2), void* pvFunc) 
	{return LuaCall<CLASS, RET>::Get(pFunc,pvFunc);}

	template<typename CLASS, typename RET, typename P1, typename P2> 
	static LuaMethodBind Maker(RET (CLASS::*pFunc)(P1,P2) const, void* pvFunc) 
	{ typedef RET (CLASS::*FuncPtr)(P1, P2); return Maker( (FuncPtr) pFunc, pvFunc ); }

	// 3 Params
	template<typename CLASS, typename RET, typename P1, typename P2, typename P3> 
	static LuaMethodBind Maker(RET (CLASS::*pFunc)(P1,P2,P3), void* pvFunc) 
	{return LuaCall<CLASS, RET>::Get(pFunc,pvFunc);}

	template<typename CLASS, typename RET, typename P1, typename P2, typename P3> 
	static LuaMethodBind Maker(RET (CLASS::*pFunc)(P1,P2,P3) const, void* pvFunc) 
	{ typedef RET (CLASS::*FuncPtr)(P1,P2,P3); return Maker( (FuncPtr) pFunc, pvFunc ); }


	// 4 Params
	template<typename CLASS, typename RET, typename P1, typename P2, typename P3, typename P4> 
	static LuaMethodBind Maker(RET (CLASS::*pFunc)(P1,P2,P3,P4), void* pvFunc) 
	{return LuaCall<CLASS, RET>::Get(pFunc,pvFunc);}

	template<typename CLASS, typename RET, typename P1, typename P2, typename P3, typename P4> 
	static LuaMethodBind Maker(RET (CLASS::*pFunc)(P1,P2,P3,P4) const, void* pvFunc) 
	{ typedef RET (CLASS::*FuncPtr)(P1,P2,P3,P4); return Maker( (FuncPtr) pFunc, pvFunc ); }


	// 5 Params
	template<typename CLASS, typename RET, typename P1, typename P2, typename P3, typename P4, typename P5> 
	static LuaMethodBind Maker(RET (CLASS::*pFunc)(P1,P2,P3,P4,P5), void* pvFunc) 
	{return LuaCall<CLASS, RET>::Get(pFunc,pvFunc);}

	template<typename CLASS, typename RET, typename P1, typename P2, typename P3, typename P4, typename P5> 
	static LuaMethodBind Maker(RET (CLASS::*pFunc)(P1,P2,P3,P4,P5) const, void* pvFunc) 
	{ typedef RET (CLASS::*FuncPtr)(P1,P2,P3,P4,P5); return Maker( (FuncPtr) pFunc, pvFunc ); }


	// 6 Params - This should be the absolute limit...
	template<typename CLASS, typename RET, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6> 
	static LuaMethodBind Maker(RET (CLASS::*pFunc)(P1,P2,P3,P4,P5,P6), void* pvFunc) 
	{return LuaCall<CLASS, RET>::Get(pFunc,pvFunc);}

	template<typename CLASS, typename RET, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6> 
	static LuaMethodBind Maker(RET (CLASS::*pFunc)(P1,P2,P3,P4,P5,P6) const, void* pvFunc) 
	{ typedef RET (CLASS::*FuncPtr)(P1,P2,P3,P4,P5,P6); return Maker( (FuncPtr) pFunc, pvFunc ); }
};


//------------------------------------------------------------------------------------------
//!
//!	LuaMemberBind                                                                                 
//! Read/Write Read-Only Member	
//!
//------------------------------------------------------------------------------------------
class LuaMemberBind
{
public:
	int Set(void* pObj, LuaState& pLua) const
	{
		if(!m_Setter)
		{
			ntPrintf("Error - read only setter used\n");
			return (0);
		}

		m_Setter(pObj, m_Offset, pLua);
		return (0);
	}

	int Get(void* pObj, LuaState& pLua) const
	{
		if(!m_Getter)
		{
			ntPrintf("Error - write only setter used\n");
			return (0);
		}
		return m_Getter(pObj, m_Offset, pLua);
	}

private:
	typedef void (*MemberSet)(void*, intptr_t, LuaState&);
	typedef int  (*MemberGet)(void*, intptr_t, LuaState&);

	MemberSet m_Setter;
	MemberGet m_Getter;
	intptr_t		  m_Offset;


// Static Maker Function
public:
	template<typename CLASS, typename TYPE>
	static LuaMemberBind Maker(TYPE CLASS::*pMember, bool bReadOnly = true)
	{
		LuaMemberBind obMember;
		obMember.m_Setter = 0;
		if(!bReadOnly)
			obMember.m_Setter = SetMember<CLASS, TYPE>;
		obMember.m_Getter = GetMember<CLASS, TYPE>;
		obMember.m_Offset = *((intptr_t*) &pMember);
		return obMember;
	}

// Helper Functions to create the setters and getters
private:
	template<typename CLASS, typename TYPE>
	static void SetMember(void* pObj, intptr_t iIndex, LuaState& pLua)
	{
		typedef TYPE CLASS::*Member;

		CLASS* myClass = (CLASS*) pObj;
		Member mMember = *(Member*) &iIndex;

		myClass->*mMember = LuaValue::Get<TYPE>(pLua, -1);
	}

	template<typename CLASS, typename TYPE>
	static int GetMember(void* pObj, intptr_t iIndex, LuaState& pLua)
	{
		typedef TYPE CLASS::*Member;

		CLASS* myClass = (CLASS*) pObj;
		Member mMember = *(Member*) &iIndex;

		return LuaValue::Push<TYPE>(pLua, myClass->*mMember);
	}
};

//------------------------------------------------------------------------------------------
//!
//!	LuaBinding
//!	Either a binding to a member or a method (cooked or raw).
//!
//------------------------------------------------------------------------------------------
class LuaBinding
{
public:
	int Call(LuaState& State, void* pObj) const
	{
		switch(m_Type)
		{
			case METHOD:
				lua_pushlightuserdata(State, (void*) &m_CookedMethod);
				lua_pushlightuserdata(State, pObj);
				lua_pushcclosure(State, &LuaMethodBind::luaCaller, 2);
				return 1;
			case METHOD_RAW:
				lua_pushlightuserdata(State, (void*) &m_RawMethod);
				lua_pushlightuserdata(State, pObj);
				lua_pushcclosure(State, &LuaMethodBind::luaCaller, 2);
				return 1;
			case METHOD_GET:
				return m_CookedMethod(pObj, State);
			case MEMBER:
				return m_Member.Get(pObj, State);
			default:
				ntPrintf("Bad Error #1.\n");
				return 0;
		}
	}

	int Get(LuaState& State, void* pObj) const
	{
		if(m_Type == MEMBER)
			return m_Member.Get(pObj, State);

		ntPrintf("Bad Error #2.\n");
		return 0;
	}

	int Set(LuaState& pLua, void* pObj) const
	{
		if(m_Type == MEMBER)
			return m_Member.Set(pObj, pLua);
		else if(m_Type == METHOD_SET)
			return m_CookedMethod(pObj, pLua);

		ntPrintf("Bad Error #3.\n");
		return 0;
	}

//private:
	enum {MEMBER, METHOD, METHOD_RAW, METHOD_GET, METHOD_SET, STATIC_METHOD} m_Type;
	union 
	{
		LuaMethodBind			m_CookedMethod;
		LuaStaticMethodBind		m_StaticMethod;
		LuaRawMethodBind		m_RawMethod;
		LuaMemberBind			m_Member;
	};

//  Static Binding Creation Functions
// -----------------------------------
public:
	// Cooked Methods
	template<typename TYPE>
	static LuaBinding Method(TYPE Func, void* pFunc)
	{
		LuaBinding obBinder;
		obBinder.m_Type		    = METHOD;
		obBinder.m_CookedMethod	= LuaMethodBind_Helper::Maker(Func, pFunc);
		return obBinder;
	}

	// Cooked Accessor Get
	template<typename TYPE>
	static LuaBinding MethodGet(TYPE Func, void* pFunc)
	{
		LuaBinding obBinder;
		obBinder.m_Type		    = METHOD_GET;
		obBinder.m_CookedMethod	= LuaMethodBind_Helper::Maker(Func, pFunc);
		return obBinder;
	}

	// Cooked Accessor Set
	template<typename TYPE>
	static LuaBinding MethodSet(TYPE Func, void* pFunc)
	{
		LuaBinding obBinder;
		obBinder.m_Type		    = METHOD_SET;
		obBinder.m_CookedMethod	= LuaMethodBind_Helper::Maker(Func, pFunc);
		return obBinder;
	}

	// Raw Methods
	template<typename CLASS>
	static LuaBinding MethodRaw(int (CLASS::*)(LuaState&), void* pFunc)
	{
		LuaBinding obBinder;
		obBinder.m_Type		= METHOD_RAW;
		obBinder.m_RawMethod = LuaRawMethodBind::Maker<CLASS>(pFunc);
		return obBinder;
	}

	// Static Methods
	template<typename TYPE>
	static LuaBinding StaticMethod( TYPE Func, void* pFunc )
	{
		LuaBinding obBinder;
		obBinder.m_Type			= STATIC_METHOD;
		obBinder.m_StaticMethod = LuaMethodBind_Helper::Maker(Func, pFunc);
		return obBinder;
	}

	// Members
	template<typename CLASS, typename TYPE>
	static LuaBinding Member(TYPE CLASS::*pMember, bool bReadOnly = true)
	{
		LuaBinding obBinder;
		obBinder.m_Type   = MEMBER;
		obBinder.m_Member = LuaMemberBind::Maker<CLASS,TYPE>(pMember, bReadOnly);
		return obBinder;
	}
};

//------------------------------------------------------------------------------------------
//!
//!	LuaExposedContainer
//!	Exposed element Lua container
//!
//------------------------------------------------------------------------------------------

class LuaExposedContainer
{
public:
	typedef ntstd::MultiMap<CHashedString, const LuaBinding* > Map;
	typedef ntstd::pair<CHashedString, const LuaBinding* > Pair;

	LuaExposedContainer() {m_pInherited=0;}

	// Add an exposed member
	void Add(CHashedString pcName, const LuaBinding& obLuaBinding, bool bGet); 
	void Inherit(LuaExposedContainer** pCont, CHashedString pcChild, CHashedString pcParent);
	
	// Find an exposed member
	const LuaBinding* Find(CHashedString pcName, bool bGet) const; 

private:
	Map m_ExposedGet;
	Map m_ExposedSet;
	LuaExposedContainer** m_pInherited;
};


//------------------------------------------------------------------------------------------
//!
//!	ExposedLuaInit
//!	Exposed element Lua container
//!
//------------------------------------------------------------------------------------------

struct ExposedLuaInit
{

	typedef void (*InitFunction)(LuaState&);

	ExposedLuaInit( InitFunction pFunc, LuaExposedContainer*& prLuaContainer ) : 
		m_Func( pFunc ),
		m_prLuaContainer( prLuaContainer )
	{ 
		m_Next = gExposedLuaList;
		gExposedLuaList = this;
		m_iSafe1 = 0x5AFE5AFE;
		m_iSafe2 = 0x5AFE5AFE;
	}

	// Called during initialisation 
	void Init(LuaState& State)
	{
		m_Func(State);

		if( m_Next )
			m_Next->Init(State);
	}

	// Called to free up all the non-static data
	void Free(void);

	// Function called during the initalisation of the registered Lua componenets
	InitFunction			m_Func;

	// Pointer to the next element
	ExposedLuaInit*			m_Next;

	// 
	int						m_iSafe1;
	LuaExposedContainer*&	m_prLuaContainer;
	int						m_iSafe2;

	// Global list entry point
	static ExposedLuaInit*	gExposedLuaList;
};

struct CallTypeRaw
{
	//------------------------------------------------------------------------------------------
	//!
	//!	CallType1Arg
	//! Specialised version of CallType1Arg where the only argument is a state pointer
	//!
	//------------------------------------------------------------------------------------------
	static int Call( lua_State* pLua )
	{
		if(!pLua) return 0; // This can be removed when gcc is fixed

		typedef int (*CallFuncType)(LuaState&);
		NinjaLua::LuaState& obState = NinjaLua::GetState( pLua );
		CallFuncType pFunc = (CallFuncType) lua_touserdata( pLua, lua_upvalueindex(1) );
		return pFunc(obState);
	}
};

#define CHECKARG(n)																								\
{																												\
	if(!NinjaLua::LuaValue::Is<P##n>(obState, n))																\
	{																											\
		LogLuaWarning(obState, "Bad Parameter, Expecting %s but got %s.\n",										\
						NinjaLua::LuaValue::ArgTypeName<P##n>(), NinjaLua::LuaValue::TypeName(obState, n));		\
		return 0;																								\
	}																											\
}

template<typename RET>
struct CallType0
{
	//------------------------------------------------------------------------------------------
	//!
	//!	CallType
	//! Class for registering calls without params
	//!
	//------------------------------------------------------------------------------------------
	static int Call( lua_State* pLua )
	{
		typedef RET (*CallFuncType)();
		NinjaLua::LuaState& obState = NinjaLua::GetState( pLua );
		CallFuncType pFunc = (CallFuncType) lua_touserdata( pLua, lua_upvalueindex(1) );
		NinjaLua::LuaValue::Push( obState, pFunc());
		return (1);
	}
};


template<typename RET, typename P1>
struct CallType1
{
	//------------------------------------------------------------------------------------------
	//!
	//!	CallType1Arg
	//! Class for registering calls with only 1 argument
	//!
	//------------------------------------------------------------------------------------------
	static int Call( lua_State* pLua )
	{
		typedef RET (*CallFuncType)(P1);
		NinjaLua::LuaState& obState = NinjaLua::GetState( pLua );
		CallFuncType pFunc = (CallFuncType) lua_touserdata( pLua, lua_upvalueindex(1) );

		// Check arguments
		CHECKARG(1);

		// Call the function, return the result
		NinjaLua::LuaValue::Push( obState, pFunc(NinjaLua::LuaValue::Get<P1>( obState, 1)));
		return (1);
	}
};

template<typename RET, typename P1, typename P2>
struct CallType2
{
	//------------------------------------------------------------------------------------------
	//!
	//!	CallType2Arg
	//! Class for registering calls with only 2 arguments
	//!
	//------------------------------------------------------------------------------------------
	static int Call( lua_State* pLua )
	{
		typedef RET (*CallFuncType)(P1, P2);
		NinjaLua::LuaState& obState = NinjaLua::GetState( pLua );
		CallFuncType pFunc = (CallFuncType) lua_touserdata( pLua, lua_upvalueindex(1) );

		// Check arguments
		CHECKARG(1);
		CHECKARG(2);

		// Call the function, return the type
		NinjaLua::LuaValue::Push( obState, pFunc(NinjaLua::LuaValue::Get<P1>( obState, 1), 
												NinjaLua::LuaValue::Get<P2>( obState, 2)));
		return (1);
	}
};


template<typename RET, typename P1, typename P2, typename P3>
struct CallType3
{
	//------------------------------------------------------------------------------------------
	//!
	//!	CallType3Arg
	//! Class for registering calls with 3 arguments
	//!
	//------------------------------------------------------------------------------------------
	static int Call( lua_State* pLua )
	{
		typedef RET (*CallFuncType)(P1,P2,P3);
		NinjaLua::LuaState& obState = NinjaLua::GetState( pLua );
		CallFuncType pFunc = (CallFuncType) lua_touserdata( pLua, lua_upvalueindex(1) );

		// Check arguments
		CHECKARG(1);
		CHECKARG(2);
		CHECKARG(3);

		// Call the function, return the type
		NinjaLua::LuaValue::Push( obState, pFunc(NinjaLua::LuaValue::Get<P1>( obState, 1), 
												NinjaLua::LuaValue::Get<P2>( obState, 2), 
												NinjaLua::LuaValue::Get<P3>( obState, 3)));
		return (1);
	}
};


template<typename RET, typename P1, typename P2, typename P3, typename P4>
struct CallType4
{
	//------------------------------------------------------------------------------------------
	//!
	//!	CallType4Arg
	//! Class for registering calls with 4 arguments
	//!
	//------------------------------------------------------------------------------------------
	static int Call( lua_State* pLua )
	{
		typedef RET (*CallFuncType)(P1,P2,P3,P4);
		NinjaLua::LuaState& obState = NinjaLua::GetState( pLua );
		CallFuncType pFunc = (CallFuncType) lua_touserdata( pLua, lua_upvalueindex(1) );

		// Check arguments
		CHECKARG(1);
		CHECKARG(2);
		CHECKARG(3);
		CHECKARG(4);

		// Call the function, return the type
		NinjaLua::LuaValue::Push( obState, pFunc(NinjaLua::LuaValue::Get<P1>( obState, 1), 
												NinjaLua::LuaValue::Get<P2>( obState, 2), 
												NinjaLua::LuaValue::Get<P3>( obState, 3), 
												NinjaLua::LuaValue::Get<P4>( obState, 4)));
		return (1);
	}
};


template<typename RET, typename P1, typename P2, typename P3, typename P4, typename P5>
struct CallType5
{
	//------------------------------------------------------------------------------------------
	//!
	//!	CallType5Arg
	//! Class for registering calls with 5 arguments
	//!
	//------------------------------------------------------------------------------------------
	static int Call( lua_State* pLua )
	{
		typedef RET (*CallFuncType)(P1,P2,P3,P4,P5);
		NinjaLua::LuaState& obState = NinjaLua::GetState( pLua );
		CallFuncType pFunc = (CallFuncType) lua_touserdata( pLua, lua_upvalueindex(1) );

		// Check arguments
		CHECKARG(1);
		CHECKARG(2);
		CHECKARG(3);
		CHECKARG(4);
		CHECKARG(5);

		// Call the function, return the type
		NinjaLua::LuaValue::Push( obState, pFunc(NinjaLua::LuaValue::Get<P1>( obState, 1), 
												NinjaLua::LuaValue::Get<P2>( obState, 2), 
												NinjaLua::LuaValue::Get<P3>( obState, 3), 
												NinjaLua::LuaValue::Get<P4>( obState, 4),
												NinjaLua::LuaValue::Get<P5>( obState, 5)));
		return (1);
	}
};

template<typename RET, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
struct CallType6
{
	//------------------------------------------------------------------------------------------
	//!
	//!	CallType6Arg
	//! Class for registering calls with 6 arguments
	//!
	//------------------------------------------------------------------------------------------
	static int Call( lua_State* pLua )
	{
		typedef RET (*CallFuncType)(P1,P2,P3,P4,P5,P6);
		NinjaLua::LuaState& obState = NinjaLua::GetState( pLua );
		CallFuncType pFunc = (CallFuncType) lua_touserdata( pLua, lua_upvalueindex(1) );

		// Check arguments
		CHECKARG(1);
		CHECKARG(2);
		CHECKARG(3);
		CHECKARG(4);
		CHECKARG(5);
		CHECKARG(6);

		// Call the function, return the type
		NinjaLua::LuaValue::Push( obState, pFunc(NinjaLua::LuaValue::Get<P1>( obState, 1), 
												NinjaLua::LuaValue::Get<P2>( obState, 2), 
												NinjaLua::LuaValue::Get<P3>( obState, 3), 
												NinjaLua::LuaValue::Get<P4>( obState, 4),
												NinjaLua::LuaValue::Get<P5>( obState, 5),
												NinjaLua::LuaValue::Get<P6>( obState, 6)));
		return (1);
	}
};

template<>
struct CallType0<void>
{
	//------------------------------------------------------------------------------------------
	//!
	//!	CallType
	//! Class for registering calls without params
	//!
	//------------------------------------------------------------------------------------------
	static int Call( lua_State* pLua )
	{
		typedef void (*CallFuncType)();
		CallFuncType pFunc = (CallFuncType) lua_touserdata( pLua, lua_upvalueindex(1) );
		pFunc();
		return (0);
	}
};


template<typename P1>
struct CallType1<void, P1>
{
	//------------------------------------------------------------------------------------------
	//!
	//!	CallType1Arg
	//! Class for registering calls with only 1 argument
	//!
	//------------------------------------------------------------------------------------------
	static int Call( lua_State* pLua )
	{
		typedef void (*CallFuncType)(P1);
		NinjaLua::LuaState& obState = NinjaLua::GetState( pLua );
		CallFuncType pFunc = (CallFuncType) lua_touserdata( pLua, lua_upvalueindex(1) );

		// Check arguments
		CHECKARG(1);

		// Call the bind function
		pFunc(NinjaLua::LuaValue::Get<P1>( obState, 1));
		return (0);
	}
};

template<typename P1, typename P2>
struct CallType2<void,P1,P2>
{
	//------------------------------------------------------------------------------------------
	//!
	//!	CallType2Arg
	//! Class for registering calls with only 2 arguments
	//!
	//------------------------------------------------------------------------------------------
	static int Call( lua_State* pLua )
	{
		typedef void (*CallFuncType)(P1,P2);
		NinjaLua::LuaState& obState = NinjaLua::GetState( pLua );
		CallFuncType pFunc = (CallFuncType) lua_touserdata( pLua, lua_upvalueindex(1) );

		// Check arguments
		CHECKARG(1);
		CHECKARG(2);

		// Call the bind function
		pFunc(NinjaLua::LuaValue::Get<P1>( obState, 1), NinjaLua::LuaValue::Get<P2>( obState, 2));
		return (0);
	}
};


template<typename P1, typename P2, typename P3>
struct CallType3<void,P1,P2,P3>
{
	//------------------------------------------------------------------------------------------
	//!
	//!	CallType3Arg
	//! Class for registering calls with 3 arguments
	//!
	//------------------------------------------------------------------------------------------
	static int Call( lua_State* pLua )
	{
		typedef void (*CallFuncType)(P1,P2,P3);
		NinjaLua::LuaState& obState = NinjaLua::GetState( pLua );
		CallFuncType pFunc = (CallFuncType) lua_touserdata( pLua, lua_upvalueindex(1) );

		// Check arguments
		CHECKARG(1);
		CHECKARG(2);
		CHECKARG(3);

		// Call the bind function
		pFunc(	NinjaLua::LuaValue::Get<P1>( obState, 1), 
				NinjaLua::LuaValue::Get<P2>( obState, 2), 
				NinjaLua::LuaValue::Get<P3>( obState, 3));
		return (0);
	}
};


template<typename P1, typename P2, typename P3, typename P4>
struct CallType4<void,P1,P2,P3,P4>
{
	//------------------------------------------------------------------------------------------
	//!
	//!	CallType4Arg
	//! Class for registering calls with 4 arguments
	//!
	//------------------------------------------------------------------------------------------
	static int Call( lua_State* pLua )
	{
		typedef void (*CallFuncType)(P1,P2,P3,P4);
		NinjaLua::LuaState& obState = NinjaLua::GetState( pLua );
		CallFuncType pFunc = (CallFuncType) lua_touserdata( pLua, lua_upvalueindex(1) );

		// Check arguments
		CHECKARG(1);
		CHECKARG(2);
		CHECKARG(3);
		CHECKARG(4);

		// Call the bind function
		pFunc(	NinjaLua::LuaValue::Get<P1>( obState, 1), 
				NinjaLua::LuaValue::Get<P2>( obState, 2), 
				NinjaLua::LuaValue::Get<P3>( obState, 3),
				NinjaLua::LuaValue::Get<P4>( obState, 4));
		return (0);
	}
};


template<typename P1, typename P2, typename P3, typename P4, typename P5>
struct CallType5<void,P1,P2,P3,P4,P5>
{
	//------------------------------------------------------------------------------------------
	//!
	//!	CallType5Arg
	//! Class for registering calls with 5 arguments
	//!
	//------------------------------------------------------------------------------------------
	static int Call( lua_State* pLua )
	{
		typedef void (*CallFuncType)(P1,P2,P3,P4,P5);
		NinjaLua::LuaState& obState = NinjaLua::GetState( pLua );
		CallFuncType pFunc = (CallFuncType) lua_touserdata( pLua, lua_upvalueindex(1) );

		// Check arguments
		CHECKARG(1);
		CHECKARG(2);
		CHECKARG(3);
		CHECKARG(4);
		CHECKARG(5);

		// Call the bind function
		pFunc(	NinjaLua::LuaValue::Get<P1>( obState, 1), 
				NinjaLua::LuaValue::Get<P2>( obState, 2), 
				NinjaLua::LuaValue::Get<P3>( obState, 3),
				NinjaLua::LuaValue::Get<P4>( obState, 4),
				NinjaLua::LuaValue::Get<P5>( obState, 5));
		return (0);
	}
};

template<typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
struct CallType6<void,P1,P2,P3,P4,P5,P6>
{
	//------------------------------------------------------------------------------------------
	//!
	//!	CallType6Arg
	//! Class for registering calls with 6 arguments
	//!
	//------------------------------------------------------------------------------------------
	static int Call( lua_State* pLua )
	{
		typedef void (*CallFuncType)(P1,P2,P3,P4,P5,P6);
		NinjaLua::LuaState& obState = NinjaLua::GetState( pLua );
		CallFuncType pFunc = (CallFuncType) lua_touserdata( pLua, lua_upvalueindex(1) );

		// Check arguments
		CHECKARG(1);
		CHECKARG(2);
		CHECKARG(3);
		CHECKARG(4);
		CHECKARG(5);
		CHECKARG(6);

		// Call the bind function
		pFunc(	NinjaLua::LuaValue::Get<P1>( obState, 1), 
				NinjaLua::LuaValue::Get<P2>( obState, 2), 
				NinjaLua::LuaValue::Get<P3>( obState, 3),
				NinjaLua::LuaValue::Get<P4>( obState, 4),
				NinjaLua::LuaValue::Get<P5>( obState, 5),
				NinjaLua::LuaValue::Get<P6>( obState, 6));
		return (0);
	}
};

class ParentSet
{
public:
	static void Add(CHashedString pcChild, CHashedString pcParent);
	static CHashedString Get(CHashedString pcChild);
	static void Clean();

private:
	typedef ntstd::MultiMap<CHashedString, CHashedString> Map;
	static Map* m_pMap;
};

//------------------------------------------------------------------------------------------
//!
//!	LUA_EXPOSED_*
//!	Macros to exposed elements of classes to Lua. 
//!
//------------------------------------------------------------------------------------------

#ifndef _RELEASE
	#define OUTPUT_BADBINDINFO																													\
			lua_getmetatable(State, 1);																											\
			NinjaLua::LuaObject obMt=NinjaLua::LuaObject(-1, State, false);																		\
			const char* pcObjType = 0;																											\
			obMt.Get("_type", pcObjType);																										\
			if(!pBind)																															\
			{																																	\
				const char* pcObjName = "Unknown";																								\
				const NinjaLua::LuaBinding* pNameBind = m_gExposedElements->Find("name", true);													\
				if(pNameBind)																													\
				{																																\
					int iNameRet = pNameBind->Call(State, pObj);																				\
					if(iNameRet)																												\
					{																															\
						pcObjName = NinjaLua::LuaValue::Get<const char*>(State, -1);															\
						lua_pop(State, iNameRet);																								\
					}																															\
				}																																\
				ntPrintf("%s: Binding `%s` not found on %s (name:%s).\n", State.FileAndLine(1), ntStr::GetString(pcName), pcObjType, pcObjName);\
			}																																	\
			else																																\
				ntPrintf("%s: Attempt to call Binding `%s` on a null %s.\n", State.FileAndLine(1), ntStr::GetString(pcName), pcObjType);
#else
	#define OUTPUT_BADBINDINFO
#endif



#define LUA_EXPOSED_START_INTERNAL(CLASS, INHERITED)																							\
	class CLASS##ExposedLua																														\
	{																																			\
	public:																																		\
		static NinjaLua::LuaExposedContainer*	m_gExposedElements;																				\
		typedef CLASS MYCLASS;																													\
		CLASS##ExposedLua()																														\
		{																																		\
			static NinjaLua::ExposedLuaInit SayCheese( __RegisterExposedLua, m_gExposedElements );												\
		}																																		\
																																				\
		static int __Do(NinjaLua::LuaState& State)																								\
		{																																		\
			lua_checkstack(State, 2);																											\
			CHashedString pcName(lua_tohashkey(State, 2));																						\
			const NinjaLua::LuaBinding* pBind = m_gExposedElements->Find(pcName, true);															\
			CLASS* pObj = *((CLASS**)lua_touserdata(State, 1));																					\
			if(pBind && pObj)																													\
				return pBind->Call(State, pObj);																								\
																																				\
			ntPrintf("### LUA ERROR ### - Bad Binding (%s).\n", State.FileAndLine());																						\
			OUTPUT_BADBINDINFO																													\
			return 0;																															\
		}																																		\
		static int __Set(NinjaLua::LuaState& State)																								\
		{																																		\
			lua_checkstack(State, 2);																											\
			CHashedString pcName(lua_tohashkey(State, 2));/*CHashedString pcName = lua_tostring(State, 2);*/									\
			CLASS* pObj = *((CLASS**)lua_touserdata(State, 1));																					\
			const NinjaLua::LuaBinding* pBind = m_gExposedElements->Find(pcName, false);														\
			if(pBind)																															\
				return pBind->Set(State, pObj);																									\
																																				\
			lua_getmetatable(State, 1);																											\
			NinjaLua::LuaObject obMt=NinjaLua::LuaObject(-1, State, false);																		\
			const char* pcObjType = 0;																											\
			obMt.Get("_type", pcObjType);																										\
			ntPrintf("%s: Binding `%s` not found on %s.\n", State.FileAndLine(1), ntStr::GetString(pcName), pcObjType);							\
			return 0;																															\
		}																																		\
		static int __Eq(NinjaLua::LuaState& State)																								\
		{																																		\
			lua_checkstack(State, 2);																											\
			CLASS* pLHS = *((CLASS**)lua_touserdata(State, 1));																					\
			CLASS* pRHS = *((CLASS**)lua_touserdata(State, 2));																					\
			NinjaLua::LuaValue::Push(State, pLHS==pRHS);																						\
			return 1;																															\
		}																																		\
																																				\
		static void __RegisterExposedLua(NinjaLua::LuaState& State);																			\
	};																																			\
	NinjaLua::LuaExposedContainer* CLASS##ExposedLua::m_gExposedElements = 0;																	\
	NinjaLua::LuaExposedContainer** CLASS##ExposedLua_ExposedElements = &CLASS##ExposedLua::m_gExposedElements;									\
	void CLASS##ExposedLua_RegisterClass(NinjaLua::LuaState& State) {CLASS##ExposedLua::__RegisterExposedLua(State);}							\
	static CLASS##ExposedLua CLASS##ExposedLuaInstance;																							\
																																				\
	void CLASS##ExposedLua::__RegisterExposedLua(NinjaLua::LuaState& State)																		\
	{																																			\
		if(m_gExposedElements != 0) return;																										\
		NinjaLua::LuaObject obMt = State.GetMetaTable( "ninjaLua."#CLASS );																		\
		obMt.Set("_type", #CLASS);																												\
		obMt.Set("__index", NinjaLua::LuaObject(State, &CLASS##ExposedLuaInstance.__Do));														\
		obMt.Set("__newindex", NinjaLua::LuaObject(State, &CLASS##ExposedLuaInstance.__Set));													\
		obMt.Set("__eq", NinjaLua::LuaObject(State, &CLASS##ExposedLuaInstance.__Eq));															\
		if(!strcmp(#INHERITED, "NONE"))																											\
			obMt.Set("_parent", #INHERITED);																									\
		m_gExposedElements = NT_NEW NinjaLua::LuaExposedContainer;

#define LUA_EXPOSED_START( CLASS )			LUA_EXPOSED_START_INTERNAL(CLASS, NONE)

#define LUA_EXPOSED_START_INHERITED(CLASS, INHERIT)																								\
	extern NinjaLua::LuaExposedContainer** INHERIT##ExposedLua_ExposedElements;																	\
	extern void INHERIT##ExposedLua_RegisterClass(NinjaLua::LuaState& State);																	\
	LUA_EXPOSED_START_INTERNAL(CLASS, INHERIT)																									\
	if(*INHERIT##ExposedLua_ExposedElements == 0)																								\
	{																																			\
		ntPrintf("Registering Parent Metatable " #INHERIT " for " #CLASS "\n");																	\
		INHERIT##ExposedLua_RegisterClass(State);																								\
	}																																			\
	m_gExposedElements->Inherit(INHERIT##ExposedLua_ExposedElements, #CLASS, #INHERIT);

#define LUA_EXPOSED_MEMBER_READONLY(NAME, VAR, HELP)																							\
	static NinjaLua::LuaBinding ob##NAME##VAR = NinjaLua::LuaBinding::Member(&MYCLASS::VAR, true);												\
	m_gExposedElements->Add(#NAME, ob##NAME##VAR, true);

#define LUA_EXPOSED_MEMBER_READWRITE(NAME, VAR, HELP)																							\
	static NinjaLua::LuaBinding ob##NAME##VAR = NinjaLua::LuaBinding::Member(&MYCLASS::VAR, false);												\
	m_gExposedElements->Add(#NAME, ob##NAME##VAR, true);

#define LUA_EXPOSED_METHOD(NAME, VAR, HELP, PARAMS, PARAMSHELP)																					\
	static NinjaLua::LuaMethodBind::Storage<MYCLASS> obS##NAME##VAR = {(NinjaLua::LuaMethodBind::Storage<MYCLASS>::FuncPtr) &MYCLASS::VAR};		\
	static NinjaLua::LuaBinding ob##NAME##VAR = NinjaLua::LuaBinding::Method(&MYCLASS::VAR, &obS##NAME##VAR);									\
	m_gExposedElements->Add(#NAME, ob##NAME##VAR, true);

#define LUA_EXPOSED_METHOD_RAW(NAME, VAR, HELP, PARAMS, PARAMSHELP)																				\
	static NinjaLua::LuaMethodBind::Storage<MYCLASS> obS##NAME##VAR = {(NinjaLua::LuaMethodBind::Storage<MYCLASS>::FuncPtr) &MYCLASS::VAR};		\
	static NinjaLua::LuaBinding ob##NAME##VAR = NinjaLua::LuaBinding::MethodRaw(&MYCLASS::VAR, &obS##NAME##VAR);								\
	m_gExposedElements->Add(#NAME, ob##NAME##VAR, true);

#define LUA_EXPOSED_METHOD_GET(NAME, VAR, HELP)																									\
	static NinjaLua::LuaMethodBind::Storage<MYCLASS> obS##NAME##VAR = {(NinjaLua::LuaMethodBind::Storage<MYCLASS>::FuncPtr) &MYCLASS::VAR};		\
	static NinjaLua::LuaBinding ob##NAME##VAR = NinjaLua::LuaBinding::MethodGet(&MYCLASS::VAR, &obS##NAME##VAR);								\
	m_gExposedElements->Add(#NAME, ob##NAME##VAR, true);

#define LUA_EXPOSED_METHOD_SET(NAME, VAR, HELP)																									\
	static NinjaLua::LuaMethodBind::Storage<MYCLASS> obS##NAME##VAR = {(NinjaLua::LuaMethodBind::Storage<MYCLASS>::FuncPtr) &MYCLASS::VAR};		\
	static NinjaLua::LuaBinding ob##NAME##VAR = NinjaLua::LuaBinding::MethodSet(&MYCLASS::VAR, &obS##NAME##VAR);								\
	m_gExposedElements->Add(#NAME, ob##NAME##VAR, false);


#define LUA_EXPOSED_END( CLASS )																												\
		}																																		\

#define LUA_EXPOSED_CONTAINER(CLASS, GET_METHOD, SET_METHOD)																					\
	class CLASS##ExposedLua {																													\
	public:																																		\
		CLASS##ExposedLua()																														\
		{																																		\
			static NinjaLua::LuaExposedContainer* pNullCont = 0;																						\
			static NinjaLua::ExposedLuaInit SayCheese(__RegisterExposedLua, pNullCont);															\
		}																																		\
																																				\
		static int __Do(NinjaLua::LuaState& State)																								\
		{																																		\
			lua_checkstack(State, 2);																											\
			CHashedString pcName(lua_tohashkey(State, 2));																						\
			CLASS* pObj = *((CLASS**)lua_touserdata(State, 1));																					\
			return pObj->GET_METHOD(pcName);																									\
		}																																		\
		static int __Set(NinjaLua::LuaState& State)																								\
		{																																		\
			lua_checkstack(State, 2);																											\
			CHashedString pcName(lua_tohashkey(State, 2));																						\
			CLASS* pObj = *((CLASS**)lua_touserdata(State, 1));																					\
			NinjaLua::LuaObject value(3, State);																								\
			return pObj->SET_METHOD(pcName, value);																								\
		}																																		\
		static int __Eq(NinjaLua::LuaState& State)																								\
		{																																		\
			lua_checkstack(State, 2);																											\
			CLASS* pLHS = *((CLASS**)lua_touserdata(State, 1));																					\
			CLASS* pRHS = *((CLASS**)lua_touserdata(State, 2));																					\
			NinjaLua::LuaValue::Push(State, pLHS==pRHS);																						\
			return 1;																															\
		}																																		\
																																				\
		static void __RegisterExposedLua(NinjaLua::LuaState& State);																			\
	};																																			\
	static CLASS##ExposedLua CLASS##ExposedLuaInstance;																							\
																																				\
	void CLASS##ExposedLua::__RegisterExposedLua(NinjaLua::LuaState& State)																		\
	{																																			\
		NinjaLua::LuaObject obMt = State.GetMetaTable("ninjaLua." #CLASS );																		\
		obMt.Set("_type", #CLASS);																												\
		obMt.Set("__index", NinjaLua::LuaObject(State, &CLASS##ExposedLuaInstance.__Do));														\
		obMt.Set("__newindex", NinjaLua::LuaObject(State, &CLASS##ExposedLuaInstance.__Set));													\
		obMt.Set("__eq", NinjaLua::LuaObject(State, &CLASS##ExposedLuaInstance.__Eq));															\
	}


	// ----------------------------------------------------------------------------------------------------------------------
	//  Lua Debug Functions
	// ----------------------------------------------------------------------------------------------------------------------
	void LogLuaWarning(NinjaLua::LuaState& state, const char* pcFormat, ...);
	void LogLuaError(NinjaLua::LuaState& state, const char* pcFormat, ...);
};


#endif // _LUAEXPOSED_H

