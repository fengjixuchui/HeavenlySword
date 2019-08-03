/***************************************************************************************************
*
*	$Header:: /game/camvolumes.h 1     11/08/03 16:30 Wil                                          $
*
*
*
*	CHANGES
*
*	11/8/2003	Wil	Created
*
***************************************************************************************************/

#ifndef _CAM_VOLUMES_H
#define _CAM_VOLUMES_H

// scee.sbashow: needs to be commented out before checking in.
//	#define CAM_VOLUMES_DEBUG


// Necessary includes

class CamVolumeSet;
class CamVolumeBox;

/***************************************************************************************************
*
*	CLASS			CamVolume
*
*	DESCRIPTION		interface for volume funtionality
*
***************************************************************************************************/
class CamVolume
{
public:


	class CIntersectResults
	{
	public:
		friend class CamVolume;
		friend class CamVolumeBox;
		friend class CamVolumeCylinder;
		friend class CamVolumeSphere;

		CIntersectResults(const CPoint& obRefPoint) :
			m_obSurface(CONSTRUCT_CLEAR),
			m_obNormal(CONSTRUCT_CLEAR),
			m_obRefPoint(obRefPoint)
		{
			m_obInfo.m_bValid = false;
		}
		CIntersectResults(const CPoint& obRefPoint,
						  const CPoint& obSurface, 
						  const CDirection& obNormal) :
			m_obSurface(obSurface),
			m_obNormal(obNormal),
			m_obRefPoint(obRefPoint)
		{
			m_obInfo.m_bValid = true;
		}

		CIntersectResults(const CPoint& obRefPoint, const CIntersectResults& obResults) :
			m_obSurface(obResults.m_obSurface),
			m_obNormal(obResults.m_obNormal),
			m_obRefPoint(obRefPoint),
			m_obInfo(obResults.m_obInfo)
		{
		}

		void	SetResults(const CIntersectResults& obResult)
		{
			if (obResult.IsValid())
			{
				m_obSurface = obResult.m_obSurface;
				m_obNormal = obResult.m_obNormal;
				m_obInfo = obResult.m_obInfo;
			}
		}

		CIntersectResults& operator=(const CIntersectResults& obResults){SetResults(obResults); return *this;}

		void InvertNormal()
		{
			m_obNormal = -m_obNormal;
		}

		void	SetResults(const CPoint& obSurface, 
						   const CDirection& obNormal)
		{
			m_obInfo.m_bValid = true;
			m_obSurface = obSurface; 
			m_obNormal = obNormal; 
		}

		// returns true only if we're already set
		bool		SetIfNearFar(		const CPoint& obSurface, const CDirection& obNormal, bool bNear = true);
		bool		IsValid()									 const {return m_obInfo.m_bValid;}
		CPoint		GetSurface()								 const {ntAssert(IsValid()); return m_obSurface;}
		CDirection	GetNormal()									 const {ntAssert(IsValid()); return m_obNormal;}
		CPoint		GetRefPos()									 const {	return m_obRefPoint; }
		const CamVolume* GetIRVolumeAndSurface(int& iSurfaceID)  const { iSurfaceID = m_obInfo.m_iSurfaceID; return m_obInfo.m_pobVolume; }

	private:

		CPoint			m_obSurface;
		CDirection		m_obNormal;
		const CPoint 	m_obRefPoint;

		struct IR_INFO
		{
			IR_INFO()
			{
				m_bValid = false;
				m_pobVolume = 0;
				m_iSurfaceID = -1;
			}

			bool				m_bValid;
			const CamVolume*  	m_pobVolume;
			int					m_iSurfaceID;
			bool				_buff;
		};

		IR_INFO	m_obInfo;
	};

	struct CLAMP_ENV_INFO
	{
		const CPoint		*m_pobPOI;
		const CDirection 	*m_pobPOIPlaneNormal;
	};

	CamVolume() {m_obLastTestPoint.Clear(); m_bSelected = false; m_bIsAdd = true;}
	virtual ~CamVolume() {};

	// point queries
	virtual bool		Inside(const CPoint& obPos) const = 0;
	virtual bool		ClampToSurf(const CPoint& obPos, const CLAMP_ENV_INFO& obCInfo, CIntersectResults& obResult) const = 0;
	virtual bool		ExClampToSurf(const CPoint& obPos, const CLAMP_ENV_INFO& obCInfo, CIntersectResults& obResult) const = 0;
	
	// line queries
	virtual bool		Intersect(const CPoint& obStart, const CPoint& obEnd) const = 0;
	virtual bool		GetIntersectAndNormal(const CPoint& obStart, 
											  const CPoint& obEnd,
											  CIntersectResults& obResult,
											  const CamVolumeSet* pobParentSet) const =0;
	// misc funcs
	virtual void		GetRandomPoint(CPoint& obPos) const = 0;
	virtual CPoint		GetPosition() const = 0;

	// debug stuff
	virtual void		Render(bool bParentSelected, bool bSiblingSelected) const = 0;
	virtual void		Render(float fRed, float fGreen, float fBlue, float fAlpha) const = 0;
	virtual void		RenderInfo(int iX, int iY) const = 0;

	virtual int 		GetSurfaceID(const CDirection& obLocalNormal ) const =0;
	// Editor Events
	// virtual void EditorSelect( bool bSelect ) { m_bSelected = bSelect; }
	bool IsSelected() const {return m_bSelected;}

	bool UsesPOI() const {	return m_bUsesPOI;	}
	bool IsAdd() const 	 {	return m_bIsAdd;	}
	bool	m_bIsAdd;
	
protected:

	mutable CPoint		m_obLastTestPoint;	// for debug rendering purposes only
	mutable bool m_bSelected;

	bool	m_bUsesPOI;
};

/***************************************************************************************************
*
*	CLASS			CamVolumeBox
*
*	DESCRIPTION		box volume
*
***************************************************************************************************/
class CamVolumeBox : public CamVolume
{
public:
	HAS_INTERFACE(CamVolumeBox)
	CamVolumeBox();
	CamVolumeBox(const CMatrix& obTransform, const CPoint& obDimension);

	virtual void PostConstruct();
	virtual bool OnEdit(CallBackParameter, CallBackParameter) {m_bDirty = true; return true;}

	// point queries
	virtual bool		Inside(const CPoint& obPos) const;
	virtual bool		ClampToSurf(const CPoint& obPos, const CLAMP_ENV_INFO& obCInfo, CIntersectResults& obResult) const;
	virtual bool		ExClampToSurf(const CPoint& obPos, const CLAMP_ENV_INFO& obCInfo, CIntersectResults& obResult) const;

	// line queries
	virtual bool		Intersect(const CPoint& obStart, const CPoint& obEnd) const;
	virtual bool		GetIntersectAndNormal(const CPoint& obStart, 
											  const CPoint& obEnd,
											  CIntersectResults& obResult,
											  const CamVolumeSet* pobParentSet) const;
	// misc funcs
	virtual void		GetRandomPoint(CPoint& obPos) const;
	virtual CPoint		GetPosition()				const {return m_obSrcMat.GetTranslation();}
	const CMatrix&		GetRotTrans()				const {return m_obSrcMat;}
	CPoint				GetDim()					const {return m_obDimensions;}
	const CMatrix&		GetWorldToLocal()			const {if(m_bDirty) RebuildMat(); return m_obWorldToLocal;}
	const CMatrix&		GetLocalToWorld()			const {if(m_bDirty) RebuildMat(); return m_obLocalToWorld;}

	void				SetPosition(const CPoint& obPos)	{m_bDirty = true; m_obSrcMat.SetTranslation(obPos);}
	void				SetRotTrans(const CMatrix& obMat)	{m_bDirty = true; m_obSrcMat = obMat;}
	void				SetDim(const CPoint& obDim)			{m_bDirty = true; m_obDimensions = obDim;}

	// debug stuff
	virtual void		Render(bool bParentSelected, bool bSiblingSelected) const;
	virtual	void		Render(float fRed, float fGreen, float fBlue, float fAlpha) const;
	virtual void		RenderInfo(int iX, int iY) const;

	// Welder Events
	virtual bool		EditorChangeValue(const char*, const char*) {m_bDirty = true; return false;}
	virtual void		DebugRender ();

	virtual int 		GetSurfaceID(const CDirection& obLocalNormal ) const;
private:
	void				RebuildMat() const;

	mutable CMatrix				m_obSrcMat;

	// Welder Attributes
	CPoint				m_obPosition;			// Matrix made from these
	CQuat				m_obOrientation;	//
	CPoint				m_obDimensions;
	
	// these are mutable so we can do last minute caluclation of them if Inside() is called.
	mutable bool		m_bDirty;
	mutable CMatrix		m_obLocalToWorld;
	mutable CMatrix		m_obWorldToLocal;

	friend class CamVolumeBoxI;
};

/***************************************************************************************************
*
*	CLASS			CamVolumeSphere
*
*	DESCRIPTION		sphere volume
*
***************************************************************************************************/
class CamVolumeSphere : public CamVolume
{
public:
	HAS_INTERFACE(CamVolumeSphere)
	CamVolumeSphere();
	CamVolumeSphere(const CPoint& obPosition, float fRadius);

	// point queries
	virtual bool		Inside(const CPoint& obPos) const;
	virtual bool		ClampToSurf(const CPoint& obPos, const CLAMP_ENV_INFO& obCInfo, CIntersectResults& obResult) const;
	virtual bool		ExClampToSurf(const CPoint& obPos, const CLAMP_ENV_INFO& obCInfo, CIntersectResults& obResult) const;

	// line queries
	virtual bool		Intersect(const CPoint& obStart, const CPoint& obEnd) const;
	virtual bool		GetIntersectAndNormal(const CPoint& obStart,  
											  const CPoint& obEnd,
											  CIntersectResults& obResult,
											  const CamVolumeSet* pobParentSet) const;
	// misc funcs
	virtual void		GetRandomPoint(CPoint& obPos) const;
	virtual CPoint		GetPosition()				const {return m_obPosition;}
	float				GetRadius()					const {return m_fRadius;}
	float				GetRadiusSq()				const {return m_fRadius * m_fRadius;}

	void				SetPosition(const CPoint& obPos)	{m_obPosition = obPos;}
	void				SetRadius(float fRadius)			{m_fRadius = fRadius;}

	// debug stuff
	virtual void		Render(bool bParentSelected, bool bSiblingSelected) const;
	virtual	void		Render(float fRed, float fGreen, float fBlue, float fAlpha) const;
	virtual void		RenderInfo(int iX, int iY) const;

	virtual int 		GetSurfaceID(const CDirection& obLocalNormal ) const	{ UNUSED(obLocalNormal); return -1;}

	// Wielder events
	virtual void		DebugRender ();
private:
	// Welder Attributes
	friend class CamVolumeSphereI;
	CPoint				m_obPosition;
	float				m_fRadius;
};

/***************************************************************************************************
*
*	CLASS			CamVolumeCylinder
*
*	DESCRIPTION		cylinder volume
*
***************************************************************************************************/
class CamVolumeCylinder : public CamVolume
{
public:
	HAS_INTERFACE( CamVolumeCylinder )
	CamVolumeCylinder();
	CamVolumeCylinder(const CPoint& obPosition, const CDirection& obAxis, float fRadius, float fHeight);

	virtual void PostConstruct();


	// point queries
	virtual bool		Inside(const CPoint& obPos) const;
	virtual bool		ClampToSurf(const CPoint& obPos, const CLAMP_ENV_INFO& obCInfo, CIntersectResults& obResult) const;
	virtual bool		ExClampToSurf(const CPoint& obPos, const CLAMP_ENV_INFO& obCInfo, CIntersectResults& obResult) const;

	// line queries
	virtual bool		Intersect(const CPoint& obStart, const CPoint& obEnd) const;
	virtual bool		GetIntersectAndNormal(const CPoint& obStart, 
											  const CPoint& obEnd,
											  CIntersectResults& obResult,
											  const CamVolumeSet* pobParentSet) const;
	// misc funcs
	virtual void		GetRandomPoint(CPoint& obPos) const;
	virtual CPoint		GetPosition()				const {return m_obPosition;}
	CDirection			GetAxis()					const {return m_obAxis;}
	float				GetRadius()				const {return m_fRadius;}
	float				GetHeight()				const {return m_fHeight;}
	const CMatrix&		GetRotTrans()				const {if(m_bDirty) RebuildMat(); return m_obSrcMat;}
	const CMatrix&		GetWorldToLocal()			const {if(m_bDirty) RebuildMat(); return m_obWorldToLocal;}
	const CMatrix&		GetLocalToWorld()			const {if(m_bDirty) RebuildMat(); return m_obLocalToWorld;}

	void				SetPosition(const CPoint& obPos)	{m_bDirty = true; m_obPosition = obPos;}
	void				SetAxis(const CDirection& obAxis)	{m_bDirty = true; m_obAxis = obAxis;}
	void				SetRadius(float fRadius)			{m_bDirty = true; m_fRadius = fRadius;}
	void				SetHeight(float fHeight)			{m_bDirty = true; m_fHeight = fHeight;}

	// debug stuff
	virtual void		Render(bool bParentSelected, bool bSiblingSelected) const;
	virtual	void		Render(float fRed, float fGreen, float fBlue, float fAlpha) const;
	virtual void		RenderInfo(int iX, int iY) const;

	virtual int 		GetSurfaceID(const CDirection& obLocalNormal ) const	{UNUSED(obLocalNormal); return -1;}

	// Wielder events
	virtual void		DebugRender ();

private:
	void				RebuildMat() const;

	// these are mutable so we can to last minute caluclation of them if Intersect is called.
	mutable bool		m_bDirty;
	mutable CMatrix		m_obSrcMat;
	mutable CMatrix		m_obLocalToWorld;
	mutable CMatrix		m_obWorldToLocal;

// Welder Attributes
private:
	friend class CamVolumeCylinderI;
	CPoint				m_obPosition;
	CDirection			m_obAxis;
	float				m_fRadius;
	float				m_fHeight;
};


/***************************************************************************************************
*
*	CLASS			CamVolumeCapsule
*
*	DESCRIPTION		scee.sbashow: todo: a capsule shape
*
***************************************************************************************************/
//class CamVolumeCapsule : public CamVolume
//{
//};


/***************************************************************************************************
*
*	CLASS			CamVolumeSet
*
*	DESCRIPTION		set of volumes, (or negative i.e exclusion volumes)
*
***************************************************************************************************/
class CamVolumeSet : public CamVolume
{
public:
	HAS_INTERFACE(CamVolumeSet)

	CamVolumeSet();
	virtual ~CamVolumeSet();

	// point queries
	virtual bool		Inside(const CPoint& obPos) const;
	virtual bool		ClampToSurf(const CPoint& obPos,const CLAMP_ENV_INFO& obCInfo, CIntersectResults& obResult) const;
	virtual bool		ExClampToSurf(const CPoint& obPos, const CLAMP_ENV_INFO& obCInfo, CIntersectResults& obResult) const;

	// line queries
	virtual bool		Intersect(const CPoint& obStart, const CPoint& obEnd) const;
	virtual bool		GetIntersectAndNormal(const CPoint& obStart, 
											  const CPoint& obEnd,
											  CIntersectResults& obResult,
											  const CamVolumeSet* pobParentSet) const;
	// misc funcs
	virtual void		GetRandomPoint(CPoint& obPos) const;
	virtual CPoint		GetPosition()	const;
	void				AddVolume(CamVolume* pobNewVolume)	 {ntAssert(pobNewVolume); m_obVolumes.push_back(pobNewVolume);	 pobNewVolume->m_bIsAdd=true;	}
	void				AddExVolume(CamVolume* pobNewVolume) {ntAssert(pobNewVolume); m_obExVolumes.push_back(pobNewVolume); pobNewVolume->m_bIsAdd=false;	}

	// debug stuff
	virtual void		Render(bool bParentSelected, bool bSiblingSelected) const;
	virtual	void		Render(float fRed, float fGreen, float fBlue, float fAlpha) const;
	virtual void		RenderInfo(int iX, int iY) const;

	// Wielder events
	virtual void		DebugRender ();
	virtual int 		GetSurfaceID(const CDirection& obLocalNormal ) const	{ UNUSED(obLocalNormal); return -1;}

	bool				IntersectPointValid( const CPoint& obPoint	) const;

private:

	bool	IsPointIncluded(const CPoint& obPos, const CamVolume* pobIgnore = NULL, const CamVolume** ppobResult = NULL ) const;
	bool	IsPointExcluded(const CPoint& obPos, const CamVolume* pobIgnore = NULL, const CamVolume** ppobResult = NULL ) const;

	// ordered insert into the list
	static void InsertResultOnDist(CIntersectResults& obResult, 
								   ntstd::List<CIntersectResults*, Mem::MC_CAMERA>& obList);

	enum ITSCT_MODE
	{
		ITSCT_MODE_NONE,
		ITSCT_MODE_PUSH_EXC,
		ITSCT_MODE_PUSH_INC
	};

	mutable ITSCT_MODE			m_eIntersectMode;
	mutable const CamVolume*	m_pobExcluded;

	void SetIntersectMode(ITSCT_MODE eMode)						const	{	m_eIntersectMode = eMode;	}
	void SetIntersectExclude(const CamVolume*	pobExcluded)	const	{	m_pobExcluded = pobExcluded;	}

	bool CalculateWeightedIntersectionPoint(const ntstd::List< CIntersectResults*, Mem::MC_CAMERA >& obResults, 
								   const CPoint& obRefPoint,
								   float fWeightFactor,
								   CPoint& obWeightedResult) const;

// Welder Attributes
private:
	friend class CamVolumeSetI;
	ntstd::List<CamVolume*, Mem::MC_CAMERA>	m_obVolumes;
	ntstd::List<CamVolume*, Mem::MC_CAMERA>	m_obExVolumes;
};

typedef CamVolume CCamVolume;
typedef CamVolumeBox CCamBox;
typedef CamVolumeSphere CCamSphere;
typedef CamVolumeCylinder CCamCylinder;
typedef CamVolumeSet CCamVolumeSet;

#endif // _CAM_VOLUMES_H
