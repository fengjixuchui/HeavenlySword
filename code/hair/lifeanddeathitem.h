#ifndef _LIFEANDDEATHITEM_H_
#define _LIFEANDDEATHITEM_H_


#include "lifeanddeath.h"


//--------------------------------------------------
//!
//!	\file lifeanddeathitem.h
//!	some simple sequences
//!
//--------------------------------------------------




template<class T>
class LinearInterpolation: public TimeSequence
{
public:
	const T* m_pBegin;
	const T* m_pEnd;
	
	LinearInterpolation(const T& pBegin, const T& pEnd, float fDuration, BitMask<TimeSequence::State>::Unsigned mask = static_cast<u_char>(0)):
		m_pBegin(&pBegin), m_pEnd(&pEnd), TimeSequence(fDuration,mask)
	{
		// nothing
	}
	
	float GetValue()
	{
		ntAssert(IsValid());
		return SimpleFunction::Lerp(*m_pBegin,*m_pEnd,GetProgress());
	}
	
	bool IsValid()
	{
		return (m_pBegin!=0) && (m_pEnd!=0);
	}
};



class SquareFunction: public TimeSequence
{
public:
	explicit SquareFunction(float fDuration, BitMask<TimeSequence::State>::Unsigned mask = static_cast<u_char>(0)):
		TimeSequence(fDuration,mask)
	{
		// nothing
	}
	
	float GetValue()
	{
		return (abs(GetProgress()-0.5f)<0.49f)?1.0f:0.0f;
	}
};


template<class T>
class GotoSequence: public TimeSequence
{
public:
	T* m_pCurrent;
	T m_end;
	
	virtual void Next()
	{
		(*m_pCurrent) = SimpleFunction::Lerp( (*m_pCurrent), m_end, GetProgress() );
	}
		
	GotoSequence(T* pCurrent, const T& end, float fDuration, BitMask<TimeSequence::State>::Unsigned mask = static_cast<u_char>(0)):
		m_pCurrent(pCurrent),
		m_end(end),
		TimeSequence(fDuration,mask)
	{
		// nothing
	}
};





#endif // end of _LIFEANDDEATHITEM_H_