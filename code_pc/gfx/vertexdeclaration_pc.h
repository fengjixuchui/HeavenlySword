/***************************************************************************************************
*
*	DESCRIPTION
*
*	NOTES
*
***************************************************************************************************/

#ifndef GFX_VERTEXDECLARATION_PC_H
#define GFX_VERTEXDECLARATION_PC_H

/***************************************************************************************************
*
*	CLASS			CVertexDeclarationManager
*
*	DESCRIPTION		A cache of vertex declarations for use by material instances.
*
***************************************************************************************************/

class CVertexDeclaration : public CComPtr<IDirect3DVertexDeclaration9>
{
public:
	CVertexDeclaration();

	explicit CVertexDeclaration( D3DVERTEXELEMENT9 const* pstDeclArray );
};

/***************************************************************************************************
*
*	CLASS			CVertexDeclarationManager
*
*	DESCRIPTION		A cache of vertex declarations for use by material instances.
*
***************************************************************************************************/

class CVertexDeclarationManager : public Singleton<CVertexDeclarationManager>
{
public:
	//! Unloads the whole cache.
	~CVertexDeclarationManager();

	//! Gets the declaration for a given element array.
	CVertexDeclaration GetDeclaration( D3DVERTEXELEMENT9 const* pstDeclArray );

	//! bytewise compare function, till we 'know' that memcmp works on xenon.
	static inline bool DeclMatch( const D3DVERTEXELEMENT9* pstSrc, const D3DVERTEXELEMENT9* pstDest )
	{
		if	(
			(pstSrc->Stream == pstDest->Stream) &&
			(pstSrc->Offset == pstDest->Offset) &&
			(pstSrc->Type == pstDest->Type) &&
			(pstSrc->Method == pstDest->Method) &&
			(pstSrc->Usage == pstDest->Usage) &&
			(pstSrc->UsageIndex == pstDest->UsageIndex)
			)
			return true;
		return false;
	}

private:
	//! A vertex declaration manager cache entry.
	class CCacheEntry
	{
	public:
		//! Creates an entry for the given element array.
		explicit CCacheEntry( D3DVERTEXELEMENT9 const* pobDeclArray );


		//! Returns true if this declaration matches the given element array.
		bool Matches( D3DVERTEXELEMENT9 const* pobDeclArray, int iArrayCount ) const;

		//! Gets the vertex declaration.
		CVertexDeclaration const& GetDecl() const { return m_pobDecl; }

	private:
		CScopedArray<D3DVERTEXELEMENT9> m_astDeclArray;	//!< The element array.
		int m_iArraySize;								//!< The number of elements.

		CVertexDeclaration m_pobDecl;					//!< The vertex declaration.
	};


	//! Clears all loaded declarations.
    void Clear();

	ntstd::List<CCacheEntry*> m_obEntries;	//!< The cache entries.
};

#ifdef TRACK_GFX_MEM

/***************************************************************************************************
*
*	CLASS			VBHandle
*
*	DESCRIPTION		A handle round IDirect3DVertexBuffer9 like a com pointer, allows us to track memory
*
***************************************************************************************************/
class VBHandle
{
public:
	VBHandle() : m_pResource(0) {}

	explicit VBHandle(IDirect3DVertexBuffer9* pResource);

	~VBHandle() { SafeRelease(); }

	VBHandle(const VBHandle& other) : m_pResource(other.m_pResource)
	{
		if(m_pResource)
			m_pResource->AddRef();
	}

	VBHandle& operator=(const VBHandle& other)
	{
		VBHandle copy(other);
		Swap(copy);
		return *this;
	}

	IDirect3DVertexBuffer9* Get() const { return m_pResource; }
	IDirect3DVertexBuffer9* operator->() const { return m_pResource; }

	void SafeRelease();

	void Swap(VBHandle& other)
	{
		IDirect3DVertexBuffer9* pTemp = m_pResource;
		m_pResource = other.m_pResource;
		other.m_pResource = pTemp;
	}

	operator bool() const { return m_pResource != 0; }

private:
	IDirect3DVertexBuffer9* m_pResource;
};

/***************************************************************************************************
*
*	CLASS			IBHandle
*
*	DESCRIPTION		A handle round IDirect3DIndexBuffer9 like a com pointer, allows us to track memory
*
***************************************************************************************************/
class IBHandle
{
public:
	IBHandle() : m_pResource(0) {}

	explicit IBHandle(IDirect3DIndexBuffer9* pResource);

	~IBHandle() { SafeRelease(); }

	IBHandle(const IBHandle& other) : m_pResource(other.m_pResource)
	{
		if(m_pResource)
			m_pResource->AddRef();
	}

	IBHandle& operator=(const IBHandle& other)
	{
		IBHandle copy(other);
		Swap(copy);
		return *this;
	}

	IDirect3DIndexBuffer9* Get() const { return m_pResource; }
	IDirect3DIndexBuffer9* operator->() const { return m_pResource; }

	void SafeRelease();

	void Swap(IBHandle& other)
	{
		IDirect3DIndexBuffer9* pTemp = m_pResource;
		m_pResource = other.m_pResource;
		other.m_pResource = pTemp;
	}

	operator bool() const { return m_pResource != 0; }

private:
	IDirect3DIndexBuffer9* m_pResource;
};

#else // TRACK_GFX_MEM

typedef CComPtr<IDirect3DVertexBuffer9> VBHandle;
typedef CComPtr<IDirect3DIndexBuffer9>	IBHandle;

#endif // TRACK_GFX_MEM

#endif // ndef _VERTEXDECLARATION_H
