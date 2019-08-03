//--------------------------------------------------
//!
//!	\file clump_ps3.cpp
//! PS3 file that loads a clump
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
#include "blendshapes/xpushapeblending.h"

extern bool g_bAllowMissingData;
// local functions
namespace
{ 

	VBHandle CreateStaticVertexBuffer( CMeshHeader* pMeshHeader, uint32_t uiStreamIndex )
	{

		static const int MAX_STREAM_SEMANTICS = 32;
		GcStreamField	aStreamElements[MAX_STREAM_SEMANTICS];

		ntAssert_p( pMeshHeader->m_pobVertexStreams[uiStreamIndex].m_iNumberOfVertexElements < MAX_STREAM_SEMANTICS, ("too many vertex elements, increase MAX_STREAM_SEMANTICS\n") );

		// pointer to the first vertex stream
		CVertexStream* pVertexStream = pMeshHeader->m_pobVertexStreams + uiStreamIndex;

		for( int i = 0;i < pVertexStream->m_iNumberOfVertexElements;i++)
		{
			uint32_t idx = i;
			aStreamElements[i] = GcStreamField( GetSemanticName(	pVertexStream->m_pobVertexElements[idx].m_eStreamSemanticTag ),
																	pVertexStream->m_pobVertexElements[idx].m_iOffset,
												GetStreamType(		pVertexStream->m_pobVertexElements[idx].m_eType ),
												GetStreamElements(	pVertexStream->m_pobVertexElements[idx].m_eType ),
												IsTypeNormalised(	pVertexStream->m_pobVertexElements[idx].m_eType ) );
		}
		
		VBHandle pobBuffer = RendererPlatform::CreateVertexStream(	pVertexStream->m_iNumberOfVertices, 
																	pVertexStream->m_iVertexStride,
																	pVertexStream->m_iNumberOfVertexElements,
																	aStreamElements,
																	Gc::kStaticBuffer );

	
        pobBuffer->Write( pVertexStream->m_pvVertexBufferData );

		// return the buffer
		return pobBuffer;
	}

IBHandle CreateIndexBuffer( CMeshHeader* pMeshHeader )
{
	// allocate a buffer
	IBHandle pobBuffer = RendererPlatform::CreateIndexStream(	Gc::kIndex16, 
																pMeshHeader->m_iNumberOfIndices,
																Gc::kStaticBuffer );

	pobBuffer->Write( pMeshHeader->m_pusIndices );

	// return the buffer
	return pobBuffer;
}

};

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

	// Load the file into memory, aligned to a 128-byte boundary.
	uint8_t *pReadResult;
	File clumpFile;
	LoadFile_ChunkMemAlign( pcClumpName, File::FT_READ | File::FT_BINARY, Mem::MC_GFX, 128, clumpFile, &pReadResult );
	size_t fileSize = clumpFile.GetFileSize();

	CClumpHeader* pobClumpHeader = (CClumpHeader*)pReadResult;

	bool remapAnimationArrays = false;
	if (pobClumpHeader->m_uiVersionTag != NEW_CLUMP_VERSION)
	{
		// Check that this is a valid clump, if not, we bail
		if (pobClumpHeader->m_uiVersionTag != CLUMP_VERSION)
		{
			return reinterpret_cast<CClumpHeader*>(BADCLUMP);
		}
		else
		{
			remapAnimationArrays = true;
			user_warn_msg(("Clump %s has not been optimized. Please run ClumpOptimizer on it asap!\n", pcClumpName));
		}

		// Resolve header components
		pobClumpHeader->m_pCharacterBoneToIndexArray.Fixup( pobClumpHeader );
		pobClumpHeader->m_pobMeshHeaderArray.Fixup( pobClumpHeader );
		pobClumpHeader->m_pobTransformLinkageArray.Fixup( pobClumpHeader );
		pobClumpHeader->m_pobBindPoseArray.Fixup( pobClumpHeader );
		pobClumpHeader->m_pobSkinToBoneArray.Fixup( pobClumpHeader );

		// Resolve mesh headers
		for ( int iMesh = 0; iMesh < pobClumpHeader->m_iNumberOfMeshes; iMesh++ )
		{
			CMeshHeader* pobMeshHeader	= &pobClumpHeader->m_pobMeshHeaderArray[ (uint32_t)iMesh ];
		
			pobMeshHeader->m_pucBoneIndices.Fixup( pobClumpHeader );	
			pobMeshHeader->m_pusIndices.Fixup( pobClumpHeader );
			pobMeshHeader->m_pobMaterialProperties.Fixup( pobClumpHeader );

			// fix all our indepedent vertex streams
			pobMeshHeader->m_pobVertexStreams.Fixup( pobClumpHeader );
			for (int iStream = 0; iStream < pobMeshHeader->m_iNumberOfVertexStreams; iStream++)
			{
				CVertexStream* pVertexStream = pobMeshHeader->m_pobVertexStreams + iStream;
				pVertexStream->m_pobVertexElements.Fixup( pobClumpHeader );
				pVertexStream->m_pvVertexBufferData.Fixup( pobClumpHeader );
			}
			
			// construct a vertex buffer and copy in the data
			bool bShareVertexStreams = !(pobMeshHeader->m_iFlags & ClumpFlags::IS_BLENDSHAPES_CAPABLE && XPUShapeBlending::Get().IsEnabled());
			if ( pobMeshHeader->m_iNumberOfVertexStreams > 0 && bShareVertexStreams )
			{
				CVertexStreamsContainter VBHandles;

				for (int iStream = 0; iStream < pobMeshHeader->m_iNumberOfVertexStreams; iStream++)
					VBHandles.SetVBHandle( CreateStaticVertexBuffer( pobMeshHeader, iStream ), iStream );

				CClumpLoader::Get().AddVBHandles( pobMeshHeader, VBHandles );		   
			}

			// allocate an index buffer and copy in the data
			CClumpLoader::Get().AddIBHandle( pobMeshHeader, CreateIndexBuffer( pobMeshHeader ) );

			// resolve material properties
			for(int iProperty = 0; iProperty < pobMeshHeader->m_iNumberOfProperties; ++iProperty)
			{
				CMaterialProperty* pobProperty = &pobMeshHeader->m_pobMaterialProperties[(uint32_t)iProperty];

				// load any textures
				if(pobProperty->m_iPropertyTag >= TEXTURE_NORMAL_MAP)
				{
					// resolve the texture name
					pobProperty->GetTextureData().pcTextureName.Fixup( pobClumpHeader );
					ntAssert(pobProperty->GetTextureData().pcTextureName);
				}
			}
		}

		pobClumpHeader->m_pAdditionalData = NT_NEW_CHUNK( Mem::MC_GFX ) ClumpHeader_RuntimeDataPS3( pcClumpName );
		pobClumpHeader->m_pAdditionalData->m_bAllocatedAsComplete = true;
		pobClumpHeader->m_pAdditionalData->m_bHasVRAMResources = true;

		// We always fix up our texture resources here
		ClumpFixup_Textures( *pobClumpHeader, bAllowMissingTex );

		if (remapAnimationArrays)
		{
			// Remap linkage to atg friendly format.
			RemapLinkageArray( pobClumpHeader );
		}
	}
	else
	{
		// create and set additional header parameters 
		pobClumpHeader->m_pAdditionalData = NT_NEW_CHUNK( Mem::MC_GFX ) ClumpHeader_RuntimeDataPS3( pcClumpName );
		pobClumpHeader->m_pAdditionalData->m_bAllocatedAsComplete = true;
		pobClumpHeader->m_pAdditionalData->m_bHasVRAMResources = false;
		pobClumpHeader->m_pAdditionalData->m_headerSize = pobClumpHeader -> m_uiHeaderPageSize;
		pobClumpHeader->m_pAdditionalData->m_resourceSize = fileSize - pobClumpHeader -> m_uiHeaderPageSize;


		ClumpFixup_GlobalSection(pobClumpHeader);
		ClumpFixup_DiscardableSection(*pobClumpHeader, pobClumpHeader);

		if (pobClumpHeader -> m_uiHeaderPageSize != 0)
		{
			NT_SHRINK_CHUNK(Mem::MC_GFX, (uintptr_t)pobClumpHeader, pobClumpHeader -> m_uiHeaderPageSize);
		}
	}

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
		bool bHasVertexStreams = !(pobMeshHeader->m_iFlags & ClumpFlags::IS_BLENDSHAPES_CAPABLE && XPUShapeBlending::Get().IsEnabled());
		if ( bHasVertexStreams )
			CClumpLoader::Get().RemoveVBHandle(pobMeshHeader);

		CClumpLoader::Get().RemoveIBHandle(pobMeshHeader);
	}	

	ClumpFreeup_Textures( *pobClumpHeader );

	// Delete the runtime data associated with this header
	NT_DELETE_CHUNK( Mem::MC_GFX, pobClumpHeader->m_pAdditionalData );

	// Deallocate the memory used by this clump
	//NT_DELETE_ARRAY_CHUNK( Mem::MC_GFX, ( char* )pobClumpHeader );
	NT_FREE_CHUNK( Mem::MC_GFX, ( uintptr_t )pobClumpHeader );
}

void  CClumpLoader::ClumpFixup_GlobalSection(CClumpHeader* pobClumpHeader)
{
	// Resolve header components
	pobClumpHeader->m_pCharacterBoneToIndexArray.Fixup( pobClumpHeader );
	pobClumpHeader->m_pobMeshHeaderArray.Fixup( pobClumpHeader );
	pobClumpHeader->m_pobTransformLinkageArray.Fixup( pobClumpHeader );
	pobClumpHeader->m_pobBindPoseArray.Fixup( pobClumpHeader );
	pobClumpHeader->m_pobSkinToBoneArray.Fixup( pobClumpHeader );

	// Resolve mesh headers
	for ( int iMesh = 0; iMesh < pobClumpHeader->m_iNumberOfMeshes; iMesh++ )
	{
		CMeshHeader* pobMeshHeader	= &pobClumpHeader->m_pobMeshHeaderArray[ (uint32_t)iMesh ];
	
		pobMeshHeader->m_pucBoneIndices.Fixup( pobClumpHeader );	
		pobMeshHeader->m_pobMaterialProperties.Fixup( pobClumpHeader );

		// fix all our indepedent vertex streams
		pobMeshHeader->m_pobVertexStreams.Fixup( pobClumpHeader );
		for (int iStream = 0; iStream < pobMeshHeader->m_iNumberOfVertexStreams; iStream++)
		{
			CVertexStream* pVertexStream = pobMeshHeader->m_pobVertexStreams + iStream;
			pVertexStream->m_pobVertexElements.Fixup( pobClumpHeader );
		}
		
		// resolve material properties
		for(int iProperty = 0; iProperty < pobMeshHeader->m_iNumberOfProperties; ++iProperty)
		{
			CMaterialProperty* pobProperty = &pobMeshHeader->m_pobMaterialProperties[(uint32_t)iProperty];

			// load any textures
			if(pobProperty->m_iPropertyTag >= TEXTURE_NORMAL_MAP)
			{
				// resolve the texture name
				pobProperty->GetTextureData().pcTextureName.Fixup( pobClumpHeader );
				ntAssert(pobProperty->GetTextureData().pcTextureName);
			}
		}
	}

}

CClumpHeader* ClumpLoad_HeaderOnly_New(File& clumpFile, const CClumpHeader& tempCH)
{

	if (tempCH.m_uiVersionTag != NEW_CLUMP_VERSION)
	{
		return reinterpret_cast<CClumpHeader*>(BADCLUMP);
	}

	size_t	sizeToLoad = tempCH.m_uiHeaderPageSize;

	char* fileData = (char*)NT_MEMALIGN_CHUNK( Mem::MC_GFX, sizeToLoad, 128 );

	clumpFile.Seek( 0, File::SEEK_FROM_START );

	size_t	sizeLoaded = clumpFile.Read(fileData, sizeToLoad);
	ntAssert_p(sizeLoaded == sizeToLoad, ("Failed to load global clump data\n"));
	UNUSED(sizeLoaded);

	CClumpHeader*	clumpHeader = (CClumpHeader*)fileData;
	CClumpLoader::ClumpFixup_GlobalSection(clumpHeader);

	return clumpHeader;
}


/***************************************************************************************************
*
*	FUNCTION		ClumpLoad_HeaderOnly
*
*	DESCRIPTION		Loads a clump and makes it available for use by calling modules. This involves
*					resolving most commonly required offsets (such as the bind pose) but leaving
*					mesh headers and mesh data unloaded.
*
*	INPUTS			pcClumpName			-	Filename of the clump we want to load.
*
*	RESULT			pobClumpHeader		-	A pointer to a usable CClumpHeader object.
*
*	NOTES			The clump is loaded into physically contiguous memory. The exception to this is
*					the (hopefully) temporary ClumpHeader_RuntimeDataPS3 struct.
*					We shall have the rest of the clump data loaded in as a result of async area
*					loading callbacks, and then the texture pointers need fixing up.
*
***************************************************************************************************/
CClumpHeader*	CClumpLoader::ClumpLoad_HeaderOnly( const char* pcClumpName )
{
	CClumpHeader* pobClumpHeader = 0;

	// Check we have a filename
	ntAssert( pcClumpName );

	// Open the file
	File clumpFile( pcClumpName, File::FT_READ | File::FT_BINARY );
	user_error_p( clumpFile.IsValid(), ("Couldn't open clump'%s'", pcClumpName ) );

	// Do a temp read into a CClumpHeader temp obj, so we can retrieve the correct
	// header size to allocate, minus the mesh headers and mesh data.
	char tempCHdata[ sizeof(CClumpHeader) ];
	size_t readSizeCH;
	readSizeCH = clumpFile.Read( &tempCHdata, sizeof( CClumpHeader ) );
	ntAssert_p( readSizeCH == sizeof( CClumpHeader ), ("Failed to extract clump header\n") );

	CClumpHeader& tempCH = *(CClumpHeader*)tempCHdata;

	// If we have no meshes, then we can use the normal ctor method
	if (tempCH.m_iNumberOfMeshes == 0)
		return ClumpLoad_Complete( pcClumpName, false );

	size_t fileSize = clumpFile.GetFileSize();

	if (tempCH.m_uiHeaderPageSize != 0)
	{
		pobClumpHeader = ClumpLoad_HeaderOnly_New(clumpFile, tempCH);

		// create and set additional header parameters 
		pobClumpHeader->m_pAdditionalData = NT_NEW_CHUNK( Mem::MC_GFX ) ClumpHeader_RuntimeDataPS3( pcClumpName );
		pobClumpHeader->m_pAdditionalData->m_bAllocatedAsComplete = false;
		pobClumpHeader->m_pAdditionalData->m_bHasVRAMResources = false;
		pobClumpHeader->m_pAdditionalData->m_headerSize = tempCH.m_uiHeaderPageSize;
		pobClumpHeader->m_pAdditionalData->m_resourceSize = fileSize - tempCH.m_uiHeaderPageSize;
	}
	else
	{
		// Check that this is a valid clump, if not, we bail
		if (tempCH.m_uiVersionTag != CLUMP_VERSION)
			return reinterpret_cast<CClumpHeader*>(BADCLUMP);

		user_warn_msg(("Clump %s has not been optimized. Please run ClumpOptimizer on it asap!\n", pcClumpName));


		// this part is fragile. what we really want is the offset in bytes from file start
		// to the begining of the mesh data. this requires finaliser changes, so for the moment
		// we piece this information together using disk pointer offsets and seeking in the file.
		size_t sizeCH = *(size_t*)(&tempCH.m_pobMeshHeaderArray);

		// seek to begining of mesh header array and read the first one in
		clumpFile.Seek( sizeCH, File::SEEK_FROM_START );

		CMeshHeader tempMH;
		size_t readSizeMH;
		readSizeMH = clumpFile.Read( &tempMH, sizeof( CMeshHeader ) );
		ntAssert_p( readSizeMH == sizeof( CMeshHeader ), ("Failed to extract mesh header\n") );

		// we 'know' the first data block after the mesh header array is the bone indices if we're 
		// skinned, or the vertex streams if we're not. like i said, this is a tad fragile...
		sizeCH = *(size_t*)(&tempMH.m_pucBoneIndices);
		if (sizeCH == 0)
			sizeCH = *(size_t*)(&tempMH.m_pobVertexStreams);

		ntAssert_p( sizeCH != 0, ("Failed to extract clump header size\n") );

		// now seek back to the beginning of the file and read the whole header in.
		clumpFile.Seek( 0, File::SEEK_FROM_START );
		char* pResult = NT_NEW_ARRAY_CHUNK( Mem::MC_GFX ) char[ sizeCH ];
		size_t readSize;
		readSize = clumpFile.Read( pResult, sizeCH );
		ntAssert_p( readSize == sizeCH, ("Failed to extract clump header\n") );

		// now partially patch up our head resources
		pobClumpHeader = (CClumpHeader*)pResult;

		// Check that this is a valid clump, if not, we bail
		if (pobClumpHeader->m_uiVersionTag != CLUMP_VERSION)
			return reinterpret_cast<CClumpHeader*>(BADCLUMP);


		// Resolve header components
		pobClumpHeader->m_pCharacterBoneToIndexArray.Fixup( pobClumpHeader );
		pobClumpHeader->m_pobTransformLinkageArray.Fixup( pobClumpHeader );
		pobClumpHeader->m_pobBindPoseArray.Fixup( pobClumpHeader );
		pobClumpHeader->m_pobSkinToBoneArray.Fixup( pobClumpHeader );
		pobClumpHeader->m_pobMeshHeaderArray.Fixup( pobClumpHeader );

		// create and set additional header parameters 
		pobClumpHeader->m_pAdditionalData = NT_NEW_CHUNK( Mem::MC_GFX ) ClumpHeader_RuntimeDataPS3( pcClumpName );
		pobClumpHeader->m_pAdditionalData->m_bAllocatedAsComplete = false;
		pobClumpHeader->m_pAdditionalData->m_bHasVRAMResources = false;
		pobClumpHeader->m_pAdditionalData->m_headerSize = sizeCH;
		pobClumpHeader->m_pAdditionalData->m_resourceSize = fileSize - sizeCH;

		// Remap linkage to atg friendly format.
		RemapLinkageArray( pobClumpHeader );
	}

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

	if (pobClumpHeader->m_pAdditionalData->m_bHasVRAMResources)
		ClumpFreeup_MeshData( *pobClumpHeader );

	// Delete the runtime data associated with this header
	NT_DELETE_CHUNK( Mem::MC_GFX, pobClumpHeader->m_pAdditionalData );

	// Deallocate the memory used by this clump
	NT_DELETE_ARRAY_CHUNK( Mem::MC_GFX, ( char* )pobClumpHeader );
}


void  CClumpLoader::ClumpFixup_DiscardableSection( CClumpHeader& header, void* pFileData )
{
	CClumpHeader*	discardableHeader = (CClumpHeader*)pFileData;

	// Resolve mesh headers
	for ( int iMesh = 0; iMesh < header.m_iNumberOfMeshes; iMesh++ )
	{
		CMeshHeader* meshHeader	= &header.m_pobMeshHeaderArray[ (uint32_t)iMesh ];

		bool bShareVertexStreams = !( meshHeader->m_iFlags & ClumpFlags::IS_BLENDSHAPES_CAPABLE && XPUShapeBlending::Get().IsEnabled() );

		// fix all our indepedent vertex streams
		for (int stream = 0; stream < meshHeader -> m_iNumberOfVertexStreams; stream++)
		{
			CVertexStream* pVertexStream = meshHeader->m_pobVertexStreams + stream;
			pVertexStream->m_pvVertexBufferData.Fixup( discardableHeader );
		}

		if (meshHeader->m_iNumberOfVertexStreams > 0 && bShareVertexStreams )
		{
			CVertexStreamsContainter VBHandles;

			for (int iStream = 0; iStream < meshHeader->m_iNumberOfVertexStreams; iStream++)
			{
				VBHandles.SetVBHandle( CreateStaticVertexBuffer( meshHeader, iStream ), iStream );
				CVertexStream* pVertexStream = meshHeader->m_pobVertexStreams + iStream;
				pVertexStream->m_pvVertexBufferData.ReverseFixup( discardableHeader );
			}

			CClumpLoader::Get().AddVBHandles( meshHeader, VBHandles );

		}

		meshHeader->m_pusIndices.Fixup( discardableHeader );

		// allocate an index buffer and copy in the data
		CClumpLoader::Get().AddIBHandle( meshHeader, CreateIndexBuffer( meshHeader ) );

		meshHeader->m_pusIndices.ReverseFixup( discardableHeader );
	}

	// now patch up our texture resources
	ClumpFixup_Textures( header, false );

	// mark this clump as now renderable...
	header.m_pAdditionalData->m_bHasVRAMResources = true;

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
***************************************************************************************************/
void	CClumpLoader::ClumpFixup_MeshData( CClumpHeader& header, void* pFileData )
{
	ntError_p( header.m_pAdditionalData->m_bAllocatedAsComplete == false, ("This clump should not have this method called upon it") );
	ntError_p( header.m_pAdditionalData->m_bHasVRAMResources == false, ("This clump already has GFX resources!") );
	ntError_p( pFileData, ("Invalid file data pointer for clump being patchted") );

	if (header.m_uiVersionTag == NEW_CLUMP_VERSION)
	{
		ClumpFixup_DiscardableSection( header, pFileData );
		return;
	}

	// NB this is 1st pass implementation, and as such keeps XDR versions of vertex
	// and index data. A better version would only load (per-mesh):
	// - m_pucBoneIndices (if skinned)
	// - m_pobVertexStreams
	// - m_pobMaterialProperties
	// and skip m_pvVertexBufferData within each CVertexStream, and m_pusIndices,
	// instead creating GFX resources explicitly from the source asyc data file

	char* pMeshData = NT_NEW_ARRAY_CHUNK( Mem::MC_GFX ) char[ header.m_pAdditionalData->m_resourceSize ];

	NT_MEMCPY( pMeshData,														// new cpu side clump data
			((char*)pFileData + header.m_pAdditionalData->m_headerSize),	// skip past header
			header.m_pAdditionalData->m_resourceSize );						// total size

	// now we run through our offset fixups again, but this time using the pMeshData
	// memory address with a mythical offset backwards so the pointer fixups work
	const void* pFakeClumpBase = pMeshData - header.m_pAdditionalData->m_headerSize;

	for ( int iMesh = 0; iMesh < header.m_iNumberOfMeshes; iMesh++ )
	{
		CMeshHeader* pobMeshHeader	= &header.m_pobMeshHeaderArray[ (uint32_t)iMesh ];
	
		pobMeshHeader->m_pucBoneIndices.Fixup( pFakeClumpBase );	
		pobMeshHeader->m_pusIndices.Fixup( pFakeClumpBase );
		pobMeshHeader->m_pobMaterialProperties.Fixup( pFakeClumpBase );

		// fix all our indepedent vertex streams
		pobMeshHeader->m_pobVertexStreams.Fixup( pFakeClumpBase );
		for (int iStream = 0; iStream < pobMeshHeader->m_iNumberOfVertexStreams; iStream++)
		{
			CVertexStream* pVertexStream = pobMeshHeader->m_pobVertexStreams + iStream;
			pVertexStream->m_pobVertexElements.Fixup( pFakeClumpBase );
			pVertexStream->m_pvVertexBufferData.Fixup( pFakeClumpBase );
		}
		
		// construct a vertex buffer and copy in the data
		bool bShareVertexStreams = !( pobMeshHeader->m_iFlags & ClumpFlags::IS_BLENDSHAPES_CAPABLE && XPUShapeBlending::Get().IsEnabled() );
		if (pobMeshHeader->m_iNumberOfVertexStreams > 0 && bShareVertexStreams )
		{
			CVertexStreamsContainter VBHandles;

			for (int iStream = 0; iStream < pobMeshHeader->m_iNumberOfVertexStreams; iStream++)
				VBHandles.SetVBHandle( CreateStaticVertexBuffer( pobMeshHeader, iStream ), iStream );

			CClumpLoader::Get().AddVBHandles( pobMeshHeader, VBHandles );
		}

		// allocate an index buffer and copy in the data
		CClumpLoader::Get().AddIBHandle( pobMeshHeader, CreateIndexBuffer( pobMeshHeader ) );

		// resolve material properties
		for(int iProperty = 0; iProperty < pobMeshHeader->m_iNumberOfProperties; ++iProperty)
		{
			CMaterialProperty* pobProperty = &pobMeshHeader->m_pobMaterialProperties[(uint32_t)iProperty];

			// load any textures
			if(pobProperty->m_iPropertyTag >= TEXTURE_NORMAL_MAP)
			{
				// resolve the texture name
				pobProperty->GetTextureData().pcTextureName.Fixup( pFakeClumpBase );
				ntAssert(pobProperty->GetTextureData().pcTextureName);
			}
		}
	}

	// now patch up our texture resources
	ClumpFixup_Textures( header, false );

	// mark this clump as now renderable...
	header.m_pAdditionalData->m_bHasVRAMResources = true;
	header.m_pAdditionalData->m_CPUMeshData = pMeshData;	// so we can free later
}

void CClumpLoader::ClumpFreeup_DiscardableSection( CClumpHeader* clumpHeader )
{
	for ( int iMesh = 0; iMesh < clumpHeader -> m_iNumberOfMeshes; iMesh++ )
	{
		CMeshHeader* pobMeshHeader	= &clumpHeader -> m_pobMeshHeaderArray[ (uint32_t)iMesh ];

		// release buffers
		bool bHasVertexStreams = !( pobMeshHeader->m_iFlags & ClumpFlags::IS_BLENDSHAPES_CAPABLE && XPUShapeBlending::Get().IsEnabled() );
		if ( bHasVertexStreams )
			CClumpLoader::Get().RemoveVBHandle(pobMeshHeader);

		CClumpLoader::Get().RemoveIBHandle(pobMeshHeader);
	}

	clumpHeader -> m_pAdditionalData->m_bHasVRAMResources = false;
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

	// release texture first
	ClumpFreeup_Textures( header );

	if (header.m_uiVersionTag == NEW_CLUMP_VERSION)
	{
		return CClumpLoader::ClumpFreeup_DiscardableSection( &header );
	}

	// we must reverse any fields that got fixed up last time...
	const void* pFakeClumpBase = header.m_pAdditionalData->m_CPUMeshData - header.m_pAdditionalData->m_headerSize;

	// release any GFX resources held by the meshes
	for ( int iMesh = 0; iMesh < header.m_iNumberOfMeshes; iMesh++ )
	{
		CMeshHeader* pobMeshHeader	= &header.m_pobMeshHeaderArray[ (uint32_t)iMesh ];

		pobMeshHeader->m_pobVertexStreams.ReverseFixup(pFakeClumpBase);
		pobMeshHeader->m_pobMaterialProperties.ReverseFixup(pFakeClumpBase);
		pobMeshHeader->m_pucBoneIndices.ReverseFixup(pFakeClumpBase);
		pobMeshHeader->m_pusIndices.ReverseFixup(pFakeClumpBase);


		// release buffers
		bool bHasVertexStreams = !( pobMeshHeader->m_iFlags & ClumpFlags::IS_BLENDSHAPES_CAPABLE && XPUShapeBlending::Get().IsEnabled() );
		if ( bHasVertexStreams )
			CClumpLoader::Get().RemoveVBHandle(pobMeshHeader);

		CClumpLoader::Get().RemoveIBHandle(pobMeshHeader);
	}

	// now delete the allocated data
	NT_DELETE_ARRAY_CHUNK( Mem::MC_GFX, header.m_pAdditionalData->m_CPUMeshData );
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
	ntError_p( header.m_pAdditionalData->m_bHasTexResources == false, ("This clump already has Texture resources!") );
	
	for ( int iMesh = 0; iMesh < header.m_iNumberOfMeshes; iMesh++ )
	{
		CMeshHeader& meshHeader	= header.m_pobMeshHeaderArray[ (uint32_t)iMesh ];
	
		for(int iProperty = 0; iProperty < meshHeader.m_iNumberOfProperties; ++iProperty)
		{
			CMaterialProperty& property = *(&meshHeader.m_pobMaterialProperties[(uint32_t)iProperty]);

			// load any textures
			if(property.m_iPropertyTag >= TEXTURE_NORMAL_MAP)
			{
				const char* pTexName = property.GetTextureData().pcTextureName;

				// need to placement new the Texture::Ptr so everything is setup o.k...
				NT_PLACEMENT_NEW ( &property.GetTextureData().pobTexture) Texture::Ptr();

				if (bAllowTextureLoad == false)
				{
					if ((g_bAllowMissingData) && (TextureManager::Get().Loaded_Neutral( pTexName ) == false))
					{
#ifdef CLUMP_USE_DEBUG_TAG
						ntPrintf( Debug::DCU_TEXTURE, "*****************************************************************\n" );
						ntPrintf( Debug::DCU_TEXTURE, "* Area %d: WARNING! missing texture %s required by clump %s.\n", 
														AreaResourceDB::Get().DebugGetLastAreaLoaded(), pTexName, header.m_pAdditionalData->m_pDebugTag);
						ntPrintf( Debug::DCU_TEXTURE, "* REGENERATE ARM!\n" );
						ntPrintf( Debug::DCU_TEXTURE, "*****************************************************************\n" );

						ntPrintf( Debug::DCU_RESOURCES, "*****************************************************************\n" );
						ntPrintf( Debug::DCU_RESOURCES, "* Area %d: WARNING! missing texture %s required by clump %s.\n", 
														AreaResourceDB::Get().DebugGetLastAreaLoaded(), pTexName, header.m_pAdditionalData->m_pDebugTag);
						ntPrintf( Debug::DCU_RESOURCES, "* REGENERATE ARM!\n" );
						ntPrintf( Debug::DCU_RESOURCES, "*****************************************************************\n" );
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
				property.GetTextureData().pobTexture = TextureManager::Get().LoadTexture_Neutral( pTexName );
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
		CMeshHeader& meshHeader	= header.m_pobMeshHeaderArray[ (uint32_t)iMesh ];

		for(int iProperty = 0; iProperty < meshHeader.m_iNumberOfProperties; ++iProperty)
		{
			CMaterialProperty& property = meshHeader.m_pobMaterialProperties[ (uint32_t)iProperty ];

			if( property.m_iPropertyTag >= TEXTURE_NORMAL_MAP )
				property.GetTextureData().pobTexture.Reset();
		}
	}

	 header.m_pAdditionalData->m_bHasTexResources = false;
}
