//--------------------------------------------------
//!
//!	\file LookAtConstraint.h
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#ifndef _LookAtConstraint_H_
#define _LookAtConstraint_H_

#include "physics/havokincludes.h"
#include "anim/characterboneid.h"
#include "game/luaglobal.h"
#include <hkmath/hkmath.h>

class LookAtConstraint;
class LookAtConstraint;
class Transform;
class CEntity;
class CHierarchy;

void draw_cone( const CMatrix& world, const CPoint& pos, const CDirection& fwd, float angle, float viewDist, uint32_t colour, bool draw_sidelines = false );
void draw_dir( const CPoint& obPosition, const CDirection& obFwdDir, float fViewDist, unsigned int uColour );


struct LookAtConstraintDef
{
	HAS_INTERFACE(LookAtConstraintDef)

	CDirection					m_obFwdDirLS;				//!< forwad direction in local (bone) space				
	float						m_fLimitAngle;				//!< rotation angle limit ( around the fwd dir )
	float						m_fWeight;					//!< the blend ammount for the solved local matrix
	float						m_fSpeed;					//!< the look-at "speed"
};


//--------------------------------------------------
//!
//!	Single LookAtConstraint
//!	Chained constraint for a single bone.
//! The solver goes from the leaf transform to 
//! the associated parent (if any).
//!
//--------------------------------------------------
class LookAtConstraint
{ 
public:
	LookAtConstraint();
	LookAtConstraint( Transform* pobTrans, const LookAtConstraintDef* pobDef );

	//! applies this constraint
	bool		Apply( const Transform* pobTarget, float fTimeStep ); 

	//! Resets the previous look-at direction to it's default (fwd state)
	void		Reset( void );

	void		SetAffectedTransform( Transform* pobTrans );

	void		DebugRender( void );

	enum LookAtFlags
	{
		//kLookAt_NoEntityInstalled	= ( 1 << 0 ),		//!< no valid entity installed. Cannot update
		kLookAt_TransformSet				= ( 1 << 1 ),		//!< transform is set
		kLookAt_TargetSet					= ( 1 << 2 ),		//!< has a valid target
		kLookAt_TargetInsideLimits			= ( 1 << 3 ),		//!< current target is visible AND inside the limiting cone
		kLookAt_TargetLocked				= ( 1 << 4 ),		//!< we are looking exactly at the target (within certain error threshold of course)

		kLookAt_AppliedSinceDebugRender		= ( 1 << 8 ),		//!< i was applied this frame
	};

	int		GetFlags		()					const		{ return m_iFlags; }
	void	SetFlags		( int iFlags )					{ m_iFlags = iFlags; }
	void	SetFlagBits		( int iFlagBits )				{ m_iFlags |= iFlagBits; }
	void	ClearFlagBits	( int iFlagBits )				{ m_iFlags &= ~iFlagBits; }

public:
	
	const LookAtConstraintDef*	m_pobDef;					//!< the soft definiton
	Transform*					m_pobTrans;					//!< the affected transform;
	CDirection					m_obPrevLookAtDirWS;		//!< the previous look-at dir. Necessary for interpolation
	mutable int					m_iFlags;					//!< state flags
};

#endif // end of _LookAtConstraint_H_

//eof

