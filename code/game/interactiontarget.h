#ifndef _INTERACTIONTARGET_H
#define _INTERACTIONTARGET_H

class CEntity;
class CUsePoint;

struct CInteractionTarget
{
	CEntity* 	m_pobInteractingEnt;
	CUsePoint* 	m_pobClosestUsePoint;
};

#endif
