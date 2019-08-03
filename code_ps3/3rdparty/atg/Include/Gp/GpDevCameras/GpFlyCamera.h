//--------------------------------------------------------------------------------------------------
/**
	@file		
	
	@brief		Fly Camera - Allows user to 'fly' around the scene using a DualShock2.

	@note		(c) Copyright Sony Computer Entertainment 2004. All Rights Reserved.	
**/
//--------------------------------------------------------------------------------------------------

#ifndef GP_FLY_CAMERA_H
#define GP_FLY_CAMERA_H

//--------------------------------------------------------------------------------------------------
//	INCLUDES
//--------------------------------------------------------------------------------------------------

#include <Fp/FpInput/FpPad.h>
#include <Gp/GpDevCameras/GpDevCamera.h>

//--------------------------------------------------------------------------------------------------
//	CLASS DEFINITION
//--------------------------------------------------------------------------------------------------

class GpFlyCamera : public GpDevCamera
{
public:                               

	// Constants
	
	static const float	kDefaultViewPosAccel;
	static const float	kDefaultViewPosStrafeAccel;
	static const float	kDefaultViewPosHoverAccel;
	static const float	kDefaultViewRotAccel;

	static const float	kDefaultViewPosMaxSpeed;
	static const float	kDefaultViewPosStrafeMaxSpeed;
	static const float	kDefaultViewPosHoverMaxSpeed;
	static const float	kDefaultViewRotMaxSpeed;


	
	// Construction & Destruction
	
	GpFlyCamera(FwPoint_arg viewPos, FwVector_arg viewRot, float aspect);
	
	virtual ~GpFlyCamera();

	
	// Operations

	virtual void	Reset();
	
	virtual void	InputHandler();
	
	virtual void	Update(float deltaTime);
	
	
	// Access

	FpPad::StickId	GetRotationAnalogStick() const;
	void			SetRotationAnalogStick( FpPad::StickId stick );
	
	FwPoint		GetViewPos() const;
	void		SetViewPos(FwPoint_arg viewPos);

	FwVector	GetViewRot() const;
	void		SetViewRot(FwVector_arg viewRot);
	
	FwVector	GetViewPosAccel() const;
	void		SetViewPosAccel(FwVector_arg accel);
	
	FwVector	GetViewPosMaxSpeed() const;
	void		SetViewPosMaxSpeed(FwVector_arg maxSpeed);
	
	FwVector	GetViewRotAccel() const;
	void		SetViewRotAccel(FwVector_arg accel);
	
	FwVector	GetViewRotMaxSpeed() const;
	void		SetViewRotMaxSpeed(FwVector_arg maxSpeed);

				   
private:

	// Constants	
	
	static const float	kDefaultFovyDelta;
	static const float	kDefaultFovyMin;
	static const float	kDefaultFovyMax;

	static const float	kLockAnalogAxisThreshold;

	static const FpPad::StickId	kDefaultRotationAnalogStick;
	
	// Attributes

	FpPad::StickId	m_rotationStick;				///< The analog stick to use for rotation.
	
	FwPoint			m_viewPos;						///< View position in world space.
	FwVector		m_viewRot;						///< View rotation euler angles in world space / radians.
	
	FwVector		m_currViewPosVel;				///< Current view position velocity, acceleration, decelaration and maximum speed.
	FwVector		m_currViewPosAccel;
	FwVector		m_currViewPosDecel;
	FwVector		m_currViewPosMaxSpd;
				
	FwVector		m_viewPosAccel;                 ///< View position acceleration.
	FwVector		m_viewPosMaxSpd;                ///< View position maximum speed.

	FwVector		m_currViewRotVel;				///< Current view rotation velocity, acceleration decelaration and maximum speed.
	FwVector		m_currViewRotAccel;
	FwVector		m_currViewRotDecel;
	FwVector		m_currViewRotMaxSpd;
	
	FwVector		m_viewRotAccel;					///< View rotation acceleration (in radians).
	FwVector		m_viewRotMaxSpd;				///< View rotation maximum speed (in radians).

	bool			m_inHorizontalPlaneMode;		///< True if in horizontal-plane movement mode.


	// Operations

	float	UpdateVelocity(float vel, float accel, float decel, float maxSpd) const;
};

//--------------------------------------------------------------------------------------------------
//	INLINE FUNCTION DEFINITIONS
//--------------------------------------------------------------------------------------------------

inline FpPad::StickId	GpFlyCamera::GetRotationAnalogStick() const						{	return m_rotationStick; }
inline void				GpFlyCamera::SetRotationAnalogStick( FpPad::StickId stick )		{	m_rotationStick = stick; }

inline FwPoint	GpFlyCamera::GetViewPos() const							{	return m_viewPos;			}
inline void		GpFlyCamera::SetViewPos(FwPoint_arg viewPos)			{	m_viewPos = viewPos;		}

inline FwVector	GpFlyCamera::GetViewRot() const							{	return m_viewRot;			}
inline void		GpFlyCamera::SetViewRot(FwVector_arg viewRot)			{	m_viewRot = viewRot;		}

inline FwVector	GpFlyCamera::GetViewPosAccel() const					{	return m_viewPosAccel;		} 
inline void	   	GpFlyCamera::SetViewPosAccel(FwVector_arg accel)		{	m_viewPosAccel = accel;		}

inline FwVector	GpFlyCamera::GetViewPosMaxSpeed() const					{	return m_viewPosMaxSpd;		}
inline void		GpFlyCamera::SetViewPosMaxSpeed(FwVector_arg maxSpeed)	{	m_viewPosMaxSpd = maxSpeed;	}

inline FwVector	GpFlyCamera::GetViewRotAccel() const					{	return m_viewRotAccel;		} 
inline void	   	GpFlyCamera::SetViewRotAccel(FwVector_arg accel)        {	m_viewRotAccel = accel;		}

inline FwVector	GpFlyCamera::GetViewRotMaxSpeed() const					{	return m_viewRotMaxSpd;		}
inline void	   	GpFlyCamera::SetViewRotMaxSpeed(FwVector_arg maxSpeed)	{	m_viewRotMaxSpd = maxSpeed;	}

//--------------------------------------------------------------------------------------------------

#endif // GP_FLY_CAMERA_H

