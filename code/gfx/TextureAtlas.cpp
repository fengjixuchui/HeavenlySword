//--------------------------------------------------
//!
//!	\file TextureAtlas.cpp
//!	Class holding all functionality texture atlas related.
//!
//--------------------------------------------------

#include "TextureAtlas.h"
#include "TextureManager.h"
#include "core/file.h"

//--------------------------------------------------
//!
//!	TextureAtlasEntry::ctor
//! pFileInfo- copy of the entry in the original atlas
//!
//--------------------------------------------------
TextureAtlasEntry::TextureAtlasEntry( const char* pFileInfo )
{
	ntAssert( pFileInfo );
	char aTexName[128];
	char aAtlasName[128];	// unused
	int iAtlasIndex;		// unused
	char aAtlasType[8];		// unused

	// <filename>		<atlas filename>, <atlas idx>, <atlas type>, <woffset>, <hoffset>, <depth offset>, <width>, <height>
	int iResults = 0;
	iResults = sscanf( pFileInfo, "%127s\t\t%127s %d, %8s %f, %f, %f, %f, %f",
					aTexName, aAtlasName, &iAtlasIndex, aAtlasType, &m_fWidthOffset,
					&m_fHeightOffset, &m_fDepthOffset, &m_fWidth, &m_fHeight );

	ntAssert( iResults == 9 );

	m_pName = NT_NEW_ARRAY_CHUNK(Mem::MC_GFX) char [strlen(aTexName)+1];
	strcpy( m_pName, aTexName );
}





//--------------------------------------------------
//!
//!	TextureAtlas::ctor
//!
//--------------------------------------------------
TextureAtlas::TextureAtlas( const char* pName, const char* pAtlasFile, const char* pTextureFile )
{
	ntAssert(pName);
	ntAssert(pAtlasFile);

	m_pName = NT_NEW_ARRAY_CHUNK( Mem::MC_GFX ) char [strlen(pName)+1];
	strcpy( m_pName, pName );
	m_texAtlas = TextureManager::Get().LoadTexture_Neutral( pTextureFile );

	ntAssert( m_texAtlas );

	File atlas( pAtlasFile, File::FT_READ | File::FT_BINARY );
	ntError_p( atlas.IsValid(), ("Atlas %s is not a valid file\n", pAtlasFile));

	// mmm temp memory pool currenty use MISC
	char* memory;
	uint32_t iFileSize = atlas.AllocateRead( &memory );

	uint32_t iCurrPos = 0;
	while (iCurrPos < iFileSize)
	{
		char* pCurr = memory + iCurrPos;
		char* pNext = strstr( pCurr, "\n" );
		
		uint32_t iLineLen = pNext ? (pNext - pCurr) : (iFileSize - iCurrPos);
		ntError_p( iLineLen < 512, ("Atlas line to big"));

		if( pCurr[0] != '\n' && pCurr[0] != '#' )
			m_entries.push_back( NT_NEW_CHUNK( Mem::MC_GFX ) TextureAtlasEntry( pCurr ) );

		iCurrPos += iLineLen+1;
	}

	NT_DELETE_ARRAY_CHUNK( Mem::MC_MISC, memory );

	ntError_p( !m_entries.empty(), ("Texture entries not found in atlas") );

	// generate hash map
	for (	TextureAtlusEntryList::iterator it =  m_entries.begin();
			it != m_entries.end(); ++it )
	{
		CHashedString temp( (*it)->GetName() );
		m_hashLookup[temp.GetValue()] = *it;
	}
}

//--------------------------------------------------
//!
//!	TextureAtlas::dtor
//!
//--------------------------------------------------
TextureAtlas::~TextureAtlas()
{
	NT_DELETE_ARRAY_CHUNK( Mem::MC_GFX, m_pName );
	while (!m_entries.empty())
	{
		NT_DELETE_CHUNK( Mem::MC_GFX, m_entries.back() );
		m_entries.pop_back();
	}
}





//--------------------------------------------------
//!
//!	TextureAtlasManager::Find
//! Find it if it exists.
//!
//--------------------------------------------------
const TextureAtlas* TextureAtlasManager::Find( const char* pName )
{
	for (	TextureAtlusList::iterator it = m_atlantes.begin();
			it != m_atlantes.end(); ++it )
	{
		// already there, return it.
		if (strcmp(pName,(*it)->GetName())==0)
			return *it;
	}

	return NULL;
}

//--------------------------------------------------
//!
//!	TextureAtlasManager::Exists
//! Does this atlas exist
//!
//--------------------------------------------------
bool TextureAtlasManager::Exists( const char* pName )
{
	// is the atlas present?
	//-----------------------------------
	static char pFullName[MAX_PATH];
	static char pDDSName[MAX_PATH];
	static char pTestName[ MAX_PATH ];

	sprintf( pTestName, "%s%s", TEXTURE_ROOT_PATH, pName );
	Util::GetFiosFilePath( pTestName, pFullName );	
//	strcat( pFullName, pName );

	if( !File::Exists( pFullName ) )
		return false;

	// is the texture present?
	//-----------------------------------
	Util::SetToPlatformResources();
	Util::GetFiosFilePath( pTestName, pDDSName );	
//	strcat( pDDSName, pName );

	strcpy( strstr( pDDSName, "." ), "0.dds" );
//	ntstd::transform( pDDSName, pDDSName + strlen( pDDSName ), pDDSName, &ntstd::Tolower );

	if( !File::Exists( pDDSName ) )
	{
		Util::SetToNeutralResources();
		return false;
	}

	Util::SetToNeutralResources();
	return true;
}

//--------------------------------------------------
//!
//!	TextureAtlasManager::GetAtlas
//! Retrieve or load this atlas
//!
//--------------------------------------------------
const TextureAtlas* TextureAtlasManager::GetAtlas( const char* pName )
{
	ntAssert(pName);
	ntAssert(IsAtlas(pName));

	// check cache
	const TextureAtlas* pResult = Find( pName );
	if (pResult)
		return pResult;

	// is the atlas present?
	//-----------------------------------
	static char pFullName[MAX_PATH];
	static char pDDSName[MAX_PATH];
	static char pTestName[ MAX_PATH ];

	sprintf( pTestName, "%s%s", TEXTURE_ROOT_PATH, pName );
	Util::GetFiosFilePath( pTestName, pFullName );	
//	strcat( pFullName, pName );

	if( !File::Exists( pFullName ) )
	{
		ntAssert_p(0,("Atlas %s does not exist.\n", pFullName));
		return 0;
	}

	// is the texture present?
	//-----------------------------------
	Util::SetToPlatformResources();
	Util::GetFiosFilePath( pTestName, pDDSName );	
//	strcat( pDDSName, pName );

	strcpy( strstr( pDDSName, "." ), "0.dds" );
//	ntstd::transform( pDDSName, pDDSName + strlen( pDDSName ), pDDSName, &ntstd::Tolower );

	if( !File::Exists( pDDSName ) )
	{
		Util::SetToNeutralResources();
		ntAssert_p(0,("Texture %s does not exist.\n", pDDSName));
		return 0;
	}
	Util::SetToNeutralResources();

	// all good, create the atlas
	//-------------------------------------
	strcpy( pDDSName, pName );
	strcpy( strstr( pDDSName, "." ), "0.dds" );

	pResult = NT_NEW_CHUNK( Mem::MC_GFX ) TextureAtlas( pName, pFullName, pDDSName );
	ntAssert( pResult );

	m_atlantes.push_back( pResult );
	return pResult;
}
