//--------------------------------------------------
//!
//!	\file EyeBlinker.cpp
//!	This is a dummy holding a few doxygen comments.
//!
//--------------------------------------------------

#include "blendshapes/anim/EyeBlinker.h"
#include "game/entity.h"
#include "game/entity.inl"
#include "blendshapes/blendshapescomponent.h"
#include "blendshapes/anim/bsanimator.h"
#include "anim/animator.h"
#include "anim/animation.h"
#include "core/exportstruct_anim.h"
#include "objectdatabase/dataobject.h"

const static float BLINK_MULT = 0.1f / RAND_MAX ;

START_CHUNKED_INTERFACE( EyeBlinkerDef, Mem::MC_PROCEDURAL )
	PUBLISH_VAR_AS(					m_obAnimShortName,						BlinkAnimation )
	PUBLISH_VAR_WITH_DEFAULT_AS(	m_fBlinkInterval,			0.1f,		BlinkInterval )
	PUBLISH_VAR_WITH_DEFAULT_AS(	m_bEnabled,					true,		Enabled )	
	PUBLISH_VAR_WITH_DEFAULT_AS(	m_fBlendWeight,				1.0f,		BlendWeight )	
	PUBLISH_VAR_WITH_DEFAULT_AS(	m_fRandomness,				0.4f,		Randomness )	
END_STD_INTERFACE


LUA_EXPOSED_START( EyeBlinker )
	LUA_EXPOSED_METHOD( Enable, Enable, "", "", "" )
	LUA_EXPOSED_METHOD( Disable, Disable, "", "", "" )
	LUA_EXPOSED_METHOD( Blink, Blink, "", "", "" )
LUA_EXPOSED_END( EyeBlinker )


EyeBlinker::EyeBlinker( CEntity* pobEnt, EyeBlinkerDef* pobDef )
: CAnonymousEntComponent( "EyeBlinker" )
, m_pobEnt( pobEnt )
, m_pobDef( pobDef )
, m_fTimeSinceLastBlink( 0.0f )
{
	ATTACH_LUA_INTERFACE(EyeBlinker);

	ntError( m_pobEnt );
	ntError( m_pobDef );

	//// create and add our blinking animation
	//m_pobBlinkAnimation =  m_pobEnt->GetAnimator()->CreateAnimation( m_pobDef->m_obAnimShortName );
	//m_pobBlinkAnimation->SetBlendWeight( 0.0f );
	//m_pobBlinkAnimation->SetFlagBits( ANIMF_INHIBIT_AUTO_DESTRUCT );
	//m_pobEnt->GetAnimator()->AddAnimation( m_pobBlinkAnimation );	

	user_warn_msg(("%s: Has EyeBlinker component. This has been deprecated. Please use the Blink animation slot in Walk/Strafe controllers\n", ntStr::GetString(m_pobEnt->GetName())));
}

EyeBlinker::~EyeBlinker()
{
	//if ( m_pobBlinkAnimation )
	//	m_pobEnt->GetAnimator()->RemoveAnimation( m_pobBlinkAnimation );
}

void EyeBlinker::Update( float fTimeStep )
{
	UNUSED( fTimeStep );
	//if ( IsEnabled() )
	//{
	//	m_fTimeSinceLastBlink +=fTimeStep;

	//	// force blend weight if still playing the blinking animation
	//	if ( m_pobBlinkAnimation->GetTime() < m_pobBlinkAnimation->GetDuration() )
	//	{
	//		m_pobBlinkAnimation->SetBlendWeight( 1.0f );
	//	}
	//	else
	//	{
	//		// if not, make sure is set to zero
	//		m_pobBlinkAnimation->SetBlendWeight( 0.0f );
	//		
	//		// if it's time to blink again, reset the anim so its weight can be forced back to one 
	//		// and reset the timer
	//		if ( m_fTimeSinceLastBlink > m_pobDef->m_fBlinkInterval )
	//		{
	//			m_pobBlinkAnimation->SetTime( 0.0f );
	//			m_fTimeSinceLastBlink = m_pobDef->m_fBlinkInterval * m_pobDef->m_fRandomness * rand() / RAND_MAX;
	//		}
	//	}
	//}
}

void EyeBlinker::Blink( void )
{

}	
