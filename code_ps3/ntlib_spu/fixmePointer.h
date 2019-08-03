#ifndef _FIXMEPOINTER_H_
#define _FIXMEPOINTER_H_

//--------------------------------------------------
//!
//!	\file fixmePointer.h
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

template<class T>
class FixmePointer
{
private:
	T* m_pPointer;
public:
	//! null constructor
	inline FixmePointer():m_pPointer(0){};
	//! set
	inline void Set(T* pPointer) {AssertIsNotValid(); m_pPointer=pPointer;}
	//! get
	inline void Get() { AssertIsValid(); return m_pPointer;}
	inline const T* Get() const { AssertIsValid(); return m_pPointer;}
	//! check
	inline bool IsValid() {return m_pPointer!=0;}
	inline void AssertIsValid() {ntAssert_p(IsValid(), ("Invalid FixmePointer"));}
	inline void AssertIsNotValid() {ntAssert_p(!IsValid(), ("Valid FixmePointer (should be invalid)"));}
}; // end of class FixmePointer

#endif // end of _FIXMEPOINTER_H_
