
#include "blendshapes/shading/bsskin.h"
#include "blendshapes/shading/bsskin_properties.h"
#include "objectdatabase/dataobject.h"
#include "gfx/shader.h"
#include "core/gatso.h"
#include "gfx/renderersettings.h"


BSSkin::BSSkin( CMaterial const* pobMaterial, 
				CMeshVertexElement * pobVertexElements, 
				int iVertexElementCount, 
				CMaterialProperty const* pobProperties, 
				int iPropertyCount,
				const char* pPropertiesName ) 
:	CGameMaterialInstance( pobMaterial, pobVertexElements, iVertexElementCount, pobProperties, iPropertyCount ),
	m_pProperties( 0 ),
	m_bDatabaseObjCreated( false )
{
	strcpy( m_pPropertiesName, pPropertiesName );
	// see if we can find a bsskin properties override somewhere
	RefreshPropertiesFromObjDatabase();
	// if not, create one and copy our definition
	if ( !m_pProperties )
	{
		CreateDatabaseObj();
		CopyMaterialProperties( m_pProperties );
	}
}

BSSkin::~BSSkin()
{
	// clean the database obj if we were the ones who created it
	/*if ( m_bDatabaseObjCreated )
	{
		DataObject* pDO = ObjectDatabase::Get().GetDataObjectFromPointer( m_pProperties );
		ntAssert_p( pDO, ("couldn't find the database object associated with our properties table\n") );
		ObjectDatabase::Get().DestroyObject( pDO );
	}*/
}


void BSSkin::RefreshPropertiesFromObjDatabase( void ) const
{
	m_pProperties = ObjectDatabase::Get().GetPointerFromName<BSSkinProperties*>( m_pPropertiesName );
}

void BSSkin::BindProperties( Shader* pVertexShader, Shader* pPixelShader, MATERIAL_DATA_CACHE const& stCache ) const
{
	//! bind everything as normal
	CGameMaterialInstance::BindProperties( pVertexShader, pPixelShader, stCache );


	CGatso::Start( "CBSSkin::UploadPSProperties" );

#ifndef _RELEASE
	RefreshPropertiesFromObjDatabase();
#endif
	//! then override if we have a reflected properties object
	if ( m_pProperties && CRendererSettings::bLiveMaterialParamsEditing )
	{
		for(int iBinding = 0; iBinding < pPixelShader->GetNumPropertyBindings(); ++iBinding)
		{
			const SHADER_PROPERTY_BINDING* pstBinding = pPixelShader->GetPropertyBinding(iBinding);
			switch( pstBinding->eSemantic )
			{
				case PROPERTY_BSSKIN_DIFFUSE_WRAP:
					{
						CVector tmp(m_pProperties->m_diffuseWrap);
						SetPSConst( pPixelShader, pstBinding->iRegister, tmp);
						break;
					}
				case PROPERTY_BSSKIN_SPECULAR_FACING:
					{
						CVector tmp(m_pProperties->m_specularFacing);
						SetPSConst( pPixelShader, pstBinding->iRegister, tmp);
						break;
					}
				case PROPERTY_BSSKIN_SPECULAR_GLANCING:
					{
						CVector tmp(m_pProperties->m_specularGlancing);
						SetPSConst( pPixelShader, pstBinding->iRegister, tmp );
						break;
					}
				case PROPERTY_BSSKIN_FUZZ_COLOUR:
					{
						SetPSConst( pPixelShader, pstBinding->iRegister, m_pProperties->m_fuzzColour );
						break;
					}
				case PROPERTY_BSSKIN_FUZZ_TIGHTNESS:
					{
						CVector tmp(m_pProperties->m_fuzzTightness);
						SetPSConst( pPixelShader, pstBinding->iRegister, tmp );
						break;
					}
				case PROPERTY_BSSKIN_SUBCOLOUR:
					{
						SetPSConst( pPixelShader, pstBinding->iRegister, m_pProperties->m_subColour );
						break;
					}
				case PROPERTY_BSSKIN_NORMAL_MAP_STRENGTH:
					{
						CVector tmp(m_pProperties->m_normalMapStrength);
						SetPSConst( pPixelShader, pstBinding->iRegister, tmp );
						break;
					}
				case PROPERTY_BSSKIN_SPECULAR_STRENGTH:
					{
						CVector tmp(m_pProperties->m_specularStrength);
						SetPSConst( pPixelShader, pstBinding->iRegister, tmp );						
						break;
					}
				default:
					// nothing. Let the other one take care of it
					break;
			}
		}
	}
	CGatso::Stop( "CBSSkin::UploadPSProperties" );
}

void BSSkin::PatchProperties(	restrict Heresy_PushBuffer* pPB,
								const MATERIAL_DATA_CACHE& stCache ) const
{
	restrict Heresy_PushBufferPatch* pPatch = (Heresy_PushBufferPatch*) (pPB+1);

	for( uint16_t i = 0; i < pPB->m_iNumPatches;i++, pPatch++ )
	{
		switch( (pPatch->m_iData & HPBP_FIXUP_TYPE_MASK) )
		{
		case HPBP_PIXEL_CONSTANT_FIXUP:
			FixupPixelShaderConstant( pPB, pPatch, stCache );
			FixupPixelShaderConstantOverride( pPB, pPatch, stCache );
			break;
		case HPBP_TEXTURE_FIXUP:
			FixupTexture( pPB, pPatch );
			break;
		case HPBP_VERTEX_CONSTANT_FIXUP:
			FixupVertexShaderConstant( pPB, pPatch, stCache );
			break;
		default:
			// probably a pixel shader fixup
			break;
		}
	}
}

void BSSkin::FixupPixelShaderConstantOverride(restrict Heresy_PushBuffer* pPB, const restrict Heresy_PushBufferPatch* pPatch, const MATERIAL_DATA_CACHE& stCache ) const
{
#ifndef _RELEASE
	RefreshPropertiesFromObjDatabase();
#endif
	//! then override if we have a reflected properties object
	if ( m_pProperties && CRendererSettings::bLiveMaterialParamsEditing )
	{
		switch( pPatch->m_Semantic )
		{
		case PROPERTY_BSSKIN_DIFFUSE_WRAP:
			{
				CVector tmp(m_pProperties->m_diffuseWrap);
				Heresy_FixupPixelShaderConstantScratch16B( pPB, pPatch, (const uint128_t* restrict)&tmp.Quadword() );
				break;
			}
		case PROPERTY_BSSKIN_SPECULAR_FACING:
			{
				CVector tmp(m_pProperties->m_specularFacing);
				Heresy_FixupPixelShaderConstantScratch16B( pPB, pPatch, (const uint128_t* restrict)&tmp.Quadword() );
				break;
			}
		case PROPERTY_BSSKIN_SPECULAR_GLANCING:
			{
				CVector tmp(m_pProperties->m_specularGlancing);
				Heresy_FixupPixelShaderConstantScratch16B( pPB, pPatch, (const uint128_t* restrict)&tmp.Quadword() );
				break;
			}
		case PROPERTY_BSSKIN_FUZZ_COLOUR:
			{
				Heresy_FixupPixelShaderConstantScratch16B( pPB, pPatch, (const uint128_t* restrict)&m_pProperties->m_fuzzColour.Quadword() );
				break;
			}
		case PROPERTY_BSSKIN_FUZZ_TIGHTNESS:
			{
				CVector tmp(m_pProperties->m_fuzzTightness);
				Heresy_FixupPixelShaderConstantScratch16B( pPB, pPatch, (const uint128_t* restrict)&tmp.Quadword() );
				break;
			}
		case PROPERTY_BSSKIN_SUBCOLOUR:
			{
				Heresy_FixupPixelShaderConstantScratch16B( pPB, pPatch, (const uint128_t* restrict)&m_pProperties->m_subColour.Quadword() );
				break;
			}
		case PROPERTY_BSSKIN_NORMAL_MAP_STRENGTH:
			{
				CVector tmp(m_pProperties->m_normalMapStrength);
				Heresy_FixupPixelShaderConstantScratch16B( pPB, pPatch, (const uint128_t* restrict)&tmp.Quadword() );
				break;
			}
		case PROPERTY_BSSKIN_SPECULAR_STRENGTH:
			{
				CVector tmp(m_pProperties->m_specularStrength);
				Heresy_FixupPixelShaderConstantScratch16B( pPB, pPatch, (const uint128_t* restrict)&tmp.Quadword() );
				break;
			}
		default:
			// don't deal with the rest
			break;
		}
	}
}


void BSSkin::CreateDatabaseObj( void )
{
	DataObject* pDO = ObjectDatabase::Get().ConstructObject( "BSSkinProperties", m_pPropertiesName );
	m_pProperties = static_cast<BSSkinProperties*>( pDO->GetBasePtr() );
	m_bDatabaseObjCreated = true;
}


void BSSkin::CopyMaterialProperties(  BSSkinProperties* pProperties ) const
{
	CMaterialProperty* p;
	p = const_cast<CMaterialProperty*>( GetMaterialProperty( PROPERTY_BSSKIN_DIFFUSE_WRAP ) );
	ntAssert_p( p, ("BSSkin GetMaterialProperty(%i) failed!", PROPERTY_BSSKIN_DIFFUSE_WRAP) );
	pProperties->m_diffuseWrap = p->GetFloatData().afFloats[0];

	p = const_cast<CMaterialProperty*>( GetMaterialProperty( PROPERTY_BSSKIN_SPECULAR_FACING ) );
	ntAssert_p( p, ("BSSkin GetMaterialProperty(%i) failed!", PROPERTY_BSSKIN_SPECULAR_FACING) );
	pProperties->m_specularFacing = p->GetFloatData().afFloats[0];

	p = const_cast<CMaterialProperty*>( GetMaterialProperty( PROPERTY_BSSKIN_SPECULAR_GLANCING ) );
	ntAssert_p( p, ("BSSkin GetMaterialProperty(%i) failed!", PROPERTY_BSSKIN_SPECULAR_GLANCING) );
	pProperties->m_specularGlancing = p->GetFloatData().afFloats[0];

	p = const_cast<CMaterialProperty*>( GetMaterialProperty( PROPERTY_BSSKIN_FUZZ_TIGHTNESS ) );
	ntAssert_p( p, ("BSSkin GetMaterialProperty(%i) failed!", PROPERTY_BSSKIN_FUZZ_TIGHTNESS) );
	pProperties->m_fuzzTightness = p->GetFloatData().afFloats[0];

	p = const_cast<CMaterialProperty*>( GetMaterialProperty( PROPERTY_BSSKIN_SUBCOLOUR ) );
	ntAssert_p( p, ("BSSkin GetMaterialProperty(%i) failed!", PROPERTY_BSSKIN_SUBCOLOUR) );
	pProperties->m_subColour = CVector( p->GetFloatData().afFloats );

	p = const_cast<CMaterialProperty*>( GetMaterialProperty( PROPERTY_BSSKIN_FUZZ_COLOUR ) );
	ntAssert_p( p, ("BSSkin GetMaterialProperty(%i) failed!", PROPERTY_BSSKIN_FUZZ_COLOUR) );
	pProperties->m_fuzzColour = CVector( p->GetFloatData().afFloats );

	// new stuff. TODO_OZZ: revert to assert once we have the new Ninja Exporter and all assets are re-exported
	p = const_cast<CMaterialProperty*>( GetMaterialProperty( PROPERTY_BSSKIN_NORMAL_MAP_STRENGTH ) );
	//ntAssert_p( p, ("BSSkin GetMaterialProperty(%i) failed!", PROPERTY_BSSKIN_NORMAL_MAP_STRENGTH) );
	pProperties->m_normalMapStrength = p ? p->GetFloatData().afFloats[0] : 1.0f;

	p = const_cast<CMaterialProperty*>( GetMaterialProperty( PROPERTY_BSSKIN_SPECULAR_STRENGTH ) );
	//ntAssert_p( p, ("BSSkin GetMaterialProperty(%i) failed!", PROPERTY_BSSKIN_SPECULAR_STRENGTH) );
	pProperties->m_specularStrength = p ? p->GetFloatData().afFloats[0] : 1.0f;
}





