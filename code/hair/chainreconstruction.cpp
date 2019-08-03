#include "chainreconstruction.h"
#include "chaincore.h"
#include "gfx/simplefunction.h"
#include "anim/hierarchy.h"

using namespace SimpleFunction;





ChainReconstruction::ChainReconstruction()
{
	// nothing
}
ChainReconstruction::~ChainReconstruction()
{
	// nothing
}

















class ChainReconstructionInfoStd: public ChainReconstructionInfo
{
public:
	CDirection m_xAxisApprox;
	ChainReconstructionInfoStd(){}
	virtual ~ChainReconstructionInfoStd(){}
	virtual void UpdateSentinel(ChainElem* pElem)
	{
		m_xAxisApprox =  pElem->m_local2world.GetXAxis();
	}

	virtual void UpdateWorldMatrixFreeze(ChainElem* pElem, const HairStyleFromWelder* pDef)
	{
		UNUSED(pDef);
		// get x approximation
		pElem->m_local = pElem->m_pDef->m_local;
		pElem->m_local2world = pElem->m_pDef->m_local * pElem->m_pParent->m_local2world;
		m_xAxisApprox = (pElem->GetMayaDef()->m_extraRotInv * pElem->m_local2world).GetXAxis();
	}

	virtual void UpdateWorldMatrix(const ChainRessource::RotIndex& p,
		ChainElem* pElem, const HairStyleFromWelder* pDef)
	{
		// get x approximation
		ChainReconstructionInfoStd* pParentRecons =
			pElem->GetParent()->GetReconstructionInfo<ChainReconstructionInfoStd>();
		CDirection exaux = pParentRecons->m_xAxisApprox;
		FRANKHAIRPRINT_FLOAT3("exaux",exaux);

		// get y axis
		CVector root = pElem->m_pParent->m_local2world[3];
		FRANKHAIRPRINT_FLOAT3("root",root);
		CVector extremity = pElem->GetExtremityWithLatency(pDef->m_bUseLatency,pDef->m_fLatency,p);
		CDirection ey = CDirection(extremity - root);
		ey *= 1.0f / ey.Length();

		//CDirection ey = CDirection(m_lastRotationAxis[0]);
		CDirection ez = exaux.Cross(ey);
		ez *= 1.0f / ez.Length();
		m_xAxisApprox = ey.Cross(ez);
		FRANKHAIRPRINT_FLOAT3("before",m_xAxisApprox);
		//m_xAxisApprox *= 1.0f / m_xAxisApprox.Length();
		m_xAxisApprox.Normalise();

		FRANKHAIRPRINT_FLOAT3("ey",ey);
		FRANKHAIRPRINT_FLOAT3("m_xAxisApprox",m_xAxisApprox);

		// set world rotation
		pElem->m_local2world = CMatrix(CONSTRUCT_CLEAR);
		pElem->m_local2world.SetXAxis(m_xAxisApprox);
		pElem->m_local2world.SetYAxis(ey);
		pElem->m_local2world.SetZAxis(ez);

		// add extra rotation to match skin stuff (because of jamexport)
		// rotation in maya is [rotateAxis] * [jointOrient]
		// the transform is just [jointOrient]
		// warning: [rotateAxis1] * [jointOrient1] , [rotateAxis2] * [jointOrient2]
		// joint 1: transform = [jointOrient1]
		// joint 2: transform = [jointOrient2] * [rotateAxis1] * [jointOrient1]
		pElem->m_local2world = pElem->m_pDef->m_extraRot * pElem->m_local2world;

		// set world translation
		//m_local2world.SetTranslation(CPoint(m_extremity.m_position));
		pElem->m_local2world.SetTranslation(CPoint(extremity));

		// compute local		
		pElem->m_local = pElem->m_local2world * pElem->m_pParent->m_local2world.GetAffineInverse();

	}
};












class ChainReconstructionInfoTassle: public ChainReconstructionInfo
{
public:
	CDirection m_xAxisApprox;
	ChainReconstructionInfoTassle(){}
	virtual ~ChainReconstructionInfoTassle(){}
	virtual void UpdateSentinel(ChainElem* pElem)
	{
		m_xAxisApprox =  pElem->m_local2world.GetXAxis();
	}

	virtual void UpdateWorldMatrixFreeze(ChainElem* pElem, const HairStyleFromWelder* pDef)
	{
		UNUSED(pDef);
		// get x approximation
		pElem->m_local = pElem->m_pDef->m_local;
		pElem->m_local2world = pElem->m_pDef->m_local * pElem->m_pParent->m_local2world;
		m_xAxisApprox = (pElem->GetMayaDef()->m_extraRotInv * pElem->m_local2world).GetXAxis();
	}
	
	CDirection GetHeightCoef(const CVector& extremity)
	{
		return CDirection(0.0f,5.0f * Lerp(0.2f, 0.0f, extremity.Y()),0.0f);
	}
	
	virtual void UpdateWorldMatrix(const ChainRessource::RotIndex& p,
		ChainElem* pElem, const HairStyleFromWelder* pDef)
	{
		// get x approximation
		ChainReconstructionInfoStd* pParentRecons =
			pElem->GetParent()->GetReconstructionInfo<ChainReconstructionInfoStd>();
		CVector extremity = pElem->GetExtremityWithLatency(pDef->m_bUseLatency,pDef->m_fLatency,p);
		CDirection exaux = pParentRecons->m_xAxisApprox + GetHeightCoef(extremity);

		// get y axis
		CVector root = pElem->m_pParent->m_local2world[3];
		CDirection ey = CDirection(extremity - root);
		ey *= 1.0f / ey.Length();

		//CDirection ey = CDirection(m_lastRotationAxis[0]);
		CDirection ez = exaux.Cross(ey);
		ez *= 1.0f / ez.Length();
		m_xAxisApprox = ey.Cross(ez);
		m_xAxisApprox *= 1.0f / m_xAxisApprox.Length();

		// set world rotation
		pElem->m_local2world = CMatrix(CONSTRUCT_CLEAR);
		pElem->m_local2world.SetXAxis(m_xAxisApprox);
		pElem->m_local2world.SetYAxis(ey);
		pElem->m_local2world.SetZAxis(ez);

		// add extra rotation to match skin stuff (because of jamexport)
		// rotation in maya is [rotateAxis] * [jointOrient]
		// the transform is just [jointOrient]
		// warning: [rotateAxis1] * [jointOrient1] , [rotateAxis2] * [jointOrient2]
		// joint 1: transform = [jointOrient1]
		// joint 2: transform = [jointOrient2] * [rotateAxis1] * [jointOrient1]
		pElem->m_local2world = pElem->m_pDef->m_extraRot * pElem->m_local2world;

		// set world translation
		//m_local2world.SetTranslation(CPoint(m_extremity.m_position));
		pElem->m_local2world.SetTranslation(CPoint(extremity));

		// compute local		
		pElem->m_local = pElem->m_local2world * pElem->m_pParent->m_local2world.GetAffineInverse();

	}
};













class ChainReconstructionInfoSleeve: public ChainReconstructionInfo
{
public:
	const ChainReconstructionSleeve* m_pMainInfo;
	
	ChainReconstructionInfoSleeve(const ChainReconstructionSleeve* pMainInfo)
	{
		m_pMainInfo = pMainInfo;
	}
	
	virtual ~ChainReconstructionInfoSleeve(){}
	virtual void UpdateSentinel(ChainElem* pElem)
	{
		UNUSED(pElem);
	}

	virtual void UpdateWorldMatrixFreeze(ChainElem* pElem, const HairStyleFromWelder* pDef)
	{
		UNUSED(pDef);
		// get x approximation
		pElem->m_local = pElem->m_pDef->m_local;
		pElem->m_local2world = pElem->m_pDef->m_local * pElem->m_pParent->m_local2world;
	}

	virtual void UpdateWorldMatrix(const ChainRessource::RotIndex& p,
		ChainElem* pElem, const HairStyleFromWelder* pDef)
	{
		// get x approximation
		CVector extremity = pElem->GetExtremityWithLatency(pDef->m_bUseLatency,pDef->m_fLatency,p);
		CDirection exaux = CDirection(CPoint(extremity) - m_pMainInfo->m_elbowWorldPosition);

		// get y axis
		CVector root = pElem->m_pParent->m_local2world[3];
		CDirection ey = CDirection(extremity - root);
		ey *= 1.0f / ey.Length();

		//CDirection ey = CDirection(m_lastRotationAxis[0]);
		CDirection ez = exaux.Cross(ey);
		ez *= 1.0f / ez.Length();
		CDirection ex = ey.Cross(ez);
		ex *= 1.0f / ex.Length();

		// set world rotation
		pElem->m_local2world = CMatrix(CONSTRUCT_CLEAR);
		pElem->m_local2world.SetXAxis(ex);
		pElem->m_local2world.SetYAxis(ey);
		pElem->m_local2world.SetZAxis(ez);

		// add extra rotation to match skin stuff (because of jamexport)
		// rotation in maya is [rotateAxis] * [jointOrient]
		// the transform is just [jointOrient]
		// warning: [rotateAxis1] * [jointOrient1] , [rotateAxis2] * [jointOrient2]
		// joint 1: transform = [jointOrient1]
		// joint 2: transform = [jointOrient2] * [rotateAxis1] * [jointOrient1]
		pElem->m_local2world = pElem->m_pDef->m_extraRot * pElem->m_local2world;

		// set world translation
		//m_local2world.SetTranslation(CPoint(m_extremity.m_position));
		pElem->m_local2world.SetTranslation(CPoint(extremity));

		// compute local		
		pElem->m_local = pElem->m_local2world * pElem->m_pParent->m_local2world.GetAffineInverse();

	}
};








void ChainReconstructionStd::SetInformation(ChainElem* pElem) const 
{
	pElem->SetChainReconstruction(NT_NEW ChainReconstructionInfoStd());
}




void ChainReconstructionTassle::SetInformation(ChainElem* pElem) const 
{
	pElem->SetChainReconstruction(NT_NEW ChainReconstructionInfoTassle());
}


ChainReconstructionSleeve::ChainReconstructionSleeve(CHierarchy* pHierarchy)
{
	int iArmIndex = pHierarchy->GetTransformIndex(CHashedString("l_elbow"));
	m_pArm = pHierarchy->GetTransform(iArmIndex);

	int iElbowIndex = pHierarchy->GetTransformIndex(CHashedString("l_elbow"));
	m_pElbow = pHierarchy->GetTransform(iElbowIndex);
	
	ntAssert(m_pArm && m_pElbow);
}
void ChainReconstructionSleeve::SetInformation(ChainElem* pElem) const 
{
	pElem->SetChainReconstruction(NT_NEW ChainReconstructionInfoSleeve(this));
}

void ChainReconstructionSleeve::Update()
{
	m_elbowWorldPosition = m_pElbow->GetWorldMatrix().GetTranslation();
}
