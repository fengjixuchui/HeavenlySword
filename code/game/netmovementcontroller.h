//------------------------------------------------------------------------------------------
//!
//!	\file netmovementcontroller.h
//!
//------------------------------------------------------------------------------------------

#ifndef	_NETMOVEMENTCONTROLLER_H
#define	_NETMOVEMENTCONTROLLER_H

// Necessary includes
#include "movementcontrollerinterface.h"

//------------------------------------------------------------------------------------------
//!
//!	NetMovementController
//!	Network movement controller for replicating general movement controllers over the 
//! network.
//!
//------------------------------------------------------------------------------------------
class NetMovementController : public MovementController
{
public:
	NetMovementController(CMovement* pobMovement);
	virtual ~NetMovementController();

	virtual bool Update(float fTimeStep,
						const CMovementInput& obMovementInput, 
						const CMovementStateRef& obCurrentMovementState,
						CMovementState& obPredictedMovementState);
};

#endif //_NETMOVEMENTCONTROLLER_H
