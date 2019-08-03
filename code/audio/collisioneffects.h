/*
#ifndef _COLLISIONEFFECTS_H
#define _COLLISIONEFFECTS_H



#define MAX_IMPACT_EVENTS			10
#define MAX_SLIDING_EVENTS			3
#define MAX_ROLLING_EVENTS			3
#define BASE_PROJVEL_REJECT			0.5f	// Base minimum projected velocity rejection




enum VOLUME_WEIGHT
{
	ANY_WEIGHT		=	0,			// This has the lowest rating
	LIGHT			=	1,
	MEDIUM			=	2,
	HEAVY			=	4,
	WORLD			=	8,
};

enum VOLUME_MATERIAL
{
	ANY_MATERIAL	=	0,			// This has the lowest rating
	FLESH			=	1,
	STONE			=	2,
	METAL_THIN		=	4,
	METAL_HEAVY		=	8,
};

enum COLLISION_TYPE
{
	ANY_COLLISION	=	0,
	BOUNCE			=	1,			// An impact with a negative projected velocity
	CRASH			=	2,			// An impact with a position projected velocity
	SLIDING			=	4,			// Movement perpendicular to its contact surface
	ROLLING			=	8,			// Angular movement whilst making contact with a surface
};








//------------------------------------------------------------------------------------------
//!
//! CollisionEvent
//!	This is represents a collision thats happened in the world.
//!
//------------------------------------------------------------------------------------------
class CollisionEvent
{
public:

	CollisionEvent () {}

	unsigned int 	m_uiMaterial1;
	unsigned int 	m_uiWeight1;
	unsigned int 	m_uiMaterial2;
	unsigned int 	m_uiWeight2;
	
	CPoint			m_obColPoint;
	float			m_fProjVel;
};






//------------------------------------------------------------------------------------------
//!
//! EntityCollisionHandlerDef
//!	This defines how collision events are handled for an entity.
//!
//------------------------------------------------------------------------------------------
class EntityCollisionHandlerDef
{
public:

	EntityCollisionHandlerDef ();

	void PostConstruct ();
	bool EditorChangeValue (CallBackParameter, CallBackParameter);

	// ----- Serialised members -----

	unsigned int	m_uiMaterialClass;			// Base material classification
	unsigned int	m_uiWeightClass;			// Base weight classification

	bool			m_bEnableImpact;
	bool			m_bEnableSlide;
	bool			m_bEnableRolling;

	float			m_fImpactMinProjVel;		// Minimum projected velocity for an impact effect
	float			m_fImpactMaxProjVel;		// Maximum projected velocity for an impact effect
	float			m_fImpactMinVolume;			// Minimum volume (scales from min to max projected velocity)
	float			m_fImpactMaxVolume;			// Maximum volume (scales from min to max projected velocity)
	float			m_fImpactMinInterval;		// The minimum time between impacts
	float			m_fImpactMaxInterval;		// The maximum time between impacts

	float			m_fSlideTime;				// Minimum slide time before slide effect becomes active
	float			m_fSlideAngleThreshold;		// Angle threshold that the object must be moving relative to the surface its sliding on
	float			m_fSlideMinVelocity;		// Min velocity the object must be travelling at before slide is triggered
	float			m_fSlideMaxVelocity;		// Max velocity the object should be travelling at before sound volume maxes out
	float			m_fSlideMinVolume;			// Min sound volume for slide (scales from min to max velocity)
	float			m_fSlideMaxVolume;			// Max sound volume for slide (scales from min to max velocity)

	float			m_fRollTime;				// Minimum rolling time before roll effect becomes active
	float			m_fRollMinAngularVelocity;	// Minimum angular velocity the object must have before this effect can be triggered
	float			m_fRollMaxAngularVelocity;	// Maximum angular velocity the object must have before the sound volume is maxed out
	float			m_fRollMinVolume;			// Min sound volume for roll (scales from min to max angular velocity)
	float			m_fRollMaxVolume;			// Max sound volume for roll (scales from min to max angular velocity)

	bool			m_bDebugEnabled;
};







//------------------------------------------------------------------------------------------
//!
//! EntityCollisionEventHandler
//!	Deals with impacts, sliding and rolling type collisions for an entity - and triggers
//! the appropriate effects.
//!
//------------------------------------------------------------------------------------------
class EntityCollisionEventHandler
{
public:

	EntityCollisionEventHandler ();

	void SetCollisionEventDefinition (const CHashedString& obName);

	void Update (float fTimeDelta);

	void ProcessImpactEvent (const CollisionEvent& obEvent);


private:

	EntityCollisionHandlerDef* m_pobCollisionHandlerDef;

	float			m_fImpactInterval;		// Impact interval
	float			m_fSlideTime;			// Time duration this object has been sliding for
	float			m_fRollTime;			// Time duration this object has been rolling for
	
	unsigned long 	m_ulSlideSoundID;		// ID for slide sound
	unsigned long 	m_ulRollSoundID;		// ID for roll sound
};
















//------------------------------------------------------------------------------------------
//!
//! CollisionEffectDef
//!	This is a serialised object that links a particular type of collision with a sound and
//! particle effect.
//!
//------------------------------------------------------------------------------------------
class CollisionEffectDef
{
public: 

	CollisionEffectDef ();
	~CollisionEffectDef ();

	void PostConstruct ();
	bool EditorChangeValue (CallBackParameter, CallBackParameter);

	float GetScore (unsigned int uiColType,const CollisionEvent& obEvent);

	// ----- Serialised data -----

	unsigned int		m_uiCollisionType;
	unsigned int		m_uiMaterial1;
	unsigned int		m_uiWeight1;
	unsigned int		m_uiMaterial2;
	unsigned int		m_uiWeight2;
	CKeyString			m_obEventGroup;
	CKeyString			m_obEvent;
	CKeyString			m_obParticleDef;

private:

	int BitCount (unsigned int n);
	
	float m_fScore; // Score represents the relevance rating and enables the most approriate definition to be selected for a collision event
};

//------------------------------------------------------------------------------------------
//!
//! CollisionEffectManager
//!	This singleton just acts as a means of storage for effect defs. It also provides an
//! interface to retrieve them based on a collision event.
//!
//------------------------------------------------------------------------------------------
class CollisionEffectManager : public Singleton<CollisionEffectManager>
{
public:

	CollisionEffectManager () {}

	void AddEffectDefinition (CollisionEffectDef* pobEffectDef);
	void RemoveEffectDefinition (CollisionEffectDef* pobEffectDef);

	CollisionEffectDef* GetEffectDefinition (COLLISION_TYPE eType,const CollisionEvent& obEvent);

private:

	ntstd::List<CollisionEffectDef*> m_obEffectDefList; // List of serialised definitions
};




#endif // _COLLISIONEFFECTS_H
*/
