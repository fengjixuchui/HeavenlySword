#include "SpeedTreeUtil.h"

#include "speedtreert.h"

u32 SpeedTreeStat::GetTotal()
{
	return m_leafTriangleCount+m_BranchTriangleCount+m_FrondTriangleCount+m_BillboardTriangleCount;
}

void SpeedTreeStat::operator += (const SpeedTreeStat& stat)
{
	m_leafTriangleCount += stat.m_leafTriangleCount;
	m_BranchTriangleCount += stat.m_BranchTriangleCount;
	m_FrondTriangleCount  += stat.m_FrondTriangleCount;
	m_BillboardTriangleCount  += stat.m_BillboardTriangleCount;
}


SpeedTreeStat::SpeedTreeStat()
	:m_leafTriangleCount(0)
	,m_BranchTriangleCount(0)
	,m_FrondTriangleCount(0)
	,m_BillboardTriangleCount(0)
{
}
SpeedTreeStat::SpeedTreeStat(u32 leafTriangleCount,
		u32 m_BranchTriangleCount,
		u32 FrondTriangleCount,
		u32 BillboardTriangleCount)
	:m_leafTriangleCount(leafTriangleCount)
	,m_BranchTriangleCount(m_BranchTriangleCount)
	,m_FrondTriangleCount(FrondTriangleCount)
	,m_BillboardTriangleCount(BillboardTriangleCount)
{
}
