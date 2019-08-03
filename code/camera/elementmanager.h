//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file elementmanager.h
//!                                                                                         
//------------------------------------------------------------------------------------------

#ifndef ELEMENTMANAGER_INC
#define ELEMENTMANAGER_INC

//------------------------------------------------------------------------------------------
// External Declarations
//------------------------------------------------------------------------------------------
class SceneElementComponent;
class CEntity;


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CamSceneElementList                                                                     
//!	A list of scene element components.                                                     
//!                                                                                         
//------------------------------------------------------------------------------------------
class CamSceneElementList : public Singleton<CamSceneElementList>
{
public:
	CamSceneElementList() {;}
	~CamSceneElementList() {RemoveAll();}

	void AddSceneElement(SceneElementComponent* pElement);
	void RemoveSceneElement(SceneElementComponent* pElement);

// Helper Functions
private:
	void RemoveAll();

// Members
private:
	ntstd::List<SceneElementComponent*> m_elements;

	friend class CamSceneElementMan;
};


//------------------------------------------------------------------------------------------
//!                                                                                         
//!	CamSceneElementMan                                                                      
//!	Determines view framing and POI placement for a CamView                                 
//!                                                                                         
//------------------------------------------------------------------------------------------
class CamSceneElementMan
{
public:
	CamSceneElementMan(const CEntity* pPrimaryEntity, bool bIsPrimaryLocked);
	~CamSceneElementMan() {;}

	// POI Methods
	// -------------
	CPoint            CalcPointOfInterest();
	const CPoint&     GetPointOfInterest()    const {return m_ptPOI;}

	// Zoom Methods
	// --------------
	float		CalcIdealZoom(const CPoint& obSrc, const CPoint& obTarg, float fAspect, float fFOV) const;
	float		CalcPlayerZoom(const CPoint& obSrc, const CPoint& obTarg, float fAspect, float fFOV) const;
	float		CalcCoolCamZoom(const CPoint& obSrc, const CPoint& obTarg, float fAspect, float fFOV,
							    const CEntity* pobAttacker, const CEntity* pobAttackee) const;

	void        SetPrimaryEntity(CEntity* pEnt);
	const SceneElementComponent*  GetPrimaryElement() const	{	return m_pPrimaryElement;	}
	int			CountEnemiesInfluencing() const;

// Members
private:
	CPoint							m_ptPOI;
	float							m_fFrustFOV;

	const SceneElementComponent*         m_pPrimaryElement;
	ntstd::List<SceneElementComponent*>& m_elements;
	bool                                 m_bPrimaryLocked;

public:
	//  Debug Methods
	// ------------------
#ifndef _GOLD_MASTER
	void		RenderDebugInfo();
#endif //_GOLD_MASTER

						
private:
	// These are just kept for displaying as debug info - JML
	mutable bool					m_bRenderPOI;
	mutable bool					m_bRenderFrust;

	float  m_fIntDepth;
	float  m_fIntOffset;
	CPoint m_obZoomConstraintPos;
	float  m_fZoomConstraintRadius;
	bool   m_bZoomX;
};

#endif  // ELEMENTMANAGER_INC

