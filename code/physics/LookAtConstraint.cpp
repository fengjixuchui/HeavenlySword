//--------------------------------------------------
//!
//!	\file AimConstraint.cpp
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#include "physics/LookAtConstraint.h"
#include "physics/LookAtComponent.h"
#include "physics/hierarchy_tools.h"
#include "physics/maths_tools.h"

#include "objectdatabase/dataobject.h"
#include "anim/transform.h"
#include "anim/hierarchy.h"
#include "game/entity.h"
#include "game/entitymanager.h"



#include "core/visualdebugger.h"

using namespace Physics;



static const CDirection			DEFAULT_FWD_DIR( 0, 0.3f, 1 );
static const float				DEFAULT_L_ANGLE				= 30.0f;
static const float				DEFAULT_SPEED				= 0.02f;
static const float				DEFAULT_WEIGHT				= 1.0f;

static const float DEBUG_VIEWDIST =	0.3f;
static const CDirection NULL_DIRECTION( CONSTRUCT_CLEAR );

static const int HASH_STR_ENTITY				= 0xf67bbea1;
static const int HASH_STR_TARGET				= 0xbe3cd535;
static const int HASH_STR_TARGETBONE			= 0xa7add769;
static const int HASH_STR_BONE					= 0x6a8befb6;
static const int HASH_STR_DEPTH					= 0xc49dcc92;
static const int HASH_STR_ENABLED				= 0x60bb7de7;
static const int HASH_STR_CONSTRAINTLINKS		= 0x2b8208e4;




START_STD_INTERFACE( LookAtConstraintDef )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_obFwdDirLS,		DEFAULT_FWD_DIR,	FwdDir )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fLimitAngle,		DEFAULT_L_ANGLE,	LimitAngle )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fWeight,			DEFAULT_WEIGHT,		Weight )
	PUBLISH_VAR_WITH_DEFAULT_AS( m_fSpeed,			DEFAULT_SPEED,		Speed )	
END_STD_INTERFACE





LookAtConstraint::LookAtConstraint()
: m_pobDef( 0 )
, m_pobTrans( 0 )
, m_obPrevLookAtDirWS( CONSTRUCT_CLEAR )
, m_iFlags( 0 )
{
	// nothing to do here	
}


LookAtConstraint::LookAtConstraint( Transform* pobTrans, const LookAtConstraintDef* pobDef )
: m_pobDef( pobDef )
, m_pobTrans( 0 )
, m_obPrevLookAtDirWS( CONSTRUCT_CLEAR )
, m_iFlags( 0 )
{
	ntError( m_pobDef );
	SetAffectedTransform( pobTrans );

	m_obPrevLookAtDirWS = m_pobDef->m_obFwdDirLS * m_pobTrans->GetWorldMatrix();
}

//! TODO_OZZ: refactor all this as visibility is now checked only once in the owner component
bool LookAtConstraint::Apply( const Transform* pobTarget, float fTimeStep  )
{
	ntError_p( GetFlags() & kLookAt_TransformSet, ("No transform set for this look-at constraint\n") );

	ClearFlagBits( kLookAt_TargetInsideLimits | kLookAt_TargetSet | kLookAt_TargetLocked );
	SetFlagBits( kLookAt_AppliedSinceDebugRender );
	
	//! get the transform information
	const CMatrix&	localToWorld = m_pobTrans->GetWorldMatrix();
	const CMatrix	worldToLocal = localToWorld.GetAffineInverse();
	
	//! we start by looking in the (animated) bone fwd direction
	CDirection fwdDirWS( m_pobDef->m_obFwdDirLS * localToWorld );
	fwdDirWS.Normalise();
	CDirection lookAtDirWS( fwdDirWS );
	
	//! if we have a target, see if it is within our field of view and set it as the new would be
	//! look-at direction
	if ( pobTarget )
	{
		const CDirection targetDirWS( pobTarget->GetWorldTranslation() - localToWorld.GetTranslation() );
		//const float distanceToTarget = targetDirWS.Length();
		//if ( fwdDirWS.Dot( targetDirWS ) > cos( m_fFOV * DEG_TO_RAD_VALUE ) && distanceToTarget < m_fDistance )
		{
			lookAtDirWS = targetDirWS;
			lookAtDirWS.Normalise();
			SetFlagBits( kLookAt_TargetSet );
		}
	}

	//! now, we start interpolating towards our new look-at direction
	UNUSED( fTimeStep );
	m_obPrevLookAtDirWS = CDirection::Lerp( m_obPrevLookAtDirWS, lookAtDirWS, m_pobDef->m_fSpeed );
	m_obPrevLookAtDirWS.Normalise();

	CDirection rotAxis( CONSTRUCT_CLEAR );
	float rotAngle = rotAxis.CalculateRotationAxis( fwdDirWS, m_obPrevLookAtDirWS );
	float angleAbsLimit = m_pobDef->m_fLimitAngle * DEG_TO_RAD_VALUE;

	// set our state flags
	if ( m_iFlags & kLookAt_TargetSet )
	{
		m_iFlags |= (abs(rotAngle) < angleAbsLimit) ? kLookAt_TargetInsideLimits : 0;
		m_iFlags |= (abs(rotAngle) < EPSILON) ? kLookAt_TargetLocked : 0;
	}

	// limit our total rotation to match our limit
	rotAngle = ntstd::Clamp( rotAngle, -angleAbsLimit, angleAbsLimit );
	
	const CMatrix rotMatrix( rotAxis, rotAngle );
	const CMatrix rotatedWorldMatrix( rotMatrix * localToWorld  ); 

	m_pobTrans->SetLocalMatrixFromWorldMatrix( CMatrix::Lerp(localToWorld,rotatedWorldMatrix, m_pobDef->m_fWeight) );

	// propagate change down the chain
	m_pobTrans->Resynchronise();
	//if ( m_pobTrans->GetParent() )
 //       m_pobTrans->GetParent()->Resynchronise();

	return ( m_iFlags & kLookAt_TargetInsideLimits );
}


void LookAtConstraint::Reset( void )
{
	ClearFlagBits( 0xffffffff ^ kLookAt_TransformSet );
	if ( m_pobTrans )
	{
		m_obPrevLookAtDirWS = m_pobDef->m_obFwdDirLS * m_pobTrans->GetWorldMatrix();
	}
	else
	{
		m_obPrevLookAtDirWS.Clear();
	}

	//m_pobTrans = 0;
}

void LookAtConstraint::SetAffectedTransform( Transform* pobTrans )
{
	ntError( pobTrans );

	if ( pobTrans != m_pobTrans )
	{
		m_pobTrans = pobTrans;
		Reset();
		SetFlagBits( kLookAt_TransformSet );
	}
}

void LookAtConstraint::DebugRender( void )
{	
	if ( GetFlags() & kLookAt_TransformSet )
	{
		const Transform* pBoneTransform = m_pobTrans;
		const CMatrix& boneToWorld( pBoneTransform->GetWorldMatrix() );
		CMatrix worldToBone( boneToWorld.GetAffineInverse() );

		CPoint obBonePosWS( boneToWorld.GetTranslation() );
		CDirection obFwdDirWS = m_pobDef->m_obFwdDirLS * boneToWorld;

		uint32_t colour = DC_BLACK;
		if ( GetFlags() & kLookAt_AppliedSinceDebugRender )		colour = DC_RED;
		if ( GetFlags() & kLookAt_TargetSet )					colour = DC_YELLOW;
		if ( GetFlags() & kLookAt_TargetInsideLimits )			colour = DC_GREEN;
		if ( GetFlags() & kLookAt_TargetLocked )				colour = DC_WHITE;

		draw_dir( obBonePosWS, m_obPrevLookAtDirWS, DEBUG_VIEWDIST, colour );
		draw_dir( obBonePosWS, obFwdDirWS, DEBUG_VIEWDIST * 0.6f, DC_BLUE );
		draw_cone( boneToWorld, obBonePosWS * worldToBone, m_pobDef->m_obFwdDirLS, m_pobDef->m_fLimitAngle, DEBUG_VIEWDIST, colour );
	}

	ClearFlagBits( kLookAt_AppliedSinceDebugRender );
}


void draw_cone( const CMatrix& world, const CPoint& pos, const CDirection& fwd, float angle, float viewDist, uint32_t colour, bool draw_sidelines )
{
#ifndef _GOLD_MASTER

	const int steps = 3;

	angle *= DEG_TO_RAD_VALUE;

	CMatrix localMatrix( CONSTRUCT_CLEAR );
	//repossessed.SetFromAxisAndAngle( CDirection(1,0,0), 90*DEG_TO_RAD_VALUE );

	if ( angle == 90 )
		angle += EPSILON;

	CPoint posTo = pos;
	for( int i = 1 ; i <= steps ; ++i )
	{
		posTo += (viewDist/steps)*fwd;
		float dist = ( posTo - pos ).Length();
		float radius = dist * (tan(angle));
		localMatrix.SetTranslation( posTo );
		g_VisualDebug->RenderArc( localMatrix * world, radius , TWO_PI,  colour);
	}

	if ( draw_sidelines )
	{

		CPoint	obDiff1( fwd * world );
		obDiff1 /= obDiff1.Length();
		obDiff1 *= viewDist;

		CPoint	obDiff2( obDiff1 );
		CPoint	obPosition( pos * world );
		

		// make one side of the cone
		float fAngularModifier = angle;

		float fCos = cos( fAngularModifier );
		float fSin = sin( fAngularModifier );
		float fNewX = fCos * obDiff1.X() + fSin * obDiff1.Z();
		float fNewZ = fCos * obDiff1.Z() - fSin * obDiff1.X();

		obDiff1.X() = fNewX;
		obDiff1.Z() = fNewZ;

		// and the other
		fAngularModifier *= -1.0f;

		fCos = cos( fAngularModifier );
		fSin = sin( fAngularModifier );
		fNewX = fCos * obDiff2.X() + fSin * obDiff2.Z();
		fNewZ = fCos * obDiff2.Z() - fSin * obDiff2.X();

		obDiff2.X() = fNewX;
		obDiff2.Z() = fNewZ;

		g_VisualDebug->RenderLine( obPosition, obPosition + obDiff1, colour, DPF_NOCULLING );
		g_VisualDebug->RenderLine( obPosition, obPosition + obDiff2, colour, DPF_NOCULLING );
	}
#endif
}

void draw_dir( const CPoint& obPosition, const CDirection& obFwdDir, float fViewDist, unsigned int uColour )
{
#ifndef _GOLD_MASTER
	CPoint obPosTo( obFwdDir );
	obPosTo /= obPosTo.Length();
	obPosTo *= fViewDist;

	g_VisualDebug->RenderLine( obPosition, obPosition + obPosTo, uColour, DPF_NOCULLING );
#endif
}



