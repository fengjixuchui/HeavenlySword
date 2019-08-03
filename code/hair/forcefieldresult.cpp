
// Necessary includes
#include "hair/forcefieldresult.h"
#include "hair/forcefield.h"
#include "game/entity.h"
#include "game/entity.inl"

CPoint ForceFieldResult::GetWorldForceFieldPosition(const CEntity* pt)
{
	return pt->GetPosition();
}

void ForceFieldResult::AddForce(const ForceField* f, const CEntity* e)
{
	m_worldForce += f->GetWorldForce(GetWorldForceFieldPosition(e));
}

