//--------------------------------------------------------------------------------------------------
/**
	@file
	
	@brief		FpInput : generic keyboard interface

	@warning	Temporary PC port. will be rewritten

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.
**/
//--------------------------------------------------------------------------------------------------

#ifndef FP_KEYBOARD_H
#define FP_KEYBOARD_H

//--------------------------------------------------------------------------------------------------
//	INCLUDES
//--------------------------------------------------------------------------------------------------

#include <Fp/FpInput/FpInputDevice.h>

//--------------------------------------------------------------------------------------------------
//	CLASS DEFINITION
//--------------------------------------------------------------------------------------------------

class FpKeyboard : public FpInputDevice
{
public:

	// Construction / Destruction
	FpKeyboard(void);

	// Enumerations
	enum KeyId
	{
		kKeyNull=0,

		// 1st (top) row, left side (uk layout)
		kKeyEscape,
		kKeyF1,
		kKeyF2,
		kKeyF3,
		kKeyF4,
		kKeyF5,
		kKeyF6,
		kKeyF7,
		kKeyF8,
		kKeyF9,
		kKeyF10,
		kKeyF11,
		kKeyF12,

		// 2nd row, left side (uk layout)
		kKeyGrave,	//TODO: check name
		kKey1,
		kKey2,
		kKey3,
		kKey4,
		kKey5,
		kKey6,
		kKey7,
		kKey8,
		kKey9,
		kKey0,
		kKeyMinus,
		kKeyEquals,
		kKeyBack,

		// 3rd row, left side (uk layout)
		kKeyTab,
		kKeyQ,
		kKeyW,
		kKeyE,
		kKeyR,
		kKeyT,
		kKeyY,
		kKeyU,
		kKeyI,
		kKeyO,
		kKeyP,
		kKeyLeftBracket,
		kKeyRightBracket,
		kKeyReturn,

		// 4th row, left side (uk layout)
		kKeyCapsLock,
		kKeyA,
		kKeyS,
		kKeyD,
		kKeyF,
		kKeyG,
		kKeyH,
		kKeyJ,
		kKeyK,
		kKeyL,
		kKeySemiColon,
		kKeyQuote,
		kKeyConsole,

		// 5th row, left side (uk layout)
		kKeyLeftShift,
		kKeyBackSlash,
		kKeyZ,
		kKeyX,
		kKeyC,
		kKeyV,
		kKeyB,
		kKeyN,
		kKeyM,
		kKeyComma,
		kKeyPeriod,
		kKeyForwardSlash,
		kKeyRightShift,

		// 6th (bottom) row, left side (uk layout)
		kKeyLeftControl,
		kKeyLeftWindows,
		kKeyLeftAlt,
		kKeySpace,
		kKeyRightAlt,
		kKeyRightWindows,
		kKeyRightControl,

		// Direction keys etc 
		kKeyPrintScreen,
		kKeyScrollLock,
		kKeyPause,
		kKeyHome,
		kKeyPageUp,
		kKeyEnd,
		kKeyPageDown,
		kKeyDelete,
		kKeyInsert,
		kKeyUp,
		kKeyLeft,
		kKeyDown,
		kKeyRight,

		// NumPad
		kKeyNumLock,
		kKeyNumDiv,
		kKeyNumMul,
		kKeyNumSub,
		kKeyNum7,
		kKeyNum8,
		kKeyNum9,
		kKeyNumAdd,
		kKeyNum4,
		kKeyNum5,
		kKeyNum6,
		kKeyNum1,
		kKeyNum2,
		kKeyNum3,
		kKeyNumEnter,
		kKeyNum0,
		kKeyNumDel,

		// Extended Keys
		kKeyApp,
		kKeyPower,
		kKeyNumEquals,
		kKeyF13,
		kKeyF14,
		kKeyF15,
		kKeyF16,
		kKeyF17,
		kKeyF18,
		kKeyF19,
		kKeyF20,
		kKeyF21,
		kKeyF22,
		kKeyF23,
		kKeyF24,
		kKeyExecute,
		kKeyHelp,
		kKeyMenu,
		kKeySelect,
		kKeyStop1,
		kKeyAgain,
		kKeyUndo,
		kKeyCut,
		kKeyCopy,
		kKeyPaste,
		kKeyFind,
		kKeyMute1,
		kKeyVolumeUp1,
		kKeyVolumeDown1,
		kKeyNextTrack,
		kKeyPrevTrack,
		kKeyStop2,
		kKeyPlay,
		kKeyMute2,
		kKeyVolumeUp2,
		kKeyVolumeDown2,

		kMaxNumKeys
	};

	// Operations
	bool GetKeyState(const KeyId id) const;

	// Operations: Helpers
	bool IsKeyPressed(const KeyId id) const;
	bool IsKeyHeld(const KeyId id) const;
	bool IsKeyReleased(const KeyId id) const;
	bool IsKeyChanged(const KeyId id) const;	

	// Update
	bool Update( void );

	// Connection status
	void SetConnected( int lv2DeviceId );	
	void SetDisconnected( void );
	bool IsConnected( void ) const;

	// Internal operations
	int	 GetLv2DeviceId( void ) const {return m_lv2DeviceId;}
	void Invalidate(void);

protected:

	// Operations
	inline void SetKeyState(const KeyId id, const bool state);
	void SetAsPrevState(void);

private: 
	// Constants
	enum { kNumU32InState = (((kMaxNumKeys)>>5)+((kMaxNumKeys&0x1f)?1:0)) };

	// Attributes (Key state, packed (one bit per key))
	u32 m_flagsState[kNumU32InState];		///< Current state (packed one bit per key)
	u32 m_flagsPrevState[kNumU32InState];	///< Previous state (packed one bit per key)

	int		m_lv2DeviceId;					///< Which device number we have been allocated by Lv2. -1 if not connected.


	// Static data
	static const struct KeyConvert
	{
		unsigned char				m_cellKey;				///< CELL_KEY code
		FpKeyboard::KeyId			m_keyId;				///< KeyId
	}								ms_convertTable[];
	static FpKeyboard::KeyId		ms_cellKeyToKeyId[256];	
	static const FpKeyboard::KeyId* ms_pCellKeyToKeyId;		

	// Operations
	static inline FpKeyboard::KeyId GetKeyIdFromCellKey(const unsigned char CellKey);
	static void						InitCellKeyToKeyId(void);


};

//--------------------------------------------------------------------------------------------------
//	INLINE METHODS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@brief	Sets state of a given key

	@param	id	see enum FpKeyboard::Key
	@param	state	boolean state:
					- true : Down
					- false : Up

	@note	This function is only supposed to be called by Update() of inherited classes

	@note	It is slow since it performs update one key at a time (requiring bit masking etc.)

	@internal
**/
//--------------------------------------------------------------------------------------------------

inline void FpKeyboard::SetKeyState(const KeyId id, const bool state)
{
	if (id>kKeyNull && id<kMaxNumKeys)
	{
		// packed 1 bit per key
		const int major=id>>5;
		const int minor=id&0x1f;
		if (state)	// set bit
		{
			const u32 bitMask=1<<minor;
			m_flagsState[major]|=bitMask;
		}
		else		// clear bit
		{
			const u32 bitMask=~(1<<minor);
			m_flagsState[major]&=bitMask;
		}		
	}
}


//--------------------------------------------------------------------------------------------------
/**
	@brief	Tests whether keyboard is plugged in

	@note	Should only really be used by FpInput

	@internal
**/
//--------------------------------------------------------------------------------------------------

inline bool FpKeyboard::IsConnected( void ) const
{
	// Currently only these two states are valid

	FW_ASSERT(	( GetStatus() == kStatusConnected ) ||
				( GetStatus() == kStatusDisconnected ) );
	return GetStatus() == kStatusConnected;
}

//--------------------------------------------------------------------------------------------------

#endif	//FP_KEYBOARD_H
