#include "proceduralMaterial_ps3.h"

#include "gfx/materialinstance.h"


// check a CMeshVertexElement array (stride and offset)
void ProceduralStream::CheckStream(CMeshVertexElement* pElem, uint32_t iNbStreamElem, uint32_t iSize)
{
	uint32_t iOffset = 0;
	for(uint32_t i = 0 ; i < iNbStreamElem ; ++i )
	{
		CMeshVertexElement& elem = pElem[i];
		ntError_p(static_cast<uint32_t>(elem.m_iStride)==GetSizeOfElement(elem.m_eType), ("CheckStream: wrong size"));
		ntError_p(static_cast<uint32_t>(elem.m_iOffset)==iOffset, ("CheckStream: wrong offset"));
		iOffset+=elem.m_iStride;
	}
	ntError_p(iSize==iOffset, ("CheckStream: wrong total size"));
}



void ProceduralStream::AllocateVertexElem(int iNbStreamElem)
{
	m_iNbStreamElem = iNbStreamElem;
	m_gameVertexElement = CSharedArray<CMeshVertexElement>(NT_NEW_CHUNK( Mem::MC_GFX ) CMeshVertexElement[m_iNbStreamElem]);
}



void ProceduralStream::GenerateGameVertexElement()
{
	m_gcVertexElem = CSharedArray<GcStreamField>(NT_NEW_CHUNK( Mem::MC_GFX ) GcStreamField[m_iNbStreamElem]);
	for( int i = 0;i < m_iNbStreamElem;i++)
	{
		m_gcVertexElem[i] = GcStreamField( GetSemanticName(	m_gameVertexElement[i].m_eStreamSemanticTag ),
																m_gameVertexElement[i].m_iOffset,
											GetStreamType(		m_gameVertexElement[i].m_eType ),
											GetStreamElements(	m_gameVertexElement[i].m_eType ),
											IsTypeNormalised(	m_gameVertexElement[i].m_eType ) );
	}
}

ProceduralStream::ProceduralStream()
	:m_iNbStreamElem(0)
	,m_gcVertexElem(0)
	,m_gameVertexElement(0)
{
	// nothing
}











CSharedArray<CMaterialProperty> ProceduralProperty::GetEmptyMaterialProperty() const
{
	CSharedArray<CMaterialProperty> pRes =   CSharedArray<CMaterialProperty>(NT_NEW_CHUNK( Mem::MC_GFX ) CMaterialProperty[m_iNbGameProperty]);
	for(int i = 0 ; i < m_iNbGameProperty ; ++i )
	{
		pRes[i].m_iPropertyTag = m_emptyMaterialProperty[i].m_iPropertyTag;
	}
	return pRes;
}

void ProceduralProperty::AllocateGameProperty(int iNbGameProperty)
{
	m_iNbGameProperty = iNbGameProperty;
	m_emptyMaterialProperty = CSharedArray<CMaterialProperty>(NT_NEW_CHUNK( Mem::MC_GFX ) CMaterialProperty[m_iNbGameProperty]);

}

ProceduralProperty::ProceduralProperty()
	:m_iNbGameProperty(0)
	,m_emptyMaterialProperty(0)
	,m_pMaterial(0)
{
	// nothing
}

