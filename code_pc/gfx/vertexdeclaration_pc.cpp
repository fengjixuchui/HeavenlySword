/***************************************************************************************************
*
*	DESCRIPTION
*
*	NOTES
*
***************************************************************************************************/

#include "gfx/vertexdeclaration.h"
#include "gfx/renderer.h"
#include "gfx/graphicsdevice.h"
#include "gfx/dxerror_pc.h"

CVertexDeclaration::CVertexDeclaration() {}

CVertexDeclaration::CVertexDeclaration( D3DVERTEXELEMENT9 const* pstDeclArray )
{
	HRESULT hr;
	hr = GetD3DDevice()->CreateVertexDeclaration( pstDeclArray, AddressOf() );
	ntAssert( SUCCEEDED( hr ) );
}

/***************************************************************************************************
*
*	FUNCTION		CVertexDeclarationManager::~CVertexDeclarationManager
*
*	DESCRIPTION		Clears the declaration cache before destroying the manager.
*
***************************************************************************************************/

CVertexDeclarationManager::~CVertexDeclarationManager()
{
	// clean up
	Clear();
}

/***************************************************************************************************
*
*	FUNCTION		CVertexDeclarationManager::Clear
*
*	DESCRIPTION		Clears knowledge of cached declarations.
*
***************************************************************************************************/

void CVertexDeclarationManager::Clear()
{
	// remove all existing declarations
	while( !m_obEntries.empty() )
	{
		NT_DELETE_CHUNK(Mem::MC_GFX,  m_obEntries.back() );
		m_obEntries.pop_back();
	}
}

/***************************************************************************************************
*
*	FUNCTION		CVertexDeclarationManager::GetDeclaration
*
*	DESCRIPTION		Gets a declaration from the cache based on its element array. If a matching
*					declaration is not found one will be created.
*
***************************************************************************************************/

CVertexDeclaration CVertexDeclarationManager::GetDeclaration( D3DVERTEXELEMENT9 const* pstDeclArray )
{
	ntAssert( pstDeclArray );

	const D3DVERTEXELEMENT9* pstLocal = pstDeclArray;
	static D3DVERTEXELEMENT9 obRef = D3DDECL_END();

	int iArrayCount = 0;
	while ( 1 )
	{
		iArrayCount++;
		if (DeclMatch( pstLocal++, &obRef ))
			break;
	}

	// check existing declarations
	for( ntstd::List<CCacheEntry*>::const_iterator obIt = m_obEntries.begin(); obIt != m_obEntries.end(); ++obIt )
	{
		CCacheEntry* pobEntry = *obIt;
		if( pobEntry->Matches( pstDeclArray, iArrayCount ) )
			return pobEntry->GetDecl();
	}

	// create a new one
	m_obEntries.push_back( NT_NEW_CHUNK(Mem::MC_GFX) CCacheEntry( pstDeclArray ) );
	ntAssert( m_obEntries.back()->Matches( pstDeclArray, iArrayCount ) );
	return m_obEntries.back()->GetDecl();
}

/***************************************************************************************************
*
*	FUNCTION		CCacheEntry::CCacheEntr
*
*	DESCRIPTION		Creates a new cache entry from the given element array.
*
***************************************************************************************************/

CVertexDeclarationManager::CCacheEntry::CCacheEntry( D3DVERTEXELEMENT9 const* pstDeclArray ) : m_iArraySize(0)
{
	// get the length of the declaration
	const D3DVERTEXELEMENT9* pstLocal = pstDeclArray;
	static D3DVERTEXELEMENT9 obRef = D3DDECL_END();

	m_iArraySize = 0;
	while ( 1 )
	{
		m_iArraySize++;
		if (DeclMatch( pstLocal++, &obRef ))
			break;
	}

	ntAssert( m_iArraySize <= 17 );

	// create a local copy
	m_astDeclArray.Reset( NT_NEW_CHUNK(Mem::MC_GFX) D3DVERTEXELEMENT9[ m_iArraySize ] );
	NT_MEMCPY( m_astDeclArray.Get(), pstDeclArray, m_iArraySize*sizeof( D3DVERTEXELEMENT9 ) );

	// create the declaration
	m_pobDecl = CVertexDeclaration( pstDeclArray );
}

/***************************************************************************************************
*
*	FUNCTION		CCacheEntry::CCacheEntr
*
*	DESCRIPTION		Returns true if the declaration matches the given element array.
*
***************************************************************************************************/

bool CVertexDeclarationManager::CCacheEntry::Matches( D3DVERTEXELEMENT9 const* pstDeclArray, int iArrayCount ) const
{
	if ( iArrayCount == m_iArraySize )
	{
		const D3DVERTEXELEMENT9* pobSrc = pstDeclArray;
		const D3DVERTEXELEMENT9* pobDest = m_astDeclArray.Get();
		int iLocal = m_iArraySize;

		while ( iLocal-- )
		{
			if (!DeclMatch( pobSrc++, pobDest++ ))
				return false;
		}

		return true;		
	}
	else
		return false;
}

#ifdef TRACK_GFX_MEM

/***************************************************************************************************
*
*	FUNCTION		VBHandle::ctor
*
*	DESCRIPTION		queries the size of the object
*
***************************************************************************************************/
VBHandle::VBHandle(IDirect3DVertexBuffer9* pResource) :
	m_pResource(pResource)
{
	if (m_pResource)
	{
		D3DVERTEXBUFFER_DESC desc;
		dxerror( m_pResource->GetDesc(&desc) );

		if (desc.Pool != D3DPOOL_SYSTEMMEM)
		{
			TRACK_GFX_ALLOC_VB(desc.Size);
		}
	}
}

/***************************************************************************************************
*
*	FUNCTION		VBHandle::SafeRelease
*
*	DESCRIPTION		release the buffer
*
***************************************************************************************************/
void VBHandle::SafeRelease()
{
	if(m_pResource)
	{
		uint32_t iRefCount = m_pResource->AddRef();
		m_pResource->Release();

		ntAssert_p( iRefCount > 1, ("Ref counting needs to be changed. Tell Wil!") );
		if (iRefCount == 2)
		{
			D3DVERTEXBUFFER_DESC desc;
			dxerror( m_pResource->GetDesc(&desc) );

			if (desc.Pool != D3DPOOL_SYSTEMMEM)
			{
				TRACK_GFX_FREE_VB( desc.Size );
			}
		}
		m_pResource->Release();
		m_pResource = 0;
	}
}


/***************************************************************************************************
*
*	FUNCTION		IBHandle::ctor
*
*	DESCRIPTION		queries the size of the object
*
***************************************************************************************************/
IBHandle::IBHandle(IDirect3DIndexBuffer9* pResource) :
	m_pResource(pResource)
{
	if (m_pResource)
	{
		D3DINDEXBUFFER_DESC desc;
		dxerror( m_pResource->GetDesc(&desc) );

		if (desc.Pool != D3DPOOL_SYSTEMMEM)
		{
			TRACK_GFX_ALLOC_IB( desc.Size );
		}
	}
}

/***************************************************************************************************
*
*	FUNCTION		IBHandle::SafeRelease
*
*	DESCRIPTION		release the buffer
*
***************************************************************************************************/
void IBHandle::SafeRelease()
{
	if(m_pResource)
	{
		uint32_t iRefCount = m_pResource->AddRef();
		m_pResource->Release();

		ntAssert_p( iRefCount > 1, ("Ref counting needs to be changed. Tell Wil!") );
		if (iRefCount == 2)
		{
			D3DINDEXBUFFER_DESC desc;
			dxerror( m_pResource->GetDesc(&desc) );

			if (desc.Pool != D3DPOOL_SYSTEMMEM)
			{
				TRACK_GFX_FREE_IB( desc.Size );
			}
		}
		m_pResource->Release();
		m_pResource = 0;
	}
}

#endif // TRACK_GFX_MEM
