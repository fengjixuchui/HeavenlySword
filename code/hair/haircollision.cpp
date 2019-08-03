#include "hair/haircollision.h"

#include "core/gatso.h"
#include "core/boostarray.inl"

#include "hair/chaincore.h"
#include "tbd/franktmp.h"

#include "gfx/renderer.h"
#include "gfx/graphicsdevice.h"

#include "anim/hierarchy.h"
#include "gfx/simplefunction.h"
#include "objectdatabase/dataobject.h"

#include "game/query.h"
#include "game/attacks.h"
#include "game/entityhero.h"
#include "physics/world.h"
#include "core/visualdebugger.h"

#include "../code_ps3/spu/hair/hair_animator_spu.h"

using namespace SimpleFunction;

#ifdef PLATFORM_PC
#define isnan _isnan
#endif // PLATFORM_PC

// get transform
const Transform* HierarchyPriorityList::GetTransform(const CHashedString& name) const
{
	for(List::const_iterator it = m_list.begin();
		it != m_list.end();
		it++)
	{
		int iIndex = (*it)->GetTransformIndex(name);
		if(iIndex>=0)
		{
			return (*it)->GetTransform(iIndex);
		}
	}
	
	return static_cast<Transform*>(0);
}

float Shrinker::GetShrinkCoef(const ClothColprimInstance& cci) const
{
	UNUSED(cci);
	return 0.0f;
}




SleeveStuff::SleeveStuff(const CHierarchy* pHero, const CollisionSword* pSword)
	:m_pArm(0)
	,m_pElbow(0)
	,m_pSword(pSword)
{
	int iArmIndex = pHero->GetTransformIndex(CHashedString("l_arm"));
	//CMatrix armMatrix(pHero->GetClumpHeader()->m_pobBindPoseArray[iArmIndex].m_obRotation);
	//m_yArmRest = armMatrix.GetYAxis();
	m_yArmRest = pHero->GetTransform(iArmIndex)->GetWorldMatrix().GetYAxis();
	m_pArm = pHero->GetTransform(iArmIndex);
	
	
	int iElbowIndex = pHero->GetTransformIndex(CHashedString("l_elbow"));
	m_pElbow = pHero->GetTransform(iElbowIndex);
	ntAssert(m_pArm && m_pElbow);
	Update();
}
void SleeveStuff::Update()
{
	m_fShrinkerBend = 0.5f * (1.0f - m_pArm->GetWorldMatrix().GetYAxis().Dot(m_pElbow->GetWorldMatrix().GetYAxis()));
	
	float fDotChest = m_pArm->GetWorldMatrix().GetYAxis().Dot(m_yArmRest);
	m_fShrinkerChest = LinearStep(0.7f,1.0f,fDotChest);
	
	if(m_pSword)
	{
		CPoint p = m_pElbow->GetWorldMatrix().GetTranslation()
			- m_pSword->m_pTransform->GetWorldMatrix().GetTranslation();
		float fLenght = p.Length();
		m_fShrinkerWeapon = LinearStep(0.2f,0.1f,fLenght);
	}
	else
	{
		m_fShrinkerWeapon = 0.0f;
	}
}
void SleeveStuff::DrawDebug()
{
#ifndef _GOLD_MASTER
	CPoint decalage(0.0f,1.0f,0.0f);
	decalage += m_pArm->GetWorldMatrix().GetTranslation();
	g_VisualDebug->RenderLine(
		decalage,
		decalage+m_pArm->GetWorldMatrix().GetYAxis(),
		DC_RED );
	g_VisualDebug->RenderLine(
		decalage,
		decalage+m_yArmRest,
		DC_BLUE );
#endif
}

float SleeveStuff::GetShrinkCoef(const ClothColprimInstance& cci) const
{
	float fRes = 0.0f;
	if(cci.m_pDef->m_mask.AllOfThem(ClothColprim::F_CHESTSHRINK))
	{
		fRes+=cci.m_pDef->m_fShrinkerChest * m_fShrinkerChest;
	}
	if(cci.m_pDef->m_mask.AllOfThem(ClothColprim::F_SPEEDSHRINK))
	{
		fRes+=cci.m_pDef->m_fShrinkerBend * m_fShrinkerBend;
	}
	if(cci.m_pDef->m_mask.AllOfThem(ClothColprim::F_WEAPONSHRINK))
	{
		fRes+=cci.m_pDef->m_fShrinkerWeapon * m_fShrinkerWeapon;
	}

	return fRes;
}

//! constructor
ClothColprimInstance::ClothColprimInstance(const ClothColprim* pDef,
	const Transform* pTransform1, const Transform* pTransform2)
		:m_transforms(pTransform1,pTransform2)
		,m_pDef(pDef)
{
	ntAssert(pTransform1);
	// just in case...
	Update(0.0f,0.0f);
}
CMatrix ClothColprimInstance::GetWorldMatrix1() const
{
	const CMatrix& local2World1 = m_transforms[0]->GetWorldMatrix();
	return m_pDef->GetInfluence1().m_sphere2Local * local2World1;
}
CMatrix ClothColprimInstance::GetWorldMatrix2() const
{
	const CMatrix& local2World2 = m_transforms[1]->GetWorldMatrix();
	return m_pDef->GetInfluence2().m_sphere2Local * local2World2;
}

void ClothColprimInstance::Update(float fShrinkCoef, float fOffset)
{
	// m_pDef->m_fShrinkCoef should be m_fShrinkInfluence
	float fCoef = clamp(1.0f - fShrinkCoef, 0.01f ,1.0f);
	CDirection scale = fCoef * CDirection(m_pDef->m_scale);
	CMatrix sphere2Word_noscale;
	CDirection decalage(scale.X()+fOffset,0.0f,0.0f);
	
	if(m_transforms[1])
	{
		CMatrix sphere2Word_noscale_1 = GetWorldMatrix1();
		CMatrix sphere2Word_noscale_2 = GetWorldMatrix2();
		sphere2Word_noscale = CMatrix::Lerp(sphere2Word_noscale_1, sphere2Word_noscale_2,m_pDef->GetInfluence2().m_fWeigth);
		
		// RENORM HERE
		// ... should I, seems to work whitout...
		// RENORM HERE
	}
	else
	{
		sphere2Word_noscale = GetWorldMatrix1();
	}
	
	CPoint newTranslation = sphere2Word_noscale.GetTranslation() + decalage * sphere2Word_noscale;
	sphere2Word_noscale.SetTranslation(newTranslation);
	m_world2Sphere = sphere2Word_noscale.GetAffineInverse() * FrankMisc::CreateInvScaleMatrix(scale);
	m_sphere2Word = FrankMisc::CreateScaleMatrix(scale) * sphere2Word_noscale;
}


void ClothColprimInstance::DrawDebug(float fRadiusCoef) const
{
#ifndef _GOLD_MASTER
	Renderer::Get().SetBlendMode(GFX_BLENDMODE_LERP);
	//GetD3DDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	//GetD3DDevice()->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	//GetD3DDevice()->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
	
	uint32_t color = DebugColour::cyan(150,150);
	if(!m_pDef->m_bJustOne)
	{
		color = DebugColour::purple(150,150);
	}
	CMatrix scale = FrankMisc::CreateScaleMatrix(fRadiusCoef,fRadiusCoef,fRadiusCoef);
	g_VisualDebug->RenderSphere( scale*m_sphere2Word, color);

	Renderer::Get().SetBlendMode(GFX_BLENDMODE_DISABLED);
	
	CMatrix m1 = GetWorldMatrix1();
	g_VisualDebug->RenderAxis(m1,0.05f);
	if(m_transforms[1])
	{
		CMatrix m2 = GetWorldMatrix2();
		g_VisualDebug->RenderAxis(m2,0.05f);
		g_VisualDebug->RenderLine(m1.GetTranslation(), m2.GetTranslation(), DC_BLACK );
	}
#endif
}



ClothColprimInstanceSet::ClothColprimInstanceSet(const HierarchyPriorityList& hierarchyList ,
	const HairStyleFromMaya* pMaya, const HairStyleFromWelder* pWelder)
		:m_pMaya(pMaya)
		,m_pWelder(pWelder)
{
	const HairStyleFromMaya::ClothList& list = m_pMaya->m_clothList;
	m_vector.reserve(list.size());
	for(HairStyleFromMaya::ClothList::const_iterator it = list.begin();
		it != list.end();
		it++)
	{
		const ClothColprim* pDef = *it;
		CHashedString hs = CHashedString(pDef->GetInfluence1().m_relativeName.c_str());
		const Transform* pTransform1 = hierarchyList.GetTransform(hs);
		ntAssert(pTransform1);
		const Transform* pTransform2 = 0;
		if(!pDef->m_bJustOne)
		{
			CHashedString hs = CHashedString(pDef->GetInfluence2().m_relativeName.c_str());
			pTransform2 = hierarchyList.GetTransform(hs);
			ntAssert(pTransform2);
		}
		m_vector.push_back(NT_NEW ClothColprimInstance(pDef,pTransform1,pTransform2));
	}
}

ClothColprimInstanceSet::~ClothColprimInstanceSet()
{
	for(u_int iIndex = 0 ; iIndex < m_vector.size() ; iIndex++ )
	{
		NT_DELETE( m_vector[iIndex] );
	}
}

void ClothColprimInstanceSet::DrawDebug(float fRadiusCoef)
{
	Pixel2 range(0,m_vector.size());
	if(ChainRessource::Get().GetGlobalDef().m_bUseDebugRange)
	{
		range = ChainRessource::Get().GetGlobalDef().GetDebugRangeWithin(range);
	}
	for(int iSphere = range[0] ; iSphere < range[1] ; iSphere++ )
	{
		m_vector[iSphere]->DrawDebug(fRadiusCoef);
	}
	if(m_pShrinker)
	{
		m_pShrinker->DrawDebug();
	}
}


void ClothColprimInstanceSet::Update()
{
	float fOffset = m_pWelder->m_fClothOffset;
	if(m_pWelder->m_bUseShrinker && m_pShrinker)
	{
		m_pShrinker->Update();
		for(u_int iIndex = 0 ; iIndex < m_vector.size() ; iIndex++ )
		{
			const ClothColprimInstance& cci = *m_vector[iIndex];
			float ShrinkCoef  = m_pShrinker->GetShrinkCoef(cci);
			m_vector[iIndex]->Update(ShrinkCoef,fOffset);
		}		
	}
	else
	{
		for(u_int iIndex = 0 ; iIndex < m_vector.size() ; iIndex++ )
		{
			m_vector[iIndex]->Update(0.0f,fOffset);
		}
	}
}


//! constructor
CollisionData::CollisionData(HairCollisionSphere* pt, const CVector& worldNorm, float fRefLengthSquare, float fPotential)
	:m_pt(pt)
	,m_fPotential(fPotential)
	,m_worldNorm(worldNorm)
	,m_fRefLengthSquare(fRefLengthSquare)
{
	// nothing
}


void CollisionData::DrawDebug(const CVector& correctedPoint, const HairStyleFromWelder* pDefWelder)
{	
#ifndef _GOLD_MASTER
	float fDistance = CorrectedPosition::Potential2Decalage(m_fPotential,pDefWelder->m_fPotentialEnd,sqrt(m_fRefLengthSquare));
	CVector diff = fDistance * m_worldNorm.ToVector();
	
	g_VisualDebug->RenderSphere(CQuat(CONSTRUCT_IDENTITY), CPoint(correctedPoint+diff),
		ChainRessource::Get().GetGlobalDef().m_fSphereSize, DC_RED );
	g_VisualDebug->RenderLine(CPoint(correctedPoint), CPoint(correctedPoint+diff), 
		DC_RED );
#endif
}










float CorrectedPosition::Potential2Decalage(float fPotential, float fPotentialEnd, float fRefLength)
{
	float fNormDist = Potential2Distance(fPotential, fPotentialEnd);
	if(fNormDist < 1.0f)
	{
		return fRefLength * (1.0f - fNormDist);
	}
	else
	{
		return 0.0f;
	}
}



float CorrectedPosition::Potential2Distance(float fPotential, float fPotentialEnd)
{
	float fDist = fPotentialEnd - sqrt(fPotential) * (fPotentialEnd - 1.0f);
	return fDist;
}

float CorrectedPosition::Distance2Potential(float fNormDistance, float fPotentialEnd)
{
		// potential
	float fPotentialAux = (fPotentialEnd - fNormDistance) / (fPotentialEnd - 1.0f);
	return fPotentialAux*fPotentialAux;
}








//! constructor
CorrectedPosition::CorrectedPosition()
	:m_worldDiff(CONSTRUCT_CLEAR)
	,m_worldNorm(CONSTRUCT_IDENTITY)
	,m_fPotential(0.0f)
	,m_fRefLengthSquare(1.0f)
	,m_fOutCoef(1.0f)
{
	Reset();
}

// reset count
void CorrectedPosition::PreDetection()
{
	m_list.clear();
}

	
// get value
void CorrectedPosition::PostDetection(const HairStyleFromWelder* pDefWelder, float fPotentialExpulsion)
{
	// no influence, keep this empty and return now
	if(m_list.size()==0)
	{
		m_fOutCoef= -1.0f;
		return;
	}
	
	float fTotalFakePotential = 0.0f;
	m_fPotential = 0.0f;
	m_fRefLengthSquare = 0.0f;
	m_worldNorm = CVector(CONSTRUCT_CLEAR);
	for(List::iterator it = m_list.begin() ; it != m_list.end()  ; it++ )
	{
		float fFakePotential =  ntstd::Max(HAIR_COLLISION_EPSILON,it->m_fPotential);
		fTotalFakePotential += fFakePotential;
		m_fPotential += it->m_fPotential;
		m_worldNorm += fFakePotential * it->m_worldNorm.ToVector();
		m_fRefLengthSquare += fFakePotential * it->m_fRefLengthSquare;
		FRANKHAIRPRINT_FLOAT("m_fPotential[]",it->m_fPotential);
		FRANKHAIRPRINT_FLOAT("m_fRefLengthSquare[]",it->m_fRefLengthSquare);
		FRANKHAIRPRINT_FLOAT3("m_worldNorm[]",it->m_worldNorm);
	}

	// normalise normal
	m_worldNorm /= m_worldNorm.Length();
	m_fRefLengthSquare /= fTotalFakePotential;
	
	float fRefLength = sqrt(m_fRefLengthSquare);
	fRefLength *= fPotentialExpulsion;
	
	// compute dist
	float fDist = CorrectedPosition::Potential2Decalage(m_fPotential,pDefWelder->m_fPotentialEnd,fRefLength);
	
	// compute world diff from old to new
	m_worldDiff = m_worldNorm * fDist;
	
	// outcoef, when do the layer stop
	//m_fOutCoef = Potential2OutCoef(collisionForceRange,m_fPotential);
	m_fOutCoef = LinearStep(pDefWelder->m_collisionForceRange[0],pDefWelder->m_collisionForceRange[1],m_fPotential);

	FRANKHAIRPRINT_FLOAT("m_fPotential",m_fPotential);
	FRANKHAIRPRINT_FLOAT("m_fRefLengthSquare",m_fRefLengthSquare);
	FRANKHAIRPRINT_FLOAT("m_fOutCoef",m_fOutCoef);
	FRANKHAIRPRINT_FLOAT3("m_worldDiff",m_worldDiff);
}




// reset count
void CorrectedPosition::Reset()
{
	PreDetection();
}














HairCollisionSphere::HairCollisionSphere(const HairSphereDef* pDef, const Transform* pTransform)
	:m_pDef(pDef)
	,m_pTransform(pTransform)
	,m_iTransformIndex(-1)
{
	// nothing
}

HairCollisionSphere::HairCollisionSphere(const HairSphereDef* pDef, const CHierarchy* pHierarchy)
	:m_pDef(pDef)
{
	SetTransform(pHierarchy);
}

HairCollisionSphere::HairCollisionSphere()
	:m_pDef()
	,m_world2Sphere(CONSTRUCT_CLEAR)
	,m_world2SphereTranspose(CONSTRUCT_CLEAR)
	,m_sphere2Word(CONSTRUCT_CLEAR)
	,m_pTransform(0)
	,m_iTransformIndex(0)
{
	// nothing
}

void HairCollisionSphere::UpdatePosition()
{
	if(m_mask.AllOfThem(F_DONOTUPDATE))
	{
		return;
	}
	// the collision computation needs data in the simulation reference frame
	// this data needs to be converted into the unit sphere reference frame
	
	const CMatrix& local2World = m_pTransform->GetWorldMatrix();
	m_world2Sphere = local2World.GetAffineInverse() * m_pDef->m_local2Sphere;
	m_sphere2Word = m_pDef->m_sphere2Local * local2World;
	m_world2SphereTranspose = m_world2Sphere.GetTranspose();
}

void HairCollisionSphere::SetTransform(const CHierarchy* pHierarchy)
{
	CHashedString name = CHashedString(m_pDef->m_parent.c_str());
	m_iTransformIndex = pHierarchy->GetTransformIndex(name);
	ntAssert(m_iTransformIndex!=-1);
	m_pTransform = pHierarchy->GetTransform(m_iTransformIndex);
}

// do not use ! (only in extreme cases)
void HairCollisionSphere::ForceTransform(const Transform* pTransfrom)
{
	m_pTransform = pTransfrom;
}

void HairCollisionSphere::DrawDebug() const
{
#ifndef _GOLD_MASTER
	Renderer::Get().SetBlendMode(GFX_BLENDMODE_LERP);

	uint8_t iColor = (uint8_t)(m_iTransformIndex%4)*255/4;
	g_VisualDebug->RenderSphere( m_sphere2Word, DebugColour::white(iColor,150));
	
	Renderer::Get().SetBlendMode(GFX_BLENDMODE_DISABLED);
#endif
}

float HairCollisionSphere::CollisionInfluence(float fDist)
{
	float fSym = abs(2.0f * fDist - 1.0f);
	float fTreshold = 0.8f;
	if(fSym<fTreshold)
	{
		return 1.0f;
	}
	else
	{
		return (1.0f - fSym) / (1.0f -  fTreshold);
	}
}
	
// detect a collision between a sphere and the joint
float HairCollisionSphere::CollisionDetection(ChainElem& elem, int iRotIndex, float fRadiusCoef, float fRealJointRadius, float fPotentialEnd)
{
	//int i = elem.GetMayaDef()->m_name == ntstd::String("hair_testcol_joint0")?1:0;i;
	
	// goto unit sphere to detect collision
	const ChainElem::Dynamic& din = elem.GetDynamic()[iRotIndex];
	CVector spherePos = din.m_worldExtremity * m_world2Sphere;
	spherePos.W() = 0.0f;
	FRANKHAIRPRINT_FLOAT3("spherePos",spherePos);
	
	// very basic approximation of how to make artificially the ellispoid bigger
	// when the radisu of the hair is bigger
	float fJointRadiusRatio = fRealJointRadius / m_pDef->GetAverageRadius();
	fRadiusCoef += fJointRadiusRatio;
	
	// distance between the center of the elipsoid and the joint (center of its sphere)
	float fSqCenter2Center = spherePos.LengthSquared() / (fRadiusCoef * fRadiusCoef);
	FRANKHAIRPRINT_FLOAT("fSqCenter2Center",fSqCenter2Center);
	
	// normalised distance treshold
	float fSqCenter2CenterTreshold = fPotentialEnd * fPotentialEnd;
	FRANKHAIRPRINT_FLOAT("fSqCenter2CenterTreshold",fSqCenter2CenterTreshold);
	
	// return if not close enough
	if(fSqCenter2Center > fSqCenter2CenterTreshold)
	{
		return fSqCenter2Center;
	}	
		
	// length between ellisoid center and joint
	float fspherePosLen = sqrt(fSqCenter2Center);
	FRANKHAIRPRINT_FLOAT("fspherePosLen",fspherePosLen);
	
	// potential
	float fPotential = CorrectedPosition::Distance2Potential(fspherePosLen,fPotentialEnd);
	FRANKHAIRPRINT_FLOAT("fPotential",fPotential);
	
	// position of the point in the surface on the small sphere
	CVector sphereNormal = spherePos / fspherePosLen;
	FRANKHAIRPRINT_FLOAT3("sphereNormal",sphereNormal);
	
	
	// prepare coefficient for distance ref
	CVector sphereNormalAux = sphereNormal * sphereNormal;
	FRANKHAIRPRINT_FLOAT3("sphereNormalAux",sphereNormalAux);

	// reference distance
	float fDistanceAux = sphereNormalAux.Dot(m_pDef->GetScale()) * fRadiusCoef;
	float fSqRefAux = fDistanceAux*fDistanceAux;
	FRANKHAIRPRINT_FLOAT("fSqRefAux",fSqRefAux);
	
	// compute normal
	CVector normalisedPlane  = sphereNormal;
	normalisedPlane.W() = - fRadiusCoef;
	CVector worldNormal  = normalisedPlane * m_world2SphereTranspose;
	worldNormal.W() = 0.0f;
	worldNormal /= worldNormal.Length();
	FRANKHAIRPRINT_FLOAT3("worldNormal",worldNormal);

	// insert collision data object
	if(!isnan(fSqRefAux))
	{
		elem.m_correctedPosition.m_list.push_back(CollisionData(this,worldNormal,fSqRefAux,fPotential));
	}
	
	// return heuristic
	return fSqCenter2Center; 
}

// set matrices
void HairCollisionSphere::SetMatrices(const CDirection& scale, const CMatrix& local2world)
{
	m_sphere2Word = FrankMisc::CreateScaleMatrix(scale) * local2world;
	m_world2Sphere = local2world.GetAffineInverse() * FrankMisc::CreateInvScaleMatrix(scale);
	m_world2SphereTranspose = m_world2Sphere.GetTranspose();
}



















void CollisionFloor::Update( float fTimeStep )
{
	UNUSED(fTimeStep);
	
	const CAttackComponent* pAttack = m_pHero->GetAttackComponent();
	switch(pAttack ? pAttack->AI_Access_GetState() : CS_STANDARD)
	{
	case CS_DEAD:
	case CS_FLOORED:
	case CS_RISE_WAIT:
		{
			m_progress.m_value += fTimeStep;
			m_bOnTheFloor = true;
			break;
		}
	default:
		{
			m_progress.m_value = 0.0f;
			m_bOnTheFloor = false;
			break;
		}
	}

	if(m_pSphere)
	{
		CDirection scale(2.0f,0.1f,2.0f);
		CPoint objectWorldTranslate = m_pTransform->GetWorldTranslation();
		CMatrix finalWorldMatrix(CONSTRUCT_IDENTITY);
		CPoint finalWorldTranslate;


		Physics::RaycastCollisionFlag flag;
		flag.base = 0;

		// [Mus] - What settings for this cast ?
		flag.flags.i_am = Physics::STATIC_ENVIRONMENT_BIT;
		flag.flags.i_collide_with = ( Physics::LARGE_INTERACTABLE_BIT );

		float fHitFraction;
		CDirection	hitNormal;
		CPoint srcPos = objectWorldTranslate;
		CDirection dir(0.0f,-10.0f,0.0f);
		
		if ( Physics::CPhysicsWorld::Exists() && Physics::CPhysicsWorld::Get().GetClosestIntersectingSurfaceDetails( srcPos, srcPos+dir, fHitFraction, hitNormal, flag ))
		{
			finalWorldTranslate = srcPos + dir*fHitFraction;
		}
		else
		{
			finalWorldTranslate = CPoint(objectWorldTranslate.X(),0.0f,objectWorldTranslate.Z());
		}
		finalWorldTranslate += CPoint(m_pSwordDef->m_translate);
		finalWorldMatrix.SetTranslation(finalWorldTranslate);
		m_pSphere->SetMatrices(CDirection(m_pSwordDef->m_scale),finalWorldMatrix);
	}
}


CollisionFloor::~CollisionFloor()
{
	// nothing
}


HairCollisionSphere* CollisionFloor::GetSphere()
{
	if(!m_pSphere)
	{
		CreateSphere();
	}
	return m_pSphere.Get();
}

CollisionFloor::CollisionFloor(const ntstd::String& name, const CEntity* pHero, const FloorCollisionDef* pDef, const Transform* pTransform)
	:CAnonymousEntComponent(name)
	,m_pTransform(pTransform)
	,m_pHero(pHero)
	,m_pSphere(0)
	,m_pSwordDef(pDef)
	,m_bOnTheFloor(true)
	,m_progress(3.0f)
{
	Init();
}

void CollisionFloor::Init()
{
	// nothing
}


// the sphere is not own by Collision sword
void CollisionFloor::CreateSphere()
{
	m_def = HairSphereDef(CVector(CONSTRUCT_CLEAR),CVector(1.0f,1.0f,1.0f,1.0f),CQuat(CONSTRUCT_IDENTITY));
	m_pSphere.Reset( NT_NEW HairCollisionSphere(&m_def,m_pTransform));
	m_pSphere->m_mask.Set(HairCollisionSphere::F_DONOTUPDATE);
	m_pSphere->m_mask.Set(HairCollisionSphere::F_DONOTDELETE);
}



















void CollisionSword::Update( float fTimeStep )
{
	if(m_bSwordIn)
	{
		m_lerp.m_value+=fTimeStep;
	}
	else
	{
		m_lerp.m_value-=fTimeStep;
	}
	m_lerp.m_value = ntstd::Clamp(m_lerp.m_value,0.0f,m_lerp.m_max);
	m_fCommonCoef = m_lerp.m_value;
	
	if(m_pSphere)
	{
		m_def.SetScale( m_pSwordDef->GetScale(m_fCommonCoef) );
		m_def.SetTranslate( m_pSwordDef->GetTranslate(m_fCommonCoef));
		m_def.SetRotate( m_pSwordDef->GetRotate(m_fCommonCoef));
		m_def.Finalise();
	}
}


CollisionSword::CollisionSword(const ntstd::String& name, const SwordCollisionDef* pSwordDef,const Hero* pHero)
	:CAnonymousEntComponent(name)
	,m_pSwordDef(pSwordDef)
	,m_pSphere(0)
	,m_sword(0)
	,m_bSwordIn(true)
	,m_lerp(1.0f,1.0f)
{
	m_sword = pHero->GetLeftWeapon();
	
	ntAssert(m_sword);
	Init();
}

CollisionSword::~CollisionSword()
{
	// nothing
}


HairCollisionSphere* CollisionSword::GetSphere()
{
	if(!m_pSphere)
	{
		CreateSphere();
	}
	return m_pSphere.Get();
}

void CollisionSword::Init()
{
	m_pTransform = m_sword->GetHierarchy()->GetRootTransform();
}


// the sphere is not own by Collision sword
void CollisionSword::CreateSphere()
{
	m_def = HairSphereDef(
		m_pSwordDef->GetTranslate(1.0f),
		m_pSwordDef->GetScale(1.0f),
		m_pSwordDef->GetRotate(1.0f));
	m_pSphere.Reset(NT_NEW HairCollisionSphere(&m_def,m_pTransform));
	m_pSphere->m_mask.Set(HairCollisionSphere::F_DONOTDELETE);
}



void CollisionSword::Add()
{
	if(!m_bSwordIn)
	{
		m_bSwordIn=true;
	}
	
}
void CollisionSword::Remove()
{
	if(m_bSwordIn)
	{
		m_bSwordIn=false;
	}
}



// sword
void CollisionSphereSet::AddSpecialSphere(HairCollisionSphere* pSphere)
{
	m_vector.push_back(pSphere);
}


// ctor
CollisionSphereSet::CollisionSphereSet(const ntstd::String& name,const HairSphereSetDef* pCollisionDef, const CEntity* pHero)
	:CAnonymousEntComponent(name)
	,m_pDef(0)
	,m_pHierarchy(pHero->GetHierarchy())
	,m_pHero(pHero)
{
	if(pCollisionDef)
	{
		Reset(pCollisionDef);
		pCollisionDef->GetRegister().Register(this);
	}
}

void CollisionSphereSet::Reset(const HairSphereSetDef* pCollisionDef)
{
	ntAssert(pCollisionDef);
	m_pDef = pCollisionDef;
	m_vector.clear();
	m_vector.reserve(m_pDef->m_list.size());
	for(HairSphereSetDef::HairSphereDefList::const_iterator it = m_pDef->m_list.begin();
		it != m_pDef->m_list.end();
		it++)
	{
		m_vector.push_back(NT_NEW HairCollisionSphere((*it), m_pHierarchy));
	}	
}

// ctor
CollisionSphereSet::~CollisionSphereSet()
{
	DeleteList();
	
	// MONSTERS\Frank 13/04/2005 09:49:37
	// because of bad init/remove in the shell
	
	//if(m_pDef)
	//{
	//	m_pDef->GetRegister().Unregister(this);
	//}
}


void CollisionSphereSet::DeleteList()
{
	for(u_int iSphere = 0 ; iSphere < m_vector.size() ; iSphere++ )
	{
		if(!m_vector[iSphere]->m_mask.AllOfThem(HairCollisionSphere::F_DONOTDELETE))
		{
			NT_DELETE( m_vector[iSphere] );
		}
	}
	m_vector.clear();
}

// draw collision sphere
void CollisionSphereSet::DrawDebugSpheres() const
{
	Pixel2 range(0,m_vector.size());
	if(ChainRessource::Get().GetGlobalDef().m_bUseDebugRange)
	{
		range = ChainRessource::Get().GetGlobalDef().GetDebugRangeWithin(range);
	}
	for(int iSphere = range[0] ; iSphere < range[1] ; iSphere++ )
	{
		m_vector[iSphere]->DrawDebug();
	}
}


// update sphere transform
void CollisionSphereSet::Update( float fTimeStep )
{
	UNUSED(fTimeStep);
	for(u_int iSphere = 0 ; iSphere < m_vector.size() ; iSphere++ )
	{
		m_vector[iSphere]->UpdatePosition();
	}
}

// position correction	
void CollisionSphereSet::CollisionDetection(ChainElem& elem, int iRotIndex,
	const HairStyleFromWelder* pDefWelder, float fPotentialExpulsion)
{
	CGatso::Start("CollisionSphereSet::CollisionDetection");
	
	Vec2 collisionForceRange = pDefWelder->m_collisionForceRange;
	float fPotentialCoef = pDefWelder->m_fPotentialCoef;
	float fPotentialEnd = pDefWelder->m_fPotentialEnd;
	float fJointRadius = elem.GetMayaDef()->GetRadius(pDefWelder);
	elem.m_correctedPosition.PreDetection();
	
	if(pDefWelder->m_bUseCollisionHeuristic)
	{
		ntError( elem.m_heuritic.m_vector.size() >= m_vector.size() );

		for(u_int iSphere = 0 ; iSphere < m_vector.size() ; iSphere++ )
		{
			if(elem.m_heuritic.m_vector[iSphere].TestMe())
			{
				float fFar = m_vector[iSphere]->CollisionDetection(elem, iRotIndex,
					fPotentialCoef, fJointRadius, fPotentialEnd);
				elem.m_heuritic.m_vector[iSphere].Set(fFar);
				ChainRessource::Get().m_iNbDetection++;
			}
		}
	}
	else
	{
		for(u_int iSphere = 0 ; iSphere < m_vector.size() ; iSphere++ )
		{
			m_vector[iSphere]->CollisionDetection(elem, iRotIndex, fPotentialCoef, fJointRadius, fPotentialEnd);
			ChainRessource::Get().m_iNbDetection++;
		}
		
	}
	FRANKHAIRPRINT_INT("collision",elem.m_correctedPosition.m_list.size());
	elem.m_correctedPosition.PostDetection(pDefWelder,fPotentialExpulsion);

	CGatso::Stop("CollisionSphereSet::CollisionDetection");
}
	
// get number of sphere
int CollisionSphereSet::GetSize() const
{
	if(m_pDef)
	{
		return m_vector.size();
	}
	else
	{
		return 0;
	}
}









//! constructor
HairCollisionHeuristic::HairCollisionHeuristic()
{
	
}


void HairCollisionHeuristic::Update(float fDecrease)
{
	for(u_int iSphere = 0 ; iSphere < m_vector.size() ; iSphere++ )
	{
		m_vector[iSphere].Update(fDecrease);
	}
}

//
void HairCollisionHeuristic::Reset(int iSize)
{
	m_vector.clear();
	m_vector = Vector(iSize, HeuristicData());
}
	

