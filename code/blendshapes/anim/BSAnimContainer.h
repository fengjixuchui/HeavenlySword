//--------------------------------------------------
//!
//!	\file BSAnimContainer.h
//!	Blendshape animation shortcut container
//!
//--------------------------------------------------


#ifndef _BSANIMCONTAINER_H_
#define _BSANIMCONTAINER_H_

#include "blendshapes/blendshapes_managers.h"

class DataObject;
class CEntity;
class BSAnimator;

//--------------------------------------------------
//!
//!	Blendshape animation shortcut
//!	Lets us associate a short, generic, name with 
//! an bsanim header. Also, in the spirit of normal
//! anim shortcuts, it holds the hash of its parent
//! container so it can be removed accordingly
//!
//--------------------------------------------------
class BSAnimShortcut
{
public:
	BSAnimShortcut( const CHashedString& shortName, const char* pFileName, const char* containerName, bool bWithinNinjaSeq );
	~BSAnimShortcut();

	void InstallAnim( const char* pDebugTag );
	void UninstallAnim();
	bool AnimInstalled() const { return (m_pobBSAnimHeader != 0); }

	const CHashedString&	GetShortNameHash() const { return m_obShortName; }
	BSAnimHeaderPtr_t		GetHeader() const
	{
		ntError_p( m_pobBSAnimHeader != 0, ("Animation Header has not been installed") );
		return m_pobBSAnimHeader;
	}

	const CHashedString&	GetContainerHash() const { return m_obContainerName; }

private:
	CHashedString			m_obShortName;
	BSAnimHeaderPtr_t		m_pobBSAnimHeader;
	CHashedString			m_obContainerName;

	// our lookup into the animation header cache, generated on construction
	uint32_t		m_animCacheKey;

	// debug functionality to allow missing animations
	ntstd::String	m_debugLoadName;
};
	
class BSAnimShortcutContainer 
{
public:
	BSAnimShortcutContainer();
	virtual ~BSAnimShortcutContainer();
	
	void				AddBSAnimsFromContainer( const CHashedString& containerName, bool bWithinNinjaSeq  );
	void				AddBSAnimsFromContainer( DataObject* pDO, bool bWithinNinjaSeq );
	void				RemoveBSAnimsFromContainer( CHashedString containerName );

	void				RemoveBSAnim( CHashedString shortName );
	void				RemoveAllBSAnims( void );

	BSAnimHeaderPtr_t	GetBSAnimHeader( CHashedString shortName ) const;
	CKeyString			GetShortName( BSAnimHeaderPtr_t pHeader ) const; NOT_IMPLEMENTED

	bool				HasBSAnim( CHashedString shortName ) const;
	bool				IsBSAnimContainerAdded( CHashedString containerName ) const;

	// called by area system to add or remove BS animations
	void				InstallAnimations( const CHashedString& containerName );
	void				UninstallAnimations( const CHashedString& containerName );

protected:
	typedef ntstd::Map<CHashedString,BSAnimShortcut*,ntstd::less<CHashedString>, Mem::MC_PROCEDURAL > BSAnimShortcutCollection;

	virtual BSAnimShortcutCollection::iterator 	RemoveBSAnim( BSAnimShortcutCollection::iterator it );

protected:
	//! contained animation shortcuts
	BSAnimShortcutCollection m_obBSAnims;

private:
	void				AddBSAnim( CHashedString shortName, const char* fileName, const char* containerName, bool bWithinNinjaSeq );
};

//------------------------------------------------------------------------------------------
//!
//! BSAnimContainerManager
//! Singleton that maintains animation containers
//! Has global rather than level scope
//!
//------------------------------------------------------------------------------------------
class BSAnimContainerManager : public Singleton<BSAnimContainerManager>
{
public:
	void RegisterContainer( BSAnimShortcutContainer* pCont );
	void UnRegisterContainer( BSAnimShortcutContainer* pCont );

	void InstallAnimsForContainer( const CHashedString& containerName );
	void RemoveAnimsForContainer( const CHashedString& containerName );

private:
	typedef ntstd::List<BSAnimShortcutContainer*, Mem::MC_PROCEDURAL> ContList;
		
	ContList	m_registerConts;
};

#endif // end of _BSANIMCONTAINER_H_
