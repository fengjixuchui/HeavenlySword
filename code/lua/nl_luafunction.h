#ifndef _LUAFUNCTION_H
#define _LUAFUNCTION_H


namespace NinjaLua
{

//------------------------------------------------------------------------------------------
//!
//!	CLASS:	LuaFunction                                                                           
//!	
//!
//------------------------------------------------------------------------------------------
struct LuaFunction : public LuaObject
{
	explicit LuaFunction() { }

	explicit LuaFunction(const LuaObject& obOther) :
		LuaObject(obOther)
	{
		// If we're not a function, then make the object a nil value
		if(!IsFunction())
			SetNil();
	}

	explicit LuaFunction(LuaState& state, int (*fnPtr)(LuaState&)) :
		LuaObject(state, fnPtr)
	{
		// If we're not a function, then make the object a nil value
		if(!IsFunction())
			SetNil();
	}

	// Assignment operator
	void operator= (const LuaObject& obOther)
	{
		LuaObject::operator =(obOther);

		// If we're not a function, then make the object a nil value
		if(!IsFunction())
			SetNil();
	}

	// Where all C++ code that wants to call Lua functions is routed through
	int Caller(int iArgs, int iResults);


	// ---------------------------------------------------------------------------
	// Function call operator overloads....

	void operator() (void)
	{
		Push();
		Caller(0, 0);
	}

	template< typename P1 >
	void operator() (P1 p1)
	{
		Push();
		LuaValue::Push<P1>(*m_State, p1);
		Caller(1, 0);
	}

	template< typename P1, typename P2 >
	void operator() (P1 p1, P2 p2)
	{
		Push();
		LuaValue::Push<P1>(*m_State, p1);
		LuaValue::Push<P2>(*m_State, p2);
		Caller(2, 0);
	}

	template< typename P1, typename P2, typename P3 >
	void operator() (P1 p1, P2 p2, P3 p3)
	{
		Push();
		LuaValue::Push<P1>(*m_State, p1);
		LuaValue::Push<P2>(*m_State, p2);
		LuaValue::Push<P3>(*m_State, p3);
		Caller(3, 0);
	}

	template< typename P1, typename P2, typename P3, typename P4 >
	void operator() (P1 p1, P2 p2, P3 p3, P4 p4)
	{
		Push();
		LuaValue::Push<P1>(*m_State, p1);
		LuaValue::Push<P2>(*m_State, p2);
		LuaValue::Push<P3>(*m_State, p3);
		LuaValue::Push<P4>(*m_State, p4);
		Caller(4, 0);
	}
};

//------------------------------------------------------------------------------------------
//!
//!	CLASS:	LuaFunctionRet                                                                           
//!	
//!
//------------------------------------------------------------------------------------------

template< typename RET >
struct LuaFunctionRet : public LuaFunction
{
	explicit LuaFunctionRet() { }
	explicit LuaFunctionRet(const LuaObject& obOther) : LuaFunction(obOther) {}

	RET operator() (void)
	{
		Push();
		Caller(0, 1);
		return LuaValue::Get<RET>(*m_State, -1);
	}

	template< typename P1 >
	RET operator() (P1 p1)
	{
		Push();
		LuaValue::Push<P1>(*m_State, p1);
		if(Caller(1, 1))
			return LuaValue::Default<RET>(*m_State);
		return LuaValue::Pop<RET>(*m_State, -1);
	}

	template< typename P1, typename P2 >
	RET operator() (P1 p1, P2 p2)
	{
		Push();
		LuaValue::Push<P1>(*m_State, p1);
		LuaValue::Push<P2>(*m_State, p2);
		if(Caller(2, 1))
			return LuaValue::Default<RET>(*m_State);
		return LuaValue::Pop<RET>(*m_State, -1);
	}

	template< typename P1, typename P2, typename P3 >
	RET operator() (P1 p1, P2 p2, P3 p3)
	{
		Push();
		LuaValue::Push<P1>(*m_State, p1);
		LuaValue::Push<P2>(*m_State, p2);
		LuaValue::Push<P3>(*m_State, p3);
		if(Caller(3, 1))
			return LuaValue::Default<RET>(*m_State);
		return LuaValue::Get<RET>(*m_State, -1);
	}

	template< typename P1, typename P2, typename P3, typename P4 >
	RET operator() (P1 p1, P2 p2, P3 p3, P4 p4)
	{
		Push();
		LuaValue::Push<P1>(*m_State, p1);
		LuaValue::Push<P2>(*m_State, p2);
		LuaValue::Push<P3>(*m_State, p3);
		LuaValue::Push<P4>(*m_State, p4);
		if(Caller(4, 1))
			return LuaValue::Default<RET>(*m_State);
		return LuaValue::Get<RET>(*m_State, -1);
	}

	template< typename P1, typename P2, typename P3, typename P4, typename P5 >
	RET operator() (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
	{
		Push();
		LuaValue::Push<P1>(*m_State, p1);
		LuaValue::Push<P2>(*m_State, p2);
		LuaValue::Push<P3>(*m_State, p3);
		LuaValue::Push<P4>(*m_State, p4);
		LuaValue::Push<P5>(*m_State, p5);
		if(Caller(5, 1))
			return LuaValue::Default<RET>(*m_State);
		return LuaValue::Get<RET>(*m_State, -1);
	}

	template< typename P1, typename P2, typename P3, typename P4, typename P5, typename P6 >
	RET operator() (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
	{
		Push();
		LuaValue::Push<P1>(*m_State, p1);
		LuaValue::Push<P2>(*m_State, p2);
		LuaValue::Push<P3>(*m_State, p3);
		LuaValue::Push<P4>(*m_State, p4);
		LuaValue::Push<P5>(*m_State, p5);
		LuaValue::Push<P6>(*m_State, p6);
		if(Caller(6, 1))
			return LuaValue::Default<RET>(*m_State);
		return LuaValue::Get<RET>(*m_State, -1);
	}
};


//------------------------------------------------------------------------------------------
//!
//!	LuaValue                                                                                 
//!	
//!
//------------------------------------------------------------------------------------------
namespace LuaValue
{
	template<>static bool			Is<LuaFunction>(LuaState& rState, int iIndex)		  {return lua_isfunction(rState, iIndex)!=0;}
	template<>static LuaFunction	Get<LuaFunction>(LuaState& rState, int iIndex)		  {return LuaFunction(LuaObject(iIndex, rState));}
	template<>static LuaFunction	Pop<LuaFunction>(LuaState& rState, int iIndex)		  {LuaAutoPop Pop(rState, 1);return LuaFunction(LuaObject(iIndex, rState));}
	template<>static int			Push<LuaFunction>(LuaState& rState, LuaFunction Value){Value.Push(); if(Value.GetState() != &rState) lua_xmove(*Value.GetState(),rState,1); return 1;}
};

}

#endif //_LUAFUNCTION_H
