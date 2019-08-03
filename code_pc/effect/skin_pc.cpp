//--------------------------------------------------
//!
//!	\file skin.cpp
//!	Contains the published data for skin functions.
//!
//--------------------------------------------------

#include "effect/skin.h"
#include "objectdatabase/dataobject.h"
#include "effect/effect_util.h"
#include "gfx/fxmaterial.h"
#include "gfx/rendercontext.h"
#include "gfx/levellighting.h"

START_STD_INTERFACE( SkinDef )

	I2REFERENCE_WITH_DEFAULT	( m_RednessFunc, 				Skin_RednessFunc,				FunctionCurve_User )
	I2REFERENCE_WITH_DEFAULT	( m_TranslucencyFacingFunc, 	Skin_TranslucencyFacingFunc,	FunctionCurve_User )
	I2REFERENCE_WITH_DEFAULT	( m_TranslucencyLuminanceFunc, 	Skin_TranslucencyLuminanceFunc,	FunctionCurve_User )
	I2REFERENCE_WITH_DEFAULT	( m_PeachFuzzFunc, 				Skin_PeachFuzzFunc,				FunctionCurve_User )
	I2REFERENCE_WITH_DEFAULT	( m_DirectSpecularFunc, 		Skin_DirectSpecularFunc,		FunctionCurve_User )
	I2REFERENCE_WITH_DEFAULT	( m_GlancingSpecularFunc, 		Skin_GlancingSpecularFunc,		FunctionCurve_User )

	I2FLOAT						( m_DiffuseMultiplier,			Skin_DiffuseMultiplier )

	PUBLISH_VAR_WITH_DEFAULT_AS					( m_RednessColour,			CVector(1,1,1,1),		Skin_RednessColour  )
	PUBLISH_VAR_WITH_DEFAULT_AS					( m_TranslucencyColour,		CVector(1,1,1,1),		Skin_TranslucencyColour )
	PUBLISH_VAR_WITH_DEFAULT_AS					( m_PeachFuzzColour,		CVector(1,1,1,1),		Skin_PeachFuzzColour )
	PUBLISH_VAR_WITH_DEFAULT_AS					( m_DirectSpecularColour,	CVector(1,1,1,1),		Skin_DirectSpecularColour )
	PUBLISH_VAR_WITH_DEFAULT_AS					( m_GlanceSpecularColour,	CVector(1,1,1,1),		Skin_GlanceSpecularColour )

	I2REFERENCE_WITH_DEFAULT	( m_RednessTOD,					Skin_RednessTOD,				ColourFunction_Palette )
	I2REFERENCE_WITH_DEFAULT	( m_TranslucencyTOD,			Skin_TranslucencyTOD,			ColourFunction_Palette )
	I2REFERENCE_WITH_DEFAULT	( m_PeachFuzzTOD,				Skin_PeachFuzzTOD,				ColourFunction_Palette )
	I2REFERENCE_WITH_DEFAULT	( m_DirectSpecularTOD,			Skin_DirectSpecularTOD,			ColourFunction_Palette )
	I2REFERENCE_WITH_DEFAULT	( m_GlanceSpecularTOD,			Skin_GlanceSpecularTOD,			ColourFunction_Palette )

	I2FLOAT						( m_DirectSpecularRoughness,	Skin_DirectSpecularRoughness )
	I2FLOAT						( m_GlanceSpecularPower,		Skin_GlanceSpecularPower )
	I2FLOAT						( m_GlanceLightZDistance,		Skin_GlanceLightLocation )
	I2FLOAT						( m_GlanceSpecularMaxDistance,	Skin_GlanceSpecularMaxDistance )

	I2BOOL						( m_ShowDiffusePass,			Skin_ShowDiffusePass )
	I2BOOL						( m_ShowRednessPass,			Skin_ShowRednessPass )
	I2BOOL						( m_ShowTranslucencyPass,		Skin_ShowTranslucencyPass )
	I2BOOL						( m_ShowPeachFuzzPass,			Skin_ShowPeachFuzzPass )
	I2BOOL						( m_ShowDirectSpecularPass,		Skin_ShowDirectSpecularPass )
	I2BOOL						( m_ShowGlanceSpecularPass,		Skin_ShowGlanceSpecularPass )

	DECLARE_POSTCONSTRUCT_CALLBACK( OnPostConstruct )
	
END_STD_INTERFACE

//*************************************************************************************
//	
//*************************************************************************************
void SkinDef::SetSkinParameters( const FXHandle &effect, const CMatrix &worldToObject ) const
{
	D3DXHANDLE h;

	float time( LevelLighting::Get().GetTimeOfDayN() );
	ntError( time >= 0.0f && time <= 1.0f );

	if (m_SkinFunctions.GetFunctionTex())
	{
		FX_GET_HANDLE_FROM_NAME( effect, h, "m_diffuse1" );
		effect->SetTexture( h, m_SkinFunctions.GetFunctionTex()->m_Platform.Get2DTexture() );
	}

	effect->SetBool( "g_bShowDiffusePass", 			m_ShowDiffusePass 			? TRUE : FALSE );
	effect->SetBool( "g_bShowRednessPass", 			m_ShowRednessPass 			? TRUE : FALSE );
	effect->SetBool( "g_bShowTranslucencyPass", 	m_ShowTranslucencyPass		? TRUE : FALSE );
	effect->SetBool( "g_bShowPeachFuzzPass", 		m_ShowPeachFuzzPass			? TRUE : FALSE );
	effect->SetBool( "g_bShowDirectSpecularPass",	m_ShowDirectSpecularPass 	? TRUE : FALSE );
	effect->SetBool( "g_bShowGlanceSpecularPass",	m_ShowGlanceSpecularPass 	? TRUE : FALSE );

	CVector redness( m_RednessTOD->GetColour( time ) * m_RednessTOD->GetColour( time ).W() );
	redness = redness * (CVector( m_RednessColour.X(), m_RednessColour.Y(), m_RednessColour.Z(), 1.f ) * m_RednessColour.W() );

	CVector translucecncy( m_TranslucencyTOD->GetColour( time ) * m_TranslucencyTOD->GetColour( time ).W() );
	translucecncy = translucecncy * (CVector( m_TranslucencyColour.X(), m_TranslucencyColour.Y(), m_TranslucencyColour.Z(), 1.f ) * m_TranslucencyColour.W() );

	CVector peach_fuzz( m_PeachFuzzTOD->GetColour( time ) * m_PeachFuzzTOD->GetColour( time ).W() );
	peach_fuzz = peach_fuzz * (CVector( m_PeachFuzzColour.X(), m_PeachFuzzColour.Y(), m_PeachFuzzColour.Z(), 1.f ) * m_PeachFuzzColour.W() );

	CVector direct_specular( m_DirectSpecularTOD->GetColour( time ) * m_DirectSpecularTOD->GetColour( time ).W() );
	direct_specular = direct_specular * (CVector( m_DirectSpecularColour.X(), m_DirectSpecularColour.Y(), m_DirectSpecularColour.Z(), 1.f ) * m_DirectSpecularColour.W() );

	CVector glance_specular( m_GlanceSpecularTOD->GetColour( time ) * m_GlanceSpecularTOD->GetColour( time ).W() );
	glance_specular = glance_specular * (CVector( m_GlanceSpecularColour.X(), m_GlanceSpecularColour.Y(), m_GlanceSpecularColour.Z(), 1.f ) * m_GlanceSpecularColour.W() );

	float dist_to_camera( ( RenderingContext::Get()->GetEyePos() - worldToObject.GetTranslation() ).Length() );
	float glance_specular_blend_out( 1.0f - ( dist_to_camera / m_GlanceSpecularMaxDistance ) );
	glance_specular_blend_out = clamp( glance_specular_blend_out, 0.0f, 1.0f );

	FX_SET_VALUE_VALIDATE( effect, "g_DiffuseMultiplier",		&m_DiffuseMultiplier,		sizeof( float ) );
	FX_SET_VALUE_VALIDATE( effect, "g_RednessColour",			&redness,					sizeof( float ) * 4 );
	FX_SET_VALUE_VALIDATE( effect, "g_TranslucencyColour",		&translucecncy,				sizeof( float ) * 4 );
	FX_SET_VALUE_VALIDATE( effect, "g_PeachFuzzColour",			&peach_fuzz,				sizeof( float ) * 4 );
	FX_SET_VALUE_VALIDATE( effect, "g_DirectSpecularColour",	&direct_specular,			sizeof( float ) * 4 );
	FX_SET_VALUE_VALIDATE( effect, "g_DirectSpecularRoughness",	&m_DirectSpecularRoughness,	sizeof( float ) );
	FX_SET_VALUE_VALIDATE( effect, "g_GlanceSpecularColour",	&glance_specular,			sizeof( float ) * 4 );
	FX_SET_VALUE_VALIDATE( effect, "g_GlanceSpecularPower",		&m_GlanceSpecularPower,		sizeof( float ) );
	FX_SET_VALUE_VALIDATE( effect, "g_GlanceSpecularBlendOut",	&glance_specular_blend_out,	sizeof( float ) );

	CVector light_dir( RenderingContext::Get()->m_toKeyLight );

	//
	//	Set the glance light direction.
	//
	CVector G = -light_dir;
	if ( G.Y() < 0.0f )
	{
		G.Y() = 0.0f;
	}
	ntAssert( G.Length() > 0.0f );
	G /= G.Length();

	// Transform to object space.
	G = G * worldToObject;

	FX_SET_VALUE_VALIDATE( effect, "g_GlanceSpecularLightDirection", &G, sizeof( float ) * 4 );
}

//*************************************************************************************
//	
//*************************************************************************************
void SkinDef::GenerateResources()
{
	m_SkinFunctions.Reset( FunctionObject::FTT_TEXTURE );

	m_SkinFunctions.AddFunction( m_RednessFunc );
	m_SkinFunctions.AddFunction( m_TranslucencyFacingFunc );
	m_SkinFunctions.AddFunction( m_TranslucencyLuminanceFunc );
	m_SkinFunctions.AddFunction( m_PeachFuzzFunc );
	m_SkinFunctions.AddFunction( m_DirectSpecularFunc );
	m_SkinFunctions.AddFunction( m_GlancingSpecularFunc );
	m_SkinFunctions.FlushCreation();

	ResourcesOutOfDate();									// this flushes any erronious refresh detects

	m_bRequireRefresh = false;
}

//*************************************************************************************
//	
//*************************************************************************************
bool SkinDef::ResourcesOutOfDate() const
{
	m_bRequireRefresh =	m_RednessFunc->DetectCurveChanged() ||
						m_TranslucencyFacingFunc->DetectCurveChanged() ||
						m_TranslucencyLuminanceFunc->DetectCurveChanged() ||
						m_PeachFuzzFunc->DetectCurveChanged() ||
						m_DirectSpecularFunc->DetectCurveChanged() ||
						m_GlancingSpecularFunc->DetectCurveChanged() ||
						m_RednessTOD->HasChanged() ||
						m_TranslucencyTOD->HasChanged() ||
						m_PeachFuzzTOD->HasChanged() ||
						m_DirectSpecularTOD->HasChanged() ||
						m_GlanceSpecularTOD->HasChanged();

	return m_bRequireRefresh;
}