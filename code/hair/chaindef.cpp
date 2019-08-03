#include "chaindef.h"
#include "chaincore.h"
#include "effectchain.h"
#include "camera/camutils.h"
#include "objectdatabase/dataobject.h"
//#include "tbd/filedate.h"
#include "anim/hierarchy.h"
#include "tbd/franktmp.h"
#include "game/entity.h"
#include "game/entity.inl"
#include "game/luaattrtable.h"
#include "game/entitybindings.h"
#include "game/luaglobal.h"
#include "gfx/renderer.h"
#include "game/entitybrowser.h"
#include "gfx/meshinstance.h"
#include "game/renderablecomponent.h"
#include "gfx/simplefunction.h"
#include "forcefield.h"

using namespace SimpleFunction;

CVector m_scaleOut;
CVector m_scaleIn;

CVector m_translateOut;
CVector m_translateIn;

CVector m_rotateOut;
CVector m_rotateIn;

// Welder interface
START_CHUNKED_INTERFACE( SwordCollisionDef, Mem::MC_GFX )
	PUBLISH_VAR(m_scaleOut)
	PUBLISH_VAR(m_scaleIn)
	PUBLISH_VAR(m_translateOut)
	PUBLISH_VAR(m_translateIn)
	PUBLISH_VAR(m_rotateOut)
	PUBLISH_VAR(m_rotateIn)
END_STD_INTERFACE


// Welder interface
START_CHUNKED_INTERFACE( FloorCollisionDef, Mem::MC_GFX )
	PUBLISH_VAR(m_scale)
	PUBLISH_VAR(m_translate)
END_STD_INTERFACE

// Welder interface
START_CHUNKED_INTERFACE( ClothSpringSetDef, Mem::MC_GFX )
	PUBLISH_CONTAINER(m_container)
	PUBLISH_VAR(m_bIsCircle)
END_STD_INTERFACE

// Welder interface
START_CHUNKED_INTERFACE( ChainGlobalDef, Mem::MC_GFX )
	
	///////////////////////////////////////////////////
	// Group external force
	PUBLISH_VAR_AS(m_obGravitation,Gravitation)
	PUBLISH_VAR_AS(m_obWindForce,WindForce)
	PUBLISH_VAR_AS(m_fArtificialWindCoef,ArtificialWindCoef)
	PUBLISH_VAR_AS(m_fCentrifugeEcho,CentrifugeEcho)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bUseAntiDivergence, true, UseAntiDivergence); 
	// End of Group external force
	///////////////////////////////////////////////////
		
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bindingDebugEnable[0], true, JointDebugRedEnable);
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bindingDebug[0], "", JointDebugRed);
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bindingDebugEnable[1], true, JointDebugGreenEnable);
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bindingDebug[1], "", JointDebugGreen);
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bindingDebugEnable[2], true, JointDebugBlueEnable);
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bindingDebug[2], "", JointDebugBlue);		   

	///////////////////////////////////////////////////
	// Group time
	PUBLISH_VAR_AS(m_fWelderTimeChange,WelderTimeChange);
	PUBLISH_VAR_AS(m_bUseWelderTime,UseWelderTime);
	PUBLISH_VAR_AS(m_bUseTimeMultiplier,UseTimeMultiplier);
	// End of Group time
	///////////////////////////////////////////////////

	///////////////////////////////////////////////////
	// Group time
	PUBLISH_VAR_AS(m_bUseDebugRange,UseDebugRange);
	PUBLISH_VAR_AS(m_debugRange[0],DebugRangeBegin);
	PUBLISH_VAR_AS(m_debugRange[1],DebugRangeEnd);
	// End of Group time
	///////////////////////////////////////////////////

	///////////////////////////////////////////////////
	// Group debug
	PUBLISH_VAR_AS(m_fDebugDecalage,DebugDecalage);
	PUBLISH_VAR_AS(m_fHeroRotationDebug,HeroRotationDebug);
	PUBLISH_VAR_AS(m_bDrawChain,DrawChain);
	PUBLISH_VAR_AS(m_bDrawDefaultPosition,DrawDefaultPosition);
	PUBLISH_VAR_AS(m_bDrawCollisionSphere,DrawCollisionSphere);
	PUBLISH_VAR_AS(m_bDrawAntiCollisionSphere,DrawAntiCollisionSphere);
	PUBLISH_VAR_AS(m_bDrawCollisionDiff,DrawCollisionDiff);
	PUBLISH_VAR_AS(m_bDrawDummy1,DrawDummy1);
	PUBLISH_VAR_AS(m_bDrawDummy2,DrawDummy2);
	PUBLISH_VAR_AS(m_bDrawMesh,DrawMesh);
	PUBLISH_VAR_AS(m_bDrawAxis,DrawAxis);
	PUBLISH_VAR_AS(m_bDrawForce,DrawForce);
	PUBLISH_VAR_AS(m_bDrawTorque,DrawTorque);
	PUBLISH_VAR_AS(m_bDrawWind,DrawWind);
	PUBLISH_VAR_AS(m_bDrawInfo,DrawInfo);
	PUBLISH_VAR_AS(m_obDummy,Dummy)
	// End of Group debug
	///////////////////////////////////////////////////

	///////////////////////////////////////////////////
	// misc
	PUBLISH_VAR_AS(m_bUseSphereSize,UseSphereSize);
	PUBLISH_VAR_AS(m_fSphereSize,SphereSize);
	// End misc
	///////////////////////////////////////////////////

	DECLARE_EDITORCHANGEVALUE_CALLBACK(EditorChangeValue)
	PUBLISH_PTR_CONTAINER_AS(m_obHairstyle,Hairstyle) 

END_STD_INTERFACE


// Welder interface
START_CHUNKED_INTERFACE(HairStyleFromWelder, Mem::MC_GFX)
	
	// number of subframe
	PUBLISH_VAR_AS(m_iNbStepPerFrame,NbStepPerFrame)
	
	// latency value
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bUseLatency,false, UseLatency);
	PUBLISH_VAR_AS(m_fLatency,Latency)
	
	// gravity
	PUBLISH_VAR_AS(m_bUseGravity,UseGravity);
	PUBLISH_VAR_AS(m_fWeight,Weight)
	PUBLISH_VAR_AS(m_fInertiaMoment,InertiaMoment)
	
	// acceleration
	PUBLISH_VAR_AS(m_bUseAcceleration,UseAcceleration);
	PUBLISH_VAR_AS(m_fAccelerationEcho,AccelerationEcho)
	PUBLISH_VAR_AS(m_fAccelerationPropagation,AccelerationPropagation)
	PUBLISH_VAR_AS(m_fSpeedKillAcceleration,SpeedKillAcceleration)
	
	// fluid
	PUBLISH_VAR_AS(m_bUseFluid,UseFluid);
	PUBLISH_VAR_AS(m_fFluid,Fluid)
	
	// damping
	PUBLISH_VAR_AS(m_bUseDamp,UseDamp);
	PUBLISH_VAR_AS(m_fDamp,Damp)
	
	// mega damp
	PUBLISH_VAR_AS(m_bUseMegaDamp,UseMegaDamp);
	PUBLISH_VAR_AS(m_fMegaDamp,MegaDamp)
	
	// clip
	PUBLISH_VAR_AS(m_bUseClipAngle,UseClipAngle);
	PUBLISH_VAR_AS(m_fClipAngle,ClipAngle)
	
	// pose stiffness
	PUBLISH_VAR_AS(m_bUsePoseStiff,UsePoseStiff);
	PUBLISH_VAR_AS(m_fPoseStiff,PoseStiff)

	// parent stifness
	PUBLISH_VAR_AS(m_bUseParentStiff,UseParentStiff);
	PUBLISH_VAR_AS(m_fParentStiff,ParentStiff)
	PUBLISH_VAR_AS(m_fParentStiffTreshold,ParentStiffTreshold)
	
	// centrifuge
	PUBLISH_VAR_AS(m_bUseCentrifuge,UseCentrifuge);
	PUBLISH_VAR_AS(m_fCentrifuge,Centrifuge)
	
	// centrifuge
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bUseSpring, false, UseSpring);
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fSpringStiffness, 1.0f, SpringStiffness)

	// centrifuge
	PUBLISH_VAR_AS(m_bUseClothCollision, UseClothCollision);
	PUBLISH_VAR_AS(m_fClothCollision, ClothCollision)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fClothOffset, 0.0f, ClothOffset)

	//resize
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fClothCollisionResize, 1.0f, ClothCollisionResize)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_fCollisionCorrectionResize, 1.0f, CollisionCorrectionResize)
	
	// position correction
	PUBLISH_VAR_AS(m_bUsePositionCorrection,UsePositionCorrection);
	PUBLISH_VAR_AS(m_collisionForceRange[0],CollisionBegin)
	PUBLISH_VAR_AS(m_collisionForceRange[1],CollisionEnd)
	PUBLISH_VAR_AS(m_fPotentialCoef,PotentialCoef)
	PUBLISH_VAR_AS(m_fPotentialExpulsion,PotentialExpulsion)
	
	// collision forces
	PUBLISH_VAR_AS(m_bUseCollisionForce,UseCollisionForce);
	PUBLISH_VAR_AS(m_fPositionCorrection,PositionCorrection)
	PUBLISH_VAR_AS(m_fSpeedCorrection,SpeedCorrection)
	PUBLISH_VAR_AS(m_fUnconstrainedCoef,UnconstrainedCoef)

	// global correction
	PUBLISH_VAR_AS(m_bUseCollisionHeuristic,UseCollisionHeuristic);
	PUBLISH_VAR_AS(m_fCollisionHeuristic,CollisionHeuristic);
	PUBLISH_VAR_AS(m_fRadius,RadiusCoef);
	PUBLISH_VAR_AS(m_fPotentialEnd,PotentialEnd);

	// use wind
	PUBLISH_VAR_AS(m_bUseWind,UseWind);
	PUBLISH_VAR_AS(m_bUseArtificial,UseArtificial);
	PUBLISH_VAR_AS(m_bUseArtificialWind,UseArtificialWind);
	PUBLISH_GLOBAL_ENUM_AS(m_eArtificialWind,ArtificialWind,ARTIFICIALWIND)
	PUBLISH_VAR_WITH_DEFAULT_AS(m_bUseShrinker,false,UseShrinker);
	
	DECLARE_POSTCONSTRUCT_CALLBACK(PostConstruct)
	DECLARE_EDITORCHANGEVALUE_CALLBACK(EditorChangeValue)

END_STD_INTERFACE


START_CHUNKED_INTERFACE( HairRootDef, Mem::MC_GFX )
	PUBLISH_VAR( m_name)
	PUBLISH_VAR( m_parentName)
	PUBLISH_VAR( m_bindName)

	PUBLISH_VAR( m_translation)
	PUBLISH_VAR( m_jointOrient)
	PUBLISH_VAR( m_rotateAxis)
	PUBLISH_VAR( m_rotate)
END_STD_INTERFACE


START_CHUNKED_INTERFACE( HairStyleElemFromMaya, Mem::MC_GFX )
	///////////////////////////////////////////////////
	// Group wavelet style
	PUBLISH_VAR_WITH_DEFAULT( m_fRadius, 0.02f)
	PUBLISH_VAR_WITH_DEFAULT( m_fInertiaMoment, 1.0f)
	PUBLISH_VAR_WITH_DEFAULT( m_fFluid, 1.0f)
	PUBLISH_VAR_WITH_DEFAULT( m_fDamp, 1.0f)
	PUBLISH_VAR_WITH_DEFAULT( m_fPoseStiff, 1.0f)
	PUBLISH_VAR_WITH_DEFAULT( m_fParentStiff, 1.0f)
	PUBLISH_VAR_WITH_DEFAULT( m_fWeight, 1.0f)
	PUBLISH_VAR_WITH_DEFAULT( m_fParentStiffTreshold, 0.1f)
	PUBLISH_VAR_WITH_DEFAULT( m_bFreeze, false)
	PUBLISH_VAR_WITH_DEFAULT( m_bIsLeaf, false)
	// End of Group wavelet style
	///////////////////////////////////////////////////
	
	///////////////////////////////////////////////////
	// Group id 
	PUBLISH_VAR( m_name)
	PUBLISH_VAR( m_parentName)
	// End of Group id
	///////////////////////////////////////////////////
	
	///////////////////////////////////////////////////
	// Group location
	PUBLISH_VAR( m_fLength)
	
	PUBLISH_VAR( m_translation)
	PUBLISH_VAR( m_jointOrient)
	PUBLISH_VAR( m_rotateAxis)
	// End of Group location
	///////////////////////////////////////////////////
	
END_STD_INTERFACE





START_CHUNKED_INTERFACE( HairStyleFromMaya, Mem::MC_GFX )
	//PUBLISH_VAR(m_rootName)
	PUBLISH_VAR(m_parentTransformName)

	//PUBLISH_VAR(m_parent2SimTranslation)
	//PUBLISH_VAR(m_jointOrient)
	//PUBLISH_VAR(m_rotateAxis)

	PUBLISH_PTR_CONTAINER( m_rootList)
	PUBLISH_PTR_CONTAINER( m_jointList)
	PUBLISH_PTR_CONTAINER( m_clothList)
	PUBLISH_PTR_CONTAINER( m_springList)
END_STD_INTERFACE


START_CHUNKED_INTERFACE( HairSphereDef, Mem::MC_GFX ) 
	PUBLISH_VAR( m_translate )
	PUBLISH_VAR( m_scale )
	PUBLISH_VAR( m_rotate )
	
	PUBLISH_VAR_WITH_DEFAULT(m_translate[3],1.0f) 
	PUBLISH_VAR_WITH_DEFAULT(m_scale[3],0.0f) 
	PUBLISH_VAR_WITH_DEFAULT(m_rotate[3],0.0f) 
	
	PUBLISH_VAR_WITH_DEFAULT( m_fPositionCorrection,1.0f)
	PUBLISH_VAR_WITH_DEFAULT( m_fSpeedCorrection,1.0f)
	PUBLISH_VAR( m_parent)
END_STD_INTERFACE



START_CHUNKED_INTERFACE( HairSphereSetDef, Mem::MC_GFX )
	PUBLISH_PTR_CONTAINER( m_list )
END_STD_INTERFACE




// Welder interface
START_CHUNKED_INTERFACE( ClothColprim, Mem::MC_GFX )
PUBLISH_VAR(m_parentId)
PUBLISH_VAR(m_scale)
PUBLISH_VAR_WITH_DEFAULT(m_fShrinkerChest,0.0f)
PUBLISH_VAR_WITH_DEFAULT(m_fShrinkerBend,0.0f)
PUBLISH_VAR_WITH_DEFAULT(m_fShrinkerWeapon,0.0f)

PUBLISH_VAR_AS(m_influences[0].m_relativeName, Relative1)
PUBLISH_VAR_AS(m_influences[0].m_rotate,       Rotate1)
PUBLISH_VAR_AS(m_influences[0].m_translate,    Translate1)
PUBLISH_VAR_AS(m_influences[0].m_fWeigth,      Weigth1)

PUBLISH_VAR_WITH_DEFAULT_AS(m_influences[1].m_relativeName,ntstd::String(),Relative2)
PUBLISH_VAR_WITH_DEFAULT_AS(m_influences[1].m_rotate, CQuat(CONSTRUCT_CLEAR) ,Rotate2)
PUBLISH_VAR_WITH_DEFAULT_AS(m_influences[1].m_translate,CVector(CONSTRUCT_CLEAR),Translate2)
PUBLISH_VAR_WITH_DEFAULT_AS(m_influences[1].m_fWeigth,0.0f,Weigth2)
END_STD_INTERFACE







SwordCollisionDef::SwordCollisionDef()
{
	m_scaleOut = CVector(0.1f,0.5f,0.1f,0.0f);
	m_scaleIn = 0.1f * m_scaleOut;
	m_translateOut = CVector(0.0f,0.2f,0.0f,0.0f);
	m_translateIn = CVector(-0.3f,0.2f,0.0f,0.0f);
	m_rotateOut = CQuat(CONSTRUCT_IDENTITY);
	m_rotateIn = CQuat(CONSTRUCT_IDENTITY);
}

CVector SwordCollisionDef::GetScale(float fOutCoef) const
{
	return CVector::Lerp(m_scaleIn,m_scaleOut,fOutCoef);
}
CVector SwordCollisionDef::GetTranslate(float fOutCoef) const
{
	return CVector::Lerp(m_translateIn,m_translateOut,fOutCoef);
}
CQuat SwordCollisionDef::GetRotate(float fOutCoef) const
{
	return CQuat::Slerp(m_rotateIn,m_rotateOut,fOutCoef);
}








//! constructor
HairStyleFromWelder::HairStyleFromWelder()
{
	SetDefault();
}

//! constructor
HairStyleFromWelder::~HairStyleFromWelder()
{
	// nothing
}

//! post construct (real constructor)
void HairStyleFromWelder::PostConstruct()
{
	m_fLatency = clamp(m_fLatency,0.0f,static_cast<float>(HairConstant::HAIR_MEMORY_SIZE-1));
}

//! check if some value change do not require sophisticated information
bool HairStyleFromWelder::EditorChangeValue(CallBackParameter param, CallBackParameter)
{
	CHashedString pcItem(param);
	
	if (HASH_STRING_NBSTEPPERFRAME == pcItem)
	{
		ntPrintf("new NbStepPerFrame: %i", m_iNbStepPerFrame);
	}
	
	return false;
}

void HairStyleFromWelder::SetDefault()
{
	//m_fDamp = 0.0f;
	//m_fPoseStiff = 0.0f;
	//m_fParentStiff = 0.0f;
	//m_fAcceleration = 0.0f;
	//m_fFluid = 0.0f;
	//m_fLength = 1.0f;
	//m_iNbSegment = 4;
	//m_obDecalage = CVector(0.0f,0.5f,0.0f,0.0f);
}
// SPU
ChainSPU::ChainDef HairStyleFromWelder::GetSPUDef() const
{
	ChainSPU::ChainDef res;
	
	// nb sub frame
	res.m_usNbStepPerFrame = uint16_t(m_iNbStepPerFrame);
	
	// flags
	res.m_flags.Set(ChainSPU::ChainDef::FLUID,m_bUseFluid);
	res.m_flags.Set(ChainSPU::ChainDef::ACCELERATION,m_bUseAcceleration);
	res.m_flags.Set(ChainSPU::ChainDef::STIFF,m_bUseParentStiff);
	res.m_flags.Set(ChainSPU::ChainDef::GRAVITY,m_bUseGravity);	
	res.m_flags.Set(ChainSPU::ChainDef::COLLISIONFORCE,m_bUseCollisionForce);
	res.m_flags.Set(ChainSPU::ChainDef::COLLISIONDISPLACEMENT,m_bUsePositionCorrection);
	res.m_flags.Set(ChainSPU::ChainDef::ANYCOLLISION,m_bUseCollisionForce || m_bUsePositionCorrection);
	res.m_flags.Set(ChainSPU::ChainDef::DAMP,m_bUseDamp);
	res.m_flags.Set(ChainSPU::ChainDef::MEGADAMP,m_bUseMegaDamp);
	res.m_flags.Set(ChainSPU::ChainDef::CLIPANGLE,m_bUseClipAngle);
	res.m_flags.Set(ChainSPU::ChainDef::SPRING,m_bUseSpring);
	res.m_flags.Set(ChainSPU::ChainDef::ANTICOLLISIONDISPLACEMENT,m_bUseClothCollision);
	res.m_flags.Set(ChainSPU::ChainDef::WIND,m_bUseWind);
	res.m_flags.Set(ChainSPU::ChainDef::WIND_E3HACK, ObjectDatabase::Get().GetPointerFromName<E3WindDef*>("e3windef") !=0);

	// force construction
	res.m_fAccelerationPropagation = m_fAccelerationPropagation;
	res.m_fUnconstrainedCoef = m_fUnconstrainedCoef;
	res.m_fSpeedCorrection = m_fSpeedCorrection;
	res.m_fPositionCorrection = m_fPositionCorrection;

	// simulation equation
	res.m_fMegaDamp = m_fMegaDamp;
	res.m_fClipAngle = m_fClipAngle;

	// collision detection
	res.m_fPotentialCoef = m_fPotentialCoef;
	res.m_fPotentialEnd = m_fPotentialEnd;

	// range
	res.m_collisionForceRange = TmpVec2(m_collisionForceRange[0],m_collisionForceRange[1]);
	res.m_fPotentialExpulsion = m_fPotentialExpulsion;
	res.m_fCollisionCorrectionResize = m_fCollisionCorrectionResize;
	
	// acceleration hack
	res.m_fAccelerationEcho = m_fAccelerationEcho;

	// anticollision
	res.m_fClothCollision = m_fClothCollision;
	res.m_fAntiCollisionResize = m_fClothCollisionResize;

	return res;
}



















//! constructor
void ChainGlobalDef::SetDefault()
{
	// nothing
}
//! constructor
Pixel2 ChainGlobalDef::GetDebugRangeWithin(Pixel2 range) const
{
	return Pixel2(max(range[0],m_debugRange[0]),min(range[1],m_debugRange[1]));
}

//! constructor
bool ChainGlobalDef::EditorChangeValue(CallBackParameter param, CallBackParameter)
{
	CHashedString pcItem(param);
	
	//if(!strncmp(pcItem,"JointDebug",10))
	// ALEXEY_TODO : This is wrong if the string is longer than "JointDebug"!!!
	if (HASH_STRING_JOINTDEBUG == pcItem)
	{
		if(CEntityBrowser::Exists() && CEntityBrowser::Get().GetCurrentEntity()!=0)
		{
			CEntity* pEntity = const_cast<CEntity*>(CEntityBrowser::Get().GetCurrentEntity());
			if(pEntity->GetHierarchy())
			{
				const CHierarchy* pHierarchy = pEntity->GetHierarchy();
				Pixel3 debugIndices(-1,-1,-1);
				for(int i = 0 ; i < 3 ; i++ )
				{
					if(m_bindingDebugEnable[i])
					{
						int iIndex = pHierarchy->GetTransformIndex(CHashedString(m_bindingDebug[i].c_str()));
						if(iIndex>0)
						{
							debugIndices[i] = iIndex;
						}
					}
				}
				
				int iCount = 0;
				
				// [scee_st] now typedefed in the class itself for chunking
				// [scee_st] used to take a copy of the list!
				
				CRenderableComponent::MeshInstanceList& list = pEntity->GetRenderableComponent()->GetMeshInstances();
				for(CRenderableComponent::MeshInstanceList::iterator it = list.begin();
					it != list.end();
					it++)
				{
					CMeshInstance* pMesh = *it;
					MaterialInstanceBase* pMaterial = pMesh->GetMaterialInstance();
					if(pMaterial!=0)
					{
						pMaterial->SetDebugIndices(debugIndices,iCount);
					}
					iCount++;
				}
			}
		}
	}
	
	return true;
}


//! constructor
ChainGlobalDef::ChainGlobalDef()
{
	SetDefault();
}

















void HairDependencyInfo::Finalise()
{
	m_hashedid = CHashedString(m_name.c_str());
	m_hashedparent = CHashedString(m_parentName.c_str());
}




CMatrix HairStyleElemFromMaya::GetWorldPose(const CMatrix& sim2word) const
{
	return (m_local2sim * sim2word);
}

// Finalise (hashtring)
void HairStyleElemFromMaya::Finalise()
{
	HairDependencyInfo::Finalise();
	
	// name id
	m_elemId = HairStyleFromMaya::GetMayaIndexFromName(m_name);
	
	m_local = CMatrix(m_jointOrient * m_rotateAxis ,CPoint(m_translation));
	m_extraRot = CMatrix(m_rotateAxis);
	m_extraRotInv = m_extraRot.GetAffineInverse();
	
	if(HasParent())
	{
		m_fDistanceToRoot = GetParent()->m_fDistanceToRoot + m_fLength;
		m_local2sim = m_local * GetParent()->GetPoseMatrix();
	}
	else
	{
		ntAssert(m_pRootParent!=0);
		m_fDistanceToRoot = m_fLength;
		m_local2sim = m_local * m_pRootParent->GetPoseMatrix();
	}
	
	//m_simRoot2extremity = m_translation * m_local2sim;
	//Vec3 sphericalCoord = SphericalCoordinate::CartesianToSpherical(CDirection(m_simRoot2extremity));
	//m_poseAngles = Vec2(sphericalCoord[1], sphericalCoord[2]);
}




//! constructor
HairStyleElemFromMaya::HairStyleElemFromMaya()
	:m_fDistanceToRoot(-1.0f)
	,m_pRootParent(0)
{
	// nothing
}

//! desturctor
HairStyleElemFromMaya::~HairStyleElemFromMaya()
{
	// nothing
}

float HairStyleElemFromMaya::GetParentStiffTreshold(const HairStyleFromWelder* pDef) const
{
	float fDegAngle = m_fParentStiffTreshold+pDef->m_fParentStiffTreshold;
	float fRadAngle = -clamp(fDegAngle,0.0f,180.0f)*DEG_TO_RAD_VALUE;
	return cos(fRadAngle);
}











//! constructor
HairStyleFromMaya::HairStyleFromMaya()
	:m_maxMayaId(-1)
{
	// nothing
}

//! destructor
HairStyleFromMaya::~HairStyleFromMaya()
{
	// nothing
}


HairStyleFromMaya::Register& HairStyleFromMaya::GetRegister() const
{
	return *static_cast<Register*>(GetKeepMe());
}




namespace HairDependency
{
	template<class T>
		inline ntstd::Vector<T*, Mem::MC_GFX> ComputeDependency(ntstd::List<T*, Mem::MC_GFX>& list)
	{
		typedef ntstd::Vector<T*, Mem::MC_GFX> Vector;
		typedef ntstd::List<T*, Mem::MC_GFX> List;
		typedef ntstd::Set<const T*, ntstd::less<const T*>, Mem::MC_GFX> Set;
		Vector dependency;
		
		dependency.clear();
		dependency.reserve(list.size());
		List undone;
		Set done;


		// set all the parent
		for(typename List::const_iterator it = list.begin();
			it != list.end();
			it++)
		{
			T& elem = *(*it);
			
			// find parent
			for(typename List::const_iterator it2 = list.begin();
				it2 != list.end();
				it2++)
			{
				T& elem2 = *(*it2);
				if(&elem2==&elem)
				{
					continue;
				}
				if(elem.m_parentName == elem2.m_name)
				{
					elem.SetParent(&elem2);
					undone.push_back(&elem);
					break;
				}
			}

			// no parent within the joint
			if(!elem.HasParent())
			{
				dependency.push_back(&elem);
				done.insert(&elem);
			}
		}

		// create dependency
		while(undone.size()!=0)
		{
			typename List::iterator it = undone.begin();
			while(it!=undone.end())
			{
				T& elem = *(*it);
				if(done.find( static_cast<const T*>(elem.GetParentT())) != done.end() )
				{
					dependency.push_back(&elem);
					done.insert(&elem);
					typename List::iterator save = it;
					it++;
					undone.erase(save);
				}
				else
				{
					it++;
				}
			}
		}

		ntAssert(dependency.size()==list.size());
		return dependency;	
	}
} // end of namespace 





// find out which node is the parent to which
void HairStyleFromMaya::ComputeDependency()
{
	// should use the m_dependency in the clump header
	// I did that because of some initialisation ordering issue
	// and because at one point I was thinking of getting rid of the
	// CHierarchy and about doing my own stuff (which was a bad idea)
	m_jointDependency = HairDependency::ComputeDependency(m_jointList);
	m_rootDependency = HairDependency::ComputeDependency(m_rootList);
	
	// link to eventual root
	for(u_int iJoint = 0 ; iJoint < m_jointDependency.size() ; iJoint++ )
	{
		if(!m_jointDependency[iJoint]->HasParent())
		{
			HairStyleElemFromMaya& elem = *m_jointDependency[iJoint];
			HairRootDef* pRootParent = FindRoot(elem.m_parentName);
			ntAssert(pRootParent!=0);
			elem.m_pRootParent = pRootParent;
		}
	}
}

HairRootDef* HairStyleFromMaya::FindRoot(const ntstd::String& name)
{
	for(u_int iRoot = 0 ; iRoot < m_rootDependency.size() ; iRoot++ )
	{
		if(m_rootDependency[iRoot]->m_name == name)
		{
			return m_rootDependency[iRoot];
		}
	}
	
	return static_cast<HairRootDef*>(0);
}

int HairStyleFromMaya::GetMayaIndexFromName(const ntstd::String& name)
{
	int iUnderscore = name.find_last_of('_');
	iUnderscore+=1; // = the underscore size
	ntstd::String num = name.substr(iUnderscore,name.size()-iUnderscore);
	return atoi(num.c_str());
}



int HairStyleFromMaya::GetGameIndexFromMayaIndex(int iMayaId) const
{
	ntAssert((iMayaId>=0) && (iMayaId<static_cast<int>(m_name2index.size())));
	return m_name2index[iMayaId];
}


bool HairStyleFromMaya::CheckRoot()
{
	if(m_rootDependency[0]->HasParent())
	{
		return false;
	}
	for(u_int iElem = 1 ; iElem < m_rootDependency.size() ; iElem++ )
	{
		if(!m_rootDependency[iElem]->HasParent())
		{
			return false;
		}
	}
	
	return true;
}

//--------------------------------------------------
//!
//!	big maya finalise function.
//!
//--------------------------------------------------
void HairStyleFromMaya::Finalise()
{
	// set transform matrix from simulation (first root) to parent (where the hair are attached)
	//m_jointOrient = CCamUtil::QuatFromEuler_XYZ(-36.995f,29.341f,-10.084f);
	//m_rotateAxis = CCamUtil::QuatFromEuler_XYZ(112.88f,43.85f,-67.186f);
	//m_invRO = CMatrix(~m_rotateAxis);
	//m_sim2parent =  CMatrix(m_jointOrient * m_rotateAxis,CPoint(m_parent2SimTranslation));
	
	// find dependency
	ComputeDependency();
	
	//root
	ntAssert(CheckRoot());
	for(u_int iElem = 0 ; iElem < m_rootDependency.size() ; iElem++ )
	{
		m_rootDependency[iElem]->Finalise();
	}

	// compute distance to root and local2sim matrix
	m_fMaxDistanceToRoot=0.0f;
	for(u_int iElem = 0 ; iElem < m_jointDependency.size() ; iElem++ )
	{
		HairStyleElemFromMaya& elem = *m_jointDependency[iElem];
		elem.Finalise();
		m_fMaxDistanceToRoot = max(m_fMaxDistanceToRoot,elem.m_fDistanceToRoot);
		m_maxMayaId = max(m_maxMayaId,elem.m_elemId);
	}
	
	// One more pass to compute norm distance
	for(u_int iElem = 0 ; iElem < m_jointDependency.size() ; iElem++ )
	{
		HairStyleElemFromMaya& elem = *m_jointDependency[iElem];
		elem.m_fNormDistanceToRoot = elem.m_fDistanceToRoot / m_fMaxDistanceToRoot;
	}
	
	if(!HasKeepMe())
	{
		SetKeepMe(NT_NEW Register());
	}
	
	//cloth
	for(ClothList::iterator it = m_clothList.begin();
		it != m_clothList.end();
		it++)
	{
		(*it)->Finalise();
	}
	

	// set the map top index container
	m_name2index = IndexMap(m_maxMayaId+1,0);
	for(u_int iElem = 0 ; iElem < m_jointDependency.size() ; iElem++ )
	{
		HairStyleElemFromMaya& elem = *m_jointDependency[iElem];
		ntAssert((elem.m_elemId>=0) && (elem.m_elemId<static_cast<int>(m_name2index.size())));
		m_name2index[elem.m_elemId] = iElem;
	}
}


// propagate change after reload
void HairStyleFromMaya::PropagateChange()
{
	Register reg = GetRegister();
	for(RegisterContainer<OneChain>::iterator itOneChain = reg.begin();
		itOneChain != reg.end();
		itOneChain++)
	{
		// the old chain need that to unregister
		(*itOneChain)->TmpResetStyleData(this);
		(*itOneChain)->UnRegister();
		
		// save
		CEntity* pEntity = (*itOneChain)->GetEntity();
		LuaAttributeTable* pLuaTable = pEntity->GetAttributeTable();
		LuaAttributeTable copy;	
		pLuaTable->DeepCopyTo( &copy );
		
		// destro old
		DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromPointer( pEntity );
		ntAssert(pDO);
		ObjectDatabase::Get().DestroyObject( pDO );
		ntAssert(reg.size() == (GetRegister().size()+1));
		
		// create new
		CreateEntityFromLuaAttributeTable(&copy);
	}
	ntAssert(reg.size() == GetRegister().size());
}


// reset all the tmp vector to 0
void HairStyleFromMaya::ResetTmp() const
{
	for(JointDependency::const_iterator it = m_jointDependency.begin();
		it != m_jointDependency.end();
		it++)
	{
		(*it)->m_pTmp = 0;
	}
}

int HairStyleFromMaya::GetNbSpring() const
{
	int res = 0;
	for(SpringList::const_iterator it = m_springList.begin();
		it != m_springList.end();
		++it)
	{
		res+=(*it)->GetSize();
	}
	return res;
}









// force scale (use in extreme case)
void HairSphereDef::SetScale(const CVector& scale)
{
	m_scale = scale;
}
// force scale (use in extreme case)
void HairSphereDef::SetTranslate(const CVector& translate)
{
	m_translate = translate;
}
// force scale (use in extreme case)
void HairSphereDef::SetRotate(const CQuat& rotate)
{
	m_rotate = rotate;
}


void HairSphereDef::Finalise()
{
	CMatrix rot(m_rotate,CPoint(m_translate));
	m_sphere2Local = FrankMisc::CreateScaleMatrix(CDirection(m_scale)) * rot;
	m_local2Sphere = rot.GetAffineInverse() * FrankMisc::CreateInvScaleMatrix(CDirection(m_scale));
}

float HairSphereDef::GetAverageRadius() const
{
	return 0.333f * (m_scale.X() + m_scale.Y() + m_scale.Z());
}


HairSphereDef::HairSphereDef()
{
	m_translate = CVector(CONSTRUCT_CLEAR);
	m_rotate = CQuat(CONSTRUCT_CLEAR);
	m_scale = CVector(CONSTRUCT_CLEAR);
	m_sphere2Local=CMatrix(CONSTRUCT_CLEAR);
	m_local2Sphere=CMatrix(CONSTRUCT_CLEAR);
	
	ntstd::String m_parent = ntstd::String();
	
	m_fPositionCorrection = 1.0f;
	m_fSpeedCorrection = 1.0f;
}

HairSphereDef::HairSphereDef(const CVector& translate, const CVector& scale, const CQuat& rotate)
{
	m_translate = translate;
	m_rotate = rotate;
	m_scale = scale;

	m_fPositionCorrection = 1.0f;
	m_fSpeedCorrection = 1.0f;

	Finalise();
}

HairSphereDef::~HairSphereDef()
{
	// nothing
}







//! constructor
HairSphereSetDef::HairSphereSetDef()
{
	// nothing
}

//! constructor
HairSphereSetDef::~HairSphereSetDef()
{
	// nothing
}

void HairSphereSetDef::Finalise()
{
	for(HairSphereDefList::iterator it = m_list.begin();
		it != m_list.end();
		it++)
	{
		(*it)->Finalise();
	}
	
	if(!HasKeepMe())
	{
		SetKeepMe(NT_NEW Register());
	}
}

HairSphereSetDef::Register& HairSphereSetDef::GetRegister() const
{
	return *static_cast<Register*>(GetKeepMe());
}

// propagate change after reload
void HairSphereSetDef::PropagateChange()
{
	for(Register::iterator itOneChain = GetRegister().begin();
		itOneChain != GetRegister().end();
		itOneChain++)
	{
		(*itOneChain)->Reset(this);
	}
}



//! constructor
void ClothColprim::OneInfluence::Finalise(float fXDec)
{
	CMatrix gotoSeed(CONSTRUCT_IDENTITY);
	gotoSeed.SetTranslation(CPoint(-fXDec,0.0f,0.0f));
	m_sphere2Local = gotoSeed * CMatrix(m_rotate,CPoint(m_translate));
	m_local2Sphere = m_sphere2Local.GetAffineInverse();
}
ClothColprim::OneInfluence::OneInfluence()
{
	// nothing
}






void ClothColprim::Finalise()
{
	m_scaleMat = FrankMisc::CreateScaleMatrix(CDirection(m_scale));
	m_scaleMatInv = FrankMisc::CreateInvScaleMatrix(CDirection(m_scale));

	m_influences[0].Finalise(m_scale.X());
	if(m_influences[1].m_relativeName.empty())
	{
		m_bJustOne =  true;
		m_sphere2Local = m_scaleMat * m_influences[0].m_sphere2Local;
		m_local2Sphere = m_influences[0].m_local2Sphere * m_scaleMatInv;
		m_influences[0].m_fWeigth = 1.0f;
	}
	else
	{
		m_bJustOne =  false;
		m_sphere2Local = CMatrix(CONSTRUCT_CLEAR);
		m_local2Sphere = CMatrix(CONSTRUCT_CLEAR);

		m_influences[1].Finalise(m_scale.X());
		
		float fTotal = m_influences[0].m_fWeigth+m_influences[1].m_fWeigth;
		m_influences[0].m_fWeigth /= fTotal;
		m_influences[1].m_fWeigth /= fTotal;
	}
	
	if(m_fShrinkerBend>0.001f)
	{
		m_mask.Set(ClothColprim::F_SPEEDSHRINK);
	}
	if(m_fShrinkerChest>0.001f)
	{
		m_mask.Set(ClothColprim::F_CHESTSHRINK);
	}
	if(m_fShrinkerWeapon>0.001f)
	{
		m_mask.Set(ClothColprim::F_WEAPONSHRINK);
	}
	 
}

//! constructor
ClothColprim::ClothColprim()
{
	CMatrix m_scaleMat;
	CMatrix m_scaleMatInv;
	m_scale = CVector(CONSTRUCT_CLEAR);
}









HairRootDef::HairRootDef()
{
	
}

void HairRootDef::Finalise()
{
	HairDependencyInfo::Finalise();
	
	m_bindid = CHashedString(m_bindName.c_str());
	m_local = CMatrix(m_jointOrient * m_rotate * m_rotateAxis ,CPoint(m_translation));
	if(HasParent())
	{
		const HairRootDef* pParent = static_cast< const HairRootDef* >(GetParentT());
		m_local2sim = m_local * pParent->m_local2sim;
	}
	else
	{
		m_local2sim = CMatrix(CONSTRUCT_IDENTITY);
	}
}
