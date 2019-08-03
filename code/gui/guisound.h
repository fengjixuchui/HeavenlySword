/***************************************************************************************************
*
*	DESCRIPTION		The main manager of the flow of the game
*
*	NOTES			Looks after front end screens and the management of levels
*
***************************************************************************************************/

#ifndef	_GUISOUND_H
#define	_GUISOUND_H

// Includes
#include "guiparse.h"

class CGuiSoundManager;

// XML element to hold details of sound
class CGuiSound : public CXMLElement
{
public:

	enum GUI_SOUND {
		ACTION_UP		=	1,
		ACTION_DOWN,
		ACTION_SELECT,
		ACTION_BACK,
		MUSIC_MAINMENU,
		MUSIC_PAUSEMENU,
		ACTION_CHANGE_CHAPTER,
		ACTION_CHANGE_CHECKPOINT,
		ACTION_CAMERA_PAN_CHAPTER,
		ACTION_CAMERA_PAN_OPTIONS,
		ACTION_CAMERA_PAN_SPECIAL_FEATURES,
		MAX_FRONTEND_SOUND
	};

	//! Construction Destruction
	CGuiSound( void );
	~CGuiSound( void );

protected:
	
	// Properties
	const char*		m_pcFileName;
	const char*		m_pcBankName;

	int m_iSoundType;

	//! To deal with parsing
	virtual bool	ProcessAttribute( const char* pcTitle, const char* pcValue );
	virtual bool	ProcessEnd( void );

	friend class CGuiSoundManager;
	friend class CGuiSoundList;
};

// Convertion for enum values to strings - null terminated for converter
const CStringUtil::STRING_FLAG astrSounds[] = 
{	
	{ CGuiSound::ACTION_UP,								"ACTION_UP"			},
	{ CGuiSound::ACTION_DOWN,							"ACTION_DOWN"		},
	{ CGuiSound::ACTION_SELECT,							"ACTION_SELECT"		},
	{ CGuiSound::ACTION_BACK,							"ACTION_BACK"		},
	{ CGuiSound::MUSIC_MAINMENU,						"MUSIC_MAINMENU"	},
	{ CGuiSound::MUSIC_PAUSEMENU,						"MUSIC_PAUSEMENU"	},
	{ CGuiSound::ACTION_CHANGE_CHAPTER,					"ACTION_CHANGECHAPTER"	},
	{ CGuiSound::ACTION_CHANGE_CHECKPOINT,				"ACTION_CHANGECHECKPOINT"	},
	{ CGuiSound::ACTION_CAMERA_PAN_CHAPTER,				"ACTION_CAMERAPANCHAPTER"	},
	{ CGuiSound::ACTION_CAMERA_PAN_OPTIONS,				"ACTION_CAMERAPANOPTIONS"	},
	{ CGuiSound::ACTION_CAMERA_PAN_SPECIAL_FEATURES,	"ACTION_CAMERAPANSPECIALFEATURES"	},
	{ 0,												0					} 
};

// XML element to hold list of sounds
class CGuiSoundList : public CXMLElement
{
public:


	//! Construction Destruction
	CGuiSoundList( void );
	~CGuiSoundList( void );

	CGuiSound*	GetSound (int iSound);

protected:
	
	// Properties

	//! Sound List
	ntstd::List< CGuiSound* >	m_obSoundList;

	//! To deal with parsing
	virtual bool	ProcessAttribute( const char* pcTitle, const char* pcValue );
	virtual bool	ProcessEnd( void );
	virtual bool	ProcessChild( CXMLElement* pobChild );

};

/***************************************************************************************************
*
*	CLASS			CGuiSoundManager
*
*	DESCRIPTION		Manage the sounds required for the GUI
*
***************************************************************************************************/

class CGuiSoundManager : public Singleton< CGuiSoundManager >
{
public:

	//! Construction Destruction
	CGuiSoundManager( void );
	~CGuiSoundManager( void );

	//! The things what this class does
	void Initialise(void);

	// Stub functions for sound support
	bool PlaySound(int iSound);
	bool StopSound(int iSound);

	bool PlayMusic(int iSound);
	bool StopMusic(int iSound);

	bool SoundFirst() {return m_bSoundFirst;};
	
protected:

	unsigned int m_aiSoundRefs[CGuiSound::MAX_FRONTEND_SOUND];

	// Do we play sound before screen and risk stutter
	// or after screen and risk a delay?
	bool m_bSoundFirst;

	//! The sounds that we are managing
	CGuiSoundList* m_pobSoundList;
};
#endif // _GUISOUND_H

