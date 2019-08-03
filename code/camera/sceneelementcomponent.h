//------------------------------------------------------------------------------------------
//!                                                                                         
//!	\file sceneelementcomponent.h
//!                                                                                         
//------------------------------------------------------------------------------------------


#ifndef _SCENEELEMENTCOMPONENT_INC
#define _SCENEELEMENTCOMPONENT_INC


//------------------------------------------------------------------------------------------
// Required Includes
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
// External Declarations
//------------------------------------------------------------------------------------------
class CEntity;

#include "lua\ninjalua.h"

//------------------------------------------------------------------------------------------
//!                                                                                         
//!	SceneElementComponentDef                                                                
//!	Definition of a SceneElementComponent.                                                  
//!                                                                                         
//------------------------------------------------------------------------------------------
class SceneElementComponentDef
{
public:
	float				m_fImportance;
	float				m_fRadius;
	CDirection			m_obOffset;
	float				m_fInfluenceRadiusIn;
	float				m_fInfluenceRadiusOut;
	float				m_fLookAheadTime;
};



//------------------------------------------------------------------------------------------
//!                                                                                         
//!	SceneElementComponent                                                                
//!	Camera interest information for an entity.                                                  
//!                                                                                         
//------------------------------------------------------------------------------------------
class SceneElementComponent
{
public:
	// Construction Destruction
	SceneElementComponent(CEntity* pEnt, const SceneElementComponentDef* pDef);
	virtual ~SceneElementComponent();

	HAS_LUA_INTERFACE()

	void Update(float fTimeDelta);

//  Accessors
//------------------------------------------------
public:
	float          GetImportance() const          {return m_fImportance;}
	float          GetRadius() const              {return m_fRadius;}
	CPoint         GetPosition() const;
	CDirection     GetVelocity() const;
	float          GetLookAheadTime() const       {return m_fLookAheadTime;}
	CDirection     GetLookAheadDirection() const;
	const CEntity* GetEntity() const              {return m_pobEntity;}
	float          GetVariableImportance() const  {return m_fImportance * m_fInfluencePercentage;}

	void SetImportance(float fImportance);
	void SetRadius(float fRadius)				    {m_fRadius = fRadius;}
	void SetOffset(const CDirection& dirOff)	    {m_dirOffset = dirOff;}
	void SetInfluenceRadius(float fIn, float fOut)  {m_fInfluenceRadiusIn = fIn; m_fInfluenceRadiusOut = fOut;}
	void SetLookAheadTime(float fTime)		        {m_fLookAheadTime = fTime;} // Now unused - Might remove later...

// Influence Checks
//------------------------------------------------
public:
	enum INFLUENCE_VALUE
	{
		INFLUENCE_NONE = 0,
		INFLUENCE_POI,
		INFLUENCE_FULL
	};

	enum INFLUENCE_VALUE IsInfluencing(CPoint& obPt);

// Members
//------------------------------------------------
private:
	float            m_fImportance;          // The importance of the element (0.0f -> 1.0f)
	float            m_fTargetImportance;    // The target imporance of this element.
	float            m_fImportanceBlendTime; // Blend, don't snap.
	float            m_fOriginalImportance;
	float            m_fRadius;              // Rough size of the element for zoom framing.
	CDirection       m_dirOffset;            // Offset to centre of element from it's root matrix.
	float            m_fInfluenceRadiusIn;   // Proximity to player for element to become interesting... 
	float            m_fInfluenceRadiusOut;  // ... with overlap
	INFLUENCE_VALUE  m_eIsInfluencing;       // Was it influencing last check?
	float            m_fLookAheadTime;       // How far ahead in time to look // Now unused - Might remove later...
	mutable CEntity* m_pobEntity;            // The associated entity
	float            m_fInfluencePercentage; // 100% = within In-Radius, 0% at boundary of Out-Radius

// Debug Functions
//------------------------------------------------
public:
	void SetRGB(float fR, float fG, float fB) {m_fR = fR; m_fB = fB; m_fG = fG;}

public:
	float				m_fR, m_fG, m_fB;
};

LV_DECLARE_USERDATA(SceneElementComponent);

#endif // _SCENEELEMENTCOMPONENT_INC
