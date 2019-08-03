/***************************************************************************************************
*
*	DESCRIPTION		Contains functionality to map out the flow of a game
*
*	NOTES			The game flow is simply a description of an entire game layout.  Which user
*					screens are presented where, where levels, videos etc are to be found.
*
***************************************************************************************************/

#ifndef	_GUIFLOW_H
#define	_GUIFLOW_H

// Includes
#include "guiparse.h"
#include "gui/guilua.h"

/***************************************************************************************************
*
*	CLASS			CScreenHeader
*
*	DESCRIPTION		This defines a screen item as recognised by the game flow class - see below.  It
*					contains the name of a screen definition and any data needed for navigation to
*					that screen or away from that screen.
*
***************************************************************************************************/

class CScreenHeader : public CXMLElement
{
public:

	// Flags to modify screen behaviour
	enum
	{	
		SCREEN_NO_BACK		= ( 1 << 0 ),
		SCREEN_NO_SKIP		= ( 1 << 1 ),
		SCREEN_HUD			= ( 1 << 2 ),
		SCREEN_ATTRACT		= ( 1 << 3 ),
		SCREEN_ROOTMENU		= ( 1 << 4 ),
		SCREEN_PAUSE		= ( 1 << 5 ),
		SCREEN_COMPLETE		= ( 1 << 6 ),
		SCREEN_LOAD			= ( 1 << 7 ),
	};

	//! Construction Destruction
	CScreenHeader( void );
	~CScreenHeader( void );

	//! The useful stuff
	CScreenHeader*	GetNextScreenP( int iOption = 0 );
	CScreenHeader*	GetBackScreenP( int iScreensBack = 0 );
	CScreenHeader*	FindForwardScreenP( int iScreenFlags );
	CScreenHeader*	FindBackScreenP( int iScreenFlags );

	CScreenHeader*	FindForwardScreenP( const char* pcTag );
	CScreenHeader*	FindBackScreenP( const char* pcTag );

	//! Access
	const char*		GetDefinitionFileNameP( void )		{ return m_pcFileName; }
	const int		GetScreenFlags( void )				{ return m_iScreenFlags; }

	const char*		GetFilename()						{ return m_pcFileName; }

	// Update our header list with our xml children
	void SyncChildren();

	const char* GetTag() const;
	NinjaLua::LuaObject GetScreenStore( void );

protected:
	
	void SetTag(const char* pcTag);

	//! To deal with parsing
	virtual bool	ProcessAttribute( const char* pcTitle, const char* pcValue );
	virtual bool	ProcessChild( CXMLElement* pobChild );
	virtual bool	ProcessEnd( void );

	// Helpers for specific attributes
	bool	ProcessFileName( const char* pcValue );
	bool	ProcessSkipBack( const char* pcValue );
	bool	ProcessFlags( const char* pcValue );

	// General utility stuff
	CScreenHeader*	GetOption( int iOption );

	//! Screen Unit List
	ntstd::List< CScreenHeader* >	m_obScreenHeaders;

	// Properties
	const char*		m_pcFileName;
	int				m_iSkipback;
	int				m_iScreenFlags;

	const char*		m_pcTag;
	NinjaLua::LuaObject m_obScreenStore;
};

// Convertion for bitwise enum values to strings - null terminated for converter
const CStringUtil::STRING_FLAG astrScreenFlags[] = 
{	
	{ CScreenHeader::SCREEN_NO_BACK,	"SCREEN_NO_BACK"	},
	{ CScreenHeader::SCREEN_NO_SKIP,	"SCREEN_NO_SKIP"	},
	{ CScreenHeader::SCREEN_HUD,		"SCREEN_HUD"		},
	{ CScreenHeader::SCREEN_ATTRACT,	"SCREEN_ATTRACT"	},
	{ CScreenHeader::SCREEN_ROOTMENU,	"SCREEN_ROOTMENU"	},
	{ CScreenHeader::SCREEN_PAUSE,		"SCREEN_PAUSE"		},
	{ CScreenHeader::SCREEN_COMPLETE,	"SCREEN_COMPLETE"	},
	{ CScreenHeader::SCREEN_LOAD,		"SCREEN_LOAD"		},
	{ 0,								0					} 
};

/***************************************************************************************************
*
*	CLASS			CScreenGroup
*
*	DESCRIPTION		This defines a collection of screen definitions. These are used to help give the
*						data files a little more structure.
*
***************************************************************************************************/

class CScreenGroup : public CXMLElement
{
public:

	//! Construction Destruction
	CScreenGroup( void );
	~CScreenGroup( void );

	const char* GetGroupFilename() const	{ return m_pcFileName; }
	const char* GetTag() const				{ return m_pcTag; }

protected:

	//! To deal with parsing
	virtual bool	ProcessAttribute( const char* pcTitle, const char* pcValue );
	virtual bool	ProcessEnd( void );

	const char*		m_pcFileName;		/// File containing the group definition
	const char*		m_pcTag;			/// A tag :) a means of labelling points in the gameflow
};

#endif // _GUIFLOW_H
