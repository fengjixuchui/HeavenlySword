#ifndef _HUDSOUND_H
#define _HUDSOUND_H

// Necessary includes

// Forward Declarations
class HudSoundManager;

class SoundDef
{
	HAS_INTERFACE( SoundDef );

public:
	SoundDef() {};
	~SoundDef() {};

	void PostConstruct( void );

private:
	CHashedString m_obName;
	ntstd::String m_obBank;
	ntstd::String m_obQue;

	friend class HudSoundManager;
};

class HudSoundManager
{
public:
	HudSoundManager() {};
	~HudSoundManager();

	void			Reset				( void );										// Clear out our list on restart/quit
	void			RegisterSoundDef	( SoundDef* pobSoundDef );						// Keep track of our sounds
	bool			PlaySound			( CHashedString obName );						// Prepare and play a sound
	void			StopSound			( CHashedString obName );						// Stop a playing sound

protected:  
	ntstd::Map< CHashedString, ntstd::pair < SoundDef*, unsigned int > > m_aobSoundList;			// Map to hold our sounds
};

typedef ntstd::Map< CHashedString, ntstd::pair < SoundDef*, unsigned int > >::iterator iSoundMapIter;
#endif // _HUDSOUND_H
