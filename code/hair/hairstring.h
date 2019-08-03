#ifndef _HAIRSTRING_H_
#define _HAIRSTRING_H_

#include "chaindef.h"

class ChainElem;
class ClothSpringInstance
{
public:
	ChainElem* m_pElem;
	Vec2 m_length;
	Vec2 m_debugCurrentLength;
	bool HasPrev()
	{
		ClothSpringInstance& csi = this[-1];
		return csi.IsValid();
	}
	bool HasNext()
	{
		ClothSpringInstance& csi = this[1];
		return csi.IsValid();
	}
	ClothSpringInstance* GetPrev()
	{
		ntAssert(HasPrev());
		return &this[-1];
	}
	ClothSpringInstance* GetNext()
	{
		ntAssert(HasNext());
		return &this[1];
	}
	float GetPrevLength()
	{
		ntAssert(HasPrev());
		return m_length[0];
	}
	float GetNextLength()
	{
		ntAssert(HasNext());
		return m_length[1];
	}
	bool IsValid() {return m_pElem!=0;}
	void ComputePrev();
	void ComputeNext();
	static float DistanceBetweenPose(ChainElem* elem1,ChainElem* elem2);
	//! constructor
	ClothSpringInstance():m_pElem(0), m_length(0,0), m_debugCurrentLength(0,0) {};
	//! constructor
	ClothSpringInstance(ChainElem* pElem, Vec2 length):m_pElem(pElem),m_length(length), m_debugCurrentLength(0,0) {};
}; // end of class ClothSpringInstance


class ClothSpringSetInstance
{
public:
	typedef ntstd::Vector<ClothSpringInstance, Mem::MC_GFX> Container;
	Container m_container;
	const ClothSpringSetDef* m_pDef;
public:
	int GetSize() {return m_container.size();}
	ClothSpringSetInstance(const ClothSpringSetDef* pDef);
}; // end of class ClothSpringSetDef



#endif // end of _HAIRSTRING_H_
