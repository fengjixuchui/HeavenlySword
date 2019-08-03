/***************************************************************************************************
*
*	$Header:: /game/tcdebugupdate.cpp	     17/02/2005 Andrew Vidler                             $
*
*
*
*	CHANGES
*
*	17/02/2005	Created by Andrew.
*
***************************************************************************************************/

#include "anim/transform.h"
#include "input/inputhardware.h"
#include "game/tcdebugupdate.h"

#ifndef PLATFORM_PS3
#	include "game/entitymanager.h"
#endif

namespace TC_DebugUpdate 
{
#	include "TCDU_Standard.inl"
#	include "TCDU_ExtraSpecial.inl"


// moved here GCC fundementally doesn't really like inlines... in release it won't link
void ExtraSpecial::Update( PAD_NUMBER ePad, float fMoveFactor, float fRotateFactor, CPoint &ptPosition, Transform *pTransform, float &fPhi, float &fTheta )	const
{
	const CInputPad &pad( CInputHardware::Get().GetPad( ePad ) );

	// Make things a teeny bit faster.
	fMoveFactor *= 1.15f;

	// Increase speed by a factor of 5 if this button is held
	if ( pad.GetHeld() & PAD_LEFT_THUMB )
	{
		fMoveFactor *= 5.0f;
	}

	bool was_top2_held = m_btop2Held;
	if ( pad.GetHeld() & PAD_TOP_2 )
	{
		m_btop2Held = true;
	}
	else
	{
		m_btop2Held = false;
	}

	if ( was_top2_held && !m_btop2Held )
	{
		m_bGoSlow = !m_bGoSlow;
	}

	if ( m_bGoSlow )
	{
		fMoveFactor *= 0.025f;
	}

	// update the position and orientation from the pad analog sticks
	CDirection obRight( -fMoveFactor * pad.GetAnalogRXFrac() * pTransform->GetWorldMatrix().GetXAxis() );
	
	CDirection	obForwards	( CONSTRUCT_CLEAR );
	CDirection	obUp		( CONSTRUCT_CLEAR );
	float		fTilt		( 0.0f );

	CDirection lookat_2d( pTransform->GetWorldMatrix().GetZAxis() );
	lookat_2d.Y() = 0.0f;
	obForwards = -fMoveFactor * pad.GetAnalogLYFrac() * lookat_2d;

	// if Y isn't held then move up, otherwise rotate up/down.
	if( ( pad.GetHeld() & PAD_TOP_1 ) != 0 )
	{
		fTilt = fRotateFactor * pad.GetAnalogRYFrac();
	}
	else
	{
		obUp = -fMoveFactor * pad.GetAnalogRYFrac() * CDirection( 0.0f, 1.0f, 0.0f );
	}

	ptPosition += obForwards + obRight + obUp;
		
	float fTurn( -fRotateFactor * pad.GetAnalogLXFrac() );

	fPhi += fTurn;
	while( fPhi < 0 )
	{
		fPhi += 2*PI;
	}

	while( fPhi > 2*PI )
	{
		fPhi -= 2*PI;
	}

	fTheta += fTilt;

#ifndef PLATFORM_PS3
	if ( CInputHardware::Get().GetKeyboard().IsKeyReleased( KEYC_S, KEYM_SHIFT|KEYM_CTRL ) )
	{
		File fp( "c:\\camera_pos.sav", File::FT_WRITE | File::FT_BINARY );
		if ( fp.IsValid() )
		{
			fp.Write( &ptPosition, sizeof( CPoint ) );
			fp.Write( &fTheta, sizeof( float ) );
			fp.Write( &fPhi, sizeof( float ) );

#			ifndef _RELEASE
				ntPrintf( "Saved camera settings." );
#			endif
		}
	}
	if ( CInputHardware::Get().GetKeyboard().IsKeyReleased( KEYC_R, KEYM_SHIFT|KEYM_CTRL ) )
	{
		File fp( "c:\\camera_pos.sav", File::FT_READ | File::FT_BINARY );
		if ( fp.IsValid() )
		{
			fp.Read( &ptPosition, sizeof( CPoint ) );
			fp.Read( &fTheta, sizeof( float ) );
			fp.Read( &fPhi, sizeof( float ) );

#			ifndef _RELEASE
				ntPrintf( "Restored camera settings." );
#			endif
		}
	}

	if ( ( pad.GetReleased() & PAD_FACE_3 ) != 0 )
	{
		m_FixHeroineToCamera = !m_FixHeroineToCamera;

		if ( m_FixHeroineToCamera )
		{
			m_VectorToHeroine_CameraS = CDirection( 0.0f, 0.0f, 5.0f );
			CEntity *player = CEntityManager::Get().GetPlayer();
			if ( player != NULL && pTransform != NULL )
			{
				CDirection to_heroine_worldS( player->GetPosition() ^ ptPosition );
				m_VectorToHeroine_CameraS = CDirection( to_heroine_worldS.Dot( pTransform->GetWorldMatrix().GetXAxis() ),
														to_heroine_worldS.Dot( pTransform->GetWorldMatrix().GetYAxis() ),
														to_heroine_worldS.Dot( pTransform->GetWorldMatrix().GetZAxis() ) );
			}
		}
	}

	if ( m_FixHeroineToCamera )
	{
		CEntity *player = CEntityManager::Get().GetPlayer();
		if ( player != NULL && pTransform != NULL )
		{
			CDirection to_heroine_worldS =	m_VectorToHeroine_CameraS.X() * pTransform->GetWorldMatrix().GetXAxis() +
											m_VectorToHeroine_CameraS.Y() * pTransform->GetWorldMatrix().GetYAxis() +
											m_VectorToHeroine_CameraS.Z() * pTransform->GetWorldMatrix().GetZAxis();
			player->SetPosition( ptPosition + to_heroine_worldS );
		}
	}
#endif // !PLATFORM_PS3
}

}


TransformControllerDebugUpdate *TransformControllerDebugUpdate::Create( TC_DEBUG_UPDATE_MODE mode )
{
	switch ( mode )
	{
		case TCDUM_STANDARD:
		{
			return NT_NEW TC_DebugUpdate::Standard();
		}

		case TCDUM_EXTRA_SPECIAL:
		{
			return NT_NEW TC_DebugUpdate::ExtraSpecial();
		}
		default:
			ntAssert_p( false, ("Unknown transform controller debug update") );
	}

	return NULL;
}

void TransformControllerDebugUpdate::Destroy( TransformControllerDebugUpdate *du )
{
	NT_DELETE( du );
}
