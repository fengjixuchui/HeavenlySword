#ifndef _CONTAINERVECTOR_H_
#define _CONTAINERVECTOR_H_

#include "core/explicittemplate.h"



//--------------------------------------------------
//!
//!	vector container (similar to ntstd::vector, simpler).
//!	The reason for creating this class is that the ntstd::vector class do not support SIMD CVector
//!	By the way, this only working with SIMD because all the allocation are currently aligned  (in the prototype engine)
//!
//! C is the memory chunk we allocate/delete from
//!
//--------------------------------------------------

template<class Elem, Mem::MEMORY_CHUNK C=Mem::MC_MISC>
class ContainerVector: CNonCopyable
{
public:
	
	/// constructor
	ContainerVector():
		m_aData(0),
		m_iSize(0)
	{
		// nothing 
	}
	
	/// constructor
	ContainerVector(int size):
		m_aData(0),
		m_iSize(size)
	{
		m_aData = NT_NEW_ARRAY_CHUNK( C ) Elem[size];
	}
	
	/// destructor
	~ContainerVector()
	{
		Clear();
	}
	
	inline Elem& operator[] (int i)
	{
		ntAssert(IsInRange(i));
		return m_aData[i];
	}
	
	inline const Elem& operator[] (int i) const
	{
		ntAssert(IsInRange(i));
		return m_aData[i];
	}
	
	inline bool IsInRange(int i) const
	{
		return (i>=0) && (i<m_iSize);
	}
	
	inline void Assign(const Elem& e)
	{
		for(int iFor = 0 ; iFor < m_iSize ; iFor++ )
		{
			m_aData[iFor]=e;
		}
	}
	
	inline void Clear()
	{
		if (m_aData!=0)
		{
			NT_DELETE_ARRAY_CHUNK( C, m_aData );
			m_aData=0;
			m_iSize=0;
		}
		else
		{
			ntAssert(m_iSize==0);
		}
	}
	
	inline int GetSize() const
	{
		return m_iSize;
	}
	
	inline const Elem* GetData() const
	{
		return m_aData;
	}
	
	inline Elem* GetData()
	{
		return m_aData;
	}
	
	inline void Reset(int size)
	{
		Clear();
		m_aData = NT_NEW_ARRAY_CHUNK ( C ) Elem[size];
		m_iSize = size;
	}
	
	inline void Swap(ContainerVector<Elem, C>& cont)
	{
		// tmp store
		Elem* aData = this->m_aData;
		int iSize = this->m_iSize;
		
		// assign this
		this->m_aData = cont.GetData();
		this->m_iSize = cont.GetSize();
		
		// assign cont
		cont.m_aData = aData;
		cont.m_iSize = iSize;
	}
	
protected:
	Elem* m_aData;
	int m_iSize;
}; // end of class ContainerVector






















#endif // end of _CONTAINERVECTOR_H_

