#include "netmovementcontroller.h"

NetMovementController::NetMovementController(CMovement* pMovement)
	: MovementController(pMovement)
{
}

NetMovementController::~NetMovementController()
{
}

bool NetMovementController::Update(float fTimeStep, const CMovementInput&,
								 const CMovementStateRef&, CMovementState&)
{
	UNUSED(fTimeStep);

	// Never signal completion
	return false;
}
