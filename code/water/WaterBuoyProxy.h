//--------------------------------------------------
//!
//!	\file WaterBuoyProxy.h
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------


#ifndef _WATERBUOYPROXY_H_
#define _WATERBUOYPROXY_H_



#include "editable/enumlist.h"
#include "tbd/xmlfileinterface.h"


class Transform;
class CEntity;
struct WaveDma;
struct WaterInstanceDef;
struct BuoyDma;
class WaterInstance;


struct BuoyProxy
{
	HAS_INTERFACE( BuoyProxy )

	CDirection		m_obUpDirection;									//!< the up direction. This will roughly equal the water normal at its current position according to buoyancy
    float			m_fBuoyancy;										//!< determines how much influence the water has over the associated transform
	float			m_fTravelSpeed;										//!< the amount this buoy is allowed to move along the water surface
	
	Transform*		m_pobTransform;

	BuoyProxy();
	~BuoyProxy();

	bool			Update( void );										//!< update if active
	bool			IsActive( void ) const;								//!< guess what this does
	void			SetActive( const bool& bState );							//!< an active buoy will take over the associated transform
	
	void			InstallParentWaterInstance( WaterInstance* pobWaterInstance );
	WaterInstance* GetParentWaterInstance( void ) { return m_pobWaterInstance; }

	bool 			EditorChangeValue(CallBackParameter, CallBackParameter);

private:
	friend class WaterInstance;

	WaterInstance*			m_pobWaterInstance;
	BuoyDma*				m_pobBuoyDma;
	bool					m_bActive;
};



#endif // end of _WATERBUOYPROXY_H_
