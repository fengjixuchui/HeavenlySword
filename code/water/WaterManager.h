//--------------------------------------------------
//!
//!	\file WaterManager.h
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#ifndef _WATERMANAGERIMPL_PS3_H_
#define _WATERMANAGERIMPL_PS3_H_

#ifdef PLATFORM_PS3
class SPUProgram;
#endif


class WaterInstance;
struct WaterInstanceDef;

class WaterManager : public Singleton<WaterManager>
{
public:
	WaterManager();
	~WaterManager();

	WaterInstance*  GetNearestWaterInstanceTo( const CPoint& obPoint );
	WaterInstance*	GetWaterInstance( CHashedString obName );
	WaterInstance*	GetWaterInstance( WaterInstanceDef& obDef );
	void			DestroyWaterInstance( WaterInstanceDef& obDef );
	void			DestroyWaterInstance( WaterInstance& obInstance );
	void			DestroyWaterInstance( CHashedString obName );


	void			CreateInstanceAreaResources( WaterInstance& obInstance );
	void			CreateInstanceAreaResources( CHashedString obName );
	void			DestroyInstanceAreaResources( WaterInstance& obInstance );
	void			DestroyInstanceAreaResources( CHashedString obName );

	void	Update( float fTimeStep );

	bool	IsEnabled( void ) const		{ return m_bEnabled; }
	void	Enable( void )				{ m_bEnabled = true; }
	void	Disable( void )				{ m_bEnabled = false; }

	void	DebugRender( void );

private:
	void	SendToSpu( WaterInstance* pobWater );

	void	DebugUpdate( void );

private:
	typedef ntstd::Map<WaterInstanceDef*, WaterInstance*> InstanceMap_t;

	InstanceMap_t				m_obInstanceMap;

#ifdef PLATFORM_PS3
	const SPUProgram*			m_pobSpuProgram;
#endif
	
	bool						m_bEnabled;

	bool						m_bDebugRender;
	bool						m_bRenderPoints;
	bool						m_bRenderBuoys;
	bool						m_bRenderSpuLines;
};




#endif // end of _WATERMANAGERIMPL_PS3_H_
