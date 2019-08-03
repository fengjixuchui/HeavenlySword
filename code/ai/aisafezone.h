//------------------------------------------------------------------------------------------
//!
//!	\file aisafezone.h
//!
//------------------------------------------------------------------------------------------

#ifndef __AI_SAFEZONE__
#define __AI_SAFEZONE__

class CAIComponent;

//------------------------------------------------------------------------------------------
//!
//!	AISafeZone
//!	Describes a convex area in a level where an AI can safely move.
//!
//------------------------------------------------------------------------------------------
class AISafeZone
{
public:
	
	HAS_INTERFACE( AISafeZone );

	AISafeZone()	{};
	AISafeZone( const CPoint& centre, const float radius ) : m_obCentre( centre ), m_fRadius( radius )	{};

	void PostConstruct();

	void AddPoint( const CPoint& point );

	// accessors
	void SetCentre( const CPoint& centre );
	void SetRadius( const float radius );

	// testing against the zone
	bool InZone( const CPoint& pos, const float radius = 0.0f ) const;

	// debugging
	void PaulsDebugRender() const;


	// public data (for serialisation)
	CPoint	m_obCentre;
	float	m_fRadius;

private:

		
};




#endif //__AI_SAFEZONE__
