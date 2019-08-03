//------------------------------------------------------------------------------------------
//!
//!	\file hud/subtitleman.h
//!
//------------------------------------------------------------------------------------------

#ifndef SUBTITLEMAN_INC
#define SUBTITLEMAN_INC


//------------------------------------------------------------------------------------------
//!
//!	SubtitleMan
//!	Subtitle Manager
//!
//------------------------------------------------------------------------------------------
class SubtitleMan : public Singleton<SubtitleMan>
{
public:
	SubtitleMan();
	void Update(float fTimeDelta);

	void Show(const char* sSubtitle);
	void Hide()							{Show("");}

	void Queue_Show(const char* sSubtitle, float fDelay);
	void Queue_Hide(float fDelay);

// Lua Exposed
public:
	static void Lua_Show(const char* sSubtitle, float fDelay);
	static void Lua_Hide(float fDelay);

private:
	struct SubtitleItem
	{
		SubtitleItem(const char* _sSubtitle, float _fDelay) : sSubtitle(_sSubtitle), fDelay(_fDelay) {;}

		const char* sSubtitle;
		float         fDelay;
	};
	
	ntstd::List<SubtitleItem> m_itemQueue;
};

#endif //SUBTITLEMAN_INC

