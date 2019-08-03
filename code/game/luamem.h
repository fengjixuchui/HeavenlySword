/***************************************************************************************************
*
*	DESCRIPTION
*
*	NOTES
*
***************************************************************************************************/

#ifndef _LUA_MEM_H
#define _LUA_MEM_H

#include "switches.h" // this is where lua hash defines come from

//#define LUA_DEBUG_MEMORY

/***************************************************************************************************
*
*	CLASS			CLuaMemoryMan
*
*	DESCRIPTION		Object that overides lua memory allocation an gyps about with tags if required
*
***************************************************************************************************/
class CLuaMemoryMan : public Singleton<CLuaMemoryMan>
{
public:
	CLuaMemoryMan( void );
	~CLuaMemoryMan( void );

	static void*	ReallocFunction(void* pvMemData,void* pvMem, size_t iOldSize, size_t iSize,
									const char* pcAllocName, unsigned int uiAllocFlags);
	static void		FreeFunction(void* pvMem, int iOldSize, void* pvMemData);

	void	StartTagging( void )	{ m_bTagMemory = true; }
	void	StopTagging( void )		{ m_bTagMemory = false; }

private:
	bool	m_bTagMemory;

#ifdef LUA_DEBUG_MEMORY
	class CTagNode
	{
	public:
		CTagNode( void* pvMem, const char* pcTag ) :
			m_pvMem( pvMem ),
			m_pcTag( pcTag )
		{
			ntAssert(m_pvMem);
			ntAssert(m_pcTag);
		}

		~CTagNode( void ) { NT_DELETE_ARRAY( m_pcTag ); }

		bool Equals( const void* pvMem ) { return (pvMem == m_pvMem); }

	private:
		void*		m_pvMem;
		const char*	m_pcTag;
	};
	ntstd::List<CTagNode*> m_obTagList;
#endif
};

#endif // _LUA_MEM_H


