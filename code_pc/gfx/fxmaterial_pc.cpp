//--------------------------------------------------
//!
//!	\file fxmaterial.cpp
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#include "gfx/fxmaterial.h"
#include "gfx/material.h"
#include "gfx/renderer.h"
#include "gfx/rendercontext.h"
#include "gfx/shadowsystem.h"
#include "gfx/depthhazeconsts.h"
#include "gfx/graphicsdevice.h"
#include "anim/hierarchy.h"
#include "core/exportstruct_clump.h"
#include "core/timer.h"
#include "core/file.h"
#include "core/OSDDisplay.h"
#include "gfx/hardwarecaps.h"
#include "objectdatabase/dataobject.h"
#include "objectdatabase/objectdatabase.h"
#include "gfx/rendercontext.h"
#include "gfx/levellighting.h"
#include "core/fileattribute.h"

#include "effect/skin.h"

//#define USE_FX_MATERIALS

bool FXMaterialManager::s_bUseFXSafety = true;

//--------------------------------------------------
//!
//! 
//!
//--------------------------------------------------
FXHandle::FXHandle( const char* pFXFile, const char* pName, ID3DXEffectPool* pPool )
{
	ntAssert( pFXFile );
	ntAssert( pName );
	
	strcpy( m_pName, pName );

	char pFullName[128];
	Util::GetFiosFilePath( pFXFile, pFullName );

	strcpy( m_pFileName, pFullName );
	m_pPool = pPool;

#ifndef _RELEASE

	if (!File::Exists(m_pFileName))
	{
		one_time_assert_p( 0x2A, 0, ("Missing FX file: %s", m_pFileName));
	}

#endif

	ReloadMe();
}

//--------------------------------------------------
//!
//! ReloadMe: 
//! NOTE: This function has been updated to compile the shader into a shader object file
//! if the object file does not exist. Loading the object file is much quicker. If the object
//! file cannot be compiled then a .fxn file is created to indicate that the code should not
//! try to compile that file again until the timestamps on the files indicate it is safe to do 
//! so
//--------------------------------------------------
void FXHandle::ReloadMe( bool bForceRecompile )
{
	SafeRelease();
	HRESULT hr;
	
	// The path and name of the fxc compiler
	static const char* pUtility = "fxshaders\\fxc.exe";
	// Options sent to the fxc compiler
	static const char* pcOptions = "/nologo /T:fx_2_0 ";

	do
	{
		// Create an ntError buffer
		LPD3DXBUFFER pError = 0;

		// Build the object file name by sticking an 'o' onto the end of the filename
		char cObjectFileName[128];
		strcpy(cObjectFileName, m_pFileName);
		strcat(cObjectFileName, "o ");


		// Get modification time of source file
		CFileAttribute oSourceStat(m_pFileName);

		// Get modification time of object file
		CFileAttribute oObjectStat(cObjectFileName);
		
		if( FXMaterialManager::s_bUseFXSafety )
		{
			if( oObjectStat.GetModifyTime() < FXMaterialManager::Get().GetExeTime() )
			{
				bForceRecompile = true;
			}
		}
				
		// See if we need to attempt to build the object file
		//bool bBuildObjectFile = (lSourceTime > lObjectTime) ? true : false;

		bool bBuildObjectFile = oSourceStat.isNewerThan(oObjectStat);

		// Build the object file if it does not exist
		if (bBuildObjectFile || bForceRecompile)
		{
			ntPrintf("Compiling shader %s...\n", m_pName );

			// Remove the old .fxo file
			remove(cObjectFileName);

			// Structures needed to spawn process
			STARTUPINFO si;
			PROCESS_INFORMATION pi;
			ZeroMemory( &si, sizeof(si) );
			si.cb = sizeof(si);
			ZeroMemory( &pi, sizeof(pi) );

			// Build the command line
			char cBuffer[512];
			sprintf(cBuffer, "%s %s /Fo%s %s", pUtility, pcOptions, cObjectFileName, m_pFileName);

			// Wait until we can open the file.
			// This is necessary as it appears that a shader edited externally can trigger
			// modification whilst the shader is still open in the editor, so we wait until it is available for opening
			while (1)
			{
				// See if we can open the source file using the platform SDK.. this ensures that some other process does not have it open also
				HANDLE pTestCanOpen = CreateFile(m_pFileName, FILE_READ_DATA, 0, 0, OPEN_EXISTING, 0, 0);
				if (pTestCanOpen != INVALID_HANDLE_VALUE)
				{
					// Close the file again
					CloseHandle(pTestCanOpen);
					break;
				}
				//Wait for a second
				Sleep(100);
				// Display ntError
				ntPrintf("Error cannot open shader %s for compilation yet\n", m_pName );
				// Try again
			}

			// Start the child process. 
			if( !CreateProcess( NULL, // No module name (use command line). 
				cBuffer,			// Command line. 
				NULL,             // Process handle not inheritable. 
				NULL,             // Thread handle not inheritable. 
				FALSE,            // Set handle inheritance to FALSE. 
				CREATE_NO_WINDOW, // No creation flags.
				NULL,             // Use parent's environment block. 
				NULL,             // Use parent's starting directory. 
				&si,              // Pointer to STARTUPINFO structure.
				&pi )             // Pointer to PROCESS_INFORMATION structure.
			) 
			{
				//Hmm this could be a problem
				ntError_p(0, ("Could not find fxc.exe in the fxshader folder"));
			}

			// Wait for the process to end 90 seconds time out
			WaitForSingleObject(pi.hProcess, 90000L);

			// Close all handles
			CloseHandle(pi.hThread);	
			CloseHandle(pi.hProcess);	

			// Update object time to enable usage of the object file
			oObjectStat.SetModifyTime(oSourceStat.GetModifyTime());
		}
		// See if we can load the object file
		if (oObjectStat.GetModifyTime() >= oSourceStat.GetModifyTime())
		{
			// Create the effect file from the object file
			hr = D3DXCreateEffectFromFile( GetD3DDevice(), cObjectFileName, NULL, NULL, 0, m_pPool, AddressOf(), &pError );
			if( hr == S_OK )
				break;
		}

		// Create the effect file from the source file instead for the ntError message
		hr = D3DXCreateEffectFromFile( GetD3DDevice(), m_pFileName, NULL, NULL, 0, m_pPool, AddressOf(), &pError );

		if(pError)
		{
			ntPrintf("Error compiling shader %s:\n%s", m_pName, pError->GetBufferPointer() );
			pError->Release();
		}
		else
		{
			ntPrintf("Error compiling shader %s (no ntError message, sorry)\n", m_pName );
		}

		Sleep( 1000 );
	}
	while (1);

	CFileAttribute oTempStat(m_pFileName);
	m_modDate = oTempStat.GetModifyTime();
	
}

//--------------------------------------------------
//!
//! 
//!
//--------------------------------------------------
bool FXHandle::IsOutOfDate() const
{
	// check to see if our material needs refreshing
	CFileAttribute oTempStat(m_pFileName);
	if (oTempStat.GetModifyTime() > m_modDate) return true;

	return false;
}


FXMaterial::FXMaterial( const char* pFXFile, const char* pName ) :
	m_pEffectResource( pFXFile, pName ),
	m_nameHash( pName )
{
	// special material not comming from maya
	// probably very bad code...
	// don't hate me
	if ( GetNameHash() == CHashedString( "skin" ) )
	{
		m_mask.Set(FXMaterial::F_ISSKIN);
	}
	if ( GetNameHash() == CHashedString( "skinning_debug" ) )
	{
		m_mask.Set(FXMaterial::F_ISDEBUGSKINNING);
	}
}



//--------------------------------------------------
//!
//! 
//!
//--------------------------------------------------
FXMaterialManager::FXMaterialManager() :
	m_missingLambert( "fxshaders\\lambert_debug.fx", "lambert_debug" ),
	m_missingMetallic( "fxshaders\\metallic_debug.fx", "metallic_debug" ),
	m_missingPhong( "fxshaders\\phong_debug.fx", "phong_debug" ),
	m_missingUnknown( "fxshaders\\missing.fx", "missing" )
{

	if( s_bUseFXSafety )
	{
		// get which version of the game we are running
		char basefilename[128];
		const int iBufferSize = 128;
		GetModuleFileName( 0, basefilename, iBufferSize );
		_splitpath( basefilename, 0, 0, 0, 0 );

		CFileAttribute oTmpStat(basefilename);
		m_exeModDate = oTmpStat.GetModifyTime();

	} else
	{
		memset( &m_exeModDate, 0, sizeof(time_t) );
	}

	if ( HardwareCapabilities::Get().SupportsPixelShader3() )
	{
#ifdef USE_FX_MATERIALS

		m_materials.push_back( NT_NEW_CHUNK(Mem::MC_GFX) FXMaterial( "fxshaders\\uber_lambert_jambert1_off.fx",				"jambert1_off" ) );
		m_materials.push_back( NT_NEW_CHUNK(Mem::MC_GFX) FXMaterial( "fxshaders\\uber_lambert_jambert1n_off.fx",				"jambert1n_off" ) );
		m_materials.push_back( NT_NEW_CHUNK(Mem::MC_GFX) FXMaterial( "fxshaders\\uber_lambert_jambert3n_off_add_add.fx",		"jambert3n_off_add_add" ) );
		m_materials.push_back( NT_NEW_CHUNK(Mem::MC_GFX) FXMaterial( "fxshaders\\uber_lambert_jambert3n_off_add_ab_mod.fx",	"jambert3n_off_add_alphablend_modulate" ) );

		m_materials.push_back( NT_NEW_CHUNK(Mem::MC_GFX) FXMaterial( "fxshaders\\uber_metallic_jambert1_off.fx",			"metallic1" ) );
		m_materials.push_back( NT_NEW_CHUNK(Mem::MC_GFX) FXMaterial( "fxshaders\\uber_metallic_jambert1n_off.fx",			"metallic1n" ) );
		m_materials.push_back( NT_NEW_CHUNK(Mem::MC_GFX) FXMaterial( "fxshaders\\uber_metallic_jambert2n_off_add.fx",		"metallic2n_add" ) );

		m_materials.push_back( NT_NEW_CHUNK(Mem::MC_GFX) FXMaterial( "fxshaders\\uber_phong_jambert1_off.fx",				"phong1" ) );
		m_materials.push_back( NT_NEW_CHUNK(Mem::MC_GFX) FXMaterial( "fxshaders\\uber_phong_jambert1n_off.fx",			"phong1n" ) );
		m_materials.push_back( NT_NEW_CHUNK(Mem::MC_GFX) FXMaterial( "fxshaders\\uber_phong_jambert2_off_mod.fx",			"phong2_modulate" ) );

#endif

		m_materials.push_back( NT_NEW_CHUNK(Mem::MC_GFX) FXMaterial( "fxshaders\\uber_jambert1_unlit.fx",					"jambert1_unlit" ) );

		m_materials.push_back( NT_NEW_CHUNK(Mem::MC_GFX) FXMaterial( "fxshaders\\uber_lambert_jambert1_lerp.fx",			"jambert1_lerp" ) );
		m_materials.push_back( NT_NEW_CHUNK(Mem::MC_GFX) FXMaterial( "fxshaders\\uber_lambert_jambert1n_lerp.fx",			"jambert1n_lerp" ) );

		m_materials.push_back( NT_NEW_CHUNK(Mem::MC_GFX) FXMaterial( "fxshaders\\uber_lambert_jambert2_lerp_mod.fx",		"jambert2_lerp_modulate" ) );
		m_materials.push_back( NT_NEW_CHUNK(Mem::MC_GFX) FXMaterial( "fxshaders\\uber_lambert_jambert2n_lerp_mod.fx",		"jambert2n_lerp_modulate" ) );

		m_materials.push_back( NT_NEW_CHUNK(Mem::MC_GFX) FXMaterial( "fxshaders\\uber_lambert_jambert2_lerp_mod_ab.fx",	"jambert2_lerp_modulate_alphablend" ) );
		m_materials.push_back( NT_NEW_CHUNK(Mem::MC_GFX) FXMaterial( "fxshaders\\uber_lambert_jambert2n_lerp_mod_ab.fx",	"jambert2n_lerp_modulate_alphablend" ) );

		m_materials.push_back( NT_NEW_CHUNK(Mem::MC_GFX) FXMaterial( "fxshaders\\uber_lambert_jambert3n_lerp_ow_ab_mod.fx",	"jambert3n_lerp_overwrite_alphablend_modulate" ) );
		
		// new alpha blended phong materials (lambert is alreaded catered for under normal export)
		// these new alpha phong materials move the specular response channell from diffuse0.alpha to a
		// seperate texturemap, leaving the diffuse0.alpha chanel to control framebuffer alpha blending.

		m_materials.push_back( NT_NEW_CHUNK(Mem::MC_GFX) FXMaterial( "fxshaders\\uber_phong_jambert1_lerp.fx",			"phong1_lerp" ) );
		m_materials.push_back( NT_NEW_CHUNK(Mem::MC_GFX) FXMaterial( "fxshaders\\uber_phong_jambert1n_lerp.fx",			"phong1n_lerp" ) );

		m_materials.push_back( NT_NEW_CHUNK(Mem::MC_GFX) FXMaterial( "fxshaders\\uber_phong_jambert2_lerp_mod.fx",		"phong2_lerp_modulate" ) );
		m_materials.push_back( NT_NEW_CHUNK(Mem::MC_GFX) FXMaterial( "fxshaders\\uber_phong_jambert2n_lerp_mod.fx",		"phong2n_lerp_modulate" ) );

		m_materials.push_back( NT_NEW_CHUNK(Mem::MC_GFX) FXMaterial( "fxshaders\\uber_phong_jambert2_lerp_mod_ab.fx",		"phong2_lerp_modulate_alphablend" ) );
		m_materials.push_back( NT_NEW_CHUNK(Mem::MC_GFX) FXMaterial( "fxshaders\\uber_phong_jambert2n_lerp_mod_ab.fx",	"phong2n_lerp_modulate_alphablend" ) );

		// new alpha blended metalic materials (see above comment)

		m_materials.push_back( NT_NEW_CHUNK(Mem::MC_GFX) FXMaterial( "fxshaders\\uber_metallic_jambert1_lerp.fx",			"metallic1_lerp" ) );
		m_materials.push_back( NT_NEW_CHUNK(Mem::MC_GFX) FXMaterial( "fxshaders\\uber_metallic_jambert1n_lerp.fx",		"metallic1n_lerp" ) );

		m_materials.push_back( NT_NEW_CHUNK(Mem::MC_GFX) FXMaterial( "fxshaders\\uber_metallic_jambert2_lerp_mod.fx",		"metallic2_lerp_modulate" ) );
		m_materials.push_back( NT_NEW_CHUNK(Mem::MC_GFX) FXMaterial( "fxshaders\\uber_metallic_jambert2n_lerp_mod.fx",	"metallic2n_lerp_modulate" ) );

		m_materials.push_back( NT_NEW_CHUNK(Mem::MC_GFX) FXMaterial( "fxshaders\\uber_metallic_jambert2_lerp_mod_ab.fx",	"metallic2_lerp_modulate_alphablend" ) );
		m_materials.push_back( NT_NEW_CHUNK(Mem::MC_GFX) FXMaterial( "fxshaders\\uber_metallic_jambert2n_lerp_mod_ab.fx",	"metallic2n_lerp_modulate_alphablend" ) );


		m_materials.push_back( NT_NEW_CHUNK(Mem::MC_GFX) FXMaterial( "fxshaders\\skin.fx", "skin" ) );

		m_materials.push_back( NT_NEW_CHUNK(Mem::MC_GFX) FXMaterial( "fxshaders\\hair_nv40.fx", "hair" ) );
		m_materials.push_back( NT_NEW_CHUNK(Mem::MC_GFX) FXMaterial( "fxshaders\\clouds.fx", "clouds" ) );
		m_materials.push_back( NT_NEW_CHUNK(Mem::MC_GFX) FXMaterial( "fxshaders\\sky2clouds.fx", "sky2clouds" ) );
		m_materials.push_back( NT_NEW_CHUNK(Mem::MC_GFX) FXMaterial( "fxshaders\\clouds2.fx", "clouds2" ) );
		
#ifdef _SPEEDTREE
		// speedtree test
		m_materials.push_back( NT_NEW_CHUNK(Mem::MC_GFX) FXMaterial( "fxshaders\\speedtree_branch.fx", "speedtree_branch" ) );
		m_materials.push_back( NT_NEW_CHUNK(Mem::MC_GFX) FXMaterial( "fxshaders\\speedtree_frond.fx", "speedtree_frond" ) );
		m_materials.push_back( NT_NEW_CHUNK(Mem::MC_GFX) FXMaterial( "fxshaders\\speedtree_leaf.fx", "speedtree_leaf" ) );
		m_materials.push_back( NT_NEW_CHUNK(Mem::MC_GFX) FXMaterial( "fxshaders\\speedtree_billboard.fx", "speedtree_billboard" ) );
#endif

	} else
	{
		m_materials.push_back( NT_NEW_CHUNK(Mem::MC_GFX) FXMaterial( "fxshaders\\hair_ati.fx", "hair" ) );

	}

//	m_materials.push_back( NT_NEW_CHUNK(Mem::MC_GFX) FXMaterial( "fxshaders\\dean_test.fx",			"phong1n" ) );

	m_materials.push_back( NT_NEW_CHUNK(Mem::MC_GFX) FXMaterial( "fxshaders\\skinning_debug.fx",			"skinning_debug" ) );
	m_materials.push_back( NT_NEW_CHUNK(Mem::MC_GFX) FXMaterial( "fxshaders\\lambert_debug.fx",			"lambert_debug" ) );
	m_materials.push_back( NT_NEW_CHUNK(Mem::MC_GFX) FXMaterial( "fxshaders\\metallic_debug.fx",			"metallic_debug" ) );
	m_materials.push_back( NT_NEW_CHUNK(Mem::MC_GFX) FXMaterial( "fxshaders\\phong_debug.fx",				"phong_debug" ) );
	m_materials.push_back( NT_NEW_CHUNK(Mem::MC_GFX) FXMaterial( "fxshaders\\missing.fx",					"missing" ) );	

}

//--------------------------------------------------
//!
//! 
//!
//--------------------------------------------------
void FXMaterialManager::DebugUpdate()
{
#ifndef _RELEASE
	for (	FXMaterialList::iterator it = m_materials.begin(); 
			it != m_materials.end(); ++it )
	{
		FXMaterial& effect = *(*it);
		if (effect.GetEffect().IsOutOfDate())
		{
			effect.RefreshEffect();
			OSD::Add( OSD::DEBUG_CHAN, 0xffffffff, "Shader: %s reloaded", effect.GetName() );
		}
	}
#endif
}

//--------------------------------------------------
//!
//!	Make sure all shaders have been recompiled 
//!
//--------------------------------------------------
void FXMaterialManager::ForceRecompile()
{
	for (	FXMaterialList::iterator it = m_materials.begin(); 
			it != m_materials.end(); ++it )
	{
		FXMaterial& effect = *(*it);
		const_cast<FXHandle&>( effect.GetEffect() ).ReloadMe( true );
	}
}

//--------------------------------------------------
//!
//! 
//!
//--------------------------------------------------
FXMaterialManager::~FXMaterialManager()
{
	while (!m_materials.empty())
	{
		NT_DELETE_CHUNK(Mem::MC_GFX,  m_materials.back() );
		m_materials.pop_back();
	}
}

//--------------------------------------------------
//!
//! 
//!
//--------------------------------------------------
FXMaterial* FXMaterialManager::FindMaterial( const char* pName )
{
	FXMaterial* pMat = FindInternal( pName );
	
	if ( (!pMat) && (!strstr(pName,"_lerp")) && (!strstr(pName,"_off")) )
	{
		// possible this is an old material name, before the 
		// '_off' and '_lerp' frame buffer alpha blend postfixes.

		char pAlternate[128];
		strcpy( pAlternate, pName );

		strcat( pAlternate, "_off" );
		pMat = FindInternal( pName );
	}

#ifdef USE_FX_MATERIALS
#ifndef _RELEASE
	if (pMat == NULL)
	{
		ntPrintf( "!!!! !!!! WARNING: missing lit material %s !!!! !!!!\n", pName );

		char pLowerName[MAX_PATH];
		strcpy( pLowerName, pName );
		ntstd::transform( pLowerName, pLowerName + strlen( pLowerName ), pLowerName, &ntstd::Tolower );

		if (strstr(pLowerName,"jambert"))
		{
			return &m_missingLambert;
		}
		else if (strstr(pLowerName,"metallic"))
		{
			return &m_missingMetallic;
		}
		else if (strstr(pLowerName,"phong"))
		{
			return &m_missingPhong;
		}
		else
		{
			return &m_missingUnknown;
		}
	}
#endif
#endif

	return pMat;
}

//--------------------------------------------------
//!
//! 
//!
//--------------------------------------------------
FXMaterial* FXMaterialManager::FindMaterial( const CHashedString& name )
{
	FXMaterial* pMat = FindInternal( name );

#ifdef USE_FX_MATERIALS
#ifndef _RELEASE
	if (pMat == NULL)
	{
		ntPrintf( "!!!! !!!! WARNING: missing lit material of hash %d !!!! !!!!\n", name.GetValue() );
		return &m_missingUnknown;
	}
#endif
#endif

	return pMat;
}

//--------------------------------------------------
//!
//! 
//!
//--------------------------------------------------
FXMaterial*	FXMaterialManager::FindInternal( const char* pName )
{
	CHashedString name( pName );
	for (	FXMaterialList::iterator it = m_materials.begin();
			it != m_materials.end(); ++it )
	{
		if ( name == (*it)->GetNameHash() )
			return *it;
	}
	return 0;
}

//--------------------------------------------------
//!
//! 
//!
//--------------------------------------------------
FXMaterial*	FXMaterialManager::FindInternal( const CHashedString& name )
{
	for (	FXMaterialList::iterator it = m_materials.begin();
			it != m_materials.end(); ++it )
	{
		if ( name == (*it)->GetNameHash() )
			return *it;
	}
	return 0;
}





//--------------------------------------------------
//!
//! 
//!
//--------------------------------------------------
FXMaterialInstance::FXMaterialInstance(	const FXMaterial* pMaterial, 

												CMeshVertexElement* pVertexElements, 
												int iVertexElementCount, 

												CMaterialProperty const* pProperties, 
												int iPropertyCount ) :
	MaterialInstanceBase(true),
	m_pMaterial( pMaterial )
{
	SetPropertyTable(pProperties,iPropertyCount);
	SetVertexElement(pVertexElements,iVertexElementCount);

	// hmm, is this a hack?
	if ( strstr( pMaterial->GetName(), "_lerp" ) )
	{
		m_bIsAlphaBlended = true;
	}
	
	BindShaders();
}

void FXMaterialInstance::BindShaders()
{
	D3DVERTEXELEMENT9 astFormat[17];
	memset(&astFormat[0], 0, 17*sizeof(D3DVERTEXELEMENT9));
	
	int iBinding = 0;
	for ( ; iBinding < m_iVertexElementCount; iBinding++ )
	{
		astFormat[iBinding].Stream = 0;
		astFormat[iBinding].Offset = static_cast<WORD>( m_pVertexElements[iBinding].m_iOffset );
		astFormat[iBinding].Type = static_cast<BYTE>( m_pVertexElements[iBinding].m_eType );
		astFormat[iBinding].Method = D3DDECLMETHOD_DEFAULT;
	
		switch( m_pVertexElements[iBinding].m_eStreamSemanticTag )
		{
		case STREAM_POSITION:
			astFormat[iBinding].Usage		= D3DDECLUSAGE_POSITION;
			astFormat[iBinding].UsageIndex = 0;
			break;

		case STREAM_NORMAL:
			astFormat[iBinding].Usage		= D3DDECLUSAGE_NORMAL;
			astFormat[iBinding].UsageIndex = 0;
			break;

		case STREAM_TANGENT:
			astFormat[iBinding].Usage		= D3DDECLUSAGE_TANGENT;
			astFormat[iBinding].UsageIndex = 0;
			break;

		case STREAM_BINORMAL:
			astFormat[iBinding].Usage		= D3DDECLUSAGE_BINORMAL;
			astFormat[iBinding].UsageIndex = 0;
			break;

		case STREAM_BLENDWEIGHTS:
			astFormat[iBinding].Usage		= D3DDECLUSAGE_BLENDWEIGHT;
			astFormat[iBinding].UsageIndex = 0;
			break;

		case STREAM_BLENDINDICES:
			astFormat[iBinding].Usage		= D3DDECLUSAGE_BLENDINDICES;
			astFormat[iBinding].UsageIndex = 0;
			break;

		case STREAM_NORMAL_MAP_TEXCOORD:
			astFormat[iBinding].Usage		= D3DDECLUSAGE_TEXCOORD;
			astFormat[iBinding].UsageIndex = 0;
			break;

		case STREAM_DIFFUSE_TEXCOORD0:
			astFormat[iBinding].Usage		= D3DDECLUSAGE_TEXCOORD;
			astFormat[iBinding].UsageIndex = 1;
			break;

		case STREAM_DIFFUSE_TEXCOORD1:
			astFormat[iBinding].Usage		= D3DDECLUSAGE_TEXCOORD;
			astFormat[iBinding].UsageIndex = 2;
			break;

		case STREAM_DIFFUSE_TEXCOORD2:
			astFormat[iBinding].Usage		= D3DDECLUSAGE_TEXCOORD;
			astFormat[iBinding].UsageIndex = 3;
			break;

		default:
			ntError( 0 );
			break;
		}
	}

	static const D3DVERTEXELEMENT9 stEnd = D3DDECL_END();
	NT_MEMCPY( &astFormat[iBinding], &stEnd, sizeof( D3DVERTEXELEMENT9 ) );

	m_pDecl = CVertexDeclarationManager::Get().GetDeclaration( &astFormat[0] );
}


//--------------------------------------------------
//!
//! Called before rendering vertex data.
//!
//--------------------------------------------------
void FXMaterialInstance::PreRender( const Transform* pobTransform, bool bIsReceivingShadow, void const* ) const
{
	InternalPreRender( pobTransform, false, false, false, bIsReceivingShadow );
}
void FXMaterialInstance::PreRenderDepth( const Transform* pobTransform, bool bShadowProject, void const* ) const
{
	InternalPreRender( pobTransform, true, bShadowProject, false, false );
}
void FXMaterialInstance::PreRenderShadowRecieve( const Transform* pobTransform, void const* ) const
{
	InternalPreRender( pobTransform, false, false, true, false );
}
void FXMaterialInstance::PostRenderDepth( bool bShadowProject ) const
{
	UNUSED(bShadowProject);
	PostRender();
}
void FXMaterialInstance::PostRenderShadowRecieve() const
{
	PostRender();
}
void FXMaterialInstance::InternalPreRender( const Transform* pobTransform, bool bDepth, bool bShadowProject, bool bShadowRecieve, bool bPickupShadows ) const
{
	int iTechnique = 0;

	if ( m_eBoundType == VSTT_SKINNED )
		iTechnique += 5;

	if ( m_eBoundType == VSTT_BATCHED )
		iTechnique += 10;

	if( bDepth || bShadowProject || bShadowRecieve )
	{
		if( bDepth )
			iTechnique += 2;
		else if( bShadowRecieve )
		{
			if( HardwareCapabilities::Get().SupportsHardwareShadowMaps() )
			{
				iTechnique += 4;
			} else
			{
				iTechnique += 3;
			}
		}
	} else
	{
		if ( CShadowSystemController::Get().IsShadowMapActive() && bPickupShadows)
			iTechnique += 1;
	}
	
	Renderer::Get().m_Platform.SetVertexDeclaration( m_pDecl );
	HRESULT hr;
	hr = m_pMaterial->GetEffect()->SetTechnique( FXMaterial::GetTechniqueName( (FXMaterial::FX_MAT_TECHNIQUE) iTechnique ) );
	ntAssert_p( hr == S_OK,(	"Technique %s not found in this FX file (%s)",
							FXMaterial::GetTechniqueName( (FXMaterial::FX_MAT_TECHNIQUE) iTechnique ),
							m_pMaterial->GetEffect().GetFileName() ) );

	// get some useful matrices
	// this bit if code was in UploadObjectParameters() before
	// move it here because worldToObject was also needed in UploadMaterialParameters
	CMatrix worldToObject;
	if (pobTransform)
	{
		worldToObject = pobTransform->GetWorldMatrixFast().GetAffineInverse();
	}
	else
	{
		worldToObject.SetIdentity();
	}

	// load shared global parameters
	UploadGlobalParameters();

	// load object specific parameters
	UploadObjectParameters( pobTransform, worldToObject );

	// load material specific properties
	UploadMaterialParameters( worldToObject );

	// ready to kick off the effect
	u_int iNumPasses;
	m_pMaterial->GetEffect()->Begin( &iNumPasses, 0 );

	ntError_p( iNumPasses == 1, ("Multipass not supported yet") );
	m_pMaterial->GetEffect()->BeginPass(0);
}

//--------------------------------------------------
//!
//! Called after rendering vertex data.
//!
//--------------------------------------------------
void FXMaterialInstance::PostRender() const
{
	m_pMaterial->GetEffect()->EndPass();
	m_pMaterial->GetEffect()->End();
}

//--------------------------------------------------
//!
//! Send up parameters used by all shared pooled effects.
//! NB. this means we need to batch all fx based materials
//! together, or we'll trash constant space with non fx
//! uploads. For the moment we'll upload every mesh.
//!
//!	NB would be much more efficient to cache parameters in handles
//!
//--------------------------------------------------
void FXMaterialInstance::UploadGlobalParameters() const
{
	FXMaterial::UploadGlobalParameters( m_pMaterial->GetEffect() );
}

void FXMaterial::UploadGlobalParameters( const FXHandle& handle )
{
	// PROPERTY_FILL_SH_MATRICES
	{
		FX_SET_VALUE_VALIDATE( handle, "g_fillSHCoeffs", RenderingContext::Get()->m_SHMatrices.m_aChannelMats, sizeof(CMatrix) * 3 );
	}
	
	// PROPERTY_KEY_DIR_COLOUR
	{
		FX_SET_VALUE_VALIDATE( handle, "g_keyDirColour", &RenderingContext::Get()->m_keyColour, sizeof(float) * 3 );
	}

	// PROPERTY_KEY_DIR_WORLDSPACE
	// PROPERTY_KEY_DIR_REFLECTANCESPACE
	{
		FX_SET_VALUE_VALIDATE( handle, "g_keyDirReflect", &RenderingContext::Get()->m_toKeyLight, sizeof(float) * 3 );
	}

	// PROPERTY_DEPTH_HAZE_CONSTS_A
	{
		const CVector temp = CDepthHazeSetting::GetAConsts();
		FX_SET_VALUE_VALIDATE( handle, "g_DHConstsA", &temp, sizeof(float) * 3 );
	}

	// PROPERTY_DEPTH_HAZE_CONSTS_G
	{
		const CVector temp = CDepthHazeSetting::GetGConsts();
		FX_SET_VALUE_VALIDATE( handle, "g_DHConstsG", &temp, sizeof(float) * 3 );
	}

	// PROPERTY_DEPTH_HAZE_BETA1PLUSBETA2
	{
		const CVector temp = CDepthHazeSetting::GetBeta1PlusBeta2();
		FX_SET_VALUE_VALIDATE( handle, "g_DHB1plusB2", &temp, sizeof(float) * 3 );		
	}

	// PROPERTY_DEPTH_HAZE_RECIP_BETA1PLUSBETA2:
	{
		const CVector temp = CDepthHazeSetting::GetOneOverBeta1PlusBeta2();
		FX_SET_VALUE_VALIDATE( handle, "g_DHRecipB1plusB2", &temp, sizeof(float) * 3 );		
	}

	// PROPERTY_DEPTH_HAZE_BETADASH1
	{
		const CVector temp = CDepthHazeSetting::GetBetaDash1();
		FX_SET_VALUE_VALIDATE( handle, "g_DHBdash1", &temp, sizeof(float) * 3 );
	}

	// PROPERTY_DEPTH_HAZE_BETADASH2
	{
		const CVector temp = CDepthHazeSetting::GetBetaDash2();
		FX_SET_VALUE_VALIDATE( handle, "g_DHBdash2", &temp, sizeof(float) * 3 );
	}

	// PROPERTY_SUN_COLOUR
	{
		const CVector temp = CDepthHazeSetting::GetSunColour();
		FX_SET_VALUE_VALIDATE( handle, "g_sunColour", &temp, sizeof(float) * 4 );
	}

	// PROPERTY_REFLECTANCE_MAP_COLOUR
	{
		FX_SET_VALUE_VALIDATE( handle, "g_reflectanceColour", &RenderingContext::Get()->m_reflectanceCol, sizeof(float) * 3 );
	}

	// PROPERTY_PARALLAX_SCALE_AND_BIAS
	{

		CVector scaleAndBias( CONSTRUCT_CLEAR );
		if (CRendererSettings::bUseParallaxMapping)
		{
			scaleAndBias.X() = 0.02f;
			scaleAndBias.Y() = -0.01f;
		}

		FX_SET_VALUE_VALIDATE( handle, "g_parallaxScaleAndBias", &scaleAndBias, sizeof(float) * 2 );
	}

	// PROPERTY_VPOS_TO_UV
	{
		CVector posToUV( CONSTRUCT_CLEAR );
		posToUV.X() = 1.f / (Renderer::Get().m_targetCache.GetWidth() - 1);
		posToUV.Y() = 1.f / (Renderer::Get().m_targetCache.GetHeight() - 1);

		FX_SET_VALUE_VALIDATE( handle, "g_VPOStoUV", &posToUV, sizeof(float) * 2 );
	}

	// PROPERTY_DEPTHOFFIELD_PARAMS:
	{
		FX_SET_VALUE_VALIDATE( handle, "g_depthOfFieldParams", &RenderingContext::Get()->m_depthOfFieldParams, sizeof(float) * 3 );		
	}

	D3DXHANDLE h;

	if ( CShadowSystemController::Get().IsShadowMapActive() )
	{
		// TEXTURE_STENCIL_MAP
		if (RenderingContext::Get()->m_pStencilTarget)
		{
			FX_GET_HANDLE_FROM_NAME( handle, h, "g_stencilMap" );
			handle->SetTexture( h, RenderingContext::Get()->m_pStencilTarget->m_Platform.Get2DTexture() );
		}

		// PROPERTY_SHADOW_MAP_RESOLUTION
		if (RenderingContext::Get()->m_pShadowMap)
		{
			float fWidth = _R( RenderingContext::Get()->m_pShadowMap->GetWidth() );
			float fHeight = _R( RenderingContext::Get()->m_pShadowMap->GetHeight() );

			const CVector temp( fWidth, fHeight, 1.0f/fWidth, 1.0f/fHeight );
			FX_SET_VALUE_VALIDATE( handle, "g_shadowMapResolution", &temp, sizeof(float) * 4 );

			// TEXTURE_SHADOW_MAP
			FX_GET_HANDLE_FROM_NAME( handle, h, "g_shadowMap" );
			handle->SetTexture( h, RenderingContext::Get()->m_pShadowMap->m_Platform.Get2DTexture() );
			// TEXTURE_SHADOW_MAP1
			FX_GET_HANDLE_FROM_NAME( handle, h, "g_shadowMap1" );
			handle->SetTexture( h, RenderingContext::Get()->m_pShadowMap->m_Platform.Get2DTexture() );
			// TEXTURE_SHADOW_MAP2
			FX_GET_HANDLE_FROM_NAME( handle, h, "g_shadowMap2" );
			handle->SetTexture( h, RenderingContext::Get()->m_pShadowMap->m_Platform.Get2DTexture() );
			// TEXTURE_SHADOW_MAP3
			FX_GET_HANDLE_FROM_NAME( handle, h, "g_shadowMap3" );
			handle->SetTexture( h, RenderingContext::Get()->m_pShadowMap->m_Platform.Get2DTexture() );
		}

		// PROPERTY_SHADOW_PLANE0 - PROPERTY_SHADOW_PLANE4
		FX_SET_VALUE_VALIDATE( handle, "g_shadowPlane0", &RenderingContext::Get()->m_shadowPlanes[0], sizeof(float) * 4 );
		FX_SET_VALUE_VALIDATE( handle, "g_shadowPlane1", &RenderingContext::Get()->m_shadowPlanes[1], sizeof(float) * 4 );
		FX_SET_VALUE_VALIDATE( handle, "g_shadowPlane2", &RenderingContext::Get()->m_shadowPlanes[2], sizeof(float) * 4 );
		FX_SET_VALUE_VALIDATE( handle, "g_shadowPlane3", &RenderingContext::Get()->m_shadowPlanes[3], sizeof(float) * 4 );
		FX_SET_VALUE_VALIDATE( handle, "g_shadowPlane4", &RenderingContext::Get()->m_shadowPlanes[4], sizeof(float) * 4 );


		FX_SET_VALUE_VALIDATE( handle, "g_shadowRadii0", &RenderingContext::Get()->m_shadowRadii[0], sizeof(float) * 4 );
		FX_SET_VALUE_VALIDATE( handle, "g_shadowRadii1", &RenderingContext::Get()->m_shadowRadii[1], sizeof(float) * 4 );
		FX_SET_VALUE_VALIDATE( handle, "g_shadowRadii2", &RenderingContext::Get()->m_shadowRadii[2], sizeof(float) * 4 );
		FX_SET_VALUE_VALIDATE( handle, "g_shadowRadii3", &RenderingContext::Get()->m_shadowRadii[3], sizeof(float) * 4 );
		
	}

	// TEXTURE_REFLECTANCE_MAP
	{
		FX_GET_HANDLE_FROM_NAME( handle, h, "g_reflectanceMap" );
		handle->SetTexture( h, RenderingContext::Get()->m_reflectanceMap->m_Platform.GetCubeTexture() );
	}
}

//--------------------------------------------------
//!
//! Send up parameters specific to this object
//!
//--------------------------------------------------
void FXMaterialInstance::UploadObjectParameters( const Transform* pTransform, const CMatrix& worldToObject ) const
{
	const CMatrix& objectToWorld = pTransform->GetWorldMatrixFast();
	
	FXMaterial::UploadObjectParameters( m_pMaterial->GetEffect(), objectToWorld, worldToObject );
	
	// PROPERTY_BLEND_TRANSFORMS:
	if (m_eBoundType == VSTT_SKINNED)
	{
		ntError_p( pTransform, ("Must have a valid transform, or we have no heirachy") );

		// ensure the transforms are up to date on the hierarchy
		pTransform->GetParentHierarchy()->UpdateSkinMatrices();
		ntError_p( m_pucBoneIndices, ( "no bone indices set on skinned material instance" ) );

		static const int iMaxBones = 200;
		ntAssert( m_iNumberOfBonesUsed < iMaxBones );

		static CSkinMatrix aTempMatrix[iMaxBones];

		// upload only the matrices used by this mesh
		for(int iBone = 0; iBone < m_iNumberOfBonesUsed; ++iBone)
		{
			// get the matrix index
			int iMatrixIndex = m_pucBoneIndices[iBone];

			// get the matrix for the given index
			const CSkinMatrix* pobBone = &pTransform->GetParentHierarchy()->GetSkinMatrixArray()[iMatrixIndex];

			aTempMatrix[iBone] = *pobBone;
		}
		
		// upload array
		FX_SET_VALUE_RAW( m_pMaterial->GetEffect(), "m_blendMats", aTempMatrix, sizeof(CSkinMatrix) * m_iNumberOfBonesUsed );
	}
}

void FXMaterial::UploadObjectParameters(	const FXHandle& handle,
												const CMatrix& objectToWorld,
												const CMatrix& worldToObject )
{
	// PROPERTY_PROJECTION:
	{
		CMatrix worldViewProj = objectToWorld * RenderingContext::Get()->m_worldToScreen;
		FX_SET_VALUE_VALIDATE( handle, "m_worldViewProj", &worldViewProj, sizeof(CMatrix) );
	}

	// PROPERTY_VIEW_TRANSFORM
	{
		CMatrix viewMat = objectToWorld * RenderingContext::Get()->m_worldToView;
		FX_SET_VALUE_VALIDATE( handle, "m_worldView", &viewMat, sizeof(CMatrix) );
	}

	// PROPERTY_WORLD_TRANSFORM:
	// PROPERTY_REFLECTANCE_MAP_TRANSFORM:
	{
		FX_SET_VALUE_VALIDATE( handle, "m_world", &objectToWorld, sizeof(CMatrix) );
		FX_SET_VALUE_VALIDATE( handle, "m_reflectanceMat", &objectToWorld, sizeof(CMatrix) );
	}

	// PROPERTY_KEY_DIR_OBJECTSPACE
	{
		CDirection keyDir = RenderingContext::Get()->m_toKeyLight * worldToObject;
		FX_SET_VALUE_VALIDATE( handle, "m_keyLightDir_objectS", &keyDir, sizeof(float) * 3 );
	}

	// PROPERTY_SUN_DIRECTION_OBJECTSPACE
	{	
		CDirection temp( CDepthHazeSetting::GetSunDir() * worldToObject );
		FX_SET_VALUE_VALIDATE( handle, "m_sunDir_objectS", &temp, sizeof(float) * 3 );
	}

	if	( CShadowSystemController::Get().IsShadowMapActive() )
	{
		{
			// PROPERTY_SHADOW_MAP_TRANSFORM
			CMatrix shadowMat = objectToWorld * RenderingContext::Get()->m_shadowMapProjection[0];
			FX_SET_VALUE_VALIDATE( handle, "m_shadowMapMat", &shadowMat, sizeof(CMatrix) );
		}
		{
			// PROPERTY_SHADOW_MAP_TRANSFORM1
			CMatrix shadowMat = objectToWorld * RenderingContext::Get()->m_shadowMapProjection[1];
			FX_SET_VALUE_VALIDATE( handle, "m_shadowMapMat1", &shadowMat, sizeof(CMatrix) );
		}
		{
			// PROPERTY_SHADOW_MAP_TRANSFORM2
			CMatrix shadowMat = objectToWorld * RenderingContext::Get()->m_shadowMapProjection[2];
			FX_SET_VALUE_VALIDATE( handle, "m_shadowMapMat2", &shadowMat, sizeof(CMatrix) );
		}
		{
			// PROPERTY_SHADOW_MAP_TRANSFORM3
			CMatrix shadowMat = objectToWorld * RenderingContext::Get()->m_shadowMapProjection[3];
			FX_SET_VALUE_VALIDATE( handle, "m_shadowMapMat3", &shadowMat, sizeof(CMatrix) );
		}

	}

	// PROPERTY_VIEW_POSITION_OBJECTSPACE
	{
		CPoint viewPos = RenderingContext::Get()->GetEyePos() * worldToObject;
		FX_SET_VALUE_VALIDATE( handle, "m_viewPosition_objectS", &viewPos, sizeof(float) * 3 );
	}
}

//--------------------------------------------------
//!
//! Send up parameters specific to this material
//!
//--------------------------------------------------
void FXMaterialInstance::UploadMaterialParameters(const CMatrix& worldToObject) const
{
	D3DXHANDLE h;

	for ( int i = 0; i < GetPropertyTableSize(); i++ )
	{
		const CMaterialProperty* pCurr = &(GetPropertyTable()[i]);
		
		switch ( pCurr->m_iPropertyTag )
		{
		// shader properties
		//---------------------------------------------------------
		case PROPERTY_ALPHATEST_THRESHOLD:
		case PROPERTY_FRESNEL_EFFECT:
		case PROPERTY_SPECULAR_POWER:
			{
				FX_SET_VALUE_VALIDATE(	m_pMaterial->GetEffect(),
										FXMaterial::GetFXPropertyTagString( pCurr->m_iPropertyTag ),
										pCurr->m_uData.stFloatData.afFloats, sizeof(float) * 1 );
			}
			break;

		case PROPERTY_DIFFUSE_COLOUR0:
		case PROPERTY_DIFFUSE_COLOUR1:
		case PROPERTY_DIFFUSE_COLOUR2:
			{
				FX_SET_VALUE_VALIDATE(	m_pMaterial->GetEffect(),
										FXMaterial::GetFXPropertyTagString( pCurr->m_iPropertyTag ),
										pCurr->m_uData.stFloatData.afFloats, sizeof(float) * 4 );
			}
			break;

		case PROPERTY_REFLECTANCE_COLOUR:
		case PROPERTY_SPECULAR_COLOUR:
			{
				FX_SET_VALUE_VALIDATE(	m_pMaterial->GetEffect(),
										FXMaterial::GetFXPropertyTagString( pCurr->m_iPropertyTag ),
										pCurr->m_uData.stFloatData.afFloats, sizeof(float) * 3 );
			}
			break;
			
		/////////////////////////////
		// hair
		case PROPERTY_SPECULAR_POWER2:
			{
				FX_SET_VALUE_VALIDATE( m_pMaterial->GetEffect(), "m_specularPower2", pCurr->m_uData.stFloatData.afFloats, sizeof(float) * 1 );
			}
			break;
		case PROPERTY_SPECULAR_COLOUR2:
			{
				FX_SET_VALUE_VALIDATE( m_pMaterial->GetEffect(), "m_specularColour2", pCurr->m_uData.stFloatData.afFloats, sizeof(float) * 3 );
			}
			break;
		case PROPERTY_SPECULAR_SHIFT:
			{
				FX_SET_VALUE_VALIDATE( m_pMaterial->GetEffect(), "m_hairSpecularShift", pCurr->m_uData.stFloatData.afFloats, sizeof(float) * 1 );
			}
			break;
		case PROPERTY_SPECULAR_SHIFT2:
			{
				FX_SET_VALUE_VALIDATE( m_pMaterial->GetEffect(), "m_hairSpecularShift2", pCurr->m_uData.stFloatData.afFloats, sizeof(float) * 1 );
			}
			break;
		case PROPERTY_FAKE_LIGHT:
			{	
				CDirection objectRimLightDir = CDirection(0.0f,0.0f,0.0f);
				//CDirection objectRimLightDir = LevelLighting::Get().GetWorldRimLight(
				//	pCurr->m_uData.stFloatData.afFloats[0],
				//	RenderingContext::Get()->m_toKeyLight,
				//	- RenderingContext::Get()->m_worldToView.GetAffineInverse().GetZAxis()) * worldToObject;
				FX_SET_VALUE_VALIDATE( m_pMaterial->GetEffect(), "m_rimLight_objectS", &objectRimLightDir, sizeof(float) * 3 );
			}
			break;
		// hair
		/////////////////////////////

		case PROPERTY_SNOW_DIFFUSE_COLOUR:
		case PROPERTY_SNOW_SPECULAR_COLOUR:
		case PROPERTY_SNOW_INTENSITY:
		case PROPERTY_ICE_FROST:
		case PROPERTY_REFRACTION_WARP:
		case PROPERTY_CAMERA_UNIT_AXIS_X:
		case PROPERTY_CAMERA_UNIT_AXIS_Y:
		case PROPERTY_CAMERA_Z_WITH_OFFSET:
		case PROPERTY_UVSCROLL0:
		case PROPERTY_UVSCROLL1:
		case PROPERTY_UVSCROLL2:
		case PROPERTY_UVSCROLL3:
		case PROPERTY_UVSCROLL4:
		case PROPERTY_UVSCROLL5:
		case PROPERTY_OVERALL_AMBIENT:
		case PROPERTY_PROGRAMMER_EFFECTS_END:
		case PROPERTY_PROGRAMMER_EFFECTS_START:
			ntError_p( 0, ("Unsupported property semantics used") );
			break;

		case PROPERTY_BATCH_TRANSFORMS:
		case PROPERTY_BLEND_TRANSFORMS:
		case PROPERTY_DEPTH_HAZE_BETA1PLUSBETA2:
		case PROPERTY_DEPTH_HAZE_BETADASH1:
		case PROPERTY_DEPTH_HAZE_BETADASH2:
		case PROPERTY_DEPTH_HAZE_CONSTS_A:
		case PROPERTY_DEPTH_HAZE_CONSTS_G:
		case PROPERTY_DEPTH_HAZE_RECIP_BETA1PLUSBETA2:
		case PROPERTY_FILL_SH:
		case PROPERTY_FILL_SH_MATRICES:
		case PROPERTY_GAME_TIME:
		case PROPERTY_KEY_DIR_COLOUR:
		case PROPERTY_KEY_DIR_OBJECTSPACE:
		case PROPERTY_KEY_DIR_REFLECTANCESPACE:
		case PROPERTY_KEY_DIR_WORLDSPACE:				
		case PROPERTY_PROJECTION:
		case PROPERTY_PROJECTION_NON_TRANSPOSE:
		case PROPERTY_REFLECTANCE_MAP_COLOUR:
		case PROPERTY_REFLECTANCE_MAP_TRANSFORM:
		case PROPERTY_SHADOW_MAP_RESOLUTION:
		case PROPERTY_SHADOW_MAP_TRANSFORM:
		case PROPERTY_SUN_COLOUR:
		case PROPERTY_SUN_DIRECTION_OBJECTSPACE:
		case PROPERTY_VIEWPORT_RECIPRICAL_SIZE:
		case PROPERTY_VIEWPORT_SCALARS:
		case PROPERTY_VIEW_POSITION_OBJECTSPACE:
		case PROPERTY_VIEW_TRANSFORM:
		case PROPERTY_WORLD_TRANSFORM:
			ntError_p( 0, ("Property semantic is not user configurable") );
			break;

		// texture properties
		//---------------------------------------------------------
		case TEXTURE_NORMAL_MAP:
		case TEXTURE_DIFFUSE0:
		case TEXTURE_DIFFUSE1:
		case TEXTURE_DIFFUSE2:
			{
				FX_GET_HANDLE_FROM_NAME( m_pMaterial->GetEffect(), h, FXMaterial::GetFXPropertyTagString( pCurr->m_iPropertyTag ) );
				m_pMaterial->GetEffect()->SetTexture( h, pCurr->m_uData.stTextureData.pobTexture );
			}
			break;


		case TEXTURE_NORMALISATION_CUBE_MAP:
		case TEXTURE_SPRITE:
			ntError_p( 0, ("Unsupported texture semantics used") );
			break;

		case TEXTURE_REFLECTANCE_MAP:
		case TEXTURE_SHADOW_MAP:
		case TEXTURE_STENCIL_MAP:
			ntError_p( 0, ("Texture semantic is not user configurable yet") );
			break;

		// unsupported
		case PROPERTY_ANISOTROPIC_FILTERING:
		case PROPERTY_PARALLAX_SCALE_AND_BIAS:
			break;

		// ntError
		//---------------------------------------------------------
		case -1:
		default:
			ntError_p( 0, ("Unrecognised semantic tag (%d)", pCurr->m_iPropertyTag ));
			break;
		}
	}	

	//
	//	Make sure we set the relevant stuff for skin shaders - ARV.
	//
	//if ( m_pMaterial->GetNameHash() == CHashedString( "skin" ) )
	if ( m_pMaterial->GetMask().AllOfThem(FXMaterial::F_ISSKIN)  )
	{
		SkinDef *skin_def( ObjectDatabase::Get().GetPointerFromName< SkinDef * >( "SkinDefinition" ) );

		if ( skin_def != NULL )
		{
			skin_def->SetSkinParameters( m_pMaterial->GetEffect(), worldToObject );
		}
	}

	if ( m_pMaterial->GetMask().AllOfThem(FXMaterial::F_ISDEBUGSKINNING)  )
	{
		Pixel4 p = GetDebugIndices();
		FX_SET_VALUE_VALIDATE( m_pMaterial->GetEffect(), "m_lookFor", &p[0], sizeof(int) * 4 );
	}

}


