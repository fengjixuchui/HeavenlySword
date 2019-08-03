#ifndef _SPEEDTREEUTIL_H_
#define _SPEEDTREEUTIL_H_


//-----------------------------------------------------------------------------
// Miscellaneous helper functions
//-----------------------------------------------------------------------------
#define SAFE_DELETE(p)       { if(p) { NT_DELETE (p);     (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p) { if(p) { NT_DELETE_ARRAY (p);   (p)=NULL; } }
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }

#define Forest_RenderBranches       (1 << 0)
#define Forest_RenderLeaves         (1 << 1)
#define Forest_RenderFronds         (1 << 2)
#define Forest_RenderBillboards     (1 << 3)
#define Forest_RenderAll            ((1 << 4) - 1)

///////////////////////////////////////////////////////////////////////  
//  Macros

inline void VERIFY(HRESULT hr)
{
	UNUSED(hr);
	ntError_p( SUCCEEDED(hr), ("Error code: %d", hr));
}

class CSpeedTreeRT;

class SpeedTreeStat
{
public:
	u32 m_leafTriangleCount;
	u32 m_BranchTriangleCount;
	u32 m_FrondTriangleCount;
	u32 m_BillboardTriangleCount;
public:
	void operator += (const SpeedTreeStat& stat);
	//SpeedTreeStat(CSpeedTreeRT* pTree);
	SpeedTreeStat(u32 leafTriangleCount,
		u32 m_BranchTriangleCount,
		u32 FrondTriangleCount,
		u32 BillboardTriangleCount);
	SpeedTreeStat();
	u32 GetTotal();
}; // end of class SpeedTreeStat


#endif // end of _SPEEDTREEUTIL_H_
