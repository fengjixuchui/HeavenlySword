#include "speedtree/SpeedTreeUtil_ps3.h"

#include <SpeedTreeRT.h>

uint32_t SpeedTreeStat::GetTotal()
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
SpeedTreeStat::SpeedTreeStat(uint32_t leafTriangleCount,
		uint32_t m_BranchTriangleCount,
		uint32_t FrondTriangleCount,
		uint32_t BillboardTriangleCount)
	:m_leafTriangleCount(leafTriangleCount)
	,m_BranchTriangleCount(m_BranchTriangleCount)
	,m_FrondTriangleCount(FrondTriangleCount)
	,m_BillboardTriangleCount(BillboardTriangleCount)
{
}
