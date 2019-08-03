/***************************************************************************************************
*
*	Anim loading
*
*	CHANGES
*
*	8/4/2004		Ben		Created
*
***************************************************************************************************/

#ifndef	_ANIMLOADER_H
#define	_ANIMLOADER_H

#include "anim/AnimationHeader.h"

typedef FpAnimClipDef CAnimationHeader;

/***************************************************************************************************
*
*	CLASS			CAnimLoader
*
*	DESCRIPTION
*
***************************************************************************************************/
class	CAnimLoader : public Singleton< CAnimLoader >
{
public:
	CAnimLoader();
	~CAnimLoader();

	void Clear();

	// This will return the anim data for the specified anim file. If the file is already loaded,
	// it'll return the exisiting pointer - the file won't be reloaded.
	const CAnimationHeader* LoadAnim_Neutral	( const char *pNeutralName );
	const CAnimationHeader* LoadAnim_Platform	( const char *pPlatformName );
	const CAnimationHeader* LoadAnim_FromData	( uint32_t cacheKey, void* pFileData, uint32_t fileSize, const char* pDebugTag );

	// these will delete from the cache, once the anim refcount equals zero
	bool UnloadAnim( const CAnimationHeader *pobAnimData );
	bool UnloadAnim_Key( uint32_t cacheKey );

	// test to see if this anim is the error anim
	bool IsErrorAnim ( const CAnimationHeader *header ) const { return (header == m_pErrorAnim); } 

	// test to see if this anim is present already
	bool Loaded_Neutral		( const char *pNeutralName )	const;
	bool Loaded_Platform	( const char *pPlatformName )	const;
	bool Loaded_Cache		( uint32_t cacheKey )			const;

	// Retrieve the platform specific name for this animation
	static void	MakePlatformAnimName( const char* pNeutralName, char* pPlatformName );

	// allow others to use our error animation as they see fit
	const CAnimationHeader* GetErrorAnimation() const { return m_pErrorAnim; }

private:
	friend struct AnimLoaderHelpers;

	struct RefCountedAnimAdaptor
	{
		RefCountedAnimAdaptor() : m_pAnim( 0 ), m_refCount( 0 ) {}
		explicit RefCountedAnimAdaptor(const CAnimationHeader *pAnim) : m_pAnim(pAnim), m_refCount(1) {}

		void AddRef() { m_refCount++; };
		void Release();

		const CAnimationHeader *m_pAnim;
		uint32_t				m_refCount;
	};

	typedef ntstd::Map< uint32_t, RefCountedAnimAdaptor >		AnimCache;
	typedef ntstd::Map< const CAnimationHeader *, uint32_t >	CacheMap;

	AnimCache	m_cache;
	CacheMap	m_cacheMap;

	const CAnimationHeader*	m_pErrorAnim;
};


#endif	//_ANIMLOADER_H
