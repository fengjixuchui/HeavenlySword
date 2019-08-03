#ifndef	_TRIGGERVOLUME_H
#define	_TRIGGERVOLUME_H

// Necessary includes
#include "game/keywords.h"
#include "shapephantom.h"

// Forward declaractions
class CEntity;
class Transform;
class CTriggerVolume;
class CMessageHandler;
class GameEventList;


//------------------------------------------------------------------------------------------
//!
//!	CTriggerVolume
//!
//------------------------------------------------------------------------------------------
class CTriggerVolume
{
public:

	HAS_INTERFACE( CTriggerVolume );

	// Constrution destruction
	CTriggerVolume( void );
	virtual ~CTriggerVolume( void );

	// Welder stuff
	virtual void PostConstruct( void );
	virtual void DebugRender( void );
	virtual bool EditorChangeValue( CallBackParameter pcItem, CallBackParameter pcValue );

	// Set if active - ideally this should be translated to lua in the future
	void SetActive( bool bActive );

	// Reset the state of the trigger
	void Reset( void );

	// Update - check for events
	void Update( float fTimeDelta );

	// Get the name of the trigger
	const char* GetName( void ) { return m_obName.c_str(); }

//protected:

	// The serialised members
	bool				m_bActive;
	ntstd::String		m_obActivatorKeywords;
	CShapePhantom*		m_pobShapeDef;
	GameEventList*		m_pobGameEventList;
	bool				m_bDebugRender;

	bool m_bNeedsTriggerButtonRender;
private:

	// A list of the entities intersecting us at the end of the last update
	ntstd::Set<CEntity*> m_obPrevIntersecting;

	// What are we called - for debug
	ntstd::String m_obName;

	// Are we currently active?
	bool m_bIsActive; 

	// Have all our messages already been sent?
	bool m_bExpired;

	// Translate our strings into keywords so they can be checked
	CKeywords m_obKeywords;

	// This is how our events are handled
	CMessageHandler* m_pobMessageHandler;
};


//------------------------------------------------------------------------------------------
//!
//!	CTriggerVolumeManager
//!
//------------------------------------------------------------------------------------------
class CTriggerVolumeManager : public Singleton<CTriggerVolumeManager>
{
public:

	// Construction destruction
	CTriggerVolumeManager( void );
	~CTriggerVolumeManager( void );

	void Reset( void );
	void Update( float fTimeDelta );

	bool ActivateTrigger( const char* pcTriggerName );
	bool DeactivateTrigger( const char* pcTriggerName );

	bool m_bNeedsTriggerButtonRender;

	// Should our registered triggers be debug rendering
	bool m_bDebugRender;
protected:

	// Triggers can add and remove themselves
	friend class CTriggerVolume;

	// The add and remove functions
	void AddTriggerVolume( CTriggerVolume* pobTriggerVolume );
	void RemoveTriggerVolume( CTriggerVolume* pobTriggerVolume );

	// Query for the debug render state
	bool DebugRenderEnabled( void ) { return m_bDebugRender; }

	// Find a trigger by name
	CTriggerVolume* FindTriggerVolume( const char* pcTriggerName );

	// A list of our registered triggers
	ntstd::List<CTriggerVolume*> m_obTriggerVolumeList;

};






#endif // _TRIGGERVOLUME_H
