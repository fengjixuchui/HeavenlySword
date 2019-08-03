#ifndef _CHAINDEF_H_
#define _CHAINDEF_H_

#include "hair/onetomanylink.h"
#include "core/explicittemplate.h"
#include "editable/enumlist.h"
#include "tbd/xmlfileinterface.h"
#include "core/bitmask.h"

#include "../code_ps3/spu/hair/hair_animator_spu.h"

class CEntity;
class OneChain;
class CollisionSphereSet;
class HairStyleFromWelder;




class ClothSpringSetDef
{
public:
	typedef ntstd::List<int, Mem::MC_GFX> Container;
	Container m_container;
	bool m_bIsCircle;
	//! constructor
	ClothSpringSetDef() {};
	int GetSize() const {return m_container.size() - (m_bIsCircle?0:1);}
}; // end of class ClothSpringSetDef













//--------------------------------------------------
//!
//!	Global hair definition
//!
//--------------------------------------------------

class ChainGlobalDef
{
public:
	//! constructor
	ChainGlobalDef();
	virtual ~ChainGlobalDef(){};

	//! constructor
	void SetDefault();

	//! constructor
	virtual bool EditorChangeValue(CallBackParameter, CallBackParameter);

	// get a debug range value clamped
	Pixel2 GetDebugRangeWithin(Pixel2 range) const;



public:
	// dummy declaration
	CVector m_obWindForce;
	CVector m_obGravitation;
	CVector m_obDummy;

	float m_fHeroRotationDebug;
	float m_fWelderTimeChange;
	float m_fArtificialWindCoef;
	float m_fDebugDecalage;
	float m_fCentrifugeEcho;
	float m_fSphereSize;
	
	bool m_bUseSphereSize;
	bool m_bUseWelderTime;
	bool m_bUseTimeMultiplier;
	bool m_bUseDebugRange;
	bool m_bUseAntiDivergence;
	
	Pixel2 m_debugRange;
	
	Array<bool,3> m_bindingDebugEnable;
	Array<ntstd::String,3> m_bindingDebug;
	
	bool m_bDrawChain;
	bool m_bDrawDefaultPosition;
	bool m_bDrawCollisionSphere;
	bool m_bDrawAntiCollisionSphere;
	bool m_bDrawDummy1;
	bool m_bDrawDummy2;
	bool m_bDrawMesh;
	bool m_bDrawForce;
	bool m_bDrawCollisionDiff;	
	bool m_bDrawAxis;
	bool m_bDrawTorque;
	bool m_bDrawWind;
	bool m_bDrawInfo;
	
	ntstd::List<HairStyleFromWelder*, Mem::MC_GFX> m_obHairstyle;	
}; // end of class ChainGlobalDef













//--------------------------------------------------
//!
//!	Per-style Welder definition
//!
//--------------------------------------------------

class HairStyleFromWelder
{
HAS_INTERFACE(HairStyleFromWelder);
public:

	HairStyleFromWelder();
	void SetDefault();
	virtual ~HairStyleFromWelder();
	
	//! post construct (real constructor)
	virtual void PostConstruct();
	//! check if some value change do not require sophisticated information
	virtual bool EditorChangeValue(CallBackParameter pcItem, CallBackParameter pcValue);

public:

	bool m_bUseClipAngle;
	bool m_bUseMegaDamp;
	bool m_bUseGravity;
	bool m_bUseWind;
	bool m_bUseCentrifuge;
	bool m_bUseArtificialWind;
	bool m_bUseParentStiff;
	bool m_bUsePoseStiff;
	bool m_bUseCollisionForce;
	bool m_bUseFluid;
	bool m_bUseAcceleration;
	bool m_bUseArtificial;
	bool m_bUsePositionCorrection;
	bool m_bUseDamp;
	bool m_bUseCollisionHeuristic;
	bool m_bUseClothCollision;
	bool m_bUseSpring;
	bool m_bUseLatency;
	bool m_bUseShrinker;

	///////////////////////////////////////////////////
	// Group influencing maya
	float m_fCollisionHeuristic;
	float m_fClipAngle;
	float m_fMegaDamp;
	float m_fDamp;
	float m_fParentStiff;
	float m_fParentStiffTreshold;
	float m_fPoseStiff;
	float m_fInertiaMoment;
	float m_fFluid;
	float m_fWeight;
	float m_fRadius;
	float m_fPotentialEnd;
	float m_fClothCollision;
	float m_fSpringStiffness;
	float m_fClothCollisionResize;
	float m_fClothOffset;
	float m_fCollisionCorrectionResize;
protected:
	float m_fPotentialExpulsion;
public:
	float GetPotentialExpulsion(float fCoef) const {return m_fPotentialExpulsion*fCoef;}
	// End of Group influencing maya
	///////////////////////////////////////////////////

	float m_fUnconstrainedCoef;
	float m_fPositionCorrection;
	float m_fSpeedCorrection;
	float m_fPotentialCoef;
	Vec2 m_collisionForceRange;

	float m_fAccelerationPropagation;
	float m_fSpeedKillAcceleration;
	float m_fCentrifuge;
	float m_fAccelerationEcho;
	int m_iNbStepPerFrame;
	float m_fLatency;
	
	ARTIFICIALWIND m_eArtificialWind;
public:
	typedef RegisterContainer<OneChain> Register;
	mutable Register m_register;
	Register& GetRegister() const {return m_register;}


	// SPU
	ChainSPU::ChainDef GetSPUDef() const;

}; // end of class HairStyleFromWelder



















class HairDependencyInfo
{
public:
	CHashedString m_hashedid;
	CHashedString m_hashedparent;

	ntstd::String m_parentName;
	ntstd::String m_name;

private:	
	HairDependencyInfo* m_pParent;
public:
	bool HasParent() const {return m_pParent!=0;}
	void SetParent(HairDependencyInfo* pParent) {m_pParent=pParent;}

	//template<class T>
	//const T* GetParentT() const { ntAssert(HasParent()); return static_cast<T*>(m_pParent);}
	//template<class T>
	//T* GetParentT() { ntAssert(HasParent()); return static_cast<T*>(m_pParent);}

	const HairDependencyInfo* GetParentT() const { ntAssert(HasParent()); return m_pParent;}
	HairDependencyInfo* GetParentT() { ntAssert(HasParent()); return m_pParent;}

	HairDependencyInfo(): m_pParent(0) {};
	void Finalise();
}; // end of class HairDependencyInfo




class HairRootDef: public HairDependencyInfo
{
HAS_INTERFACE(HairRootDef);
public:
	// name of the joint it's bound to
	ntstd::String m_bindName;
	CHashedString m_bindid;

protected:
	// maya transform info
	CVector m_translation;
	CQuat m_jointOrient;
	CQuat m_rotateAxis;
	CQuat m_rotate;

	// local transform
	CMatrix m_local;
	// local to pose transform
	CMatrix m_local2sim;

public:
	// get rotate axis, useful to output joint orient in the game
	CQuat GetRotateAxis() const {return m_rotateAxis;}
	const CMatrix& GetPoseMatrix() const {return m_local2sim;}
public:
	HairRootDef();
	// compute matrices
	void Finalise();
}; // end of class HairRootDef



class CHierarchy;
class ChainElem;
class HairStyleElemFromMayaInterface;

//--------------------------------------------------
//!
//!	Per-joint maya definition
//!
//--------------------------------------------------

class HairStyleElemFromMaya: public HairDependencyInfo
{
HAS_INTERFACE(HairStyleElemFromMaya);
protected:
	///////////////////////////////////////////////////
	// Group from Maya attributes
	float m_fRadius;
	float m_fDamp;
	float m_fParentStiff;
	float m_fPoseStiff;
	float m_fFluid;
	float m_fInertiaMoment;
	float m_fWeight;
	float m_fParentStiffTreshold;
	bool m_bFreeze;
	bool m_bIsLeaf;
	// End of Group from Maya attributes
	///////////////////////////////////////////////////
public:
	
	int m_elemId;	
	
	///////////////////////////////////////////////////
	// Group From maya scene
	float m_fDistanceToRoot;
	float m_fLength;

	CVector m_translation;
	CQuat m_jointOrient;
	CQuat m_rotateAxis;
	
	// End of Group From maya scene
	///////////////////////////////////////////////////
public:
	///////////////////////////////////////////////////
	// Group get
	float GetDamp(const HairStyleFromWelder* pDef) const {return m_fDamp*pDef->m_fDamp;}
	float GetParentStiff(const HairStyleFromWelder* pDef) const {return m_fParentStiff*pDef->m_fParentStiff;}
	float GetPoseStiff(const HairStyleFromWelder* pDef) const {return m_fPoseStiff*pDef->m_fPoseStiff;}
	float GetFluid(const HairStyleFromWelder* pDef) const {return m_fFluid*pDef->m_fFluid;}
	float GetInertiaMoment(const HairStyleFromWelder* pDef) const {return m_fInertiaMoment*pDef->m_fInertiaMoment;}
	float GetWeight(const HairStyleFromWelder* pDef) const {return m_fWeight*pDef->m_fWeight;}
	float GetParentStiffTreshold(const HairStyleFromWelder* pDef) const;
	float GetRadius(const HairStyleFromWelder* pDef) const {return pDef->m_fRadius * m_fRadius;}
	bool GetFreeze() const {return m_bFreeze;}
	bool IsLeaf() const {return m_bIsLeaf;}
	// End of Group get
	///////////////////////////////////////////////////

	///////////////////////////////////////////////////
	// Group spu


	// spu code
	ChainSPU::ChainElemDef GetSPUDef(const HairStyleFromWelder& def) const
	{
		ChainSPU::ChainElemDef res;
	
		// forces
		res.m_fFluid = GetFluid(&def);
		res.m_fWeight = GetWeight(&def);
		res.m_fStiffDotTreshold = GetParentStiffTreshold(&def);
		res.m_fStiff = GetParentStiff(&def);

		// equation
		res.m_fLength = m_fLength;
		res.m_fInertiaMoment = GetInertiaMoment(&def);
		res.m_fDamp = GetDamp(&def);

		// distance to root
		res.m_fDistanceToRoot = m_fDistanceToRoot;

		// collision
		res.m_fRadius = GetRadius(&def);
		
		// parent to local transform
		res.m_local = m_local;

		// hack to find back the right joint orent axis
		res.m_extraRotInv = m_extraRotInv;
		res.m_extraRot = m_extraRot;
		
		// flags
		res.m_flags.Set(ChainSPU::ChainElemDef::ISFROZEN,GetFreeze());
		return res;
	}
	// spu code
	static ChainSPU::ChainElemDef GetDefaultSPUDef()
	{
		ChainSPU::ChainElemDef res;
	
		// forces
		res.m_fFluid = 0.0f;
		res.m_fWeight =  0.0f;
		res.m_fStiffDotTreshold =  0.0f;
		res.m_fStiff =  0.0f;

		// equation
		res.m_fLength =  0.0f;
		res.m_fInertiaMoment =  0.0f;
		res.m_fDamp =  0.0f;
		
		// distance to root
		res.m_fDistanceToRoot = 0.0f;

		// collision
		res.m_fRadius =  0.0f;
		
		// parent to local transform
		res.m_local = CMatrix(CONSTRUCT_CLEAR);

		// hack to find back the right joint orent axis
		res.m_extraRotInv = CMatrix(CONSTRUCT_CLEAR);
		res.m_extraRot = CMatrix(CONSTRUCT_CLEAR);
		
		// flags -> default
		//res.m_flags...
		return res;
	}
	// Group spu
	///////////////////////////////////////////////////



	///////////////////////////////////////////////////
	// Group finalise
protected:
	CMatrix m_local2sim;
public:
	CMatrix m_local;
	CMatrix m_extraRot;
	CMatrix m_extraRotInv;

	float m_fNormDistanceToRoot;

	// pointer to root joint, null if parent exist
	HairRootDef* m_pRootParent;

	// End of Group finalise
	///////////////////////////////////////////////////
public:
	mutable ChainElem* m_pTmp;	
	
public:
	///////////////////////////////////////////////////
	// Group get
	const CMatrix& GetPoseMatrix() const {return m_local2sim;}
	CVector GetLocalTranslation() const {return m_local[3];};
	CMatrix GetWorldPose(const CMatrix& sim2word) const;

	// End of Group get
	///////////////////////////////////////////////////
	
	// Finalise
	void Finalise();
	
	HairStyleElemFromMaya();
	~HairStyleElemFromMaya();

	// get parent
	const HairStyleElemFromMaya* GetParent() const { return static_cast<const HairStyleElemFromMaya*>(GetParentT());}

}; // end of class HairStyleElemFromMaya






//--------------------------------------------------
//!
//!	class describing a pshere in which the node must stay
//!
//--------------------------------------------------

class ClothColprim
{
HAS_INTERFACE(ClothColprim);
public:	
	typedef enum
	{
		F_CHESTSHRINK  = BITFLAG(0),
		F_SPEEDSHRINK  = BITFLAG(1),
		F_WEAPONSHRINK  = BITFLAG(2),
	} State;
	typedef BitMask<State> Mask;
	Mask m_mask;
protected:
	class OneInfluence
	{
	public:
		ntstd::String m_relativeName;

		CQuat m_rotate;
		CVector m_translate;

		CMatrix m_local2Sphere;
		CMatrix m_sphere2Local;

		float m_fWeigth;

		//! constructor
		void Finalise(float fXDec);
		OneInfluence();
	}; // end of class OneInfluence

protected:
	Array<OneInfluence,2> m_influences;
public:
	bool m_bJustOne;

	float m_fShrinkerChest;
	float m_fShrinkerBend;
	float m_fShrinkerWeapon;

	int m_parentId;
	CVector m_scale;
	
	// only valid if just one influecne (otherwise, shrink)
	CMatrix m_scaleMat;
	CMatrix m_scaleMatInv;

	// only valid if just one influecne
	CMatrix m_local2Sphere;
	CMatrix m_sphere2Local;
	
public:	
	//! constructor
	const OneInfluence& GetInfluence1() const {return m_influences[0];};
	const OneInfluence& GetInfluence2() const {ntAssert(!m_bJustOne);return m_influences[1];};
	void Finalise();
	ClothColprim();
	
}; // end of class ClothColprim


// register typedef
class OneChainRegister: public  RegisterContainer<OneChain>, public KeepMeContainer::KeepMe
{
public:
	OneChainRegister(){}
};

class HairStyleFromMayaInterface;

//--------------------------------------------------
//!
//!	Per-style maya definition
//!
//--------------------------------------------------

class HairStyleFromMaya: public XmlFileInterface
{
friend class HairStyleFromMayaInterface;
public:
	// joint to reparent to
	ntstd::String m_parentTransformName;
	
	// maximum distance to root
	float m_fMaxDistanceToRoot;

	// max id in maya
	int m_maxMayaId;

	// dependency container
	typedef ntstd::List<HairStyleElemFromMaya*, Mem::MC_GFX> List;
	List m_jointList;

	// dependency container
	typedef ntstd::List<HairRootDef*, Mem::MC_GFX> RootList;
	RootList m_rootList;

	// dependency container
	typedef ntstd::List<ClothColprim*, Mem::MC_GFX> ClothList;
	ClothList m_clothList;

	// dependency container
	typedef ntstd::List<ClothSpringSetDef*, Mem::MC_GFX> SpringList;
	SpringList m_springList;


public:
	
	// dependency container
	typedef ntstd::Vector<HairStyleElemFromMaya*, Mem::MC_GFX> JointDependency;
	JointDependency m_jointDependency;

	// dependency container
	typedef ntstd::Vector<HairRootDef*, Mem::MC_GFX> RootDependency;
	RootDependency m_rootDependency;

	// dependency container
	typedef ntstd::Vector<int, Mem::MC_GFX> IndexMap;
	IndexMap m_name2index;
	
	// get id
	static int GetMayaIndexFromName(const ntstd::String& name);
	int GetGameIndexFromMayaIndex(int iMayaId) const;
protected:

	// propagate change after reload
	virtual void PropagateChange();
	
public:

	//! constructor
	HairStyleFromMaya();

	//! destructor
	~HairStyleFromMaya();

	HairRootDef* FindRoot(const ntstd::String& name);

	int GetNbSpring() const;
	
	// get number of elems
	inline int GetNbElem() const
	{
		return m_jointDependency.size();
	}

	// finalise function (post construct)
	void Finalise();	
	
	// debug check
	bool CheckRoot();

	// reset all the tmp vector to 0
	void ResetTmp() const;	

	// find out which node is the parent to which
	void ComputeDependency();	

	// register typedef
	typedef OneChainRegister Register;
	Register& GetRegister() const;

}; // end of class ChainCollisionDef













class SwordCollisionDef
{
public:
	CVector m_scaleOut;
	CVector m_scaleIn;
	
	CVector m_translateOut;
	CVector m_translateIn;

	CQuat m_rotateOut;
	CQuat m_rotateIn;
public:
	CVector GetScale(float fOutCoef) const;
	CVector GetTranslate(float fOutCoef) const;
	CQuat GetRotate(float fOutCoef) const;
	
	//typedef RegisterContainer<CollisionSword> Register;
	//mutable Register m_register;
	
	//! constructor
	SwordCollisionDef();
}; // end of class SwordCollisionDef







class FloorCollisionDef
{
public:
	CVector m_scale;
	CVector m_translate;
	FloorCollisionDef() {};
}; // end of class FloorCollisionDef







class HairSphereDefInterface;

//--------------------------------------------------
//!
//!	Per sphere maya definition
//!
//--------------------------------------------------

class HairSphereDef
{
friend class HairSphereDefInterface;
public:
	// non xml construct
	HairSphereDef(const CVector& translate, const CVector& scale, const CQuat& rotate);

	//! constructor
	HairSphereDef();

	//! destructor
	~HairSphereDef();
	
	// finalise
	void Finalise();
	
	ChainSPU::MetaBallDef GetSpuDef() const
	{
		ChainSPU::MetaBallDef res;
		res.m_fAverageRadius = GetAverageRadius();
		res.m_scale = m_scale;
		return res;
	}

protected:

	///////////////////////////////////////////////////
	// comnig from maya
	float m_fPositionCorrection;
	float m_fSpeedCorrection;
	
	CVector m_translate;
	CQuat m_rotate;
	CVector m_scale;
	// End comnig from maya
	///////////////////////////////////////////////////
	
public:
	// local matrix
	CMatrix m_sphere2Local;
	
	// local matrix
	CMatrix m_local2Sphere;
	
	// parent name
	ntstd::String m_parent;
	
	// get average radius
	float GetAverageRadius() const;

	// get average radius
	CVector GetScale() const {return m_scale;}

	// force scale (use in extreme case)
	void SetScale(const CVector& scale);
	void SetRotate(const CQuat& rotate);
	void SetTranslate(const CVector& translate);
public:
	float GetPositionCorrection(const HairStyleFromWelder* pDef) const {return m_fPositionCorrection*pDef->m_fPositionCorrection;}
	float GetSpeedCorrection(const HairStyleFromWelder* pDef) const {return m_fSpeedCorrection*pDef->m_fSpeedCorrection;}
	
}; // end of class ChainCollisionDef





// register typedef
class HairCollisionInstanceRegister: public RegisterContainer<CollisionSphereSet>, public KeepMeContainer::KeepMe
{
public:
	HairCollisionInstanceRegister(){}
};
//--------------------------------------------------
//!
//!	 Per collision data maya def
//!
//--------------------------------------------------

class HairSphereSetDef: public XmlFileInterface
{
public:
	// list of sphere def type
	typedef ntstd::List<HairSphereDef*,Mem::MC_GFX> HairSphereDefList;
	HairSphereDefList m_list;
	
	// register typedef
	typedef HairCollisionInstanceRegister Register;

protected:

	// propagate change after reload
	virtual void PropagateChange();

public:
	//! constructor
	HairSphereSetDef();
	
	//! constructor
	~HairSphereSetDef();
	
	// get register
	Register& GetRegister() const;

	// finalise
	void Finalise();
}; // end of class SetHairSphereDef






#endif // end of _CHAINDEF_H_

