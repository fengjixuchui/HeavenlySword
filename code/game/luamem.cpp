/***************************************************************************************************
*
*	DESCRIPTION
*
*	NOTES
*
***************************************************************************************************/

#include "game/luamem.h"
#include "game/luaglobal.h"

/***************************************************************************************************
*
*	FUNCTION		CLuaMemoryMan::Constructor
*
*	DESCRIPTION		Sets up the overidden memory alloc stuff
*
***************************************************************************************************/
CLuaMemoryMan::CLuaMemoryMan( void )
{
	m_bTagMemory = false;
	// lua_setdefaultmemoryfunctions( ReallocFunction, FreeFunction, NULL );
//	lua_setdefaultallocfunction( ReallocFunction, 0 );
}

/***************************************************************************************************
*
*	FUNCTION		CLuaMemoryMan::Constructor
*
*	DESCRIPTION		Sets up the overidden memory alloc stuff
*
***************************************************************************************************/
CLuaMemoryMan::~CLuaMemoryMan( void )
{
#ifdef LUA_DEBUG_MEMORY
	while (!m_obTagList.empty())
	{
		NT_DELETE( m_obTagList.back() );
		m_obTagList.pop_back();
	}
#endif
}

/***************************************************************************************************
*
*	FUNCTION		CLuaMemoryMan::ReallocFunction
*
*	DESCRIPTION		our overidden memory allocation function
*
***************************************************************************************************/
void*	CLuaMemoryMan::ReallocFunction(	void* pvMemData, void* pvMem, size_t iOldSize, size_t iSize, 
										const char* pcAllocName, unsigned int uiAllocFlags)
{
	UNUSED(pvMemData);
	UNUSED(pcAllocName);
	UNUSED(uiAllocFlags);

	void* pvResult = NULL;
	
	if (iSize != 0)
	{
#ifdef LUA_DEBUG_MEMORY
		if (CLuaMemoryMan::Get().m_bTagMemory)
		{
			char* pcTag = tagged_new( "CLuaMemoryMan::ReallocFunction::Tag", 0 ) char [128];

			strcpy( pcTag, "CLuaMemoryMan::LuaAllocation::" );
			if (pcAllocName)
				strcat( pcTag, pcAllocName );
			else
				strcat( pcTag, "UNKNOWN" );

			pvResult = tagged_new( pcTag, 0 ) uint8_t [iSize];

			CTagNode* pobNewTagNode = tagged_new( "CLuaMemoryMan::ReallocFunction::CTagNode", 0 ) CTagNode(pvResult,pcTag);

			CLuaMemoryMan::Get().m_obTagList.push_front(pobNewTagNode);
		}
		else
#endif
		{
			pvResult = NT_NEW uint8_t [iSize];
		}

		ntAssert(pvResult);

		if (pvMem != NULL)
		{
			if (iOldSize <= iSize)
				NT_MEMCPY( pvResult, pvMem, iOldSize );
			else
				NT_MEMCPY( pvResult, pvMem, iSize );

			FreeFunction( pvMem, iOldSize, pvMemData );
		}
	}
	else if (pvMem)
	{
		FreeFunction( pvMem, iOldSize, pvMemData );
	}

	return pvResult;
}

/***************************************************************************************************
*
*	FUNCTION		CLuaMemoryMan::FreeFunction
*
*	DESCRIPTION		our overidden memory de-allocation function
*
***************************************************************************************************/
void	CLuaMemoryMan::FreeFunction(void* pvMem, int iOldSize, void* pvMemData)
{
	UNUSED(iOldSize);
	UNUSED(pvMemData);

#ifdef LUA_DEBUG_MEMORY
	for(	ntstd::List<CTagNode*>::iterator obIt = CLuaMemoryMan::Get().m_obTagList.begin();
			obIt != CLuaMemoryMan::Get().m_obTagList.end(); ++obIt )
	{
		if ((*obIt)->Equals(pvMem))
		{
			NT_DELETE( *obIt );
			obIt = CLuaMemoryMan::Get().m_obTagList.erase( obIt );
			break;
		}
	}
#endif

	NT_DELETE_ARRAY( (uint8_t*) pvMem );
}



