#ifndef _VERLETMATERIAL_H_
#define _VERLETMATERIAL_H_

//--------------------------------------------------
//!
//!	\file verletmaterial.h
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------


#include "gfx/proceduralMaterial_ps3.h"
#include "gfx/materialinstance.h"


namespace Physics
{

class VerletMaterialInstanceDef;
class VerletGameLink;

//--------------------------------------------------
//!
//!	shader-material link between verlet and the game.
//!
//--------------------------------------------------
class VerletGameLink
{
public:
	ProceduralStream m_streamDynamic;
	ProceduralStream m_streamStatic;
	ProceduralStream m_streamBoth;
	ProceduralProperty m_property;
public:
	VerletGameLink();
	void Set();
}; // end of class SpeedTreeGameLink








//--------------------------------------------------
//!
//!	verlet materual instance
//! contaions property and the gameMaterialInstance
//!
//--------------------------------------------------
class VerletMaterialInstance
{
public:
	CSharedArray<CMaterialProperty> m_gameMaterialProperty;
	CScopedPtr<CGameMaterialInstance> m_pMaterialInstance;
public:
	//! constructor
	VerletMaterialInstance(const VerletMaterialInstanceDef& materialDef, const VerletGameLink& link);
	// set material
	void SetMaterial(const VerletMaterialInstanceDef& materialDef, const VerletGameLink& link);
}; // end of class VerletMaterial


} //Physics

#endif // end of _VERLETMATERIAL_H_
