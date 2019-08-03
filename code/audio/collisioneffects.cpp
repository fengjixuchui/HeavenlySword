/*
#include "audio/collisioneffects.h"
#include "audio/audiosystem.h"

#include "objectdatabase/dataobject.h"



START_STD_INTERFACE							( EntityCollisionHandlerDef )
	PUBLISH_VAR_AS							( m_uiMaterialClass,			MaterialClass )
	PUBLISH_VAR_AS							( m_uiWeightClass,				WeightClass )
	PUBLISH_VAR_AS							( m_bEnableImpact,				EnableImpact )
	PUBLISH_VAR_AS							( m_bEnableSlide,				EnableSlide )
	PUBLISH_VAR_AS							( m_bEnableRolling,				EnableRolling )
	PUBLISH_VAR_AS							( m_fImpactMinProjVel,			ImpactMinProjVel )
	PUBLISH_VAR_AS							( m_fImpactMaxProjVel,			ImpactMaxProjVel )
	PUBLISH_VAR_AS							( m_fImpactMinVolume,			ImpactMinVolume )
	PUBLISH_VAR_AS							( m_fImpactMaxVolume,			ImpactMaxVolume )
	PUBLISH_VAR_AS							( m_fImpactMinInterval,			ImpactMinInterval )
	PUBLISH_VAR_AS							( m_fImpactMaxInterval,			ImpactMaxInterval )
	PUBLISH_VAR_AS							( m_fSlideTime,					SlideTime )
	PUBLISH_VAR_AS							( m_fSlideAngleThreshold,		SlideAngleThreshold )
	PUBLISH_VAR_AS							( m_fSlideMinVelocity,			SlideMinVelocity )
	PUBLISH_VAR_AS							( m_fSlideMaxVelocity,			SlideMaxVelocity )
	PUBLISH_VAR_AS							( m_fSlideMinVolume,			SlideMinVolume )
	PUBLISH_VAR_AS							( m_fSlideMaxVolume,			SlideMaxVolume )
	PUBLISH_VAR_AS							( m_fRollTime,					RollTime )
	PUBLISH_VAR_AS							( m_fRollMinAngularVelocity,	RollMinAngularVelocity )
	PUBLISH_VAR_AS							( m_fRollMaxAngularVelocity,	RollMaxAngularVelocity )
	PUBLISH_VAR_AS							( m_fRollMinVolume,				RollMinVolume )
	PUBLISH_VAR_AS							( m_fRollMaxVolume,				RollMaxVolume )
	PUBLISH_VAR_AS							( m_bDebugEnabled,				DebugEnabled )
	DECLARE_POSTCONSTRUCT_CALLBACK			( PostConstruct )
	DECLARE_EDITORCHANGEVALUE_CALLBACK		( EditorChangeValue )
END_STD_INTERFACE

START_STD_INTERFACE							( CollisionEffectDef )
	PUBLISH_VAR_AS							( m_uiCollisionType,		CollisionType )	
	PUBLISH_VAR_AS							( m_uiMaterial1,			Material1 )
	PUBLISH_VAR_AS							( m_uiWeight1,				Weight1 )
	PUBLISH_VAR_AS							( m_uiMaterial2,			Material2 )
	PUBLISH_VAR_AS							( m_uiWeight2,				Weight2 )
	PUBLISH_VAR_AS							( m_obEventGroup,			EventGroup )
	PUBLISH_VAR_AS							( m_obEvent,				Event )
	PUBLISH_VAR_AS							( m_obParticleDef,			ParticleDef )
	DECLARE_POSTCONSTRUCT_CALLBACK			( PostConstruct )
	DECLARE_EDITORCHANGEVALUE_CALLBACK		( EditorChangeValue )
END_STD_INTERFACE



//-----------------------------------------------------------------------------------------------------------------------------------------------------------------

EntityCollisionHandlerDef::EntityCollisionHandlerDef () :
	m_uiMaterialClass(0),
	m_uiWeightClass(0),
	m_bEnableImpact(false),
	m_bEnableSlide(false),
	m_bEnableRolling(false),
	m_fImpactMinProjVel(0.5f),
	m_fImpactMaxProjVel(10.0f),
	m_fImpactMinVolume(0.1f),
	m_fImpactMaxVolume(1.0f),
	m_fImpactMinInterval(0.25f),
	m_fImpactMaxInterval(0.35f),
	m_fSlideTime(0.5f),
	m_fSlideAngleThreshold(0.1f),
	m_fSlideMinVelocity(0.25f),
	m_fSlideMaxVelocity(4.0f),
	m_fSlideMinVolume(0.1f),
	m_fSlideMaxVolume(1.0f),
	m_fRollTime(0.5f),
	m_fRollMinAngularVelocity(0.25f),
	m_fRollMaxAngularVelocity(1.0f),
	m_fRollMinVolume(0.1f),
	m_fRollMaxVolume(1.0f)
{
}

void EntityCollisionHandlerDef::PostConstruct ()
{
	m_bDebugEnabled=false;
}

bool EntityCollisionHandlerDef::EditorChangeValue (CallBackParameter, CallBackParameter)
{
	return true;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------------

EntityCollisionEventHandler::EntityCollisionEventHandler () :
	m_pobCollisionHandlerDef(0),
	m_fImpactInterval(0.0f),
	m_fSlideTime(0.0f),
	m_fRollTime(0.0f),
	m_ulSlideSoundID(0),
	m_ulRollSoundID(0)
{
}

void EntityCollisionEventHandler::SetCollisionEventDefinition (const CHashedString& obName)
{
	m_pobCollisionHandlerDef=ObjectDatabase::Get().GetPointerFromName<EntityCollisionHandlerDef*>(obName);
}

void EntityCollisionEventHandler::Update (float fTimeDelta)
{
	if (!m_pobCollisionHandlerDef)
		return;

	if (m_fImpactInterval>0.0f)
	{
		m_fImpactInterval-=fTimeDelta;

		if (m_fImpactInterval<0.0f)
			m_fImpactInterval=0.0f;
	}

	if (m_pobCollisionHandlerDef->m_bEnableSlide)
	{
		// Get slide time
		
		if (m_ulSlideSoundID)
		{
			m_fSlideTime+=fTimeDelta;
		}

		//AudioSystem::Get().Sound_Stop(m_ulSlideSoundID);
		//m_ulSlideSoundID=0;
	}

	if (m_pobCollisionHandlerDef->m_bEnableRolling)
	{
		// Get roll time

		if (m_ulRollSoundID)
		{
			m_fRollTime+=fTimeDelta;
		}
	}
}


void EntityCollisionEventHandler::ProcessImpactEvent (const CollisionEvent& obEvent)
{
	if (!m_pobCollisionHandlerDef) // Ignore if we don't have valid handler def
		return;

	if (m_pobCollisionHandlerDef->m_bEnableImpact && m_fImpactInterval==0.0f) // Check to see if impacts are enabled and we can
	{
		float fProjVel=fabs(obEvent.m_fProjVel); // Ensure projected velocity is positive value

		if (fProjVel>m_pobCollisionHandlerDef->m_fImpactMinProjVel) // Reject the event if it falls below the min projected velocity threshold
		{
			CollisionEffectDef* pobEffectDef;

			if (obEvent.m_fProjVel<0.0f) // This is a bounce impact
			{
				pobEffectDef=CollisionEffectManager::Get().GetEffectDefinition(BOUNCE,obEvent); // Find an effect def best suited to this event
			}
			else // This is a crash impact
			{
				pobEffectDef=CollisionEffectManager::Get().GetEffectDefinition(CRASH,obEvent);
			}

			if (pobEffectDef) // We have a an appropriate effect for this event
			{
				// Play sound effect
				unsigned int id;

				if (AudioSystem::Get().Sound_Prepare(id,pobEffectDef->m_obEventGroup.GetString(),pobEffectDef->m_obEvent.GetString())) // Play our sound
				{
					// Calculate the volume level for the sound
					if (fProjVel>m_pobCollisionHandlerDef->m_fImpactMaxProjVel)
						fProjVel=m_pobCollisionHandlerDef->m_fImpactMaxProjVel;

					float fFraction=(fProjVel-m_pobCollisionHandlerDef->m_fImpactMinProjVel)/(m_pobCollisionHandlerDef->m_fImpactMaxProjVel-m_pobCollisionHandlerDef->m_fImpactMinProjVel);
					float fVolume=(fFraction * (m_pobCollisionHandlerDef->m_fImpactMaxProjVel-m_pobCollisionHandlerDef->m_fImpactMinProjVel)) + m_pobCollisionHandlerDef->m_fImpactMinProjVel;

					AudioSystem::Get().Sound_SetVolume(id,fVolume);
					AudioSystem::Get().Sound_SetPosition(id,obEvent.m_obColPoint);
					AudioSystem::Get().Sound_Play(id);
				}

				// Spawn particle effect here
			}
		}
	}
}






//-----------------------------------------------------------------------------------------------------------------------------------------------------------------

CollisionEffectDef::CollisionEffectDef () :
	m_uiCollisionType(0),
	m_uiMaterial1(0),
	m_uiWeight1(0),
	m_uiMaterial2(0),
	m_uiWeight2(0),
	m_fScore(0.0f)
{
	CollisionEffectManager::Get().AddEffectDefinition(this);
}

CollisionEffectDef::~CollisionEffectDef ()
{
	if (CollisionEffectManager::Exists())
		CollisionEffectManager::Get().RemoveEffectDefinition(this);
}

void CollisionEffectDef::PostConstruct ()
{
	// Score = 1/(material1 total) + 1/(weight1 total) + 1/(material2 total) + 1/(weight2 total) + 1/(col total)

	float fMaterial1=(float)BitCount(m_uiMaterial1);
	float fWeight1=(float)BitCount(m_uiWeight1);
	float fMaterial2=(float)BitCount(m_uiMaterial2);
	float fWeight2=(float)BitCount(m_uiWeight2);
	float fColType=(float)BitCount(m_uiCollisionType);

	m_fScore=0.0f;

	if (fMaterial1>0.0f)
		m_fScore+=1.0f/fMaterial1;

	if (fWeight1>0.0f)
		m_fScore+=1.0f/fWeight1;

	if (fMaterial2>0.0f)
		m_fScore+=1.0f/fMaterial2;

	if (fWeight2>0.0f)
		m_fScore+=1.0f/fWeight2;

	if (fColType>0.0f)
		m_fScore+=1.0f/fColType;
}

bool CollisionEffectDef::EditorChangeValue (CallBackParameter, CallBackParameter)
{
	PostConstruct(); // Recalculate score if values are altered in wielder

	return true;
}


float CollisionEffectDef::GetScore (unsigned int uiColType,const CollisionEvent& obEvent)
{
	// Test the collision type

	if (m_uiCollisionType==0 || (m_uiCollisionType & uiColType))
	{
		if ( ((m_uiMaterial1==0 || (m_uiMaterial1 & obEvent.m_uiMaterial1)) && (m_uiMaterial2==0 || (m_uiMaterial2 & obEvent.m_uiMaterial2))) || // Check to see if materials match
			((m_uiMaterial1==0 || (m_uiMaterial1 & obEvent.m_uiMaterial2)) && (m_uiMaterial2==0 || (m_uiMaterial2 & obEvent.m_uiMaterial1))) ) // Switch materials and see if they match
		{

			if ( ((m_uiWeight1==0 || (m_uiWeight1 & obEvent.m_uiWeight1)) && (m_uiWeight2==0 || (m_uiWeight2 & obEvent.m_uiWeight2))) || // Check to see if the weights match
				((m_uiWeight1==0 || (m_uiWeight1 & obEvent.m_uiWeight2)) && (m_uiWeight2==0 || (m_uiWeight2 & obEvent.m_uiWeight1))) ) // Switch weights and see if they match
			{
				return m_fScore; // We have a match
			}
		}
	}

	return -1.0f; // This def is not a match
}

int CollisionEffectDef::BitCount (unsigned int n)
{
	// Parallel bit count algorithm

	#define TWO(c) (0x1u << (c))
	#define MASK(c) (((unsigned int)(-1)) / (TWO(TWO(c)) + 1u))
	#define COUNT(x,c) ((x) & MASK(c)) + (((x) >> (TWO(c))) & MASK(c))

	n = COUNT(n, 0);
	n = COUNT(n, 1);
	n = COUNT(n, 2);
	n = COUNT(n, 3);
	n = COUNT(n, 4);
	// n = COUNT(n, 5) ;    //for 64-bit integers 

	return n ;
}



//-----------------------------------------------------------------------------------------------------------------------------------------------------------------


void CollisionEffectManager::AddEffectDefinition (CollisionEffectDef* pobEffectDef)
{
	m_obEffectDefList.push_back(pobEffectDef);		
}

void CollisionEffectManager::RemoveEffectDefinition (CollisionEffectDef* pobEffectDef)
{
	m_obEffectDefList.remove(pobEffectDef);
}

CollisionEffectDef* CollisionEffectManager::GetEffectDefinition (COLLISION_TYPE eType,const CollisionEvent& obEvent)
{
	float fScore=0.0f;
	CollisionEffectDef* pobEffectDef=0;
	unsigned int uiColType=(unsigned int)eType;

	for(ntstd::List<CollisionEffectDef*>::iterator obIt=m_obEffectDefList.begin(); obIt!=m_obEffectDefList.end(); ++obIt)
	{
		float fThisScore=(*obIt)->GetScore(uiColType,obEvent);

		if (fThisScore>fScore)
		{
			fScore=fThisScore;
			pobEffectDef=*obIt;
		}
	}

	return pobEffectDef;
}

*/
