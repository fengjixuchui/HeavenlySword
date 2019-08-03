#ifndef _CHAINRECONSTRUCTION_H_
#define _CHAINRECONSTRUCTION_H_

#include "chaindef.h"
#include "effectchain.h"

class ChainElem;

class ChainReconstructionInfo
{
public:
	ChainReconstructionInfo(){}
	virtual ~ChainReconstructionInfo(){}
	virtual void UpdateSentinel(ChainElem* pElem) = 0;
	virtual void UpdateWorldMatrix(const ChainRessource::RotIndex& p,
		ChainElem* pElem, const HairStyleFromWelder* pDef) = 0;
	virtual void UpdateWorldMatrixFreeze(ChainElem* pElem, const HairStyleFromWelder* pDef) = 0;
};


class ChainReconstruction
{
public:
	ChainReconstruction();
	virtual ~ChainReconstruction();
	virtual void Update() {};
	virtual void SetInformation(ChainElem* pElem) const = 0;
};




class ChainReconstructionStd: public ChainReconstruction
{
public:
	virtual void SetInformation(ChainElem* pElem) const;
}; // end of class ChainReconstructionStd



class ChainReconstructionTassle: public ChainReconstruction
{
public:
	virtual void SetInformation(ChainElem* pElem) const;
}; // end of class ChainReconstructionStd


class Transform;
class CHierarchy;
class ChainReconstructionSleeve: public ChainReconstruction
{
public:
	Transform* m_pArm;
	Transform* m_pElbow;
	CPoint m_elbowWorldPosition;

	virtual void Update();
	virtual void SetInformation(ChainElem* pElem) const;
	ChainReconstructionSleeve(CHierarchy* pHierarchy);
}; // end of class ChainReconstructionStd



#endif // end of _CHAINRECONSTRUCTION_H_


