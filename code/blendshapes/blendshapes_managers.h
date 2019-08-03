
//--------------------------------------------------
//!
//!	\file blendshapes_managers.h
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------



#ifndef _BLENDSHAPES_LOADERS_H_
#define _BLENDSHAPES_LOADERS_H_

#include "blendshapes/blendshapes_export.h"
#include "blendshapes/anim/blendshapes_anim_export.h"
#include "blendshapes/headermanager.h"

struct BSClumpHeaderLoader
{
	//! converts neutral name into the plaform name. used as our cache key
	static void	MakePlatformName( const char* pNeutralName, char* pPlatformName );

	BSClumpHeader* operator()( void* pFileData, uint32_t fileSize, const char* pDebugTag );
};

struct BSAnimHeaderLoader
{
	//! converts neutral name into the plaform name. used as our cache key
	static void	MakePlatformName( const char* pNeutralName, char* pPlatformName );

	BSAnimExport* operator()( void* pFileData, uint32_t fileSize, const char* pDebugTag );
};

template <class HeaderType>
class BSHeaderPtr
{
	typedef typename HeaderType::HeaderAdaptor AdapterType;
	void BoolDummy() const {}

	typedef void (BSHeaderPtr::*BoolType) () const;

	void Release()
	{
		if (ptr_)
		{
			if (0 == -- ptr_ -> m_iRefCount)
			{
				HeaderType::Get().Unload(ptr_);				
			}
		}
	}

	void AddRef()
	{
		if (ptr_) ++ ptr_ -> m_iRefCount;
	}
public:
	BSHeaderPtr()
		: ptr_(NULL)
	{
	}

	BSHeaderPtr(BSHeaderPtr const& lhs)
		: ptr_(lhs.ptr_)
	{
		AddRef();
	}

	BSHeaderPtr(AdapterType* ptr)
		: ptr_(ptr)
	{
		//++ ptr_ -> m_iRefCount;
	}



	BSHeaderPtr&	operator= (BSHeaderPtr const& lhs)
	{
		Release();

		ptr_= lhs.ptr_;

		AddRef();

		return *this;
	}

	~BSHeaderPtr()
	{
		Release();
	}

	typename AdapterType::HeaderType* operator -> ()
	{
		if (ptr_)
		{
			return ptr_ -> m_pobHeader;
		}
		else
		{
			return NULL;
		}
	}

	typename AdapterType::HeaderType const* operator -> () const
	{
		if (ptr_)
		{
			return ptr_ -> m_pobHeader;
		}
		else
		{
			return NULL;
		}

	}


	bool friend operator == (BSHeaderPtr const& lhs, AdapterType* rhs)
	{
		return lhs.ptr_ == rhs;
	}
	bool friend operator == (AdapterType* lhs, BSHeaderPtr const& rhs)
	{
		return rhs.ptr_ == lhs;
	}

	bool friend operator != (BSHeaderPtr const& lhs, AdapterType* rhs)
	{
		return ! (lhs == rhs);
	}

	operator BoolType () const
	{
		if (NULL == ptr_ || NULL == ptr_ -> m_pobHeader)
		{
			return NULL;
		}
		else
		{
			return &BSHeaderPtr::BoolDummy;
		}
	}


private:
	AdapterType*	ptr_;

};

typedef HeaderManager< BSAnimExport, BSAnimHeaderLoader, HeaderDeleterProcedural<BSAnimExport> > BSAnimManager;
typedef HeaderManager< BSClumpHeader, BSClumpHeaderLoader, HeaderDeleterProcedural<BSClumpHeader> > BSClumpManager;
//typedef BSClumpHeader* BSClumpHeaderPtr_t;
//typedef BSAnimExport* BSAnimHeaderPtr_t;
//typedef BSHeaderPtr< BSClumpManager::HeaderAdaptor > BSClumpHeaderPtr_t;
typedef BSHeaderPtr< BSClumpManager >	BSClumpHeaderPtr_t;
typedef BSHeaderPtr< BSAnimManager >		BSAnimHeaderPtr_t;

#endif // end of _BLENDSHAPES_LOADERS_H_

//eof
