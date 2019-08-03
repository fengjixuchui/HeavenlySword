#ifndef _SIMPLEFUNCTION_HPP_
#define _SIMPLEFUNCTION_HPP_


//--------------------------------------------------
//!
//!	\file simplefunction.h
//!	Simple math function, because I don't like to rewrite them everytime
//!
//--------------------------------------------------

//#include "core/explicittemplate.h"

#ifdef SN_TARGET_PS3_SPU
#include <util_spu.h>	
#endif // SN_TARGET_PS3_SPU


namespace SimpleFunction
{
	// the most stupid function ever
	inline float Id(float t)
	{
		return t;
	}
	
	// return the sign of t
	inline float Sign(float t)
	{
		return (t<0.0f)?-1.0f:1.0f;
	}
	
	// smooth t value whitin [0-1] (wyvill function)
	inline float Fade(float t)
	{
		return t * t * t * (t * (t * 6 - 15) + 10);
	}
	
	// smooth t value whitin [0-1] (faster than wyvill function)
	inline float FastFade(float t)
	{
		return t * t * (3.0f - 2.0f * t);
	}
	
	// clamp between 0 and 1
	inline float Saturate(float t)
	{
		return ntstd::Clamp(t,0.0f,1.0f);
	}
	
	// 0 if t==a, 1 if t==b
	inline float LinearStep(float a, float b, float t)
	{
		return Saturate((t-a)/(b-a));
	}
	
	// linear interpolation
	template<class T>
	inline T Lerp(const T& a, const T& b, float t)
	{
		return a + (b - a)*t;
	}
		
	// linear interpolation witrh filp argument, backward compatibility
	template<class T>
	inline T LerpFlipArg(float t, const T& a, const T& b)
	{
		return Lerp(a,b,t);
	}
	
	// smooth interpolation
	// return a for t==0, b for t==1, smoothed
	inline float SmoothLerp(float a, float b, float t)
	{
		return Lerp(a,b,FastFade(t));
	}

	// return 0 for t==a, 1 for t==b, smoothed (null derivative on a and b)
	inline float SmoothStep(float a, float b, float t)
	{
		return FastFade(LinearStep(a, b,t));
	}
	
	// equal 0 on for t==0 and t==1
	// derivative is also equal to 0 for t==0 and t==1
	inline float DerivativeFade(float t)
	{
		return 30 * t * t * (t * (t - 2) + 1);
	}
	
	// 0 everywhere execpt between a, and b -> 1
	inline float SquarePeek( float a, float b, float t)
	{
		ntAssert(a<b);
		if(t<a)
		{
			return 0.0f;
		}
		else if (t>b)
		{
			return 0.0f;
		}
		else
		{
			return 1.0f;
		}		
	}
	
	// 0 everywhere
	inline float NormalisedSmoothPeek(float a, float b, float t)
	{
		ntAssert((0.0f<=a)&&(a<=b)&&(b<=1.0f));
		t=ntstd::Clamp(t,0.0f,1.0f);
		if(t<a)
		{
			return FastFade(t/a);
		}
		else if (t>b)
		{
			return 1.0f - FastFade((t-b)/(1.0f-b));
		}
		else
		{
			return 1.0f;
		}
	}
}



/*
UNUSED

//--------------------------------------------------
//!
//!	Short Class Description.
//!	Long Class Description
//!	Exciting class with lots of stuff to describe
//!
//--------------------------------------------------
class AffineFunc
{
public:
	Vec2 m_a;
	Vec2 m_b;
	//! constructor
	AffineFunc(): m_a(0.0f,0.0f), m_b(0.0f,0.0f) {}
	AffineFunc(Vec2 a, Vec2 b): m_a(a), m_b(b) {}
	inline Vec2 Get(const Vec2& e) { return m_b+m_a*e;}
	inline AffineFunc CreateInverse() {return AffineFunc(Vec2(1.0f/m_a[0],1.0f/m_a[1]), Vec2(-m_b[0]/m_a[0],-m_b[1]/m_a[1]));} 
}; // end of class AffineFunc

*/


/*
//--------------------------------------------------
//!
//!	give some sense to float between 0 and 10
//!	Let's say we are going to put slider in Maya ranging from 0 to 10
//! Artist wants this slider to make some sense (e.g. have some noticable influence)
//! This is what this function is trying to do for positive value:
//! The function is bijection from [0,10] to [0,fMaximum] where the derivative
//! of the function at x=1 is xideal * n, where n is suppose to be "not be big": 
//!
//--------------------------------------------------


class PertinenceFunc
{
public:
	float m_fIdeal;
	float m_fMaximum;
	
	float m_fPower;
	
	//! constructor
	PertinenceFunc(float fIdeal, float fMaximum):
		m_fIdeal(fIdeal),
		m_fMaximum(fMaximum)
	{
		ntAssert((0.0f < fIdeal) && (fIdeal<fMaximum));
		m_fPower = log10(fIdeal/fMaximum);
	}
	
	float Evaluate(float fX)
	{
		return m_fIdeal * pow(fX,m_fPower);
	}
	
	float InvEvaluate(float fX)
	{
		return pow(fX/m_fIdeal, 1.0f / m_fMaximum);
	}

}; // end of class PertinenceFunc
*/

#endif // end of _SIMPLEFUNCTION_HPP_
