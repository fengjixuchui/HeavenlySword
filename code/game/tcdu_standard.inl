/***************************************************************************************************
*
*	The original, standard, transform controller debug update.
*
***************************************************************************************************/


class Standard : public TransformControllerDebugUpdate
{
	public:
		//
		//	Implementation of debug update mode interface.
		//
		TC_DEBUG_UPDATE_MODE	GetMode		()	const	{ return TCDUM_STANDARD; }
		const char * const		GetDesc		()	const	{ return "Standard Update"; }

		void					Update		( PAD_NUMBER ePad, float fMoveFactor, float fRotateFactor, CPoint &ptPosition,
											  Transform *pTransform, float &fPhi, float &fTheta )	const;
};

void Standard::Update( PAD_NUMBER ePad, float fMoveFactor, float fRotateFactor, CPoint &ptPosition, Transform *pTransform, float &fPhi, float &fTheta )	const
{
	// Find out which pad to read
	const CInputPad &obPad( CInputHardware::Get().GetPad( ePad ) );

	// Increase speed by a factor of 5 if this button is held
	if ( obPad.GetHeld() & PAD_LEFT_THUMB )
	{
		fMoveFactor *= 5.0f;
	}

	// update the position and orientation from the pad analog sticks
	CDirection obRight = -fMoveFactor * obPad.GetAnalogLXFrac() * pTransform->GetWorldMatrix().GetXAxis();
	
	CDirection obForwards( CONSTRUCT_CLEAR );
	CDirection obUp		 ( CONSTRUCT_CLEAR );

	// if Y is held then move up, otherwise move forwards
	if( ( obPad.GetHeld() & PAD_FACE_4 ) == 0 )
	{
		obForwards = -fMoveFactor * obPad.GetAnalogLYFrac() * pTransform->GetWorldMatrix().GetZAxis();
	}
	else
	{
		obUp = -fMoveFactor * obPad.GetAnalogLYFrac() * pTransform->GetWorldMatrix().GetYAxis();
	}

	ptPosition += obForwards + obRight + obUp;
		
	float fTurn = -fRotateFactor * obPad.GetAnalogRXFrac();
	float fTilt = -fRotateFactor * obPad.GetAnalogRYFrac();

	fPhi += fTurn;
	while( fPhi < 0 )
	{
		fPhi += 2*PI;
	}

	while( fPhi > 2*PI )
	{
		fPhi -= 2*PI;
	}

	fTheta += fTilt;


}

