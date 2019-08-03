//! --------------------------------------------------------
//! AINavigGraphMan.h
//!
//! New AIs Navigation Grpah Manager
//!
//! It handles AIs navigation system for the whole level 
//!
//! Author: Dario Sancho-Pradel (DSP)
//!
//!--------------------------------------------

#ifndef _AINAVIGGRAPHMAN_H
#define _AINAVIGGRAPHMAN_H

#include "ainavigastar.h"
#include "ainaviggraph.h"
#include "lua/ninjalua.h"

//#define NGMAN_MAX_DIST_TO_NODE (5000.0f)

// Forward declarations

//typedef struct _SNavigGraphLink SNavigGraphLink;

class CAINavigGraphMan : public Singleton<CAINavigGraphMan>
{

	public:
		// Ctor et al.

		CAINavigGraphMan();
		CAINavigGraphMan::~CAINavigGraphMan();

		HAS_LUA_INTERFACE()

		void DebugRender();

		// Methods
		// Note: Enable means [DOOR -> Open, LADDER -> In place]
		CAINavigNode*						GetNode				( CEntity*, CAINavigGraph* );
		bool								IsDoorOpen			( CEntity* );
		bool								IsLadderAvailable	( CEntity* );
		bool								SetEnableDoor		( CEntity*, bool );
		bool								SetEnableLadder		( CEntity*, bool );
		bool								DeleteNode			( CEntity* );
//		bool								IsDoor				( CEntity* pEnt) { return ( GetNodeType(pEnt) == NDT_DOOR ); }
//		bool								IsLadder			( CEntity* pEnt) { return ( GetNodeType(pEnt) == NDT_LADDER ); }
		CAINavigNode*						GetClosestNode		( CPoint * , float* fDist  );
		const AINavigArrowList*	GetTgtArrows		( CAINavigNode* ) const;
		CAINavigGraph*						GetNavigGraphArea	( const char* psAreaName );
		bool								IsDebugRenderOn		( void ) const	{ return m_bRender; }
		void								Add					( CAINavigGraph* pNG);
		void								LevelUnload			( void );

		void								MakePath			( float x1, float y1, float z1, float x2, float y2, float z2 ); // !!! - Temporary
		//void								PrintPath			( void ); // !!! - Temporary
		void			Update( float fTimeChange );
		

	private:
		
		CAINavigArrow* GetLadder	( CEntity* pEnt );
		CAINavigArrow* GetDoor		( CEntity* pEnt );

	public:

		bool						m_bRender;

	private:
		
		AINavigGraphList			m_listNavigGraphs;
		CNavigAStar					m_obAStar; // !!! - Temporary
		int							m_iNumberOfRegionsInLevel;
};

LV_DECLARE_USERDATA(CAINavigGraphMan);

#endif // _AINAVIGGRAPHMAN_H


