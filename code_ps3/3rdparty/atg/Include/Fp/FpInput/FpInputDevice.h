//--------------------------------------------------------------------------------------------------
/**
	@file
	
	@brief		FpInput : base class shared by all input devices.

	@warning	Temporary PC port. will be rewritten

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.
**/
//--------------------------------------------------------------------------------------------------

#ifndef FP_INPUT_DEVICE_H
#define FP_INPUT_DEVICE_H

//--------------------------------------------------------------------------------------------------
//	CLASS DEFINITION
//--------------------------------------------------------------------------------------------------

class FpInputDevice : FwNonCopyable
{
public:                               
	// Enumerations
	enum Status
	{
		kStatusNone=0,			///< No peripheral.
		kStatusInitializing,	///< Peripheral is initializing
		kStatusDisconnected,	///< Peripheral is disconnected
		kStatusConnecting,		///< Peripheral is connecting
		kStatusJustConnected,	///< Connected (special case for first frame)
		kStatusConnected,		///< Connected and working
		kStatusIntercepted		///< Connected but intercepted by LV2
	};

	// Construction & Destruction
	FpInputDevice(void);
   ~FpInputDevice(void);

	// Accessors
	inline Status	GetStatus(void) const;
	void			SetStatus(const Status st);

private:
	// Attributes
	Status			m_status;	///< Status of the peripheral
};

//--------------------------------------------------------------------------------------------------
//	ACCESSORS
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
	@brief	Get input device status

	@return	Status of the peripheral
**/
//--------------------------------------------------------------------------------------------------

inline FpInputDevice::Status FpInputDevice::GetStatus() const 
{
	return m_status;
}

//--------------------------------------------------------------------------------------------------

#endif	//FP_INPUT_DEVICE_H
