#ifndef _RELATIVEPOINTER_H_
#define _RELATIVEPOINTER_H_

//--------------------------------------------------
//!
//!	\file relativePointer.h
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#ifdef PLATFORM_PS3
#include <stdint.h>
#endif // PLATFORM_PS3

//--------------------------------------------------
//!
//!	a pointer using an offset comparing to itself (no reference needed)
//!
//--------------------------------------------------
template<class T>
class RelativePointer
{
private:
	// using 32 bit pointer
	typedef int32_t PointerType;
	// offset relative to itself
	PointerType m_iRelPtr;
protected:
	// make a normal pointer relative
	static void RelativizePointer(T* pAbs, PointerType* pRel)
	{
		intptr_t iTmp = reinterpret_cast<intptr_t>(pAbs) - reinterpret_cast<intptr_t>(pRel);
		(*pRel) = static_cast<PointerType>(iTmp);
	}
	// make an offest absolute
	static T* GetAbsoluteFromRelativePointer(const PointerType& pVal)
	{
		return reinterpret_cast<T*>( static_cast<uintptr_t>(pVal) + reinterpret_cast<uintptr_t>(&pVal) );
	}
public:
	// null ctor, pointing on itfelf !
	RelativePointer()
		:m_iRelPtr(0) // pointing on itfelf !
	{
		// nothing
	}
	// ctor
	RelativePointer(T* pPointer)
	{
		RelativizePointer(pPointer,&m_iRelPtr);
	}
	// ctor
	RelativePointer(const RelativePointer& relPointer)
	{
		RelativizePointer(relPointer.Get(),&m_iRelPtr);
	}
	// ctor
	RelativePointer& operator=(const RelativePointer& relPointer)
	{
		RelativizePointer(relPointer.Get(),&m_iRelPtr);
		return (*this);
	}
	// set new value
	void Set(T* pPointer)
	{
		RelativizePointer(pPointer,&m_iRelPtr);
	}
	// get absolute pointer
	T* Get() const
	{
		return GetAbsoluteFromRelativePointer(m_iRelPtr);
	}
}; // end of class RelativePointer




#endif // end of _RELATIVEPOINTER_H_
