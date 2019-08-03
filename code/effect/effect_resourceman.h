//--------------------------------------------------
//!
//!	\file effect_resourceman.h
//!	Manages exotic resources used by effects
//!
//--------------------------------------------------

#ifndef EFFECT_RESOURCEMAN_H
#define EFFECT_RESOURCEMAN_H

#include "gfx/fxmaterial.h"

//--------------------------------------------------
//!
//!	EffectResource
//! Object normally owned by an XML definiton,
//! caches the construction of function tables, palettes
//! et-al, and will one day be in charge of serialising
//! or WADing them up.
//!
//--------------------------------------------------
class EffectResource
{
public:
	EffectResource() : m_bRequireRefresh(false) {};

	virtual ~EffectResource() {};	
	virtual void GenerateResources() = 0;
	virtual bool ResourcesOutOfDate() const { return m_bRequireRefresh; }

	void MarkForRefresh() { m_bRequireRefresh = true; }

//	virtual void LoadFromDisk() = 0;
//	virtual void SaveToDisk() = 0;

protected:
	mutable bool m_bRequireRefresh;
};

//--------------------------------------------------
//!
//!	ScriptResource
//! Usefull lua file resource type
//!
//--------------------------------------------------
class ScriptResource : public EffectResource
{
public:
	ScriptResource();

	void SetFile( const char* );
	virtual ~ScriptResource();
	virtual void GenerateResources();
	virtual bool ResourcesOutOfDate() const;

private:
	char*	m_pScriptName;
	time_t	m_scriptModDate;
	mutable bool	m_bFileExists;
	mutable float	m_fCheckTimer;
};

//--------------------------------------------------
//!
//!	EffectResourceMan
//! Effect resource managing singleton, assumes level
//! scope for all resources
//!
//--------------------------------------------------
class EffectResourceMan : public Singleton<EffectResourceMan>
{
public:
	EffectResourceMan();
	~EffectResourceMan();

	// find an fx handle from the pooled shaders
	FXHandle* RetrieveFXHandle( const char* pcName );

	// for the moment this forces all resource object to build themselves
	// at some point this should load from disk
	void LoadResources()
	{
		for (	ntstd::List<EffectResource*, Mem::MC_EFFECTS>::iterator it = m_resources.begin();
				it != m_resources.end(); ++it )
		{
			(*it)->GenerateResources();
		}
	}

	// this thing will need building after serialsation has finished
	void RegisterResource( EffectResource& resource )
	{
		m_resources.push_back( &resource );
	}
	
	// remove from the refresh list
	void ReleaseResource( const EffectResource& resource )
	{
		for (	ntstd::List<EffectResource*, Mem::MC_EFFECTS>::iterator it = m_resources.begin();
				it != m_resources.end(); ++it )
		{
			if ((*it) == &resource)
			{
				m_resources.erase( it );
				return;
			}
		}
		ntError_p( 0, ("Releasing a non-existant effect resource") );
	}

	// Make sure resources are uptodate...handy debug stuff
	void RefreshResources();

	void ForceRecompile();

private:
	// internal FX file caching and pooling
	void AddFXFileToPool( const char* pFXFile, const char* pName, FXPoolHandle pool );

	ntstd::List<EffectResource*, Mem::MC_EFFECTS>	m_resources;
	ntstd::List<FXHandle*, Mem::MC_EFFECTS>		m_FXHandles;
	FXPoolHandle	m_effectPool;
	FXPoolHandle	m_fakeMaterialPool;

	int		m_iFirstRefreshed[2];
	float	m_fRefreshDDA[2];
	float	m_fRefreshRate[2];
	float	m_fCurrentTime;
	int		m_iRefreshCounter;
};

#endif // EFFECT_RESOURCEMAN_H
