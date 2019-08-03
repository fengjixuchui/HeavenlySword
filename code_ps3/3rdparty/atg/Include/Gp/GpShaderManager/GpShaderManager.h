//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		Basic Shader Resource Manager.
	
	@note		(c) Copyright Sony Computer Entertainment 2005. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef GP_SHADER_MANAGER_H
#define GP_SHADER_MANAGER_H

#include <Fp/FpObjectManager/FpObjectManager.h>
#include <Gc/GcShader.h>

//--------------------------------------------------------------------------------------------------
/**
	@class			GpShaderManager
	
	@brief			Shader resource manager, based on FpObjectManager implementation.
**/
//--------------------------------------------------------------------------------------------------

class GpShaderManager : public FwSingleton< GpShaderManager >, public FpObjectManager< GcShader >
{
public:
	
	// Initialise & shutdown
	
	static void				Initialise( void )	{	FW_NEW GpShaderManager();	}
	static void				Shutdown( void )	{	GpShaderManager::Destroy();		}
};

//--------------------------------------------------------------------------------------------------

#endif // GP_SHADER_MANAGER_H
