//------------------------------------------------------------------------------------------
//!
//!	\file	CopyController.h
//!
//!			Applies a related set of animations to one entity based on another.
//!
//------------------------------------------------------------------------------------------

#ifndef	COPYCONTROLLER_INC_
#define	COPYCONTROLLER_INC_

//**************************************************************************************
//	Required Includes
//**************************************************************************************
#include "movementcontrollerinterface.h"
#include "anim/animation.h"

//**************************************************************************************
//	External Classes
//**************************************************************************************
class CEntity;
class CMovement;
class CopyController;

//**************************************************************************************
//	CopyControllerDef
//**************************************************************************************
class CopyControllerDef : public MovementControllerDef
{
	public:
		//
		//	MovementControllerDef implementation.
		//
								// Create an instance of what we describe
		MovementController *	CreateInstance( CMovement *movement ) const;

								// Make a clone of ourselves
		MovementControllerDef *	Clone() const { return NT_NEW CopyControllerDef( *this ); }

	public:
		//
		// Ctor.
		//
		CopyControllerDef();
		virtual ~CopyControllerDef() {}	// GCC bitches about having virtual func's but no virtual dtor, even though we don't need one. Sigh.

	public:
		//
		//	Data members.
		//
		CHashedString	m_CopyEntityName;	// Name of the entity to copy.
};

//**************************************************************************************
//	CopyController
//**************************************************************************************
class CopyController : public MovementController
{
	public:
		//
		//	MovementController implementation.
		//
		bool Update( float time_delta, const CMovementInput &input, const CMovementStateRef &currentState, CMovementState &predictedState );

	private:
		//
		//	Helper functions.
		//
		void			UpdateAnimation( const CHashedString& shortName, float weight );
		CAnimationPtr	CreateAnimation( const CHashedString& shortName );

	public:
		//
		//	Ctor, dtor.
		//
		CopyController( CMovement *movement, const CopyControllerDef *def );
		~CopyController();

	private:
		//
		//	Data members.
		//
		struct UpdateTrackedAnim
		{
			CAnimationPtr	m_Anim;
			bool			m_WasUpdated;
			UpdateTrackedAnim( CAnimationPtr anim, bool up ) : m_Anim( anim ), m_WasUpdated( up ) {}
		};
		typedef ntstd::Map< uint32_t, UpdateTrackedAnim > AnimationMap;

		CEntity *			m_CopyEntity;
		AnimationMap		m_Animations;
};


#endif // !COPYCONTROLLER_INC_
