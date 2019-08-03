/***************************************************************************************************
*
*	DESCRIPTION		A very simple resource manager to stop mulitple loads of files
*
*	NOTES			
*
***************************************************************************************************/

// Includes
#include "guiresource.h"
#include "core/file.h"

/***************************************************************************************************
*
*	FUNCTION		CGuiResource::CGuiResource
*
*	DESCRIPTION		Construction
*
***************************************************************************************************/

CGuiResource::CGuiResource( void )
{
	// There is nothing to do here
}


/***************************************************************************************************
*
*	FUNCTION		CGuiResource::~CGuiResource
*
*	DESCRIPTION		Destruction
*
***************************************************************************************************/

CGuiResource::~CGuiResource( void )
{
	// If we have any resources left in our list then free them up
	for( ntstd::List< GUI_RESOURCE* >::iterator obIt = m_obResources.begin(); obIt != m_obResources.end(); ++obIt)
	{
		user_warn_p ( ( *obIt )->iRefs == 0 , ("CGuiResource %s not released\n", (*obIt)->pcFileName) );
		
		( *obIt )->iRefs = 0;
		DropResource( *( *obIt ) ); 
		NT_DELETE( ( *obIt ) );
	}

	m_obResources.clear();
}


/***************************************************************************************************
*
*	FUNCTION		CGuiResource::GetResource
*
*	DESCRIPTION		Returns a pointer to the file loaded into memory.  Returns a null pointer if it
*					can't find or load it.
*
*					If we are given a pointer to a file size then we'll hand it over.
*
***************************************************************************************************/

uint8_t* CGuiResource::GetResource( const char* pcFileName, int* piByteFileSize, bool bBinaryFile )
{
	// Check if we already have this resource in our list
	for( ntstd::List< GUI_RESOURCE* >::iterator obIt = m_obResources.begin(); obIt != m_obResources.end(); ++obIt)
	{
		// If we have something with the same filename...
		if ( strcmp( ( *obIt )->pcFileName, pcFileName ) == 0 )
		{
			// Increase the reference
			( *obIt )->iRefs += 1;

			// Return the preloaded data
			ntAssert( ( *obIt )->pData );
			return ( *obIt )->pData;
		}
	}

	// Create a new structure to hold our details in
	GUI_RESOURCE* pstrNewResource = NT_NEW GUI_RESOURCE;

	// Set up the basic details
	pstrNewResource->iRefs = 1;
	pstrNewResource->pcFileName = NT_NEW char[ MAX_PATH ];
	strcpy( pstrNewResource->pcFileName, pcFileName );

	// Build the full file name
	char acTemp[ MAX_PATH ] = "data\\";
	strcat( acTemp, pcFileName );

	char acNewResource[ MAX_PATH ] = { 0 };
	Util::GetFiosFilePath( acTemp, acNewResource );

	// Read in our resource
	uint32_t flags = File::FT_READ;
	if (bBinaryFile)
		flags |= File::FT_BINARY;

	File resource( acNewResource, flags );
	user_error_p( resource.IsValid(), ("Missing resource'%s'", acNewResource ) );
	if ( !resource.IsValid() )
	{
		// Clean up.
		NT_DELETE_ARRAY( pstrNewResource->pcFileName );
		NT_DELETE_ARRAY( pstrNewResource );

		return NULL;
	}
	
	pstrNewResource->iFileSize = resource.AllocateRead( (char**)&pstrNewResource->pData );

	// Add the details to our list
	m_obResources.push_back( pstrNewResource );

	// Let them have it ( I hold no responsibility for any users interpretation of this statement )
	if ( piByteFileSize )
		*piByteFileSize = pstrNewResource->iFileSize;

	return pstrNewResource->pData;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiResource::ReleaseResource
*
*	DESCRIPTION		Returns true if the resource is found and the reference decreased.
*
*					If the reference becomes zero then the memory is freed
*
***************************************************************************************************/

bool CGuiResource::ReleaseResource( const char* pcFileName )
{
	// Can we find a resource with this file name?
	for( ntstd::List< GUI_RESOURCE* >::iterator obIt = m_obResources.begin(); obIt != m_obResources.end(); ++obIt)
	{
		// If we have something with the same filename...
		if ( strcmp( ( *obIt )->pcFileName, pcFileName ) == 0 )
		{
			// If we can find it reduce it's reference count
			( *obIt )->iRefs -= 1;

			// If the count is now zero release the memory
			if ( ( *obIt )->iRefs < 1 )
			{
				DropResource( *( *obIt ) ); 
				NT_DELETE( ( *obIt ) );
				m_obResources.erase( obIt );
			}

			// Return true - all successful
			return true;
		}
	}

	user_warn_p( 0, ("CGuiResource Could not release %s.\n", pcFileName) );

	// If not return false
	return false;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiResource::ReleaseResource
*
*	DESCRIPTION		Returns true if the resource is found and the reference decreased.
*
*					If the reference becomes zero then the memory is freed
*
***************************************************************************************************/

bool CGuiResource::ReleaseResource( uint8_t* pData )
{
	// Can we find a resource with this file name?
	for( ntstd::List< GUI_RESOURCE* >::iterator obIt = m_obResources.begin(); obIt != m_obResources.end(); ++obIt)
	{
		// If we have something with the same data pointer
		if ( ( *obIt )->pData == pData )
		{
			// If we can find it reduce it's reference count
			( *obIt )->iRefs -= 1;

			// If the count is now zero release the memory
			if ( ( *obIt )->iRefs < 1 )
			{
				DropResource( *( *obIt ) ); 
				NT_DELETE( ( *obIt ) );
				m_obResources.erase( obIt );
			}

			// Return true - all successful
			return true;
		}
	}

	// If not return false
	return false;
}


/***************************************************************************************************
*
*	FUNCTION		CGuiResource::DropResource
*
*	DESCRIPTION		Cleans up a resource 
*
***************************************************************************************************/

void CGuiResource::DropResource( GUI_RESOURCE& strResource )
{
	// Are we sure?
	ntAssert( strResource.iRefs == 0 );

	// Release the file memory
	NT_DELETE_ARRAY( strResource.pData );

	// Release the memory for the file name
	NT_DELETE_ARRAY( strResource.pcFileName );
}
