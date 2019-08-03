//------------------------------------------------------------------------------------------
//!
//!	\file vaultingtransition.h
//!
//------------------------------------------------------------------------------------------

#ifndef	VAULTINGTRANSITION_H_
#define	VAULTINGTRANSITION_H_

#include "game/movementcontrollerinterface.h"

class ArcherVaultParams;


//------------------------------------------------------------------------------------------
//!
//!	VaultingTransitionDef
//!	A simple structure that needs to be filled out to create a simple transition.  Can be
//! predefined in xml if one wishes.
//!
//------------------------------------------------------------------------------------------
class VaultingTransitionDef : public MovementControllerDef
{
	public:
		// To create an instance of what we describe
		virtual MovementController *CreateInstance( CMovement *movement ) const;

		// Make a clone of ourselves
		virtual MovementControllerDef *Clone() const { return NT_NEW VaultingTransitionDef( *this ); }

	public:
		//
		//	Ctor, dtor.
		//
		VaultingTransitionDef();
		virtual ~VaultingTransitionDef() {}

	public:
		//
		//	Internal data.
		//
		bool		m_ApplyGravity;
		CPoint		m_FinalPosition;
		CDirection	m_Facing;
		float		m_AnimSpeed;
		bool		m_AllowAnimToBeSquashed;
		float		m_StartScalePercent;
		float		m_EndScalePercent;
		float		m_VerticalScale;

		// Pointer to archer vaulting params.
		const ArcherVaultParams* m_pVaultingParams;
};

//------------------------------------------------------------------------------------------
//!
//!	VaultingTransition
//!	Used to play a non looping animation on more compolex entities with a movement component
//!
//------------------------------------------------------------------------------------------
class VaultingTransition : public MovementController
{
	public:
		// The major functionality of any movement constroller
		virtual bool Update(	float						fTimeStep, 
								const CMovementInput&		obMovementInput,
								const CMovementStateRef&	obCurrentMovementState,
								CMovementState&				obPredictedMovementState );

	public:
		//
		//	Ctor, dtor.
		//
		VaultingTransition( CMovement *movement, const VaultingTransitionDef &definition );
		virtual ~VaultingTransition();

	private:
		//
		//	Internal data.
		//
		VaultingTransitionDef	m_Definition;				// A COPY of our defintion
		CAnimationPtr			m_SingleAnimation;			// A pointer to our single animation
		CPoint					m_AnimDistance;
};



#endif // !VAULTINGTRANSITION_H_
