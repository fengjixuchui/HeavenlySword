//--------------------------------------------------
//!
//!	\file WaterInstanceDef.h
//!	Water instance soft definition
//!
//--------------------------------------------------

#ifndef _WATERINSTANCEDEF_H_
#define _WATERINSTANCEDEF_H_


#include "anim/transform.h"
#include "tbd/xmlfileinterface.h"



struct WaveEmitter;
class WaterInstance;
struct WaterInstanceDef;
struct BuoyProxy;





struct WaterInstanceDef
{
	HAS_INTERFACE( WaterInstanceDef )

	ntstd::List<WaveEmitter*>	m_obEmitters;
	ntstd::List<BuoyProxy*>		m_obBuoyProxies;

	// 
	// Transform data
	//
	CPoint			m_obPos;							//!< world position
	CQuat			m_obRot;							//!< world rotation
	float			m_fVScale;							//!< global vertical scale. Affects all waves and attacks 


	//////////////////////////////////////////
	// changing these, invalidates dma data //
	// so don't do it outside debug as it	//
	// will have no effect					//
	//////////////////////////////////////////
	float			m_fWidth;							//!< surface width (X)
	float			m_fLength;							//!< surfave length (Z)
	float			m_fResolution;						//!< grid resolution
	//////////////////////////////////////////

	//
	// Rendering parameters
	//
	ntstd::String	m_obNormalMap0;
	ntstd::String	m_obNormalMap1;
	CVector			m_obMapSize;						//!< < u0,v0,u1,v1 >
	CVector			m_obMapSpeed;						//!< wind speed affects normal maps only < x0, z0, x1, z1 >
	CVector			m_obBaseColour;						//!< self-explanatory;
	float			m_fFresnelStrength;					//!< self-explanatory
	float			m_fReflectivity;					//!< self-explanatory
	float			m_fSpecularPower;					//!< self-explanatory
	
	// 
	// Misc
	//
	int				m_iSectorBits;						//! area resource sector bits


	WaterInstanceDef();
	~WaterInstanceDef();

	void PostConstruct( void );
	void PostPostConstruct( void );
	bool EditorChangeValue(CallBackParameter, CallBackParameter);

private:
	friend class WaterInstance;
	Transform		m_obTransform;
	WaterInstance*	m_pobInstance;
};






#endif // end of _WATERINSTANCEDEF_H_
