#ifndef _AUDIOCONSOLE_H
#define _AUDIOCONSOLE_H

class AudioConsole : public Singleton<AudioConsole>
{
public:

	AudioConsole ();

	void Update ();
	void RenderUpdate ();

protected:

	enum AUDIO_MENU
	{
		AUDIO_MENU_EVENTS,
		AUDIO_MENU_ASSETS,
		AUDIO_MENU_CATEGORIES,
		AUDIO_MENU_MIXERPROFILES,
		AUDIO_MENU_REVERBZONES,
		AUDIO_MENU_AMBIENCE,
		AUDIO_MENU_OPTIONS,
		AUDIO_MENU_CHATTERBOX,
		AUDIO_MENU_TOTAL,
	};

	void PreviousMenu ();
	void NextMenu ();
	void PreviousMenuItem ();
	void NextMenuItem ();
	
	bool					m_bEnabled;

	float					m_fX;
	float					m_fY;

	unsigned int			m_uiDebugFlags;
	int						m_iMenu;
	int						m_iMenuItem;
	int						m_iMenuItemViewable;

	float					m_fEventSystemTime;
	float					m_fEventStartTime;
	float					m_fEventSystemTimeHigh;
	float					m_fEventStartTimeHigh;
};


#endif // _AUDIOCONSOLE_H

