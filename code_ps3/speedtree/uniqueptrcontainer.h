#ifndef UNIQUEPTRCONTAINER_H
#define UNIQUEPTRCONTAINER_H

class CUniquePtrContainer
{
	typedef ntstd::Set<void*>					PtrCnt;
	typedef ntstd::pair<PtrCnt::iterator, bool>	InsertResultType;
public:
	template <typename T>
	void	AddPtr(T* ptr)
	{
		InsertResultType	res = cont_.insert((void*)ptr);
		ntAssert(res.second);
	}

	template <typename T>
	const T*		GetPtr(T* ptr)
	{
		PtrCnt::iterator	res = cont_.find((void*)ptr);
		if (res != cont_.end())
		{
			return (const T*)*res;
		}
		else
		{
			return NULL;
		}
	}

private:
	PtrCnt	cont_;
};


#endif
