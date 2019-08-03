//--------------------------------------------------
//!
//!	\file clump_pc.cpp
//! PC file that loads a clump
//!
//--------------------------------------------------

#include "gfx/clump.h"
#include "core/exportstruct_clump.h"
#include "gfx/renderer.h"
#include "core/semantics.h"
#include "core/file.h"
#include "gfx/texturemanager.h"
#include "gfx/fxmaterial.h"
#include "gfx/graphicsdevice.h"
#include "area/arearesourcedb.h"

//!!!!!!!!!!!!!!!!!!!! TEMP HACK !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
extern bool g_bAllowOldData;
extern bool g_bAllowMissingData;

//#define _DEBUG_OUTPUT_MESH_OPTIMISATION

static void OptimiseMesh( CMeshHeader* pobMeshHeader );

static void LockAndCopyVertexData( VBHandle pobBuffer, void const* pvData, int iDataSize )
{
	// copy in the vertex data	
	void* pvLock =0;
	HRESULT hr;
	hr = pobBuffer->Lock( 0, 0, &pvLock, 0 );
	ntAssert( SUCCEEDED( hr ) );
	NT_MEMCPY( pvLock, pvData, iDataSize );
	pobBuffer->Unlock();

}

static VBHandle CreateStaticVertexBuffer( void const* pvData, int iDataSize )
{
	// allocate a buffer
	VBHandle pobBuffer = Renderer::Get().m_Platform.CreateStaticVertexBuffer( iDataSize );
	
	LockAndCopyVertexData( pobBuffer, pvData, iDataSize );

	// return the buffer
	return pobBuffer;
}

static VBHandle CreateSystemVertexBuffer( void const* pvData, int iDataSize )
{
	// allocate a buffer
	VBHandle pobBuffer = Renderer::Get().m_Platform.CreateSystemVertexBuffer( iDataSize );

	LockAndCopyVertexData( pobBuffer, pvData, iDataSize );

	// return the buffer
	return pobBuffer;
}


static IBHandle CreateIndexBuffer( void const* pvData, int iDataSize )
{
	// allocate a buffer
	IBHandle pobBuffer = Renderer::Get().m_Platform.CreateStaticIndexBuffer( iDataSize );
	
	// copy in the index data	
	void* pvLock =0;
	HRESULT hr;
	hr = pobBuffer->Lock( 0, 0, &pvLock, 0 );
	ntAssert( SUCCEEDED( hr ) );
	NT_MEMCPY( pvLock, pvData, iDataSize );
	pobBuffer->Unlock();

	// return the buffer
	return pobBuffer;
}





/***************************************************************************************************
*
*	FUNCTION		ClumpLoad_Complete
*
*	DESCRIPTION		Loads a clump and makes it available for use by calling modules. This involves
*					resolving all offsets into absolute pointers, constructing all GFX resources
*
*	INPUTS			pcClumpName			-	Filename of the clump we want to load.
*
*	RESULT			pobClumpHeader		-	A pointer to a usable CClumpHeader object.
*
*	NOTES			The clump is loaded into physically contiguous memory. The exception to this is
*					the (hopefully) temporary ClumpHeader_RuntimeDataPS3 struct.
*					This method is designed for immediate use clumps, such a
*					HUD resource, as opposed to clumps that are partially loaded untill area load.
*
***************************************************************************************************/
CClumpHeader*	CClumpLoader::ClumpLoad_Complete( const char* pcClumpName, bool bAllowMissingTex )
{
	// Check we have a filename
	ntAssert( pcClumpName );

	// Open the file
	File clumpFile;
	uint8_t *pReadResult = NULL;
	LoadFile_Chunk( pcClumpName, File::FT_READ | File::FT_BINARY, Mem::MC_GFX, clumpFile, &pReadResult );
	CClumpHeader* pobClumpHeader = (CClumpHeader*)pReadResult;

	// Check that this is a valid clump
	if( g_bAllowOldData )
	{
		one_time_assert_p( 0xDE908, (pobClumpHeader->m_uiVersionTag == OLD_CLUMP_VERSION ), ("There are clumps with the old format, check clump.log for details of files that need re-exporting") );
		if(pobClumpHeader->m_uiVersionTag == OLD_CLUMP_VERSION )
		{
			ntPrintf( Debug::DCU_CLUMP, "Slightly Old Clump %s being allowed to load but it needs re-exporting. FIX NOW!\n", pcClumpName );
		} 
		else if(pobClumpHeader->m_uiVersionTag != CLUMP_VERSION)
		{
			ntPrintf( Debug::DCU_CLUMP, "Really old Clump %s that will stop the game loading. It needs re-exporting. FIX NOW!\n", pcClumpName );
		}
	}
	else
	{
		user_error_p( (pobClumpHeader->m_uiVersionTag == CLUMP_VERSION), ( "'%s' is old format (needs re-exporting)!\n", pcClumpName) );
	}

	// Resolve header components
	ResolveOffset( pobClumpHeader->m_pCharacterBoneToIndexArray,	pobClumpHeader );
	ResolveOffset( pobClumpHeader->m_pobMeshHeaderArray,			pobClumpHeader );
	ResolveOffset( pobClumpHeader->m_pobTransformLinkageArray,		pobClumpHeader );
	ResolveOffset( pobClumpHeader->m_pobBindPoseArray,				pobClumpHeader );
	ResolveOffset( pobClumpHeader->m_pobSkinToBoneArray,			pobClumpHeader );

	// Resolve mesh headers
	for ( int iMesh = 0; iMesh < pobClumpHeader->m_iNumberOfMeshes; iMesh++ )
	{
		CMeshHeader* pobMeshHeader	= &pobClumpHeader->m_pobMeshHeaderArray[ iMesh ];
	
		ResolveOffset( pobMeshHeader->m_pucBoneIndices,			pobClumpHeader );	
		ResolveOffset( pobMeshHeader->m_pobVertexElements,		pobClumpHeader );
		ResolveOffset( pobMeshHeader->m_pvVertexBufferData,		pobClumpHeader );
		ResolveOffset( pobMeshHeader->m_pusIndices,				pobClumpHeader );
		ResolveOffset( pobMeshHeader->m_pobMaterialProperties,	pobClumpHeader );

		if( CRendererSettings::OptimiseMeshesOnLoad() )
		{
			// optimise the mesh in-place and store the buffers
			OptimiseMesh( pobMeshHeader );
		} 
		else
		{
			// construct a vertex buffer and copy in the data
			CClumpLoader::Get().AddVBHandle(	pobMeshHeader, 
												CreateStaticVertexBuffer(
												pobMeshHeader->m_pvVertexBufferData, 
												pobMeshHeader->m_iNumberOfVertices*pobMeshHeader->m_iVertexStride ) );

			// allocate an index buffer and copy in the data
			CClumpLoader::Get().AddIBHandle(	pobMeshHeader,
												CreateIndexBuffer(
												pobMeshHeader->m_pusIndices, 
												pobMeshHeader->m_iNumberOfIndices*sizeof(u_short) ) );
		}

		// resolve material properties
		for(int iProperty = 0; iProperty < pobMeshHeader->m_iNumberOfProperties; ++iProperty)
		{
			CMaterialProperty* pobProperty = &pobMeshHeader->m_pobMaterialProperties[iProperty];

			// load any textures
			if(pobProperty->m_iPropertyTag >= TEXTURE_NORMAL_MAP)
			{
				// resolve the texture name
				ResolveOffset(pobProperty->m_uData.stTextureData.pcTextureName, pobClumpHeader);

				// TODO Deano this is all fairly crappy here and requires a re-jig
				// load it and store a pointer to it
				ntAssert(pobProperty->m_uData.stTextureData.pcTextureName);
			}
		}
	}
	
	pobClumpHeader->m_pAdditionalData = NT_NEW_CHUNK( Mem::MC_GFX ) ClumpHeader_RuntimeDataPC( pcClumpName );
	pobClumpHeader->m_pAdditionalData->m_bAllocatedAsComplete = true;
	pobClumpHeader->m_pAdditionalData->m_bHasVRAMResources = true;

	// We always fix up our texture resources here
	ClumpFixup_Textures( *pobClumpHeader, bAllowMissingTex );

	// Remap linkage to atg friendly format.
	RemapLinkageArray( pobClumpHeader );

	// Return the CClumpHeader address
	return pobClumpHeader;
}

/***************************************************************************************************
*
*	FUNCTION		ClumpUnload_Complete
*
*	DESCRIPTION		Unloads a clump. 
*
*	INPUTS			pobClumpHeader		-	A pointer to the CClumpHeader we want to unload.
*
*	NOTES			This clump header must have been created via the ClumpLoad_Complete method.
*
***************************************************************************************************/
void	CClumpLoader::ClumpUnload_Complete( CClumpHeader* pobClumpHeader )
{
	ntAssert( pobClumpHeader );

	// release any GFX resources held by the meshes
	for ( int iMesh = 0; iMesh < pobClumpHeader->m_iNumberOfMeshes; iMesh++ )
	{
		CMeshHeader* pobMeshHeader	= &pobClumpHeader->m_pobMeshHeaderArray[ (uint32_t)iMesh ];

		// release buffers
		CClumpLoader::Get().RemoveVBHandle(pobMeshHeader);
		CClumpLoader::Get().RemoveIBHandle(pobMeshHeader);
	}	

	ClumpFreeup_Textures( *pobClumpHeader );

	// Delete the runtime data associated with this header
	NT_DELETE_CHUNK( Mem::MC_GFX, pobClumpHeader->m_pAdditionalData );

	// Deallocate the memory used by this clump
	NT_DELETE_ARRAY_CHUNK( Mem::MC_GFX, ( char* )pobClumpHeader );
}





/***************************************************************************************************
*
*	FUNCTION		ClumpLoad_HeaderOnly
*
*	DESCRIPTION		Same as above but with no texture fixups. purely to match PS3 funcs
*
*	INPUTS			pcClumpName			-	Filename of the clump we want to load.
*
*	RESULT			pobClumpHeader		-	A pointer to a usable CClumpHeader object.
*
***************************************************************************************************/
CClumpHeader*	CClumpLoader::ClumpLoad_HeaderOnly( const char* pcClumpName )
{
	// Check we have a filename
	ntAssert( pcClumpName );

	// Open the file
	uint8_t *pReadResult = NULL;

	File clumpFile;
	LoadFile( pcClumpName, File::FT_READ | File::FT_BINARY, clumpFile, &pReadResult );
	CClumpHeader* pobClumpHeader = (CClumpHeader*)pReadResult;

	// Check that this is a valid clump
	if( g_bAllowOldData )
	{
		one_time_assert_p( 0xDE908, (pobClumpHeader->m_uiVersionTag == OLD_CLUMP_VERSION ), ("There are clumps with the old format, check clump.log for details of files that need re-exporting") );
		if(pobClumpHeader->m_uiVersionTag == OLD_CLUMP_VERSION )
		{
			ntPrintf( Debug::DCU_CLUMP, "Slightly Old Clump %s being allowed to load but it needs re-exporting. FIX NOW!\n", pcClumpName );
		} 
		else if(pobClumpHeader->m_uiVersionTag != CLUMP_VERSION)
		{
			ntPrintf( Debug::DCU_CLUMP, "Really old Clump %s that will stop the game loading. It needs re-exporting. FIX NOW!\n", pcClumpName );
		}
	}
	else
	{
		user_error_p( (pobClumpHeader->m_uiVersionTag == CLUMP_VERSION), ( "'%s' is old format (needs re-exporting)!\n", pcClumpName) );
	}

	// Resolve header components
	ResolveOffset( pobClumpHeader->m_pCharacterBoneToIndexArray,	pobClumpHeader );
	ResolveOffset( pobClumpHeader->m_pobMeshHeaderArray,			pobClumpHeader );
	ResolveOffset( pobClumpHeader->m_pobTransformLinkageArray,		pobClumpHeader );
	ResolveOffset( pobClumpHeader->m_pobBindPoseArray,				pobClumpHeader );
	ResolveOffset( pobClumpHeader->m_pobSkinToBoneArray,			pobClumpHeader );

	// Resolve mesh headers
	for ( int iMesh = 0; iMesh < pobClumpHeader->m_iNumberOfMeshes; iMesh++ )
	{
		CMeshHeader* pobMeshHeader	= &pobClumpHeader->m_pobMeshHeaderArray[ iMesh ];
	
		ResolveOffset( pobMeshHeader->m_pucBoneIndices,			pobClumpHeader );	
		ResolveOffset( pobMeshHeader->m_pobVertexElements,		pobClumpHeader );
		ResolveOffset( pobMeshHeader->m_pvVertexBufferData,		pobClumpHeader );
		ResolveOffset( pobMeshHeader->m_pusIndices,				pobClumpHeader );
		ResolveOffset( pobMeshHeader->m_pobMaterialProperties,	pobClumpHeader );

		if( CRendererSettings::OptimiseMeshesOnLoad() )
		{
			// optimise the mesh in-place and store the buffers
			OptimiseMesh( pobMeshHeader );
		} 
		else
		{
			// construct a vertex buffer and copy in the data
			CClumpLoader::Get().AddVBHandle(	pobMeshHeader, 
												CreateStaticVertexBuffer(
												pobMeshHeader->m_pvVertexBufferData, 
												pobMeshHeader->m_iNumberOfVertices*pobMeshHeader->m_iVertexStride ) );

			// allocate an index buffer and copy in the data
			CClumpLoader::Get().AddIBHandle(	pobMeshHeader,
												CreateIndexBuffer(
												pobMeshHeader->m_pusIndices, 
												pobMeshHeader->m_iNumberOfIndices*sizeof(u_short) ) );
		}

		// resolve material properties
		for(int iProperty = 0; iProperty < pobMeshHeader->m_iNumberOfProperties; ++iProperty)
		{
			CMaterialProperty* pobProperty = &pobMeshHeader->m_pobMaterialProperties[iProperty];

			// load any textures
			if(pobProperty->m_iPropertyTag >= TEXTURE_NORMAL_MAP)
			{
				// resolve the texture name
				ResolveOffset(pobProperty->m_uData.stTextureData.pcTextureName, pobClumpHeader);

				// TODO Deano this is all fairly crappy here and requires a re-jig
				// load it and store a pointer to it
				ntAssert(pobProperty->m_uData.stTextureData.pcTextureName);
			}
		}
	}
	
	pobClumpHeader->m_pAdditionalData = NT_NEW_CHUNK( Mem::MC_GFX ) ClumpHeader_RuntimeDataPC( pcClumpName );
	pobClumpHeader->m_pAdditionalData->m_bAllocatedAsComplete = false;
	pobClumpHeader->m_pAdditionalData->m_bHasVRAMResources = false;

	// Remap linkage to atg friendly format.
	RemapLinkageArray( pobClumpHeader );

	// Return the CClumpHeader address
	return pobClumpHeader;
}

/***************************************************************************************************
*
*	FUNCTION		ClumpUnload_HeaderOnly
*
*	DESCRIPTION		Unloads a clump. 
*
*	INPUTS			pobClumpHeader		-	A pointer to the CClumpHeader we want to unload.
*
*	NOTES			This clump header must have been created via the ClumpLoad_HeaderOnly method.
*
***************************************************************************************************/
void	CClumpLoader::ClumpUnload_HeaderOnly( CClumpHeader* pobClumpHeader )
{
	ntAssert( pobClumpHeader );
	ntError_p( pobClumpHeader->m_pAdditionalData->m_bAllocatedAsComplete == false, ("This clump should not have this method called upon it") );

	// release any GFX resources held by the meshes
	for ( int iMesh = 0; iMesh < pobClumpHeader->m_iNumberOfMeshes; iMesh++ )
	{
		CMeshHeader* pobMeshHeader	= &pobClumpHeader->m_pobMeshHeaderArray[ (uint32_t)iMesh ];

		// release buffers
		CClumpLoader::Get().RemoveVBHandle(pobMeshHeader);
		CClumpLoader::Get().RemoveIBHandle(pobMeshHeader);
	}	

	if (pobClumpHeader->m_pAdditionalData->m_bHasVRAMResources)
		ClumpFreeup_MeshData( *pobClumpHeader );

	// Delete the runtime data associated with this header
	NT_DELETE_CHUNK( Mem::MC_GFX, pobClumpHeader->m_pAdditionalData );

	// Deallocate the memory used by this clump
	NT_DELETE_ARRAY_CHUNK( Mem::MC_GFX, ( char* )pobClumpHeader );
}





/***************************************************************************************************
*
*	FUNCTION		ClumpFixup_MeshData
*
*	DESCRIPTION		Takes a header only clump header, and loads in the mesh data required for 
*					rendering.
*
*	INPUTS			header			-	Existing partially loaded clump header
*					pFileData		-	Pointer to a temporarily loaded copy of the clump file
*
*	NOTES			This is a fake call to match PS3 validation
*
***************************************************************************************************/
void	CClumpLoader::ClumpFixup_MeshData( CClumpHeader& header, void* pFileData )
{
	UNUSED(pFileData);

	ntError_p( header.m_pAdditionalData->m_bAllocatedAsComplete == false, ("This clump should not have this method called upon it") );
	ntError_p( header.m_pAdditionalData->m_bHasVRAMResources == false, ("This clump already has GFX resources!") );

	// now patch up our texture resources
	ClumpFixup_Textures( header, false );

	// mark this clump as now renderable...
	header.m_pAdditionalData->m_bHasVRAMResources = true;
}

/***************************************************************************************************
*
*	FUNCTION		ClumpFreeup_MeshData
*
*	DESCRIPTION		Takes a header only clump header, and frees the data required for rendering.
*
*	INPUTS			header			-	Existing clump header, that MUST have been created via ClumpLoad_HeaderOnly
*
*	NOTES			Called upon sector unload, when clump is no longer required.
*
***************************************************************************************************/
void	CClumpLoader::ClumpFreeup_MeshData( CClumpHeader& header )
{
	ntError_p( header.m_pAdditionalData->m_bAllocatedAsComplete == false, ("This clump should not have this method called upon it") );
	ntError_p( header.m_pAdditionalData->m_bHasVRAMResources == true, ("This clump does not have GFX resources!") );

	ClumpFreeup_Textures( header );

	header.m_pAdditionalData->m_bHasVRAMResources = false;
}




/***************************************************************************************************
*
*	FUNCTION		CClumpLoader::ClumpFixup_Textures
*
*	DESCRIPTION		Fixup texture resources after those textures have been loaded into the texture
*					cache
*
***************************************************************************************************/
void	CClumpLoader::ClumpFixup_Textures( CClumpHeader& header, bool bAllowTextureLoad )
{
	UNUSED(bAllowTextureLoad);
	ntError_p( header.m_pAdditionalData->m_bHasTexResources == false, ("This clump already has Texture resources!") );
	
	for ( int iMesh = 0; iMesh < header.m_iNumberOfMeshes; iMesh++ )
	{
		CMeshHeader& meshHeader	= header.m_pobMeshHeaderArray[iMesh];
	
		for(int iProperty = 0; iProperty < meshHeader.m_iNumberOfProperties; ++iProperty)
		{
			CMaterialProperty& property = meshHeader.m_pobMaterialProperties[iProperty];

			if(property.m_iPropertyTag >= TEXTURE_NORMAL_MAP)
			{
				const char* pTexName = property.m_uData.stTextureData.pcTextureName;

				if (bAllowTextureLoad == false)
				{
					// clumps contain references to neutral path textures
					if ((g_bAllowMissingData) && (TextureManager::Get().Loaded_Neutral( pTexName ) == false))
					{
#ifdef CLUMP_USE_DEBUG_TAG
						ntPrintf( Debug::DCU_TEXTURE, "*****************************************************************\n" );
						ntPrintf( Debug::DCU_TEXTURE, "* Area %d: WARNING! missing texture %s required by clump %s.\n", 
														AreaResourceDB::Get().DebugGetLastAreaLoaded(), pTexName, header.m_pAdditionalData->m_pDebugTag);
						ntPrintf( Debug::DCU_TEXTURE, "* REGENERATE ARM!\n" );
						ntPrintf( Debug::DCU_TEXTURE, "*****************************************************************\n" );
#endif

						uint32_t areaMask = 0xffffffff;
						if (AreaManager::Get().LevelActive())
							areaMask = 1 << ( AreaResourceDB::Get().DebugGetLastAreaLoaded()-1 );
	
						uint32_t resID = AreaResourceDB::Get().AddAreaResource( pTexName, AreaResource::TEXTURE, areaMask );

						if (AreaManager::Get().LevelActive())
						{
							AreaResource* pResource = AreaResourceDB::Get().GetEntry(resID);
							pResource->Request( AreaResource::LOAD_SYNC );
						}
					}
					else
					{
						// this texture will already be in the texture cache, as the area system
						// will have primed the cache before we get here.
#						ifdef CLUMP_USE_DEBUG_TAG
							user_error_p( TextureManager::Get().Loaded_Neutral( pTexName ), 
								("Area %d: WARNING! missing texture %s required by clump %s. REGENERATE ARM!\n", 
								AreaResourceDB::Get().DebugGetLastAreaLoaded(),
								pTexName,header.m_pAdditionalData->m_pDebugTag));
#						else
							user_error_p( TextureManager::Get().Loaded_Neutral( pTexName ), 
								("Area %d: WARNING! missing texture %s required by a clump (name unknown as clump debug-tags are disabled). REGENERATE ARM!\n", 
								AreaResourceDB::Get().DebugGetLastAreaLoaded()));
#						endif
					}
				}

				Texture::Ptr pobTexture = TextureManager::Get().LoadTexture_Neutral( pTexName );
				property.m_uData.stTextureData.pobTexture = pobTexture->m_Platform.GetTexture();
				property.m_uData.stTextureData.pobTexture->AddRef();
			}
		}
	}

	header.m_pAdditionalData->m_bHasTexResources = true;
}

/***************************************************************************************************
*
*	FUNCTION		ClumpFreeup_Textures
*
*	DESCRIPTION		Release texture resources that are un-loaded on an area basis.
*					NOTE! this is not responsible for the actual unload of the data.
*
***************************************************************************************************/
void	CClumpLoader::ClumpFreeup_Textures( CClumpHeader& header )
{
	ntError_p( header.m_pAdditionalData->m_bHasTexResources == true, ("This clump has no Texture resources!") );

	for ( int iMesh = 0; iMesh < header.m_iNumberOfMeshes; iMesh++ )
	{
		CMeshHeader& meshHeader	= header.m_pobMeshHeaderArray[iMesh];
	
		for(int iProperty = 0; iProperty < meshHeader.m_iNumberOfProperties; ++iProperty)
		{
			CMaterialProperty& property = meshHeader.m_pobMaterialProperties[iProperty];

			if(property.m_iPropertyTag >= TEXTURE_NORMAL_MAP)
			{
				property.m_uData.stTextureData.pobTexture->Release();
			}
		}
	}

	header.m_pAdditionalData->m_bHasTexResources = false;
}

/***************************************************************************************************
*
*	Mesh optimisation stuff follows....
*
***************************************************************************************************/

static void DegenerateStripToTriangleList( unsigned short const* pusStrip, 
										   int iNumStripIndices, 
										   ntstd::Vector<unsigned short>& obList )
{
	// de-strip into a triangle list
	int iNumFaces = iNumStripIndices - 2;
	for( int iFace = 0; iFace < iNumFaces; ++iFace )
	{
		bool bDegenerate = ( pusStrip[iFace] == pusStrip[iFace + 1] )
			|| ( pusStrip[iFace + 1] == pusStrip[iFace + 2] )
			|| ( pusStrip[iFace + 2] == pusStrip[iFace] );

		if( !bDegenerate )
		{
			if( ( iFace & 0x1 ) == 0 )
			{
				// clockwise
				obList.push_back( pusStrip[iFace] );
				obList.push_back( pusStrip[iFace + 1] );
				obList.push_back( pusStrip[iFace + 2] );
			}
			else
			{
				// anti-clockwise
				obList.push_back( pusStrip[iFace] );
				obList.push_back( pusStrip[iFace + 2] );
				obList.push_back( pusStrip[iFace + 1] );
			}
		}
	}
}

static void OptimiseMesh( CMeshHeader* pobMeshHeader )
{
	D3DXDebugMute( TRUE );
	// compute a triangle list
	ntstd::Vector<unsigned short> obTriangleList;
	DegenerateStripToTriangleList( pobMeshHeader->m_pusIndices, pobMeshHeader->m_iNumberOfIndices, obTriangleList );

	int iFaceCount = obTriangleList.size() / 3;

#ifdef _DEBUG_OUTPUT_MESH_OPTIMISATION
	ntPrintf( "OPTIMISING MESH\n" );
	ntPrintf( "original vertex count: %d, original strip polys: %d\n", pobMeshHeader->m_iNumberOfVertices, pobMeshHeader->m_iNumberOfIndices - 2 );
	ntPrintf( "pre-optimised triangle list polys (degenerates removed): %d\n", iFaceCount );
#endif // _DEBUG_OUTPUT_MESH_OPTIMISATION

	// create a fake mesh
	HRESULT hr;
	CComPtr<ID3DXMesh> pobMesh;
	hr = D3DXCreateMeshFVF( 
		iFaceCount, 
		pobMeshHeader->m_iNumberOfVertices, 
		D3DXMESH_SYSTEMMEM, 
		D3DFVF_XYZ, 
		GetD3DDevice(), 
		pobMesh.AddressOf()
	);
	ntError_p( SUCCEEDED( hr ), ( "failed to create optimising mesh" ) );

	// lock and fill the indices
	unsigned short* pusIndexLock = 0;
	hr = pobMesh->LockIndexBuffer( 0, reinterpret_cast<void**>( &pusIndexLock ) );
	ntError_p( SUCCEEDED( hr ), ( "failed to lock optimising mesh indices" ) );
	NT_MEMCPY( pusIndexLock, &obTriangleList[0], 3*iFaceCount*sizeof( unsigned short ) );
	pobMesh->UnlockIndexBuffer();

	// lock and fill the positions
	float* pfVertexLock = 0;
	hr = pobMesh->LockVertexBuffer( 0, reinterpret_cast<void**>( &pfVertexLock ) );
	ntError_p( SUCCEEDED( hr ), ( "failed to lock optimising mesh vertices" ) );
	for( int iVertex = 0; iVertex < pobMeshHeader->m_iNumberOfVertices; ++iVertex )
	{
		float fValue = _R( iVertex );
		for( int iComponent = 0; iComponent < 3; ++iComponent )
			*pfVertexLock++ = fValue;
	}
	pobMesh->UnlockVertexBuffer();
	
	// compute the face adjacency
	CScopedArray<DWORD> pdwAdjacency( NT_NEW DWORD[3*iFaceCount] );
	hr = pobMesh->GenerateAdjacency( 0.1f, pdwAdjacency.Get() );
	ntError_p( SUCCEEDED( hr ), ( "failed to generate optimising mesh adjacency" ) );

	// optimise for vertex cache coherency
	CComPtr<ID3DXBuffer> pobVertexMap;
	hr = pobMesh->OptimizeInplace( 
		D3DXMESHOPT_VERTEXCACHE, 
		pdwAdjacency.Get(), 
		0, 
		0, 
		pobVertexMap.AddressOf()
	);
	ntError_p( SUCCEEDED( hr ), ( "failed to optimise mesh for cache efficiency" ) );

	// copy the index buffer
	/*uint32_t dwIndexedStripCount = 0;
	IBHandle pobIndexBuffer;
	hr = D3DXConvertMeshSubsetToSingleStrip( 
		pobMesh.Get(), 
		0, 
		D3DXMESH_IB_MANAGED, 
		pobIndexBuffer.AddressOf(), 
		&dwIndexedStripCount
	);
	ntError_p( SUCCEEDED( hr ), ( "failed to generate a single strip" ) );

	ntAssert_p( CClumpLoader::Get().RetrieveIBHandle(pobMeshHeader).Get() == 0, ("Already have a IB for this mesh header"));
	CClumpLoader::Get().AddIBHandle( pobMeshHeader, pobIndexBuffer );

	// and update the index count accordingly
	pobMeshHeader->m_iNumberOfIndices = dwIndexedStripCount;
	ntPrintf( "optimised single strip polys: %d\n", dwIndexedStripCount - 2 );*/

	IBHandle pobIndexBuffer = Renderer::Get().m_Platform.CreateStaticIndexBuffer( 3*iFaceCount*sizeof( unsigned short ) );
	unsigned short* pusCopyIndexLock = 0;

	hr = pobIndexBuffer->Lock( 0, 0, reinterpret_cast<void**>( &pusCopyIndexLock ), 0 );
	ntError_p( SUCCEEDED( hr ), ( "failed to lock mesh header index buffer" ) );
	hr = pobMesh->LockIndexBuffer( D3DLOCK_READONLY, reinterpret_cast<void**>( &pusIndexLock ) );
	ntError_p( SUCCEEDED( hr ), ( "failed to lock optimising mesh indices" ) );

	NT_MEMCPY( pusCopyIndexLock, pusIndexLock, 3*iFaceCount*sizeof( unsigned short ) );

	pobMesh->UnlockIndexBuffer();
	pobIndexBuffer->Unlock();

	CClumpLoader::Get().AddIBHandle( pobMeshHeader, pobIndexBuffer );

	// and update the index count accordingly
	pobMeshHeader->m_iNumberOfIndices = 3*iFaceCount;
	VBHandle pobVertexBuffer = Renderer::Get().m_Platform.CreateStaticVertexBuffer( pobMeshHeader->m_iNumberOfVertices*pobMeshHeader->m_iVertexStride );

	char* pcVertexLock = 0;
	char const* pcSourceLock = reinterpret_cast<char*>( pobMeshHeader->m_pvVertexBufferData );
	hr = pobVertexBuffer->Lock( 0, 0, reinterpret_cast<void**>( &pcVertexLock ), 0 );
	ntError_p( SUCCEEDED( hr ), ( "failed to lock vertex buffer" ) );
	uint32_t* pdwVertexMap = reinterpret_cast<uint32_t*>( pobVertexMap->GetBufferPointer() );
	int const iStride = pobMeshHeader->m_iVertexStride;
	for( int iVertex = 0; iVertex < pobMeshHeader->m_iNumberOfVertices; ++iVertex )
	{
		if ( pdwVertexMap[iVertex] != 0xffffffff ) // optimizer has removed the point completely
		{
			ntAssert( pdwVertexMap[iVertex] < uint32_t( pobMeshHeader->m_iNumberOfVertices ) );

			char const* pcSource = pcSourceLock + pdwVertexMap[iVertex]*iStride;
			char* pcDest = pcVertexLock + iVertex*iStride;

			NT_MEMCPY( pcDest, pcSource, iStride );
		}
	}
	pobVertexBuffer->Unlock();

	CClumpLoader::Get().AddVBHandle( pobMeshHeader, pobVertexBuffer );

	D3DXDebugMute( FALSE );
}
