#ifndef _PROCEDURALMATERIAL_PS3_H_
#define _PROCEDURALMATERIAL_PS3_H_

//--------------------------------------------------
//!
//!	\file proceduralMaterial_ps3.h
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#include "gfx/vertexdeclaration.h"
#include "core/exportstruct_clump.h"

class CMaterial;

//--------------------------------------------------
//!
//!	link between speedtree and the game.
//!
//--------------------------------------------------
class ProceduralStream
{
public:
	int m_iNbStreamElem;
	CSharedArray<GcStreamField> m_gcVertexElem;
	CSharedArray<CMeshVertexElement> m_gameVertexElement;
public:
	ProceduralStream();
	static void CheckStream(CMeshVertexElement* pElem, uint32_t iNbStreamElem, uint32_t iSize);
	void AllocateVertexElem(int iNbStreamElem);
	void GenerateGameVertexElement();
}; // end of class SpeedTreeGameLink




//--------------------------------------------------
//!
//!	link between speedtree and the game.
//!
//--------------------------------------------------
class ProceduralProperty
{
public:
	int m_iNbGameProperty;
	CSharedArray<CMaterialProperty> m_emptyMaterialProperty;
	const CMaterial* m_pMaterial;
public:
	ProceduralProperty();
	CSharedArray<CMaterialProperty> GetEmptyMaterialProperty() const;
	void AllocateGameProperty(int iNbGameProperty);
}; // end of class SpeedTreeGameLink


#endif // end of _PROCEDURALMATERIAL_PS3_H_
