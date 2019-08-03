#ifndef _MATERIALBASE_H_
#define _MATERIALBASE_H_

#include "material.h"
#include "vertexdeclaration.h"
#include "core/explicittemplate.h"

class Transform;
class CSkinMatrix;
class CHierarchy;
class CMaterialProperty;
class CMeshVertexElement;


//--------------------------------------------------
//!
//!	common class between exportFX and gamematerial.
//!
//--------------------------------------------------



class MaterialInstanceBase
{
public:

	MaterialInstanceBase(bool bIsFX);
	virtual ~MaterialInstanceBase();
	
	// rendering stuff
	virtual void PreRender( Transform const* pobTransform, bool bRecieveShadows, void const* pDecompMatrix = NULL ) const = 0;
	virtual void PostRender() const = 0;
	
	// shadow rendering stuff
	virtual void PreRenderDepth( Transform const* pobTransform, bool bShadowProject, void const* pDecompMatrix = NULL ) const = 0;
	virtual void PostRenderDepth( bool bShadowProject ) const = 0;
	virtual void PreRenderShadowRecieve( Transform const* pobTransform, void const* pDecompMatrix = NULL ) const = 0;
	virtual void PostRenderShadowRecieve() const = 0;

	// property table
	void SetPropertyTable( CMaterialProperty const* pobProperties, int iPropertyCount );
	void SetPropertyTableOveride( bool bCopyTable );
	int							GetPropertyTableSize( void ) const;
	CMaterialProperty const*	GetPropertyTable( void ) const;
	CMaterialProperty const*	GetPropertyDefaultTable( void ) const;
	CMaterialProperty*			GetPropertyOverideTable( void ) const;
	
	//! Gets the bound vertex shader type in this material.
	VERTEXSHADER_TRANSFORM_TYPE GetBoundType() const { return m_eBoundType; }

	//! skin
	void SetBoneIndices( const u_char* pucBoneIndices, int iNumberOfBonesUsed );
	void CopySkinMatrices( CHierarchy* pobHierarchy, CSkinMatrix* pBoneArray ) const;

	// flags
	bool IsAlphaBlended()	const { return m_bIsAlphaBlended; }
	bool isFX() 		const { return m_bIsFX; }

	// set vertex element
	void SetVertexElement(CMeshVertexElement* pVertexElements,int iVertexElementCount);
	void SetDebugIndices(Pixel3 debugIndices, int iMesh = 0);
	Pixel4 GetDebugIndices() const { return m_debugIndices;}

private:
	// property table prop
	//CMaterialProperty const* m_pobDefaultProperties;
	int	m_iPropertyCount;
	const CMaterialProperty* m_pDefaultProperties; 
	CScopedArray<CMaterialProperty> m_propertyOverideTable;
	int m_iPropertyOverideTableRefCount;

protected:	
	// properties binding map
	ntstd::Map<int, const CMaterialProperty*> m_PropertiesMap; 

	VERTEXSHADER_TRANSFORM_TYPE m_eBoundType;						//!< The vertex shader type we're bound to.

	//! skin param
	const u_char* m_pucBoneIndices;			//!< The bone mapping for this material.
	int m_iNumberOfBonesUsed;				//!< The number of bones in the mapping.

	bool m_bIsFX;
	bool m_bIsAlphaBlended;

	// vertex elments
	CMeshVertexElement* m_pVertexElements;	//!< An array of vertex elements.
	int m_iVertexElementCount;				//!< The number of vertex elements in the array.
	
	Pixel4 m_debugIndices;
}; // end of class MaterialInstanceBase


#endif // end of _MATERIALBASE_H_
