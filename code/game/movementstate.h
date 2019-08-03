/***************************************************************************************************
*
*       DESCRIPTION	
*
*       NOTES          
*
***************************************************************************************************/

#ifndef	_MOVEMENTSTATE_H
#define	_MOVEMENTSTATE_H

// Forward declarations
class CMovement;

/***************************************************************************************************
*	
*	CLASS			CMovementState
*
*	DESCRIPTION		This represents how the current entity is moving and what their environment
*					is like e.g. what are they standing on etc.
*
***************************************************************************************************/

class CMovementState
{
public:

	// Construction
	CMovementState( void );
	
	// Assignment
	CMovementState& operator = ( const CMovementState& obState );

	// Movement properties
	CDirection				m_obProceduralRootDelta;			// The extra delta along the facing direction ( + to anim )
	float					m_fProceduralYaw;					// The procedural rotation about the local Y axis			
	float					m_fProceduralPitch;					// The procedural rotation about the local X axis
	float					m_fProceduralRoll;					// The procedural rotation about the local Z axis
	CDirection				m_obRootDeltaScalar;				// Used for targeted movement
	float					m_fRootRotationDeltaScalar;			// Used by some controllers to eliminate animation rotation
	bool					m_bApplyExplicitRotations;			// When set to true, procedural rotations are applied explicitly rather than as deltas
};


/***************************************************************************************************
*	
*	CLASS			CMovementStateRef
*
*	DESCRIPTION		This represents how the current entity is moving and what their environment
*					is like e.g. what are they standing on etc.
*
***************************************************************************************************/

class CMovementStateRef
{
public:

	// Construction
	CMovementStateRef( void );
	
	// Assignment
	CMovementStateRef& operator = ( const CMovementStateRef& obState );

	// Movement properties
	CDirection				m_obLastRequestedVelocity;			// The requested value - not necessarily what dynamics managed
	CDirection				m_obLastRequestedAcceleration;		// The requested value - not necessarily what dynamics managed
	CMatrix					m_obRootMatrix;						// Used a lot - obvious
	CDirection				m_obFacing;							// Used a lot too
	CQuat					m_obOrientation;					// Used only on the walking controller
	CPoint					m_obPosition;						// Used a lot
	bool					m_bOnGround;						// Used in lots of controllers - probably should be accessed from a controller directly in dynamics

};

#endif //_MOVEMENTSTATE_H
