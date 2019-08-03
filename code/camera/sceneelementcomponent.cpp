//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file sceneelementcomponent.cpp                                                         
//!                                                                                         
//------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------
// Required Includes                                                                        
//------------------------------------------------------------------------------------------
#include "camera/sceneelementcomponent.h"
#include "camera/elementmanager.h"

#include "objectdatabase/dataobject.h"

#include "game/entity.h"
#include "game/entity.inl"
#include "game/entityinfo.h"
#include "game/luaexptypes.h"
#include "game/entityplayer.h"
#include "game/entityarcher.h"
#include "lua/ninjalua.h"

//------------------------------------------------------------------------------------------
// NinjaLua bindings
//------------------------------------------------------------------------------------------
LUA_EXPOSED_START(SceneElementComponent)
	LUA_EXPOSED_METHOD_SET(Importance, SetImportance, "The importance of this element.")
	LUA_EXPOSED_METHOD_SET(Radius,     SetRadius,     "The radius of this element to keep in view.")
	LUA_EXPOSED_METHOD(SetInfluenceRadius,   SetInfluenceRadius,  "Set the radius which this element is influencing over.", "number in, number out", "Full influence before this distance|No influence after this distance")
LUA_EXPOSED_END(SceneElementComponent)

//------------------------------------------------------------------------------------------
// Definition Interface                                                                     
//------------------------------------------------------------------------------------------
START_CHUNKED_INTERFACE(SceneElementComponentDef, Mem::MC_CAMERA)
	_IFLOAT(Importance)	
	_IFLOAT(Radius)	
	_IVECTOR(Offset)		
	_IFLOAT(InfluenceRadiusIn)
	_IFLOAT(InfluenceRadiusOut)
	_IFLOAT(LookAheadTime)
END_STD_INTERFACE


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	SceneElementComponent::SceneElementComponent                                            
//!	Construction                                                                            
//!                                                                                         
//------------------------------------------------------------------------------------------
SceneElementComponent::SceneElementComponent(CEntity* pEnt, const SceneElementComponentDef* pDef) : 
	m_fImportance( 0.f ),
	m_fTargetImportance( -1.f ),
	m_fImportanceBlendTime( 0.f ),
	m_fOriginalImportance( 0.f ),
	m_fRadius( 0.f ),
	m_dirOffset( CONSTRUCT_CLEAR ),
	m_fInfluenceRadiusIn( 0.f ),
	m_fInfluenceRadiusOut( 0.f ),
	m_eIsInfluencing(INFLUENCE_NONE),
	m_fLookAheadTime( 0.f ),
	m_pobEntity( 0 ),
	m_fInfluencePercentage( 0.f ),
	m_fR( 1.f ),
	m_fG( 0.f ),
	m_fB( 0.f )
{
	ATTACH_LUA_INTERFACE(SceneElementComponent);

	ntAssert(pEnt);
	ntAssert(pDef);

	// Assign us to our entity
	m_pobEntity = pEnt;

	// Set attributes from definition
	m_fImportance         = pDef->m_fImportance;
	m_fRadius             = pDef->m_fRadius;
	m_dirOffset           = pDef->m_obOffset;
	m_fInfluenceRadiusIn  = pDef->m_fInfluenceRadiusIn;
	m_fInfluenceRadiusOut = pDef->m_fInfluenceRadiusOut;
	m_fLookAheadTime      = pDef->m_fLookAheadTime;

	// No Lookahead if we don't have an info component
	if(!pEnt->IsCharacter())
	{
		m_fLookAheadTime = 0.f;
	}

	//Register the component with the element manager
	CamSceneElementList::Get().AddSceneElement(this);
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	SceneElementComponent::~SceneElementComponent                                            
//!	Destruction                                                                            
//!                                                                                         
//------------------------------------------------------------------------------------------
SceneElementComponent::~SceneElementComponent()
{
	if(CamSceneElementList::Exists())
		CamSceneElementList::Get().RemoveSceneElement(this);
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	SceneElementComponent::GetPosition                                                      
//!	Get the position of the element.                                                        
//!                                                                                         
//------------------------------------------------------------------------------------------
CPoint SceneElementComponent::GetPosition() const
{
	ntAssert(m_pobEntity);

	CPoint ptPos = m_pobEntity->GetPosition()/*m_pobEntity->GetLocation()*/;
	CPoint ptRet = ptPos;

	if(m_pobEntity->GetHierarchy())
	{
		ptRet += m_dirOffset * m_pobEntity->GetRootTransformP()->GetWorldMatrix();
	}


	// !!!Nasty!!! special case to prevent the archer from causing camera (Y) chase when vaulting objects. 
	if( m_pobEntity->IsPlayer() )
	{
		const Player* pPlayer = m_pobEntity->ToPlayer();

		if( pPlayer->IsArcher() )
		{
			const Archer* pArcher = pPlayer->ToArcher();
			
			// Only null the (Y) camera movement if the archer is vaulting. 
			if( pArcher->IsVaulting() )
			{
				ptRet.Y() -= ptPos.Y() - pArcher->GetVaultingCameraY();
			}
		}
	}

	return ptRet;
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	SceneElementComponent::GetVelocity                                                      
//!	Get the velocity of the element.                                                        
//!                                                                                         
//------------------------------------------------------------------------------------------
CDirection SceneElementComponent::GetVelocity() const
{
	ntAssert(m_pobEntity);

	return m_pobEntity->GetCalcVelocity();
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	SceneElementComponent::GetLookAheadDirection                                                    
//!	Direction = Velocity * Time                                                                            
//!                                                                                         
//------------------------------------------------------------------------------------------
CDirection SceneElementComponent::GetLookAheadDirection() const
{
	if(m_fLookAheadTime <= 0.f)
		return CDirection(CONSTRUCT_CLEAR);

	return GetVelocity() * m_fLookAheadTime;
}



//------------------------------------------------------------------------------------------
//!                                                                                         
//!	SceneElementComponent::SetImportance                                                    
//!	Change the importance of this element.  If it's active then it needs to blend to the    
//! new value.                                                                              
//!                                                                                         
//------------------------------------------------------------------------------------------
void SceneElementComponent::SetImportance(float fImportance)
{
	m_fTargetImportance = fImportance;
}

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	SceneElementComponent::IsInfluencing                                                    
//!	Should the element influence the camera system.                                         
//! Return value specifies what type it should have.                                        
//!                                                                                         
//------------------------------------------------------------------------------------------
SceneElementComponent::INFLUENCE_VALUE SceneElementComponent::IsInfluencing(CPoint& obPt)
{
	ntError_p(m_pobEntity,("Missing entity for scene element"));

	if(GetEntity()->IsPaused() || 
	   m_fInfluenceRadiusIn <= 0.0f || 
	   m_fInfluenceRadiusOut <= 0.0f || 
	   m_fImportance <= 0.0f)
	{
		m_fInfluencePercentage = 1.0f;
		return INFLUENCE_NONE;
	}

	CDirection obDir = GetPosition() ^ obPt;
	m_fInfluencePercentage = obDir.Length();
	if(m_fInfluencePercentage < m_fInfluenceRadiusIn)
		m_fInfluencePercentage = 1.0f;
	else if(m_fInfluencePercentage > m_fInfluenceRadiusOut)
		m_fInfluencePercentage = 0.0f;
	else
		m_fInfluencePercentage = (m_fInfluenceRadiusOut - m_fInfluencePercentage) /
								 (m_fInfluenceRadiusOut - m_fInfluenceRadiusIn);

	if(m_eIsInfluencing == INFLUENCE_FULL)
	{
		if(GetPosition().Compare(obPt, m_fInfluenceRadiusOut * m_fInfluenceRadiusOut))
			m_eIsInfluencing = INFLUENCE_FULL;
		else
			m_eIsInfluencing = INFLUENCE_POI;
	}
	else
	{
		if(GetPosition().Compare(obPt, m_fInfluenceRadiusIn * m_fInfluenceRadiusIn))
			m_eIsInfluencing = INFLUENCE_FULL;
		else
		{
			if(GetPosition().Compare(obPt, m_fInfluenceRadiusOut * m_fInfluenceRadiusOut))
				m_eIsInfluencing = INFLUENCE_POI;
			else
				m_eIsInfluencing = INFLUENCE_NONE;
		}
	}

	return m_eIsInfluencing;
}


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	SceneElementComponent::Update                                            
//!	Update the scene element.                                                               
//!                                                                                         
//------------------------------------------------------------------------------------------
void SceneElementComponent::Update(float fTimeDelta)
{
	if(m_fTargetImportance >= 0.f)
	{
		if(m_fOriginalImportance < 0.f)
		{
			m_fOriginalImportance = m_fImportance;
			m_fImportanceBlendTime = 0.f;
		}

		m_fImportanceBlendTime += fTimeDelta;
		if(m_fImportanceBlendTime > 1.f)
		{
			m_fImportance         = m_fTargetImportance;
			m_fTargetImportance   = -1.f;
			m_fOriginalImportance = -1.f;
			m_fImportanceBlendTime = 0.f;
			return;
		}

		float fRatio = ntstd::Clamp(m_fImportanceBlendTime / 1.f, 0.f, 1.f);

		fRatio = (fRatio - .5f) * 8.f;                                // Mapping fn for Sigmoid fn.
		fRatio = 1.f / (1.f + pow(MATH_E, -fRatio));                  // Sigmoid fn.
		fRatio = ntstd::Clamp(fRatio * 1.02f/.98f - 0.02f, 0.f, 1.f); // Re-limiting correction.

		m_fImportance = m_fOriginalImportance*(1.f-fRatio) + m_fTargetImportance*fRatio;
		//ntPrintf("%.2f(%.2f), %.2f(%.2f) -> %.2f\n", m_fOriginalImportance, (1.f-fRatio), m_fTargetImportance, fRatio, m_fImportance);
	}
}
