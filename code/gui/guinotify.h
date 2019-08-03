
#ifndef _GUINOTIFY_H_
#define _GUINOTIFY_H_

#include "core\smartptr.h"

#ifndef _RELEASE
#define _REGISTER_GUINOTIFY_KEYS
#endif

class CGuiNotify : CNonCopyable
{
public:
	static void ChapterComplete(int iChapterNumber, bool bUnlockNext);	//these params are now unused :/
	static void PlayerDeath();
};

#ifdef _REGISTER_GUINOTIFY_KEYS

#include "core\singleton.h"
#include "game\command.h"

class CGuiNotifyKeys : public Singleton<CGuiNotifyKeys>
{
public:
	COMMAND_RESULT ChapterComplete();
	COMMAND_RESULT PlayerDeath();
	void Register();
};

#endif

#endif // _GUINOTIFY_H_
