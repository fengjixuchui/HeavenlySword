#include "hairstring.h"


#include "hair/chaincore.h"


float ClothSpringInstance::DistanceBetweenPose(ChainElem* elem1,ChainElem* elem2)
{
	CPoint p1 = elem1->GetMayaDef()->GetPoseMatrix().GetTranslation();
	CPoint p2 = elem2->GetMayaDef()->GetPoseMatrix().GetTranslation();
	return (p1-p2).Length();
}

void ClothSpringInstance::ComputePrev()
{
	m_length[0] = DistanceBetweenPose(m_pElem,GetPrev()->m_pElem);
	GetPrev()->m_length[1] = m_length[0];
}
 
void ClothSpringInstance::ComputeNext()
{
	m_length[1] = DistanceBetweenPose(m_pElem,GetNext()->m_pElem);
	GetNext()->m_length[0] = m_length[1];
}






ClothSpringSetInstance::ClothSpringSetInstance(const ClothSpringSetDef* pDef)
	:m_pDef(pDef)
{
	// I should use a circle list here, but I just added the end at the
	// begin end the begin at the end
	m_container = Container(pDef->m_container.size()+2);
}



