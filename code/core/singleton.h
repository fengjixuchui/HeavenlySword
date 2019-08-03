/***************************************************************************************************
*
*	$Header:: /game/singleton.h 3     7/07/03 10:25 Simonb                                         $
*
*	General purpose singleton classes.
*
*	CHANGES
*
*	08/10/2002	Wil	Created
*
***************************************************************************************************/

#ifndef _SINGLETON_H
#define _SINGLETON_H

/***************************************************************************************************
*
*	CLASS			Singleton
*
*	DESCRIPTION		General singleton class
*
***************************************************************************************************/

template <typename T, Mem::MEMORY_CHUNK	Chunk = Mem::MC_MISC> class Singleton
{
public:
	Singleton( void )
	{
		ntError_p(!m_pobSingleton, ( __FUNCTION__": we already have an instance" ) );
		m_pobSingleton = static_cast<T*>(this);
	}

	~Singleton( void )
	{
		ntError_p(m_pobSingleton, ( __FUNCTION__": we don't have an instance" ) );
		m_pobSingleton = 0;
	}

	static T& Get( void )
	{
		ntError_p(m_pobSingleton, ( __FUNCTION__": we don't have an instance" ) );
		return *m_pobSingleton;
	}

	static T* GetP( void )
	{
		return m_pobSingleton;
	}

	static bool Exists( void )
	{
		return m_pobSingleton != 0;
	}

	static void Kill( void )
	{
		if (m_pobSingleton)
		{
			NT_DELETE_CHUNK( Chunk, m_pobSingleton );
		}
		m_pobSingleton = 0;
	}

protected:
	static T* m_pobSingleton;
};

template <typename T, Mem::MEMORY_CHUNK	Chunk> T* Singleton<T, Chunk>::m_pobSingleton = 0;

#endif // _SINGLETON_H

