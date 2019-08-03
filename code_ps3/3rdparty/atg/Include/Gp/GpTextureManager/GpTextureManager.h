//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		Basic Texture Resource Manager.
	
	@note		(c) Copyright Sony Computer Entertainment 2005. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef GP_TEXTURE_MANAGER_H
#define GP_TEXTURE_MANAGER_H

#include <Fp/FpObjectManager/FpObjectManager.h>
#include <Gc/GcTexture.h>

//--------------------------------------------------------------------------------------------------
/**
	@class			GpTextureManager
	
	@brief			Texture resource manager, based on FpObjectManager implementation.
**/
//--------------------------------------------------------------------------------------------------

class	GpTextureManager : public FwSingleton< GpTextureManager >, public FpObjectManager< GcTexture >
{
public:
	// Initialise & shutdown
	static void				Initialise( void )	{	FW_NEW GpTextureManager();		}
	static void				Shutdown( void )	{	GpTextureManager::Destroy();	}

protected:
	// Construction
	GpTextureManager();

	static GcTextureHandle	LoadTexture( const FwResourceHandle& hResource );
};

//--------------------------------------------------------------------------------------------------

#endif // GP_TEXTURE_MANAGER_H
