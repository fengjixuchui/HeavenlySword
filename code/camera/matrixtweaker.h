/***************************************************************************************************
*
*	$Header:: /game/matrixtweaker.h 1     11/08/03 16:31 Wil                                       $
*
*
*
*	CHANGES
*
*	11/8/2003	Wil	Created
*
***************************************************************************************************/

#ifndef MATRIX_TWEAKER
#define MATRIX_TWEAKER

#include "camera/converger.h"
#include "camera/smoother.h"

/***************************************************************************************************
*
*	CLASS			CMatrixTweakerDef
*
*	DESCRIPTION		Defines parameters of the tweaker
*
***************************************************************************************************/
class CMatrixTweakerDef
{
public:
	#define MT_SPEED	90.0f
	#define MT_SPRING	5.0f
	#define MT_DAMP		0.1f
	
	#define MT_SAMPLES	30
	#define MT_TIGHT	0.5f
	#define MT_LAG		0.25f
	
	#define MT_MAX_LONG	120.0f
	#define MT_MAX_LAT	80.0f
	
	CMatrixTweakerDef(const char* pcName) :
		m_obConDef("INVALID"),
		m_obSmoothDef("INVALID"),
		m_eParentPad(PAD_0),
		m_fMaxLong(MT_MAX_LONG),
		m_fMaxLat(MT_MAX_LAT),
		m_bPreserveY(false),
		m_bReverseY(false)
	{
		UNUSED(pcName);

		m_obConDef.SetSpeed(MT_SPEED);
		m_obConDef.SetSpring(MT_SPRING);
		m_obConDef.SetDamp(MT_DAMP);

		m_obSmoothDef.SetNumSamples(MT_SAMPLES);
		m_obSmoothDef.SetTightness(MT_TIGHT);
		m_obSmoothDef.SetLagFrac(MT_LAG);
	}


	void	SetPad(PAD_NUMBER ePad)		{ ntAssert(ePad < PAD_NUM); m_eParentPad = ePad; }
	void	SetMaxLong(float fMaxLong)	{ m_fMaxLong = fMaxLong; }
	void	SetMaxLat(float fMaxLat)		{ m_fMaxLat = fMaxLat; }
	void	SetYPreserve(bool bYPres)		{ m_bPreserveY = bYPres; }
	void	SetYReverse(bool bYPres)		{ m_bPreserveY = bYPres; }

	PAD_NUMBER	GetPad() const		{ return m_eParentPad; }
	float		GetMaxLong() const	{ return m_fMaxLong; }
	float		GetMaxLat() const		{ return m_fMaxLat; }
	bool		GetYPreserve() const	{ return m_bPreserveY; }
	bool		GetYReverse() const	{ return m_bReverseY; }

	CConvergerDef	m_obConDef;
	CSmootherDef	m_obSmoothDef;
	
private:
	PAD_NUMBER		m_eParentPad;
	float			m_fMaxLong;
	float			m_fMaxLat;
	bool			m_bPreserveY;
	bool			m_bReverseY;
};

/***************************************************************************************************
*
*	CLASS			CMatrixTweaker
*
*	DESCRIPTION		Allows the player to adjust the direction the camera looks at
*
***************************************************************************************************/
class CMatrixTweaker
{
public:
	enum MT_TYPE
	{
		MANUAL			= 0,
		ELASTIC			= 1,

		MT_TYPE_MAX,
	};

	CMatrixTweaker(const CMatrixTweakerDef& obDef);
	virtual ~CMatrixTweaker();

	// The entire public interface must be virtual so it may be wrapped with an editor object
	virtual void			Reset();
	virtual CMatrix			ApplyTweak(const CMatrix& obSrc, float fTimeChange);
	virtual void			Render() {};
	virtual void			RenderInfo(int iX, int iY);
	virtual CMatrix			GetLastSrc()		{ ntAssert(m_bInitialised); return m_obLastSrc; }
	virtual CMatrix			GetLastTweaked()	{ ntAssert(m_bInitialised); return m_obLastTweaked; }

	// We need to know whether an item is really serialised
	virtual bool IsSerialised() const { return true; }

	// Provide a static method to destroy these items based on whether they are serialised
	static void Destroy(CMatrixTweaker* pobItem)
	{
		// Check that all is well
		ntAssert(pobItem);
		
		// Delete the item if it was built by the serialiser
		if (pobItem->IsSerialised())
		{
			NT_DELETE_CHUNK(Mem::MC_CAMERA, pobItem );
		}
	}

protected:
	virtual void CalculateLatLongOffsets(float fTimeChange) = 0;
	const CMatrixTweakerDef	m_obDef;
	float 		m_fLongOffset;
	float 		m_fLatOffset;

private:
	CMatrixTweaker&	operator = (const CMatrixTweaker& obCopy);

	bool		m_bInitialised;
	CMatrix		m_obLastSrc;
	CMatrix		m_obLastTweaked;

	CSmoother*	m_pobLongSmoother;
	CSmoother*	m_pobLatSmoother;

	CConverger	m_obLongConverger;
	CConverger	m_obLatConverger;
};

/***************************************************************************************************
*
*	CLASS			CCameraElasticTweaker
*
*	DESCRIPTION		Looked at direction changes according to viewer input
*
***************************************************************************************************/
class CCameraElasticTweaker : public CMatrixTweaker
{
public:
	CCameraElasticTweaker(const CMatrixTweakerDef& obDef) : CMatrixTweaker(obDef) {}

protected:
	virtual void CalculateLatLongOffsets(float fTimeChange);
};

/***************************************************************************************************
*
*	CLASS			CCameraManualTweaker
*
*	DESCRIPTION		Looked at direction changes according to viewer input, and persists
*
***************************************************************************************************/
class CCameraManualTweaker : public CMatrixTweaker
{
public:
	CCameraManualTweaker(const CMatrixTweakerDef& obDef) : CMatrixTweaker(obDef) { m_bLookaround = false; }

protected:
	virtual void CalculateLatLongOffsets(float fTimeChange);

private:
	bool	m_bLookaround;
};

#endif // MATRIX_TWEAKER
