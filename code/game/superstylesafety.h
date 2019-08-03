#ifndef	_SUPERSTYLESAFETY_H
#define	_SUPERSTYLESAFETY_H

class hkShape;

//------------------------------------------------------------------------------------------
//!
//!	SuperStyleSafetyVolume
//!
//------------------------------------------------------------------------------------------
class SuperStyleSafetyVolume
{
public:
	SuperStyleSafetyVolume();
	virtual ~SuperStyleSafetyVolume();

	virtual void DebugRender (u_long /*ulFrameColour*/,u_long /*ulBodyColour*/) { ntAssert(0); };
	virtual CPoint GetClosestPointInVolumeTo(const CPoint& obPoint) { ntAssert(0); return obPoint; };
	virtual bool IsInside (const CPoint& obPoint) { UNUSED(obPoint); ntAssert(0); return true; };
	
	const CMatrix& GetMatrix() { return m_obMatrix; };
	SuperStyleSafetyVolume* GetContinueVolume() { return m_pobContinueVolume; };
protected:
	CMatrix m_obMatrix;

	SuperStyleSafetyVolume* m_pobContinueVolume;
};

//------------------------------------------------------------------------------------------
//!
//!	SuperStyleSafetyBoxVolume
//!
//------------------------------------------------------------------------------------------
class SuperStyleSafetyBoxVolume : public SuperStyleSafetyVolume
{
	HAS_INTERFACE( SuperStyleSafetyBoxVolume );
public:
	SuperStyleSafetyBoxVolume();
	virtual ~SuperStyleSafetyBoxVolume();
	virtual void PostConstruct();
	virtual void DebugRender(u_long ulFrameColour,u_long ulBodyColour);
	virtual CPoint GetClosestPointInVolumeTo(const CPoint& obPoint);
	virtual bool IsInside (const CPoint& obPoint);
protected:
	CPoint m_obMinPoint, m_obMaxPoint;
	CQuat m_obOrientation; // Special orientation Quat for MrEd integration
	CDirection m_obHalfExtents;
};

//------------------------------------------------------------------------------------------
//!
//!	SuperStyleSafetySphereVolume
//!
//------------------------------------------------------------------------------------------
class SuperStyleSafetySphereVolume : public SuperStyleSafetyVolume
{
	HAS_INTERFACE( SuperStyleSafetySphereVolume );
public:
	SuperStyleSafetySphereVolume();
	virtual ~SuperStyleSafetySphereVolume ();
	virtual void PostConstruct ();
	virtual void DebugRender(u_long ulFrameColour,u_long ulBodyColour);
	virtual CPoint GetClosestPointInVolumeTo(const CPoint& obPoint);
	virtual bool IsInside (const CPoint& obPoint);
protected:
	CPoint m_obPosition;
	float m_fRadius;
};

//------------------------------------------------------------------------------------------
//!
//!	SuperStyleSafetyVolumeCollection
//!
//------------------------------------------------------------------------------------------
class SuperStyleSafetyVolumeCollection
{
	HAS_INTERFACE( SuperStyleSafetyVolumeCollection )
public:
	SuperStyleSafetyVolumeCollection() {};
	~SuperStyleSafetyVolumeCollection() {};
private:
	ntstd::List<SuperStyleSafetyVolume*> m_obStartVolumes, m_obContinueVolumes;

	friend class SuperStyleSafetyManager;
};

//------------------------------------------------------------------------------------------
//!
//!	SuperStyleSafetyManager
//!
//------------------------------------------------------------------------------------------
class SuperStyleSafetyManager : public Singleton<SuperStyleSafetyManager>
{
public:
	SuperStyleSafetyManager();
	~SuperStyleSafetyManager();

	void	SetSuperStyleSafetyVolumeCollection(SuperStyleSafetyVolumeCollection* pobCollection); // Each sector should set this

	SuperStyleSafetyVolume*	PointInSuperStyleStartVolume(const CPoint& obPoint);// Is obPoint in a SSV?
	SuperStyleSafetyVolume*	PointInSuperStyleContinueVolume(const CPoint& obPoint); // Is obPoint in a SCV?
	CPoint	GetSuperStyleSafeContinuePoint(const CPoint& obPoint, SuperStyleSafetyVolume* pobStartVolume = 0 ); // From obPoint, return the closest point in a SCV

	CPoint	GetLastStartPoint() { return m_obLastStartPoint; };
	CPoint	GetLastHitPoint() { return m_obLastHitPoint; };

	void	DebugRender();
private:
	CPoint m_obLastStartPoint, m_obLastHitPoint;

	SuperStyleSafetyVolumeCollection* m_pobCurrentVolumeCollection;
};

#endif // _SUPERSTYLESAFETY_H
