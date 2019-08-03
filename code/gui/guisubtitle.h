/**
	@file subtitle.h

	@author campf

    Subtitle classes	
*/

#ifndef _SUBTITLE_H
#define _SUBTITLE_H

#include <list>
#include <string>
#include "gui/guitext.h"

/**
*/
class CSubtitleBlock
{
	public:

	CSubtitleBlock() : m_fDelay(0.0f)
	{
		m_strBlock = NT_NEW std::wstring;
	}
	~CSubtitleBlock()
	{
		NT_DELETE(m_strBlock);
	}

	std::wstring *m_strBlock;
	float m_fDelay;
};

/**
	@class CSubtitle

	A class representing a block of subtitle captions.
*/
class CSubtitle
{
	public:

	/// This is the number of allowed characters displayed on screen in one subtitle block.
	enum
	{
		NUM_ALLOWED_CHARACTERS = 100
	};

	/// Array of strings representing chunks of subtitles.
	std::vector<CSubtitleBlock *> m_strBlockList;

	/// Currently displaying subtitle.
	u_int m_uiCurrentlyRenderingBlock;

	float m_fTime;
	CStringDefinition m_obDefinition;
	Transform *m_pobTransform;
	CString *m_pobCurrentBlockString;
	bool m_bPause : 1;
	bool m_bStopped : 1;

	CSubtitle(const char *strID, const char *strFontName);
	~CSubtitle();

	void ProcessString(const WCHAR_T *strSubtitleString);
	void Update(const float fDt);
	void Render(void);
	void MakeBlockString(const WCHAR_T *strString);
	void Pause(const bool bPause)
	{
		m_bPause = bPause;
	}
	void Stop(const bool bStop)
	{
		m_bStopped = bStop;
	}

};

/**
	Thin mangager singleton for subtitle system.
*/
class CSubtitleMan : public Singleton<CSubtitleMan>
{
	public:

	CSubtitleMan(	const char *strFont) : 
					m_strFont(strFont),
					m_pobSubtitle(NULL),
					m_bEnable(false)
	{

	}
	~CSubtitleMan()
	{
		Stop();
	}

	/**
		@bEnable Enable/disable the subtitle system from displaying anything.
	*/
	void Enable(const bool bEnable)
	{
		m_bEnable = bEnable;
		if (!bEnable) Stop();
	}

	void Play(const char *strID);
	void Stop(void);
	void Update(const float fDt)
	{
		if (m_pobSubtitle)
		{
			m_pobSubtitle->Update(fDt);

			if (m_pobSubtitle->m_bStopped)
			{
				Stop();
			}
		}
	}
	void Render(void)
	{
		if (m_pobSubtitle)
		{
			m_pobSubtitle->Render();
		}
	}

	const char *m_strFont;
    CSubtitle *m_pobSubtitle;
	bool m_bEnable;
};

#endif //_SUBTITLE_H
