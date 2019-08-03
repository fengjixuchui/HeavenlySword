//--------------------------------------------------------------------------------------------------
/**
	@file
	
	@brief		FpInput : generic Dualshock2-like pad interface

	@warning	Temporary PC port. will be rewritten

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.
**/
//--------------------------------------------------------------------------------------------------

#ifndef FP_PAD_H
#define FP_PAD_H

//--------------------------------------------------------------------------------------------------
//	SCE LIBRARY INCLUDES
//--------------------------------------------------------------------------------------------------

#include <cell/pad.h>

//--------------------------------------------------------------------------------------------------
// INCLUDES
//--------------------------------------------------------------------------------------------------

#include <Fp/FpInput/FpInputDevice.h>

//--------------------------------------------------------------------------------------------------
//	CLASS DEFINITION
//--------------------------------------------------------------------------------------------------

class FpPad : public FpInputDevice
{
public:

	// Construction
	FpPad(void);

	// Enumerations: Buttons
	enum ButtonId
	{
		kButtonFirst=0,					///< internal 			
			kButtonSelect=kButtonFirst,	///< "Select" button
			kButtonL3,					///< "L3" button (also known as I, or left stick click)
			kButtonR3,					///< "R3" button (also known as J, or right stick click)
			kButtonStart,				///< "Start" button	
			kButtonUp,					///< "Up" direction button (on left digital pad)
			kButtonRight,				///< "Right" direction button (on left digital pad)
			kButtonDown,				///< "Down" direction button (on left digital pad)
			kButtonLeft,				///< "Left" direction button (on left digital pad)
			kButtonL2,					///< "L2" shoulder button
			kButtonR2,					///< "R2" shoulder button
			kButtonL1,					///< "L1" shoulder button
			kButtonR1,					///< "R1" shoulder button
			kButtonTriangle,			///< "Triangle" button (on right digital pad)
			kButtonCircle,				///< "Circle" (on right digital pad)
			kButtonCross,				///< "Cross" (on right digital pad)
			kButtonSquare,				///< "Square" (on right digital pad)
		kButtonEnd,						///< internal 	
		kNumButtons=kButtonEnd			///< Total number of buttons
	};

	// Enumerations: Sticks
	enum StickId
	{
		kStickFirst=0,					///< internal		
			kStickLeft=kStickFirst,		///< Left analog stick
			kStickRight,				///< Right analog stick
		kStickEnd,						///< internal
		kNumSticks=kStickEnd			///< Total number of buttons
	};

	// Enumerations: Stick correction mode
	enum StickMode
	{
		kStickModeRaw=0,				///< Raw Data
		kStickModeSquare,				///< Square (= regular dualshock output) + square dead zone
		kStickModeCircle,				///< Circular (= remapped to a circle) + circular dead zone
		kStickModeCircleLocalDeadZone,	///< Circular (= remapped to a circle) + circular dead zone + local dead zone
		kNumStickModes,

		kStickModeUser,					///< User supplied function
	};

	// Enumerations : Sensors
	enum SensorId
	{
		kSensorAccelX = 0,
		kSensorAccelY,
		kSensorAccelZ,
		kSensorGyroY,
		kNumSensors
	};

	// Typedefs
	typedef void (*StickCorrectionFn) (float& rOutputX, float &rOutputY,
									   const float inputX, const float inputY, 
									   void* pUserData);

	// Operations

	///@name Main state of a button
	//@{ 
	inline u32		GetButtonFlags(void) const;
	inline bool		GetButtonDigital(const ButtonId id) const;
	inline float	GetButtonAnalog(const ButtonId id)const;
	inline bool		HasPressure(void) const;
	//@}

	///@name Helpers: digital states of all buttons (flags)
	//@{
	inline u32 		GetButtonFlagsPressed(void) const;
	inline u32 		GetButtonFlagsHeld(void) const;
	inline u32 		GetButtonFlagsReleased(void) const;
	inline u32 		GetButtonFlagsChanged(void) const;
	//@}

	///@name Helpers: digital state of a button
	//@{
	inline bool 	IsButtonPressed(const ButtonId id) const;
	inline bool 	IsButtonHeld(const ButtonId id) const;
	inline bool 	IsButtonReleased(const ButtonId id) const;
	inline bool 	IsButtonChanged(const ButtonId id) const;
	//@}

	///@name Sticks	
	//@{ 
	inline float 	GetStickRawX(const StickId id) const;
	inline float 	GetStickRawY(const StickId id) const;
	inline float 	GetStickX(const StickId id) const;
	inline float 	GetStickY(const StickId id) const;
	//@}

	///@name Parameters
	//@{
	void 			SetStickCorrectionMode(const StickId id, const StickMode mode);
	void 			SetStickUserCorrection(const StickId id, StickCorrectionFn pUserFunction, void* pUserData);
	void 			SetStickDeadZones(const StickId id, float mainDeadZone, const float localDeadZone=-1.0f);
	StickMode		GetStickCorrectionMode(const StickId id) const;
	float 			GetStickMainDeadZone(const StickId id) const;
	float 			GetStickLocalDeadZone(const StickId id) const;
	//@}

	///@name Sensors
	//@{
	inline bool		HasSensors(void) const;
	inline int		GetSensorSampleCountLastUpdate(void) const;
	int				GetSensorRawSamples(u16* pOutput, const SensorId idSensor, int maxOuput) const;
	//@}
		
	// Update
	void			StartUpdate();
	bool			Update(int retCellPadGetData, const CellPadData& data);
	void			EndUpdate();

	// Connection
	void			SetConnected(int lv2DeviceId);
	void			SetDisconnected(void);
	inline bool		IsConnected(void) const;

	// Internal operation
	inline int		GetLv2DeviceId(void) const {return m_lv2DeviceId;}
	void 			Invalidate(void);

protected:
	// Flags
	enum
	{
		kPadModePressure = 0x01,
		kPadModeSensor   = 0x02
	};
	// Enumerations
	enum
	{
		kSensorMaxSamples = 128	
	};

	// Operations
	bool 				Init(void);

	// Sensor-related operations
	void				AddSensorSample(const u16 accelX, const u16 accelY, const u16 accelZ, const u16 gyroY);
	static bool			IsValidSensorId(SensorId idSensor);

	// Attributes: Buttons
	struct Buttons
	{
		// Construction & init
		inline	Buttons(void) {SetDefaultParameters(); Invalidate();}
		void	Invalidate(void);
	
		// Attributes: Main state
		u32 	m_flagsState;				///< Digital state (all analog buttons have an emulated digital state)
		u32 	m_flagsPrevState;			///< Digital state (all analog buttons have an emulated digital state)
		float	m_analog[kNumButtons];		///< Analog state (all digital buttons have an emulated analog state)
		
		// Attributes: Digital transitions 
		u32 	m_flagsChanged;		 		///< Transition 1->0 or 0->1 : "changed" means any form of transition.
		u32 	m_flagsReleased;	 		///< Transition 1->0 : were "on" at previous update, and are now "off".
		u32 	m_flagsPressed;		 		///< Transition 0->1 : were "off" at previous update, and are now "on".
		u32		m_flagsHeld;				///< Kept 1->1 : were on both at previous update and now.

	private:	
		// Operations
		void	SetDefaultParameters(void);
	} m_buttons; ///< Button data

	// Attributes: Sticks
	class Stick
	{		
	public:
		// Construction & init
		inline				Stick(void) {SetDefaultParameters();Invalidate();}		
		void				Invalidate(void);	

		// Operations
		void				Update(const u32 rawX, const u32 rawY, const u32 maxRange);
		void 				SetDefaultParameters(void);
		void 				SetCorrectionMode(const StickMode mode);	
		void 				SetUserCorrection(StickCorrectionFn pUserFunction, void* pUserData);
		inline void 		SetDeadZones(const float mainDeadZone, const float localDeadZone) {m_dataDeadZone.m_mainDeadZone=mainDeadZone;m_dataDeadZone.m_localDeadZone=localDeadZone;}
		inline StickMode	GetCorrectionMode(void) const {return m_mode;}
		inline float 		GetMainDeadZone(void) const {return m_dataDeadZone.m_mainDeadZone;}
		inline float 		GetLocalDeadZone(void) const {return m_dataDeadZone.m_localDeadZone;}

		// Operations: Stick position
		inline float 		GetX(void) const;
		inline float 		GetY(void) const;
		inline float 		GetRawX(void) const;
		inline float 		GetRawY(void) const;
	private:

		// Operations: Stick correction
		static void 		CorrectCircularLocalDz(float& rOutputX, float &rOutputY,
												   const float inputX, const float inputY, 
												   void* pData);
		static void 		CorrectSquareDz(float& rOutputX, float &rOutputY,
											const float inputX, const float inputY, 
											void* pData);
		static void 		CorrectCircularDz(float& rOutputX, float &rOutputY,
											  const float inputX, const float inputY, 
											  void* pData);
		
		// Attributes: Current position
		float 						m_rawX;						///< Normalized raw x axis value, in [-1 +1] range. Duplicate, but better precision
		float 						m_rawY;						///< Normalized raw y axis value, in [-1 +1] range. Duplicate, but better precision
		float 						m_x;						///< Normalized (and corrected) x axis value, in [0.0f 1.0f] range.
		float 						m_y;						///< Normalized (and corrected) y axis value, in [0.0f 1.0f] range.

		// Attributes: Stick parameters 
		StickMode					m_mode;						///< Correction mode
		FpPad::StickCorrectionFn	m_pStickCorrectionFunction;	///< Correction function (can be a user function)
		void*						m_pStickCorrectionData;		///< Data associated with m_pStickCorrectionFunction
		
		// Attributes: Data for correction functions
		struct SimpleDeadZoneData
		{
			float m_mainDeadZone;								///< Dead zone.. (normalized in 0..1)
		};

		struct LocalDeadZoneData : public SimpleDeadZoneData
		{
			enum 
			{
				kNumPreviousSamples=16							///< Number of previous samples kept in "local dead zone" code
			};
			float m_localDeadZone;								///< Local dead zone.. (normalized in 0..1)
			int	  m_numPrevSamplesValid;						///< Internally used by "local dead zone" code; Number of valid samples in the "previous" buffers 
			float m_angle[kNumPreviousSamples];					///< Internally used by "local dead zone" code; Previous angle (radians, clockwise, origin=front)
			float m_magnitude[kNumPreviousSamples];				///< Internally used by "local dead zone" code;	Previous magnitude
			float m_angleDelta[kNumPreviousSamples];			///< Internally used by "local dead zone" code; Previous angle delta (radians, clockwise, origin=front)
			float m_magnitudeDelta[kNumPreviousSamples];		///< Internally used by "local dead zone" code; Previous magnitude delta
			float m_localDeadOriginX;							///< Internally used by "local dead zone" code; Current center of local dead zone.
			float m_localDeadOriginY;							///< Internally used by "local dead zone" code; Current center of local dead zone.
		} m_dataDeadZone;

	}m_stick[kNumSticks]; ///< Sticks (x2)

	// Attribute : sensor ring buffer
	u16				m_sensorSamples[kNumSensors][kSensorMaxSamples];	///< Sensor ring buffer
	int				m_sensorSampleLastIndex;							///< Sensor ring buffer management
	int				m_sensorSampleCount;								///< Sensor ring buffer management
	int				m_sensorSampleCountLastUpdate;						///< Sensor samples for last update (useful for IIR filters)

	int				m_lv2DeviceId;										///< Lv2 device id - matches the FpInput id
	unsigned int	m_padMode;											///< Pad flags
};	

//--------------------------------------------------------------------------------------------------
//	INLINES
//--------------------------------------------------------------------------------------------------

#include <Fp/FpInput/FpPad.inl>

//--------------------------------------------------------------------------------------------------

#endif	//FP_PAD_H
