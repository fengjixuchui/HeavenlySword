#ifndef _LUAARGS_H
#define _LUAARGS_H

namespace NinjaLua
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class LuaArgs
{
public:
	LuaArgs(LuaState& State) : 
		m_State( State )
	{
	}

	template<typename T>
	void Get(T& v, int i)
	{
		if(Check<T>(i))
			v = LuaValue::Get<T>(m_State,  i );
		else
			LogLuaWarning(m_State, "LuaArgs: Type Mismatch\n");
	}

	template<typename T>
	bool Check(int i)
	{
		return LuaValue::Is<T>(m_State, i );
	}


private:
	LuaState& m_State;
};

};


#endif // _LUAARGS_H
