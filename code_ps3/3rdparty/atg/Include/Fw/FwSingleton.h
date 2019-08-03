//--------------------------------------------------------------------------------------------------
/**
	@file		FwSingleton.h
	
	@brief		Default Singleton Class

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef	FW_SINGLETON_H
#define	FW_SINGLETON_H

//--------------------------------------------------------------------------------------------------
/**
	@class			FwSingleton
	
	@brief			Assists in the creation of singleton objects. 

	Derive from this class if you want to construct a singleton object. Usage is extremely
	straightforward. 

	@code
		class	MyEntityManager	: public FwSingleton<MyEntityManager>
		{
			<class stuff goes in here>
		};

		// Construct our singleton
		FW_NEW MyEntityManager();

		// Optionally use our singleton - if you always have a singleton present, then there's
		// no need to do this check, as Get() will ensure that you have an object.
		if ( MyEntityManager::Exists() )
		{
			MyEntityManager&	rEntityManager = MyEntityManager::Get();
			rEntityManager.SomeFunction();
		}
		
		// Destroy our singleton
		MyEntityManager::Destroy();

	@endcode

**/
//--------------------------------------------------------------------------------------------------

template <typename T>
class FwSingleton : public FwNonCopyable
{
public:
	FwSingleton( void )
	{
		FW_ASSERT_MSG( !ms_pSingleton, ( "%s: we already have an instance", __FUNCTION__ ) );
		ms_pSingleton = static_cast<T*>(this);
	}
	
	static T& Get( void )
	{
		FW_ASSERT_MSG( ms_pSingleton, ( "%s: we don't have an instance", __FUNCTION__ ) );
		return *ms_pSingleton;
	}

	static bool Exists( void )
	{
		return ms_pSingleton != 0;
	}

	static void Destroy( void )
	{
		if ( ms_pSingleton )
		{
			FW_DELETE( ms_pSingleton );
		}
		ms_pSingleton = 0;
	}

//protected:

	~FwSingleton( void )
	{
		FW_ASSERT_MSG( ms_pSingleton, ( ": we don't have an instance", __FUNCTION__ ) );
		ms_pSingleton = 0;
	}

	static T* ms_pSingleton;
};

template <typename T> T* FwSingleton<T>::ms_pSingleton = 0;

#endif	// FW_SINGLETON_H
