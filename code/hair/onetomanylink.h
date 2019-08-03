#ifndef _ONETOTLINK_H_
#define _ONETOTLINK_H_

//--------------------------------------------------
//!
//!	\file onetomanylink.h
//!	registration abilities
//!	typically use when you want to share one piece of data amid some instances
//!	basically just a wrap around ntstd::set
//!
//--------------------------------------------------


#include "core/nt_std.h"

template<class T>
class RegisterContainer: public ntstd::Set<T*>
{
public:
	typedef ntstd::Set<T*> List;
public:	
	RegisterContainer()
	{
		// nothing
	}
	
	~RegisterContainer()
	{
		//ntAssert(this->size()==0);
	}
	
	bool Register(T* pNewT)
	{
		return this->insert(pNewT).second;
	}
	bool Check(T* pNewT)
	{
		typename List::iterator it = this->find(pNewT);
		return it != this->end();
	}
	bool Unregister(T* pNewT)
	{
		typename List::iterator it = this->find(pNewT);
		if(it != this->end())
		{
			this->erase(it);
			return true;
		}
		else
		{
			return false;
		}
	}
};




#endif // end of _ONETOTLINK_H_
