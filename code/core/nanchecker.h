#ifndef NANCHECKER_H
#define NANCHECKER_H

//! a NAN-checks toolkit
//! can bve used on both PPU and SPU except for the SPU matrices will need a different CheckNaN function
//! as unlike the PPU ones they don't define a subscript operator returning a vector

//! NOTE: it uses IsNan function which I placed in the vecmath_ps3.h.
//! Attention: at the moment IsNan is NaN will also consider the values greater than 5000000 as NaNs. 
//! Always check if that's the behavior you want

//! give this macro a name of a variable that needs to be checked (any type of vector, PPU matrix or a float)
#define CHECK_FOR_NAN(obj) CheckNAN(obj, #obj)

//! a simple smart pointer class to perform any checks on an object every time it's being dereferenced
//! parametrized on the object type and a checker function
template <class T, void (*Checker)(T*)>
class CheckedPtr
{
public:
	CheckedPtr()
		: ptr_(NULL)
	{
	}

	CheckedPtr(T* ptr)
		: ptr_(ptr)
	{
	}

	CheckedPtr&	operator = (T* ptr)
	{
		ptr_ = ptr;

		return *this;
	}

	T* operator ->() const
	{
		Checker(ptr_);
		return ptr_;
	}

	operator T*()  const
	{
		return ptr_;
	}

private:
	T*	ptr_;
};

//! Helper functions to check for NANs
// this will work for both vectors and matrices (on PPU)
template <typename Type>
inline void CheckNAN(Type const& obj, const char* name)
{
	for (int i = 0; i < 4; ++ i)
	{
		CheckNAN(obj[i], name);
	}
}

// a specialization for floats where all CheckNANs will eventually end up with
inline void CheckNAN(float f, const char* name)
{
	if (IsNan(f))
	{
		ntError_p(0, ("NAN!!!  in %s\n", name));
	}
}


#endif
