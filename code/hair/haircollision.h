#ifndef _HAIRCOLLISION_H_
#define _HAIRCOLLISION_H_

#include "hair/effectchain.h"
#include "core/explicittemplate.h"
#include "hair/chaindef.h"
#include "game/anonymouscomponent.h"
#include "tbd/cvectorwithstl.h"

#define HAIR_COLLISION_EPSILON 0.00001f

///////////////////////////////////////////////////
// Group pre-declaration
class HairSphereDef;
class CHierarchy;
class ChainElem;
class Transform;
class HairCollisionSphere;
class ClothColprim;
class Hero;
// End of Group pre-declaration
///////////////////////////////////////////////////



class HierarchyPriorityList
{
public:
	typedef ntstd::List<const CHierarchy*> List;
	List m_list;
public:
	//! constructor
	HierarchyPriorityList() {}

	//! constructor
	HierarchyPriorityList(const CHierarchy* pHierarchy)
	{
		m_list.push_back(pHierarchy);
	}
	
	//! constructor
	HierarchyPriorityList(const CHierarchy* pHierarchy1, const CHierarchy* pHierarchy2)
	{
		m_list.push_back(pHierarchy1);
		m_list.push_back(pHierarchy2);
	}
	
	// get transform
	const Transform* GetTransform(const CHashedString& name) const;
}; // end of class HierarchyPriorityList


class ClothColprimInstance;





class Shrinker
{
public:
	Shrinker(){};
	virtual ~Shrinker() {};
	virtual float GetShrinkCoef(const ClothColprimInstance& cci) const = 0;
	virtual void Update() = 0;
	virtual void DrawDebug() {};
}; // end of class Shrinker

//--------------------------------------------------
//!
//!	this class make the spheres smaller when the joints
//!	of the arm are bending
//!	
//--------------------------------------------------
class CollisionSword;
class SleeveStuff: public Shrinker
{
public:
	// 0 -> big sphere
	float m_fShrinkerBend;
	float m_fShrinkerChest;
	float m_fShrinkerWeapon;
	
	CDirection m_yArmRest;
	// transfrom pointer
	const Transform* m_pArm;
	const Transform* m_pElbow;
	const CollisionSword* m_pSword;
public:
	SleeveStuff(const CHierarchy* pHero, const CollisionSword* pSword);
	virtual void Update();
	virtual float GetShrinkCoef(const ClothColprimInstance& cci) const;
	virtual void DrawDebug();
}; // end of class SleeveStuff


//--------------------------------------------------
//!
//!	one instance of a clothprim
//!
//--------------------------------------------------


class ClothColprimInstance
{
public:
	Array<const Transform*,2> m_transforms;
	const ClothColprim* m_pDef;
	
	CMatrix m_world2Sphere;
	CMatrix m_sphere2Word;
public:
	//! constructor
	ClothColprimInstance(const ClothColprim* pDef, const Transform* pTransform1, const Transform* pTransform2);
	void Update(float fShrinkCoef, float fOffset);
	void DrawDebug(float fRadiusCoef) const;
	CMatrix GetWorldMatrix1() const;
	CMatrix GetWorldMatrix2() const;
}; // end of class ClothColprimInstance



//--------------------------------------------------
//!
//!	a colprim set
//!
//--------------------------------------------------

class ClothColprimInstanceSet
{
public:
	// I like comment
	typedef ntstd::Vector<ClothColprimInstance*, Mem::MC_GFX> Vector;

	// list of collision sphere; 
	Vector m_vector;

	// sleevestuff
	CScopedPtr<Shrinker> m_pShrinker;
	
	// def
	const HairStyleFromMaya* m_pMaya;
	const HairStyleFromWelder* m_pWelder;
public:
	//! constructor
	ClothColprimInstanceSet(const HierarchyPriorityList& hierarchyList ,
		const HairStyleFromMaya* pMaya, const HairStyleFromWelder* pWelder);
	~ClothColprimInstanceSet();
	int GetSize() const {return m_vector.size();}
	void DrawDebug(float fRadiusCoef);
	void Update();
}; // end of class ClothColprimInstance






















//--------------------------------------------------
//!
//!	one collision information.
//!	This class store the result between the collision between 1 sphere and 1 joint
//!	When the collision are entirely computed for one joint, all the related Collision data
//! are "sumed" into the class CorrectedPosition
//!
//--------------------------------------------------

class CollisionData
{
public:
	// pointer to collision sphere
	HairCollisionSphere* m_pt;
	
	// potential, defining a layer around the ellipsoid
	float m_fPotential;
	
	// normal pointing to the exterior of the collision ellipsoid
	CVectorContainer m_worldNorm;
	
	// negative means that the joint is in the sphere
	float m_fRefLengthSquare;
public:
	//! constructor
	CollisionData(HairCollisionSphere* pt, const CVector& worldNorm, float fRefLengthSquare, float fPotential);
	
	void DrawDebug(const CVector& correctedPoint, const HairStyleFromWelder* pDefWelder);
}; // end of class CollisionData











//--------------------------------------------------
//!
//!	final result of all the collision applying on one joint.
//!	used by ChainElem to compute the collision forces and the collision correction
//!
//--------------------------------------------------

class CorrectedPosition
{
public:
	// cumulation of all the world difference
	CVector m_worldDiff;
	
	// cumulation of all the world difference
	CVector m_worldNorm;

	// out coeficient
	float m_fPotential;

	// out coeficient
	float m_fRefLengthSquare;
	
	// out coeficient
	float m_fOutCoef;
	
	// list of colliding sphere
	typedef ntstd::List<CollisionData, Mem::MC_GFX> List;
	List m_list;
public:
	
	//! constructor
	CorrectedPosition();
	
	// reset count
	void Reset();
	
	// reset count
	void PreDetection();
	
	// get value
	void PostDetection(const HairStyleFromWelder* pDefWelder, float fPotentialExpulsion);
	
	// get value
	CVector GetWordDiff() {return m_worldDiff;}
		
	// is colliding
	inline bool IsColliding() const
	{
		return m_list.size()>0;
	}
	
	inline bool IsOutside() const
	{
		return m_fPotential<1.0f;
	}

	inline bool IsCollidingAndInside() const
	{
		return IsColliding() && !IsOutside();
	}
	
	static float Potential2Distance(float fPotential, float fPotentialEnd);
	static float Distance2Potential(float fNormDistance, float fPotentialEnd);
	static float Potential2Decalage(float fPotential, float fPotentialEnd, float fRefLength);
	
}; // end of class CorrectedPosition















//--------------------------------------------------
//!
//!	heuristic to decresase the number of collisions.
//!	it is stored in each chainElem (ie. joint).
//! this class store how far is a joint and only test a joint if it's close enough
//! every frame, all the distance are artificially decrease by a magic value.
//!
//--------------------------------------------------

class HairCollisionHeuristic
{
public:
	class HeuristicData
	{
	public:
		float m_fFar;
		//! constructor
		HeuristicData():
			m_fFar(-1.0f)
		{
			// nothing
		}
		
		void Set(float fFar) {m_fFar = fFar;}
		void Update(float fDecrease) {m_fFar -= fDecrease;}
		bool TestMe() {return m_fFar < 0.0f;}
	}; // end of class HeuristicData
	
	typedef ntstd::Vector<HeuristicData, Mem::MC_GFX> Vector;
	Vector m_vector;
	
	//! constructor
	HairCollisionHeuristic();

	
	void Update(float fDecrease);
	
	//
	void Reset(int iSize);
	
}; // end of class HairCollisionHeuristic













//--------------------------------------------------
//!
//!	one collision sphere attached to a transform
//! and with a def coming from maya
//!
//--------------------------------------------------

class HairCollisionSphere
{
public:
	// constructor
	HairCollisionSphere(const HairSphereDef* pDef, const CHierarchy* pHierarchy);
	HairCollisionSphere(const HairSphereDef* pDef, const Transform* pTransform);

	// constructor
	HairCollisionSphere();
public:
	// enum for bitmask
	typedef enum
	{
		F_DONOTUPDATE = BITFLAG(0),
		F_DONOTDELETE = BITFLAG(1),
	} State;

	// am I active?
	BitMask<State> m_mask;
	
	// update position in simspace
	void UpdatePosition();

	// set hierachy to which it is related using name in def
	void SetTransform(const CHierarchy* pHierarchy);

	// set hierachy to which it is related using name in def
	const Transform* GetTransform()
	{
		return m_pTransform;
	}

	// do not use ! (only in extreme cases)
	void ForceTransform(const Transform* pTransfrom);

	// draw the psheres
	void DrawDebug() const;

	
	// detect a collision between a sphere and the joint
	float CollisionDetection(ChainElem& elem, int iRotIndex, float fRadiusCoef, float fRealJointRadius, float fPotentialEnd);

	// get world position of the center
	inline CVector GetWordPosition() const
	{
		return m_sphere2Word[3];
	}
	
	const HairSphereDef* GetDef() const {return m_pDef;}
	
	// create a sphere not coming from maya
	static HairCollisionSphere* CreateWithDef(const Transform* pTransform);
	
	// set matrices
	void SetMatrices(const CDirection& scale, const CMatrix& local2sphere);
	
	CMatrix GetWorld2Sphere() const {return m_world2Sphere;}
protected:
	// collision influence
	float CollisionInfluence(float fDist);
	
	// sphere definition
	const HairSphereDef* m_pDef;

	// world position
	CMatrix m_world2Sphere;

	// world position
	CMatrix m_world2SphereTranspose;
	
	// world position
	CMatrix m_sphere2Word;
	
	// transfrom  whose pshere is linked to
	const Transform* m_pTransform;
	
	// transform index
	int m_iTransformIndex;
	
};






//--------------------------------------------------
//!
//!	procedfural sphere to deal with the collision with the sword
//!
//--------------------------------------------------

class CollisionSword: public CAnonymousEntComponent
{
public:
	//CVector m_scale;
	//CVector m_translate;
	float m_fCommonCoef;
	HairSphereDef m_def;
	const SwordCollisionDef* m_pSwordDef;
	Transform* m_pTransform;
	CScopedPtr<HairCollisionSphere>  m_pSphere;
	CEntity* m_sword;
	bool m_bSwordIn;
	ValueAndMax<float> m_lerp;
public:
	void Add();
	void Remove();
	virtual void Update( float fTimeStep );
	~CollisionSword();
	CollisionSword(const ntstd::String& name, const SwordCollisionDef* pSwordDef, const Hero* pHero);
	void CreateSphere();
	HairCollisionSphere* GetSphere();
	void Init();

}; // end of class CollisionSword







//--------------------------------------------------
//!
//!	procedfural sphere to deal with the collision with the sword
//!
//--------------------------------------------------

class CollisionFloor: public CAnonymousEntComponent
{
public:
	const Transform* m_pTransform;
	const CEntity* m_pHero;
	HairSphereDef m_def;
	CScopedPtr<HairCollisionSphere> m_pSphere;
	const FloorCollisionDef* m_pSwordDef;
	bool m_bOnTheFloor;
	ValueAndMax<float> m_progress;
public:
	float GetProgress() {return m_progress.m_value/m_progress.m_max;}
	virtual void Update( float fTimeStep );
	~CollisionFloor();
	CollisionFloor(const ntstd::String& name, const CEntity* pHero, 
		const FloorCollisionDef* pDef, const Transform* pTransform);
	void CreateSphere();
	HairCollisionSphere* GetSphere();
	void Init();
}; // end of class CollisionSword












//--------------------------------------------------
//!
//!	a set of collision sphere attached to the same hierarchy.
//!
//--------------------------------------------------

class CollisionSphereSet: public CAnonymousEntComponent
{
private:
	// I like comment
	typedef ntstd::Vector<HairCollisionSphere*, Mem::MC_GFX> Vector;
	
	// list of collision sphere; 
	Vector m_vector;
	
	// collision def
	const HairSphereSetDef* m_pDef;
	
	// collision def
	const CHierarchy* m_pHierarchy;
	
	// collision def
	const CEntity* m_pHero;
public:
	// ctor
	CollisionSphereSet(const ntstd::String& name,
		const HairSphereSetDef* pCollisionDef, const CEntity* pHero);

	// ctor
	~CollisionSphereSet();
	
	// reset
	void Reset(const HairSphereSetDef* pCollisionDef);
	
	// sword
	void AddSpecialSphere(HairCollisionSphere* pSphere);	
	
	// get def
	const HairSphereSetDef* GetDef() const {return m_pDef;}
	
	// draw collision sphere
	void DrawDebugSpheres() const;

	// update sphere transform
	virtual void Update( float fTimeStep );
	
	// modify forces
	void CollisionDetection(ChainElem& elem, int iRotIndex,
		const HairStyleFromWelder* pDefWelder, float fPotentialExpulsion);
	
	// get number of sphere
	int GetSize() const;
	const HairCollisionSphere* GetSphere(int iSize) const 
	{
		ntAssert(iSize>=0 && iSize<GetSize());
		return m_vector[iSize];
	}

protected:
	// free all the elem of m_list
	void DeleteList();
};



#endif // end of _HAIRCOLLISION_H_
