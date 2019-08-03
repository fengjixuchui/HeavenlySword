//------------------------------------------------------------------------------------------
//!
//!	\file luacall.h
//!
//------------------------------------------------------------------------------------------

#ifndef LUA_CALL_FUNC
#ntError 
#endif

LUA_CALL_TEMPLATE
struct LuaCall LUA_CALL_SPECIALISED
{
	//
	// -------------------------------------------- 
	//
	static int Bind( void* pvObj, void* pvFunc, LuaState& rState )
	{
		LUA_CALL_CHECK_ARG_COUNT(0);
		typedef LUA_CALL_RETURN (CLASS::*FuncPtr)( );
		LUA_CALL_FUNC( ((((CLASS*) pvObj)->*(*(FuncPtr*) pvFunc))( )) );
	}

	static LuaMethodBind Get( LUA_CALL_RETURN (CLASS::*)(), void* pvFunc ) 
	{ 
		return LuaMethodBind::Make( &Bind, pvFunc ); 
	}

	

	//
	// -------------------------------------------- 
	//
	template < typename P1 >
	static int Bind( void* pvObj, void* pvFunc, LuaState& rState )
	{
		CLASS* test1 = (CLASS*)pvObj;
		Dynamics_Lua* test2 = (Dynamics_Lua*)pvObj;
		Physics::System* test3 = (Physics::System*)pvObj;
		UNUSED(test1); UNUSED(test2); UNUSED(test3);

		LUA_CALL_CHECK_ARG_COUNT(1);
		LUA_CALL_CHECK_ARG(1,1);
		typedef LUA_CALL_RETURN (CLASS::*FuncPtr)( P1 );
		LUA_CALL_FUNC(( (((CLASS*) pvObj)->*(*(FuncPtr*) pvFunc))( LuaValue::Get<P1>( rState, -1 ) ) ));
	}

	template < typename P1 > 
	static LuaMethodBind Get( LUA_CALL_RETURN (CLASS::*)(P1), void* pvFunc )  
	{  
		return LuaMethodBind::Make( &Bind<P1>,  pvFunc );  
	}

	//
	// -------------------------------------------- 
	//
	template < typename P1, typename P2 >
	static int Bind( void* pvObj, void* pvFunc, LuaState& rState )
	{
		LUA_CALL_CHECK_ARG_COUNT(2);
		LUA_CALL_CHECK_ARG(1,2); LUA_CALL_CHECK_ARG(2,2);
		typedef LUA_CALL_RETURN (CLASS::*FuncPtr)( P1, P2 );
		LUA_CALL_FUNC(( (((CLASS*) pvObj)->*(*(FuncPtr*) pvFunc))( LuaValue::Get<P1>( rState, -2 ),
																	LuaValue::Get<P2>( rState, -1 ) ) ));
	}

	template < typename P1, typename P2 >
	static LuaMethodBind Get( LUA_CALL_RETURN (CLASS::*)(P1, P2), void* pvFunc )  
	{  
		return LuaMethodBind::Make( &Bind<P1, P2>,  pvFunc );  
	}

	//
	// -------------------------------------------- 
	//
	template < typename P1, typename P2, typename P3 >
	static int Bind( void* pvObj, void* pvFunc, LuaState& rState )
	{
		LUA_CALL_CHECK_ARG_COUNT(3);
		LUA_CALL_CHECK_ARG(1,3); LUA_CALL_CHECK_ARG(2,3); LUA_CALL_CHECK_ARG(3,3);
		typedef LUA_CALL_RETURN (CLASS::*FuncPtr)( P1, P2, P3 );
		LUA_CALL_FUNC(( (((CLASS*) pvObj)->*(*(FuncPtr*) pvFunc))( LuaValue::Get<P1>( rState, -3 ),
																	LuaValue::Get<P2>( rState, -2 ),
																	LuaValue::Get<P3>( rState, -1 )) ));
	}

	template < typename P1, typename P2, typename P3 >
	static LuaMethodBind Get( LUA_CALL_RETURN (CLASS::*)(P1, P2, P3), void* pvFunc )  
	{  
		return LuaMethodBind::Make( &Bind<P1, P2, P3>,  pvFunc );  
	}

	//
	// -------------------------------------------- 
	//
	template < typename P1, typename P2, typename P3, typename P4 >
	static int Bind( void* pvObj, void* pvFunc, LuaState& rState )
	{
		LUA_CALL_CHECK_ARG_COUNT(4);
		LUA_CALL_CHECK_ARG(1,4); LUA_CALL_CHECK_ARG(2,4); LUA_CALL_CHECK_ARG(3,4); LUA_CALL_CHECK_ARG(4,4);
		typedef LUA_CALL_RETURN (CLASS::*FuncPtr)( P1, P2, P3, P4 );
		LUA_CALL_FUNC(( (((CLASS*) pvObj)->*(*(FuncPtr*) pvFunc))( LuaValue::Get<P1>( rState, -4 ),
																	LuaValue::Get<P2>( rState, -3 ),
																	LuaValue::Get<P3>( rState, -2 ),
																	LuaValue::Get<P4>( rState, -1 ) ) ));
	}

	template < typename P1, typename P2, typename P3, typename P4 >
	static LuaMethodBind Get( LUA_CALL_RETURN (CLASS::*)(P1, P2, P3, P4), void* pvFunc )  
	{  
		return LuaMethodBind::Make( &Bind<P1, P2, P3, P4>,  pvFunc );  
	}

	//
	// -------------------------------------------- 
	//
	template < typename P1, typename P2, typename P3, typename P4, typename P5 >
	static int Bind( void* pvObj, void* pvFunc, LuaState& rState )
	{
		LUA_CALL_CHECK_ARG_COUNT(5);
		LUA_CALL_CHECK_ARG(1,5); LUA_CALL_CHECK_ARG(2,5); LUA_CALL_CHECK_ARG(3,5); LUA_CALL_CHECK_ARG(4,5); LUA_CALL_CHECK_ARG(5,5);
		typedef LUA_CALL_RETURN (CLASS::*FuncPtr)( P1, P2, P3, P4, P5 );
		LUA_CALL_FUNC(( (((CLASS*) pvObj)->*(*(FuncPtr*) pvFunc))( LuaValue::Get<P1>( rState, -5 ),
																	LuaValue::Get<P2>( rState, -4 ),
																	LuaValue::Get<P3>( rState, -3 ),
																	LuaValue::Get<P4>( rState, -2 ),
																	LuaValue::Get<P5>( rState, -1 ) ) ));
	}

	template < typename P1, typename P2, typename P3, typename P4, typename P5 >
	static LuaMethodBind Get( LUA_CALL_RETURN (CLASS::*)(P1, P2, P3, P4, P5), void* pvFunc )  
	{  
		return LuaMethodBind::Make( &Bind<P1, P2, P3, P4, P5>, pvFunc );
	}

	//
	// -------------------------------------------- 
	//
	template < typename P1, typename P2, typename P3, typename P4, typename P5, typename P6 >
	static int Bind( void* pvObj, void* pvFunc, LuaState& rState )
	{
		LUA_CALL_CHECK_ARG_COUNT(6);
		LUA_CALL_CHECK_ARG(1,6); LUA_CALL_CHECK_ARG(2,6); LUA_CALL_CHECK_ARG(3,6); LUA_CALL_CHECK_ARG(4,6); LUA_CALL_CHECK_ARG(5,6); LUA_CALL_CHECK_ARG(6,6);
		typedef LUA_CALL_RETURN (CLASS::*FuncPtr)( P1, P2, P3, P4, P5, P6 );
		LUA_CALL_FUNC(( (((CLASS*) pvObj)->*(*(FuncPtr*) pvFunc))( LuaValue::Get<P1>( rState, -6 ),
																	LuaValue::Get<P2>( rState, -5 ),
																	LuaValue::Get<P3>( rState, -4 ),
																	LuaValue::Get<P4>( rState, -3 ),
																	LuaValue::Get<P5>( rState, -2 ),
																	LuaValue::Get<P6>( rState, -1 ) ) ));
	}

	template < typename P1, typename P2, typename P3, typename P4, typename P5, typename P6 >
	static LuaMethodBind Get( LUA_CALL_RETURN (CLASS::*)(P1, P2, P3, P4, P5, P6), void* pvFunc )  
	{  
		return LuaMethodBind::Make( &Bind<P1, P2, P3, P4, P5, P6>, pvFunc );
	}

};

LUA_CALL_TEMPLATE
struct LuaStaticCall LUA_CALL_SPECIALISED
{
	//
	// -------------------------------------------- 
	//
	static int Bind( void* pvFunc, LuaState& rState )
	{
		typedef LUA_CALL_RETURN (*FuncPtr)( );
		LUA_CALL_FUNC( (*(FuncPtr*)&pvFunc)() );
	}

	static LuaStaticMethodBind Get( LUA_CALL_RETURN (*pFunc)() ) 
	{ 
		return LuaStaticMethodBind::Make( &Bind, &pFunc ); 
	}
};

#undef LUA_CALL_FUNC
#undef LUA_CALL_TEMPLATE	
#undef LUA_CALL_RETURN		
#undef LUA_CALL_SPECIALISED	
