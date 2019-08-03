#ifndef _FRANKTMP_H_
#define _FRANKTMP_H_

//--------------------------------------------------
//!
//!	\file franktmp.h
//!	This file contains various stuff that I (Frank) am waiting
//! to put somewhere else or to supress definetely
//!
//--------------------------------------------------


#define SAFEDELETE(punk)    if (true) { NT_DELETE( punk ); punk = 0; }

#include "core/explicittemplate.h"
#include "game/randmanager.h"

class Transform;
























































namespace FrankMisc
{
	void Split(const ntstd::String& in, ntstd::Vector<ntstd::String>& out, const ntstd::String& sep);

	inline int GetSeedFromPointer(const void* pEntity)
	{
		return static_cast<int>(reinterpret_cast<uintptr_t>(pEntity));
	}
	
	CMatrix CreateScaleMatrix(float fSx, float fSy, float fSz);
	inline CMatrix CreateScaleMatrix(const CDirection& v)
	{
		return CreateScaleMatrix(v.X(),v.Y(),v.Z());
	}
	inline CMatrix CreateInvScaleMatrix(float fSx, float fSy, float fSz)
	{
		return CreateScaleMatrix(1.0f / fSx, 1.0f / fSy, 1.0f / fSz);
	}
	inline CMatrix CreateInvScaleMatrix(const CDirection& v)
	{
		return CreateInvScaleMatrix(v.X(),v.Y(),v.Z());
	}

	inline bool IsInForRange(int iV,Pixel3 p)
	{
		if(p[0]>p[1])
		{
			int iTmp = p[0];
			p[0] = p[1];
			p[1] = iTmp;
		}
		return ((p[0]<=iV) && (iV<=p[1]));
	}

	inline uint32_t GetDWord(const Pixel4& colour)
	{
		return NTCOLOUR_ARGB(colour[0],colour[1],colour[2],colour[3]);
	}
	
	inline bool OnScreen(const float& x, const float& y, float fScreenSize)
	{
		return ntstd::Max(abs(x),abs(y))<fScreenSize;
	}
	inline bool OnScreen(const float& x, const float& y)
	{
		return ntstd::Max(abs(x),abs(y))<1.0f;
	}
	inline bool OnScreen(const Vec2& v, float fScreenSize)
	{
		return OnScreen(v[0],v[1],fScreenSize);
	}
	inline float Norm2D(const CVector& v)
	{
		return sqrt(v.X()*v.X() + v.Y()*v.Y());
	}
	
	CMatrix GetRelativeMatrix(const Transform* pLocal, const Transform* pRelative);

	template<int N>
	inline void FillWithRand(Array<float,N>& array, int iSeed)
	{
		erands(iSeed);
		for(int iIndex = 0 ; iIndex < array.size() ; iIndex++ )
		{
			array[iIndex] = erandf(1.0f);
		}
	}
	
	
	void DrawGrid(Pixel3 size, const CPoint& origin, const CDirection& realSize, uint32_t color);
} // end of namespace frankMisc












namespace Conversion
{
	template<int SIZE>
	inline Array<int,SIZE> Floor(const Array<float,SIZE>& v)
	{
		Array<int,SIZE> res;
		for(int iFor = 0 ; iFor < SIZE ; iFor++ )
		{
			res[iFor] = static_cast<int>(floor(v[iFor]));
		}
		return res;
	}
	template<int SIZE>
	inline Array<int,SIZE> Ceil(const Array<float,SIZE>& v)
	{
		Array<int,SIZE> res;
		for(int iFor = 0 ; iFor < SIZE ; iFor++ )
		{
			res[iFor] = static_cast<int>(ceil(v[iFor]));
		}
		return res;
	}
	
	template<int SIZE>
	inline Array<float,SIZE> Int2Float(const Array<int,SIZE>& v)
	{
		Array<float,SIZE> res;
		for(int iFor = 0 ; iFor < SIZE ; iFor++ )
		{
			res[iFor] = static_cast<float>(v[iFor]);
		}
		return res;
	}
	
	inline CPoint Pixel2CPoint(const Array<int,3>& v)
	{
		return CPoint(static_cast<float>(v[0]),static_cast<float>(v[1]),static_cast<float>(v[2]));
	}
	inline CDirection Pixel2CDirection(const Array<int,3>& v)
	{
		return CDirection(static_cast<float>(v[0]),static_cast<float>(v[1]),static_cast<float>(v[2]));
	}

} // end of namespace VectorConversion








class Lerpable
{
public:
	Lerpable() {};
	virtual ~Lerpable() {};
	
	virtual void operator *= (float fCoef) const = 0;
	virtual void operator += (const Lerpable& obj) = 0;
}; // end of class Lerpable


//namespace Derivation
//{
//	inline Pixel3 GetBaseVector(int iDimension)
//	{
//		switch(iDimension)
//		{
//			case 0: return Pixel3(1,0,0);
//			case 1: return Pixel3(0,1,0);
//			case 2: return Pixel3(0,0,1);
//			default: ntAssert(false); return Pixel3(0,0,0);
//		}
//	}
//
//	template<int iDimension, class Functor, class Grid>
//	inline float GetPartialDerivative(const Pixel3& p, const Grid& grid)
//	{
//		const Pixel3 dec = GetBaseVector(iDimension);
//		
//		if(p[iDimension]>0)
//		{
//			if(p[iDimension]<grid.GetSize()[iDimension]-1)
//			{
//				return Functor::f(p-dec,grid) - 2*Functor::f(p,grid) + Functor::f(p+dec,grid);
//			}
//			else
//			{
//				if(p[iDimension]==(grid.GetSize()[iDimension]-1))
//				{
//					return Functor::f(p-dec,grid) - Functor::f(p,grid);
//				}
//				else
//				{
//					ntAssert(false);
//					return -666.0f;
//				}
//			}
//		}
//		else
//		{
//			if(p[iDimension]==0)
//			{
//				return Functor::f(p,grid)-Functor::f(p+dec,grid);
//			}
//			else
//			{
//				ntAssert(false);
//				return -666.0f;
//			}
//		}
//		
//	}
//		
//	template<class Functor, class Grid>
//	CDirection GetDerivative(const Pixel3& p, const Grid& grid)
//	{
//		return CDirection(
//			GetPartialDerivative<0,Functor,Grid>(p,grid),	
//			GetPartialDerivative<1,Functor,Grid>(p,grid),
//			GetPartialDerivative<2,Functor,Grid>(p,grid));	
//	}
//
//
//} // end of















#endif // end of _FRANKTMP_H_
