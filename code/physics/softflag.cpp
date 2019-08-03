//------------------------------------------------------------------------------------------
//!
//! softflag.cpp
//! 
//!
//------------------------------------------------------------------------------------------

#include "anim/hierarchy.h"
#include "anim/transform.h"
#include "game/entity.h"
#include "game/entity.inl"
#include "gfx/renderable.h"
#include "game/renderablecomponent.h"
#include "gfx/sector.h"
#include "physics/verlet.h"
#include "physics/verletdef.h"
#include "physics/verletmanager.h"
#include "objectdatabase/dataobject.h"
#include "physics/softflag.h"


#ifdef PLATFORM_PS3
#include "physics/verletrenderable_ps3.h"
#endif

START_STD_INTERFACE	( Template_Flag )
		PUBLISH_VAR_WITH_DEFAULT_AS	( m_obConstructionScript,	"SoftObject_Construct",		ConstructionScript)	
//		PUBLISH_VAR_WITH_DEFAULT_AS	( m_obPosition,           	CVector(0.0f,0.0f,0.0f,0.0f),	Position) 	
//		PUBLISH_VAR_WITH_DEFAULT_AS	( m_obOrientation,        	CQuat(0.0f,0.0f,-1.0f,0.0f),	Orientation)        	
		PUBLISH_ACCESSOR_WITH_DEFAULT( CPoint, Position, GetPosition, SetPosition, CPoint(0.0f, 0.0f, 0.0f) )
		PUBLISH_ACCESSOR_WITH_DEFAULT( CQuat, Orientation, GetOrientation, SetOrientation, CQuat(0.0f, 0.0f, 0.0f, 1.0f) )

		PUBLISH_PTR_AS				( m_pobParentEntity,       	ParentEntity)       	
//		PUBLISH_PTR_ACCESSOR		( void*, ParentEntity, GetParentEntity, SetParentEntity )

		PUBLISH_VAR_WITH_DEFAULT_AS	( m_obParentTransform,    	"WORLD",					ParentTransform)    	
		PUBLISH_VAR_WITH_DEFAULT_AS	( m_uiResolutionX,			14,							ResolutionX)				
		PUBLISH_VAR_WITH_DEFAULT_AS	( m_uiResolutionY,			14,							ResolutionY)			
		PUBLISH_VAR_WITH_DEFAULT_AS	( m_fSizeX,					1.0f,						SizeX)					
		PUBLISH_VAR_WITH_DEFAULT_AS	( m_fSizeY,					4.5f,						SizeY)					
		PUBLISH_VAR_WITH_DEFAULT_AS	( m_obTexture,             	"e3_flags.dds",				Texture)            	
		PUBLISH_VAR_WITH_DEFAULT_AS	( m_obNormalMap,            "flat_normalmap.tga",		NormalMap)              
		PUBLISH_PTR_AS	( m_pobGlobalWind,						GlobalWind)	
		PUBLISH_PTR_AS	( m_pobSoftMaterial,					SoftMaterial)	
		DECLARE_POSTCONSTRUCT_CALLBACK( PostConstruct )
		DECLARE_POSTPOSTCONSTRUCT_CALLBACK( PostPostConstruct )
		DECLARE_EDITORCHANGEVALUE_CALLBACK( EditorChangeValue )
	END_STD_INTERFACE
	
void ForceLinkFunction27()
{
	ntPrintf("!ATTN! Calling ForceLinkFunction27() !ATTN!\n");
}

//------------------------------------------------------------------------------------------
//!
//! Template_Flag::Template_Flag()
//! Constructor
//!
//------------------------------------------------------------------------------------------
Template_Flag::Template_Flag() : 
	m_bDonePostPostConstruct( false )
{
}

//------------------------------------------------------------------------------------------
//!
//! Template_Flag::PostConstruct()
//! 
//!
//------------------------------------------------------------------------------------------
void Template_Flag::PostConstruct()
{
	//	SetLocalTransform( this.attrib.PositionOffset.x,this.attrib.PositionOffset.y,this.attrib.PositionOffset.z, this.attrib.RotationOffset.x,this.attrib.RotationOffset.y,this.attrib.RotationOffset.z )
	//	Dynamics_ConstructSoftFromClump( this.attrib.Resolution,  this.attrib.SizeX,  this.attrib.SizeZ, this.attrib.ParticleMass, this.attrib.ConstraintIterations, this.attrib.WindForce, this.attrib.DragFactor, this.attrib.Texture, this.attrib.NormalMap )
}

void Template_Flag::PostPostConstruct()
{
#ifdef PLATFORM_PS3
	m_obPosition.W() = 0.0f;
	CMatrix obMat(m_obOrientation, *(CPoint*)&m_obPosition);
	m_obTransform.SetLocalMatrix(obMat);

	//Transform* pobParentTransform = m_pobParentEntity->GetHierarchy()->GetTransform(*m_obParentTransform);

	if( Physics::VerletManager::IsEnabled() == true )
	{
		m_pobVerlet = Physics::VerletManager::Get().CreateASimpleGenericVerletSystem( &m_obTransform, this );
		Physics::VerletInstance* pobI = m_pobVerlet;
		Physics::VerletRenderableInstance* pobR = pobI->GetRenderable();
		CSector::Get().GetRenderables().AddRenderable( pobR );

		// NOTE! This will cause asserts to trigger on level shutdown (catapult on TGS level)
		// We need to remove ourselves from our parent entity when we destruct, likewise our parent
		// should do something with us when it destructs.
		if (m_pobParentEntity)
		{
			pobI->AttachToEntity(m_pobParentEntity, m_obParentTransform);
		}
		
	}

	m_bDonePostPostConstruct = true;

#endif
}

Template_Flag::~Template_Flag()
{
#ifdef PLATFORM_PS3
	if( Physics::VerletManager::IsEnabled() == true )
	{
		Physics::VerletManager::Get().Unregister( m_pobVerlet );
		CSector::Get().GetRenderables().RemoveRenderable( m_pobVerlet->GetRenderable() );
		NT_DELETE( m_pobVerlet->GetRenderable() );
		NT_DELETE( m_pobVerlet );
	}
#endif
}

//------------------------------------------------------------------------------------------
//!
//! Template_Flag::EditorChangeValue
//! The editor has changed a member... perform update
//!
//------------------------------------------------------------------------------------------
bool Template_Flag::EditorChangeValue(CallBackParameter , CallBackParameter )
{

	// Set rotation and position
	CMatrix obMat(m_obOrientation, *(CPoint*)&m_obPosition);
	m_obTransform.SetLocalMatrix(obMat);
	return true;
}

void Template_Flag::SetPosition( const CPoint& pos )
{
	m_obPosition.X() = pos.X();
	m_obPosition.Y() = pos.Y();
	m_obPosition.Z() = pos.Z();

	// Set rotation and position
	CMatrix obMat(m_obOrientation, *(CPoint*)&m_obPosition);
	m_obTransform.SetLocalMatrix(obMat);
}

CPoint Template_Flag::GetPosition( ) const
{
	return CPoint( m_obPosition.X(), m_obPosition.Y(), m_obPosition.Z() );
}

void Template_Flag::SetOrientation( const CQuat& orient )
{
	m_obOrientation = orient;

	// Set rotation and position
	CMatrix obMat(m_obOrientation, *(CPoint*)&m_obPosition);
	m_obTransform.SetLocalMatrix(obMat);
}

CQuat Template_Flag::GetOrientation() const 
{
	return m_obOrientation;
}

void Template_Flag::SetParentEntity( CEntity* pParent, CKeyString transform )
{
	// we do the attachment later if we haven't yet post post constructed else now is good
	if( m_bDonePostPostConstruct == false )
	{
		m_pobParentEntity = pParent;
		m_obParentTransform = transform;
	} else
	{
		Physics::VerletInstance* pobI = m_pobVerlet;

		// NOTE! This will cause asserts to trigger on level shutdown (catapult on TGS level)
		// We need to remove ourselves from our parent entity when we destruct, likewise our parent
		// should do something with us when it destructs.
		if (pParent)
		{
			m_pobParentEntity = pParent;
			m_obParentTransform = transform;
			pobI->AttachToEntity(m_pobParentEntity, m_obParentTransform);
		} else
		{
			pobI->DetachFromParent();
			m_pobParentEntity = 0;
		}
	}
}

CEntity* Template_Flag::GetParentEntity() const
{
	return m_pobParentEntity;
}

void Template_Flag::SetParentTransform( Transform* pParent )
{
	if( m_bDonePostPostConstruct == false )
	{
		ntError_p( false, ("SetParentTransform must be called after post post construct") );
	} else
	{
		Physics::VerletInstance* pobI = m_pobVerlet;

		// NOTE! This will cause asserts to trigger on level shutdown (catapult on TGS level)
		// We need to remove ourselves from our parent entity when we destruct, likewise our parent
		// should do something with us when it destructs.
		if (pParent)
		{
			m_pobParentEntity = 0;
			m_obParentTransform = 0;
			pobI->AttachToTransform(pParent);
		} else
		{
			pobI->DetachFromParent();
			m_pobParentEntity = 0;
			m_obParentTransform = 0;
		}
	}
}
