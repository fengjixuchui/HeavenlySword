#ifndef _SPEEDTREEDEF_PS3_H_
#define _SPEEDTREEDEF_PS3_H_

//--------------------------------------------------
//!
//!	\file SpeedTreeDef_ps3.h
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#include "core/vecmath.h"
#include <SpeedTreeRT.h>
#include "core/explicittemplate.h"




class SpeedTreeXmlTreeInstance
{
	HAS_INTERFACE(SpeedTreeXmlTreeInstance);
public:
	CPoint	m_position;
	CQuat	m_rotation;
	bool	m_needsCollision;
public:
	SpeedTreeXmlTreeInstance() : m_needsCollision(false) {};
	SpeedTreeXmlTreeInstance(const CPoint& position, bool needsCollision) 
		: m_position(position)
		, m_needsCollision(needsCollision)
	{
		// nothing
	}
}; // end of class SpeedTreeXmlTreeInstance


class SpeedTreeXmlTree
{
	HAS_INTERFACE(SpeedTreeXmlTree);
public:
	ntstd::String m_templateName;
//	uint32_t m_seed;
//	float m_fSize;
	float m_fVariance;
	float m_fNearLodDistance;
	float m_fFarLodDistance;
	CPoint m_position;
	CQuat	m_rotation;
	typedef ntstd::List<SpeedTreeXmlTreeInstance*> List;	
	List m_list;	
	bool	m_needsCollision;
public:
	SpeedTreeXmlTree() {};
	//SpeedTreeXmlTree(uint32_t seed, float fSize, float fVariance, float fNearLodDistance, float fFarLodDistance, const CPoint& position, bool needsCollision) 
	//	:m_seed(seed)
	//	,m_fSize(fSize)
	//	,m_fVariance(fVariance)
	//	,m_fNearLodDistance(fNearLodDistance)
	//	,m_fFarLodDistance(fFarLodDistance) 
	//	,m_position(position)
	//	,m_needsCollision(needsCollision)
	//{
	//}
	void AddTreenstance(SpeedTreeXmlTreeInstance* pInstance)
	{
		m_list.push_back(pInstance);
	}
}; // end of class SpeedTreeXmlTree

class SpeedTreeXmlForest
{
	HAS_INTERFACE(SpeedTreeXmlForest);
public:
	typedef ntstd::List<SpeedTreeXmlTree*> List;	

	ntstd::String	m_speedWindFilename;
	uint32_t		m_sectorBits;
	List			m_list;	
public:
	SpeedTreeXmlForest() {};
	~SpeedTreeXmlForest();

	void AddTree(SpeedTreeXmlTree* pTree);
	void PostConstruct();
}; // SpeedTreeXmlForest




// SpeedTreeXmlWind governs the overall behavior of the wind matrix group
class SpeedTreeXmlWind
{
	HAS_INTERFACE(SpeedTreeXmlWind);
public:
	enum EControlParameter
	{
		P, I, D, A
	};

	enum EIndices
	{
		MIN, MAX
	};

	// enumerations
	enum ELeafAngles
	{
		ROCK, RUSTLE, NUM_LEAF_ANGLES
	};

	// matrices
	unsigned int	m_uiNumMatrices;

	Vec4			m_afBendLowWindControl;
	Vec4			m_afBendHighWindControl;

	Vec4			m_afVibrationLowWindControl;
	Vec4			m_afVibrationHighWindControl;

	Vec2			m_afVibrationFrequency;
	Vec2			m_afVibrationAngles;

	float			m_fMaxBendAngle;
	float			m_fStrengthAdjustmentExponent;

	// gusting
	Vec2			m_afGustStrength;
	Vec2			m_afGustDuration;
	float			m_fGustFrequency;

	Vec4			m_afGustControl;

	// leaves
	unsigned int	m_uiNumLeafAngles;
	float			m_fLeafStrengthExponent;

	// leaf angles
	Vec4			m_afLeafAngleLowWindControl[NUM_LEAF_ANGLES];
	Vec4			m_afLeafAngleHighWindControl[NUM_LEAF_ANGLES];

	Vec2			m_afLeafAngleFrequency[NUM_LEAF_ANGLES];
	Vec2			m_afLeafAngleAngles[NUM_LEAF_ANGLES];

public:
	SpeedTreeXmlWind( );
};

#endif // end of _SPEEDTREEDEF_PS3_H_
