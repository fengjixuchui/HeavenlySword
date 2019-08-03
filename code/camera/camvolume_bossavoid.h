//------------------------------------------------------------------------------------------
//!
//!	\file camvolume_bossavoid.h
//!
//------------------------------------------------------------------------------------------

#ifndef _CAMVOLUME_BOSSAVOID_H
#define _CAMVOLUME_BOSSAVOID_H

class CamVolBossAvoid
{
	public:
		explicit CamVolBossAvoid( void );
		~CamVolBossAvoid( void );

		CPoint GetPosition( void ) const { return m_obPosition; }
		float GetHeight( void ) const { return m_fHeight; }
		float GetRadius( void ) const { return m_fRadius; }
		float GetFieldStrength( void ) const { return m_fFieldStrength; }

	protected:
		CPoint m_obPosition;
		float m_fHeight;
		float m_fRadius;
		float m_fFieldStrength;

		friend class CamVolBossAvoidInterface;

};

#endif
