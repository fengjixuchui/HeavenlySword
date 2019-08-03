#ifndef _AUDIOREVERB_H
#define _AUDIOREVERB_H




//------------------------------------------------------------------------------------------
//!
//! AudioReverbZone
//!
//!
//------------------------------------------------------------------------------------------
class AudioReverbZone
{

public:

	AudioReverbZone ();
	virtual ~AudioReverbZone ();

	virtual bool IsInside (const CPoint& obPoint) = 0;
	virtual void DebugRender () = 0;

	const CHashedString& GetName () { return m_obName; }
	const char* GetReverbDef () { return ntStr::GetString(m_obReverbDef); }
	int GetPriority () const { return m_iPriority; }

	void SetActive (bool bActive); // This determines if the FMOD reverb is active or not
	bool IsActive () const { return m_bActive; }

	void SetEnabled (bool bEnable) { m_bEnabled=bEnable; } // This determines if the zone is active or not

	void SetDebug (bool bEnable) { m_bDebugRender=bEnable; }
	bool IsDebugEnabled () const { return m_bDebugRender; }

	// ----- Serialized members -----

	CKeyString m_obReverbDef; // String for the reverb definition

	CPoint m_obPosition; // World space position of this zone

	int m_iPriority; // Priority which is used to determine how reverb should be used when areas overlap. Higher value means higher priority

	bool m_bEnabled; // This determines if the reverb zone is active on serialisation

protected:

	CHashedString m_obName; // Name of this interface

	bool m_bActive;

	bool m_bDebugRender;
};


//------------------------------------------------------------------------------------------
//!
//! AudioReverbSphere
//!
//!
//------------------------------------------------------------------------------------------
class AudioReverbSphere : public AudioReverbZone
{
public:

	AudioReverbSphere ();

	void OnPostConstruct ();

	virtual bool IsInside (const CPoint& obPoint);
	virtual void DebugRender ();

	// ----- Serialised members -----

	float m_fRadius;	
};

//------------------------------------------------------------------------------------------
//!
//! AudioReverbBox
//!
//!
//------------------------------------------------------------------------------------------
class AudioReverbBox : public AudioReverbZone
{
public:

	AudioReverbBox ();

	void OnPostConstruct ();
	bool EditorChangeValue (CallBackParameter/*pcItem*/, CallBackParameter/*pcValue*/);

	virtual bool IsInside (const CPoint& obPoint);
	virtual void DebugRender ();

	// ----- Serialised members -----

	CDirection m_obRotation;
	
	CDirection m_obHalfExtents;

protected:

	CMatrix m_obWorldMatrix;
};


//------------------------------------------------------------------------------------------
//!
//! AudioReverbManager
//!	Class for managing reverb zones. Uses AudioSystem for activating/deactivating reverb zones.
//!
//------------------------------------------------------------------------------------------
class AudioReverbManager : public Singleton<AudioReverbManager>
{
public:

	friend class AudioConsole;
	friend class AudioReverbZone;

	AudioReverbManager ();
	~AudioReverbManager ();

	void Update ();

	void ActivateZone (const CHashedString& obName);
	void DeactivateZone (const CHashedString& obName);

	void SetEnable (bool bEnable);
	bool IsEnabled () const { return m_bEnabled; }

protected:

	void AddReverbZone (AudioReverbZone* pobReverbZone);
	void RemoveReverbZone (AudioReverbZone* pobReverbZone);

	bool m_bEnabled;

	ntstd::List<AudioReverbZone*>	m_obReverbZoneList;
};


#endif // _AUDIOREVERB_H

