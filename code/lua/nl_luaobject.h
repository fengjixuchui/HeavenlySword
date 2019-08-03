#ifndef _LuaObject_H
#define _LuaObject_H


namespace NinjaLua
{

//-------------------------------------------------------------------------------------------------
// CLASS:		
// DESCRIPTION:	
//-------------------------------------------------------------------------------------------------

class LuaObject
{
public:
	// -----------------------------------------------------------------------------------
	// constructor

	LuaObject();
	LuaObject( LuaState& );
	LuaObject( const LuaObject& obOther );
	LuaObject( int iIndex, LuaState& rState, bool bCopy = true );
	LuaObject( LuaState& rState, bool Boolean );
	LuaObject( LuaState& rState, int Number );
	LuaObject( LuaState& rState, const char* pcString );
	LuaObject( LuaState& rState, lua_Number Number );
	LuaObject( LuaState& rState, float Number );
	LuaObject( LuaState& state, int (*fnPtr)(LuaState&));
	LuaObject( LuaState& rState, const CHashedString& str );


	// -----------------------------------------------------------------------------------
	// destructor
	~LuaObject();
	
	// -----------------------------------------------------------------------------------
	// operator overloads
	void operator=( const LuaObject& obOther );
	bool operator==( const LuaObject& obOther );
	inline LuaObject operator[]( const char* pcName ) const { return Get<LuaObject>( pcName ); }
	inline LuaObject operator[]( int Index ) const { return Get<LuaObject>( Index ); }
	inline LuaObject operator[](const CHashedString& keystring) const { return Get<LuaObject>(keystring); }

	// -----------------------------------------------------------------------------------
	// General helpers

	static LuaObject CreateTable(LuaState& State); // Return a new table object
	static const LuaObject CreateFromStack( int iIndex, const LuaState& State );
	inline void Push(LuaState* toState = 0) const;
	inline void Pop(LuaState& State);
	inline int GetType() const;
	inline const char* GetTypeName() const;
	inline const char* ToString() const;
	
	LuaState*	GetState() const { return m_State; } // Return the associated Lua state

	void            SetMetaTable(LuaObject& tbl);
	LuaObject       GetMetaTable();
	const LuaObject GetMetaTable() const;
	bool			CheckMetaTable(CHashedString pcType);

	// Move the lua object to another state
	void XMove( LuaState& );

	// -----------------------------------------------------------------------------------
	// 
	bool IsNone() const;
	bool IsNil() const; 
	bool IsBoolean() const; 
	bool IsLightUserData() const; 
	bool IsNumber() const; 
	bool IsString()	const; 
	bool IsTable() const; 
	bool IsFunction() const; 
	bool IsCFunction() const;
	bool IsUserData() const; 
	bool IsThread()	const; 

	bool IsInteger() const { return IsNumber(); }

	// -----------------------------------------------------------------------------------
	// Table functions
	int GetSize() const { ntAssert(IsTable()); Push(); LuaAutoPop Pop(*m_State, 1); return luaL_getn(*m_State, -1); }


	// -----------------------------------------------------------------------------------
	// LuaPlus compatibility 
	void AssignNil(LuaState&) { SetNil(); }
	void AssignBoolean(LuaState& state, bool value) { *this = LuaObject( state, value ); }
	void AssignInteger(LuaState& state, int value) { *this = LuaObject( state, value ); }
	void AssignNumber(LuaState& state, lua_Number value) { *this = LuaObject( state, value ); }
	void AssignString(LuaState& state, const char* value) { *this = LuaObject( state, value ); }
	void AssignHashedString(LuaState& state, const CHashedString& value) { *this = LuaObject( state, value ); }
//	void AssignLightUserData(LuaState& state, void* value) { this = LuaObject( state, value ); }
	void AssignNewTable(LuaState& state) { *this = LuaObject::CreateTable(state); }

	template<typename T> void Assign(LuaState& state, T value) {LuaValue::Push<T>(state, value); *this = LuaObject(-1, state, true);}


	// -----------------------------------------------------------------------------------
	// 
	bool 			GetBoolean() const;
	void*			GetLightUserData() const; 
	lua_Number		GetNumber() const; 
	const char*		GetString()	const; 
	void* 			GetUserData() const; 
	CHashedString	GetHashedString() const;

	
	template<class TYPE> bool Is() const;
	template<class TYPE> TYPE GetUserData() const;

	
	float			GetFloat() const { return (float) GetNumber(); }
	int				GetInteger() const { return (int) GetNumber(); }

	// -----------------------------------------------------------------------------------
	// Table Accessors
	template<class TYPE, class TARG> inline void Set(const TARG& key, const TYPE& value) const;
	template<class TYPE, class TARG> inline void Get(const TARG& key, TYPE& value) const;
	template<class TYPE, class TARG> inline TYPE Get(const TARG& key) const;
	template<class TYPE, class TARG> inline TYPE GetOpt(const TARG& key, const TYPE& value) const;

	// -----------------------------------------------------------------------------------
	// Register methods.
	void RegisterRaw( const char* pcName, int (*FuncPtr)(LuaState* pLua));
	template< typename RET > void Register( const char* pcName, RET (*FuncPtr)() );
	template< typename RET, typename P1 > void Register( const char* pcName, RET (*FuncPtr)(P1) );
	template< typename RET, typename P1, typename P2 >  void Register( const char* pcName, RET (*FuncPtr)(P1, P2) );
	template< typename RET, typename P1, typename P2, typename P3 > void Register( const char* pcName, RET (*FuncPtr)(P1, P2, P3) );
	template< typename RET, typename P1, typename P2, typename P3, typename P4 >  void Register( const char* pcName, RET (*FuncPtr)(P1, P2, P3, P4) );
	template< typename RET, typename P1, typename P2, typename P3, typename P4, typename P5 > void Register( const char* pcName, RET (*FuncPtr)(P1, P2, P3, P4, P5) );
	template< typename RET, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6 > void Register( const char* pcName, RET (*FuncPtr)(P1, P2, P3, P4, P5, P6) );

	// -----------------------------------------------------------------------------------
	// 
	void			SetNil();

	// -----------------------------------------------------------------------------------
	// 
	LuaObject		Clone();

	// -----------------------------------------------------------------------------------
	// Return the number of elements in a hash+index table
	int				GetEntryCount();


	// -----------------------------------------------------------------------------------
	// Only us in debug code. 
#ifdef _DEBUG
	int			GetRef() const { return m_iRef; }
#endif

protected:
	LuaState* m_State;
	int       m_iRef;
};


//------------------------------------------------------------------------------------------
//!
//!	LuaObject::Push
//!	
//!
//------------------------------------------------------------------------------------------

void LuaObject::Push(LuaState* toState) const
{
	if( m_State )
		lua_rawgeti(*m_State, LUA_REGISTRYINDEX, m_iRef);

	// If required, move the value to another lua state
	if( toState && (toState != m_State) )
		lua_xmove( *m_State, *toState, 1 );
}

//------------------------------------------------------------------------------------------
//!
//!	LuaObject::Pop
//!	
//!
//------------------------------------------------------------------------------------------

void LuaObject::Pop(LuaState& State)
{
	SetNil();
	m_iRef	= luaL_ref(State, LUA_REGISTRYINDEX);
	m_State	= &State;
}

//------------------------------------------------------------------------------------------
//!
//!	LuaObject::GetType
//!	
//!
//------------------------------------------------------------------------------------------

int LuaObject::GetType() const
{
	if( !m_State ) return LUA_TNIL;
	Push();
	int iType = lua_type( *m_State, -1 );		   
	lua_pop(*m_State, 1);
	return iType;
}

//------------------------------------------------------------------------------------------
//!
//!	LuaObject::GetTypeName
//!	
//!
//------------------------------------------------------------------------------------------

const char* LuaObject::GetTypeName() const
{
	if(!m_State) return "NIL";
	return lua_typename( *m_State, GetType() );
}

//------------------------------------------------------------------------------------------
//!
//!	LuaObject::GetTypeName
//!	
//!
//------------------------------------------------------------------------------------------

const char* LuaObject::ToString() const
{
	if(!m_State) return "NIL";
	Push();
	const char* pcString = lua_tostring( *m_State, -1 );
	lua_pop(*m_State, 1);
	return pcString;
}


//------------------------------------------------------------------------------------------
//!
//!	LuaObject::Set
//!	
//!
//------------------------------------------------------------------------------------------

template<class TYPE, class TARG>
void LuaObject::Set(const TARG& key, const TYPE& value) const
{
	Push();                                  // Push this on to the stack
	LuaValue::Push(*m_State, key);           // Push the key onto the stack
	LuaValue::Push(*m_State, value);	     // Push the value on to the stack
	lua_settable( *m_State, -3 );            // Set the value in the table
	lua_pop( *m_State, 1 );                  // pop this object off the stack
}


//------------------------------------------------------------------------------------------
//!
//!	LuaObject::Get
//!	
//!
//------------------------------------------------------------------------------------------

template<class TYPE, class TARG>
void LuaObject::Get(const TARG& key, TYPE& value) const
{
	Push();											// Push this on to the stack
	LuaValue::Push(*m_State, key);					// Push the key onto the stack
	lua_gettable(*m_State, -2);						// Get the related value onto the stack
	if(LuaValue::Is<TYPE>(*m_State, -1))
		value = LuaValue::Get<TYPE>(*m_State, -1);	// Load in the value
	lua_pop(*m_State, 2);							// Don't leave junk on the stack
}

//------------------------------------------------------------------------------------------
//!
//!	LuaValue
//!	
//!
//------------------------------------------------------------------------------------------
template<class TYPE, class TARG>
TYPE LuaObject::Get(const TARG& key) const
{
	ntAssert_p( IsTable() || IsUserData(), ("LuaObject::Get Type is %s", GetTypeName() ) );
	LuaAutoPop obPop( *m_State, 2 );
	Push();										    // Push this on to the stack
	if (!LuaValue::Push(*m_State, key))                  // Push the key onto the stack
	{
		//return TYPE();
		lua_pushnil(*m_State);
	}
	lua_gettable(*m_State, -2);                     // Get the related value onto the stack
	ntAssert( LuaValue::Is<TYPE>(*m_State, -1) );	// Check that the type is correct
	return LuaValue::Get<TYPE>(*m_State, -1);	    // Load in the value
}

//------------------------------------------------------------------------------------------
//!
//!	LuaObject::GetOpt
//!	Similar to Get, but returns the second argument if the first isn't valid.
//!
//------------------------------------------------------------------------------------------
template<class TYPE, class TARG>
TYPE LuaObject::GetOpt(const TARG& key, const TYPE& rDefault) const
{
	if(!IsTable()) return rDefault;
	LuaAutoPop obPop( *m_State, 2 );
	Push();										// Push this on to the stack
	LuaValue::Push(*m_State, key);              // Push the key onto the stack
	lua_gettable(*m_State, -2);                 // Get the related value onto the stack
	if( lua_isnil( *m_State, -1 ) ) return rDefault;
	return LuaValue::Is<TYPE>(*m_State, -1) ? LuaValue::Get<TYPE>(*m_State, -1) : rDefault;	
}

//------------------------------------------------------------------------------------------
//!
//!	LuaObject::GetUserData
//!	
//!
//------------------------------------------------------------------------------------------
template<class TYPE> 
bool LuaObject::Is() const
{
	LuaAutoPop obPop(*m_State,1);
	Push();
	return LuaValue::Is<TYPE>( *m_State, -1 );
}

template<class TYPE>
TYPE LuaObject::GetUserData() const
{
	LuaAutoPop obPop(*m_State,1);
	Push();
	return LuaValue::Get<TYPE>( *m_State, -1 );
}


//------------------------------------------------------------------------------------------
//!
//!	LuaObject::Register
//!	
//!
//------------------------------------------------------------------------------------------
template< typename RET >
void LuaObject::Register( const char* pcName, RET (*FuncPtr)() )
{
	if( !m_State ) return;
	Push();
	lua_pushstring( *m_State, pcName );
	lua_pushlightuserdata( *m_State, (void*) FuncPtr );
	lua_pushcclosure( *m_State, &CallType0<RET>::Call, 1);
	lua_settable( *m_State, -3 );
	lua_pop( *m_State, 1 );
}

template< typename RET, typename P1 >
void LuaObject::Register( const char* pcName, RET (*FuncPtr)(P1) )
{
	if( !m_State ) return;
	Push();
	lua_pushstring( *m_State, pcName );
	lua_pushlightuserdata( *m_State, (void*) FuncPtr );
	lua_pushcclosure( *m_State, &CallType1<RET,P1>::Call, 1);
	lua_settable( *m_State, -3 );
	lua_pop( *m_State, 1 );
}

template< typename RET, typename P1, typename P2 >
void LuaObject::Register( const char* pcName, RET (*FuncPtr)(P1, P2) )
{
	if( !m_State ) return;
	Push();
	lua_pushstring( *m_State, pcName );
	lua_pushlightuserdata( *m_State, (void*) FuncPtr );
	lua_pushcclosure( *m_State, &CallType2<RET,P1,P2>::Call, 1);
	lua_settable( *m_State, -3 );
	lua_pop( *m_State, 1 );
}

template< typename RET, typename P1, typename P2, typename P3 >
void LuaObject::Register( const char* pcName, RET (*FuncPtr)(P1, P2, P3) )
{
	if( !m_State ) return;
	Push();
	lua_pushstring( *m_State, pcName );
	lua_pushlightuserdata( *m_State, (void*) FuncPtr );
	lua_pushcclosure( *m_State, &CallType3<RET,P1,P2,P3>::Call, 1);
	lua_settable( *m_State, -3 );
	lua_pop( *m_State, 1 );
}

template< typename RET, typename P1, typename P2, typename P3, typename P4 >
void LuaObject::Register( const char* pcName, RET (*FuncPtr)(P1, P2, P3, P4) )
{
	if( !m_State ) return;
	Push();
	lua_pushstring( *m_State, pcName );
	lua_pushlightuserdata( *m_State, (void*) FuncPtr );
	lua_pushcclosure( *m_State, &CallType4<RET,P1,P2,P3,P4>::Call, 1);
	lua_settable( *m_State, -3 );
	lua_pop( *m_State, 1 );
}

template< typename RET, typename P1, typename P2, typename P3, typename P4, typename P5 >
void LuaObject::Register( const char* pcName, RET (*FuncPtr)(P1, P2, P3, P4, P5) )
{
	if( !m_State ) return;
	Push();
	lua_pushstring( *m_State, pcName );
	lua_pushlightuserdata( *m_State, (void*) FuncPtr );
	lua_pushcclosure( *m_State, &CallType5<RET,P1,P2,P3,P4,P5>::Call, 1);
	lua_settable( *m_State, -3 );
	lua_pop( *m_State, 1 );
}

template< typename RET, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6 >
void LuaObject::Register( const char* pcName, RET (*FuncPtr)(P1, P2, P3, P4, P5, P6) )
{
	if( !m_State ) return;
	Push();
	lua_pushstring( *m_State, pcName );
	lua_pushlightuserdata( *m_State, (void*) FuncPtr );
	lua_pushcclosure( *m_State, &CallType6<RET,P1,P2,P3,P4,P5,P6>::Call, 1);
	lua_settable( *m_State, -3 );
	lua_pop( *m_State, 1 );
}

//------------------------------------------------------------------------------------------
//!
//!	LuaIterator
//!	
//!
//------------------------------------------------------------------------------------------
class LuaIterator
{
public:

	// Has the iterator finshed?
	operator bool() const { return !m_Finished; }

	// Move on to the next element in the array
	inline bool operator++();

	// Constructor for the itertor
	inline LuaIterator( const LuaObject& obTable );

	// Return the elements
	const LuaObject& GetKey() const { return m_Key; }
	const LuaObject& GetValue() const { return m_Value; }
	
private:
	bool	 m_Finished;

	LuaObject	m_Table;
	LuaObject	m_Key;
	LuaObject	m_Value;

};

//------------------------------------------------------------------------------------------
//!
//!	LuaIterator
//!	
//!
//------------------------------------------------------------------------------------------
bool LuaIterator::operator++()
{
	// If the 
	if( m_Finished ) 
		return false;

	m_Finished = true;

	// Push the table on the stack
	m_Table.Push();

	// Add the key onto the stack
	m_Key.Push();
	
	// Get the next element from the array
	if( lua_next( *m_Table.GetState(), -2 ) )
	{
		m_Value.Pop(*m_Table.GetState());
		m_Key.Pop(*m_Table.GetState());
		m_Finished = false;
	}

	// Take the table off the stack again
	lua_pop( *m_Table.GetState(), 1);

	// return the 
	return m_Finished;
}

//------------------------------------------------------------------------------------------
//!
//!	LuaValue
//!	
//!
//------------------------------------------------------------------------------------------
LuaIterator::LuaIterator( const LuaObject& obTable ) :
	m_Finished(true)
{
	// If table isn't a really table, return now. 
	if( !obTable.IsTable() ) return;
	
	m_Table = obTable;
	m_Key	= LuaObject( *obTable.GetState() );
	m_Finished = false;

	m_Finished = operator++();
}


//------------------------------------------------------------------------------------------
//!
//!	LuaValue
//!	
//!
//------------------------------------------------------------------------------------------
namespace LuaValue
{
	template<>static bool		Is<LuaObject>(LuaState&, int)							{return true;}
	template<>static LuaObject	Get<LuaObject>(LuaState& rState, int iIndex)			{return LuaObject(iIndex, rState);}
	template<>static LuaObject	Pop<LuaObject>(LuaState& rState, int iIndex)			{return LuaObject(iIndex, rState, false );}
	template<>static LuaObject	Default<LuaObject>(LuaState& rState)					{return LuaObject(rState);}
	template<>static int		Push<LuaObject>(LuaState& rState, LuaObject Value)		
	{
		// No State - just return
		if( !Value.GetState() )  
			return 0; 

		Value.Push(); 

		if(Value.GetState() != &rState) 
			lua_xmove(*Value.GetState(),rState,1); 

		return 1;
	}

	template<>static int Push<const LuaObject&>(LuaState& rState, const LuaObject& Value)		
	{
		// No State - just return
		if( !Value.GetState() )  
			return 0; 

		Value.Push(); 

		if(Value.GetState() != &rState) 
			lua_xmove(*Value.GetState(),rState,1); 

		return 1;
	}
};

};

#endif // _LuaObject_H
