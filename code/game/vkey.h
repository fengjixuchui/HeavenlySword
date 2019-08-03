/***************************************************************************************************
*
*	Virtual key mapping classes
*
*	CHANGES
*
*	19/02/2004	Mike	Created
*
***************************************************************************************************/

#ifndef _VKEY_H
#define _VKEY_H

#include "input/inputhardware.h"
#include "editable/enumlist.h"

// Forward declaractions
class CInputPad;
class CVKeyManager;

/***************************************************************************************************
*
*	CLASS			CVKey
*
*	DESCRIPTION		Base class for a virtual key mapping
*
***************************************************************************************************/

class CVKey
{
public:
	// Constructor
	CVKey(VIRTUAL_BUTTON_TYPE eResult)
		: m_eResult(eResult) {}

	// Destructor
	virtual ~CVKey() {}

	// Result accessor
	VIRTUAL_BUTTON_TYPE GetResult() const {return m_eResult;}

	// Is the button pressed. Must be overridden
	virtual bool IsPressed(CInputPad* pobPad, float fStick1AngleCameraRelative, float fStick2AngleCameraRelative) = 0;
	virtual bool IsHeld(CInputPad* pobPad, float fStick1AngleCameraRelative, float fStick2AngleCameraRelative) = 0;

protected:
	// The resulting VKey
	VIRTUAL_BUTTON_TYPE m_eResult;
};


/***************************************************************************************************
*
*	CLASS			CVKeyButton
*
*	DESCRIPTION		Base class for a button -> virtual key mapping
*
***************************************************************************************************/

class CVKeyButton : public CVKey
{
public:
	// Constructor
	CVKeyButton(VIRTUAL_BUTTON_TYPE eResult, PAD_BUTTON uiButtonValue)
		: CVKey(eResult), m_uiButtonValue(uiButtonValue) {}

	// Destructor
	~CVKeyButton() {}

	// Button accessor
	PAD_BUTTON GetButtonValue() const {return m_uiButtonValue;};

	// Is it pressed ?
	virtual bool IsPressed(CInputPad* pobPad, float fStick1AngleCameraRelative, float fStick2AngleCameraRelative);
	virtual bool IsHeld(CInputPad* pobPad, float fStick1AngleCameraRelative, float fStick2AngleCameraRelative);

private:
	// Stop default constructor being used
	CVKeyButton();
	// The input pad value
	PAD_BUTTON m_uiButtonValue;
};


/***************************************************************************************************
*
*	CLASS			CVKey
*
*	DESCRIPTION		Base class for a stick -> virtual key  mapping
*
***************************************************************************************************/

class CVKeyStick : public CVKey
{
public:
	// Constructor
	CVKeyStick(VIRTUAL_BUTTON_TYPE eResult, u_int uiWhichStick, float fMagnitude, float fMinAngle, float fMaxAngle)
		: CVKey(eResult), 
		m_uiWhichStick(uiWhichStick), 
		m_fMinAngle(fMinAngle), 
		m_fMaxAngle(fMaxAngle),
		m_fMagnitude(fMagnitude),
		m_bHeld(false)
	{}

	// Destructor
	~CVKeyStick() {}

	// Which stick accessor
	u_int GetWhichStick() const {return m_uiWhichStick;}

	// AngleMin accessor
	float GetMinAngle() const {return m_fMinAngle;}

	// AngleMax accessor
	float GetMaxAngle() const {return m_fMaxAngle;}

	// Is it pressed ?
	virtual bool IsPressed(CInputPad* pobPad, float fStick1AngleCameraRelative, float fStick2AngleCameraRelative);
	virtual bool IsHeld(CInputPad* pobPad, float fStick1AngleCameraRelative, float fStick2AngleCameraRelative);

private:
	// Stop default constructor being used
	CVKeyStick();

	// Which analogue stick
	u_int m_uiWhichStick;
	// Minimum angle
	float m_fMinAngle;
	// Maximum angle
	float m_fMaxAngle;
	// stick magnitude
	float m_fMagnitude;
	
	bool m_bHeld;
};


/***************************************************************************************************
*
*	CLASS			CVKeyCombiner
*
*	DESCRIPTION		Combines two virtual keys into a third virutal key. Note that this will stop the 
*					individual keys being triggered until the combination time is over.
*
***************************************************************************************************/

class CVKeyCombiner
{
public:
	// Constructor
	CVKeyCombiner(VIRTUAL_BUTTON_TYPE eResult, VIRTUAL_BUTTON_TYPE eKey1, VIRTUAL_BUTTON_TYPE eKey2, float fCombineTime) : 
		m_fCombineTime(fCombineTime),
		m_eResult(eResult), 
		m_eKey1(eKey1), 
		m_eKey2(eKey2)
	{
		Reset();
	}

	// Destructor
	~CVKeyCombiner() {}

	// Reset to known state
	void Reset() 
	{
		m_fLastSingle = 100000.0f; 
		m_eState = OPEN;
	}

	// Update the combiner
	void Update(float fTimeDelta, u_int& uiVKeysHeld);

	// Result accessor
	VIRTUAL_BUTTON_TYPE GetResult() const {return m_eResult;}

protected:
	// Block default constructor
	CVKeyCombiner();

	// The time in which keys can be combined
	float m_fCombineTime;

	float m_fLastSingle;

	// The resulting VKey
	VIRTUAL_BUTTON_TYPE m_eResult;

	// Combiner key 1
	VIRTUAL_BUTTON_TYPE m_eKey1;

	// Combiner key 2
	VIRTUAL_BUTTON_TYPE m_eKey2;

	enum COMBINER_STATE
	{
		OPEN,
		WAITING_FOR_CLOSE,
		CLOSED,
		WAITING_FOR_OPEN,
		FAILED,
	};

	// is it combined
	COMBINER_STATE m_eState;
};

/***************************************************************************************************
*
*	CLASS			CVKeyLayout
*
*	DESCRIPTION		A set of virtual key mappings and combiners. This is essentially the pad layout
*
***************************************************************************************************/

class CVKeyLayout
{
public:
	friend class CVKeyManager;

	// Constructor
	CVKeyLayout();
	// Destructor
	~CVKeyLayout();

	// Add a VKey to the layout
	void AddKey(CVKey* pobKey);

	// Add a combiner to the layout
	void AddCombiner(CVKeyCombiner* pobCombiner);

private:
	// List of VKeys
	ntstd::List<CVKey*> m_obVKeyList;

	// List of combiners
	ntstd::List<CVKeyCombiner*> m_obCombinerList;
};

/***************************************************************************************************
*
*	CLASS			CVKeyManager
*
*	DESCRIPTION		Each entity requiring pad input will need to have one of these.
*					Query this class for your game input.
*
***************************************************************************************************/

class CVKeyManager
{
public:
	// Constructor
	CVKeyManager(CInputPad* pobPad);

	// Destructor
	~CVKeyManager();

	// Reset it to known state
	void Reset();

	// Update call
	void Update(float fTimeDelta, float fStick1AngleCameraRelative, float fStick2AngleCameraRelative);

	// Get held Pressed
	u_int GetVPressed() const {return m_uiVKeysPressed;};

	// Get held keys
	u_int GetVHeld() const {return m_uiVKeysHeld;};

	// Get held keys
	u_int GetVReleased() const {return m_uiVKeysReleased;};


private:
	// Block default constructor
	CVKeyManager() {}

	// pointer to this managers VKey layout class
	CVKeyLayout* m_pobVKeyLayout;

	// pointer to the hardware pad
	CInputPad* m_pobPad;

	// Held VKeys
	u_int m_uiVKeysHeld;

	// Pressed VKeys
	u_int m_uiVKeysPressed;

	// Released VKeys
	u_int m_uiVKeysReleased;

};

#endif //_VKEY_H
