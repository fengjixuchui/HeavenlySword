#ifndef _CHAINCORE_H_
#define _CHAINCORE_H_


#include "core/explicittemplate.h"
#include "core/vecmath.h"
#include "chaindef.h"
#include "effectchain.h"
#include "core/rotationnalindex.h"
#include "tbd/containervector.h"
#include "haircollision.h"
#include "anim/transform.h"
#include "game/entity.h"
#include "game/entity.inl"
#include "core/smartptr.h"
#include "hair/forcefieldresult.h"
#include "hairstring.h"
#include "game/anonymouscomponent.h"
#include "..\code_ps3\spu\hair\hair_animator_spu.h"

//#define CHECK_HAIR_NANS

#ifdef	CHECK_HAIR_NANS
#include "core/nanchecker.h"
#endif

class Transform;
class ChainElem;
class ArtificialWind;



//--------------------------------------------------
//!
//!	statistic on the extremity.
//!	position, speed, acceleration, eventual filter, used by child node
//!
//--------------------------------------------------

class ExtremityState
{
public:
	// speed of the extremity
	CVector m_position;
	
	// speed of the extremity
	CVector m_speed;

	// acceleration of the extemity
	CVector m_acceleration;

public:	
	// null ctor
	ExtremityState():m_position(CONSTRUCT_CLEAR),m_speed(CONSTRUCT_CLEAR),m_acceleration(CONSTRUCT_CLEAR){}

	// init ctor
	ExtremityState(const CVector& position):m_position(position),m_speed(CONSTRUCT_CLEAR),m_acceleration(CONSTRUCT_CLEAR){}

	// init ctor
	ExtremityState(const CVector& position, const CVector& speed, const CVector& acceleration):
		m_position(position),m_speed(speed),m_acceleration(acceleration){}

	// decrease acceleration smoothly, because the game acceleration can be very "square"
	void UpdateAcceleration(const CVector& acceleration,float fCoef);
};


//--------------------------------------------------
//!
//!	axis and angle.
//!	That's is, just a "sometime handy" way of storing a quaternion
//!	Exciting class with lots of stuff to describe
//!
//--------------------------------------------------

class AxisAndAngle
{
public:
	// rotation angle
	float m_fAngle;
	// rotation axis
	CDirection m_axis;
	//! constructor
	AxisAndAngle() {};
}; // end of class AxisAndAngle









class HairCollisionSphere;
class PerChainInstanceModifier;
class CollisionSphereSet;
class ClothSpringInstance;
class ChainReconstructionInfo;
class ChainReconstructionInfoStd;
class ChainReconstructionInfoTassle;
class ChainReconstructionInfoSleeve;


class ChainElem
{
friend class CollisionSphereSet;
friend class HairCollisionSphere;
friend class ChainReconstructionInfoStd;
friend class ChainReconstructionInfoTassle;
friend class ChainReconstructionInfoSleeve;
public:
	//--------------------------------------------------
	//!
	//!	Dynamic data for physics simulation.
	//!	contain the data which are needed at different time
	//!	Exciting class with lots of stuff to describe
	//!
	//--------------------------------------------------

	class Dynamic
	{
	public:
		/// position of the extremity in world RF
		CVector m_worldExtremity;
		/// angles in sim RF
		Vec2 m_worldAngles;
	public:
		/// null constructor
		Dynamic(): m_worldExtremity(CONSTRUCT_CLEAR), m_worldAngles(0.0f,0.0f) {}
		/// constructor
		Dynamic(const CVector& extremity, const Vec2& angles): m_worldExtremity(extremity), m_worldAngles(angles) {}
		/// scale value (to do some average
		Dynamic operator * (float fCoef);
		// addition
		void operator += (const Dynamic& din);
		// substarction
		void operator -= (const Dynamic& din);
	};
	
	/// Dynamic time array
	typedef Array<Dynamic,HairConstant::HAIR_MEMORY_SIZE> DynamicArray;
	
	
protected:	
	// parent
	ChainElem* m_pParent;

	// position speed and acceleration of the root
	ClothSpringInstance* m_pSpring;
	
	// save last position before collision correction
	CVector m_lastPosition;
	
	// dynamic variable
	DynamicArray m_dynamic;

	// last valid value
	ExtremityState m_extremity;

	// link to the game engine
	Transform* m_pExtremityTransform;
	
	// link to maya def
	const HairStyleElemFromMaya* m_pDef;

	// last rotation axis
	CMatrix m_lastRotationAxis;
		
	///////////////////////////////////////////////////
	// Group RECONSTRUCTION
	
	// word matrix
	CMatrix m_local2world;

	// local matrix (to be injected in the CHierarchy)
	CMatrix m_local;

	// End of Group RECONSTRUCTION
	///////////////////////////////////////////////////
		
	HairCollisionHeuristic m_heuritic;

	// instance of cloth
	ClothColprimInstance* m_pClothSphere;

	// approximation of the x axis
	CScopedPtr<ChainReconstructionInfo> m_pChainReconstruction;

public:
	void SetChainReconstruction(ChainReconstructionInfo* pt);
	template<class T>
	inline T* GetReconstructionInfo()
	{
		return static_cast<T*>(m_pChainReconstruction.Get());
	}

	//de bug
	void DrawSpringDebug(const ChainRessource::RotIndex& p);

	// link cloth
	void LinkCloth(ClothColprimInstance* pCi) {m_pClothSphere=pCi;}
	
	// position correction
	CorrectedPosition m_correctedPosition;

	// null constructor
	ChainElem();
	
	// position speed and acceleration of the root
	void SetSpring(ClothSpringInstance* pSpring);
	
	///////////////////////////////////////////////////
	// Group get
	const ExtremityState& GetExtremity() const {return m_extremity;}
	ExtremityState& GetExtremity() {return m_extremity;}
	const ExtremityState& GetRootExtremity();
	const CMatrix& GetRotationAxisMatrix() const {return m_lastRotationAxis;}
	const CMatrix& GetLocalMatrix() const {return m_local;}
	const CMatrix& GetWorldMatrix() const {return m_local2world;}
	Transform* GetExtremityTransform() {return m_pExtremityTransform;}
	Transform* GetRootTransform() {return m_pExtremityTransform->GetParent();}
	Vec2 GetAngleFromHierarchy();
	Vec2 GetAngleFromPose(const CMatrix& sim2world);
	ChainElem* GetParent() {return m_pParent;}
	bool HasDef() const { return m_pDef!=0;}
	bool ParentHasDef() const {ntAssert(m_pParent); return m_pParent->HasDef();}
	const HairStyleElemFromMaya* GetMayaDef() const { ntAssert(m_pDef); return m_pDef;}
	const DynamicArray& GetDynamic() const {return m_dynamic;}
	// End of Group get
	///////////////////////////////////////////////////

	///////////////////////////////////////////////////
	// Group sentinel
	void InitSentinel(Transform* pTransform);
	void ResetSentinel(const HairStyleFromMaya* m_pMayaDef);
	void UpdateSentinel(const ChainRessource::RotIndex& p, float fDelta,
		const HairStyleFromWelder* pDef, const HairStyleFromMaya* m_pMayaDef);
	// End of Group sentinel
	///////////////////////////////////////////////////	
	
	///////////////////////////////////////////////////
	// Group set
	void SetExtremityTransform(Transform* pExtTrans);
	void SetLocalMatrixExtremityTransform();
	void SetParent(ChainElem* pElem) {m_pParent = pElem;}
	void SetDef(const HairStyleElemFromMaya* pDef) {m_pDef = pDef;}
	// End of Group set
	///////////////////////////////////////////////////

	// position correction	
	void ApplyDiff(const ChainRessource::RotIndex& p, const CVector& worldDiff, float fKillAcc, float fResize);
	
	// position correction	
	void CollisionForce(const ChainRessource::RotIndex& p, CVector& forces, const HairStyleFromWelder* pDefWelder, float fDelta);

	// position correction	
	void ClothCollisionCorrection(const ChainRessource::RotIndex& p, const HairStyleFromWelder* pDefWelder);
	
	// compute extremity with latency	
	CVector GetExtremityWithLatency(bool bUseLatency, float fLatency, const ChainRessource::RotIndex& p);

	// set axis matrix
	void SetAxisMatrix(const ChainRessource::RotIndex& p);
	
	// compute nextr time step of frozen joint
	void ComputeFreeze(const ChainRessource::RotIndex& p, float fDelta, const HairStyleFromWelder* pDefWelder);

	// compute nextr time step
	void Compute(Vec2 torque, const ChainRessource::RotIndex& p,
		const HairStyleFromWelder* pDefWelder, float fDelta, const PerChainInstanceModifier& modifier);

	// compute nextr time step
	bool Finalise(const ChainRessource::RotIndex& p,
		const HairStyleFromWelder* pDefWelder, float fDelta);
	
	// update world matrix (weird stuff going on here)
	void UpdateWorldMatrix(const ChainRessource::RotIndex& p, const HairStyleFromWelder* pWelderDef);
	
	// get local forces (fluid and accelertion)
	CVector GetLocalForces(const HairStyleFromWelder* pDefWelder, const ChainRessource::RotIndex& p,
		const CVector& unitGravityForce, const CVector& wind, const CVector& artificial,
		float fDelta, const PerChainInstanceModifier& modifier);
	
	// reset root (dummy rotationnal index)
	void Reset(const ChainRessource::RotIndex& p, Vec2 angle, int iSphereSize, const HairStyleFromWelder* pDefWelder);
	
	// push filter value
	void PushFilterValue(int iFilterSize, const ChainRessource::RotIndex& p);

	// from force to torque
	Vec2 ForceToTorque(const CVector& forces, const ChainRessource::RotIndex& p);

	// Get X axis approximation
	CDirection GetXAxisApprox();

	// draw debug collision information
	void DrawDebugCollision(const ChainRessource::RotIndex& p, const HairStyleFromWelder* pDefWelder);

	// return true if numeric value appear to be ok
	bool IsNumericOk(const ChainRessource::RotIndex& p);

	// renormalise angle to avoid numerical deviation
	void RenormaliseAngle(const ChainRessource::RotIndex& p);

protected:
	// update state with new value
	void UpdateExtremityState(const ChainRessource::RotIndex& p, float fDelta, const HairStyleFromWelder* pDef);

};

















class HairSentinel
{
public:
	// number of sample used ti filter out position, velocity and acceleration
	static const int m_iNbFrame = 3;
	static const int m_iNbSubframe = 2;

	typedef RotationnalIndex<int,HairSentinel::m_iNbFrame> RotPerFrame;
	typedef RotationnalIndex<int,HairSentinel::m_iNbSubframe> RotPerSubFrame;

private:
	// info store per frame
	class PerFrameInfo
	{
	public:
		CMatrix m_sim2World;
		CQuat m_quat;
		PerFrameInfo(): m_sim2World(CONSTRUCT_CLEAR), m_quat(CONSTRUCT_CLEAR) {}
		explicit PerFrameInfo(const CMatrix& m): m_sim2World(m), m_quat(m) {}
	};

	// info store every sub frame
	class PerSubFrameInfo
	{
	public:
		CQuat m_quat;
		PerSubFrameInfo(): m_quat(CONSTRUCT_IDENTITY){}
		explicit PerSubFrameInfo(const CQuat& q): m_quat(q){}
	};

	// head position
	Array<PerFrameInfo,m_iNbFrame> m_perFrameInfo;
	Array<PerSubFrameInfo,m_iNbSubframe> m_perSubframeInfo;

public:
	ChainElem m_chainElem;
	const HairRootDef* m_pDef;
	Transform* m_pSrcTransform;
	Transform* m_pDestTransform;
	HairSentinel* m_pParent;
public:
	HairSentinel();
	~HairSentinel();
	void SetDef(const HairRootDef* m_pDef);

	void Init(Transform* pSrcTransform, Transform* pDestTransform);
	void Reset(const HairStyleFromMaya* m_pMayaDef);
	void Update(const RotPerFrame& p);
	void HighFreqUpdate(const RotPerFrame& frame, const RotPerSubFrame& subframe,
		const ChainRessource::RotIndex& p, float fDelta, float fLerp,
		const HairStyleFromWelder* pDefWelder, const HairStyleFromMaya* pMayaDef);

	const Transform* GetSrcTransform() const {return m_pSrcTransform;}
	CQuat GetRotationDiff(const RotPerSubFrame& subframe);
public:
	CVector GetWorldSpacePosition(const RotPerFrame& frame, float fLerp) const;
	CVector GetWorldSpaceSpeed(const RotPerFrame& frame, float fLerp, float fDelta) const;
	CVector GetWorldSpaceAcceleration(const RotPerFrame& frame, float fLerp, float fDelta) const;
}; // end of class Sentinel



class ChainReconstruction;

//--------------------------------------------------
//!
//!	root tracking.
//!	to do proper sub frame computation, the root position, speed and acceleration
//! are smoothly computed between their value every frame
//! to some centrifuge and smooth acceleration stuff
//!
//--------------------------------------------------
class RootTracking: CNonCopyable
{
protected:		
	// index for high freq time coherence
	HairSentinel::RotPerFrame m_perFrameIndex;
	HairSentinel::RotPerSubFrame m_perSubFrameIndex;

	// parent hierarchy
	const CHierarchy* m_pSrcHierarchy;
	CHierarchy* m_pDestHierarchy;

	// position speed and acceleration of the root
	ContainerVector<HairSentinel, Mem::MC_PROCEDURAL > m_sentinels;

	// rotation description
	AxisAndAngle m_axisAndAngle;

	// centrifuge power
	float m_fCentrifuge;

public:	
	///////////////////////////////////////////////////
	// Group get
	const Transform* GetFirstTransform() const {return m_sentinels[0].GetSrcTransform();}
	const AxisAndAngle& GetAxisAndAngle() const {return m_axisAndAngle;}
	// End of Group get
	///////////////////////////////////////////////////

	// null constructor
	RootTracking(const HairStyleFromMaya* pMayaDef);
	~RootTracking();

	// get centrifuge power
	float GetCentrifugePower() {return m_fCentrifuge;}

	// reset value
	void Init(const HairStyleFromMaya* pMayaDef,
		const CHierarchy* pSrcHierarchy, CHierarchy* pDestHierarchy);

	// reset value
	void Reset(const HairStyleFromMaya* m_pMayaDef, const ChainReconstruction* pHairReconstruction);

	// per frame update value
	void Update();

	// high freq update
	void HighFreqUpdate(float fLerp, float fDelta, const ChainRessource::RotIndex& p,
		const HairStyleFromWelder* pDef, const HairStyleFromMaya* pMayaDef);

	// get stupid debug decalage to see what's happening
	CDirection GetDebugWorldSpaceDecalage(float fDec);

	// get debug position
	CPoint GetDebugWorldSpacePosition();

	// get acceleration coef
	float GetKillAccCoef(const HairStyleFromWelder* pDef, float fHighFreqProgress, float fDelta) const;

	// compute centrifuge coef
	void ComputeCentrifuge(float fDelta);

	// find sentinel
	ChainElem* GetSentinel(const CHashedString& id);
	ChainElem* GetSentinel(uint32_t uiId);
	int GetNbSentinel() const {return m_sentinels.GetSize();}

	// draw debug
	void DrawDebug(bool bDrawAxis);
	void DrawDefaultPosition(bool bDrawAxis);
};




class PerChainInstanceModifier
{
public:
	float m_fPotentialExpulsionCoef;
	bool m_bForceUseMegaDamp;
	float m_fMegaDampCoef;
	float m_fAccCoef;
	//! constructor
	PerChainInstanceModifier();
}; // end of class PerInstanceModifier


class ChainAnimation2Render;
class CollisionFloor;

class HairSpecialStuff
{
public:
	ArtificialWind* m_pArtificialWind;
	CollisionSword* m_pSword;
	CollisionFloor* m_pFloor;
	CScopedPtr<ChainAnimation2Render, Mem::MC_PROCEDURAL> m_pForWill;
public:
	void Update(const HairStyleFromWelder* pDefWelder, PerChainInstanceModifier& modifier);
	HairSpecialStuff();
	~HairSpecialStuff();
}; // end of class HairSpecialStuff



class CollisionSphereSet;


#ifdef PLATFORM_PS3
#include "spu/hair/hair_animator_spu.h"
#endif // PLATFORM_PS3


class SPUProgram;


//--------------------------------------------------
//!
//!	Short Class Description.
//!	Long Class Description
//!	Exciting class with lots of stuff to describe
//!
//--------------------------------------------------

class OneChain: public CAnonymousEntComponent
{
public:
	typedef enum
	{
		ONECHAIN = 0, // init
		CHAINELEM_STATIC,
		CHAINELEM_DYNAMIC,
		CHAINELEM_SENTINEL,
		CHAINELEM_SPRING,
		CHAINELEM_SPRINGINDEX,
		CHAINELEM_ANTICOLLISION,
		DYNAMIC_0_POS,
		DYNAMIC_0_ANGLE,
		DYNAMIC_1_POS,
		DYNAMIC_1_ANGLE,
		DYNAMIC_2_POS,
		DYNAMIC_2_ANGLE,
		ONECHAIN_MEM_SIZE,
		// nb elem in this enum:
		ONECHAIN_NB_ELEMS,
	} ONECHAIN_LOCATION;

	typedef enum
	{
		METABALL = 0, // init
		METABALL_ARRAY,
		METABALL_MEM_SIZE,
		// nb elem in this enum:
		METABALL_NB_ELEMS,
	} METABALL_LOCATION;

	// for placement new
	uint32_t m_uiDmaOneChainLocation[ONECHAIN_NB_ELEMS];
	CScopedArray<uint8_t, Mem::MC_PROCEDURAL> m_pOneChainDmaMemory;

	uint32_t m_uiDmaMetaBallsLocation[METABALL_NB_ELEMS];
	CScopedArray<uint8_t, Mem::MC_PROCEDURAL> m_pMetaBallsDmaMemory;

	ChainSPU::MetaBallGroup* m_pMetaBallGroup;

#ifndef CHECK_HAIR_NANS
	ChainSPU::OneChain* m_pOneChain;
#else
	static void CheckOneChain(ChainSPU::OneChain* chain);
	CheckedPtr<ChainSPU::OneChain, &CheckOneChain> m_pOneChain;
#endif

	ChainSPU::Global* m_pGlobal;				///<! NB allocated in MC_PROCEDURAL

#ifdef PLATFORM_PS3
	const SPUProgram* m_pElf;
#endif

	void ComputeLocation();
	void CreatePS3Data();
	void InitPS3Data();
	void DestroyPS3Data();
	void AllocateDmaMemory();
	void UpdateSPU(const CVector& wind, const CVector& unitGravityForce);
	void UpdateAntiCollisionData();
	void UpdateDef();
	void SetStaticInfo();
	void UpdateSentinel();
	void InitSentinel();
	void InitIterativeData();
	void PutDataIntoHierarchySPU();
	void UpdateCollisionData();
	void InitSPUDynamic(const ChainRessource::RotIndex& p);
	uint32_t GetIndexOf(const ChainElem* pElem);

private:
	// parent hair hierarchy
	CHierarchy* m_pHierarchy;

	// hackiness to modyfy the def on a per instance basis...
	// need some proper solution here
	PerChainInstanceModifier m_modifier;
	
	// special stuff
	HairSpecialStuff m_special;
	
	// approximation of the x axis
	CScopedPtr<ChainReconstruction, Mem::MC_PROCEDURAL> m_pChainReconstruction;
	
	// an instance of the root tracking class (which can be see as a sub-frame observer)
	RootTracking m_rootTracking;
		
	// index for high-frequency time coherence
	ChainRessource::RotIndex m_highFreqIndex;

	// welder style-def
	const HairStyleFromWelder* m_pDefWelder;
	
	// maya style-def
	const HairStyleFromMaya* m_pDefMaya;
	
	// the actual dynamic joints
	ContainerVector<ChainElem, Mem::MC_PROCEDURAL > m_dynamicjoints;
	
	// collision sphere
	CollisionSphereSet* m_pCollisionSpheres;
	
	// cloth
	CScopedPtr<ClothColprimInstanceSet,Mem::MC_PROCEDURAL> m_clothSpheres;

	// entity
	CEntity* m_pSelf;
	
	// sub frame indices
	int m_iSubFrames;
	
	// elem to renormalise
	int m_NumRenormalised;
	
	// enum for bitmask
	typedef enum
	{
		F_ISACTIVE = BITFLAG(0),
		F_NEEDTOGRABDATAFROMPOSE = BITFLAG(1),
	} State;

	// am I active?
	BitMask<State> m_mask;
	
	// time info
	TimeInfo m_timeInfo;

	typedef ntstd::Vector<ClothSpringSetInstance, Mem::MC_GFX> ClothSpringSetInstanceVector;
	ClothSpringSetInstanceVector m_clothSpringContainer;
public:
	
	///////////////////////////////////////////////////
	// get
	const HairStyleFromMaya* GetMayaDef() const { ntAssert(m_pDefMaya); return m_pDefMaya;}
	const HairStyleFromWelder* GetWelderDef() const { ntAssert(m_pDefWelder); return m_pDefWelder;}
	CHierarchy* GetHierarchy() { ntAssert(m_pHierarchy); return m_pHierarchy;}
	const CHierarchy* GetParentHierarchy() const;
	// End get
	///////////////////////////////////////////////////
	
	// get entity
	CEntity* GetEntity() {return m_pSelf;}

	// get the number of collision spheres
	int GetNbCollisionSpheres() const;
	int GetNbAntiCollisionSpheres() const;
	
	// assign cloth to chainelem
	void AssignClothCollision();
	
	// spring
	void AssignSpring();
	
	// perform per type operation
	void PerType();
	void InstallSwordCollision(const ntstd::String& name, bool bInsertIntoCollision);
	void InstallSleeveShrinker();
	void InstallArtificialWind(const ntstd::String& name);
	void InstallCollision(const ntstd::String& name);
	void InstallFloorCollision(const ntstd::String& name, Transform* pTransfrom);
	void InstallForWill();

	// turn on
	void TurnOn();
	
	// turn off
	void TurnOff();
	
	// toggle
	bool Toggle();

	// get data from the hierarchy and set everything to be ok for
	// starting the simulation
	void GetDataFromHierarchy();

	// get data from the pose in and set everything to be ok for
	// starting the simulation
	void GetDataFromPose();

	// put the data into the hierarchy... This is expensive and I'm not sure
	// that the hierarchy should be the target...
	// perhaps the animation should be extracted from the hierarchy, or perhaps the
	// hierarchy should be completely bypassed
	void PutDataIntoHierarchy();
	
	// nasty hack, need a proper management system
	void TmpResetStyleData(const HairStyleFromMaya* pDefMaya);
	
	// draw chain
	void DrawDebug(bool bDrawAxis);
	
	// draw default position
	void DrawDefaultPosition();
		
	// draw chain
	void DrawDebugHierarchy();
	void DrawDebugRawHierarchy();

	// draw debug collision information
	void DrawDebugCollision();
	
	// draw collision sphere
	void DrawCollisionSpheres() const;
	void DrawAntiCollisionSpheres() const;
	
	// draw wind debug
	void DrawWindDebug(const CVector& wind);

	// spring debug
	void DrawSpringDebug();
	
	// dummy drawe
	void DrawDummy();

	// draw main frame
	void DrawMainFrame();
	
	// compute next state (high frequency call)
	void HighFreqUpdate(float fDelta, const CVector& globalForces, const CVector& unitGravityForce, const TimeInfo& time);
	
	// put together clump and xml coming from maya
	void Link();
	
	// compute next frame
	//virtual void Update(const TimeInfo& time);
	virtual void Update(float fTimeStep);
	
	// get wind
	CVector GetWind();
		
	// constructor:
	// construct an instance of a chain / hair
	// sharing style information from maya (dean xml) and welder (for global control)
	// this is constructed by a binding function in lua
	OneChain(CEntity* pSelf,
		const ntstd::String& collisionName,
		const HairStyleFromWelder* pDefWelder,
		const HairStyleFromMaya* pDefMaya);
	
	// destructor
	virtual ~OneChain();
	
	void Register();
	void UnRegister();
	
	ChainAnimation2Render* GetIronChain();
	bool HasIronChain();
private:
	static int GetHierarchySize(CHierarchy* pH);
}; // end of class OneChain



#endif // end of _CHAINCORE_H_
