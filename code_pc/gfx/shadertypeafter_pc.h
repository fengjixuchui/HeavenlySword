#ifndef _SHADERTYPEAFTER_H_
#define _SHADERTYPEAFTE_H_

#undef uniform


/*
template<class T>
inline void CheckStructSize()
{
	ntAssert((sizeof(T) & 15) == 0);
	int iSizeOf = sizeof(T) / 16;
	int iDeclaredSize = T::g_iSize;
	ntAssert(iSizeOf == iDeclaredSize);
	iDeclaredSize;
	iSizeOf;
}

inline int CheckAllStructSize()
{
	CheckStructSize<PerCellData>();
	CheckStructSize<CloudsLayer>();
	CheckStructSize<CloudsGlobal>();
	CheckStructSize<DepthHaze>();
	CheckStructSize<CloudsGPUNoise>();
	return 0;
}

static const int g_iStaticAssertCheckStructSize = CheckAllStructSize();


*/
#endif // end of _SHADERTYPEAFTE_H_