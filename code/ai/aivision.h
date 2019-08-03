//! -------------------------------------------
//! aivision.h
//!
//! AI Vision
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!--------------------------------------------

#ifndef _AIVISION_H
#define _AIVISION_H

// Definitions and macros

#define FAR_FAR_AWAY (999999.0f)	// Some very big distance
#define FACT_CACHE_LIMIT	(8)

// Forward declaration of classes and structures

class CAIComponent;
class CEntity;
class AI;
class Character;

// Enums

enum eVISION_RESULTS
{
	VIS_RESULT_NOTHING_SEEN = 0,
	VIS_RESULT_ENEMY_SEEN,
	VIS_RESULT_SHOOTING_RANGE,
	VIS_RESULT_MELE_RANGE,
	VIS_RESULT_SOMETHING_SEEN,
	VIS_RESULT_GO_AROUND
};

// Structures

typedef struct _SVisionParams
{
	_SVisionParams() :	fCloseDistSQR			(9.0f)			, /*						*/
						fHalfCloseViewAngle		( PI*3/4)		, /*						*/
						fMaxViewDistSQR			(400.0f)		, /* 20 m.					*/
						fHalfViewAngle			( HALF_PI / 3 )	, /* 30 deg. (60 total)		*/
						fOPPMaxViewDistSQR		(625.0f)		, /* 25 m.					*/
						fOPPHalfViewAngle		(HALF_PI/1.5f )	, /* 45 deg. (90 total)	*/
						fInnerRadiusSQR			(9.0f)			, /* 3 m. */
						fVisionYOffset			(1.5f)			, // 1.5 metres (eyes height)
						fShootAimDistSQR		(10000.0f)		, // 100 metres
						fShootAimHalfAngle		( HALF_PI / 9 )	  // 10 deg.		
						{}

	float fCloseDistSQR;
	float fHalfCloseViewAngle;
	float fMaxViewDistSQR;
	float fHalfViewAngle;
	float fOPPMaxViewDistSQR;
	float fOPPHalfViewAngle;		// Other People's Problem View Angle (see The Hitchickers Guide to the Galaxy)
	float fInnerRadiusSQR;
	float fVisionYOffset;
	float fShootAimDistSQR;
	float fShootAimHalfAngle;

} SVisionParams;


//!------------------------------------------------------------------------------
//!  CAIVisibleFact
//!  This is a container for a visible entities, it acts like a cache of visible
//!  entities to owner and should therefore help CPU load.
//!
//!  @author GavB @date 29/06/2006
//!------------------------------------------------------------------------------
class CAIFact
{
public:
	float	CalculateScore		( const AI* pRelativeTo )	const;

	CAIFact(void) : m_pEnt( 0 ) {}

public:
	//!----------------------------------------------------------------------------
	//!  CPoint m_obLastKnownPosition
	//!	 This is the last known position for the entity.
	//!	
	//!  @author GavB @date 29/06/2006
	//!----------------------------------------------------------------------------
	CPoint		m_obLastKnownPosition;

	//!----------------------------------------------------------------------------
	//!  float m_fAge
	//!  The age of the visiable fact, this age is updated each frame
	//!
	//!  @author GavB @date 29/06/2006
	//!----------------------------------------------------------------------------
	float		m_fAge;
	

	//!----------------------------------------------------------------------------
	//!  m_uiVisibilityFlags
	//!
	//!  @author GavB @date 29/06/2006
	//!----------------------------------------------------------------------------
	enum VISIBILITY_FLAGS
	{
		OPP_RANGE				= 1 << 0,
		SHOOT_RANGE				= 1 << 1,
		MAIN_RANGE				= 1 << 2,
		LINE_OF_SIGHT			= 1 << 3,
		CLOSE_RANGE				= 1 << 4,
		INNER_RANGE				= 1 << 5,
		LAST_VISIBILITY_FLAG,

	};

	enum VISIBILITY_MASK
	{
		CONFIRMED_SIGHTING		= LINE_OF_SIGHT | MAIN_RANGE,
		PRIORITY_MASK			= LAST_VISIBILITY_FLAG - 2,
	};

	u_int		m_uiVisibilityFlags;


	//!----------------------------------------------------------------------------
	//!  const CEntity * m_pEnt
	//!  Just a pointer to the entity instance. 
	//!
	//!  @author GavB @date 29/06/2006
	//!----------------------------------------------------------------------------
	const Character* m_pEnt;
};

//!--------------------------------------------
//! CAIVision
//!--------------------------------------------
class CAIVision
{
	public:

		// Ctor...
		CAIVision();

		// Get Information

		bool			DoISeeEnemyThroughGoAroundVolumes ( void ) const { return ( m_Active && (m_VisionResults==VIS_RESULT_GO_AROUND) ); }

		bool			DoISeeTheEnemy			( void ) const	{ return ( m_Active && (m_VisionResults==VIS_RESULT_ENEMY_SEEN) ); }
		bool			DoISeeSomething			( void ) const	{ return ( m_Active && (m_VisionResults==VIS_RESULT_SOMETHING_SEEN) ); }
		bool			DoISeeNothing			( void ) const	{ return ( m_Active && (m_VisionResults==VIS_RESULT_NOTHING_SEEN) ); }
		bool			HaveIEverSeenTheEnemy	( void ) const	{ return ( m_Active && m_bEverSeenEnemy ); }
		float			GetDist2EnemySQR		( void ) const	{ return m_fDist2EnemySQR; }
		CPoint			GetLastKnowEnemyPos		( void ) const	{ return m_LastKnownEnemyPos; }
		bool			IsTargetInAttackRange	( void ) const;
		bool			IsTargetInShootingRange	( void ) const	{ return ( m_Active && (m_VisionResults==VIS_RESULT_SHOOTING_RANGE) ); }
		bool			IsTargetInMeleRange		( void ) const	{ return ( m_Active && (m_VisionResults==VIS_RESULT_MELE_RANGE) ); }
		bool			DoISeeMyTarget			( void ) const	{ return (DoISeeTheEnemy()); }
		float			GetEyeLevel				( void ) const	{ return m_Params.fVisionYOffset; }

		const SVisionParams*	GetParams		( void ) const	{ return &m_Params; }

		// Activation

		bool			IsActive				( void ) const	{ return m_Active; }
		void			Activete				( void )		{ m_Active = true; }
		void			Deactivate				( void )		{ m_Active = false;}
		void			SetVision				( bool b )		{ m_Active = b;}

		// Set Parameters
		void			SetVisionParam			( unsigned int uiParam, float fValue);
		void			SetMaxViewDistSQR		( float f ) { if (f > 0.0f) m_Params.fMaxViewDistSQR = f; }
		void			SetHalfViewAngle		( float f ) { if (f > 0.0f) m_Params.fHalfViewAngle = f; }
		void			SetOPPMaxViewDistSQR	( float f ) { if (f > 0.0f) m_Params.fOPPMaxViewDistSQR = f; }
		void			SetOPPHalfViewAngle		( float f ) { if (f > 0.0f) m_Params.fOPPHalfViewAngle = f; }
		void			SetShootingDistance		( float f ) { if (f > 0.0f) m_Params.fShootAimDistSQR = f*f; }
		void			SetShootingAngle		( float f ) { if (f > 0.0f) m_Params.fShootAimHalfAngle = f/2; }

		// Parent, Player and AIComponent Accessors
		void			SetParent	( CEntity*		pEnt);
//		void			SetPlayer	( CEntity*		pEnt)	{ if (pEnt) { m_pPlayer		= pEnt; } }
		void			SetCAIComp	( CAIComponent* pCAI)	{ if (pCAI) { m_pCAIComp	= pCAI; } }
		void			SetTarget	( const CEntity*pTarg)	{ m_pTarget = pTarg; }

		AI*				GetParent	( void )	const		{ return m_pEnt;					}
//		CEntity*		GetPlayer	( void )	const		{ return m_pPlayer;					}
		CAIComponent*	GetCAIComp	( void )	const		{ return m_pCAIComp;				}
		const CEntity*	GetTarget	( void)		const		{ return m_pTarget;					}

		bool			AreAIsInShootingRange ( void );

		

		// Update

		void	Update		( float );


		// Return a good entity for the AI to target. 

		const CEntity* GetBestAttackTarget(void) const;

		// Return a fact known for the given entity
		const CAIFact* GetFact( const CEntity* pEnt ) const;

		// Forces a vision fact scan on the next update. 
		void ForceVisionScanNextUpdate(void);


		// Debug

		void		DebugRender	( void ) const;
		static void DrawViewCone( AI*, unsigned int );

	private:
		
		// Generate an internal list of entities that can be seen

		void		GenerateListOfVisibleEnemies(void);

		void		UpdateFact( const CAIFact& );

	private:
		
		SVisionParams	m_Params;

		CPoint			m_LastKnownEnemyPos;

		AI*				m_pEnt;			// AI Bot
		const CEntity*	m_pTarget;		// Enemy
		CAIComponent*	m_pCAIComp;		// Pointer to my parent CAIComponent
		float			m_fDist2EnemySQR;
		bool			m_Active;
		bool			m_bDoIScanView;	
		bool			m_bAlerted;		// !!! - (Dario) Neeed?
		bool			m_bEverSeenEnemy;	// !!! - (Dario) Needed?
		eVISION_RESULTS m_VisionResults;

		u_int			m_EnemyType;

		// If the entity has specified an update time for the vision then 
		// this counter keeps track of the current time
		float			m_VisionUpdateTime;


		// fact cache.
		typedef ntstd::Vector<CAIFact> TFactCache;
		TFactCache m_Facts;
};

#endif //_AIVISION_H


