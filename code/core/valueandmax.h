#ifndef _VALUEANDMAX_H_
#define _VALUEANDMAX_H_

//--------------------------------------------------
//!
//!	a value and its maximumvalue, that's it
//!
//--------------------------------------------------

template<class T>
struct ValueAndMax
{
	T m_value;	
	T m_max;
	ValueAndMax() {}
	ValueAndMax(const T& m): m_value(m), m_max(m) {}
	ValueAndMax(const T& m, const T& v): m_value(v), m_max(m) {}
};

#endif // end of _VALUEANDMAX_H_
