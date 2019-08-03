//------------------------------------------------------------------------------------------
//!
//!	\file archerwalkruncontroller.h
//!
//------------------------------------------------------------------------------------------

#ifndef	ARCHERWALKRUNCONTROLLER_H_
#define	ARCHERWALKRUNCONTROLLER_H_

#include "game/walkingcontroller.h"

class ArcherWalkRunControllerDef : public WalkRunControllerDef
{
	public:
		//
		//	Ctor, dtor.
		//
		ArcherWalkRunControllerDef() : WalkRunControllerDef(), m_HasCrouchedAnimSet( false ) {}
		~ArcherWalkRunControllerDef() {}

	public:
		// To create an instance of what we describe
		virtual MovementController *CreateInstance( CMovement *movement ) const;

		// Make a clone of ourselves
		virtual MovementControllerDef *Clone() const { return NT_NEW ArcherWalkRunControllerDef( *this ); }

		bool	m_HasCrouchedAnimSet;
};


//------------------------------------------------------------------------------------------
//!
//!	ArcherWalkRunController
//!	The main controller for the standard movement of the Archer.
//!
//! Work mainly as the standard walk-run controller but also works out if we should
//! automatically crouch or not.
//!
//------------------------------------------------------------------------------------------
class ArcherWalkRunController : public WalkRunController
{
	public:
		//
		//	Ctor, dtor.
		//
		ArcherWalkRunController( CMovement *movement, const ArcherWalkRunControllerDef &definition );
		~ArcherWalkRunController();

		// The main update details
		virtual bool Update(	float						fTimeStep, 
								const CMovementInput&		obMovementInput,
								const CMovementStateRef&	obCurrentMovementState,
								CMovementState&				obPredictedMovementState );
		
		// Disable one-off partial animations for this derived controller 
		virtual bool IsPlayingPartialAnim() const { return false; };
		virtual bool CanPlayPartialAnim() const	{ return false; };
		virtual void PlayPartialAnim( const CHashedString& m_pobPartialAnim, float fFadeIn, float fFadeOut ) { UNUSED( m_pobPartialAnim ); UNUSED( fFadeIn ); UNUSED( fFadeOut ); };

	protected:
		// A copy of our defintion
		bool	m_HasCrouchedAnimSet;
		bool	m_OkToStartNewController;
		bool	m_bFirstFrame;
};

#endif // !ARCHERWALKRUNCONTROLLER_H_

