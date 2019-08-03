#ifndef	_HUDOVERLAY_H
#define	_HUDOVERLAY_H

//	scee.sbashow : this is for the hud image overlay scheme
class HudImageRenderer;


template <class T> class COverlayResourceManager
{
	public:

		COverlayResourceManager( int iSize	) : m_iSize(iSize)
		{
			ntAssert(iSize>0 && iSize<=32);
			m_paobOverlays = NT_NEW_ARRAY T[iSize];
			m_iCurrIndex = -1;
			m_uiUsed = 0;
		}

		~COverlayResourceManager()
		{
			 NT_DELETE_ARRAY( m_paobOverlays );
		}

		T* Request()
		{
			int iLastCurr = m_iCurrIndex++;

			if (m_iCurrIndex==m_iSize)
			{
				m_iCurrIndex = 0;
			}

			while(m_iCurrIndex!=iLastCurr 
				  && ((1<<m_iCurrIndex)&m_uiUsed))
			{
                m_iCurrIndex++;

				if (m_iCurrIndex==m_iSize)
				{
					m_iCurrIndex = 0;
				}
			}

			if (m_iCurrIndex!=iLastCurr)
			{
				m_uiUsed|=(1<<m_iCurrIndex);

				return &m_paobOverlays[m_iCurrIndex];
			}

			return 0;
		}

		void Release(T* pobToRelease)
		{
			m_iCurrIndex = 
				(pobToRelease-m_paobOverlays);

			ntError(m_uiUsed&(1<<m_iCurrIndex));
			m_uiUsed&=~(1<<m_iCurrIndex);

			m_iCurrIndex--;
		}

	private:
		T* 						m_paobOverlays;
		int 					m_iSize;
		int						m_iCurrIndex;
		u_int					m_uiUsed;	
};

class HudImageOverlay
{
public:

	virtual ~HudImageOverlay(){}

	enum OverlayFiler
	{
		OF_POS	=		1,
		OF_COLOUR	=	1<<1,
		OF_SCALE	=	1<<2,
		OF_RENDFLAG =	1<<3,
		OF_ALL = (OF_POS | OF_COLOUR | OF_SCALE | OF_RENDFLAG)
	};

	enum OverlayType
	{
		OT_SLAP,
		OT_INCREMENTAL
	};

	enum	OverlayMode
	{
		OM_LINEAR,
		OM_OSCILLATE
	};

	virtual OverlayType	GetType() const =0;
	virtual OverlayMode GetMode() const =0;
private:
};

class HudImageOverlaySlap : public HudImageOverlay
{
public:
	CVector m_obColour;
	
	// scee.sbashow : could make into a vector, 
	//	to speed up simple interpolation of all values at once.
	float 	m_fTopLeftX;
	float 	m_fTopLeftY;
	float 	m_fScale;

    bool	m_bRendering;

	HudImageOverlaySlap();

	OverlayType GetType() const	{	return OT_SLAP;		}
	OverlayMode GetMode() const	{	return OM_LINEAR; 	}

	static void InterpToOverlay(HudImageOverlaySlap& obTarg,
                                const HudImageOverlaySlap& obSource,
								const HudImageOverlaySlap& obFinal,
								float fInterp,
								int iFilter);

	static void	ApplyImmOverlay( HudImageOverlaySlap& obTarg, 
								 const HudImageOverlaySlap& obSource, int iFilter);



};

class HudImageOverlayIncremental : public HudImageOverlay
{
public:
	HudImageOverlayIncremental(OverlayMode eMode = OM_LINEAR, 
							   float fCharacteristicTime = 0.0f)
		:	m_fApplicationWeight(1.0f),
            m_iFilter(OF_POS|OF_SCALE),
			m_fCharacteristicTime(fCharacteristicTime),
			m_fCurrParam(1.0f),
			m_fTimeGoing(0.0f),
			m_eOverlayMode(eMode)
	{}

	float 	m_fDX;
	float 	m_fDY;
	float   m_fDfScale;
	float	m_fApplicationWeight;
	CVector m_obDColour;
	int		m_iFilter;

	OverlayType GetType() const	{	return OT_INCREMENTAL;	}
	OverlayMode GetMode() const	{	return m_eOverlayMode; 	}

	bool Update( float fTimeStep );
	const HudImageOverlaySlap ApplyTo(const HudImageOverlaySlap& obIn);

private:
	float		m_fCharacteristicTime;
	float		m_fCurrParam;
	float		m_fTimeGoing;
	OverlayMode m_eOverlayMode;
};


#endif	//_HUDOVERLAY_H
