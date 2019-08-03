/***************************************************************************************************
*
*	DESCRIPTION		A very simple resource manager to stop mulitple loads of files
*
*	NOTES			
*
***************************************************************************************************/

#ifndef _GUIRESOURCE_H
#define _GUIRESOURCE_H

/***************************************************************************************************
*
*	CLASS			CGuiResource
*
*	DESCRIPTION		A simple resource system based on file names.  This means everything gets 
*					cleared up properly and that when a proper resource system is in place it will
*					be easy to update the code.
*
*					We can assume that all the GUI stuff is happily sat in a single GUI directory.
*					Filenames will be specified relative to this.
*
***************************************************************************************************/

class CGuiResource : public Singleton< CGuiResource >
{
public:

	// Construction Destruction
	CGuiResource( void );
	~CGuiResource( void );

	// Resource getting stuff
	uint8_t*	GetResource( const char* pcFileName, int* iByteFileSize, bool bBinaryFile = false );

	// Resource releasing stuff
	bool	ReleaseResource( const char* pcFileName );
	bool	ReleaseResource( uint8_t* pData );


protected:

	// Our resource information
	struct GUI_RESOURCE
	{
		char*	pcFileName;
		uint8_t*	pData;
		int		iFileSize;
		int		iRefs;
	};

	// Things that help
	void	DropResource( GUI_RESOURCE& strResource );

	// What we keep our resource information in
	ntstd::List< GUI_RESOURCE* >	m_obResources;

};

#endif // _GUIRESOURCE_H
