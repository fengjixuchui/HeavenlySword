//---------------------------------------------------------------------------------------------------------
//!
//!	\file physics/projectilelg.h
//!	
//!	DYNAMICS COMPONENT:
//!		Logic groups supporting the old CDynamicsStates.
//!
//!	Author: Mustapha Bismi (mustapha.bismi@ninjatheory.com)
//! Created: 2005.08.06
//!
//---------------------------------------------------------------------------------------------------------

#ifndef _DYNAMICS_PROJECTILE_LG_INC
#define _DYNAMICS_PROJECTILE_LG_INC

#include "config.h"

#include "logicgroup.h"

#include "editable/enumlist.h"
#include "physics/world.h"

class Transform;
struct Projectile_Data;

// ---------------------------------------------------------------
//	Properties that define the behaviour of a projectile.
// ---------------------------------------------------------------

class ProjectileProperties
{
public:
	typedef enum CollideWithCharactersType
	{
		NOT_COLLIDE = 0,
		COLLIDE_INEXACT = 1,
		COLLIDE_EXACT = 2
	};

	ProjectileProperties ():
	m_eMotionType(MOTION_LINEAR),
	m_fInitialSpeed(0.0f),
	m_fAcceleration(1.0f),
	m_fAccelerationTime(5.0f),
	m_fRicochetProbability(0.0f),
	m_fRicochetEnergyLoss(0.1f),
	m_fMass(1.0f),
	m_fGravity(0.0f),
	m_fPenetrability(1.0f), 
	m_fLinearDamping ( 0.0f ),
	m_fAftertouchEnterSpeedClamp( FLT_MAX ),
	m_fSplineInterval(1.0f),
	m_fSplineInitialRadius(0.0f),
	m_fSplineMaxRadius(0.0f),
	m_fSplineRadiusIncreaseRate(0.0f),
	m_bNoCollide(false),
	m_CollideWithCharacters( COLLIDE_INEXACT ),
	m_bCollideWithStatic( true ),
	m_bLinearSpin(false),
	m_obLocalAxisToFace( CONSTRUCT_CLEAR )
	{};

	PROJECTILE_MOTION_TYPE	m_eMotionType;

	// For linear movement
	float 					m_fInitialSpeed;
	float 					m_fAcceleration;
	float 					m_fAccelerationTime;
	float 					m_fRicochetProbability;
	float 					m_fRicochetEnergyLoss;
	float 					m_fMass;
	float					m_fGravity;
	float					m_fPenetrability; 
	float					m_fLinearDamping;
	float					m_fAftertouchEnterSpeedClamp;

	// For spline movement
	float					m_fSplineInterval;
	float					m_fSplineInitialRadius;
	float					m_fSplineMaxRadius;
	float					m_fSplineRadiusIncreaseRate;

	// For vector lerp movement
	CDirection				m_obParentRelativeStartDirection;
	float					m_fPercentageDistanceToLerpOver;
	float					m_fMaxHomingSpeed;
	float					m_fSpinSpeed;

	bool					m_bNoCollide;
	CollideWithCharactersType m_CollideWithCharacters; // 0 - not collide, 1 - collide with capsule, 2 - collide with ragdoll
	bool					m_bCollideWithStatic;
        bool                                    m_bLinearSpin;
	
	CDirection				m_obLocalAxisToFace;
};
 

// ---------------------------------------------------------------
//	Bazooka proeprties
// ---------------------------------------------------------------
class BazookaProjectileProperties : public ProjectileProperties
{
public:

	BazookaProjectileProperties ():
	m_fDropOffAcceleration(-9.81f),
	m_fAftertouchDropThreshold(0.15f),
	m_bUseLODEffects(0)
	{};

	// Drop Off Acceleration - typically a gravity sort of value
	float					m_fDropOffAcceleration;

	// Amount of time after going into aftertouch that must have passed for the instant drop to occur
	float					m_fAftertouchDropThreshold;

	// Times at which the individual rockets get the drop off acceleration applied vertically.
	ntstd::List<float>		m_DropOffTimeList;

	// Does this rocket use LOD effects and explosion?
	bool					m_bUseLODEffects;
};

// ---------------------------------------------------------------
//	Projectile raycast filter based on its properties 
// ---------------------------------------------------------------
class ProjectilePropertiesRaycastFilter : public CastRayFilter
{
public:
	ProjectilePropertiesRaycastFilter(ProjectileProperties * pobProp) 
		: m_pobProperties(pobProp)  {};

	bool operator() (CEntity *pobEntity) const
	{					
		if (!m_pobProperties)
			return true;
		if (pobEntity->IsCharacter())
			return m_pobProperties->m_CollideWithCharacters != ProjectileProperties::NOT_COLLIDE;
		else
			return m_pobProperties->m_bCollideWithStatic && (pobEntity->GetEntType() & CEntity::EntType_Static || pobEntity->GetEntType() & CEntity::EntType_Interactable);			
	}
protected:
	ProjectileProperties * m_pobProperties;
};


namespace Physics
{
	class SplineBuilder
	{
		public:
		
			SplineBuilder ();

			~SplineBuilder ();

			void Update (); // Update the spline

			void AddPoint (const CPoint& obNewPoint) // Adds a new point to the curve
			{
				m_obNextPos=obNewPoint;
			}

			void SetPoints (int iIndex,const CPoint& obP1,const CPoint& obP2,const CPoint& obP3,const CPoint& obP4)
			{
				m_fT=0.0f;
				m_iIndex=iIndex;
				m_aobPoint[0]=obP1;
				m_aobPoint[1]=obP2;
				m_aobPoint[2]=obP3;
				m_aobPoint[3]=obP4;
			}

			void SetPoints (const CPoint& obPosition) // Reset all points to specified position
			{
				for(int iCount=0; iCount<4; iCount++)
					m_aobPoint[iCount]=obPosition;
			}

			void SetVelocity (const CPoint& obVelocity) // If the entire curve is moving, its velocity may be set here
			{
				m_obVelocity=obVelocity;
			}
			
			void SetStep (float fStep) // Set rate of movement along the spline 0->1
			{
				m_fTStep=fStep;
			}

			void SetOffset (float fOffset) // Generates a random offset from the point fed in
			{
				m_fOffset=fOffset;
			}

			const CPoint& GetPosition () // Returns current position on curve
			{
				return m_obPosition;
			}

			float GetStep () const
			{
				return m_fTStep;
			}

			float GetOffset () const
			{
				return m_fOffset;
			}

		private:

			CPoint		m_obPosition;		// Current position along the spline
			CPoint		m_obVelocity;		// This is set if the curve is moving

			CPoint		m_obNextPos;		// Next point in spline
			CPoint		m_aobPoint [4];		// Four points needed to define a spline
			
			float		m_fOffset;			// A random offset from the point fed in (this value represents halfwidth)
			float		m_fT;				// movement along spline 0->1
			float		m_fTStep;			// speed to travel spline 0->1
			int			m_iIndex;			// Point index 0->3
	};

	class ProjectileManager : public Singleton<ProjectileManager>
	{
		public:

			ProjectileManager() {}
			~ProjectileManager();

			void AddStaticProjectile(CEntity* pobEntity);
			void RemoveChildProjectiles(CEntity* pobParent);

		private:

			ntstd::List<CEntity*> m_obStaticProjectileList;
	};

	

	// ---------------------------------------------------------------
	//	Projectile are specific components defined by a raycast.
	// ---------------------------------------------------------------
	class ProjectileLG : public LogicGroup
	{
	private:

		bool						m_bForceStraightVectorLerp;
		bool						m_bFreeze;
		float						m_fSpinOffset;
		CDirection					m_obSpinAxis;
		float						m_fTimeToLerp;
		CDirection					m_obLastToTarget, m_obLastVelocity;
		CDirection					m_obStartDirection;
		Projectile_Data*			m_pobData;

		ProjectileProperties*		m_pobProperties;
		CDirection					m_obVelocity;
		CDirection					m_obThrustDirection; // Propelled direction
		float						m_fSplineOffsetRadius;
		Transform*					m_pobRoot;
		bool						m_bMoving;
		bool						m_bFirstFrame;
		float						m_fTime;
		float						m_fFallTime;
		float						m_fFallAcceleration;
		float						m_AddToInitialSpeed;
		SplineBuilder				m_obSplineBuilder;
		bool						m_bHasRicocheted;
		bool						m_bStopMovingOnCollision;
		bool						m_bInAftertouch;

	public:

									ProjectileLG( const ntstd::String& p_name, CEntity* p_entity );
		virtual						~ProjectileLG( );

		virtual const GROUP_TYPE	GetType( ) const;
		virtual void				Update( const float p_timestep );
		virtual void				Activate( bool activateInHavok = false);
		virtual void				Deactivate( );

		virtual RigidBody*			AddRigidBody( const BodyCInfo* p_info );

		static  ProjectileLG*		Construct( CEntity*, ProjectileProperties*, CPoint&, CDirection&, float fFallTime = 0.0f, float fFallAcceleration = 0.0f, Projectile_Data* pobData = 0);
		void						Initialise( ProjectileProperties* pobProperties, const CPoint& obPosition, const CDirection& obDirection, float fFallTime, float fFallAcceleration, Projectile_Data* pobData = 0);
		void						StopMovingOnCollision( bool bStopMovingOnCollision ) { m_bStopMovingOnCollision = bStopMovingOnCollision; }

		virtual void				ApplyLocalisedLinearImpulse( const CDirection& p_vel, const CVector& p_point );
		virtual CDirection			GetLinearVelocity( ) { return m_obVelocity; }
		virtual void				SetLinearVelocity( const CDirection& p_linearVelocity ) { m_obVelocity = p_linearVelocity; };

		void						SetSplineRadius (float fRadius) { m_fSplineOffsetRadius=fRadius; }
		bool						IsMoving () const		{ return m_bMoving; }
		float						GetMass () const		{ return m_pobProperties->m_fMass; }
		float						GetStateTime () const	{ return m_fTime; }
		ProjectileProperties*		GetProperties() { return m_pobProperties; }

		float						GetFallTime() const		{ return m_fFallTime; }
		void						SetFallTime( float fNewFallTime )	{ m_fFallTime = fNewFallTime; }

		CDirection					GetThrustDirection( ) { return m_obThrustDirection; }
		void						SetThrustDirection( CDirection& p_dir ) { m_obThrustDirection = p_dir; }

		// Duncan5 1337 h4ck5
		void						SetMoving(bool bMoving);
		void						Reset( Projectile_Data* pobData );
		bool						HasMissedTarget(float fByAngle);
		void						SetFrozen(bool bFreeze)  { m_bFreeze = bFreeze; };
		bool						GetFrozen()  { return m_bFreeze; };
		void						SetStraightVectorLerp(bool bForceStraight) { m_bForceStraightVectorLerp = bForceStraight; };

		// Allow entities to modify their initial speed of the arrow
		void						AddToInitialSpeed( float fValue ) { m_AddToInitialSpeed = fValue; }
	};
}

#endif // _DYNAMICS_PROJECTILE_LG_INC
