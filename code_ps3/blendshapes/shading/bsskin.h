//--------------------------------------------------
//!
//!	\file bsskin.h
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#ifndef _BSSKIN_H_
#define _BSSKIN_H_

#include "gfx/materialinstance.h"

#ifndef PLATFORM_PS3
#	error BSSkin if for PS3 ONLY
#endif

class CMeshVertexElement;
class CMeshInstance;
class CHierarchy;
class CSkinMatrix;
class Shader;

class BSSkinProperties;

enum BSSkinType
{
	kBSSkin_Wrinkled,
	kBSSkin_NonWrinkled,
};

static const char* cDefaultBSSkinPropertiesName = "bsskin_properties";


class BSSkin : public CGameMaterialInstance
{
public:
	/*BSSkin( CMaterial const* pobMaterial, 
						   CMeshVertexElement * pobVertexElements, 
						   int iVertexElementCount, 
						   CMaterialProperty const* pobProperties, 
						   int iPropertyCount );*/

	BSSkin( CMaterial const* pobMaterial, 
						   CMeshVertexElement * pobVertexElements, 
						   int iVertexElementCount, 
						   CMaterialProperty const* pobProperties, 
						   int iPropertyCount,
						   const char* pPropertiesName = cDefaultBSSkinPropertiesName );
	virtual ~BSSkin();

	virtual void PatchProperties(	restrict Heresy_PushBuffer* pPB, const MATERIAL_DATA_CACHE& stCache ) const;

protected:
	virtual void BindProperties( Shader* pVertexShader, Shader* pPixelShader, MATERIAL_DATA_CACHE const& stCache ) const;


private:
	void RefreshPropertiesFromObjDatabase( void ) const;
	void FixupPixelShaderConstantOverride(restrict Heresy_PushBuffer* pPB, const restrict Heresy_PushBufferPatch* pPatch, const MATERIAL_DATA_CACHE& stCache ) const;
	void CopyMaterialProperties( BSSkinProperties* pProperties ) const;
	void CreateDatabaseObj( void );
private: 
	//! reflected properties
	mutable BSSkinProperties*	m_pProperties;
	bool						m_bDatabaseObjCreated;

	char	m_pPropertiesName[32];
};


#endif // end of _BSSKIN_H_
