//--------------------------------------------------
//!
//!	\file fxmaterial_ps3.cpp
//! Just a stub to get the game linking
//!
//--------------------------------------------------

#include "gfx/fxmaterial.h"

bool FXMaterialManager::s_bUseFXSafety = true;

FXHandle::FXHandle( const char* pFXFile, const char* pName, ID3DXEffectPool* pPool )
{
	ntAssert( pFXFile );
	ntAssert( pName );
	UNUSED( pPool );
}

void FXHandle::ReloadMe( bool bForceRecompile )
{
	UNUSED( bForceRecompile );
}
bool FXHandle::IsOutOfDate() const
{
	return true;
}
FXMaterial::FXMaterial( const char* pFXFile, const char* pName )
{
	UNUSED( pFXFile );
	UNUSED( pName );
}


FXMaterialManager::FXMaterialManager() :
	m_missingLambert( "fxshaders\\lambert_debug.fx", "lambert_debug" ),
	m_missingMetallic( "fxshaders\\metallic_debug.fx", "metallic_debug" ),
	m_missingPhong( "fxshaders\\phong_debug.fx", "phong_debug" ),
	m_missingUnknown( "fxshaders\\missing.fx", "missing" )
{
}

FXMaterialManager::~FXMaterialManager()
{
}

void FXMaterialManager::DebugUpdate()
{
}

FXMaterial*	FXMaterialManager::FindMaterial( const char* pName )
{
	UNUSED( pName );
	return 0;
}

FXMaterial*	FXMaterialManager::FindMaterial( const CHashedString& name )
{
	UNUSED( name );
	return 0;
}
void FXMaterialManager::ForceRecompile()
{
}

