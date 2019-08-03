/***************************************************************************************************
*
*	The new, extra-special, transform controller debug update.
*
***************************************************************************************************/

class ExtraSpecial : public TransformControllerDebugUpdate
{
	public:
		//
		//	Implementation of debug update mode interface.
		//
		TC_DEBUG_UPDATE_MODE	GetMode		()	const	{ return TCDUM_EXTRA_SPECIAL; }
		const char * const		GetDesc		()	const;

		void					Update		( PAD_NUMBER ePad, float fMoveFactor, float fRotateFactor, CPoint &ptPosition,
											  Transform *pTransform, float &fPhi, float &fTheta )	const;

	public:
		//
		//	Ctor, dtor.
		//
		ExtraSpecial()
		:	m_bGoSlow	( false )
		,	m_btop2Held	( false )
		,	m_FixHeroineToCamera( false )
		,	m_VectorToHeroine_CameraS( 0.0f, 0.0f, 5.0f )
		{}

		virtual ~ExtraSpecial()
		{}

	private:
		//
		//	Aggregated members.
		//
		mutable bool		m_bGoSlow;
		mutable bool		m_btop2Held;
		mutable bool		m_FixHeroineToCamera;
		mutable CDirection	m_VectorToHeroine_CameraS;
};



const char * const ExtraSpecial::GetDesc() const
{
	if ( m_bGoSlow )
	{
		return "Extra Special Update - Slow Movement";
	}

	return "Extra Special Update";
}

