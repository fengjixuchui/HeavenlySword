//--------------------------------------------------
//!
//!	\file TextureAtlas.h
//!	Class holding all functionality texture atlas related.
//!
//--------------------------------------------------

#ifndef _TEXTURE_ATLAS_H
#define _TEXTURE_ATLAS_H

#include "gfx/texture.h"

//--------------------------------------------------
//!
//!	TextureAtlasEntry
//! this class corresponds to an entry in the original atlas file
//!
//--------------------------------------------------
class TextureAtlasEntry
{
public:
	TextureAtlasEntry( const char* pFileInfo );
	~TextureAtlasEntry()
	{
		NT_DELETE_ARRAY_CHUNK( Mem::MC_GFX, m_pName );
	}

	// accessors
	const char* GetName()			const { return m_pName; }
	float		GetWidthOffset()	const { return m_fWidthOffset; }
	float		GetHeightOffset()	const { return m_fHeightOffset; }
	float		GetDepthOffset()	const { return m_fDepthOffset; }
	float		GetWidth()			const { return m_fWidth; }
	float		GetHeight()			const { return m_fHeight; }

private:
	char*	m_pName;
	float	m_fWidthOffset;
	float	m_fHeightOffset;
	float	m_fDepthOffset;
	float	m_fWidth;
	float	m_fHeight;
};

//--------------------------------------------------
//!
//!	TextureAtlas
//! Class that represents the information used to 
//! decode a texture generated from NVIDIA's
//! AtlasCreationTool.
//!
//! To make a texture atlas, create a folder within
//! content/atlas/ that contains your textures,
//! add 'atlas_directory _DIRNAME_' to buildatlas.sh,
//! then run this script in a bash shell.
//!
//! This should result in two files within 
//! content/data/ '_DIRNAME_.tai'; which is the file 
//! read directly by this class, and '_DIRNAME_0.dds',
//! the atlas texture itself.
//!
//--------------------------------------------------
class TextureAtlas
{
public:
	TextureAtlas( const char* pName, const char* pAtlasFile, const char* pTextureFile );
	~TextureAtlas();

	Texture::Ptr GetAtlasTexture() const { return m_texAtlas; }
	const char*		GetName() const			{ return m_pName; }
	
	u_int GetNumEntries() const { return m_entries.size(); }
	
	// lookup by table index
	const TextureAtlasEntry* GetEntryByIndex( u_int iIndex ) const
	{
		ntAssert( iIndex < GetNumEntries() );

		TextureAtlusEntryList::const_iterator it( m_entries.begin() );
		ntstd::advance( it, iIndex );

		return *it;
	}

	// lookup by original texture name
	const TextureAtlasEntry* GetEntryByName( const char* pTexName ) const
	{
		ntAssert( pTexName );
		CHashedString temp( pTexName );
		
		HashLookup::const_iterator it = m_hashLookup.find(temp.GetValue());
		if ( it != m_hashLookup.end() )
			return it->second;

		return NULL;
	}

private:
	typedef ntstd::Map<u_int,TextureAtlasEntry*> HashLookup;
	typedef ntstd::List<TextureAtlasEntry*, Mem::MC_GFX> TextureAtlusEntryList;

	Texture::Ptr				m_texAtlas;
	char*						m_pName;

	TextureAtlusEntryList		m_entries;
	HashLookup					m_hashLookup;
};

//--------------------------------------------------
//!
//!	TextureAtlasManager
//! Singleton created by the effect manager, handles
//! ownership of texture atlases
//!
//--------------------------------------------------
class TextureAtlasManager : public Singleton<TextureAtlasManager>
{
public:
	~TextureAtlasManager() { Reset(); }

	static bool Exists( const char* pName );

	static bool IsAtlas( const char* pName )
	{
		ntAssert(pName);
		return (strstr(pName,".tai") ? true : false);
	}

	const TextureAtlas* GetAtlas( const char* pName );

	void Reset()
	{
		while (!m_atlantes.empty())
		{
			NT_DELETE_CHUNK( Mem::MC_GFX, m_atlantes.back() );
			m_atlantes.pop_back();
		}
	}

private:
	typedef ntstd::List<const TextureAtlas*, Mem::MC_GFX> TextureAtlusList;

	const TextureAtlas* Find( const char* pName );
	TextureAtlusList m_atlantes;
};

#endif //_TEXTURE_ATLAS_H
