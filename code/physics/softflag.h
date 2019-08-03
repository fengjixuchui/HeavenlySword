//------------------------------------------------------------------------------------------
//!
//! softflag.h
//! 
//!
//------------------------------------------------------------------------------------------
#ifndef _SOFT_FLAG_H
#define _SOFT_FLAG_H

// ---- Forward declarations
class CEntity;
class GlobalWind;
class SoftMaterial;
class VerletInstance;

//------------------------------------------------------------------------------------------
//!
//! Template_Flag
//! Description here...
//!
//------------------------------------------------------------------------------------------
class Template_Flag
{
public:	
	HAS_INTERFACE( Template_Flag );

	// Const/dest
	Template_Flag();
	~Template_Flag();

	// Soft material accessor
	SoftMaterial*	GetSoftMaterial() {return m_pobSoftMaterial;}

	// Callback for values changed in editor
	bool EditorChangeValue(CallBackParameter pcItem, CallBackParameter pcValue);

	// Accessors
	float GetSizeX() const {return m_fSizeX;}
	float GetSizeY() const {return m_fSizeY;}
	int GetResolutionX() const {return m_uiResolutionX;}
	int GetResolutionY() const {return m_uiResolutionY;}
	GlobalWind* GetWind() const {return m_pobGlobalWind;}
	const char* GetTexture() const {return ntStr::GetString(m_obTexture);}
	const char* GetNormalMap() const {return ntStr::GetString(m_obNormalMap);}

	void SetPosition( const CPoint& pos );
	CPoint GetPosition( void ) const;

	void SetOrientation( const CQuat& orient );
	CQuat GetOrientation( void ) const;

	void SetParentEntity( CEntity* pParent, CKeyString transform );
	CEntity* GetParentEntity() const;

	void SetParentTransform( Transform* pParent );

protected:
	// PostConstruct call
	void PostConstruct();
	void PostPostConstruct();

	// Parameters
	CKeyString	m_obConstructionScript;		
	CVector 	m_obPosition;           	
	CQuat 		m_obOrientation;        	
	CEntity*	m_pobParentEntity;       	
	CKeyString 	m_obParentTransform;    	
	uint32_t 	m_uiResolutionX;				
	uint32_t 	m_uiResolutionY;			
	float 		m_fSizeX;					
	float 		m_fSizeY;					
	ntstd::String	m_obTexture;             	
	ntstd::String	m_obNormalMap;              
	GlobalWind*	m_pobGlobalWind;		
	SoftMaterial*	m_pobSoftMaterial;

	Physics::VerletInstance* m_pobVerlet;
	Transform	m_obTransform;
	bool		m_bDonePostPostConstruct;
};

#endif // _SOFT_FLAG_H
