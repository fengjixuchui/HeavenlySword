#ifndef _TRANSFORMCONTROLLER_H
#define _TRANSFORMCONTROLLER_H

class Transform;

class TransformControllerDebugUpdate;

#include "input/inputhardware.h"

#include "game/tcdebugupdatemode.h"

/***************************************************************************************************
*
*	CLASS			CTransformController
*
*	DESCRIPTION		Controls a transform using the analog controls on pad zero.
*
***************************************************************************************************/

class CTransformController
{
public:
	//! Creates an empty transform controller.
	CTransformController();
	~CTransformController();

	//! Sets the transform to control.
	void SetTransform(Transform* pobTransform);

	//! Reset Controller to this matrix
	void ResetController( const CMatrix& obMat );

	//! Sets the world-space position of the camera.
	void SetPosition(CPoint const& obPosition);

	//! Gets the world space position of the camera.
	CPoint const& GetPosition() const { return m_obPosition; }

	// Get the transform of this controller
	Transform& GetTransform() {return *m_pobTransform;}

	//! Sets phi in spherical polar coordinates (the longtitude/turn).
	void SetPhi(float fPhi);

	//! Sets theta in spherical polar coordinates (the latitude/pitch).
	void SetTheta(float fTheta);

	//! Gets phi in spherical polar coordinates (i.e. the camera turn or longtitude).
	float GetPhi() const { return m_fPhi; }

	//! Gets theta in spherical polar coordinates (i.e. the camera pitch or latitude).
	float GetTheta() const { return m_fTheta; }

	
	//! Updates the transform using halo-style controls on pad zero.
	void DebugUpdateFromPad(PAD_NUMBER ePad);

	//! Updates the transform using halo-style controls on pad zero.
	void DebugUpdateFromPad(PAD_NUMBER ePad, float fMoveFactor, float fRotateFactor);


	//! Set the debug update mode that the above two functions use.
	void					SetDebugUpdateMode		( TC_DEBUG_UPDATE_MODE mode );
	TC_DEBUG_UPDATE_MODE	GetDebugUpdateMode		()	const;

	const char * const		GetDebugUpdateModeDesc	()	const;

	//! Advance the debug update mode - wraps at the end of the mode list.
	void					AdvanceDebugUpdateMode	();
	
private:
	void UpdateTransform();

	CPoint m_obPosition;
	float m_fPhi, m_fTheta;

	Transform* m_pobTransform;		//!< The transform to control.

	TransformControllerDebugUpdate *m_pobDebugUpdateMode;
};

#endif // ndef _TRANSFORMCONTROLLER_H

